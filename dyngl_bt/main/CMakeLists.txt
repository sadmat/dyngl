idf_component_register(SRCS "dyngl.c" "dyngl_bt.c" "dyngl_spi.c" "dyngl_gpio.c"
                    INCLUDE_DIRS "."
                    REQUIRES bt esp_hid nvs_flash
                    REQUIRES esp_driver_spi esp_driver_gpio esp_timer
                    REQUIRES dyngl_common
)

