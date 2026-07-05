idf_component_register(SRCS "app_main.cpp" "web_server.cpp" "telemetry.cpp"
                    INCLUDE_DIRS "." "../json/"
                    REQUIRES esp_http_server esp_wifi esp_eth esp_event esp_netif nvs_flash
                             protocol_examples_common
                    EMBED_FILES "ws_test.html")
