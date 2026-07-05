#include "web_server.hpp"

#include <string>
#include <vector>

#include "esp_log.h"

namespace {

constexpr char kTag[] = "web_server";
constexpr size_t kMaxClients = 8;
constexpr TickType_t kPublishPeriod = pdMS_TO_TICKS(50);

}  // namespace

WebServer::WebServer(telemetry::Source &telemetry_source)
    : telemetry_source_(telemetry_source) {}

WebServer::~WebServer() {
    stop();
}

bool WebServer::running() const {
    return server_ != nullptr;
}

esp_err_t WebServer::serve_index(httpd_req_t *request) {
    extern const unsigned char ws_test_html_start[]
        asm("_binary_ws_test_html_start");
    extern const unsigned char ws_test_html_end[]
        asm("_binary_ws_test_html_end");

    const size_t size = ws_test_html_end - ws_test_html_start;
    httpd_resp_set_type(request, "text/html; charset=utf-8");
    return httpd_resp_send(
        request, reinterpret_cast<const char *>(ws_test_html_start), size);
}

esp_err_t WebServer::handle_websocket(httpd_req_t *request) {
    if (request->method == HTTP_GET) {
        ESP_LOGI(kTag, "WebSocket client connected");
        return ESP_OK;
    }

    httpd_ws_frame_t frame = {};
    frame.type = HTTPD_WS_TYPE_TEXT;
    esp_err_t result = httpd_ws_recv_frame(request, &frame, 0);
    if (result != ESP_OK || frame.len == 0) {
        return result;
    }

    std::vector<uint8_t> payload(frame.len + 1, 0);
    frame.payload = payload.data();
    result = httpd_ws_recv_frame(request, &frame, frame.len);
    if (result != ESP_OK) {
        ESP_LOGW(kTag, "Failed to receive WebSocket frame: %s",
                 esp_err_to_name(result));
        return result;
    }

    return httpd_ws_send_frame(request, &frame);
}

void WebServer::publisher_entry(void *context) {
    static_cast<WebServer *>(context)->publish_loop();
}

void WebServer::publish_loop() {
    int clients[kMaxClients];

    while (true) {
        const std::string payload =
            telemetry::to_json(telemetry_source_.read());
        size_t client_count = kMaxClients;

        if (server_ != nullptr &&
            httpd_get_client_list(server_, &client_count, clients) == ESP_OK) {
            for (size_t index = 0; index < client_count; ++index) {
                if (httpd_ws_get_fd_info(server_, clients[index]) !=
                    HTTPD_WS_CLIENT_WEBSOCKET) {
                    continue;
                }

                httpd_ws_frame_t frame = {};
                frame.final = true;
                frame.type = HTTPD_WS_TYPE_TEXT;
                frame.payload = reinterpret_cast<uint8_t *>(
                    const_cast<char *>(payload.data()));
                frame.len = payload.size();

                const esp_err_t result =
                    httpd_ws_send_frame_async(server_, clients[index], &frame);
                if (result != ESP_OK) {
                    ESP_LOGW(kTag, "Failed to publish telemetry: %s",
                             esp_err_to_name(result));
                }
            }
        }
        vTaskDelay(kPublishPeriod);
    }
}

esp_err_t WebServer::start() {
    if (running()) {
        return ESP_OK;
    }

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    ESP_LOGI(kTag, "Starting HTTP server on port %d", config.server_port);
    esp_err_t result = httpd_start(&server_, &config);
    if (result != ESP_OK) {
        server_ = nullptr;
        return result;
    }

    const httpd_uri_t index_route = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = serve_index,
        .user_ctx = this,
        .is_websocket = false,
    };
    const httpd_uri_t websocket_route = {
        .uri = "/ws",
        .method = HTTP_GET,
        .handler = handle_websocket,
        .user_ctx = this,
        .is_websocket = true,
    };

    result = httpd_register_uri_handler(server_, &index_route);
    if (result == ESP_OK) {
        result = httpd_register_uri_handler(server_, &websocket_route);
    }
    if (result != ESP_OK) {
        stop();
        return result;
    }

    if (xTaskCreate(publisher_entry, "telemetry", 4096, this,
                    tskIDLE_PRIORITY + 2, &publisher_task_) != pdPASS) {
        stop();
        return ESP_ERR_NO_MEM;
    }
    return ESP_OK;
}

void WebServer::stop() {
    if (publisher_task_ != nullptr) {
        vTaskDelete(publisher_task_);
        publisher_task_ = nullptr;
    }
    if (server_ != nullptr) {
        httpd_stop(server_);
        server_ = nullptr;
    }
}
