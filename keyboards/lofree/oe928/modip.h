// Copyright 2024 WestBerry Tech (https://www.westberrytech.com)
// Copyright 2024 JoyLee (@itarze)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

typedef enum {
    MODIP_MODE_NONE = 0,
    MODIP_MODE_USB,
    MODIP_MODE_BT,
    MODIP_MODE_2G4,
    MODIP_MODE_WIRELESS,
    MODIP_MODE_OFF,
} modip_mode_t;

void modip_init(void);
bool modip_scan(bool update);
modip_mode_t modip_get_mode(void);
bool modip_detection(bool update);
