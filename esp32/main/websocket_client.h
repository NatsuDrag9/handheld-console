#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLIENT_H

#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include "esp_tls.h"
#include "esp_http_client.h"

#include "esp_websocket_client.h"
#include "../components/cmp/cmp.h"

// Configuration
#define MSGPACK_BUFFER_SIZE 1024
#define WEBSOCKET_URI "ws://192.168.53.151:3001"
#define WEBSOCKET_RECONNECT_TIMEOUT_MS 10000
#define SENSOR_SAMPLING_INTERVAL 5000

void websocket_app_main(void);

#endif // WEBSOCKET_CLIENT_H