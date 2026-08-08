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
#include "iprint.h"
#include "wlsidr.h"
#include "modip.h"
#include "lpkc.h"
// #include "i2c_master.h"
#include "rgb_record.h"
#include "rgb_matrix_blink.h"
#include "hs_rgblight.h"

#include <lib/lib8tion/lib8tion.h>

typedef union {
    uint32_t raw;
    struct {
        uint8_t flag : 1;
        uint8_t rgb_speed : 1;
        uint8_t dir : 1;
        uint8_t rgb_enable : 1;
        uint8_t hs_deb : 1;
        uint8_t hs_music_cut : 1;
        uint8_t record_channel : 2;
        
#ifdef WIRELESS_ENABLE
        wireless_devs_t devs;
#endif
    };
} confinfo_t;
confinfo_t confinfo;

_Static_assert(sizeof(confinfo_t) <= 4, "confinfo_t must be <= 4 bytes");

#define keymap_is_mac_system() ((get_highest_layer(default_layer_state) == _MBL) || (get_highest_layer(default_layer_state) == _MFL))
#define keymap_is_base_layer() ((get_highest_layer(default_layer_state) == _BL) || (get_highest_layer(default_layer_state) == _FL))
#define WIITE_B     0X20

extern void last_matrix_activity_trigger(void);
void ind_indicators_hook(uint8_t index);

uint32_t get_remote_wakeup_delay(void);
void bat_indicators_hook(uint8_t index);
bool led_blink_all_cb(uint8_t led_index);
void matrix_init_user(void);
void nkr_indicators_hook(uint8_t index);
void rgb_test(void);

bool music_cut_flag             = true;
bool rgblight_cut_flag          = false;
bool hs_mucis_calibration       = true;
uint32_t mucis_calibration_time = 0;

bool test_white_light_flag = false;
uint32_t jump_boot_timer   = 0x00;
uint8_t ee_clr_flag        = 0;
uint8_t im_test_rgb_state  = 0;
uint32_t post_init_timer   = 0x00;
static bool no_record_fg   = false;
bool inqbat_flag           = false;
bool test_rate_flag        = false; // 测试使用不公开

blink_rgb_t blink_rgbs[RGB_MATRIX_BLINK_COUNT] = {
    {.index = WLSIDR_LED_INDEX_BT1,  .interval = 250, .times = 1, .color = {.r = 0x00, .g = 0xFF, .b = 0x00},    .blink_cb = wlsidr_blink_cb },
    {.index = WLSIDR_LED_INDEX_BT2,  .interval = 250, .times = 1, .color = {.r = 0x00, .g = 0x00, .b = 0xFF},    .blink_cb = wlsidr_blink_cb },
    {.index = WLSIDR_LED_INDEX_BT3,  .interval = 250, .times = 1, .color = {.r = 0x00, .g = 0xFF, .b = 0x00},    .blink_cb = wlsidr_blink_cb },
    {.index = WLSIDR_LED_INDEX_2G4,  .interval = 250, .times = 1, .color = {.r = 0x00, .g = 0xFF, .b = 0x00},    .blink_cb = wlsidr_blink_cb },
    {.index = WLSIDR_LED_INDEX_USB,  .interval = 250, .times = 1, .color = {.r = 0x00, .g = 0xFF, .b = 0x00},    .blink_cb = wlsidr_blink_cb },
    {.index = WLSIDR_LED_INDEX_WIN,  .interval = 250, .times = 3, .color = {.r = 0xFF, .g = 0xFF, .b = 0xFF},    .blink_cb = NULL },
    {.index = WLSIDR_LED_INDEX_MAC,  .interval = 250, .times = 3, .color = {.r = 0xFF, .g = 0xFF, .b = 0xFF},    .blink_cb = NULL },

    {.index = WLSIDR_LED_INDEX_VAI,  .interval = 250, .times = 3, .color = {.r = 0xFF, .g = 0xFF, .b = 0xFF},    .blink_cb = NULL },
    {.index = WLSIDR_LED_INDEX_VAD,  .interval = 250, .times = 3, .color = {.r = 0xFF, .g = 0xFF, .b = 0xFF},    .blink_cb = NULL },
    {.index = WLSIDR_LED_INDEX_SPD,  .interval = 250, .times = 3, .color = {.r = 0xFF, .g = 0xFF, .b = 0xFF},    .blink_cb = NULL },
    {.index = WLSIDR_LED_INDEX_SPI,  .interval = 250, .times = 3, .color = {.r = 0xFF, .g = 0xFF, .b = 0xFF},    .blink_cb = NULL },
    {.index = WLSIDR_LED_INDEX_DEB,  .interval = 250, .times = 3, .color = {.r = 0xFF, .g = 0xFF, .b = 0xFF},    .blink_cb = NULL },

    {.index = WLSIDR_BLINK_INDEX_BAT,.interval = 250, .times = 1, .color = {.r = 0x00, .g = 0x00, .b = 0x00},    .blink_cb = bat_indicators_hook },
    {.index = WLSIDR_LED_INDEX_DIR,  .interval = 250, .times = 3, .color = {.r = 0xFF, .g = 0xFF, .b = 0xFF},    .blink_cb = ind_indicators_hook},
    {.index = WLSIDR_LED_INDEX_ALL,  .interval = 250, .times = 3, .color = {.r = WIITE_B, .g = WIITE_B, .b = WIITE_B},    .blink_cb = nkr_indicators_hook },
};

void eeconfig_confinfo_update(uint32_t raw) {

    eeconfig_update_kb(raw);
}

uint32_t eeconfig_confinfo_read(void) {

    return eeconfig_read_kb();
}
#include "hal.h"
#include "analog.h" // 如果QMK有提供针对WB32的analog.h，优先使用它


void eeconfig_confinfo_default(void) {
    
    confinfo.record_channel    = 0;
    confinfo.flag              = true;
    confinfo.dir               = false;
    confinfo.hs_deb            = false;
    confinfo.rgb_enable        = true;
    // confinfo.rgb_speed         = 0;
    no_record_fg               = false;
    
#ifdef WIRELESS_ENABLE
    confinfo.devs.now = DEVS_USB;
    confinfo.devs.bt  = DEVS_BT1;
    confinfo.devs.wl  = DEVS_BT1;
#endif
    // hs_rgblight_init(confinfo.rgb_speed);
    eeconfig_init_user_datablock();
    eeconfig_confinfo_update(confinfo.raw);
}

uint8_t get_debounce(void) {
    if (!confinfo.hs_deb) return 5;
    else return 1;
}

void eeconfig_confinfo_init(void) {

    confinfo.raw = eeconfig_confinfo_read();
    if (!confinfo.raw) {
        eeconfig_confinfo_default();
        eeprom_write_byte((uint8_t *)CONFINFO_RGB_MOD_ADDR, 0xFF);
        keymap_config.nkro = 1;
        eeconfig_update_keymap(keymap_config.raw);
    }
}

void nkr_indicators_hook(uint8_t index) {
    if (index == WLSIDR_LED_INDEX_ALL) {
        if ((blink_rgbs[RGB_MATRIX_BLINK_COUNT - 1].color.r == WIITE_B) && (blink_rgbs[RGB_MATRIX_BLINK_COUNT - 1].color.g == 0x00) && (blink_rgbs[RGB_MATRIX_BLINK_COUNT - 1].color.b == 0x00)) {
            rgb_matrix_blink_set_color(index, 0x00, 0x00, WIITE_B);
            rgb_matrix_blink_set_times(index, 1);
            rgb_matrix_blink_set(index);
 
        } else if ((blink_rgbs[RGB_MATRIX_BLINK_COUNT - 1].color.r == 0x00) && (blink_rgbs[RGB_MATRIX_BLINK_COUNT - 1].color.g == 0x00) && (blink_rgbs[RGB_MATRIX_BLINK_COUNT - 1].color.b == WIITE_B)) {
            rgb_matrix_blink_set_color(index, 0x00, WIITE_B, 0x00);
            rgb_matrix_blink_set_times(index, 1);
            rgb_matrix_blink_set(index);
 
        }
    }
    if (blink_rgbs[RGB_MATRIX_BLINK_COUNT - 1].color.r == WIITE_B && blink_rgbs[RGB_MATRIX_BLINK_COUNT - 1].color.g == WIITE_B && blink_rgbs[RGB_MATRIX_BLINK_COUNT - 1].color.b == WIITE_B) {
        mcu_reset();
    }
}

typedef enum {
    ind_state_none = 0,
    ind_state_exec,
    ind_state_blink3,
    ind_state_blinkon,
    ind_state_blinkoff,
} ind_status_t;

ind_status_t ind_status = ind_state_none;

void ind_indicators_hook(uint8_t index) {

    switch(ind_status) {
        case ind_state_none: {
            rgb_matrix_blink_set_color(index, host_keyboard_led_state().caps_lock, !keymap_is_mac_system() ? keymap_config.no_gui : 0 , 0x00);
            rgb_matrix_blink_set_interval_times(index, 50, 0xFF);
        } break;
        case ind_state_blink3: {
            rgb_matrix_blink_set_color(index, 0xFF, 0xFF, 0xFF);
            rgb_matrix_blink_set_interval_times(index, 200, 3);
            ind_status = ind_state_exec;
        } break;
        case ind_state_blinkon: {
            rgb_matrix_blink_set_color(index, 0xFF, 0xFF, 0xFF);
            rgb_matrix_blink_set_interval_times(index, 200, 1);
        } break;
        case ind_state_blinkoff: {
            ind_status = ind_state_none;
            ind_indicators_hook(index);
        } break;
        case ind_state_exec: {
            ind_status = ind_state_none;
            ind_indicators_hook(index);
        } break;
        default: break;
    }

    rgb_matrix_blink_set(index);
}

//设置 状态机状态 (会清空原来的状态 闪烁状态也会停止)
void ind_set_blink(ind_status_t status) {
    ind_status = status;

    rgb_matrix_blink_set_remain_time(WLSIDR_LED_INDEX_DIR, 0x00);
    ind_indicators_hook(WLSIDR_LED_INDEX_DIR);
}

void keyboard_post_init_kb(void) {

#ifdef GPIO_UART_ENABLE
    iprint_init();
    wait_ms(3000);
#endif

#ifdef CONSOLE_ENABLE
    debug_enable = true;
#endif
    ee_clr_flag = 0;

    jump_boot_timer = 0x00;

    eeconfig_confinfo_init();

    // hs_rgblight_init(confinfo.rgb_speed);

#ifdef USB_CABLE_PIN
    gpio_set_pin_input(USB_CABLE_PIN);
#endif

#ifdef BAT_FULL_PIN
    gpio_set_pin_input_high(BAT_FULL_PIN);
#endif

#ifdef LED_POWER_EN_PIN
    gpio_set_pin_output(LED_POWER_EN_PIN);
    if (confinfo.rgb_enable) gpio_write_pin_high(LED_POWER_EN_PIN);
#endif

#ifdef LED_INDICATOR_EN_PIN
    gpio_set_pin_output(LED_INDICATOR_EN_PIN);
    gpio_write_pin_high(LED_INDICATOR_EN_PIN);
#endif

    // gpio_set_pin_output(A9);
    // gpio_write_pin_high(A9);
#ifdef USB_POWER_EN_PIN
    gpio_write_pin_low(USB_POWER_EN_PIN);
    gpio_set_pin_output(USB_POWER_EN_PIN);
#endif

#ifdef WIRELESS_ENABLE
    modip_init();
    wireless_init(&confinfo.devs);
    post_init_timer = timer_read32();
#endif

#ifdef WLSIDR_BLINK_INDEX_BAT
    rgb_matrix_blink_set_interval_times(WLSIDR_BLINK_INDEX_BAT, 200, 0xFF);
    rgb_matrix_blink_set(WLSIDR_BLINK_INDEX_BAT);
#endif

    rgb_matrix_blink_set(WLSIDR_LED_INDEX_DIR);

    matrix_init_user();
    rgbrec_init(confinfo.record_channel);

    gpio_set_pin_output(LED_POWER_EN2_PIN);
    gpio_write_pin_high(LED_POWER_EN2_PIN);

    gpio_set_pin_output(MCU_SLEEP);
    gpio_set_pin_output(MCU_INT);
    gpio_set_pin_output(CPAS_PIN);
    gpio_set_pin_output(RESET_PIN);

    keyboard_post_init_user();

    // keyrecord_t record;
    // record.event.pressed = true;
    // record.event.key.row = 5;
    // record.event.key.col = 3;
    // process_record_kb(MUS_CUT, &record);
    // record.event.pressed = false;
    // process_record_kb(MUS_CUT, &record);

    // i2c_init();
}

void led_power_enable(void) {

#ifdef LED_POWER_EN_PIN
    gpio_write_pin_high(LED_POWER_EN_PIN);
#endif

#ifdef LED_POWER_EN2_PIN
    gpio_write_pin_high(LED_POWER_EN2_PIN);
#endif
}

void led_power_disable(void) {

#ifdef LED_POWER_EN_PIN
    gpio_write_pin_low(LED_POWER_EN_PIN);
#endif

#ifdef LED_POWER_EN2_PIN
    gpio_write_pin_low(LED_POWER_EN2_PIN);
#endif

}

bool led_power_is_enabled(void) {
    bool state = true;

#ifdef LED_POWER_EN_PIN
    state = gpio_read_pin(LED_POWER_EN_PIN);
#endif

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

    
}

void suspend_power_down_kb(void) {

#ifdef BAT_FULL_PIN
    gpio_write_pin_low(BAT_FULL_PIN);
    gpio_set_pin_output(BAT_FULL_PIN);
#endif

    led_power_disable();
    gpio_write_pin_low(LED_INDICATOR_EN_PIN);
    gpio_write_pin_high(MCU_SLEEP);

#ifdef WIRELESS_ENABLE
    if (wireless_get_devs().now != DEVS_USB) {
        usb_power_disconnect();
    }

    if (wireless_get_devs().now == DEVS_USB) {
#    ifdef USB_CABLE_PIN
        if ((lpwr_get_mode() == LPWR_MODE_SHUTDOWN) || !usb_cable_is_connected()) {
            /* USB模式处于关机状态时，或者未插入USB时停止USB外设 */
            usb_transport_enable(false);
            /* 未插入USB时进入关机 */
            lpwr_set_manual_shutdown(true);
        }
#    endif
    }
#endif

    suspend_power_down_user();
}

void suspend_wakeup_init_kb(void) {

    if (confinfo.rgb_enable) led_power_enable();
    gpio_write_pin_high(LED_INDICATOR_EN_PIN);
    lpkc_init();

    if (!ee_clr_flag) gpio_write_pin_low(MCU_SLEEP);

   

#ifdef WIRELESS_ENABLE
    /* 非正常USB模式休眠时重启USB外设 */
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
uint32_t moude_time = 0,brightness_time = 0;
// loop housekeeping_task_kb被wireless功能使用, 此处只能用user级函数
void housekeeping_task_user(void) {

#ifndef MD_THREAD_MODE
    // USB模式下模拟鼠标数据测试回报率，内部使用，不公开
    void md_main_task_hook(void);
    md_main_task_hook();
#endif

    if (jump_boot_timer && (timer_elapsed32(jump_boot_timer) >= 500)) {
        reset_keyboard();
    }

    if (moude_time && timer_elapsed32(moude_time) >= HS_MUSIC_DEFAULT_TIMER) {
        moude_time = 0;
        gpio_write_pin_low(MCU_INT);
    }

    if (brightness_time && timer_elapsed32(brightness_time) >= HS_MUSIC_CALIBRATION_TIMER) {
        brightness_time = 0;
        gpio_write_pin_low(MCU_INT);
    }


    // if (pulse_active && timer_elapsed32(pulse_start_time) >= HS_MUSIC_DEFAULT_TIMER) {
	//         gpio_write_pin_low(HS_MUSIC_MODE_PIN);  // <20>ָ<EFBFBD>Ĭ<EFBFBD>ϵ<EFBFBD>ƽ״̬
    //     pulse_active = false;
    //     pulse_start_time = timer_read32();
    // }
    
	//     if (hs_mucis_calibration && timer_elapsed32(mucis_calibration_time) >= HS_MUSIC_CALIBRATION_TIMER) { // У׼<D0A3><D7BC><EFBFBD><EFBFBD><EFBFBD>ɶ<EFBFBD>оƬ
	//         gpio_write_pin_low(HS_MUSIC_MODE_PIN);  // <20>ָ<EFBFBD>Ĭ<EFBFBD>ϵ<EFBFBD>ƽ״̬
    //     hs_mucis_calibration = false;
    //     mucis_calibration_time = timer_read32();
    // }
}

bool rgb_matrix_blink_user(blink_rgb_t *blink_rgb) {
    switch (blink_rgb->index) {
        case WLSIDR_LED_INDEX_USB:{
            if (post_init_timer && timer_elapsed32(post_init_timer) < 100) return false;
        }break;
        case WLSIDR_BLINK_INDEX_BAT: {
            // if (!charging && (*md_getp_bat() <= BATTERY_CAPACITY_LOW))  rgb_matrix_set_color_all(0x00, 0x00, 0x00);
            if (blink_rgb->color.r == 0 && blink_rgb->color.g == 0 && blink_rgb->color.b == 0)  return false;
            
            if (blink_rgb->flip) {
                rgb_matrix_set_color(10, blink_rgb->color.r, blink_rgb->color.g, blink_rgb->color.b);
            } else {
                rgb_matrix_set_color(10, 0, 0, 0);
            }
            return false;
        } break;
        case WLSIDR_LED_INDEX_DIR: {
            // if (blink_rgb->flip) {
            //     if(ind_status == ind_state_blink3 || ind_status == ind_state_blinkon || ind_status == ind_state_exec){
            //         rgb_matrix_set_color(0, WIITE_B, WIITE_B, WIITE_B);
            //         rgb_matrix_set_color(1, WIITE_B, WIITE_B, WIITE_B);
            //         rgb_matrix_set_color(2, WIITE_B, WIITE_B, WIITE_B);
            //     }
            // } else {
            //     if(ind_status == ind_state_blink3 || ind_status == ind_state_blinkon || ind_status == ind_state_exec){
            //         rgb_matrix_set_color(0, 0x00, 0x00, 0x00);
            //         rgb_matrix_set_color(1, 0x00, 0x00, 0x00);
            //         rgb_matrix_set_color(2, 0x00, 0x00, 0x00);
            //     }
            // }
            return false;
        } break;
        default :break;
    }
    return true;
}

bool lpwr_is_allow_timeout_hook(void) {

    if (wireless_get_devs().now == DEVS_USB) {
        return false;
    }

    return true;
}

bool lpwr_is_allow_presleep_hook(void) {

    
    return true;
}

void lpwr_post_stop_hook(void) {

    if ((lpwr_get_mode() == LPWR_MODE_SHUTDOWN) && (lpwr_get_state() == LPWR_WAKEUP)) {
        mcu_reset();
        while (1) {}
    }
}

void lpwr_wakeup_hook(void) {

    hs_now_mode = 0;

    if (!keymap_is_base_layer()) {
        uint8_t data[] = {0xA7, 0x80};
        md_send_smsg_pkt(data, 2);
    } else {
        uint8_t data[] = {0xA7, 0x81};
        md_send_smsg_pkt(data, 2);
    }
}

// lpwr中断回调钩子函数
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
        default:
            break;
    }
}

// lpwr exti 初始化钩子函数
void lpwr_exti_init_hook(lpwr_mode_t mode) {

#ifdef USB_CABLE_PIN
    // if ((mode == LPWR_MODE_SHUTDOWN) || (mode == LPWR_MODE_LOWBAT))
    {
        palEnableLineEvent(USB_CABLE_PIN, PAL_EVENT_MODE_RISING_EDGE);
    }
#endif

// #ifdef BAT_FULL_PIN
//     palEnableLineEvent(BAT_FULL_PIN, PAL_EVENT_MODE_FALLING_EDGE);
// #endif

#ifdef MODIP_SWITCH_PIN
    palEnableLineEvent(MODIP_SWITCH_PIN, PAL_EVENT_MODE_RISING_EDGE);
#endif

#ifdef MODIP_WL_PIN
    palEnableLineEvent(MODIP_WL_PIN, PAL_EVENT_MODE_RISING_EDGE);
#endif

#ifdef MODIP_USB_PIN
    palEnableLineEvent(MODIP_USB_PIN, PAL_EVENT_MODE_RISING_EDGE);
#endif

#ifdef ENCODER_ENABLE
    static pin_t encoder_pins[NUM_ENCODERS] = ENCODER_A_PINS;
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
        if (modip_scan(true) && (wireless_get_devs().now != DEVS_USB)) {
            wireless_devs_change(!wireless_get_devs().now, wireless_get_devs().now, false);
        }

        if (!keymap_is_base_layer()){
            uint8_t data[] = {0xA7,0x80};
            md_send_smsg_pkt(data,2);
        }
        else{
            uint8_t data[] = {0xA7,0x81};
            md_send_smsg_pkt(data,2);
        }
    }

    if (!post_init_timer) {
        if (!ee_clr_flag) modip_scan(false);
        wlsidr_task();
    }
}

void wireless_devs_change_user(uint8_t old_devs, uint8_t new_devs, bool reset) {

    eeconfig_confinfo_update(confinfo.raw);

    if (wireless_get_devs().now == DEVS_2G4) {
        md_send_devctrl(0xFF);
    } else if (wireless_get_devs().now != DEVS_USB) {
        md_send_devctrl(0x80 + RF_DEEPSLEEP_TIME);
    }
}

bool process_record_wls(uint16_t keycode, keyrecord_t *record) {

    switch (keycode) {
        case HS_PRF1: {
            if (record->event.pressed && (wireless_get_devs().now != DEVS_BT1)) {
                modip_mode_t mode = modip_get_mode();
                if ((mode == MODIP_MODE_BT) || (mode == MODIP_MODE_WIRELESS) || (mode == MODIP_MODE_NONE))
                    wireless_devs_change(wireless_get_devs().now, DEVS_BT1, false);
            }
        } break;
        case HS_PRF2: {
            if (record->event.pressed && (wireless_get_devs().now != DEVS_BT2)) {
                modip_mode_t mode = modip_get_mode();
                if ((mode == MODIP_MODE_BT) || (mode == MODIP_MODE_WIRELESS) || (mode == MODIP_MODE_NONE))
                    wireless_devs_change(wireless_get_devs().now, DEVS_BT2, false);
            }
        } break;
        case HS_PRF3: {
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
        case HS_2P4G: {
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
        case HS_BATQ: {
            inqbat_flag    = record->event.pressed;
            // test_rate_flag = record->event.pressed; // 测试使用，不公开
        } break;
        default: {
            return true;
        } break;
    }

    return false;
}

uint8_t bat_blink      = 0; // 0: off 1: full 2: charging 3: low
uint16_t bat_breathing = 0;
bool bat_indicators_status = false;
bool hs_frequency_test = false, hs_color_test = false;

void bat_indicators_hook(uint8_t index) {
    static uint32_t bat_timer = 0,bat_bull_timer = 0;
    uint8_t bat_level         = *md_getp_bat();

    if (hs_frequency_test && hs_color_test) {
       
        return;
    }

#ifdef USB_CABLE_PIN
    charging = usb_cable_is_connected();
#endif

#ifdef BAT_FULL_PIN
    bat_full_flag = gpio_read_pin(BAT_FULL_PIN);
#else
    bat_full_flag = bat_level >= 100;
#endif

    // charging  = false;
    // bat_full_flag = 1;
    // bat_level = BATTERY_CAPACITY_LOW;

    if (!charging) bat_bull_timer = 0;
    if (charging){
        if (bat_full_flag) { // 满电
        // 增加2秒延迟判断
            if (bat_bull_timer == 0) {
                bat_bull_timer = timer_read32(); // 记录首次检测到满电的时间
            }
            
            // 检查是否已经过了2秒
            if (timer_elapsed32(bat_bull_timer) >= 2000) { // 2秒后进入满电状态
                                                          //充满
                rgb_matrix_blink_set_color(index, 0x00, 0x80, 0x00);
                rgb_matrix_blink_set_interval_times(index, 300, 0xFF); // 灭灯
                if (hs_now_mode != MD_SND_CMD_DEVCTRL_CHARGING_DONE) {
                    hs_now_mode = MD_SND_CMD_DEVCTRL_CHARGING_DONE;
                    md_send_devctrl(hs_now_mode);
                }
            } else {
                // 在2秒延迟期间，保持充电状态显示
                rgb_matrix_blink_set_color(index, 0x80, 0x00, 0x00);
                rgb_matrix_blink_set_interval_times(index, 300, 0xFF); // 红灯充电
            }
        } else { // 正在充电（未满电）
            bat_bull_timer = 0; // 重置计时器
            rgb_matrix_blink_set_color(index, 0x80, 0x00, 0x00);
            rgb_matrix_blink_set_interval_times(index, 300, 0xFF); // 红灯充电
            if (hs_now_mode != MD_SND_CMD_DEVCTRL_CHARGING) {
                hs_now_mode = MD_SND_CMD_DEVCTRL_CHARGING;
                md_send_devctrl(hs_now_mode);
            }
        }
        bat_timer = 0;
    } else if ((bat_level <= BATTERY_CAPACITY_LOW) && (bat_level > BATTERY_CAPACITY_STOP)) { // 低电
     
        bat_timer = 0;
        rgb_matrix_blink_set_color(index, 0x80, 0x00, 0x00);
        rgb_matrix_blink_set_interval_times(index, 250, 1); // 红灯闪烁
        if (hs_now_mode != MD_SND_CMD_DEVCTRL_CHARGING_STOP){
            hs_now_mode  = MD_SND_CMD_DEVCTRL_CHARGING_STOP;
            md_send_devctrl(hs_now_mode);
        }
    } else if (bat_level <= BATTERY_CAPACITY_STOP) { // 低电
        rgb_matrix_blink_set_color(index, 0x80, 0x00, 0x00);
        rgb_matrix_blink_set_interval_times(index, 250, 1); // 红灯闪烁

        if (!bat_timer) bat_timer = timer_read32();
        if (timer_elapsed32(bat_timer) > 10000) {
            bat_timer = 0;
            lpwr_set_manual_lowbat(true);
        }

        if (hs_now_mode != MD_SND_CMD_DEVCTRL_CHARGING_STOP){
            hs_now_mode  = MD_SND_CMD_DEVCTRL_CHARGING_STOP;
            md_send_devctrl(hs_now_mode);
        }
    } else { // 正常状态
        bat_timer     = 0;
        rgb_matrix_blink_set_color(index, 0x00, 0x00, 0x00);
        if (hs_now_mode != MD_SND_CMD_DEVCTRL_CHARGING_STOP){
            hs_now_mode  = MD_SND_CMD_DEVCTRL_CHARGING_STOP;
            md_send_devctrl(hs_now_mode);
        }
    }
    if (bat_indicators_status) rgb_matrix_blink_set_color(index, 0x00, 0x00, 0x00);
    rgb_matrix_blink_set(index);
}

void hs_reset_settings(void) {

    wait_ms(15);
    gpio_write_pin_high(MCU_SLEEP);
    ee_clr_flag = 1;
    eeconfig_disable();
    eeconfig_init();
    eeconfig_update_rgb_matrix_default();
   
#ifdef RGBLIGHT_ENABLE
    extern void rgblight_init(void);
    is_rgblight_initialized = false;
    rgblight_init();
    eeconfig_update_rgblight_default();
    rgblight_enable();
#endif

    
#if defined(WIRELESS_ENABLE)
    wireless_devs_change(wireless_get_devs().now, DEVS_USB, false);
#endif
    
    rgb_matrix_blink_set_interval_times(WLSIDR_LED_INDEX_ALL, 250, 3); 
    rgb_matrix_blink_set_color(WLSIDR_LED_INDEX_ALL,WIITE_B,WIITE_B,WIITE_B);
    rgb_matrix_blink_set(WLSIDR_LED_INDEX_ALL);
}

bool process_lpkc_keycode(uint16_t keycode, bool pressed) {

    switch (keycode) {
#ifdef WIRELESS_ENABLE
        case HS_PRF1: {
            if (pressed && (wireless_get_devs().now == DEVS_BT1)) {
                wireless_devs_change(wireless_get_devs().now, DEVS_BT1, true);
            }
        } break;
        case HS_PRF2: {
            if (pressed && (wireless_get_devs().now == DEVS_BT2)) {
                wireless_devs_change(wireless_get_devs().now, DEVS_BT2, true);
            }
        } break;
        case HS_PRF3: {
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
        case HS_2P4G: {
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
        case EE_CLR: {
            if (pressed) {
                // ee_clr_flag = 1;

                // gpio_write_pin_low(MCU_INT);
                // wait_ms(5);
                // uint8_t data = 1;
                // i2c_write_register(0xA0,0x80,&data,0x01,10);//地址 数据 长度 超时时间
                // gpio_write_pin_high(MCU_INT);
                gpio_write_pin_high(RESET_PIN);
                wait_ms(100);
                gpio_write_pin_low(RESET_PIN);
                hs_reset_settings();
        
                // led_blink_register(LED_BLINK_INDEX_ALL, 0xFF, 2, 500, false, led_blink_all_cb);
            }
        } break;
        case HS_DEB:{
            if (pressed) {
                confinfo.hs_deb = !confinfo.hs_deb;
                eeconfig_confinfo_update(confinfo.raw);

                if (!confinfo.hs_deb)  rgb_matrix_blink_set_color(WLSIDR_LED_INDEX_DEB, 0xFF, 0xFF, 0xFF);
                else rgb_matrix_blink_set_color(WLSIDR_LED_INDEX_DEB, 0xFF, 0x00, 0x00);
                rgb_matrix_blink_set(WLSIDR_LED_INDEX_DEB);
            }
            return false;
        }break;
        case TO(_MBL): {
            if (pressed) {
                rgb_matrix_blink_set_remain_time(WLSIDR_LED_INDEX_WIN, 0); // 添加这句可以让进入这个条件时先停止win系统的指示灯闪烁
                if (!keymap_is_mac_system()) {
                    set_single_persistent_default_layer(_MBL);
                    rgb_matrix_blink_set_color(WLSIDR_LED_INDEX_MAC, 0xFF, 0xFF, 0xFF);
                    layer_move(0);
                    uint8_t data[] = {0xA7, 0x80};
                    md_send_smsg_pkt(data, 2);
                } else {
                    set_single_persistent_default_layer(_BL);
                    rgb_matrix_blink_set_color(WLSIDR_LED_INDEX_MAC, 0x00, 0x00, 0xFF);
                    layer_move(0);
                    uint8_t data[] = {0xA7, 0x81};
                    md_send_smsg_pkt(data, 2);
                }
                rgb_matrix_blink_set_interval_times(WLSIDR_LED_INDEX_MAC, 250, 3);
                rgb_matrix_blink_set(WLSIDR_LED_INDEX_MAC);
            }

            return false;
        } break;
        case TO(_BL): {
            if (pressed) {
                rgb_matrix_blink_set_remain_time(WLSIDR_LED_INDEX_MAC, 0); // 添加这句可以让进入这个条件时先停止win系统的指示灯闪烁
                rgb_matrix_blink_set_interval_times(WLSIDR_LED_INDEX_WIN, 250, 3);
                rgb_matrix_blink_set(WLSIDR_LED_INDEX_WIN);
                if (keymap_is_mac_system()) {
                    set_single_persistent_default_layer(_BL);
                    layer_move(0);
                    uint8_t data[] = {0xA7, 0x81};
                    md_send_smsg_pkt(data, 2);
                }
            }

            return false;
        } break;
        default:
            break;
    }

    return true;
}
bool dir = false,gui_press_fg = false;
bool process_record_user(uint16_t keycode, keyrecord_t *record) {

    if (test_white_light_flag && record->event.pressed) {
        test_white_light_flag = false;
        rgb_matrix_set_color_all(0x00, 0x00, 0x00);
    }

    switch (keycode) {
        case MO(_FL):
        case MO(_MFL): {
            if (!record->event.pressed ) {
                if (gui_press_fg) {
                    unregister_code(KC_LGUI);
                    unregister_code(KC_TAB);
                    gui_press_fg = false;
                }
                if (rgbrec_is_started()) {
                    if (no_record_fg == true) {
                        no_record_fg = false;
                        rgbrec_register_record(keycode, record);
                    }
                    no_record_fg = true;
                }
            }
            break;
        }
        case RP_END:
        case RP_P0:
        case RP_P1:
        case RP_P2:
        case UG_NEXT:
        case UG_PREV:
            break;
        default: {
            if (rgbrec_is_started()) {
                if (!IS_QK_MOMENTARY(keycode) && record->event.pressed) {
                    wlsidr_blink_set_timer(timer_read32());
                    rgbrec_register_record(keycode, record);
                    return false;
                }
            }
        } break;
    }

    if (rgbrec_is_started() && (!(keycode == RP_P0 || keycode == RP_P1 || keycode == RP_P2 || keycode == RP_END || keycode == UG_PREV || keycode == UG_NEXT || keycode == MO(_FL) || keycode == MO(_MFL)))) {

        return false;
    }

    return true;
}
bool keyboard_lock = false,bat_lock = false;
bool process_record_kb(uint16_t keycode, keyrecord_t *record) {

    if (hs_frequency_test && hs_color_test) {
        if (!(keycode >= KC_1 && keycode <= KC_3))

            return false;
    }
    if (record->event.pressed && keycode != HS_TASK) {
        if (gui_press_fg) {
            unregister_code(KC_LGUI);
            unregister_code(KC_TAB);
            gui_press_fg = false;
        }
    }
    
    switch (keycode) {
        case EE_CLR: {
            process_lpkc_record(keycode, 3000, record->event.pressed);
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
        case QK_TOGGLE_LAYER ... QK_TOGGLE_LAYER_MAX: {
            if (record->event.pressed) {
                layer_invert(QK_TOGGLE_LAYER_GET_LAYER(keycode));
            }
            return false;
        }break;
        case MUS_CUT:{ //<2F>Ѳ<EFBFBD><D1B2><EFBFBD><EFBFBD>л<EFBFBD><D0BB><EFBFBD>ʰ<EFBFBD><CAB0><EFBFBD>Ƶ<EFBFBD><C6B5><EFBFBD>
            if (record->event.pressed){
                // gpio_write_pin_low(MCU_INT);
                // wait_ms(5);
                // uint8_t data = 0;
                // if (host_keyboard_led_state().caps_lock) data |= 0x10;
                // data = data | 0x02;
                // i2c_write_register(0xA0,0x80,&data,0x01,10);//地址 数据 长度 超时时间
                gpio_write_pin_high(MCU_INT);
                moude_time = timer_read32();
            }
            return false;
        }break;
        case MUS_STA: { 
            if (record->event.pressed){
                // gpio_write_pin_low(MCU_INT);
                // wait_ms(5);
                // uint8_t data = 0;
                // if (host_keyboard_led_state().caps_lock) data |= 0x10;
                // data = data | 0x04;
                // i2c_write_register(0xA0,0x80,&data,0x01,10);//地址 数据 长度 超时时间
                // gpio_write_pin_high(MCU_INT);
                gpio_write_pin_high(MCU_INT);
                brightness_time = timer_read32();
            }
            return false;
        } break;
        case HS_TASK:{
            if (record->event.pressed) {
                register_code16(KC_LGUI);
                register_code16(KC_TAB);
                gui_press_fg = true;
            } else {
                unregister_code16(KC_TAB);
            }
            return false;
        }break;
        case KC_MCTL:{
            if (record->event.pressed) {
                register_code16(KC_LCTL);
                register_code16(KC_UP);
            } else {
                unregister_code16(KC_LCTL);
                unregister_code16(KC_UP);
            }
            return false;
        }break;
        case NK_TOGG:{
            if (record->event.pressed) {
                // rgb_matrix_blink_set_interval_times(WLSIDR_LED_INDEX_ALL, 250, 1); 
                // rgb_matrix_blink_set_color(WLSIDR_LED_INDEX_ALL,WIITE_B, 0, 0);
                // rgb_matrix_blink_set(WLSIDR_LED_INDEX_ALL);
            }
        }break;
        case RL_MOD: {
            if (rgbrec_is_started()) {

                return false;
            }
            if (record->event.pressed) {
                hs_rgblight_increase();
            }

            return false;
        } break;
        case QK_BOOT: {
            if (record->event.pressed) {
                dprintf("into boot!!!\r\n");
                eeconfig_disable();
                bootloader_jump();
            }
        } break;
        case KC_1: { //
            if (hs_frequency_test && hs_color_test) {
                if (record->event.pressed) {
                    md_rf_send_stop();
                    md_send_devctrl(0x60);
                    md_rf_send_carrier(0, 5, 0x01);
                }
                return false;
            }
        } break;

        case KC_2: { //
            if (hs_frequency_test && hs_color_test) {
                if (record->event.pressed) {
                    md_rf_send_stop();
                    md_send_devctrl(0x60);
                    md_rf_send_carrier(19, 5, 0x01);
                }
                return false;
            }
        } break;

        case KC_3: { //
            if (hs_frequency_test && hs_color_test) {
                if (record->event.pressed) {
                    md_rf_send_stop();
                    md_send_devctrl(0x60);
                    md_rf_send_carrier(39, 5, 0x01);
                }
                return false;
            }
        } break;
        case HS_DIR: {
            if (record->event.pressed) {
                // confinfo.dir = !confinfo.dir;
                // eeconfig_confinfo_update(confinfo.raw);
                dir = !dir;
            }
        }break;
        case KC_W:{
            if (dir){
                if (record->event.pressed){
                    register_code16(KC_UP);
                } else {
                    unregister_code16(KC_UP);
                }
                return false;
            }
        }break;
        case KC_S:{
            if (dir){
                if (record->event.pressed){
                    register_code16(KC_DOWN);
                } else {
                    unregister_code16(KC_DOWN);
                }
                return false;
            }
        }break;
        case KC_A:{
            if (dir){
                if (record->event.pressed){
                    register_code16(KC_LEFT);
                } else {
                    unregister_code16(KC_LEFT);
                }
                return false;
            }
        }break;
        case KC_D:{
            if (dir){
                if (record->event.pressed){
                    register_code16(KC_RGHT);
                } else {
                    unregister_code16(KC_RGHT);
                }
                return false;
            }
        }break;
        case KC_UP:{
            if (dir){
                if (record->event.pressed){
                    register_code16(KC_W);
                } else {
                    unregister_code16(KC_W);
                }
                return false;
            }
        }break;
        case KC_DOWN:{
            if (dir){
                if (record->event.pressed){
                    register_code16(KC_S);
                } else {
                    unregister_code16(KC_S);
                }
                return false;
            }
        }break;
        case KC_LEFT:{
            if (dir){
                if (record->event.pressed){
                    register_code16(KC_A);
                } else {
                    unregister_code16(KC_A);
                }
                return false;
            }
        }break;
        case KC_RGHT:{
            if (dir){
                if (record->event.pressed){
                    register_code16(KC_D);
                } else {
                    unregister_code16(KC_D);
                }
                return false;
            }
        }break;
        case HS_DEB:
        case TO(_MBL):
        case TO(_BL):{
            return false;
        }break;
        // case TO(_MBL): {
        //     if (record->event.pressed) {
        //         rgb_matrix_blink_set_remain_time(WLSIDR_LED_INDEX_WIN, 0); // 添加这句可以让进入这个条件时先停止win系统的指示灯闪烁
        //         rgb_matrix_blink_set_interval_times(WLSIDR_LED_INDEX_MAC, 250, 3);
        //         rgb_matrix_blink_set(WLSIDR_LED_INDEX_MAC);
        //         if (!keymap_is_mac_system()) {
        //             set_single_persistent_default_layer(_MBL);
        //             layer_move(0);
        //             uint8_t data[] = {0xA7, 0x80};
        //             md_send_smsg_pkt(data, 2);
        //         }
        //     }

        //     return false;
        // } break;
        // case TO(_BL): {
        //     if (record->event.pressed) {
        //         rgb_matrix_blink_set_remain_time(WLSIDR_LED_INDEX_MAC, 0); // 添加这句可以让进入这个条件时先停止win系统的指示灯闪烁
        //         rgb_matrix_blink_set_interval_times(WLSIDR_LED_INDEX_WIN, 250, 3);
        //         rgb_matrix_blink_set(WLSIDR_LED_INDEX_WIN);
        //         if (keymap_is_mac_system()) {
        //             set_single_persistent_default_layer(_BL);
        //             layer_move(0);
        //             uint8_t data[] = {0xA7, 0x81};
        //             md_send_smsg_pkt(data, 2);
        //         }
        //     }

        //     return false;
        // } break;
        case KC_LCMD: {
            if (keymap_is_mac_system()) {
                if (keymap_config.no_gui && !rgbrec_is_started()) {
                    if (record->event.pressed) {
                        register_code16(KC_LCMD);
                    } else {
                        unregister_code16(KC_LCMD);
                    }
                }
            }

            return true;
        } break;
        case KC_RCMD: {
            if (keymap_is_mac_system()) {
                if (keymap_config.no_gui && !rgbrec_is_started()) {
                    if (record->event.pressed) {
                        register_code16(KC_RCMD);
                    } else {
                        unregister_code16(KC_RCMD);
                    }
                }
            }

            return true;
        } break;
        case HS_DKT: {
            if (record->event.pressed){
                bat_indicators_status = !bat_indicators_status;
            }
            return false;
        } break;
        case KC_APP: {
            if ((!keymap_is_mac_system() && keymap_config.no_gui)) return false;
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
        case RP_END: {
            if (record->event.pressed) {
                
                if (rgb_matrix_get_val() == 0) {
                    return false;
                }
                if (rgb_matrix_get_mode() != RGB_MATRIX_CUSTOM_RGBR_PLAY) {
                    rgb_matrix_mode(RGB_MATRIX_CUSTOM_RGBR_PLAY);
                    confinfo.record_channel = 0;
                    rgbrec_read_current_channel(confinfo.record_channel);
                    rgbrec_end(confinfo.record_channel);
                    eeconfig_confinfo_update(confinfo.raw);
                    rgbrec_show(confinfo.record_channel);
                    return false;
                }
                if (!rgbrec_is_started()) {
                    rgbrec_start(0);
                    rgbrec_set_close_all(HSV_BLACK);
                    ind_set_blink(ind_state_blinkon);
                    no_record_fg         = false;
                } else {
                    rgbrec_end(0);
                    ind_set_blink(ind_state_none);
                    no_record_fg         = false;
                }
            }
            return false;
        } break;
        // case RP_P0: {
        //     if (record->event.pressed) {
                
        //         if (rgb_matrix_get_val() == 0) {
        //             return false;
        //         }
        //         // if (rgb_matrix_get_mode() != RGB_MATRIX_CUSTOM_RGBR_PLAY) {
        //             rgb_matrix_mode(RGB_MATRIX_CUSTOM_RGBR_PLAY);
        //             confinfo.record_channel = 0;
        //             rgbrec_read_current_channel(confinfo.record_channel);
        //             rgbrec_end(confinfo.record_channel);
        //             eeconfig_confinfo_update(confinfo.raw);
        //             rgbrec_show(confinfo.record_channel);
        //             return false;
        //         // }
        //     }
        //     return false;
        // } break;
        // case RP_P1: {
        //     if (record->event.pressed) {
                
        //         if (rgb_matrix_get_val() == 0) {
        //             return false;
        //         }
        //         // if (rgb_matrix_get_mode() != RGB_MATRIX_CUSTOM_RGBR_PLAY) {
        //             rgb_matrix_mode(RGB_MATRIX_CUSTOM_RGBR_PLAY);
        //             confinfo.record_channel = 1;
        //             rgbrec_read_current_channel(confinfo.record_channel);
        //             rgbrec_end(confinfo.record_channel);
        //             eeconfig_confinfo_update(confinfo.raw);
        //             rgbrec_show(confinfo.record_channel);
        //             return false;
        //         // }
        //     }
        //     return false;
        // } break;
        // case RP_P2: {
        //     if (record->event.pressed) {
                
        //         if (rgb_matrix_get_val() == 0) {
        //             return false;
        //         }
        //         // if (rgb_matrix_get_mode() != RGB_MATRIX_CUSTOM_RGBR_PLAY) {
        //             rgb_matrix_mode(RGB_MATRIX_CUSTOM_RGBR_PLAY);
        //             confinfo.record_channel = 2;
        //             rgbrec_read_current_channel(confinfo.record_channel);
        //             rgbrec_end(confinfo.record_channel);
        //             eeconfig_confinfo_update(confinfo.raw);
        //             rgbrec_show(confinfo.record_channel);
        //             return false;
        //         // }
        //     }
        //     return false;
        // } break;
        // case RP_P3: {
        //     if (record->event.pressed) {
                
        //         if (rgb_matrix_get_val() == 0) {
        //             return false;
        //         }
        //         // if (rgb_matrix_get_mode() != RGB_MATRIX_CUSTOM_RGBR_PLAY) {
        //             rgb_matrix_mode(RGB_MATRIX_CUSTOM_RGBR_PLAY);
        //             confinfo.record_channel = 3;
        //             rgbrec_read_current_channel(confinfo.record_channel);
        //             rgbrec_end(confinfo.record_channel);
        //             eeconfig_confinfo_update(confinfo.raw);
        //             rgbrec_show(confinfo.record_channel);
        //             return false;
        //         // }
        //     }
        //     return false;
        // } break;
        // case RP_P4: {
        //     if (record->event.pressed) {
                
        //         if (rgb_matrix_get_val() == 0) {
        //             return false;
        //         }
        //         // if (rgb_matrix_get_mode() != RGB_MATRIX_CUSTOM_RGBR_PLAY) {
        //             rgb_matrix_mode(RGB_MATRIX_CUSTOM_RGBR_PLAY);
        //             confinfo.record_channel = 4;
        //             rgbrec_read_current_channel(confinfo.record_channel);
        //             rgbrec_end(confinfo.record_channel);
        //             eeconfig_confinfo_update(confinfo.raw);
        //             rgbrec_show(confinfo.record_channel);
        //             return false;
        //         // }
        //     }
        //     return false;
        // } break;
        case UG_HUEU: {
            if (rgbrec_is_started()) return false;
            if (record->event.pressed) {
                record_color_hsv(true);
                return false;
            }
        }break;

        case UG_SATU: {
            if (rgbrec_is_started()) return false;
            if (record->event.pressed) {
                // record_color_hsv(true);
                record_color_sat(true);
                return false;
            }
        }break;
        case UG_SATD: {
            if (rgbrec_is_started()) return false;
            if (record->event.pressed) {
                // record_color_hsv(true);
                record_color_sat(false);
                return false;
            }
        }break;
        // case RM_HUED: {
        //     if (rgbrec_is_started()) return false;
        //     if (record->event.pressed) {
        //         record_color_hsv(false);
        //         return false;
        //     }
        // }break;
        case UG_SPDD: {
            if (rgbrec_is_started()) return false;
            if (record->event.pressed){
                if (rgb_matrix_get_speed() <= RGB_MATRIX_SPD_STEP) {
                    rgb_matrix_set_speed(0);
                    // ind_set_blink(ind_state_blink3);
                    rgb_matrix_blink_set_remain_time(WLSIDR_LED_INDEX_SPI, 0); // 添加这句可以让进入这个条件时先停止win系统的指示灯闪烁
                    rgb_matrix_blink_set_interval_times(WLSIDR_LED_INDEX_SPD, 250, 3);
                    rgb_matrix_blink_set(WLSIDR_LED_INDEX_SPD);
                    return false;
                }
            }
        }break;
        case UG_SPDU: {
            if (rgbrec_is_started()) return false;
            if (record->event.pressed){
                if (rgb_matrix_get_speed() >= RGB_MATRIX_SPD_STEP * 4) {
                    rgb_matrix_set_speed(RGB_MATRIX_SPD_STEP * 5);
                    // ind_set_blink(ind_state_blink3);
                    rgb_matrix_blink_set_remain_time(WLSIDR_LED_INDEX_SPD, 0); // 添加这句可以让进入这个条件时先停止win系统的指示灯闪烁
                    rgb_matrix_blink_set_interval_times(WLSIDR_LED_INDEX_SPI, 250, 3);
                    rgb_matrix_blink_set(WLSIDR_LED_INDEX_SPI);
                    return false;
                }
            }
        }break;
        case UG_VALD: {
            if (rgbrec_is_started()) return false;
            if (record->event.pressed){
                if (rgb_matrix_get_val() <= RGB_MATRIX_VAL_STEP) {
                    // ind_set_blink(ind_state_blink3);
                    rgb_matrix_blink_set_remain_time(WLSIDR_LED_INDEX_VAI, 0); // 添加这句可以让进入这个条件时先停止win系统的指示灯闪烁
                    rgb_matrix_blink_set_interval_times(WLSIDR_LED_INDEX_VAD, 250, 3);
                    rgb_matrix_blink_set(WLSIDR_LED_INDEX_VAD);
                    if (rgb_matrix_get_val() != 0) rgb_matrix_set_color_all(0x00, 0x00, 0x00);
                }
            }
        }break;
        case UG_VALU: {
            if (rgbrec_is_started()) return false;
            if (record->event.pressed){
                if (rgb_matrix_get_val() >= (RGB_MATRIX_MAXIMUM_BRIGHTNESS - RGB_MATRIX_VAL_STEP)) {
                    // ind_set_blink(ind_state_blink3);
                    rgb_matrix_blink_set_remain_time(WLSIDR_LED_INDEX_VAD, 0); // 添加这句可以让进入这个条件时先停止win系统的指示灯闪烁
                    rgb_matrix_blink_set_interval_times(WLSIDR_LED_INDEX_VAI, 250, 3);
                    rgb_matrix_blink_set(WLSIDR_LED_INDEX_VAI);
                }
            }
        }break;
        case UG_NEXT: {
            if (record->event.pressed) {
                // rgb_blink_dir();
                if (rgb_matrix_get_mode() == RGB_MATRIX_CUSTOM_RGBR_PLAY) {
                    if (rgbrec_is_started()) {
                        rgbrec_read_current_channel(confinfo.record_channel);
                        rgbrec_end(confinfo.record_channel);
                        no_record_fg = false;
                        ind_set_blink(ind_state_none);
                    }
                    if (eeprom_read_byte((uint8_t *)CONFINFO_RGB_MOD_ADDR) != 0xFF)
                        rgb_matrix_mode(eeprom_read_byte((uint8_t *)CONFINFO_RGB_MOD_ADDR));
                    else{
                        rgb_matrix_mode(RGB_MATRIX_DEFAULT_MODE);
                        eeprom_write_byte((uint8_t *)CONFINFO_RGB_MOD_ADDR, RGB_MATRIX_DEFAULT_MODE);
                    }
                    // start_hsv = rgb_matrix_get_hsv();
                    return false;
                }

                // if (rgb_matrix_get_mode() == 9) {
                //     rgb_matrix_mode(13);
                //     eeprom_write_byte((uint8_t *)CONFINFO_RGB_MOD_ADDR, 13);
                //     return false;
                // }  //切换饱和度第一个模式保存不住

                uint8_t mode = eeprom_read_byte((uint8_t *)CONFINFO_RGB_MOD_ADDR);
                record_rgbmatrix_increase(&mode);
                eeprom_write_byte((uint8_t *)CONFINFO_RGB_MOD_ADDR, mode);
                // start_hsv = rgb_matrix_get_hsv();
            }

            return false;
        } break;
        case RM_VALU:{             //侧灯亮度循环
            // if (record->event.pressed){
            //     if (rgblight_get_val() != RGBLIGHT_LIMIT_VAL){
            //         rgblight_increase_val();
            //     } else {
            //         rgblight_sethsv(rgblight_get_hue(),rgblight_get_sat(),0);
            //     }
            // }
            return false;
        }break;
        case RM_SPDU:{             //侧灯速度循环
            if (record->event.pressed){
                // if (!rgblight_is_enabled() || rgblight_get_mode() == 1 || rgbrec_is_started()) return false;
                // confinfo.rgb_speed = (confinfo.rgb_speed + 1) % 5;
                // eeconfig_confinfo_update(confinfo.raw);
                // hs_rgblight_init(confinfo.rgb_speed);
            }
            return false;
        }break;
        // case RM_PREV: {
        //     if (record->event.pressed) {
        //         // rgb_blink_dir();
        //         if (rgb_matrix_get_mode() == RGB_MATRIX_CUSTOM_RGBR_PLAY) {
        //             if (rgbrec_is_started()) {
        //                 rgbrec_read_current_channel(confinfo.record_channel);
        //                 rgbrec_end(confinfo.record_channel);
        //                 no_record_fg = false;
        //                 ind_set_blink(ind_state_none);
        //             }
        //             if (eeprom_read_byte((uint8_t *)CONFINFO_RGB_MOD_ADDR) != 0xFF)
        //                 rgb_matrix_mode(eeprom_read_byte((uint8_t *)CONFINFO_RGB_MOD_ADDR));
        //             else{
        //                 rgb_matrix_mode(RGB_MATRIX_DEFAULT_MODE);
        //                 eeprom_write_byte((uint8_t *)CONFINFO_RGB_MOD_ADDR, RGB_MATRIX_DEFAULT_MODE);
        //             }
        //             // start_hsv = rgb_matrix_get_hsv();
        //             return false;
        //         }

        //         if (rgb_matrix_get_mode() == 13) {
        //             rgb_matrix_mode(9);
        //             eeprom_write_byte((uint8_t *)CONFINFO_RGB_MOD_ADDR, 9);
        //             return false;
        //         }

        //         uint8_t mode = eeprom_read_byte((uint8_t *)CONFINFO_RGB_MOD_ADDR);
        //         record_rgbmatrix_decrease(&mode);
        //         eeprom_write_byte((uint8_t *)CONFINFO_RGB_MOD_ADDR, mode);
        //         // start_hsv = rgb_matrix_get_hsv();
        //     }

        //     return false;
        // } break;
        case UG_TOGG:{
            if (record->event.pressed){
                confinfo.rgb_enable  = !confinfo.rgb_enable;
                if (confinfo.rgb_enable){
                    led_power_enable();
                } else {
                    led_power_disable();
                }
                eeconfig_confinfo_update(confinfo.raw);
            }
            return false;
        }break;
        case RGB_M_T:{
            if (record->event.pressed) {
                im_test_rgb_state = 1;
            }
        }break;
        case BT_TEST: {
            if (record->event.pressed) {
                md_send_devctrl(0x62);
            }
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

bool modip_update(uint8_t mode) {

    last_matrix_activity_trigger();

    return true;
}

void wlsidr_task(void) {

    if ((lpwr_get_state() == LPWR_NORMAL) && (wlsidr_blink_get_state() == WLSIDR_STATE_NONE)) {
        if (wireless_get_devs().now != DEVS_USB) {
            if (*md_getp_state() != MD_STATE_CONNECTED) {
                wireless_devs_change(DEVS_USB, wireless_get_devs().now, false);
            }
        } else {
            // bool wlsidr_set_blink(uint8_t led_index, wlsidr_state_t state);
            // wlsidr_set_blink(WLSIDR_LED_INDEX_BT1, WLSIDR_STATE_NONE);
            if (USB_DRIVER.state != USB_ACTIVE) {
                wireless_devs_change(!DEVS_USB, DEVS_USB, false);
            }
        }
    }

    if (wlsidr_blink_get_state() != WLSIDR_STATE_NONE) {
        last_matrix_activity_trigger();
    }
}

bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    // static bool caps_fg = false;
    RGB rgb_white = hsv_to_rgb((HSV){.h = 0, .s = 0, .v = RGB_MATRIX_VAL_STEP * 5}); 

    // if (rgb_matrix_get_mode() == 37 || rgb_matrix_get_mode() == 33) {
    //     extern ws2812_led_t ws2812_leds[WS2812_LED_COUNT];
    //     ws2812_leds[23].r = ws2812_leds[25].r = ws2812_leds[24].r;
    //     ws2812_leds[23].g = ws2812_leds[25].g = ws2812_leds[24].g;
    //     ws2812_leds[23].b = ws2812_leds[25].b = ws2812_leds[24].b;

    //     ws2812_leds[52].r = ws2812_leds[50].r = ws2812_leds[51].r;
    //     ws2812_leds[52].g = ws2812_leds[50].g = ws2812_leds[51].g;
    //     ws2812_leds[52].b = ws2812_leds[50].b = ws2812_leds[51].b;

    //     ws2812_leds[66].r = ws2812_leds[64].r = ws2812_leds[65].r;
    //     ws2812_leds[66].g = ws2812_leds[64].g = ws2812_leds[65].g;
    //     ws2812_leds[66].b = ws2812_leds[64].b = ws2812_leds[65].b;

    //     ws2812_leds[87].r = ws2812_leds[88].r = ws2812_leds[84].r = ws2812_leds[85].r = ws2812_leds[86].r;
    //     ws2812_leds[87].g = ws2812_leds[88].g = ws2812_leds[84].g = ws2812_leds[85].g = ws2812_leds[86].g;
    //     ws2812_leds[87].b = ws2812_leds[88].b = ws2812_leds[84].b = ws2812_leds[85].b = ws2812_leds[86].b;
    // }

    if (!confinfo.rgb_enable ||  (!charging && (*md_getp_bat() <= BATTERY_CAPACITY_LOW))) {
        rgb_matrix_set_color_all(0, 0, 0);
    }

    if (!keymap_is_mac_system() && keymap_config.no_gui && !rgbrec_is_started())
        rgb_matrix_set_color(7, rgb_white.r,rgb_white.g,rgb_white.b);
    // } else {
    //     rgb_matrix_set_color(0, 0, 0, 0);
    // }

    if (host_keyboard_led_state().caps_lock && !rgbrec_is_started()){
        // rgb_matrix_set_color(6, rgb_white.r,rgb_white.g,rgb_white.b);
        gpio_write_pin_high(CPAS_PIN);
    } else {
        gpio_write_pin_low(CPAS_PIN);
    }

    // if (host_keyboard_led_state().caps_lock && !rgbrec_is_started()){
    //     rgb_matrix_set_color(6, rgb_white.r,rgb_white.g,rgb_white.b);
    //     if (!caps_fg){
    //         gpio_write_pin_low(MCU_INT);
    //         wait_ms(5);
    //         uint8_t data = 0;
    //         data |= 0x10;
    //         i2c_write_register(0xA0,0x80,&data,0x01,10);//地址 数据 长度 超时时间
    //         gpio_write_pin_high(MCU_INT);
    //     }
    //     caps_fg = true;
    // } else {
    //     if (caps_fg){
    //         gpio_write_pin_low(MCU_INT);
    //         wait_ms(5);
    //         uint8_t data = 0;
    //         i2c_write_register(0xA0,0x80,&data,0x01,10);//地址 数据 长度 超时时间
    //         gpio_write_pin_high(MCU_INT);
    //     }
    //     caps_fg = false;
    // }

    // if (host_keyboard_led_state().num_lock && !rgbrec_is_started()){
    //     rgb_matrix_set_color(3, rgb_white.r,rgb_white.g,rgb_white.b);
    // } else {
    //     rgb_matrix_set_color(3, 0, 0, 0);
    // }
    
    if (host_keyboard_led_state().scroll_lock && !rgbrec_is_started()){
        rgb_matrix_set_color(0, rgb_white.r,rgb_white.g,rgb_white.b);
    } else {
    //     rgb_matrix_set_color(1, 0, 0, 0);
    }
    

    rgb_matrix_indicators_advanced_rgblight(led_min, led_max);

    rgb_matrix_blink_task(led_min,led_max);

    rgb_test();

    return true;
}

// 出现异常时，直接复位MCU，内部使用，不公开
void _unhandled_exception(void) {
    mcu_reset();
}

// USB模式下模拟鼠标数据测试回报率，内部使用，不公开
void report_rate_test_task(void) {
    extern void host_mouse_send(report_mouse_t * report);

    static uint8_t flip                = 0;
    static report_mouse_t mouse_format = {0};

    switch (flip) {
        case 0: { // 右移
            mouse_format.x = 10;
            mouse_format.y = 0;
#ifdef WIRELESS_ENABLE
            bool md_is_busy(void);
            if (get_transport() == TRANSPORT_WLS)
                if (md_is_busy()) return;
#endif
            host_mouse_send(&mouse_format);
            flip = 1;
        } break;
        case 1: { // 上移
            mouse_format.x = 0;
            mouse_format.y = -10;
#ifdef WIRELESS_ENABLE
            bool md_is_busy(void);
            if (get_transport() == TRANSPORT_WLS)
                if (md_is_busy()) return;
#endif
            host_mouse_send(&mouse_format);
            flip = 2;
        } break;
        case 2: { // 左移
            mouse_format.x = -10;
            mouse_format.y = 0;
#ifdef WIRELESS_ENABLE
            bool md_is_busy(void);
            if (get_transport() == TRANSPORT_WLS)
                if (md_is_busy()) return;
#endif
            host_mouse_send(&mouse_format);
            flip = 3;
        } break;
        case 3: { // 下移
            mouse_format.x = 0;
            mouse_format.y = 10;
#ifdef WIRELESS_ENABLE
            bool md_is_busy(void);
            if (get_transport() == TRANSPORT_WLS)
                if (md_is_busy()) return;
#endif
            host_mouse_send(&mouse_format);
            flip = 0;
        } break;
        default: {
            flip = 0;
        } break;
    }
}

void md_main_task_hook(void) {

    if (test_rate_flag) {
        report_rate_test_task();
    }
}

void rgb_test(void){
    static uint32_t im_test_rgb_timer = 0x00;
    static uint16_t RGB_MATRIX_TEST_TIME = 3000; // RGB测试时间，单位为毫秒
    if (im_test_rgb_state) {

        if (!im_test_rgb_timer) {
            im_test_rgb_timer = timer_read32();
        }

        if (timer_elapsed32(im_test_rgb_timer) >= (RGB_MATRIX_TEST_TIME)) {
            im_test_rgb_state += 1;
            im_test_rgb_timer = timer_read32();
        }

        RGB rgb_white = hsv_to_rgb((HSV){.h = 0, .s = 0, .v = RGB_MATRIX_VAL_STEP * 5});

        switch (im_test_rgb_state) {
            case 1: {
                rgb_matrix_set_color_all(rgb_white.r, 0, 0);
            } break;
            case 2: {
                rgb_matrix_set_color_all(0, rgb_white.g, 0);
            } break;
            case 3: {
                rgb_matrix_set_color_all(0, 0, rgb_white.b);
            } break;
            case 4:
            default: {
                im_test_rgb_state = 0x00;
                im_test_rgb_timer = 0x00;
            } break;
        }
    }
    query();
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

                if (row == 0 && col == 0) {

                    hs_frequency_test = true;
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

// i2c_readReg  i2c_writeReg
