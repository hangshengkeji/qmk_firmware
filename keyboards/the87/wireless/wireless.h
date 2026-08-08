// Copyright 2024 Su (@isuua)
// Copyright 2024 JoyLee (@itarze)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "transport.h"
#include "lowpower.h"
#include "module.h"

typedef struct {
    uint8_t now;
    uint8_t bt;
    uint8_t wl;
} wireless_devs_t;

void wireless_init(wireless_devs_t *devs);
void wireless_devs_change(uint8_t old_devs, uint8_t new_devs, bool reset);
wireless_devs_t wireless_get_devs(void);
void wireless_pre_task(void);
void wireless_post_task(void);
void wireless_clear_keys(void);
void wireless_wait_md_free(void);
void wireless_wait_md_full(void);
