#ifndef MY_UI_H
#define MY_UI_H


void my_ui(void);
#include "lvgl.h"
extern lv_obj_t *lbl_cnt;
extern lv_obj_t *btn1; // 声明标签
extern lv_obj_t *btn2; // 声明标签
extern lv_obj_t *btn3; // 声明标签
extern lv_obj_t *_menu;
extern lv_obj_t *blt_sw;
extern lv_obj_t *wifi_sw;
extern lv_obj_t *blt_md;
extern lv_obj_t *wifi_md;

void menu(lv_event_t * e);
void show_number(lv_obj_t *lbl, int value);

#endif
