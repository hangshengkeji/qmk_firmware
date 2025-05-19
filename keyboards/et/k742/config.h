// Copyright 2024 yangzheng20003 (@yangzheng20003)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#define USB_POWER_EN_PIN                    B1 // USB ENABLE pin
#define LED_POWER_EN_PIN                    D2 // LED ENABLE pin
#define HS_BAT_CABLE_PIN                    A7 // USB insertion detection pin
#define HS_LED_BOOSTING_PIN                 A8 // LED BOOSTING

#define BAT_FULL_PIN                        A15
#define BAT_FULL_STATE                      1

#define HS_RGB_INDICATOR_COUNT              99
#define HS_RGB_BAT_COUNT                    1

#define MD_BT1_NAME                         "K742 BT5.0 KB_1"
#define MD_BT2_NAME                         "K742 BT5.0 KB_2"
#define MD_BT3_NAME                         "K742 BT5.0 KB_3"
#define MD_DONGLE_PRODUCT                   "K742"

/* Device Connection RGB Indicator Light Index And Color */
#define HS_RGB_BLINK_INDEX_BT1              49
#define HS_RGB_BLINK_INDEX_BT2              49
#define HS_RGB_BLINK_INDEX_BT3              49
#define HS_RGB_BLINK_INDEX_2G4              49
#define HS_RGB_BLINK_INDEX_USB              49

#define WIITE_B                             0X50

#define HS_LBACK_COLOR_BT1                  RGB_BLUE
#define HS_LBACK_COLOR_BT2                  RGB_RED
#define HS_LBACK_COLOR_BT3                  RGB_GREEN
#define HS_LBACK_COLOR_2G4                  RGB_WHITE
#define HS_LBACK_COLOR_USB                  RGB_WHITE

#define HS_PAIR_COLOR_BT1                   RGB_BLUE
#define HS_PAIR_COLOR_BT2                   RGB_RED
#define HS_PAIR_COLOR_BT3                   RGB_GREEN
#define HS_PAIR_COLOR_2G4                   RGB_WHITE
#define HS_PAIR_COLOR_USB                   RGB_WHITE

/* Battery */
#define BATTERY_CAPACITY_LOW                15
#define BATTERY_CAPACITY_STOP               0
#define RGB_MATRIX_BAT_INDEX_MAP            {18, 19, 20, 21, 22, 23, 24, 25, 26, 27}

/* Status Indicator Lamp */
#define HS_MATRIX_BLINK_INDEX_BAT           68
#define LED_CAP_PIN                         A9
// #define WIN_PIN                             B0
#define LED_NUM_PIN                         B15
// #define HS_RGB_INDEX_CAPS                   52
// #define HS_RGB_INDEX_WIN_LOCK               80

#define HS_RGB_BLINK_INDEX_WIN              55
#define HS_RGB_BLINK_INDEX_MAC              56

// #define SYSTEM_WIN_PIN                      C15
// #define SYSTEM_MAC_PIN                      C14

/* UART */
#define SERIAL_DRIVER                       SD3
#define SD1_TX_PIN                          C10
#define SD1_RX_PIN                          C11

/* Encoder */
#define ENCODER_MAP_KEY_DELAY               1

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
#define RGB_TRIGGER_ON_KEYDOWN

/* RGB Light */
#define RGBLED_NUM 10
#define RGBLIGHT_CUSTOM_DRIVER
#define RGBLIGHT_DISABLE_KEYCODES
#define RGBLIGHT_LIMIT_VAL                  100 
#define breathing_val                       100
#define rainbow_mood                        100
#define LED_TYPE                            RGB

// /* WS2812 */
// #define WS2812_SPI_DRIVER  SPIDM2
// #define WS2812_SPI_DIVISOR 32

#define DRIVER_1_CS B9
#define DRIVER_2_CS B8
#define DRIVER_1_EN D2
#define DRIVER_2_EN D2
	
#define DRIVER_COUNT 2
#define DRIVER_1_LED_TOTAL 71 
#define DRIVER_2_LED_TOTAL 35
#define RGB_MATRIX_LED_COUNT                (DRIVER_1_LED_TOTAL + DRIVER_2_LED_TOTAL) // 灯珠数量

/* rgb_record */
#define ENABLE_RGB_MATRIX_RGBR_PLAY
#define RGBREC_CHANNEL_NUM         4
#define EECONFIG_CONFINFO_USE_SIZE (4 + 16)
#define EECONFIG_RGBREC_USE_SIZE   (RGBREC_CHANNEL_NUM * MATRIX_ROWS * MATRIX_COLS * 2)
#define EECONFIG_USER_DATA_SIZE    (EECONFIG_RGBREC_USE_SIZE + EECONFIG_CONFINFO_USE_SIZE)
#define RGBREC_EECONFIG_ADDR       (uint8_t *)(EECONFIG_USER_DATABLOCK)
#define CONFINFO_EECONFIG_ADDR     (uint32_t *)((uint32_t)RGBREC_EECONFIG_ADDR + (uint32_t)EECONFIG_RGBREC_USE_SIZE)
