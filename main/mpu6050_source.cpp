#include "mpu6050_source.hpp"

#include "driver/i2c.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern "C" {
#include "mpu6050/mpu6050.h"
}

namespace {

constexpr char kTag[] = "mpu6050";
constexpr i2c_port_t kI2cPort = I2C_NUM_0;
constexpr gpio_num_t kSdaPin = GPIO_NUM_21;
constexpr gpio_num_t kSclPin = GPIO_NUM_22;
constexpr double kRawUnitsPerG = 16384.0;

}  // namespace

namespace telemetry {

esp_err_t Mpu6050Source::initialize() {
    if (initialized_) {
        return ESP_OK;
    }

    i2c_config_t config = {};
    config.mode = I2C_MODE_MASTER;
    config.sda_io_num = kSdaPin;
    config.scl_io_num = kSclPin;
    config.sda_pullup_en = GPIO_PULLUP_ENABLE;
    config.scl_pullup_en = GPIO_PULLUP_ENABLE;
    config.master.clk_speed = 400000;

    esp_err_t result = i2c_param_config(kI2cPort, &config);
    if (result != ESP_OK) {
        return result;
    }
    result = i2c_driver_install(kI2cPort, config.mode, 0, 0, 0);
    if (result != ESP_OK && result != ESP_ERR_INVALID_STATE) {
        return result;
    }

    mpu6050_init();
    mpu6050_set_i2c_master_mode_enabled(true);
    mpu6050_reset_sensors();
    mpu6050_set_fifo_enabled(true);
    initialized_ = mpu6050_test_connection();

    if (!initialized_) {
        ESP_LOGW(kTag, "MPU6050 did not respond; samples will be zero");
        return ESP_OK;
    }
    ESP_LOGI(kTag, "MPU6050 connected on I2C address 0x%02x",
             mpu6050_device_address);
    return ESP_OK;
}

Sample Mpu6050Source::read() {
    const double uptime_seconds =
        static_cast<double>(xTaskGetTickCount()) * portTICK_PERIOD_MS / 1000.0;
    if (!initialized_) {
        return {uptime_seconds, {0.0, 0.0, -500.0}, {1.0, 0.0, 0.0, 0.0}};
    }

    mpu6050_acceleration_t acceleration = {};
    mpu6050_get_acceleration(&acceleration);
    return {
        uptime_seconds,
        {
            acceleration.accel_x / kRawUnitsPerG * 100.0,
            acceleration.accel_y / kRawUnitsPerG * 100.0,
            -500.0 + acceleration.accel_z / kRawUnitsPerG * 100.0,
        },
        {1.0, 0.0, 0.0, 0.0},
    };
}

}  // namespace telemetry
