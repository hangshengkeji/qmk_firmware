// Copyright 2024 sdk66 (@sdk66)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#define USB_POWER_EN_PIN                    B1 // USB ENABLE pin
// #define LED_POWER_EN_PIN                    D2 // LED ENABLE pin
#define HS_BAT_CABLE_PIN                    A7 // USB insertion detection pin
#define HS_LED_BOOSTING_PIN                 A5 // LED BOOSTING

#define BAT_FULL_PIN                        A15
#define BAT_FULL_STATE                      1

/* Battery */

// #define MM_BT_DEF_PIN                       C14
// #define MM_2G4_DEF_PIN                      C15
#define MD_BT1_NAME                         "BT5.0 Keyboard_1"
#define MD_BT2_NAME                         "BT5.0 Keyboard_2"
#define MD_BT3_NAME                         "BT5.0 Keyboard_3"
#define MD_DONGLE_PRODUCT                   "2.4G Wireless Rx"

/* Device Connection RGB Indicator Light Index And Color */
#define HS_RGB_BLINK_INDEX_BT1              15
#define HS_RGB_BLINK_INDEX_BT2              16
#define HS_RGB_BLINK_INDEX_BT3              17
#define HS_RGB_BLINK_INDEX_2G4              18
#define HS_RGB_BLINK_INDEX_USB              19

#define WIITE_B                             0X50

#define HS_LBACK_COLOR_BT1                  RGB_BLUE
#define HS_LBACK_COLOR_BT2                  RGB_BLUE
#define HS_LBACK_COLOR_BT3                  RGB_BLUE
#define HS_LBACK_COLOR_2G4                  RGB_BLUE
#define HS_LBACK_COLOR_USB                  RGB_WHITE

#define HS_PAIR_COLOR_BT1                   RGB_RED
#define HS_PAIR_COLOR_BT2                   RGB_RED
#define HS_PAIR_COLOR_BT3                   RGB_RED
#define HS_PAIR_COLOR_2G4                   RGB_RED
#define HS_PAIR_COLOR_USB                   RGB_WHITE

/* Battery */
#define BATTERY_CAPACITY_LOW                15
#define BATTERY_CAPACITY_STOP               0
#define RGB_MATRIX_BAT_INDEX_MAP            {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}

/* Status Indicator Lamp */
#define RGB_MATRIX_BLINK_INDEX_ALL          0xFF 
#define RGB_MATRIX_BLINK_INDEX_BAT          250
#define RGB_MATRIX_BLINK_INDEX_IND          251
#define RGB_MATRIX_BLINK_COUNT              2
#define HS_DOUBLE_CLICK_TIME_INTERVAL       350
#define CONTINUOUS_PRESS_COUNT              2

#define HS_MATRIX_BLINK_INDEX_BAT           0
#define HS_RGB_INDEX_CAPS                   51
#define HS_RGB_INDEX_SCR                    15
#define HS_RGB_INDEX_WIN_LOCK               81
#define HS_RGB_BLINK_INDEX_REC              13
#define HS_RGB_BLINK_INDEX_F_LAYER          62

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
#define VIA_EEPROM_ALLOW_RESET 

/* RGB Matrix */
#define RGB_MATRIX_FRAMEBUFFER_EFFECTS
#define RGB_MATRIX_KEYPRESSES
#define RGB_TRIGGER_ON_KEYDOWN

/* RGB Light */
#define RGBLED_NUM 6
#define RGBLIGHT_CUSTOM_DRIVER
#define RGBLIGHT_DISABLE_KEYCODES
#define BREATHING_VAL           40
#define RAINBOW_MOOD            40
#define LED_TYPE                RGB
#define RGBLIGHT_LIMIT_VAL      100
#define RGBLIGHT_LED_MAP {0, 1, 2, 3, 4, 5}

/* WS2812 */
#define AW20216S_CS_PIN_1 B9
#define AW20216S_CS_PIN_2 B8
#define AW20216S_EN_PIN_1 D2
#define LED_POWER_EN_PIN AW20216S_EN_PIN_1

#define DRIVER_COUNT 1
#define DRIVER_1_LED_TOTAL 66
#define DRIVER_2_LED_TOTAL 31
#define RGB_MATRIX_LED_COUNT (DRIVER_1_LED_TOTAL + DRIVER_2_LED_TOTAL)

/* rgb_record */
#define ENABLE_RGB_MATRIX_RGBR_PLAY
#define RGBREC_CHANNEL_NUM         4
#define EECONFIG_CONFINFO_USE_SIZE (4 + 16)
#define EECONFIG_RGBREC_USE_SIZE   (RGBREC_CHANNEL_NUM * MATRIX_ROWS * MATRIX_COLS * 2)
#define EECONFIG_USER_DATA_SIZE    (EECONFIG_RGBREC_USE_SIZE + EECONFIG_CONFINFO_USE_SIZE)
#define RGBREC_EECONFIG_ADDR       (uint8_t *)(EECONFIG_USER_DATABLOCK)
#define CONFINFO_EECONFIG_ADDR     (uint32_t *)((uint32_t)RGBREC_EECONFIG_ADDR + (uint32_t)EECONFIG_RGBREC_USE_SIZE)