#include <stdio.h>
#include "onenet_token.h"
#include "onenet_mqtt.h"
#include "wifi_manager.h"
#include "nvs_flash.h"
#define DEFAULT_SSID        "111"
#define DEFAULT_PASSWORD    "12345678"
#define EV_WIFI_CONNECTED_BIT           (1<<0)
static EventGroupHandle_t s_wifi_ev;

/**
 * wifi事件回调函数
 * @param ev wifi事件
 * @return 无
 */
void wifi_callback(WIFI_STATE ev)
{
    if(ev == WIFI_STATE_CONNECTED)
    {
        xEventGroupSetBits(s_wifi_ev,EV_WIFI_CONNECTED_BIT);
    }
}

void app_main(void)
{
     nvs_flash_init();
     s_wifi_ev = xEventGroupCreate();//创建事件组
     wifi_manager_init(wifi_callback);//初始化wifi
     wifi_manager_connect(DEFAULT_SSID, DEFAULT_PASSWORD);//连接wifi
         while(1)
         {
        //这里等待一个WIFI连接成功的事件，然后再启动onenet连接
        EventBits_t bits = xEventGroupWaitBits(s_wifi_ev,EV_WIFI_CONNECTED_BIT,pdTRUE,pdFALSE,pdMS_TO_TICKS(5000));
        if(bits&EV_WIFI_CONNECTED_BIT)
        {
            one_mqtt_start();
        }
    }
}
