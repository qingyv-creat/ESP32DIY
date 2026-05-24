#pragma  once
#include <stdint.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include  "led_rgb.h"
#include "led_rgb.h"
#define BUTTON_PIN       GPIO_NUM_9
#define DOORBELL_PIN     GPIO_NUM_3
#define BUUTON_OPEN_PIN  GPIO_NUM_4
#define SHORT_PRESS_TIME_MS 50      // 短按去抖时间
#define LONG_PRESS_TIME_MS  10000     // 长按时间5秒
#define DOORBELL_PIN_RESET  gpio_set_level(DOORBELL_PIN, 0) // 门铃引脚复位宏定义
#define DOORBELL_PIN_SET    gpio_set_level(DOORBELL_PIN, 1)
extern TaskHandle_t button_task_handle;
extern uint8_t send_buutton_flag;

extern void open_lock_updata();
// 按键初始化函数
void button_gpio_InIt();
// 按键任务函数
void button_task(void *arg);
//门铃初始化函数
void DoorBell_GPIO_InIt();
//响铃函数
void DoorBell(void);
//开锁任务函数
void open_button_task();

extern void gateway_init_updata(void);
extern void erase_ssn_key_uuid_from_flash(void);
extern void delete_all_lock(void);