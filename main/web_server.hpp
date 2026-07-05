#pragma once

#include "esp_err.h"
#include "esp_http_server.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "telemetry.hpp"

class WebServer final {
public:
    explicit WebServer(telemetry::Source &telemetry_source);
    ~WebServer();

    WebServer(const WebServer &) = delete;
    WebServer &operator=(const WebServer &) = delete;

    esp_err_t start();
    void stop();
    bool running() const;

private:
    static esp_err_t serve_index(httpd_req_t *request);
    static esp_err_t handle_websocket(httpd_req_t *request);
    static void publisher_entry(void *context);

    void publish_loop();

    telemetry::Source &telemetry_source_;
    httpd_handle_t server_ = nullptr;
    TaskHandle_t publisher_task_ = nullptr;
};
