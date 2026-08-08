// Copyright 2024 WestBerry Tech (https://www.westberrytech.com)
// Copyright 2024 JoyLee (@itarze)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "wireless.h"
#include "usb_main.h"
#include "rgb_matrix_blink.h"
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
#    define WLSIDR_LINK_INTERVAL 5000
#endif

#ifndef WLSIDR_PAIR_INTERVAL
#    define WLSIDR_PAIR_INTERVAL 2000
#endif

#ifndef WLSIDR_SUCCEED_TIME
#    define WLSIDR_SUCCEED_TIME 2000
#endif

#ifndef WLSIDR_LBACK_COLOR_BT1
#    define WLSIDR_LBACK_COLOR_BT1 0x00, 0x00, 0xFF
#endif

#ifndef WLSIDR_LBACK_COLOR_BT2
#    define WLSIDR_LBACK_COLOR_BT2 0x00, 0x00, 0xFF
#endif

#ifndef WLSIDR_LBACK_COLOR_BT3
#    define WLSIDR_LBACK_COLOR_BT3 0x00, 0x00, 0xFF
#endif

#ifndef WLSIDR_LBACK_COLOR_BT4
#    define WLSIDR_LBACK_COLOR_BT4 0x00, 0x00, 0xFF
#endif

#ifndef WLSIDR_LBACK_COLOR_BT5
#    define WLSIDR_LBACK_COLOR_BT5 0x00, 0x00, 0xFF
#endif

#ifndef WLSIDR_LBACK_COLOR_2G4
#    define WLSIDR_LBACK_COLOR_2G4 0x00, 0xFF, 0x00
#endif

#ifndef WLSIDR_PAIR_COLOR_BT1
#    define WLSIDR_PAIR_COLOR_BT1 0x00, 0x00, 0xFF
#endif

#ifndef WLSIDR_PAIR_COLOR_BT2
#    define WLSIDR_PAIR_COLOR_BT2 0x00, 0x00, 0xFF
#endif

#ifndef WLSIDR_PAIR_COLOR_BT3
#    define WLSIDR_PAIR_COLOR_BT3 0x00, 0x00, 0xFF
#endif

#ifndef WLSIDR_PAIR_COLOR_BT4
#    define WLSIDR_PAIR_COLOR_BT4 0x00, 0x00, 0xFF
#endif

#ifndef WLSIDR_PAIR_COLOR_BT5
#    define WLSIDR_PAIR_COLOR_BT5 0x00, 0x00, 0xFF
#endif

#ifndef WLSIDR_PAIR_COLOR_2G4
#    define WLSIDR_PAIR_COLOR_2G4 0x00, 0xFF, 0x00
#endif

#ifndef WLSIDR_LBACK_COLOR_USB
#    define WLSIDR_LBACK_COLOR_USB 0xFF, 0xFF, 0xFF
#endif


// clang-format off
#define WLSIDR_SET_LED(value) (value)
// clang-format on

static wlsidr_state_t wlsidr_state = WLSIDR_STATE_NONE;
static uint32_t wlsidr_timer       = 0x00;

bool wlsidr_blink_cb_hook(uint8_t led_index, wlsidr_state_t state);

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

 void wlsidr_blink_cb(uint8_t led_index) {

    wlsidr_blink_cb_hook(led_index, wlsidr_state);
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
                        rgb_matrix_blink_set_color(led_index, WLSIDR_LBACK_COLOR_BT1);
                    } break;
#endif
#ifdef WLSIDR_LED_INDEX_BT2
                    case WLSIDR_LED_INDEX_BT2: {
                        rgb_matrix_blink_set_color(led_index, WLSIDR_LBACK_COLOR_BT2);
                    } break;
#endif
#ifdef WLSIDR_LED_INDEX_BT3
                    case WLSIDR_LED_INDEX_BT3: {
                        rgb_matrix_blink_set_color(led_index, WLSIDR_LBACK_COLOR_BT3);
                    } break;
#endif
#ifdef WLSIDR_LED_INDEX_2G4
                    case WLSIDR_LED_INDEX_2G4: {
                        rgb_matrix_blink_set_color(led_index, WLSIDR_LBACK_COLOR_2G4);
                    } break;
#endif

#ifdef WLSIDR_LED_INDEX_USB
                    case WLSIDR_LED_INDEX_USB: {
                        rgb_matrix_blink_set_color(led_index, WLSIDR_LBACK_COLOR_USB);
                    } break;
#endif
                    default:
                        return false;
                }
                rgb_matrix_blink_set_interval_times(led_index, WLSIDR_LINK_INTERVAL, 1);
                rgb_matrix_blink_set(led_index);
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
                } else {
                    if (wireless_get_devs().now == DEVS_USB && USB_DRIVER.state == USB_ACTIVE) {
                        wlsidr_state = WLSIDR_STATE_NONE;
                        return false;
                    }
                    rgb_matrix_blink_set(led_index);
                    
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
                        rgb_matrix_blink_set_color(led_index, WLSIDR_PAIR_COLOR_BT1);
                    } break;
#endif
#ifdef WLSIDR_LED_INDEX_BT2
                    case WLSIDR_LED_INDEX_BT2: {
                        rgb_matrix_blink_set_color(led_index, WLSIDR_PAIR_COLOR_BT2);
                    } break;
#endif
#ifdef WLSIDR_LED_INDEX_BT3
                    case WLSIDR_LED_INDEX_BT3: {
                        rgb_matrix_blink_set_color(led_index, WLSIDR_PAIR_COLOR_BT3);
                    } break;
#endif
#ifdef WLSIDR_LED_INDEX_2G4
                    case WLSIDR_LED_INDEX_2G4: {
                        rgb_matrix_blink_set_color(led_index, WLSIDR_PAIR_COLOR_2G4);
                    } break;
#endif
#ifdef WLSIDR_LED_INDEX_USB
                    case WLSIDR_LED_INDEX_USB: {
                        rgb_matrix_blink_set_color(led_index, WLSIDR_LBACK_COLOR_USB);
                    } break;
#endif
                    default:
                        return false;
                }
                rgb_matrix_blink_set_interval_times(led_index, WLSIDR_PAIR_INTERVAL, 1);
                rgb_matrix_blink_set(led_index);
            } else {
                if (sync_timer_elapsed32(wlsidr_timer) >= WLSIDR_PAIR_TIMEOUT || (wireless_get_devs().now == DEVS_2G4 && sync_timer_elapsed32(wlsidr_timer) >= WLSIDR_2G4_PAIR_TIMEOUT)) {
                    wlsidr_timer = 0;
                    wlsidr_state = WLSIDR_STATE_PAIR_TIMEOUT;
                    return wlsidr_blink_cb_hook(led_index, wlsidr_state);
                }

                if ((get_transport() == TRANSPORT_WLS) && (*md_getp_state() == MD_STATE_CONNECTED)) {
                    wlsidr_timer = 0;
                    wlsidr_state = WLSIDR_STATE_PAIR_SUCCEED;
                    return wlsidr_blink_cb_hook(led_index, wlsidr_state);
                } else {
                    if (wireless_get_devs().now != DEVS_USB) {
                        rgb_matrix_blink_set(led_index);
                    } else {
                        wlsidr_state = WLSIDR_STATE_NONE;
                    }
                }
            }
            return true;
        } break;
        case WLSIDR_STATE_LINK_SUCCEED:
        case WLSIDR_STATE_PAIR_SUCCEED: {
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

bool wlsidr_set_blink(uint8_t led_index, wlsidr_state_t state) {

    bool retval = false;

#    ifdef IM_MM_RGB_BLINK_INDEX_MIXED
    retval = rgb_matrix_blink_set_remain_time(IM_MM_RGB_BLINK_INDEX_MIXED, 0x00);
    if (retval != true) {
        return false;
    }
#    else
#        ifdef WLSIDR_LED_INDEX_BT1
    retval = rgb_matrix_blink_set_remain_time(WLSIDR_LED_INDEX_BT1, 0x00);
    if (retval != true) {
        return false;
    }
#        endif
#        ifdef WLSIDR_LED_INDEX_BT2
    retval = rgb_matrix_blink_set_remain_time(WLSIDR_LED_INDEX_BT2, 0x00);
    if (retval != true) {
        return false;
    }
#        endif
#        ifdef WLSIDR_LED_INDEX_BT3
    retval = rgb_matrix_blink_set_remain_time(WLSIDR_LED_INDEX_BT3, 0x00);
    if (retval != true) {
        return false;
    }
#        endif
#        ifdef WLSIDR_LED_INDEX_BT4
    retval = rgb_matrix_blink_set_remain_time(WLSIDR_LED_INDEX_BT4, 0x00);
    if (retval != true) {
        return false;
    }
#        endif
#        ifdef WLSIDR_LED_INDEX_BT5
    retval = rgb_matrix_blink_set_remain_time(WLSIDR_LED_INDEX_BT5, 0x00);
    if (retval != true) {
        return false;
    }
#        endif
#        ifdef WLSIDR_LED_INDEX_2G4
    retval = rgb_matrix_blink_set_remain_time(WLSIDR_LED_INDEX_2G4, 0x00);
    if (retval != true) {
        return false;
    }
#        endif
#        ifdef WLSIDR_LED_INDEX_USB
    retval = rgb_matrix_blink_set_remain_time(WLSIDR_LED_INDEX_USB, 0x00);
    if (retval != true) {
        return false;
    }
#        endif
#    endif
    wlsidr_state = state;
    wlsidr_timer = 0x00;
    wlsidr_blink_cb_hook(led_index, wlsidr_state);

    return retval;
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
        //wlsidr_set_blink(WLSIDR_LED_INDEX_BT1, WLSIDR_STATE_NONE);
        if (new_devs == DEVS_USB) WLSIDR_SET_BLINK(WLSIDR_LED_INDEX_USB);
    }
}

void wlsidr_task(void) __attribute__((weak));
void wlsidr_task(void) {

    if ((lpwr_get_state() == LPWR_NORMAL) && (wlsidr_state == WLSIDR_STATE_NONE)) {
        if (wireless_get_devs().now != DEVS_USB) {
            if (*md_getp_state() != MD_STATE_CONNECTED) {
                wireless_devs_change(DEVS_USB, wireless_get_devs().now, false);
            }
        } else {
            if (USB_DRIVER.state != USB_ACTIVE) {
                wireless_devs_change(!DEVS_USB, DEVS_USB, false);
            }
        }
    }
}

void md_rf_send_carrier(uint8_t channel, uint8_t tx_power, uint8_t phy) {
    uint8_t sdata[4] = {0x00};

    sdata[0] = CONTINUE;
    sdata[1] = channel;
    sdata[2] = tx_power;
    sdata[3] = phy;

    md_send_smsg_pkt(sdata, 4);
}

void md_rf_send_stop(void) {
    uint8_t sdata[2] = {0xB4, 0x00};

    md_send_smsg_pkt(sdata, 2);
}
