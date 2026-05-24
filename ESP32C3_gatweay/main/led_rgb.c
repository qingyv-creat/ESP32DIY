#include "led_rgb.h"


    


uint8_t led_state_wifi=LED_WIFI_CLINK_ON;
 TaskHandle_t led_rgb_task_handle=NULL;




void led_rgb_gpio_InIt(){
//配置LED引脚为输出模式
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE; // 禁用中断
    io_conf.mode = GPIO_MODE_OUTPUT;       // 设置为输出模式
    io_conf.pin_bit_mask = (1ULL << LED_R_PIN) | (1ULL << WIFI_LED_PIN) | (1ULL << DOORBELL_LED_PIN); // 配置LED引脚
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE; // 禁用下拉
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;     // 禁用上拉
    gpio_config(&io_conf);
//初始化时关闭LED默认全部拉高
    gpio_set_level(LED_R_PIN, 1);
    gpio_set_level(WIFI_LED_PIN, 1);

//
}



void led_rgb_task(void *arg){
    while(1){
      if(led_state_wifi==LED_WIFI_CLINK_ON){

        LED_WIFI_OFF();
      }
      if(led_state_wifi==LED_WIFI_CLINK){
         LED_WIFI_OFF();
         vTaskDelay(pdMS_TO_TICKS(1000));
         LED_WIFI_ON();
         vTaskDelay(pdMS_TO_TICKS(200));

      }
      if(led_state_wifi==LED_WIFI_CONNECTED){
        LED_WIFI_ON();


      }


      vTaskDelay(pdMS_TO_TICKS(50)); 



    }
}
