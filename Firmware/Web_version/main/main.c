/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_flash.h"
#include "esp_timer.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_smartconfig.h"
#include "esp_netif.h"
#include "driver/gpio.h"
#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "my_ui.h"
#include "nvs_flash.h"
#include "icm42688p/include/icm42688p.h"
#include "esp_task_wdt.h"

extern void initialise_wifi(void);

static const char *TAG = "MAIN";
int level = 0; // menu level
float deg_roll;
float deg_pitch;
float deg_yaw;
int blt_sw_state = 0;
int wifi_sw_state = 0;
bool icm42688p_up = false;
bool icm42688p_down = false;
// WiFi相关全局变量
bool wifi_initialized = false;
bool wifi_connecting = false;
// GPIO 引脚定义
#define HALL1 5
#define HALL2 4
#define HALL3 8
#define BTN 20
#define MTR 9

// 跳绳计数
static volatile int jump_count = 0;

// 状态机 - 扩展为5个状态
typedef enum
{
    IDLE = 0,
    DETECTED_HALL1, // 检测到HALL1触发
    DETECTED_HALL2, // 检测到HALL2触发
    DETECTED_HALL3, // 检测到HALL3触发
    JUMP_COMPLETE   // 完整跳绳动作完成
} skp_state_t;

// 跳绳方向
typedef enum
{
    DIRECTION_NONE = 0,
    DIRECTION_FORWARD, // hall1 -> hall2 -> hall3
    DIRECTION_BACKWARD // hall3 -> hall2 -> hall1
} jump_direction_t;

// 全局 IMU 数据
#define MAX_Y_ANGLE 30.0f
#define MAX_INTERVAL_MS 1000
Attitude_t g_att; // app_main 更新

// LVGL 时钟
static void lv_tick_task(void *arg)
{
    (void)arg;
    lv_tick_inc(10);
}

// GPIO 配置
void pin_setup(void)
{
    gpio_config_t hall_config = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << HALL1) | (1ULL << HALL2) | (1ULL << HALL3),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE};
    gpio_config(&hall_config);
    gpio_config_t mtr_config = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << MTR),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE};
    gpio_config(&mtr_config);
    gpio_config_t btn_config = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << BTN),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE};
    gpio_config(&btn_config);
}

void skp_cnt_task(void *arg)
{
    skp_state_t state = IDLE;
    jump_direction_t direction = DIRECTION_NONE;

    bool counting_started = false;
    TickType_t last_jump_tick = 0;

    // 防抖变量
    int last_hall1_state = 1;
    int last_hall2_state = 1;
    int last_hall3_state = 1;
    TickType_t state_change_time = 0;
    TickType_t last_detection_time = 0;

    ESP_LOGI("SKIP2", "跳绳计数任务启动");

    // 注册进 WDT
    ESP_ERROR_CHECK(esp_task_wdt_add(NULL));

    while (1)
    {
        // 读取当前GPIO状态
        int hall1_state = gpio_get_level(HALL1);
        int hall2_state = gpio_get_level(HALL2);
        int hall3_state = gpio_get_level(HALL3);

        // 检测下降沿触发
        bool hall1_triggered = (last_hall1_state == 1 && hall1_state == 0);
        bool hall2_triggered = (last_hall2_state == 1 && hall2_state == 0);
        bool hall3_triggered = (last_hall3_state == 1 && hall3_state == 0);

        TickType_t current_time = xTaskGetTickCount();

        switch (state)
        {
        case IDLE:
            // 检测起始传感器：HALL1 或 HALL3
            if (hall1_triggered)
            {
                state = DETECTED_HALL1;
                direction = DIRECTION_FORWARD;
                state_change_time = current_time;
                last_detection_time = current_time;
                ESP_LOGD("SKIP2", "检测到HALL1触发，开始正向检测");
            }
            else if (hall3_triggered)
            {
                state = DETECTED_HALL3;
                direction = DIRECTION_BACKWARD;
                state_change_time = current_time;
                last_detection_time = current_time;
                ESP_LOGD("SKIP2", "检测到HALL3触发，开始反向检测");
            }
            break;

        case DETECTED_HALL1:
            // 正向序列：期待HALL2触发
            if (hall2_triggered && (current_time - last_detection_time) > pdMS_TO_TICKS(10))
            {
                state = DETECTED_HALL2;
                last_detection_time = current_time;
                ESP_LOGD("SKIP2", "正向序列：HALL1->HALL2");
            }
            // 检测到错误序列或超时，重置
            else if (hall1_triggered || hall3_triggered ||
                     (current_time - state_change_time) > pdMS_TO_TICKS(500))
            {
                ESP_LOGD("SKIP2", "正向序列异常，重置到IDLE");
                state = IDLE;
                direction = DIRECTION_NONE;
            }
            break;

        case DETECTED_HALL3:
            // 反向序列：期待HALL2触发
            if (hall2_triggered && (current_time - last_detection_time) > pdMS_TO_TICKS(10))
            {
                state = DETECTED_HALL2;
                last_detection_time = current_time;
                ESP_LOGD("SKIP2", "反向序列：HALL3->HALL2");
            }
            // 检测到错误序列或超时，重置
            else if (hall1_triggered || hall3_triggered ||
                     (current_time - state_change_time) > pdMS_TO_TICKS(500))
            {
                ESP_LOGD("SKIP2", "反向序列异常，重置到IDLE");
                state = IDLE;
                direction = DIRECTION_NONE;
            }
            break;

        case DETECTED_HALL2:
            if (direction == DIRECTION_FORWARD)
            {
                // 正向序列：期待HALL3触发
                if (hall3_triggered && (current_time - last_detection_time) > pdMS_TO_TICKS(10))
                {
                    state = JUMP_COMPLETE;
                    ESP_LOGD("SKIP2", "正向序列完成：HALL1->HALL2->HALL3");
                }
                // 检测到错误序列或超时
                else if (hall1_triggered || hall2_triggered ||
                         (current_time - state_change_time) > pdMS_TO_TICKS(500))
                {
                    ESP_LOGD("SKIP2", "正向序列HALL2后异常，重置到IDLE");
                    state = IDLE;
                    direction = DIRECTION_NONE;
                }
            }
            else if (direction == DIRECTION_BACKWARD)
            {
                // 反向序列：期待HALL1触发
                if (hall1_triggered && (current_time - last_detection_time) > pdMS_TO_TICKS(10))
                {
                    state = JUMP_COMPLETE;
                    ESP_LOGD("SKIP2", "反向序列完成：HALL3->HALL2->HALL1");
                }
                // 检测到错误序列或超时
                else if (hall2_triggered || hall3_triggered ||
                         (current_time - state_change_time) > pdMS_TO_TICKS(500))
                {
                    ESP_LOGD("SKIP2", "反向序列HALL2后异常，重置到IDLE");
                    state = IDLE;
                    direction = DIRECTION_NONE;
                }
            }
            break;

        case JUMP_COMPLETE:
            jump_count++;
            last_jump_tick = current_time;

            if (!counting_started)
            {
                counting_started = true;
                ESP_LOGI("SKIP2", "开始计数");
            }

            const char *dir_str = (direction == DIRECTION_FORWARD) ? "正向" : "反向";
            ESP_LOGI("SKIP2", "检测到%s跳绳，本次总数: %d", dir_str, jump_count);
            show_number(lbl_cnt, jump_count);
            // 重置状态，防止重复计数
            state = IDLE;
            direction = DIRECTION_NONE;
            vTaskDelay(pdMS_TO_TICKS(50));
            break;
        }

        // 检查是否超时（1秒没跳）
        if (counting_started && (current_time - last_jump_tick) > pdMS_TO_TICKS(MAX_INTERVAL_MS))
        {
            ESP_LOGI("SKIP2", "计数完成，本次跳绳总数: %d", jump_count);

            counting_started = false;
        }

        // 更新上次状态
        last_hall1_state = hall1_state;
        last_hall2_state = hall2_state;
        last_hall3_state = hall3_state;

        // 喂狗 + 延时
        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(50)); // 保持较快的响应速度
    }
}

void gpio_check_task(void *arg)
{
    ESP_ERROR_CHECK(esp_task_wdt_add(NULL));

    while (1)
    {
        if (level == 0&&gpio_get_level(BTN) == 1)
        {
            menu(NULL);
            level = 1;
        }

        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(50)); // 10ms轮询一次
    }
}
// WiFi开关状态检查任务
void wifi_switch_task(void *arg)
{
    ESP_ERROR_CHECK(esp_task_wdt_add(NULL));
    
    int last_wifi_sw_state = 0;
    
    while (1)
    {
        // 检查WiFi开关状态
        if (wifi_sw != NULL)
        {
            bool current_state = lv_obj_has_state(wifi_sw, LV_STATE_CHECKED);
            
            // 检测状态变化
            if (current_state != last_wifi_sw_state)
            {
                last_wifi_sw_state = current_state;
                
                if (current_state && !wifi_initialized)
                {
                    // 开关被打开且WiFi未初始化
                    ESP_LOGI(TAG, "WiFi开关打开，开始初始化WiFi配网");
                    wifi_connecting = true;
                    
                    // 更新UI显示
                    if (wifi_md != NULL)
                    {
                        lv_label_set_text(wifi_md, "正在配网...");
                    }
                    
                    // 初始化WiFi配网
                    initialise_wifi();
                    wifi_initialized = true;
                    
                    ESP_LOGI(TAG, "WiFi配网已启动，请使用手机APP进行配网");
                }
                else if (!current_state && wifi_initialized)
                {
                    // 开关被关闭
                    ESP_LOGI(TAG, "WiFi开关关闭");
                    wifi_connecting = false;
                    
                    // 更新UI显示
                    if (wifi_md != NULL)
                    {
                        lv_label_set_text(wifi_md, "WiFi已关闭");
                    }
                    
                    // 可以在这里添加WiFi断开的代码
                    esp_wifi_stop();
                    wifi_initialized = false;
                }
            }
        }
        
        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(100)); // 100ms检查一次
    }
}

void app_main(void)
{

    // LVGL 初始化
    lv_init();
    lv_port_disp_init();
    pin_setup();
    ESP_ERROR_CHECK(nvs_flash_init());
    lv_port_indev_init();

    // IMU 初始化
    if (ICM42688_Init(I2C_NUM_0, 0, 1) != ESP_OK)
    {
        ESP_LOGE(TAG, "ICM42688 init failed!");
        return;
    }
    else
    {
        ESP_LOGI(TAG, "ICM42688 init success!");
    }

    // 跳绳计数任务 - 降低优先级

    // LVGL 定时器
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "periodic_gui"};
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 10 * 1000));

    xTaskCreate(gpio_check_task, "gpio_check", 2048, NULL, 3, NULL);
    xTaskCreate(skp_cnt_task, "skp_cnt_task", 4096, NULL, 3, NULL);
    xTaskCreate(wifi_switch_task, "wifi_switch_task", 4096, NULL, 2, NULL); // 新增WiFi开关检查任务

    my_ui();
    jump_count = 0;
    ESP_LOGI(TAG, "ui ok,waiting for using");
    while (1)
    {
        // --- IMU 姿态更新 ---
        ICM42688_AttitudeUpdate(&g_att);

        // --- 显示角度（可调试用）---
        deg_roll = g_att.roll;
        deg_pitch = g_att.pitch;
        deg_yaw = g_att.yaw;

        // LVGL 循环
        lv_task_handler();
        lv_tick_inc(10);
        show_number(lbl_cnt, jump_count);
        vTaskDelay(pdMS_TO_TICKS(30)); // 从20ms增加到30ms，给其他任务更多时间

    }
}