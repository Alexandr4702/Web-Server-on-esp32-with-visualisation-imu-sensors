#pragma once

#include "esp_err.h"
#include "telemetry.hpp"

namespace telemetry {

class Mpu6050Source final : public Source {
public:
    esp_err_t initialize();
    Sample read() override;

private:
    bool initialized_ = false;
};

}  // namespace telemetry
