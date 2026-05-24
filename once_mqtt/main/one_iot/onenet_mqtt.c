#include "onenet_mqtt.h"
#define ONENET_MQTT_DECIVEID "y0l8Aq8c9m"
#define ONENET_MQTT_USERNAME "led001"
#define ONENET_MQTT_PASSWORD "G4l/Qdn/7SlpdeltH0NzJYOr2Z1j3+mFKwXx4c3ht48="
//token有效时间（2030年1月1日）
#define TM_EXPIRE_TIME 1924833600
#define TAG "one_mqtt"
static esp_mqtt_client_handle_t one_mqtt_client=NULL;


static void onenet_mqtt_event_handler(void* event_handler_arg,
                                        esp_event_base_t event_base,
                                        int32_t event_id,
                                        void* event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:  //连接成功
            ESP_LOGI(TAG, "连接成功");
           
            break;
        case MQTT_EVENT_DISCONNECTED:   //连接断开
           ESP_LOGI(TAG, "连接断开");
           
            break;

        case MQTT_EVENT_SUBSCRIBED:     //收到订阅消息ACK
          ESP_LOGI(TAG, "收到订阅消息ACK");
            break;
        case MQTT_EVENT_UNSUBSCRIBED:   //收到解订阅消息ACK
           ESP_LOGI(TAG, "收到解订阅消息ACK");
            break;
        case MQTT_EVENT_PUBLISHED:      //收到发布消息ACK
           ESP_LOGI(TAG, "收到发布消息ACK");

            break;
        case MQTT_EVENT_DATA:
           
            break;
        case MQTT_EVENT_ERROR:
          

            break;
        default:
            break;
    }
}


















void one_mqtt_init(void){

esp_mqtt_client_config_t one_mqtt_sconfig;
memset(&one_mqtt_sconfig, 0, sizeof(one_mqtt_sconfig));//清零
one_mqtt_sconfig.broker.address.uri= "mqtt://mqtts.heclouds.com";//mqtt服务器地址
one_mqtt_sconfig.broker.address.port= 1883;//mqtt服务器端口

one_mqtt_sconfig.credentials.username=ONENET_MQTT_DECIVEID;//设备名称
one_mqtt_sconfig.credentials.client_id=ONENET_MQTT_USERNAME;//设备

static char token[256];//设备接入密钥
dev_token_generate(token, SIG_METHOD_SHA256, TM_EXPIRE_TIME, ONENET_MQTT_DECIVEID,
NULL, ONENET_MQTT_PASSWORD);
one_mqtt_sconfig.credentials.authentication.password=token;//密钥
//将鉴权信息打印出来
ESP_LOGI(TAG,"onenet connect->clientId:%s,username:%s,password:%s",
one_mqtt_sconfig.credentials.client_id,one_mqtt_sconfig.credentials.username,
one_mqtt_sconfig.credentials.authentication.password);
one_mqtt_client=esp_mqtt_client_init(&one_mqtt_sconfig);//注册mqtt客户端句柄
//注册mqtt事件处理函数
esp_mqtt_client_register_event(one_mqtt_client,//one_mqtt_client是mqtt客户端句柄
     ESP_EVENT_ANY_ID, //监听所有事件
     onenet_mqtt_event_handler, //事件处理函数
     NULL);//事件处理函数的参数



}

void one_mqtt_start(void){
one_mqtt_init();//初始化mqtt客户端
esp_mqtt_client_start(one_mqtt_client);//启动mqtt客户端

}
