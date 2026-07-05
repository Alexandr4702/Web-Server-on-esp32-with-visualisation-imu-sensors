#include "web_server.hpp"
#include "mpu6050_source.hpp"

#include "esp_event.h"
#include "esp_eth.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"

namespace {

constexpr char kTag[] = "imu_app";
telemetry::Mpu6050Source telemetry_source;
WebServer web_server(telemetry_source);

void on_connected(void *context, esp_event_base_t, int32_t, void *) {
    auto &server = *static_cast<WebServer *>(context);
    if (server.start() != ESP_OK) {
        ESP_LOGE(kTag, "Failed to start web server");
    }
}

void on_disconnected(void *context, esp_event_base_t, int32_t, void *) {
    static_cast<WebServer *>(context)->stop();
}

}  // namespace

extern "C" void app_main(void) {
    esp_err_t nvs_result = nvs_flash_init();
    if (nvs_result == ESP_ERR_NVS_NO_FREE_PAGES ||
        nvs_result == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        nvs_result = nvs_flash_init();
    }
    ESP_ERROR_CHECK(nvs_result);
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(telemetry_source.initialize());
    ESP_ERROR_CHECK(example_connect());

#ifdef CONFIG_EXAMPLE_CONNECT_WIFI
    ESP_ERROR_CHECK(esp_event_handler_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &on_connected, &web_server));
    ESP_ERROR_CHECK(esp_event_handler_register(
        WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &on_disconnected, &web_server));
#endif
#ifdef CONFIG_EXAMPLE_CONNECT_ETHERNET
    ESP_ERROR_CHECK(esp_event_handler_register(
        IP_EVENT, IP_EVENT_ETH_GOT_IP, &on_connected, &web_server));
    ESP_ERROR_CHECK(esp_event_handler_register(
        ETH_EVENT, ETHERNET_EVENT_DISCONNECTED, &on_disconnected, &web_server));
#endif

    // example_connect() returns only after the first connection is established.
    ESP_ERROR_CHECK(web_server.start());
}
