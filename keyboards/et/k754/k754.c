// Copyright 2024 sdk66 (@sdk66)
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include "wls/wls.h"
#include "rgb_record/rgb_record.h"
#include "rgb_record/rgb_rgblight.h"

#ifdef WIRELESS_ENABLE
#    include "wireless.h"
#    include "usb_main.h"
#    include "lowpower.h"
#    include "rgb_matrix_blink.h"
#endif

typedef union {
    uint32_t raw;
    struct {
        uint8_t hs_mute_flag : 1;
        uint8_t devs : 3;
        uint8_t rgblight_flag: 1;
        uint8_t last_btdevs : 3;
        uint8_t record_channel : 2;  
        uint8_t sleep_timeout : 3;
        uint8_t hs_led_tog_flag : 1;
        uint8_t rgb_light_mode : 4;
        uint8_t rgb_light_speed : 4;
        uint8_t record_last_mode;
    };
} confinfo_t;
confinfo_t confinfo;

typedef struct {
    bool active;
    uint32_t timer;
    uint32_t interval;
    uint32_t times;
    uint8_t index;
    RGB rgb;
    void (*blink_cb)(uint8_t);
} hs_rgb_indicator_t;

enum layers {
    _BL = 0,
    _FL,
    // _MBL,
    // _MFL,
    _DEFA, 
};

void hs_reset_settings(void);
// void rgb_matrix_start_rec(void);
void matrix_init_user(void);
void hs_rgblight_increase(uint8_t rgblight_mode);
uint8_t buff[] = {14,8,2,1,1,1,1,1,1,1,0}; 
static uint8_t mode = 0;
static bool hs_dir_flag = true;

#define keymap_is_base_layer() ((get_highest_layer(default_layer_state) == _BL) || (get_highest_layer(default_layer_state) == _FL))

uint32_t post_init_timer     = 0x00;
bool inqbat_flag             = false;
bool charging_state          = false;
bool bat_full_flag           = false;
bool enable_bat_indicators   = true;
uint32_t bat_indicator_cnt   = true;
static uint32_t ee_clr_timer = 0;
bool hs_clear_flag = false;
bool test_white_light_flag   = false;
bool hs_color_test;
bool hs_frequency_test;
bool lower_sleep = false;
bool no_record_fg;
bool im_test_rate_flag;
uint32_t rec_time;
bool rec_filp;
HSV start_hsv;

uint32_t fn_last_tap_time = 0;
uint8_t fn_tap_count = 0;

void bat_indicators_hook(uint8_t index);
void ind_indicators_hook(uint8_t index);
void nkr_indicators_hook(uint8_t index);
blink_rgb_t blink_rgbs[RGB_MATRIX_BLINK_COUNT] = {
    // {.index = HS_RGB_BLINK_INDEX_USB, .interval = 5000, .times = 1, .color = {.r = 0xFF, .g = 0x00, .b = 0x00}, .blink_cb = NULL},
    // {.index = RGB_MATRIX_BLINK_INDEX_BAT, .interval = 250, .times = 3, .color = {.r = 0x00, .g = 0x00, .b = 0x00}, .blink_cb = bat_indicators_hook},
    {.index = RGB_MATRIX_BLINK_INDEX_IND, .interval = 250, .times = 3, .color = {.r = 0x00, .g = 0x00, .b = 0x00}, .blink_cb = ind_indicators_hook},
    {.index = RGB_MATRIX_BLINK_INDEX_ALL, .interval = 250, .times = 3, .color = {.r = WIITE_B, .g = 0x00, .b = 0x00}, .blink_cb = nkr_indicators_hook},
};

void nkr_indicators_hook(uint8_t index) {
    if (index == RGB_MATRIX_BLINK_INDEX_ALL) {
        if ((blink_rgbs[RGB_MATRIX_BLINK_COUNT - 1].color.r == WIITE_B) && (blink_rgbs[RGB_MATRIX_BLINK_COUNT - 1].color.g == 0x00) && (blink_rgbs[RGB_MATRIX_BLINK_COUNT - 1].color.b == 0x00)) {
            if (rgb_matrix_blink_set_color(index, 0x00, 0x00, WIITE_B) != true) {
                dprintf("RGB_BLUE again\n");
            }
            rgb_matrix_blink_set_times(index, 1);
            rgb_matrix_blink_set(index);
            dprintf("GREEM\r\n");
        } else if ((blink_rgbs[RGB_MATRIX_BLINK_COUNT - 1].color.r == 0x00) && (blink_rgbs[RGB_MATRIX_BLINK_COUNT - 1].color.g == 0x00) && (blink_rgbs[RGB_MATRIX_BLINK_COUNT - 1].color.b == WIITE_B)) {
            if (rgb_matrix_blink_set_color(index, 0x00, WIITE_B, 0x00) != true) {
                dprintf("RGB_GREEN again\n");
            }
            rgb_matrix_blink_set_times(index, 1);
            rgb_matrix_blink_set(index);
            dprintf("BLUE\r\n");
        }
    }
}

void bat_indicators_hook(uint8_t index) {
    static uint32_t battery_process_time = 0;
    static bool bat_flip = true;

    // charging_state = 0;
    // bat_full_flag = 0;
    // *md_getp_bat() = BATTERY_CAPACITY_STOP;

    if (charging_state && ( (bat_full_flag))) { // power full
        battery_process_time = 0;
        bat_flip = true;
        rgb_matrix_blink_set_color(index, 0x00, 0x00, 0x00);
        rgb_matrix_blink_set_interval_times(index, 300, 0xFF); // led off
    } else if (charging_state) { // charing...
        battery_process_time = 0;
        bat_flip = true;
        rgb_matrix_blink_set_color(index, 0xFF, 0x00, 0x00);
        rgb_matrix_blink_set_interval_times(index, 300, 0xFF); 
    } else if (*md_getp_bat()<= BATTERY_CAPACITY_LOW) { // low power

        if (bat_flip) {
            rgb_matrix_blink_set_color(index, 0xFF, 0x00, 0x00);
            rgb_matrix_blink_set_interval_times(index, 500, 1);
        } else {
            rgb_matrix_blink_set_color(index, 0x00, 0x00, 0x00);
            rgb_matrix_blink_set_interval_times(index, 500, 0xFF); 
        }
        // bat_flip = !bat_flip;

        if (*md_getp_bat() <= BATTERY_CAPACITY_STOP) {
            if (!battery_process_time) {
                battery_process_time = timer_read32();
            }

            if (battery_process_time && timer_elapsed32(battery_process_time) > 25000) { 
                battery_process_time = 0;
                lower_sleep = true;
                md_send_devctrl(MD_SND_CMD_DEVCTRL_USB);
                lpwr_set_timeout_manual(true);
            }
        }
    } else {
        battery_process_time = 0;
        bat_flip = true;
        rgb_matrix_blink_set_color(index, 0x00, 0x00, 0x00);
        rgb_matrix_blink_set_interval_times(index, 300, 0xFF); // 灭灯
    }

    rgb_matrix_blink_set(index);
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
            rgb_matrix_blink_set_color(index, 0x00, 0x00, 0x00);
            rgb_matrix_blink_set_interval_times(index, 50, 0xFF);
        } break;
        case ind_state_blink3: {
            rgb_matrix_blink_set_color(index, 0xFF, 0xFF, 0xFF);
            rgb_matrix_blink_set_interval_times(index, 250, 3);
            ind_status = ind_state_exec;
        } break;
        case ind_state_blinkon: {
            rgb_matrix_blink_set_color(index, 0xFF, 0xFF, 0xFF);
            rgb_matrix_blink_set_interval_times(index, 500, 1);
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

void ind_set_blink(ind_status_t status) {
    ind_status = status;
    rgb_matrix_blink_set_remain_time(RGB_MATRIX_BLINK_INDEX_IND, 0x00);
    ind_indicators_hook(RGB_MATRIX_BLINK_INDEX_IND);
}

bool rgb_matrix_blink_user(blink_rgb_t *blink_rgb) {

    switch (blink_rgb->index) {
        case RGB_MATRIX_BLINK_INDEX_BAT: {
            if (ind_status != ind_state_none) {
                return false;
            }
            if(test_white_light_flag == true){
                return false;
            }

            // if (blink_rgb->flip) {
            //     if (blink_rgb->color.r)
            //         rgb_matrix_set_color(HS_MATRIX_BLINK_INDEX_BAT, 0xff, 0x00, 0x00);
            // }else {
            //     rgb_matrix_set_color(HS_MATRIX_BLINK_INDEX_BAT, 0x00, 0x00, 0x00);
            // }
            return false;
        } break;

        case RGB_MATRIX_BLINK_INDEX_IND: {
            
            switch(ind_status) {
                case ind_state_none: {
                    return false; // 按键灯做指示灯时需要恢复灯效
                } break;
                case ind_state_blinkon:
                case ind_state_blinkoff: {
                    if (!no_record_fg) return false;
                    // 控制backspace按键灯
                    if (!blink_rgb->flip) {  //控制开始录制时需要闪烁的灯珠
                        rgb_matrix_set_color(HS_RGB_BLINK_INDEX_REC, blink_rgb->color.r, blink_rgb->color.g, blink_rgb->color.b);
                    } 
                } break;
                case ind_state_exec: {
                    // 开启灯效录制 不闪烁回连
                    if (rgbrec_is_started()) {
                        return false;
                    }

                    // 控制四颗方向键灯
                    if (blink_rgb->flip) {
                        for (uint8_t i = 91; i <= 96; i++) {
                            rgb_matrix_set_color(i, 0x3f, 0x3f, 0x3f);
                        }
                    } else {
                        for (uint8_t i = 91; i <= 96; i++) {
                            rgb_matrix_set_color(i, 0x00, 0x00, 0x00);
                        }
                    }
                } break;
                default: break;
            }
            return false;
        } break;
#undef HS_RGB_BLINK_INDEX_REC
#ifdef HS_RGB_BLINK_INDEX_REC
        case HS_RGB_BLINK_INDEX_REC: {
        } break;
#endif
        default: { 
            //开启一键白光 不闪烁回连
            if (test_white_light_flag) {
                return false;
            }
            //开启灯效录制 不闪烁回连
            if (rgbrec_is_started()) {
                return false;
            }
        } break;
    }

    return true;
}

void eeconfig_confinfo_update(void) {

    eeconfig_update_kb(confinfo.raw);
}

uint32_t eeconfig_confinfo_read(void) {

    return eeconfig_read_kb();
}

uint8_t hs_get_sleep_timeout(void){
    return confinfo.sleep_timeout;
}

void hs_set_sleep_timeout(uint8_t sleep_timeout){
    confinfo.sleep_timeout = sleep_timeout;
}

bool hs_test_ee_fg = false;
bool press_ee_clr_flag = false;
void eeconfig_confinfo_default(void) {

    confinfo.hs_mute_flag      = true;
    confinfo.record_channel    = 0;
    confinfo.record_last_mode  = 0xff;
    confinfo.last_btdevs       = 1;
    confinfo.sleep_timeout     = 7;
    confinfo.rgblight_flag     = true;
    confinfo.rgb_light_mode    = 0;
    confinfo.rgb_light_speed   = 2;
    hs_dir_flag                = true;
    keymap_config.no_gui       = false;
    confinfo.hs_led_tog_flag   = false;
    // #ifdef WIRELESS_ENABLE
    //     confinfo.devs = DEVS_USB;
    // #endif
#ifdef WIRELESS_ENABLE
    if (press_ee_clr_flag) {
        hs_test_ee_fg = true;
        press_ee_clr_flag = false;
    } 
#endif
    // confinfo.devs = wireless_get_current_devs();
    eeconfig_init_user_datablock();
    eeconfig_confinfo_update();
    
#if defined(NKRO_ENABLE) && defined(FORCE_NKRO)
    keymap_config.nkro = 1;
    eeconfig_update_keymap(keymap_config.raw);
#endif

#ifdef RGBLIGHT_ENABLE
    rgblight_mode(buff[0]);
#endif
}

void eeconfig_confinfo_init(void) {

    confinfo.raw = eeconfig_confinfo_read();
    if (!confinfo.raw) {
        eeconfig_confinfo_default();
    }
}

void keyboard_post_init_kb(void) {

#ifdef CONSOLE_ENABLE
    debug_enable = true;
#endif
    
    eeconfig_confinfo_init();

#ifdef LED_POWER_EN_PIN
    gpio_set_pin_output(LED_POWER_EN_PIN);
    gpio_write_pin_high(LED_POWER_EN_PIN);

#endif

#ifdef MM_BT_DEF_PIN
    setPinInputHigh(MM_BT_DEF_PIN);
#endif

#ifdef MM_2G4_DEF_PIN
    setPinInputHigh(MM_2G4_DEF_PIN);
#endif

#ifdef USB_POWER_EN_PIN
    gpio_write_pin_low(USB_POWER_EN_PIN);
    gpio_set_pin_output(USB_POWER_EN_PIN);
#endif

#ifdef HS_BAT_CABLE_PIN
    setPinInput(HS_BAT_CABLE_PIN);
#endif

#ifdef BAT_FULL_PIN
    setPinInputHigh(BAT_FULL_PIN);
#endif

#ifdef HS_LED_BOOSTING_PIN
    gpio_set_pin_output(HS_LED_BOOSTING_PIN);
    gpio_write_pin_high(HS_LED_BOOSTING_PIN);
#endif

#ifdef WIRELESS_ENABLE
    wireless_init();
    wireless_devs_change(!DEVS_USB, confinfo.devs, false); 
    post_init_timer = timer_read32();
#endif

#ifdef RGB_MATRIX_BLINK_INDEX_BAT
    rgb_matrix_blink_set_interval_times(RGB_MATRIX_BLINK_INDEX_BAT, 200, 0xFF);
    rgb_matrix_blink_set(RGB_MATRIX_BLINK_INDEX_BAT);
#endif

#ifdef RGB_MATRIX_BLINK_INDEX_IND
    rgb_matrix_blink_set_interval_times(RGB_MATRIX_BLINK_INDEX_IND, 200, 0xFF);
    rgb_matrix_blink_set(RGB_MATRIX_BLINK_INDEX_IND);
#endif

#ifdef HS_RGB_BLINK_INDEX_REC
    rgb_matrix_blink_set_interval_times(HS_RGB_BLINK_INDEX_REC, 200, 0xFF);
    rgb_matrix_blink_set(HS_RGB_BLINK_INDEX_REC);
#endif
    if (wireless_get_current_devs() == DEVS_2G4) {
        md_send_devctrl(0xFF); // 设定无线设备的深度休眠时间
    } else {
        md_send_devctrl(0x80+HS_WIRESS_SLEEP_TIME); // 设定无线设备的深度休眠时间
    }

    matrix_init_user();
    keyboard_post_init_user();
    rgbrec_init(confinfo.record_channel);
    keymap_config.no_gui = false;
}

#ifdef WIRELESS_ENABLE

void usb_power_connect(void) {

#    ifdef USB_POWER_EN_PIN
    gpio_write_pin_low(USB_POWER_EN_PIN);
#    endif
}

void usb_power_disconnect(void) {

#    ifdef USB_POWER_EN_PIN
    gpio_write_pin_high(USB_POWER_EN_PIN);
#    endif
}

void suspend_power_down_kb(void) {

#    ifdef LED_POWER_EN_PIN
    gpio_write_pin_low(LED_POWER_EN_PIN);
#    endif

#    ifdef HS_LED_BOOSTING_PIN
    gpio_write_pin_low(HS_LED_BOOSTING_PIN);
#    endif

    suspend_power_down_user();
}

void suspend_wakeup_init_kb(void) {

#    ifdef LED_POWER_EN_PIN
    gpio_write_pin_high(LED_POWER_EN_PIN);
#    endif

#    ifdef HS_LED_BOOSTING_PIN
    gpio_write_pin_high(HS_LED_BOOSTING_PIN);
#    endif

    wireless_devs_change(wireless_get_current_devs(), wireless_get_current_devs(), false);
    suspend_wakeup_init_user();
    hs_rgb_blink_set_timer(timer_read32());
}

bool lpwr_is_allow_timeout_hook(void) {

    // if (wireless_get_current_devs() == DEVS_USB) {
    //     return false;
    // }

    return true;
}

void wireless_post_task(void) {

    // auto switching devs
    if (post_init_timer && timer_elapsed32(post_init_timer) >= 100) {

        md_send_devctrl(MD_SND_CMD_DEVCTRL_FW_VERSION);   // get the module fw version.
        md_send_devctrl(MD_SND_CMD_DEVCTRL_SLEEP_BT_EN);  // timeout 30min to sleep in bt mode, enable
        md_send_devctrl(MD_SND_CMD_DEVCTRL_SLEEP_2G4_EN); // timeout 30min to sleep in 2.4g mode, enable
        wireless_devs_change(!confinfo.devs, confinfo.devs, false);
        post_init_timer = 0x00;
    }

    hs_mode_scan(false, confinfo.devs, confinfo.last_btdevs);
}

bool state = false;
uint32_t wls_process_long_press(uint32_t trigger_time, void *cb_arg) {
    uint16_t keycode = *((uint16_t *)cb_arg);

    switch (keycode) {
        case KC_BT1: {
            uint8_t mode = confinfo.devs;
            hs_modeio_detection(true, &mode, confinfo.last_btdevs);
            if ((mode == hs_bt) || (mode == hs_wireless) || (mode == hs_none)) {
                wireless_devs_change(wireless_get_current_devs(), DEVS_BT1, true);
                // if (wireless_get_current_devs() != DEVS_BT1) {
                    state = true;
                    hs_rgb_blink_set_timer(timer_read32());
                // }
            }

        } break;
        case KC_BT2: {
            uint8_t mode = confinfo.devs;
            hs_modeio_detection(true, &mode, confinfo.last_btdevs);
            if ((mode == hs_bt) || (mode == hs_wireless) || (mode == hs_none)) {
                wireless_devs_change(wireless_get_current_devs(), DEVS_BT2, true);
                // if (wireless_get_current_devs() != DEVS_BT1) {
                    state = true;
                    hs_rgb_blink_set_timer(timer_read32());
                // }
            }
        } break;
        case KC_BT3: {
            uint8_t mode = confinfo.devs;
            hs_modeio_detection(true, &mode, confinfo.last_btdevs);
            if ((mode == hs_bt) || (mode == hs_wireless) || (mode == hs_none)) {
                wireless_devs_change(wireless_get_current_devs(), DEVS_BT3, true);
                // if (wireless_get_current_devs() != DEVS_BT1) {
                    state = true;
                    hs_rgb_blink_set_timer(timer_read32());
                // }
            }
        } break;
        case KC_2G4: {
            uint8_t mode = confinfo.devs;
            hs_modeio_detection(true, &mode, confinfo.last_btdevs);
            if ((mode == hs_2g4) || (mode == hs_wireless) || (mode == hs_none)) {
                wireless_devs_change(wireless_get_current_devs(), DEVS_2G4, true);
                // if (wireless_get_current_devs() != DEVS_BT1) {
                    state = true;
                    hs_rgb_blink_set_timer(timer_read32());
                // }
            }
        } break;
        // case BT_TEST: {
        //     md_send_devctrl(0x62);
        //     // wait_ms(30);
        //     // wireless_devs_change(wireless_get_current_devs(), DEVS_BT2, true);
        // } break;
        default:
            break;
    }

    return 0;
}

bool process_record_wls(uint16_t keycode, keyrecord_t *record) {
    static uint16_t keycode_shadow                     = 0x00;
    static deferred_token wls_process_long_press_token = INVALID_DEFERRED_TOKEN;

    keycode_shadow = keycode;

#    ifndef WLS_KEYCODE_PAIR_TIME
#        define WLS_KEYCODE_PAIR_TIME 3000
#    endif

#    define WLS_KEYCODE_EXEC(wls_dev)                                                                                          \
        do {                                                                                                                   \
            if (record->event.pressed) {                                                                                       \
                if (wireless_get_current_devs() != wls_dev)                                                                    \
                    wireless_devs_change(wireless_get_current_devs(), wls_dev, false);                                         \
                if (wls_process_long_press_token == INVALID_DEFERRED_TOKEN) {                                                  \
                    wls_process_long_press_token = defer_exec(WLS_KEYCODE_PAIR_TIME, wls_process_long_press, &keycode_shadow); \
                }                                                                                                              \
            } else {                                                                                                           \
                cancel_deferred_exec(wls_process_long_press_token);                                                            \
                wls_process_long_press_token = INVALID_DEFERRED_TOKEN;                                                         \
            }                                                                                                                  \
        } while (false)

    switch (keycode) {
        // case BT_TEST:{
        //     WLS_KEYCODE_EXEC(wireless_get_current_devs());
        // }break;

        case KC_BT1: {
            uint8_t mode = confinfo.devs;
            hs_modeio_detection(true, &mode, confinfo.last_btdevs);
            if ((mode == hs_bt) || (mode == hs_wireless) || (mode == hs_none)) {
                WLS_KEYCODE_EXEC(DEVS_BT1);
                
                if (record->event.pressed) {suspend_wakeup_init();state = false;
                hs_rgb_blink_set_timer(timer_read32());}
            }
            md_send_devctrl(0x80+HS_WIRESS_SLEEP_TIME);
        } break;
        case KC_BT2: {
            uint8_t mode = confinfo.devs;
            hs_modeio_detection(true, &mode, confinfo.last_btdevs);
            if ((mode == hs_bt) || (mode == hs_wireless) || (mode == hs_none)) {
                WLS_KEYCODE_EXEC(DEVS_BT2);
     
                if (record->event.pressed) {suspend_wakeup_init();state = false;
                hs_rgb_blink_set_timer(timer_read32());}
            }
            md_send_devctrl(0x80+HS_WIRESS_SLEEP_TIME);
        } break;
        case KC_BT3: {
            uint8_t mode = confinfo.devs;
            hs_modeio_detection(true, &mode, confinfo.last_btdevs);
            if ((mode == hs_bt) || (mode == hs_wireless) || (mode == hs_none)) {
                WLS_KEYCODE_EXEC(DEVS_BT3);
              
                if (record->event.pressed) {suspend_wakeup_init();state = false;
                hs_rgb_blink_set_timer(timer_read32());}
            }
            md_send_devctrl(0x80+HS_WIRESS_SLEEP_TIME);
        } break;
        case KC_2G4: {
            uint8_t mode = confinfo.devs;
            hs_modeio_detection(true, &mode, confinfo.last_btdevs);
            if ((mode == hs_2g4) || (mode == hs_wireless) || (mode == hs_none)) {
                WLS_KEYCODE_EXEC(DEVS_2G4);
           
                if (record->event.pressed) {suspend_wakeup_init();state = false;
                hs_rgb_blink_set_timer(timer_read32());}
            }
            md_send_devctrl(0xFF);
        } break;

        case KC_USB: {
            uint8_t mode = confinfo.devs;
            hs_modeio_detection(true, &mode, confinfo.last_btdevs);
            if ((mode == hs_2g4) || (mode == hs_wireless) || (mode == hs_none)) {
                WLS_KEYCODE_EXEC(DEVS_USB);
                state = false;
                hs_rgb_blink_set_timer(timer_read32());
            }
            // rgb_matrix_blink_set_remain_time(HS_RGB_BLINK_INDEX_USB, 0); 
            // rgb_matrix_blink_set_interval_times(HS_RGB_BLINK_INDEX_USB, 5000, 0xff);
            // rgb_matrix_blink_set(HS_RGB_BLINK_INDEX_USB);
        } break;
        default:
            return true;
    }

    return false;
}
#endif

bool process_record_user(uint16_t keycode, keyrecord_t *record) {

    // if (test_white_light_flag && record->event.pressed) {
    //     test_white_light_flag = false;
    //     rgb_matrix_set_color_all(0x00, 0x00, 0x00);
    // }

    if (*md_getp_state() == MD_STATE_CONNECTED && record->event.pressed) {
        hs_rgb_blink_set_timer(timer_read32());
    }

    switch (keycode) {
        case MO(_FL):
        // case MO(_MFL): {
            if (!record->event.pressed && rgbrec_is_started()) {
                if (no_record_fg == true) {
                    no_record_fg = false;
                    rgbrec_register_record(keycode, record);
                }
                no_record_fg = true;
            }
            break;
        // }
        case RP_END:
        case RP_P0:
        case RP_P1:
        case RP_P2:
        case RGB_MOD:
            break;
        default: {
            if (rgbrec_is_started()) {
                if (!IS_QK_MOMENTARY(keycode) && record->event.pressed) {
                    rgbrec_register_record(keycode, record);

                    return false;
                }
            }
        } break;
    }

    if (rgbrec_is_started() && (!(keycode == RP_P0 || keycode == RP_P1 || keycode == RP_P2 || keycode == RP_END || keycode == RGB_MOD || keycode == MO(_FL)))) {

        return false;
    }

    return true;
}

void im_rgblight_increase(void) {
    HSV rgb;
    uint8_t moude;
    moude = rgblight_get_mode();
    
    if (moude == 1){
        rgb = rgblight_get_hsv();
        if (rgb.h == 0 && rgb.s != 0) mode = 3;
        else mode = 9;
        switch (rgb.h){
            case 40:mode = 4;break;
            case 80:mode = 5;break;
            case 120:mode = 6;break;
            case 160:mode = 7;break;
            case 200:mode = 8;break;
            default :break;
        }
    }
    mode++;
    dprintf("mode = %d\r\n",mode);
    if(mode == 11) mode=0;
    if (mode == 10) {rgb = rgblight_get_hsv();rgblight_sethsv(0,255,rgb.v);rgblight_disable();confinfo.rgblight_flag = false;}
    else 
    {
        rgblight_enable();
        
        rgblight_mode(buff[mode]);
        confinfo.rgblight_flag = true;
    }
    rgb = rgblight_get_hsv();
    switch (mode){
        case 3: rgblight_sethsv(0,255,rgb.v);break;
        case 4: rgblight_sethsv(40,255,rgb.v);break;
        case 5: rgblight_sethsv(80,255,rgb.v);break;
        case 6: rgblight_sethsv(120,255,rgb.v);break;
        case 7: rgblight_sethsv(160,255,rgb.v);break;
        case 8: rgblight_sethsv(200,255,rgb.v);break;
        case 9: rgblight_sethsv(0,0,rgb.v);break;
        case 0: rgblight_set_speed(255);break;
        default:rgblight_set_speed(200);break;
    } 
}

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {

    if (process_record_user(keycode, record) != true) {
        return false;
    }

#ifdef WIRELESS_ENABLE
    if (process_record_wls(keycode, record) != true) {
        return false;
    }
#endif

    if (hs_frequency_test && hs_color_test){
        if (!(keycode >= KC_1 && keycode <= KC_4))
            return false;
    } 

    if ((confinfo.devs != DEVS_USB && *md_getp_state() != MD_STATE_CONNECTED)){
        if(IS_BASIC_KEYCODE(keycode) || IS_MODIFIER_KEYCODE(keycode)){
            return false;
        }
    }

    switch (keycode) {
        case NK_TOGG: {
            if (record->event.pressed) {
                rgb_matrix_blink_set_color(RGB_MATRIX_BLINK_INDEX_ALL, WIITE_B, 0x00, 0x00);
                rgb_matrix_blink_set_times(RGB_MATRIX_BLINK_INDEX_ALL, 1);
                rgb_matrix_blink_set(RGB_MATRIX_BLINK_INDEX_ALL);
            }
        } break;
        case EX_RATE: {
            if (record->event.pressed) {
                im_test_rate_flag = true;
            } else {
                im_test_rate_flag = false;
            }
            return false;
        } break;
        case KC_CUT: { 
            if(record->event.pressed){
                register_code16(KC_LCTL);
                register_code16(KC_X);
            }
            else{
                unregister_code16(KC_LCTL);
                unregister_code16(KC_X);
            }
            return false;
        } break;
        case KC_COPY: {
            if(record->event.pressed){
                register_code16(KC_LCTL);
                register_code16(KC_C);
            }
            else{
                unregister_code16(KC_LCTL);
                unregister_code16(KC_C);
            }
            return false;
        } break;
        case KC_PASTE: { 
            if(record->event.pressed){
                register_code16(KC_LCTL);
                register_code16(KC_V);
            }
            else{
                unregister_code16(KC_LCTL);
                unregister_code16(KC_V);
            }
            return false;
        } break;
        case KC_FIND: { 
            if(record->event.pressed){
                register_code16(KC_LCTL);
                register_code16(KC_F);
            }
            else{
                unregister_code16(KC_LCTL);
                unregister_code16(KC_F);
            }
            return false;
        } break;
        case KC_1: { 
            if(hs_frequency_test && hs_color_test){
                if (record->event.pressed) {
                    md_send_devctrl(0x60);
                    md_rf_send_carrier(0, 5, 0x01);
                
                } 
                return false;
            }
        } break;
        case KC_2: { 
            if(hs_frequency_test && hs_color_test){
                if (record->event.pressed) {
                    md_send_devctrl(0x60);
                    md_rf_send_carrier(19, 5, 0x01);
                   
                } 
                return false;
            }
        } break;
        case KC_3: { 
            if(hs_frequency_test && hs_color_test){
                if (record->event.pressed) {
                    md_send_devctrl(0x60);
                    md_rf_send_carrier(39, 5, 0x01);
                   
                } 
                return false;
            }
        } break;
        case KC_4: { 
            if(hs_frequency_test && hs_color_test){
                if (record->event.pressed) {
                    md_rf_send_stop();
                } 
                return false;
            } 
        } break;
        case QK_BOOT: {
            if (record->event.pressed) {
                dprintf("into boot!!!\r\n");
                eeconfig_disable();
                bootloader_jump();
            }
        } break;
        case KC_TEST: {
            if (!record->event.pressed) {
                if (hs_clear_flag == false) {
                    test_white_light_flag = !test_white_light_flag;
                }
                if (hs_clear_flag) {
                    hs_clear_flag = false;
                }
                ee_clr_timer = 0;
            } else {
                ee_clr_timer = timer_read32();
            }
            return false;
        } break;
        case HS_DIR: {
            if (record->event.pressed) {
                hs_dir_flag = !hs_dir_flag;
            }
            return true;
        } break;
        case KC_W: { 
            if (!hs_dir_flag) {
                if (record->event.pressed && !rgbrec_is_started()) {
                    register_code(KC_UP);
                } else {
                    unregister_code(KC_UP);
                }
                return false;
            } else {
                return true;
            }
        } break;
        case KC_A: { 
            if (!hs_dir_flag) {
                if (record->event.pressed && !rgbrec_is_started()) {
                    register_code(KC_LEFT);
                } else {
                    unregister_code(KC_LEFT);
                }
                return false;
            } else {
                return true;
            }
        } break;
        case KC_S: { 
            if (!hs_dir_flag) {
                if (record->event.pressed && !rgbrec_is_started()) {
                    register_code(KC_DOWN);
                } else {
                    unregister_code(KC_DOWN);
                }
                return false;
            } else {
                return true;
            }
        } break;
        case KC_D: { 
            if (!hs_dir_flag) {
                if (record->event.pressed && !rgbrec_is_started()) {
                    register_code(KC_RGHT);
                } else {
                    unregister_code(KC_RGHT);
                }
                return false;
            } else {
                return true;
            }
        } break;
        case KC_UP: { 
            if (!hs_dir_flag) {
                if (record->event.pressed && !rgbrec_is_started()) {
                    register_code(KC_W);
                } else {
                    unregister_code(KC_W);
                }
                return false;
            } else {
                return true;
            }
        } break;
        case KC_LEFT: { 
            if (!hs_dir_flag) {
                if (record->event.pressed && !rgbrec_is_started()) {
                    register_code(KC_A);
                } else {
                    unregister_code(KC_A);
                }
                return false;
            } else {
                return true;
            }
        } break;
        case KC_DOWN: { 
            if (!hs_dir_flag) {
                if (record->event.pressed && !rgbrec_is_started()) {
                    register_code(KC_S);
                } else {
                    unregister_code(KC_S);
                }
                return false;
            } else {
                return true;
            }
        } break;
        case KC_RGHT: { 
            if (!hs_dir_flag) {
                if (record->event.pressed && !rgbrec_is_started()) {
                    register_code(KC_D);
                } else {
                    unregister_code(KC_D);
                }
                return false;
            } else {
                return true;
            }
        } break;
        case HS_RWCK: { // 任务窗口
            if (record->event.pressed && !rgbrec_is_started()) {
                register_code(KC_LALT);
                register_code(KC_TAB);
            } else {
                unregister_code(KC_LALT);
                unregister_code(KC_TAB);
            }
            return true;
        } break;
        case HS_YYIN: { // 语音
            if (record->event.pressed && !rgbrec_is_started()) {
                register_code(KC_LCMD);
                register_code(KC_H);
            } else {
                unregister_code(KC_LCMD);
                unregister_code(KC_H);
            }
            return true;
        } break;
        case HS_DESK: { // 显示桌面
            if (record->event.pressed && !rgbrec_is_started()) {
                register_code(KC_LCMD);
                register_code(KC_D);
            } else {
                unregister_code(KC_LCMD);
                unregister_code(KC_D);
            }
            return true;
        } break;
        case HS_FILP: { // FN+编码器切换开关
            if (record->event.pressed) {
                confinfo.hs_mute_flag = !confinfo.hs_mute_flag;
                eeconfig_confinfo_update();
            } 
            return true;
        } break;
        case HS_MUTE: { // 静音和切换灯效模式
            if (!confinfo.hs_mute_flag) {
                if (record->event.pressed) {
                    if (rgb_matrix_get_mode() == RGB_MATRIX_CUSTOM_RGBR_PLAY) {
                        if (rgbrec_is_started()) {
                            rgbrec_read_current_channel(confinfo.record_channel);
                            rgbrec_end(confinfo.record_channel);
                            no_record_fg = false;
                        }
                        if (confinfo.record_last_mode != 0xFF)
                            rgb_matrix_mode(confinfo.record_last_mode);
                        else
                            rgb_matrix_mode(RGB_MATRIX_DEFAULT_MODE);
                        eeconfig_confinfo_update();
                        dprintf("confinfo.record_last_mode = %d\r\n", confinfo.record_last_mode);
                        start_hsv = rgb_matrix_get_hsv();
                        return false;
                    }
                    record_rgbmatrix_increase(&(confinfo.record_last_mode));
                    eeconfig_confinfo_update();
                    start_hsv = rgb_matrix_get_hsv();
                    return false;
                } 
            } else {
                if (record->event.pressed) {
                    register_code(KC_MUTE);
                } else {
                    unregister_code(KC_MUTE);
                }
            }
            return false;
        } break;
        case HS_VOLU: { // 音量加和亮度加
            if (!confinfo.hs_mute_flag) {
                if (record->event.pressed) {
                    if (rgbrec_is_started()) return false;
                    start_hsv = rgb_matrix_get_hsv();
                    rgb_matrix_sethsv(start_hsv.h,start_hsv.s,start_hsv.v + RGB_MATRIX_VAL_STEP);
                    if (rgb_matrix_get_val() >= (RGB_MATRIX_MAXIMUM_BRIGHTNESS)) {
                        ind_set_blink(ind_state_blink3);
                        start_hsv.v = RGB_MATRIX_MAXIMUM_BRIGHTNESS;
                    } else {
                        start_hsv.v = rgb_matrix_get_val() + RGB_MATRIX_VAL_STEP;
                    }
                    return false;
                } 
            } else {
                if (record->event.pressed) {
                    register_code(KC_VOLU);
                } else {
                    unregister_code(KC_VOLU);
                }
            }
            return false;
        } break;
        case HS_VOLD: { // 音量减和亮度键
            if (!confinfo.hs_mute_flag) {
                if (record->event.pressed) {
                    if (rgbrec_is_started()) return false;
                    start_hsv = rgb_matrix_get_hsv();

                    if (rgb_matrix_get_val() != 0) {
                        rgb_matrix_sethsv(start_hsv.h,start_hsv.s,start_hsv.v - RGB_MATRIX_VAL_STEP);
                    }
                    
                    if (rgb_matrix_get_val() < RGB_MATRIX_VAL_STEP) {
                        ind_set_blink(ind_state_blink3);
                        for (uint8_t i = 0; i < RGB_MATRIX_LED_COUNT; i++) {
                            rgb_matrix_set_color(i, 0, 0, 0);
                        }
                        start_hsv.v = 0;
                    } else {
                        
                        start_hsv.v = rgb_matrix_get_val() - RGB_MATRIX_VAL_STEP;
                    }
                    return false;
                } 
            } else {
                if (record->event.pressed) {
                    register_code(KC_VOLD);
                } else {
                    unregister_code(KC_VOLD);
                }
            }
            return false;
        } break;
        case RGB_SPI: {
            if (rgbrec_is_started()) return false;
            if (record->event.pressed) {
                if (rgb_matrix_get_speed() >= (55 + RGB_MATRIX_SPD_STEP * 3)) {
                    ind_set_blink(ind_state_blink3);
                }
            }
        } break;
        case RGB_SPD: {
            if (rgbrec_is_started()) return false;
            if (!record->event.pressed) {
                if (rgb_matrix_get_speed() <= (55 + RGB_MATRIX_SPD_STEP*0)) {
                    ind_set_blink(ind_state_blink3);
                    rgb_matrix_set_speed(55 + (RGB_MATRIX_SPD_STEP*0));
                    return false;
                }
            }
        } break;
        case RGB_VAI: {
            if (rgbrec_is_started()) return false;
            if (record->event.pressed) {
                if (rgb_matrix_get_val() >= (RGB_MATRIX_MAXIMUM_BRIGHTNESS - RGB_MATRIX_VAL_STEP)) {
                    ind_set_blink(ind_state_blink3);
                    start_hsv.v = RGB_MATRIX_MAXIMUM_BRIGHTNESS;
                } else {
                    start_hsv.v = rgb_matrix_get_val() + RGB_MATRIX_VAL_STEP;
                }
            }
        } break;
        case RGB_VAD: {
            if (rgbrec_is_started()) return false;
            if (record->event.pressed) {
                if (rgb_matrix_get_val() <= RGB_MATRIX_VAL_STEP) {
                    ind_set_blink(ind_state_blink3);
                    for (uint8_t i = 0; i < RGB_MATRIX_LED_COUNT; i++) {
                        rgb_matrix_set_color(i, 0, 0, 0);
                    }
                    start_hsv.v = 0;
                } else {
                    start_hsv.v = rgb_matrix_get_val() - RGB_MATRIX_VAL_STEP;
                }
            }
        } break;
        case HS_BATQ: {
            extern bool rk_bat_req_flag;
            rk_bat_req_flag = (confinfo.devs != DEVS_USB) && record->event.pressed;
            return false;
        } break;
        case EE_CLR: {
            if (record->event.pressed) {
                ee_clr_timer = timer_read32();
                press_ee_clr_flag = true;
            } else {
                ee_clr_timer = 0;
            }

            return false;
        } break;
        case TO(_BL): {
            if (record->event.pressed) {
                // if (keymap_is_mac_system()) {
                    set_single_persistent_default_layer(_BL);
                    layer_move(0);
                // }
            }

            return false;
        } break;      
        case RP_P0: {
            if (record->event.pressed) {
                confinfo.record_channel = 0;
                rgbrec_read_current_channel(confinfo.record_channel);
                rgbrec_end(confinfo.record_channel);
                eeconfig_confinfo_update();
                rgbrec_show(confinfo.record_channel);
                dprintf("confinfo.record_last_mode = %d\r\n", confinfo.record_last_mode);
            }

            return false;
        } break;
        case RP_P1: {
            if (record->event.pressed) {
                confinfo.record_channel = 1;
                rgbrec_read_current_channel(confinfo.record_channel);
                rgbrec_end(confinfo.record_channel);
                eeconfig_confinfo_update();
                rgbrec_show(confinfo.record_channel);
            }

            return false;
        } break;
        case RP_P2: {
            if (record->event.pressed) {
                confinfo.record_channel = 2;
                rgbrec_read_current_channel(confinfo.record_channel);
                rgbrec_end(confinfo.record_channel);
                eeconfig_confinfo_update();
                rgbrec_show(confinfo.record_channel);
            }

            return false;
        } break;
        case BT_TEST: {
            md_send_devctrl(0x62);
            // wait_ms(30);
            // wireless_devs_change(wireless_get_current_devs(), DEVS_BT2, true);
        } break;
        case RP_END: {
            if (record->event.pressed) {
                // 非录制灯效不允许处理
                if (rgb_matrix_get_mode() != RGB_MATRIX_CUSTOM_RGBR_PLAY) { //不是录制灯效模式就切换到录制灯效模式
                    confinfo.record_channel = 0;
                    rgbrec_read_current_channel(confinfo.record_channel);
                    rgbrec_end(confinfo.record_channel);
                    eeconfig_confinfo_update();
                    rgbrec_show(confinfo.record_channel);
                    ind_set_blink(ind_state_none);
                } else { //是在录制灯效模式中就判断是否正在录制中，如果正在录制中，结束录制保存；如果不在录制中就开始录制
                    if (!rgbrec_is_started()) { //不在录制中，就开始录制
                        rgbrec_start(confinfo.record_channel);
                        no_record_fg = false;
                        rec_time     = timer_read32();
                        rgbrec_set_close_all(HSV_BLACK);
                        ind_set_blink(ind_state_blinkon);
                    } else { //在录制中，结束录制并保存
                        rec_time = 0;
                        rgbrec_end(confinfo.record_channel);
                        ind_set_blink(ind_state_none);
                    }
                }
            }

            return false;
        } break;
        // case RL_MOD: {
        //     if (record->event.pressed) {
        //         if (confinfo.rgblight_flag == false) {
        //             mode = 10;
        //         }
        //         im_rgblight_increase();
        //         eeconfig_confinfo_update();
        //     }

        //     return false;
        // } break;
        case RL_MOD: {
            if (record->event.pressed) {
                confinfo.rgb_light_mode ++;
                if (confinfo.rgb_light_mode > 9) {
                    confinfo.rgb_light_mode = 0;
                }
                eeconfig_confinfo_update();
            }
            return false;
        } break;
        case RL_VAID: {
            if (record->event.pressed) {
                if (confinfo.rgb_light_mode == 0) {
                    confinfo.rgb_light_speed ++;
                    if (confinfo.rgb_light_speed > 4) {
                        confinfo.rgb_light_speed = 0;
                    }
                    eeconfig_confinfo_update();
                }
            }
            return false;
        } break;
        case RGB_MOD: {
            if (record->event.pressed) {
                if (rgb_matrix_get_mode() == RGB_MATRIX_CUSTOM_RGBR_PLAY) {
                    if (rgbrec_is_started()) {
                        rgbrec_read_current_channel(confinfo.record_channel);
                        rgbrec_end(confinfo.record_channel);
                        no_record_fg = false;
                    }
                    if (confinfo.record_last_mode != 0xFF)
                        rgb_matrix_mode(confinfo.record_last_mode);
                    else
                        rgb_matrix_mode(RGB_MATRIX_DEFAULT_MODE);
                    eeconfig_confinfo_update();
                    dprintf("confinfo.record_last_mode = %d\r\n", confinfo.record_last_mode);
                    start_hsv = rgb_matrix_get_hsv();
                    return false;
                }
                record_rgbmatrix_increase(&(confinfo.record_last_mode));
                eeconfig_confinfo_update();
                start_hsv = rgb_matrix_get_hsv();
            }

            return false;
        } break;
        case RGB_HUI: {
            if (record->event.pressed) {
                record_color_hsv(true);
                start_hsv = rgb_matrix_get_hsv();
            }

            return false;
        } break;
        case GU_TOGG: {
            if (record->event.pressed) {
                keymap_config.no_gui = !keymap_config.no_gui;
            }
            return false;
        } break;
        case KC_LCMD: {
            // if (keymap_is_mac_system()) {
                if (keymap_config.no_gui && !rgbrec_is_started()) {
                    // if (record->event.pressed) {
                    //     register_code16(KC_LCMD);
                    // } else {
                    //     unregister_code16(KC_LCMD);
                    // }
                    return false;
                }
            // }
            return true;
        } break;
        case KC_RCMD: {
            // if (keymap_is_mac_system()) {
                if (keymap_config.no_gui && !rgbrec_is_started()) {
                    // if (record->event.pressed) {
                    //     register_code16(KC_RCMD);
                    // } else {
                    //     unregister_code16(KC_RCMD);
                    // }
                    return false;
                }
            // }
            return true;
        } break;
        case KC_APP: {
            // if (keymap_is_mac_system()) {
                if (keymap_config.no_gui && !rgbrec_is_started()) {
                    // if (record->event.pressed) {
                    //     register_code16(KC_RCMD);
                    // } else {
                    //     unregister_code16(KC_RCMD);
                    // }
                    return false;
                }
            // }
            return true;
        } break;
        case RGB_TOG: {
            if (record->event.pressed) {
                confinfo.hs_led_tog_flag = !confinfo.hs_led_tog_flag;
                eeconfig_confinfo_update();
            }
        } return false;
        default:
            break;
    }

    return true;
}

void housekeeping_task_user(void) { // loop
    uint8_t hs_now_mode;
    static uint32_t hs_current_time;

    charging_state = readPin(HS_BAT_CABLE_PIN);

    bat_full_flag = readPin(BAT_FULL_PIN);

    if (charging_state && (bat_full_flag)) {
        hs_now_mode = MD_SND_CMD_DEVCTRL_CHARGING_DONE;
    } else if (charging_state) {
        hs_now_mode = MD_SND_CMD_DEVCTRL_CHARGING;
    } else {
        hs_now_mode = MD_SND_CMD_DEVCTRL_CHARGING_STOP;
    }

    if (!hs_current_time || timer_elapsed32(hs_current_time) > 1000) {

        hs_current_time = timer_read32();
        md_send_devctrl(hs_now_mode);
        md_send_devctrl(MD_SND_CMD_DEVCTRL_INQVOL);
    }

    // if (hs_test_ee_fg) {
    //     // 复位清除蓝牙配对信息再切换回usb模式
    //     wireless_devs_change(wireless_get_current_devs(), DEVS_BT3, true); 
    //     wireless_devs_change(wireless_get_current_devs(), DEVS_BT3, false); 
    //     wait_ms(100);
    //     wireless_devs_change(wireless_get_current_devs(), DEVS_BT2, true); 
    //     wireless_devs_change(wireless_get_current_devs(), DEVS_BT2, false); 
    //     wait_ms(100); 
    //     wireless_devs_change(wireless_get_current_devs(), DEVS_BT1, true); 
    //     wireless_devs_change(wireless_get_current_devs(), DEVS_BT1, false); 
    //     wait_ms(100);
    //     hs_test_ee_fg = false;
    //     confinfo.devs = DEVS_USB;
    // }

    extern bool jumpboot_flag;
    if (jumpboot_flag) {
        reset_keyboard();
    }
}

#ifdef RGB_MATRIX_ENABLE

#    ifdef WIRELESS_ENABLE
bool wls_rgb_indicator_reset        = false;
uint32_t wls_rgb_indicator_timer    = 0x00;
uint32_t wls_rgb_indicator_interval = 0;
uint32_t wls_rgb_indicator_times    = 0;
uint32_t wls_rgb_indicator_index    = 0;
RGB wls_rgb_indicator_rgb           = {0};

void rgb_matrix_wls_indicator_set(uint8_t index, RGB rgb, uint32_t interval, uint8_t times) {

    wls_rgb_indicator_timer = timer_read32();

    wls_rgb_indicator_index    = index;
    wls_rgb_indicator_interval = interval;
    wls_rgb_indicator_times    = times * 2;
    wls_rgb_indicator_rgb      = rgb;
}

void wireless_devs_change_kb(uint8_t old_devs, uint8_t new_devs, bool reset) {

    wls_rgb_indicator_reset = reset;

    if (confinfo.devs != wireless_get_current_devs()) {
        confinfo.devs = wireless_get_current_devs();
        if (confinfo.devs > 0 && confinfo.devs < 4) confinfo.last_btdevs = confinfo.devs;
        eeconfig_confinfo_update();
    }

    if (!reset && (wireless_get_current_devs() == new_devs &&  *md_getp_state() == MD_STATE_CONNECTED)) return;

    switch (new_devs) {
	    case DEVS_USB: {
            // if (reset) {
            //     rgb_matrix_wls_indicator_set(HS_RGB_BLINK_INDEX_USB, (RGB){HS_LBACK_COLOR_USB}, 200, 1);
            // } else {
            //     rgb_matrix_wls_indicator_set(HS_RGB_BLINK_INDEX_USB, (RGB){HS_PAIR_COLOR_USB}, 500, 1);
            // }
        } break;
        case DEVS_BT1: {
            if (reset) {
                rgb_matrix_wls_indicator_set(HS_RGB_BLINK_INDEX_BT1, (RGB){HS_LBACK_COLOR_BT1}, 200, 1);
            } else {
                rgb_matrix_wls_indicator_set(HS_RGB_BLINK_INDEX_BT1, (RGB){HS_PAIR_COLOR_BT1}, 500, 1);
            }
        } break;
        case DEVS_BT2: {
            if (reset) {
                rgb_matrix_wls_indicator_set(HS_RGB_BLINK_INDEX_BT2, (RGB){HS_LBACK_COLOR_BT2}, 200, 1);
            } else {
                rgb_matrix_wls_indicator_set(HS_RGB_BLINK_INDEX_BT2, (RGB){HS_PAIR_COLOR_BT2}, 500, 1);
            }
        } break;
        case DEVS_BT3: {
            if (reset) {
                rgb_matrix_wls_indicator_set(HS_RGB_BLINK_INDEX_BT3, (RGB){HS_LBACK_COLOR_BT3}, 200, 1);
            } else {
                rgb_matrix_wls_indicator_set(HS_RGB_BLINK_INDEX_BT3, (RGB){HS_PAIR_COLOR_BT3}, 500, 1);
            }
        } break;
        case DEVS_2G4: {
            if (reset) {
                rgb_matrix_wls_indicator_set(HS_RGB_BLINK_INDEX_2G4, (RGB){HS_LBACK_COLOR_2G4}, 200, 1);
            } else {
                rgb_matrix_wls_indicator_set(HS_RGB_BLINK_INDEX_2G4, (RGB){HS_PAIR_COLOR_2G4}, 500, 1);
            }
        } break;
        default:
            break;
    }
}

bool rgb_matrix_wls_indicator_cb(void) {

    if (*md_getp_state() != MD_STATE_CONNECTED) {
        if (!(wireless_get_current_devs() == DEVS_USB && USB_DRIVER.state == USB_ACTIVE)) wireless_devs_change_kb(wireless_get_current_devs(), wireless_get_current_devs(), wls_rgb_indicator_reset);
        return true;
    }

    // refresh led
    led_wakeup();

    return false;
}

void rgb_matrix_wls_indicator(void) {

    if (wls_rgb_indicator_timer) {

        if (timer_elapsed32(wls_rgb_indicator_timer) >= wls_rgb_indicator_interval) {
            wls_rgb_indicator_timer = timer_read32();

            if (wls_rgb_indicator_times) {
                wls_rgb_indicator_times--;
            }

            if (wls_rgb_indicator_times <= 0) {
                wls_rgb_indicator_timer = 0x00;
                if (rgb_matrix_wls_indicator_cb() != true) {
                    return;
                }
            }
        }

        if (wireless_get_current_devs() != DEVS_USB) {
            rgb_matrix_set_color_all(0,0,0);
        }

        if (wls_rgb_indicator_times % 2) {
            rgb_matrix_set_color(wls_rgb_indicator_index, wls_rgb_indicator_rgb.g, wls_rgb_indicator_rgb.r, wls_rgb_indicator_rgb.b);
        } else {
            rgb_matrix_set_color(wls_rgb_indicator_index, 0x00, 0x00, 0x00);
        }
    }
}

#    endif

#endif

bool hs_reset_settings_user(void) {
    rgb_matrix_blink_set_color(RGB_MATRIX_BLINK_INDEX_ALL, WIITE_B, WIITE_B, WIITE_B);
    rgb_matrix_blink_set_interval_times(RGB_MATRIX_BLINK_INDEX_ALL, 250, 3);
    rgb_matrix_blink_set(RGB_MATRIX_BLINK_INDEX_ALL);

    return true;
}

void rgb_matrix_start_rec(void) {

    if (rgbrec_is_started()) {
        if (!rec_time || timer_elapsed32(rec_time) > 250) {
            rec_time = timer_read32();
            rec_filp = !rec_filp;
        }
        if (rec_filp) {
            #ifdef HS_RGB_BLINK_INDEX_REC
                rgb_matrix_set_color(HS_RGB_BLINK_INDEX_REC, WIITE_B, WIITE_B, WIITE_B);
            #endif
        } else {
            #ifdef HS_RGB_BLINK_INDEX_REC
                rgb_matrix_set_color(HS_RGB_BLINK_INDEX_REC, 0x00, 0x00, 0x00);
            #endif
        }
    } else {
        rec_time = 0;
        rec_filp = false;
    }
}

void hs_blink_rgb_matrix(uint8_t r, uint8_t g, uint8_t b) {
    int i;
    for(i = 0;i < RGB_MATRIX_LED_COUNT;i++){
        rgb_matrix_set_color(i, r,g,b);
    }
}

bool rgb_matrix_indicators_advanced_kb(uint8_t led_min, uint8_t led_max) {

    if (ee_clr_timer && timer_elapsed32(ee_clr_timer) > 3000) {
        confinfo.hs_led_tog_flag = true;
        eeconfig_confinfo_update();
        ee_clr_timer = 0;
        hs_clear_flag = true;
        test_white_light_flag = false;
        hs_reset_settings();
    } 

    if (test_white_light_flag && !hs_clear_flag) {
        gpio_write_pin_high(LED_POWER_EN_PIN);
        RGB rgb_test_open = hsv_to_rgb((HSV){.h = 0, .s = 0, .v = RGB_MATRIX_VAL_STEP * 5});
        rgb_matrix_set_color_all(rgb_test_open.r, rgb_test_open.g, rgb_test_open.b);

        return false;
    }
#ifdef RGBLIGHT_ENABLE
    if (rgb_matrix_indicators_advanced_user(led_min, led_max) != true) {

        return false;
    }
#endif

    if (confinfo.hs_led_tog_flag && !test_white_light_flag) {
        gpio_write_pin_low(LED_POWER_EN_PIN);
    } else {
        gpio_write_pin_high(LED_POWER_EN_PIN);
    }

    static uint8_t last_caps_state = 0xff;
    if (last_caps_state != host_keyboard_led_state().caps_lock) {
        last_caps_state = host_keyboard_led_state().caps_lock;
        if (!host_keyboard_led_state().caps_lock) {
            rgb_matrix_set_color(HS_RGB_INDEX_CAPS, RGB_BLACK);
        }
    }
    if (host_keyboard_led_state().caps_lock && !rgbrec_is_started()){
        rgb_matrix_set_color(HS_RGB_INDEX_CAPS,0xff,0xff,0xff);
    }

    static uint8_t last_scr_state = 0xff;
    if (last_scr_state != host_keyboard_led_state().scroll_lock) {
        last_scr_state = host_keyboard_led_state().scroll_lock;
        if (!host_keyboard_led_state().scroll_lock) {
            rgb_matrix_set_color(HS_RGB_INDEX_SCR, RGB_BLACK);
        }
    }
    if (host_keyboard_led_state().scroll_lock && !rgbrec_is_started()){
        rgb_matrix_set_color(HS_RGB_INDEX_SCR,0xff,0xff,0xff);
    }

    // if (!rgbrec_is_started() && keymap_config.no_gui  && !rgbrec_is_started()){
    //     rgb_matrix_set_color(HS_RGB_INDEX_WIN_LOCK,0xff,0xff,0xff);
    // }
        
    // if (!keymap_is_base_layer()  && !rgbrec_is_started()) { // 进到F层，亮指示灯
    //     rgb_matrix_set_color(HS_RGB_BLINK_INDEX_F_LAYER,0xff,0xff,0xff);
    // } 

    hs_rgblight_increase(confinfo.rgb_light_mode);

#ifdef WIRELESS_ENABLE 
    rgb_matrix_wls_indicator();
#endif

    query();
    rgb_matrix_blink_task(led_min, led_max);
    return true;
}

void hs_reset_settings(void) {
    eeconfig_init();
    eeconfig_update_rgb_matrix_default();
#ifdef RGBLIGHT_ENABLE
    extern void rgblight_init(void);
    is_rgblight_initialized = false;
    rgblight_init();
    eeconfig_update_rgblight_default();
    rgblight_enable();
#endif

    keymap_config.raw = eeconfig_read_keymap();
    
    if (hs_reset_settings_user() != true) {
        return;
    }

    keyboard_post_init_kb();
}


void lpwr_wakeup_hook(void) {
    if (wireless_get_current_devs() == DEVS_USB && USB_DRIVER.state == USB_STOP) {
        usb_power_connect();
        restart_usb_driver(&USBD1);
        wireless_devs_change(!DEVS_USB, DEVS_USB, false);
    }
    hs_mode_scan(false, confinfo.devs, confinfo.last_btdevs);
#ifdef LED_POWER_EN_PIN
    gpio_write_pin_high(LED_POWER_EN_PIN);
#endif

#ifdef HS_LED_BOOSTING_PIN
    gpio_write_pin_high(HS_LED_BOOSTING_PIN);
#endif
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

void rgb_light_color_set(uint8_t r, uint8_t g, uint8_t b) {
    const uint8_t led_list[] = {
        91, 92, 93, 94, 95, 96
    };
    const uint8_t led_count = sizeof(led_list) / sizeof(led_list[0]);
    for (uint8_t i = 0; i < led_count; i++) {
        rgb_matrix_set_color(led_list[i], r, g, b);
    }
}

static uint8_t neon_color_index = 0;
void hs_rgblight_increase(uint8_t rgblight_mode) {

    switch (rgblight_mode)
    {
        // case 0: {  
        //     static uint16_t rainbow_counter = 0;
        //     static uint32_t last_update = 0;
        //     uint32_t interval = 20;

        //     switch (confinfo.rgb_light_speed) { // confinfo.rgb_light_speed越大，速度越快
        //         case 0: interval = 60; break;
        //         case 1: interval = 35; break;
        //         case 2: interval = 20; break;
        //         case 3: interval = 10; break;
        //         case 4: interval = 3; break;
        //         default:
        //             break;
        //     }

        //     if (timer_elapsed32(last_update) >= interval) {
        //         last_update = timer_read32();
        //         rainbow_counter += 3;
        //     }

        //     for (uint8_t i = 0; i < RGBLED_NUM; i++) {
        //         uint8_t hue = (rainbow_counter + (RGBLED_NUM - 1 - i) * 8) % 255;
        //         HSV hsv = { .h = hue, .s = 255, .v = (RGBLIGHT_LIMIT_VAL) };
        //         RGB rgb = hsv_to_rgb(hsv);
        //         rgb_matrix_set_color(i+91, rgb.r, rgb.g, rgb.b);
        //     }
        // } break;

        case 0: {
            static uint8_t hue_offset = 0; // 控制整体流光偏移
            static uint32_t hs_test_time = 0;  
            uint32_t now = timer_read32();

            uint32_t delay;
            uint8_t hue_step;

            switch (confinfo.rgb_light_speed) { // confinfo.rgb_light_speed数值越大，速度越快
                case 4: delay = 15;  hue_step = 10; break;
                case 3: delay = 35; hue_step = 8;  break;
                case 2: delay = 70; hue_step = 8;  break;
                case 1: delay = 120; hue_step = 8;  break;
                case 0: delay = 200; hue_step = 10;  break;
                default: delay = 70; hue_step = 8; break;
            }

            // **只在时间满足时，更新 hs_test_time**
            bool update_light = false;
            if (timer_elapsed32(hs_test_time) > delay) {
                hs_test_time = now;
                update_light = true;
            }

            for (uint8_t i = 0; i < RGBLED_NUM; i++) {
                uint8_t hue = (hue_offset + (5 - i) * 36) % 255;  // **修改方向，让流光从左向右**
                HSV hsv = { .h = hue, .s = 255, .v = RGBLIGHT_LIMIT_VAL };
                RGB rgb = hsv_to_rgb(hsv);
                rgb_matrix_set_color(91 + i, rgb.r, rgb.g, rgb.b);
            }

            if (update_light) {
                hue_offset = (hue_offset + hue_step) % 255;
            }
        } break;

        case 1: // 红
            neon_color_index = 0;
            rgb_light_color_set(RGBLIGHT_LIMIT_VAL, 0x00, 0x00); break;

        case 2: // 绿
            rgb_light_color_set(0, RGBLIGHT_LIMIT_VAL, 0); break;

        case 3: // 蓝
            rgb_light_color_set(0, 0, RGBLIGHT_LIMIT_VAL); break;

        case 4: // 黄
            rgb_light_color_set(RGBLIGHT_LIMIT_VAL - 30, RGBLIGHT_LIMIT_VAL-30, 0); break;

        case 5: {// 粉
            RGB rgb_pink_open = hsv_to_rgb((HSV){.h = 234, .s = 255, .v = RGBLIGHT_LIMIT_VAL});
            rgb_light_color_set(rgb_pink_open.r, rgb_pink_open.g, rgb_pink_open.b); 
        } break;  

        case 6: // 青
            rgb_light_color_set(0, RGBLIGHT_LIMIT_VAL - 30, RGBLIGHT_LIMIT_VAL - 30); break;
        
        case 7: // 白
            rgb_light_color_set(RGBLIGHT_LIMIT_VAL - 50, RGBLIGHT_LIMIT_VAL - 50, RGBLIGHT_LIMIT_VAL - 50); break;

        case 8: { // 霓虹效果
            static const RGB neon_colors[] = {
                {RGBLIGHT_LIMIT_VAL, 0, 0},         // 红
                {0, RGBLIGHT_LIMIT_VAL, 0},         // 绿
                {0, 0, RGBLIGHT_LIMIT_VAL},         // 蓝
                {RGBLIGHT_LIMIT_VAL-10, RGBLIGHT_LIMIT_VAL-10, 0},   // 黄
                {RGBLIGHT_LIMIT_VAL-10, 20, RGBLIGHT_LIMIT_VAL-10},   // 粉色
                {0, RGBLIGHT_LIMIT_VAL-10, RGBLIGHT_LIMIT_VAL-10},   // 青
                {RGBLIGHT_LIMIT_VAL-10, RGBLIGHT_LIMIT_VAL-10, RGBLIGHT_LIMIT_VAL-10}  // 白
            };

            static uint8_t neon_brightness = 0;
            static bool neon_increasing = true;

            static uint32_t last_update = 0;
            uint32_t interval = 15;  // 每 20ms 调整一次亮度，数值越大越慢

            if (timer_elapsed32(last_update) >= interval) {
                last_update = timer_read32();

                uint8_t max_val = RGBLIGHT_LIMIT_VAL - 30;
                if (neon_increasing) {
                    neon_brightness++;
                    if (neon_brightness >= max_val) {
                        neon_brightness = max_val;
                        neon_increasing = false;
                    }
                } else {
                    if (neon_brightness > 0) {
                        neon_brightness--;
                    } else {
                        neon_increasing = true;
                        neon_color_index = (neon_color_index + 1) % (sizeof(neon_colors) / sizeof(neon_colors[0]));
                    }
                }
            }

            RGB current_color = neon_colors[neon_color_index];
            rgb_light_color_set(
                    (current_color.g * neon_brightness) / (RGBLIGHT_LIMIT_VAL - 0),
                    (current_color.r * neon_brightness) / (RGBLIGHT_LIMIT_VAL - 0),
                    (current_color.b * neon_brightness) / (RGBLIGHT_LIMIT_VAL - 0));
        } break;

        case 9: // 熄灭
            rgb_light_color_set(0, 0, 0); break;

        default:
            confinfo.rgb_light_mode = 0;
            break;
    }
}
