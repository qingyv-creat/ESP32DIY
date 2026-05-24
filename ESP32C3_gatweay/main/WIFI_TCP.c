#include "WIFI_TCP.h"
#include "BLUE_gatt.h"
#include "nvs_flash.h"  // NVS闪存初始化头文件
#include "nvs.h"  // NVS操作头文件
#include "esp_ota_ops.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "esp_system.h" // 用于esp_restart()
#include <inttypes.h>  // 提供PRIu32等宏

uint32_t duration = 20;
extern uint8_t key_user;//密钥为0表示使用公钥，1表示使用私钥
esp_ota_handle_t ota_handle = 0;// OTA操作句柄
esp_partition_t *update_partition = NULL;// OTA分区指针

uint8_t  general_data[128]={0};//通用数据数据包
uint8_t TCP_lock_data[128]={0};//网关下发的锁的数据包
uint8_t TCP_lock_data_len=0;//TCP数据包长度

uint8_t time_data[4]={0};
uint8_t tcp_buffer[OTA_SIZE_MAX]={0};

//新增一个心跳任务的句柄
TaskHandle_t heartbeat_task_handle = NULL;
//TCP任务解析的句柄
TaskHandle_t recive_task_handle = NULL;

//对于TCP数据的解析函数
static void tcp_data_parse(uint8_t *data, uint16_t len);



extern uint8_t mac_addr[6];



 


static const char *payload = "Message from ESP32";
static TaskHandle_t client_task_handle;
int sock = -1;
struct sockaddr_in dest_addr;
bool is_connected = false;
// 计算CRC-16 Modbus
unsigned short modbusCRC16(unsigned char *data, unsigned short length) {
    unsigned short crc = 0xFFFF;
    unsigned char i;
    for (unsigned short j = 0; j < length; j++) {
        crc ^= (unsigned short)data[j];
        for (i = 0; i < 8; i++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc = crc >> 1;
            }
        }
    }
    return crc;
}
 
 
/*********************************************************************
 * 函数名       crc16_modulebus
 *
 * 功能         crc校验
 *
 * 返回         CRC16
 *********************************************************************/
void crc16_modulebus(uint8_t *p,uint16_t len,uint8_t *crc_table)
{
    uint16_t  i,j;
    uint16_t  tmp,CRC16;

    CRC16=0xffff;

    for (i=0;i<len;i++)
    {
    CRC16=*p^CRC16;
    for (j=0;j< 8;j++)
    {
      tmp=CRC16 & 0x0001;
      CRC16 =CRC16 >>1;
      if (tmp)
      CRC16=CRC16 ^ 0xa001;
    }
        p++;
    }

    crc_table[1]=CRC16 & 0xff;
    crc_table[0]=CRC16 >>8& 0xff;
}


bool crc_check(uint8_t *normal,uint16_t nlen){
uint8_t crc_table[2];
crc16_modulebus(normal+3,nlen-9,crc_table);
if(crc_table[0]==normal[nlen-6]&&crc_table[1]==normal[nlen-5]){




    return true;
}
else{
    return false;
}

}



/**
 * 在字节数组中查找子数组
 * @param normal 主字节数组
 * @param nlen 主数组长度
 * @param find 要查找的子数组
 * @param flen 子数组长度
 * @return 找到的位置索引（从0开始），未找到返回0xFFFFFFFF
 */
unsigned int mystrstr(unsigned char *normal, unsigned int nlen, 
    unsigned char *find, unsigned int flen) {
// 参数检查
if (normal == NULL || find == NULL || nlen == 0 || flen == 0 || flen > nlen) {
return 0xFFFFFFFF;  // 无效参数
}

// 查找主循环
for (unsigned int i = 0; i <= nlen - flen; i++) {
// 比较normal从位置i开始的flen个字节是否与find相同
int match = 1;
for (unsigned int j = 0; j < flen; j++) {
if (normal[i + j] != find[j]) {
match = 0;
break;
}
}
if (match) {
return i;  // 找到匹配，返回起始位置
}
}

return 0xFFFFFFFF;  // 未找到
}



// 添加这个自定义打印函数
void print_hex_buffer(const char *tag, uint8_t *data, int len) {
    char *buffer = (char *)malloc(3 * len + 1); // 每个字节2个十六进制字符+1个空格
    if (buffer == NULL) return;
    
    char *p = buffer;
    for (int i = 0; i < len; i++) {
        p += sprintf(p, "%02x ", data[i]);
    }
    *p = '\0'; // 字符串结束符
    
    ESP_LOGI(tag, "%s", buffer);
    free(buffer);
}










//初始化TCP 协议

 void Tcp_Init()
 {
    
    inet_pton(AF_INET, HOST_IP_ADDR, &dest_addr.sin_addr);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(HOST_PORT);
    is_connected = false;

     
 }





extern uint8_t Gateway_sloft_banbeng[];
 void tcp_client(void *args)
{
sock = socket(AF_INET, SOCK_STREAM, 0);//创建套接字
if (sock < 0) {
ESP_LOGE(TAG, "创建套接字失败");
}else {
ESP_LOGI(TAG, "创建套接字成功");
}
int connect_ret = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
if(connect_ret != 0) {
ESP_LOGE(TAG, "连接服务器失败 errno=%d", errno);
is_connected = false;
}  else {
    
ESP_LOGI(TAG, "连接服务器成功");
is_connected = true;
uint8_t data[16]={0x0D,0x00,0x00,
0x32,0x36,0x2E,0x30,0x31,0x2e,0x32,0x33,//26.01.23,软件版本
0x2C,
0x32,0x36,0x2E,0x30,0x31,0x2e,0x31,0x34};//26.01.14硬件版本
memcpy(data+3,Gateway_sloft_banbeng,8);//软件版本




data[2]=user_flag;//绑定状态

uint16_t length=Gateway_Pack_data(data,sizeof(data),0x02,mac_addr,0,time_data,0x48);
Gateway_Send_data(general_data,length);

}
 
  
}


void TCP_Send_data(uint8_t *data,uint8_t len)
{
//要发送之前先判断是否连接成功
int err = send(sock, data, len, 0);
if (err < 0) {
ESP_LOGE(TAG, "发送数据失败");
} else {
ESP_LOGI(TAG, "发送数据成功");
}





}


// 修改TCP_recieve函数
void TCP_recieve() {
 if (!is_connected) {
        return;  // 未连接时直接返回
    }
       // 设置socket为非阻塞模式
       int flags = fcntl(sock, F_GETFL, 0);
       fcntl(sock, F_SETFL, flags | O_NONBLOCK);
   
    int len = recv(sock, tcp_buffer, sizeof(tcp_buffer) - 1, 0);//接受数据并根据数据长度判断情况
    
    if (len < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // 非阻塞模式下正常情况
            return;
        }
        ESP_LOGE(TAG, "接收数据失败, errno=%d", errno);
        // 连接可能已断开
        is_connected = false;
        close(sock);
        sock = -1;
    } else if (len == 0) {
        ESP_LOGI(TAG, "连接被服务器关闭");
        is_connected = false;
        close(sock);
        sock = -1;
    } else {

 //  ESP_LOG_BUFFER_HEX(TAG,tcp_buffer,len);//清除数据
   tcp_data_parse(tcp_buffer,len);//调用数据解析函数
   memset(tcp_buffer, 0, sizeof(tcp_buffer));//清除数据

    }
}



extern uint8_t lock_num;
void heartbeat_task(void *pvParameters){
    TickType_t xLastWakeTime;
    uint8_t i=0;
 
    const TickType_t xFrequency = pdMS_TO_TICKS(3* 60 * 1000); // 3分钟转换为tick数
   uint8_t data[2]={0x2a,0x01};


 
    xLastWakeTime = xTaskGetTickCount();
    
    while(1)
    {
        // 使用vTaskDelayUntil实现精确的3分钟周期
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        if(is_connected==true){
     
           uint16_t length=Gateway_Pack_data(data,2,0x02,mac_addr,0,time_data,0);
          Gateway_Send_data(general_data,length);
          // ESP_LOGI(TAG, "心跳包已发送");
          }
       
           



     
    }

}
extern bool is_wifi_connected ;
extern esp_ble_adv_params_t adv_params;
void recive_task(void *pvParameters){
    uint8_t i=0;
    while(1){
        if(is_wifi_connected )
        {
            TCP_recieve();//接受数据
        }
        


            if(key_user==1){
                i=1;
                esp_ble_gap_stop_advertising();//蓝牙停止广播
             

            }else if(key_user==0&&i==1){ 
                i=0;
                esp_ble_gap_start_advertising(&adv_params);//蓝牙开始广播
        
        
            }
        vTaskDelay(pdMS_TO_TICKS(10)); // 延时10毫秒，避免任务占用过多CPU时间
    }
}

uint8_t tcp_lock_task(void){

uint8_t data[8]={0x2b,0x03};

uint16_t length=0;
if(lock1_flag==1&&lock2_flag==1&&lock3_flag==1&&lock4_flag==1&&lock5_flag==1&&lock6_flag==1){
    ESP_LOGI(TAG, "无空位");
    data[1]=0x02;
    memcpy(data+2,self_lock_mac,6);
 
   
}
else  if(verification_lock_mac(self_lock_mac)!=0){
    ESP_LOGI(TAG, "已绑定");

    data[1]=0x01;
    memcpy(data+2,self_lock_mac,6);

   
}
else{ 
    data[1]=0x00;
    memcpy(data+2,self_lock_mac,6);
    write_lock_to_flash(self_lock_mac);
}




length=Gateway_Pack_data(data,sizeof(data),0x02,mac_addr,0,time_data,0);
Gateway_Send_data(general_data,length);
 
return 0;  
}



/*
解析TCPOTAs升级准备的函数
功能判断软件版本
参数data:接收到的数据
*/
void SoftHard_version(uint8_t *data){
    uint16_t i,len;
    uint32_t length;
    uint8_t new_version[3];
    uint8_t data_size[4];
    uint8_t data1[4]={0x27,0x00,0x10,0x00};//准备升级指令
   
    uint32_t version_num1=0x1a0118;//26.01.24版本号
    uint32_t version_num2;
memcpy(new_version,data+2,3);
memcpy(data_size,data+5,4);
ESP_LOG_BUFFER_HEX(TAG, new_version,3);
version_num2=new_version[1]+(new_version[1]<<8)+(new_version[0]<<16);
length = (data_size[0] << 24) | (data_size[1] << 16) | (data_size[2] << 8) | data_size[3];
//打印版本号和升级包大小

if(version_num2>version_num1&&length<0x190000){
    ESP_LOGI(TAG, "软件版本更新");
   data1[1]=0x00;
// 1. 找到要写入的 OTA 分区（未运行的那个 ota_x）
update_partition= esp_ota_get_next_update_partition(NULL);
// 2. 开始 OTA 会话
esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &ota_handle);

}else {
    ESP_LOGI(TAG, "软件版本不需要更新");
    data1[1]=0x01;

}




len=Gateway_Pack_data(data1,sizeof(data1),0x02,mac_addr,0,time_data,0);
Gateway_Send_data(general_data,len);








}






/*
Begin_tcp_ota函数
功能：开始TCP OTA升级
*/
void Begin_tcp_ota(uint8_t *data,uint16_t len){
    uint32_t ota_address=0;
    uint16_t ota_pack_len=1;
    uint8_t crc_data[2]={0};
   uint8_t data1[2]={0x28,0x00};//开始升级指令
    ota_address=data[21]|(data[20]<<8)|(data[19]<<16)|(data[18]<<24);
    ota_pack_len=data[23]+(data[22]<<8);
    uint16_t length;
crc16_modulebus(data+26,ota_pack_len,crc_data);
if(crc_data[0]==data[24]&&crc_data[1]==data[25]){
//crc校验通过准备进行ota升级
//07开始的

if(ota_pack_len==0){
    data1[1]=0x01;
    //如果接收的升级包长度为0，表示升级完成，重启设备
length=Gateway_Pack_data(data1,sizeof(data1),0x02,mac_addr,0,time_data,0);//回复服务器升级包接收成功
Gateway_Send_data(general_data,length);
vTaskDelay(pdMS_TO_TICKS(1000)); // 任务延时，避免占用过多CPU
esp_ota_end(ota_handle);
esp_ota_set_boot_partition(update_partition);
esp_restart();
}else if (ota_pack_len!=0){
 data1[1]=0x00;
 ESP_LOGI(TAG, "地址为：%"PRId32, ota_address);

esp_ota_write_with_offset(ota_handle, data+26, ota_pack_len, ota_address);
length=Gateway_Pack_data(data1,sizeof(data1),0x02,mac_addr,0,time_data,0);//回复服务器升级包接收成功
Gateway_Send_data(general_data,length);
}



}else {

    //crc校验失败，回复服务器升级包接收失败
    data1[1]=0x02;
    length=Gateway_Pack_data(data1,sizeof(data1),0x02,mac_addr,0,time_data,0);
    Gateway_Send_data(general_data,length);
}






} 



//写一个tcp传输数据解析的函数
void tcp_data_parse(uint8_t *data, uint16_t len){
    BaseType_t task_ret4;
unsigned char const key_user_data[8]={201,9,93,93,176,178,97,98};
uint8_t rxdata[128]={0};

if(len<9){
      return;
  }

if(data[0]!=0x5b||data[len-1]!=0x6e||data[len-2]!=0x72||data[len-3]!=0xb5){
    ESP_LOGI(TAG, "帧头或者帧尾错误");
    return;
}




bool ret=crc_check(data,len);
    if (ret==false)
    {
        ESP_LOGI(TAG, "crc校验失败");
        return;
}
if(data[3]==0x02){ 
if(data[16]!=TCP_OTA_BEGIN){
if(key_user==0x00){
    //使用公钥解密
    Ble_JieMa_t ble_param;
    ble_param.mode = 0x01 | 0x20; // 使用预设密钥表
    memcpy(ble_param.gattway_id, mac_addr, 6);
    Fun_Ble_JieMa(data+16, len-22, rxdata, ble_param);
 
}else{ 
  Ble_JieMa_t ble_param1;
  ble_param1.mode = 0x22; // 使用预设密钥表
  memcpy(ble_param1.gattway_id, mac_addr, 6);
  memcpy(ble_param1.key, user_key_data, 16);
  Fun_Ble_JieMa(data+16, len-22, rxdata, ble_param1);
}
switch(rxdata[0]){
case TCP_User_BINDS://服务器下发用户绑定指令
if(user_flag==0x01){
    ESP_LOGI(TAG, "用户已经绑定，无法重复绑定");
    uint8_t data[16]={0};

    data[0]=0x09;
    data[1]=0x01;
    memcpy(data+2,rxdata+12,6);
    memcpy(data+8,key_user_data,8);
    uint16_t length=Gateway_Pack_data(data,16,0x02,mac_addr,0,time_data,0);
    Gateway_Send_data(general_data,length);
    BLUE_SendNotifyData(flail_data,1);
    return;
}
//没有被绑定，进行绑定操作
else if(user_flag==0x00){
uint8_t data[16]={0};

data[0]=0x09;
data[1]=0x00;
memcpy(data+2,rxdata+12,6);
memcpy(data+8,key_user_data,8);
uint16_t length=Gateway_Pack_data(data,16,0x02,mac_addr,0,time_data,0);
Gateway_Send_data(general_data,length);
key_user=0x01;//使用私钥
user_flag=0x01;//设置为已绑定状态
memcpy(user_key_data,rxdata+18,8);
memcpy(user_key_data+8,key_user_data,8);

//
write_key_to_flash(user_key_data);//写入flash秘钥
read_key_from_flash(user_key_data);
memcpy(user_uuid_data,rxdata+2,10);

write_uuid_to_flash(user_uuid_data);
memcpy(user_ssn_data,rxdata+12,6);
write_ssn_to_flash(user_ssn_data);
BLUE_SendNotifyData(sucess_data,1);//蓝牙通知用户app绑定成功
}
    break;
case TCP_User_UNBINDS:
  
    key_user=0x00;//使用公钥
    user_flag=0x00;//设置为未绑定状态
    esp_wifi_restore();//恢复出厂设置，清除wifi配置
    erase_ssn_key_uuid_from_flash();
    delete_all_lock();
    read_data_from_flash();
    led_state_wifi=LED_WIFI_CLINK_ON;
    esp_restart();//重启设备
    break;

case TCP_Down_LOGIN:
break;
case TCP_LOCK_NEW:
break;

case TCP_DoorBell:

send_buutton_flag=0x01;
DoorBell();
//创建开锁任务
send_buutton_flag=1;
if(send_buutton_flag==1){ 
    task_ret4=xTaskCreate(open_button_task, "open_button_task", 3072, NULL, 2, &button_open_task_handle);}
break;


case TCP_LOCK_UNBIND:
memcpy(self_lock_mac,rxdata+2,6);  
uint8_t data[8]={0} ;
memcpy(data+2,self_lock_mac,6); 
uint8_t l=verification_lock_mac(self_lock_mac);//判断有没有这把锁
if(l==0){
    data[0]=TCP_LOCK_UNBIND;
    data[1]=0X01;//没有这把锁
 
}
else {

    data[0]=TCP_LOCK_UNBIND;
    data[1]=0X00;//成功
    erase_lock_from_flash(l);//删除这个位置的锁

}
uint16_t length=Gateway_Pack_data(data,sizeof(data),0x02,mac_addr,0,time_data,0);
Gateway_Send_data(general_data,length) ;
break;


case TCP_CLINE_LOCK:

memcpy(self_lock_mac,rxdata+2,6);
tcp_lock_task();

break;

case TCP_OTA_PREPARE:
    ESP_LOGI(TAG, "收到准备升级指令");
    SoftHard_version(rxdata);
    break;
case TCP_OTA_BEGIN:
    //舍弃因为这个ota升级包不经过加密锁一不要进行解密处理舍弃在这里判断改为在下面的if 语句中判断
  
    break;

default:
   
    ESP_LOGI(TAG, "收到未知指令%x",data[16]);
    break;

}}
else if(data[16]==TCP_OTA_BEGIN){
    Begin_tcp_ota(data,len);

}




}else {//门锁的数据包
memcpy(self_lock_mac,data+10,6);
TCP_lock_data_len=len;
memcpy(TCP_lock_data,data,len);
if(verification_lock_mac(self_lock_mac)==0){//没有这把锁
ESP_LOGI(TAG, "没有这把锁");
return;
}else if(verification_lock_mac(self_lock_mac)==1){
    //ESP_LOGI(TAG, "蓝牙扫描开始");
if (conn_device_a==false){
    ESP_LOGI(TAG, "蓝牙未连接");

    // 扫描持续时间20秒
   esp_ble_gap_start_scanning(duration); } // 开始扫描
else {
  
   send_data_to_lock( gl_profile_tab1[PROFILE_A_APP_ID].gattc_if, 
    gl_profile_tab1[PROFILE_A_APP_ID].conn_id,
    gl_profile_tab1[PROFILE_A_APP_ID].write_char_handle,
    TCP_lock_data, 
    TCP_lock_data_len);
}

}
else if(verification_lock_mac(self_lock_mac)==2) {
    if (conn_device_b==false){
        ESP_LOGI(TAG, "蓝牙2未连接");
    
        // 扫描持续时间20秒
       esp_ble_gap_start_scanning(duration); } // 开始扫描
    else{
       ESP_LOGI(TAG, "蓝牙2已连接");
       send_data_to_lock( gl_profile_tab1[PROFILE_B_APP_ID].gattc_if, 
        gl_profile_tab1[PROFILE_B_APP_ID].conn_id,
        gl_profile_tab1[PROFILE_B_APP_ID].write_char_handle,
        TCP_lock_data, 
        TCP_lock_data_len);
    }




}
else if(verification_lock_mac(self_lock_mac)==3) {
    if (conn_device_c==false){
        ESP_LOGI(TAG, "蓝牙未连接");
    
       // 扫描持续时间20秒
       esp_ble_gap_start_scanning(duration); } // 开始扫描
    else{
       ESP_LOGI(TAG, "蓝牙已连接");
       send_data_to_lock( gl_profile_tab1[PROFILE_C_APP_ID].gattc_if, 
        gl_profile_tab1[PROFILE_C_APP_ID].conn_id,
        gl_profile_tab1[PROFILE_C_APP_ID].write_char_handle,
        TCP_lock_data, 
        TCP_lock_data_len);
    }



}else if(verification_lock_mac(self_lock_mac)==4) {
    if (conn_device_d==false){
        ESP_LOGI(TAG, "蓝牙未连接");
    
        // 扫描持续时间20秒
       esp_ble_gap_start_scanning(duration); } // 开始扫描
    else{
       ESP_LOGI(TAG, "蓝牙已连接");
       send_data_to_lock( gl_profile_tab1[PROFILE_D_APP_ID].gattc_if, 
        gl_profile_tab1[PROFILE_D_APP_ID].conn_id,
        gl_profile_tab1[PROFILE_D_APP_ID].write_char_handle,
        TCP_lock_data, 
        TCP_lock_data_len);
    }


}else if(verification_lock_mac(self_lock_mac)==5) {
    if (conn_device_e==false){
        ESP_LOGI(TAG, "蓝牙未连接，需要扫描");
    
       // 扫描持续时间20秒
       esp_ble_gap_start_scanning(duration); } // 开始扫描
    else{
       ESP_LOGI(TAG, "蓝牙已连接");
       send_data_to_lock( gl_profile_tab1[PROFILE_E_APP_ID].gattc_if, 
        gl_profile_tab1[PROFILE_E_APP_ID].conn_id,
        gl_profile_tab1[PROFILE_E_APP_ID].write_char_handle,
        TCP_lock_data, 
        TCP_lock_data_len);
    }


}else if(verification_lock_mac(self_lock_mac)==6) {
    if (conn_device_f==false){
        ESP_LOGI(TAG, "蓝牙未连接");
    
        // 扫描持续时间20秒
       esp_ble_gap_start_scanning(duration); } // 开始扫描
    else{
       ESP_LOGI(TAG, "蓝牙已连接");
       send_data_to_lock( gl_profile_tab1[PROFILE_F_APP_ID].gattc_if, 
        gl_profile_tab1[PROFILE_F_APP_ID].conn_id,
        gl_profile_tab1[PROFILE_F_APP_ID].write_char_handle,
        TCP_lock_data, 
        TCP_lock_data_len);
    }


}















}
memset(rxdata, 0, sizeof(rxdata));
}

/*
用公用秘钥对私有数据包进行加密
data:私有数据包未加密
selfdata:私有数据包加密后的数据
leng:数据包长度
*/ 
void self_data(uint8_t *data,uint8_t*selfdata,uint8_t leng){


    if(key_user==0x00){    
    Ble_JieMa_t ble_param;
    ble_param.mode = 0x01 | 0x10; // 使用预设密钥表，并加密（0x01为密钥表，0x10为加密）
    memcpy(ble_param.gattway_id, mac_addr, 6);
    // 加密
    Fun_Ble_JieMa(data, leng, selfdata, ble_param);}
    else if(key_user==0x01){ 
    Ble_JieMa_t ble_param;
    ble_param.mode = 0x12; // 使用预设密钥表，并加密（0x01为密钥表，0x10为加密）
    memcpy(ble_param.gattway_id, mac_addr, 6);
    memcpy(ble_param.key, user_key_data, 16);//使用用户秘钥,
    // 加密
    Fun_Ble_JieMa(data, leng, selfdata, ble_param);

}



}
    





/*组合生成网关通用数据包
data:私有数据包
len:私有数据包长度
type:协议类型
Mac:mac地址
encryption_type：加密类型
ID:数据帧识别四字节ID
status:数据帧状态标识
*/
uint16_t Gateway_Pack_data(uint8_t *data,uint16_t len,uint8_t type,uint8_t Mac[6],uint8_t encryption_type,uint8_t *ID,uint8_t status){
uint16_t length=0;
general_data[length++]=type;
memcpy(general_data+length,Mac,6);
length+=6;
general_data[length++]=encryption_type;
memcpy(general_data+length,ID,4);
length+=4;
general_data[length++]=status;



self_data(data,general_data+length,len);
length+=len;




return length;
}




/*
把通用数据包组合成网关数据包并发送
data:通用数据包
len:通用数据包长度

*/


void Gateway_Send_data(uint8_t *data,uint16_t len){


 
 
 
    uint8_t data_send[128]={0};//要发送的数据包
    
    uint16_t length=len+3;//数据包的长度
    uint8_t crc_data[2]={0};//crc校验
    uint8_t i=0;
    data_send[i++]=0x5b;
    data_send[i++]=length >> 8& 0xff;//长度的高位
    data_send[i++]=length & 0xff;//长度的低位
    memcpy(data_send+i,general_data,len);
    i=i+len;
  
   
    crc16_modulebus(data,len,crc_data);

    data_send[i++]=crc_data[0];
    data_send[i++]=crc_data[1];
    data_send[i++]=0x02;
    data_send[i++]=0xb5;
    data_send[i++]=0x72;
    data_send[i++]=0x6e;

   // ESP_LOG_BUFFER_HEX(TAG, data_send,i);
//发送数据包
   // ESP_LOGI(TAG, "发送数据包");
    TCP_Send_data(data_send,i);
}

/*
更新门锁状态
无参数
*/
void lock_updata(){
lock_init();//跟新门锁信息  
uint8_t lock_data[128]={0};
uint8_t time[4]={0};
uint8_t len=0;
lock_data[len++]=0x3A;
lock_data[len++]=0x01;
lock_data[len++]=lock_num;

if(lock_num>0){
if(conn_device_a){
    memcpy(lock_data+len,lock_mac1,6);
    len+=6;
}
if(conn_device_b){
    memcpy(lock_data+len,lock_mac2,6);
    len+=6;
}
if(conn_device_c){
    memcpy(lock_data+len,lock_mac3,6);
    len+=6;
}
if(conn_device_d){
    memcpy(lock_data+len,lock_mac4,6);
    len+=6;
}
if(conn_device_e){
    memcpy(lock_data+len,lock_mac5,6);
    len+=6;
}
if(conn_device_f){
    memcpy(lock_data+len,lock_mac6,6);
    len+=6;
}}
uint16_t length=Gateway_Pack_data(lock_data,len,0x02,mac_addr,0,time,0);
Gateway_Send_data(general_data,length) ;
ESP_LOGI(TAG, "更新门锁状态");
ESP_LOG_BUFFER_HEX(TAG, lock_data,len);

len=0;
}
/*
gateway_init_updata网关长按之后初始化进行上报
无参数
返回值：无

*/


void gateway_init_updata(){


uint8_t data[2]={0x0e,0x00};
uint16_t length=Gateway_Pack_data(data,2,0x02,mac_addr,0,time,0);
Gateway_Send_data(general_data,length) ;


}




void open_lock_updata(){
    uint8_t data[2]={0x13,0x00};
    uint16_t length=Gateway_Pack_data(data,2,0x02,mac_addr,0,time,0);
    Gateway_Send_data(general_data,length) ;

}



