#pragma once
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#define SOFTAP_SSID      "ESP32-Config"
#define SOFTAP_PASSWORD  "12345678"  // 至少8位

// 定义事件组句柄和事件位
extern EventGroupHandle_t wifi_event_group;
extern const int WIFI_CONNECTED_BIT;
extern const int WIFI_FAIL_BIT;

void wifi_init_softap(void);
void connect_to_wifi(const char* ssid, const char* password);