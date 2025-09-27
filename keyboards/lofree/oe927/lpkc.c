// Copyright 2024 WestBerry Tech (https://www.westberrytech.com)
// Copyright 2024 JoyLee (@itarze)
// SPDX-License-Identifier: GPL-2.0-or-later

#include <deferred_exec.h>
#include "lpkc.h"

#ifndef LPKC_CNT
#    define LPKC_CNT 8
#endif

typedef struct {
    deferred_token token;
    uint32_t keycode;
} lpkc_t;

static lpkc_t lpkc_list[LPKC_CNT + 1] = {0};

bool process_lpkc_keycode(uint16_t keycode, bool pressed) __attribute__((weak));
static uint32_t defer_exec_callback(uint32_t trigger_time, void *cb_arg);

static inline uint8_t keycode_registered(uint16_t keycode) {

    for (uint8_t i = 1; i <= LPKC_CNT; i++) {
        if (keycode == (lpkc_list[i].keycode & 0xFFFF)) {
            return i;
        }
    }

    return 0;
}

static inline uint8_t allocate_index_in_list(void) {

    for (uint8_t i = 1; i <= LPKC_CNT; i++) {
        if (lpkc_list[i].keycode == 0x00) {
            return i;
        }
    }

    return 0;
}

static inline uint8_t keycode_add_to_list(uint16_t keycode) {

    if (!keycode_registered(keycode)) {
        return allocate_index_in_list();
    }

    return 0;
}

static inline bool lpkc_register(uint16_t keycode, uint32_t press_time) {
    deferred_token token;
    uint8_t index;

    index = keycode_add_to_list(keycode);
    if (index) {
        token = defer_exec(press_time, defer_exec_callback, &lpkc_list[index].keycode);
        if (token == INVALID_DEFERRED_TOKEN) return false;
        lpkc_list[index].keycode = keycode;
        lpkc_list[index].token   = token;
        return true;
    }

    return false;
}

static inline bool lpkc_unregister(uint16_t keycode) {
    uint8_t index;

    index = keycode_registered(keycode);
    if (index) {
        cancel_deferred_exec(lpkc_list[index].token);
        if (lpkc_list[index].keycode & (0x01U << 16)) {
            process_lpkc_keycode(lpkc_list[index].keycode, false);
        }

        lpkc_list[index].keycode = 0x00;
        lpkc_list[index].token   = INVALID_DEFERRED_TOKEN;
        return true;
    }

    return false;
}

static uint32_t defer_exec_callback(uint32_t trigger_time, void *cb_arg) {
    uint16_t keycode = *((uint32_t *)cb_arg) & 0xFFFF;

    (void)trigger_time;

    *((uint32_t *)cb_arg) |= (0x01U << 16);
    process_lpkc_keycode(keycode, true);

    return 0;
}

void lpkc_init(void) {

    for (uint8_t i = 0; i <= LPKC_CNT; i++) {
        lpkc_list[i].keycode = 0x00;
        lpkc_list[i].token   = INVALID_DEFERRED_TOKEN;
    }
}

bool process_lpkc_record(uint16_t keycode, uint32_t press_time, bool pressed) {

    if (pressed) {
        return lpkc_register(keycode, press_time);
    } else {
        return lpkc_unregister(keycode);
    }
}

bool process_lpkc_keycode(uint16_t keycode, bool pressed) {

    (void)keycode;
    (void)pressed;

    return true;
}
