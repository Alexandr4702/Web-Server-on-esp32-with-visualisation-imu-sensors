idf_component_register(SRCS "app_main.cpp" "web_server.cpp" "telemetry.cpp"
                             "mpu6050_source.cpp"
                             "../esp32-mpu6050/mpu6050.c"
                             "../esp32-i2c_rw/esp32_i2c_rw.c"
                    INCLUDE_DIRS "." "../json/" "../esp32-mpu6050/include/"
                                 "../esp32-i2c_rw/include/"
                    REQUIRES esp_http_server esp_wifi esp_eth esp_event esp_netif nvs_flash
                             protocol_examples_common
                    EMBED_FILES "ws_test.html")
