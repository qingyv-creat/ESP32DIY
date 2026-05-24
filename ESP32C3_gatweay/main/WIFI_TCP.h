#ifndef     WIFI_TCP_H
#define     WIFI_TCP_H 

#include <stdint.h> 
#include "WIFI_Init.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_mac.h"
#include <string.h>
#include "esp_ota_ops.h"
#include "esp_ota_ops.h"
#include "nvs_flash.h"
#include "sys/socket.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "netdb.h"
#include "arpa/inet.h"
#include "AES.h"
#include "esp_err.h"
#include "storage.h"
#include "led_rgb.h"
#include "BLUE_gatt.h"
#include "button_Doorbell.h"
#define TAG "wifi_tcp"
#define HOST_IP_ADDR "192.168.2.112"
#define HOST_PORT 8868
#define max_len 256
#define OTA_SIZE_MAX 2100
extern uint8_t user_flag;//用户标识
extern uint8_t mac_addr[6] ;//蓝牙mac地址
extern char tcp_ip_addr[16]; //目标IP
extern uint8_t user_uuid_data[10];
extern uint8_t user_key_data[16];//用户秘钥组合之后的用户秘钥
extern uint8_t user_mac_data[6];
extern uint8_t user_ssn_data[6];
extern uint8_t self_lock_mac[6] ;//要链接锁的Mac
extern TaskHandle_t button_open_task_handle;
extern uint8_t send_buutton_flag;

// 设备A连接状态
extern  bool conn_device_a   ;

// 设备B连接状态
 extern bool conn_device_b   ;

// 设备C连接状态
 extern bool conn_device_c  ;

// 设备D连接状态
 extern bool conn_device_d ;
// 设备E连接状态
 extern bool conn_device_e   ;
// 设备F连接状态
 extern bool conn_device_f   ;



 extern uint8_t lock_num;



//新增一个心跳任务的句柄
extern TaskHandle_t heartbeat_task_handle ;
//TCP任务解析的句柄
extern TaskHandle_t recive_task_handle;

//定义一个数据类型的枚举
enum TCP_DATA_TYPE
{
TCP_User_BINDS=0x09,
TCP_User_UNBINDS=0x0a,
TCP_Down_LOGIN=0x0d,
TCP_INIT=0x0E,
LOCK_UNBIND=0x10,
TCP_DoorBell=0x12,
TCP_LOCK_UPADATE=0x16,

TCP_LOCK_NEW=0x1a,//更新关联子设备
TCP_LOCK_UNBIND=0x1c,
TCP_OTA_PREPARE=0X27,//准备开始升级
TCP_OTA_BEGIN=0x28,//开始升级
TCP_HEART=0x2a,
TCP_CLINE_LOCK=0x2b,

TCP_NEW_LOCK=0x3A,


};



//TCP初始化
void Tcp_Init();  
// TCP客户端链接 
void tcp_client(void *args);
//tcp客户端发送函数
void TCP_Send_data(uint8_t *data,uint8_t len);
//tcp客户端接收函数
void TCP_recieve();
//心跳任务
void heartbeat_task(void *pvParameters);
//接受任务
void recive_task(void *pvParameters);
//发送网关数据包
void Gateway_Send_data(uint8_t *data,uint16_t len);
//通用数据包的组包函数
uint16_t Gateway_Pack_data(uint8_t *data,uint16_t len,uint8_t type,uint8_t Mac[6],uint8_t encryption_type,uint8_t *ID,uint8_t status);
/*
用公用秘钥对私有数据包进行加密
data:私有数据包未加密
selfdata:私有数据包加密后的数据
*/ 
void self_data(uint8_t *data,uint8_t*selfdata,uint8_t leng);
//门锁关联任务
uint8_t  tcp_lock_task(void );

//上报门锁状态
void lock_updata();

extern void read_data_from_flash(void);
//网关初始化上报
void gateway_init_updata();

extern void send_data_to_lock(esp_gatt_if_t gattc_if, uint16_t conn_id, uint16_t char_handle, uint8_t *data, uint16_t len);//发送数据到锁端

//开锁命令
void open_lock_updata();

extern void open_button_task();

#endif