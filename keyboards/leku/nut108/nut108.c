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

enum layers {
    _BL = 0,
    _FL,
    _MBL,
    _MFL,
    _DEFA, 
};

enum hs_layer_choose {
    HS_WIN_BASE  = 0,
    HS_MAC_BASE  = 2,
};

typedef union {
    uint32_t raw;
    struct {
        uint8_t devs : 3;
        uint8_t hs_led_tog_flag : 1;
        uint8_t last_btdevs : 3;
        uint8_t DEB_flag: 1;

        uint8_t sleep_timeout : 3;
        uint8_t hs_music_cut : 1;
        uint8_t rgb_light_val : 3;
        uint8_t hs_rgb_mode_choose : 1;

        uint8_t record_last_mode;

        uint8_t rgb_light_mode : 4;
        uint8_t record_channel : 2;
        uint8_t hs_num_flag : 1;
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

void hs_reset_settings(void);
void matrix_init_user(void);
bool hs_side_light_switch(void);

#define keymap_is_mac_system() ((get_highest_layer(default_layer_state) == _MBL))

uint32_t post_init_timer      = 0x00;
bool charging_state           = false;
bool bat_full_flag            = false;
static uint32_t ee_clr_timer  = 0;
uint8_t mac_status            = false;
bool hs_rgb_blink_flag        = false;
bool lower_sleep              = false;
uint8_t hs_layer_cnt          = 0;
uint16_t hs_deb               = DEBOUNCE;
static bool hs_charing_filp   = false;
bool no_record_fg;
bool im_test_rate_flag;
uint32_t rec_time;
bool rec_filp;
HSV start_hsv;

bool music_cut_flag                         = true;
bool rgblight_cut_flag                      = false;
static bool pulse_active                    = false;
static uint32_t pulse_start_time            = 0;
static bool side_light_pulse_active         = false;
static uint32_t side_light_pulse_start_time = 0;
static bool side_light_val_active           = false;
static uint32_t side_light_val_start_time   = 0;
bool hs_mucis_calibration                   = true;
uint32_t mucis_calibration_time             = 0;

void bat_indicators_hook(uint8_t index);
void ind_indicators_hook(uint8_t index);
void nkr_indicators_hook(uint8_t index);
blink_rgb_t blink_rgbs[RGB_MATRIX_BLINK_COUNT] = {
    {.index = HS_RGB_BLINK_INDEX_VAI, .interval = 250, .times = 1, .color = {.r = 0xFF, .g = 0xFF, .b = 0xFF}, .blink_cb = NULL},
    {.index = HS_RGB_BLINK_INDEX_VAD, .interval = 250, .times = 1, .color = {.r = 0xFF, .g = 0xFF, .b = 0xFF}, .blink_cb = NULL},
    {.index = HS_RGB_BLINK_INDEX_SPI, .interval = 250, .times = 1, .color = {.r = 0xFF, .g = 0xFF, .b = 0xFF}, .blink_cb = NULL},
    {.index = HS_RGB_BLINK_INDEX_SPD, .interval = 250, .times = 1, .color = {.r = 0xFF, .g = 0xFF, .b = 0xFF}, .blink_cb = NULL},
    {.index = HS_RGB_BLINK_INDEX_SYS_TEM, .interval = 250, .times = 1, .color = {.r = 0xFF, .g = 0xFF, .b = 0xFF}, .blink_cb = NULL},
    {.index = HS_RGB_BLINK_INDEX_DEB, .interval = 250, .times = 1, .color = {.r = 0xFF, .g = 0xFF, .b = 0xFF}, .blink_cb = NULL},
    {.index = HS_RGB_BLINK_INDEX_MUSIC, .interval = 250, .times = 1, .color = {.r = 0xFF, .g = 0xFF, .b = 0xFF}, .blink_cb = NULL},
    {.index = RGB_MATRIX_BLINK_INDEX_BAT, .interval = 250, .times = 3, .color = {.r = 0x00, .g = 0x00, .b = 0x00}, .blink_cb = bat_indicators_hook},
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

    if (charging_state && ( (bat_full_flag))) { // power full
        hs_rgb_blink_flag = false;
        battery_process_time = 0;
        bat_flip = true;
        rgb_matrix_blink_set_color(index, 0x00, 0xFF, 0x00);
        rgb_matrix_blink_set_interval_times(index, 10, 0xFF); // led off
    } else if (charging_state) { // charing...
        hs_rgb_blink_flag = false;
        battery_process_time = 0;
        bat_flip = true;
        rgb_matrix_blink_set_color(index, 0xFF, 0x00, 0x00);
        rgb_matrix_blink_set_interval_times(index, 10, 0xFF); 
    } else if (*md_getp_bat() <= BATTERY_CAPACITY_LOW) { // low power
        hs_rgb_blink_flag = false;
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
                clear_keyboard();
                if (battery_process_time && timer_elapsed32(battery_process_time) > 26000) {
                    clear_keyboard();
                    battery_process_time = 0;
                    lower_sleep          = true;
                    md_send_devctrl(MD_SND_CMD_DEVCTRL_USB);
                    lpwr_set_timeout_manual(true);
                } 
            }
        }
    } else {
        hs_rgb_blink_flag = true;
        battery_process_time = 0;
        bat_flip = true;
        rgb_matrix_blink_set_color(index, 0x00, 0x00, 0x00);
        rgb_matrix_blink_set_interval_times(index, 300, 0xFF); 
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
            if (ind_status != ind_state_none || !confinfo.hs_led_tog_flag) {
                return false;
            }

            if (rgb_matrix_get_mode() == 29 || rgb_matrix_get_mode() == 32) {
                if (blink_rgb->flip) {
                    hs_charing_filp = true;
                } else {
                    hs_charing_filp = false;
                }
                return false;
            }

            if (blink_rgb->flip) {
                if (!blink_rgb->color.r && blink_rgb->color.g && !blink_rgb->color.b) {
                    rgb_matrix_set_color(HS_MATRIX_BLINK_INDEX_BAT, 0x00, 0xff, 0x00);
                } else if (blink_rgb->color.r && !blink_rgb->color.g && !blink_rgb->color.b) {
                    rgb_matrix_set_color(HS_MATRIX_BLINK_INDEX_BAT, 0xff, 0x00, 0x00);
                }
            }else {
                
                rgb_matrix_set_color(HS_MATRIX_BLINK_INDEX_BAT, 0x00, 0x00, 0x00);
            }
            return false;
        } break;

        case RGB_MATRIX_BLINK_INDEX_IND: {
            
            switch(ind_status) {
                case ind_state_none: {
                    return false; 
                } break;
                case ind_state_blinkon:
                case ind_state_blinkoff: {
                    if (!no_record_fg) return false;

                    if (blink_rgb->flip) {  
                        rgb_matrix_set_color(HS_RGB_BLINK_INDEX_REC, blink_rgb->color.r, blink_rgb->color.g, blink_rgb->color.b);
                    } 
                } break;
                case ind_state_exec: {
                    if (rgbrec_is_started()) {
                        return false;
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

bool hs_get_rgb_mode_status(void) {
    return confinfo.hs_rgb_mode_choose;
}


void eeconfig_confinfo_default(void) {

    confinfo.hs_led_tog_flag   = true;
    confinfo.record_channel    = 0;
    confinfo.record_last_mode  = 0xff;
    confinfo.last_btdevs       = 1;
    confinfo.sleep_timeout     = 7;
    confinfo.rgb_light_mode    = 0;
    confinfo.rgb_light_val     = 4;
    confinfo.DEB_flag          = false;
    confinfo.hs_rgb_mode_choose = true;
    confinfo.hs_num_flag       = false;

    rgblight_cut_flag          = false;
    music_cut_flag             = true;
    confinfo.hs_music_cut      = false;

    confinfo.devs = wireless_get_current_devs();
    eeconfig_init_user_datablock();
    eeconfig_confinfo_update();

    switch (hs_layer_cnt) {
        case HS_WIN_BASE: set_single_persistent_default_layer(_BL); break;
        case HS_MAC_BASE: set_single_persistent_default_layer(_MBL); break;
    }
    layer_move(0); 

#if defined(NKRO_ENABLE) && defined(FORCE_NKRO)
    keymap_config.nkro = 1;
    eeconfig_update_keymap(keymap_config.raw);
#endif

}

void eeconfig_confinfo_init(void) {

    confinfo.raw = eeconfig_confinfo_read();
    if (!confinfo.raw) {
        eeconfig_confinfo_default();
    }
}

void keyboard_pre_init_user(void) {
#ifdef GPIO_UART_ENABLE 
    wait_ms(3000);
    iprint_init();
#endif
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

#ifdef HS_LED_EN_PIN
    gpio_set_pin_output(HS_LED_EN_PIN);
    gpio_write_pin_high(HS_LED_EN_PIN);
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

#ifdef HS_ENCODERS_PAD_A
    setPinInputHigh(HS_ENCODERS_PAD_A);
#endif

#ifdef HS_ENCODERS_PAD_B
    setPinInputHigh(HS_ENCODERS_PAD_B);
#endif

#ifdef HS_LED_BOOSTING_PIN
    gpio_set_pin_output(HS_LED_BOOSTING_PIN);
    gpio_write_pin_high(HS_LED_BOOSTING_PIN);
#endif

#ifdef HS_SIDE_LIGHT_EN_PIN
    gpio_set_pin_output(HS_SIDE_LIGHT_EN_PIN);
    gpio_write_pin_high(HS_SIDE_LIGHT_EN_PIN);
#endif

#ifdef HS_SIDE_LIGHT_SLEEP_PIN
    gpio_set_pin_output(HS_SIDE_LIGHT_SLEEP_PIN);
    gpio_write_pin_high(HS_SIDE_LIGHT_SLEEP_PIN);
#endif

#ifdef WIRELESS_ENABLE
    wireless_init();
#if !(defined(MM_BT_DEF_PIN) && defined(MM_2G4_DEF_PIN))
    wireless_devs_change(!DEVS_USB, confinfo.devs, false);
#endif
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
        md_send_devctrl(0xFF);
    } else {
        md_send_devctrl(0x80+HS_WIRESS_SLEEP_TIME); 
    }

    keyboard_post_init_user();
    rgbrec_init(confinfo.record_channel);

    gpio_set_pin_output(HS_MUSIC_MODE_PIN);
    gpio_write_pin_low(HS_MUSIC_MODE_PIN);

    gpio_set_pin_output(HS_SIDE_LIGHT_MODE_PIN);
    gpio_write_pin_low(HS_SIDE_LIGHT_MODE_PIN);

    gpio_set_pin_output(HS_MUSIC_CUT_PIN);
    if (confinfo.hs_music_cut) {
        gpio_write_pin_high(HS_MUSIC_CUT_PIN);
        music_cut_flag = false;
    } else {
        gpio_write_pin_low(HS_MUSIC_CUT_PIN);
        rgblight_cut_flag = false;
    }

    if(confinfo.DEB_flag){
        hs_deb = DEBOUNCE; 
    }else{
        hs_deb = 1; 
    }

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

#    ifdef HS_LED_EN_PIN
    gpio_write_pin_low(HS_LED_EN_PIN);
#    endif

#    ifdef HS_SIDE_LIGHT_EN_PIN
    gpio_write_pin_low(HS_SIDE_LIGHT_EN_PIN);
#    endif

#    ifdef HS_SIDE_LIGHT_SLEEP_PIN
    gpio_write_pin_low(HS_SIDE_LIGHT_SLEEP_PIN);
#    endif

    suspend_power_down_user();
}

void suspend_wakeup_init_kb(void) {

#    ifdef LED_POWER_EN_PIN
    gpio_write_pin_high(LED_POWER_EN_PIN);
#    endif

#    ifdef HS_LED_EN_PIN
    gpio_write_pin_high(HS_LED_EN_PIN);
#    endif

#ifdef HS_LED_BOOSTING_PIN
    gpio_write_pin_high(HS_LED_BOOSTING_PIN);
#endif

#ifdef HS_SIDE_LIGHT_EN_PIN
    gpio_write_pin_high(HS_SIDE_LIGHT_EN_PIN);
#endif

#ifdef HS_SIDE_LIGHT_SLEEP_PIN
    gpio_write_pin_high(HS_SIDE_LIGHT_SLEEP_PIN);
#endif

    gpio_set_pin_output(HS_MUSIC_MODE_PIN);
    gpio_write_pin_low(HS_MUSIC_MODE_PIN);

    gpio_set_pin_output(HS_SIDE_LIGHT_MODE_PIN);
    gpio_write_pin_low(HS_SIDE_LIGHT_MODE_PIN);

    gpio_set_pin_output(HS_MUSIC_CUT_PIN);
    if (confinfo.hs_music_cut) {
        gpio_write_pin_high(HS_MUSIC_CUT_PIN);
        music_cut_flag = false;
    } else {
        gpio_write_pin_low(HS_MUSIC_CUT_PIN);
        rgblight_cut_flag = false;
    }

    wireless_devs_change(wireless_get_current_devs(), wireless_get_current_devs(), false);
    suspend_wakeup_init_user();
    hs_rgb_blink_set_timer(timer_read32());
}

bool lpwr_is_allow_timeout_hook(void) {

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
                state = true;
                hs_rgb_blink_set_timer(timer_read32());
            }

        } break;
        case KC_BT2: {
            uint8_t mode = confinfo.devs;
            hs_modeio_detection(true, &mode, confinfo.last_btdevs);
            if ((mode == hs_bt) || (mode == hs_wireless) || (mode == hs_none)) {
                wireless_devs_change(wireless_get_current_devs(), DEVS_BT2, true);
                state = true;
                hs_rgb_blink_set_timer(timer_read32());
            }
        } break;
        case KC_BT3: {
            uint8_t mode = confinfo.devs;
            hs_modeio_detection(true, &mode, confinfo.last_btdevs);
            if ((mode == hs_bt) || (mode == hs_wireless) || (mode == hs_none)) {
                wireless_devs_change(wireless_get_current_devs(), DEVS_BT3, true);
                state = true;
                hs_rgb_blink_set_timer(timer_read32());
            }
        } break;
        case KC_2G4: {
            uint8_t mode = confinfo.devs;
            hs_modeio_detection(true, &mode, confinfo.last_btdevs);
            if ((mode == hs_2g4) || (mode == hs_wireless) || (mode == hs_none)) {
                wireless_devs_change(wireless_get_current_devs(), DEVS_2G4, true);
                state = true;
                hs_rgb_blink_set_timer(timer_read32());
            }
        } break;
        case SYS_CUT: {
            mac_status = keymap_is_mac_system();    
            if (mac_status) {  
                set_single_persistent_default_layer(_BL);               
                layer_move(0); 
                rgb_matrix_blink_set_color(HS_RGB_BLINK_INDEX_SYS_TEM,0x00,0x00,0xff);
                md_send_devctrl_bat(0x81);
            } else { 
                set_single_persistent_default_layer(_MBL);               
                layer_move(0);
                rgb_matrix_blink_set_color(HS_RGB_BLINK_INDEX_SYS_TEM,0xff,0xff,0xff);
                md_send_devctrl_bat(0x80);
            } 
            rgb_matrix_blink_set_remain_time(HS_RGB_BLINK_INDEX_SYS_TEM, 0); 
            rgb_matrix_blink_set_interval_times(HS_RGB_BLINK_INDEX_SYS_TEM, 250, 3);
            rgb_matrix_blink_set(HS_RGB_BLINK_INDEX_SYS_TEM);
        } break;
        case DEB_TOG: {
            confinfo.DEB_flag = !confinfo.DEB_flag;
            if(confinfo.DEB_flag) {
                hs_deb = 1; 
                rgb_matrix_blink_set_color(HS_RGB_BLINK_INDEX_DEB,0xff,0x00,0x00);
                rgb_matrix_blink_set_remain_time(HS_RGB_BLINK_INDEX_DEB, 0); 
                rgb_matrix_blink_set_interval_times(HS_RGB_BLINK_INDEX_DEB, 250, 3);
                rgb_matrix_blink_set(HS_RGB_BLINK_INDEX_DEB);
            } else {
                hs_deb = DEBOUNCE; 
                rgb_matrix_blink_set_color(HS_RGB_BLINK_INDEX_DEB,0xff,0xff,0xff);
                rgb_matrix_blink_set_remain_time(HS_RGB_BLINK_INDEX_DEB, 0); 
                rgb_matrix_blink_set_interval_times(HS_RGB_BLINK_INDEX_DEB, 250, 3);
                rgb_matrix_blink_set(HS_RGB_BLINK_INDEX_DEB);
            }
            eeconfig_confinfo_update();
        } break;
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
        case SYS_CUT:{
            WLS_KEYCODE_EXEC(wireless_get_current_devs());
        }break;
        case DEB_TOG:{
            WLS_KEYCODE_EXEC(wireless_get_current_devs());
        }break;

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
        } break;
        default:
            return true;
    }

    return false;
}
#endif

bool process_record_user(uint16_t keycode, keyrecord_t *record) {

    if (*md_getp_state() == MD_STATE_CONNECTED) {
        hs_rgb_blink_set_timer(timer_read32());
    }

    switch (keycode) {
        case MO(_FL):
        case MO(_MFL): {
            if (!record->event.pressed && rgbrec_is_started()) {
                if (no_record_fg == true) {
                    no_record_fg = false;
                    rgbrec_register_record(keycode, record);
                }
                no_record_fg = true;
            }
            break;
        }
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

    if (rgbrec_is_started() && (!(keycode == RP_P0 || keycode == RP_P1 || keycode == RP_P2 || keycode == RP_END || keycode == RGB_MOD || keycode == MO(_FL) || keycode == MO(_MFL)))) {

        return false;
    }

    return true;
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


    if ((confinfo.devs != DEVS_USB && *md_getp_state() == MD_STATE_DISCONNECTED)){
        if(IS_BASIC_KEYCODE(keycode) || IS_MODIFIER_KEYCODE(keycode)){
            return false;
        }
    }

    switch (keycode) {
        case NUM_TOG: {
            if (record->event.pressed) {
                confinfo.hs_num_flag = !confinfo.hs_num_flag;
                eeconfig_confinfo_update();
            }
            return false;
        } break;
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
            } else{
                unregister_code16(KC_LCTL);
                unregister_code16(KC_X);
            }
            return false;
        } break;
        case KC_COPY: {
            if(record->event.pressed){
                register_code16(KC_LCTL);
                register_code16(KC_C);
            } else{
                unregister_code16(KC_LCTL);
                unregister_code16(KC_C);
            }
            return false;
        } break;
        case KC_PASTE: { 
            if(record->event.pressed){
                register_code16(KC_LCTL);
                register_code16(KC_V);
            } else{
                unregister_code16(KC_LCTL);
                unregister_code16(KC_V);
            }
            return false;
        } break;
        case KC_FIND: { 
            if(record->event.pressed){
                register_code16(KC_LCTL);
                register_code16(KC_F);
            } else{
                unregister_code16(KC_LCTL);
                unregister_code16(KC_F);
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
        case HS_DESK: { 
            if (record->event.pressed && !rgbrec_is_started()) {
                register_code(KC_LCMD);
                register_code(KC_D);
            } else {
                unregister_code(KC_LCMD);
                unregister_code(KC_D);
            }
            return true;
        } break;
        case LED_TOG: {
            if (record->event.pressed) {
                confinfo.hs_led_tog_flag = !confinfo.hs_led_tog_flag;
                eeconfig_confinfo_update();
            } 
            return false;
        } break;
        case RGB_SPI: {
            if (rgbrec_is_started()) return false;
            if (record->event.pressed) {
                if (rgb_matrix_get_speed() >= (55 + RGB_MATRIX_SPD_STEP * 4)) {
                    rgb_matrix_blink_set_remain_time(HS_RGB_BLINK_INDEX_SPD, 0); 
                    rgb_matrix_blink_set_interval_times(HS_RGB_BLINK_INDEX_SPI, 250, 3);
                    rgb_matrix_blink_set(HS_RGB_BLINK_INDEX_SPI);
                }
            }
        } break;
        case RGB_SPD: {
            if (rgbrec_is_started()) return false;
            if (!record->event.pressed) {
                if (rgb_matrix_get_speed() <= (55 + RGB_MATRIX_SPD_STEP*1)) {
                    rgb_matrix_blink_set_remain_time(HS_RGB_BLINK_INDEX_SPI, 0); 
                    rgb_matrix_blink_set_interval_times(HS_RGB_BLINK_INDEX_SPD, 250, 3);
                    rgb_matrix_blink_set(HS_RGB_BLINK_INDEX_SPD);
                    rgb_matrix_set_speed(55 + (RGB_MATRIX_SPD_STEP*0));
                    return false;
                }
            }
        } break;
        case RGB_VAI: {
            if (rgbrec_is_started()) return false;
            if (record->event.pressed) {
                if (rgb_matrix_get_val() >= (RGB_MATRIX_MAXIMUM_BRIGHTNESS - RGB_MATRIX_VAL_STEP)) {
                    rgb_matrix_blink_set_remain_time(HS_RGB_BLINK_INDEX_VAD, 0); 
                    rgb_matrix_blink_set_interval_times(HS_RGB_BLINK_INDEX_VAI, 250, 3);
                    rgb_matrix_blink_set(HS_RGB_BLINK_INDEX_VAI);
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
                    rgb_matrix_blink_set_remain_time(HS_RGB_BLINK_INDEX_VAI, 0); 
                    rgb_matrix_blink_set_interval_times(HS_RGB_BLINK_INDEX_VAD, 250, 3);
                    rgb_matrix_blink_set(HS_RGB_BLINK_INDEX_VAD);

                    for (uint8_t i = 0; i < RGB_MATRIX_LED_COUNT  - RGBLED_NUM; i++) {
                        if (host_keyboard_led_state().caps_lock) {
                            if (i == HS_RGB_INDEX_CAPS) continue;
                        }
                        if (host_keyboard_led_state().num_lock) {
                            if (i == HS_RGB_INDEX_NUM) continue;
                        }
                        if (host_keyboard_led_state().scroll_lock) {
                            if (i == HS_RGB_INDEX_SCR) continue;
                        }
                        if (i == HS_RGB_INDEX_WIN_LOCK) continue;
                        if (i == HS_MATRIX_BLINK_INDEX_BAT) continue;
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
            } else {
                ee_clr_timer = 0;
            }

            return false;
        } break;
        case TO(_BL): {
            if (record->event.pressed) {
                if (keymap_is_mac_system()) {
                    set_single_persistent_default_layer(_BL);
                    layer_move(0);
                }
            }

            return false;
        } break;
        case TO(_MBL): {
            if (record->event.pressed) {
                if (!keymap_is_mac_system()) {
                    set_single_persistent_default_layer(_MBL);
                    layer_move(0);
                }
            }

            return false;
        } break;       
        case RP_P0: {
            if (record->event.pressed) {
                rgbrec_end(confinfo.record_channel,false);
                confinfo.record_channel = 0;
                rgbrec_read_current_channel(confinfo.record_channel);
                eeconfig_confinfo_update();
                rgbrec_show(confinfo.record_channel);
                ind_set_blink(ind_state_none);
                dprintf("confinfo.record_last_mode = %d\r\n", confinfo.record_last_mode);
            }

            return false;
        } break;
        case RP_P1: {
            if (record->event.pressed) {
                rgbrec_end(confinfo.record_channel,false);
                confinfo.record_channel = 1;
                rgbrec_read_current_channel(confinfo.record_channel);
                eeconfig_confinfo_update();
                rgbrec_show(confinfo.record_channel);
                ind_set_blink(ind_state_none);
            }

            return false;
        } break;
        case RP_P2: {
            if (record->event.pressed) {
                rgbrec_end(confinfo.record_channel,false);
                confinfo.record_channel = 2;
                rgbrec_read_current_channel(confinfo.record_channel);
                eeconfig_confinfo_update();
                rgbrec_show(confinfo.record_channel);
                ind_set_blink(ind_state_none);
            }

            return false;
        } break;
        case RP_END: {
            if (record->event.pressed) {
                if (rgb_matrix_get_mode() != RGB_MATRIX_CUSTOM_RGBR_PLAY) {

                    return false;
                }
                if (!rgbrec_is_started()) {
                    rgbrec_start(confinfo.record_channel);
                    no_record_fg = false;
                    rec_time     = timer_read32();
                    rgbrec_set_close_all(HSV_BLACK);
                    ind_set_blink(ind_state_blinkon);
                } else {
                    rec_time = 0;
                    rgbrec_end(confinfo.record_channel,true);
                    ind_set_blink(ind_state_none);
                }
                eeconfig_confinfo_update();
                dprintf("confinfo.record_last_mode = %d\r\n", confinfo.record_last_mode);
            }

            return false;
        } break;
        case RL1_MOD:{ 
            static bool hs_once_fg = false;
            if (record->event.pressed && !hs_once_fg) { 
                gpio_write_pin_high(HS_SIDE_LIGHT_MODE_PIN); 
                side_light_pulse_start_time = timer_read32();
                side_light_pulse_active     = true;  
                hs_once_fg = true;            
            } else {
                hs_once_fg = false;
            }
            return false;
        }break;
        case RL1_VAI:{ 
            static bool hs_once_fg = false;
            if (record->event.pressed && !hs_once_fg) { 
                gpio_write_pin_high(HS_SIDE_LIGHT_MODE_PIN); 
                side_light_val_start_time = timer_read32();
                side_light_val_active     = true;  
                hs_once_fg = true;            
            } else {
                hs_once_fg = false;
            }
            return false;
        }break;
        case MUS_CUT:{ 
            static bool hs_once_fg = false;
            if (record->event.pressed && !hs_once_fg) { 
                confinfo.hs_music_cut = true;
                rgblight_cut_flag     = true;
                gpio_write_pin_high(HS_MUSIC_CUT_PIN);

                if (music_cut_flag) { 
                    music_cut_flag         = false;
                } else {
                    gpio_write_pin_high(HS_MUSIC_MODE_PIN); 
                    pulse_start_time = timer_read32();
                    pulse_active     = true;
                }
                eeconfig_confinfo_update();  
                hs_once_fg = true;            
            } else {
                hs_once_fg = false;
            }
            return false;
        }break;
        case MUS_STA: { 
            static bool hs_once_flag = false;
            if (record->event.pressed && !hs_once_flag) {
                gpio_write_pin_high(HS_MUSIC_MODE_PIN); 
                hs_mucis_calibration   = true;
                mucis_calibration_time = timer_read32();
                rgb_matrix_blink_set_color(HS_RGB_BLINK_INDEX_MUSIC,0xff,0xff,0xff);
                rgb_matrix_blink_set_remain_time(HS_RGB_BLINK_INDEX_MUSIC, 0); 
                rgb_matrix_blink_set_interval_times(HS_RGB_BLINK_INDEX_MUSIC, 250, 3);
                rgb_matrix_blink_set(HS_RGB_BLINK_INDEX_MUSIC);
                hs_once_flag = true;
            } else {
                hs_once_flag = false;
            }
            return false;
        } break;
        case RL_MOD: {
            if (!hs_side_light_switch()) return false;
            if (record->event.pressed) {
                confinfo.hs_music_cut = false;
                music_cut_flag        = true;
                gpio_write_pin_low(HS_MUSIC_CUT_PIN);

                if (rgblight_cut_flag) {
                    rgblight_cut_flag = false;
                } else {
                    confinfo.rgb_light_mode ++;
                    if (confinfo.rgb_light_mode > HS_SIDE_LIGHT_MAX_MODE) {
                        confinfo.rgb_light_mode = 0;
                    }
                }
                eeconfig_confinfo_update();
            }
            return false;
        } break;
        case RL_VAI: {
            if (!hs_side_light_switch()) return false;
            if (confinfo.hs_music_cut || confinfo.rgb_light_mode == HS_SIDE_LIGHT_MAX_MODE) {
                return false;
            }
            if (record->event.pressed) {
                if (confinfo.rgb_light_val < 3) {
                    confinfo.rgb_light_val ++;
                }
                eeconfig_confinfo_update();
            }
            return false;
        } break;
        case RL_VAD: {
            if (!hs_side_light_switch()) return false;
            if (confinfo.hs_music_cut || confinfo.rgb_light_mode == HS_SIDE_LIGHT_MAX_MODE) {
                return false;
            }
            if (record->event.pressed) {
                if (confinfo.rgb_light_val > 0) {
                    confinfo.rgb_light_val --;
                }
                eeconfig_confinfo_update();
            }
            return false;
        } break;
        case RGB_MOD: {
            if (record->event.pressed) {
                if (rgb_matrix_get_mode() == RGB_MATRIX_CUSTOM_RGBR_PLAY) {
                    if (rgbrec_is_started()) {
                        rgbrec_read_current_channel(confinfo.record_channel);
                        rgbrec_end(confinfo.record_channel,false);
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
                confinfo.hs_rgb_mode_choose = true;
                start_hsv = rgb_matrix_get_hsv();
                eeconfig_confinfo_update();
            }
            return false;
        } break;
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
        default:
            break;
    }

    return true;
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

    switch (new_devs) {
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
        case DEVS_USB: {
            if (reset) {
                rgb_matrix_wls_indicator_set(HS_RGB_BLINK_INDEX_USB, (RGB){HS_LBACK_COLOR_USB}, 200, 1);
            } else {
                rgb_matrix_wls_indicator_set(HS_RGB_BLINK_INDEX_USB, (RGB){HS_PAIR_COLOR_USB}, 500, 1);
            }
        } break;
        default:
            break;
    }
}

bool rgb_matrix_wls_indicator_cb(void) {

    if (*md_getp_state() != MD_STATE_CONNECTED || (wireless_get_current_devs() == DEVS_USB && USB_DRIVER.state != USB_ACTIVE)) {
        if (!(wireless_get_current_devs() == DEVS_USB && USB_DRIVER.state == USB_ACTIVE)) {
            wireless_devs_change_kb(wireless_get_current_devs(), wireless_get_current_devs(), wls_rgb_indicator_reset);
        } 
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
    rgb_matrix_blink_set_color(RGB_MATRIX_BLINK_INDEX_ALL, WIITE_B, 0x00, 0x00);
    rgb_matrix_blink_set_times(RGB_MATRIX_BLINK_INDEX_ALL, 1);
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

void housekeeping_task_user(void) { // loop
    uint8_t hs_now_mode;
    static uint32_t hs_current_time;
    static uint8_t last_charging_state = false;
    
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

    if (last_charging_state != charging_state) {
        last_charging_state = charging_state;
        if (!last_charging_state && wireless_get_current_devs() == DEVS_USB)
            rgb_matrix_wls_indicator_set(HS_RGB_BLINK_INDEX_USB, (RGB){HS_PAIR_COLOR_USB}, 500, 1);
    }

    switch (get_highest_layer(default_layer_state)) {
        case _BL: hs_layer_cnt = HS_WIN_BASE; break;
        case _MBL: hs_layer_cnt = HS_MAC_BASE; break;
    }

    if (pulse_active && timer_elapsed32(pulse_start_time) >= HS_MUSIC_DEFAULT_TIMER) { 
        gpio_write_pin_low(HS_MUSIC_MODE_PIN);  
        pulse_active = false;
        pulse_start_time = timer_read32();
    }
    
    if (hs_mucis_calibration && timer_elapsed32(mucis_calibration_time) >= HS_MUSIC_CALIBRATION_TIMER) { 
        gpio_write_pin_low(HS_MUSIC_MODE_PIN);  
        hs_mucis_calibration = false;
        mucis_calibration_time = timer_read32();
    }

    if (side_light_pulse_active && timer_elapsed32(side_light_pulse_start_time) >= HS_SIDE_LIGHT_DEFAULT_TIMER) { 
        gpio_write_pin_low(HS_SIDE_LIGHT_MODE_PIN);  
        side_light_pulse_active = false;
        side_light_pulse_start_time = timer_read32();
    }

    if (side_light_val_active && timer_elapsed32(side_light_val_start_time) >= HS_SIDE_LIGHT_VAI_TIMER) { 
        gpio_write_pin_low(HS_SIDE_LIGHT_MODE_PIN);  
        side_light_val_active = false;
        side_light_val_start_time = timer_read32();
    }

    static uint8_t bat_once_flag = false;
    if (!bat_once_flag) { 
        if (keymap_is_mac_system()) {
            md_send_devctrl_bat(0x80); 
        } else {
            md_send_devctrl_bat(0x81); 
        }
        bat_once_flag = true;
    }

    /* Set the sleep state of wireless mode */
    static uint8_t last_devs_mode = 0xFF; 
    if (wireless_get_current_devs() != last_devs_mode) {
        last_devs_mode = wireless_get_current_devs(); 
        if (wireless_get_current_devs() == DEVS_2G4) {
            md_send_devctrl(0xFF); 
        } else {
            md_send_devctrl(0x80 + HS_WIRESS_SLEEP_TIME); 
        }
    }

    if (rgb_matrix_get_mode() == 29 || rgb_matrix_get_mode() == 32) {
        if (!(ind_status != ind_state_none || !confinfo.hs_led_tog_flag)) {
            if (charging_state && ( (bat_full_flag))) { // power full
                rgb_matrix_set_color(HS_MATRIX_BLINK_INDEX_BAT, 0x00, 0xff, 0x00);
                hs_charing_filp = true;
            } else if (charging_state) { // charing...
                rgb_matrix_set_color(HS_MATRIX_BLINK_INDEX_BAT, 0xff, 0x00, 0x00);
                hs_charing_filp = true;
            } else if (*md_getp_bat() <= BATTERY_CAPACITY_LOW) { // low power
                if (!hs_charing_filp) {
                    rgb_matrix_set_color(HS_MATRIX_BLINK_INDEX_BAT, 0x00, 0x00, 0x00);
                } else {
                    rgb_matrix_set_color(HS_MATRIX_BLINK_INDEX_BAT, 0xff, 0x00, 0x00);
                } 
            } else {
                hs_charing_filp = true;
            }
        }
    }
}

#define WINDOW_SIZE 1 
#define DELAY_TIME 17  
void lightUpRainbowWindow(int startIndex);
bool start_paoma_flag           = true;

bool rgb_matrix_indicators_advanced_kb(uint8_t led_min, uint8_t led_max) {
if (!start_paoma_flag) {
    extern LED_TYPE rgb_matrix_ws2812_array[RGB_MATRIX_LED_COUNT];
    
    const uint8_t mirror_map[][2] = {
        {111, 109}, {111, 110},  {111, 112},  {111, 113},
        {70, 59},

        {115, 116}, {115, 117}, {115, 118}, {115, 119}, {115, 120},  // 115 -- 120
        {121, 122}, {121, 123}, {121, 124}, {121, 125}, {121, 126},  // 121 -- 126
        {127, 128}, {127, 129}, {127, 130}, {127, 131}, {127, 132},  // 127 -- 132
        {133, 134}, {133, 135}, {133, 136}, {133, 137}, {133, 138},  // 133 -- 138
        {139, 140}, {139, 141}, {139, 142}, {139, 143}, {139, 144},  // 139 -- 144

        {145, 146}, {145, 147}, {145, 148}, {145, 149},              // 145 -- 149
        {150, 151}, {150, 152}, {150, 153}, {150, 154},              // 150 -- 154
        {155, 156}, {155, 157}, {155, 158}, {155, 159},              // 155 -- 159
        {160, 161}, {160, 162}, {160, 163}, {160, 164},              // 160 -- 164
        {165, 166}, {165, 167}, {165, 168}, {165, 169},              // 165 -- 169
        {170, 171}, {170, 172}, {170, 173}, {170, 174},              // 170 -- 174
        {175, 176}, {175, 177}, {175, 178}, {175, 179},              // 175 -- 179
        {180, 181}, {180, 182}, {180, 183}, {180, 184},              // 180 -- 184
        {185, 186}, {185, 187}, {185, 188}, {185, 189},              // 185 -- 189
        {190, 191}, {190, 192}, {190, 193}, {190, 194},              // 190 -- 194
        {195, 196}, {195, 197}, {195, 198}, {195, 199},              // 195 -- 199

        {200, 201}, {200, 202}, {200, 203}, {200, 204}, {200, 205},  // 200 -- 205
        {206, 207}, {206, 208}, {206, 209}, {206, 210}, {206, 211},  // 206 -- 211
        {212, 213}, {212, 214}, {212, 215}, {212, 216}, {212, 217},  // 212 -- 217
        {218, 219}, {218, 220}, {218, 221}, {218, 222}, {218, 223},  // 218 -- 223
        {224, 225}, {224, 226}, {224, 227}, {224, 228}, {224, 229},  // 224 -- 229

    };

    if (ee_clr_timer && timer_elapsed32(ee_clr_timer) > 3000) {
        hs_reset_settings();
        ee_clr_timer = 0;
    }

    extern bool rk_bat_req_flag;
    if (!rk_bat_req_flag) {
        if (!rgbrec_is_started() && (USB_DRIVER.state == USB_ACTIVE || *md_getp_state() == MD_STATE_CONNECTED)) {
            static uint8_t last_caps_state = 0xff;
            if (last_caps_state != host_keyboard_led_state().caps_lock) {
                last_caps_state = host_keyboard_led_state().caps_lock;
                if (!host_keyboard_led_state().caps_lock) {
                    rgb_matrix_set_color(HS_RGB_INDEX_CAPS, RGB_BLACK);
                }
            }
            if (host_keyboard_led_state().caps_lock) {
                rgb_matrix_set_color(HS_RGB_INDEX_CAPS,0xff, 0xff, 0xff);
            }

            if (!host_keyboard_led_state().num_lock && !confinfo.hs_num_flag) {
                rgb_matrix_set_color(HS_RGB_INDEX_NUM, 0xff, 0xff, 0xff);
            }

            static uint8_t last_scroll_state = 0xff;
            if (last_scroll_state != host_keyboard_led_state().scroll_lock) {
                last_scroll_state = host_keyboard_led_state().scroll_lock;
                if (!host_keyboard_led_state().scroll_lock) {
                    rgb_matrix_set_color(HS_RGB_INDEX_SCR, RGB_BLACK);
                }
            }
            if (host_keyboard_led_state().scroll_lock) {
                rgb_matrix_set_color(HS_RGB_INDEX_SCR,0xff, 0xff, 0xff);
            }
        } 

        if (!keymap_is_mac_system()) {
            static uint8_t last_win_state = 0xff;
            if (last_win_state != keymap_config.no_gui) {
                last_win_state = keymap_config.no_gui;
                if (!keymap_config.no_gui) {
                    rgb_matrix_set_color(HS_RGB_INDEX_WIN_LOCK, RGB_BLACK);
                }
            }
        }

        if (keymap_config.no_gui && !keymap_is_mac_system()){
            rgb_matrix_set_color(HS_RGB_INDEX_WIN_LOCK,0xff,0xff,0xff);
        } 

        for (uint8_t i = 0; i < ARRAY_SIZE(mirror_map); i++) {
            uint8_t src = mirror_map[i][0];
            uint8_t dst = mirror_map[i][1];
            uint8_t r = rgb_matrix_ws2812_array[src].r;
            uint8_t g = rgb_matrix_ws2812_array[src].g;
            uint8_t b = rgb_matrix_ws2812_array[src].b;
            rgb_matrix_set_color(dst, r, g, b);
        }

        if (hs_side_light_switch()) {
            hs_rgblight_increase(confinfo.rgb_light_mode, confinfo.rgb_light_val);
        }
    }

#ifdef WIRELESS_ENABLE 
    rgb_matrix_wls_indicator();
#endif

    // rgb_matrix_start_rec();

    rgb_matrix_blink_task(led_min, led_max);
    
    query();
}
    if (start_paoma_flag) {
        static uint8_t led_start = 0;
        if (led_start < (RGB_MATRIX_LED_COUNT - RGBLED_NUM - WINDOW_SIZE + 1)) {
            do {
                rgb_matrix_set_color_all(0x00, 0x00, 0x00); 
            } while (0);
            
            static uint32_t time_delay = 0;
            if (timer_elapsed32(time_delay) > DELAY_TIME) {
                time_delay = timer_read32();
                led_start++;
            }
            lightUpRainbowWindow(led_start);
        } else {
            start_paoma_flag = false;
        }
    }

    return true;
}

bool hs_side_light_switch(void) {
    if (rgb_matrix_get_mode() != 31 && rgb_matrix_get_mode() != 44 &&
        rgb_matrix_get_mode() != 36 && rgb_matrix_get_mode() != 37 &&
        rgb_matrix_get_mode() != 38 && rgb_matrix_get_mode() != 39 &&
        rgb_matrix_get_mode() != 40 && rgb_matrix_get_mode() != 41 &&
        rgb_matrix_get_mode() != 42 && rgb_matrix_get_mode() != 43 ) {
        return true;
    }
    return false;
}

uint8_t hs_rgb_buff_index[(RGB_MATRIX_LED_COUNT - RGBLED_NUM)] = {
    64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 6,  5,  4,
    3,  81, 80, 79, 78, 77, 76, 75, 74, 73, 72, 71, 70, 69, 68, 67, 65, 31, 32,  0, 33, 34, 
    35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 2,  82,
    84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 66, 30, 29, 28, 27, 
    26, 25, 1,  24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 49,
    50, 7,  83, 9,  8,  114,113,112,111,110,109,108,107,106,105,104,103,102,101,100
};

void lightUpRainbowWindow(int startIndex) {
    for (int i = 0; i < WINDOW_SIZE; i++) {
        int led_index = hs_rgb_buff_index[startIndex + i];
	    uint8_t hue = (startIndex + i) * 10; 
        HSV hsv = { .h = hue % 256, .s = 255, .v = 120 };
        RGB rgb = hsv_to_rgb(hsv);
        rgb_matrix_set_color(led_index, rgb.r, rgb.g, rgb.b);
    }

    for (int i = 0; i < startIndex; i++) {
        int led_index = hs_rgb_buff_index[i];
        uint8_t hue = i * 10;  
        HSV hsv = { .h = hue % 256, .s = 255, .v = 120 };
        RGB rgb = hsv_to_rgb(hsv);
        rgb_matrix_set_color(led_index, rgb.r, rgb.g, rgb.b);
    }
}

void hs_reset_settings(void) {
    eeconfig_init();
    
#ifdef RGBLIGHT_ENABLE
    extern void rgblight_init(void);
    is_rgblight_initialized = false;
    rgblight_init();
    eeconfig_update_rgblight_default();
    rgblight_enable();
#endif

    eeconfig_update_rgb_matrix_default();
    keymap_config.raw = eeconfig_read_keymap();

    if (hs_reset_settings_user() != true) {
        return;
    }

    keyboard_post_init_kb();
}


void lpwr_wakeup_hook(void) { // awake

    if (wireless_get_current_devs() == DEVS_USB && USB_DRIVER.state == USB_STOP) {
        usb_power_connect();
        restart_usb_driver(&USBD1);
        wireless_devs_change(!DEVS_USB, DEVS_USB, false);
    }
    
    hs_mode_scan(false, confinfo.devs, confinfo.last_btdevs);
#ifdef LED_POWER_EN_PIN
    gpio_write_pin_high(LED_POWER_EN_PIN);
#endif

#ifdef HS_LED_EN_PIN
    gpio_write_pin_high(HS_LED_EN_PIN);
#endif

#ifdef HS_LED_BOOSTING_PIN
    gpio_write_pin_high(HS_LED_BOOSTING_PIN);
#endif

#ifdef HS_SIDE_LIGHT_EN_PIN
    gpio_write_pin_high(HS_SIDE_LIGHT_EN_PIN);
#endif

#ifdef HS_SIDE_LIGHT_SLEEP_PIN
    gpio_write_pin_high(HS_SIDE_LIGHT_SLEEP_PIN);
#endif

#ifdef HS_ENCODERS_PAD_A
    setPinInputHigh(HS_ENCODERS_PAD_A);
#endif

#ifdef HS_ENCODERS_PAD_B
    setPinInputHigh(HS_ENCODERS_PAD_B);
#endif

    gpio_set_pin_output(HS_MUSIC_MODE_PIN);
    gpio_write_pin_low(HS_MUSIC_MODE_PIN);

    gpio_set_pin_output(HS_SIDE_LIGHT_MODE_PIN);
    gpio_write_pin_low(HS_SIDE_LIGHT_MODE_PIN);

    gpio_set_pin_output(HS_MUSIC_CUT_PIN);
    if (confinfo.hs_music_cut) {
        gpio_write_pin_high(HS_MUSIC_CUT_PIN);
        music_cut_flag = false;
    } else {
        gpio_write_pin_low(HS_MUSIC_CUT_PIN);
        rgblight_cut_flag = false;
    }
}

