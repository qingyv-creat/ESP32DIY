#ifndef BLUE_GATT_H
#define BLUE_GATT_H

#include "esp_err.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_gattc_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gatt_common_api.h"
#include "esp_mac.h"
#include "storage.h"
#include "led_rgb.h"
#ifdef __cplusplus
extern "C" {
#endif

// 配置参数
#define PROFILE_NUMC      6
#define CUSTOM_PROFILE_APP_ID 0
#define CUSTOM_NUM_HANDLE 6  // 增加句柄数量以支持第二个特征


// 配置文件A的应用ID
#define PROFILE_A_APP_ID 0

// 配置文件B的应用ID
#define PROFILE_B_APP_ID 1

// 配置文件C的应用ID
#define PROFILE_C_APP_ID 2
// 配置文件D的应用ID
#define PROFILE_D_APP_ID 3
// 配置文件E的应用ID
#define PROFILE_E_APP_ID 4
// 配置文件F的应用ID
#define PROFILE_F_APP_ID 5
//*******************************************************************************/
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
 struct gattc_profile_inst {
    esp_gattc_cb_t gattc_cb;
    uint16_t gattc_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_start_handle;
    uint16_t service_end_handle;
    uint16_t write_char_handle;  // 写特征句柄
    uint16_t notify_char_handle; // 通知特征句柄
    esp_bd_addr_t remote_bda;
} ;
extern uint8_t lock_num;
extern  struct gattc_profile_inst gl_profile_tab1[PROFILE_NUMC] ;
//*******************************************************************************/
extern uint8_t sucess_data[2];
extern uint8_t flail_data[2];
extern void lock_updata();

// 服务UUID (ccc97050-c419-493e-a536-d545fc73df8b)
extern const uint8_t custom_service_uuid[2];

// 特征UUID
extern const uint8_t custom_char_uuid[2];

// 特征UUID2 
extern const uint8_t custom_char_uuid2[2];
extern uint8_t key_user;//密钥为0表示使用公钥，1表示使用私钥
extern uint8_t user_flag;//用户绑定状态标志0x00表示未绑定，0x01表示已绑定
/**
 * @brief 初始化GATT服务
 * @return esp_err_t 初始化结果
 */
esp_err_t BLUE_GATT_Init(void);

/**
 * @brief 启动蓝牙广播
 * @return esp_err_t 启动结果
 */
esp_err_t BLUE_StartAdvertising(void);

/**
 * @brief 获取GATT服务是否已创建
 * @return bool 服务创建状态
 */
bool BLUE_IsServiceCreated(void);

/**
 * @brief 获取设备是否已连接
 * @return bool 连接状态
 */
bool BLUE_IsConnected(void);

/**
 * @brief 获取Notify是否已启用
 * @return bool Notify启用状态
 */
bool BLUE_IsNotifyEnabled(void);

/**
 * @brief 通过Notify发送数据
 * @param data 要发送的数据
 * @param length 数据长度（最大244字节）
 * @return esp_err_t 发送结果
 */
esp_err_t BLUE_SendNotifyData(const uint8_t *data, uint16_t length);


void send_data_to_lock(esp_gatt_if_t gattc_if, uint16_t conn_id, uint16_t char_handle, uint8_t *data, uint16_t len);//发送数据到锁端

//*******************************************************************************/
 //tcp客户端发送函数
 extern void TCP_Send_data(uint8_t *data,uint8_t len);


 extern uint8_t TCP_lock_data[128];//网关下发的锁的数据包
 extern uint8_t TCP_lock_data_len;//TCP数据包长度



#ifdef __cplusplus
}
#endif

#endif /* BLUE_GATT_H */