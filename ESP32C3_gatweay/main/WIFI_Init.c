
#include "WIFI_Init.h"
#include <stdio.h>
#include "esp_log.h"
#include <string.h>
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "BLue_gatt.h"

#include "esp_timer.h"  

#include <stdio.h>

#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include <sys/time.h>
#include "esp_sntp.h"


#define TAG     "wifi_manager"
#define WIFI_CONNECT_TIMEOUT_S 10  // 连接超时时间，单位秒发送密码错误
//重连次数
 int sta_connect_count = 0;
// 在某个.c文件中定义（只能在一个地方定义）
uint8_t wifi_ssid_state = 0;
uint8_t wifi_password_state = 0;
//回调函数
static p_wifi_state_callback    wifi_state_cb = NULL;

//当前sta连接状态
static bool is_sta_connected = false;
// 定时器句柄

 TimerHandle_t connect_timeout_timer = NULL;

static void connect_timeout_callback(TimerHandle_t xTimer);
/** 事件回调函数
 * @param arg   用户传递的参数
 * @param event_base    事件类别
 * @param event_id      事件ID
 * @param event_data    事件携带的数据
 * @return 无
*/
static void event_handler(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data)
{   
    if(event_base == WIFI_EVENT)
    {
        switch (event_id)
        {
        case WIFI_EVENT_STA_START:      //WIFI以STA模式启动后触发此事件
        {
            wifi_mode_t mode;
            esp_wifi_get_mode(&mode);
            if(mode == WIFI_MODE_STA)
                esp_wifi_connect();         //启动WIFI连接
            break;
        }
        case WIFI_EVENT_STA_CONNECTED:  //WIFI连上路由器后，触发此事件
            ESP_LOGI(TAG, "Connected to AP");
            break;
        case WIFI_EVENT_STA_DISCONNECTED:   //WIFI从路由器断开连接后触发此事件
            //断开连接分为两种情况，一种是用户主动断开连接，另一种是连接丢失（例如路由器重启，信号弱等）
            if(wifi_password_state == 1 || wifi_ssid_state == 1)
            {   
                ESP_LOGI(TAG, "用户主动断开连接不处理");
                wifi_password_state = 0;
                wifi_ssid_state = 0;
            }else
            { 
                ESP_LOGI(TAG, "连接丢失，尝试重连");
                if(sta_connect_count <= MAX_CONNECT_RETRY)  {
                esp_wifi_connect();
                sta_connect_count++;}
                
            }





            break;
        default:
            break;
        }
    }
    if(event_base == IP_EVENT)                  //IP相关事件
    {
        switch(event_id)
        {
            case IP_EVENT_STA_GOT_IP:           //只有获取到路由器分配的IP，才认为是连上了路由器
                ESP_LOGI(TAG,"Get ip address");
                wifi_ssid_state=0;
                wifi_password_state=0;
                sta_connect_count = 0;
                // 停止FreeRTOS定时器
          if (connect_timeout_timer != NULL) {
          xTimerStop(connect_timeout_timer, portMAX_DELAY);
          xTimerDelete(connect_timeout_timer, portMAX_DELAY);
          //打印删除了
          ESP_LOGI(TAG, "已删除定时器");
          
          connect_timeout_timer = NULL;
        }
                is_sta_connected = true;
              

                if(wifi_state_cb)
                    wifi_state_cb(WIFI_STATE_CONNECTED);
                break;
            default:break;
        }
    }
}

/** 初始化wifi，默认进入STA模式
 * @param 无
 * @return 无 
*/
void wifi_manager_init(p_wifi_state_callback f)
{
    ESP_ERROR_CHECK(esp_netif_init());  //用于初始化tcpip协议栈
    ESP_ERROR_CHECK(esp_event_loop_create_default());       //创建一个默认系统事件调度循环，之后可以注册回调函数来处理系统的一些事件
    esp_netif_create_default_wifi_sta();    //使用默认配置创建STA对象
    //初始化WIFI
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    //注册事件
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,ESP_EVENT_ANY_ID,&event_handler,NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT,IP_EVENT_STA_GOT_IP,&event_handler,NULL));

    wifi_state_cb = f;
    //启动WIFI
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );         //设置工作模式为STA
    ESP_ERROR_CHECK(esp_wifi_start() );                         //启动WIFI
    
    ESP_LOGI(TAG, "wifi_init finished.");
}

/** 连接wifi
 * @param ssid
 * @param password
 * @return 成功/失败
*/
esp_err_t wifi_manager_connect(const char* ssid, const char* password)
{
    
    //断开之前的连接
    esp_wifi_disconnect();
   
    
    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    
    // 确保字符串正确复制和终止
    strncpy((char*)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid) - 1);
    wifi_config.sta.ssid[sizeof(wifi_config.sta.ssid) - 1] = '\0';
    
    strncpy((char*)wifi_config.sta.password, password, sizeof(wifi_config.sta.password) - 1);
    wifi_config.sta.password[sizeof(wifi_config.sta.password) - 1] = '\0';
    
    ESP_LOGI(TAG, " SSID: %s, Password: %s", 
             wifi_config.sta.ssid, wifi_config.sta.password);
    ESP_LOGI(TAG, "密码的长度: %d", strlen((char*)wifi_config.sta.password));
    
    ESP_ERROR_CHECK(esp_wifi_disconnect());
    
    wifi_mode_t mode;
    esp_wifi_get_mode(&mode);
    if(mode != WIFI_MODE_STA) {
        ESP_ERROR_CHECK(esp_wifi_stop());
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
        esp_wifi_start();
    } else {
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
        esp_wifi_connect();
    }
    if (connect_timeout_timer != NULL) {
        xTimerDelete(connect_timeout_timer, portMAX_DELAY);
    }
    
    connect_timeout_timer = xTimerCreate("wifi_connect_timeout", 
                                        pdMS_TO_TICKS(WIFI_CONNECT_TIMEOUT_S * 1000),
                                        pdFALSE, 
                                        NULL, 
                                        connect_timeout_callback);
    
    if (connect_timeout_timer != NULL) {
        xTimerStart(connect_timeout_timer, portMAX_DELAY);
    }


    return ESP_OK;
}

static void connect_timeout_callback(TimerHandle_t xTimer) {
    ESP_LOGI(TAG, "WiFi connection timed out after %d seconds", WIFI_CONNECT_TIMEOUT_S);
    
    esp_wifi_disconnect();
    uint8_t timeout_data[] = {0xff};
    ESP_LOGI(TAG, "发送连接超时通知");
    BLUE_SendNotifyData(timeout_data, 1);
}




