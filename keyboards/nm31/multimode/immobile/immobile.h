// Copyright 2024 JoyLee (@itarze)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "quantum.h"

#ifndef IM_LKEY_COUNT
#    define IM_LKEY_COUNT 1
#endif

#define MAC_TOG         AG_TOGG
#define keymap_is_mac() (keymap_config.swap_lalt_lgui != false)
#define keymap_mac_toggle()                                           \
    do {                                                              \
        keymap_config.swap_lalt_lgui = !keymap_config.swap_lalt_lgui; \
        keymap_config.swap_ralt_rgui = keymap_config.swap_lalt_lgui;  \
        eeconfig_update_keymap(keymap_config.raw);                    \
    } while (0)

// 无线相关固定按键值, 面向终端用户
enum im_keys {
    IM_BT1 = QK_KB_0,
    IM_BT2,
    IM_BT3,
    IM_2G4,
    IM_USB,
    KC_LED,
    HS_SIRI,
    HS_TOGG,
    MM_BATQ,
    CONTROL,
    HS_LPAD,
    IM_BT4,
    IM_BT5,
    IM_BATQ,
    RL_TOG,
    RL_MOD,
    RL_RMOD,
    RL_VAI,
    RL_VAD,
    RL_HUI,
    RL_HUD,
    RL_SAI,
    RL_SAD,
    IM_TUON,
    IM_TUOF,
    IM_TUTG,
    IM_USER, // 暴露给终端用户使用
};

// 内部使用按键值
enum extra_keys {
    EX_VERS = QK_USER, // 版本打印
    EX_RATE,           // 回报率测试
    EX_LPEN,           // 立即进入低功耗模式
    EX_TRGB,           // RGB 测试模式
    EX_USER,           // 暴露给用户使用
};

typedef enum {
    mlrs_none = 0,
    mlrs_lback,
    mlrs_pair,
    mlrs_lback_succeed,
    mlrs_pair_succeed,
    mlrs_lback_timeout,
    mlrs_pair_timeout,
    mlrs_user_fields,
} mm_linker_rgb_t;

typedef struct {
    uint32_t press_time;
    uint32_t hole_time;
    uint16_t keycode;
    uint8_t upvalid;
} im_lkey_t;

extern im_lkey_t lkey_define_user[IM_LKEY_COUNT];

extern void mm_set_rgb_enable(bool state);
extern bool mm_get_rgb_enable(void);

// Led 引脚初始化函数
extern bool im_led_init_user(void);
// Led 引脚复位函数
extern bool im_led_deinit_user(void);

// 恢复出厂设置的函数
extern void im_reset_settings(void);
extern bool im_reset_settings_user(void);

// 上电时初始化函数
extern bool im_pre_init_user(void);
// 系统参数配置完毕后的初始化函数
extern bool im_init_user(void);

// 主循环函数
extern bool im_loop_user(void);

// 按键记录处理函数
extern bool im_process_record_user(uint16_t keycode, keyrecord_t *record);
// 长按按键处理函数
extern bool im_lkey_process_user(uint16_t keycode, bool pressed);

// RGBLIGHT 处理函数
extern bool rgb_matrix_indicators_advanced_rgblight(uint8_t led_min, uint8_t led_max);

// 设定当前无线状态rgb blink的状态
void im_mm_rgb_blink_set_state(mm_linker_rgb_t state);
// 获取当前无线状态rgb blink的状态
mm_linker_rgb_t im_mm_rgb_blink_get_state(void);
// 设定当前无线状态rgb blink的执行时间
void im_mm_rgb_blink_set_timer(uint32_t time);
// 获取当前无线状态rgb blink的执行时间
uint32_t im_mm_rgb_blink_get_timer(void);
// 设定无线状态修改时的RGB状态
extern bool im_mm_set_rgb_blink(uint8_t index, mm_linker_rgb_t state);
// 设定无线状态修改时的RGB状态用户函数
extern bool im_mm_set_rgb_blink_user(uint8_t index, mm_linker_rgb_t state);
// rgb blink 结束时的Hook函数
extern void im_mm_rgb_blink_hook(uint8_t index, mm_linker_rgb_t state);

// 无线断连时的RGB状态控制
extern void im_mm_rgb_blink_auto_lback_user(void);
// 无线状态rgb blink 暴露给用户的执行函数
extern bool im_mm_rgb_blink_hook_user(uint8_t index, mm_linker_rgb_t state);
// 无线状态rgb blink暴露给 rgb blink的回调函数
extern void im_mm_rgb_blink_cb(uint8_t index);

// 特定按键开机的处理
extern bool im_lp_special_wakeup_process_user(uint16_t keycode, keyrecord_t *record);
// 获取开关机状态
extern bool im_get_power_flag(void);
// 设定开机
extern void im_set_power_on(void);
// 设定关机
extern void im_set_power_off(void);
// 翻转开关机
extern void im_set_power_toggle(void);

extern bool mm_is_to_sleep_user(void);
extern bool mm_is_to_sleep_kb(void);

// 休眠时与唤醒时的用户函数
extern bool im_sleep_kb(void);
extern bool im_sleep_user(void);

// 超时无操作休眠条件
bool im_is_allow_timeout_user(void);
// 设定低电量关机
void im_set_lowpower(bool state);
// 获取当前低电量关机状态
bool im_get_lowpower(void);
// 翻转低电量关机
void im_toggle_lowpower(void);
// 设定关机状态
void im_set_shutdown(bool state);
// 获取当前关机状态
bool im_get_shutdown(void);
// 翻转关机状态
void im_toggle_shutdown(void);
