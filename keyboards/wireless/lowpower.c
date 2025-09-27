// Copyright 2024 Su (@isuua)
// Copyright 2024 JoyLee (@itarze)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "quantum.h"
#include "wireless.h"
#include "usb_main.h"

#ifndef LPWR_TIMEOUT
#    define LPWR_TIMEOUT 300000 // 5min
#endif

#ifndef LPWR_PRESLEEP_DELAY
#    define LPWR_PRESLEEP_DELAY 200
#endif

#ifndef LPWR_STOP_DELAY
#    define LPWR_STOP_DELAY 200
#endif

#ifndef LPWR_WAKEUP_DELAY
#    define LPWR_WAKEUP_DELAY 200
#endif

static lpwr_state_t lpwr_state       = LPWR_NORMAL;
static lpwr_mode_t lpwr_mode         = LPWR_MODE_TIMEOUT;
static uint32_t lpwr_timeout_value   = LPWR_TIMEOUT;
static uint32_t lpwr_timestamp       = 0x00;
static lpwr_wakeupcd_t lpwr_wakeupcd = LPWR_WAKEUP_NONE;
static bool manual_timeout           = false;
static bool manual_shutdown          = false;
static bool manual_lowbat            = false;

static bool rgb_enable_bak = false;

extern void last_matrix_activity_trigger(void);
extern void lpwr_enter_stop(lpwr_mode_t mode);
extern void lpwr_exti_init(lpwr_mode_t mode);
extern void mcu_stop_mode(void);
extern void lpwr_clock_enable(void);
extern void matrix_init_pins(void);

void lpwr_enter_stop(lpwr_mode_t mode) {

    chSysLock();
    lpwr_exti_init(mode);
    chSysUnlock();

    chSysDisable();
    mcu_stop_mode();
    lpwr_clock_enable();
    matrix_init_pins();
    chSysEnable();
}

lpwr_state_t lpwr_get_state(void) {
    return lpwr_state;
}

void lpwr_set_state(lpwr_state_t state) {
    lpwr_state = state;
}

lpwr_mode_t lpwr_get_mode(void) {
    return lpwr_mode;
}

void lpwr_set_mode(lpwr_mode_t mode) {
    lpwr_mode = mode;
}

void lpwr_set_timeout_value(uint32_t timeout) {
    lpwr_timeout_value = timeout;
}

uint32_t lpwr_timeout_value_read(void) {
    return lpwr_timeout_value;
}

void lpwr_update_timestamp(void) {
    lpwr_timestamp = sync_timer_read32();
}

uint32_t lpwr_timestamp_read(void) {
    return lpwr_timestamp;
}

void lpwr_set_wakeupcd(lpwr_wakeupcd_t wakeupcd) {
    lpwr_wakeupcd = wakeupcd;
}

lpwr_wakeupcd_t lpwr_get_wakeupcd(void) {
    return lpwr_wakeupcd;
}

void lpwr_set_manual_timeout(bool enable) {
    manual_timeout = enable;
}

void lpwr_set_manual_shutdown(bool enable) {
    manual_shutdown = enable;
}

void lpwr_set_manual_lowbat(bool enable) {
    manual_lowbat = enable;
}

bool lpwr_is_allow_timeout_hook(void) __attribute__((weak));
bool lpwr_is_allow_timeout_hook(void) {
    return true;
}

bool lpwr_is_allow_timeout(void) __attribute__((weak));
bool lpwr_is_allow_timeout(void) {
    uint32_t timeout = lpwr_timeout_value;

    if (lpwr_is_allow_timeout_hook() != true) {
        manual_timeout = false;
        return false;
    }

    if ((wireless_get_devs().now == DEVS_USB) && (USB_DRIVER.state == USB_ACTIVE)) {
        manual_timeout = false;
        return false;
    }

    if (manual_timeout || (timeout && (last_input_activity_elapsed() >= timeout))) {
        manual_timeout = false;
        return true;
    }

    return false;
}

bool lpwr_is_allow_shutdown_hook(void) __attribute__((weak));
bool lpwr_is_allow_shutdown_hook(void) {
    return true;
}

bool lpwr_is_allow_shutdown(void) __attribute__((weak));
bool lpwr_is_allow_shutdown(void) {

    if (lpwr_is_allow_shutdown_hook() != true) {
        manual_shutdown = false;
        return false;
    }

    if (manual_shutdown) {
        manual_shutdown = false;

        if ((md_get_devs() != DEVS_USB) && (*md_getp_state() == MD_STATE_CONNECTED)) {
            wireless_clear_keys();
            wireless_wait_md_free();
        }

        return true;
    }

    return false;
}

bool lpwr_is_allow_lowbat_hook(void) __attribute__((weak));
bool lpwr_is_allow_lowbat_hook(void) {
    return true;
}

bool lpwr_is_allow_lowbat(void) __attribute__((weak));
bool lpwr_is_allow_lowbat(void) {

    if (lpwr_is_allow_lowbat_hook() != true) {
        manual_lowbat = false;
        return false;
    }

    if (manual_lowbat) {
        manual_lowbat = false;

        if ((md_get_devs() != DEVS_USB) && (*md_getp_state() == MD_STATE_CONNECTED)) {
            wireless_clear_keys();
            wireless_wait_md_free();
        }

        return true;
    }

    return false;
}

bool lpwr_is_allow_presleep_hook(void) __attribute__((weak));
bool lpwr_is_allow_presleep_hook(void) {
    return true;
}

bool lpwr_is_allow_presleep(void) __attribute__((weak));
bool lpwr_is_allow_presleep(void) {
    uint32_t delay = LPWR_PRESLEEP_DELAY;

    if (lpwr_is_allow_presleep_hook() != true) {
        return false;
    }

    if (!delay || (sync_timer_elapsed32(lpwr_timestamp_read()) >= delay)) {
        extern matrix_row_t raw_matrix[MATRIX_ROWS];
        matrix_row_t empty[MATRIX_ROWS] = {0};
        bool allow                      = true;

        if (lpwr_mode == LPWR_MODE_TIMEOUT) {
            allow = !memcmp(raw_matrix, empty, sizeof(empty));
        }

        return allow;
    }

    return false;
}

bool lpwr_is_allow_stop_hook(void) __attribute__((weak));
bool lpwr_is_allow_stop_hook(void) {
    return true;
}

bool lpwr_is_allow_stop(void) __attribute__((weak));
bool lpwr_is_allow_stop(void) {
    uint32_t delay = LPWR_STOP_DELAY;

    if (lpwr_is_allow_stop_hook() != true) {
        return false;
    }

    if (!delay || (sync_timer_elapsed32(lpwr_timestamp_read()) >= delay)) {
        return md_is_free();
    }

    return false;
}

bool lpwr_is_allow_wakeup_hook(void) __attribute__((weak));
bool lpwr_is_allow_wakeup_hook(void) {
    return true;
}

bool lpwr_is_allow_wakeup(void) __attribute__((weak));
bool lpwr_is_allow_wakeup(void) {
    uint32_t delay = LPWR_WAKEUP_DELAY;

    if (lpwr_is_allow_wakeup_hook() != true) {
        return false;
    }

    if (!delay || (sync_timer_elapsed32(lpwr_timestamp_read()) >= delay)) {
        return true;
    }

    return false;
}

void lpwr_presleep_hook(void) __attribute__((weak));
void lpwr_presleep_hook(void) {}

void lpwr_presleep_cb(void) __attribute__((weak));
void lpwr_presleep_cb(void) {

#if defined(RGB_MATRIX_ENABLE)
    rgb_enable_bak = rgb_matrix_is_enabled();
    rgb_matrix_disable_noeeprom();
#elif defined(RGBLIGHT_ENABLE)
    rgb_enable_bak = rgblight_is_enabled();
    rgblight_disable_noeeprom();
#else
    rgb_enable_bak = false;
#endif
    suspend_power_down();
    lpwr_presleep_hook();
}

void lpwr_pre_stop_hook(void) __attribute__((weak));
void lpwr_pre_stop_hook(void) {}

void lpwr_post_stop_hook(void) __attribute__((weak));
void lpwr_post_stop_hook(void) {}

void lpwr_stop_cb(void) __attribute__((weak));
void lpwr_stop_cb(void) {

    // 非连接状态切换模块进入USB模式，降低功耗
    if (((md_get_devs() != DEVS_USB) && (*md_getp_state() != MD_STATE_CONNECTED)) || lpwr_mode == LPWR_MODE_LOWBAT) {
        md_switch_to_usb();
        wireless_wait_md_free();
        for (uint32_t i = 0; i < 400; i++) {
            wireless_wait_md_free();
            wait_ms(1);
        }
    }

    lpwr_pre_stop_hook();
    lpwr_set_wakeupcd(LPWR_WAKEUP_NONE);
    lpwr_enter_stop(lpwr_mode);

    switch (lpwr_mode) {
        case LPWR_MODE_TIMEOUT: {
            switch (lpwr_wakeupcd) {
                case LPWR_WAKEUP_NONE:
                case LPWR_WAKEUP_RTC:
                case LPWR_WAKEUP_UART: {
                    lpwr_set_state(LPWR_STOP);
                } break;
                default: {
                    lpwr_set_state(LPWR_WAKEUP);
                } break;
            }
        } break;
        case LPWR_MODE_SHUTDOWN: {
            switch (lpwr_wakeupcd) {
                case LPWR_WAKEUP_CABLE:
                case LPWR_WAKEUP_ONEKEY:
                case LPWR_WAKEUP_SWITCH: {
                    lpwr_set_state(LPWR_WAKEUP);
                } break;
                default: {
                    lpwr_set_state(LPWR_STOP);
                } break;
            }
        } break;
        case LPWR_MODE_LOWBAT: {
            switch (lpwr_wakeupcd) {
                case LPWR_WAKEUP_CABLE: {
                    lpwr_set_state(LPWR_WAKEUP);
                } break;
                default: {
                    lpwr_set_state(LPWR_STOP);
                } break;
            }
        } break;
        default:
            break;
    }

    lpwr_post_stop_hook();
}

void lpwr_wakeup_hook(void) __attribute__((weak));
void lpwr_wakeup_hook(void) {}

void lpwr_wakeup_cb(void) __attribute__((weak));
void lpwr_wakeup_cb(void) {
    extern matrix_row_t matrix_previous[MATRIX_ROWS];

    if (rgb_enable_bak) {
#if defined(RGB_MATRIX_ENABLE)
        rgb_matrix_enable_noeeprom();
#elif defined(RGBLIGHT_ENABLE)
        rgblight_enable_noeeprom();
#endif
    }

    suspend_wakeup_init();
    lpwr_wakeup_hook();
    matrix_init();
    memset(matrix_previous, 0, sizeof(matrix_previous));

    last_matrix_activity_trigger();
}

void lpwr_task(void) __attribute__((weak));
void lpwr_task(void) {

    switch (lpwr_get_state()) {
        case LPWR_NORMAL: {
            if (lpwr_is_allow_timeout()) {
                lpwr_update_timestamp();
                lpwr_set_mode(LPWR_MODE_TIMEOUT);
                lpwr_set_state(LPWR_PRESLEEP);
            }

            if (lpwr_is_allow_shutdown()) {
                lpwr_update_timestamp();
                lpwr_set_mode(LPWR_MODE_SHUTDOWN);
                lpwr_set_state(LPWR_PRESLEEP);
            }

            if (lpwr_is_allow_lowbat()) {
                lpwr_update_timestamp();
                lpwr_set_mode(LPWR_MODE_LOWBAT);
                lpwr_set_state(LPWR_PRESLEEP);
            }
        } break;
        case LPWR_PRESLEEP: {
            if (lpwr_is_allow_presleep()) {
                lpwr_update_timestamp();
                lpwr_presleep_cb();
                lpwr_set_state(LPWR_STOP);
            }
        } break;
        case LPWR_STOP: {
            if (lpwr_is_allow_stop()) {
                lpwr_update_timestamp();
                lpwr_stop_cb();
            }
        } break;
        case LPWR_WAKEUP: {
            if (lpwr_is_allow_wakeup()) {
                lpwr_update_timestamp();
                lpwr_wakeup_cb();
                lpwr_set_state(LPWR_NORMAL);
                lpwr_set_mode(LPWR_MODE_TIMEOUT);
            }
        } break;
        default:
            break;
    }
}
