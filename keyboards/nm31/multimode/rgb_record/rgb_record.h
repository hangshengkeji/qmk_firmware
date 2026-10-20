// Copyright 2024 JoyLee (@itarze)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <math.h>
#include "color.h"
#include "eeprom.h"
#include "quantum.h"

#define HS_GET_H(value) ((uint8_t)(value >> 8))
#define HS_GET_S(value) ((uint8_t)(value & 0xFF))

/* HSV Color bit[15:8] H  bit[7:0] S */
#define HS_AZURE        0x8466
#define HS_BLACK        0x0000
#define HS_BLUE         0xAAFF
#define HS_CHARTREUSE   0x40FF
#define HS_CORAL        0x0BB0
#define HS_GOLD         0x24FF
#define HS_GOLDENROD    0x1EDA
#define HS_GREEN        0x55FF
#define HS_MAGENTA      0xD5FF
#define HS_ORANGE       0x15FF
#define HS_PINK         0xEA80
#define HS_PURPLE       0xBFFF
#define HS_RED          0x00FF
#define HS_CYAN         0x6AFF
#define HS_TEAL         0x80FF
#define HS_TURQUOISE    0x7B5A
#define HS_WHITE        0x0101
#define HS_YELLOW       0x2BFF
#define ________        HS_BLACK

#ifndef RGB_RECORD_HS_LISTS
#    define RGB_RECORD_HS_LISTS \
        { HS_BLACK, HS_RED, HS_GREEN, HS_BLUE, HS_YELLOW, HS_PURPLE, HS_CYAN, HS_WHITE }
#endif

// 灯效录制初始化
void rgbrec_init(uint8_t channel);
// 显示当前通道
bool rgbrec_show(uint8_t channel);
// 开始灯效录制
bool rgbrec_start(uint8_t channel);
// 停止灯效录制
bool rgbrec_end(uint8_t channel);
// 灯效录制播放
void rgbrec_play(uint8_t led_min, uint8_t led_max);
// 设定所有等下录制hsv
void rgbrec_set_close_all(uint8_t h, uint8_t s, uint8_t v);
// 手动注册一个灯效录制操作
bool rgbrec_register_record(uint16_t keycode, keyrecord_t *record);
// 从一块地址中更新当前通道
void rgbrec_update_from_buffer(const uint8_t *effect, uint32_t size, uint8_t channel);
// 获取当前灯效录制是否是开启状态
bool rgbrec_is_started(void);
// 将当前通道录制的灯效录制数据存储到eeprom
void rgbrec_update_current_channel(uint8_t channel);
// 从eeprom中获取保存的灯效录制数据
void rgbrec_read_current_channel(uint8_t channel);

/* --- via --- */

// 获取支持的最多通道数
uint8_t rgbrec_get_channle_num(void);
// 切换到当前通道
void rgbrec_switch_channel(uint8_t channel);
// 获取单个矩阵位置hs值
uint16_t rgbrec_get_hs_data(uint8_t channel, uint8_t row, uint8_t column);
// 设定单个矩阵位置hs值
void rgbrec_set_hs_data(uint8_t channel, uint8_t row, uint8_t column, uint16_t hs);
// 获取单个矩阵位置hs值
void rgbrec_get_hs_buffer(uint16_t offset, uint16_t size, uint8_t *data);
// 设定单个矩阵位置hs值
void rgbrec_set_hs_buffer(uint16_t offset, uint16_t size, uint8_t *data);
