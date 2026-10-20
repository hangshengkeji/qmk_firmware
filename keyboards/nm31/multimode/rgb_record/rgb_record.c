// Copyright 2024 JoyLee (@itarze)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "rgb_record.h"
#include "rgb_matrix.h"
#include "eeprom.h"

#define RGBREC_STATE_ON 1
#define RGBREC_STATE_OFF 0

#ifndef RGBREC_CHANNEL_NUM
#    error "must be define RGBREC_CHANNEL_NUM"
#endif

#ifndef RGBREC_EECONFIG_ADDR
#    define RGBREC_EECONFIG_ADDR EECONFIG_USER_DATABLOCK
#endif

// #if (RGBREC_EECONFIG_ADDR != EECONFIG_USER_DATABLOCK)
// #    error "RGBREC_EECONFIG_ADDR must be the same as EECONFIG_USER_DATABLOCK"
// #endif

#define RGBREC_COLOR_NUM (sizeof(rgbrec_hs_lists) / sizeof(rgbrec_hs_lists[0]))

typedef struct {
    uint8_t state;
    uint8_t channel;
    uint16_t value;
} rgbrec_info_t;

typedef uint16_t (*rgbrec_effects_t)[MATRIX_COLS];

rgbrec_effects_t p_rgbrec_effects = NULL;

static uint16_t rgbrec_hs_lists[] = RGB_RECORD_HS_LISTS;
static uint8_t rgbrec_buffer[MATRIX_ROWS * MATRIX_COLS * 2];
static rgbrec_info_t rgbrec_info = {
    .state   = RGBREC_STATE_OFF,
    .channel = 0,
    .value   = 0xFF,
};

extern const uint16_t PROGMEM rgbrec_default_effects[RGBREC_CHANNEL_NUM][MATRIX_ROWS][MATRIX_COLS];

static bool find_matrix_row_col(uint8_t index, uint8_t *row, uint8_t *col);
static inline RGB hs_to_rgb(uint8_t h, uint8_t s);
static inline void rgb_matrix_set_hs(int index, uint16_t hs);
static inline void cycle_rgb_next_color(uint8_t row, uint8_t col);

static bool find_matrix_row_col(uint8_t index, uint8_t *row, uint8_t *col) {
    uint8_t i, j;

    for (i = 0; i < MATRIX_ROWS; i++) {
        for (j = 0; j < MATRIX_COLS; j++) {
            if (g_led_config.matrix_co[i][j] != NO_LED) {
                if (g_led_config.matrix_co[i][j] == index) {
                    *row = i;
                    *col = j;
                    return true;
                }
            }
        }
    }

    return false;
}

static inline RGB hs_to_rgb(uint8_t h, uint8_t s) {
    if ((h == 0) && (s == 0)) {
        return hsv_to_rgb((HSV){0, 0, 0});
    } else if ((h == 1) && (s == 1)) {
        return hsv_to_rgb((HSV){0, 0, rgbrec_info.value});
    } else {
        return hsv_to_rgb((HSV){h, s, rgbrec_info.value});
    }
}

static inline void rgb_matrix_set_hs(int index, uint16_t hs) {
    RGB rgb = hs_to_rgb(HS_GET_H(hs), HS_GET_S(hs));
    rgb_matrix_set_color(index, rgb.r, rgb.g, rgb.b);
}

static inline void cycle_rgb_next_color(uint8_t row, uint8_t col) {

    if (p_rgbrec_effects == NULL) {
        return;
    }

    for (uint8_t index = 0; index < RGBREC_COLOR_NUM; index++) {
        if (rgbrec_hs_lists[index] == p_rgbrec_effects[row][col]) {
            index                      = ((index + 1) % RGBREC_COLOR_NUM);
            p_rgbrec_effects[row][col] = rgbrec_hs_lists[index];
            return;
        }
    }

    p_rgbrec_effects[row][col] = rgbrec_hs_lists[0];
}

// 根据keycode record实现灯效录制操作
static bool rgbrec_process_record(uint16_t keycode, keyrecord_t *record) {

    if (rgbrec_info.state == RGBREC_STATE_ON) {
        if (IS_QK_MOMENTARY(keycode)) {
            return true;
        } else if (record->event.pressed) {
            cycle_rgb_next_color(record->event.key.row, record->event.key.col);
        }
        return false;
    }

    return true;
}

// 灯效录制初始化
void rgbrec_init(uint8_t channel) {

    p_rgbrec_effects    = (rgbrec_effects_t)rgbrec_buffer;
    rgbrec_info.state   = RGBREC_STATE_OFF;
    rgbrec_info.channel = channel;
    rgbrec_info.value   = rgb_matrix_get_val();

    rgbrec_read_current_channel(rgbrec_info.channel);
}

// 显示当前通道
bool rgbrec_show(uint8_t channel) {

    if (channel >= RGBREC_CHANNEL_NUM) {
        return false;
    }

    rgbrec_info.channel = channel;
    rgbrec_read_current_channel(rgbrec_info.channel);

    if (rgb_matrix_get_mode() != RGB_MATRIX_CUSTOM_RGBR_PLAY) {
        rgb_matrix_mode(RGB_MATRIX_CUSTOM_RGBR_PLAY);
    }

    return true;
}

// 开始灯效录制
bool rgbrec_start(uint8_t channel) {
    if (channel >= RGBREC_CHANNEL_NUM) {
        return false;
    }

    if (rgbrec_info.state == RGBREC_STATE_OFF) {
        rgbrec_info.state   = RGBREC_STATE_ON;
        rgbrec_info.channel = channel;
        rgbrec_read_current_channel(rgbrec_info.channel);
        if (rgb_matrix_get_mode() != RGB_MATRIX_CUSTOM_RGBR_PLAY) {
            rgb_matrix_mode(RGB_MATRIX_CUSTOM_RGBR_PLAY);
        }

        return true;
    }

    return false;
}

// 停止灯效录制
bool rgbrec_end(uint8_t channel) {

    if (channel >= RGBREC_CHANNEL_NUM) {
        return false;
    }

    if ((rgbrec_info.state == RGBREC_STATE_ON) && (channel == rgbrec_info.channel)) {
        rgbrec_info.state = RGBREC_STATE_OFF;
        rgbrec_update_current_channel(rgbrec_info.channel);

        return true;
    }

    return false;
}

// 灯效录制播放
void rgbrec_play(uint8_t led_min, uint8_t led_max) {
    uint8_t row = 0xFF, col = 0xFF;
    uint16_t hs_color;

    rgbrec_info.value = rgb_matrix_get_val();

    for (uint8_t i = led_min; i < led_max; i++) {
        if (find_matrix_row_col(i, &row, &col)) {
            if (p_rgbrec_effects != NULL) {
                hs_color = p_rgbrec_effects[row][col];
                rgb_matrix_set_hs(i, hs_color);
            }
        }
    }
}

// 设定所有等下录制hsv
void rgbrec_set_close_all(uint8_t h, uint8_t s, uint8_t v) {

    if (!h && !s && !v) {
        memset(rgbrec_buffer, 0, sizeof(rgbrec_buffer));
    } else {
        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
            for (uint8_t col = 0; col < MATRIX_COLS; col++) {
                rgbrec_buffer[row * MATRIX_COLS * col * 2]       = s;
                rgbrec_buffer[(row * MATRIX_COLS * col * 2) + 1] = v;
            }
        }
    }
}

// 手动注册一个灯效录制操作
bool rgbrec_register_record(uint16_t keycode, keyrecord_t *record) {
    (void)keycode;

    if (rgbrec_info.state == RGBREC_STATE_ON) {
        cycle_rgb_next_color(record->event.key.row, record->event.key.col);
        return true;
    }

    return false;
}

// 从一块地址中更新当前通道
void rgbrec_update_from_buffer(const uint8_t *effect, uint32_t size, uint8_t channel) {

    if (size != sizeof(rgbrec_buffer)) {
        return;
    }

    memcpy(rgbrec_buffer, effect, size);
    rgbrec_update_current_channel(channel);
}

// 获取当前灯效录制是否是开启状态
bool rgbrec_is_started(void) {
    return (rgbrec_info.state == RGBREC_STATE_ON);
}

/* --- via --- */

uint32_t rgbrec_calc_address(uint8_t channel, uint8_t row, uint8_t column) {
    uint32_t addr = 0x00;

    addr = (uint32_t)(RGBREC_EECONFIG_ADDR) + (channel * sizeof(rgbrec_buffer)) + ((row * MATRIX_COLS + column) * 2);

    return addr;
}

// 获取支持的最多通道数
uint8_t rgbrec_get_channle_num(void) {
    return (uint8_t)RGBREC_CHANNEL_NUM;
}

// 切换到当前通道
__WEAK void rgbrec_switch_channel(uint8_t channel) {
    if (channel >= RGBREC_CHANNEL_NUM) {
        return;
    }

    rgbrec_read_current_channel(channel); // 从eeprom中读取数据 覆盖原来没有保存的灯效录制
    rgbrec_end(channel);                  // 结束录制并保存到eeprom中
    rgbrec_show(channel);
}

// 获取单个矩阵位置hs值
uint16_t rgbrec_get_hs_data(uint8_t channel, uint8_t row, uint8_t column) {
    if (channel >= RGBREC_CHANNEL_NUM || row >= MATRIX_ROWS || column >= MATRIX_COLS) {
        return 0x0000;
    }

    void *address = (uint32_t *)rgbrec_calc_address(channel, row, column);

    // Little endian
    uint16_t hs = eeprom_read_byte(address);
    hs |= eeprom_read_byte(address + 1) << 8;
    return hs;
}

// 设定单个矩阵位置hs值
void rgbrec_set_hs_data(uint8_t channel, uint8_t row, uint8_t column, uint16_t hs) {
    if (channel >= RGBREC_CHANNEL_NUM || row >= MATRIX_ROWS || column >= MATRIX_COLS) {
        return;
    }

    void *address = (uint32_t *)rgbrec_calc_address(channel, row, column);

    // Little endian
    eeprom_update_byte(address, (uint8_t)(hs & 0xFF));
    eeprom_update_byte(address + 1, (uint8_t)(hs >> 8));
}

// 获取单个矩阵位置hs值
void rgbrec_get_hs_buffer(uint16_t offset, uint16_t size, uint8_t *data) {
    uint16_t hs_total_size = RGBREC_CHANNEL_NUM * MATRIX_ROWS * MATRIX_COLS * 2;
    void *source           = (void *)(RGBREC_EECONFIG_ADDR + offset);
    uint8_t *target        = data;
    for (uint16_t i = 0; i < size; i++) {
        if (offset + i < hs_total_size) {
            *target = eeprom_read_byte(source);
        } else {
            *target = 0x00;
        }
        source++;
        target++;
    }
}

// 设定单个矩阵位置hs值
void rgbrec_set_hs_buffer(uint16_t offset, uint16_t size, uint8_t *data) {
    uint16_t hs_total_size = RGBREC_CHANNEL_NUM * MATRIX_ROWS * MATRIX_COLS * 2;
    void *target           = (void *)(RGBREC_EECONFIG_ADDR + offset);
    uint8_t *source        = data;
    for (uint16_t i = 0; i < size; i++) {
        if ((offset + i) < hs_total_size) {
            eeprom_update_byte(target, *source);
        }
        source++;
        target++;
    }
}

/* --- eeconfig --- */

// 将当前通道录制的灯效录制数据存储到eeprom
void rgbrec_update_current_channel(uint8_t channel) {
    uint32_t addr = 0;

    if (channel >= RGBREC_CHANNEL_NUM) {
        return;
    }

    addr = (uint32_t)(RGBREC_EECONFIG_ADDR) + (channel * sizeof(rgbrec_buffer));
    eeprom_update_block(rgbrec_buffer, (void *)addr, sizeof(rgbrec_buffer));
}

// 从eeprom中获取保存的灯效录制数据
void rgbrec_read_current_channel(uint8_t channel) {
    uint32_t addr = 0;

    if (channel >= RGBREC_CHANNEL_NUM) {
        return;
    }

    addr = (uint32_t)(RGBREC_EECONFIG_ADDR) + (channel * sizeof(rgbrec_buffer));
    eeprom_read_block(rgbrec_buffer, (void *)addr, sizeof(rgbrec_buffer));
}

const uint8_t dummy_user[EECONFIG_USER_DATA_SIZE] = {0};
// 恢复出厂设置时，将默认灯效存储到eeprom
void eeconfig_init_user_datablock(void) {

    // // 用户存储区的首地址需要和灯效录制存储首地址一致
    // if (RGBREC_EECONFIG_ADDR == EECONFIG_USER_DATABLOCK) {
    //     memcpy(dummy_user, rgbrec_default_effects, sizeof(rgbrec_default_effects));
    // }

    eeconfig_update_user_datablock(dummy_user);
    eeprom_update_block(rgbrec_default_effects, RGBREC_EECONFIG_ADDR, sizeof(rgbrec_default_effects));
}

/* qmk retarget */
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    return rgbrec_process_record(keycode, record);
}
