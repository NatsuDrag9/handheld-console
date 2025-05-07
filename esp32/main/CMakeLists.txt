idf_component_register(SRCS "wifi.c"
                    PRIV_REQUIRES spi_flash esp_wifi nvs_flash esp_event
                    INCLUDE_DIRS ".")