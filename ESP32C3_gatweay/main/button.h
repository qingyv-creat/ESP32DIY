#pragma  once
#include <stdint.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include  "led_rgb.h"
extern TaskHandle_t button_task_handle;
// 按键初始化函数
void button_gpio_InIt();
// 按键任务函数
void button_task(void *arg);
extern void erase_ssn_key_uuid_from_flash(void);
extern void delete_all_lock(void);