#include "BLUE_Init.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_mac.h"
#define TAG "BLUE_INIT"
#include "storage.h"
extern uint8_t mac_addr[6];
esp_err_t BLUE_Init(void)
{
    esp_err_t ret;
   
    

    // 释放经典蓝牙内存
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    // 初始化蓝牙控制器
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        ESP_LOGE(TAG, "初始化控制器失败: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        ESP_LOGE(TAG, "启用控制器失败: %s", esp_err_to_name(ret));
        return ret;
    }

    // 初始化Bluedroid
    ret = esp_bluedroid_init();
    if (ret) {
        ESP_LOGE(TAG, "初始化蓝牙协议栈失败: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_bluedroid_enable();
    if (ret) {
        ESP_LOGE(TAG, "启用蓝牙协议栈失败: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "蓝牙初始化完成");
    return ESP_OK;
}