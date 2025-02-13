idf_component_register(SRCS "dyngl.c" "dyngl_spi.c" "dyngl_usb.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_driver_spi esp_driver_gpio
                    REQUIRES dyngl_common
)
