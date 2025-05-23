// Copyright 2024 sdk66 (@sdk66)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#define USB_POWER_EN_PIN                    B1 // USB ENABLE pin
#define LED_POWER_EN_PIN                    A8 // LED ENABLE pin
#define HS_BAT_CABLE_PIN                    A7 // USB insertion detection pin
#define HS_LED_BOOSTING_PIN                 A9 // LED BOOSTING
#define RGB_DRIVER_EN_STATE                 1

#define BAT_FULL_PIN                        A15
#define BAT_FULL_STATE                      1

#define RGB_MATRIX_LED_COUNT                (88)

#define MD_BT1_NAME                         "Luma84-1"
#define MD_BT2_NAME                         "Luma84-2"
#define MD_BT3_NAME                         "Luma84-3"
#define MD_DONGLE_PRODUCT                   "Luma84-2.4G"

/* Device Connection RGB Indicator Light Index And Color */
#define HS_RGB_BLINK_INDEX_BT1              48
#define HS_RGB_BLINK_INDEX_BT2              47
#define HS_RGB_BLINK_INDEX_BT3              46
#define HS_RGB_BLINK_INDEX_2G4              45
#define HS_RGB_BLINK_INDEX_USB              44

#define WIITE_B                             0X50

#define HS_LBACK_COLOR_BT1                  RGB_RED
#define HS_LBACK_COLOR_BT2                  RGB_YELLOW
#define HS_LBACK_COLOR_BT3                  RGB_BLUE
#define HS_LBACK_COLOR_2G4                  RGB_GREEN
#define HS_LBACK_COLOR_USB                  RGB_WHITE

#define HS_PAIR_COLOR_BT1                   RGB_RED
#define HS_PAIR_COLOR_BT2                   RGB_YELLOW
#define HS_PAIR_COLOR_BT3                   RGB_BLUE
#define HS_PAIR_COLOR_2G4                   RGB_GREEN
#define HS_PAIR_COLOR_USB                   RGB_WHITE

/* Battery */
#define BATTERY_CAPACITY_LOW                15
#define BATTERY_CAPACITY_STOP               0
#define RGB_MATRIX_BAT_INDEX_MAP            {21, 22, 23, 24, 25, 26, 27, 28, 29, 30}

/* Status Indicator Lamp */
#define RGB_MATRIX_BLINK_INDEX_ALL          0xFF 
#define RGB_MATRIX_BLINK_INDEX_BAT          250
#define RGB_MATRIX_BLINK_INDEX_IND          251
#define RGB_MATRIX_BLINK_COUNT              6

#define HS_MATRIX_BLINK_INDEX_BAT           83
#define HS_RGB_INDEX_CAP_LOCK               50
#define HS_RGB_INDEX_WIN_LOCK               79
#define HS_RGB_BLINK_INDEX_VAI              65
#define HS_RGB_BLINK_INDEX_VAD              86
#define HS_RGB_BLINK_INDEX_SPI              87
#define HS_RGB_BLINK_INDEX_SPD              85

#define HS_RGB_BLINK_INDEX_WIN              51
#define HS_RGB_BLINK_INDEX_MAC              52

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
#define RGBLED_NUM 4
#define RGBLIGHT_CUSTOM_DRIVER
#define RGBLIGHT_DISABLE_KEYCODES

#define breathing_val      90
#define rainbow_mood       90
#define LED_TYPE           RGB

// /* WS2812 */
#define WS2812_SPI_DRIVER  SPIDM2
#define WS2812_SPI_DIVISOR 32

/* rgb_record */
#define ENABLE_RGB_MATRIX_RGBR_PLAY
#define RGBREC_CHANNEL_NUM         4
#define EECONFIG_CONFINFO_USE_SIZE (4 + 16)
#define EECONFIG_RGBREC_USE_SIZE   (RGBREC_CHANNEL_NUM * MATRIX_ROWS * MATRIX_COLS * 2)
#define EECONFIG_USER_DATA_SIZE    (EECONFIG_RGBREC_USE_SIZE + EECONFIG_CONFINFO_USE_SIZE)
#define RGBREC_EECONFIG_ADDR       (uint8_t *)(EECONFIG_USER_DATABLOCK)
#define CONFINFO_EECONFIG_ADDR     (uint32_t *)((uint32_t)RGBREC_EECONFIG_ADDR + (uint32_t)EECONFIG_RGBREC_USE_SIZE)
