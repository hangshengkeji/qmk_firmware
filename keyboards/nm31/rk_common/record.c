
#include QMK_KEYBOARD_H
#include "immobile.h"

#include <stdio.h>
#include <string.h>

#ifdef GPIO_UART_ENABLE
#include "iprint.h"
#include <stdarg.h>
#define DEBUG(fmt, ...) iprintf(fmt, ##__VA_ARGS__)
#else
#define DEBUG(fmt, ...)
#endif

#define RGB_HSV_MAX 7
#define LASET_MOD_NO 0xFF
#include "eeprom.h"
// 主灯效 hs
// #ifdef RK_RGB
// static uint8_t rgb_hsvs[RGB_HSV_MAX][2] = {
//     {0,255},
//     {85,255},
//     {170,255},
//     {43,255},
//     {191,255},
//     {128,255},
//     {4,80},
// };
// #else
// static uint8_t rgb_hsvs[RGB_HSV_MAX][2] = {
//     {0,255},
//     {85,255},
//     {170,255},
//     {43,255},
//     {191,255},
//     {128,255},
//     {0,0},
// };
// #endif

// 主灯效索引
static const uint8_t rgbmatrix_buff[] = {13, 5,  1,  21, 28, 24, 41, 33};
// hs只有6档的索引
static const uint8_t sixth_gear_buff[] = {6, 13, 15, 16, 25, 26, 34};
bool rk_bat_req_flag = false;

#ifdef RGBLIGHT_ENABLE
static const uint8_t rgblight_buff[] = {14,8,2,1,1,1,1,1,1,1,0};
#endif

//传入上次灯效last_mode  如果上次灯效last_mode == LASET_MOD_NO 则代表没有需要切换到上次灯效的记忆
void record_rgbmatrix_increase(uint8_t* last_mode) {
    uint8_t moude_dev = 0;
    bool mode_get_flage = false;
    uint8_t index;

    if ((*last_mode) != LASET_MOD_NO){
        moude_dev = (*last_mode);
    } else {
        moude_dev = rgb_matrix_get_mode();
    }

    for (index = 0; index < (sizeof(rgbmatrix_buff) / sizeof(rgbmatrix_buff[0])); index ++) {
        if (moude_dev == rgbmatrix_buff[index]) {
            mode_get_flage = true;
            break;
        }
    }

    if (!mode_get_flage) {
        DEBUG("record_rgbmatrix_increase ERR\r\n");
    }

    if ((*last_mode) != LASET_MOD_NO) {
        (*last_mode) = LASET_MOD_NO;
    } else {
        index = (index + 1) % (sizeof(rgbmatrix_buff) / sizeof(rgbmatrix_buff[0]));
    }
    
    rgb_matrix_mode(rgbmatrix_buff[index]);
}

#ifdef RGB_RECORD_ENABLE
// 传入需要切换到的通道 record_channel 上次的模式last_mode
uint8_t record_led_channel(uint8_t record_channel ,uint8_t* last_mode) {

    if ((*last_mode) == LASET_MOD_NO) {
        (*last_mode) = rgb_matrix_get_mode();
    }

    return record_channel;
}
#endif

// 清除全部记忆的灯效HS
void all_color_reset(void) {
    for (uint32_t i = 0; i < (sizeof(rgbmatrix_buff) / sizeof(rgbmatrix_buff[0]) * 2); i++) {
        eeprom_write_byte((uint8_t *)(((uint32_t)CONFINFO_EECONFIG_ADDR + 4) +  i), 0xFF);
    }
    // eeprom_write_byte((uint8_t *)(((uint32_t)CONFINFO_EECONFIG_ADDR + 4) +  2), 0x0);
    // eeprom_write_byte((uint8_t *)(((uint32_t)CONFINFO_EECONFIG_ADDR + 4) +  10), 0x0);

    eeprom_write_byte((uint8_t *)(((uint32_t)CONFINFO_EECONFIG_ADDR + 4) +  7), 0x0);
    eeprom_write_byte((uint8_t *)(((uint32_t)CONFINFO_EECONFIG_ADDR + 4) +  15), 0x0);

    eeprom_write_byte((uint8_t *)(((uint32_t)CONFINFO_EECONFIG_ADDR + 4) +  1), 0xAA);
}

// 获取灯效对应数组的索引
uint8_t get_rgb_matrix_index(void) {
    uint8_t now_mode_temp = rgb_matrix_get_mode();
    bool mode_get_flage = false;
    uint8_t index = 0;
    for (index = 0; index < sizeof(rgbmatrix_buff) / sizeof(rgbmatrix_buff[0]); index++) {
        if (now_mode_temp == rgbmatrix_buff[index]) {
            mode_get_flage = true;
            break;
        }
    }

    if (!mode_get_flage) {
        DEBUG("get_rgb_matrix_index ERR\r\n");
    }

    DEBUG("index =%d \r\n", index);
    return index;
}

// 写入对应灯效的HS所对应的索引
void record_color_write_h_data(uint8_t color_index) {
    uint8_t hs_mode = get_rgb_matrix_index();

    uint8_t *ptr = (uint8_t *)(((uint32_t)CONFINFO_EECONFIG_ADDR + 4) + hs_mode);
    eeprom_write_byte(ptr, color_index);
}

void record_color_write_s_data(uint8_t color_index) {
    uint8_t hs_mode = get_rgb_matrix_index();

    uint8_t *ptr = (uint8_t *)(((uint32_t)CONFINFO_EECONFIG_ADDR + 4) + hs_mode + 8);
    eeprom_write_byte(ptr, color_index);
}

// 读取对应灯效的HS所对应的索引
uint8_t record_color_read_h_data(void) {
    uint8_t hs_mode = get_rgb_matrix_index();

    const uint8_t *ptr      = (const uint8_t *)(((uint32_t)CONFINFO_EECONFIG_ADDR + 4) + hs_mode);
    uint8_t hs_c            = eeprom_read_byte(ptr);
    DEBUG("hs_c =%d hs_mode =%d\r\n",hs_c,hs_mode);
    if (hs_c > RGB_HSV_MAX) {
        DEBUG("record_color_read_data ERR\r\n");
        return hs_c;
    } else {
        return hs_c;
    }
}

uint8_t record_color_read_s_data(void) {
    uint8_t hs_mode = get_rgb_matrix_index();

    const uint8_t *ptr      = (const uint8_t *)(((uint32_t)CONFINFO_EECONFIG_ADDR + 4) + hs_mode + 8);
    uint8_t hs_c            = eeprom_read_byte(ptr);
    DEBUG("hs_c =%d hs_mode =%d\r\n",hs_c,hs_mode);
    if (hs_c > RGB_HSV_MAX) {
        DEBUG("record_color_read_data ERR\r\n");
        return hs_c;
    } else {
        return hs_c;
    }
}

// 设置每个灯效所存储的HS
void record_color_hsv(void) {
    uint8_t now_mode = rgb_matrix_get_mode();
    uint8_t temp = 0;

    for (uint8_t i = 0; i < (sizeof(sixth_gear_buff)/sizeof(sixth_gear_buff[0])); i++) {
        if (now_mode == sixth_gear_buff[i]) {
            temp = RGB_HSV_MAX - 1;
            break;
        }
    }

    if (!temp) {
        temp = RGB_HSV_MAX;
        DEBUG("record_color_hsv ERR \r\n");
    }

    uint8_t rgb_h_index = record_color_read_h_data();
    uint8_t rgb_s_index = record_color_read_s_data();

    rgb_matrix_sethsv(rgb_h_index,rgb_s_index, rgb_matrix_get_val());

}

// 切换灯效 传入 上次灯效
void record_rgbmatrix_mod(uint8_t* last_mode) {
    record_rgbmatrix_increase(last_mode);
    record_color_hsv();
    // uint8_t rgb_hsv_index = record_color_read_data();
    // DEBUG("rgb_hsv_index =%d \r\n",rgb_hsv_index);
    // rgb_matrix_sethsv(rgb_hsvs[rgb_hsv_index][0],rgb_hsvs[rgb_hsv_index][1], rgb_matrix_get_val());
}

#ifdef RGBLIGHT_ENABLE
uint8_t record_rgblight_mod(uint8_t rgblight_num) {

    rgblight_num = ((rgblight_num+1) % (sizeof(rgblight_buff)/sizeof(rgblight_buff[0])));
    switch (rgblight_num){
        case 3: rgblight_sethsv(0,  255, rgblight_get_val());break;
        case 4: rgblight_sethsv(40, 255, rgblight_get_val());break;
        case 5: rgblight_sethsv(80, 255, rgblight_get_val());break;
        case 6: rgblight_sethsv(120,255, rgblight_get_val());break;
        case 7: rgblight_sethsv(160,255, rgblight_get_val());break;
        case 8: rgblight_sethsv(200,255, rgblight_get_val());break;
        case 9: rgblight_sethsv(0,  0,   rgblight_get_val());break;
        case 10: {
            rgblight_sethsv(200,255, rgblight_get_val());
            rgblight_disable();
            } break;
        case 0: {
            rgblight_enable();
            rgblight_set_speed(255);
            } break;
        default: {
            rgblight_set_speed(200);
            } break;
    }
    rgblight_mode(rgblight_buff[rgblight_num]);
    return rgblight_num;
}
#endif

#include <math.h>
#include <lib/lib8tion/lib8tion.h>
//特定rgb灯珠呼吸,hsv呼吸灯的hsv,speed灯效速度,index灯珠索引
void rgb_breathing_indicator(HSV hsv, uint8_t speed, uint8_t index) {

    uint16_t time = scale16by8(g_rgb_timer, speed / 8);
    hsv.v         = scale8(abs8(sin8(time) - 128) * 2, hsv.v);
    RGB rgb       = hsv_to_rgb(hsv);

    rgb_matrix_set_color(index, rgb.r, rgb.g, rgb.b);
}

// 给rk重定义的 电量查询
bool rgb_matrix_indicators_advanced_rk(uint8_t led_min, uint8_t led_max) {

#ifndef RGB_MATRIX_TEST_VAL
#    ifdef RGB_MATRIX_MAXIMUM_BRIGHTNESS
#        define RGB_MATRIX_TEST_VAL RGB_MATRIX_MAXIMUM_BRIGHTNESS
#    else
#        define RGB_MATRIX_TEST_VAL 0x77
#    endif
#endif

#ifndef RGB_MATRIX_BAT_VAL
#    define RGB_MATRIX_BAT_VAL RGB_MATRIX_TEST_VAL
#endif

#ifndef RGB_MATRIX_BAT_INDEX_MAP
#    define RGB_MATRIX_BAT_INDEX_MAP {17, 18, 19, 20, 21, 22, 23, 24, 25, 26}
#endif

#ifndef IM_BAT_REQ_LEVEL1_VAL
#    define IM_BAT_REQ_LEVEL1_VAL 50
#endif

#ifndef IM_BAT_REQ_LEVEL2_VAL
#    define IM_BAT_REQ_LEVEL2_VAL 30
#endif

#ifndef IM_BAT_REQ_LEVEL1_COLOR
#    define IM_BAT_REQ_LEVEL1_COLOR 0x00, RGB_MATRIX_BAT_VAL, 0x00
#endif

#ifndef IM_BAT_REQ_LEVEL2_COLOR
#    define IM_BAT_REQ_LEVEL2_COLOR RGB_MATRIX_BAT_VAL, RGB_MATRIX_BAT_VAL, 0x00
#endif

#ifndef IM_BAT_REQ_LEVEL3_COLOR
#    define IM_BAT_REQ_LEVEL3_COLOR RGB_MATRIX_BAT_VAL, 0x00, 0x00
#endif

#ifdef MULTIMODE_ENABLE
    if (rk_bat_req_flag) {
#ifdef IM_RGB_BAT_REQ_CLOSE_ALL
#    ifdef RGBLIGHT_ENABLE
        for (uint8_t i = 0; i < (RGB_MATRIX_LED_COUNT - RGBLED_NUM); i++) {
            rgb_matrix_set_color(i, 0, 0, 0);
        }
#    else
        rgb_matrix_set_color_all(0x00, 0x00, 0x00);
#    endif
#endif
        for (uint8_t i = 0; i < 10; i++) {
            uint8_t mi_index[10] = RGB_MATRIX_BAT_INDEX_MAP;
            if ((i < (bts_info.bt_info.pvol / 10)) || (i < 1)) {
                if (bts_info.bt_info.pvol >= (IM_BAT_REQ_LEVEL1_VAL)) {
                    rgb_matrix_set_color(mi_index[i], IM_BAT_REQ_LEVEL1_COLOR);
                } else if (bts_info.bt_info.pvol >= (IM_BAT_REQ_LEVEL2_VAL)) {
                    rgb_matrix_set_color(mi_index[i], IM_BAT_REQ_LEVEL2_COLOR);
                } else {
                    rgb_matrix_set_color(mi_index[i], IM_BAT_REQ_LEVEL3_COLOR);
                }
            } else {
                rgb_matrix_set_color(mi_index[i], 0x00, 0x00, 0x00);
            }
        }
    }
#endif
    return true;
}
