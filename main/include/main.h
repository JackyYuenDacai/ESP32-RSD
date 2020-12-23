#ifndef MAIN_H
#define MAIN_H
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "nvs_flash.h"
#include "esp_tls.h"
#include "esp_crt_bundle.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "esp_sleep.h"
#include "esp_sntp.h"
#include "esp_task_wdt.h"
#include "esp_int_wdt.h"
#include "wifi_connect.h"
#include "freewifi_req.h"
#include "sound_task.h"
#include "time_task.h"
#include "nas_task.h"
#include "nas_queue.h"
#include "reboot_task.h"

static const char *TAG = "RSD";
#endif