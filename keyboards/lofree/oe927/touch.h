// Copyright 2025 Su
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

void touch_init(void *cb);
void touch_deinit(void);
void touch_task(void);
bool touch_deep_sleep(uint8_t time);
bool touch_wakeup(void);
bool touch_close(void);
void touch_wait_free(uint32_t timeout);
