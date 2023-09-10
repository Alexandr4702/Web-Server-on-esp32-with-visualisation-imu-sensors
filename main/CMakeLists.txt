idf_component_register(SRCS "ws_echo_server.cpp" "../esp32-mpu6050/mpu6050.c" "../esp32-i2c_rw/esp32_i2c_rw.c"
                    INCLUDE_DIRS "." "../json/" "../esp32-mpu6050/include/" "../esp32-i2c_rw/include/"
                    EMBED_FILES "ws_test.html")
