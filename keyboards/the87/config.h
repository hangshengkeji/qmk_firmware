// Copyright 2024 Su (@isuua)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#ifdef WIRELESS_ENABLE
// module run mode
#    define MD_THREAD_MODE
#    define MD_XX_ENABLE true

// no key timeout
#    define LPWR_TIMEOUT (3 * 60000) // 60s

#    define LPWR_PRESLEEP_DELAY 50
#    define LPWR_WAKEUP_DELAY   200

#    define MODIP_WL_PIN C14
#    define MODIP_USB_PIN C15
#    define MODIP_USB_MODE_STATE 1, 1
#    define MODIP_WL_MODE_STATE  0, 1
#    define MODIP_OFF_MODE_STATE 1, 0


// wireless state indicator
#    define WLSIDR_LED_INDEX_BT1 5
#    define WLSIDR_LED_INDEX_BT2 4
#    define WLSIDR_LED_INDEX_BT3 3
#    define WLSIDR_LED_INDEX_2G4 2
#    define WLSIDR_LED_INDEX_USB 1

#    define WLSIDR_LINK_TIMEOUT 30000 // 10秒
#    define WLSIDR_PAIR_TIMEOUT 60000 // 1分钟
#    define WLSIDR_2G4_PAIR_TIMEOUT 20000 // 1分钟
#    define WLSIDR_SUCCEED_TIME 3000  // 3秒

#    define WLSIDR_LINK_INTERVAL 500
#    define WLSIDR_PAIR_INTERVAL 250

#    define WLSIDR_LBACK_COLOR_BT1 RGB_BLUE
#    define WLSIDR_LBACK_COLOR_BT2 RGB_BLUE
#    define WLSIDR_LBACK_COLOR_BT3 RGB_BLUE
#    define WLSIDR_LBACK_COLOR_2G4 RGB_GREEN

#    define WLSIDR_PAIR_COLOR_BT1 RGB_BLUE
#    define WLSIDR_PAIR_COLOR_BT2 RGB_BLUE
#    define WLSIDR_PAIR_COLOR_BT3 RGB_BLUE
#    define WLSIDR_PAIR_COLOR_2G4 RGB_GREEN

#    define BT1_NAME       "THE 87-BT1"
#    define BT2_NAME       "THE 87-BT2"
#    define BT3_NAME       "THE 87-BT3"
#    define DONGLE_PRODUCT "THE 87-2.4G"

#    define WLSIDR_BLINK_INDEX_BAT        0xFD
#    define WLS_INQUIRY_BAT_TIME          1000
#    define RGB_MATRIX_BLINK_COUNT        15
#    define WLSIDR_LED_INDEX_WIN          63
#    define WLSIDR_LED_INDEX_MAC          72
#    define WLSIDR_LED_INDEX_DIR          0xFE
#    define WLSIDR_LED_INDEX_ALL          0xFF

#    define WLSIDR_LED_INDEX_VAI          67
#    define WLSIDR_LED_INDEX_VAD          81
#    define WLSIDR_LED_INDEX_SPD          82
#    define WLSIDR_LED_INDEX_SPI          80
#    define WLSIDR_LED_INDEX_DEB          64
#endif

/* MUSIC */
// #define HS_MUSIC_CUT_PIN D2
// #define HS_MUSIC_ON_STATE 1
// #define HS_MUSIC_MODE_PIN A9
// #define HS_MUSIC_DEFAULT_MODE_STATE 0
#define HS_MUSIC_DEFAULT_TIMER 30
#define HS_MUSIC_CALIBRATION_TIMER 100

#define HS_MUSIC_LED_COUNT  60

#define MCU_INT    A9
#define MCU_SLEEP  D2
#define CPAS_PIN   B6
#define RESET_PIN  B7


#define LED_BLINK_INDEX_ALL 0xFF
#define USB_POWER_DOWN_DELAY 7000
#define USB_CABLE_PIN    A7
#define USB_POWER_EN_PIN B1
#define LED_POWER_EN_PIN A8
#define LED_POWER_EN2_PIN B8
#define LED_INDICATOR_EN_PIN B9

#define BAT_FULL_PIN A15

#define HS_DEBOUNCE

/* RF */
#define RF_DEEPSLEEP_TIME 30 // min

/* User */
#define BOOTMAGIC_DEBOUNCE 1000
#define MATRIX_IO_DELAY    2

/* Battery */
#define BATTERY_CAPACITY_LOW                 5
#define BATTERY_CAPACITY_STOP                0
#define RGB_MATRIX_BAT_INDEX_MAP            {42, 41, 40, 39, 38, 37, 36, 35, 34, 33}

/* UART */
#define UART_DRIVER      SD3
#define UART_TX_PAL_MODE 7
#define UART_RX_PAL_MODE 7
#define UART_TX_PIN      C10
#define UART_RX_PIN      C11

/* Encoder */
#define ENCODER_MAP_KEY_DELAY 1

#define VIA_EEPROM_ALLOW_RESET

/* SPI */
#define SPI_DRIVER   SPIDQ
#define SPI_SCK_PIN  B3
#define SPI_MOSI_PIN B5
#define SPI_MISO_PIN B4

#define I2C_DRIVER        I2CD1
#define I2C1_OPMODE       OPMODE_I2C
#define I2C1_CLOCK_SPEED  100000

/* Flash */
#define EXTERNAL_FLASH_SPI_SLAVE_SELECT_PIN C12
#define WEAR_LEVELING_LOGICAL_SIZE          (WEAR_LEVELING_BACKING_SIZE / 2)

/* RGB Matrix */
#define RGB_MATRIX_FRAMEBUFFER_EFFECTS
#define RGB_MATRIX_KEYPRESSES
#define RGB_TRIGGER_ON_KEYDOWN

/* RGB Light */
#define RGBLIGHT_CUSTOM_DRIVER
#define RGBLIGHT_DISABLE_KEYCODES

/* WS2812 */
#define WS2812_SPI_DRIVER  SPIDM2
#define WS2812_SPI_DIVISOR 24

// 1. 定义模拟输入引脚
// #define ANALOG_JOYSTICK_X_AXIS_PIN C4 // 假设编码器输出接在B12
// // 2. 定义ADC参数
// #define JOYSTICK_AXES_RESOLUTION 8 // ADC分辨率（8位：0-255）
// #define JOYSTICK_AXES_COUNT 1 // 使用1个轴（你只有一个编码器）
// // 3. 定义死区（Deadzone），防止中心点漂移
// #define JOYSTICK_AXIS_DEADZONE 8
/* rgb_record */
#define ENABLE_RGB_MATRIX_RGBR_PLAY
#define RGBREC_CHANNEL_NUM          1
#define EECONFIG_CONFINFO_USE_SIZE (4 + 16 + 16)
#define EECONFIG_RGBREC_USE_SIZE   (RGBREC_CHANNEL_NUM * MATRIX_ROWS * MATRIX_COLS * 2)
#define EECONFIG_USER_DATA_SIZE    (EECONFIG_RGBREC_USE_SIZE + EECONFIG_CONFINFO_USE_SIZE + 1)
#define RGBREC_EECONFIG_ADDR       (uint8_t *)(EECONFIG_USER_DATABLOCK)
#define CONFINFO_EECONFIG_ADDR     (uint32_t *)((uint32_t)RGBREC_EECONFIG_ADDR + (uint32_t)EECONFIG_RGBREC_USE_SIZE)
#define CONFINFO_RGB_MOD_ADDR      (uint32_t *)((uint32_t)RGBREC_EECONFIG_ADDR + (uint32_t)EECONFIG_RGBREC_USE_SIZE + (uint32_t)EECONFIG_CONFINFO_USE_SIZE)