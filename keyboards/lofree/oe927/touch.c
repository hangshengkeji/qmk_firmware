// Copyright 2025 Su
// SPDX-License-Identifier: GPL-2.0-or-later

#include <stddef.h>

#include "quantum.h"
#include "touch_uart.h"
#include "touch.h"

void touch_send_task(void);
void touch_receive_task(void);
bool touch_send(uint8_t *data, uint32_t size);
static void touch_send_ack(void);

uint8_t touch_receive_buf[3];
uint8_t touch_send_buf[3];
static void (*touch_cb)(uint8_t value) = NULL;

bool touch_uart_busy = false;

void touch_init(void *cb) {

    touch_uart_init(TOUCH_UART_BITRATE);
    touch_cb = (void (*)(uint8_t))cb;

    // touch_wakeup();
}

void touch_deinit(void) {

    touch_uart_deinit();
}

void touch_task(void) {

    touch_receive_task();
    touch_send_task();
}

void touch_receive_task(void) {
    static uint32_t data_count = 0x00;

    while (touch_uart_available()) {
        uint8_t rbyte = touch_uart_read();
        bool send_ack = true;

        switch (data_count) {
            case 0: {
                if ((rbyte != 0x66) && (rbyte != 0x80) && (rbyte != 0x61)) {
                    data_count = 0;
                    continue;
                }

                touch_receive_buf[data_count++] = rbyte;
                continue;
            } // break;
            case 1: {
                touch_receive_buf[data_count++] = rbyte;
                continue;
            } // break;
            case 2: {
                if ((rbyte != 0x65) && (rbyte != 0x0A)) {
                    data_count = 0;
                    continue;
                }
                touch_receive_buf[data_count++] = rbyte;
            } break;
            default: {
                data_count = 0;
                continue;
            } // break;
        }

        // touch data
        switch (touch_receive_buf[0]) {
            case 0x61: { // ack data
                send_ack = false;
                if ((touch_receive_buf[1] == 0x0D) && (touch_receive_buf[2] == 0x0A)) {
                    touch_uart_busy = false;
                }
            } break;
            case 0x66: {
                if (touch_cb != NULL) {
                    touch_cb(touch_receive_buf[1]);
                }
            } break;
            case 0x80: {
                // do nothing
            } break;
            default: {
                send_ack = false;
            } break;
        }

        if (send_ack) {
            touch_send_ack();
        }

        data_count = 0;
        break;
    }
}

void touch_send_task(void) {
    static uint32_t touch_timer = 0x00;

    if (!touch_uart_busy) {
        touch_timer = timer_read32();
    }

    if (!touch_timer || timer_elapsed32(touch_timer) >= 50) {
        touch_timer = timer_read32();

        touch_send(touch_send_buf, 3);
    }
}

bool touch_send(uint8_t *data, uint32_t size) {

    if (size > sizeof(touch_send_buf)) {
        return false;
    }

    // dprintf("send: ");
    for (uint32_t i = 0; i < size; i++) {
        touch_send_buf[i] = data[i];
        // dprintf("0x%02X ", data[i]);
    }
    // dprintf("\r\n");

    touch_uart_transmit(touch_send_buf, size);

    touch_uart_busy = true;

    return true;
}

bool touch_deep_sleep(uint8_t time) {
    uint8_t data[3] = {0x80, 0x00, 0x65};

    if (time > 0xF0) time = 0xF0;

    data[1] = time;

    return touch_send(data, 3);
}

bool touch_wakeup(void) {
    uint8_t data[3] = {0x80, 0x00, 0x65};

    return touch_send(data, 3);
}

bool touch_close(void) {
    uint8_t data[3] = {0x80, 0xFF, 0x65};

    return touch_send(data, 3);
}

void touch_wait_free(uint32_t timeout) {
    uint32_t timeout_timer = 0x00;

    timeout_timer = timer_read32();
    while (touch_uart_busy && (timer_elapsed32(timeout_timer) <= timeout)) {
        touch_task();
    }
}

static void touch_send_ack(void) {
    uint8_t data[3] = {0x61, 0x0D, 0x0A};

    touch_uart_transmit(data, 3);
}
