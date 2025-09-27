// Copyright 2024 WestBerry Tech (https://www.westberrytech.com)
// Copyright 2024 JoyLee (@itarze)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

typedef bool (*led_callback_t)(uint8_t);

bool led_blink_task(void);
bool led_blink_mod_value(uint8_t led_index, uint8_t value);
bool led_blink_repeat(uint8_t led_index, uint8_t times, uint32_t interval, bool always);
bool led_blink_register(uint8_t led_index, uint8_t value, uint8_t times, uint32_t interval, bool always, led_callback_t cb);
