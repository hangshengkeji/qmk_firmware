
// Copyright 2024 Su (@isuua)
// Copyright 2024 JoyLee (@itarze)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "quantum.h"
#include "wireless.h"
#include "uart.h"

#ifndef WLS_INQUIRY_BAT_TIME
#    define WLS_INQUIRY_BAT_TIME 3000
#endif

#ifndef WLS_KEYBOARD_REPORT_KEYS
#    define WLS_KEYBOARD_REPORT_KEYS KEYBOARD_REPORT_KEYS
#endif

#ifndef BT1_NAME
#    define BT1_NAME PRODUCT " BT1"
#endif

#ifndef BT2_NAME
#    define BT2_NAME PRODUCT " BT2"
#endif

#ifndef BT3_NAME
#    define BT3_NAME PRODUCT " BT3"
#endif

#ifndef BT4_NAME
#    define BT4_NAME PRODUCT " BT4"
#endif

#ifndef BT5_NAME
#    define BT5_NAME PRODUCT " BT5"
#endif

#ifndef DONGLE_MANUFACTURER
#    define DONGLE_MANUFACTURER MANUFACTURER
#endif

#ifndef DONGLE_PRODUCT
#    define DONGLE_PRODUCT PRODUCT " Dongle"
#endif

#ifndef MD_THREAD_INTERVAL
#    define MD_THREAD_INTERVAL 50
#endif

#ifndef MD_XX_ENABLE
#    define MD_XX_ENABLE false
#endif

#define USBCONCAT(a, b) a##b
#define USBSTR(s) USBCONCAT(L, s)

static wireless_devs_t *wlsdevs = NULL;

extern void last_matrix_activity_trigger(void);

uint8_t wireless_keyboard_leds(void);
void wireless_send_keyboard(report_keyboard_t *report);
void wireless_send_nkro(report_nkro_t *report);
void wireless_send_mouse(report_mouse_t *report);
void wireless_send_extra(report_extra_t *report);
void wireless_task(void);

host_driver_t wireless_driver = {
    .keyboard_leds = wireless_keyboard_leds,
    .send_keyboard = wireless_send_keyboard,
    .send_nkro     = wireless_send_nkro,
    .send_mouse    = wireless_send_mouse,
    .send_extra    = wireless_send_extra,
};

#ifdef MD_THREAD_MODE
void md_main_task_hook(void) __attribute__((weak));
void md_main_task_hook(void) {}

static THD_WORKING_AREA(wlsThread, 4096);
static THD_FUNCTION(Thread, arg) {
    (void)arg;

    chRegSetThreadName("wireless");

    while (true) {
        md_main_task(wlsdevs->now);
        md_main_task_hook();
        chThdSleepMicroseconds(MD_THREAD_INTERVAL);
    }
}
#endif

void wireless_init(wireless_devs_t *devs) {

    void raw_receive_cb(uint8_t * data, uint8_t length);
    void host_state_cb(bool resume);

    wlsdevs = devs;

    md_parms_t parms = {
        .uart_available = uart_available,
        .uart_read      = uart_read,
        .uart_transmit  = uart_transmit,
#if defined(SPLIT_KEYBOARD) && !defined(DISABLE_SYNC_TIMER)
        .timer_read32 = sync_timer_read32,
#else
        .timer_read32 = timer_read32,
#endif
        .host_state_cb  = host_state_cb,
        .raw_receive_cb = raw_receive_cb,
        .pkt_receive_cb = NULL,
    };

    uart_init(115200);
    md_init(parms);

#ifdef MD_THREAD_MODE
    chThdCreateStatic(wlsThread, sizeof(wlsThread), HIGHPRIO, Thread, NULL);
#endif
}

uint8_t wireless_keyboard_leds(void) __attribute__((weak));
uint8_t wireless_keyboard_leds(void) {

    if (*md_getp_state() == MD_STATE_CONNECTED) {
        return *md_getp_indicator();
    }

    return 0;
}

void wireless_send_keyboard(report_keyboard_t *report) __attribute__((weak));
void wireless_send_keyboard(report_keyboard_t *report) {
    uint8_t wls_report_kb[8] = {0};

    _Static_assert((8) - (WLS_KEYBOARD_REPORT_KEYS) >= 2, "WLS_KEYBOARD_REPORT_KEYS cannot be greater than MD_SND_CMD_KB_LEN - 2.");

    if (report && (lpwr_get_mode() == LPWR_MODE_TIMEOUT)) {
        memcpy(wls_report_kb, (uint8_t *)&report->mods, WLS_KEYBOARD_REPORT_KEYS + 2);
    }

#ifndef MD_THREAD_MODE
    wireless_wait_md_free();
#endif

    md_send_kb(wls_report_kb);
}

void wireless_send_nkro(report_nkro_t *report) __attribute__((weak));
void wireless_send_nkro(report_nkro_t *report) {

#ifndef MD_THREAD_MODE
    wireless_wait_md_free();
#endif

    if (report && (lpwr_get_mode() == LPWR_MODE_TIMEOUT)) {
        md_send_nkro((uint8_t *)&report->mods, NKRO_REPORT_BITS);
    } else {
        md_send_nkro(NULL, NKRO_REPORT_BITS);
    }
}

void wireless_send_mouse(report_mouse_t *report) __attribute__((weak));
void wireless_send_mouse(report_mouse_t *report) {
    typedef struct {
        uint8_t buttons;
        int8_t x;
        int8_t y;
        int8_t z;
        int8_t h;
    } __attribute__((packed)) wls_report_mouse_t;

    wls_report_mouse_t wls_report_mouse = {0};

    if (report && (lpwr_get_mode() == LPWR_MODE_TIMEOUT)) {
        wls_report_mouse.buttons = report->buttons;
        wls_report_mouse.x       = report->x;
        wls_report_mouse.y       = report->y;
        wls_report_mouse.z       = report->h;
        wls_report_mouse.h       = report->v;
    }

#ifndef MD_THREAD_MODE
    wireless_wait_md_free();
#endif

    md_send_mouse((uint8_t *)&wls_report_mouse);
}

void wireless_send_extra(report_extra_t *report) __attribute__((weak));
void wireless_send_extra(report_extra_t *report) {
    uint16_t usage = 0;

    if (report && (lpwr_get_mode() == LPWR_MODE_TIMEOUT)) {
        usage = report->usage;

#ifndef MD_THREAD_MODE
        wireless_wait_md_free();
#endif

        switch (usage) {
            case 0x81:
            case 0x82:
            case 0x83: { // system usage
                usage = 0x01 << (usage - 0x81);
                md_send_system((uint8_t *)&usage);
            } break;
            default: {
                md_send_consumer((uint8_t *)&usage);
            } break;
        }
    }
}

void wireless_clear_keys(void) {

    clear_mods();
    clear_weak_mods();
    clear_keys();
    send_keyboard_report();
#ifdef MOUSEKEY_ENABLE
    mousekey_clear();
#endif /* MOUSEKEY_ENABLE */
#ifdef PROGRAMMABLE_BUTTON_ENABLE
    programmable_button_clear();
#endif /* PROGRAMMABLE_BUTTON_ENABLE */
#ifdef EXTRAKEY_ENABLE
    host_system_send(0);
    host_consumer_send(0);
#endif /* EXTRAKEY_ENABLE */
}

void wireless_wait_md_free(void) {

    do {
#ifndef MD_THREAD_MODE
        md_main_task(wlsdevs->now);
#endif
    } while (md_is_busy());
}

void wireless_wait_md_full(void) {

    do {
#ifndef MD_THREAD_MODE
        md_main_task(wlsdevs->now);
#endif
    } while (md_is_full());
}

void host_state_cb_user(bool resume) __attribute__((weak));
void host_state_cb_user(bool resume) {
    (void) resume;
}

// 2.4g mode, host state
void host_state_cb(bool resume) {

    host_state_cb_user(resume);

    if (resume) {
        if (lpwr_get_state() != LPWR_NORMAL) {
            lpwr_update_timestamp();
            lpwr_set_state(LPWR_WAKEUP);
        }
    } else {
        if (lpwr_get_state() == LPWR_NORMAL) {
            lpwr_set_manual_timeout(true);
        }
    }
}

// 2.4g mode, raw data receive
void raw_receive_cb(uint8_t *data, uint8_t length) {
    void raw_hid_receive(uint8_t * data, uint8_t length);

    if (lpwr_get_state() != LPWR_NORMAL) {
        lpwr_set_state(LPWR_WAKEUP);
    }

    raw_hid_receive(data, length);
}

void wireless_devs_change_user(uint8_t old_devs, uint8_t new_devs, bool reset) __attribute__((weak));
void wireless_devs_change_user(uint8_t old_devs, uint8_t new_devs, bool reset) {}

void wireless_devs_change_kb(uint8_t old_devs, uint8_t new_devs, bool reset) __attribute__((weak));
void wireless_devs_change_kb(uint8_t old_devs, uint8_t new_devs, bool reset) {}

void wireless_devs_change(uint8_t old_devs, uint8_t new_devs, bool reset) {
    bool changed = (old_devs == DEVS_USB) ? (new_devs != DEVS_USB) : (new_devs == DEVS_USB);

    if (changed) {
        set_transport((new_devs != DEVS_USB) ? TRANSPORT_WLS : TRANSPORT_USB);
    }

    /* reset lpwr */
    last_matrix_activity_trigger();
    lpwr_set_manual_timeout(false);
    lpwr_set_manual_shutdown(false);
    lpwr_set_manual_lowbat(false);
    if (lpwr_get_state() != LPWR_NORMAL) {
        lpwr_update_timestamp();
        lpwr_set_state(LPWR_WAKEUP);
    }

    if ((wlsdevs->now != new_devs) || reset) {
        *md_getp_state()     = MD_STATE_DISCONNECTED;
        *md_getp_indicator() = 0;
    }

    wlsdevs->now = new_devs;
    if (wlsdevs->now != DEVS_USB) {
        wlsdevs->wl = wlsdevs->now;
        if ((wlsdevs->wl >= DEVS_BT1) && (wlsdevs->wl <= DEVS_BT5)) {
            wlsdevs->bt = wlsdevs->wl;
        }
    }

    switch (new_devs) {
        case DEVS_USB: {
            md_switch_to_usb();
        } break;
        case DEVS_2G4: {
            md_switch_to_2g4(reset,
                             (char *)USBSTR(DONGLE_MANUFACTURER),
                             sizeof(USBSTR(DONGLE_MANUFACTURER)) - 2,
                             (char *)USBSTR(DONGLE_PRODUCT),
                             sizeof(USBSTR(DONGLE_PRODUCT)) - 2,
                             VENDOR_ID,
                             PRODUCT_ID,
                             MD_XX_ENABLE);
        } break;
        case DEVS_BT1: {
            md_switch_to_bt1(reset, BT1_NAME);
        } break;
        case DEVS_BT2: {
            md_switch_to_bt2(reset, BT2_NAME);
        } break;
        case DEVS_BT3: {
            md_switch_to_bt3(reset, BT3_NAME);
        } break;
        case DEVS_BT4: {
            md_switch_to_bt4(reset, BT4_NAME);
        } break;
        case DEVS_BT5: {
            md_switch_to_bt5(reset, BT5_NAME);
        } break;
        default:
            break;
    }

    wireless_devs_change_kb(old_devs, new_devs, reset);
    wireless_devs_change_user(old_devs, new_devs, reset);
}

wireless_devs_t wireless_get_devs(void) {
    return *wlsdevs;
}

void wireless_pre_task(void) __attribute__((weak));
void wireless_pre_task(void) {}

void wireless_post_task(void) __attribute__((weak));
void wireless_post_task(void) {}

void wireless_task(void) {

    wireless_pre_task();
    lpwr_task();
#ifndef MD_THREAD_MODE
    md_main_task(wlsdevs->now);
#endif
    wireless_post_task();

    /* usb_remote_wakeup() should be invoked last so that we have chance
     * to switch to wireless after start-up when usb is not connected
     */
    if (get_transport() == TRANSPORT_USB) {
        usb_remote_wakeup();
    } else if (lpwr_get_state() == LPWR_NORMAL) {
        static uint32_t inqtimer = 0x00;

        if (sync_timer_elapsed32(inqtimer) >= (WLS_INQUIRY_BAT_TIME)) {
            if (md_inquire_bat()) {
                inqtimer = sync_timer_read32();
            }
        }
    }
}

void housekeeping_task_kb(void) {

    wireless_task();
}
