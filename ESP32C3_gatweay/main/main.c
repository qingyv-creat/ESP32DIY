

#include "main.h"
#define TAG "MAIN"

extern bool is_connected ;
// 新增：WIFI是否连接成功标志（用于判断WIFI是否连接成功）
bool is_wifi_connected = false;
uint8_t user_uuid_data[10]={0};
uint8_t user_key_data[16]={0};//用户秘钥组合之后的用户秘钥
uint8_t user_ssn_data[6]={0};
uint8_t key_user=0x00;//密钥为0表示使用公钥，1表示使用私钥
uint8_t user_flag=0x00;//用户绑定状态标志0x00表示未绑定，0x01表示已绑定
uint8_t user_lock_mac[6]={0};//要链接锁的Mac
uint8_t lock1_flag=0x00;//锁绑定状态标志0x00表示未绑定，0x01表示已绑定
uint8_t lock2_flag=0x00;//锁绑定状态标志0x00表示未绑定，0x01表示已绑定
uint8_t lock3_flag=0x00;//锁绑定状态标志0x00表示未绑定，0x01表示已绑定
uint8_t lock4_flag=0x00;//锁绑定状态标志0x00表示未绑定，0x01表示已绑定
uint8_t lock5_flag=0x00;//锁绑定状态标志0x00表示未绑定，0x01表示已绑定
uint8_t lock6_flag=0x00;//锁绑定状态标志0x00表示未绑定，0x01表示已绑定
uint8_t lock_mac1[6]={0};//锁的mac地址
uint8_t lock_mac2[6]={0};
uint8_t lock_mac3[6]={0};
uint8_t lock_mac4[6]={0};
uint8_t lock_mac5[6]={0};
uint8_t lock_mac6[6]={0};
uint8_t my_mac_flag=0;


//WIFI的回调函数
void wifi_state_handler(WIFI_STATE state)
{
    if(state == WIFI_STATE_CONNECTED)
    {  BLUE_SendNotifyData(sucess_data,1);
        if (connect_timeout_timer != NULL) {
            xTimerStop(connect_timeout_timer, portMAX_DELAY);
            xTimerDelete(connect_timeout_timer, portMAX_DELAY);
            connect_timeout_timer = NULL;
          }
            
        ESP_LOGI(TAG,"Wifi 成功链接!");
        Tcp_Init();
        tcp_client(NULL);
        is_wifi_connected = true;
        led_state_wifi=LED_WIFI_CONNECTED;//wifi状态指示灯常亮
        
    }
    else
    {   is_wifi_connected = false;
        ESP_LOGI(TAG,"Wifi 断开链接! ");






        
    }
}
/*
uuid48a1b95f332d
key f6 90 1a 49 4e 3b 48 2a ae 52 f8 d2 90 39 32 9f
mac020122C0 00 03
*/
//初始化读取的函数 
void read_data_from_flash(void)
{lock_num=0;//初始化锁的数量
lock_updata();
esp_err_t ret1,ret2,ret3;
//测试程序完全体要把这个给注释掉
uint8_t mac[6]={0x02,0x01,0x22,0xC0 ,0x00,0x01};
write_mac_to_flash(mac);//写入默认MAC
//从这里读取获得的数据
ret1=read_mac_from_flash(mac_addr);
ret2=read_uuid_from_flash(user_uuid_data);
ret3=read_key_from_flash(user_key_data);
lock_init();
if(ret1!=ESP_OK)
{
    ESP_LOGI(TAG,"读取MAC失败，使用默认值");
    my_mac_flag=0;

}else {
    //

esp_base_mac_addr_set(mac_addr);



    
}
if(ret2!=ESP_OK)
{
    user_flag=0x00;//设置为wei绑定状态
    //打印日志
    ESP_LOGI(TAG,"读取UUID失败，使用默认值");
}
else {

user_flag=0x01;//设置为已绑定状态
ESP_LOGI(TAG,"读取UUID成功，使用默认值");
}
if(ret3!=ESP_OK)
{
 key_user=0x00;//使用公钥
 ESP_LOGI(TAG,"读取秘钥失败，使用默认值");
}
else {
 key_user=0x01;//使用秘钥
ESP_LOGI(TAG,"读取秘钥成功，使用默认值");
}


}



extern uint8_t general_data[128];
void textest()
{ 
uint8_t data[]={0x02, 0x69, 0x7c, 0x4c, 0xf6, 0x48, 0x0f, 0x4c, 0x8c, 0xa1, 0xcf, 0x7d, 0x7a, 0xc3, 0x22, 0xad, 
    0x0d, 0x27, 0xf9, 0x50, 0x17, 0x55, 0xfb, 0x76, 0x04, 0x86 ,0x16,0xe5,0x84,0xec,0xc6 };
uint8_t rxdata[128]={0};
    Ble_JieMa_t ble_param1;
    ble_param1.mode = 0x22; // 使用预设密钥表
    
    memcpy(ble_param1.gattway_id, mac_addr, 6);
    memcpy(ble_param1.key, user_key_data, 16);
    Fun_Ble_JieMa(data, sizeof(data), rxdata, ble_param1);
    ESP_LOGI(TAG, "使用私钥解密");
//打印解密后的数据
ESP_LOG_BUFFER_HEX(TAG, rxdata,  sizeof(rxdata));


}







void app_main(void)
{   
    esp_err_t ret=ESP_OK;
    BaseType_t task_ret,task_ret1,task_ret2,task_ret3;  // 声明一个FreeRTOS基础类型变量
    /* 初始化NVS */
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

read_data_from_flash();
button_gpio_InIt();
// 初始化RFID
#if (CRAD_YES_OR_NO == 1)
RFID_Init();
#endif


//初始化蓝牙
BLUE_Init();
//初始化蓝牙GATT服务
BLUE_GATT_Init();
//初始化WIFI
wifi_manager_init(wifi_state_handler);//WIFI的回调函数

//初始化led
led_rgb_gpio_InIt();
DoorBell_GPIO_InIt();








// 创建心跳任务
 task_ret = xTaskCreate(
heartbeat_task,                   // 任务函数
"heartbeat_task",                // 任务名称
3072,                           // 栈大小（字节），对于简单的打印任务，2048足够
NULL,                          // 任务参数
2,                            // 优先级（1是低优先级，不会影响蓝牙/WiFi等高优先级任务）
&heartbeat_task_handle       // 任务句柄
);
// 创建按钮检测任务
task_ret1 =xTaskCreate(
button_task,
"button_task",
2048,
NULL,
1,
&button_task_handle
);

// 创建tcp接收处理任务
task_ret2 =xTaskCreate(
recive_task,
"recive_task",
9096,
NULL,
5,
&recive_task_handle
);
//创建LED状态指示任务
task_ret3 =xTaskCreate(
led_rgb_task,
"led_rgb_task",
2048,
NULL,
2,// 优先级（1是低优先级，不会影响蓝牙/WiFi等高优先级任务）
&led_rgb_task_handle
);





}

