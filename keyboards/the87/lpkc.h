// Copyright 2024 WestBerry Tech (https://www.westberrytech.com)
// Copyright 2024 JoyLee (@itarze)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

void lpkc_init(void);
bool process_lpkc_record(uint16_t keycode, uint32_t press_time, bool pressed);
