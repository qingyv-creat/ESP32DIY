#pragma once
#include <esp_log.h>
#include "rc522.h"
#include "driver/rc522_spi.h"
#include "driver/rc522_i2c.h"
#include "rc522_picc.h"








// RFID_Init: 初始化RC522模块
void RFID_Init(void);
//RFID_事件处理函数
void RFID_event_handler(void *arg, esp_event_base_t base, int32_t event_id, void *data);
