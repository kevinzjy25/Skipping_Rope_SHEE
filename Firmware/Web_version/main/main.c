/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_timer.h" 
#include "esp_system.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_demos.h"
#include "lv_port_indev.h"
static void lv_tick_task(void *arg){
    (void)arg;
    lv_tick_inc(10);
}

void app_main(void)
{
    lv_init();
    lv_port_disp_init();

    gpio_config_t pGPIOConfig =
    {
        .intr_type =GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask =(1ULL << GPIO_NUM_20),
        .pull_down_en=GPIO_PULLDOWN_DISABLE,
        .pull_up_en =GPIO_PULLUP_DISABLE
    };
    gpio_config(&pGPIOConfig);
    pGPIOConfig.pin_bit_mask =(1ULL << GPIO_NUM_20);

    gpio_config_t pGPIOConfig_hall =
    {
        .intr_type =GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask =(1ULL << GPIO_NUM_0),
        .pull_down_en=GPIO_PULLDOWN_DISABLE,
        .pull_up_en =GPIO_PULLUP_ENABLE
    };
    gpio_config(&pGPIOConfig_hall);
    pGPIOConfig_hall.pin_bit_mask =(1ULL << GPIO_NUM_0);
    pGPIOConfig_hall.pin_bit_mask =(1ULL << GPIO_NUM_1);



    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "periodic_gui"};
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args,&periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 10 * 1000));
    lv_demo_stress();
    while(1){
        if(gpio_get_level(20) == 1){
            ESP_LOGI("main","KEY20\r\n");
        }
        if(gpio_get_level(0) == 0){
            ESP_LOGI("main","KEY0\r\n");
        }
        if(gpio_get_level(1) == 0){
            ESP_LOGI("main","KEY1\r\n");
        }
        lv_task_handler();
        lv_tick_inc(10);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
