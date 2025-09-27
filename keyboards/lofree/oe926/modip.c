// Copyright 2024 WestBerry Tech (https://www.westberrytech.com)
// Copyright 2024 JoyLee (@itarze)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "wireless.h"
#include "sync_timer.h"
#include "modip.h"
#include "gpio.h"

#ifndef MODIP_DETECTION_TIME
#    define MODIP_DETECTION_TIME 50
#endif

#ifdef MODIP_SWITCH_PIN // detection in wireless or wire mode
#    ifndef MODIP_USB_MODE_STATE
#        define MODIP_USB_MODE_STATE 0
#    endif
#else
#    ifndef MODIP_WL_MODE_STATE
#        define MODIP_WL_MODE_STATE 0, 1
#    endif

#    ifndef MODIP_USB_MODE_STATE
#        define MODIP_USB_MODE_STATE 1, 0
#    endif

#    ifndef MODIP_OFF_MODE_STATE
#        define MODIP_OFF_MODE_STATE 1, 1
#    endif
#endif

static modip_mode_t modip_mode = MODIP_MODE_NONE;

void modip_init(void) __attribute__((weak));
void modip_init(void) {

#ifdef MODIP_SWITCH_PIN
    gpio_set_pin_input_high(MODIP_SWITCH_PIN);
#else
#    ifdef MODIP_WL_PIN
    gpio_set_pin_input_high(MODIP_WL_PIN);
#    endif
#    ifdef MODIP_USB_PIN
    gpio_set_pin_input_high(MODIP_USB_PIN);
#    endif
#endif
}

bool modip_detection(bool update) {
    static uint32_t scan_timer = 0x00;

    if ((update != true) && (sync_timer_elapsed32(scan_timer) <= (MODIP_DETECTION_TIME))) {
        return false;
    }
    scan_timer = sync_timer_read32();

#ifdef MODIP_SWITCH_PIN
    uint8_t now_mode = false;
    uint8_t usb_sws  = false;
    uint8_t now_devs = wireless_get_devs().now;

    now_mode = !!MODIP_USB_MODE_STATE ? !readPin(MODIP_SWITCH_PIN) : readPin(MODIP_SWITCH_PIN);
    usb_sws  = !!now_devs ? !now_mode : now_mode;

    if (now_mode) {
        modip_mode = MODIP_MODE_WIRELESS;
    } else {
        modip_mode = MODIP_MODE_USB;
    }

    if (usb_sws) {
        return true;
    }
#elif defined(MODIP_WL_PIN) && defined(MODIP_USB_PIN)
#    define MM_GET_MODE_PIN_(pin_wl, pin_usb) ((((#pin_wl)[0] == 'x') || ((readPin(MODIP_WL_PIN) + 0x30) == ((#pin_wl)[0]))) && (((#pin_usb)[0] == 'x') || ((readPin(MODIP_USB_PIN) + 0x30) == ((#pin_usb)[0]))))
#    define MM_GET_MODE_PIN(state)            MM_GET_MODE_PIN_(state)

    uint8_t now_mode         = 0x00;
    uint8_t mm_mode          = 0x00;
    static uint8_t last_mode = 0x00;
    bool sw_mode             = false;
    uint8_t now_devs         = wireless_get_devs().now;

    now_mode  = (MM_GET_MODE_PIN(MODIP_USB_MODE_STATE) ? 3 : (MM_GET_MODE_PIN(MODIP_WL_MODE_STATE) ? 1 : ((MM_GET_MODE_PIN(MODIP_OFF_MODE_STATE) ? 2 : 0))));
    mm_mode   = (now_devs >= DEVS_BT1 && now_devs <= DEVS_2G4) ? 1 : ((now_devs == DEVS_USB) ? 3 : 2);
    sw_mode   = ((update || (last_mode == now_mode)) && (mm_mode != now_mode)) ? true : false;
    last_mode = now_mode;

    switch (now_mode) {
        case 1: { // wireless mode
            modip_mode = MODIP_MODE_WIRELESS;
        } break;
        case 2: { // off mode
            modip_mode = MODIP_MODE_OFF;
        } break;
        case 3: { // USB mode
            modip_mode = MODIP_MODE_USB;
        } break;
        default: {
            modip_mode = MODIP_MODE_NONE;
        } break;
    }

    if (sw_mode) {
        return true;
    }
#else
    modip_mode = MODIP_MODE_NONE;
#endif

    return false;
}

bool modip_update(uint8_t mode) __attribute__((weak));
bool modip_update(uint8_t mode) {
    return true;
}

bool modip_scan(bool update) {

    if (modip_detection(update)) {

        if (modip_update(modip_mode) != true) {
            return false;
        }

        switch (modip_mode) {
            case MODIP_MODE_USB: {
                wireless_devs_change(wireless_get_devs().now, DEVS_USB, false);
            } break;
            case MODIP_MODE_BT: {
                wireless_devs_change(wireless_get_devs().now, wireless_get_devs().bt, false);
            } break;
            case MODIP_MODE_2G4: {
                wireless_devs_change(wireless_get_devs().now, DEVS_2G4, false);
            } break;
            case MODIP_MODE_WIRELESS: {
                wireless_devs_change(wireless_get_devs().now, wireless_get_devs().wl, false);
            } break;
            default:
                return false;
        }

        return true;
    }

    return false;
}

modip_mode_t modip_get_mode(void) {
    return modip_mode;
}
