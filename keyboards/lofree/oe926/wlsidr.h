// Copyright 2024 WestBerry Tech (https://www.westberrytech.com)
// Copyright 2024 JoyLee (@itarze)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

/* sample */
// #define WLSIDR_LED_INDEX_MIXED <rgb_index>
// #define WLSIDR_USB_DISABLE // The definition for when the USB indicator is not needed.

// #define WLSIDR_LED_INDEX_BT1 <rgb_index>
// #define WLSIDR_LED_INDEX_BT2 <rgb_index>
// #define WLSIDR_LED_INDEX_BT3 <rgb_index>
// #define WLSIDR_LED_INDEX_BT4 <rgb_index>
// #define WLSIDR_LED_INDEX_BT5 <rgb_index>
// #define WLSIDR_LED_INDEX_2G4 <rgb_index>
// #define WLSIDR_LED_INDEX_USB <rgb_index>

typedef enum {
    WLSIDR_STATE_NONE = 0,
    WLSIDR_STATE_LINK,
    WLSIDR_STATE_PAIR,
    WLSIDR_STATE_LINK_SUCCEED,
    WLSIDR_STATE_PAIR_SUCCEED,
    WLSIDR_STATE_LINK_TIMEOUT,
    WLSIDR_STATE_PAIR_TIMEOUT,
    WLSIDR_STATE_USER,
} wlsidr_state_t;


void wlsidr_task(void);
void wlsidr_blink_set_state(wlsidr_state_t state);
wlsidr_state_t wlsidr_blink_get_state(void);
void wlsidr_blink_set_timer(uint32_t timer);
uint32_t wlsidr_blink_get_timer(void);
