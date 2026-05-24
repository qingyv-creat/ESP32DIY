#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"
#include "esp_log.h"
#include "st7789_i80.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "xl9555.h"
#include "ft6336u_driver.h"

/*
1.初始化和注册LVGL显示驱动
2.初始化和注册LVGL触摸驱动
3.初始化ST7789硬件接口
4.初始化ft6336硬件接口
5.提供一个定时器给LVGL使用
*/

#define TAG     "lv_port"

#define LCD_WIDTH       320
#define LCD_HEIGHT      240

#define LCD_RST_IO           IO1_3
#define LCD_TP_RST_IO        IO1_0

static lv_display_t     *disp_drv;

bool lv_flush_done_cb(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    lv_disp_flush_ready(disp_drv);
    return false;
}

void disp_flush(lv_display_t * disp_drv, const lv_area_t * area, uint8_t * color_p)
{
    st7789_i80_flush(area->x1,area->x2,area->y1,area->y2,color_p);
}

void lv_disp_init(void)
{
    const size_t disp_buf_size = LCD_WIDTH*(LCD_HEIGHT/8);
    lv_color_t *disp1 = heap_caps_malloc(disp_buf_size*sizeof(lv_color_t),MALLOC_CAP_INTERNAL|MALLOC_CAP_DMA);
    lv_color_t *disp2 = heap_caps_malloc(disp_buf_size*sizeof(lv_color_t),MALLOC_CAP_INTERNAL|MALLOC_CAP_DMA);
    if(!disp1 || !disp2)
    {
        ESP_LOGE(TAG,"disp buff malloc fail!");
        return;
    }
    disp_drv = lv_display_create(LCD_WIDTH,LCD_HEIGHT);
    lv_display_set_flush_cb(disp_drv, disp_flush);
    lv_display_set_buffers(disp_drv, disp1, disp2, disp_buf_size*sizeof(lv_color_t), LV_DISPLAY_RENDER_MODE_PARTIAL);

}
void IRAM_ATTR indev_read(lv_indev_t * indev_drv, lv_indev_data_t * data)
{
    int16_t x,y;
    int state;
    ft6336u_read(&x,&y,&state);
    data->point.x = LCD_WIDTH - y;
    data->point.y = x;
    data->state = state;
}

void lv_indev_init(void)
{
    static lv_indev_t* indev_drv;
    indev_drv = lv_indev_create();
    lv_indev_set_type(indev_drv, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev_drv, indev_read);
}

void st7789_hw_init(void)
{
    xl9555_pin_write(LCD_RST_IO | LCD_TP_RST_IO,0);
    vTaskDelay(pdMS_TO_TICKS(200));
    xl9555_pin_write(LCD_RST_IO | LCD_TP_RST_IO,1);
    vTaskDelay(pdMS_TO_TICKS(200));
    
    st7789_i80_cfg_t st7789_config = {
        .cs = GPIO_NUM_2,         //片选
        .rs = GPIO_NUM_1,         //命令
        .wr = GPIO_NUM_41,         //写使能
        .rst = GPIO_NUM_NC,        //复位
        .bl = GPIO_NUM_NC,         //背光
        .data = {GPIO_NUM_40,GPIO_NUM_38,GPIO_NUM_39,GPIO_NUM_48,GPIO_NUM_45,GPIO_NUM_21,GPIO_NUM_47,GPIO_NUM_14},
        .bus_width = 8,             //数据位宽
        .width = LCD_WIDTH,         //宽
        .height = LCD_HEIGHT,       //长
        .spin = ST7789_SPIN_90,     //选择屏方向(0不旋转，1顺时针旋转90, 2旋转180，3顺时针旋转270)
        .done_cb = lv_flush_done_cb,    //数据传输完成回调函数
        .cb_param = &disp_drv,          //回调函数参数
    };
    st7789_i80_hw_init(&st7789_config);
    
    ESP_LOGI(TAG,"i80 init finish");
}

esp_err_t ft6336u_hw_init(void)
{
    ft6336u_cfg_t ft6336u_config = 
    {
        .scl = GPIO_NUM_12,
        .sda = GPIO_NUM_13,
        .fre = 200*1000,
        .x_limit = LCD_HEIGHT,
        .y_limit = LCD_WIDTH,
    };
    return ft6336u_init(&ft6336u_config);
}

void lv_timer_cb(void* arg)
{
    uint32_t tick_interval = *((uint32_t*)arg);
    lv_tick_inc(tick_interval);
}

void lv_tick_init(void)
{
    static uint32_t tick_interval = 5;
    const esp_timer_create_args_t arg = 
    {
        .arg = &tick_interval,
        .callback = lv_timer_cb,
        .name = "",
        .dispatch_method = ESP_TIMER_TASK,
        .skip_unhandled_events = true,
    };

    esp_timer_handle_t timer_handle;
    esp_timer_create(&arg,&timer_handle);
    esp_timer_start_periodic(timer_handle,tick_interval*1000);
}

void lv_port_init(void)
{
    lv_init();
    st7789_hw_init();
    lv_disp_init();
    st7789_i80_clear(LCD_WIDTH,LCD_HEIGHT);
    if(ft6336u_hw_init() == ESP_OK)
        lv_indev_init();
    lv_tick_init();
}
