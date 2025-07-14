idf_component_register(
    SRCS 
        "main.c" 
        "wifi.c"
        "uart_comm.c"
        "websocket_client.c"
        "helpers/websocket-client/websocket_client_to_server.c"
        "helpers/websocket-client/websocket_client_to_stm32.c"
    PRIV_REQUIRES
        driver
        spi_flash 
        esp_wifi 
        nvs_flash 
        esp_event
        esp_http_client
        esp_websocket_client
    INCLUDE_DIRS "." "helpers" "helpers/websocket-client"
)