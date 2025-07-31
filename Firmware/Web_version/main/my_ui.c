#include "my_ui.h"
#include "lvgl.h"
#include <stdio.h>
#include <unistd.h>
extern const lv_font_t SiYuanHeiTiGoogleBan_14;
LV_FONT_DECLARE(SiYuanHeiTiGoogleBan_14);
extern const lv_font_t SiYuanHeiTiGoogleBan_68;
LV_FONT_DECLARE(SiYuanHeiTiGoogleBan_68);
extern const lv_font_t SiYuanHeiTiGoogleBan_16;
LV_FONT_DECLARE(SiYuanHeiTiGoogleBan_16);

// 声明事件回调函数
static void pg1_btn1(lv_event_t * e);
static void pg1_btn2(lv_event_t * e);
static void pg1_btn3(lv_event_t * e);
static void menu(lv_event_t * e);
static void exm_md_appear(lv_event_t * e);
lv_obj_t *main_menu; // 声明主菜单
lv_obj_t *lbl_md; // 声明标签
lv_obj_t *lbl_ssid;
// 新增判断函数
int is_exam_mode_active = 0;

uint32_t selected_time = 0;  // 全局变量，存储选中的时间（秒）
static lv_obj_t *selected_label = NULL;  // 记录当前点击的 label
static bool is_connecting = false;       // 连接状态标志
static const char *selected_ssid = NULL; // 当前选择的 WiFi

static void show_menu_cb(lv_event_t * e) {
    if (is_exam_mode_active == 1) {
        return;
    }
    menu(e);
}

static void on_time_button_click(lv_event_t * e) {
    lv_obj_t *btn = lv_event_get_target(e);
    lv_obj_t *label = lv_obj_get_child(btn, 0);
    const char *txt = lv_label_get_text(label);

    // 提取前面的数字
    sscanf(txt, "%lu", &selected_time);

    // 可选：调试输出
    printf("time: %lu s\n", selected_time);

    // 返回主界面
    lv_scr_load_anim(main_menu, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 500, 0, false);
}

void my_ui(void){
    main_menu = lv_scr_act();//设为起始页
    lv_obj_set_style_bg_color(main_menu, lv_color_hex(0x001d3d), LV_PART_MAIN);//设置背景颜色

    lv_obj_t *lbl_cnt = lv_label_create(main_menu);//创建标签
    lv_label_set_text(lbl_cnt, "0000");
    lv_obj_set_style_text_color(lbl_cnt, lv_color_hex(0xfbfbfb), 0);
    lv_obj_set_style_text_font(lbl_cnt, &SiYuanHeiTiGoogleBan_68, 0);
    lv_obj_align(lbl_cnt, LV_ALIGN_TOP_LEFT, 0, -3);

    lbl_md = lv_label_create(main_menu);//创建标签
    lv_label_set_text(lbl_md, "考试模式");
    lv_obj_set_style_text_color(lbl_md, lv_color_hex(0x4c6077), 0);
    lv_obj_set_style_bg_opa(lbl_md, LV_OPA_COVER, 0); // 设置背景透明度
    lv_obj_set_style_bg_color(lbl_md, lv_color_hex(0xCCCCCC), 0); // 设置背景颜色
    lv_obj_set_style_radius(lbl_md, 4, 0); // 设置圆角
    lv_obj_set_style_text_font(lbl_md, &SiYuanHeiTiGoogleBan_16, 0);
    lv_obj_align(lbl_md, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_add_flag(main_menu, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(main_menu, show_menu_cb, LV_EVENT_CLICKED, NULL);

}
static void menu(lv_event_t * e) {
    printf("menu\n");
    lv_obj_t *menu = lv_obj_create(NULL);//创建标签
    lv_obj_set_style_bg_color(menu, lv_color_hex(0x001d3d), LV_PART_MAIN);//设置背景颜色

    lv_obj_t *btn1 = lv_label_create(menu);//创建按钮1
    lv_obj_set_size(btn1, 100, 20);
    lv_obj_align(btn1, LV_ALIGN_CENTER, 0, -25);
    lv_obj_set_style_bg_color(btn1, lv_color_hex(0xCCCCCC), 0);
    lv_obj_set_style_bg_opa(btn1, LV_OPA_COVER, 0); // 设置背景透明度
    lv_obj_set_style_radius(btn1, 5, 0); // 设置圆角
    lv_label_set_text(btn1, " ");
    lv_obj_t *lbl1 = lv_label_create(btn1);
    lv_label_set_text(lbl1, "设置计时");
    lv_obj_align(lbl1, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_text_font(lbl1, &SiYuanHeiTiGoogleBan_14, 0);
    lv_obj_set_style_text_color(lbl1, lv_color_hex(0x464646), 0);

    lv_obj_t *btn2 = lv_label_create(menu);//创建按钮2
    lv_obj_set_size(btn2, 100, 20);
    lv_obj_align(btn2, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(btn2, lv_color_hex(0xCCCCCC), 0);
    lv_obj_set_style_bg_opa(btn2, LV_OPA_COVER, 0); // 设置背景透明度
    lv_obj_set_style_radius(btn2, 5, 0); // 设置圆角
    lv_label_set_text(btn2, " ");
    lv_obj_t *lbl2 = lv_label_create(btn2);
    lv_label_set_text(lbl2, "连接蓝牙");
    lv_obj_align(lbl2, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_text_font(lbl2, &SiYuanHeiTiGoogleBan_14, 0);
    lv_obj_set_style_text_color(lbl2, lv_color_hex(0x464646), 0);

    lv_obj_t *btn3 = lv_label_create(menu);//创建按钮3
    lv_obj_set_size(btn3, 100, 20);
    lv_obj_align(btn3, LV_ALIGN_CENTER, 0, 25);
    lv_obj_set_style_bg_color(btn3, lv_color_hex(0xCCCCCC), 0);
    lv_obj_set_style_bg_opa(btn3, LV_OPA_COVER, 0); // 设置背景透明度
    lv_obj_set_style_radius(btn3, 5, 0); // 设置圆角
    lv_label_set_text(btn3, " ");
    lv_obj_t *lbl3 = lv_label_create(btn3);
    lv_label_set_text(lbl3, "考试模式");
    lv_obj_align(lbl3, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_text_font(lbl3, &SiYuanHeiTiGoogleBan_14, 0);
    lv_obj_set_style_text_color(lbl3, lv_color_hex(0x464646), 0);

    lv_obj_add_flag(lbl1, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(lbl2, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(lbl3, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_add_event_cb(lbl1, pg1_btn1, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(lbl2, pg1_btn2, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(lbl3, pg1_btn3, LV_EVENT_CLICKED, NULL);

    lv_scr_load_anim(menu,LV_SCR_LOAD_ANIM_OVER_BOTTOM,500,0,false);
}

static void pg1_btn1(lv_event_t * e) {
    lv_obj_t *st_tm = lv_obj_create(NULL);
    lv_obj_set_size(st_tm, 160, 80); // 屏幕大小
    lv_obj_set_style_bg_color(st_tm, lv_color_hex(0x001d3d), LV_PART_MAIN);
    lv_scr_load_anim(st_tm, LV_SCR_LOAD_ANIM_OVER_LEFT, 500, 0, false);

    int y_offset = 5; // 初始纵向偏移
    for (uint32_t i = 30; i <= 300; i += 30) {
        lv_obj_t *btn = lv_label_create(st_tm);
        lv_obj_set_size(btn, 100, 20);
        lv_label_set_text(btn, " ");
        lv_obj_set_style_bg_color(btn, lv_color_hex(0xCCCCCC), 0);
        lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0); // 设置背景透明度
        lv_obj_set_style_radius(btn, 4, 0);
        lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, y_offset); // 水平居中，纵向递增
        y_offset += 24; // 下一按钮位置（20高 + 4间距）

        lv_obj_t *label = lv_label_create(btn);
        lv_label_set_text_fmt(label, "%lu s", i);
        lv_obj_center(label);   
        lv_obj_set_style_text_font(label, &SiYuanHeiTiGoogleBan_14, 0);
        lv_obj_set_style_text_color(label, lv_color_hex(0x464646), 0);

        lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(btn, on_time_button_click, LV_EVENT_CLICKED, NULL);
    }
}

static void pg1_btn2(lv_event_t * e) {
    lv_obj_t * blt_cnt = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(blt_cnt, lv_color_hex(0x001d3d), LV_PART_MAIN);
    lv_scr_load_anim(blt_cnt,LV_SCR_LOAD_ANIM_OVER_LEFT,500,0,false);


    lv_obj_t *blt_cnt_btn = lv_label_create(blt_cnt);
    //lv_label_set_text(blt_cnt_btn, "打开蓝牙");

    lv_obj_set_style_radius(blt_cnt_btn, 10, 0); // 设置圆角
    lv_obj_set_style_bg_color(blt_cnt_btn, lv_color_hex(0xCCCCCC), 0); // 设置背景颜色
    lv_obj_set_style_bg_opa(blt_cnt_btn, LV_OPA_COVER, 0); // 设置背景透明度
    lv_obj_set_style_text_font(blt_cnt_btn, &SiYuanHeiTiGoogleBan_14, 0); // 设置字体
    lv_obj_align(blt_cnt_btn, LV_ALIGN_TOP_MID, 0, 4); // 居中对齐
    lv_obj_set_size(blt_cnt_btn, 140, 30); // 设置按钮大小
    lv_label_set_text(blt_cnt_btn, " ");
    lv_text_align_t align = LV_TEXT_ALIGN_CENTER; // 设置文本居中对齐

    lv_obj_t *blt_lbl = lv_label_create(blt_cnt_btn);
    lv_label_set_text(blt_lbl, "打开蓝牙");
    lv_obj_set_style_text_font(blt_lbl, &SiYuanHeiTiGoogleBan_14, 0);
    lv_obj_align(blt_lbl, LV_ALIGN_CENTER, -35, 0);
    lv_obj_set_style_text_color(blt_lbl, lv_color_hex(0x464646), 0); // 设置文字颜色

    // 右侧开关
    lv_obj_t * blt_sw = lv_switch_create(blt_cnt_btn);
    lv_obj_set_size(blt_sw, 40, 20);
    lv_obj_align(blt_sw, LV_ALIGN_CENTER, 46, 0);

    lv_obj_t * lbl_md = lv_label_create(blt_cnt);
    lv_label_set_text(lbl_md, "正在连接...");
    lv_obj_set_style_text_font(lbl_md, &SiYuanHeiTiGoogleBan_14, 0);
    lv_obj_set_style_text_color(lbl_md, lv_color_hex(0x464646), 0);
    lv_obj_align_to(lbl_md, blt_cnt_btn, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10); // 将文本对齐到按钮中心


}


// 模拟 Wi-Fi 名称和密码（预设）
const char* wifi_list[] = {"ZJY_5G", "ZJY_guest", "ZJY_lab","TEST"};

// 你的 pg1_btn3 函数：添加 Wi-Fi 选择页面
static void pg1_btn3(lv_event_t* e) {
    lv_obj_t *exm_md = lv_obj_create(NULL);
    lv_obj_set_size(exm_md, 160, 80); // 屏幕大小
    lv_obj_set_style_bg_color(exm_md, lv_color_hex(0x001d3d), LV_PART_MAIN);
    lv_scr_load_anim(exm_md, LV_SCR_LOAD_ANIM_OVER_LEFT, 500, 0, false);

    lv_obj_t* lbl_slt_ins = lv_label_create(exm_md);
    lv_label_set_text(lbl_slt_ins, "请选择 Wi-Fi");
    lv_obj_set_style_text_font(lbl_slt_ins, &SiYuanHeiTiGoogleBan_16, 0);
    lv_obj_set_style_text_color(lbl_slt_ins, lv_color_hex(0xfbfbfb), 0);
    lv_obj_align(lbl_slt_ins, LV_ALIGN_TOP_LEFT, 0, 0);


    int y_offset = 25; // 初始纵向偏移
    for (uint32_t i = 0; i < sizeof(wifi_list) / sizeof(wifi_list[0]); i++) {
        lv_obj_t *btn = lv_label_create(exm_md);
        lv_obj_set_size(btn, 100, 20);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0xCCCCCC), 0);
        lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0); // 设置背景透明度
        lv_obj_set_style_radius(btn, 4, 0);
        lv_label_set_text(btn, " ");
        lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, y_offset); // 水平居中，纵向递增
        y_offset += 24; // 下一按钮位置（20高 + 4间距）

        lv_obj_t *lbl_ssid = lv_label_create(btn);
        lv_label_set_text(lbl_ssid, wifi_list[i]);
        lv_obj_center(lbl_ssid);
        lv_obj_set_style_text_font(lbl_ssid, &SiYuanHeiTiGoogleBan_14, 0);
        lv_obj_set_style_text_color(lbl_ssid, lv_color_hex(0x464646), 0);

        lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(btn, exm_md_appear, LV_EVENT_CLICKED, (void*)wifi_list[i]);
    }

}


// 切换到主界面的回调
static void exam_mode_enter_cb(lv_timer_t *timer) {
    lv_scr_load_anim(main_menu, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 500, 0, false);
    lv_obj_set_style_text_color(lbl_md, lv_color_hex(0xfbfbfb), 0);
    lv_obj_set_style_bg_color(lbl_md, lv_color_hex(0x339933), 0); // 恢复背景颜色
    is_exam_mode_active = 1;

    // 恢复状态
    selected_label = NULL;
    is_connecting = false;

    lv_timer_del(timer); // 删除定时器
}

// 点击 WiFi 按钮回调
static void exm_md_appear(lv_event_t *e) {
    lv_obj_t *btn = lv_event_get_target(e);
    lv_obj_t *label = lv_obj_get_child(btn, 0); // 获取按钮内的第一个子对象（即label）
    const char *ssid = (const char *)lv_event_get_user_data(e);

    if (!is_connecting) {
        // 第一次点击
        lv_label_set_text(label, "正在连接...");
        printf("wifi_select: %s\n", ssid);
        selected_label = label;
        selected_ssid = ssid;
        is_connecting = true;
    } else if (label == selected_label) {
        // 第二次点击同一个按钮
        lv_label_set_text(label, "已连接");

        // 创建1秒后跳转主界面的定时器
        lv_timer_t *timer = lv_timer_create(exam_mode_enter_cb, 1000, NULL);
        lv_timer_set_repeat_count(timer, 1); // 只执行一次
    }
}
