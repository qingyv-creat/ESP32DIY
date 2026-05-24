#ifndef BLUE_INIT_H
#define BLUE_INIT_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化蓝牙子系统
 * @return esp_err_t 初始化结果
 */
esp_err_t BLUE_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* BLUE_INIT_H */