#pragma once

#include "my_wifi.h"
#include "esp_log.h"

// 删除这两行，使用ESP-IDF默认配置
// #define HTTPD_MAX_REQ_HDR_LEN 2048
// #define HTTPD_MAX_URI_LEN 512

void start_webserver(void);
void erase_wifi_credentials(void);