/*#include "my_ui.h"
#include "lvgl.h"
#include "driver/gpio.h"
#include "lv_port_indev.h"
#include "esp_log.h"
#include <stdio.h>
#include <unistd.h>
extern const lv_font_t SiYuanHeiTiGoogleBan_14;
LV_FONT_DECLARE(SiYuanHeiTiGoogleBan_14);
extern const lv_font_t SiYuanHeiTiGoogleBan_68;
LV_FONT_DECLARE(SiYuanHeiTiGoogleBan_68);
extern const lv_font_t SiYuanHeiTiGoogleBan_16;
LV_FONT_DECLARE(SiYuanHeiTiGoogleBan_16);

// 声明事件回调函数
static void pg1_btn1(lv_event_t *e);
static void pg1_btn2(lv_event_t *e);
static void pg1_btn3(lv_event_t *e);
lv_obj_t *main_menu; // 声明主菜单
lv_obj_t *_menu;
lv_obj_t *lbl_cnt;
lv_obj_t *lbl_md; // 声明标签
lv_obj_t *lbl_ssid;
lv_obj_t *btn1;
lv_obj_t *btn2;
lv_obj_t *btn3;
lv_obj_t *blt_sw ;
lv_obj_t *wifi_sw ;
lv_obj_t *blt_md;
lv_obj_t *wifi_md;

// 新增判断函数
int is_exam_mode_active = 0;
int tm_st_vl;
extern int level;
uint32_t selected_time = 0;              // 全局变量，存储选中的时间（秒）
static lv_obj_t *selected_label = NULL;  // 记录当前点击的 label
static bool is_connecting = false;       // 连接状态标志
static const char *selected_ssid = NULL; // 当前选择的 WiFi

static void show_menu_cb(lv_event_t *e)
{
    if (is_exam_mode_active == 1)
    {
        return;
    }
    // menu(e);
}

static void tm_st_button_click(lv_event_t *e)
{
    lv_obj_t *btn = lv_event_get_target(e);
    lv_obj_t *label = lv_obj_get_child(btn, 0);
    const char *txt = lv_label_get_text(label);

    // 提取前面的数字
    sscanf(txt, "%lu", &selected_time);

    // 可选：调试输出
    printf("time: %lu s\n", selected_time);
    tm_st_vl = selected_time;
    lv_scr_load_anim(main_menu, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 500, 0, false);
    level = 0;
}

void show_number(lv_obj_t *lbl, int value) // 数位补足函数
{
    if (value < 0)
        value = 0;
    if (value > 9999)
        value = 9999;

    lv_label_set_text_fmt(lbl, "%04d", value);
}

void my_ui(void)
{
    main_menu = lv_scr_act(); // 设为起始页
    ESP_LOGI("my_ui", "UI initialized");
    lv_obj_set_style_bg_color(main_menu, lv_color_hex(0x001d3d), LV_PART_MAIN); // 设置背景颜色

    lbl_cnt = lv_label_create(main_menu); // 创建标签
    lv_label_set_text(lbl_cnt, "0000");
    lv_obj_set_style_text_color(lbl_cnt, lv_color_hex(0xfbfbfb), 0);
    lv_obj_set_style_text_font(lbl_cnt, &SiYuanHeiTiGoogleBan_68, 0);
    lv_obj_align(lbl_cnt, LV_ALIGN_TOP_LEFT, 0, -3);

    lbl_md = lv_label_create(main_menu); // 创建标签
    lv_label_set_text(lbl_md, "考试模式");
    lv_obj_set_style_text_color(lbl_md, lv_color_hex(0x4c6077), 0);
    lv_obj_set_style_bg_opa(lbl_md, LV_OPA_COVER, 0);             // 设置背景透明度
    lv_obj_set_style_bg_color(lbl_md, lv_color_hex(0xCCCCCC), 0); // 设置背景颜色
    lv_obj_set_style_radius(lbl_md, 4, 0);                        // 设置圆角
    lv_obj_set_style_text_font(lbl_md, &SiYuanHeiTiGoogleBan_16, 0);
    lv_obj_align(lbl_md, LV_ALIGN_BOTTOM_LEFT, 0, 0);
}

void menu(lv_event_t *e)
{   
    lv_group_t *menu_group;
    _menu = lv_obj_create(NULL);                                            // 创建标签
    lv_obj_set_style_bg_color(_menu, lv_color_hex(0x001d3d), LV_PART_MAIN); // 设置背景颜色

    lv_obj_t *btn1 = lv_btn_create(_menu); // 创建按钮1
    lv_obj_set_size(btn1, 100, 20);
    lv_obj_align(btn1, LV_ALIGN_CENTER, 0, -25);
    lv_obj_set_style_bg_color(btn1, lv_color_hex(0xCCCCCC), 0);
    lv_obj_set_style_bg_opa(btn1, LV_OPA_COVER, 0); // 设置背景透明度
    lv_obj_set_style_radius(btn1, 5, 0);            // 设置圆角
    lv_obj_t *lbl1 = lv_label_create(btn1);
    lv_label_set_text(lbl1, "设置计时");
    lv_obj_align(lbl1, LV_ALIGN_TOP_MID, 0, -5);
    lv_obj_set_style_text_font(lbl1, &SiYuanHeiTiGoogleBan_14, 0);
    lv_obj_set_style_text_color(lbl1, lv_color_hex(0x464646), 0);

    lv_obj_t *btn2 = lv_btn_create(_menu); // 创建按钮2
    lv_obj_set_size(btn2, 100, 20);
    lv_obj_align(btn2, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(btn2, lv_color_hex(0xCCCCCC), 0);
    lv_obj_set_style_bg_opa(btn2, LV_OPA_COVER, 0); // 设置背景透明度
    lv_obj_set_style_radius(btn2, 5, 0);            // 设置圆角
    lv_obj_t *lbl2 = lv_label_create(btn2);
    lv_label_set_text(lbl2, "连接蓝牙");
    lv_obj_align(lbl2, LV_ALIGN_TOP_MID, 0, -5);
    lv_obj_set_style_text_font(lbl2, &SiYuanHeiTiGoogleBan_14, 0);
    lv_obj_set_style_text_color(lbl2, lv_color_hex(0x464646), 0);

    lv_obj_t *btn3 = lv_btn_create(_menu); // 创建按钮3
    lv_obj_set_size(btn3, 100, 20);
    lv_obj_align(btn3, LV_ALIGN_CENTER, 0, 25);
    lv_obj_set_style_bg_color(btn3, lv_color_hex(0xCCCCCC), 0);
    lv_obj_set_style_bg_opa(btn3, LV_OPA_COVER, 0); // 设置背景透明度
    lv_obj_set_style_radius(btn3, 5, 0);            // 设置圆角
    lv_obj_t *lbl3 = lv_label_create(btn3);
    lv_label_set_text(lbl3, "考试模式");
    lv_obj_align(lbl3, LV_ALIGN_TOP_MID, 0, -5);
    lv_obj_set_style_text_font(lbl3, &SiYuanHeiTiGoogleBan_14, 0);
    lv_obj_set_style_text_color(lbl3, lv_color_hex(0x464646), 0);

    lv_obj_add_event_cb(btn1, pg1_btn1, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(btn2, pg1_btn2, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(btn3, pg1_btn3, LV_EVENT_CLICKED, NULL);

    lv_scr_load_anim(_menu, LV_SCR_LOAD_ANIM_OVER_LEFT, 400, 0, false);

    menu_group = lv_group_create();
    lv_indev_set_group(indev_keypad, menu_group);
    lv_group_add_obj(menu_group, btn1);
    lv_group_add_obj(menu_group, btn2);
    lv_group_add_obj(menu_group, btn3);
    lv_group_focus_obj(btn1);
}

static void pg1_btn1(lv_event_t *e)
{
    lv_obj_t *st_tm = lv_obj_create(NULL);
    lv_obj_set_size(st_tm, 160, 80); // 屏幕大小
    lv_obj_set_style_bg_color(st_tm, lv_color_hex(0x001d3d), LV_PART_MAIN);
    lv_scr_load_anim(st_tm, LV_SCR_LOAD_ANIM_OVER_LEFT, 500, 0, false);

    // 先解绑输入设备
    lv_indev_set_group(indev_keypad, NULL);
    lv_group_del(menu_group);
    menu_group = NULL;
    lv_group_t *pg1_group = lv_group_create();
    lv_indev_set_group(indev_keypad, pg1_group);

    int y_offset = 5; // 初始纵向偏移
    for (uint32_t i = 30; i <= 300; i += 30)
    {
        lv_obj_t *btn = lv_btn_create(st_tm);
        lv_obj_set_size(btn, 100, 20);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0xCCCCCC), 0);
        lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0); // 设置背景透明度
        lv_obj_set_style_radius(btn, 4, 0);
        lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, y_offset); // 水平居中，纵向递增
        y_offset += 24;                                   // 下一按钮位置（20高 + 4间距）

        lv_obj_t *label = lv_label_create(btn);
        lv_label_set_text_fmt(label, "%lu s", i);
        lv_obj_center(label);
        lv_obj_set_style_text_font(label, &SiYuanHeiTiGoogleBan_14, 0);
        lv_obj_set_style_text_color(label, lv_color_hex(0x464646), 0);

        lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(btn, tm_st_button_click, LV_EVENT_CLICKED, NULL);
        lv_group_add_obj(pg1_group, btn);
    }
}

static void pg1_btn2(lv_event_t *e)
{
    lv_obj_t *blt_cnt = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(blt_cnt, lv_color_hex(0x001d3d), LV_PART_MAIN);
    lv_scr_load_anim(blt_cnt, LV_SCR_LOAD_ANIM_OVER_LEFT, 500, 0, false);

    lv_indev_set_group(indev_keypad, NULL);
    lv_group_del(menu_group);
    menu_group = NULL;
    lv_group_t *pg2_group = lv_group_create();
    lv_indev_set_group(indev_keypad, pg2_group);

    lv_obj_t *blt_cnt_btn = lv_label_create(blt_cnt);
    lv_obj_set_style_radius(blt_cnt_btn, 10, 0);                          // 设置圆角
    lv_obj_set_style_bg_color(blt_cnt_btn, lv_color_hex(0xCCCCCC), 0);    // 设置背景颜色
    lv_obj_set_style_bg_opa(blt_cnt_btn, LV_OPA_COVER, 0);                // 设置背景透明度
    lv_obj_set_style_text_font(blt_cnt_btn, &SiYuanHeiTiGoogleBan_14, 0); // 设置字体
    lv_obj_align(blt_cnt_btn, LV_ALIGN_TOP_MID, 0, 4);                    // 居中对齐
    lv_obj_set_size(blt_cnt_btn, 140, 30);                                // 设置按钮大小
    lv_label_set_text(blt_cnt_btn, " ");
    lv_text_align_t align = LV_TEXT_ALIGN_CENTER; // 设置文本居中对齐

    lv_obj_t *blt_lbl = lv_label_create(blt_cnt_btn);
    lv_label_set_text(blt_lbl, "打开蓝牙");
    lv_obj_set_style_text_font(blt_lbl, &SiYuanHeiTiGoogleBan_14, 0);
    lv_obj_align(blt_lbl, LV_ALIGN_CENTER, -35, 0);
    lv_obj_set_style_text_color(blt_lbl, lv_color_hex(0x464646), 0); // 设置文字颜色

    // 右侧开关
    blt_sw = lv_switch_create(blt_cnt_btn);
    lv_obj_set_size(blt_sw, 40, 20);
    lv_obj_align(blt_sw, LV_ALIGN_CENTER, 46, 0);
    lv_group_add_obj(pg2_group, blt_sw);

    blt_md = lv_label_create(blt_cnt);
    lv_label_set_text(blt_md, " ");
    lv_obj_set_style_text_font(blt_md, &SiYuanHeiTiGoogleBan_14, 0);
    lv_obj_set_style_text_color(blt_md, lv_color_hex(0xCCCCCC), 0);
    lv_obj_align_to(blt_md, blt_cnt_btn, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10); // 将文本对齐到按钮中心
}

static void pg1_btn3(lv_event_t *e)
{
    lv_obj_t *wifi_cnt = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(wifi_cnt, lv_color_hex(0x001d3d), LV_PART_MAIN);
    lv_scr_load_anim(wifi_cnt, LV_SCR_LOAD_ANIM_OVER_LEFT, 500, 0, false);

    lv_indev_set_group(indev_keypad, NULL);
    lv_group_del(menu_group);
    menu_group = NULL;
    lv_group_t *pg3_group = lv_group_create();
    lv_indev_set_group(indev_keypad, pg3_group);

    lv_obj_t *wifi_cnt_btn = lv_label_create(wifi_cnt);
    lv_obj_set_style_radius(wifi_cnt_btn, 10, 0);                          // 设置圆角
    lv_obj_set_style_bg_color(wifi_cnt_btn, lv_color_hex(0xCCCCCC), 0);    // 设置背景颜色
    lv_obj_set_style_bg_opa(wifi_cnt_btn, LV_OPA_COVER, 0);                // 设置背景透明度
    lv_obj_set_style_text_font(wifi_cnt_btn, &SiYuanHeiTiGoogleBan_14, 0); // 设置字体
    lv_obj_align(wifi_cnt_btn, LV_ALIGN_TOP_MID, 0, 4);                    // 居中对齐
    lv_obj_set_size(wifi_cnt_btn, 140, 30);                                // 设置按钮大小
    lv_label_set_text(wifi_cnt_btn, " ");
    lv_text_align_t align = LV_TEXT_ALIGN_CENTER; // 设置文本居中对齐

    lv_obj_t *wifi_lbl = lv_label_create(wifi_cnt_btn);
    lv_label_set_text(wifi_lbl, "打开Wi-Fi");
    lv_obj_set_style_text_font(wifi_lbl, &SiYuanHeiTiGoogleBan_14, 0);
    lv_obj_align(wifi_lbl, LV_ALIGN_CENTER, -35, 0);
    lv_obj_set_style_text_color(wifi_lbl, lv_color_hex(0x464646), 0); // 设置文字颜色
    // 右侧开关
    wifi_sw = lv_switch_create(wifi_cnt_btn);
    lv_obj_set_size(wifi_sw, 40, 20);
    lv_obj_align(wifi_sw, LV_ALIGN_CENTER, 46, 0);
    lv_group_add_obj(pg3_group, wifi_sw);

    wifi_md = lv_label_create(wifi_cnt);
    lv_label_set_text(wifi_md, " ");
    lv_obj_set_style_text_font(wifi_md, &SiYuanHeiTiGoogleBan_14, 0);
    lv_obj_set_style_text_color(wifi_md, lv_color_hex(0xCCCCCC), 0);
    lv_obj_align_to(wifi_md, wifi_cnt_btn, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10); // 将文本对齐到按钮中心
}*/
#include "my_ui.h"
#include "lvgl.h"
#include "driver/gpio.h"
#include "lv_port_indev.h"
#include "esp_log.h"
#include <stdio.h>
#include <unistd.h>

extern const lv_font_t SiYuanHeiTiGoogleBan_14;
LV_FONT_DECLARE(SiYuanHeiTiGoogleBan_14);
extern const lv_font_t SiYuanHeiTiGoogleBan_68;
LV_FONT_DECLARE(SiYuanHeiTiGoogleBan_68);
extern const lv_font_t SiYuanHeiTiGoogleBan_16;
LV_FONT_DECLARE(SiYuanHeiTiGoogleBan_16);

// 声明事件回调函数
static void pg1_btn1(lv_event_t *e);
static void pg1_btn2(lv_event_t *e);
static void pg1_btn3(lv_event_t *e);

lv_obj_t *main_menu; // 声明主菜单
lv_obj_t *_menu;
lv_obj_t *lbl_cnt;
lv_obj_t *lbl_md; // 声明标签
lv_obj_t *lbl_ssid;
lv_obj_t *btn1;
lv_obj_t *btn2;
lv_obj_t *btn3;
lv_obj_t *blt_sw;
lv_obj_t *wifi_sw;
lv_obj_t *blt_md;
lv_obj_t *wifi_md;

// 新增判断函数
int is_exam_mode_active = 0;
int tm_st_vl;
extern int level;
uint32_t selected_time = 0;              // 全局变量，存储选中的时间（秒）
static lv_obj_t *selected_label = NULL;  // 记录当前点击的 label
static bool is_connecting = false;       // 连接状态标志
static const char *selected_ssid = NULL; // 当前选择的 WiFi

// 添加防重复触发的全局变量
static bool button_processing = false;
static uint32_t last_button_time = 0;
static lv_group_t *current_group = NULL;

// 定义定时器回调函数
static void menu_setup_timer_cb(lv_timer_t *timer);
static void pg1_setup_timer_cb(lv_timer_t *timer);
static void pg2_setup_timer_cb(lv_timer_t *timer);
static void pg3_setup_timer_cb(lv_timer_t *timer);

// 防重复触发的辅助函数
static bool is_button_debounce_ok(void)
{
    uint32_t current_time = lv_tick_get();
    if (current_time - last_button_time < 200) { // 200ms防抖
        return false;
    }
    last_button_time = current_time;
    return true;
}

// 安全的组切换函数
static void safe_group_switch(lv_group_t *new_group)
{
    if (current_group) {
        lv_indev_set_group(indev_keypad, NULL);
        lv_group_del(current_group);
        current_group = NULL;
    }
    
    if (new_group) {
        current_group = new_group;
        lv_indev_set_group(indev_keypad, current_group);
    }
}

static void show_menu_cb(lv_event_t *e)
{
    if (is_exam_mode_active == 1)
    {
        return;
    }
}

static void tm_st_button_click(lv_event_t *e)
{
    // 防重复触发检查
    if (!is_button_debounce_ok() || button_processing) {
        return;
    }
    button_processing = true;
    
    lv_obj_t *btn = lv_event_get_target(e);
    lv_obj_t *label = lv_obj_get_child(btn, 0);
    const char *txt = lv_label_get_text(label);

    // 提取前面的数字
    sscanf(txt, "%lu", &selected_time);

    // 可选：调试输出
    printf("time: %lu s\n", selected_time);
    tm_st_vl = selected_time;
    
    // 先切换输入组，再切换页面
    safe_group_switch(NULL);
    
    lv_scr_load_anim(main_menu, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 500, 0, false);
    level = 0;
    
    // 创建定时器来重置处理标志
    lv_timer_t *reset_timer = lv_timer_create(menu_setup_timer_cb, 520, NULL);
    lv_timer_set_repeat_count(reset_timer, 1);
}

void show_number(lv_obj_t *lbl, int value) // 数位补足函数
{
    if (value < 0)
        value = 0;
    if (value > 9999)
        value = 9999;

    lv_label_set_text_fmt(lbl, "%04d", value);
}

void my_ui(void)
{
    main_menu = lv_scr_act(); // 设为起始页
    ESP_LOGI("my_ui", "UI initialized");
    lv_obj_set_style_bg_color(main_menu, lv_color_hex(0x001d3d), LV_PART_MAIN); // 设置背景颜色

    lbl_cnt = lv_label_create(main_menu); // 创建标签
    lv_label_set_text(lbl_cnt, "0000");
    lv_obj_set_style_text_color(lbl_cnt, lv_color_hex(0xfbfbfb), 0);
    lv_obj_set_style_text_font(lbl_cnt, &SiYuanHeiTiGoogleBan_68, 0);
    lv_obj_align(lbl_cnt, LV_ALIGN_TOP_LEFT, 0, -3);

    lbl_md = lv_label_create(main_menu); // 创建标签
    lv_label_set_text(lbl_md, "考试模式");
    lv_obj_set_style_text_color(lbl_md, lv_color_hex(0x4c6077), 0);
    lv_obj_set_style_bg_opa(lbl_md, LV_OPA_COVER, 0);             // 设置背景透明度
    lv_obj_set_style_bg_color(lbl_md, lv_color_hex(0xCCCCCC), 0); // 设置背景颜色
    lv_obj_set_style_radius(lbl_md, 4, 0);                        // 设置圆角
    lv_obj_set_style_text_font(lbl_md, &SiYuanHeiTiGoogleBan_16, 0);
    lv_obj_align(lbl_md, LV_ALIGN_BOTTOM_LEFT, 0, 0);
}

// 定时器回调函数实现
static void menu_setup_timer_cb(lv_timer_t *timer)
{
    button_processing = false; // 重置处理标志
    lv_timer_del(timer);
}

static void menu_main_setup_timer_cb(lv_timer_t *timer)
{
    lv_obj_t *current_screen = lv_scr_act();
    lv_group_t *menu_group = lv_group_create();
    
    // 重新获取按钮对象（按创建顺序）
    lv_obj_t *btn1 = lv_obj_get_child(current_screen, 0);
    lv_obj_t *btn2 = lv_obj_get_child(current_screen, 1);
    lv_obj_t *btn3 = lv_obj_get_child(current_screen, 2);
    
    lv_group_add_obj(menu_group, btn1);
    lv_group_add_obj(menu_group, btn2);
    lv_group_add_obj(menu_group, btn3);
    lv_group_focus_obj(btn3);
    
    safe_group_switch(menu_group);
    button_processing = false; // 允许按键响应
    lv_timer_del(timer);
}

static void pg1_setup_timer_cb(lv_timer_t *timer)
{
    lv_obj_t *current_screen = lv_scr_act();
    lv_group_t *pg1_group = lv_group_create();
    
    int y_offset = 5; // 初始纵向偏移
    for (uint32_t i = 30; i <= 300; i += 30)
    {
        lv_obj_t *btn = lv_btn_create(current_screen);
        lv_obj_set_size(btn, 100, 20);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0xCCCCCC), 0);
        lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0); // 设置背景透明度
        lv_obj_set_style_radius(btn, 4, 0);
        lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, y_offset); // 水平居中，纵向递增
        y_offset += 24;                                   // 下一按钮位置（20高 + 4间距）

        lv_obj_t *label = lv_label_create(btn);
        lv_label_set_text_fmt(label, "%lu s", i);
        lv_obj_center(label);
        lv_obj_set_style_text_font(label, &SiYuanHeiTiGoogleBan_14, 0);
        lv_obj_set_style_text_color(label, lv_color_hex(0x464646), 0);

        lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(btn, tm_st_button_click, LV_EVENT_CLICKED, NULL);
        lv_group_add_obj(pg1_group, btn);
    }
    
    safe_group_switch(pg1_group);
    button_processing = false; // 允许按键响应
    lv_timer_del(timer);
}

static void pg2_setup_timer_cb(lv_timer_t *timer)
{
    lv_obj_t *current_screen = lv_scr_act();
    lv_group_t *pg2_group = lv_group_create();

    lv_obj_t *blt_cnt_btn = lv_label_create(current_screen);
    lv_obj_set_style_radius(blt_cnt_btn, 10, 0);                          // 设置圆角
    lv_obj_set_style_bg_color(blt_cnt_btn, lv_color_hex(0xCCCCCC), 0);    // 设置背景颜色
    lv_obj_set_style_bg_opa(blt_cnt_btn, LV_OPA_COVER, 0);                // 设置背景透明度
    lv_obj_set_style_text_font(blt_cnt_btn, &SiYuanHeiTiGoogleBan_14, 0); // 设置字体
    lv_obj_align(blt_cnt_btn, LV_ALIGN_TOP_MID, 0, 4);                    // 居中对齐
    lv_obj_set_size(blt_cnt_btn, 140, 30);                                // 设置按钮大小
    lv_label_set_text(blt_cnt_btn, " ");

    lv_obj_t *blt_lbl = lv_label_create(blt_cnt_btn);
    lv_label_set_text(blt_lbl, "打开蓝牙");
    lv_obj_set_style_text_font(blt_lbl, &SiYuanHeiTiGoogleBan_14, 0);
    lv_obj_align(blt_lbl, LV_ALIGN_CENTER, -35, 0);
    lv_obj_set_style_text_color(blt_lbl, lv_color_hex(0x464646), 0); // 设置文字颜色

    // 右侧开关
    blt_sw = lv_switch_create(blt_cnt_btn);
    lv_obj_set_size(blt_sw, 40, 20);
    lv_obj_align(blt_sw, LV_ALIGN_CENTER, 46, 0);
    lv_group_add_obj(pg2_group, blt_sw);

    blt_md = lv_label_create(current_screen);
    lv_label_set_text(blt_md, " ");
    lv_obj_set_style_text_font(blt_md, &SiYuanHeiTiGoogleBan_14, 0);
    lv_obj_set_style_text_color(blt_md, lv_color_hex(0xCCCCCC), 0);
    lv_obj_align_to(blt_md, blt_cnt_btn, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10); // 将文本对齐到按钮中心
    
    safe_group_switch(pg2_group);
    button_processing = false;
    lv_timer_del(timer);
}

static void pg3_setup_timer_cb(lv_timer_t *timer)
{
    lv_obj_t *current_screen = lv_scr_act();
    lv_group_t *pg3_group = lv_group_create();

    lv_obj_t *wifi_cnt_btn = lv_label_create(current_screen);
    lv_obj_set_style_radius(wifi_cnt_btn, 10, 0);                          // 设置圆角
    lv_obj_set_style_bg_color(wifi_cnt_btn, lv_color_hex(0xCCCCCC), 0);    // 设置背景颜色
    lv_obj_set_style_bg_opa(wifi_cnt_btn, LV_OPA_COVER, 0);                // 设置背景透明度
    lv_obj_set_style_text_font(wifi_cnt_btn, &SiYuanHeiTiGoogleBan_14, 0); // 设置字体
    lv_obj_align(wifi_cnt_btn, LV_ALIGN_TOP_MID, 0, 4);                    // 居中对齐
    lv_obj_set_size(wifi_cnt_btn, 140, 30);                                // 设置按钮大小
    lv_label_set_text(wifi_cnt_btn, " ");

    lv_obj_t *wifi_lbl = lv_label_create(wifi_cnt_btn);
    lv_label_set_text(wifi_lbl, "打开Wi-Fi");
    lv_obj_set_style_text_font(wifi_lbl, &SiYuanHeiTiGoogleBan_14, 0);
    lv_obj_align(wifi_lbl, LV_ALIGN_CENTER, -35, 0);
    lv_obj_set_style_text_color(wifi_lbl, lv_color_hex(0x464646), 0); // 设置文字颜色
    
    // 右侧开关
    wifi_sw = lv_switch_create(wifi_cnt_btn);
    lv_obj_set_size(wifi_sw, 40, 20);
    lv_obj_align(wifi_sw, LV_ALIGN_CENTER, 46, 0);
    lv_group_add_obj(pg3_group, wifi_sw);

    wifi_md = lv_label_create(current_screen);
    lv_label_set_text(wifi_md, " ");
    lv_obj_set_style_text_font(wifi_md, &SiYuanHeiTiGoogleBan_14, 0);
    lv_obj_set_style_text_color(wifi_md, lv_color_hex(0xCCCCCC), 0);
    lv_obj_align_to(wifi_md, wifi_cnt_btn, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10); // 将文本对齐到按钮中心
    
    safe_group_switch(pg3_group);
    button_processing = false;
    lv_timer_del(timer);
}

void menu(lv_event_t *e)
{
    // 防重复触发检查
    if (!is_button_debounce_ok() || button_processing) {
        return;
    }
    button_processing = true;
    
    _menu = lv_obj_create(NULL);                                            // 创建标签
    lv_obj_set_style_bg_color(_menu, lv_color_hex(0x001d3d), LV_PART_MAIN); // 设置背景颜色

    lv_obj_t *btn1 = lv_btn_create(_menu); // 创建按钮1
    lv_obj_set_size(btn1, 100, 20);
    lv_obj_align(btn1, LV_ALIGN_CENTER, 0, -25);
    lv_obj_set_style_bg_color(btn1, lv_color_hex(0xCCCCCC), 0);
    lv_obj_set_style_bg_opa(btn1, LV_OPA_COVER, 0); // 设置背景透明度
    lv_obj_set_style_radius(btn1, 5, 0);            // 设置圆角
    lv_obj_t *lbl1 = lv_label_create(btn1);
    lv_label_set_text(lbl1, "设置计时");
    lv_obj_align(lbl1, LV_ALIGN_TOP_MID, 0, -5);
    lv_obj_set_style_text_font(lbl1, &SiYuanHeiTiGoogleBan_14, 0);
    lv_obj_set_style_text_color(lbl1, lv_color_hex(0x464646), 0);

    lv_obj_t *btn2 = lv_btn_create(_menu); // 创建按钮2
    lv_obj_set_size(btn2, 100, 20);
    lv_obj_align(btn2, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(btn2, lv_color_hex(0xCCCCCC), 0);
    lv_obj_set_style_bg_opa(btn2, LV_OPA_COVER, 0); // 设置背景透明度
    lv_obj_set_style_radius(btn2, 5, 0);            // 设置圆角
    lv_obj_t *lbl2 = lv_label_create(btn2);
    lv_label_set_text(lbl2, "连接蓝牙");
    lv_obj_align(lbl2, LV_ALIGN_TOP_MID, 0, -5);
    lv_obj_set_style_text_font(lbl2, &SiYuanHeiTiGoogleBan_14, 0);
    lv_obj_set_style_text_color(lbl2, lv_color_hex(0x464646), 0);

    lv_obj_t *btn3 = lv_btn_create(_menu); // 创建按钮3
    lv_obj_set_size(btn3, 100, 20);
    lv_obj_align(btn3, LV_ALIGN_CENTER, 0, 25);
    lv_obj_set_style_bg_color(btn3, lv_color_hex(0xCCCCCC), 0);
    lv_obj_set_style_bg_opa(btn3, LV_OPA_COVER, 0); // 设置背景透明度
    lv_obj_set_style_radius(btn3, 5, 0);            // 设置圆角
    lv_obj_t *lbl3 = lv_label_create(btn3);
    lv_label_set_text(lbl3, "考试模式");
    lv_obj_align(lbl3, LV_ALIGN_TOP_MID, 0, -5);
    lv_obj_set_style_text_font(lbl3, &SiYuanHeiTiGoogleBan_14, 0);
    lv_obj_set_style_text_color(lbl3, lv_color_hex(0x464646), 0);

    lv_obj_add_event_cb(btn1, pg1_btn1, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(btn2, pg1_btn2, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(btn3, pg1_btn3, LV_EVENT_CLICKED, NULL);

    // 先切换页面，再设置输入组
    lv_scr_load_anim(_menu, LV_SCR_LOAD_ANIM_OVER_BOTTOM, 400, 0, false);
    
    // 延迟设置输入组，等动画完成
    lv_timer_t *setup_timer = lv_timer_create(menu_main_setup_timer_cb, 450, NULL);
    lv_timer_set_repeat_count(setup_timer, 1);
}

static void pg1_btn1(lv_event_t *e)
{
    // 防重复触发检查
    if (!is_button_debounce_ok() || button_processing) {
        return;
    }
    button_processing = true;
    
    lv_obj_t *st_tm = lv_obj_create(NULL);
    lv_obj_set_size(st_tm, 160, 80); // 屏幕大小
    lv_obj_set_style_bg_color(st_tm, lv_color_hex(0x001d3d), LV_PART_MAIN);
    
    // 先切换到新页面，再设置输入组
    lv_scr_load_anim(st_tm, LV_SCR_LOAD_ANIM_OVER_LEFT, 500, 0, false);
    
    // 先清除当前输入组
    safe_group_switch(NULL);

    // 延迟创建新的输入组和按钮
    lv_timer_t *setup_timer = lv_timer_create(pg1_setup_timer_cb, 520, NULL);
    lv_timer_set_repeat_count(setup_timer, 1);
}

static void pg1_btn2(lv_event_t *e)
{
    // 防重复触发检查
    if (!is_button_debounce_ok() || button_processing) {
        return;
    }
    button_processing = true;
    
    lv_obj_t *blt_cnt = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(blt_cnt, lv_color_hex(0x001d3d), LV_PART_MAIN);
    lv_scr_load_anim(blt_cnt, LV_SCR_LOAD_ANIM_OVER_LEFT, 500, 0, false);

    safe_group_switch(NULL);
    
    lv_timer_t *setup_timer = lv_timer_create(pg2_setup_timer_cb, 520, NULL);
    lv_timer_set_repeat_count(setup_timer, 1);
}

static void pg1_btn3(lv_event_t *e)
{
    // 防重复触发检查
    if (!is_button_debounce_ok() || button_processing) {
        return;
    }
    button_processing = true;
    
    lv_obj_t *wifi_cnt = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(wifi_cnt, lv_color_hex(0x001d3d), LV_PART_MAIN);
    lv_scr_load_anim(wifi_cnt, LV_SCR_LOAD_ANIM_OVER_LEFT, 500, 0, false);

    safe_group_switch(NULL);
    
    lv_timer_t *setup_timer = lv_timer_create(pg3_setup_timer_cb, 520, NULL);
    lv_timer_set_repeat_count(setup_timer, 1);
}