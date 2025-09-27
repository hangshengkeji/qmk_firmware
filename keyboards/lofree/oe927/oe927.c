// Copyright 2024 Su (@isuua)
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H

#ifdef WIRELESS_ENABLE
#    include "wireless.h"
#    include "usb_main.h"
#endif

#ifdef WESTBERRY_COMMON_ENABLE
#    include "westberry_common.h"
#endif

#include "uart.h"

#include "touch.h"
#include "wlsidr.h"
#include "modip.h"
#include "lpkc.h"
#include "led_blink.h"

#include <lib/lib8tion/lib8tion.h>

typedef union {
    uint32_t raw;
    struct {
        uint8_t flag : 1;
        uint8_t backlight_mode : 2;
        uint8_t touch_enable : 1;
        uint8_t mult_filp: 1;
#ifdef WIRELESS_ENABLE
        wireless_devs_t devs;
#endif
    };
} confinfo_t;
confinfo_t confinfo;

_Static_assert(sizeof(confinfo_t) <= 4, "confinfo_t must be <= 4 bytes");

#define keymap_is_mac_system() ((get_highest_layer(default_layer_state) == _MBL) || (get_highest_layer(default_layer_state) == _MFL))
#define keymap_is_base_layer() ((get_highest_layer(default_layer_state) == _BL) || (get_highest_layer(default_layer_state) == _FL))

extern void last_matrix_activity_trigger(void);

void touch_cb(uint8_t value);
uint32_t get_remote_wakeup_delay(void);
bool led_blink_bat_cb(uint8_t led_index);
bool led_blink_all_cb(uint8_t led_index);
void matrix_init_user(void);

uint32_t jump_boot_timer = 0x00;
uint8_t virtual_close    = false;
uint8_t ee_clr_flag      = 0;
uint32_t post_init_timer = 0x00;
bool inqbat_flag         = false;

void eeconfig_confinfo_update(uint32_t raw) {

    eeconfig_update_kb(raw);
}

uint32_t eeconfig_confinfo_read(void) {

    return eeconfig_read_kb();
}

void eeconfig_confinfo_default(void) {

    confinfo.flag           = true;
    confinfo.backlight_mode = 1;
    confinfo.touch_enable   = true;
    confinfo.mult_filp      = false;
#ifdef WIRELESS_ENABLE
    confinfo.devs.now = DEVS_USB;
    confinfo.devs.bt  = DEVS_BT1;
    confinfo.devs.wl  = DEVS_BT1;
#endif

    eeconfig_confinfo_update(confinfo.raw);
}

void eeconfig_confinfo_init(void) {

    confinfo.raw = eeconfig_confinfo_read();
    if (!confinfo.raw) {
        eeconfig_confinfo_default();
        keymap_config.nkro = 1;
        eeconfig_update_keymap(keymap_config.raw);
    }
}

static PWMConfig pwmCFG = {
    .frequency = 0xFFFF, /* PWM clock frequency  */
    .period    = 256,    /* PWM period in counter ticks. e.g. clock frequency is 10KHz, period is 256 ticks then t_period is 25.6ms */
};

void keyboard_post_init_kb(void) {

#ifdef CONSOLE_ENABLE
    debug_enable = true;
#endif
    ee_clr_flag = 0;

    jump_boot_timer = 0x00;

    eeconfig_confinfo_init();

#ifdef USB_CABLE_PIN
    gpio_set_pin_input(USB_CABLE_PIN);
#endif

#ifdef BAT_FULL_PIN
    gpio_set_pin_input_high(BAT_FULL_PIN);
#endif

#ifdef LED_POWER_EN_PIN
    gpio_set_pin_output(LED_POWER_EN_PIN);
    gpio_write_pin_high(LED_POWER_EN_PIN);
#endif

#ifdef USB_POWER_EN_PIN
    gpio_write_pin_low(USB_POWER_EN_PIN);
    gpio_set_pin_output(USB_POWER_EN_PIN);
#endif

    gpio_set_pin_output(LED_PIN_MODE_DET);
    gpio_write_pin_low(LED_PIN_MODE_DET);

    palSetPadMode(PAL_PORT(LED_PIN_BAT_WHITE_DET), PAL_PAD(LED_PIN_BAT_WHITE_DET), PAL_MODE_ALTERNATE(2));
    pwmCFG.channels[0].mode = PWM_OUTPUT_ACTIVE_HIGH;
    pwmStart(&PWMD4, &pwmCFG);

    palSetPadMode(PAL_PORT(LED_PIN_BAT_RED_DET), PAL_PAD(LED_PIN_BAT_RED_DET), PAL_MODE_ALTERNATE(1));
    pwmCFG.channels[0].mode = PWM_OUTPUT_ACTIVE_HIGH;
    pwmStart(&PWMD1, &pwmCFG);

    // gpio_set_pin_output(LED_PIN_BAT_WHITE_DET);
    // gpio_write_pin_low(LED_PIN_BAT_WHITE_DET);

    // gpio_set_pin_output(LED_PIN_BAT_RED_DET);
    // gpio_write_pin_low(LED_PIN_BAT_RED_DET);

#ifdef WIRELESS_ENABLE
    modip_init();
    wireless_init(&confinfo.devs);
    post_init_timer = timer_read32();
#endif

    led_blink_register(LED_BLINK_INDEX_BAT, 0, 1, 500, false, led_blink_bat_cb);

    touch_init(touch_cb);

    if (confinfo.touch_enable) {
        touch_wakeup();
    } else {
        touch_close();
    }

    backlight_level_noeeprom(0);
    if (is_backlight_breathing() && (confinfo.backlight_mode == 2)) {
        breathing_enable();
    }

    matrix_init_user();

    keyboard_post_init_user();
}

void led_power_enable(void) {

// #ifdef LED_POWER_EN_PIN
//     gpio_write_pin_high(LED_POWER_EN_PIN);
// #endif

    palSetPadMode(PAL_PORT(LED_PIN_BAT_WHITE_DET), PAL_PAD(LED_PIN_BAT_WHITE_DET), PAL_MODE_ALTERNATE(2));
    pwmEnableChannel(&PWMD4, 0, 0x00);

    palSetPadMode(PAL_PORT(LED_PIN_BAT_RED_DET), PAL_PAD(LED_PIN_BAT_RED_DET), PAL_MODE_ALTERNATE(1));
    pwmEnableChannel(&PWMD1, 0, 0x00);

    gpio_set_pin_output(LED_PIN_MODE_DET);
    gpio_write_pin_low(LED_PIN_MODE_DET);
}

void led_power_disable(void) {

// #ifdef LED_POWER_EN_PIN
//     gpio_write_pin_low(LED_POWER_EN_PIN);
// #endif

    pwmDisableChannel(&PWMD4, 0);
    pwmDisableChannel(&PWMD1, 0);
    gpio_set_pin_input_low(LED_PIN_BAT_WHITE_DET);
    gpio_set_pin_input_low(LED_PIN_BAT_RED_DET);

    gpio_set_pin_input_low(LED_PIN_MODE_DET);
}

bool led_power_is_enabled(void) {
    bool state = true;

// #ifdef LED_POWER_EN_PIN
//     state = gpio_read_pin(LED_POWER_EN_PIN);
// #endif

    return state;
}

bool usb_cable_is_connected(void) {

#ifdef USB_CABLE_PIN
    return gpio_read_pin(USB_CABLE_PIN);
#else
    return true;
#endif
}

void usb_power_connect(void) {

#ifdef USB_POWER_EN_PIN
    gpio_write_pin_low(USB_POWER_EN_PIN);
#endif
}

void usb_power_disconnect(void) {

#ifdef USB_POWER_EN_PIN
    gpio_write_pin_high(USB_POWER_EN_PIN);
#endif
}

void lpwr_pre_stop_hook(void) {

    setPinInputHigh(TOUCH_UART_TX_PIN);
}

void suspend_power_down_kb(void) {

#ifdef BAT_FULL_PIN
    gpio_write_pin_low(BAT_FULL_PIN);
    gpio_set_pin_output(BAT_FULL_PIN);
#endif

    led_power_disable();
    if (confinfo.touch_enable) {
        touch_deep_sleep(TOUCH_DEEPSLEEP_TIME);
        touch_wait_free(1000);
    }

#ifdef WIRELESS_ENABLE
    if (wireless_get_devs().now != DEVS_USB) {
        usb_power_disconnect();
    }

    if (wireless_get_devs().now == DEVS_USB) {
#    ifdef USB_CABLE_PIN
        if ((lpwr_get_mode() == LPWR_MODE_SHUTDOWN) || !usb_cable_is_connected()) {
            usb_transport_enable(false);
            lpwr_set_manual_shutdown(true);
        }
#    endif
    }
#endif

    if ((wireless_get_devs().now != DEVS_USB) || !usb_cable_is_connected()) {
        setPinInputHigh(TOUCH_UART_RX_PIN);
        waitInputPinDelay();
    }

    suspend_power_down_user();
}

void suspend_wakeup_init_kb(void) {

    palSetLineMode(TOUCH_UART_TX_PIN, PAL_MODE_ALTERNATE(TOUCH_UART_TX_PAL_MODE) | PAL_OUTPUT_TYPE_PUSHPULL | PAL_WB32_PUPDR_PULLUP | PAL_OUTPUT_SPEED_HIGHEST);
    palSetLineMode(TOUCH_UART_RX_PIN, PAL_MODE_ALTERNATE(TOUCH_UART_RX_PAL_MODE) | PAL_OUTPUT_TYPE_PUSHPULL | PAL_WB32_PUPDR_PULLUP | PAL_OUTPUT_SPEED_HIGHEST);

    led_power_enable();
    if (confinfo.touch_enable) touch_wakeup();

    if (!post_init_timer) {
        extern backlight_config_t backlight_config;
        backlight_set(backlight_config.enable ? backlight_config.level : 0);
    }

    lpkc_init();

#ifdef WIRELESS_ENABLE
    if ((wireless_get_devs().now == DEVS_USB) && (USB_DRIVER.state == USB_STOP)) {
        usb_power_connect();
        restart_usb_driver(&USBD1);
    }

    wireless_devs_change(wireless_get_devs().now, wireless_get_devs().now, false);
#endif

#ifdef BAT_FULL_PIN
    gpio_set_pin_input_high(BAT_FULL_PIN);
#endif

    suspend_wakeup_init_user();
}

bool bat_full_flag  = false;
bool charging       = true;
uint8_t hs_now_mode = 0;

void housekeeping_task_user(void) {

#ifndef MD_THREAD_MODE
  
    void md_main_task_hook(void);
    md_main_task_hook();
#endif

    touch_task();

    static uint32_t pwr_timer = 0x00;
    if ((gpio_read_pin(MODIP_WL_PIN) == 0) && (gpio_read_pin(MODIP_USB_PIN) == 1)) {
        if (!pwr_timer) {
            pwr_timer = timer_read32();
        }

        if (timer_elapsed32(pwr_timer) >= 10) {
            pwr_timer = timer_read32();
            if ((gpio_read_pin(MODIP_WL_PIN) == 0) && (gpio_read_pin(MODIP_USB_PIN) == 1)) {
                if (lpwr_get_state() == LPWR_NORMAL) {
                    backlight_level_noeeprom(0);
                    if (!usb_cable_is_connected()) {
                        lpwr_set_manual_shutdown(true);
                    }
                    virtual_close = 1;
                }
            }
        }
    } else {
        pwr_timer = timer_read32();
        if (virtual_close == 1) {
            mcu_reset();
        }
    }

    if (jump_boot_timer && (timer_elapsed32(jump_boot_timer) >= 500)) {
        reset_keyboard();
    }

    // if (charging && (bat_full_flag)) {
    //     hs_now_mode = MD_SND_CMD_DEVCTRL_CHARGING_DONE;
    // } else if (charging) {
    //     hs_now_mode = MD_SND_CMD_DEVCTRL_CHARGING;
    // } else {
    //     hs_now_mode = MD_SND_CMD_DEVCTRL_CHARGING_STOP;
    // }

    // if (!hs_current_time || timer_elapsed32(hs_current_time) > 1000) {
    //     hs_current_time = timer_read32();
    //     md_send_devctrl(hs_now_mode);
    // }
}

bool lpwr_is_allow_timeout_hook(void) {

    if (wireless_get_devs().now == DEVS_USB) {
        return false;
    }

    return true;
}

bool lpwr_is_allow_presleep_hook(void) {

    virtual_close = usb_cable_is_connected() ? 2 : 0;
    if (virtual_close) {
        backlight_level_noeeprom(0);
        return false;
    }

    return true;
}

void lpwr_post_stop_hook(void) {

    if ((lpwr_get_mode() == LPWR_MODE_SHUTDOWN) && (lpwr_get_state() == LPWR_WAKEUP)) {
        mcu_reset();
        while (1) {}
    }

    if (lpwr_get_state() == LPWR_WAKEUP){
        palSetLineMode(TOUCH_UART_TX_PIN, PAL_MODE_ALTERNATE(TOUCH_UART_TX_PAL_MODE) | PAL_OUTPUT_TYPE_PUSHPULL | PAL_WB32_PUPDR_PULLUP | PAL_OUTPUT_SPEED_HIGHEST);
        // palSetLineMode(TOUCH_UART_RX_PIN, PAL_MODE_ALTERNATE(TOUCH_UART_RX_PAL_MODE) | PAL_OUTPUT_TYPE_PUSHPULL | PAL_WB32_PUPDR_PULLUP | PAL_OUTPUT_SPEED_HIGHEST);
    }
}

void lpwr_wakeup_hook(void) {

    virtual_close = 0;
    hs_now_mode = 0;

    if (keymap_is_base_layer()) {
        uint8_t data[] = {0xA7, 0x80};
        md_send_smsg_pkt(data, 2);
    }
}

void palcallback_hook(uint8_t line) {

    switch (line) {
#ifdef USB_CABLE_PIN
        case PAL_PAD(USB_CABLE_PIN): {
            lpwr_set_wakeupcd(LPWR_WAKEUP_CABLE);
        } break;
#endif
#ifdef MODIP_SWITCH_PIN
        case PAL_PAD(MODIP_SWITCH_PIN): {
            lpwr_set_wakeupcd(LPWR_WAKEUP_SWITCH);
        } break;
#endif
#ifdef MODIP_WL_PIN
        case PAL_PAD(MODIP_WL_PIN): {
            lpwr_set_wakeupcd(LPWR_WAKEUP_SWITCH);
        } break;
#endif
#ifdef MODIP_USB_PIN
        case PAL_PAD(MODIP_USB_PIN): {
            lpwr_set_wakeupcd(LPWR_WAKEUP_SWITCH);
        } break;
#endif
        case PAL_PAD(TOUCH_UART_RX_PIN): {
            lpwr_set_wakeupcd(LPWR_WAKEUP_MATRIX);
        } break;
        default:
            break;
    }
}

void lpwr_exti_init_hook(lpwr_mode_t mode) {

#ifdef USB_CABLE_PIN
    // if ((mode == LPWR_MODE_SHUTDOWN) || (mode == LPWR_MODE_LOWBAT))
    {
        palEnableLineEvent(USB_CABLE_PIN, PAL_EVENT_MODE_RISING_EDGE);
    }
#endif

#ifdef MODIP_SWITCH_PIN
    palEnableLineEvent(MODIP_SWITCH_PIN, PAL_EVENT_MODE_RISING_EDGE);
#endif

#ifdef MODIP_WL_PIN
    palEnableLineEvent(MODIP_WL_PIN, PAL_EVENT_MODE_RISING_EDGE);
#endif

#ifdef MODIP_USB_PIN
    palEnableLineEvent(MODIP_USB_PIN, PAL_EVENT_MODE_RISING_EDGE);
#endif

    if (mode == LPWR_MODE_TIMEOUT && confinfo.touch_enable) {
        palEnableLineEvent(TOUCH_UART_RX_PIN, PAL_EVENT_MODE_FALLING_EDGE);
    }

#ifdef ENCODER_ENABLE
    static pin_t encoder_pins[1] = ENCODER_A_PINS;
    for (uint8_t i = 0; i < ARRAY_SIZE(encoder_pins); i++) {
        palEnableLineEvent(encoder_pins[i], PAL_EVENT_MODE_BOTH_EDGES);
    }
#endif
}

// wireless loop
void wireless_post_task(void) {

    // auto switching devs
    if (post_init_timer && timer_elapsed32(post_init_timer) >= 100) {
        post_init_timer = 0x00;
        md_sleep_bt(true);
        md_sleep_2g4(true);
        if (!modip_scan(true) && (wireless_get_devs().now != DEVS_USB)) {
            wireless_devs_change(!wireless_get_devs().now, wireless_get_devs().now, false);
        }
        backlight_init();
        extern backlight_config_t backlight_config;
        if (get_backlight_level() == 0) {
            backlight_level(2);
        }
        backlight_set(backlight_config.enable ? backlight_config.level : 0);

        if (keymap_is_base_layer()){
            uint8_t data[] = {0xA7,0x80};
            md_send_smsg_pkt(data,2);
        }
        else{
            uint8_t data[] = {0xA7,0x81};
            md_send_smsg_pkt(data,2);
        }
    }

    if (!post_init_timer) {
        modip_scan(false);
        wlsidr_task();
        led_blink_task();
    }
}

void wireless_devs_change_user(uint8_t old_devs, uint8_t new_devs, bool reset) {

    eeconfig_confinfo_update(confinfo.raw);

    if (wireless_get_devs().now == DEVS_2G4) {
        md_send_devctrl(0xFF);
    } else if (wireless_get_devs().now != DEVS_USB) {
        md_send_devctrl(0x80 + 10);
    }
}

bool process_record_wls(uint16_t keycode, keyrecord_t *record) {

    switch (keycode) {
        case BT_PRF1: {
            if (record->event.pressed && (wireless_get_devs().now != DEVS_BT1)) {
                modip_mode_t mode = modip_get_mode();
                if ((mode == MODIP_MODE_BT) || (mode == MODIP_MODE_WIRELESS) || (mode == MODIP_MODE_NONE))
                    wireless_devs_change(wireless_get_devs().now, DEVS_BT1, false);
            }
        } break;
        case BT_PRF2: {
            if (record->event.pressed && (wireless_get_devs().now != DEVS_BT2)) {
                modip_mode_t mode = modip_get_mode();
                if ((mode == MODIP_MODE_BT) || (mode == MODIP_MODE_WIRELESS) || (mode == MODIP_MODE_NONE))
                    wireless_devs_change(wireless_get_devs().now, DEVS_BT2, false);
            }
        } break;
        case BT_PRF3: {
            if (record->event.pressed && (wireless_get_devs().now != DEVS_BT3)) {
                modip_mode_t mode = modip_get_mode();
                if ((mode == MODIP_MODE_BT) || (mode == MODIP_MODE_WIRELESS) || (mode == MODIP_MODE_NONE))
                    wireless_devs_change(wireless_get_devs().now, DEVS_BT3, false);
            }
        } break;
        case BT_PRF4: {
            if (record->event.pressed && (wireless_get_devs().now != DEVS_BT4)) {
                modip_mode_t mode = modip_get_mode();
                if ((mode == MODIP_MODE_BT) || (mode == MODIP_MODE_WIRELESS) || (mode == MODIP_MODE_NONE))
                    wireless_devs_change(wireless_get_devs().now, DEVS_BT4, false);
            }
        } break;
        case BT_PRF5: {
            if (record->event.pressed && (wireless_get_devs().now != DEVS_BT5)) {
                modip_mode_t mode = modip_get_mode();
                if ((mode == MODIP_MODE_BT) || (mode == MODIP_MODE_WIRELESS) || (mode == MODIP_MODE_NONE))
                    wireless_devs_change(wireless_get_devs().now, DEVS_BT5, false);
            }
        } break;
        case OU_2P4G: {
            if (record->event.pressed && (wireless_get_devs().now != DEVS_2G4)) {
                modip_mode_t mode = modip_get_mode();
                if ((mode == MODIP_MODE_2G4) || (mode == MODIP_MODE_WIRELESS) || (mode == MODIP_MODE_NONE))
                    wireless_devs_change(wireless_get_devs().now, DEVS_2G4, false);
            }
        } break;
        case OU_USB: {
            if (record->event.pressed && (wireless_get_devs().now != DEVS_USB)) {
                modip_mode_t mode = modip_get_mode();
                if ((mode == MODIP_MODE_USB) || (mode == MODIP_MODE_NONE))
                    wireless_devs_change(wireless_get_devs().now, DEVS_USB, false);
            }
        } break;
        default: {
            return true;
        } break;
    }

    return false;
}

uint8_t bat_blink      = 0; // 0: off 1: full 2: charging 3: low
uint16_t bat_breathing = 0;
bool hs_frequency_test = false, hs_color_test = false;

bool led_blink_bat_cb(uint8_t led_index) {
    static uint32_t bat_timer = 0;
    uint8_t bat_level         = *md_getp_bat();

    if (hs_frequency_test && hs_color_test) {
        static uint32_t last_matrix_activity_timer_hs = 0x00;

        if (!bat_timer) bat_timer = sync_timer_read32();
        if (last_matrix_activity_timer_hs != last_matrix_activity_time()) {
            last_matrix_activity_timer_hs = last_matrix_activity_time();
            bat_timer                     = sync_timer_read32();
        }
        backlight_level_noeeprom(0);
        led_blink_mod_value(led_index, 0xFF);
        led_blink_repeat(led_index, 1, 250, false);
        bat_breathing = 0;
        bat_blink     = 4;
        wlsidr_blink_set_timer(timer_read32());
        return true;
    }

#ifdef USB_CABLE_PIN
    charging = usb_cable_is_connected();
#endif

#ifdef BAT_FULL_PIN
    bat_full_flag = gpio_read_pin(BAT_FULL_PIN);
#else
    bat_full_flag = bat_level >= 100;
#endif

    // charging  = true;
    // bat_level = 10;

    if (charging && (bat_full_flag)) {
        bat_timer = 0;
        led_blink_mod_value(led_index, 0xFF);
        led_blink_repeat(led_index, 1, 300, true);
        bat_breathing = 0;
        bat_blink     = 1;
        if (hs_now_mode != MD_SND_CMD_DEVCTRL_CHARGING_DONE){
            hs_now_mode  = MD_SND_CMD_DEVCTRL_CHARGING_DONE;
            md_send_devctrl(hs_now_mode);
        }

    } else if (charging) {
        uint8_t temp;
        uint8_t value = 0xFF;
        bat_timer     = 0;

        bat_breathing += 5;
        if (bat_breathing >= 0x1FF) {
            bat_breathing = 5;
        }

        if (bat_breathing & 0x100) {
            temp = 0xFF - (bat_breathing & 0xFF);
        } else {
            temp = bat_breathing & 0xFF;
        }

        value = scale8(value, temp);

        led_blink_mod_value(led_index, value);
        led_blink_repeat(led_index, 1, 25, true);

        bat_blink   = 2;
        if (hs_now_mode != MD_SND_CMD_DEVCTRL_CHARGING){
            hs_now_mode  = MD_SND_CMD_DEVCTRL_CHARGING;
            md_send_devctrl(hs_now_mode);
        }
    } else if ((bat_level <= 20) && (bat_level > 5)) {
        static uint32_t last_matrix_activity_timer = 0x00;
        uint8_t temp;
        uint8_t value = 0xFF;

        bat_breathing += 5;
        if (bat_breathing >= 0x1FF) {
            bat_breathing = 5;
        }

        if (bat_breathing & 0x100) {
            temp = 0xFF - (bat_breathing & 0xFF);
        } else {
            temp = bat_breathing & 0xFF;
        }

        value = scale8(value, temp);

        if (!bat_timer) bat_timer = sync_timer_read32();
        if (last_matrix_activity_timer != last_matrix_activity_time()) {
            last_matrix_activity_timer = last_matrix_activity_time();
            bat_timer                  = sync_timer_read32();
        }

        led_blink_mod_value(led_index, value);
        led_blink_repeat(led_index, 1, 25, true);
        bat_blink   = 3;
        if (hs_now_mode != MD_SND_CMD_DEVCTRL_CHARGING_STOP){
            hs_now_mode  = MD_SND_CMD_DEVCTRL_CHARGING_STOP;
            md_send_devctrl(hs_now_mode);
        }
    } else if (bat_level <= 5) {
        static uint32_t last_matrix_activity_timer = 0x00;

        if (!bat_timer) bat_timer = sync_timer_read32();
        if (last_matrix_activity_timer != last_matrix_activity_time()) {
            last_matrix_activity_timer = last_matrix_activity_time();
            bat_timer                  = sync_timer_read32();
        }

        if (sync_timer_elapsed32(bat_timer) > 10000) {

            if (bat_level <= 3) {
                bat_timer = 0;
                lpwr_set_manual_lowbat(true);
            }
        }

        led_blink_mod_value(led_index, 0xFF);
        led_blink_repeat(led_index, 1, 250, false);
        bat_breathing = 0;
        bat_blink     = 4;
        if (hs_now_mode != MD_SND_CMD_DEVCTRL_CHARGING_STOP){
            hs_now_mode  = MD_SND_CMD_DEVCTRL_CHARGING_STOP;
            md_send_devctrl(hs_now_mode);
        }
    } else {
        bat_timer     = 0;
        bat_breathing = 0;
        bat_blink     = 0;
        if (hs_now_mode != MD_SND_CMD_DEVCTRL_CHARGING_STOP){
            hs_now_mode  = MD_SND_CMD_DEVCTRL_CHARGING_STOP;
            md_send_devctrl(hs_now_mode);
        }
    }
    // md_send_devctrl(hs_now_mode);
    return true;
}

bool led_blink_all_cb(uint8_t led_index) {

    if (ee_clr_flag == 1) {
        ee_clr_flag = 2;

        led_blink_repeat(LED_BLINK_INDEX_ALL, 1, 500, true);
        md_switch_to_bt1(true, BT1_NAME);
        md_switch_to_bt2(true, BT2_NAME);
        md_switch_to_bt3(true, BT3_NAME);
        eeconfig_disable();
        return true;
    }

    if (ee_clr_flag == 2) {
#ifdef NO_RESET
        eeconfig_init();
#else
        // soft_reset_keyboard();
        mcu_reset();
#endif
        return false;
    }

    return false;
}

bool led_blink_set_hook(uint8_t led_index, bool on, uint8_t value) {

    switch (led_index) {
        case LED_BLINK_INDEX_BAT: {
            if (ee_clr_flag) {
                pwmEnableChannel(&PWMD4, 0, 0x00);

                return false;
            }

            if (!bat_blink || ((lpwr_get_state() != LPWR_NORMAL) && (lpwr_get_state() != LPWR_PRESLEEP))) {

                pwmEnableChannel(&PWMD4, 0, 0x00);
                pwmEnableChannel(&PWMD1, 0, 0x00);
                return false;
            }

            switch (bat_blink) {
                case 1: { // full
                    if (on) {
                        pwmEnableChannel(&PWMD4, 0, value);
                    } else {
                        pwmEnableChannel(&PWMD4, 0, 0x00);
                    }
                    pwmEnableChannel(&PWMD1, 0, 0x00);
                } break;
                case 2: { // charging
                    if (on) {
                        pwmEnableChannel(&PWMD4, 0, value);
                    } else {
                        pwmEnableChannel(&PWMD4, 0, 0x00);
                    }
                    pwmEnableChannel(&PWMD1, 0, 0x00);
                } break;
                case 3: { // low
                    if (on) {
                        pwmEnableChannel(&PWMD1, 0, value);
                    } else {
                        pwmEnableChannel(&PWMD1, 0, 0x00);
                    }
                    pwmEnableChannel(&PWMD4, 0, 0x00);
                } break;
                case 4: { // off
                    if (on) {
                        pwmEnableChannel(&PWMD1, 0, value);
                    } else {
                        pwmEnableChannel(&PWMD1, 0, 0x00);
                    }
                    pwmEnableChannel(&PWMD4, 0, 0x00);
                } break;
            }

            return false;
        } break;
        case WLSIDR_LED_INDEX_BT1:
        case WLSIDR_LED_INDEX_BT2:
        case WLSIDR_LED_INDEX_BT3:
        case WLSIDR_LED_INDEX_2G4: {

            if (virtual_close) {
                gpio_write_pin_low(LED_PIN_MODE_DET);
                return false;
            }

            if (on) {
                gpio_write_pin_high(LED_PIN_MODE_DET);
            } else {
                gpio_write_pin_low(LED_PIN_MODE_DET);
            }

            return false;
        } break;
        case LED_BLINK_INDEX_ALL: {
            void backlight_pins_on(void);
            void backlight_pins_off(void);

            if (on) {
                pwmEnableChannel(&PWMD1, 0, 0xFF);
                backlight_pins_on();
            } else {
                pwmEnableChannel(&PWMD1, 0, 0x00);
                backlight_pins_off();
            }

            return false;
        } break;
        default:
            break;
    }

    return true;
}

bool process_lpkc_keycode(uint16_t keycode, bool pressed) {

    switch (keycode) {
#ifdef WIRELESS_ENABLE
        case BT_PRF1: {
            if (pressed && (wireless_get_devs().now == DEVS_BT1)) {
                wireless_devs_change(wireless_get_devs().now, DEVS_BT1, true);
            }
        } break;
        case BT_PRF2: {
            if (pressed && (wireless_get_devs().now == DEVS_BT2)) {
                wireless_devs_change(wireless_get_devs().now, DEVS_BT2, true);
            }
        } break;
        case BT_PRF3: {
            if (pressed && (wireless_get_devs().now == DEVS_BT3)) {
                wireless_devs_change(wireless_get_devs().now, DEVS_BT3, true);
            }
        } break;
        case BT_PRF4: {
            if (pressed && (wireless_get_devs().now == DEVS_BT4)) {
                wireless_devs_change(wireless_get_devs().now, DEVS_BT4, true);
            }
        } break;
        case BT_PRF5: {
            if (pressed && (wireless_get_devs().now == DEVS_BT5)) {
                wireless_devs_change(wireless_get_devs().now, DEVS_BT5, true);
            }
        } break;
        case OU_2P4G: {
            if (pressed && (wireless_get_devs().now == DEVS_2G4)) {
                wireless_devs_change(wireless_get_devs().now, DEVS_2G4, true);
            }
        } break;
        // case KC_ESC: {
        //     if (pressed) {
        //         lpwr_set_manual_shutdown(true);
        //     }
        // } break;
        // case KC_DEL: {
        //     if (pressed) {
        //         lpwr_set_manual_timeout(true);
        //     }
        // } break;
#endif
        case BT_TEST:{
            if (pressed){
                md_send_devctrl(0x62);
                wireless_devs_change(wireless_get_devs().now, DEVS_BT2, true);
            }
        }break;
        case EE_CLR: {
            if (pressed) {
                ee_clr_flag = 1;
                led_blink_register(LED_BLINK_INDEX_ALL, 0xFF, 2, 500, false, led_blink_all_cb);
                backlight_level_noeeprom(0);
            }
        } break;
        case TH_EN: {
            if (pressed) {
                confinfo.touch_enable = !confinfo.touch_enable;
                eeconfig_confinfo_update(confinfo.raw);
                if (confinfo.touch_enable) {
                    touch_wakeup();
                    touch_wait_free(1000);
                    touch_wakeup();
                } else {
                    touch_close();
                    touch_wait_free(1000);
                    touch_close();
                }
            }
        } break;
        default:
            break;
    }

    return true;
}

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {

    switch (wlsidr_blink_get_state()) {
        case WLSIDR_STATE_LINK: {
            wlsidr_blink_set_timer(timer_read32());
        } break;
        case WLSIDR_STATE_PAIR: {
            wlsidr_blink_set_timer(timer_read32());
        } break;
        default:
            break;
    }

    if (virtual_close) {
        if (lpwr_get_state() != LPWR_NORMAL) {
            virtual_close = 0;
            lpwr_set_state(LPWR_WAKEUP);
        } else {
            return false;
        }
    }

    switch (keycode) {
        case TH_EN: {
            process_lpkc_record(keycode, 2000, record->event.pressed);
        } break;
        default: {
            process_lpkc_record(keycode, 3000, record->event.pressed);
        } break;
    }

    if (process_record_user(keycode, record) != true) {
        return false;
    }

#ifdef WIRELESS_ENABLE
    if (process_record_wls(keycode, record) != true) {
        return false;
    }
#endif

    switch (keycode) {
        // case QK_BOOT: {
        //     if (record->event.pressed) {
        //         dprintf("into boot!!!\r\n");
        //         eeconfig_disable();
        //         bootloader_jump();
        //     }
        // } break;
        case HS_MULT:{
            if (record->event.pressed) {
                confinfo.mult_filp = !confinfo.mult_filp;
                eeconfig_confinfo_update(confinfo.raw);
            }
            return false;
        }break;
        case KC_F1:{
            if (confinfo.mult_filp) {
                if (record->event.pressed) {
                    register_code16(KC_BRID);
                } else {
                    unregister_code16(KC_BRID);
                }
                return false;
            }
        }break;
        case KC_F2:{
            if (confinfo.mult_filp) {
                if (record->event.pressed) {
                    register_code16(KC_BRIU);
                } else {
                    unregister_code16(KC_BRIU);
                }
                return false;
            }
        }break;
        case KC_F3:{
            if (confinfo.mult_filp) {
                if (record->event.pressed) {
                if (keymap_is_mac_system()) {
                    register_code16(KC_LGUI);
                    register_code16(KC_TAB);
                } else {
                    register_code16(KC_LCTL);
                    register_code16(KC_UP);
                }

            } else {
                if (keymap_is_mac_system()) {
                    unregister_code16(KC_LGUI);
                    unregister_code16(KC_TAB);
                } else {
                    unregister_code16(KC_LCTL);
                    unregister_code16(KC_UP);
                }
            }
                return false;
            }
        }break;
        case KC_F4:{
            if (confinfo.mult_filp) {
                if (record->event.pressed) {
                    register_code16(KC_WSCH);
                } else {
                    unregister_code16(KC_WSCH);
                }
                return false;
            }
        }break;
        case KC_F5:{
            if (confinfo.mult_filp) {
                if (record->event.pressed) {
                    if (get_backlight_level() == 0) return false;
                    if ((confinfo.backlight_mode != 1) && (confinfo.backlight_mode != 2)) return false;
                    backlight_decrease();
                } 
                return false;
            }
        }break;
        case KC_F6:{
            if (confinfo.mult_filp) {
                if (record->event.pressed) {
                    if (get_backlight_level() == 4) return false;
                    if ((confinfo.backlight_mode != 1) && (confinfo.backlight_mode != 2)) return false;
                    backlight_increase();
                } 
                return false;
            }
        }break;
        case KC_F7:{
            if (confinfo.mult_filp) {
                if (record->event.pressed) {
                    register_code16(KC_MPRV);
                } else {
                    unregister_code16(KC_MPRV);
                }
                return false;
            }
        }break;
        case KC_F8:{
            if (confinfo.mult_filp) {
                if (record->event.pressed) {
                    register_code16(KC_MPLY);
                } else {
                    unregister_code16(KC_MPLY);
                }
                return false;
            }
        }break;
        case KC_F9:{
            if (confinfo.mult_filp) {
                if (record->event.pressed) {
                    register_code16(KC_MNXT);
                } else {
                    unregister_code16(KC_MNXT);
                }
                return false;
            }
        }break;
        case KC_F10:{
            if (confinfo.mult_filp) {
                if (record->event.pressed) {
                    register_code16(KC_MUTE);
                } else {
                    unregister_code16(KC_MUTE);
                }
                return false;
            }
        }break;
        case KC_F11:{
            if (confinfo.mult_filp) {
                if (record->event.pressed) {
                    register_code16(KC_VOLD);
                } else {
                    unregister_code16(KC_VOLD);
                }
                return false;
            }
        }break;
        case KC_F12:{
            if (confinfo.mult_filp) {
                if (record->event.pressed) {
                    register_code16(KC_VOLU);
                } else {
                    unregister_code16(KC_VOLU);
                }
                return false;
            }
        }break;
        
        case KC_BRID:{
            if (confinfo.mult_filp) {
                if (record->event.pressed) {
                    register_code16(KC_F1);
                } else {
                    unregister_code16(KC_F1);
                }
                return false;
            }
        }break;

        case KC_BRIU:{
            if (confinfo.mult_filp) {
                if (record->event.pressed) {
                    register_code16(KC_F2);
                } else {
                    unregister_code16(KC_F2);
                }
                return false;
            }
        }break;

        case KC_WSCH:{
            if (confinfo.mult_filp) {
                if (record->event.pressed) {
                    register_code16(KC_F4);
                } else {
                    unregister_code16(KC_F4);
                }
                return false;
            }
        }break;
        case KC_MPRV:{
            if (confinfo.mult_filp) {
                if (record->event.pressed) {
                    register_code16(KC_F7);
                } else {
                    unregister_code16(KC_F7);
                }
                return false;
            }
        }break;
        case KC_MPLY:{
            if (confinfo.mult_filp) {
                if (record->event.pressed) {
                    register_code16(KC_F8);
                } else {
                    unregister_code16(KC_F8);
                }
                return false;
            }
        }break;
        case KC_MNXT:{
            if (confinfo.mult_filp) {
                if (record->event.pressed) {
                    register_code16(KC_F9);
                } else {
                    unregister_code16(KC_F9);
                }
                return false;
            }
        }break;
        case KC_MUTE:{
            if (confinfo.mult_filp) {
                if (record->event.pressed) {
                    register_code16(KC_F10);
                } else {
                    unregister_code16(KC_F10);
                }
                return false;
            }
        }break;
        case KC_VOLD:{
            if (confinfo.mult_filp) {
                if (record->event.pressed) {
                    register_code16(KC_F11);
                } else {
                    unregister_code16(KC_F11);
                }
                return false;
            }
        }break;
        case KC_VOLU:{
            if (confinfo.mult_filp) {
                if (record->event.pressed) {
                    register_code16(KC_F12);
                } else {
                    unregister_code16(KC_F12);
                }
                return false;
            }
        }break;

        case BL_UP: {
            if (confinfo.mult_filp) {
                if (record->event.pressed) {
                    register_code16(KC_F6);
                } else {
                    unregister_code16(KC_F6);
                }
                return false;
            }
            if (get_backlight_level() == 4) return false;
            if ((confinfo.backlight_mode != 1) && (confinfo.backlight_mode != 2)) return false;
            // if ((get_backlight_level() == 4) || (confinfo.backlight_mode == 2)) return false;
        } break;
        case BL_DOWN: {
            if (confinfo.mult_filp) {
                if (record->event.pressed) {
                    register_code16(KC_F5);
                } else {
                    unregister_code16(KC_F5);
                }
                return false;
            }
            if (get_backlight_level() == 0) return false;
            if ((confinfo.backlight_mode != 1) && (confinfo.backlight_mode != 2)) return false;
            // if ((get_backlight_level() == 0) || (confinfo.backlight_mode == 2)) return false;
        } break;
        case HS_DIR: {
            if (record->event.pressed) {
                confinfo.backlight_mode = (confinfo.backlight_mode + 1) % 3;
                eeconfig_confinfo_update(confinfo.raw);

                switch (confinfo.backlight_mode) {
                    case 2: {
                        if (get_backlight_level() == 0) {
                            backlight_level(2);
                        }
                        backlight_disable_breathing();
                        backlight_enable_breathing();
                        backlight_enable();
                    } break;
                    case 0: {
                        backlight_disable();
                    } break;
                    default: {
                        confinfo.backlight_mode = 0;
                        eeconfig_confinfo_update(confinfo.raw);
                    } // break;
                    case 1: {
                        if (get_backlight_level() == 0) {
                            backlight_level(2);
                        }
                        backlight_disable_breathing();
                        backlight_enable();
                    } break;
                }
            }

            return false;
        } break;
        case KC_ENCW: { //
            if (record->event.pressed) {

                if (confinfo.backlight_mode > 0) {
                    confinfo.backlight_mode--;
                } else {
                    confinfo.backlight_mode = 2;
                }

                eeconfig_confinfo_update(confinfo.raw);

                switch (confinfo.backlight_mode) {
                    case 2: {
                        if (get_backlight_level() == 0) {
                            backlight_level(2);
                        }
                        backlight_disable_breathing();
                        backlight_enable_breathing();
                        backlight_enable();
                    } break;
                    case 0: {
                        backlight_disable();
                    } break;
                    default: {
                        confinfo.backlight_mode = 0;
                        eeconfig_confinfo_update(confinfo.raw);
                    } // break;
                    case 1: {
                        if (get_backlight_level() == 0) {
                            backlight_level(2);
                        }
                        backlight_disable_breathing();
                        backlight_disable();
                        backlight_enable();
                    } break;
                }
            }
            return false;
        } break;
        case TO(_MBL): {
            if (record->event.pressed) {
                if (!keymap_is_mac_system()) {
                    set_single_persistent_default_layer(_MBL);
                    layer_move(0);
                    uint8_t data[] = {0xA7, 0x81};
                    md_send_smsg_pkt(data, 2);
                }
            }

            return false;
        } break;
        case TO(_BL): {
            if (record->event.pressed) {
                if (keymap_is_mac_system()) {
                    set_single_persistent_default_layer(_BL);
                    layer_move(0);
                    uint8_t data[] = {0xA7, 0x80};
                    md_send_smsg_pkt(data, 2);
                }
            }

            return false;
        } break;
        case HS_DKT: {
            if (!keymap_is_mac_system()) {
                if (record->event.pressed) {
                    register_code16(KC_LCTL);
                    register_code16(KC_LGUI);
                    register_code16(KC_Q);
                } else {
                    unregister_code16(KC_LCTL);
                    unregister_code16(KC_LGUI);
                    unregister_code16(KC_Q);
                }
            } else {
                if (record->event.pressed) {
                    register_code16(KC_LGUI);
                    register_code16(KC_L);
                } else {
                    unregister_code16(KC_LGUI);
                    unregister_code16(KC_L);
                }
            }
            return false;
        } break;
        case KC_APP: {
            if ((!keymap_is_mac_system() && keymap_config.no_gui)) return false;
        } break;
        case HS_TASK: {
            if (confinfo.mult_filp) {
                if (record->event.pressed) {
                    register_code16(KC_F3);
                } else {
                    unregister_code16(KC_F3);
                }
                return false;
            }
            if (record->event.pressed) {
                if (keymap_is_mac_system()) {
                    register_code16(KC_LGUI);
                    register_code16(KC_TAB);
                } else {
                    // if (wireless_get_current_devs() != DEVS_USB){
                    //     uint8_t data[2] = {0x9F, 0x02};
                    //     md_send_consumer(data);
                    // }
                    // else{
                    //     host_consumer_send(0x029F);
                    // }
                    register_code16(KC_LCTL);
                    register_code16(KC_UP);
                }

            } else {
                if (keymap_is_mac_system()) {
                    unregister_code16(KC_LGUI);
                    unregister_code16(KC_TAB);
                } else {
                    // if (wireless_get_current_devs() != DEVS_USB){
                    //     uint8_t data[2] = {0x00, 0x00};
                    //     md_send_consumer(data);
                    // }else{
                    //     host_consumer_send(0x0000);
                    // }
                    unregister_code16(KC_LCTL);
                    unregister_code16(KC_UP);
                }
            }
            return false;
        } break;
        case HS_SIRI: {
            if (keymap_is_mac_system()) {
                if (record->event.pressed) {
                    register_code(KC_LCMD);
                    register_code(KC_C);
                    wait_ms(20);
                } else {
                    unregister_code(KC_LCMD);
                    unregister_code(KC_C);
                }
            } else {
                if (wireless_get_devs().now != DEVS_USB) {
                    uint8_t data[2] = {0xCF, 0x00};
                    md_send_consumer(data);
                } else {
                    host_consumer_send(0x00CF);
                }
            }
            return false;
        } break;
        case KC_PSCR: { //
            if (!keymap_is_mac_system()) {
                if (record->event.pressed) {
                    register_code16(KC_LGUI);
                    register_code16(KC_LSFT);
                    register_code16(KC_3);
                } else {
                    unregister_code16(KC_LGUI);
                    unregister_code16(KC_LSFT);
                    unregister_code16(KC_3);
                }
                return false;
            }

        } break;
        case BT_TEST: {
            if (record->event.pressed) {
                md_send_devctrl(0x62);
            }
            return false;
        } break;
        case TH_EN: {
            return false;
        } break;
        case EE_CLR: {
            return false;
        } break;
        default:
            break;
    }

    return true;
}

void touch_update(bool up) {
    static uint32_t wakup_timer = 0x00;

    bool fn = (get_highest_layer(layer_state) == 1) || (get_highest_layer(layer_state) == 3);

    if (!confinfo.touch_enable) {
        touch_close();
        return;
    }

    if ((wireless_get_devs().now == DEVS_USB) && (USB_DRIVER.state == USB_SUSPENDED)) {
        if (!get_usb_suspend_state()) {
            wakup_timer = 0x00;
            return;
        }

        if (!wakup_timer || timer_elapsed32(wakup_timer) >= 200) {
            if (USB_DRIVER.status & 2U) {
                usbWakeupHost(&USB_DRIVER);
#if USB_SUSPEND_WAKEUP_DELAY > 0
                // Some hubs, kvm switches, and monitors do
                // weird things, with USB device state bouncing
                // around wildly on wakeup, yielding race
                // conditions that can corrupt the keyboard state.
                //
                // Pause for a while to let things settle...
                wait_ms(USB_SUSPEND_WAKEUP_DELAY);
#endif
            }
            wakup_timer = timer_read32();
            return;
        }
    }

    if (up) {
        if (fn) {
            tap_code16(KC_BRIU);
        } else {
            tap_code16(KC_VOLU);
        }
        dprintf("%d touch up\r\n", timer_read());
    } else {
        if (fn) {
            tap_code16(KC_BRID);
        } else {
            tap_code16(KC_VOLD);
        }
        dprintf("%d touch down\r\n", timer_read());
    }
}

void touch_cb(uint8_t value) {
    static uint32_t value_timer = 0x00;
    static uint8_t value_laste  = 0x00;
    static int8_t dir           = 0; // >0: up  <0: down
    static int8_t filter        = 0;

#define TOUCH_INTERVAL 50
#define TOUCH_ELAPSED  2
#define TOUCH_KEEP_NUM 4

    // new touch
    if (!value_timer || timer_elapsed32(value_timer) >= TOUCH_INTERVAL) {
        value_laste = 0x00;
        dir         = 0;
        filter      = 0;
        dprintf("------\r\n");
    }

    if (value != value_laste) {
        if (virtual_close) {
            if (lpwr_get_state() != LPWR_NORMAL) {
                virtual_close = 0;
                lpwr_set_state(LPWR_WAKEUP);
            } else {
                return;
            }
        }

        if (value > value_laste) { // down
            if (dir > 0) {
                filter--;
                if (filter <= -(TOUCH_KEEP_NUM)) {
                    dir = -(TOUCH_KEEP_NUM);
                }
            } else {
                dir--;
                filter = 0;
            }
            if (TIMER_DIFF_8(value, value_laste) >= TOUCH_ELAPSED) {
                if (dir <= -(TOUCH_KEEP_NUM)) {
                    touch_update(false);
                }
                value_laste = value;
            }
        } else if (value < value_laste) { // up
            if (dir < 0) {
                filter++;
                if (filter >= TOUCH_KEEP_NUM) {
                    dir = TOUCH_KEEP_NUM;
                }
            } else {
                dir++;
                filter = 0;
            }
            if (TIMER_DIFF_8(value_laste, value) >= TOUCH_ELAPSED) {
                if (dir >= TOUCH_KEEP_NUM) {
                    touch_update(true);
                }
                value_laste = value;
            }
        }
    }

    dprintf("tv %8ld->[%8ld] = %2d, %2d\r\n", timer_read32(), timer_elapsed32(value_timer), value, dir);

    last_matrix_activity_trigger();

    value_timer = timer_read32();
    // if (confinfo.touch_enable) touch_wakeup();
}

bool modip_update(uint8_t mode) {

    last_matrix_activity_trigger();

    return true;
}

void wlsidr_task(void) {

    if ((lpwr_get_state() == LPWR_NORMAL) && (wlsidr_blink_get_state() == WLSIDR_STATE_NONE) && !virtual_close) {
        if (wireless_get_devs().now != DEVS_USB) {
            if (*md_getp_state() != MD_STATE_CONNECTED) {
                wireless_devs_change(DEVS_USB, wireless_get_devs().now, false);
            }
        } else {
            bool wlsidr_set_blink(uint8_t led_index, wlsidr_state_t state);
            wlsidr_set_blink(WLSIDR_LED_INDEX_BT1, WLSIDR_STATE_NONE);
        }
    }

    if (virtual_close) {
        gpio_write_pin_low(LED_PIN_MODE_DET);
        gpio_write_pin_low(B8);
    }

    if (wlsidr_blink_get_state() != WLSIDR_STATE_NONE) {
        last_matrix_activity_trigger();
    }
}


void md_main_task_hook(void) {

}

void bootmagic_scan(void) {
    matrix_scan();
    wait_ms(5);
    matrix_scan();
}

void matrix_init_user(void) {

    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            if (matrix_is_on(row, col)) {

                if (row == 5 && col == 0) {
                    hs_color_test = true;
                }
            }
        }
    }
}

bool via_command_kb(uint8_t *data, uint8_t length) {
    uint8_t *command_id   = &(data[0]);
    uint8_t *command_data = &(data[1]);

    switch (*command_id) {
        case 0x0B: {
            command_data[0] = 1;
            jump_boot_timer = timer_read32();
        } break;
        default: {
            return false;
        } break;
    }

    // Return the same buffer, optionally with values changed
    // (i.e. returning state to the host, or the unhandled state).
    void replaced_hid_send(uint8_t *, uint8_t);
    replaced_hid_send(data, length);

    return true;
}
