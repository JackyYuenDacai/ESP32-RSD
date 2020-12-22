#include"main.h"

#ifndef WIFI_CONNECT_H
#define WIFI_CONNECT_H

#define EXAMPLE_ESP_WIFI_SSID      "Wi-Fi.HK via HKUST"
#define EXAMPLE_ESP_WIFI_PASS      ""
#define EXAMPLE_ESP_MAXIMUM_RETRY  1000
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1
extern EventGroupHandle_t s_wifi_event_group;
extern int s_retry_num;


void wifi_init_sta(void);

#endif