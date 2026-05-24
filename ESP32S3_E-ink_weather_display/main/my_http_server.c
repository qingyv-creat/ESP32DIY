#include "my_http_server.h"
#include <sys/param.h>
#include "esp_http_server.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"

#define TAG "my_http_server"
static httpd_handle_t server = NULL;

// 根路径处理器 - 返回配置页面
static esp_err_t root_get_handler(httpd_req_t *req) {
    const char* html_page = 
        "<!DOCTYPE html>"
        "<html><head><meta charset='UTF-8'><title>Wi-Fi 配置</title>"
        "<meta name='viewport' content='width=device-width, initial-scale=1'>"
        "<style>body{font-family:Arial;margin:40px;background:#f5f5f5}"
        ".container{max-width:400px;margin:auto;background:white;padding:30px;border-radius:10px;box-shadow:0 2px 10px rgba(0,0,0,0.1)}"
        "input,button{width:100%;padding:12px;margin:8px 0;border:1px solid #ddd;border-radius:5px;box-sizing:border-box}"
        "button{background:#007bff;color:white;border:none;font-weight:bold;cursor:pointer}"
        "button:hover{background:#0056b3}</style></head>"
        "<body><div class='container'><h2>🔧 Wi-Fi 配置</h2>"
        "<form action='/connect' method='post'>"
        "<label>Wi-Fi 名称 (SSID):</label>"
        "<input type='text' name='ssid' required placeholder='输入 Wi-Fi 名称'>"
        "<label>密码:</label>"
        "<input type='password' name='password' placeholder='输入 Wi-Fi 密码'>"
        "<button type='submit'>连接网络</button>"
        "</form></div></body></html>";
    
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, html_page, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// URL解码函数
static void url_decode(char *dst, const char *src, size_t dst_size) {
    size_t i = 0, j = 0;
    while (src[i] && j < dst_size - 1) {
        if (src[i] == '%' && src[i+1] && src[i+2]) {
            int hex;
            sscanf(src + i + 1, "%2x", &hex);
            dst[j++] = (char)hex;
            i += 3;
        } else if (src[i] == '+') {
            dst[j++] = ' ';
            i++;
        } else {
            dst[j++] = src[i++];
        }
    }
    dst[j] = '\0';
}

// 连接处理器 - 接收表单数据
static esp_err_t connect_post_handler(httpd_req_t *req) {
    char buf[512] = {0};
    int ret = httpd_req_recv(req, buf, req->content_len);
    
    if (ret <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }
    buf[ret] = '\0';
    
    ESP_LOGI(TAG, "收到POST数据: %s", buf);
    
    char ssid[33] = {0};
    char password[65] = {0};
    char decoded_ssid[33] = {0};
    char decoded_pass[65] = {0};
    
    // 解析SSID
    char *ssid_start = strstr(buf, "ssid=");
    if (ssid_start) {
        ssid_start += 5;
        char *ssid_end = strchr(ssid_start, '&');
        if (ssid_end) {
            int len = ssid_end - ssid_start;
            if (len > 32) len = 32;
            strncpy(ssid, ssid_start, len);
        } else {
            strncpy(ssid, ssid_start, 32);
        }
        url_decode(decoded_ssid, ssid, sizeof(decoded_ssid));
    }
    
    // 解析密码
    char *pass_start = strstr(buf, "password=");
    if (pass_start) {
        pass_start += 9;
        char *pass_end = strchr(pass_start, '&');
        if (pass_end) {
            int len = pass_end - pass_start;
            if (len > 63) len = 63;
            strncpy(password, pass_start, len);
        } else {
            strncpy(password, pass_start, 63);
        }
        url_decode(decoded_pass, password, sizeof(decoded_pass));
    }
    
    ESP_LOGI(TAG, "解析到 SSID: %s", decoded_ssid);
    
    if (strlen(decoded_ssid) == 0) {
        const char* resp = "<html><body><h3>错误：SSID不能为空</h3>"
                          "<a href='/'>返回重新配置</a></body></html>";
        httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
        return ESP_OK;
    }
    
    // 保存到NVS
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (err == ESP_OK) {
        nvs_set_str(nvs_handle, "ssid", decoded_ssid);
        nvs_set_str(nvs_handle, "password", decoded_pass);
        nvs_commit(nvs_handle);
        nvs_close(nvs_handle);
        ESP_LOGI(TAG, "WiFi凭证已保存");
    } else {
        ESP_LOGE(TAG, "打开NVS失败: %s", esp_err_to_name(err));
    }
    
    // 返回响应
    const char* resp = "<!DOCTYPE html><html><head><meta charset='UTF-8'>"
                      "<title>配置成功</title></head>"
                      "<body><h3>✅ 配置已保存！</h3>"
                      "<p>设备正在连接网络并重启...</p>"
                      "<p>请重新连接到WiFi: <strong>%s</strong></p>"
                      "</body></html>";
    
    char resp_buf[512];
    snprintf(resp_buf, sizeof(resp_buf), resp, decoded_ssid);
    httpd_resp_send(req, resp_buf, HTTPD_RESP_USE_STRLEN);
    
    // 延迟后重启
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    esp_restart();
    
    return ESP_OK;
}

void start_webserver(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    
    config.server_port = 80;
    config.ctrl_port = 32768;
    config.max_open_sockets = 7;
    config.lru_purge_enable = true;
    config.stack_size = 8192;
    config.max_uri_handlers = 8;
    config.max_resp_headers = 8;
    
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t root = {
            .uri       = "/",
            .method    = HTTP_GET,
            .handler   = root_get_handler,
            .user_ctx  = NULL
        };
        
        httpd_uri_t connect = {
            .uri       = "/connect",
            .method    = HTTP_POST,
            .handler   = connect_post_handler,
            .user_ctx  = NULL
        };
        
        httpd_register_uri_handler(server, &root);
        httpd_register_uri_handler(server, &connect);
        
        ESP_LOGI(TAG, "HTTP服务器启动成功，端口：%d", config.server_port);
    } else {
        ESP_LOGE(TAG, "HTTP服务器启动失败");
    }
}

void erase_wifi_credentials(void) {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    
    if (err == ESP_OK) {
        nvs_erase_key(nvs_handle, "ssid");
        nvs_erase_key(nvs_handle, "password");
        nvs_commit(nvs_handle);
        nvs_close(nvs_handle);
        
        ESP_LOGI(TAG, "WiFi凭证已清除");
        
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        esp_restart();
    } else {
        ESP_LOGE(TAG, "打开NVS失败: %s", esp_err_to_name(err));
    }
}