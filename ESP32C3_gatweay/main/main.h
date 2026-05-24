#ifndef __MAIN_H
#define __MAIN_H
#include "led_rgb.h"
#include "button_Doorbell.h"

#if (CRAD_YES_OR_NO == 1)
#include "RFID.h"
#endif
#include "BLUE_Init.h"
#include "BLUE_gatt.h"
#include "WIFI_Init.h"
#include "WIFI_TCP.h"
#include "AES.h"
#include <esp_log.h>
#include "rc522.h"
#include "driver/rc522_spi.h"
#include "rc522_picc.h"
#include "led_rgb.h"
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "storage.h"
#define CRAD_YES_OR_NO     0//为1代表启用刷卡功能0不启用

//重连次数
extern int sta_connect_count ;
// 在某个.c文件中定义（只能在一个地方定义）
extern uint8_t wifi_ssid_state;
extern uint8_t wifi_password_state;

extern bool is_wifi_connected;
extern uint8_t user_uuid_data[10];
extern uint8_t user_key_data[16];//用户秘钥组合之后的用户秘钥
extern uint8_t user_ssn_data[6];
extern uint8_t key_user;//密钥为0表示使用公钥，1表示使用私钥
extern uint8_t user_flag;//用户绑定状态标志0x00表示未绑定，0x01表示已绑定
extern uint8_t user_lock_mac[6];//要链接锁的Mac
extern uint8_t lock1_flag;//锁绑定状态标志0x00表示未绑定，0x01表示已绑定
extern uint8_t lock2_flag;//锁绑定状态标志0x00表示未绑定，0x01表示已绑定
extern uint8_t lock3_flag;//锁绑定状态标志0x00表示未绑定，0x01表示已绑定
extern uint8_t lock4_flag;//锁绑定状态标志0x00表示未绑定，0x01表示已绑定
extern uint8_t lock5_flag;//锁绑定状态标志0x00表示未绑定，0x01表示已绑定
extern uint8_t lock6_flag;//锁绑定状态标志0x00表示未绑定，0x01表示已绑定extern 
extern uint8_t lock_mac1[6];//锁的mac地址
extern uint8_t lock_mac2[6];
extern uint8_t lock_mac3[6];
extern uint8_t lock_mac4[6];
extern uint8_t lock_mac5[6];
extern uint8_t lock_mac6[6];
extern uint8_t my_mac_flag;
extern  bool is_wifi_connected;//WIFI连接状态
extern esp_err_t uid_storage_init(void);
extern uint8_t mac_addr[6];
extern uint32_t duration;//扫描持续时间
extern uint8_t key_user;//密钥为0表示使用公钥，1表示使用私钥
extern esp_ota_handle_t ota_handle;// OTA操作句柄
extern esp_partition_t *update_partition;// OTA分区指针

extern uint8_t  general_data[128];//要发送的通用数据数据包
extern uint8_t TCP_lock_data[128];//网关下发的锁的数据包
extern uint8_t TCP_lock_data_len;//TCP数据包长度
extern int sock ;//TCPsock
//新增一个心跳任务的句柄
extern TaskHandle_t heartbeat_task_handle ;
//TCP任务解析的句柄
extern TaskHandle_t recive_task_handle ;
extern uint8_t time_data[4];//时间戳数据包
extern uint8_t tcp_buffer[OTA_SIZE_MAX];//TCP接收数据缓冲区
uint8_t Gateway_sloft_banbeng[]={"26.01.25"};//网关软件版本
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











void read_data_from_flash(void);


#endif