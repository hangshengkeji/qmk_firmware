// Copyright 2024 Stefan Kerkmann (@KarlK90)
// Copyright 2021 QMK
// SPDX-License-Identifier: GPL-2.0-or-later

#include "touch_uart.h"
#include "gpio.h"
#include "chibios_config.h"
#include <hal.h>

#ifndef TOUCH_UART_DRIVER
#    define TOUCH_UART_DRIVER SD1
#endif

#ifndef TOUCH_UART_BITRATE
#    define TOUCH_UART_BITRATE 115200
#endif

#ifndef TOUCH_UART_TX_PIN
#    define TOUCH_UART_TX_PIN A9
#endif

#ifndef TOUCH_UART_TX_PAL_MODE
#    define TOUCH_UART_TX_PAL_MODE 7
#endif

#ifndef TOUCH_UART_RX_PIN
#    define TOUCH_UART_RX_PIN A10
#endif

#ifndef TOUCH_UART_RX_PAL_MODE
#    define TOUCH_UART_RX_PAL_MODE 7
#endif

#ifndef TOUCH_UART_CTS_PIN
#    define TOUCH_UART_CTS_PIN A11
#endif

#ifndef TOUCH_UART_CTS_PAL_MODE
#    define TOUCH_UART_CTS_PAL_MODE 7
#endif

#ifndef TOUCH_UART_RTS_PIN
#    define TOUCH_UART_RTS_PIN A12
#endif

#ifndef TOUCH_UART_RTS_PAL_MODE
#    define TOUCH_UART_RTS_PAL_MODE 7
#endif

#ifndef TOUCH_UART_CR1
#    define TOUCH_UART_CR1 0
#endif

#ifndef TOUCH_UART_CR2
#    define TOUCH_UART_CR2 0
#endif

#ifndef TOUCH_UART_CR3
#    define TOUCH_UART_CR3 0
#endif

#ifndef TOUCH_UART_WRDLEN
#    define TOUCH_UART_WRDLEN 3
#endif

#ifndef TOUCH_UART_STPBIT
#    define TOUCH_UART_STPBIT 0
#endif

#ifndef TOUCH_UART_PARITY
#    define TOUCH_UART_PARITY 0
#endif

#ifndef TOUCH_UART_ATFLCT
#    define TOUCH_UART_ATFLCT 0
#endif

static bool is_initialised       = false;
static SerialConfig serialConfig = {
    TOUCH_UART_BITRATE,
    TOUCH_UART_WRDLEN,
    TOUCH_UART_STPBIT,
    TOUCH_UART_PARITY,
    TOUCH_UART_ATFLCT,
};

void touch_uart_init(uint32_t baud) {

    if (is_initialised) {
        return;
    }
    is_initialised = true;

    serialConfig.speed = baud;

    palSetLineMode(TOUCH_UART_TX_PIN, PAL_MODE_ALTERNATE(TOUCH_UART_TX_PAL_MODE) | PAL_OUTPUT_TYPE_PUSHPULL | PAL_WB32_PUPDR_PULLUP | PAL_OUTPUT_SPEED_HIGHEST);
    palSetLineMode(TOUCH_UART_RX_PIN, PAL_MODE_ALTERNATE(TOUCH_UART_RX_PAL_MODE) | PAL_OUTPUT_TYPE_PUSHPULL | PAL_WB32_PUPDR_PULLUP | PAL_OUTPUT_SPEED_HIGHEST);

    sdStart(&TOUCH_UART_DRIVER, &serialConfig);
}

void touch_uart_deinit(void) {

     if (!is_initialised) {
        return;
    }

    is_initialised = false;

    sdStop(&TOUCH_UART_DRIVER);

    gpio_set_pin_input_high(TOUCH_UART_TX_PIN);
    gpio_set_pin_input_high(TOUCH_UART_RX_PIN);
}

void touch_uart_write(uint8_t data) {
    sdPut(&TOUCH_UART_DRIVER, data);
}

uint8_t touch_uart_read(void) {
    return (uint8_t)sdGet(&TOUCH_UART_DRIVER);
}

void touch_uart_transmit(const uint8_t *data, uint16_t length) {
    sdWrite(&TOUCH_UART_DRIVER, data, length);
}

void touch_uart_receive(uint8_t *data, uint16_t length) {
    sdRead(&TOUCH_UART_DRIVER, data, length);
}

bool touch_uart_available(void) {
    return !sdGetWouldBlock(&TOUCH_UART_DRIVER);
}
