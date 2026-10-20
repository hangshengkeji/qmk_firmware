// Copyright 2024 JoyLee (@itarze)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "immobile.h"

#ifdef MULTIMODE_ENABLE
#    include "multimode.h"
#endif

#ifdef MULTIMODE_ENABLE
#    include "usb_main.h"
#endif

#ifdef RGB_MATRIX_BLINK_ENABLE
#    include "rgb_matrix_blink.h"
#endif

#ifdef SPLIT_KEYBOARD
#    include "transactions.h"
#endif

#ifdef IM_DEBUG_ENABLE
#    ifndef IM_DEBUG_INFO
#        define IM_DEBUG_INFO(fmt, ...) dprintf(fmt, ##__VA_ARGS__)
#    endif
#else
#    define IM_DEBUG_INFO(fmt, ...)
#endif

#ifndef RGB_DRIVER_EN_STATE
#    define RGB_DRIVER_EN_STATE 1
#endif

uint32_t keys_count        = 0x00;
bool im_bat_req_flag       = false;
uint8_t im_test_rgb_state  = 0x00;
uint32_t im_test_rgb_timer = 0x00;

static bool im_led_init_flag  = false;
static bool im_sleep_lowpower = false;
static bool im_sleep_shutdown = false;

static bool reset_settings_flag = false;

bool im_sleep_process_user(uint16_t keycode, keyrecord_t *record);
/************************* DEF USER API *****************************/

#define LED_DEINIT_STE setPinInputLow
#if LED_PIN_ON_STATE == 0
#    undef LED_DEINIT_STE
#    define LED_DEINIT_STE setPinInputHigh
#endif

// led pin init
__WEAK bool im_led_init_user(void) {
    return true;
}

void led_blink_deinit(void) {
#ifdef LED_BLINK_KB_PIN_1
    LED_DEINIT_STE(LED_BLINK_KB_PIN_1);
#endif
#ifdef LED_BLINK_KB_PIN_2
    LED_DEINIT_STE(LED_BLINK_KB_PIN_2);
#endif
#ifdef LED_BLINK_KB_PIN_3
    LED_DEINIT_STE(LED_BLINK_KB_PIN_3);
#endif
#ifdef LED_BLINK_KB_PIN_4
    LED_DEINIT_STE(LED_BLINK_KB_PIN_4);
#endif
#ifdef LED_BLINK_KB_PIN_5
    LED_DEINIT_STE(LED_BLINK_KB_PIN_5);
#endif
}

void led_deinit(void) {
#ifdef LED_NUM_LOCK_PIN
    LED_DEINIT_STE(LED_NUM_LOCK_PIN);
#endif
#ifdef LED_CAPS_LOCK_PIN
    LED_DEINIT_STE(LED_CAPS_LOCK_PIN);
#endif
#ifdef LED_SCROLL_LOCK_PIN
    LED_DEINIT_STE(LED_SCROLL_LOCK_PIN);
#endif
#ifdef LED_COMPOSE_PIN
    LED_DEINIT_STE(LED_COMPOSE_PIN);
#endif
#ifdef LED_KANA_PIN
    LED_DEINIT_STE(LED_KANA_PIN);
#endif
#ifdef LED_BLINK_ENABLE
    led_blink_deinit();
#endif
}

bool im_led_init_kb(void) {
    if (im_led_init_flag != true) {
        im_led_init_user();
        led_init_ports();
        led_set(host_keyboard_leds());
        im_led_init_flag = true;
    }
    return true;
}

// led pin deinit
__WEAK bool im_led_deinit_user(void) {
    return true;
}

bool im_led_deinit_kb(void) {
    if (im_led_init_flag != false) {
        im_led_deinit_user();
        led_deinit();
        im_led_init_flag = false;
    }
    return true;
}

// 恢复出厂设置的函数
__WEAK bool im_reset_settings_user(void) {
    return true;
}

// recover settings
void im_reset_settings(void) {

    eeconfig_init();
#ifdef RGB_MATRIX_ENABLE
    eeconfig_update_rgb_matrix_default();
#endif
    keymap_config.raw = eeconfig_read_keymap();
#if defined(NKRO_ENABLE) && defined(FORCE_NKRO)
    keymap_config.nkro = 1;
#endif
    eeconfig_update_keymap(keymap_config.raw);
#ifdef RGBLIGHT_ENABLE
    extern bool is_rgblight_initialized;
    extern void rgblight_init(void);
    is_rgblight_initialized = false;
    rgblight_init();
    eeconfig_update_rgblight_default();
    rgblight_enable();
#endif

    if (im_reset_settings_user() != true) {
        return;
    }

    // keyboard_post_init_kb();
}

#ifdef MULTIMODE_ENABLE

void usb_mode_test_report_task(void) {
    extern void host_mouse_send(report_mouse_t * report);

    static uint8_t flip                = 0;
    static report_mouse_t mouse_format = {0};

    switch (flip) {
        case 0: { // 右移
            mouse_format.x = 10;
            mouse_format.y = 0;
            host_mouse_send(&mouse_format);
            flip = 1;
        } break;
        case 1: { // 上移
            mouse_format.x = 0;
            mouse_format.y = -10;
            host_mouse_send(&mouse_format);
            flip = 2;
        } break;
        case 2: { // 左移
            mouse_format.x = -10;
            mouse_format.y = 0;
            host_mouse_send(&mouse_format);
            flip = 3;
        } break;
        case 3: { // 下移
            mouse_format.x = 0;
            mouse_format.y = 10;
            host_mouse_send(&mouse_format);
            flip = 0;
        } break;
        default: {
            flip = 0;
        } break;
    }
}

bool im_test_rate_flag = false;
void mm_bts_task_user(uint8_t devs) {
    if (im_test_rate_flag) {
        if (mm_eeconfig.devs == DEVS_USB) {
            usb_mode_test_report_task();
        } else {
            bts_test_report_rate_task();
        }
    }
}

__WEAK bool im_wakeup_user(void) {
    return true;
}

__WEAK bool im_sleep_user(void) {
    return true;
}

void mm_wakeup_user(void) {
    if (im_led_init_flag != true) {
        im_led_init_kb();
    }

    im_wakeup_user();
}

void mm_sleep_user(void) {
    if (im_led_init_flag != false) {
        im_led_deinit_kb();
    }

    im_mm_rgb_blink_set_state(mlrs_none);
    im_mm_rgb_blink_set_timer(0x00);

    im_sleep_user();
}

#endif

// 预休眠时清除所有按键扫描
void mm_reset_matrix_with_presleep(void) {
    extern void matrix_init(void);
    matrix_init();
#ifndef MATRIX_LKEY_DISABLE
    extern void im_lkey_clear_all(void);
    im_lkey_clear_all();
#endif
    keys_count = 0;
}

// 超时无操作休眠条件，面向用户函数
__WEAK bool im_is_allow_timeout_user(void) {
    return true;
}

bool mm_is_allow_timeout_user(void) {
    return im_is_allow_timeout_user();
}

// 低电量条件
bool mm_is_allow_lowpower(void) {
    return im_sleep_lowpower;
}

// 设定低电量关机
void im_set_lowpower(bool state) {
    im_sleep_lowpower = !!state;
}

void mm_set_lowpower(bool state) {
    im_set_lowpower(state);
}

// 获取当前低电量关机状态
bool im_get_lowpower(void) {
    return im_sleep_lowpower;
}

// 翻转低电量关机
void im_toggle_lowpower(void) {
    im_sleep_lowpower = !im_sleep_lowpower;
}

// 关机条件
bool mm_is_allow_shutdown(void) {

    // if ((mm_eeconfig.devs == DEVS_USB) && (USB_DRIVER.state == USB_ACTIVE)) {
    //     return false;
    // }

    return im_sleep_shutdown;
}

// 设定关机状态
void mm_set_shutdown(bool state) {
    im_set_shutdown(state);
}

// 设定关机状态
void im_set_shutdown(bool state) {

    im_sleep_shutdown = state;

    if (im_sleep_shutdown != true) {
        switch (mm_get_sleep_status()) {
            case mm_state_normal: // 为了可以处理手动唤醒增加此条件
            case mm_state_presleep:
            case mm_state_stop: {
                mm_set_sleep_state(mm_state_wakeup);
                mm_set_sleep_level(mm_sleep_level_timeout);
            } break;
            default:
                break;
        }
    }
}

// 获取当前关机状态
bool im_get_shutdown(void) {
    return im_sleep_shutdown;
}

// 翻转关机状态
void im_toggle_shutdown(void) {
    im_set_shutdown(!im_sleep_shutdown);
}

void im_set_power_on(void) {
    im_set_shutdown(false);
}

void im_set_power_off(void) {
    im_set_shutdown(true);
}

void im_set_power_toggle(void) {
    im_toggle_shutdown();
}

bool im_get_power_flag(void) {
    return !im_get_shutdown();
}

__WEAK bool im_sleep_process_user(uint16_t keycode, keyrecord_t *record) {
    bool rev = true;

    if (im_get_shutdown() != false) {
        rev = false;
    }

    if (im_get_lowpower() != false) {
        rev = false;
    }

    if (IS_QK_MOMENTARY(keycode)) {
        return true;
    }

    return rev;
}

// 按键记录处理函数
__WEAK bool im_process_record_user(uint16_t keycode, keyrecord_t *record) {
    return true;
}

static bool im_process_record_kb(uint16_t keycode, keyrecord_t *record) {

    if (im_process_record_user(keycode, record) != true) {
        return false;
    }

    switch (keycode) {
        case EX_TRGB: {
            return false;
        } break;
#ifdef MULTIMODE_ENABLE
        case EX_VERS: {
            if (record->event.pressed) {
                mm_print_version();
            }
            return false;
        } break;
        case EX_RATE: {
            if (record->event.pressed) {
                im_test_rate_flag = true;
            } else {
                im_test_rate_flag = false;
            }
            return false;
        } break;
        case EX_LPEN: {
            if (record->event.pressed) {
                mm_set_to_sleep();
            }
            return false;
        } break;
        case IM_BATQ: {
            im_bat_req_flag = record->event.pressed;
            return false;
        } break;
        case IM_BT1: {
            if (record->event.pressed && (mm_eeconfig.devs != DEVS_BT1)) {
                uint8_t mode = mio_none;
                mm_modeio_detection(true, &mode);
                if ((mode == mio_bt) || (mode == mio_wireless) || (mode == mio_none)) {
                    mm_switch_mode(mm_eeconfig.devs, DEVS_BT1, false);
                }
            }
            return false;
        } break;
        case IM_BT2: {
            if (record->event.pressed && (mm_eeconfig.devs != DEVS_BT2)) {
                uint8_t mode = mio_none;
                mm_modeio_detection(true, &mode);
                if ((mode == mio_bt) || (mode == mio_wireless) || (mode == mio_none)) {
                    mm_switch_mode(mm_eeconfig.devs, DEVS_BT2, false);
                }
            }
            return false;
        } break;
        case IM_BT3: {
            if (record->event.pressed && (mm_eeconfig.devs != DEVS_BT3)) {
                uint8_t mode = mio_none;
                mm_modeio_detection(true, &mode);
                if ((mode == mio_bt) || (mode == mio_wireless) || (mode == mio_none)) {
                    mm_switch_mode(mm_eeconfig.devs, DEVS_BT3, false);
                }
            }
            return false;
        } break;
        case IM_BT4: {
            if (record->event.pressed && (mm_eeconfig.devs != DEVS_BT4)) {
                uint8_t mode = mio_none;
                mm_modeio_detection(true, &mode);
                if ((mode == mio_bt) || (mode == mio_wireless) || (mode == mio_none)) {
                    mm_switch_mode(mm_eeconfig.devs, DEVS_BT4, false);
                }
            }
            return false;
        } break;
        case IM_BT5: {
            if (record->event.pressed && (mm_eeconfig.devs != DEVS_BT5)) {
                uint8_t mode = mio_none;
                mm_modeio_detection(true, &mode);
                if ((mode == mio_bt) || (mode == mio_wireless) || (mode == mio_none)) {
                    mm_switch_mode(mm_eeconfig.devs, DEVS_BT5, false);
                }
            }
            return false;
        } break;
        case IM_2G4: {
            if (record->event.pressed && (mm_eeconfig.devs != DEVS_2G4)) {
                uint8_t mode = mio_none;
                mm_modeio_detection(true, &mode);
                if ((mode == mio_2g4) || (mode == mio_wireless) || (mode == mio_none)) {
                    mm_switch_mode(mm_eeconfig.devs, DEVS_2G4, false);
                }
            }
            return false;
        } break;
        case IM_USB: {
            if (record->event.pressed && (mm_eeconfig.devs != DEVS_USB)) {
                uint8_t mode = mio_none;
                mm_modeio_detection(true, &mode);
                if ((mode == mio_usb) || (mode == mio_none)) {
                    mm_switch_mode(mm_eeconfig.devs, DEVS_USB, false);
                }
            }
            return false;
        } break;
#endif
#ifdef RGB_MATRIX_ENABLE
        case RGB_TOG: {
#    ifdef RGBLIGHT_ENABLE
#        ifdef RGB_TRIGGER_ON_KEYDOWN
            if (record->event.pressed) {
#        else
            if (!record->event.pressed) {
#        endif
                mm_set_rgb_enable(!mm_get_rgb_enable());

                if (mm_get_rgb_enable()) {
                    // enable
                    rgb_matrix_enable();
#        ifdef RGB_DRIVER_EN_PIN
                    writePin(RGB_DRIVER_EN_PIN, RGB_DRIVER_EN_STATE);
#        endif
#        ifdef RGB_DRIVER_EN2_PIN
                    writePin(RGB_DRIVER_EN2_PIN, RGB_DRIVER_EN_STATE);
#        endif
                } else {
                    if ((rgblight_is_enabled() != true) && (mm_get_rgb_enable() != true)) {
                        rgb_matrix_disable();
#        ifdef RGB_DRIVER_EN_PIN
                        writePin(RGB_DRIVER_EN_PIN, !RGB_DRIVER_EN_STATE);
#        endif
#        ifdef RGB_DRIVER_EN2_PIN
                        writePin(RGB_DRIVER_EN2_PIN, !RGB_DRIVER_EN_STATE);
#        endif
                    }
                }
            }
            return false;
#    else
#        ifdef RGB_TRIGGER_ON_KEYDOWN
            if (record->event.pressed) {
#        else
            if (!record->event.pressed) {
#        endif
#        ifdef RGB_DRIVER_EN_PIN
                if (rgb_matrix_is_enabled()) {
                    // disable
                    writePin(RGB_DRIVER_EN_PIN, !RGB_DRIVER_EN_STATE);
#            ifdef RGB_DRIVER_EN2_PIN
                    writePin(RGB_DRIVER_EN2_PIN, !RGB_DRIVER_EN_STATE);
#            endif
                } else {
                    // enable
                    writePin(RGB_DRIVER_EN_PIN, RGB_DRIVER_EN_STATE);
#            ifdef RGB_DRIVER_EN2_PIN
                    writePin(RGB_DRIVER_EN2_PIN, RGB_DRIVER_EN_STATE);
#            endif
                }
#        endif
            }
            return true;
#    endif
        } break;
#endif
#ifdef RGBLIGHT_ENABLE
        case RL_TOG: {
            if (record->event.pressed) {
                rgblight_toggle();
            }

            if (rgblight_is_enabled()) {
                // enable
                rgb_matrix_enable();
#    ifdef RGB_DRIVER_EN_PIN
                writePin(RGB_DRIVER_EN_PIN, RGB_DRIVER_EN_STATE);
#        ifdef RGB_DRIVER_EN2_PIN
                writePin(RGB_DRIVER_EN2_PIN, RGB_DRIVER_EN_STATE);
#        endif
#    endif
            } else {
                if ((rgblight_is_enabled() != true) && (mm_get_rgb_enable() != true)) {
                    rgb_matrix_disable();
#    ifdef RGB_DRIVER_EN_PIN
                    writePin(RGB_DRIVER_EN_PIN, !RGB_DRIVER_EN_STATE);
#        ifdef RGB_DRIVER_EN2_PIN
                    writePin(RGB_DRIVER_EN2_PIN, !RGB_DRIVER_EN_STATE);
#        endif
#    endif
                }
            }

            return false;
        } break;
        case RL_MOD: {
            if (record->event.pressed) {
                rgblight_increase();
            }
            return false;
        } break;
        case RL_RMOD: {
            if (record->event.pressed) {
                rgblight_decrease();
            }
            return false;
        } break;
        case RL_VAI: {
            if (record->event.pressed) {
                rgblight_increase_val();
            }
            return false;
        } break;
        case RL_VAD: {
            if (record->event.pressed) {
                rgblight_decrease_val();
            }
            return false;
        } break;
        case RL_HUI: {
            if (record->event.pressed) {
                rgblight_increase_hue();
            }
            return false;
        } break;
        case RL_HUD: {
            if (record->event.pressed) {
                rgblight_decrease_hue();
            }
            return false;
        } break;
        case RL_SAI: {
            if (record->event.pressed) {
                rgblight_increase_sat();
            }
            return false;
        } break;
        case RL_SAD: {
            if (record->event.pressed) {
                rgblight_decrease_sat();
            }
            return false;
        } break;
#endif
        case EE_CLR: {
            return false;
        } break;
    }

    return true;
}

#ifndef MATRIX_LKEY_DISABLE

extern im_lkey_t lkey_define[11];
__WEAK im_lkey_t lkey_define[11] = {
    {.keycode = IM_BT1,  .hole_time = 3000},
    {.keycode = IM_BT2,  .hole_time = 3000},
    {.keycode = IM_BT3,  .hole_time = 3000},
    {.keycode = IM_BT4,  .hole_time = 3000},
    {.keycode = IM_BT5,  .hole_time = 3000},
    {.keycode = IM_2G4,  .hole_time = 3000},
    {.keycode = EX_TRGB, .hole_time = 3000},
    {.keycode = EE_CLR,  .hole_time = 3000},
    {.keycode = IM_TUON, .hole_time = 3000},
    {.keycode = IM_TUOF, .hole_time = 3000},
    {.keycode = IM_TUTG, .hole_time = 3000},
};

__WEAK im_lkey_t lkey_define_user[IM_LKEY_COUNT] = {};

// 长按按键处理函数
__WEAK bool im_lkey_process_user(uint16_t keycode, bool pressed) {
    return true;
}

void im_lkey_clear_all(void) {
    for (uint8_t i = 0; i < (sizeof(lkey_define_user) / sizeof(lkey_define_user[0])); i++) {
        lkey_define_user[i].upvalid    = false;
        lkey_define_user[i].press_time = 0;
    }

    for (uint8_t i = 0; i < (sizeof(lkey_define) / sizeof(lkey_define[0])); i++) {
        lkey_define[i].upvalid    = false;
        lkey_define[i].press_time = 0;
    }

    IM_DEBUG_INFO("im_lkey_clear_all");
}

static bool im_lkey_process(uint16_t keycode, bool pressed) {

    if (im_lkey_process_user(keycode, pressed) != true) {
        return false;
    }

    if (pressed) {
        switch (keycode) {
#    ifdef MULTIMODE_ENABLE
            case IM_BT1: {
                uint8_t mode = mio_none;
                mm_modeio_detection(true, &mode);
                if ((mode == mio_bt) || (mode == mio_wireless) || (mode == mio_none)) {
                    mm_switch_mode(mm_eeconfig.devs, DEVS_BT1, true);
                }
            } break;
            case IM_BT2: {
                uint8_t mode = mio_none;
                mm_modeio_detection(true, &mode);
                if ((mode == mio_bt) || (mode == mio_wireless) || (mode == mio_none)) {
                    mm_switch_mode(mm_eeconfig.devs, DEVS_BT2, true);
                }
            } break;
            case IM_BT3: {
                uint8_t mode = mio_none;
                mm_modeio_detection(true, &mode);
                if ((mode == mio_bt) || (mode == mio_wireless) || (mode == mio_none)) {
                    mm_switch_mode(mm_eeconfig.devs, DEVS_BT3, true);
                }
            } break;
            case IM_BT4: {
                uint8_t mode = mio_none;
                mm_modeio_detection(true, &mode);
                if ((mode == mio_bt) || (mode == mio_wireless) || (mode == mio_none)) {
                    mm_switch_mode(mm_eeconfig.devs, DEVS_BT4, true);
                }
            } break;
            case IM_BT5: {
                uint8_t mode = mio_none;
                mm_modeio_detection(true, &mode);
                if ((mode == mio_bt) || (mode == mio_wireless) || (mode == mio_none)) {
                    mm_switch_mode(mm_eeconfig.devs, DEVS_BT5, true);
                }
            } break;
            case IM_2G4: {
                uint8_t mode = mio_none;
                mm_modeio_detection(true, &mode);
                if ((mode == mio_2g4) || (mode == mio_wireless) || (mode == mio_none)) {
                    mm_switch_mode(mm_eeconfig.devs, DEVS_2G4, true);
                }
            } break;
#    endif
            case IM_TUON: {
                im_set_shutdown(false);
            } break;
            case IM_TUOF: {
                im_set_shutdown(true);
            } break;
            case IM_TUTG: {
                im_toggle_shutdown();
            } break;
            case EX_TRGB: {
                im_test_rgb_state = 1;
                im_test_rgb_timer = 0x00;
            } break;
            case EE_CLR: {
                im_reset_settings();
            } break;
            default:
                break;
        }
    }

    return true;
}

static void im_lkey_task(void) {

    for (uint8_t i = 0; i < (sizeof(lkey_define_user) / sizeof(lkey_define_user[0])); i++) {
        if ((lkey_define_user[i].press_time != 0) && !lkey_define_user[i].upvalid &&
            (timer_elapsed32(lkey_define_user[i].press_time) >= lkey_define_user[i].hole_time)) {
            lkey_define_user[i].upvalid = true;
            im_lkey_process(lkey_define_user[i].keycode, true);
        }
    }

    for (uint8_t i = 0; i < (sizeof(lkey_define) / sizeof(lkey_define[0])); i++) {
        if ((lkey_define[i].press_time != 0) && !lkey_define[i].upvalid &&
            (timer_elapsed32(lkey_define[i].press_time) >= lkey_define[i].hole_time)) {
            lkey_define[i].upvalid = true;
            im_lkey_process(lkey_define[i].keycode, true);
        }
    }
}

static bool im_lkey_process_record(uint16_t keycode, keyrecord_t *record) {
    bool retval = true;

    for (uint8_t i = 0; i < (sizeof(lkey_define_user) / sizeof(lkey_define_user[0])); i++) {
        if (keycode == lkey_define_user[i].keycode) {
            if (record->event.pressed) {
                lkey_define_user[i].press_time = timer_read32();
            } else {
                if (lkey_define_user[i].upvalid) {
                    retval                      = im_lkey_process(lkey_define_user[i].keycode, false);
                    lkey_define_user[i].upvalid = false;
                }
                lkey_define_user[i].press_time = 0;
            }
            break;
        }
    }

    if (retval != true) {
        return false;
    }

    for (uint8_t i = 0; i < (sizeof(lkey_define) / sizeof(lkey_define[0])); i++) {
        if (keycode == lkey_define[i].keycode) {
            if (record->event.pressed) {
                lkey_define[i].press_time = timer_read32();
            } else {
                if (lkey_define[i].upvalid) {
                    retval                 = im_lkey_process(lkey_define[i].keycode, false);
                    lkey_define[i].upvalid = false;
                }
                lkey_define[i].press_time = 0;
            }
            break;
        }
    }

    return retval;
}

#endif

#ifdef RGBLIGHT_ENABLE

LED_TYPE led[RGBLED_NUM];

extern rgblight_config_t rgblight_config;
extern const uint8_t led_map[];

void rgblight_call_driver(LED_TYPE *start_led, uint8_t num_leds) {

    memcpy(led, start_led, sizeof(LED_TYPE) * num_leds);
}

void rgblight_set(void) {
    LED_TYPE *start_led;
    uint8_t num_leds = rgblight_ranges.clipping_num_leds;

    if (!rgblight_config.enable) {
        for (uint8_t i = rgblight_ranges.effect_start_pos; i < rgblight_ranges.effect_end_pos; i++) {
            led[i].r = 0;
            led[i].g = 0;
            led[i].b = 0;
        }
    }

#    ifdef RGBLIGHT_LED_MAP
    LED_TYPE led0[RGBLED_NUM];
    for (uint8_t i = 0; i < RGBLED_NUM; i++) {
        led0[i] = led[pgm_read_byte(&led_map[i])];
    }
    start_led = led0 + rgblight_ranges.clipping_start_pos;
#    else
    start_led = led + rgblight_ranges.clipping_start_pos;
#    endif

    rgblight_call_driver(start_led, num_leds);
}

// RGBLIGHT 处理函数
__WEAK bool rgb_matrix_indicators_advanced_rgblight(uint8_t led_min, uint8_t led_max) {

    extern bool mm_get_rgb_enable(void);
    if (mm_get_rgb_enable() != true) {
        rgb_matrix_set_color(0, 0, 0, 0);
        rgb_matrix_set_color(1, 0, 0, 0);
        rgb_matrix_set_color(2, 0, 0, 0);
        rgb_matrix_set_color(36, 0, 0, 0);
        rgb_matrix_set_color(37, 0, 0, 0);
        rgb_matrix_set_color(38, 0, 0, 0);
    }
    else{
        rgb_matrix_set_color(2, led[0].r, led[0].g, led[0].b); // rgb light
        rgb_matrix_set_color(1, led[1].r, led[1].g, led[1].b);
        rgb_matrix_set_color(0, led[2].r, led[2].g, led[2].b);
        rgb_matrix_set_color(36, led[3].r, led[3].g, led[3].b);
        rgb_matrix_set_color(37, led[4].r, led[4].g, led[4].b);
        rgb_matrix_set_color(38, led[5].r, led[5].g, led[5].b);
    }
    return true;
}

#endif

// RGB MATRIX INDICATOR 处理函数
__WEAK bool rgb_matrix_indicators_advanced_indicator(uint8_t led_min, uint8_t led_max) {

#ifndef RGB_MATRIX_INDICATOR_VAL
#    ifdef RGB_MATRIX_MAXIMUM_BRIGHTNESS
#        define RGB_MATRIX_INDICATOR_VAL RGB_MATRIX_MAXIMUM_BRIGHTNESS
#    else
#        define RGB_MATRIX_INDICATOR_VAL 0x77
#    endif
#endif

#if defined(RGB_MATRIX_CPASLOCK_INDEX) || defined(RGB_MATRIX_NUMLOCK_INDEX) || defined(RGB_MATRIX_SCRLOCK_INDEX) || defined(RGB_MATRIX_WINLOCK_INDEX)
    switch (rgb_matrix_get_mode()) {
#    ifdef ENABLE_RGB_MATRIX_BAND_SPIRAL_SAT
        case RGB_MATRIX_BAND_SPIRAL_SAT:
#    endif
#    ifdef ENABLE_RGB_MATRIX_BAND_PINWHEEL_SAT
        case RGB_MATRIX_BAND_PINWHEEL_SAT:
#    endif
#    ifdef ENABLE_RGB_MATRIX_BAND_SAT
        case RGB_MATRIX_BAND_SAT:
#    endif
#    ifdef ENABLE_RGB_MATRIX_DIGITAL_RAIN
        case RGB_MATRIX_DIGITAL_RAIN:
#    endif
#    ifdef ENABLE_RGB_MATRIX_PIXEL_FLOW
        case RGB_MATRIX_PIXEL_FLOW:
#    endif
#    ifdef ENABLE_RGB_MATRIX_PIXEL_RAIN
        case RGB_MATRIX_PIXEL_RAIN:
#    endif
#    ifdef ENABLE_RGB_MATRIX_PIXEL_FRACTAL
        case RGB_MATRIX_PIXEL_FRACTAL:
#    endif
#    ifdef ENABLE_RGB_MATRIX_RAINDROPS
        case RGB_MATRIX_RAINDROPS:
#    endif
#    ifdef ENABLE_RGB_MATRIX_JELLYBEAN_RAINDROPS
        case RGB_MATRIX_JELLYBEAN_RAINDROPS:
#    endif
        case RGB_MATRIX_NONE: {
#    ifdef RGB_MATRIX_CPASLOCK_INDEX
            rgb_matrix_set_color(RGB_MATRIX_CPASLOCK_INDEX, 0x00, 0x00, 0x00);
#    endif
#    ifdef RGB_MATRIX_NUMLOCK_INDEX
            rgb_matrix_set_color(RGB_MATRIX_NUMLOCK_INDEX, 0x00, 0x00, 0x00);
#    endif
#    ifdef RGB_MATRIX_SCRLOCK_INDEX
            rgb_matrix_set_color(RGB_MATRIX_SCRLOCK_INDEX, 0x00, 0x00, 0x00);
#    endif
#    ifdef RGB_MATRIX_WINLOCK_INDEX
            rgb_matrix_set_color(RGB_MATRIX_WINLOCK_INDEX, 0x00, 0x00, 0x00);
#    endif
        } break;
    }

#    ifdef RGB_MATRIX_CPASLOCK_INDEX
    if (host_keyboard_led_state().caps_lock) {
        rgb_matrix_set_color(RGB_MATRIX_CPASLOCK_INDEX, RGB_MATRIX_INDICATOR_VAL, RGB_MATRIX_INDICATOR_VAL, RGB_MATRIX_INDICATOR_VAL);
    }
#    endif

#    ifdef RGB_MATRIX_NUMLOCK_INDEX
    if (host_keyboard_led_state().num_lock) {
        rgb_matrix_set_color(RGB_MATRIX_NUMLOCK_INDEX, RGB_MATRIX_INDICATOR_VAL, RGB_MATRIX_INDICATOR_VAL, RGB_MATRIX_INDICATOR_VAL);
    }
#    endif

#    ifdef RGB_MATRIX_SCRLOCK_INDEX
    if (host_keyboard_led_state().scroll_lock) {
        rgb_matrix_set_color(RGB_MATRIX_SCRLOCK_INDEX, RGB_MATRIX_INDICATOR_VAL, RGB_MATRIX_INDICATOR_VAL, RGB_MATRIX_INDICATOR_VAL);
    }
#    endif

#    ifdef RGB_MATRIX_WINLOCK_INDEX
    if (keymap_config.no_gui) {
        rgb_matrix_set_color(RGB_MATRIX_WINLOCK_INDEX, RGB_MATRIX_INDICATOR_VAL, RGB_MATRIX_INDICATOR_VAL, RGB_MATRIX_INDICATOR_VAL);
    }
#    endif

#endif

    return true;
}

/************************** QMK KB API ******************************/

// 上电时初始化函数
__WEAK bool im_pre_init_user(void) {
    return true;
}

// pre init
void keyboard_pre_init_user(void) {

    im_led_init_kb();

#ifdef MULTIMODE_ENABLE
    mm_init();
#endif

    if (im_pre_init_user() != true) {
        return;
    }

#ifndef RGB_DRIVER_EN_PIN
#    ifdef DRIVER_1_EN
#        define RGB_DRIVER_EN_PIN DRIVER_1_EN
#    endif
#endif

#ifndef RGB_DRIVER_CS1_PIN
#    ifdef DRIVER_1_CS
#        define RGB_DRIVER_CS1_PIN DRIVER_1_CS
#    endif
#endif

#ifndef RGB_DRIVER_CS2_PIN
#    ifdef DRIVER_2_CS
#        define RGB_DRIVER_CS2_PIN DRIVER_2_CS
#    endif
#endif

#ifdef RGB_DRIVER_EN_PIN
    setPinOutput(RGB_DRIVER_EN_PIN);
    writePin(RGB_DRIVER_EN_PIN, RGB_DRIVER_EN_STATE);
#endif

#ifdef RGB_DRIVER_EN2_PIN
    setPinOutput(RGB_DRIVER_EN2_PIN);
    writePin(RGB_DRIVER_EN2_PIN, RGB_DRIVER_EN_STATE);
#endif

#ifdef RGB_DRIVER_CS1_PIN
    setPinOutput(RGB_DRIVER_CS1_PIN);
    writePinHigh(RGB_DRIVER_CS1_PIN);
#endif

#ifdef RGB_DRIVER_CS2_PIN
    setPinOutput(RGB_DRIVER_CS2_PIN);
    writePinHigh(RGB_DRIVER_CS2_PIN);
#endif
}

// 系统参数配置完毕后的初始化函数
__WEAK bool im_init_user(void) {
    return true;
}

// post init
void keyboard_post_init_kb(void) {

#ifdef CONSOLE_ENABLE
    debug_enable = true;
#endif

    im_bat_req_flag   = false;
    im_test_rgb_state = 0x00;
    im_test_rgb_timer = 0x00;

#ifdef RGB_MATRIX_ENABLE
#    if defined(RGBLIGHT_ENABLE) && defined(RGB_DRIVER_EN_PIN)
    if ((rgblight_is_enabled() != true) && (mm_get_rgb_enable() != true)) {
        rgb_matrix_disable();
        writePin(RGB_DRIVER_EN_PIN, !RGB_DRIVER_EN_STATE);
#        ifdef RGB_DRIVER_EN2_PIN
        writePin(RGB_DRIVER_EN2_PIN, !RGB_DRIVER_EN_STATE);
#        endif
    } else {
        writePin(RGB_DRIVER_EN_PIN, RGB_DRIVER_EN_STATE);
#        ifdef RGB_DRIVER_EN2_PIN
        writePin(RGB_DRIVER_EN2_PIN, RGB_DRIVER_EN_STATE);
#        endif
    }
#    elif defined(RGB_DRIVER_EN_PIN)
    if (rgb_matrix_is_enabled() != true) {
        rgb_matrix_disable();
        writePin(RGB_DRIVER_EN_PIN, !RGB_DRIVER_EN_STATE);
#        ifdef RGB_DRIVER_EN2_PIN
        writePin(RGB_DRIVER_EN2_PIN, !RGB_DRIVER_EN_STATE);
#        endif
    } else {
        writePin(RGB_DRIVER_EN_PIN, RGB_DRIVER_EN_STATE);
#        ifdef RGB_DRIVER_EN2_PIN
        writePin(RGB_DRIVER_EN2_PIN, RGB_DRIVER_EN_STATE);
#        endif
    }
#    endif
#endif

    im_init_user();
    keyboard_post_init_user();
}

// 主循环函数
__WEAK bool im_loop_user(void) {
    return true;
}

// loop while
void housekeeping_task_user(void) {

#ifdef MULTIMODE_ENABLE
    mm_task();
#endif

#ifndef MATRIX_LKEY_DISABLE
    im_lkey_task();
#endif

#if defined(WB32IAP_ENABLE)
    void kb_info_hook(void);
    kb_info_hook();
#endif

    if (reset_settings_flag) {
        reset_settings_flag = false;
        while (bts_is_busy()) {};
        im_reset_settings();
    }

    im_loop_user();
}

// suspend wakeup init for usb mode
void suspend_wakeup_init_kb(void) {
    if (im_led_init_flag != true) {
        im_led_init_kb();
    }
#ifdef RGB_DRIVER_EN_PIN
    writePin(RGB_DRIVER_EN_PIN, RGB_DRIVER_EN_STATE);
#endif
#ifdef RGB_DRIVER_EN2_PIN
    writePin(RGB_DRIVER_EN2_PIN, RGB_DRIVER_EN_STATE);
#endif
    suspend_wakeup_init_user();
}

// suspend power down for usb mode
void suspend_power_down_kb(void) {
    if (im_led_init_flag != false) {
        im_led_deinit_kb();
    }
#ifdef RGB_DRIVER_EN_PIN
    writePin(RGB_DRIVER_EN_PIN, !RGB_DRIVER_EN_STATE);
#endif
#ifdef RGB_DRIVER_EN2_PIN
    writePin(RGB_DRIVER_EN2_PIN, !RGB_DRIVER_EN_STATE);
#endif
    suspend_power_down_user();
}

// process record
bool process_record_kb(uint16_t keycode, keyrecord_t *record) {

    keys_count = record->event.pressed ?
                     ((keys_count < sizeof(keys_count)) ? (keys_count + 1) : keys_count) :
                     ((keys_count > 0) ? (keys_count - 1) : keys_count);

    IM_DEBUG_INFO("keys_count = %d, row = %d, col = %d, keycode = 0x%X", keys_count, record->event.key.row, record->event.key.col, keycode);
    mm_update_key_press_time();

#ifndef MATRIX_LKEY_DISABLE
    if (im_lkey_process_record(keycode, record) != true) {
        IM_DEBUG_INFO("im_lkey_process_record return false");
        return false;
    }
#endif

    if (im_sleep_process_user(keycode, record) != true) {
        IM_DEBUG_INFO("im_sleep_process_user return false");
        return false;
    }

    if (im_process_record_kb(keycode, record) != true) {
        IM_DEBUG_INFO("im_process_record_kb return false");
        return false;
    }

    if (process_record_user(keycode, record) != true) {
        IM_DEBUG_INFO("process_record_user return false");
        return false;
    }

    if (process_record_usb_suspend(keycode, record) != true) {
        IM_DEBUG_INFO("process_record_usb_suspend return false");
        return false;
    }

#ifdef MULTIMODE_ENABLE
    if (process_record_multimode(keycode, record) != true) {
        return false;
    }
#endif

    return true;
}

#ifndef IM_MM_LBACK_TIMEOUT
#    define IM_MM_LBACK_TIMEOUT 60000
#endif

#ifndef IM_MM_LBACK_INTERVAL
#    define IM_MM_LBACK_INTERVAL 500
#endif

#ifndef IM_MM_PAIR_TIMEOUT
#    define IM_MM_PAIR_TIMEOUT 60000
#endif

#ifndef IM_MM_PAIR_INTERVAL
#    define IM_MM_PAIR_INTERVAL 200
#endif

#ifndef IM_MM_SUCCEED_TIME
#    define IM_MM_SUCCEED_TIME 2000
#endif

#ifndef IM_MM_USB_TIMES
#    define IM_MM_USB_TIMES 0xFF
#endif

#ifndef IM_MM_LBACK_COLOR_BT1
#    define IM_MM_LBACK_COLOR_BT1 0x00, 0xFF, 0x00
#endif

#ifndef IM_MM_LBACK_COLOR_BT2
#    define IM_MM_LBACK_COLOR_BT2 0x00, 0xFF, 0x00
#endif

#ifndef IM_MM_LBACK_COLOR_BT3
#    define IM_MM_LBACK_COLOR_BT3 0x00, 0xFF, 0x00
#endif

#ifndef IM_MM_LBACK_COLOR_BT4
#    define IM_MM_LBACK_COLOR_BT4 0x00, 0xFF, 0x00
#endif

#ifndef IM_MM_LBACK_COLOR_BT5
#    define IM_MM_LBACK_COLOR_BT5 0x00, 0xFF, 0x00
#endif

#ifndef IM_MM_LBACK_COLOR_2G4
#    define IM_MM_LBACK_COLOR_2G4 0x00, 0xFF, 0x00
#endif

#ifndef IM_MM_PAIR_COLOR_BT1
#    define IM_MM_PAIR_COLOR_BT1 0xFF, 0x00, 0x00
#endif

#ifndef IM_MM_PAIR_COLOR_BT2
#    define IM_MM_PAIR_COLOR_BT2 0xFF, 0x00, 0x00
#endif

#ifndef IM_MM_PAIR_COLOR_BT3
#    define IM_MM_PAIR_COLOR_BT3 0xFF, 0x00, 0x00
#endif

#ifndef IM_MM_PAIR_COLOR_BT4
#    define IM_MM_PAIR_COLOR_BT4 0xFF, 0x00, 0x00
#endif

#ifndef IM_MM_PAIR_COLOR_BT5
#    define IM_MM_PAIR_COLOR_BT5 0xFF, 0x00, 0x00
#endif

#ifndef IM_MM_PAIR_COLOR_2G4
#    define IM_MM_PAIR_COLOR_2G4 0xFF, 0x00, 0x00
#endif

#ifndef IM_MM_LBACK_COLOR_USB
#    define IM_MM_LBACK_COLOR_USB 0x00, 0x00, 0x00
#endif

#ifndef IM_MM_PAIR_COLOR_USB
#    define IM_MM_PAIR_COLOR_USB 0xFF, 0xFF, 0xFF
#endif

static uint8_t mm_linker_rgb_state  = mlrs_none;
static uint32_t mm_linker_rgb_timer = 0x00;

void im_mm_rgb_blink_set_state(mm_linker_rgb_t state) {
    mm_linker_rgb_state = state;
}

mm_linker_rgb_t im_mm_rgb_blink_get_state(void) {
    return mm_linker_rgb_state;
}

void im_mm_rgb_blink_set_timer(uint32_t time) {
    mm_linker_rgb_timer = time;
}

uint32_t im_mm_rgb_blink_get_timer(void) {
    return mm_linker_rgb_timer;
}

__WEAK bool im_mm_rgb_blink_hook_user(uint8_t index, mm_linker_rgb_t state) {
    return true;
}

void im_mm_rgb_blink_hook(uint8_t index, mm_linker_rgb_t state) {

    if (im_mm_rgb_blink_hook_user(index, state) != true) {
        return;
    }

#if defined(RGB_MATRIX_BLINK_ENABLE) && defined(MULTIMODE_ENABLE)
    switch (state) {
        case mlrs_none: {
            im_mm_rgb_blink_set_timer(0x00);
        } break;
        case mlrs_lback: {
            if (im_mm_rgb_blink_get_timer() == 0x00) {
                uint32_t times = 0;
                switch (index) {
#    ifdef IM_MM_RGB_BLINK_INDEX_MIXED
                    case IM_MM_RGB_BLINK_INDEX_MIXED: {
                        switch (mm_eeconfig.devs) {
                            case DEVS_BT1: {
                                rgb_matrix_blink_set_color(index, IM_MM_LBACK_COLOR_BT1);
                                times = 1;
                            } break;
                            case DEVS_BT2: {
                                rgb_matrix_blink_set_color(index, IM_MM_LBACK_COLOR_BT2);
                                times = 1;
                            } break;
                            case DEVS_BT3: {
                                rgb_matrix_blink_set_color(index, IM_MM_LBACK_COLOR_BT3);
                                times = 1;
                            } break;
                            case DEVS_BT4: {
                                rgb_matrix_blink_set_color(index, IM_MM_LBACK_COLOR_BT4);
                                times = 1;
                            } break;
                            case DEVS_BT5: {
                                rgb_matrix_blink_set_color(index, IM_MM_LBACK_COLOR_BT5);
                                times = 1;
                            } break;
                            case DEVS_2G4: {
                                rgb_matrix_blink_set_color(index, IM_MM_LBACK_COLOR_2G4);
                                times = 1;
                            } break;
#        ifndef IM_MM_RGB_BLINK_DISABLE_USB
                            case DEVS_USB: {
                                rgb_matrix_blink_set_color(index, IM_MM_LBACK_COLOR_USB);
                                times = IM_MM_USB_TIMES;
                            } break;
#        endif
                        }
                    } break;
#    else
#        ifdef IM_MM_RGB_BLINK_INDEX_BT1
                    case IM_MM_RGB_BLINK_INDEX_BT1: {
                        rgb_matrix_blink_set_color(index, IM_MM_LBACK_COLOR_BT1);
                        times = 1;
                    } break;
#        endif
#        ifdef IM_MM_RGB_BLINK_INDEX_BT2
                    case IM_MM_RGB_BLINK_INDEX_BT2: {
                        rgb_matrix_blink_set_color(index, IM_MM_LBACK_COLOR_BT2);
                        times = 1;
                    } break;
#        endif
#        ifdef IM_MM_RGB_BLINK_INDEX_BT3
                    case IM_MM_RGB_BLINK_INDEX_BT3: {
                        rgb_matrix_blink_set_color(index, IM_MM_LBACK_COLOR_BT3);
                        times = 1;
                    } break;
#        endif
#        ifdef IM_MM_RGB_BLINK_INDEX_BT4
                    case IM_MM_RGB_BLINK_INDEX_BT4: {
                        rgb_matrix_blink_set_color(index, IM_MM_LBACK_COLOR_BT4);
                        times = 1;
                    } break;
#        endif
#        ifdef IM_MM_RGB_BLINK_INDEX_BT5
                    case IM_MM_RGB_BLINK_INDEX_BT5: {
                        rgb_matrix_blink_set_color(index, IM_MM_LBACK_COLOR_BT5);
                        times = 1;
                    } break;
#        endif
#        ifdef IM_MM_RGB_BLINK_INDEX_2G4
                    case IM_MM_RGB_BLINK_INDEX_2G4: {
                        rgb_matrix_blink_set_color(index, IM_MM_LBACK_COLOR_2G4);
                        times = 1;
                    } break;
#        endif
#        ifdef IM_MM_RGB_BLINK_INDEX_USB
                    case IM_MM_RGB_BLINK_INDEX_USB: {
                        rgb_matrix_blink_set_color(index, IM_MM_LBACK_COLOR_USB);
                        times = IM_MM_USB_TIMES;
                    } break;
#        endif
#    endif
                    default:
                        break;
                }
                rgb_matrix_blink_set_interval_times(index, IM_MM_LBACK_INTERVAL, times);
                rgb_matrix_blink_set(index);
                im_mm_rgb_blink_set_timer(timer_read32());
            } else {
                if (timer_elapsed32(im_mm_rgb_blink_get_timer()) >= IM_MM_LBACK_TIMEOUT) {
                    im_mm_rgb_blink_set_state(mlrs_lback_timeout);
                    im_mm_rgb_blink_hook(index, im_mm_rgb_blink_get_state());
                } else {
#    if defined(IM_MM_RGB_BLINK_INDEX_MIXED) && !defined(IM_MM_RGB_BLINK_DISABLE_USB)
                    if ((mm_eeconfig.devs == DEVS_USB) && (index == IM_MM_RGB_BLINK_INDEX_MIXED)) {
                        if (USB_DRIVER.state == USB_ACTIVE) {
                            im_mm_rgb_blink_set_state(mlrs_lback_succeed);
                            rgb_matrix_blink_set_color(index, IM_MM_PAIR_COLOR_USB);
                            im_mm_rgb_blink_hook(index, im_mm_rgb_blink_get_state());
                        } else {
                            rgb_matrix_blink_set(index);
                        }
                        break;
                    }
#    elif defined(IM_MM_RGB_BLINK_INDEX_USB)
                    if ((mm_eeconfig.devs == DEVS_USB) && (index == IM_MM_RGB_BLINK_INDEX_USB)) {
                        if (USB_DRIVER.state == USB_ACTIVE) {
                            im_mm_rgb_blink_set_state(mlrs_lback_succeed);
                            rgb_matrix_blink_set_color(index, IM_MM_PAIR_COLOR_USB);
                            im_mm_rgb_blink_hook(index, im_mm_rgb_blink_get_state());
                        } else {
                            rgb_matrix_blink_set(index);
                        }
                        break;
                    }
#    endif
                    if (bts_info.bt_info.paired) {
                        im_mm_rgb_blink_set_state(mlrs_lback_succeed);
                        im_mm_rgb_blink_hook(index, im_mm_rgb_blink_get_state());
                    } else {
                        if (mm_eeconfig.devs != DEVS_USB) {
                            rgb_matrix_blink_set(index);
                        } else {
                            im_mm_rgb_blink_set_state(mlrs_none);
                        }
                    }
                }
            }
        } break;
        case mlrs_pair: {
            if (im_mm_rgb_blink_get_timer() == 0x00) {
                switch (index) {
#    ifdef IM_MM_RGB_BLINK_INDEX_MIXED
                    case IM_MM_RGB_BLINK_INDEX_MIXED: {
                        switch (mm_eeconfig.devs) {
                            case DEVS_BT1: {
                                rgb_matrix_blink_set_color(index, IM_MM_PAIR_COLOR_BT1);
                            } break;
                            case DEVS_BT2: {
                                rgb_matrix_blink_set_color(index, IM_MM_PAIR_COLOR_BT2);
                            } break;
                            case DEVS_BT3: {
                                rgb_matrix_blink_set_color(index, IM_MM_PAIR_COLOR_BT3);
                            } break;
                            case DEVS_BT4: {
                                rgb_matrix_blink_set_color(index, IM_MM_PAIR_COLOR_BT4);
                            } break;
                            case DEVS_BT5: {
                                rgb_matrix_blink_set_color(index, IM_MM_PAIR_COLOR_BT5);
                            } break;
                            case DEVS_2G4: {
                                rgb_matrix_blink_set_color(index, IM_MM_PAIR_COLOR_2G4);
                            } break;
                            default:
                                return;
                        }
                    } break;
#    else
#        ifdef IM_MM_RGB_BLINK_INDEX_BT1
                    case IM_MM_RGB_BLINK_INDEX_BT1: {
                        rgb_matrix_blink_set_color(index, IM_MM_PAIR_COLOR_BT1);
                    } break;
#        endif
#        ifdef IM_MM_RGB_BLINK_INDEX_BT2
                    case IM_MM_RGB_BLINK_INDEX_BT2: {
                        rgb_matrix_blink_set_color(index, IM_MM_PAIR_COLOR_BT2);
                    } break;
#        endif
#        ifdef IM_MM_RGB_BLINK_INDEX_BT3
                    case IM_MM_RGB_BLINK_INDEX_BT3: {
                        rgb_matrix_blink_set_color(index, IM_MM_PAIR_COLOR_BT3);
                    } break;
#        endif
#        ifdef IM_MM_RGB_BLINK_INDEX_BT4
                    case IM_MM_RGB_BLINK_INDEX_BT4: {
                        rgb_matrix_blink_set_color(index, IM_MM_PAIR_COLOR_BT4);
                    } break;
#        endif
#        ifdef IM_MM_RGB_BLINK_INDEX_BT5
                    case IM_MM_RGB_BLINK_INDEX_BT5: {
                        rgb_matrix_blink_set_color(index, IM_MM_PAIR_COLOR_BT5);
                    } break;
#        endif
#        ifdef IM_MM_RGB_BLINK_INDEX_2G4
                    case IM_MM_RGB_BLINK_INDEX_2G4: {
                        rgb_matrix_blink_set_color(index, IM_MM_PAIR_COLOR_2G4);
                    } break;
#        endif
#    endif
                    default:
                        return;
                }
                rgb_matrix_blink_set_interval_times(index, IM_MM_PAIR_INTERVAL, 1);
                rgb_matrix_blink_set(index);
                im_mm_rgb_blink_set_timer(timer_read32());
            } else {
                if (timer_elapsed32(im_mm_rgb_blink_get_timer()) >= IM_MM_PAIR_TIMEOUT) {
                    im_mm_rgb_blink_set_state(mlrs_pair_timeout);
                    im_mm_rgb_blink_hook(index, im_mm_rgb_blink_get_state());
                } else {
                    if (bts_info.bt_info.paired) {
                        im_mm_rgb_blink_set_state(mlrs_pair_succeed);
                        im_mm_rgb_blink_hook(index, im_mm_rgb_blink_get_state());
                    } else {
                        if (mm_eeconfig.devs != DEVS_USB) {
                            rgb_matrix_blink_set(index);
                        } else {
                            im_mm_rgb_blink_set_state(mlrs_none);
                        }
                    }
                }
            }
        } break;
        case mlrs_lback_succeed:
        case mlrs_pair_succeed: {
            rgb_matrix_blink_set_interval_times(index, IM_MM_SUCCEED_TIME, 0xFF);
            rgb_matrix_blink_set(index);
            im_mm_rgb_blink_set_state(mlrs_none);
        } break;
        case mlrs_lback_timeout:
        case mlrs_pair_timeout: {
            im_mm_rgb_blink_set_state(mlrs_none);
            mm_set_to_sleep();
            rgb_matrix_blink_set(index);
        } break;
        default:
            break;
    }
#endif
}

__WEAK void im_mm_rgb_blink_cb(uint8_t index) {

    im_mm_rgb_blink_hook(index, im_mm_rgb_blink_get_state());
}

__WEAK bool im_mm_set_rgb_blink_user(uint8_t index, mm_linker_rgb_t state) {
    return true;
}

bool im_mm_set_rgb_blink(uint8_t index, mm_linker_rgb_t state) {
    bool retval = false;

#ifdef RGB_MATRIX_BLINK_ENABLE

#    ifdef IM_MM_RGB_BLINK_INDEX_MIXED
    retval = rgb_matrix_blink_set_remain_time(IM_MM_RGB_BLINK_INDEX_MIXED, 0x00);
    if (retval != true) {
        return false;
    }
#    else
#        ifdef IM_MM_RGB_BLINK_INDEX_BT1
    retval = rgb_matrix_blink_set_remain_time(IM_MM_RGB_BLINK_INDEX_BT1, 0x00);
    if (retval != true) {
        return false;
    }
#        endif
#        ifdef IM_MM_RGB_BLINK_INDEX_BT2
    retval = rgb_matrix_blink_set_remain_time(IM_MM_RGB_BLINK_INDEX_BT2, 0x00);
    if (retval != true) {
        return false;
    }
#        endif
#        ifdef IM_MM_RGB_BLINK_INDEX_BT3
    retval = rgb_matrix_blink_set_remain_time(IM_MM_RGB_BLINK_INDEX_BT3, 0x00);
    if (retval != true) {
        return false;
    }
#        endif
#        ifdef IM_MM_RGB_BLINK_INDEX_BT4
    retval = rgb_matrix_blink_set_remain_time(IM_MM_RGB_BLINK_INDEX_BT4, 0x00);
    if (retval != true) {
        return false;
    }
#        endif
#        ifdef IM_MM_RGB_BLINK_INDEX_BT5
    retval = rgb_matrix_blink_set_remain_time(IM_MM_RGB_BLINK_INDEX_BT5, 0x00);
    if (retval != true) {
        return false;
    }
#        endif
#        ifdef IM_MM_RGB_BLINK_INDEX_2G4
    retval = rgb_matrix_blink_set_remain_time(IM_MM_RGB_BLINK_INDEX_2G4, 0x00);
    if (retval != true) {
        return false;
    }
#        endif
#        ifdef IM_MM_RGB_BLINK_INDEX_USB
    retval = rgb_matrix_blink_set_remain_time(IM_MM_RGB_BLINK_INDEX_USB, 0x00);
    if (retval != true) {
        return false;
    }
#        endif
#    endif

    retval = im_mm_set_rgb_blink_user(index, state);
    if (retval != true) {
        return false;
    }

    im_mm_rgb_blink_set_state(state);
    im_mm_rgb_blink_set_timer(0x00);

    im_mm_rgb_blink_hook(index, state);

#endif
    return retval;
}

#ifdef MULTIMODE_ENABLE

bool mm_switch_mode_kb(uint8_t last_mode, uint8_t now_mode, uint8_t reset) {

    if (mm_switch_mode_user(last_mode, now_mode, reset) != true) {
        return false;
    }

#    ifdef RGB_MATRIX_ENABLE
    if ((now_mode != DEVS_USB) && (bts_info.bt_info.paired != true)) {
#        ifdef IM_MM_RGB_BLINK_INDEX_MIXED
        switch (now_mode) {
            case DEVS_USB:
            case DEVS_BT1:
            case DEVS_BT2:
            case DEVS_BT3:
            case DEVS_BT4:
            case DEVS_BT5:
            case DEVS_2G4: {
                if (reset) {
                    im_mm_set_rgb_blink(IM_MM_RGB_BLINK_INDEX_MIXED, mlrs_pair);
                } else {
                    im_mm_set_rgb_blink(IM_MM_RGB_BLINK_INDEX_MIXED, mlrs_lback);
                }
            } break;
        }
#        else
        switch (now_mode) {
#            ifdef IM_MM_RGB_BLINK_INDEX_BT1
            case DEVS_BT1: {
                if (reset) {
                    im_mm_set_rgb_blink(IM_MM_RGB_BLINK_INDEX_BT1, mlrs_pair);
                } else {
                    im_mm_set_rgb_blink(IM_MM_RGB_BLINK_INDEX_BT1, mlrs_lback);
                }
            } break;
#            endif
#            ifdef IM_MM_RGB_BLINK_INDEX_BT2
            case DEVS_BT2: {
                if (reset) {
                    im_mm_set_rgb_blink(IM_MM_RGB_BLINK_INDEX_BT2, mlrs_pair);
                } else {
                    im_mm_set_rgb_blink(IM_MM_RGB_BLINK_INDEX_BT2, mlrs_lback);
                }
            } break;
#            endif
#            ifdef IM_MM_RGB_BLINK_INDEX_BT3
            case DEVS_BT3: {
                if (reset) {
                    im_mm_set_rgb_blink(IM_MM_RGB_BLINK_INDEX_BT3, mlrs_pair);
                } else {
                    im_mm_set_rgb_blink(IM_MM_RGB_BLINK_INDEX_BT3, mlrs_lback);
                }
            } break;
#            endif
#            ifdef IM_MM_RGB_BLINK_INDEX_BT4
            case DEVS_BT4: {
                if (reset) {
                    im_mm_set_rgb_blink(IM_MM_RGB_BLINK_INDEX_BT4, mlrs_pair);
                } else {
                    im_mm_set_rgb_blink(IM_MM_RGB_BLINK_INDEX_BT4, mlrs_lback);
                }
            } break;
#            endif
#            ifdef IM_MM_RGB_BLINK_INDEX_BT5
            case DEVS_BT5: {
                if (reset) {
                    im_mm_set_rgb_blink(IM_MM_RGB_BLINK_INDEX_BT5, mlrs_pair);
                } else {
                    im_mm_set_rgb_blink(IM_MM_RGB_BLINK_INDEX_BT5, mlrs_lback);
                }
            } break;
#            endif
#            ifdef IM_MM_RGB_BLINK_INDEX_2G4
            case DEVS_2G4: {
                if (reset) {
                    im_mm_set_rgb_blink(IM_MM_RGB_BLINK_INDEX_2G4, mlrs_pair);
                } else {
                    im_mm_set_rgb_blink(IM_MM_RGB_BLINK_INDEX_2G4, mlrs_lback);
                }
            } break;
#            endif
            default:
                break;
        }
#        endif
    }

    // if ((now_mode == DEVS_USB) && (USB_DRIVER.state != USB_ACTIVE)) {
    if (now_mode == DEVS_USB) {
        switch (now_mode) {
            case DEVS_USB: {
                if (reset) {
#        if defined(IM_MM_RGB_BLINK_INDEX_MIXED) && !defined(IM_MM_RGB_BLINK_DISABLE_USB)
                    im_mm_set_rgb_blink(IM_MM_RGB_BLINK_INDEX_MIXED, mlrs_pair);
#        elif defined(IM_MM_RGB_BLINK_INDEX_USB)
                    im_mm_set_rgb_blink(IM_MM_RGB_BLINK_INDEX_USB, mlrs_pair);
#        endif
                } else {
#        if defined(IM_MM_RGB_BLINK_INDEX_MIXED) && !defined(IM_MM_RGB_BLINK_DISABLE_USB)
                    im_mm_set_rgb_blink(IM_MM_RGB_BLINK_INDEX_MIXED, mlrs_lback);
#        elif defined(IM_MM_RGB_BLINK_INDEX_USB)
                    im_mm_set_rgb_blink(IM_MM_RGB_BLINK_INDEX_USB, mlrs_lback);
#        endif
                }
            } break;
        }
    }
#    endif

    return true;
}
#endif

__WEAK void im_mm_rgb_blink_auto_lback_user(void) {
#ifdef MULTIMODE_ENABLE
    if ((im_mm_rgb_blink_get_state() == mlrs_none) && (mm_get_sleep_status() == mm_state_normal)) {
        if (mm_eeconfig.devs != DEVS_USB) {
            if (bts_info.bt_info.paired != true) {
                mm_switch_mode(DEVS_USB, mm_eeconfig.last_devs, false);
            }
        } else if (mm_eeconfig.devs == DEVS_USB) {
#    if defined(IM_MM_RGB_BLINK_INDEX_USB) || (defined(IM_MM_RGB_BLINK_INDEX_MIXED) && !defined(IM_MM_RGB_BLINK_DISABLE_USB))
            if (USB_DRIVER.state != USB_ACTIVE) {
                mm_switch_mode(DEVS_USB, DEVS_USB, false);
            }
#    endif
        }
    }
#endif
}

__WEAK void im_rgb_matrix_indicators_advanced_pre(void) {}

#ifdef RGB_MATRIX_ENABLE
bool rgb_matrix_indicators_advanced_kb(uint8_t led_min, uint8_t led_max) {

    im_rgb_matrix_indicators_advanced_pre();

    im_mm_rgb_blink_auto_lback_user();

#    ifndef RGB_MATRIX_TEST_VAL
#        ifdef RGB_MATRIX_MAXIMUM_BRIGHTNESS
#            define RGB_MATRIX_TEST_VAL RGB_MATRIX_MAXIMUM_BRIGHTNESS
#        else
#            define RGB_MATRIX_TEST_VAL 0x77
#        endif
#    endif

#    ifndef RGB_TEST1_COLOR
#        define RGB_TEST1_COLOR RGB_MATRIX_TEST_VAL, 0x00, 0x00
#    endif

#    ifndef RGB_TEST2_COLOR
#        define RGB_TEST2_COLOR 0x00, RGB_MATRIX_TEST_VAL, 0x00
#    endif

#    ifndef RGB_TEST3_COLOR
#        define RGB_TEST3_COLOR 0x00, 0x00, RGB_MATRIX_TEST_VAL
#    endif

#    ifndef RGB_TEST4_COLOR
#        define RGB_TEST4_COLOR RGB_MATRIX_TEST_VAL, RGB_MATRIX_TEST_VAL, RGB_MATRIX_TEST_VAL
#    endif

#    ifndef RGB_MATRIX_TEST_TIME
#        define RGB_MATRIX_TEST_TIME 5000
#    endif

    if (im_test_rgb_state) {

        if (!im_test_rgb_timer) {
            im_test_rgb_timer = timer_read32();
        }

        if (timer_elapsed32(im_test_rgb_timer) >= (RGB_MATRIX_TEST_TIME)) {
            im_test_rgb_state += 1;
            im_test_rgb_timer = timer_read32();
        }

        switch (im_test_rgb_state) {
            case 1: {
                rgb_matrix_set_color_all(RGB_TEST1_COLOR);
            } break;
            case 2: {
                rgb_matrix_set_color_all(RGB_TEST2_COLOR);
            } break;
            case 3: {
                rgb_matrix_set_color_all(RGB_TEST3_COLOR);
            } break;
            case 4: {
                rgb_matrix_set_color_all(RGB_TEST4_COLOR);
            } break;
            case 5:
            default: {
                im_test_rgb_state = 0x00;
                im_test_rgb_timer = 0x00;
            } break;
        }

        return false;
    }

#    ifndef RGB_MATRIX_BAT_VAL
#        define RGB_MATRIX_BAT_VAL RGB_MATRIX_TEST_VAL
#    endif

#    ifndef RGB_MATRIX_BAT_INDEX_MAP
#        define RGB_MATRIX_BAT_INDEX_MAP \
            { 17, 18, 19, 20, 21, 22, 23, 24, 25, 26 }
#    endif

#    ifndef IM_BAT_REQ_LEVEL1_VAL
#        define IM_BAT_REQ_LEVEL1_VAL 50
#    endif

#    ifndef IM_BAT_REQ_LEVEL2_VAL
#        define IM_BAT_REQ_LEVEL2_VAL 30
#    endif

#    ifndef IM_BAT_REQ_LEVEL1_COLOR
#        define IM_BAT_REQ_LEVEL1_COLOR 0x00, RGB_MATRIX_BAT_VAL, 0x00
#    endif

#    ifndef IM_BAT_REQ_LEVEL2_COLOR
#        define IM_BAT_REQ_LEVEL2_COLOR RGB_MATRIX_BAT_VAL, RGB_MATRIX_BAT_VAL, 0x00
#    endif

#    ifndef IM_BAT_REQ_LEVEL3_COLOR
#        define IM_BAT_REQ_LEVEL3_COLOR RGB_MATRIX_BAT_VAL, 0x00, 0x00
#    endif

#    ifdef MULTIMODE_ENABLE
    if (im_bat_req_flag) {
#        ifdef IM_RGB_BAT_REQ_CLOSE_ALL
#            ifdef RGBLIGHT_ENABLE
        for (uint8_t i = 0; i < (RGB_MATRIX_LED_COUNT - RGBLED_NUM); i++) {
            rgb_matrix_set_color(i, 0, 0, 0);
        }
#            else
        rgb_matrix_set_color_all(0x00, 0x00, 0x00);
#            endif
#        endif
        for (uint8_t i = 0; i < 10; i++) {
            uint8_t mi_index[10] = RGB_MATRIX_BAT_INDEX_MAP;
            if (i < (bts_info.bt_info.pvol / 10)) {
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
#    endif

#    ifdef RGBLIGHT_ENABLE
    if (rgb_matrix_indicators_advanced_rgblight(led_min, led_max) != true) {
        return false;
    }
#    endif

    if (rgb_matrix_indicators_advanced_indicator(led_min, led_max) != true) {
        return false;
    }

    if (rgb_matrix_indicators_advanced_user(led_min, led_max) != true) {
        return false;
    }

#    ifdef RGB_MATRIX_BLINK_ENABLE
    rgb_matrix_blink_task(led_min, led_max);
#    endif

    return true;
}
#endif

#ifdef RAW_ENABLE

#    include "via.h"
#    include "raw_hid.h"

bool via_command_user(uint8_t *data, uint8_t length) {
    uint8_t *command_id   = &(data[0]);
    uint8_t *command_data = &(data[1]);
    bool retval           = true;

    (void)command_data;

    // 防止通讯和上位机通讯过程中超时休眠
    mm_update_key_press_time();

    // 处理在休眠过程中上位机发来数据，立即唤醒
    if (mm_get_sleep_status() != mm_state_normal) {
        mm_set_sleep_state(mm_state_wakeup);
    }

    switch (*command_id) {
        case id_eeprom_reset: {
            reset_settings_flag = true;
        } break;
        default: {
            retval = false;
        } break;
    }

    if (retval) {
        if (mm_eeconfig.devs == DEVS_USB) {
            raw_hid_send(data, length);
        } else {
            bts_send_via(data, length);
        }
    }

    return retval;
}

#endif

#if defined(WB32IAP_ENABLE)
extern uint32_t __ram0_end__;
#    define SYMVAL(sym)                  (uint32_t)(((uint8_t *)&(sym)) - ((uint8_t *)0))
#    define BOOTLOADER_MAGIC             0xDEADBEEF
#    define MAGIC_ADDR                   (unsigned long *)(SYMVAL(__ram0_end__) - 4)
#    define RTC_BOOTLOADER_JUST_UPLOADED 0x4A379CEB
void bootloader_jump(void) {
    PWR->GPREG0 = RTC_BOOTLOADER_JUST_UPLOADED;
    *MAGIC_ADDR = BOOTLOADER_MAGIC; // set magic flag => reset handler will jump into boot loader
    NVIC_SystemReset();
}
#endif

#if defined(WB32IAP_ENABLE)
#    pragma GCC push_options
#    pragma GCC optimize("O0")

typedef struct { // length must be <= 63
    uint8_t reserved;
    uint8_t version[15];
    uint16_t pid;
    uint16_t vid;
    uint8_t name[22];
    uint8_t manufacturer[21];
} __attribute__((packed)) kb_info_t;

const kb_info_t kb_info __attribute__((section(".flash1"))) = {
    .version      = STR(KB_VERSION),
    .pid          = PRODUCT_ID,
    .vid          = VENDOR_ID,
    .name         = PRODUCT,
    .manufacturer = MANUFACTURER,
};

void kb_info_hook(void) {
    uint32_t temp = (uint32_t)&kb_info;

    (void)temp;
}
#    pragma GCC pop_options

#endif

#include "usb_device_state.h"

const char *usb_device_state_str[] = {
    "USB_DEVICE_STATE_NO_INIT",
    "USB_DEVICE_STATE_INIT",
    "USB_DEVICE_STATE_CONFIGURED",
    "USB_DEVICE_STATE_SUSPEND",
};

void notify_usb_device_state_change_kb(enum usb_device_state usb_device_state) {

    switch (usb_device_state) {
        case USB_DEVICE_STATE_CONFIGURED: {
            switch (mm_get_sleep_status()) {
                case mm_state_presleep:
                case mm_state_stop: {
                    mm_set_sleep_state(mm_state_wakeup);
                    mm_set_sleep_level(mm_sleep_level_timeout);
                } break;
                default:
                    break;
            }
        } break;
        case USB_DEVICE_STATE_INIT: {
            keyboard_protocol = 1;
        } break;
        default:
            break;
    }

    IM_DEBUG_INFO("[USB] change state: [%s]", usb_device_state_str[usb_device_state]);

    notify_usb_device_state_change_user(usb_device_state);
}

void restart_usb_driver(USBDriver *usbp) {

#if USB_SUSPEND_WAKEUP_DELAY > 0
    // Some hubs, kvm switches, and monitors do
    // weird things, with USB device state bouncing
    // around wildly on wakeup, yielding race
    // conditions that can corrupt the keyboard state.
    //
    // Pause for a while to let things settle...
    wait_ms(USB_SUSPEND_WAKEUP_DELAY);
#endif
}

/******************************** END *******************************/
