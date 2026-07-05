#pragma once

#include <array>
#include <string>

namespace telemetry {

struct Sample {
    double uptime_seconds;
    std::array<double, 3> translation;
    std::array<double, 4> quaternion;
};

class Source {
public:
    virtual ~Source() = default;
    virtual Sample read() = 0;
};

class DemoSource final : public Source {
public:
    Sample read() override;
};

std::string to_json(const Sample &sample);

}  // namespace telemetry
