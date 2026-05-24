#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "lv_port.h"
#include "lvgl.h"
#include "lv_demos.h"
#include "st7789_i80.h"
#include "driver/i2s_std.h"
#include "xl9555.h"
#include "my_display.h"
#include "gui_guider.h"
#include "weather.h"
#define LCD_RST_IO           IO1_3
#define LCD_BL_IO            IO1_2
#define TP_RST_IO            IO1_0

#define XL9555_SDA  GPIO_NUM_10
#define XL9555_SCL  GPIO_NUM_11
lv_ui guider_ui;
void i2c_and_xl9555_init(void)
{
    xl9555_init(XL9555_SDA,XL9555_SCL,GPIO_NUM_NC,NULL);
    xl9555_ioconfig((~(LCD_RST_IO | LCD_BL_IO | TP_RST_IO))&0xFFFF);

}
void my_display_init(void)
{
   i2c_and_xl9555_init();          //初始化XL9555
    st7789_i80_lcd_backlight(true);         //打开背光
    lv_port_init();    
}
void my_display_task(void){
    my_display_init();
   // lv_demo_widgets();              //初始化控件demo程序
    setup_ui(&guider_ui);
    weather_config_t config = {
        .api_key = "youkey",   // 👈 替换成你的私钥
        .type = WEATHER_XINZHI,           // 心知天气
        .city = "nanyang",              // 改成你所在的城市拼音，或 NULL 自动定位
    };
    weather_info_t *info = weather_get(&config);
       if (info) {
        //weather_print_info(info);         // 串口打印调试
        //printf("天气：%s\n", info->weather);
     if(strcmp(info->weather, "小雨") == 0
     ||strcmp(info->weather, "中雨") == 0
     || strcmp(info->weather, "大雨") == 0
     ||strcmp(info->weather, "暴雨") == 0){
            lv_label_set_text(guider_ui.screen_label_Weather, "雨");
        }
           if(strcmp(info->weather, "晴") == 0
   ){
            lv_label_set_text(guider_ui.screen_label_Weather, "晴");
        }
        //weather_info_free(info);
    }
    while(1)
    {
        vTaskDelay(1);
        lv_task_handler();          //LVGL循环处理
    }
    vTaskDelete(NULL);
}