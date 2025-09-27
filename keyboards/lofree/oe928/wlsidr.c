// Copyright 2024 WestBerry Tech (https://www.westberrytech.com)
// Copyright 2024 JoyLee (@itarze)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "wireless.h"
#include "usb_main.h"
#include "led_blink.h"
#include "sync_timer.h"
#include "wlsidr.h"
#include "uart.h"

#ifndef WLSIDR_USB_LINK_TIMEOUT
#    define WLSIDR_USB_LINK_TIMEOUT 3000
#endif

#ifndef WLSIDR_LINK_TIMEOUT
#    define WLSIDR_LINK_TIMEOUT 60000
#endif

#ifndef WLSIDR_PAIR_TIMEOUT
#    define WLSIDR_PAIR_TIMEOUT 60000
#endif

#ifndef WLSIDR_USB_LINK_INTERVAL
#    define WLSIDR_USB_LINK_INTERVAL WLSIDR_LINK_INTERVAL
#endif

#ifndef WLSIDR_LINK_INTERVAL
#    define WLSIDR_LINK_INTERVAL 500
#endif

#ifndef WLSIDR_PAIR_INTERVAL
#    define WLSIDR_PAIR_INTERVAL 200
#endif

#ifndef WLSIDR_SUCCEED_TIME
#    define WLSIDR_SUCCEED_TIME 2000
#endif

#ifndef WLSIDR_LINK_LED_BT1
#    define WLSIDR_LINK_LED_BT1 0xFF
#endif

#ifndef WLSIDR_LINK_LED_BT2
#    define WLSIDR_LINK_LED_BT2 0xFF
#endif

#ifndef WLSIDR_LINK_LED_BT3
#    define WLSIDR_LINK_LED_BT3 0xFF
#endif

#ifndef WLSIDR_LINK_LED_BT4
#    define WLSIDR_LINK_LED_BT4 0xFF
#endif

#ifndef WLSIDR_LINK_LED_BT5
#    define WLSIDR_LINK_LED_BT5 0xFF
#endif

#ifndef WLSIDR_LINK_LED_2G4
#    define WLSIDR_LINK_LED_2G4 0xFF
#endif

#ifndef WLSIDR_LINK_LED_USB
#    define WLSIDR_LINK_LED_USB 0xFF
#endif

#ifndef WLSIDR_PAIR_LED_BT1
#    define WLSIDR_PAIR_LED_BT1 0xFF
#endif

#ifndef WLSIDR_PAIR_LED_BT2
#    define WLSIDR_PAIR_LED_BT2 0xFF
#endif

#ifndef WLSIDR_PAIR_LED_BT3
#    define WLSIDR_PAIR_LED_BT3 0xFF
#endif

#ifndef WLSIDR_PAIR_LED_BT4
#    define WLSIDR_PAIR_LED_BT4 0xFF
#endif

#ifndef WLSIDR_PAIR_LED_BT5
#    define WLSIDR_PAIR_LED_BT5 0xFF
#endif

#ifndef WLSIDR_PAIR_LED_2G4
#    define WLSIDR_PAIR_LED_2G4 0xFF
#endif

// clang-format off
#define WLSIDR_SET_LED(value) (value)
// clang-format on

static wlsidr_state_t wlsidr_state = WLSIDR_STATE_NONE;
static uint32_t wlsidr_timer       = 0x00;

static bool wlsidr_blink_cb(uint8_t led_index);

void wlsidr_blink_set_state(wlsidr_state_t state) {
    wlsidr_state = state;
}

wlsidr_state_t wlsidr_blink_get_state(void) {
    return wlsidr_state;
}

void wlsidr_blink_set_timer(uint32_t timer) {
    wlsidr_timer = timer;
}

uint32_t wlsidr_blink_get_timer(void) {
    return wlsidr_timer;
}

bool wlsidr_blink_cb_hook(uint8_t led_index, wlsidr_state_t state) __attribute__((weak));
bool wlsidr_blink_cb_hook(uint8_t led_index, wlsidr_state_t state) {
    static uint32_t link_timeout = WLSIDR_LINK_TIMEOUT;

    switch (state) {
        case WLSIDR_STATE_NONE: {
            wlsidr_timer = 0x00;
        } break;
        case WLSIDR_STATE_LINK: {
            if (!wlsidr_timer) {
                wlsidr_timer = sync_timer_read32();
                link_timeout = WLSIDR_LINK_TIMEOUT;

                switch (led_index) {
#ifdef WLSIDR_LED_INDEX_BT1
                    case WLSIDR_LED_INDEX_BT1: {
                        led_blink_register(WLSIDR_LED_INDEX_BT1, WLSIDR_SET_LED(WLSIDR_LINK_LED_BT1), 1, WLSIDR_LINK_INTERVAL, false, wlsidr_blink_cb);
                    } break;
#endif
#ifdef WLSIDR_LED_INDEX_BT2
                    case WLSIDR_LED_INDEX_BT2: {
                        led_blink_register(WLSIDR_LED_INDEX_BT2, WLSIDR_SET_LED(WLSIDR_LINK_LED_BT2), 1, WLSIDR_LINK_INTERVAL, false, wlsidr_blink_cb);
                    } break;
#endif
#ifdef WLSIDR_LED_INDEX_BT3
                    case WLSIDR_LED_INDEX_BT3: {
                        led_blink_register(WLSIDR_LED_INDEX_BT3, WLSIDR_SET_LED(WLSIDR_LINK_LED_BT3), 1, WLSIDR_LINK_INTERVAL, false, wlsidr_blink_cb);
                    } break;
#endif
#ifdef WLSIDR_LED_INDEX_2G4
                    case WLSIDR_LED_INDEX_2G4: {
                        led_blink_register(WLSIDR_LED_INDEX_2G4, WLSIDR_SET_LED(WLSIDR_LINK_LED_2G4), 1, WLSIDR_LINK_INTERVAL, false, wlsidr_blink_cb);
                    } break;
#endif
                    default:
                        return false;
                }
            } else {
                if (sync_timer_elapsed32(wlsidr_timer) >= link_timeout) {
                    wlsidr_timer = 0;
                    wlsidr_state = WLSIDR_STATE_LINK_TIMEOUT;
                    return wlsidr_blink_cb_hook(led_index, wlsidr_state);
                }

                if ((get_transport() == TRANSPORT_WLS) && (*md_getp_state() == MD_STATE_CONNECTED)) {
                    wlsidr_timer = 0;
                    wlsidr_state = WLSIDR_STATE_LINK_SUCCEED;
                    return wlsidr_blink_cb_hook(led_index, wlsidr_state);
                }
            }
            return true;
        } break;
        case WLSIDR_STATE_PAIR: {
            if (!wlsidr_timer) {
                wlsidr_timer = sync_timer_read32();

                switch (led_index) {
#ifdef WLSIDR_LED_INDEX_BT1
                    case WLSIDR_LED_INDEX_BT1: {
                        led_blink_register(WLSIDR_LED_INDEX_BT1, WLSIDR_SET_LED(WLSIDR_PAIR_LED_BT1), 1, WLSIDR_PAIR_INTERVAL, false, wlsidr_blink_cb);
                    } break;
#endif
#ifdef WLSIDR_LED_INDEX_BT2
                    case WLSIDR_LED_INDEX_BT2: {
                        led_blink_register(WLSIDR_LED_INDEX_BT2, WLSIDR_SET_LED(WLSIDR_PAIR_LED_BT2), 1, WLSIDR_PAIR_INTERVAL, false, wlsidr_blink_cb);
                    } break;
#endif
#ifdef WLSIDR_LED_INDEX_BT3
                    case WLSIDR_LED_INDEX_BT3: {
                        led_blink_register(WLSIDR_LED_INDEX_BT3, WLSIDR_SET_LED(WLSIDR_PAIR_LED_BT3), 1, WLSIDR_PAIR_INTERVAL, false, wlsidr_blink_cb);
                    } break;
#endif
#ifdef WLSIDR_LED_INDEX_2G4
                    case WLSIDR_LED_INDEX_2G4: {
                        led_blink_register(WLSIDR_LED_INDEX_2G4, WLSIDR_SET_LED(WLSIDR_PAIR_LED_2G4), 1, WLSIDR_PAIR_INTERVAL, false, wlsidr_blink_cb);
                    } break;
#endif
                    default:
                        return false;
                }
            } else {
                if (sync_timer_elapsed32(wlsidr_timer) >= WLSIDR_PAIR_TIMEOUT) {
                    wlsidr_timer = 0;
                    wlsidr_state = WLSIDR_STATE_PAIR_TIMEOUT;
                    return wlsidr_blink_cb_hook(led_index, wlsidr_state);
                }

                if ((get_transport() == TRANSPORT_WLS) && (*md_getp_state() == MD_STATE_CONNECTED)) {
                    wlsidr_timer = 0;
                    wlsidr_state = WLSIDR_STATE_PAIR_SUCCEED;
                    return wlsidr_blink_cb_hook(led_index, wlsidr_state);
                }
            }
            return true;
        } break;
        case WLSIDR_STATE_LINK_SUCCEED:
        case WLSIDR_STATE_PAIR_SUCCEED: {
            led_blink_repeat(led_index, 1, WLSIDR_SUCCEED_TIME, false);
            wlsidr_timer = 0;
            wlsidr_state = WLSIDR_STATE_NONE;
            return true;
        } break;
        case WLSIDR_STATE_LINK_TIMEOUT:
        case WLSIDR_STATE_PAIR_TIMEOUT: {
            wlsidr_timer = 0;
            wlsidr_state = WLSIDR_STATE_NONE;
#ifdef USB_CABLE_PIN
            extern bool usb_cable_is_connected(void);
            if ((wireless_get_devs().now == DEVS_USB) && (!usb_cable_is_connected())) {
                lpwr_set_manual_shutdown(true);
                return false;
            }
#endif
            lpwr_set_manual_timeout(true); // when timeout to sleep
            return false;
        } break;
        default:
            break;
    }

    return false;
}

static bool wlsidr_blink_cb(uint8_t led_index) {

    return wlsidr_blink_cb_hook(led_index, wlsidr_state);
}

bool wlsidr_set_blink(uint8_t led_index, wlsidr_state_t state) {

#ifdef WLSIDR_LED_INDEX_BT1
    led_blink_register(WLSIDR_LED_INDEX_BT1, 0, 0, 0, false, NULL);
#endif
#ifdef WLSIDR_LED_INDEX_BT2
    led_blink_register(WLSIDR_LED_INDEX_BT2, 0, 0, 0, false, NULL);
#endif
#ifdef WLSIDR_LED_INDEX_BT3
    led_blink_register(WLSIDR_LED_INDEX_BT3, 0, 0, 0, false, NULL);
#endif
#ifdef WLSIDR_LED_INDEX_2G4
    led_blink_register(WLSIDR_LED_INDEX_2G4, 0, 0, 0, false, NULL);
#endif

    wlsidr_timer = 0x00;
    wlsidr_state = state;
    wlsidr_blink_cb_hook(led_index, wlsidr_state);

    return true;
}

void wireless_devs_change_kb(uint8_t old_devs, uint8_t new_devs, bool reset) {

#define WLSIDR_SET_BLINK(led_index)                           \
    do {                                                      \
        if (reset) {                                          \
            wlsidr_set_blink((led_index), WLSIDR_STATE_PAIR); \
        } else {                                              \
            wlsidr_set_blink((led_index), WLSIDR_STATE_LINK); \
        }                                                     \
    } while (false)

    if ((get_transport() == TRANSPORT_WLS) && (*md_getp_state() != MD_STATE_CONNECTED)) {
        switch (new_devs) {
            case DEVS_BT1: {
#ifdef WLSIDR_LED_INDEX_BT1
                WLSIDR_SET_BLINK(WLSIDR_LED_INDEX_BT1);
#endif
            } break;
            case DEVS_BT2: {
#ifdef WLSIDR_LED_INDEX_BT2
                WLSIDR_SET_BLINK(WLSIDR_LED_INDEX_BT2);
#endif
            } break;
            case DEVS_BT3: {
#ifdef WLSIDR_LED_INDEX_BT3
                WLSIDR_SET_BLINK(WLSIDR_LED_INDEX_BT3);
#endif
            } break;
            case DEVS_2G4: {
#ifdef WLSIDR_LED_INDEX_2G4
                WLSIDR_SET_BLINK(WLSIDR_LED_INDEX_2G4);
#endif
            } break;
        }
    } else if (get_transport() == TRANSPORT_USB) {
        wlsidr_set_blink(WLSIDR_LED_INDEX_BT1, WLSIDR_STATE_NONE);
    }
}

void wlsidr_task(void) __attribute__((weak));
void wlsidr_task(void) {

    if ((lpwr_get_state() == LPWR_NORMAL) && (wlsidr_state == WLSIDR_STATE_NONE)) {
        if (wireless_get_devs().now != DEVS_USB) {
            if (*md_getp_state() != MD_STATE_CONNECTED) {
                wireless_devs_change(DEVS_USB, wireless_get_devs().now, false);
            }
        }
    }
}
