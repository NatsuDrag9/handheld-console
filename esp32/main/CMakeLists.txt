idf_component_register(
    SRCS 
        "main.c" 
        "wifi.c"
        "uart_comm.c"
        "websocket_client.c"
    PRIV_REQUIRES
        driver
        spi_flash 
        esp_wifi 
        nvs_flash 
        esp_event
        esp_http_client
        esp_websocket_client
    INCLUDE_DIRS "."
)