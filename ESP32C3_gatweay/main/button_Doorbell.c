#include "button_Doorbell.h"
#include "esp_log.h"
#include <string.h>

TaskHandle_t button_task_handle =NULL;
TaskHandle_t button_open_task_handle =NULL;
uint8_t send_buutton_flag = 0;

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
void button_gpio_InIt(){
//配置按键引脚为输入模式    
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE; // 配置为下降沿中断
    io_conf.mode = GPIO_MODE_INPUT;        // 设置为输入模式
    io_conf.pin_bit_mask = (1ULL << BUTTON_PIN)|(1ULL << BUUTON_OPEN_PIN); // 配置按键引脚
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE; // 
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;      // 
    gpio_config(&io_conf);
}
void DoorBell_GPIO_InIt(){
gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE; // 禁用中断
    io_conf.mode =  GPIO_MODE_OUTPUT;        // 设置为输出模式
    io_conf.pin_bit_mask = (1ULL << DOORBELL_PIN); // 配置按键引脚
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE; // 禁用下拉
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;      // 启用上拉
    gpio_config(&io_conf);
    //DOORBELL_PIN_SET;



}

void DoorBell(void){
    DOORBELL_PIN_RESET; // 门铃引脚复位
    vTaskDelay(pdMS_TO_TICKS(200)); // 延时100ms
    DOORBELL_PIN_SET;
    DOORBELL_LED_ON;
}


// 按键任务函数
void button_task(void *arg){
    button_state_t button_state = BUTTON_STATE_RELEASED;
    uint32_t press_start_time = 0;
    uint32_t current_time = 0;
    while(1){


    current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;// 获取当前时间（毫秒）
     switch(button_state){
      case BUTTON_STATE_RELEASED://按键被释放状态
        if(gpio_get_level(BUTTON_PIN) == 1){
            button_state = BUTTON_STATE_PRESSED;// 按键按下
            press_start_time = current_time;// 记录按下时间
        }
        else{
            //保持释放状态
            button_state = BUTTON_STATE_RELEASED;}
        break;
      case BUTTON_STATE_PRESSED://按键被按下状态
       // 检测按键是否仍然按下
        if(gpio_get_level(BUTTON_PIN) == 0){
            uint32_t press_duration = current_time - press_start_time;
            if(press_duration >= SHORT_PRESS_TIME_MS && press_duration < LONG_PRESS_TIME_MS) {
                // 短按
                ESP_LOGI("BUTTON", "短按");
              
                button_state = BUTTON_STATE_SHORT_PRESS;
            } else {
                // 按下时间太短，认为是抖动，忽略
                button_state = BUTTON_STATE_RELEASED;
            }

        }
        else if(gpio_get_level(BUTTON_PIN) == 1)
        {
            if((current_time - press_start_time) >= LONG_PRESS_TIME_MS) {
                ESP_LOGI("BUTTON", "长按检测 ");
                button_state = BUTTON_STATE_LONG_PRESS;
              //  LED_R_ON();               
                delete_all_lock();
                erase_ssn_key_uuid_from_flash();
                vTaskDelay(pdMS_TO_TICKS(1000));
                long_down_init();
                gateway_init_updata();
                esp_restart();
              //   LED_R_OFF();


                
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
        vTaskDelay(pdMS_TO_TICKS(20)); // 任务延时，避免占用过多CPU
    

    }

}



void open_button_task(){
    send_buutton_flag=0;
    ESP_LOGI("BUTTON","任务创建！\n");

    const TickType_t timeout_ticks = pdMS_TO_TICKS(10000);
    TickType_t start_ticks = xTaskGetTickCount();
    TickType_t elapsed_ticks = 0;
while(1){
 // 计算已流逝时间
 elapsed_ticks = xTaskGetTickCount()-start_ticks;
 //打印按键高低电平
// 条件1：超时（10秒未按按键）→ 销毁任务
if (elapsed_ticks >= timeout_ticks)
{
    DOORBELL_LED_OFF;
    ESP_LOGI("BUTTON","超时，销毁任务！\n");
        vTaskDelete(NULL);  // NULL 表示删除当前任务
}
if(gpio_get_level(BUUTON_OPEN_PIN) == 1){
    vTaskDelay(pdMS_TO_TICKS(50));
    if(gpio_get_level(BUUTON_OPEN_PIN) == 1){
    ESP_LOGI("BUTTON","检测到按键按下，销毁任务！\n");
   open_lock_updata();
    DOORBELL_LED_OFF;
    vTaskDelete(NULL);
}

  // NULL 表示删除当前任务
}


vTaskDelay(pdMS_TO_TICKS(50));
}



}