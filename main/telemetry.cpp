#include "telemetry.hpp"

#include <cmath>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "single_include/nlohmann/json.hpp"

namespace telemetry {

Sample DemoSource::read() {
    const double uptime_seconds =
        static_cast<double>(xTaskGetTickCount()) * portTICK_PERIOD_MS / 1000.0;

    return {
        uptime_seconds,
        {std::sin(uptime_seconds) * 100.0, 0.0, -500.0},
        {1.0, 0.0, 0.0, 0.0},
    };
}

std::string to_json(const Sample &sample) {
    nlohmann::json document;
    document["uptime"] = sample.uptime_seconds;
    document["translation"] = sample.translation;
    document["quaternion"] = sample.quaternion;
    return document.dump();
}

}  // namespace telemetry
