#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "lv_port.h"
#include "lvgl.h"
#include "lv_demos.h"
#include "st7789_i80.h"
#include "driver/i2s_std.h"
#include "xl9555.h"
#include "gui_guider.h"
#include "custom.h"
#include "events_init.h"
#include "my_http_server.h"
#include "my_wifi.h"
#include "esp_log.h"
#define TAG "main"
void app_main(void) {
    // 初始化NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // 检查是否已有保存的Wi-Fi凭证
    nvs_handle_t nvs_handle;
    char saved_ssid[32] = {0};
    char saved_password[64] = {0};
    
    if (nvs_open("storage", NVS_READONLY, &nvs_handle) == ESP_OK) {
        size_t ssid_len = sizeof(saved_ssid);
        size_t pass_len = sizeof(saved_password);
        
        if (nvs_get_str(nvs_handle, "ssid", saved_ssid, &ssid_len) == ESP_OK &&
            nvs_get_str(nvs_handle, "password", saved_password, &pass_len) == ESP_OK) {
            
            
            connect_to_wifi(saved_ssid, saved_password);
            return;
        }
        nvs_close(nvs_handle);
    }
    
    // 无保存凭证，进入配网模式
    ESP_LOGI(TAG, "未找到保存的Wi-Fi凭证，进入配网模式");
    wifi_init_softap();
    start_webserver();
    
    // 保持运行，等待用户配置
    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

