#ifndef MY_UI_H
#define MY_UI_H


void my_ui(void);
#include "lvgl.h"
extern lv_obj_t *lbl_cnt;
void menu(void);
void show_number(lv_obj_t *lbl, int value);
#endif
