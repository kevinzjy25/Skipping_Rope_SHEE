#ifndef ICM42688P_H
#define ICM42688P_H

#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <math.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

// -------------------- 配置宏 --------------------
#define ICM42688_I2C_PORT    I2C_NUM_0   // 默认 I2C 端口
#define ICM42688_ADDRESS     0x68        // 7位 I2C 地址
#define ICM42688_WHO_AM_I    0x75
#define ICM42688_WHOAMI_VAL  0x47

// -------------------- 数据结构 --------------------
typedef struct {
    float roll;   // 横滚角
    float pitch;  // 俯仰角
    float yaw;    // 航向角
} Attitude_t;

// -------------------- API 函数 --------------------

/**
 * @brief 初始化 ICM42688（包括 I2C 初始化、寄存器配置、陀螺标定）
 * 
 * @param port      I2C 端口号 (如 I2C_NUM_0 / I2C_NUM_1)
 * @param sda_gpio  SDA 引脚号
 * @param scl_gpio  SCL 引脚号
 * @return esp_err_t 
 *         - ESP_OK: 成功
 *         - ESP_FAIL: WHO_AM_I 检查失败
 */
esp_err_t ICM42688_Init(i2c_port_t port, int sda_gpio, int scl_gpio);

/**
 * @brief 获取原始加速度数据
 * @param ax 指针，存储 X 轴加速度
 * @param ay 指针，存储 Y 轴加速度
 * @param az 指针，存储 Z 轴加速度
 */
void ICM42688_getacc(int16_t *ax, int16_t *ay, int16_t *az);

/**
 * @brief 获取原始陀螺仪数据
 * @param gx 指针，存储 X 轴角速度
 * @param gy 指针，存储 Y 轴角速度
 * @param gz 指针，存储 Z 轴角速度
 */
void ICM42688_getgyro(int16_t *gx, int16_t *gy, int16_t *gz);

/**
 * @brief 使用互补滤波更新姿态
 * @param att 指针，返回姿态角（roll, pitch, yaw）
 */
void ICM42688_AttitudeUpdate(Attitude_t *att);

#ifdef __cplusplus
}
#endif

#endif // ICM42688P_H
