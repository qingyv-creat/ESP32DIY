#include <stdio.h>
#include "my_wifi.h"
#include "my_http_server.h"
#include <string.h>
#define TAG "main"
int connect_num = 0;
#define max_connect_num 5
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        // 可以在这里处理重连逻辑
        ESP_LOGI(TAG, "Wi-Fi 断开，尝试重连...");
      
      
      if(connect_num<max_connect_num) {esp_wifi_connect();}
connect_num++;
if(connect_num >= max_connect_num){
            ESP_LOGI(TAG, "Wi-Fi 超出重连次数，停止Wi-Fi...");
    //esp_wifi_stop();  

    erase_wifi_credentials();
  

}

    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "获得IP地址: " IPSTR, IP2STR(&event->ip_info.ip));
        // 设置连接成功的事件位
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void connect_to_wifi(const char* ssid, const char* password) {
    ESP_LOGI(TAG, "连接Wi-Fi: %s", ssid);

    // 1. 创建事件组
    wifi_event_group = xEventGroupCreate();

    // 2. 初始化网络接口和事件循环
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    // 3. 初始化Wi-Fi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // 4. 注册Wi-Fi和IP事件处理程序
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));
    // 5. 配置Wi-Fi STA模式
    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        }
    };
    strncpy((char*)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char*)wifi_config.sta.password, password, sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    // 6. 等待连接成功
    ESP_LOGI(TAG, "等待Wi-Fi连接...");
    EventBits_t bits = xEventGroupWaitBits(wifi_event_group,
                                            WIFI_CONNECTED_BIT,
                                            pdFALSE, // 不清除位
                                            pdTRUE,  // 等待所有位
                                            portMAX_DELAY);
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Wi-Fi连接成功！");
    } else {
        ESP_LOGE(TAG, "Wi-Fi连接失败或超时");
    }
}
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
            
            // 已有凭证，直接连接Wi-Fi
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

// 连接目标Wi-Fi的函数
