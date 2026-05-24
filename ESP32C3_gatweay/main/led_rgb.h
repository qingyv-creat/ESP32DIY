#pragma once

#include <stdint.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LED_R_PIN GPIO_NUM_6
#define WIFI_LED_PIN GPIO_NUM_7
///#define LED_B_PIN GPIO_NUM_10
#define DOORBELL_LED_PIN GPIO_NUM_5

#define LED_R_ON()  gpio_set_level(LED_R_PIN, 0)
#define LED_R_OFF() gpio_set_level(LED_R_PIN, 1)
#define LED_WIFI_ON()  gpio_set_level(WIFI_LED_PIN, 0)
#define LED_WIFI_OFF() gpio_set_level(WIFI_LED_PIN, 1)
#define DOORBELL_LED_OFF  gpio_set_level(DOORBELL_LED_PIN, 0) // 门铃引脚复位宏定义
#define DOORBELL_LED_ON   gpio_set_level(DOORBELL_LED_PIN, 1)



//#define LED_B_ON()  gpio_set_level(LED_B_PIN, 0)
//#define LED_B_OFF() gpio_set_level(LED_B_PIN, 1)
#define   LED_WIFI_CLINK_ON    0x00//未连接网络
#define   LED_WIFI_CLINK        0x01//正在连接网络
#define   LED_WIFI_CONNECTED    0x02//已连接网络
extern uint8_t led_state_wifi;//wifi状态指示灯变量
extern TaskHandle_t led_rgb_task_handle;

// LED初始化函数
void led_rgb_gpio_InIt();
// LED任务函数
void led_rgb_task(void *arg);
