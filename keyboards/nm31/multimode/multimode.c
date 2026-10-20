// Copyright 2024 JoyLee (@itarze)
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H

#include "quantum.h"
#include "uart.h"
#include "report.h"
#include "usb_main.h"
#include "eeprom.h"
#include "multimode.h"
#include "usb_descriptor_common.h"

#ifdef RGB_RECORD_ENABLE
#    include "rgb_record.h"
#endif

#ifndef MM_EXE_TIME
#    define MM_EXE_TIME 50 // us
#endif

#ifndef MM_READY_TIME
#    define MM_READY_TIME 2000 // ms
#endif

#ifndef MM_BT_SLEEP_EN
#    define MM_BT_SLEEP_EN true
#endif

#ifndef MM_2G4_SLEEP_EN
#    define MM_2G4_SLEEP_EN true
#endif

#ifndef MM_BT1_NAME
#    define MM_BT1_NAME "WB KB 1"
#endif

#ifndef MM_BT2_NAME
#    define MM_BT2_NAME "WB KB 2"
#endif

#ifndef MM_BT3_NAME
#    define MM_BT3_NAME "WB KB 3"
#endif

#ifndef MM_BT4_NAME
#    define MM_BT4_NAME "WB KB 4"
#endif

#ifndef MM_BT5_NAME
#    define MM_BT5_NAME "WB KB 5"
#endif

#ifndef MM_USB_EN_STATE
#    define MM_USB_EN_STATE 1
#endif

#ifndef RGB_DRIVER_EN_STATE
#    define RGB_DRIVER_EN_STATE 1
#endif

#ifndef MM_BAT_REQ_TIME
#    define MM_BAT_REQ_TIME 3000
#endif

#ifndef MM_DONGLE_MANUFACTURER
#    define MM_DONGLE_MANUFACTURER MANUFACTURER
#endif

#ifndef MM_DONGLE_PRODUCT
#    define MM_DONGLE_PRODUCT PRODUCT " Dongle"
#endif

#ifdef MM_DEBUG_ENABLE
#    ifndef MM_DEBUG_INFO
#        define MM_DEBUG_INFO(fmt, ...) dprintf(fmt, ##__VA_ARGS__)
#    endif
#else
#    define MM_DEBUG_INFO(fmt, ...)
#endif

#define MATRIX_SPLIT_IS_MASTER() true

#ifdef SPLIT_KEYBOARD
#    undef MATRIX_SPLIT_IS_MASTER
#    define MATRIX_SPLIT_IS_MASTER() is_keyboard_master()
#endif

#ifdef MM_DEBUG_ENABLE
const char *mm_sleep_status_str[] = {
    "mm_state_normal",
    "mm_state_presleep",
    "mm_state_stop",
    "mm_state_wakeup",
};

const char *mm_sleep_level_str[] = {
    "mm_sleep_level_timeout",
    "mm_sleep_level_shutdown",
    "mm_sleep_level_lowpower",
};

const char *mm_wakeupcd_str[] = {
    "mm_wakeup_none",
    "mm_wakeup_matrix",
    "mm_wakeup_uart",
    "mm_wakeup_cable",
    "mm_wakeup_usb",
    "mm_wakeup_onekey",
    "mm_wakeup_encoder",
    "mm_wakeup_switch",
};

#endif

typedef struct {
    mm_sleep_status_t status;     // 当前休眠状态
    mm_sleep_status_t laststatus; // 上次休眠状态
    mm_sleep_level_t level;       // 当前休眠等级
    mm_wakeupcd_t wakeupcd;       // 唤醒条件
    uint32_t timestamp;           // 记录每次切换状态的时间戳
    uint32_t key_press_time;      // 按键上次按下时间

    bool (*is_allow_timeout)(void);  // 满足超时休眠的条件
    bool (*is_allow_lowpower)(void); // 满足低电量休眠的条件
    bool (*is_allow_shutdown)(void); // 满足关机休眠的条件

    bool (*is_allow_presleep)(void); // 满足预休眠条件
    bool (*is_allow_stop)(void);     // 满足进入STOP条件
    bool (*is_allow_wakeup)(void);   // 满足进入唤醒条件
} mm_sleep_info_t;

// globals
mm_eeconfig_t mm_eeconfig;
bts_info_t bts_info = {
    .bt_name        = {MM_BT1_NAME, MM_BT2_NAME, MM_BT3_NAME, MM_BT4_NAME, MM_BT5_NAME},
    .uart_init      = uart_init,
    .uart_read      = uart_read,
    .uart_transmit  = uart_transmit,
    .uart_receive   = uart_receive,
    .uart_available = uart_available,
    .timer_read32   = timer_read32,
    .bts_pcstate_cb = mm_pcstate_cb,
};

typedef struct {
    uint8_t buttons;
    int8_t x;
    int8_t y;
    int8_t z;
    int8_t h;
} __attribute__((packed)) mm_report_mouse_t;

static mm_sleep_info_t mm_sleep_info = {
    .status            = mm_state_normal,        // 默认正常工作模式
    .laststatus        = mm_state_normal,        // 默认正常工作模式
    .level             = mm_sleep_level_timeout, // 默认超时休眠
    .wakeupcd          = mm_wakeup_none,         // 默认无唤醒方式
    .timestamp         = 0x00,                   // 时间戳默认为0
    .key_press_time    = 0x00,                   // 上次按键按下时间戳
    .is_allow_timeout  = mm_is_allow_timeout,
    .is_allow_lowpower = mm_is_allow_lowpower,
    .is_allow_shutdown = mm_is_allow_shutdown,

    .is_allow_presleep = mm_is_allow_presleep,
    .is_allow_stop     = mm_is_allow_stop,
    .is_allow_wakeup   = mm_is_allow_wakeup,
};

extern uint32_t keys_count;
extern host_driver_t chibios_driver;
extern void rs_updater_usb_suspend_timer(void);
void mm_send_mouse(report_mouse_t *report);
bool mm_modeio_detection(bool update, uint8_t *mode);

// local
static uint32_t bt_init_time = 0;
static bool mm_sleep_manual  = false;
static bool jumpboot_flag    = false;
static void mm_sleep_task(void);

#ifndef MULTIMODE_THREAD_DISABLE
static THD_WORKING_AREA(waThread1, 128);
static THD_FUNCTION(Thread1, arg) {
    (void)arg;

    chRegSetThreadName("multimode");

    while (true) {
        bts_task(mm_eeconfig.devs);
        mm_bts_task_kb(mm_eeconfig.devs);
        chThdSleepMicroseconds(MM_EXE_TIME);
    }
}
#endif

// user-defined overridable functions
__WEAK void mm_init_user(void) {}

__WEAK void mm_init_kb(void) {

#ifdef MM_MODE_SW_PIN
    setPinInputHigh(MM_MODE_SW_PIN);
#endif

#ifdef MM_BT_DEF_PIN
    setPinInputHigh(MM_BT_DEF_PIN);
#endif

#ifdef MM_2G4_DEF_PIN
    setPinInputHigh(MM_2G4_DEF_PIN);
#endif

#ifdef MM_BAT_CABLE_PIN
    setPinInput(MM_BAT_CABLE_PIN);
#endif

    mm_init_user();
    waitInputPinDelay();
}

__WEAK void mm_bts_task_user(uint8_t devs) {}

__WEAK void mm_bts_task_kb(uint8_t devs) {
    mm_bts_task_user(devs);
}

void eeconfig_update_multimode_default(void) {
    dprintf("eeconfig_update_multimode_default\n");
    mm_eeconfig.devs          = DEVS_USB;
    mm_eeconfig.last_devs     = DEVS_BT1;
    mm_eeconfig.last_btdevs   = DEVS_BT1;
    mm_eeconfig.sleep_timeout = mm_sleep_timeout_vendor;
    bt_init_time              = timer_read32();
    eeconfig_update_kb(mm_eeconfig.raw);
}

void eeconfig_debug_multimode(void) {
    dprintf("eeconfig_debug_multimode EEPROM\n");
    dprintf("mm_eeconfig.devs = %d\n", mm_eeconfig.devs);
    dprintf("mm_eeconfig.last_devs = %d\n", mm_eeconfig.last_devs);
    dprintf("mm_eeconfig.last_btdevs = %d\n", mm_eeconfig.last_btdevs);
}

void keyboard_pre_init_kb(void) {

#ifdef MM_USB_EN_PIN
    setPinOutput(MM_USB_EN_PIN);
    writePin(MM_USB_EN_PIN, !MM_USB_EN_STATE);
#endif

    keyboard_pre_init_user();
}

void mm_init(void) {
    wait_ms(200);
#ifdef GPIO_UART_ENABLE
    wait_ms(3000);
    iprint_init();
#endif

    bts_init(&bts_info);
    bts_info.bt_info.active = true;

    mm_eeconfig.raw = eeconfig_read_kb();
    if (!mm_eeconfig.raw) {
        eeconfig_update_multimode_default();
    }
    eeconfig_debug_multimode(); // display current eeprom values
    mm_eeconfig.charging = true;

    bt_init_time = timer_read32();

#ifndef MULTIMODE_THREAD_DISABLE
    chThdCreateStatic(waThread1, sizeof(waThread1), HIGHPRIO, Thread1, NULL);
#endif

    mm_init_kb();

    mm_mode_scan(true);
}

void mm_task(void) {
    static uint32_t last_time = 0;

    if (MATRIX_SPLIT_IS_MASTER()) {
        if ((bt_init_time != 0) && (timer_elapsed32(bt_init_time) >= MM_READY_TIME)) {
            bt_init_time = 0;

            if (MM_BT_SLEEP_EN) {
                bts_send_vendor(v_en_sleep_bt);
            } else {
                bts_send_vendor(v_dis_sleep_bt);
            }

            if (MM_2G4_SLEEP_EN) {
                bts_send_vendor(v_en_sleep_wl);
            } else {
                bts_send_vendor(v_dis_sleep_wl);
            }

            bts_send_vendor(v_get_version);

            mm_switch_mode(!mm_eeconfig.devs, mm_eeconfig.devs, false);
        }

        // Set led indicators
        if (mm_eeconfig.devs != DEVS_USB) {
            extern uint8_t keyboard_led_state;
            keyboard_led_state = bts_info.bt_info.indicators;
        }

        if (!bt_init_time && (timer_elapsed32(last_time) >= (MM_BAT_REQ_TIME))) {
            if ((mm_eeconfig.devs != DEVS_USB) && (bts_is_busy() == 0) && bts_info.bt_info.active) {
                last_time = timer_read32();
                bts_send_vendor(v_query_vol);

                if (!bts_info.bt_info.version) {
                    bts_send_vendor(v_get_version);
                }
            }
        }

#ifdef MM_BAT_CABLE_PIN
        mm_eeconfig.charging = readPin(MM_BAT_CABLE_PIN);
#endif

        // mode scan
        mm_mode_scan(false);
    }

#ifndef MM_SLEEP_DISABLE
    mm_sleep_task();
#endif

#ifdef MULTIMODE_THREAD_DISABLE
    bts_task(mm_eeconfig.devs);
    mm_bts_task_kb(mm_eeconfig.devs);
#endif

    if (MATRIX_SPLIT_IS_MASTER()) {
        bts_via_task();

        if (jumpboot_flag) {
            reset_keyboard();
        }
    }
}

__WEAK bool mm_switch_mode_user(uint8_t last_mode, uint8_t now_mode, uint8_t reset) {
    return true;
}

__WEAK bool mm_switch_mode_kb(uint8_t last_mode, uint8_t now_mode, uint8_t reset) {

    if (mm_switch_mode_user(last_mode, now_mode, reset) != true) {
        return false;
    }

    return true;
}

__WEAK bool mm_switch_mode(uint8_t last_mode, uint8_t now_mode, uint8_t reset) {
    bool usb_sws = !!last_mode ? !now_mode : !!now_mode;

    if (usb_sws) {
        if (!!now_mode) {
#ifndef MM_USB_DISABLE
#    ifdef MM_USB_EN_PIN
            writePin(MM_USB_EN_PIN, !MM_USB_EN_STATE);
#    endif
            if (USB_DRIVER.state != USB_STOP) {
                usbDisconnectBus(&USB_DRIVER);
                usbStop(&USB_DRIVER);
                // chibios_driver.send_mouse = mm_send_mouse;
                // host_set_driver(&chibios_driver);
            }
#endif
        } else {
#ifdef MM_USB_EN_PIN
            writePin(MM_USB_EN_PIN, MM_USB_EN_STATE);
#endif
            if (USB_DRIVER.state != USB_ACTIVE) {
                extern void send_mouse(report_mouse_t *report);
                extern uint8_t keyboard_led_state;
                keyboard_led_state        = 0x00;
                chibios_driver.send_mouse = send_mouse;
                init_usb_driver(&USB_DRIVER);
                // host_set_driver(&chibios_driver);
            }
        }
    }

    if (last_mode != now_mode) {
        mm_clear_all_keys();
    }

    MM_DEBUG_INFO("[switch mode] : last_mode = %d, now_mode = %d, %s", last_mode, now_mode, mm_sleep_status_str[mm_get_sleep_status()]);

    switch (mm_get_sleep_status()) {
        case mm_state_presleep:
        case mm_state_stop: {
            mm_set_sleep_state(mm_state_wakeup);
            mm_set_sleep_level(mm_sleep_level_timeout);
        } break;
        default: break;
    }

    if ((mm_eeconfig.devs != now_mode) || (reset)) {
        bts_info.bt_info.pairing       = false;
        bts_info.bt_info.paired        = false;
        bts_info.bt_info.come_back     = false;
        bts_info.bt_info.come_back_err = false;
        bts_info.bt_info.no_pair_info  = false;
        bts_info.bt_info.mode_switched = false;
    }

    mm_eeconfig.devs = now_mode;
    if (mm_eeconfig.devs != DEVS_USB) {
        mm_eeconfig.last_devs = mm_eeconfig.devs;
        if ((mm_eeconfig.devs >= DEVS_BT1) && (mm_eeconfig.devs <= DEVS_BT5)) {
            mm_eeconfig.last_btdevs = mm_eeconfig.devs;
        }
    }
    eeconfig_update_kb(mm_eeconfig.raw);

    switch (mm_eeconfig.devs) {
        case DEVS_BT1: {
            if (reset != false) {
                bts_send_vendor(v_bt1);
                bts_send_vendor(v_clear);
                bts_send_name(DEVS_BT1);
                bts_send_vpid(VENDOR_ID, PRODUCT_ID);
                bts_send_vendor(v_pair);
            } else {
                bts_send_vendor(v_bt1);
            }
        } break;
        case DEVS_BT2: {
            if (reset != false) {
                bts_send_vendor(v_bt2);
                bts_send_vendor(v_clear);
                bts_send_name(DEVS_BT2);
                bts_send_vpid(VENDOR_ID, PRODUCT_ID);
                bts_send_vendor(v_pair);
            } else {
                bts_send_vendor(v_bt2);
            }
        } break;
        case DEVS_BT3: {
            if (reset != false) {
                bts_send_vendor(v_bt3);
                bts_send_vendor(v_clear);
                bts_send_name(DEVS_BT3);
                bts_send_vpid(VENDOR_ID, PRODUCT_ID);
                bts_send_vendor(v_pair);
            } else {
                bts_send_vendor(v_bt3);
            }
        } break;
        case DEVS_BT4: {
            if (reset != false) {
                bts_send_vendor(v_bt4);
                bts_send_vendor(v_clear);
                bts_send_name(DEVS_BT4);
                bts_send_vpid(VENDOR_ID, PRODUCT_ID);
                bts_send_vendor(v_pair);
            } else {
                bts_send_vendor(v_bt4);
            }
        } break;
        case DEVS_BT5: {
            if (reset != false) {
                bts_send_vendor(v_bt5);
                bts_send_vendor(v_clear);
                bts_send_name(DEVS_BT5);
                bts_send_vpid(VENDOR_ID, PRODUCT_ID);
                bts_send_vendor(v_pair);
            } else {
                bts_send_vendor(v_bt5);
            }
        } break;
        case DEVS_2G4: {
            if (reset != false) {
                bts_send_vendor(v_2g4);
                bts_send_manufacturer((char *)USBSTR(MM_DONGLE_MANUFACTURER), sizeof(USBSTR(MM_DONGLE_MANUFACTURER)) - 2);
                bts_send_product((char *)USBSTR(MM_DONGLE_PRODUCT), sizeof(USBSTR(MM_DONGLE_PRODUCT)) - 2);
                bts_send_vpid(VENDOR_ID, PRODUCT_ID);
                bts_send_vendor(v_clear);
                bts_send_vendor(v_pair);
            } else {
                bts_send_vendor(v_2g4);
            }
        } break;
        case DEVS_USB: {
            bts_send_vendor(v_usb);
        } break;
        default: {
            bts_send_vendor(v_usb);
            eeconfig_update_multimode_default();
        } break;
    }

    mm_update_key_press_time();
    mm_switch_mode_kb(last_mode, now_mode, reset);

    return true;
}

void mm_clear_all_keys(void) {
    bool delay = false;

#ifndef MM_CLEAR_KEY_DELAY
#    define MM_CLEAR_KEY_DELAY 200
#endif

    if ((mm_eeconfig.devs == DEVS_USB) && (USB_DRIVER.state == USB_ACTIVE)) {
        clear_mods();
        clear_weak_mods();
        clear_keys();
        send_keyboard_report();
#ifdef MOUSEKEY_ENABLE
        extern void mousekey_clear(void);
        extern void mousekey_send(void);
        mousekey_send();
        mousekey_clear();
#endif /* MOUSEKEY_ENABLE */
#ifdef EXTRAKEY_ENABLE
        host_system_send(0);
        host_consumer_send(0);
#endif /* EXTRAKEY_ENABLE */
    } else {
        delay = bts_clear_all_keys();
    }

    if (delay) {
        while (bts_is_busy()) {}
        wait_ms(MM_CLEAR_KEY_DELAY);
    }
}

bool mm_modeio_detection(bool update, uint8_t *mode) {
    static uint32_t scan_timer = 0x00;

    *mode = mio_none;

#ifndef MM_MODEIO_DETECTION_TIME
#    define MM_MODEIO_DETECTION_TIME 50
#endif

    if ((update != true) && (timer_elapsed32(scan_timer) <= (MM_MODEIO_DETECTION_TIME))) {
        return false;
    }
    scan_timer = timer_read32();

#ifdef MM_MODE_SW_PIN // 用来检测无线和有线模式的PIN
#    ifndef MM_USB_MODE_STATE
#        define MM_USB_MODE_STATE 0 // 默认低电平为USB状态(有线状态)
#    endif

    uint8_t now_mode = false;
    uint8_t usb_sws  = false;

    now_mode = !!MM_USB_MODE_STATE ? !readPin(MM_MODE_SW_PIN) : readPin(MM_MODE_SW_PIN);
    usb_sws  = !!mm_eeconfig.devs ? !now_mode : now_mode;

    if (now_mode) {
        *mode = mio_wireless;
    } else {
        *mode = mio_usb;
    }

    if (usb_sws) {
        return true;
    }
#elif defined(MM_BT_DEF_PIN) && defined(MM_2G4_DEF_PIN)
#    ifndef MM_BT_PIN_STATE
#        define MM_BT_PIN_STATE 0, 1
#    endif

#    ifndef MM_2G4_PIN_STATE
#        define MM_2G4_PIN_STATE 1, 0
#    endif

#    ifndef MM_USB_PIN_STATE
#        define MM_USB_PIN_STATE 1, 1
#    endif

#    define MM_GET_MODE_PIN_(pin_bt, pin_2g4) ((((#pin_bt)[0] == 'x') || ((readPin(MM_BT_DEF_PIN) + 0x30) == ((#pin_bt)[0]))) && (((#pin_2g4)[0] == 'x') || ((readPin(MM_2G4_DEF_PIN) + 0x30) == ((#pin_2g4)[0]))))
#    define MM_GET_MODE_PIN(state)            MM_GET_MODE_PIN_(state)

    uint8_t now_mode         = 0x00;
    uint8_t mm_mode          = 0x00;
    static uint8_t last_mode = 0x00;
    bool sw_mode             = false;

    now_mode  = (MM_GET_MODE_PIN(MM_USB_PIN_STATE) ? 3 : (MM_GET_MODE_PIN(MM_BT_PIN_STATE) ? 1 : ((MM_GET_MODE_PIN(MM_2G4_PIN_STATE) ? 2 : 0))));
    mm_mode   = (mm_eeconfig.devs >= DEVS_BT1 && mm_eeconfig.devs <= DEVS_BT5) ? 1 : ((mm_eeconfig.devs == DEVS_2G4) ? 2 : ((mm_eeconfig.devs == DEVS_USB) ? 3 : 0));
    sw_mode   = ((update || (last_mode == now_mode)) && (mm_mode != now_mode)) ? true : false;
    last_mode = now_mode;

    switch (now_mode) {
        case 1: { // BT mode
            *mode = mio_bt;
        } break;
        case 2: { // 2G4 mode
            *mode = mio_2g4;
        } break;
        case 3: { // USB mode
            *mode = mio_usb;
        } break;
        default: {
            *mode = mio_none;
        } break;
    }

    if (sw_mode) {
        // extern void master_sync_mms_slave(uint8_t last_mode, uint8_t now_mode, uint8_t reset);
        // master_sync_mms_slave(mm_eeconfig.last_devs, mm_eeconfig.devs, bts_info.bt_info.pvol);
        return true;
    }
#else
    *mode = mio_none;
#endif

    return false;
}
extern bool pair_lback_timeout;
__WEAK bool mm_mode_scan(bool update) {
    uint8_t mode = mio_none;

    if (!MATRIX_SPLIT_IS_MASTER()) {
        return true;
    }

    if (mm_modeio_detection(update, &mode)) {
        pair_lback_timeout = false;
        mm_update_key_press_time();
        switch (mode) {
            case mio_usb: {
                mm_switch_mode(mm_eeconfig.devs, DEVS_USB, false);
            } break;
            case mio_bt: {
                mm_switch_mode(mm_eeconfig.devs, mm_eeconfig.last_btdevs, false);
                // bts_send_vendor(0x80 + 30);
            } break;
            case mio_2g4: {
                mm_switch_mode(mm_eeconfig.devs, DEVS_2G4, false);
                // bts_send_vendor(0xFF);
            } break;
            case mio_wireless: {
                mm_switch_mode(mm_eeconfig.devs, mm_eeconfig.last_devs, false);
            } break;
        }

        return true;
    }

    return false;
}

__WEAK void mm_wakeup_user(void) {}
__WEAK void mm_sleep_user(void) {}

__WEAK bool mm_get_rgb_enable(void) {
#ifdef RGB_MATRIX_ENABLE
    return rgb_matrix_config.enable;
#endif
    return false;
}

__WEAK void mm_set_rgb_enable(bool state) {
#ifdef RGB_MATRIX_ENABLE
    rgb_matrix_config.enable = state;
#endif
}

__WEAK bool mm_is_to_sleep(void) {

    switch (mm_get_sleep_level()) {
        case mm_sleep_level_lowpower: {
            return true;
        } break;
        case mm_sleep_level_shutdown: {
#ifndef SHUTDOWN_TRIGGER_ON_KEYUP
            // 低电和关机时不判断按键是否抬起
            if (mm_get_sleep_laststatus() == mm_state_presleep) {
                return true;
            }
#endif
        } break;
        default:
            break;
    }

    if (keys_count == 0) {
        MM_DEBUG_INFO("all keys up allow sleep");
    }

    return (keys_count == 0);
}

void mm_update_key_press_time(void) {
    mm_sleep_info.key_press_time = timer_read32();
    // MM_DEBUG_INFO("mm_update_key_press_time");
}

uint32_t mm_get_key_press_time(void) {
    return mm_sleep_info.key_press_time;
}

void mm_set_sleep_state(mm_sleep_status_t state) {
    mm_sleep_info.laststatus = mm_sleep_info.status;
    mm_sleep_info.status     = state;
    mm_sleep_info.timestamp  = timer_read32();

    mm_update_key_press_time();
    rs_updater_usb_suspend_timer();
    mm_sleep_manual = false; // 防止唤醒后立即又休眠
    bts_send_vendor(v_query_vol); // 查询一次电量信息

    MM_DEBUG_INFO("[mm_set_sleep_state]");
    MM_DEBUG_INFO("\t laststatus = %s", mm_sleep_status_str[mm_sleep_info.laststatus]);
    MM_DEBUG_INFO("\t status = %s", mm_sleep_status_str[mm_sleep_info.status]);
    MM_DEBUG_INFO("\t timestamp = %d", mm_sleep_info.timestamp);
}

mm_sleep_status_t mm_get_sleep_status(void) {
    return mm_sleep_info.status;
}

mm_sleep_status_t mm_get_sleep_laststatus(void) {
    return mm_sleep_info.laststatus;
}

void mm_set_sleep_level(mm_sleep_level_t level) {
    mm_sleep_info.level = level;

    MM_DEBUG_INFO("[mm_set_sleep_level]");
    MM_DEBUG_INFO("\t mm_sleep_info.level = %s", mm_sleep_level_str[mm_sleep_info.level]);
}

mm_sleep_level_t mm_get_sleep_level(void) {
    return mm_sleep_info.level;
}

void mm_set_sleep_wakeupcd(mm_wakeupcd_t wakeupcd) {
    mm_sleep_info.wakeupcd = wakeupcd;
}

mm_wakeupcd_t mm_get_sleep_wakeupcd(void) {
    return mm_sleep_info.wakeupcd;
}

void mm_set_to_sleep(void) {
    mm_sleep_manual = true;
    MM_DEBUG_INFO("mm_set_to_sleep");
}

// 超时无操作休眠条件，面向用户函数
__WEAK bool mm_is_allow_timeout_user(void) {
    return true;
}

// 超时无操作休眠条件，返回true时系统进入休眠
__WEAK bool mm_is_allow_timeout(void) {
#ifndef MM_SLEEP_TIMEOUT
#    define MM_SLEEP_TIMEOUT (2 * 60000) // 2 min
#endif

    uint32_t timeout = 0;

    switch (mm_eeconfig.sleep_timeout) {
        case mm_sleep_timeout_none: {
            timeout = 0;
        } break;
        case mm_sleep_timeout_1min: {
            timeout = 1 * 60000;
        } break;
        case mm_sleep_timeout_3min: {
            timeout = 3 * 60000;
        } break;
        case mm_sleep_timeout_5min: {
            timeout = 5 * 60000;
        } break;
        case mm_sleep_timeout_10min: {
            timeout = 10 * 60000;
        } break;
        case mm_sleep_timeout_20min: {
            timeout = 20 * 60000;
        } break;
        default:
        case mm_sleep_timeout_30min: {
            timeout = 30 * 60000;
        } break;
        case mm_sleep_timeout_vendor: {
            timeout = MM_SLEEP_TIMEOUT;
        } break;
    }

    if (!mm_get_key_press_time()) {
        mm_update_key_press_time();
        return false;
    }

    // USB模式并且枚举成功状态，更新超时计数器
    if ((mm_eeconfig.devs == DEVS_USB) && (USB_DRIVER.state == USB_ACTIVE)) {
        mm_update_key_press_time();
    }

    if (!is_keyboard_master()) timeout = 0;

    if (mm_sleep_manual || (timeout && (timer_elapsed32(mm_get_key_press_time()) >= timeout))) {
        MM_DEBUG_INFO("allow timeout: mm_sleep_manual = %d, timeout = %d", mm_sleep_manual, timeout);
        
        mm_sleep_manual = false;

        if (timeout && (timer_elapsed32(mm_get_key_press_time()) >= timeout) && timer_elapsed32(mm_get_key_press_time()) < (timeout * 2)){
            pair_lback_timeout = true;
            // mm_update_key_press_time();
            return false;
        }
        pair_lback_timeout = false;
        if (MATRIX_SPLIT_IS_MASTER()) {
#ifdef MM_BAT_CABLE_PIN
            mm_eeconfig.charging = readPin(MM_BAT_CABLE_PIN);
#else
            mm_eeconfig.charging = true;
#endif
        } else {
            mm_eeconfig.charging = false;
        }

        if ((mm_eeconfig.devs == DEVS_USB) && (USB_DRIVER.state == USB_ACTIVE)) {
            mm_update_key_press_time();
            return false;
        }

#ifdef RGB_RECORD_ENABLE
        bool rgbrec_is_started(void);
        if (rgbrec_is_started()) {
            return false;
        }
#endif
        if (mm_is_allow_timeout_user() != true) {
            return false;
        }

        return true;
    }

    return false;
}

// 低电量休眠条件，返回true时系统进入休眠
__WEAK bool mm_is_allow_lowpower(void) {
    return false;
}

// 设定低电量休眠
__WEAK void mm_set_lowpower(bool state) {
    (void)state;
}

// 关机条件，返回true时系统进入休眠
__WEAK bool mm_is_allow_shutdown(void) {
    return false;
}

// 设定关机状态
__WEAK void mm_set_shutdown(bool state) {
    (void)state;
}

// 预休眠时清除所有按键扫描
__WEAK void mm_reset_matrix_with_presleep(void) {
}

bool mm_is_allow_presleep(void) {

    switch (mm_get_sleep_level()) {
        case mm_sleep_level_lowpower: {
            return true;
        } break;
        case mm_sleep_level_shutdown: {
#ifndef SHUTDOWN_TRIGGER_ON_KEYUP
            // 低电和关机时不判断其它条件，直接允许presleep
            return true;
#endif
        } break;
        default:
            break;
    }

    return mm_is_to_sleep();
}

bool mm_is_allow_stop(void) {
#ifndef MM_STOP_DELAY
#    define MM_STOP_DELAY 200
#endif

    switch (mm_sleep_info.laststatus) {
        case mm_state_presleep: {
#ifndef SHUTDOWN_TRIGGER_ON_KEYUP
            // 当处于关机模式时，第一次休眠不判断其它条件
            if (mm_get_sleep_level() == mm_sleep_level_shutdown) {
                mm_reset_matrix_with_presleep();
                MM_DEBUG_INFO("shutdown first");
                return true;
            }
#endif
        }
        case mm_state_stop: {
            // 上次切换模式的时间戳和现在的时间相差MM_WAKEUP_DELAY时，允许唤醒
            if (timer_elapsed32(mm_sleep_info.timestamp) >= (MM_STOP_DELAY)) {
                // MM_DEBUG_INFO("timeout to sleep");
                return mm_is_to_sleep();
            }
        } break;
        default:
            return false;
    }

    return false;
}

bool mm_is_allow_wakeup(void) {
#ifndef MM_WAKEUP_DELAY
#    define MM_WAKEUP_DELAY 200
#endif

    switch (mm_sleep_info.laststatus) {
        case mm_state_normal: // 为了可以处理手动唤醒增加此条件
        case mm_state_presleep:
        case mm_state_stop:
        case mm_state_wakeup: {
            // 上次切换模式的时间戳和现在的时间相差MM_WAKEUP_DELAY时，允许唤醒
            if (timer_elapsed32(mm_sleep_info.timestamp) >= (MM_WAKEUP_DELAY)) {
                return true;
            }
        } break;
        default:
            return false;
    }

    return false;
}

// 2.4G状态回调函数
void mm_pcstate_cb(uint8_t state) {

    if (state) { // resume
        switch (mm_get_sleep_status()) {
            case mm_state_presleep:
            case mm_state_stop: {
                switch (mm_get_sleep_level()) {
                    case mm_sleep_level_timeout: {
                        mm_set_sleep_state(mm_state_wakeup);
                    } break;
                    default:
                        break;
                }
            } break;
            default:
                break;
        }
        MM_DEBUG_INFO("[2.4G resume]");
    } else { // suspend
        mm_set_to_sleep();
        MM_DEBUG_INFO("[2.4G suspend]");
    }
}

static void mm_sleep_task(void) {
    static bool bak_rgb_enable = false;
    static uint8_t entry_devs  = DEVS_USB;

    switch (mm_get_sleep_status()) {
        case mm_state_normal: {
            if (mm_sleep_info.is_allow_timeout && mm_sleep_info.is_allow_timeout()) {
                mm_set_sleep_state(mm_state_presleep);
                mm_set_sleep_level(mm_sleep_level_timeout);
            }

            if (mm_sleep_info.is_allow_lowpower && mm_sleep_info.is_allow_lowpower()) {
                mm_set_sleep_state(mm_state_presleep);
                mm_set_sleep_level(mm_sleep_level_lowpower);
            }

            if (mm_sleep_info.is_allow_shutdown && mm_sleep_info.is_allow_shutdown()) {
                mm_set_sleep_state(mm_state_presleep);
                mm_set_sleep_level(mm_sleep_level_shutdown);
            }
        } break;
        case mm_state_presleep: {
            void master_sync_mms_slave(uint8_t last_mode, uint8_t now_mode, uint8_t reset);
            if (is_keyboard_master()) master_sync_mms_slave(mm_eeconfig.last_devs, mm_eeconfig.devs,bts_info.bt_info.pvol );
            if (mm_sleep_info.is_allow_presleep && mm_sleep_info.is_allow_presleep()) {
                bak_rgb_enable = mm_get_rgb_enable();
#if defined(RGBLIGHT_ENABLE) && defined(RGB_MATRIX_ENABLE)
                bak_rgb_enable = bak_rgb_enable || rgblight_is_enabled();
#endif
#ifdef RGB_MATRIX_ENABLE
                rgb_matrix_disable_noeeprom();
                writePin(RGB_DRIVER_EN_PIN, !RGB_DRIVER_EN_STATE);
#endif

#ifdef SPLIT_KEYBOARD
                if (MATRIX_SPLIT_IS_MASTER()) {
                    master_to_slave_t m2s = {0};
                    slave_to_master_t s2m = {0};
                    m2s.cmd               = 0xAA;
                    if (transaction_rpc_exec(USER_SYNC_MMS, sizeof(m2s), &m2s, sizeof(s2m), &s2m)) {
                        if (s2m.resp == 0x00) {}
                        dprintf("Slave Sleep OK\n");
                    } else {
                        dprint("Slave sync failed!\n");
                    }
                    
                    if (mm_eeconfig.devs != DEVS_USB) {
                        palSetLineMode(SERIAL_USART_RX_PIN, PAL_OUTPUT_TYPE_OPENDRAIN);
                        palSetLineMode(SERIAL_USART_TX_PIN, PAL_OUTPUT_TYPE_OPENDRAIN);
                    }
                }
#endif

                entry_devs              = mm_eeconfig.devs;
                bts_info.bt_info.active = false;

                // USB模式下的特殊处理
                if ((mm_eeconfig.devs == DEVS_USB) && (!mm_eeconfig.charging)) {
                    // 关闭 USB DP EN, 停止USB
                    if (USB_DRIVER.state != USB_STOP) {
#ifdef MM_USB_EN_PIN
                        writePin(MM_USB_EN_PIN, !MM_USB_EN_STATE);
#endif
                        usbDisconnectBus(&USB_DRIVER);
                        usbStop(&USB_DRIVER);
                        MM_DEBUG_INFO("[USB] usbDisconnectBus & Stop");
                    }
                }

                mm_set_sleep_state(mm_state_stop);
            }
        } break;
        case mm_state_stop: {
            if (mm_sleep_info.is_allow_stop && mm_sleep_info.is_allow_stop()) {
                mm_sleep_manual = false; // 处理2.4G模式下，在键盘已经休眠过程中发送suspend信号时，导致键盘正常唤醒后又立即休眠的问题

#ifdef RGB_DRIVER_EN_PIN
                writePin(RGB_DRIVER_EN_PIN, !RGB_DRIVER_EN_STATE);
#endif
#ifdef RGB_DRIVER_EN2_PIN
                writePin(RGB_DRIVER_EN2_PIN, !RGB_DRIVER_EN_STATE);
#endif

                MM_DEBUG_INFO("[allow stop]");
                MM_DEBUG_INFO("\t laststatus = %s", mm_sleep_status_str[mm_sleep_info.laststatus]);
                MM_DEBUG_INFO("\t status = %s", mm_sleep_status_str[mm_sleep_info.status]);
                MM_DEBUG_INFO("\t timestamp = %d", mm_sleep_info.timestamp);
                MM_DEBUG_INFO("\t sleep level = %s", mm_sleep_level_str[mm_get_sleep_level()]);

                if ((mm_eeconfig.devs != DEVS_USB) || (!mm_eeconfig.charging)) {
                    switch (mm_get_sleep_level()) {
                        case mm_sleep_level_lowpower:
                        case mm_sleep_level_shutdown: {
                            if (mm_sleep_info.laststatus != mm_state_stop) {
                                // 低电和关机时清除所有按键并切换到usb模式
                                if (is_keyboard_master()){
                                    mm_clear_all_keys();
                                    bts_send_vendor(v_usb);
                                    while (bts_is_busy()) {}
                                }
                            }
                        } break;
                        case mm_sleep_level_timeout: {
                            // 超时休眠时清令模组切换到usb模式
                            if (bts_info.bt_info.paired != true) {
                                if (is_keyboard_master()){
                                    bts_send_vendor(v_usb);
                                    while (bts_is_busy()) {}
                                    wait_ms(300);
                                }
                            }
                        } break;
                    }
                }

                mm_sleep_user();
                mm_set_sleep_wakeupcd(mm_wakeup_none);
                if ((mm_eeconfig.devs == entry_devs)) {
                    // USB模式下的特殊处理
                    if ((mm_eeconfig.devs != DEVS_USB) || (!mm_eeconfig.charging)) {
                        MM_DEBUG_INFO("stop start");
                        lp_system_sleep(mm_get_sleep_level());
                    }
                }

                MM_DEBUG_INFO("stop end");
                MM_DEBUG_INFO("\t wakeupcd = %s", mm_wakeupcd_str[mm_get_sleep_wakeupcd()]);

                // 根据唤醒条件进行对应处理
                if (!is_keyboard_master() && mm_get_sleep_level() == mm_sleep_level_lowpower) {
                    mm_set_sleep_state(mm_state_wakeup);
                    mm_set_sleep_level(mm_sleep_level_timeout);
                    mm_set_lowpower(false);
                }
                else {
                    switch (mm_get_sleep_level()) {
                        case mm_sleep_level_timeout: {
                            if (mm_stop_process_timeout() != true) {
                                return;
                            }
                        } break;
                        case mm_sleep_level_lowpower: {
                            if (mm_stop_process_lowpower() != true) {
                                return;
                            }
                        } break;
                        case mm_sleep_level_shutdown: {
                            if (mm_stop_process_shutdown() != true) {
                                return;
                            }
                        } break;
                        default:
                            break;
                    }
                }

                MM_DEBUG_INFO("sleep stop exit\n\n");
            }
        } break;
        case mm_state_wakeup: {
            if (mm_sleep_info.is_allow_wakeup && mm_sleep_info.is_allow_wakeup()) {

                // 确保是事先休眠过才能允许执行wakeup
                // if (bts_info.bt_info.active != true)
                {
                    bts_info.bt_info.active = true;

                    MM_DEBUG_INFO("wakeup system");

                    if (bak_rgb_enable) {
#ifdef RGB_DRIVER_EN_PIN
                        writePin(RGB_DRIVER_EN_PIN, RGB_DRIVER_EN_STATE);
#endif
#ifdef RGB_DRIVER_EN2_PIN
                        writePin(RGB_DRIVER_EN2_PIN, RGB_DRIVER_EN_STATE);
#endif
#ifdef RGB_MATRIX_ENABLE
                        rgb_matrix_enable_noeeprom();
#endif
                    }

                    if (mm_mode_scan(true) != true) {
                        if (mm_eeconfig.devs != DEVS_USB) {
                            mm_switch_mode(mm_eeconfig.last_devs, mm_eeconfig.last_devs, false);
                        } else {
#if defined(WB32_EXTI_REQUIRED) && !defined(WB32_USB_WAKEUP_DISABLE)
                            if (USB_DRIVER.state == USB_STOP)
#endif
                            {
                                mm_switch_mode(mm_eeconfig.last_devs, DEVS_USB, false);
                            }
                        }
                    }

                    mm_sleep_manual = false; // 防止唤醒后立即又休眠
                    mm_wakeup_user();
                    mm_update_key_press_time();
                    rs_updater_usb_suspend_timer();
                    mm_set_sleep_state(mm_state_normal);
                    bts_send_vendor(v_query_vol); // 查询一次电量信息
                }
            }
        } break;
        default:
            break;
    }
}

// 从STOP模式退出后的执行函数 timeout 休眠等级
__WEAK bool mm_stop_process_timeout(void) {
    bool retval = false;

    switch (mm_get_sleep_wakeupcd()) {
        case mm_wakeup_none:
        case mm_wakeup_uart: {
            // UART唤醒不直接唤醒
            mm_set_sleep_state(mm_state_stop);
        } break;
        default: {
            // 非UART唤醒时，不判断唤醒条件，直接唤醒
            mm_set_sleep_state(mm_state_wakeup);
            retval = true;
        } break;
    }

    return retval;
}

// 从STOP模式退出后的执行函数 lowpower 休眠等级
__WEAK bool mm_stop_process_lowpower(void) {
    bool retval = false;

    switch (mm_get_sleep_wakeupcd()) {
        case mm_wakeup_usb:
        case mm_wakeup_cable: {
            // 低电量休眠只能插入USB唤醒
            mm_set_sleep_state(mm_state_wakeup);
            mm_set_sleep_level(mm_sleep_level_timeout);
            mm_set_lowpower(false);
            retval = true;
        } break;
        default: {
            mm_set_sleep_state(mm_state_stop);
        } break;
    }

    return retval;
}

// 从STOP模式退出后的执行函数 shutdown 休眠等级
__WEAK bool mm_stop_process_shutdown(void) {
    bool retval = false;

    switch (mm_get_sleep_wakeupcd()) {
        case mm_wakeup_usb:
        case mm_wakeup_cable: {
            // 低电量休眠只能插入USB唤醒
            mm_set_sleep_state(mm_state_wakeup);
            mm_set_sleep_level(mm_sleep_level_timeout);
            mm_set_shutdown(false);
            retval = true;
        } break;
        case mm_wakeup_onekey: {
            // 低电量休眠只能插入开机键唤醒, 唤醒后先保持STOP状态，等待长按触发唤醒
            mm_set_sleep_state(mm_state_stop);
        } break;
        default: {
            mm_set_sleep_state(mm_state_stop);
        } break;
    }

    return retval;
}

// user-defined overridable functions
__WEAK bool process_record_multimode_user(uint16_t keycode, keyrecord_t *record) {
    return true;
}

__WEAK bool process_record_multimode_kb(uint16_t keycode, keyrecord_t *record) {
    return process_record_multimode_user(keycode, record);
}

bool process_record_multimode(uint16_t keycode, keyrecord_t *record) {

#ifndef MM_SLEEP_DISABLE
    mm_update_key_press_time();
#endif

    if (process_record_multimode_kb(keycode, record) != true) {
        return false;
    }

    // if (mm_eeconfig.devs != DEVS_USB) {
    //     return bts_process_keys(keycode, record->event.pressed, mm_eeconfig.devs, keymap_config.no_gui, keymap_config.nkro);
    // }

    return true;
}

#include "version.h"
#include <stdio.h>
void mm_print_version(void) {
    char tstr[50];

    snprintf(tstr, sizeof(tstr), "module ver: %d\r\n", bts_info.bt_info.version);
    send_string(tstr);
    send_string(bts_get_version());
    send_string("\r\nbuild date: " QMK_BUILDDATE);
}

#include "command.h"
#include "action.h"

extern void register_mouse(uint8_t mouse_keycode, bool pressed);

// void register_code(uint8_t code) {
//     if (mm_eeconfig.devs != DEVS_USB) {
//         bts_process_keys(code, true, mm_eeconfig.devs, false, keymap_config.nkro);
//     } else {
//         if (code == KC_NO) {
//             return;

// #ifdef LOCKING_SUPPORT_ENABLE
//         } else if (KC_LOCKING_CAPS_LOCK == code) {
// #    ifdef LOCKING_RESYNC_ENABLE
//             // Resync: ignore if caps lock already is on
//             if (host_keyboard_leds() & (1 << USB_LED_CAPS_LOCK)) {
//                 return;
//             }
// #    endif
//             add_key(KC_CAPS_LOCK);
//             send_keyboard_report();
//             wait_ms(TAP_HOLD_CAPS_DELAY);
//             del_key(KC_CAPS_LOCK);
//             send_keyboard_report();

//         } else if (KC_LOCKING_NUM_LOCK == code) {
// #    ifdef LOCKING_RESYNC_ENABLE
//             if (host_keyboard_leds() & (1 << USB_LED_NUM_LOCK)) {
//                 return;
//             }
// #    endif
//             add_key(KC_NUM_LOCK);
//             send_keyboard_report();
//             wait_ms(100);
//             del_key(KC_NUM_LOCK);
//             send_keyboard_report();

//         } else if (KC_LOCKING_SCROLL_LOCK == code) {
// #    ifdef LOCKING_RESYNC_ENABLE
//             if (host_keyboard_leds() & (1 << USB_LED_SCROLL_LOCK)) {
//                 return;
//             }
// #    endif
//             add_key(KC_SCROLL_LOCK);
//             send_keyboard_report();
//             wait_ms(100);
//             del_key(KC_SCROLL_LOCK);
//             send_keyboard_report();
// #endif

//         } else if (IS_BASIC_KEYCODE(code)) {
//             // TODO: should push command_proc out of this block?
//             if (command_proc(code)) {
//                 return;
//             }

//             // Force a new key press if the key is already pressed
//             // without this, keys with the same keycode, but different
//             // modifiers will be reported incorrectly, see issue #1708
//             if (is_key_pressed(keyboard_report, code)) {
//                 del_key(code);
//                 send_keyboard_report();
//             }
//             add_key(code);
//             send_keyboard_report();
//         } else if (IS_MODIFIER_KEYCODE(code)) {
//             add_mods(MOD_BIT(code));
//             send_keyboard_report();

// #ifdef EXTRAKEY_ENABLE
//         } else if (IS_SYSTEM_KEYCODE(code)) {
//             host_system_send(KEYCODE2SYSTEM(code));
//         } else if (IS_CONSUMER_KEYCODE(code)) {
//             host_consumer_send(KEYCODE2CONSUMER(code));
// #endif

//         } else if (IS_MOUSE_KEYCODE(code)) {
//             register_mouse(code, true);
//         }
//     }
// }

// void unregister_code(uint8_t code) {
//     if (mm_eeconfig.devs != DEVS_USB) {
//         bts_process_keys(code, false, mm_eeconfig.devs, false, keymap_config.nkro);
//     } else {
//         if (code == KC_NO) {
//             return;

// #ifdef LOCKING_SUPPORT_ENABLE
//         } else if (KC_LOCKING_CAPS_LOCK == code) {
// #    ifdef LOCKING_RESYNC_ENABLE
//             // Resync: ignore if caps lock already is off
//             if (!(host_keyboard_leds() & (1 << USB_LED_CAPS_LOCK))) {
//                 return;
//             }
// #    endif
//             add_key(KC_CAPS_LOCK);
//             send_keyboard_report();
//             del_key(KC_CAPS_LOCK);
//             send_keyboard_report();

//         } else if (KC_LOCKING_NUM_LOCK == code) {
// #    ifdef LOCKING_RESYNC_ENABLE
//             if (!(host_keyboard_leds() & (1 << USB_LED_NUM_LOCK))) {
//                 return;
//             }
// #    endif
//             add_key(KC_NUM_LOCK);
//             send_keyboard_report();
//             del_key(KC_NUM_LOCK);
//             send_keyboard_report();

//         } else if (KC_LOCKING_SCROLL_LOCK == code) {
// #    ifdef LOCKING_RESYNC_ENABLE
//             if (!(host_keyboard_leds() & (1 << USB_LED_SCROLL_LOCK))) {
//                 return;
//             }
// #    endif
//             add_key(KC_SCROLL_LOCK);
//             send_keyboard_report();
//             del_key(KC_SCROLL_LOCK);
//             send_keyboard_report();
// #endif

//         } else if (IS_BASIC_KEYCODE(code)) {
//             del_key(code);
//             send_keyboard_report();
//         } else if (IS_MODIFIER_KEYCODE(code)) {
//             del_mods(MOD_BIT(code));
//             send_keyboard_report();

// #ifdef EXTRAKEY_ENABLE
//         } else if (IS_SYSTEM_KEYCODE(code)) {
//             host_system_send(0);
//         } else if (IS_CONSUMER_KEYCODE(code)) {
//             host_consumer_send(0);
// #endif

//         } else if (IS_MOUSE_KEYCODE(code)) {
//             register_mouse(code, false);
//         }
//     }
// }

// void register_mods(uint8_t mods) {
//     if (mods) {
//         if (mm_eeconfig.devs != DEVS_USB) {
//             while (mods) {
//                 uint8_t bitindex = biton(mods);
//                 bts_process_keys(0xE0 + bitindex, true, mm_eeconfig.devs, false, keymap_config.nkro);

//                 mods &= ~(0x01 << bitindex);
//             }
//         } else {
//             add_mods(mods);
//             send_keyboard_report();
//         }
//     }
// }

// void unregister_mods(uint8_t mods) {
//     if (mods) {
//         if (mm_eeconfig.devs != DEVS_USB) {
//             while (mods) {
//                 uint8_t bitindex = biton(mods);
//                 bts_process_keys(0xE0 + bitindex, false, mm_eeconfig.devs, false, keymap_config.nkro);

//                 mods &= ~(0x01 << bitindex);
//             }
//         } else {
//             del_mods(mods);
//             send_keyboard_report();
//         }
//     }
// }

// void register_weak_mods(uint8_t mods) {
//     if (mods) {
//         if (mm_eeconfig.devs != DEVS_USB) {
//             while (mods) {
//                 uint8_t bitindex = biton(mods);
//                 bts_process_keys(0xE0 + bitindex, true, mm_eeconfig.devs, false, keymap_config.nkro);

//                 mods &= ~(0x01 << bitindex);
//             }
//         } else {
//             add_weak_mods(mods);
//             send_keyboard_report();
//         }
//     }
// }

// void unregister_weak_mods(uint8_t mods) {
//     if (mods) {
//         if (mm_eeconfig.devs != DEVS_USB) {
//             while (mods) {
//                 uint8_t bitindex = biton(mods);
//                 bts_process_keys(0xE0 + bitindex, false, mm_eeconfig.devs, false, keymap_config.nkro);

//                 mods &= ~(0x01 << bitindex);
//             }
//         } else {
//             del_weak_mods(mods);
//             send_keyboard_report();
//         }
//     }
// }

void mm_send_mouse(report_mouse_t *report) {
    mm_report_mouse_t mm_mouse_report = {0};

    mm_mouse_report.buttons = report->buttons;
    mm_mouse_report.x       = report->x;
    mm_mouse_report.y       = report->y;
    mm_mouse_report.z       = report->h;
    mm_mouse_report.h       = report->v;
    bts_send_mouse_report((uint8_t *)&mm_mouse_report);
}

#ifdef RAW_ENABLE

#    include "via.h"
#    include "raw_hid.h"

extern void via_custom_value_command(uint8_t *data, uint8_t length);
extern bool via_command_user(uint8_t *data, uint8_t length);
extern bool bts_send_via(uint8_t *data, uint8_t length);

// Keyboard level code can override this, but shouldn't need to.
// Controlling custom features should be done by overriding
// via_custom_value_command_kb() instead.
__WEAK bool via_command_user(uint8_t *data, uint8_t length) {
    return false;
}

bool via_command_kb(uint8_t *data, uint8_t length) {

    uint8_t *command_id   = &(data[0]);
    uint8_t *command_data = &(data[1]);

    // If via_command_user() returns true, the command was fully
    // handled, including calling raw_hid_send()
    if (via_command_user(data, length)) {
        return true;
    }

    switch (*command_id) {
        case id_get_protocol_version: {
            command_data[0] = VIA_PROTOCOL_VERSION >> 8;
            command_data[1] = VIA_PROTOCOL_VERSION & 0xFF;
            break;
        }
        case id_get_keyboard_value: {
            switch (command_data[0]) {
                case id_uptime: {
                    uint32_t value  = timer_read32();
                    command_data[1] = (value >> 24) & 0xFF;
                    command_data[2] = (value >> 16) & 0xFF;
                    command_data[3] = (value >> 8) & 0xFF;
                    command_data[4] = value & 0xFF;
                    break;
                }
                case id_layout_options: {
                    uint32_t value  = via_get_layout_options();
                    command_data[1] = (value >> 24) & 0xFF;
                    command_data[2] = (value >> 16) & 0xFF;
                    command_data[3] = (value >> 8) & 0xFF;
                    command_data[4] = value & 0xFF;
                    break;
                }
                case id_switch_matrix_state: {
                    uint8_t offset = command_data[1];
                    uint8_t rows   = 28 / ((MATRIX_COLS + 7) / 8);
                    uint8_t i      = 2;
                    for (uint8_t row = 0; row < rows && row + offset < MATRIX_ROWS; row++) {
                        matrix_row_t value = matrix_get_row(row + offset);
#    if (MATRIX_COLS > 24)
                        command_data[i++] = (value >> 24) & 0xFF;
#    endif
#    if (MATRIX_COLS > 16)
                        command_data[i++] = (value >> 16) & 0xFF;
#    endif
#    if (MATRIX_COLS > 8)
                        command_data[i++] = (value >> 8) & 0xFF;
#    endif
                        command_data[i++] = value & 0xFF;
                    }
                    break;
                }
                case id_firmware_version: {
                    uint32_t value  = VIA_FIRMWARE_VERSION;
                    command_data[1] = (value >> 24) & 0xFF;
                    command_data[2] = (value >> 16) & 0xFF;
                    command_data[3] = (value >> 8) & 0xFF;
                    command_data[4] = value & 0xFF;
                    break;
                }
                default: {
                    // The value ID is not known
                    // Return the unhandled state
                    *command_id = id_unhandled;
                    break;
                }
            }
            break;
        }
        case id_set_keyboard_value: {
            switch (command_data[0]) {
                case id_layout_options: {
                    uint32_t value = ((uint32_t)command_data[1] << 24) | ((uint32_t)command_data[2] << 16) | ((uint32_t)command_data[3] << 8) | (uint32_t)command_data[4];
                    via_set_layout_options(value);
                    break;
                }
                case id_device_indication: {
                    uint8_t value = command_data[1];
                    via_set_device_indication(value);
                    break;
                }
                default: {
                    // The value ID is not known
                    // Return the unhandled state
                    *command_id = id_unhandled;
                    break;
                }
            }
            break;
        }
        case id_dynamic_keymap_get_keycode: {
            uint16_t keycode = dynamic_keymap_get_keycode(command_data[0], command_data[1], command_data[2]);
            command_data[3]  = keycode >> 8;
            command_data[4]  = keycode & 0xFF;
            break;
        }
        case id_dynamic_keymap_set_keycode: {
            dynamic_keymap_set_keycode(command_data[0], command_data[1], command_data[2], (command_data[3] << 8) | command_data[4]);
            break;
        }
        case id_dynamic_keymap_reset: {
            dynamic_keymap_reset();
            break;
        }
        case id_custom_set_value:
        case id_custom_get_value:
        case id_custom_save: {
            if (command_data[0] == id_qmk_rgb_matrix_channel && command_id[0] == id_custom_set_value && command_id[2] == id_qmk_rgb_matrix_effect) {
                extern bool led_power_en;
                master_to_slave_t m2s = {0};
                slave_to_master_t s2m = {0};
               
                
                if (command_id[3]){
                    led_power_en = true;
                    mm_set_rgb_enable(true);
                    writePinHigh(RGB_DRIVER_EN_PIN);
                    m2s.cmd               = 0xC5;
                } else {
                    led_power_en = false;
                    m2s.cmd               = 0xC6;
                }
                transaction_rpc_exec(USER_SYNC_MMS, sizeof(m2s), &m2s, sizeof(s2m), &s2m);
                if (!command_id[3]) break;
            }
            via_custom_value_command(data, length);
            break;
        }
#    ifdef VIA_EEPROM_ALLOW_RESET
        case id_eeprom_reset: {
            via_eeprom_set_valid(false);
            eeconfig_init_via();
            break;
        }
#    endif
        case id_dynamic_keymap_macro_get_count: {
            command_data[0] = dynamic_keymap_macro_get_count();
            break;
        }
        case id_dynamic_keymap_macro_get_buffer_size: {
            uint16_t size   = dynamic_keymap_macro_get_buffer_size();
            command_data[0] = size >> 8;
            command_data[1] = size & 0xFF;
            break;
        }
        case id_dynamic_keymap_macro_get_buffer: {
            uint16_t offset = (command_data[0] << 8) | command_data[1];
            uint16_t size   = command_data[2]; // size <= 28
            dynamic_keymap_macro_get_buffer(offset, size, &command_data[3]);
            break;
        }
        case id_dynamic_keymap_macro_set_buffer: {
            uint16_t offset = (command_data[0] << 8) | command_data[1];
            uint16_t size   = command_data[2]; // size <= 28
            dynamic_keymap_macro_set_buffer(offset, size, &command_data[3]);
            break;
        }
        case id_dynamic_keymap_macro_reset: {
            dynamic_keymap_macro_reset();
            break;
        }
        case id_dynamic_keymap_get_layer_count: {
            command_data[0] = dynamic_keymap_get_layer_count();
            break;
        }
        case id_dynamic_keymap_get_buffer: {
            uint16_t offset = (command_data[0] << 8) | command_data[1];
            uint16_t size   = command_data[2]; // size <= 28
            dynamic_keymap_get_buffer(offset, size, &command_data[3]);
            break;
        }
        case id_dynamic_keymap_set_buffer: {
            uint16_t offset = (command_data[0] << 8) | command_data[1];
            uint16_t size   = command_data[2]; // size <= 28
            dynamic_keymap_set_buffer(offset, size, &command_data[3]);
            break;
        }
#    ifdef ENCODER_MAP_ENABLE
        case id_dynamic_keymap_get_encoder: {
            uint16_t keycode = dynamic_keymap_get_encoder(command_data[0], command_data[1], command_data[2] != 0);
            command_data[3]  = keycode >> 8;
            command_data[4]  = keycode & 0xFF;
            break;
        }
        case id_dynamic_keymap_set_encoder: {
            dynamic_keymap_set_encoder(command_data[0], command_data[1], command_data[2] != 0, (command_data[3] << 8) | command_data[4]);
            break;
        }
#    endif
        case id_bootloader_jump: {
            command_data[0] = true;
            jumpboot_flag   = true;
        } break;
		case 0x21: { // 驱动获取、设置六键和全键  读：0x21 0x00 0x00  写：0x21 0x01 0x00
            if (command_data[0]){ // 写
                if(command_data[1]){  // 全键
                    keymap_config.nkro = true;
                }else{ // 六键
                    keymap_config.nkro = false;
                }
            } else { // 读
                command_data[1] = keymap_config.nkro;
            }
        } break;
        default: {
            // The command ID is not known
            // Return the unhandled state
            *command_id = id_unhandled;
            break;
        }
    }

    // Return the same buffer, optionally with values changed
    // (i.e. returning state to the host, or the unhandled state).

    if (mm_eeconfig.devs == DEVS_USB) {
        raw_hid_send(data, length);
    } else {
        bts_send_via(data, length);
    }

    return true;
}

enum via_custom_defined_id {
    id_sleep_timeout = 0x01,
    id_rgbrec_channel,
    id_rgbrec_hs_data,
    id_rgbrec_hs_buffer,
};

void via_custom_value_command_kb(uint8_t *data, uint8_t length) {
    // data = [ command_id, channel_id, value_id, value_data ]
    uint8_t *command_id = &(data[0]);
    uint8_t *channel_id = &(data[1]);
    uint8_t *value_id   = &(data[2]);
    uint8_t *value_data = &(data[3]);

    if (*channel_id != id_custom_channel) {
        // Return the unhandled state
        *command_id = id_unhandled;
        return;
    }

    switch (*value_id) {
        case id_sleep_timeout: {
            switch (*command_id) {
                case id_custom_get_value: {
                    value_data[0] = mm_eeconfig.sleep_timeout;
                } break;
                case id_custom_set_value: {
                    if (value_data[0] <= mm_sleep_timeout_vendor) {
                        mm_eeconfig.sleep_timeout = value_data[0];
                        eeconfig_update_kb(mm_eeconfig.raw);
                    }
                } break;
                default: {
                    // Return the unhandled state
                    *command_id = id_unhandled;
                } break;
            }
        } break;
#ifdef RGB_RECORD_ENABLE
        case id_rgbrec_channel: {
            switch (*command_id) {
                case id_custom_get_value: {
                    value_data[0] = rgbrec_get_channle_num();
                } break;
                case id_custom_set_value: {
                    rgbrec_switch_channel(value_data[0]);
                } break;
                default: {
                    // Return the unhandled state
                    *command_id = id_unhandled;
                } break;
            }
        } break;
        case id_rgbrec_hs_data: {
            switch (*command_id) {
                case id_custom_get_value: {
                    uint16_t hs = rgbrec_get_hs_data(value_data[0], value_data[1], value_data[2]);
                    value_data[3] = hs & 0xFF;
                    value_data[4] = hs >> 8;
                } break;
                case id_custom_set_value: {
                    rgbrec_set_hs_data(value_data[0], value_data[1], value_data[2], value_data[3] | value_data[4] << 8);
                } break;
                default: {
                    // Return the unhandled state
                    *command_id = id_unhandled;
                } break;
            }
        } break;
        case id_rgbrec_hs_buffer: {
            switch (*command_id) {
                case id_custom_get_value: {
                    uint16_t offset = (value_data[0] << 8) | value_data[1];
                    uint16_t size   = value_data[2]; // size <= 26
                    rgbrec_get_hs_buffer(offset, size, &value_data[3]);
                } break;
                case id_custom_set_value: {
                    uint16_t offset = (value_data[0] << 8) | value_data[1];
                    uint16_t size   = value_data[2]; // size <= 26
                    rgbrec_set_hs_buffer(offset, size, &value_data[3]);
                } break;
                default: {
                    // Return the unhandled state
                    *command_id = id_unhandled;
                } break;
            }
        } break;
#endif
        default: {
            // Return the unhandled state
            *command_id = id_unhandled;
        } break;
    }
}
#endif
