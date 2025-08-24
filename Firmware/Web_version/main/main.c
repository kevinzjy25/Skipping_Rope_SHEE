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
#include "esp_eap_client.h"
#include "esp_flash.h"
#include "esp_timer.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_smartconfig.h"
#include "esp_netif.h"
#include "esp_mac.h"
#include "driver/gpio.h"
#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "my_ui.h"
#include "nvs_flash.h"
#include "icm42688p/include/icm42688p.h"
extern void initialise_wifi(void);
static const char *TAG = "MAIN";
int level = 0; // menu level
float deg_roll;
float deg_pitch;
float deg_yaw;
int blt_sw_state = 0;
int wifi_sw_state = 0;

// lvgl
static void event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (code == LV_EVENT_VALUE_CHANGED)
    {
        uint32_t id = lv_btnmatrix_get_selected_btn(obj);
        const char *txt = lv_btnmatrix_get_btn_text(obj, id);

        LV_LOG_USER("%s was pressed\n", txt);
    }
}
static void lv_tick_task(void *arg)
{
    (void)arg;
    lv_tick_inc(10);
}
void pin_setup(void)
{
    gpio_config_t pGPIOConfig =
        {
            .intr_type = GPIO_INTR_DISABLE,
            .mode = GPIO_MODE_INPUT,
            .pin_bit_mask = (1ULL << GPIO_NUM_20),
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .pull_up_en = GPIO_PULLUP_DISABLE};
    gpio_config_t pGPIOConfig_hall =
        {
            .intr_type = GPIO_INTR_DISABLE,
            .mode = GPIO_MODE_INPUT,
            .pin_bit_mask = (1ULL << GPIO_NUM_5) | (1ULL << GPIO_NUM_4) | (1ULL << GPIO_NUM_8),
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .pull_up_en = GPIO_PULLUP_ENABLE};
    gpio_config_t pGPIOConfig_motor =
        {
            .intr_type = GPIO_INTR_DISABLE,
            .mode = GPIO_MODE_INPUT,
            .pin_bit_mask = (1ULL << GPIO_NUM_9),
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .pull_up_en = GPIO_PULLUP_ENABLE};
    gpio_config(&pGPIOConfig);       // 物理按键
    gpio_config(&pGPIOConfig_hall);  // 霍尔开关1
    gpio_config(&pGPIOConfig_motor); // 震动马达
}
// 主动读取开关状态
void blt_stt_check(void)
{
    if (blt_sw == NULL) {
        ESP_LOGW(TAG, "blt_sw not created yet!");
        return;
    }

    if (lv_obj_has_state(blt_sw, LV_STATE_CHECKED)) {
        ESP_LOGI(TAG, "blt_sw is ON");
        lv_label_set_text(blt_md, "ON");
    } else {
        ESP_LOGI(TAG, "blt_sw is OFF");
        lv_label_set_text(blt_md, "OFF");
    }
}
void wifi_stt_check(void)
{
    if (wifi_sw == NULL) {
        ESP_LOGW(TAG, "wifi_sw not created yet!");
        return;
    }
    if (lv_obj_has_state(wifi_sw, LV_STATE_CHECKED)) {
        ESP_LOGI(TAG, "wifi_sw is ON");
        lv_label_set_text(wifi_md, "ON");
        ESP_ERROR_CHECK(nvs_flash_init());
        initialise_wifi();
    } else {
        ESP_LOGI(TAG, "wifi_sw is OFF");
        lv_label_set_text(wifi_md, "OFF");
    }
}
// GPIO 引脚
#define HALL1 4
#define HALL2 5
#define HALL3 8

// 跳绳计数
static volatile int jump_count = 0;

// GPIO 顺序状态机
typedef enum {
    IDLE = 0,
    STEP1,
    STEP2,
    STEP3
} skp_state_t;

// y 轴角度阈值（单位：度）
#define MAX_Y_ANGLE 30.0f

// 函数：轮询检测 GPIO，顺序或倒序
void skp_cnt_task(void *arg) {
    skp_state_t state = IDLE;
    int seq[3] = {HALL1, HALL2, HALL3};
    int rev[3] = {HALL3, HALL2, HALL1};
    bool counting_started = false;
    TickType_t last_jump_tick = 0;

    while (1) {
        // --- 顺序/倒序状态机 ---
        switch(state) {
            case IDLE:
                if (gpio_get_level(seq[0]) == 0 || gpio_get_level(rev[0]) == 0) {
                    state = STEP1;
                }
                break;
            case STEP1:
                if (gpio_get_level(seq[1]) == 0 || gpio_get_level(rev[1]) == 0) state = STEP2;
                break;
            case STEP2:
                if (gpio_get_level(seq[2]) == 0 || gpio_get_level(rev[2]) == 0) state = STEP3;
                break;
            case STEP3:
                // --- 完成一次跳绳 ---
                Attitude_t att;
                ICM42688_AttitudeUpdate(&att);

                // y 轴角度限制检测
                if (fabs(att.pitch) <= MAX_Y_ANGLE) {
                    jump_count++;
                    last_jump_tick = xTaskGetTickCount();
                    if (!counting_started) {
                        counting_started = true;
                        ESP_LOGI(TAG, "开始计数");
                    }
                }
                state = IDLE;
                break;
        }

        // --- 检测 1 秒无跳绳事件 ---
        if (counting_started && (xTaskGetTickCount() - last_jump_tick) > pdMS_TO_TICKS(1000)) {
            ESP_LOGI(TAG, "计数完成，本次跳绳总数: %d", jump_count);
            counting_started = false;
        }

        vTaskDelay(pdMS_TO_TICKS(5)); // 5ms 轮询
    }
}

// 初始化计数任务
void skp_cnt_init() {
    xTaskCreate(skp_cnt_task, "skp_cnt_task", 2048, NULL, 10, NULL);
}


void app_main(void)
{
    // 初始化 Wi-Fi
    // 初始化 NVS（必需的，因为Wi-Fi配置存储在NVS中）
    // ESP_ERROR_CHECK(nvs_flash_init());
    // initialise_wifi();
    // lvgl初始化
    lv_init();
    lv_port_disp_init();
    pin_setup();
    lv_port_indev_init();
    skp_cnt_init(); // 初始化跳绳计数任务
    // ICM42688P 初始化
    if (ICM42688_Init(I2C_NUM_0, 0, 1) != ESP_OK)
    {
        printf("ICM42688 init failed!\n");
        return;
    }
    else
    {
        printf("ICM42688 init success!\n");
    }

    Attitude_t att;
    // 定时器初始化
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "periodic_gui"};
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 10 * 1000));
    my_ui();
    while (1)
    {
        ICM42688_AttitudeUpdate(&att);
        lv_task_handler();
        lv_tick_inc(10);
        deg_roll = att.roll;
        deg_pitch = att.pitch;
        deg_yaw = att.yaw;

        if (level == 0 && gpio_get_level(5) == 0)
        {
            menu(NULL);
            level = 1;
        }
        blt_stt_check();
        wifi_stt_check();
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
