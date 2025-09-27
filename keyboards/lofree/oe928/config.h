// Copyright 2024 Su (@isuua)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#ifdef WIRELESS_ENABLE
// module run mode
#    define MD_THREAD_MODE
#    define MD_XX_ENABLE true

// dip switch mode
#    define MODIP_WL_PIN B9
#    define MODIP_USB_PIN C14

#    define MODIP_WL_MODE_STATE 1, 1
#    define MODIP_USB_MODE_STATE 1, 0
#    define MODIP_OFF_MODE_STATE 0, 1

// no key timeout
#    define LPWR_TIMEOUT (60 * 1000) // 60s

#    define LPWR_PRESLEEP_DELAY 50
#    define LPWR_WAKEUP_DELAY   200

// wireless state indicator
#    define WLSIDR_LED_INDEX_BT1 1
#    define WLSIDR_LED_INDEX_BT2 2
#    define WLSIDR_LED_INDEX_BT3 3
#    define WLSIDR_LED_INDEX_2G4 4

#    define WLSIDR_LINK_TIMEOUT 10000
#    define WLSIDR_PAIR_TIMEOUT 60000
#    define WLSIDR_SUCCEED_TIME 3000

#    define WLSIDR_LINK_INTERVAL 250
#    define WLSIDR_PAIR_INTERVAL 500

#    define WLSIDR_LINK_LED_BT1 0xFF
#    define WLSIDR_LINK_LED_BT2 0xFF
#    define WLSIDR_LINK_LED_BT3 0xFF
#    define WLSIDR_LINK_LED_2G4 0xFF

#    define WLSIDR_PAIR_LED_BT1 0xFF
#    define WLSIDR_PAIR_LED_BT2 0xFF
#    define WLSIDR_PAIR_LED_BT3 0xFF
#    define WLSIDR_PAIR_LED_2G4 0xFF

#    define BT1_NAME       "Flow2-68@Lofree"
#    define BT2_NAME       "Flow2-68@Lofree"
#    define BT3_NAME       "Flow2-68@Lofree"
#    define DONGLE_PRODUCT "Flow2@Lofree"

#    define LED_BLINK_INDEX_BAT  0
#    define WLS_INQUIRY_BAT_TIME 1000
#endif

#define LED_BLINK_INDEX_ALL 0xFF

#define USB_CABLE_PIN    C15
#define USB_POWER_EN_PIN B1
#define LED_POWER_EN_PIN C7

#define LED_PIN_MODE_DET      B7
#define LED_PIN_BAT_WHITE_DET B6
#define LED_PIN_BAT_RED_DET   A8

#define BAT_FULL_PIN D2

/* TOUCH */
#define TOUCH_DEEPSLEEP_TIME 10 // min

/* User */
#define BOOTMAGIC_DEBOUNCE 1000
#define MATRIX_IO_DELAY    2

/* UART */
#define UART_DRIVER      SD3
#define UART_TX_PAL_MODE 7
#define UART_RX_PAL_MODE 7
#define UART_TX_PIN      C10
#define UART_RX_PIN      C11

/* TOUCH UART */
#define TOUCH_UART_DRIVER      SD1
#define TOUCH_UART_TX_PIN      A9
#define TOUCH_UART_RX_PIN      A10
#define TOUCH_UART_TX_PAL_MODE 7
#define TOUCH_UART_RX_PAL_MODE 7
#define TOUCH_UART_BITRATE     9600

/* Encoder */
#define ENCODER_MAP_KEY_DELAY 1

/* SPI */
#define SPI_DRIVER   SPIDQ
#define SPI_SCK_PIN  B3
#define SPI_MOSI_PIN B5
#define SPI_MISO_PIN B4

/* Flash */
#define EXTERNAL_FLASH_SPI_SLAVE_SELECT_PIN C12
#define WEAR_LEVELING_LOGICAL_SIZE          (WEAR_LEVELING_BACKING_SIZE / 2)

/* Backlight */
#define BACKLIGHT_GPT_DRIVER GPTD3

#define VIA_EEPROM_ALLOW_RESET
