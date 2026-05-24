你好这是一个蓝牙网关项目主控是ESP32C3
实现了下列功能
1.蓝牙配网具体的实现在：BLUE_GATT.c下面static void gatts_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) 函数中ESP_GATTS_WRITE_EVT事件中处理

2.蓝牙透传：我的蓝牙透传主要分为两部分：分别是服务器下发数据给蓝牙和蓝牙主动上报主要实现的代码具体在tcp数据解析里void tcp_data_parse(uint8_t *data, uint16_t len)这个函数末尾和在gatt客户端函数里面的    case ESP_GATTC_NOTIFY_EVT:  // 通知事件下面处理

3.RFID识别：RFID识别主要实现在rfid_parse.c里面主要就是下载rc552模块的代码不做介绍
4.WIFI的TCP协议解析：tcp_data_parse(uint8_t *data, uint16_t len)这个函数进行处理
项目架构基于ESP_IDFV5.3.3
硬件原理图不开放
