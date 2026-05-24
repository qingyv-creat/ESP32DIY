#include "RFID.h"

#include <esp_check.h>
#include <string.h>
#include "nvs_flash.h"
#include "esp_system.h"
#include <stdint.h>
#include <string.h>
static const char *TAG = "rc522-basic-example";
static void judgment_card_type(const rc522_picc_t *picc);


#define RC522_I2C_ADDRESS      (0x28)
#define RC522_I2C_GPIO_SDA     (3)
#define RC522_I2C_GPIO_SCL     (0)
#define RC522_SCANNER_GPIO_RST (1) // soft-reset





static nvs_handle_t menvs_handle = 0;// NVS句柄
static bool nvs_initialized = false;// NVS是否初始化标志
static rc522_i2c_config_t driver_config = {
    .port = I2C_NUM_0,
    .device_address = RC522_I2C_ADDRESS,
    .rw_timeout_ms = 1000,
    .config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = RC522_I2C_GPIO_SDA,
        .scl_io_num = RC522_I2C_GPIO_SCL,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,// 100kHz
    },
    .rst_io_num = RC522_SCANNER_GPIO_RST,
};


static rc522_driver_handle_t driver;
static rc522_handle_t scanner;





void RFID_event_handler(void *arg, esp_event_base_t base, int32_t event_id, void *data){
    rc522_picc_state_changed_event_t *event = (rc522_picc_state_changed_event_t *)data;
    rc522_picc_t *picc = event->picc;

    if (picc->state == RC522_PICC_STATE_ACTIVE) {
        ESP_LOGI(TAG, "卡靠近了");
        
         rc522_picc_print(picc);




    }
    else if (picc->state == RC522_PICC_STATE_IDLE && event->old_state >= RC522_PICC_STATE_ACTIVE) {
        ESP_LOGI(TAG, "卡远离了");
    }
}


void  RFID_Init(){
    rc522_i2c_create(&driver_config, &driver);
    rc522_driver_install(driver);

    rc522_config_t scanner_config = {
        .driver = driver,
    };

    rc522_create(&scanner_config, &scanner);
    rc522_register_events(scanner, RC522_EVENT_PICC_STATE_CHANGED, RFID_event_handler, NULL);
    rc522_start(scanner);






}















