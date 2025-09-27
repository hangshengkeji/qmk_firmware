// Copyright 2024 WestBerry Tech (https://www.westberrytech.com)
// Copyright 2024 JoyLee (@itarze)
// SPDX-License-Identifier: GPL-2.0-or-later

#include <stdint.h>
#include <stdbool.h>

#include "sync_timer.h"
#include "led_blink.h"

#ifndef LED_BLINK_CNT
#    define LED_BLINK_CNT 10
#endif

#define GET_LED_INDEX(index) (((index) == 0xFF) ? (index) : ((index) - 1))
#define GET_INDEX(led_index) (((led_index) == 0xFF) ? (led_index) : ((led_index) + 1))

typedef enum {
    LED_STATE_NONE = 0,
    LED_STATE_WAIT_ON,
    LED_STATE_WAIT_OFF,
} led_state_t;

typedef struct
{
    bool always;
    uint8_t index;
    uint8_t times;
    uint32_t timer;
    uint32_t interval;
    led_callback_t cb;
    led_state_t state;
    uint8_t value;
} led_param_t;

static led_param_t blink_list[LED_BLINK_CNT + 1] = {0};

bool led_blink_set_hook(uint8_t led_index, bool on, uint8_t value) __attribute__((weak));
bool led_blink_set_hook(uint8_t led_index, bool on, uint8_t value) {
    return true;
}

static inline uint8_t led_blink_registered(uint8_t led_index) {

    for (uint8_t i = 1; i <= LED_BLINK_CNT; i++) {
        if (GET_INDEX(led_index) == blink_list[i].index) {
            return i;
        }
    }

    return 0;
}

static inline uint8_t allocate_index_in_list(void) {

    for (uint8_t i = 1; i <= LED_BLINK_CNT; i++) {
        if (blink_list[i].index == 0x00) {
            return i;
        }
    }

    return 0;
}

static inline uint8_t led_blink_add_to_list(uint8_t led_index) {
    uint8_t index;

    index = led_blink_registered(led_index);
    if (!index) {
        return allocate_index_in_list();
    }

    return index;
}

static inline void led_blink_set(led_param_t *p, bool on) {

    if (led_blink_set_hook(GET_LED_INDEX(p->index), on, p->value) != true) {
        return;
    }

    if (on) {
        if (p->index == 0xFF) {
            // led_set_value_all(p->value);
        } else {
            // led_set_value(GET_LED_INDEX(p->index), p->value);
        }
    } else {
        if (p->index == 0xFF) {
            // led_set_value_all(0, 0, 0);
        } else {
            // led_set_value(GET_LED_INDEX(p->index), 0, 0, 0);
        }
    }
}

bool led_blink_mod_value(uint8_t led_index, uint8_t value) {
    uint8_t index;

    index = led_blink_add_to_list(led_index);
    if (index) {
        blink_list[index].index = GET_INDEX(led_index);
        blink_list[index].value = value;
        blink_list[index].state = LED_STATE_NONE;
        return true;
    }

    return false;
}

bool led_blink_repeat(uint8_t led_index, uint8_t times, uint32_t interval, bool always) {
    uint8_t index;

    index = led_blink_add_to_list(led_index);
    if (index) {
        blink_list[index].index    = GET_INDEX(led_index);
        blink_list[index].times    = times;
        blink_list[index].always   = always;
        blink_list[index].interval = interval;
        blink_list[index].state    = LED_STATE_NONE;
        return true;
    }

    return false;
}

bool led_blink_register(uint8_t led_index, uint8_t value, uint8_t times, uint32_t interval, bool always, led_callback_t cb) {
    uint8_t index;

    index = led_blink_add_to_list(led_index);
    if (index) {
        blink_list[index].index    = GET_INDEX(led_index);
        blink_list[index].value    = value;
        blink_list[index].times    = times;
        blink_list[index].always   = always;
        blink_list[index].interval = interval;
        blink_list[index].cb       = cb;
        blink_list[index].state    = LED_STATE_NONE;
        return true;
    }

    return false;
}

bool led_blink_task(void) {

    for (uint8_t i = 0; i < LED_BLINK_CNT; i++) {
        if (blink_list[i].index) {
            switch (blink_list[i].state) {
                case LED_STATE_NONE: {
                    if (blink_list[i].times) {
                        led_blink_set(&blink_list[i], true);
                        blink_list[i].timer = sync_timer_read32();
                        blink_list[i].state = LED_STATE_WAIT_OFF;
                    }
                } break;
                case LED_STATE_WAIT_OFF: {
                    led_blink_set(&blink_list[i], true);

                    if (blink_list[i].timer && (sync_timer_elapsed32(blink_list[i].timer) >= blink_list[i].interval)) {
                        blink_list[i].timer = sync_timer_read32();
                        if (blink_list[i].always) {
                            blink_list[i].state = LED_STATE_WAIT_OFF;

                            if (blink_list[i].times) blink_list[i].times--;
                            if (!blink_list[i].times) {
                                if (blink_list[i].cb && blink_list[i].cb(GET_LED_INDEX(blink_list[i].index))) {
                                    blink_list[i].state = LED_STATE_WAIT_OFF;
                                } else {
                                    blink_list[i].state = LED_STATE_NONE;
                                    blink_list[i].index = 0;
                                }
                            }
                        } else {
                            blink_list[i].state = LED_STATE_WAIT_ON;
                        }
                    }
                } break;
                case LED_STATE_WAIT_ON: {
                    led_blink_set(&blink_list[i], false);

                    if (blink_list[i].timer && (sync_timer_elapsed32(blink_list[i].timer) >= blink_list[i].interval)) {
                        blink_list[i].timer = sync_timer_read32();
                        blink_list[i].state = LED_STATE_WAIT_OFF;

                        if (blink_list[i].times) blink_list[i].times--;
                        if (!blink_list[i].times) {
                            if (blink_list[i].cb && blink_list[i].cb(GET_LED_INDEX(blink_list[i].index))) {
                                blink_list[i].state = LED_STATE_WAIT_OFF;
                            } else {
                                blink_list[i].state = LED_STATE_NONE;
                                blink_list[i].index = 0;
                            }
                        }
                    }
                } break;
            }
        }
    }

    return true;
}
