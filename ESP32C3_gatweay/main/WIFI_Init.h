#ifndef WIFI_INIT_H
#define WIFI_INIT_H
#include "esp_err.h"
#include "stdint.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h" 




typedef enum
{
    WIFI_STATE_CONNECTED,//连接成功
    WIFI_STATE_DISCONNECTED,//断开连接
}WIFI_STATE;
extern uint8_t wifi_ssid_state;
extern uint8_t wifi_password_state;
extern uint8_t Wifi_ssid[32];//WIFI名称
extern uint8_t Wifi_password[64];//WIFI密码
extern int sta_connect_count ;//重连次数
extern  TimerHandle_t connect_timeout_timer ;



#define MAX_CONNECT_RETRY  6
//wifi状态变化回调函数
typedef void(*p_wifi_state_callback)(WIFI_STATE state);

/** 初始化wifi，默认进入STA模式
 * @param f wifi状态变化回调函数
 * @return 无 
*/
void wifi_manager_init(p_wifi_state_callback f);

/** 连接wifi
 * @param ssid
 * @param password
 * @return 成功/失败
*/
esp_err_t wifi_manager_connect(const char* ssid,const char* password);
void sntp_task(void *param);
struct tm* get_time(void);


#endif