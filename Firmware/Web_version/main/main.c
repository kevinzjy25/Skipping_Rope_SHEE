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
#include "lv_port_indev.h"
#include "my_ui.h"

static void event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
        uint32_t id = lv_btnmatrix_get_selected_btn(obj);
        const char * txt = lv_btnmatrix_get_btn_text(obj, id);
 
        LV_LOG_USER("%s was pressed\n", txt);
    }
}
 
static const char * btnm_map[] = {"1", "2", "3", "4", "5", "\n",
                                  "6", "7", "8", "9", "0", "\n",
                                  "Action1", "Action2", ""
                                 };
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
    gpio_config_t pGPIOConfig_hall =
    {
        .intr_type =GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask =(1ULL << GPIO_NUM_0),
        .pull_down_en=GPIO_PULLDOWN_DISABLE,
        .pull_up_en =GPIO_PULLUP_ENABLE
    };
    gpio_config(&pGPIOConfig);
    gpio_config(&pGPIOConfig_hall);
    pGPIOConfig.pin_bit_mask =(1ULL << GPIO_NUM_20);
    pGPIOConfig_hall.pin_bit_mask =(1ULL << GPIO_NUM_0);
    pGPIOConfig_hall.pin_bit_mask =(1ULL << GPIO_NUM_1);
    lv_port_indev_init();

    lv_group_t *group = lv_group_create();
    lv_indev_set_group(indev_keypad, group);
 
    

    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "periodic_gui"};
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args,&periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 10 * 1000));
    if(gpio_get_level(0) == 0){
            ESP_LOGI("main","KEY0\r\n");
        }
        if(gpio_get_level(1) == 0){
            ESP_LOGI("main","KEY1\r\n");
        }
    lv_obj_t * btnm1 = lv_btnmatrix_create(lv_scr_act());
    lv_btnmatrix_set_map(btnm1, btnm_map);
    lv_btnmatrix_set_btn_width(btnm1, 10, 2);        /*Make "Action1" twice as wide as "Action2"*/
    lv_btnmatrix_set_btn_ctrl(btnm1, 10, LV_BTNMATRIX_CTRL_CHECKABLE);
    lv_btnmatrix_set_btn_ctrl(btnm1, 11, LV_BTNMATRIX_CTRL_CHECKED);
    lv_obj_align(btnm1, LV_ALIGN_CENTER, 0, 0);
    
    lv_group_add_obj(group ,btnm1);
    my_ui();
    while (1){
        lv_task_handler();
        lv_tick_inc(10);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
