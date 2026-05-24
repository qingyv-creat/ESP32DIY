#pragma once


#include <stdint.h>
#include "esp_err.h"

// MAC地址存储结构
typedef struct {
    uint8_t mac_addr[6];

} __attribute__((packed)) mac_storage_t;
//key的存储结构
typedef struct {
        uint8_t key_addr[16];
}__attribute__((packed)) key_storage_t;
//UUID存储结构
typedef struct {
        uint8_t uuid_addr[10];
}__attribute__((packed)) uuid_storage_t;
//SSN存储结构
typedef struct {
        uint8_t ssn_addr[6];
}__attribute__((packed)) ssn_storage_t;

// 在 storage.h 中添加以下定义
#define MAX_LOCK_COUNT 6
#define LOCK_DATA_PARTITION "lock_data"

// 锁存储结构
typedef struct {
    uint8_t lock_mac[6];

} __attribute__((packed)) lock_storage_t;
extern uint8_t lock1_flag;//锁绑定状态标志0x00表示未绑定，0x01表示已绑定
extern uint8_t lock2_flag;//锁绑定状态标志0x00表示未绑定，0x01表示已绑定
extern uint8_t lock3_flag;//锁绑定状态标志0x00表示未绑定，0x01表示已绑定
extern uint8_t lock4_flag;//锁绑定状态标志0x00表示未绑定，0x01表示已绑定
extern uint8_t lock5_flag;//锁绑定状态标志0x00表示未绑定，0x01表示已绑定
extern uint8_t lock6_flag;//锁绑定状态标志0x00表示未绑定，0x01表示已绑定
extern uint8_t lock_mac1[6];
extern uint8_t lock_mac2[6];
extern uint8_t lock_mac3[6];
extern uint8_t lock_mac4[6];
extern uint8_t lock_mac5[6];
extern uint8_t lock_mac6[6];









//MAC 函数声明
esp_err_t write_mac_to_flash(const uint8_t *mac);
esp_err_t read_mac_from_flash(uint8_t *mac);
esp_err_t get_custom_mac_address(uint8_t *mac_addr);

//KEY 函数声明
esp_err_t write_key_to_flash(const uint8_t *key);
esp_err_t read_key_from_flash(uint8_t *key);
esp_err_t get_custom_key_address(uint8_t *key);

//UUID 函数声明
esp_err_t write_uuid_to_flash(const uint8_t *uuid);
esp_err_t read_uuid_from_flash(uint8_t *uuid);
esp_err_t get_custom_uuid_address(uint8_t *uuid);

//SSN 函数声明
esp_err_t write_ssn_to_flash(const uint8_t *ssn);
esp_err_t read_ssn_from_flash(uint8_t *ssn);
esp_err_t get_custom_ssn_address(uint8_t *uuid);
//clear SSN key uuid
void erase_ssn_key_uuid_from_flash(void);

/************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************/
//锁端的MAC地址存储函数声明
uint8_t erase_lock_from_flash(uint8_t index);
esp_err_t write_lock_to_flash(const uint8_t *lock);
uint8_t read_lock_from_flash(uint8_t *lock, uint8_t index);//返回1表示读取成功这个地方有数据，0表示这个地方没有数据fff表示读取失败
void lock_init(void);
uint8_t verification_lock_mac(uint8_t *lock_mac);//验证锁的MAC地址是否有重复的，返回几表示有重复的在第几位，0表示没有重复的
void delete_all_lock(void);