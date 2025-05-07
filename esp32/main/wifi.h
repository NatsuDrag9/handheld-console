#ifndef WIFI_H
#define WIFI_H

#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

// Configuration for WiFi connection
#define EXAMPLE_ESP_WIFI_SSID      "moto g(30)_1128"
#define EXAMPLE_ESP_WIFI_PASS      "Moto G30"
#define EXAMPLE_ESP_MAXIMUM_RETRY  5

/* The examples use WiFi configuration that you can set via project configuration menu.
   If you'd rather not, just change the below entries to strings with the config you want
   - ie #define EXAMPLE_ESP_WIFI_SSID "mywifissid" */

#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_BOTH
#define EXAMPLE_H2E_IDENTIFIER ""

#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK

   // Function prototypes
void wifi_init_sta(void);

#endif /* WIFI_H */