#include "BLUE_gatt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_gatt_common_api.h"
#include "string.h"
#include "esp_mac.h"
#include "WIFI_Init.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_gattc_api.h"
#include "nvs_flash.h"
#define TAG "BLUE_GATT"
const char *ccTAG = "写的数据";
#define PROFILE_NUMS 1
uint8_t Wifi_ssid[32] = {0};
uint8_t Wifi_password[64] = {0};
uint8_t wifi_ssid_len=0;
uint8_t wifi_password_len=0;
uint8_t mac_addr[6] = {0};
uint8_t sucess_data[2] = {0x00,0x00};
uint8_t flail_data[2] = {0xff,0xff};
uint8_t self_lock_mac[6] = {0};
uint8_t lock1_send_data[128] = {0};
uint8_t lock2_send_data[128] = {0};
uint8_t lock3_send_data[128] = {0};
uint8_t lock4_send_data[128] = {0};
uint8_t lock5_send_data[128] = {0};
uint8_t lock6_send_data[128] = {0};
uint8_t lock1_send_len=0;
uint8_t lock2_send_len=0;
uint8_t lock3_send_len=0;
uint8_t lock4_send_len=0;
uint8_t lock5_send_len=0;
uint8_t lock6_send_len=0;




extern uint8_t  general_data[128];//通用数据数据包
extern  void Gateway_Send_data(uint8_t *data,uint16_t len);
//通用数据包的组包函数
extern  uint16_t Gateway_Pack_data(uint8_t *data,uint16_t len,uint8_t type,uint8_t Mac[6],uint8_t encryption_type,uint8_t *ID,uint8_t status);
extern void print_hex_buffer(const char *tag, uint8_t *data, int len);

// 自定义服务UUID (0xc9c1)
const uint8_t custom_service_uuid[2] = {
   0xc1,0xc9
};

// 自定义特征UUID
const uint8_t custom_char_uuid[2] = {
0xe1, 0xff
};

// 自定义特征UUID2
const uint8_t custom_char_uuid2[2] = {
    0xe2, 0xff
};



static uint8_t raw_adv_data[23] = {
    // 广播标志
    0x02, 0x01, 0x06,
    // 网关UUID
    0x03, 0x02, 0xc1, 0xc9,
    // 网关uuid服务数据
    0x0f,0x16, 0xc1, 0xc9, 0x00,0x00,0x00,0x00,0x00,0x00, 0x04,0xff,0x01,0x01,0x01,0x00,
};

static uint8_t raw_scan_data[] = {
    // 网关名称
    0x0a, 0x09, 'Z', '-', 'G', 'a', 't', 'e', 'w', 'a', 'y'
};

void setup_adv_data_raw(void) {
    esp_err_t ret = ESP_OK;
    
    // 获取蓝牙 MAC 地址
    ret = esp_read_mac(mac_addr, ESP_MAC_BT);
    // 把MAC地址放到广播数据中
    raw_adv_data[11] = mac_addr[0];
    raw_adv_data[12] = mac_addr[1];
    raw_adv_data[13] = mac_addr[2];
    raw_adv_data[14] = mac_addr[3];
    raw_adv_data[15] = mac_addr[4];
    raw_adv_data[16] = mac_addr[5];

    ret = esp_ble_gap_config_adv_data_raw(raw_adv_data, sizeof(raw_adv_data));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "配置原始广播数据失败: %s", esp_err_to_name(ret));
    }
    ret = esp_ble_gap_config_scan_rsp_data_raw(raw_scan_data, sizeof(raw_scan_data));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "配置原始扫描响应数据失败: %s", esp_err_to_name(ret));
    }
}

// 内部结构体和变量定义
typedef struct {
    esp_gatts_cb_t gatts_cb;
    uint16_t gatts_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_handle;
    esp_gatt_srvc_id_t service_id;
    
    // 第一个特征（Write withoutresponse）
 
    uint16_t char_handle;
    esp_bt_uuid_t char_uuid;
    esp_gatt_perm_t perm;
    esp_gatt_char_prop_t property;
    uint16_t descr_handle;
    esp_bt_uuid_t descr_uuid;
    
    // 第二个特征（notify）
    uint16_t char_handle2;
    esp_bt_uuid_t char_uuid2;
    esp_gatt_perm_t perm2;
    esp_gatt_char_prop_t property2;
    uint16_t descr_handle2;
    esp_bt_uuid_t descr_uuid2;
    
    

} gatts_profile_inst_t;

static uint8_t custom_char_value[244] = {0};
static uint8_t custom_char_value2[244] = {0};  // 第二个特征的值
static bool custom_service_created = false;
static esp_gatt_char_prop_t custom_property = 0;
static esp_gatt_char_prop_t custom_property2 = 0;
static bool is_notify_enabled = true;  // Notify使能状态

static esp_attr_value_t custom_char_attr = {
    .attr_max_len = 244,
    .attr_len = sizeof(custom_char_value),
    .attr_value = custom_char_value,
};

static esp_attr_value_t custom_char_attr2 = {
    .attr_max_len = 244,
    .attr_len = sizeof(custom_char_value2),
    .attr_value = custom_char_value2,
};

static esp_ble_adv_data_t adv_data = {
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = true,
    .min_interval = 0x0006,
    .max_interval = 0x0010,
    .appearance = 0x0000,
    .manufacturer_len = 0,
    .p_manufacturer_data = NULL,
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len =16,
    .p_service_uuid = custom_service_uuid,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

 esp_ble_adv_params_t adv_params = {
    .adv_int_min = 0x20,
    .adv_int_max = 0x40,
    .adv_type = ADV_TYPE_IND,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .channel_map = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

static gatts_profile_inst_t gl_profile_tab[PROFILE_NUMS] = {
    [CUSTOM_PROFILE_APP_ID] = {
        .gatts_cb = NULL, // 将在初始化时设置
        .gatts_if = ESP_GATT_IF_NONE,
    },
};
//*******************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************// 

//*******************************************************客户端************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************// 

#define GATTC_TAG "GATTC_DEMO"


#define INVALID_HANDLE   0

static char remote_device_name[ESP_BLE_ADV_NAME_LEN_MAX] = "ESP_GATTS_DEMO";
static bool connect    = false;
 bool get_servera = false;

static bool get_serverb = false;

static bool get_serverc = false;

static bool get_serverd = false;

static bool get_servere = false;

static bool get_serverf = false;

/* Declare static functions */

static void esp_gattc_cb(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);
static void gattc_profile_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);


static esp_bt_uuid_t remote_filter_service_uuid = {
    .len = ESP_UUID_LEN_128,
    .uuid = {.uuid128={0x8b, 0xdf, 0x73, 0xfc, 0x45, 0xd5, 0x36, 0xa5,
        0x3e, 0x49, 0x19, 0xc4, 0x50, 0x70, 0xc9, 0xcc},},
};

static esp_bt_uuid_t write_filter_char_uuid = {
    .len = ESP_UUID_LEN_128,
    .uuid = {.uuid128 = { 0x8b, 0xdf, 0x73, 0xfc, 0x45, 0xd5, 0x36, 0xa5,
        0x3e, 0x49, 0x19, 0xc4, 0x51, 0x70, 0xc9, 0xcc},},
};

static esp_bt_uuid_t notify_descr_uuid = {
    .len = ESP_UUID_LEN_128,
    .uuid = {.uuid128 = { 0x8b, 0xdf, 0x73, 0xfc, 0x45, 0xd5, 0x36, 0xa5,
        0x3e, 0x49, 0x19, 0xc4, 0x52, 0x70, 0xc9, 0xcc},},
};

static esp_ble_scan_params_t ble_scan_params = {
    .scan_type              = BLE_SCAN_TYPE_ACTIVE,
    .own_addr_type          = BLE_ADDR_TYPE_PUBLIC,
    .scan_filter_policy     = BLE_SCAN_FILTER_ALLOW_ALL,
    .scan_interval          = 0x50,
    .scan_window            = 0x30,
    .scan_duplicate         = BLE_SCAN_DUPLICATE_DISABLE
};



/* 设备连接状态标志 */
uint8_t connect_flag = 0;

// 设备A连接状态
 bool conn_device_a   = false;

// 设备B连接状态
 bool conn_device_b   = false;

// 设备C连接状态
 bool conn_device_c   = false;

// 设备D连接状态
 bool conn_device_d   = false;
// 设备E连接状态
 bool conn_device_e   = false;
// 设备F连接状态
 bool conn_device_f   = false;



 uint8_t lock_num=0;
/* 服务获取状态标志 */




/* 连接状态标志 */

// 正在连接状态
static bool Isconnecting    = false;

// 扫描停止完成状态
static bool stop_scan_done  = false;

/* 特征和描述符结果指针 */

// 设备A特征元素结果指针
static esp_gattc_char_elem_t  *char_elem_result_a   = NULL;

// 设备A描述符元素结果指针
static esp_gattc_descr_elem_t *descr_elem_result_a  = NULL;

// 设备B特征元素结果指针
static esp_gattc_char_elem_t  *char_elem_result_b   = NULL;

// 设备B描述符元素结果指针
static esp_gattc_descr_elem_t *descr_elem_result_b  = NULL;

// 设备C特征元素结果指针
static esp_gattc_char_elem_t  *char_elem_result_c   = NULL;

// 设备C描述符元素结果指针
static esp_gattc_descr_elem_t *descr_elem_result_c  = NULL;
// 设备D特征元素结果指针
static esp_gattc_char_elem_t  *char_elem_result_d   = NULL;
// 设备D描述符元素结果指针
static esp_gattc_descr_elem_t *descr_elem_result_d  = NULL;
// 设备E特征元素结果指针
static esp_gattc_char_elem_t  *char_elem_result_e   = NULL;
// 设备E描述符元素结果指针
static esp_gattc_descr_elem_t *descr_elem_result_e  = NULL;
// 设备F特征元素结果指针
static esp_gattc_char_elem_t  *char_elem_result_f   = NULL;
// 设备F描述符元素结果指针
static esp_gattc_descr_elem_t *descr_elem_result_f  = NULL;

//******************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************** */
extern uint8_t selflock_mac[6][6];//要链接锁的Mac
//根据蓝牙广播数据判断是不是锁的广播
uint8_t lock_data[7] = {0x02, 0x01, 0x06, 0x03, 0x02, 0xc0, 0xc9};
//处理蓝牙广播数据更具MAC地址判断是不是自己要连接的锁
static uint8_t get_lock_mac1(uint8_t *data) { 
    if (lock1_flag == 1) {  // 这个位置已经存锁不能走锁关联任务
     
        if (memcmp(lock_mac1, self_lock_mac, 6) == 0 && memcmp(lock_mac1, data + 11, 6) == 0) {  // 这个位置有锁可以走锁关联任务
            ESP_LOG_BUFFER_HEX("进入锁1", self_lock_mac, 6);
            memset(self_lock_mac, 0, sizeof(self_lock_mac));   
 
            return 0x01;
        }
    } else if (lock1_flag == 0 ) {  // 这个位置没有锁可以走锁关联任务
        return 0x00;
    }
    return 0x00;
}

static uint8_t get_lock_mac2(uint8_t *data) { 
    if (lock2_flag == 1) {  // 这个位置已经存锁不能走锁关联任务
       if (memcmp(lock_mac2, self_lock_mac, 6) == 0&& memcmp(lock_mac2, data + 11, 6) == 0) {  // 这个位置有锁可以走锁关联任务 
        ESP_LOG_BUFFER_HEX("进入锁2", self_lock_mac, 6);
        memset(self_lock_mac, 0, sizeof(self_lock_mac));   
        return 0x01;
        }
    } else if (lock2_flag == 0) {  // 这个位置没有锁可以走锁关联任务
      
        return 0x00;
    }
    return 0x00;
}

static uint8_t get_lock_mac3(uint8_t *data) { 
    if (lock3_flag == 1) {  // 这个位置已经存锁不能走锁关联任务
        if (memcmp(lock_mac3, data + 11, 6) == 0&& memcmp(lock_mac3, self_lock_mac, 6) == 0) {  // 这个位置有锁可以走锁关联任务 
            memset(self_lock_mac, 0, sizeof(self_lock_mac));   

            return 0x01;
        }
    } else if (lock3_flag == 0) {  // 这个位置没有锁可以走锁关联任务
        
        return 0x00;
    }
   
    return 0x00;
}

static uint8_t get_lock_mac4(uint8_t *data) { 
    if (lock4_flag == 1) {  // 这个位置已经存锁不能走锁关联任务
        if (memcmp(lock_mac4, data + 11, 6) == 0&& memcmp(lock_mac4, self_lock_mac, 6) == 0) {  // 这个位置有锁可以走锁关联任务 
            memset(self_lock_mac, 0, sizeof(self_lock_mac));   

            return 0x01;
        }
    } else if (lock4_flag == 0) {  // 这个位置没有锁可以走锁关联任务
        
        return 0x00;
    }
   
    return 0x00;
}

static uint8_t get_lock_mac5(uint8_t *data) { 
    if (lock5_flag == 1) {  // 这个位置已经存锁不能走锁关联任务
      if (memcmp(lock_mac5, data + 11, 6) == 0&& memcmp(lock_mac5, self_lock_mac, 6) == 0) {  // 这个位置有锁可以走锁关联任务 
        memset(self_lock_mac, 0, sizeof(self_lock_mac));   

        return 0x01;
        }
    } else if (lock5_flag == 0) {  // 这个位置没有锁可以走锁关联任务
 
        return 0x00;   
    }
    return 0x00;
}

static uint8_t get_lock_mac6(uint8_t *data) { 
        if (lock6_flag == 1) {  // 这个位置已经存锁不能走锁关联任务
         if (memcmp(lock_mac6, data + 11, 6) == 0&& memcmp(lock_mac6, self_lock_mac, 6) == 0) {  // 这个位置有锁可以走锁关联任务 
        
            memset(self_lock_mac, 0, sizeof(self_lock_mac));   
 
            return 0x01;
        }
    } else if (lock6_flag == 0) {  // 这个位置没有锁可以走锁关联任务
        return 0x00;
    }
    return 0x00;
}



 void send_data_to_lock(esp_gatt_if_t gattc_if, uint16_t conn_id, uint16_t char_handle, uint8_t *data, uint16_t len)
{
    ESP_LOGI(GATTC_TAG, "准备发送数据到锁端, 数据长度: %d", len);
    ESP_LOG_BUFFER_HEX(GATTC_TAG, data, len);

    esp_gatt_status_t status = esp_ble_gattc_write_char(
        gattc_if,
        conn_id,
        char_handle,
        len,
        data,
        ESP_GATT_WRITE_TYPE_RSP,
        ESP_GATT_AUTH_REQ_NONE);
    
    if (status != ESP_GATT_OK) {
        ESP_LOGE(GATTC_TAG, "写入特征失败, 状态: 0x%x", status);
    } else {
        ESP_LOGI(GATTC_TAG, "写入特征请求已发送");
    }
}



















//*******************************************************客户端************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************// 

esp_err_t Blue_Broadcast() {
    setup_adv_data_raw();
    esp_ble_gap_start_advertising(&adv_params);
    return ESP_OK;
}


// 静态函数声明
static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
static void gatts_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
static void example_write_event_env(esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
//
static void gattc_profile_a_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);
static void gattc_profile_b_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);
static void gattc_profile_c_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);
static void gattc_profile_d_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);
static void gattc_profile_e_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);
static void gattc_profile_f_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);


 struct gattc_profile_inst gl_profile_tab1[PROFILE_NUMC] = {
    [PROFILE_A_APP_ID] = {
        .gattc_cb = gattc_profile_a_event_handler,  // 配置文件A回调函数
        .gattc_if = ESP_GATT_IF_NONE,               // 初始为无GATT接口
    },
    [PROFILE_B_APP_ID] = {
        .gattc_cb = gattc_profile_b_event_handler,  // 配置文件B回调函数
        .gattc_if = ESP_GATT_IF_NONE,               // 初始为无GATT接口
    },
    [PROFILE_C_APP_ID] = {
        .gattc_cb = gattc_profile_c_event_handler,  // 配置文件C回调函数
        .gattc_if = ESP_GATT_IF_NONE,               // 初始为无GATT接口
    },
    [PROFILE_D_APP_ID] = {
        .gattc_cb = gattc_profile_d_event_handler,  // 配置文件D回调函数
        .gattc_if = ESP_GATT_IF_NONE,               // 默认无GATT接口
    },
    [PROFILE_E_APP_ID] = {
        .gattc_cb = gattc_profile_e_event_handler,  // 配置文件E回调函数
        .gattc_if = ESP_GATT_IF_NONE,               // 默认无GATT接口
    },
    [PROFILE_F_APP_ID] = {
        .gattc_cb = gattc_profile_f_event_handler,  // 配置文件F回调函数
        .gattc_if = ESP_GATT_IF_NONE,               // 默认无GATT接口
    },


};



// GAP事件处理
static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
    uint8_t data[128]={0};//接收蓝牙广播的数据
    uint8_t len=0;


    switch (event) {
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
        ESP_LOGI(TAG, "广播数据设置完成, 状态 %d", param->adv_data_cmpl.status);
        break;
    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
        if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
            ESP_LOGE(TAG, "广播启动失败, 状态 %d", param->adv_start_cmpl.status);
            break;
        }
        ESP_LOGI(TAG, "广播启动成功");
        break;
    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
        ESP_LOGI(TAG, "连接参数更新, 状态 %d, 连接间隔 %d, 延迟 %d, 超时 %d",
                 param->update_conn_params.status,
                 param->update_conn_params.conn_int,
                 param->update_conn_params.latency,
                 param->update_conn_params.timeout);

      
        break;
        case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT: {
          
          
            break;
        }
        case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
            
            if (param->scan_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
               
                break;
            }
          
    
            break;
            case ESP_GAP_BLE_SCAN_RESULT_EVT: {
                // 类型转换：扫描结果参数
                esp_ble_gap_cb_param_t *scan_result = (esp_ble_gap_cb_param_t *)param;
                // 根据扫描事件子类型处理
                switch (scan_result->scan_rst.search_evt) {
                // 扫描到广播设备事件
                case ESP_GAP_SEARCH_INQ_RES_EVT:
               //打印设备广播的信息用16进制显示
              
              memcpy(data,scan_result->scan_rst.ble_adv,scan_result->scan_rst.adv_data_len);
              len=scan_result->scan_rst.adv_data_len;
              int index=memcmp(lock_data,data,7);                  
                if(index==0){
              //停止扫描
     
          if(get_lock_mac1(data)){
            esp_ble_gap_stop_scanning();
           // 连接参数配置
           esp_ble_gatt_creat_conn_params_t creat_conn_params1 = {0};
           // 复制远程设备地址
           memcpy(&creat_conn_params1.remote_bda, scan_result->scan_rst.bda, ESP_BD_ADDR_LEN);
           // 远程设备地址类型
           creat_conn_params1.remote_addr_type = scan_result->scan_rst.ble_addr_type;
           // 自身地址类型
           creat_conn_params1.own_addr_type = BLE_ADDR_TYPE_PUBLIC;
           // 直接连接
           creat_conn_params1.is_direct = true;
           // 禁用辅助广告连接
           creat_conn_params1.is_aux = false;
           // PHY掩码（默认）
           creat_conn_params1.phy_mask = 0x0;
           // 发起增强型BLE连接
           esp_ble_gattc_open(gl_profile_tab1[0].gattc_if, 
            creat_conn_params1.remote_bda, 
            creat_conn_params1.remote_addr_type, 
            true);
                               break; }
                           
            if(get_lock_mac2(data)){
            // 连接参数配置
            esp_ble_gap_stop_scanning();
            esp_ble_gatt_creat_conn_params_t creat_conn_params2 = {0};
            // 复制远程设备地址
            memcpy(&creat_conn_params2.remote_bda, scan_result->scan_rst.bda, ESP_BD_ADDR_LEN);
            // 远程设备地址类型
            creat_conn_params2.remote_addr_type = scan_result->scan_rst.ble_addr_type;
            // 自身地址类型
            creat_conn_params2.own_addr_type = BLE_ADDR_TYPE_PUBLIC;
            // 直接连接
            creat_conn_params2.is_direct = true;
            // 禁用辅助广告连接
            creat_conn_params2.is_aux = false;
            // PHY掩码（默认）
            creat_conn_params2.phy_mask = 0x0;
            // 发起增强型BLE连接
            esp_ble_gattc_enh_open(gl_profile_tab1[PROFILE_B_APP_ID].gattc_if,
                                &creat_conn_params2);
                                break;               
                            }
            if(get_lock_mac3(data)){
            // 连接参数配置
            esp_ble_gap_stop_scanning();
            esp_ble_gatt_creat_conn_params_t creat_conn_params3 = {0};
            // 复制远程设备地址
            memcpy(&creat_conn_params3.remote_bda, scan_result->scan_rst.bda, ESP_BD_ADDR_LEN);
            // 远程设备地址类型
            creat_conn_params3.remote_addr_type = scan_result->scan_rst.ble_addr_type;
            // 自身地址类型
            creat_conn_params3.own_addr_type = BLE_ADDR_TYPE_PUBLIC;
            // 直接连接
            creat_conn_params3.is_direct = true;
            // 禁用辅助广告连接
            creat_conn_params3.is_aux = false;
            // PHY掩码（默认）
            creat_conn_params3.phy_mask = 0x0;
            // 发起增强型BLE连接
            esp_ble_gattc_enh_open(gl_profile_tab1[PROFILE_C_APP_ID].gattc_if,
                                &creat_conn_params3);
                                break; }
                            
             if(get_lock_mac4(data)){
             // 连接参数配置
             esp_ble_gap_stop_scanning();
             esp_ble_gatt_creat_conn_params_t creat_conn_params = {0};
             // 复制远程设备地址
            memcpy(&creat_conn_params.remote_bda, scan_result->scan_rst.bda, ESP_BD_ADDR_LEN);
             // 远程设备地址类型
             creat_conn_params.remote_addr_type = scan_result->scan_rst.ble_addr_type;
             // 自身地址类型
             creat_conn_params.own_addr_type = BLE_ADDR_TYPE_PUBLIC;
             // 直接连接
             creat_conn_params.is_direct = true;
             // 禁用辅助广告连接
             creat_conn_params.is_aux = false;
             // PHY掩码（默认）
            creat_conn_params.phy_mask = 0x0;
             // 发起增强型BLE连接
             esp_ble_gattc_enh_open(gl_profile_tab1[PROFILE_D_APP_ID].gattc_if,
                                 &creat_conn_params);
                                 break;               
                             }

             if(get_lock_mac5(data)){
             // 连接参数配置
            // esp_ble_gap_stop_scanning();
             esp_ble_gatt_creat_conn_params_t creat_conn_params = {0};
             // 复制远程设备地址
            memcpy(&creat_conn_params.remote_bda, scan_result->scan_rst.bda, ESP_BD_ADDR_LEN);
             // 远程设备地址类型
             creat_conn_params.remote_addr_type = scan_result->scan_rst.ble_addr_type;
             // 自身地址类型
             creat_conn_params.own_addr_type = BLE_ADDR_TYPE_PUBLIC;
             // 直接连接
             creat_conn_params.is_direct = true;
             // 禁用辅助广告连接
             creat_conn_params.is_aux = false;
             // PHY掩码（默认）
            creat_conn_params.phy_mask = 0x0;
             // 发起增强型BLE连接
             esp_ble_gattc_enh_open(gl_profile_tab1[PROFILE_E_APP_ID].gattc_if,
                                 &creat_conn_params);
                                 break;               
                             }
              if(get_lock_mac6(data)){
                                // 连接参数配置
                // esp_ble_gap_stop_scanning();
                 esp_ble_gatt_creat_conn_params_t creat_conn_params = {0};
                  // 复制远程设备地址
                 memcpy(&creat_conn_params.remote_bda, scan_result->scan_rst.bda, ESP_BD_ADDR_LEN);
                                // 远程设备地址类型
                 creat_conn_params.remote_addr_type = scan_result->scan_rst.ble_addr_type;
                                // 自身地址类型
                creat_conn_params.own_addr_type = BLE_ADDR_TYPE_PUBLIC;
                                // 直接连接
                 creat_conn_params.is_direct = true;
                                // 禁用辅助广告连接
                 creat_conn_params.is_aux = false;
                                // PHY掩码（默认）
                creat_conn_params.phy_mask = 0x0;
                                // 发起增强型BLE连接
                 esp_ble_gattc_enh_open(gl_profile_tab1[PROFILE_F_APP_ID].gattc_if,
                                                    &creat_conn_params);
                                                    break;               
                                                }

       
           
                  
                   
                }  
                // 扫描完成事件（主动扫描结束）
                case ESP_GAP_SEARCH_INQ_CMPL_EVT:
                    break;
                // 其他未处理扫描子事件
                default:
                    break;
                }
                break;
            }
        case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:
            if (param->scan_stop_cmpl.status != ESP_BT_STATUS_SUCCESS){
             
                break;
            }
          
            break;
    
        case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
            if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS){
             
                break;
            }
          
            break;
  
        case ESP_GAP_BLE_SET_PKT_LENGTH_COMPLETE_EVT:
        
            break;
            default:
           
        break;    }
}

//******************************************* GATT服务器事件处理******************************************* //
static void gatts_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
    esp_err_t ret;
    
    switch (event) {
    case ESP_GATTS_REG_EVT:
        ESP_LOGI(TAG, "GATT服务器注册, 状态 %d, 应用ID %d", param->reg.status, param->reg.app_id);
        
        // 配置服务ID
        gl_profile_tab[CUSTOM_PROFILE_APP_ID].service_id.is_primary = true;
        gl_profile_tab[CUSTOM_PROFILE_APP_ID].service_id.id.inst_id = 0x00;
        gl_profile_tab[CUSTOM_PROFILE_APP_ID].service_id.id.uuid.len = ESP_UUID_LEN_16;
      
        gl_profile_tab[CUSTOM_PROFILE_APP_ID].service_id.id.uuid.uuid.uuid16 = custom_service_uuid[0] | (custom_service_uuid[1] << 8);
        Blue_Broadcast();

        // 创建服务
        ret = esp_ble_gatts_create_service(gatts_if, &gl_profile_tab[CUSTOM_PROFILE_APP_ID].service_id, CUSTOM_NUM_HANDLE);
        if (ret) {
            ESP_LOGE(TAG, "创建服务失败, 错误代码 = %x", ret);
        }
        break;
        
    case ESP_GATTS_CREATE_EVT:
        ESP_LOGI(TAG, "服务创建, 状态 %d, 服务句柄 %d", param->create.status, param->create.service_handle);
        
        if (param->create.status != ESP_GATT_OK) {
            ESP_LOGE(TAG, "服务创建失败");
            break;
        }
        
        gl_profile_tab[CUSTOM_PROFILE_APP_ID].service_handle = param->create.service_handle;
        
        // 启动服务
        ret = esp_ble_gatts_start_service(gl_profile_tab[CUSTOM_PROFILE_APP_ID].service_handle);
        if (ret) {
            ESP_LOGE(TAG, "启动服务失败, 错误代码 = %x", ret);
            break;
        }
        
        
            // 配置第一个特征属性：Write
            custom_property = ESP_GATT_CHAR_PROP_BIT_WRITE;
            
            // 设置第一个特征UUID
            gl_profile_tab[CUSTOM_PROFILE_APP_ID].char_uuid.len = ESP_UUID_LEN_16;
            gl_profile_tab[CUSTOM_PROFILE_APP_ID].char_uuid.uuid.uuid16 = custom_char_uuid[0] | (custom_char_uuid[1] << 8);
            
            // 添加第一个特征（Write）
            ret = esp_ble_gatts_add_char(gl_profile_tab[CUSTOM_PROFILE_APP_ID].service_handle, 
                                       &gl_profile_tab[CUSTOM_PROFILE_APP_ID].char_uuid,
                                       ESP_GATT_PERM_WRITE,
                                       custom_property,
                                       &custom_char_attr, 
                                       NULL);
            
            if (ret) {
                ESP_LOGE(TAG, "添加第一个特征失败, 错误代码 = %x", ret);
            }
        
        break;
        
   // 在 gatts_profile_event_handler 函数中修改 ESP_GATTS_ADD_CHAR_EVT 部分
case ESP_GATTS_ADD_CHAR_EVT:// 添加特征事件处理
ESP_LOGI(TAG, "特征添加, 状态 %d, 属性句柄 %d", 
         param->add_char.status, param->add_char.attr_handle);
         
if (param->add_char.status == ESP_GATT_OK) {
    // 判断是哪个特征
    if (gl_profile_tab[CUSTOM_PROFILE_APP_ID].char_handle == 0) {
        // 第一个特征（Write）
        gl_profile_tab[CUSTOM_PROFILE_APP_ID].char_handle = param->add_char.attr_handle;
        
        // 添加第二个特征（Notify）
        custom_property2 = ESP_GATT_CHAR_PROP_BIT_NOTIFY;
        gl_profile_tab[CUSTOM_PROFILE_APP_ID].char_uuid2.len = ESP_UUID_LEN_16;
        gl_profile_tab[CUSTOM_PROFILE_APP_ID].char_uuid2.uuid.uuid16 = 
            custom_char_uuid2[0] | (custom_char_uuid2[1] << 8);
        
        ret = esp_ble_gatts_add_char(gl_profile_tab[CUSTOM_PROFILE_APP_ID].service_handle, 
                                   &gl_profile_tab[CUSTOM_PROFILE_APP_ID].char_uuid2,
                                   ESP_GATT_PERM_READ,
                                   custom_property2,
                                   &custom_char_attr2, 
                                   NULL);
    } else {
        // 第二个特征（Notify）
        gl_profile_tab[CUSTOM_PROFILE_APP_ID].char_handle2 = param->add_char.attr_handle;
        
        // 为第二个特征添加CCCD描述符
        gl_profile_tab[CUSTOM_PROFILE_APP_ID].descr_uuid2.len = ESP_UUID_LEN_16;
        gl_profile_tab[CUSTOM_PROFILE_APP_ID].descr_uuid2.uuid.uuid16 = 
            ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
        
        ret = esp_ble_gatts_add_char_descr(gl_profile_tab[CUSTOM_PROFILE_APP_ID].service_handle, 
                                         &gl_profile_tab[CUSTOM_PROFILE_APP_ID].descr_uuid2,
                                         ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, 
                                         NULL, NULL);
    }
}
break;
        
 case ESP_GATTS_ADD_CHAR_DESCR_EVT:// 添加特征描述符事件处理
            // 位置4：描述符添加完成事件，这里只做日志记录和状态确认
            ESP_LOGI(TAG, "描述符添加完成, 状态 %d, 属性句柄 %d",
                param->add_char_descr.status, param->add_char_descr.attr_handle);
       if (param->add_char_descr.status == ESP_GATT_OK) {
           gl_profile_tab[CUSTOM_PROFILE_APP_ID].descr_handle2 = param->add_char_descr.attr_handle;
           custom_service_created = true; // 标记服务创建全部完成
           ESP_LOGI(TAG, "自定义GATT服务创建完成，包含两个特征");
       }
       break;
        
    case ESP_GATTS_WRITE_EVT:
    // 特征写入事件处理
        ESP_LOGI(ccTAG, "特征写入, 值长度 %u", param->write.len);
        is_notify_enabled = true;
        // 检查是否是第一个特征被写入
        if (param->write.handle == gl_profile_tab[CUSTOM_PROFILE_APP_ID].char_handle) {
            // 打印接收到的数据
            if (param->write.len > 0) {
               

                




                // 更新特征值
                if (param->write.len <= 64) {
                  //  memcpy(custom_char_value, param->write.value, param->write.len);
                  //  custom_char_attr.attr_len = param->write.len;
                  // 
                  //  // 设置属性值
                  

                uint8_t data=param->write.value[0];
                if(data==0x2b){
                    ESP_LOGI(ccTAG, "接收到WIFI名称:");
                    led_state_wifi=LED_WIFI_CLINK;
                    memcpy(Wifi_ssid, param->write.value+1, param->write.len-1);
                   //打印这个数组
                  ESP_LOG_BUFFER_HEX(ccTAG, Wifi_ssid, param->write.len-1);
                   wifi_ssid_len=param->write.len-1;
                   
                   
                }
                else if(data==0x2d){
                    ESP_LOGI(ccTAG, "接收到WIFI密码:");
                 
                    memcpy(Wifi_password, param->write.value+1, param->write.len-1);
                    //打印这个数组
                    ESP_LOG_BUFFER_HEX(ccTAG, Wifi_password, param->write.len-1);}
                    wifi_password_len=param->write.len-1;
                 
                    wifi_manager_connect((const char*)Wifi_ssid,(const char*)Wifi_password); 
                    ESP_LOGI(ccTAG, " SSID: %s, Password: %s", 
                       Wifi_ssid , Wifi_password);


                  
                }
            }
        }
        // 检查是否是第二个特征被写入
        else if (param->write.handle == gl_profile_tab[CUSTOM_PROFILE_APP_ID].descr_handle2) {
            // 处理客户端特征配置描述符写入（启用/禁用Notify）
            ESP_LOGI(ccTAG, "接收到第2个特征数据:");
            ESP_LOG_BUFFER_HEX(ccTAG, param->write.value, param->write.len);
            ESP_LOGI(TAG, "客户端特征配置描述符写入, 值长度 %u", param->write.len);
            if (param->write.len == 2) {
                uint8_t descr_value = param->write.value[0]<<4;
                ESP_LOGI(TAG, "descr_value的值%x ", descr_value);
                ESP_LOGI(TAG, "descr_value的值%x ", ESP_GATT_CHAR_PROP_BIT_NOTIFY);
                

                if (descr_value == ESP_GATT_CHAR_PROP_BIT_NOTIFY) {
                    is_notify_enabled = true;
                    ESP_LOGI(TAG, "Notify已启用");
                } else {
                    is_notify_enabled = false;
                    ESP_LOGI(TAG, "Notify已禁用");
                }
            }
        }
        // 处理写入响应
        example_write_event_env(gatts_if, param);
        break;
        
    case ESP_GATTS_READ_EVT:
        ESP_LOGI(TAG, "特征读取请求");
        break;
        
    case ESP_GATTS_CONNECT_EVT:
        ESP_LOGI(TAG, "设备连接, 连接ID %u", param->connect.conn_id);
        gl_profile_tab[CUSTOM_PROFILE_APP_ID].conn_id = param->connect.conn_id;
        is_notify_enabled = true; // 连接时重置Notify状态
        break;
        
    case ESP_GATTS_DISCONNECT_EVT:
        ESP_LOGI(TAG, "设备断开连接, 原因 0x%02x", param->disconnect.reason);
        is_notify_enabled = false; // 断开连接时禁用Notify
        // 重新开始广播

        if(key_user==0){
        esp_ble_gap_start_advertising(&adv_params);}
        break;
        
    case ESP_GATTS_MTU_EVT:
        ESP_LOGI(TAG, "MTU更新: %d", param->mtu.mtu);
        break;
        


    default:
        ESP_LOGD(TAG, "未处理事件: %d", event);
        break;
    }
}

//*********************************************GATT服务器事件分发器************************************************//
static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
    if (event == ESP_GATTS_REG_EVT) {
        if (param->reg.status == ESP_GATT_OK) {
            gl_profile_tab[param->reg.app_id].gatts_if = gatts_if;
        } else {
            ESP_LOGI(TAG, "注册应用失败, 应用ID %04x, 状态 %d",
                    param->reg.app_id,
                    param->reg.status);
            return;
        }
    }

    // 调用对应的profile回调函数
    for (int idx = 0; idx < PROFILE_NUMS; idx++) {
        if (gatts_if == ESP_GATT_IF_NONE || gatts_if == gl_profile_tab[idx].gatts_if) {
            if (gl_profile_tab[idx].gatts_cb) {
                gl_profile_tab[idx].gatts_cb(event, gatts_if, param);
            }
        }
    }
}

// 写入事件环境处理
static void example_write_event_env(esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
    if (param->write.need_rsp) {
        esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
    }
}

// 发送Notify数据
esp_err_t BLUE_SendNotifyData(const uint8_t *data, uint16_t length) {
    // 添加状态检查
  
    if (length > 244) {
        ESP_LOGE(TAG, "数据长度超过244字节限制");
        return ESP_FAIL;
    }
    
    esp_err_t ret = esp_ble_gatts_send_indicate(
        gl_profile_tab[CUSTOM_PROFILE_APP_ID].gatts_if,
        gl_profile_tab[CUSTOM_PROFILE_APP_ID].conn_id,
        gl_profile_tab[CUSTOM_PROFILE_APP_ID].char_handle2,
        length,
        data,
        false);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "发送Notify失败: %s", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Notify发送成功，长度: %d", length);
    }
    
    return ret;
}



// 公共函数实现
esp_err_t BLUE_GATT_Init(void) {
    esp_err_t ret;

    // 设置Profile回调
    gl_profile_tab[CUSTOM_PROFILE_APP_ID].gatts_cb = gatts_profile_event_handler;

    // 注册GAP回调
    ret = esp_ble_gap_register_callback(gap_event_handler);
    if (ret) {
        ESP_LOGE(TAG, "GAP回调注册错误, 错误代码 = %x", ret);
        return ret;
    }

    // 注册GATT服务端回调
    ret = esp_ble_gatts_register_callback(gatts_event_handler);
    if (ret) {
        ESP_LOGE(TAG, "GATT回调注册错误, 错误代码 = %x", ret);
        return ret;
    }

    // 注册GATT服务端应用
    ret = esp_ble_gatts_app_register(CUSTOM_PROFILE_APP_ID);
    if (ret) {
        ESP_LOGE(TAG, "应用注册错误, 错误代码 = %x", ret);
        return ret;
    }

    
    ret = esp_ble_gattc_register_callback(esp_gattc_cb);

    ret= esp_ble_gattc_app_register(PROFILE_A_APP_ID);
    ret= esp_ble_gattc_app_register(PROFILE_B_APP_ID);
    ret= esp_ble_gattc_app_register(PROFILE_C_APP_ID);
    ret= esp_ble_gattc_app_register(PROFILE_D_APP_ID);
    ret= esp_ble_gattc_app_register(PROFILE_E_APP_ID);
    ret= esp_ble_gattc_app_register(PROFILE_F_APP_ID);


    // 设置较大的MTU以支持244字节数据
    ret = esp_ble_gatt_set_local_mtu(500);
    if (ret) {
        ESP_LOGE(TAG, "设置本地MTU失败, 错误代码 = %x", ret);
        return ret;
    }

    ESP_LOGI(TAG, "GATT服务初始化完成");
    return ESP_OK;
}


esp_err_t BLUE_StartAdvertising(void) {
    setup_adv_data_raw();
    esp_err_t ret = esp_ble_gap_start_advertising(&adv_params);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "启动广播失败: %s", esp_err_to_name(ret));
    }
    return ret;
}

/*****************************
锁端回报数据给TCP服务器
data:数据
len:数据长度
index:锁的索引
*****************************/
void lock_send_tcp_sever(uint8_t *data, uint16_t len, uint8_t index) {
 switch (index)
 {
 case 1:
 memcpy(lock1_send_data+lock1_send_len,data,len);
 lock1_send_len+=len;
if(data[len-1]==0x6e&&data[len-2]==0x72&&data[len-3]==0xb5){
    ESP_LOGI(TAG, "锁1发送数据到服务器:");
    TCP_Send_data(lock1_send_data,lock1_send_len);
    memset(lock1_send_data,0,lock1_send_len);
    lock1_send_len=0;

}


    break;
case 2:
memcpy(lock2_send_data+lock2_send_len,data,len);
lock2_send_len+=len;
if(data[len-1]==0x6e&&data[len-2]==0x72&&data[len-3]==0xb5){
   ESP_LOGI(TAG, "锁2发送数据到服务器:");
   TCP_Send_data(lock2_send_data,lock2_send_len);
   memset(lock2_send_data,0,lock2_send_len);
   lock2_send_len=0;

}

    break;
case 3:
memcpy(lock3_send_data+lock3_send_len,data,len);
lock3_send_len+=len;
if(data[len-1]==0x6e&&data[len-2]==0x72&&data[len-3]==0xb5){
   ESP_LOGI(TAG, "锁3发送数据到服务器:");
   TCP_Send_data(lock3_send_data,lock3_send_len);
   memset(lock3_send_data,0,lock3_send_len);
   lock3_send_len=0;

}
    break;
case 4:
memcpy(lock4_send_data+lock4_send_len,data,len);
lock4_send_len+=len;
if(data[len-1]==0x6e&&data[len-2]==0x72&&data[len-3]==0xb5){
   ESP_LOGI(TAG, "锁4发送数据到服务器:");
   TCP_Send_data(lock4_send_data,lock4_send_len);
   memset(lock4_send_data,0,lock4_send_len);
   lock4_send_len=0;

}

    break;
case 5:
memcpy(lock5_send_data+lock5_send_len,data,len);
lock5_send_len+=len;
if(data[len-1]==0x6e&&data[len-2]==0x72&&data[len-3]==0xb5){
   ESP_LOGI(TAG, "锁5发送数据到服务器:");
   TCP_Send_data(lock5_send_data,lock5_send_len);
   memset(lock5_send_data,0,lock5_send_len);
   lock5_send_len=0;

}
    break;
case 6:
memcpy(lock6_send_data+lock6_send_len,data,len);
lock6_send_len+=len;
if(data[len-1]==0x6e&&data[len-2]==0x72&&data[len-3]==0xb5){
   ESP_LOGI(TAG, "锁6发送数据到服务器:");
   TCP_Send_data(lock6_send_data,lock6_send_len);
   memset(lock6_send_data,0,lock6_send_len);
   lock6_send_len=0;

}

    break;
 
 default:
    break;
 }
    

}













//******************************************************************GATT客户端出俩函数***********************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************客户端************************************************************************************************************// 

static void gattc_profile_a_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{
    esp_ble_gattc_cb_param_t *p_data = (esp_ble_gattc_cb_param_t *)param;  // 参数指针


    switch (event) {
    case ESP_GATTC_REG_EVT:  // GATT客户端注册事件
        ESP_LOGI(GATTC_TAG, "GATT客户端注册, 状态 %d, 应用ID %d, GATT接口 %d", param->reg.status, param->reg.app_id, gattc_if);
        esp_err_t scan_ret = esp_ble_gap_set_scan_params(&ble_scan_params);  // 设置扫描参数
        if (scan_ret){
            ESP_LOGE(GATTC_TAG, "设置扫描参数错误, 错误代码 = %x", scan_ret);
        }
        break;
    case ESP_GATTC_CONNECT_EVT:{  // 连接事件
       //判断是不是我要找的mac
       if (memcmp(p_data->connect.remote_bda, lock_mac1, 6) != 0) {
        ESP_LOGI(GATTC_TAG, "忽略非本 Profile 的连接事件");
        break; // 不保存 conn_id，也不发送 MTU 请求
         }
        gl_profile_tab1[PROFILE_A_APP_ID].conn_id = p_data->connect.conn_id;  // 保存连接ID
        ESP_LOGI(TAG, "保存后的PROFILE_A_APP_IDconnid: %d", gl_profile_tab1[PROFILE_A_APP_ID].conn_id);


        memcpy(gl_profile_tab1[PROFILE_A_APP_ID].remote_bda, p_data->connect.remote_bda, sizeof(esp_bd_addr_t));  // 复制远程设备地址
        esp_err_t mtu_ret = esp_ble_gattc_send_mtu_req (gattc_if, p_data->connect.conn_id);  // 发送MTU请求
        if (mtu_ret){
            ESP_LOGE(GATTC_TAG, "配置MTU错误, 错误代码 = %x", mtu_ret);
        }


     
        break;
    }
    case ESP_GATTC_OPEN_EVT:  // 打开连接事件
        if (param->open.status != ESP_GATT_OK){
            ESP_LOGE(GATTC_TAG, "打开失败, 状态 %d", p_data->open.status);
            break;
        }
       // ESP_LOGI(GATTC_TAG, "成功打开, MTU %u", p_data->open.mtu);
       conn_device_a = true;  // 标记设备A已连接
        break;
    case ESP_GATTC_DIS_SRVC_CMPL_EVT:  // 发现服务完成事件
        if (param->dis_srvc_cmpl.status != ESP_GATT_OK){
            ESP_LOGE(GATTC_TAG, "服务发现失败, 状态 %d", param->dis_srvc_cmpl.status);
            break;
        }
        ESP_LOGI(GATTC_TAG, "服务发现完成, 连接ID %d", param->dis_srvc_cmpl.conn_id);
         // 打印要搜索的服务UUID
         ESP_LOGI(GATTC_TAG, "开始搜索服务, UUID:");
         esp_log_buffer_hex(GATTC_TAG, remote_filter_service_uuid.uuid.uuid128, 16);
        esp_ble_gattc_search_service(gattc_if, param->dis_srvc_cmpl.conn_id, NULL);  // 搜索特定UUID的服务
        break;
    case ESP_GATTC_CFG_MTU_EVT:  // MTU配置事件
        ESP_LOGI(GATTC_TAG, "MTU交换, 状态 %d, MTU %d", param->cfg_mtu.status, param->cfg_mtu.mtu);
        break;
        case ESP_GATTC_SEARCH_RES_EVT: {  // 服务搜索结果事件
            ESP_LOGI(GATTC_TAG, "服务搜索结果, 连接ID = %x", p_data->search_res.conn_id);
            ESP_LOGI(GATTC_TAG, "起始句柄 %d, 结束句柄 %d", 
                     p_data->search_res.start_handle, p_data->search_res.end_handle);
        
            if (p_data->search_res.srvc_id.uuid.len == ESP_UUID_LEN_128) {
             
                ESP_LOGI(GATTC_TAG, "服务UUID128:");
                esp_log_buffer_hex(GATTC_TAG, p_data->search_res.srvc_id.uuid.uuid.uuid128, 16);
                
             
                
                // BLE UUID是小端序，需要反转比较
                uint8_t reversed_uuid[16];
                for (int i = 0; i < 16; i++) {
                    reversed_uuid[i] = p_data->search_res.srvc_id.uuid.uuid.uuid128[15 - i];
                }
                
               // ESP_LOGI(GATTC_TAG, "反转后服务UUID128:");
              //  esp_log_buffer_hex(GATTC_TAG, reversed_uuid, 16);
                
                // 比较UUID（注意BLE是小端序）
                if (memcmp(reversed_uuid, remote_filter_service_uuid.uuid.uuid128, 16) == 0) {
                    ESP_LOGI(GATTC_TAG, "*** 匹配到目标服务 ***");
                    get_servera = true;
                    gl_profile_tab1[PROFILE_A_APP_ID].service_start_handle = p_data->search_res.start_handle;
                    gl_profile_tab1[PROFILE_A_APP_ID].service_end_handle = p_data->search_res.end_handle;
                   
                } else {
                    ESP_LOGI(GATTC_TAG, "UUID不匹配");
                }
            }
            break;
        }
        case ESP_GATTC_SEARCH_CMPL_EVT:  // 服务搜索完成事件
        ESP_LOGI(GATTC_TAG, "服务搜索完成, 状态 %d", p_data->search_cmpl.status);
        ESP_LOGI(GATTC_TAG, "get_server标志: %d", get_servera);
        
        if (p_data->search_cmpl.status != ESP_GATT_OK){
            ESP_LOGE(GATTC_TAG, "服务搜索失败, 状态 %x", p_data->search_cmpl.status);
            break;
        }
        
        if (get_servera){  // 如果找到目标服务
            uint16_t count = 0;
            esp_gatt_status_t status = esp_ble_gattc_get_attr_count( 
                gattc_if,
                p_data->search_cmpl.conn_id,
                ESP_GATT_DB_CHARACTERISTIC,
                gl_profile_tab1[PROFILE_A_APP_ID].service_start_handle,
                gl_profile_tab1[PROFILE_A_APP_ID].service_end_handle,
                INVALID_HANDLE,
                &count);
                
            if (status != ESP_GATT_OK){
                ESP_LOGE(GATTC_TAG, "获取特征数量错误, 状态: %d", status);
                break;
            }
            
            ESP_LOGI(GATTC_TAG, "找到 %d 个特征", count);
            
            if (count > 0){
                // 1. 首先获取并显示所有特征
                char_elem_result_a = (esp_gattc_char_elem_t *)malloc(sizeof(esp_gattc_char_elem_t) * count);
                if (!char_elem_result_a){
                    ESP_LOGE(GATTC_TAG, "内存不足, 无法分配特征元素数组");
                    break;
                }
                
                // 获取所有特征
                status = esp_ble_gattc_get_all_char(
                    gattc_if,
                    p_data->search_cmpl.conn_id,
                    gl_profile_tab1[PROFILE_A_APP_ID].service_start_handle,
                    gl_profile_tab1[PROFILE_A_APP_ID].service_end_handle,
                    char_elem_result_a,
                    &count,
                    0); // 偏移量0
                    
                if (status == ESP_GATT_OK && count > 0) {
                    ESP_LOGI(GATTC_TAG, "========== 服务中的所有特征 ==========");
                    for (int i = 0; i < count; i++) {
                        ESP_LOGI(GATTC_TAG, "特征[%d]: 句柄=%d, 属性=0x%02x", 
                                 i, char_elem_result_a[i].char_handle, char_elem_result_a[i].properties);
                        
                        // 打印特征属性描述
                        if (char_elem_result_a[i].properties & ESP_GATT_CHAR_PROP_BIT_READ) {
                            ESP_LOGI(GATTC_TAG, "  - 支持读取");
                        }
                        if (char_elem_result_a[i].properties & ESP_GATT_CHAR_PROP_BIT_WRITE) {
                            ESP_LOGI(GATTC_TAG, "  - 支持写入");
                        }
                        if (char_elem_result_a[i].properties & ESP_GATT_CHAR_PROP_BIT_WRITE_NR) {
                            ESP_LOGI(GATTC_TAG, "  - 支持无响应写入");
                        }
                        if (char_elem_result_a[i].properties & ESP_GATT_CHAR_PROP_BIT_NOTIFY) {
                            ESP_LOGI(GATTC_TAG, "  - 支持通知");
                        }
                        if (char_elem_result_a[i].properties & ESP_GATT_CHAR_PROP_BIT_INDICATE) {
                            ESP_LOGI(GATTC_TAG, "  - 支持指示");
                        }
                        
                        // 打印特征UUID
                        if (char_elem_result_a[i].uuid.len == ESP_UUID_LEN_128) {
                            ESP_LOGI(GATTC_TAG, "  - UUID128:");
                            esp_log_buffer_hex(GATTC_TAG, char_elem_result_a[i].uuid.uuid.uuid128, 16);
                            
                            // 反转UUID（BLE是小端序）
                            uint8_t reversed_uuid[16];
                            for (int j = 0; j < 16; j++) {
                                reversed_uuid[j] = char_elem_result_a[i].uuid.uuid.uuid128[15 - j];
                            }
                            ESP_LOGI(GATTC_TAG, "  - 反转后UUID128:");
                            esp_log_buffer_hex(GATTC_TAG, reversed_uuid, 16);
                        }
                        ESP_LOGI(GATTC_TAG, "----------------------------------------");
                    }
                    
                    // 2. 直接遍历特征数组，查找需要的特征
                    uint16_t write_char_handle = 0;
                    uint16_t notify_char_handle = 0;
                    
                    for (int i = 0; i < count; i++) {
                        // 检查特征UUID
                        if (char_elem_result_a[i].uuid.len == ESP_UUID_LEN_128) {
                            // 反转UUID进行比较
                            uint8_t reversed_uuid[16];
                            for (int j = 0; j < 16; j++) {
                                reversed_uuid[j] = char_elem_result_a[i].uuid.uuid.uuid128[j];
                            }
                            
                            // 检查是否是写特征
                            esp_bt_uuid_t target_write_uuid;
                            target_write_uuid.len = ESP_UUID_LEN_128;
                            for (int j = 0; j < 16; j++) {
                                target_write_uuid.uuid.uuid128[j] = write_filter_char_uuid.uuid.uuid128[15 - j];
                            }

                            ESP_LOGI(GATTC_TAG, "  - 检查特征UUID:");
                            esp_log_buffer_hex(GATTC_TAG, target_write_uuid.uuid.uuid128, 16);
                            if (memcmp(reversed_uuid, target_write_uuid.uuid.uuid128, 16) == 0) {
                                write_char_handle = char_elem_result_a[i].char_handle;
                                ESP_LOGI(GATTC_TAG, "找到写特征: 句柄=%d", write_char_handle);
                            }
                            
                            // 检查是否是通知特征
                            esp_bt_uuid_t target_notify_uuid;
                            target_notify_uuid.len = ESP_UUID_LEN_128;
                            for (int j = 0; j < 16; j++) {
                                target_notify_uuid.uuid.uuid128[j] = notify_descr_uuid.uuid.uuid128[15 - j];
                            }
                            
                            if (memcmp(reversed_uuid, target_notify_uuid.uuid.uuid128, 16) == 0) {
                                notify_char_handle = char_elem_result_a[i].char_handle;
                                ESP_LOGI(GATTC_TAG, "找到通知特征: 句柄=%d", notify_char_handle);
                            }
                        }
                    }
                    
                    // 3. 处理找到的特征
                    if (write_char_handle != 0) {
                        gl_profile_tab1[PROFILE_A_APP_ID].write_char_handle = write_char_handle;
                        ESP_LOGI(GATTC_TAG, "使用写特征句柄: %d", write_char_handle);
                        
                        if (notify_char_handle != 0) {
                       
                            esp_ble_gattc_register_for_notify(
                                gattc_if, 
                                gl_profile_tab1[PROFILE_A_APP_ID].remote_bda, 
                                notify_char_handle);
                        } else {
                            ESP_LOGW(GATTC_TAG, "未找到通知特征");
                        }
                        
                        // 4. 直接发送开锁数据
                       
                    } else {
                        ESP_LOGE(GATTC_TAG, "未找到写特征");
                    }
                } else {
                    ESP_LOGE(GATTC_TAG, "获取所有特征失败, 状态: %d", status);
                }
                
                free(char_elem_result_a);
                char_elem_result_a = NULL;
            } else {
                ESP_LOGE(GATTC_TAG, "服务中没有特征");
            }
        } else {
            ESP_LOGE(GATTC_TAG, "未找到目标服务");
        }
        break;
        case ESP_GATTC_REG_FOR_NOTIFY_EVT: {  // 注册通知事件
            if (p_data->reg_for_notify.status != ESP_GATT_OK){
                ESP_LOGE(GATTC_TAG, "通知注册失败, 状态 %d", p_data->reg_for_notify.status);
            } else {
                ESP_LOGI(GATTC_TAG, " 通知注册成功");
                
           
                
                // 启用通知（写入CCCD）
                uint16_t count = 0;
                uint16_t notify_en = 1;  // 启用通知的值
                
                // 使用标准的CCCD UUID (0x2902)
                esp_bt_uuid_t cccd_uuid = {
                    .len = ESP_UUID_LEN_16,
                    .uuid = {.uuid16 = 0x2902}
                };
        
                esp_gatt_status_t ret_status = esp_ble_gattc_get_attr_count( 
                    gattc_if,
                    gl_profile_tab1[PROFILE_A_APP_ID].conn_id,
                    ESP_GATT_DB_DESCRIPTOR,
                    gl_profile_tab1[PROFILE_A_APP_ID].service_start_handle,
                    gl_profile_tab1[PROFILE_A_APP_ID].service_end_handle,
                    p_data->reg_for_notify.handle,  // 通知特征的句柄
                    &count);
                    
                if (ret_status != ESP_GATT_OK){
                    ESP_LOGE(GATTC_TAG, "获取描述符数量错误, 状态: %d", ret_status);
                    // 如果获取描述符失败，直接发送数据
                    ESP_LOGI(GATTC_TAG, " 无法启用通知，直接发送数据");
                    
                    break;
                }
                
                ESP_LOGI(GATTC_TAG, "找到 %d 个描述符", count);
                
                if (count > 0){
                    descr_elem_result_a = malloc(sizeof(esp_gattc_descr_elem_t) * count);
                    if (!descr_elem_result_a){
                        ESP_LOGE(GATTC_TAG, "内存分配错误");
                        // 内存分配失败，直接发送数据
                        ESP_LOGI(GATTC_TAG, " 内存不足，直接发送数据");
                       
                        break;
                    }
                    
                    // 搜索CCCD描述符
                    ret_status = esp_ble_gattc_get_descr_by_char_handle(
                        gattc_if,
                        gl_profile_tab1[PROFILE_A_APP_ID].conn_id,
                        p_data->reg_for_notify.handle,
                        cccd_uuid,
                        descr_elem_result_a,
                        &count);
                        
                    if (ret_status != ESP_GATT_OK || count == 0){
                      ///  ESP_LOGE(GATTC_TAG, "获取CCCD描述符失败, 状态: %d, 数量: %d", ret_status, count);
                        free(descr_elem_result_a);
                        descr_elem_result_a = NULL;
                        // 如果找不到CCCD，直接发送数据
                      ///  ESP_LOGI(GATTC_TAG, " 未找到CCCD描述符，直接发送数据");
                       
                        break;
                    }
                    
                 ///   ESP_LOGI(GATTC_TAG, "找到CCCD描述符, 句柄: %d", descr_elem_result_a[0].handle);
                    
                    // 写入CCCD以启用通知
                    ret_status = esp_ble_gattc_write_char_descr(
                        gattc_if,
                        gl_profile_tab1[PROFILE_A_APP_ID].conn_id,
                        descr_elem_result_a[0].handle,
                        sizeof(notify_en),
                        (uint8_t *)&notify_en,
                        ESP_GATT_WRITE_TYPE_RSP,
                        ESP_GATT_AUTH_REQ_NONE);
                        
                    if (ret_status != ESP_GATT_OK){
                    //    ESP_LOGE(GATTC_TAG, "写入CCCD描述符失败, 状态: %d", ret_status);
                        // 写入失败，直接发送数据
                   //     ESP_LOGI(GATTC_TAG, " 启用通知失败，直接发送数据");
                       
                    } else {
                  //      ESP_LOGI(GATTC_TAG, " 写入CCCD描述符请求已发送，等待启用完成...");
                        // 数据发送将在ESP_GATTC_WRITE_DESCR_EVT事件中处理
                    }
                    
                    free(descr_elem_result_a);
                    descr_elem_result_a = NULL;
                } else {
                    ESP_LOGW(GATTC_TAG, "未找到描述符");
                
                }
            }
            break;
        }
    case ESP_GATTC_NOTIFY_EVT:  // 通知事件

   
     // TCP_Send_data(p_data->notify.value,p_data->notify.value_len);
    

      lock_send_tcp_sever(p_data->notify.value,p_data->notify.value_len,1);
   //  ESP_LOGI(GATTC_TAG, "进入次数: %02x",lock1_send_flag);




        break;
        case ESP_GATTC_WRITE_DESCR_EVT:  // 写入描述符事件
        if (p_data->write.status != ESP_GATT_OK){
            ESP_LOGE(GATTC_TAG, " 描述符写入失败, 状态 %x", p_data->write.status);
            // 即使描述符写入失败，也尝试发送数据
          
           
        } else {
         
            // 现在可以安全地发送数据
            ESP_LOGI(GATTC_TAG, " 发送开锁数据...");
           //发送开锁数据
           send_data_to_lock(gattc_if, 
            gl_profile_tab1[PROFILE_A_APP_ID].conn_id,
            gl_profile_tab1[PROFILE_A_APP_ID].write_char_handle,
            TCP_lock_data, 
            TCP_lock_data_len);
        
        
        }
        break;
    case ESP_GATTC_SRVC_CHG_EVT: {  // 服务变更事件
        esp_bd_addr_t bda;
        memcpy(bda, p_data->srvc_chg.remote_bda, sizeof(esp_bd_addr_t));  // 复制设备地址
        ESP_LOGI(GATTC_TAG, "服务变更来自 "ESP_BD_ADDR_STR"", ESP_BD_ADDR_HEX(bda));
        break;
    }
    case ESP_GATTC_WRITE_CHAR_EVT:  // 写入特征事件
        if (p_data->write.status != ESP_GATT_OK){
         //   ESP_LOGE(GATTC_TAG, "特征写入失败, 状态 %x", p_data->write.status);
            break;
        }
        ESP_LOGI(GATTC_TAG, "特征写入成功");
        break;
    case ESP_GATTC_DISCONNECT_EVT:  // 断开连接事件
    if (memcmp(p_data->connect.remote_bda+3, lock_mac1, 6) != 0) {
        ESP_LOGI(GATTC_TAG, "忽略非本 Profile 的断开事件");
        break; // 不保存 conn_id，也不发送 MTU 请求
         }
        conn_device_a = false;     // 重置连接标志
        get_servera = false;  // 重置获取服务器标志
        ESP_LOGI(GATTC_TAG, "已断开连接, 远程设备A "ESP_BD_ADDR_STR", 原因 0x%02x",
                 ESP_BD_ADDR_HEX(p_data->disconnect.remote_bda), p_data->disconnect.reason);
        break;
    default:
        break;
    }
}


static void gattc_profile_b_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{
    esp_ble_gattc_cb_param_t *p_data = (esp_ble_gattc_cb_param_t *)param;  // 参数指针

    switch (event) {
    case ESP_GATTC_REG_EVT:  // GATT客户端注册事件
        ESP_LOGI(GATTC_TAG, "GATT客户端注册, 状态 %d, 应用ID %d, GATT接口 %d", param->reg.status, param->reg.app_id, gattc_if);
        esp_err_t scan_ret = esp_ble_gap_set_scan_params(&ble_scan_params);  // 设置扫描参数
        if (scan_ret){
            ESP_LOGE(GATTC_TAG, "设置扫描参数错误, 错误代码 = %x", scan_ret);
        }
        break;
    case ESP_GATTC_CONNECT_EVT:{  // 连接事件
        if (memcmp(p_data->connect.remote_bda, lock_mac2, 6) != 0) {
            ESP_LOGI(GATTC_TAG, "忽略非本 Profile 的连接事件");
            break; // 不保存 conn_id，也不发送 MTU 请求
             }
        ESP_LOGI(GATTC_TAG, "已连接, 连接ID %d, 远程设备 "ESP_BD_ADDR_STR"", p_data->connect.conn_id,
                 ESP_BD_ADDR_HEX(p_data->connect.remote_bda));
        gl_profile_tab1[PROFILE_B_APP_ID].conn_id = p_data->connect.conn_id;  // 保存连接ID
        ESP_LOGI(TAG, "保存后的PROFILE_B_APP_IDconnid: %d", gl_profile_tab1[PROFILE_B_APP_ID].conn_id);


        memcpy(gl_profile_tab1[PROFILE_B_APP_ID].remote_bda, p_data->connect.remote_bda, sizeof(esp_bd_addr_t));  // 复制远程设备地址
        esp_err_t mtu_ret = esp_ble_gattc_send_mtu_req (gattc_if, p_data->connect.conn_id);  // 发送MTU请求
        if (mtu_ret){
            ESP_LOGE(GATTC_TAG, "配置MTU错误, 错误代码 = %x", mtu_ret);
        }


     
        break;
    }
    case ESP_GATTC_OPEN_EVT:  // 打开连接事件
        if (param->open.status != ESP_GATT_OK){
            //ESP_LOGE(GATTC_TAG, "打开失败, 状态 %d", p_data->open.status);
            break;
        }
        conn_device_b = true;
      //  ESP_LOGI(GATTC_TAG, "成功打开, MTU %u", p_data->open.mtu);
        break;
    case ESP_GATTC_DIS_SRVC_CMPL_EVT:  // 发现服务完成事件
        if (param->dis_srvc_cmpl.status != ESP_GATT_OK){
            ESP_LOGE(GATTC_TAG, "服务发现失败, 状态 %d", param->dis_srvc_cmpl.status);
            break;
        }
        ESP_LOGI(GATTC_TAG, "服务发现完成, 连接ID %d", param->dis_srvc_cmpl.conn_id);
         // 打印要搜索的服务UUID
         ESP_LOGI(GATTC_TAG, "开始搜索服务, UUID:");
         esp_log_buffer_hex(GATTC_TAG, remote_filter_service_uuid.uuid.uuid128, 16);
        esp_ble_gattc_search_service(gattc_if, param->dis_srvc_cmpl.conn_id, NULL);  // 搜索特定UUID的服务
        break;
    case ESP_GATTC_CFG_MTU_EVT:  // MTU配置事件
        ESP_LOGI(GATTC_TAG, "MTU交换, 状态 %d, MTU %d", param->cfg_mtu.status, param->cfg_mtu.mtu);
        break;
        case ESP_GATTC_SEARCH_RES_EVT: {  // 服务搜索结果事件
            ESP_LOGI(GATTC_TAG, "服务搜索结果, 连接ID = %x", p_data->search_res.conn_id);
            ESP_LOGI(GATTC_TAG, "起始句柄 %d, 结束句柄 %d", 
                     p_data->search_res.start_handle, p_data->search_res.end_handle);
        
            if (p_data->search_res.srvc_id.uuid.len == ESP_UUID_LEN_128) {
                
                
                // BLE UUID是小端序，需要反转比较
                uint8_t reversed_uuid[16];
                for (int i = 0; i < 16; i++) {
                    reversed_uuid[i] = p_data->search_res.srvc_id.uuid.uuid.uuid128[15 - i];
                }
                
                ESP_LOGI(GATTC_TAG, "反转后服务UUID128:");
                esp_log_buffer_hex(GATTC_TAG, reversed_uuid, 16);
                
                // 比较UUID（注意BLE是小端序）
                if (memcmp(reversed_uuid, remote_filter_service_uuid.uuid.uuid128, 16) == 0) {
                    ESP_LOGI(GATTC_TAG, "*** 匹配到目标服务 ***");
                    get_serverb = true;
                    gl_profile_tab1[PROFILE_B_APP_ID].service_start_handle = p_data->search_res.start_handle;
                    gl_profile_tab1[PROFILE_B_APP_ID].service_end_handle = p_data->search_res.end_handle;
                    ESP_LOGI(GATTC_TAG, "服务句柄范围: %d - %d", 
                             gl_profile_tab1[PROFILE_B_APP_ID].service_start_handle,
                             gl_profile_tab1[PROFILE_B_APP_ID].service_end_handle);
                } else {
                    ESP_LOGI(GATTC_TAG, "UUID不匹配");
                }
            }
            break;
        }
        case ESP_GATTC_SEARCH_CMPL_EVT:  // 服务搜索完成事件
        ESP_LOGI(GATTC_TAG, "服务搜索完成, 状态 %d", p_data->search_cmpl.status);
        ESP_LOGI(GATTC_TAG, "get_server标志: %d", get_serverb);
        
        if (p_data->search_cmpl.status != ESP_GATT_OK){
            ESP_LOGE(GATTC_TAG, "服务搜索失败, 状态 %x", p_data->search_cmpl.status);
            break;
        }
        
        if (get_serverb){  // 如果找到目标服务
            uint16_t count = 0;
            esp_gatt_status_t status = esp_ble_gattc_get_attr_count( 
                gattc_if,
                p_data->search_cmpl.conn_id,
                ESP_GATT_DB_CHARACTERISTIC,
                gl_profile_tab1[PROFILE_B_APP_ID].service_start_handle,
                gl_profile_tab1[PROFILE_B_APP_ID].service_end_handle,
                INVALID_HANDLE,
                &count);
                
            if (status != ESP_GATT_OK){
                ESP_LOGE(GATTC_TAG, "获取特征数量错误, 状态: %d", status);
                break;
            }
            
            ESP_LOGI(GATTC_TAG, "找到 %d 个特征", count);
            
            if (count > 0){
                // 1. 首先获取并显示所有特征
                char_elem_result_b = (esp_gattc_char_elem_t *)malloc(sizeof(esp_gattc_char_elem_t) * count);
                if (!char_elem_result_b){
                    ESP_LOGE(GATTC_TAG, "内存不足, 无法分配特征元素数组");
                    break;
                }
                
                // 获取所有特征
                status = esp_ble_gattc_get_all_char(
                    gattc_if,
                    p_data->search_cmpl.conn_id,
                    gl_profile_tab1[PROFILE_B_APP_ID].service_start_handle,
                    gl_profile_tab1[PROFILE_B_APP_ID].service_end_handle,
                    char_elem_result_b,
                    &count,
                    0); // 偏移量0
                    
                if (status == ESP_GATT_OK && count > 0) {
                    ESP_LOGI(GATTC_TAG, "========== 服务中的所有特征 ==========");
                    for (int i = 0; i < count; i++) {
                        ESP_LOGI(GATTC_TAG, "特征[%d]: 句柄=%d, 属性=0x%02x", 
                                 i, char_elem_result_b[i].char_handle, char_elem_result_b[i].properties);
                        
                        // 打印特征属性描述
                        if (char_elem_result_b[i].properties & ESP_GATT_CHAR_PROP_BIT_READ) {
                            ESP_LOGI(GATTC_TAG, "  - 支持读取");
                        }
                        if (char_elem_result_b[i].properties & ESP_GATT_CHAR_PROP_BIT_WRITE) {
                            ESP_LOGI(GATTC_TAG, "  - 支持写入");
                        }
                        if (char_elem_result_b[i].properties & ESP_GATT_CHAR_PROP_BIT_WRITE_NR) {
                            ESP_LOGI(GATTC_TAG, "  - 支持无响应写入");
                        }
                        if (char_elem_result_b[i].properties & ESP_GATT_CHAR_PROP_BIT_NOTIFY) {
                            ESP_LOGI(GATTC_TAG, "  - 支持通知");
                        }
                        if (char_elem_result_b[i].properties & ESP_GATT_CHAR_PROP_BIT_INDICATE) {
                            ESP_LOGI(GATTC_TAG, "  - 支持指示");
                        }
                        
                        // 打印特征UUID
                        if (char_elem_result_b[i].uuid.len == ESP_UUID_LEN_128) {
                            ESP_LOGI(GATTC_TAG, "  - UUID128:");
                            esp_log_buffer_hex(GATTC_TAG, char_elem_result_b[i].uuid.uuid.uuid128, 16);
                            
                            // 反转UUID（BLE是小端序）
                            uint8_t reversed_uuid[16];
                            for (int j = 0; j < 16; j++) {
                                reversed_uuid[j] = char_elem_result_b[i].uuid.uuid.uuid128[15 - j];
                            }
                            ESP_LOGI(GATTC_TAG, "  - 反转后UUID128:");
                            esp_log_buffer_hex(GATTC_TAG, reversed_uuid, 16);
                        }
                        ESP_LOGI(GATTC_TAG, "----------------------------------------");
                    }
                    ESP_LOGI(GATTC_TAG, "========== 特征列表结束 ==========");
                    
                    // 2. 直接遍历特征数组，查找需要的特征
                    uint16_t write_char_handle = 0;
                    uint16_t notify_char_handle = 0;
                    
                    for (int i = 0; i < count; i++) {
                        // 检查特征UUID
                        if (char_elem_result_b[i].uuid.len == ESP_UUID_LEN_128) {
                            // 反转UUID进行比较
                            uint8_t reversed_uuid[16];
                            for (int j = 0; j < 16; j++) {
                                reversed_uuid[j] = char_elem_result_b[i].uuid.uuid.uuid128[j];
                            }
                            
                            // 检查是否是写特征
                            esp_bt_uuid_t target_write_uuid;
                            target_write_uuid.len = ESP_UUID_LEN_128;
                            for (int j = 0; j < 16; j++) {
                                target_write_uuid.uuid.uuid128[j] = write_filter_char_uuid.uuid.uuid128[15 - j];
                            }

                            ESP_LOGI(GATTC_TAG, "  - 检查特征UUID:");
                            esp_log_buffer_hex(GATTC_TAG, target_write_uuid.uuid.uuid128, 16);


                            ESP_LOGI(GATTC_TAG, "  - 检查111111111特征UUID:");
                            esp_log_buffer_hex(GATTC_TAG, reversed_uuid, 16);
                            if (memcmp(reversed_uuid, target_write_uuid.uuid.uuid128, 16) == 0) {
                                write_char_handle = char_elem_result_b[i].char_handle;
                                ESP_LOGI(GATTC_TAG, "找到写特征: 句柄=%d", write_char_handle);
                            }
                            
                            // 检查是否是通知特征
                            esp_bt_uuid_t target_notify_uuid;
                            target_notify_uuid.len = ESP_UUID_LEN_128;
                            for (int j = 0; j < 16; j++) {
                                target_notify_uuid.uuid.uuid128[j] = notify_descr_uuid.uuid.uuid128[15 - j];
                            }
                            
                            if (memcmp(reversed_uuid, target_notify_uuid.uuid.uuid128, 16) == 0) {
                                notify_char_handle = char_elem_result_b[i].char_handle;
                                ESP_LOGI(GATTC_TAG, "找到通知特征: 句柄=%d", notify_char_handle);
                            }
                        }
                    }
                    
                    // 3. 处理找到的特征
                    if (write_char_handle != 0) {
                        gl_profile_tab1[PROFILE_B_APP_ID].write_char_handle = write_char_handle;
                        ESP_LOGI(GATTC_TAG, "使用写特征句柄: %d", write_char_handle);
                        
                        if (notify_char_handle != 0) {
                            ESP_LOGI(GATTC_TAG, "注册通知...");
                            esp_ble_gattc_register_for_notify(
                                gattc_if, 
                                gl_profile_tab1[PROFILE_B_APP_ID].remote_bda, 
                                notify_char_handle);
                        } else {
                            ESP_LOGW(GATTC_TAG, "未找到通知特征");
                        }
                        
                        // 4. 直接发送开锁数据
                       
                    } else {
                        ESP_LOGE(GATTC_TAG, "未找到写特征");
                    }
                } else {
                    ESP_LOGE(GATTC_TAG, "获取所有特征失败, 状态: %d", status);
                }
                
                free(char_elem_result_b);
                char_elem_result_b = NULL;
            } else {
                ESP_LOGE(GATTC_TAG, "服务中没有特征");
            }
        } else {
            ESP_LOGE(GATTC_TAG, "未找到目标服务");
        }
        break;
        case ESP_GATTC_REG_FOR_NOTIFY_EVT: {  // 注册通知事件
            if (p_data->reg_for_notify.status != ESP_GATT_OK){
                ESP_LOGE(GATTC_TAG, "通知注册失败, 状态 %d", p_data->reg_for_notify.status);
            } else {
                ESP_LOGI(GATTC_TAG, " 通知注册成功");
                
           
                
                // 启用通知（写入CCCD）
                uint16_t count = 0;
                uint16_t notify_en = 1;  // 启用通知的值
                
                // 使用标准的CCCD UUID (0x2902)
                esp_bt_uuid_t cccd_uuid = {
                    .len = ESP_UUID_LEN_16,
                    .uuid = {.uuid16 = 0x2902}
                };
        
                esp_gatt_status_t ret_status = esp_ble_gattc_get_attr_count( 
                    gattc_if,
                    gl_profile_tab1[PROFILE_B_APP_ID].conn_id,
                    ESP_GATT_DB_DESCRIPTOR,
                    gl_profile_tab1[PROFILE_B_APP_ID].service_start_handle,
                    gl_profile_tab1[PROFILE_B_APP_ID].service_end_handle,
                    p_data->reg_for_notify.handle,  // 通知特征的句柄
                    &count);
                    
                if (ret_status != ESP_GATT_OK){
                    ESP_LOGE(GATTC_TAG, "获取描述符数量错误, 状态: %d", ret_status);
                    // 如果获取描述符失败，直接发送数据
                    ESP_LOGI(GATTC_TAG, " 无法启用通知，直接发送数据");
                    
                    break;
                }
                
                ESP_LOGI(GATTC_TAG, "找到 %d 个描述符", count);
                
                if (count > 0){
                    descr_elem_result_b = malloc(sizeof(esp_gattc_descr_elem_t) * count);
                    if (!descr_elem_result_b){
                        ESP_LOGE(GATTC_TAG, "内存分配错误");
                        // 内存分配失败，直接发送数据
                        ESP_LOGI(GATTC_TAG, " 内存不足，直接发送数据");
                       
                        break;
                    }
                    
                    // 搜索CCCD描述符
                    ret_status = esp_ble_gattc_get_descr_by_char_handle(
                        gattc_if,
                        gl_profile_tab1[PROFILE_B_APP_ID].conn_id,
                        p_data->reg_for_notify.handle,
                        cccd_uuid,
                        descr_elem_result_b,
                        &count);
                        
                    if (ret_status != ESP_GATT_OK || count == 0){
                        ESP_LOGE(GATTC_TAG, "获取CCCD描述符失败, 状态: %d, 数量: %d", ret_status, count);
                        free(descr_elem_result_b);
                        descr_elem_result_b = NULL;
                        // 如果找不到CCCD，直接发送数据
                        ESP_LOGI(GATTC_TAG, " 未找到CCCD描述符，直接发送数据");
                       
                        break;
                    }
                    
                    ESP_LOGI(GATTC_TAG, "找到CCCD描述符, 句柄: %d", descr_elem_result_b[0].handle);
                    
                    // 写入CCCD以启用通知
                    ret_status = esp_ble_gattc_write_char_descr(
                        gattc_if,
                        gl_profile_tab1[PROFILE_B_APP_ID].conn_id,
                        descr_elem_result_b[0].handle,
                        sizeof(notify_en),
                        (uint8_t *)&notify_en,
                        ESP_GATT_WRITE_TYPE_RSP,
                        ESP_GATT_AUTH_REQ_NONE);
                        
                    if (ret_status != ESP_GATT_OK){
                        ESP_LOGE(GATTC_TAG, "写入CCCD描述符失败, 状态: %d", ret_status);
                        // 写入失败，直接发送数据
                        ESP_LOGI(GATTC_TAG, " 启用通知失败，直接发送数据");
                       
                    } else {
                        ESP_LOGI(GATTC_TAG, " 写入CCCD描述符请求已发送，等待启用完成...");
                        // 数据发送将在ESP_GATTC_WRITE_DESCR_EVT事件中处理
                    }
                    
                    free(descr_elem_result_b);
                    descr_elem_result_b = NULL;
                } else {
                    ESP_LOGW(GATTC_TAG, "未找到描述符，直接发送数据");
                
                }
            }
            break;
        }
    case ESP_GATTC_NOTIFY_EVT:  // 通知事件

    lock_send_tcp_sever(p_data->notify.value,p_data->notify.value_len,2);

        break;
        case ESP_GATTC_WRITE_DESCR_EVT:  // 写入描述符事件
        if (p_data->write.status != ESP_GATT_OK){
            ESP_LOGE(GATTC_TAG, " 描述符写入失败, 状态 %x", p_data->write.status);
            // 即使描述符写入失败，也尝试发送数据
            ESP_LOGI(GATTC_TAG, " 通知启用失败，尝试发送数据...");
           
        } else {
            ESP_LOGI(GATTC_TAG, " 描述符写入成功，通知已启用");
            // 现在可以安全地发送数据
            ESP_LOGI(GATTC_TAG, " 发送开锁数据...");
          
           //发送开锁数据
           send_data_to_lock(gattc_if, 
            gl_profile_tab1[PROFILE_B_APP_ID].conn_id,
            gl_profile_tab1[PROFILE_B_APP_ID].write_char_handle,
            TCP_lock_data, 
            TCP_lock_data_len);
        
        
        }
        break;
    case ESP_GATTC_SRVC_CHG_EVT: {  // 服务变更事件
        esp_bd_addr_t bda;
        memcpy(bda, p_data->srvc_chg.remote_bda, sizeof(esp_bd_addr_t));  // 复制设备地址
        ESP_LOGI(GATTC_TAG, "服务变更来自 "ESP_BD_ADDR_STR"", ESP_BD_ADDR_HEX(bda));
        break;
    }
    case ESP_GATTC_WRITE_CHAR_EVT:  // 写入特征事件
        if (p_data->write.status != ESP_GATT_OK){
         //   ESP_LOGE(GATTC_TAG, "2特征写入失败, 状态 %x", p_data->write.status);
            break;
        }
        ESP_LOGI(GATTC_TAG, "特征写入成功");
        break;
    case ESP_GATTC_DISCONNECT_EVT:  // 断开连接事件
    if (memcmp(p_data->connect.remote_bda+3, lock_mac2, 6) != 0) {
        ESP_LOGI(GATTC_TAG, "忽略非本 Profile 的断开事件");
        break; // 不保存 conn_id，也不发送 MTU 请求
         }
        conn_device_b = false;     // 重置连接标志
        get_serverb = false;  // 重置获取服务器标志
        ESP_LOGI(GATTC_TAG, "已断开连接, 远程设备B "ESP_BD_ADDR_STR", 原因 0x%02x",
                 ESP_BD_ADDR_HEX(p_data->disconnect.remote_bda), p_data->disconnect.reason);
        break;
    default:
        break;
    }
}



static void gattc_profile_c_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{
    esp_ble_gattc_cb_param_t *p_data = (esp_ble_gattc_cb_param_t *)param;  // 参数指针

    switch (event) {
    case ESP_GATTC_REG_EVT:  // GATT客户端注册事件
        ESP_LOGI(GATTC_TAG, "GATT客户端注册, 状态 %d, 应用ID %d, GATT接口 %d", param->reg.status, param->reg.app_id, gattc_if);
        esp_err_t scan_ret = esp_ble_gap_set_scan_params(&ble_scan_params);  // 设置扫描参数
        if (scan_ret){
            ESP_LOGE(GATTC_TAG, "设置扫描参数错误, 错误代码 = %x", scan_ret);
        }
        break;
    case ESP_GATTC_CONNECT_EVT:{  
        if (memcmp(p_data->connect.remote_bda, lock_mac3, 6) != 0) {
            ESP_LOGI(GATTC_TAG, "忽略非本 Profile 的连接事件");
            break; // 不保存 conn_id，也不发送 MTU 请求
             }
        // 连接事件
        ESP_LOGI(GATTC_TAG, "已连接, 连接ID %d, 远程设备 "ESP_BD_ADDR_STR"", p_data->connect.conn_id,
                 ESP_BD_ADDR_HEX(p_data->connect.remote_bda));
        gl_profile_tab1[PROFILE_C_APP_ID].conn_id = p_data->connect.conn_id;  // 保存连接ID
        memcpy(gl_profile_tab1[PROFILE_C_APP_ID].remote_bda, p_data->connect.remote_bda, sizeof(esp_bd_addr_t));  // 复制远程设备地址
        esp_err_t mtu_ret = esp_ble_gattc_send_mtu_req (gattc_if, p_data->connect.conn_id);  // 发送MTU请求
        if (mtu_ret){
            ESP_LOGE(GATTC_TAG, "配置MTU错误, 错误代码 = %x", mtu_ret);
        }


     
        break;
    }
    case ESP_GATTC_OPEN_EVT:  // 打开连接事件
        if (param->open.status != ESP_GATT_OK){
          // ESP_LOGE(GATTC_TAG, "打开失败, 状态 %d", p_data->open.status);
            break;
        }
        conn_device_c = true;
        ESP_LOGI(GATTC_TAG, "成功打开, MTU %u", p_data->open.mtu);
        break;
    case ESP_GATTC_DIS_SRVC_CMPL_EVT:  // 发现服务完成事件
        if (param->dis_srvc_cmpl.status != ESP_GATT_OK){
            ESP_LOGE(GATTC_TAG, "服务发现失败, 状态 %d", param->dis_srvc_cmpl.status);
            break;
        }
        ESP_LOGI(GATTC_TAG, "服务发现完成, 连接ID %d", param->dis_srvc_cmpl.conn_id);
         // 打印要搜索的服务UUID
         ESP_LOGI(GATTC_TAG, "开始搜索服务, UUID:");
         esp_log_buffer_hex(GATTC_TAG, remote_filter_service_uuid.uuid.uuid128, 16);
        esp_ble_gattc_search_service(gattc_if, param->dis_srvc_cmpl.conn_id, NULL);  // 搜索特定UUID的服务
        break;
    case ESP_GATTC_CFG_MTU_EVT:  // MTU配置事件
        ESP_LOGI(GATTC_TAG, "MTU交换, 状态 %d, MTU %d", param->cfg_mtu.status, param->cfg_mtu.mtu);
        break;
        case ESP_GATTC_SEARCH_RES_EVT: {  // 服务搜索结果事件
            ESP_LOGI(GATTC_TAG, "服务搜索结果, 连接ID = %x", p_data->search_res.conn_id);
            ESP_LOGI(GATTC_TAG, "起始句柄 %d, 结束句柄 %d", 
                     p_data->search_res.start_handle, p_data->search_res.end_handle);
        
            if (p_data->search_res.srvc_id.uuid.len == ESP_UUID_LEN_128) {
                ESP_LOGI(GATTC_TAG, "找到128位UUID服务");
                ESP_LOGI(GATTC_TAG, "服务UUID128:");
                esp_log_buffer_hex(GATTC_TAG, p_data->search_res.srvc_id.uuid.uuid.uuid128, 16);
                
                // 打印目标UUID进行对比
                ESP_LOGI(GATTC_TAG, "目标UUID128:");
                esp_log_buffer_hex(GATTC_TAG, remote_filter_service_uuid.uuid.uuid128, 16);
                
                // BLE UUID是小端序，需要反转比较
                uint8_t reversed_uuid[16];
                for (int i = 0; i < 16; i++) {
                    reversed_uuid[i] = p_data->search_res.srvc_id.uuid.uuid.uuid128[15 - i];
                }
                
                ESP_LOGI(GATTC_TAG, "反转后服务UUID128:");
                esp_log_buffer_hex(GATTC_TAG, reversed_uuid, 16);
                
                // 比较UUID（注意BLE是小端序）
                if (memcmp(reversed_uuid, remote_filter_service_uuid.uuid.uuid128, 16) == 0) {
                    ESP_LOGI(GATTC_TAG, "*** 匹配到目标服务 ***");
                    get_serverc = true;
                    gl_profile_tab1[PROFILE_C_APP_ID].service_start_handle = p_data->search_res.start_handle;
                    gl_profile_tab1[PROFILE_C_APP_ID].service_end_handle = p_data->search_res.end_handle;
                    ESP_LOGI(GATTC_TAG, "服务句柄范围: %d - %d", 
                             gl_profile_tab1[PROFILE_C_APP_ID].service_start_handle,
                             gl_profile_tab1[PROFILE_C_APP_ID].service_end_handle);
                } else {
                    ESP_LOGI(GATTC_TAG, "UUID不匹配");
                }
            }
            break;
        }
        case ESP_GATTC_SEARCH_CMPL_EVT:  // 服务搜索完成事件
        ESP_LOGI(GATTC_TAG, "服务搜索完成, 状态 %d", p_data->search_cmpl.status);
        ESP_LOGI(GATTC_TAG, "get_server标志: %d", get_serverc);
        
        if (p_data->search_cmpl.status != ESP_GATT_OK){
            ESP_LOGE(GATTC_TAG, "服务搜索失败, 状态 %x", p_data->search_cmpl.status);
            break;
        }
        
        if (get_serverc){  // 如果找到目标服务
            uint16_t count = 0;
            esp_gatt_status_t status = esp_ble_gattc_get_attr_count( 
                gattc_if,
                p_data->search_cmpl.conn_id,
                ESP_GATT_DB_CHARACTERISTIC,
                gl_profile_tab1[PROFILE_C_APP_ID].service_start_handle,
                gl_profile_tab1[PROFILE_C_APP_ID].service_end_handle,
                INVALID_HANDLE,
                &count);
                
            if (status != ESP_GATT_OK){
                ESP_LOGE(GATTC_TAG, "获取特征数量错误, 状态: %d", status);
                break;
            }
            
            ESP_LOGI(GATTC_TAG, "找到 %d 个特征", count);
            
            if (count > 0){
                // 1. 首先获取并显示所有特征
                char_elem_result_c = (esp_gattc_char_elem_t *)malloc(sizeof(esp_gattc_char_elem_t) * count);
                if (!char_elem_result_c){
                    ESP_LOGE(GATTC_TAG, "内存不足, 无法分配特征元素数组");
                    break;
                }
                
                // 获取所有特征
                status = esp_ble_gattc_get_all_char(
                    gattc_if,
                    p_data->search_cmpl.conn_id,
                    gl_profile_tab1[PROFILE_C_APP_ID].service_start_handle,
                    gl_profile_tab1[PROFILE_C_APP_ID].service_end_handle,
                    char_elem_result_c,
                    &count,
                    0); // 偏移量0
                    
                if (status == ESP_GATT_OK && count > 0) {
                    ESP_LOGI(GATTC_TAG, "========== 服务中的所有特征 ==========");
                    for (int i = 0; i < count; i++) {
                        ESP_LOGI(GATTC_TAG, "特征[%d]: 句柄=%d, 属性=0x%02x", 
                                 i, char_elem_result_c[i].char_handle, char_elem_result_c[i].properties);
                        
                        // 打印特征属性描述
                        if (char_elem_result_c[i].properties & ESP_GATT_CHAR_PROP_BIT_READ) {
                            ESP_LOGI(GATTC_TAG, "  - 支持读取");
                        }
                        if (char_elem_result_c[i].properties & ESP_GATT_CHAR_PROP_BIT_WRITE) {
                            ESP_LOGI(GATTC_TAG, "  - 支持写入");
                        }
                        if (char_elem_result_c[i].properties & ESP_GATT_CHAR_PROP_BIT_WRITE_NR) {
                            ESP_LOGI(GATTC_TAG, "  - 支持无响应写入");
                        }
                        if (char_elem_result_c[i].properties & ESP_GATT_CHAR_PROP_BIT_NOTIFY) {
                            ESP_LOGI(GATTC_TAG, "  - 支持通知");
                        }
                        if (char_elem_result_c[i].properties & ESP_GATT_CHAR_PROP_BIT_INDICATE) {
                            ESP_LOGI(GATTC_TAG, "  - 支持指示");
                        }
                        
                        // 打印特征UUID
                        if (char_elem_result_c[i].uuid.len == ESP_UUID_LEN_128) {
                            ESP_LOGI(GATTC_TAG, "  - UUID128:");
                            esp_log_buffer_hex(GATTC_TAG, char_elem_result_c[i].uuid.uuid.uuid128, 16);
                            
                            // 反转UUID（BLE是小端序）
                            uint8_t reversed_uuid[16];
                            for (int j = 0; j < 16; j++) {
                                reversed_uuid[j] = char_elem_result_c[i].uuid.uuid.uuid128[15 - j];
                            }
                            ESP_LOGI(GATTC_TAG, "  - 反转后UUID128:");
                            esp_log_buffer_hex(GATTC_TAG, reversed_uuid, 16);
                        }
                        ESP_LOGI(GATTC_TAG, "----------------------------------------");
                    }
                    ESP_LOGI(GATTC_TAG, "========== 特征列表结束 ==========");
                    
                    // 2. 直接遍历特征数组，查找需要的特征
                    uint16_t write_char_handle = 0;
                    uint16_t notify_char_handle = 0;
                    
                    for (int i = 0; i < count; i++) {
                        // 检查特征UUID
                        if (char_elem_result_c[i].uuid.len == ESP_UUID_LEN_128) {
                            // 反转UUID进行比较
                            uint8_t reversed_uuid[16];
                            for (int j = 0; j < 16; j++) {
                                reversed_uuid[j] = char_elem_result_c[i].uuid.uuid.uuid128[j];
                            }
                            
                            // 检查是否是写特征
                            esp_bt_uuid_t target_write_uuid;
                            target_write_uuid.len = ESP_UUID_LEN_128;
                            for (int j = 0; j < 16; j++) {
                                target_write_uuid.uuid.uuid128[j] = write_filter_char_uuid.uuid.uuid128[15 - j];
                            }

                            ESP_LOGI(GATTC_TAG, "  - 检查特征UUID:");
                            esp_log_buffer_hex(GATTC_TAG, target_write_uuid.uuid.uuid128, 16);


                            ESP_LOGI(GATTC_TAG, "  - 检查111111111特征UUID:");
                            esp_log_buffer_hex(GATTC_TAG, reversed_uuid, 16);
                            if (memcmp(reversed_uuid, target_write_uuid.uuid.uuid128, 16) == 0) {
                                write_char_handle = char_elem_result_c[i].char_handle;
                                ESP_LOGI(GATTC_TAG, "找到写特征: 句柄=%d", write_char_handle);
                            }
                            
                            // 检查是否是通知特征
                            esp_bt_uuid_t target_notify_uuid;
                            target_notify_uuid.len = ESP_UUID_LEN_128;
                            for (int j = 0; j < 16; j++) {
                                target_notify_uuid.uuid.uuid128[j] = notify_descr_uuid.uuid.uuid128[15 - j];
                            }
                            
                            if (memcmp(reversed_uuid, target_notify_uuid.uuid.uuid128, 16) == 0) {
                                notify_char_handle = char_elem_result_c[i].char_handle;
                                ESP_LOGI(GATTC_TAG, "找到通知特征: 句柄=%d", notify_char_handle);
                            }
                        }
                    }
                    
                    // 3. 处理找到的特征
                    if (write_char_handle != 0) {
                        gl_profile_tab1[PROFILE_C_APP_ID].write_char_handle = write_char_handle;
                        ESP_LOGI(GATTC_TAG, "使用写特征句柄: %d", write_char_handle);
                        
                        if (notify_char_handle != 0) {
                            ESP_LOGI(GATTC_TAG, "注册通知...");
                            esp_ble_gattc_register_for_notify(
                                gattc_if, 
                                gl_profile_tab1[PROFILE_C_APP_ID].remote_bda, 
                                notify_char_handle);
                        } else {
                            ESP_LOGW(GATTC_TAG, "未找到通知特征");
                        }
                        
                        
                       
                    } else {
                        ESP_LOGE(GATTC_TAG, "未找到写特征");
                    }
                } else {
                    ESP_LOGE(GATTC_TAG, "获取所有特征失败, 状态: %d", status);
                }
                
                free(char_elem_result_c);
                char_elem_result_c = NULL;
            } else {
                ESP_LOGE(GATTC_TAG, "服务中没有特征");
            }
        } else {
            ESP_LOGE(GATTC_TAG, "未找到目标服务");
        }
        break;
        case ESP_GATTC_REG_FOR_NOTIFY_EVT: {  // 注册通知事件
            if (p_data->reg_for_notify.status != ESP_GATT_OK){
                ESP_LOGE(GATTC_TAG, "通知注册失败, 状态 %d", p_data->reg_for_notify.status);
            } else {
                ESP_LOGI(GATTC_TAG, " 通知注册成功");
                
           
                
                // 启用通知（写入CCCD）
                uint16_t count = 0;
                uint16_t notify_en = 1;  // 启用通知的值
                
                // 使用标准的CCCD UUID (0x2902)
                esp_bt_uuid_t cccd_uuid = {
                    .len = ESP_UUID_LEN_16,
                    .uuid = {.uuid16 = 0x2902}
                };
        
                esp_gatt_status_t ret_status = esp_ble_gattc_get_attr_count( 
                    gattc_if,
                    gl_profile_tab1[PROFILE_C_APP_ID].conn_id,
                    ESP_GATT_DB_DESCRIPTOR,
                    gl_profile_tab1[PROFILE_C_APP_ID].service_start_handle,
                    gl_profile_tab1[PROFILE_C_APP_ID].service_end_handle,
                    p_data->reg_for_notify.handle,  // 通知特征的句柄
                    &count);
                    
                if (ret_status != ESP_GATT_OK){
                    ESP_LOGE(GATTC_TAG, "获取描述符数量错误, 状态: %d", ret_status);
                    // 如果获取描述符失败，直接发送数据
                    ESP_LOGI(GATTC_TAG, " 无法启用通知，直接发送数据");
                    
                    break;
                }
                
                ESP_LOGI(GATTC_TAG, "找到 %d 个描述符", count);
                
                if (count > 0){
                    descr_elem_result_c = malloc(sizeof(esp_gattc_descr_elem_t) * count);
                    if (!descr_elem_result_c){
                        ESP_LOGE(GATTC_TAG, "内存分配错误");
                        // 内存分配失败，直接发送数据
                        ESP_LOGI(GATTC_TAG, " 内存不足，直接发送数据");
                       
                        break;
                    }
                    
                    // 搜索CCCD描述符
                    ret_status = esp_ble_gattc_get_descr_by_char_handle(
                        gattc_if,
                        gl_profile_tab1[PROFILE_C_APP_ID].conn_id,
                        p_data->reg_for_notify.handle,
                        cccd_uuid,
                        descr_elem_result_c,
                        &count);
                        
                    if (ret_status != ESP_GATT_OK || count == 0){
                        ESP_LOGE(GATTC_TAG, "获取CCCD描述符失败, 状态: %d, 数量: %d", ret_status, count);
                        free(descr_elem_result_c);
                        descr_elem_result_c = NULL;
                        // 如果找不到CCCD，直接发送数据
                        ESP_LOGI(GATTC_TAG, " 未找到CCCD描述符，直接发送数据");
                       
                        break;
                    }
                    
                    ESP_LOGI(GATTC_TAG, "找到CCCD描述符, 句柄: %d", descr_elem_result_c[0].handle);
                    
                    // 写入CCCD以启用通知
                    ret_status = esp_ble_gattc_write_char_descr(
                        gattc_if,
                        gl_profile_tab1[PROFILE_C_APP_ID].conn_id,
                        descr_elem_result_c[0].handle,
                        sizeof(notify_en),
                        (uint8_t *)&notify_en,
                        ESP_GATT_WRITE_TYPE_RSP,
                        ESP_GATT_AUTH_REQ_NONE);
                        
                    if (ret_status != ESP_GATT_OK){
                        ESP_LOGE(GATTC_TAG, "写入CCCD描述符失败, 状态: %d", ret_status);
                        // 写入失败，直接发送数据
                        ESP_LOGI(GATTC_TAG, " 启用通知失败，直接发送数据");
                       
                    } else {
                        ESP_LOGI(GATTC_TAG, " 写入CCCD描述符请求已发送，等待启用完成...");
                        // 数据发送将在ESP_GATTC_WRITE_DESCR_EVT事件中处理
                    }
                    
                    free(descr_elem_result_c);
                    descr_elem_result_c = NULL;
                } else {
                    ESP_LOGW(GATTC_TAG, "未找到描述符，直接发送数据");
                
                }
            }
            break;
        }
    case ESP_GATTC_NOTIFY_EVT:  // 通知事件

    lock_send_tcp_sever(p_data->notify.value,p_data->notify.value_len,3);

        break;
        case ESP_GATTC_WRITE_DESCR_EVT:  // 写入描述符事件
        if (p_data->write.status != ESP_GATT_OK){
            ESP_LOGE(GATTC_TAG, " 描述符写入失败, 状态 %x", p_data->write.status);
            // 即使描述符写入失败，也尝试发送数据
            ESP_LOGI(GATTC_TAG, " 通知启用失败，尝试发送数据...");
           
        } else {
            ESP_LOGI(GATTC_TAG, " 描述符写入成功，通知已启用");
            // 现在可以安全地发送数据
            ESP_LOGI(GATTC_TAG, " 发送开锁数据...");
           //发送开锁数据
           send_data_to_lock(gattc_if, 
            gl_profile_tab1[PROFILE_C_APP_ID].conn_id,
            gl_profile_tab1[PROFILE_C_APP_ID].write_char_handle,
            TCP_lock_data, 
            TCP_lock_data_len);
        
        
        }
        break;
    case ESP_GATTC_SRVC_CHG_EVT: {  // 服务变更事件
        esp_bd_addr_t bda;
        memcpy(bda, p_data->srvc_chg.remote_bda, sizeof(esp_bd_addr_t));  // 复制设备地址
        ESP_LOGI(GATTC_TAG, "服务变更来自 "ESP_BD_ADDR_STR"", ESP_BD_ADDR_HEX(bda));
        break;
    }
    case ESP_GATTC_WRITE_CHAR_EVT:  // 写入特征事件
        if (p_data->write.status != ESP_GATT_OK){
          //  ESP_LOGE(GATTC_TAG, "3特征写入失败, 状态 %x", p_data->write.status);
            break;
        }
        ESP_LOGI(GATTC_TAG, "特征写入成功");
        break;
    case ESP_GATTC_DISCONNECT_EVT:  // 断开连接事件
    ESP_LOG_BUFFER_HEX("设备三断开时间",p_data->connect.remote_bda+3,6);
    if (memcmp(p_data->connect.remote_bda+3, lock_mac3, 6) != 0) {
        ESP_LOGI(GATTC_TAG, "忽略非本 Profile 的断开事件");
        break; // 不保存 conn_id，也不发送 MTU 请求
         }
        conn_device_c = false;     // 重置连接标志
        get_serverc = false;  // 重置获取服务器标志
        ESP_LOGI(GATTC_TAG, "已断开连接, 远程设备C "ESP_BD_ADDR_STR", 原因 0x%02x",
                 ESP_BD_ADDR_HEX(p_data->disconnect.remote_bda), p_data->disconnect.reason);
        break;
    default:
        break;
    }
}


static void gattc_profile_d_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{
    esp_ble_gattc_cb_param_t *p_data = (esp_ble_gattc_cb_param_t *)param;  // 参数指针

    switch (event) {
    case ESP_GATTC_REG_EVT:  // GATT客户端注册事件
        ESP_LOGI(GATTC_TAG, "GATT客户端注册, 状态 %d, 应用ID %d, GATT接口 %d", param->reg.status, param->reg.app_id, gattc_if);
        esp_err_t scan_ret = esp_ble_gap_set_scan_params(&ble_scan_params);  // 设置扫描参数
        if (scan_ret){
            ESP_LOGE(GATTC_TAG, "设置扫描参数错误, 错误代码 = %x", scan_ret);
        }
        break;
    case ESP_GATTC_CONNECT_EVT:{  // 连接事件
        if (memcmp(p_data->connect.remote_bda, lock_mac4, 6) != 0) {
            ESP_LOGI(GATTC_TAG, "忽略非本 Profile 的连接事件");
            break; // 不保存 conn_id，也不发送 MTU 请求
             }
        ESP_LOGI(GATTC_TAG, "已连接, 连接ID %d, 远程设备 "ESP_BD_ADDR_STR"", p_data->connect.conn_id,
                 ESP_BD_ADDR_HEX(p_data->connect.remote_bda));
        gl_profile_tab1[PROFILE_D_APP_ID].conn_id = p_data->connect.conn_id;  // 保存连接ID
        memcpy(gl_profile_tab1[PROFILE_D_APP_ID].remote_bda, p_data->connect.remote_bda, sizeof(esp_bd_addr_t));  // 复制远程设备地址
        esp_err_t mtu_ret = esp_ble_gattc_send_mtu_req (gattc_if, p_data->connect.conn_id);  // 发送MTU请求
        if (mtu_ret){
            ESP_LOGE(GATTC_TAG, "配置MTU错误, 错误代码 = %x", mtu_ret);
        }


     
        break;
    }
    case ESP_GATTC_OPEN_EVT:  // 打开连接事件
        if (param->open.status != ESP_GATT_OK){
          //  ESP_LOGE(GATTC_TAG, "打开失败, 状态 %d", p_data->open.status);
            break;
        }
        conn_device_d = true;
        ESP_LOGI(GATTC_TAG, "成功打开, MTU %u", p_data->open.mtu);
        break;
    case ESP_GATTC_DIS_SRVC_CMPL_EVT:  // 发现服务完成事件
        if (param->dis_srvc_cmpl.status != ESP_GATT_OK){
            ESP_LOGE(GATTC_TAG, "服务发现失败, 状态 %d", param->dis_srvc_cmpl.status);
            break;
        }
        ESP_LOGI(GATTC_TAG, "服务发现完成, 连接ID %d", param->dis_srvc_cmpl.conn_id);
         // 打印要搜索的服务UUID
         ESP_LOGI(GATTC_TAG, "开始搜索服务, UUID:");
         esp_log_buffer_hex(GATTC_TAG, remote_filter_service_uuid.uuid.uuid128, 16);
        esp_ble_gattc_search_service(gattc_if, param->dis_srvc_cmpl.conn_id, NULL);  // 搜索特定UUID的服务
        break;
    case ESP_GATTC_CFG_MTU_EVT:  // MTU配置事件
        ESP_LOGI(GATTC_TAG, "MTU交换, 状态 %d, MTU %d", param->cfg_mtu.status, param->cfg_mtu.mtu);
        break;
        case ESP_GATTC_SEARCH_RES_EVT: {  // 服务搜索结果事件
            ESP_LOGI(GATTC_TAG, "服务搜索结果, 连接ID = %x", p_data->search_res.conn_id);
            ESP_LOGI(GATTC_TAG, "起始句柄 %d, 结束句柄 %d", 
                     p_data->search_res.start_handle, p_data->search_res.end_handle);
        
            if (p_data->search_res.srvc_id.uuid.len == ESP_UUID_LEN_128) {
                ESP_LOGI(GATTC_TAG, "找到128位UUID服务");
                ESP_LOGI(GATTC_TAG, "服务UUID128:");
                esp_log_buffer_hex(GATTC_TAG, p_data->search_res.srvc_id.uuid.uuid.uuid128, 16);
                
                // 打印目标UUID进行对比
                ESP_LOGI(GATTC_TAG, "目标UUID128:");
                esp_log_buffer_hex(GATTC_TAG, remote_filter_service_uuid.uuid.uuid128, 16);
                
                // BLE UUID是小端序，需要反转比较
                uint8_t reversed_uuid[16];
                for (int i = 0; i < 16; i++) {
                    reversed_uuid[i] = p_data->search_res.srvc_id.uuid.uuid.uuid128[15 - i];
                }
                
                ESP_LOGI(GATTC_TAG, "反转后服务UUID128:");
                esp_log_buffer_hex(GATTC_TAG, reversed_uuid, 16);
                
                // 比较UUID（注意BLE是小端序）
                if (memcmp(reversed_uuid, remote_filter_service_uuid.uuid.uuid128, 16) == 0) {
                    ESP_LOGI(GATTC_TAG, "*** 匹配到目标服务 ***");
                    get_serverd = true;
                    gl_profile_tab1[PROFILE_D_APP_ID].service_start_handle = p_data->search_res.start_handle;
                    gl_profile_tab1[PROFILE_D_APP_ID].service_end_handle = p_data->search_res.end_handle;
                    ESP_LOGI(GATTC_TAG, "服务句柄范围: %d - %d", 
                             gl_profile_tab1[PROFILE_D_APP_ID].service_start_handle,
                             gl_profile_tab1[PROFILE_D_APP_ID].service_end_handle);
                } else {
                    ESP_LOGI(GATTC_TAG, "UUID不匹配");
                }
            }
            break;
        }
        case ESP_GATTC_SEARCH_CMPL_EVT:  // 服务搜索完成事件
        ESP_LOGI(GATTC_TAG, "服务搜索完成, 状态 %d", p_data->search_cmpl.status);
        ESP_LOGI(GATTC_TAG, "get_server标志: %d", get_serverd);
        
        if (p_data->search_cmpl.status != ESP_GATT_OK){
            ESP_LOGE(GATTC_TAG, "服务搜索失败, 状态 %x", p_data->search_cmpl.status);
            break;
        }
        
        if (get_serverd){  // 如果找到目标服务
            uint16_t count = 0;
            esp_gatt_status_t status = esp_ble_gattc_get_attr_count( 
                gattc_if,
                p_data->search_cmpl.conn_id,
                ESP_GATT_DB_CHARACTERISTIC,
                gl_profile_tab1[PROFILE_D_APP_ID].service_start_handle,
                gl_profile_tab1[PROFILE_D_APP_ID].service_end_handle,
                INVALID_HANDLE,
                &count);
                
            if (status != ESP_GATT_OK){
                ESP_LOGE(GATTC_TAG, "获取特征数量错误, 状态: %d", status);
                break;
            }
            
            ESP_LOGI(GATTC_TAG, "找到 %d 个特征", count);
            
            if (count > 0){
                // 1. 首先获取并显示所有特征
                char_elem_result_d = (esp_gattc_char_elem_t *)malloc(sizeof(esp_gattc_char_elem_t) * count);
                if (!char_elem_result_d){
                    ESP_LOGE(GATTC_TAG, "内存不足, 无法分配特征元素数组");
                    break;
                }
                
                // 获取所有特征
                status = esp_ble_gattc_get_all_char(
                    gattc_if,
                    p_data->search_cmpl.conn_id,
                    gl_profile_tab1[PROFILE_D_APP_ID].service_start_handle,
                    gl_profile_tab1[PROFILE_D_APP_ID].service_end_handle,
                    char_elem_result_d,
                    &count,
                    0); // 偏移量0
                    
                if (status == ESP_GATT_OK && count > 0) {
                    ESP_LOGI(GATTC_TAG, "========== 服务中的所有特征 ==========");
                    for (int i = 0; i < count; i++) {
                        ESP_LOGI(GATTC_TAG, "特征[%d]: 句柄=%d, 属性=0x%02x", 
                                 i, char_elem_result_d[i].char_handle, char_elem_result_d[i].properties);
                        
                        // 打印特征属性描述
                        if (char_elem_result_d[i].properties & ESP_GATT_CHAR_PROP_BIT_READ) {
                            ESP_LOGI(GATTC_TAG, "  - 支持读取");
                        }
                        if (char_elem_result_d[i].properties & ESP_GATT_CHAR_PROP_BIT_WRITE) {
                            ESP_LOGI(GATTC_TAG, "  - 支持写入");
                        }
                        if (char_elem_result_d[i].properties & ESP_GATT_CHAR_PROP_BIT_WRITE_NR) {
                            ESP_LOGI(GATTC_TAG, "  - 支持无响应写入");
                        }
                        if (char_elem_result_d[i].properties & ESP_GATT_CHAR_PROP_BIT_NOTIFY) {
                            ESP_LOGI(GATTC_TAG, "  - 支持通知");
                        }
                        if (char_elem_result_d[i].properties & ESP_GATT_CHAR_PROP_BIT_INDICATE) {
                            ESP_LOGI(GATTC_TAG, "  - 支持指示");
                        }
                        
                        // 打印特征UUID
                        if (char_elem_result_d[i].uuid.len == ESP_UUID_LEN_128) {
                            ESP_LOGI(GATTC_TAG, "  - UUID128:");
                            esp_log_buffer_hex(GATTC_TAG, char_elem_result_d[i].uuid.uuid.uuid128, 16);
                            
                            // 反转UUID（BLE是小端序）
                            uint8_t reversed_uuid[16];
                            for (int j = 0; j < 16; j++) {
                                reversed_uuid[j] = char_elem_result_d[i].uuid.uuid.uuid128[15 - j];
                            }
                            ESP_LOGI(GATTC_TAG, "  - 反转后UUID128:");
                            esp_log_buffer_hex(GATTC_TAG, reversed_uuid, 16);
                        }
                        ESP_LOGI(GATTC_TAG, "----------------------------------------");
                    }
                    ESP_LOGI(GATTC_TAG, "========== 特征列表结束 ==========");
                    
                    // 2. 直接遍历特征数组，查找需要的特征
                    uint16_t write_char_handle = 0;
                    uint16_t notify_char_handle = 0;
                    
                    for (int i = 0; i < count; i++) {
                        // 检查特征UUID
                        if (char_elem_result_d[i].uuid.len == ESP_UUID_LEN_128) {
                            // 反转UUID进行比较
                            uint8_t reversed_uuid[16];
                            for (int j = 0; j < 16; j++) {
                                reversed_uuid[j] = char_elem_result_d[i].uuid.uuid.uuid128[j];
                            }
                            
                            // 检查是否是写特征
                            esp_bt_uuid_t target_write_uuid;
                            target_write_uuid.len = ESP_UUID_LEN_128;
                            for (int j = 0; j < 16; j++) {
                                target_write_uuid.uuid.uuid128[j] = write_filter_char_uuid.uuid.uuid128[15 - j];
                            }

                            ESP_LOGI(GATTC_TAG, "  - 检查特征UUID:");
                            esp_log_buffer_hex(GATTC_TAG, target_write_uuid.uuid.uuid128, 16);


                            ESP_LOGI(GATTC_TAG, "  - 检查111111111特征UUID:");
                            esp_log_buffer_hex(GATTC_TAG, reversed_uuid, 16);
                            if (memcmp(reversed_uuid, target_write_uuid.uuid.uuid128, 16) == 0) {
                                write_char_handle = char_elem_result_d[i].char_handle;
                                ESP_LOGI(GATTC_TAG, "找到写特征: 句柄=%d", write_char_handle);
                            }
                            
                            // 检查是否是通知特征
                            esp_bt_uuid_t target_notify_uuid;
                            target_notify_uuid.len = ESP_UUID_LEN_128;
                            for (int j = 0; j < 16; j++) {
                                target_notify_uuid.uuid.uuid128[j] = notify_descr_uuid.uuid.uuid128[15 - j];
                            }
                            
                            if (memcmp(reversed_uuid, target_notify_uuid.uuid.uuid128, 16) == 0) {
                                notify_char_handle = char_elem_result_d[i].char_handle;
                                ESP_LOGI(GATTC_TAG, "找到通知特征: 句柄=%d", notify_char_handle);
                            }
                        }
                    }
                    
                    // 3. 处理找到的特征
                    if (write_char_handle != 0) {
                        gl_profile_tab1[PROFILE_D_APP_ID].write_char_handle = write_char_handle;
                        ESP_LOGI(GATTC_TAG, "使用写特征句柄: %d", write_char_handle);
                        
                        if (notify_char_handle != 0) {
                            ESP_LOGI(GATTC_TAG, "注册通知...");
                            esp_ble_gattc_register_for_notify(
                                gattc_if, 
                                gl_profile_tab1[PROFILE_D_APP_ID].remote_bda, 
                                notify_char_handle);
                        } else {
                            ESP_LOGW(GATTC_TAG, "未找到通知特征");
                        }
                        
                        // 4. 直接发送开锁数据
                       
                    } else {
                        ESP_LOGE(GATTC_TAG, "未找到写特征");
                    }
                } else {
                    ESP_LOGE(GATTC_TAG, "获取所有特征失败, 状态: %d", status);
                }
                
                free(char_elem_result_d);
                char_elem_result_d = NULL;
            } else {
                ESP_LOGE(GATTC_TAG, "服务中没有特征");
            }
        } else {
            ESP_LOGE(GATTC_TAG, "未找到目标服务");
        }
        break;
        case ESP_GATTC_REG_FOR_NOTIFY_EVT: {  // 注册通知事件
            if (p_data->reg_for_notify.status != ESP_GATT_OK){
                ESP_LOGE(GATTC_TAG, "通知注册失败, 状态 %d", p_data->reg_for_notify.status);
            } else {
                ESP_LOGI(GATTC_TAG, " 通知注册成功");
                
           
                
                // 启用通知（写入CCCD）
                uint16_t count = 0;
                uint16_t notify_en = 1;  // 启用通知的值
                
                // 使用标准的CCCD UUID (0x2902)
                esp_bt_uuid_t cccd_uuid = {
                    .len = ESP_UUID_LEN_16,
                    .uuid = {.uuid16 = 0x2902}
                };
        
                esp_gatt_status_t ret_status = esp_ble_gattc_get_attr_count( 
                    gattc_if,
                    gl_profile_tab1[PROFILE_D_APP_ID].conn_id,
                    ESP_GATT_DB_DESCRIPTOR,
                    gl_profile_tab1[PROFILE_D_APP_ID].service_start_handle,
                    gl_profile_tab1[PROFILE_D_APP_ID].service_end_handle,
                    p_data->reg_for_notify.handle,  // 通知特征的句柄
                    &count);
                    
                if (ret_status != ESP_GATT_OK){
                    ESP_LOGE(GATTC_TAG, "获取描述符数量错误, 状态: %d", ret_status);
                    // 如果获取描述符失败，直接发送数据
                    ESP_LOGI(GATTC_TAG, " 无法启用通知，直接发送数据");
                    
                    break;
                }
                
                ESP_LOGI(GATTC_TAG, "找到 %d 个描述符", count);
                
                if (count > 0){
                    descr_elem_result_d = malloc(sizeof(esp_gattc_descr_elem_t) * count);
                    if (!descr_elem_result_d){
                        ESP_LOGE(GATTC_TAG, "内存分配错误");
                        // 内存分配失败，直接发送数据
                        ESP_LOGI(GATTC_TAG, " 内存不足，直接发送数据");
                       
                        break;
                    }
                    
                    // 搜索CCCD描述符
                    ret_status = esp_ble_gattc_get_descr_by_char_handle(
                        gattc_if,
                        gl_profile_tab1[PROFILE_D_APP_ID].conn_id,
                        p_data->reg_for_notify.handle,
                        cccd_uuid,
                        descr_elem_result_d,
                        &count);
                        
                    if (ret_status != ESP_GATT_OK || count == 0){
                        ESP_LOGE(GATTC_TAG, "获取CCCD描述符失败, 状态: %d, 数量: %d", ret_status, count);
                        free(descr_elem_result_d);
                        descr_elem_result_d = NULL;
                        // 如果找不到CCCD，直接发送数据
                        ESP_LOGI(GATTC_TAG, " 未找到CCCD描述符，直接发送数据");
                       
                        break;
                    }
                    
                    ESP_LOGI(GATTC_TAG, "找到CCCD描述符, 句柄: %d", descr_elem_result_d[0].handle);
                    
                    // 写入CCCD以启用通知
                    ret_status = esp_ble_gattc_write_char_descr(
                        gattc_if,
                        gl_profile_tab1[PROFILE_D_APP_ID].conn_id,
                        descr_elem_result_d[0].handle,
                        sizeof(notify_en),
                        (uint8_t *)&notify_en,
                        ESP_GATT_WRITE_TYPE_RSP,
                        ESP_GATT_AUTH_REQ_NONE);
                        
                    if (ret_status != ESP_GATT_OK){
                        ESP_LOGE(GATTC_TAG, "写入CCCD描述符失败, 状态: %d", ret_status);
                        // 写入失败，直接发送数据
                        ESP_LOGI(GATTC_TAG, " 启用通知失败，直接发送数据");
                       
                    } else {
                        ESP_LOGI(GATTC_TAG, " 写入CCCD描述符请求已发送，等待启用完成...");
                        // 数据发送将在ESP_GATTC_WRITE_DESCR_EVT事件中处理
                    }
                    
                    free(descr_elem_result_d);
                    descr_elem_result_d = NULL;
                } else {
                    ESP_LOGW(GATTC_TAG, "未找到描述符，直接发送数据");
                
                }
            }
            break;
        }
    case ESP_GATTC_NOTIFY_EVT:  // 通知事件

    lock_send_tcp_sever(p_data->notify.value,p_data->notify.value_len,4);

        break;
        case ESP_GATTC_WRITE_DESCR_EVT:  // 写入描述符事件
        if (p_data->write.status != ESP_GATT_OK){
            ESP_LOGE(GATTC_TAG, " 描述符写入失败, 状态 %x", p_data->write.status);
            // 即使描述符写入失败，也尝试发送数据
            ESP_LOGI(GATTC_TAG, " 通知启用失败，尝试发送数据...");
           
        } else {
            ESP_LOGI(GATTC_TAG, " 描述符写入成功，通知已启用");
            // 现在可以安全地发送数据
            ESP_LOGI(GATTC_TAG, " 发送开锁数据...");
           //发送开锁数据
           send_data_to_lock(gattc_if, 
            gl_profile_tab1[PROFILE_D_APP_ID].conn_id,
            gl_profile_tab1[PROFILE_D_APP_ID].write_char_handle,
            TCP_lock_data, 
            TCP_lock_data_len);
        
        
        }
        break;
    case ESP_GATTC_SRVC_CHG_EVT: {  // 服务变更事件
        esp_bd_addr_t bda;
        memcpy(bda, p_data->srvc_chg.remote_bda, sizeof(esp_bd_addr_t));  // 复制设备地址
        ESP_LOGI(GATTC_TAG, "服务变更来自 "ESP_BD_ADDR_STR"", ESP_BD_ADDR_HEX(bda));
        break;
    }
    case ESP_GATTC_WRITE_CHAR_EVT:  // 写入特征事件
        if (p_data->write.status != ESP_GATT_OK){
            ESP_LOGE(GATTC_TAG, "特征写入失败, 状态 %x", p_data->write.status);
            break;
        }
        ESP_LOGI(GATTC_TAG, "特征写入成功");
        break;
    case ESP_GATTC_DISCONNECT_EVT:  // 断开连接事件
    if (memcmp(p_data->connect.remote_bda, lock_mac4, 6) != 0) {
        ESP_LOGI(GATTC_TAG, "忽略非本 Profile 的断开事件");
        break; // 不保存 conn_id，也不发送 MTU 请求
         }
        conn_device_d = false;     // 重置连接标志
        get_serverd = false;  // 重置获取服务器标志
        ESP_LOGI(GATTC_TAG, "已断开连接, 远程设备D "ESP_BD_ADDR_STR", 原因 0x%02x",
                 ESP_BD_ADDR_HEX(p_data->disconnect.remote_bda), p_data->disconnect.reason);
        break;
    default:
        break;
    }
}


static void gattc_profile_e_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{
    esp_ble_gattc_cb_param_t *p_data = (esp_ble_gattc_cb_param_t *)param;  // 参数指针

    switch (event) {
    case ESP_GATTC_REG_EVT:  // GATT客户端注册事件
        ESP_LOGI(GATTC_TAG, "GATT客户端注册, 状态 %d, 应用ID %d, GATT接口 %d", param->reg.status, param->reg.app_id, gattc_if);
        esp_err_t scan_ret = esp_ble_gap_set_scan_params(&ble_scan_params);  // 设置扫描参数
        if (scan_ret){
            ESP_LOGE(GATTC_TAG, "设置扫描参数错误, 错误代码 = %x", scan_ret);
        }
        break;
    case ESP_GATTC_CONNECT_EVT:{  // 连接事件
    
        if (memcmp(p_data->connect.remote_bda, lock_mac5, 6) != 0) {
            ESP_LOGI(GATTC_TAG, "忽略非本 Profile 的连接事件");
            break; // 不保存 conn_id，也不发送 MTU 请求
             }
        ESP_LOGI(GATTC_TAG, "已连接, 连接ID %d, 远程设备 "ESP_BD_ADDR_STR"", p_data->connect.conn_id,
                 ESP_BD_ADDR_HEX(p_data->connect.remote_bda));
        gl_profile_tab1[PROFILE_E_APP_ID].conn_id = p_data->connect.conn_id;  // 保存连接ID
        memcpy(gl_profile_tab1[PROFILE_E_APP_ID].remote_bda, p_data->connect.remote_bda, sizeof(esp_bd_addr_t));  // 复制远程设备地址
        esp_err_t mtu_ret = esp_ble_gattc_send_mtu_req (gattc_if, p_data->connect.conn_id);  // 发送MTU请求
        if (mtu_ret){
            ESP_LOGE(GATTC_TAG, "配置MTU错误, 错误代码 = %x", mtu_ret);
        }


     
        break;
    }
    case ESP_GATTC_OPEN_EVT:  // 打开连接事件
        if (param->open.status != ESP_GATT_OK){
            ESP_LOGE(GATTC_TAG, "打开失败, 状态 %d", p_data->open.status);
            break;
        }
        conn_device_e = true;
        //ESP_LOGI(GATTC_TAG, "成功打开, MTU %u", p_data->open.mtu);
        break;
    case ESP_GATTC_DIS_SRVC_CMPL_EVT:  // 发现服务完成事件
        if (param->dis_srvc_cmpl.status != ESP_GATT_OK){
            ESP_LOGE(GATTC_TAG, "服务发现失败, 状态 %d", param->dis_srvc_cmpl.status);
            break;
        }
        ESP_LOGI(GATTC_TAG, "服务发现完成, 连接ID %d", param->dis_srvc_cmpl.conn_id);
         // 打印要搜索的服务UUID
         ESP_LOGI(GATTC_TAG, "开始搜索服务, UUID:");
         esp_log_buffer_hex(GATTC_TAG, remote_filter_service_uuid.uuid.uuid128, 16);
        esp_ble_gattc_search_service(gattc_if, param->dis_srvc_cmpl.conn_id, NULL);  // 搜索特定UUID的服务
        break;
    case ESP_GATTC_CFG_MTU_EVT:  // MTU配置事件
        ESP_LOGI(GATTC_TAG, "MTU交换, 状态 %d, MTU %d", param->cfg_mtu.status, param->cfg_mtu.mtu);
        break;
        case ESP_GATTC_SEARCH_RES_EVT: {  // 服务搜索结果事件
            ESP_LOGI(GATTC_TAG, "服务搜索结果, 连接ID = %x", p_data->search_res.conn_id);
            ESP_LOGI(GATTC_TAG, "起始句柄 %d, 结束句柄 %d", 
                     p_data->search_res.start_handle, p_data->search_res.end_handle);
        
            if (p_data->search_res.srvc_id.uuid.len == ESP_UUID_LEN_128) {
                ESP_LOGI(GATTC_TAG, "找到128位UUID服务");
                ESP_LOGI(GATTC_TAG, "服务UUID128:");
                esp_log_buffer_hex(GATTC_TAG, p_data->search_res.srvc_id.uuid.uuid.uuid128, 16);
                
                // 打印目标UUID进行对比
                ESP_LOGI(GATTC_TAG, "目标UUID128:");
                esp_log_buffer_hex(GATTC_TAG, remote_filter_service_uuid.uuid.uuid128, 16);
                
                // BLE UUID是小端序，需要反转比较
                uint8_t reversed_uuid[16];
                for (int i = 0; i < 16; i++) {
                    reversed_uuid[i] = p_data->search_res.srvc_id.uuid.uuid.uuid128[15 - i];
                }
                
                ESP_LOGI(GATTC_TAG, "反转后服务UUID128:");
                esp_log_buffer_hex(GATTC_TAG, reversed_uuid, 16);
                
                // 比较UUID（注意BLE是小端序）
                if (memcmp(reversed_uuid, remote_filter_service_uuid.uuid.uuid128, 16) == 0) {
                    ESP_LOGI(GATTC_TAG, "*** 匹配到目标服务 ***");
                    get_servere = true;
                    gl_profile_tab1[PROFILE_E_APP_ID].service_start_handle = p_data->search_res.start_handle;
                    gl_profile_tab1[PROFILE_E_APP_ID].service_end_handle = p_data->search_res.end_handle;
                    ESP_LOGI(GATTC_TAG, "服务句柄范围: %d - %d", 
                             gl_profile_tab1[PROFILE_E_APP_ID].service_start_handle,
                             gl_profile_tab1[PROFILE_E_APP_ID].service_end_handle);
                } else {
                    ESP_LOGI(GATTC_TAG, "UUID不匹配");
                }
            }
            break;
        }
        case ESP_GATTC_SEARCH_CMPL_EVT:  // 服务搜索完成事件
        ESP_LOGI(GATTC_TAG, "服务搜索完成, 状态 %d", p_data->search_cmpl.status);
        ESP_LOGI(GATTC_TAG, "get_servere标志: %d", get_servere);
        
        if (p_data->search_cmpl.status != ESP_GATT_OK){
            ESP_LOGE(GATTC_TAG, "服务搜索失败, 状态 %x", p_data->search_cmpl.status);
            break;
        }
        
        if (get_servere){  // 如果找到目标服务
            uint16_t count = 0;
            esp_gatt_status_t status = esp_ble_gattc_get_attr_count( 
                gattc_if,
                p_data->search_cmpl.conn_id,
                ESP_GATT_DB_CHARACTERISTIC,
                gl_profile_tab1[PROFILE_E_APP_ID].service_start_handle,
                gl_profile_tab1[PROFILE_E_APP_ID].service_end_handle,
                INVALID_HANDLE,
                &count);
                
            if (status != ESP_GATT_OK){
                ESP_LOGE(GATTC_TAG, "获取特征数量错误, 状态: %d", status);
                break;
            }
            
            ESP_LOGI(GATTC_TAG, "找到 %d 个特征", count);
            
            if (count > 0){
                // 1. 首先获取并显示所有特征
                char_elem_result_e = (esp_gattc_char_elem_t *)malloc(sizeof(esp_gattc_char_elem_t) * count);
                if (!char_elem_result_e){
                    ESP_LOGE(GATTC_TAG, "内存不足, 无法分配特征元素数组");
                    break;
                }
                
                // 获取所有特征
                status = esp_ble_gattc_get_all_char(
                    gattc_if,
                    p_data->search_cmpl.conn_id,
                    gl_profile_tab1[PROFILE_E_APP_ID].service_start_handle,
                    gl_profile_tab1[PROFILE_E_APP_ID].service_end_handle,
                    char_elem_result_e,
                    &count,
                    0); // 偏移量0
                    
                if (status == ESP_GATT_OK && count > 0) {
                    ESP_LOGI(GATTC_TAG, "========== 服务中的所有特征 ==========");
                    for (int i = 0; i < count; i++) {
                        ESP_LOGI(GATTC_TAG, "特征[%d]: 句柄=%d, 属性=0x%02x", 
                                 i, char_elem_result_e[i].char_handle, char_elem_result_e[i].properties);
                        
                        // 打印特征属性描述
                        if (char_elem_result_e[i].properties & ESP_GATT_CHAR_PROP_BIT_READ) {
                            ESP_LOGI(GATTC_TAG, "  - 支持读取");
                        }
                        if (char_elem_result_e[i].properties & ESP_GATT_CHAR_PROP_BIT_WRITE) {
                            ESP_LOGI(GATTC_TAG, "  - 支持写入");
                        }
                        if (char_elem_result_e[i].properties & ESP_GATT_CHAR_PROP_BIT_WRITE_NR) {
                            ESP_LOGI(GATTC_TAG, "  - 支持无响应写入");
                        }
                        if (char_elem_result_e[i].properties & ESP_GATT_CHAR_PROP_BIT_NOTIFY) {
                            ESP_LOGI(GATTC_TAG, "  - 支持通知");
                        }
                        if (char_elem_result_e[i].properties & ESP_GATT_CHAR_PROP_BIT_INDICATE) {
                            ESP_LOGI(GATTC_TAG, "  - 支持指示");
                        }
                        
                        // 打印特征UUID
                        if (char_elem_result_e[i].uuid.len == ESP_UUID_LEN_128) {
                            ESP_LOGI(GATTC_TAG, "  - UUID128:");
                            esp_log_buffer_hex(GATTC_TAG, char_elem_result_e[i].uuid.uuid.uuid128, 16);
                            
                            // 反转UUID（BLE是小端序）
                            uint8_t reversed_uuid[16];
                            for (int j = 0; j < 16; j++) {
                                reversed_uuid[j] = char_elem_result_e[i].uuid.uuid.uuid128[15 - j];
                            }
                            ESP_LOGI(GATTC_TAG, "  - 反转后UUID128:");
                            esp_log_buffer_hex(GATTC_TAG, reversed_uuid, 16);
                        }
                        ESP_LOGI(GATTC_TAG, "----------------------------------------");
                    }
                    ESP_LOGI(GATTC_TAG, "========== 特征列表结束 ==========");
                    
                    // 2. 直接遍历特征数组，查找需要的特征
                    uint16_t write_char_handle = 0;
                    uint16_t notify_char_handle = 0;
                    
                    for (int i = 0; i < count; i++) {
                        // 检查特征UUID
                        if (char_elem_result_e[i].uuid.len == ESP_UUID_LEN_128) {
                            // 反转UUID进行比较
                            uint8_t reversed_uuid[16];
                            for (int j = 0; j < 16; j++) {
                                reversed_uuid[j] = char_elem_result_e[i].uuid.uuid.uuid128[j];
                            }
                            
                            // 检查是否是写特征
                            esp_bt_uuid_t target_write_uuid;
                            target_write_uuid.len = ESP_UUID_LEN_128;
                            for (int j = 0; j < 16; j++) {
                                target_write_uuid.uuid.uuid128[j] = write_filter_char_uuid.uuid.uuid128[15 - j];
                            }

                            ESP_LOGI(GATTC_TAG, "  - 检查特征UUID:");
                            esp_log_buffer_hex(GATTC_TAG, target_write_uuid.uuid.uuid128, 16);


                            ESP_LOGI(GATTC_TAG, "  - 检查111111111特征UUID:");
                            esp_log_buffer_hex(GATTC_TAG, reversed_uuid, 16);
                            if (memcmp(reversed_uuid, target_write_uuid.uuid.uuid128, 16) == 0) {
                                write_char_handle = char_elem_result_e[i].char_handle;
                                ESP_LOGI(GATTC_TAG, "找到写特征: 句柄=%d", write_char_handle);
                            }
                            
                            // 检查是否是通知特征
                            esp_bt_uuid_t target_notify_uuid;
                            target_notify_uuid.len = ESP_UUID_LEN_128;
                            for (int j = 0; j < 16; j++) {
                                target_notify_uuid.uuid.uuid128[j] = notify_descr_uuid.uuid.uuid128[15 - j];
                            }
                            
                            if (memcmp(reversed_uuid, target_notify_uuid.uuid.uuid128, 16) == 0) {
                                notify_char_handle = char_elem_result_e[i].char_handle;
                                ESP_LOGI(GATTC_TAG, "找到通知特征: 句柄=%d", notify_char_handle);
                            }
                        }
                    }
                    
                    // 3. 处理找到的特征
                    if (write_char_handle != 0) {
                        gl_profile_tab1[PROFILE_E_APP_ID].write_char_handle = write_char_handle;
                        ESP_LOGI(GATTC_TAG, "使用写特征句柄: %d", write_char_handle);
                        
                        if (notify_char_handle != 0) {
                            ESP_LOGI(GATTC_TAG, "注册通知...");
                            esp_ble_gattc_register_for_notify(
                                gattc_if, 
                                gl_profile_tab1[PROFILE_E_APP_ID].remote_bda, 
                                notify_char_handle);
                        } else {
                            ESP_LOGW(GATTC_TAG, "未找到通知特征");
                        }
                        
                        // 4. 直接发送开锁数据
                       
                    } else {
                        ESP_LOGE(GATTC_TAG, "未找到写特征");
                    }
                } else {
                    ESP_LOGE(GATTC_TAG, "获取所有特征失败, 状态: %d", status);
                }
                
                free(char_elem_result_e);
                char_elem_result_e = NULL;
            } else {
                ESP_LOGE(GATTC_TAG, "服务中没有特征");
            }
        } else {
            ESP_LOGE(GATTC_TAG, "未找到目标服务");
        }
        break;
        case ESP_GATTC_REG_FOR_NOTIFY_EVT: {  // 注册通知事件
            if (p_data->reg_for_notify.status != ESP_GATT_OK){
                ESP_LOGE(GATTC_TAG, "通知注册失败, 状态 %d", p_data->reg_for_notify.status);
            } else {
                ESP_LOGI(GATTC_TAG, " 通知注册成功");
                
           
                
                // 启用通知（写入CCCD）
                uint16_t count = 0;
                uint16_t notify_en = 1;  // 启用通知的值
                
                // 使用标准的CCCD UUID (0x2902)
                esp_bt_uuid_t cccd_uuid = {
                    .len = ESP_UUID_LEN_16,
                    .uuid = {.uuid16 = 0x2902}
                };
        
                esp_gatt_status_t ret_status = esp_ble_gattc_get_attr_count( 
                    gattc_if,
                    gl_profile_tab1[PROFILE_E_APP_ID].conn_id,
                    ESP_GATT_DB_DESCRIPTOR,
                    gl_profile_tab1[PROFILE_E_APP_ID].service_start_handle,
                    gl_profile_tab1[PROFILE_E_APP_ID].service_end_handle,
                    p_data->reg_for_notify.handle,  // 通知特征的句柄
                    &count);
                    
                if (ret_status != ESP_GATT_OK){
                    ESP_LOGE(GATTC_TAG, "获取描述符数量错误, 状态: %d", ret_status);
                    // 如果获取描述符失败，直接发送数据
                    ESP_LOGI(GATTC_TAG, " 无法启用通知，直接发送数据");
                    
                    break;
                }
                
                ESP_LOGI(GATTC_TAG, "找到 %d 个描述符", count);
                
                if (count > 0){
                    descr_elem_result_e = malloc(sizeof(esp_gattc_descr_elem_t) * count);
                    if (!descr_elem_result_e){
                        ESP_LOGE(GATTC_TAG, "内存分配错误");
                        // 内存分配失败，直接发送数据
                        ESP_LOGI(GATTC_TAG, " 内存不足，直接发送数据");
                       
                        break;
                    }
                    
                    // 搜索CCCD描述符
                    ret_status = esp_ble_gattc_get_descr_by_char_handle(
                        gattc_if,
                        gl_profile_tab1[PROFILE_E_APP_ID].conn_id,
                        p_data->reg_for_notify.handle,
                        cccd_uuid,
                        descr_elem_result_e,
                        &count);
                        
                    if (ret_status != ESP_GATT_OK || count == 0){
                        ESP_LOGE(GATTC_TAG, "获取CCCD描述符失败, 状态: %d, 数量: %d", ret_status, count);
                        free(descr_elem_result_e);
                        descr_elem_result_e = NULL;
                        // 如果找不到CCCD，直接发送数据
                        ESP_LOGI(GATTC_TAG, " 未找到CCCD描述符，直接发送数据");
                       
                        break;
                    }
                    
                    ESP_LOGI(GATTC_TAG, "找到CCCD描述符, 句柄: %d", descr_elem_result_e[0].handle);
                    
                    // 写入CCCD以启用通知
                    ret_status = esp_ble_gattc_write_char_descr(
                        gattc_if,
                        gl_profile_tab1[PROFILE_E_APP_ID].conn_id,
                        descr_elem_result_e[0].handle,
                        sizeof(notify_en),
                        (uint8_t *)&notify_en,
                        ESP_GATT_WRITE_TYPE_RSP,
                        ESP_GATT_AUTH_REQ_NONE);
                        
                    if (ret_status != ESP_GATT_OK){
                        ESP_LOGE(GATTC_TAG, "写入CCCD描述符失败, 状态: %d", ret_status);
                        // 写入失败，直接发送数据
                        ESP_LOGI(GATTC_TAG, " 启用通知失败，直接发送数据");
                       
                    } else {
                        ESP_LOGI(GATTC_TAG, " 写入CCCD描述符请求已发送，等待启用完成...");
                        // 数据发送将在ESP_GATTC_WRITE_DESCR_EVT事件中处理
                    }
                    
                    free(descr_elem_result_e);
                    descr_elem_result_e = NULL;
                } else {
                    ESP_LOGW(GATTC_TAG, "未找到描述符，直接发送数据");
                
                }
            }
            break;
        }
    case ESP_GATTC_NOTIFY_EVT:  // 通知事件

    lock_send_tcp_sever(p_data->notify.value,p_data->notify.value_len,5);

        break;
        case ESP_GATTC_WRITE_DESCR_EVT:  // 写入描述符事件
        if (p_data->write.status != ESP_GATT_OK){
            ESP_LOGE(GATTC_TAG, " 描述符写入失败, 状态 %x", p_data->write.status);
            // 即使描述符写入失败，也尝试发送数据
            ESP_LOGI(GATTC_TAG, " 通知启用失败，尝试发送数据...");
           
        } else {
            ESP_LOGI(GATTC_TAG, " 描述符写入成功，通知已启用");
            // 现在可以安全地发送数据
            ESP_LOGI(GATTC_TAG, " 发送开锁数据...");
           //发送开锁数据
           send_data_to_lock(gattc_if, 
            gl_profile_tab1[PROFILE_E_APP_ID].conn_id,
            gl_profile_tab1[PROFILE_E_APP_ID].write_char_handle,
            TCP_lock_data, 
            TCP_lock_data_len);
        
        
        }
        break;
    case ESP_GATTC_SRVC_CHG_EVT: {  // 服务变更事件
        esp_bd_addr_t bda;
        memcpy(bda, p_data->srvc_chg.remote_bda, sizeof(esp_bd_addr_t));  // 复制设备地址
        ESP_LOGI(GATTC_TAG, "服务变更来自 "ESP_BD_ADDR_STR"", ESP_BD_ADDR_HEX(bda));
        break;
    }
    case ESP_GATTC_WRITE_CHAR_EVT:  // 写入特征事件
        if (p_data->write.status != ESP_GATT_OK){
            ESP_LOGE(GATTC_TAG, "特征写入失败, 状态 %x", p_data->write.status);
            break;
        }
        ESP_LOGI(GATTC_TAG, "特征写入成功");
        break;
    case ESP_GATTC_DISCONNECT_EVT:  // 断开连接事件
    if (memcmp(p_data->connect.remote_bda, lock_mac5, 6) != 0) {
        ESP_LOGI(GATTC_TAG, "忽略非本 Profile 的断开事件");
        break; // 不保存 conn_id，也不发送 MTU 请求
         }
        conn_device_e = false;     // 重置连接标志
        get_servere = false;  // 重置获取服务器标志
        ESP_LOGI(GATTC_TAG, "已断开连接, 远程设备E "ESP_BD_ADDR_STR", 原因 0x%02x",
                 ESP_BD_ADDR_HEX(p_data->disconnect.remote_bda), p_data->disconnect.reason);
        break;
    default:
        break;
    }
}


static void gattc_profile_f_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{
    esp_ble_gattc_cb_param_t *p_data = (esp_ble_gattc_cb_param_t *)param;  // 参数指针

    switch (event) {
    case ESP_GATTC_REG_EVT:  // GATT客户端注册事件
        ESP_LOGI(GATTC_TAG, "GATT客户端注册, 状态 %d, 应用ID %d, GATT接口 %d", param->reg.status, param->reg.app_id, gattc_if);
        esp_err_t scan_ret = esp_ble_gap_set_scan_params(&ble_scan_params);  // 设置扫描参数
        if (scan_ret){
            ESP_LOGE(GATTC_TAG, "设置扫描参数错误, 错误代码 = %x", scan_ret);
        }
        break;
    case ESP_GATTC_CONNECT_EVT:{  // 连接事件
        if (memcmp(p_data->connect.remote_bda, lock_mac6, 6) != 0) {
            ESP_LOGI(GATTC_TAG, "忽略非本 Profile 的连接事件");
            break; // 不保存 conn_id，也不发送 MTU 请求
             }
        ESP_LOGI(GATTC_TAG, "已连接, 连接ID %d, 远程设备 "ESP_BD_ADDR_STR"", p_data->connect.conn_id,
                 ESP_BD_ADDR_HEX(p_data->connect.remote_bda));
        gl_profile_tab1[PROFILE_F_APP_ID].conn_id = p_data->connect.conn_id;  // 保存连接ID
        memcpy(gl_profile_tab1[PROFILE_F_APP_ID].remote_bda, p_data->connect.remote_bda, sizeof(esp_bd_addr_t));  // 复制远程设备地址
        esp_err_t mtu_ret = esp_ble_gattc_send_mtu_req (gattc_if, p_data->connect.conn_id);  // 发送MTU请求
        if (mtu_ret){
            ESP_LOGE(GATTC_TAG, "配置MTU错误, 错误代码 = %x", mtu_ret);
        }


     
        break;
    }
    case ESP_GATTC_OPEN_EVT:  // 打开连接事件
        if (param->open.status != ESP_GATT_OK){
            ESP_LOGE(GATTC_TAG, "打开失败, 状态 %d", p_data->open.status);
            break;
        }
        conn_device_f = true;
      //  ESP_LOGI(GATTC_TAG, "成功打开, MTU %u", p_data->open.mtu);
        break;
    case ESP_GATTC_DIS_SRVC_CMPL_EVT:  // 发现服务完成事件
        if (param->dis_srvc_cmpl.status != ESP_GATT_OK){
            ESP_LOGE(GATTC_TAG, "服务发现失败, 状态 %d", param->dis_srvc_cmpl.status);
            break;
        }
        ESP_LOGI(GATTC_TAG, "服务发现完成, 连接ID %d", param->dis_srvc_cmpl.conn_id);
         // 打印要搜索的服务UUID
         ESP_LOGI(GATTC_TAG, "开始搜索服务, UUID:");
         esp_log_buffer_hex(GATTC_TAG, remote_filter_service_uuid.uuid.uuid128, 16);
        esp_ble_gattc_search_service(gattc_if, param->dis_srvc_cmpl.conn_id, NULL);  // 搜索特定UUID的服务
        break;
    case ESP_GATTC_CFG_MTU_EVT:  // MTU配置事件
        ESP_LOGI(GATTC_TAG, "MTU交换, 状态 %d, MTU %d", param->cfg_mtu.status, param->cfg_mtu.mtu);
        break;
        case ESP_GATTC_SEARCH_RES_EVT: {  // 服务搜索结果事件
            ESP_LOGI(GATTC_TAG, "服务搜索结果, 连接ID = %x", p_data->search_res.conn_id);
            ESP_LOGI(GATTC_TAG, "起始句柄 %d, 结束句柄 %d", 
                     p_data->search_res.start_handle, p_data->search_res.end_handle);
        
            if (p_data->search_res.srvc_id.uuid.len == ESP_UUID_LEN_128) {
                //ESP_LOGI(GATTC_TAG, "找到128位UUID服务");
               // ESP_LOGI(GATTC_TAG, "服务UUID128:");
                //esp_log_buffer_hex(GATTC_TAG, p_data->search_res.srvc_id.uuid.uuid.uuid128, 16);
                
                // 打印目标UUID进行对比
               // ESP_LOGI(GATTC_TAG, "目标UUID128:");
               // esp_log_buffer_hex(GATTC_TAG, remote_filter_service_uuid.uuid.uuid128, 16);
                
                // BLE UUID是小端序，需要反转比较
                uint8_t reversed_uuid[16];
                for (int i = 0; i < 16; i++) {
                    reversed_uuid[i] = p_data->search_res.srvc_id.uuid.uuid.uuid128[15 - i];
                }
                
               // ESP_LOGI(GATTC_TAG, "反转后服务UUID128:");
             //   esp_log_buffer_hex(GATTC_TAG, reversed_uuid, 16);
                
                // 比较UUID（注意BLE是小端序）
                if (memcmp(reversed_uuid, remote_filter_service_uuid.uuid.uuid128, 16) == 0) {
                    ESP_LOGI(GATTC_TAG, "*** 匹配到目标服务 ***");
                    get_serverf = true;
                    gl_profile_tab1[PROFILE_F_APP_ID].service_start_handle = p_data->search_res.start_handle;
                    gl_profile_tab1[PROFILE_F_APP_ID].service_end_handle = p_data->search_res.end_handle;
                    ESP_LOGI(GATTC_TAG, "服务句柄范围: %d - %d", 
                             gl_profile_tab1[PROFILE_F_APP_ID].service_start_handle,
                             gl_profile_tab1[PROFILE_F_APP_ID].service_end_handle);
                } else {
                    ESP_LOGI(GATTC_TAG, "UUID不匹配");
                }
            }
            break;
        }
        case ESP_GATTC_SEARCH_CMPL_EVT:  // 服务搜索完成事件
        ESP_LOGI(GATTC_TAG, "服务搜索完成, 状态 %d", p_data->search_cmpl.status);
        ESP_LOGI(GATTC_TAG, "get_server标志: %d", get_serverf);
        
        if (p_data->search_cmpl.status != ESP_GATT_OK){
            ESP_LOGE(GATTC_TAG, "服务搜索失败, 状态 %x", p_data->search_cmpl.status);
            break;
        }
        
        if (get_serverf){  // 如果找到目标服务
            uint16_t count = 0;
            esp_gatt_status_t status = esp_ble_gattc_get_attr_count( 
                gattc_if,
                p_data->search_cmpl.conn_id,
                ESP_GATT_DB_CHARACTERISTIC,
                gl_profile_tab1[PROFILE_F_APP_ID].service_start_handle,
                gl_profile_tab1[PROFILE_F_APP_ID].service_end_handle,
                INVALID_HANDLE,
                &count);
                
            if (status != ESP_GATT_OK){
                ESP_LOGE(GATTC_TAG, "获取特征数量错误, 状态: %d", status);
                break;
            }
            
            ESP_LOGI(GATTC_TAG, "找到 %d 个特征", count);
            
            if (count > 0){
                // 1. 首先获取并显示所有特征
                char_elem_result_f = (esp_gattc_char_elem_t *)malloc(sizeof(esp_gattc_char_elem_t) * count);
                if (!char_elem_result_f){
                    ESP_LOGE(GATTC_TAG, "内存不足, 无法分配特征元素数组");
                    break;
                }
                
                // 获取所有特征
                status = esp_ble_gattc_get_all_char(
                    gattc_if,
                    p_data->search_cmpl.conn_id,
                    gl_profile_tab1[PROFILE_F_APP_ID].service_start_handle,
                    gl_profile_tab1[PROFILE_F_APP_ID].service_end_handle,
                    char_elem_result_f,
                    &count,
                    0); // 偏移量0
                    
                if (status == ESP_GATT_OK && count > 0) {
                    ESP_LOGI(GATTC_TAG, "========== 服务中的所有特征 ==========");
                    for (int i = 0; i < count; i++) {
                        ESP_LOGI(GATTC_TAG, "特征[%d]: 句柄=%d, 属性=0x%02x", 
                                 i, char_elem_result_f[i].char_handle, char_elem_result_f[i].properties);
                        
                        // 打印特征属性描述
                        if (char_elem_result_f[i].properties & ESP_GATT_CHAR_PROP_BIT_READ) {
                            ESP_LOGI(GATTC_TAG, "  - 支持读取");
                        }
                        if (char_elem_result_f[i].properties & ESP_GATT_CHAR_PROP_BIT_WRITE) {
                            ESP_LOGI(GATTC_TAG, "  - 支持写入");
                        }
                        if (char_elem_result_f[i].properties & ESP_GATT_CHAR_PROP_BIT_WRITE_NR) {
                            ESP_LOGI(GATTC_TAG, "  - 支持无响应写入");
                        }
                        if (char_elem_result_f[i].properties & ESP_GATT_CHAR_PROP_BIT_NOTIFY) {
                            ESP_LOGI(GATTC_TAG, "  - 支持通知");
                        }
                        if (char_elem_result_f[i].properties & ESP_GATT_CHAR_PROP_BIT_INDICATE) {
                            ESP_LOGI(GATTC_TAG, "  - 支持指示");
                        }
                        
                        // 打印特征UUID
                        if (char_elem_result_f[i].uuid.len == ESP_UUID_LEN_128) {
                            ESP_LOGI(GATTC_TAG, "  - UUID128:");
                            esp_log_buffer_hex(GATTC_TAG, char_elem_result_f[i].uuid.uuid.uuid128, 16);
                            
                            // 反转UUID（BLE是小端序）
                            uint8_t reversed_uuid[16];
                            for (int j = 0; j < 16; j++) {
                                reversed_uuid[j] = char_elem_result_f[i].uuid.uuid.uuid128[15 - j];
                            }
                            ESP_LOGI(GATTC_TAG, "  - 反转后UUID128:");
                            esp_log_buffer_hex(GATTC_TAG, reversed_uuid, 16);
                        }
                        ESP_LOGI(GATTC_TAG, "----------------------------------------");
                    }
                    ESP_LOGI(GATTC_TAG, "========== 特征列表结束 ==========");
                    
                    // 2. 直接遍历特征数组，查找需要的特征
                    uint16_t write_char_handle = 0;
                    uint16_t notify_char_handle = 0;
                    
                    for (int i = 0; i < count; i++) {
                        // 检查特征UUID
                        if (char_elem_result_f[i].uuid.len == ESP_UUID_LEN_128) {
                            // 反转UUID进行比较
                            uint8_t reversed_uuid[16];
                            for (int j = 0; j < 16; j++) {
                                reversed_uuid[j] = char_elem_result_f[i].uuid.uuid.uuid128[j];
                            }
                            
                            // 检查是否是写特征
                            esp_bt_uuid_t target_write_uuid;
                            target_write_uuid.len = ESP_UUID_LEN_128;
                            for (int j = 0; j < 16; j++) {
                                target_write_uuid.uuid.uuid128[j] = write_filter_char_uuid.uuid.uuid128[15 - j];
                            }

                           // ESP_LOGI(GATTC_TAG, "  - 检查特征UUID:");
                           // esp_log_buffer_hex(GATTC_TAG, target_write_uuid.uuid.uuid128, 16);


                           
                            esp_log_buffer_hex(GATTC_TAG, reversed_uuid, 16);
                            if (memcmp(reversed_uuid, target_write_uuid.uuid.uuid128, 16) == 0) {
                                write_char_handle = char_elem_result_f[i].char_handle;
                                ESP_LOGI(GATTC_TAG, "找到写特征: 句柄=%d", write_char_handle);
                            }
                            
                            // 检查是否是通知特征
                            esp_bt_uuid_t target_notify_uuid;
                            target_notify_uuid.len = ESP_UUID_LEN_128;
                            for (int j = 0; j < 16; j++) {
                                target_notify_uuid.uuid.uuid128[j] = notify_descr_uuid.uuid.uuid128[15 - j];
                            }
                            
                            if (memcmp(reversed_uuid, target_notify_uuid.uuid.uuid128, 16) == 0) {
                                notify_char_handle = char_elem_result_f[i].char_handle;
                                ESP_LOGI(GATTC_TAG, "找到通知特征: 句柄=%d", notify_char_handle);
                            }
                        }
                    }
                    
                    // 3. 处理找到的特征
                    if (write_char_handle != 0) {
                        gl_profile_tab1[PROFILE_F_APP_ID].write_char_handle = write_char_handle;
                        ESP_LOGI(GATTC_TAG, "使用写特征句柄: %d", write_char_handle);
                        
                        if (notify_char_handle != 0) {
                            ESP_LOGI(GATTC_TAG, "注册通知...");
                            esp_ble_gattc_register_for_notify(
                                gattc_if, 
                                gl_profile_tab1[PROFILE_F_APP_ID].remote_bda, 
                                notify_char_handle);
                        } else {
                            ESP_LOGW(GATTC_TAG, "未找到通知特征");
                        }
                        
                        // 4. 直接发送开锁数据
                       
                    } else {
                        ESP_LOGE(GATTC_TAG, "未找到写特征");
                    }
                } else {
                    ESP_LOGE(GATTC_TAG, "获取所有特征失败, 状态: %d", status);
                }
                
                free(char_elem_result_f);
                char_elem_result_f = NULL;
            } else {
                ESP_LOGE(GATTC_TAG, "服务中没有特征");
            }
        } else {
            ESP_LOGE(GATTC_TAG, "未找到目标服务");
        }
        break;
        case ESP_GATTC_REG_FOR_NOTIFY_EVT: {  // 注册通知事件
            if (p_data->reg_for_notify.status != ESP_GATT_OK){
                ESP_LOGE(GATTC_TAG, "通知注册失败, 状态 %d", p_data->reg_for_notify.status);
            } else {
                ESP_LOGI(GATTC_TAG, " 通知注册成功");
                
           
                
                // 启用通知（写入CCCD）
                uint16_t count = 0;
                uint16_t notify_en = 1;  // 启用通知的值
                
                // 使用标准的CCCD UUID (0x2902)
                esp_bt_uuid_t cccd_uuid = {
                    .len = ESP_UUID_LEN_16,
                    .uuid = {.uuid16 = 0x2902}
                };
        
                esp_gatt_status_t ret_status = esp_ble_gattc_get_attr_count( 
                    gattc_if,
                    gl_profile_tab1[PROFILE_F_APP_ID].conn_id,
                    ESP_GATT_DB_DESCRIPTOR,
                    gl_profile_tab1[PROFILE_F_APP_ID].service_start_handle,
                    gl_profile_tab1[PROFILE_F_APP_ID].service_end_handle,
                    p_data->reg_for_notify.handle,  // 通知特征的句柄
                    &count);
                    
                if (ret_status != ESP_GATT_OK){
                    ESP_LOGE(GATTC_TAG, "获取描述符数量错误, 状态: %d", ret_status);
                    // 如果获取描述符失败，直接发送数据
                    ESP_LOGI(GATTC_TAG, " 无法启用通知，直接发送数据");
                    
                    break;
                }
                
                ESP_LOGI(GATTC_TAG, "找到 %d 个描述符", count);
                
                if (count > 0){
                    descr_elem_result_f = malloc(sizeof(esp_gattc_descr_elem_t) * count);
                    if (!descr_elem_result_f){
                        ESP_LOGE(GATTC_TAG, "内存分配错误");
                        // 内存分配失败，直接发送数据
                        ESP_LOGI(GATTC_TAG, " 内存不足，直接发送数据");
                       
                        break;
                    }
                    
                    // 搜索CCCD描述符
                    ret_status = esp_ble_gattc_get_descr_by_char_handle(
                        gattc_if,
                        gl_profile_tab1[PROFILE_F_APP_ID].conn_id,
                        p_data->reg_for_notify.handle,
                        cccd_uuid,
                        descr_elem_result_f,
                        &count);
                        
                    if (ret_status != ESP_GATT_OK || count == 0){
                        ESP_LOGE(GATTC_TAG, "获取CCCD描述符失败, 状态: %d, 数量: %d", ret_status, count);
                        free(descr_elem_result_f);
                        descr_elem_result_f = NULL;
                        // 如果找不到CCCD，直接发送数据
                        ESP_LOGI(GATTC_TAG, " 未找到CCCD描述符，直接发送数据");
                       
                        break;
                    }
                    
                    ESP_LOGI(GATTC_TAG, "找到CCCD描述符, 句柄: %d", descr_elem_result_f[0].handle);
                    
                    // 写入CCCD以启用通知
                    ret_status = esp_ble_gattc_write_char_descr(
                        gattc_if,
                        gl_profile_tab1[PROFILE_F_APP_ID].conn_id,
                        descr_elem_result_f[0].handle,
                        sizeof(notify_en),
                        (uint8_t *)&notify_en,
                        ESP_GATT_WRITE_TYPE_RSP,
                        ESP_GATT_AUTH_REQ_NONE);
                        
                    if (ret_status != ESP_GATT_OK){
                        ESP_LOGE(GATTC_TAG, "写入CCCD描述符失败, 状态: %d", ret_status);
                      
                       
                    } else {
                        ESP_LOGI(GATTC_TAG, " 写入CCCD描述符请求已发送，等待启用完成...");
                        // 数据发送将在ESP_GATTC_WRITE_DESCR_EVT事件中处理
                    }
                    
                    free(descr_elem_result_f);
                    descr_elem_result_f = NULL;
                } else {
                    ESP_LOGW(GATTC_TAG, "未找到描述符，直接发送数据");
                
                }
            }
            break;
        }
    case ESP_GATTC_NOTIFY_EVT:  // 通知事件

    lock_send_tcp_sever(p_data->notify.value,p_data->notify.value_len,6);

        break;
        case ESP_GATTC_WRITE_DESCR_EVT:  // 写入描述符事件
        if (p_data->write.status != ESP_GATT_OK){
            ESP_LOGE(GATTC_TAG, " 描述符写入失败, 状态 %x", p_data->write.status);
            // 即使描述符写入失败，也尝试发送数据
            ESP_LOGI(GATTC_TAG, " 通知启用失败，尝试发送数据...");
           
        } else {
            ESP_LOGI(GATTC_TAG, " 描述符写入成功，通知已启用");
            // 现在可以安全地发送数据
            ESP_LOGI(GATTC_TAG, " 发送开锁数据...");
           //发送开锁数据
           send_data_to_lock(gattc_if, 
            gl_profile_tab1[PROFILE_F_APP_ID].conn_id,
            gl_profile_tab1[PROFILE_F_APP_ID].write_char_handle,
            TCP_lock_data, 
            TCP_lock_data_len);
        
        
        }
        break;
    case ESP_GATTC_SRVC_CHG_EVT: {  // 服务变更事件
        esp_bd_addr_t bda;
        memcpy(bda, p_data->srvc_chg.remote_bda, sizeof(esp_bd_addr_t));  // 复制设备地址
        ESP_LOGI(GATTC_TAG, "服务变更来自 "ESP_BD_ADDR_STR"", ESP_BD_ADDR_HEX(bda));
        break;
    }
    case ESP_GATTC_WRITE_CHAR_EVT:  // 写入特征事件
        if (p_data->write.status != ESP_GATT_OK){
            ESP_LOGE(GATTC_TAG, "特征写入失败, 状态 %x", p_data->write.status);
            break;
        }
        ESP_LOGI(GATTC_TAG, "特征写入成功");
        break;
    case ESP_GATTC_DISCONNECT_EVT:  // 断开连接事件
    if (memcmp(p_data->connect.remote_bda, lock_mac6, 6) != 0) {
        ESP_LOGI(GATTC_TAG, "忽略非本 Profile 的连接事件");
        break; // 不保存 conn_id，也不发送 MTU 请求
         }



        conn_device_f = false;     // 重置连接标志
        get_serverf = false;  // 重置获取服务器标志
        ESP_LOGI(GATTC_TAG, "已断开连接, 远程设备f "ESP_BD_ADDR_STR", 原因 0x%02x",
                 ESP_BD_ADDR_HEX(p_data->disconnect.remote_bda), p_data->disconnect.reason);
        break;
    default:
        break;
    }
}



static void esp_gattc_cb(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{
    /* If event is register event, store the gattc_if for each profile */
    if (event == ESP_GATTC_REG_EVT) {
        if (param->reg.status == ESP_GATT_OK) {
            gl_profile_tab1[param->reg.app_id].gattc_if = gattc_if;
        } else {
            ESP_LOGI(GATTC_TAG, "%04x, status %d",
                    param->reg.app_id,
                    param->reg.status);
            return;
        }
    }

    /* If the gattc_if equal to profile A, call profile A cb handler,
     * so here call each profile's callback */
    do {
        int idx;
        for (idx = 0; idx < PROFILE_NUMC; idx++) {
            if (gattc_if == ESP_GATT_IF_NONE || /* ESP_GATT_IF_NONE, not specify a certain gatt_if, need to call every profile cb function */
                    gattc_if == gl_profile_tab1[idx].gattc_if) {
                if (gl_profile_tab1[idx].gattc_cb) {
                    gl_profile_tab1[idx].gattc_cb(event, gattc_if, param);
                }
            }
        }
    } while (0);
}
