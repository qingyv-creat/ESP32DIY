#include "esp_log.h"
#include "esp_partition.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "string.h"
#include "storage.h"
#define TAG "STORAGE"

// 写入MAC地址到Flash
esp_err_t write_mac_to_flash(const uint8_t *mac) {
    esp_err_t ret;
    
    // 查找自定义分区
    const esp_partition_t *partition = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "mac_data");
    
    if (partition == NULL) {
        ESP_LOGE(TAG, "找不到mac_data分区");
        return ESP_ERR_NOT_FOUND;
    }
    
    // 准备数据
    mac_storage_t mac_data;
    memcpy(mac_data.mac_addr, mac, 6);

    
    // 擦除分区（需要先擦除再写入）
    ret = esp_partition_erase_range(partition, 0, partition->size);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "擦除分区失败: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // 写入数据
    ret = esp_partition_write(partition, 0, &mac_data, sizeof(mac_data));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "写入MAC地址失败: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "MAC地址写入成功");
   // print_mac_address(TAG, mac);
    
    return ESP_OK;
}
esp_err_t read_mac_from_flash(uint8_t *mac) {
    esp_err_t ret;
    
    // 查找自定义分区
    const esp_partition_t *partition = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "mac_data");
    
    if (partition == NULL) {
        ESP_LOGE(TAG, "找不到mac_data分区");
        return ESP_ERR_NOT_FOUND;
    }
    
    // 读取数据
    mac_storage_t mac_data;
    ret = esp_partition_read(partition, 0, &mac_data, sizeof(mac_data));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "读取MAC地址失败: %s", esp_err_to_name(ret));
        return ret;
    }
    

    // 检查MAC地址是否有效（非全0或全FF）
    int all_zero = 1, all_ff = 1;
    for (int i = 0; i < 6; i++) {
        if (mac_data.mac_addr[i] != 0x00) all_zero = 0;
        if (mac_data.mac_addr[i] != 0xFF) all_ff = 0;
    }
    
    if (all_zero || all_ff) {
        ESP_LOGE(TAG, "MAC地址无效");
        return ESP_ERR_INVALID_ARG;
    }
    
    memcpy(mac, mac_data.mac_addr, 6);
    ESP_LOGI(TAG, "MAC地址读取成功");
    //print_mac_address(TAG, mac);
   
    return ESP_OK;
}


esp_err_t write_key_to_flash(const uint8_t *key){
    esp_err_t ret;
    
    // 查找自定义分区
    const esp_partition_t *partition = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "key_data");
    
    if (partition == NULL) {
        ESP_LOGE(TAG, "key_data分区不存在");
        return ESP_ERR_NOT_FOUND;
    }
    
    // 准备数据
    key_storage_t key_data;
    memcpy(key_data.key_addr, key, 16);

    
    // 擦除分区（需要先擦除再写入）
    ret = esp_partition_erase_range(partition, 0, partition->size);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "擦除分区失败: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // 写入数据
    ret = esp_partition_write(partition, 0, &key_data, sizeof(key_data));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "写入KEY地址失败: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "KEY地址写入成功");
   // print_mac_address(TAG, mac);
    
    return ESP_OK;


}
esp_err_t read_key_from_flash(uint8_t *key){

    esp_err_t ret;
    
    // 查找自定义分区
    const esp_partition_t *partition = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "key_data");
    
    if (partition == NULL) {
        ESP_LOGE(TAG, "找不到key_data分区");
        return ESP_ERR_NOT_FOUND;
    }
    
    // 读取数据
    key_storage_t key_data;
    ret = esp_partition_read(partition, 0, &key_data, sizeof(key_data));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "读取KEY地址失败: %s", esp_err_to_name(ret));
        
        return ret;
    }
    

    // 检查MAC地址是否有效（非全0或全FF）
    int all_zero = 1, all_ff = 1;
    for (int i = 0; i <16; i++) {
        if (key_data.key_addr[i] != 0x00) all_zero = 0;
        if (key_data.key_addr[i] != 0xFF) all_ff = 0;
    }
    
    if (all_zero || all_ff) {
        ESP_LOGE(TAG, "key地址无效");
        return ESP_ERR_INVALID_ARG;
    }
    
    memcpy(key, key_data.key_addr, 16);
    ESP_LOGI(TAG, "key地址读取成功");
    //print_mac_address(TAG, mac);
    
    return ESP_OK;


}




esp_err_t write_uuid_to_flash(const uint8_t *uuid){

    esp_err_t ret;
    
    // 查找自定义分区
    const esp_partition_t *partition = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "uuid_data");
    
    if (partition == NULL) {
        ESP_LOGE(TAG, "找不到uuid_data分区");
        return ESP_ERR_NOT_FOUND;
    }
    
    // 准备数据
    uuid_storage_t uuid_data;
    memcpy(uuid_data.uuid_addr, uuid, 10);

    
    // 擦除分区（需要先擦除再写入）
    ret = esp_partition_erase_range(partition, 0, partition->size);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "擦除分区失败: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // 写入数据
    ret = esp_partition_write(partition, 0, &uuid_data, sizeof(uuid_data));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "写入uuid_data地址失败: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "uuid_data地址写入成功");
   // print_mac_address(TAG, mac);
    
    return ESP_OK;


}
esp_err_t read_uuid_from_flash(uint8_t *uuid){

    esp_err_t ret;
    
    // 查找自定义分区
    const esp_partition_t *partition = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "uuid_data");
    
    if (partition == NULL) {
        ESP_LOGE(TAG, "找不到uuid_data分区");
        return ESP_ERR_NOT_FOUND;
    }
    
    // 读取数据
    uuid_storage_t uuid_data;
    ret = esp_partition_read(partition, 0, &uuid_data, sizeof(uuid_data));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "读取uuid_data地址失败: %s", esp_err_to_name(ret));
        return ret;
    }
    

    // 检查MAC地址是否有效（非全0或全FF）
    int all_zero = 1, all_ff = 1;
    for (int i = 0; i < 10; i++) {
        if (uuid_data.uuid_addr[i] != 0x00) all_zero = 0;
        if (uuid_data.uuid_addr[i] != 0xFF) all_ff = 0;
    }
    
    if (all_zero || all_ff) {
        ESP_LOGE(TAG, "uuid_addr");
        return ESP_ERR_INVALID_ARG;
    }
    
    memcpy(uuid, uuid_data.uuid_addr, 10);
    ESP_LOGI(TAG, "uuid_addr");
    //print_mac_address(TAG, mac);
    
    return ESP_OK;


}
/////////////////////SSN///////////////

esp_err_t write_ssn_to_flash(const uint8_t *ssn){

    esp_err_t ret;
    
    // 查找自定义分区
    const esp_partition_t *partition = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "ssn_data");
    
    if (partition == NULL) {
        ESP_LOGE(TAG, "找不到ssn_data分区");
        return ESP_ERR_NOT_FOUND;
    }
    
    // 准备数据
    ssn_storage_t ssn_data;
    memcpy(ssn_data.ssn_addr, ssn, 6);

    
    // 擦除分区（需要先擦除再写入）
    ret = esp_partition_erase_range(partition, 0, partition->size);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "擦除分区失败: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // 写入数据
    ret = esp_partition_write(partition, 0, &ssn_data, sizeof(ssn_data));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "写入ssn_data地址失败: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "ssn_data地址写入成功");
   // print_mac_address(TAG, mac);
    
    return ESP_OK;


}
esp_err_t read_ssn_from_flash(uint8_t *ssn){

    esp_err_t ret;
    
    // 查找自定义分区
    const esp_partition_t *partition = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "ssn_data");
    
    if (partition == NULL) {
        ESP_LOGE(TAG, "找不到ssn_data分区");
        return ESP_ERR_NOT_FOUND;
    }
    
    // 读取数据
    ssn_storage_t ssn_data;
    ret = esp_partition_read(partition, 0, &ssn_data, sizeof(ssn_data));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "读取ssn_data地址失败: %s", esp_err_to_name(ret));
        return ret;
    }
    

    // 检查SSN地址是否有效（非全0或全FF）
    int all_zero = 1, all_ff = 1;
    for (int i = 0; i < 6; i++) {
        if (ssn_data.ssn_addr[i] != 0x00) all_zero = 0;
        if (ssn_data.ssn_addr[i] != 0xFF) all_ff = 0;
    }
    
    if (all_zero || all_ff) {
        ESP_LOGE(TAG, "ssn_data");
        return ESP_ERR_INVALID_ARG;
    }
    
    memcpy(ssn, ssn_data.ssn_addr, 6);
    ESP_LOGI(TAG, "ssn_data");
    //print_mac_address(TAG, mac);
    
    return ESP_OK;


}
//clear SSN key uuid
extern uint8_t user_uuid_data[10];
extern uint8_t user_key_data[16];//用户秘钥组合之后的用户秘钥
extern uint8_t user_ssn_data[6];
void erase_ssn_key_uuid_from_flash(void){
 // 查找自定义分区
 esp_err_t ret=ESP_OK;
 const esp_partition_t *partition1 = esp_partition_find_first(
    ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "ssn_data");

if (partition1 == NULL) {
    ESP_LOGE(TAG, "找不到ssn_data分区");
   
}

    // 查找自定义分区
const esp_partition_t *partition2 = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "uuid_data");
    
    if (partition2 == NULL) {
        ESP_LOGE(TAG, "找不到uuid_data分区");
        
    }
    // 查找自定义分区
    const esp_partition_t *partition3 = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "key_data");
    
    if (partition3 == NULL) {
        ESP_LOGE(TAG, "key_data分区不存在");
      
    }
    ///////
    ret = esp_partition_erase_range(partition1, 0, partition1->size);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "擦除ssn_data分区失败: %s", esp_err_to_name(ret));
     
    }    
    ret = esp_partition_erase_range(partition2, 0, partition2->size);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "擦除uuid_data分区失败: %s", esp_err_to_name(ret));
     
    } 
    ret = esp_partition_erase_range(partition3, 0, partition3->size);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "擦除key_data分区失败: %s", esp_err_to_name(ret));
       
    } 
memset(user_key_data, 0, 16);
memset(user_uuid_data, 0, 10);
memset(user_ssn_data, 0, 6);





}




/*****************************************************锁的******************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************/
uint8_t erase_lock_from_flash(uint8_t index){
    if (index < 1 || index > MAX_LOCK_COUNT) {
        ESP_LOGE(TAG, "索引名称无效: %d", index);
        return 0xff;
    }
    uint8_t data[6]={0};
    esp_err_t ret=ESP_OK;
    const char *partition_names[] = {"lock1_data", "lock2_data", "lock3_data", "lock4_data", "lock5_data", "lock6_data"};
    uint8_t *flags[] = {&lock1_flag, &lock2_flag, &lock3_flag, &lock4_flag, &lock5_flag, &lock6_flag};
    const esp_partition_t *partition = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, partition_names[index-1]);
switch (index)
{
    case 1:
    /* code */
   
    *flags[index-1]=0x00;
    ret = esp_partition_erase_range(partition, 0, partition->size);
    memcpy(lock_mac1,data, 6);
    break;
    case 2:
    /* code */
    *flags[index-1]=0x00;
    ret = esp_partition_erase_range(partition, 0, partition->size);
    memcpy(lock_mac2,data, 6);
    break;
    case 3:
    /* code */
    *flags[index-1]=0x00;
    ret = esp_partition_erase_range(partition, 0, partition->size);
    memcpy(lock_mac3,data, 6);
    break;
    case 4:
    /* code */
    *flags[index-1]=0x00;
    ret = esp_partition_erase_range(partition, 0, partition->size);
    memcpy(lock_mac4,data, 6);
    break;
    case 5:
    /* code */
    *flags[index-1]=0x00;
    ret = esp_partition_erase_range(partition, 0, partition->size);
    memcpy(lock_mac5,data, 6);
    break;
    case 6:
    /* code */
    *flags[index-1]=0x00;
    ret = esp_partition_erase_range(partition, 0, partition->size);
    memcpy(lock_mac6,data, 6);
    break;

default:
    break;
}





return 0x00;
}
extern uint8_t connect_flag ;
esp_err_t write_lock_to_flash(const uint8_t *lock) {
    esp_err_t ret = ESP_OK;
    esp_err_t overall_status = ESP_OK; // 记录整体状态
    lock_init();
    // 准备数据
    lock_storage_t lock_data;
    memcpy(lock_data.lock_mac, lock, 6);

    // 定义分区名称数组
    const char *partition_names[] = {"lock1_data", "lock2_data", "lock3_data", "lock4_data", "lock5_data", "lock6_data"};
    uint8_t *flags[] = {&lock1_flag, &lock2_flag, &lock3_flag, &lock4_flag, &lock5_flag, &lock6_flag};

    // 遍历所有分区
    for (int i = 0; i < 6; i++) {
        if (*flags[i] == 0x00) { // 如果该分区未被写入
            const esp_partition_t *partition = esp_partition_find_first(
                ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, partition_names[i]);

            if (partition == NULL) {
             
                overall_status = ESP_ERR_NOT_FOUND; // 记录错误状态
                continue; // 跳过当前分区，继续处理下一个
            }




            // 擦除分区
            ret = esp_partition_erase_range(partition, 0, partition->size);
            if (ret != ESP_OK) {
               
                overall_status = ret; // 记录错误状态
                continue; // 跳过当前分区，继续处理下一个

            }

            // 写入数据
            ret = esp_partition_write(partition, 0, &lock_data, sizeof(lock_data));
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "写入 %s 地址失败: %s", partition_names[i], esp_err_to_name(ret));
                overall_status = ret; // 记录错误状态
                continue; // 跳过当前分区，继续处理下一个
            }else {
                // 写入成功
            ESP_LOGI(TAG, "%s 地址写入成功", partition_names[i]);
            *flags[i] = 0x01; // 标记为已写入把这个标志设置为1
            connect_flag=i+1;
        //打印这个标志位
       
           
            return ESP_OK; // 成功写入后立即返回
            }

           
        }
    }
    lock_init();
    // 返回整体状态
    return overall_status;
}

uint8_t read_lock_from_flash(uint8_t *lock, uint8_t index){
    const char *partition_names[] = {"lock1_data", "lock2_data", "lock3_data", "lock4_data", "lock5_data", "lock6_data"};
if (index < 1 || index > MAX_LOCK_COUNT) {
     //   ESP_LOGE(TAG, "索引名称无效: %d", index);
        return 0xff;
    }
if (index == 1) {
            // 查找自定义分区
    const esp_partition_t *partition = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, partition_names[index-1]);
        if (partition == NULL) {
         //   ESP_LOGE(TAG, "找不到 %s 分区", partition_names[index-1]);
            return 0x00;
        }
        
        // 读取数据
        lock_storage_t lock_data;
        esp_err_t ret = esp_partition_read(partition, 0, &lock_data, sizeof(lock_data));
        if (ret != ESP_OK) {
          //  ESP_LOGE(TAG, "读取 %s 地址失败: %s", partition_names[index-1], esp_err_to_name(ret));
            return 0x00;
        }
        
        // 检查MAC地址是否有效（非全0或全FF）
        int all_zero = 1, all_ff = 1;
        for (int i = 0; i < 6; i++) {
            if (lock_data.lock_mac[i] != 0x00) all_zero = 0;
            if (lock_data.lock_mac[i] != 0xFF) all_ff = 0;
        }
        
        if (all_zero || all_ff) {
         //   ESP_LOGE(TAG, "%s 地址无效", partition_names[index-1]);
            return 0x00;
        }
        
        memcpy(lock, lock_data.lock_mac, 6);
        //ESP_LOGI(TAG, "%s 地址读取成功", partition_names[index-1]);
        
        return 0x01; // 返回
    
 
 
 
    } 
else if(index == 2){
        const esp_partition_t *partition = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, partition_names[index-1]);
        if (partition == NULL) {
           // ESP_LOGE(TAG, "找不到 %s 分区", partition_names[index-1]);
            return 0x00;
        }
        
        // 读取数据
        lock_storage_t lock_data;
        esp_err_t ret = esp_partition_read(partition, 0, &lock_data, sizeof(lock_data));
        if (ret != ESP_OK) {
         //   ESP_LOGE(TAG, "读取 %s 地址失败: %s", partition_names[index-1], esp_err_to_name(ret));
            return 0x00;
        }
        
        // 检查MAC地址是否有效（非全0或全FF）
        int all_zero = 1, all_ff = 1;
        for (int i = 0; i < 6; i++) {
            if (lock_data.lock_mac[i] != 0x00) all_zero = 0;
            if (lock_data.lock_mac[i] != 0xFF) all_ff = 0;
        }
        
        if (all_zero || all_ff) {
        //    ESP_LOGE(TAG, "%s 地址无效", partition_names[index-1]);
            return 0x00;
        }
        
        memcpy(lock, lock_data.lock_mac, 6);
    //    ESP_LOGI(TAG, "%s 地址读取成功", partition_names[index-1]);
        
        return 0x01; // 返回
     
  
  
    }
else if(index == 3){
        const esp_partition_t *partition = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, partition_names[index-1]);
        if (partition == NULL) {
            ESP_LOGE(TAG, "找不到 %s 分区", partition_names[index-1]);
            return 0x00;
        }
        
        // 读取数据
        lock_storage_t lock_data;
        esp_err_t ret = esp_partition_read(partition, 0, &lock_data, sizeof(lock_data));
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "读取 %s 地址失败: %s", partition_names[index-1], esp_err_to_name(ret));
            return 0x00;
        }
        
        // 检查MAC地址是否有效（非全0或全FF）
        int all_zero = 1, all_ff = 1;
        for (int i = 0; i < 6; i++) {
            if (lock_data.lock_mac[i] != 0x00) all_zero = 0;
            if (lock_data.lock_mac[i] != 0xFF) all_ff = 0;
        }
        
        if (all_zero || all_ff) {
            ESP_LOGE(TAG, "%s 地址无效", partition_names[index-1]);
            return 0x00;
        }
        
        memcpy(lock, lock_data.lock_mac, 6);
        ESP_LOGI(TAG, "%s 地址读取成功", partition_names[index-1]);
        
        return 0x01; // 返回
    
 
    }
else if(index == 4){
        const esp_partition_t *partition = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, partition_names[index-1]);
        if (partition == NULL) {
            ESP_LOGE(TAG, "找不到 %s 分区", partition_names[index-1]);
            return 0x00;
        }
        
        // 读取数据
        lock_storage_t lock_data;
        esp_err_t ret = esp_partition_read(partition, 0, &lock_data, sizeof(lock_data));
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "读取 %s 地址失败: %s", partition_names[index-1], esp_err_to_name(ret));
            return 0x00;
        }
        
        // 检查MAC地址是否有效（非全0或全FF）
        int all_zero = 1, all_ff = 1;
        for (int i = 0; i < 6; i++) {
            if (lock_data.lock_mac[i] != 0x00) all_zero = 0;
            if (lock_data.lock_mac[i] != 0xFF) all_ff = 0;
        }
        
        if (all_zero || all_ff) {
            ESP_LOGE(TAG, "%s 地址无效", partition_names[index-1]);
            return 0x00;
        }
        
        memcpy(lock, lock_data.lock_mac, 6);
        ESP_LOGI(TAG, "%s 地址读取成功", partition_names[index-1]);
        
        return 0x01; // 返回
    
 
    }
else if(index == 5){
        const esp_partition_t *partition = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, partition_names[index-1]);
  
        if (partition == NULL) {
            ESP_LOGE(TAG, "找不到 %s 分区", partition_names[index-1]);
            return 0x00;
        }
        
        // 读取数据
        lock_storage_t lock_data;
        esp_err_t ret = esp_partition_read(partition, 0, &lock_data, sizeof(lock_data));
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "读取 %s 地址失败: %s", partition_names[index-1], esp_err_to_name(ret));
            return 0x00;
        }
        
        // 检查MAC地址是否有效（非全0或全FF）
        int all_zero = 1, all_ff = 1;
        for (int i = 0; i < 6; i++) {
            if (lock_data.lock_mac[i] != 0x00) all_zero = 0;
            if (lock_data.lock_mac[i] != 0xFF) all_ff = 0;
        }
        
        if (all_zero || all_ff) {
            ESP_LOGE(TAG, "%s 地址无效", partition_names[index-1]);
            return 0x00;
        }
        
        memcpy(lock, lock_data.lock_mac, 6);
        ESP_LOGI(TAG, "%s 地址读取成功", partition_names[index-1]);
        
        return 0x01; // 返回
    
  
    }
else if(index == 6){
        const esp_partition_t *partition = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, partition_names[index-1]);
        if (partition == NULL) {
            ESP_LOGE(TAG, "找不到 %s 分区", partition_names[index-1]);
            return 0x00;
        }
        
        // 读取数据
        lock_storage_t lock_data;
        esp_err_t ret = esp_partition_read(partition, 0, &lock_data, sizeof(lock_data));
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "读取 %s 地址失败: %s", partition_names[index-1], esp_err_to_name(ret));
            return 0x00;
        }
        
        // 检查MAC地址是否有效（非全0或全FF）
        int all_zero = 1, all_ff = 1;
        for (int i = 0; i < 6; i++) {
            if (lock_data.lock_mac[i] != 0x00) all_zero = 0;
            if (lock_data.lock_mac[i] != 0xFF) all_ff = 0;
        }
        
        if (all_zero || all_ff) {
            ESP_LOGE(TAG, "%s 地址无效", partition_names[index-1]);
            return 0x00;
        }
        
        memcpy(lock, lock_data.lock_mac, 6);
        ESP_LOGI(TAG, "%s 地址读取成功", partition_names[index-1]);
        
        return 0x01; // 返回
    
  
    }

   return 0x00; // 默认返回0表示没有数据

}
 void lock_init(){
uint8_t flag=0x00;

for(uint8_t index=1;index<7;index++){
switch (index){
    case 1:
        flag=read_lock_from_flash(lock_mac1,index);
        if(flag==0x01){
            lock1_flag=0x01;
        }
        break;
    case 2:
        flag=read_lock_from_flash(lock_mac2,index);
        if(flag==0x01){
            lock2_flag=0x01;
        }
        break;
    case 3:
        flag=read_lock_from_flash(lock_mac3,index);
        if(flag==0x01){
            lock3_flag=0x01;
        }
        break;
    case 4:
        flag=read_lock_from_flash(lock_mac4,index);
        if(flag==0x01){
            lock4_flag=0x01;
        }
        break;
    case 5:
        flag=read_lock_from_flash(lock_mac5,index);
        if(flag==0x01){
            lock5_flag=0x01;
        }
        break;
    case 6:
        flag=read_lock_from_flash(lock_mac6,index);
        if(flag==0x01){
            lock6_flag=0x01;
        }
        break;
    default:
        break;

}

 }}


 uint8_t verification_lock_mac(uint8_t *lock_mac){

    lock_init();
for(uint8_t index=1;index<7;index++){
switch (index) { 
    case 1:
        if(memcmp(lock_mac,lock_mac1,6)==0){
            return 0x01;
        }
    break;
    case 2:
        if(memcmp(lock_mac,lock_mac2,6)==0){
            return 0x02;
        }
        break;
    case 3:
        if(memcmp(lock_mac,lock_mac3,6)==0){
            return 0x03;
        }
        break;
    case 4:
        if(memcmp(lock_mac,lock_mac4,6)==0){
            return 0x04;
        }
        break;
    case 5:
        if(memcmp(lock_mac,lock_mac5,6)==0){
            return 0x05;
        }
        break;
    case 6:
        if(memcmp(lock_mac,lock_mac6,6)==0){
            return 0x06;
        }
        break;
    default:
        break;
}

}
return 0x00;
 }

 void delete_all_lock(void){
  
            erase_lock_from_flash(1);
            erase_lock_from_flash(2);
            erase_lock_from_flash(3);
            erase_lock_from_flash(4);
            erase_lock_from_flash(5);
            erase_lock_from_flash(6);
        
 }