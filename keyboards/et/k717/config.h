// Copyright 2024 yangzheng20003 (@yangzheng20003)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#define USB_POWER_EN_PIN B1 //USB ENABLE pin
#define LED_POWER_EN_PIN A5 //LED ENABLE pin
#define HS_BAT_CABLE_PIN A7 //USB insertion detection pin
#define HS_LED_CAPS_PIN  C4

#define BAT_FULL_PIN                        A15
#define BAT_FULL_STATE                      1

/* Battery */

#define HS_RGB_INDICATOR_COUNT 99
#define HS_RGB_BAT_COUNT 1

#define MM_BT_DEF_PIN                       C14
#define MM_2G4_DEF_PIN                      C15

#define MD_BT1_NAME                         "BT5.0 Keyboard_1"
#define MD_BT2_NAME                         "BT5.0 Keyboard_2"
#define MD_BT3_NAME                         "BT5.0 Keyboard_3"
#define MD_DONGLE_PRODUCT                   "2.4G Wireless Rx"

/* Device Connection RGB Indicator Light Index And Color */
#define HS_RGB_BLINK_INDEX_BT1              66
#define HS_RGB_BLINK_INDEX_BT2              67
#define HS_RGB_BLINK_INDEX_BT3              68
#define HS_RGB_BLINK_INDEX_2G4              69

#define HS_LBACK_COLOR_BT1                  RGB_BLUE
#define HS_LBACK_COLOR_BT2                  RGB_BLUE
#define HS_LBACK_COLOR_BT3                  RGB_BLUE
#define HS_LBACK_COLOR_2G4                  RGB_GREEN

#define HS_PAIR_COLOR_BT1                   RGB_BLUE
#define HS_PAIR_COLOR_BT2                   RGB_BLUE
#define HS_PAIR_COLOR_BT3                   RGB_BLUE
#define HS_PAIR_COLOR_2G4                   RGB_GREEN

/* Battery */
#define BATTERY_CAPACITY_LOW                15
#define BATTERY_CAPACITY_STOP               0
#define RGB_MATRIX_BAT_INDEX_MAP            {18, 19, 20, 21, 22, 23, 24, 25, 26, 27}

/* Status Indicator Lamp */
#define HS_MATRIX_BLINK_INDEX_BAT           2

/* UART */
#define SERIAL_DRIVER                       SD3
#define SD1_TX_PIN                          C10
#define SD1_RX_PIN                          C11

/* SPI */
#define SPI_DRIVER                          SPIDQ
#define SPI_SCK_PIN                         B3
#define SPI_MOSI_PIN                        B5
#define SPI_MISO_PIN                        B4

/* Flash */
#define EXTERNAL_FLASH_SPI_SLAVE_SELECT_PIN C12
#define WEAR_LEVELING_LOGICAL_SIZE          (WEAR_LEVELING_BACKING_SIZE / 2)

/* RGB Matrix */
#define RGB_MATRIX_FRAMEBUFFER_EFFECTS
#define RGB_MATRIX_KEYPRESSES

/* WS2812 */
#define AW20216S_CS_PIN_1 B9
#define AW20216S_EN_PIN_1 D2

#define DRIVER_COUNT 1
#define DRIVER_1_LED_TOTAL 65
//#define DRIVER_2_LED_TOTAL 42
#define RGB_MATRIX_LED_COUNT (DRIVER_1_LED_TOTAL + 0)

/* rgb_record */
#define ENABLE_RGB_MATRIX_RGBR_PLAY
#define RGBREC_CHANNEL_NUM         4
#define EECONFIG_CONFINFO_USE_SIZE (4 + 16)
#define EECONFIG_RGBREC_USE_SIZE   (RGBREC_CHANNEL_NUM * MATRIX_ROWS * MATRIX_COLS * 2)
#define EECONFIG_USER_DATA_SIZE    (EECONFIG_RGBREC_USE_SIZE + EECONFIG_CONFINFO_USE_SIZE)
#define RGBREC_EECONFIG_ADDR       (uint8_t *)(EECONFIG_USER_DATABLOCK)
#define CONFINFO_EECONFIG_ADDR     (uint32_t *)((uint32_t)RGBREC_EECONFIG_ADDR + (uint32_t)EECONFIG_RGBREC_USE_SIZE)
