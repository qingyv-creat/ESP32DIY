#include "button.h"
#include "esp_log.h"
#include <string.h>
#define BUTTON_PIN GPIO_NUM_9

#define SHORT_PRESS_TIME_MS 50      // 短按去抖时间
#define LONG_PRESS_TIME_MS 5000     // 长按时间5秒

extern uint8_t lock_mac1[6];
extern uint8_t lock_mac2[6];
extern uint8_t lock_mac3[6];
extern uint8_t lock_mac4[6];
extern uint8_t lock_mac5[6];
extern uint8_t lock_mac6[6];

extern uint8_t lock1_flag;//锁绑定状态标志0x00表示未绑定，0x01表示已绑定
extern uint8_t lock2_flag;//锁绑定状态标志0x00表示未绑定，0x01表示已绑定
extern uint8_t lock3_flag;//锁绑定状态标志0x00表示未绑定，0x01表示已绑定
extern uint8_t lock4_flag;//锁绑定状态标志0x00表示未绑定，0x01表示已绑定
extern uint8_t lock5_flag;//锁绑定状态标志0x00表示未绑定，0x01表示已绑定
extern uint8_t lock6_flag;//锁绑定状态标志0x00表示未绑定，0x01表示已绑定

extern uint8_t key_user;//密钥为0表示使用公钥，1表示使用私钥
extern uint8_t user_flag;//用户绑定状态标志0x00表示未绑定，0x01表示已绑定


void long_down_init(){
    memset(lock_mac1,0,6);
    memset(lock_mac2,0,6);
    memset(lock_mac3,0,6);
    memset(lock_mac4,0,6);
    memset(lock_mac5,0,6);
    memset(lock_mac6,0,6);
    lock1_flag = 0x00;
    lock2_flag = 0x00;
    lock3_flag = 0x00;  
    lock4_flag = 0x00;
    lock5_flag = 0x00;
    lock6_flag = 0x00;
    key_user = 0x00;
    user_flag = 0x00;

}




typedef enum {
    BUTTON_STATE_RELEASED = 0,     // 按键释放
    BUTTON_STATE_PRESSED,          // 按键按下
    BUTTON_STATE_SHORT_PRESS,      // 短按
    BUTTON_STATE_LONG_PRESS        // 长按
} button_state_t;
TaskHandle_t button_task_handle =NULL;
void button_gpio_InIt(){
//配置按键引脚为输入模式    
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_NEGEDGE; // 配置为下降沿中断
    io_conf.mode = GPIO_MODE_INPUT;        // 设置为输入模式
    io_conf.pin_bit_mask = (1ULL << BUTTON_PIN); // 配置按键引脚
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE; // 禁用下拉
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;      // 启用上拉
    gpio_config(&io_conf);
}












extern void gateway_init_updata(void);

// 按键任务函数
void button_task(void *arg){
    button_state_t button_state = BUTTON_STATE_RELEASED;
    uint32_t press_start_time = 0;
    uint32_t current_time = 0;
    while(1){
        
        current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;// 获取当前时间（毫秒）
     switch(button_state){
      case BUTTON_STATE_RELEASED://按键被释放状态
        if(gpio_get_level(BUTTON_PIN) == 0){
            button_state = BUTTON_STATE_PRESSED;// 按键按下
            press_start_time = current_time;// 记录按下时间
            ESP_LOGI("BUTTON", "开始按下");
        }
        else{
            //保持释放状态
            button_state = BUTTON_STATE_RELEASED;}
        break;
      case BUTTON_STATE_PRESSED://按键被按下状态
       // 检测按键是否仍然按下
        if(gpio_get_level(BUTTON_PIN) == 1){
            uint32_t press_duration = current_time - press_start_time;
            if(press_duration >= SHORT_PRESS_TIME_MS && press_duration < LONG_PRESS_TIME_MS) {
                // 短按
              
                button_state = BUTTON_STATE_SHORT_PRESS;
            } else {
                // 按下时间太短，认为是抖动，忽略
                button_state = BUTTON_STATE_RELEASED;
            }


      

        }
        else if(gpio_get_level(BUTTON_PIN) == 0)
        {
            if((current_time - press_start_time) >= LONG_PRESS_TIME_MS) {
                ESP_LOGI("BUTTON", "长按检测 ");
                button_state = BUTTON_STATE_LONG_PRESS;
                LED_R_ON();               
                delete_all_lock();
                erase_ssn_key_uuid_from_flash();
                vTaskDelay(pdMS_TO_TICKS(1000));
                long_down_init();
                gateway_init_updata();
               // esp_restart();
                 LED_R_OFF();


                
            }



        }
        break;
      case BUTTON_STATE_SHORT_PRESS:
          // 短按处理完成，回到释放状态
          button_state = BUTTON_STATE_RELEASED;
          break;
        case BUTTON_STATE_LONG_PRESS:
            // 长按处理完成，回到释放状态






            
            button_state = BUTTON_STATE_RELEASED;
            break;


        }
        vTaskDelay(pdMS_TO_TICKS(10)); // 任务延时，避免占用过多CPU


    }

}