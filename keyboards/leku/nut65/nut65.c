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
    HS_WIN_BASE  = _BL,
    HS_MAC_BASE  = _MBL,
};

typedef union {
    uint32_t raw;
    struct {
        uint8_t hs_rgb_tog_flag : 1;
        uint8_t devs : 3;
        uint8_t hs_led_tog_flag : 1;
        uint8_t last_btdevs : 3;

        uint8_t sleep_timeout : 3;
        uint8_t DEB_flag: 1;
        uint8_t hs_cut_f_layer: 1;
        uint8_t hs_rgb_mode_choose : 1;
        uint8_t hs_music_cut : 1;
        uint8_t rl_val : 3;
        uint8_t record_channel : 2;
        uint8_t rgb_light_mode : 4;

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

void hs_reset_settings(void);
void matrix_init_user(void);
void hs_color_test(void);
void hs_rl_brightness_switch(void);

#define keymap_is_mac_system() ((get_highest_layer(default_layer_state) == _MBL))

uint32_t post_init_timer      = 0x00;
bool charging_state           = false;
bool bat_full_flag            = false;
static uint32_t ee_clr_timer  = 0;
static uint32_t hs_freq_timer = 0;
bool test_white_light_flag    = false;
uint8_t test_color_cnt        = 0;
uint8_t mac_status            = false;
static bool shifted           = false;
bool hs_frequency_test        = false;
bool hs_rgb_blink_flag        = false;
bool lower_sleep = false;
bool no_record_fg;
bool im_test_rate_flag;
uint32_t rec_time;
bool rec_filp;
HSV start_hsv;
uint8_t hs_layer_cnt            = 0;
uint16_t hs_deb                 = DEBOUNCE;
static bool hs_charing_filp     = false;

bool music_cut_flag             = true;
bool rgblight_cut_flag          = false;
static bool pulse_active        = false;
static uint32_t pulse_start_time = 0;
bool hs_mucis_calibration       = true;
uint32_t mucis_calibration_time = 0;
bool start_paoma_flag           = true;
uint8_t rl_rgb_brightness_val   = 0;

static bool linker_ok = false;
static uint32_t linker_time = 0;
bool linker_blink = false;
void hs_rgb_blink_hook_user(void);

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
            if (ind_status != ind_state_none || test_white_light_flag == true || !confinfo.hs_led_tog_flag) {
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
            
            if (test_white_light_flag) {
                return false;
            }
            
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

    confinfo.hs_rgb_tog_flag   = true;
    confinfo.hs_led_tog_flag   = true;
    confinfo.record_channel    = 0;
    confinfo.record_last_mode  = 0xff;
    confinfo.last_btdevs       = 1;
    confinfo.sleep_timeout     = 7;
    confinfo.rgb_light_mode    = 0;
    confinfo.DEB_flag          = false;
    confinfo.hs_cut_f_layer    = false;
    confinfo.hs_rgb_mode_choose = true;

    rgblight_cut_flag          = false;
    music_cut_flag             = true;
    confinfo.hs_music_cut      = false;
    confinfo.rl_val            = 4;

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

#ifdef RGBLIGHT_ENABLE
    rgblight_mode(8);
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
        md_send_devctrl(0xFF); 
    }

    keyboard_post_init_user();
    rgbrec_init(confinfo.record_channel);

    gpio_set_pin_output(HS_MUSIC_MODE_PIN);
    gpio_write_pin_low(HS_MUSIC_MODE_PIN);

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

    hs_rl_brightness_switch();
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

    suspend_power_down_user();
}

void suspend_wakeup_init_kb(void) {

#    ifdef LED_POWER_EN_PIN
    gpio_write_pin_high(LED_POWER_EN_PIN);
#    endif

#ifdef HS_LED_BOOSTING_PIN
    gpio_write_pin_high(HS_LED_BOOSTING_PIN);
#endif

    gpio_set_pin_output(HS_MUSIC_MODE_PIN);
    gpio_write_pin_low(HS_MUSIC_MODE_PIN);

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
            md_send_devctrl(0xFF);
            linker_ok = false;
        } break;
        case KC_BT2: {
            uint8_t mode = confinfo.devs;
            hs_modeio_detection(true, &mode, confinfo.last_btdevs);
            if ((mode == hs_bt) || (mode == hs_wireless) || (mode == hs_none)) {
                WLS_KEYCODE_EXEC(DEVS_BT2);
     
                if (record->event.pressed) {suspend_wakeup_init();state = false;
                hs_rgb_blink_set_timer(timer_read32());}
            }
            md_send_devctrl(0xFF);
            linker_ok = false;
        } break;
        case KC_BT3: {
            uint8_t mode = confinfo.devs;
            hs_modeio_detection(true, &mode, confinfo.last_btdevs);
            if ((mode == hs_bt) || (mode == hs_wireless) || (mode == hs_none)) {
                WLS_KEYCODE_EXEC(DEVS_BT3);
              
                if (record->event.pressed) {suspend_wakeup_init();state = false;
                hs_rgb_blink_set_timer(timer_read32());}
            }
            md_send_devctrl(0xFF);
            linker_ok = false;
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
            linker_ok = false;
        } break;

        case KC_USB: {
            uint8_t mode = confinfo.devs;
            hs_modeio_detection(true, &mode, confinfo.last_btdevs);
            if ((mode == hs_2g4) || (mode == hs_wireless) || (mode == hs_none)) {
                WLS_KEYCODE_EXEC(DEVS_USB);
                state = false;
                hs_rgb_blink_set_timer(timer_read32());
            }
            linker_ok = false;
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

static void handle_key_with_layer(uint16_t normal_code, uint16_t mac_code, uint16_t win_code, keyrecord_t *record) {
    if (confinfo.hs_cut_f_layer) {
        uint16_t target_code = keymap_is_mac_system() ? mac_code : win_code;
        if (record->event.pressed) {
            register_code(target_code);
        } else {
            unregister_code(target_code);
        }
    } else {
        if (record->event.pressed) {
            register_code(normal_code);
        } else {
            unregister_code(normal_code);
        }
    }
}

bool hs_process_record(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case KC_1: handle_key_with_layer(KC_1, KC_BRID, KC_F1, record); return false;
        case KC_2: handle_key_with_layer(KC_2, KC_BRIU, KC_F2, record); return false;
        case KC_3: handle_key_with_layer(KC_3, KC_MCTL, KC_F3, record); return false;
        case KC_4: handle_key_with_layer(KC_4, KC_LPAD, KC_F4, record); return false;
        case KC_5: handle_key_with_layer(KC_5, KC_F5,  KC_F5, record); return false;
        case KC_6: handle_key_with_layer(KC_6, KC_F6,  KC_F6, record); return false;
        case KC_7: handle_key_with_layer(KC_7, KC_MPRV, KC_F7, record); return false;
        case KC_8: handle_key_with_layer(KC_8, KC_MPLY, KC_F8, record); return false;
        case KC_9: handle_key_with_layer(KC_9, KC_MNXT, KC_F9, record); return false;
        case KC_0: handle_key_with_layer(KC_0, KC_MUTE, KC_F10, record); return false;
        case KC_MINS: handle_key_with_layer(KC_MINS, KC_VOLD, KC_F11, record); return false;
        case KC_EQL:  handle_key_with_layer(KC_EQL,  KC_VOLU, KC_F12, record); return false;

        case KC_F1:  handle_key_with_layer(KC_F1, KC_F1, KC_MYCM, record); return false;
        case KC_F2:  handle_key_with_layer(KC_F2, KC_F2, KC_WHOM, record); return false;
        case KC_F3:  handle_key_with_layer(KC_F3, KC_F3, KC_MAIL, record); return false;
        case KC_F4:  handle_key_with_layer(KC_F4, KC_F4, KC_CALC, record); return false;
        case KC_F5:  handle_key_with_layer(KC_F5, KC_F5, KC_MSEL, record); return false;
        case KC_F6:  handle_key_with_layer(KC_F6, KC_F6, KC_MSTP, record); return false;
        case KC_F7:  handle_key_with_layer(KC_F7, KC_F7, KC_MPRV, record); return false;
        case KC_F8:  handle_key_with_layer(KC_F8, KC_F8, KC_MPLY, record); return false;
        case KC_F9:  handle_key_with_layer(KC_F9, KC_F9, KC_MNXT, record); return false;
        case KC_F10: handle_key_with_layer(KC_F10, KC_F10, KC_MUTE, record); return false;
        case KC_F11: handle_key_with_layer(KC_F11, KC_F11, KC_VOLD, record); return false;
        case KC_F12: handle_key_with_layer(KC_F12, KC_F12, KC_VOLU, record); return false;
    }
    return true;
}


bool process_record_kb(uint16_t keycode, keyrecord_t *record) {

    if (process_record_user(keycode, record) != true) {
        return false;
    }

    if (hs_process_record(keycode, record) != true) {
        return false;
    }

#ifdef WIRELESS_ENABLE
    if (process_record_wls(keycode, record) != true) {
        return false;
    }
#endif

    if (hs_frequency_test){
        if (!(keycode == HS_FREQ)) {
            return false;
        }
    } 

    if (test_white_light_flag && record->event.pressed && keycode != KC_SPC) {
        test_white_light_flag = false;
        rgb_matrix_set_color_all(0x00, 0x00, 0x00);
    }

    if ((confinfo.devs != DEVS_USB && *md_getp_state() == MD_STATE_DISCONNECTED)){
        if(IS_BASIC_KEYCODE(keycode) || IS_MODIFIER_KEYCODE(keycode)){
            return false;
        }
    }

    switch (keycode) {
        case RL_VAI:{
            if (confinfo.rgb_light_mode == 12) return false;
            if (record->event.pressed) {
                if(confinfo.rl_val < 4){
                    confinfo.rl_val++;
                }
                if (confinfo.rl_val > 4) {
                    confinfo.rl_val = 4;
                }
                hs_rl_brightness_switch();
                eeconfig_confinfo_update();
            }
            return false;
        } break;
        case RL_VAD:{
            if (confinfo.rgb_light_mode == 12) return false;
            if (record->event.pressed) {
                if(confinfo.rl_val > 0){
                    confinfo.rl_val--;
                }
                
                if (confinfo.rl_val <= 0) {
                    confinfo.rl_val = 0;
                }
                hs_rl_brightness_switch();
                eeconfig_confinfo_update();
            }
            return false;
        } break;
        case QK_GESC: {
            static bool grave_esc_was_shifted = false;
            if (record->event.pressed) {
                grave_esc_was_shifted = shifted;
                register_code16(shifted ? KC_GRAVE : KC_ESCAPE);
            } else {
                unregister_code16(grave_esc_was_shifted ? KC_GRAVE : KC_ESCAPE);
            }
            return false;   
        } break;
        case KC_LSFT:
        case KC_RSFT: {
            shifted = record->event.pressed;
            return true;
        } break;
        case KC_FTOG:{
            if (record->event.pressed) {
                confinfo.hs_cut_f_layer = !confinfo.hs_cut_f_layer;
                eeconfig_confinfo_update();
            }
            return false;
            break;
        }
        case HS_FREQ: {
            if (record->event.pressed) {
                hs_freq_timer = timer_read32();
            } else {
                hs_freq_timer = 0;
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
        case BT_TEST: {
            if(record->event.pressed) {
                md_send_devctrl(0x62);
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
            if (record->event.pressed) {
                test_white_light_flag = true;
            }
            return false;
        } break;
        case KC_SPC: {
            if (test_white_light_flag) {
                if (record->event.pressed) {
                    if ((++test_color_cnt) > 3) {
                        test_color_cnt = 0;
                    }
                }
                return false;
            }
            return true;
        } break;
        case HS_RWCK: { 
            if (record->event.pressed && !rgbrec_is_started()) {
                register_code(KC_LALT);
                register_code(KC_TAB);
            } else {
                unregister_code(KC_LALT);
                unregister_code(KC_TAB);
            }
            return true;
        } break;
        case HS_YYIN: { 
            if (record->event.pressed && !rgbrec_is_started()) {
                register_code(KC_LCMD);
                register_code(KC_H);
            } else {
                unregister_code(KC_LCMD);
                unregister_code(KC_H);
            }
            return true;
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
        case RGB_TOG: {
            if (record->event.pressed) {
                confinfo.hs_rgb_tog_flag = !confinfo.hs_rgb_tog_flag;
                eeconfig_confinfo_update();
            } 
            return false;
        } break;
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
                    if (!(host_keyboard_led_state().caps_lock)) {
                        for (uint8_t i = 0; i < RGB_MATRIX_LED_COUNT; i++) {
                            rgb_matrix_set_color(i, 0, 0, 0);
                        }
                    }
                    start_hsv.v = 0;
                } else {
                    start_hsv.v = rgb_matrix_get_val() - RGB_MATRIX_VAL_STEP;
                }
            }
        } break;
        case RGB_SAD: {
            if (record->event.pressed) {
                if (rgb_matrix_get_mode() == 2) {
                    confinfo.hs_rgb_mode_choose = false;
                    eeconfig_confinfo_update();
                }
                hs_record_color_s_ins();
            }
            return false;
        } break;
        case RGB_SAI: {
            if (record->event.pressed) {
                if (rgb_matrix_get_mode() == 2) {
                    confinfo.hs_rgb_mode_choose = false;
                    eeconfig_confinfo_update();
                }
                hs_record_color_s_add();
            }
            return false;
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
        case RL_MOD: {
            if (record->event.pressed) {
                confinfo.hs_music_cut = false;
                music_cut_flag        = true;
                gpio_write_pin_low(HS_MUSIC_CUT_PIN);

                if (rgblight_cut_flag) {
                    rgblight_cut_flag = false;
                } else {
                    confinfo.rgb_light_mode ++;
                    if (confinfo.rgb_light_mode > 12) {
                        confinfo.rgb_light_mode = 0;
                    }
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
                shifted = record->event.pressed;
                if (keymap_config.no_gui && !rgbrec_is_started()) {
                    if (record->event.pressed) {
                        register_code16(KC_LCMD);
                    } else {
                        unregister_code16(KC_LCMD);
                    }
                }
            } else {
                if (!keymap_config.no_gui) {
                    shifted = record->event.pressed;
                }
            }

            return true;
        } break;
        case KC_RCMD: {
            if (keymap_is_mac_system()) {
                shifted = record->event.pressed;
                if (keymap_config.no_gui && !rgbrec_is_started()) {
                    if (record->event.pressed) {
                        register_code16(KC_RCMD);
                    } else {
                        unregister_code16(KC_RCMD);
                    }
                }
            } else {
                if (!keymap_config.no_gui) {
                    shifted = record->event.pressed;
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

bool previous_matrix[MATRIX_ROWS][MATRIX_COLS] = {0};  

void hs_matrix_loop(void) {
    if (!hs_frequency_test) return;

    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            bool is_on_now = matrix_is_on(row, col);

            if (is_on_now && !previous_matrix[row][col]) {

                if (row == 0 && col == 1) { 
                    md_send_devctrl(0x60);
                    md_rf_send_carrier(0, 5, 0x01);
                } else if (row == 0 && col == 2) { 
                    md_send_devctrl(0x60);
                    md_rf_send_carrier(19, 5, 0x01);
                } else if (row == 0 && col == 3) { 
                    md_send_devctrl(0x60);
                    md_rf_send_carrier(39, 5, 0x01);
                } else if (row == 0 && col == 4) { 
                    md_rf_send_stop();
                }
            }

            previous_matrix[row][col] = is_on_now;
        }
    }
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

    hs_matrix_loop();

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
            md_send_devctrl(0xFF); 
        }
    }

    if (rgb_matrix_get_mode() == 29 || rgb_matrix_get_mode() == 32) {
        if (!(ind_status != ind_state_none || test_white_light_flag == true || !confinfo.hs_led_tog_flag)) {
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

#define WINDOW_SIZE 5  
#define DELAY_TIME 50  
void lightUpRainbowWindow(int startIndex);
bool rgb_matrix_indicators_advanced_kb(uint8_t led_min, uint8_t led_max) {
    if (!start_paoma_flag) {
        extern LED_TYPE rgb_matrix_ws2812_array[RGB_MATRIX_LED_COUNT];

        const uint8_t mirror_map[][2] = {
            {12, 10}, {12, 11}, {12, 13}, {12, 14},

            {71, 72}, {71, 73}, {71, 74}, {71, 75}, {71, 76},       // 72 -- 76
            {77, 78}, {77, 79}, {77, 80}, {77, 81}, {77, 82},       // 73 -- 82
            {83, 84}, {83, 85}, {83, 86}, {83, 87}, {83, 88},       // 84 -- 88
            {89, 90}, {89, 91}, {89, 92}, {89, 93}, {89, 94},       // 90 -- 94
            {95, 96}, {95, 97}, {95, 98}, {95, 99}, {95, 100},      // 96 -- 100

            {101, 102}, {101, 103}, {101, 104}, {101, 105},            // 102 -- 105
            {106, 107}, {106, 108}, {106, 109}, {106, 110},            // 107 -- 110
            {111, 112}, {111, 113}, {111, 114}, {111, 115},            // 118 -- 115
            {116, 117}, {116, 118}, {116, 119}, {116, 120},            // 117 -- 120
            {121, 122}, {121, 123}, {121, 124}, {121, 125},            // 122 -- 125
            {126, 127}, {126, 128}, {126, 129}, {126, 130},            // 127 -- 130
            {131, 132}, {131, 133}, {131, 134}, {131, 135},            // 132 -- 135
            {136, 137}, {136, 138}, {136, 139}, {136, 140},            // 137 -- 140
            {141, 142}, {141, 143}, {141, 144}, {141, 145},            // 142 -- 145
            {146, 147}, {146, 148}, {146, 149}, {146, 150},            // 147 -- 150
        };

        for (uint8_t i = 0; i < ARRAY_SIZE(mirror_map); i++) {
            uint8_t src = mirror_map[i][0];
            uint8_t dst = mirror_map[i][1];
            uint8_t r = rgb_matrix_ws2812_array[src].r;
            uint8_t g = rgb_matrix_ws2812_array[src].g;
            uint8_t b = rgb_matrix_ws2812_array[src].b;
            rgb_matrix_set_color(dst, r, g, b);
        }

        if (ee_clr_timer && timer_elapsed32(ee_clr_timer) > 3000) {
            hs_reset_settings();
            ee_clr_timer = 0;
        }

        if (hs_freq_timer && timer_elapsed32(hs_freq_timer) > 10000) {
            hs_frequency_test = !hs_frequency_test;
            hs_freq_timer = 0;
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

            if (rgb_matrix_get_mode() != 31 && rgb_matrix_get_mode() != 44 &&
                rgb_matrix_get_mode() != 36 && rgb_matrix_get_mode() != 37 &&
                rgb_matrix_get_mode() != 38 && rgb_matrix_get_mode() != 39 &&
                rgb_matrix_get_mode() != 40 && rgb_matrix_get_mode() != 41 &&
                rgb_matrix_get_mode() != 42 && rgb_matrix_get_mode() != 43 ) {
                hs_rgblight_increase(confinfo.rgb_light_mode, rl_rgb_brightness_val, confinfo.rl_val);
            }
        }

        if (hs_frequency_test) {
            rgb_matrix_set_color(HS_RGB_BLINK_INDEX_FREQ, RGB_WHITE);
        }

#ifdef WIRELESS_ENABLE 
        hs_rgb_blink_hook_user();
        if (wireless_get_current_devs() != DEVS_USB) {
            if (!linker_blink) {
                rgb_matrix_wls_indicator();
            }
        } else {
            rgb_matrix_wls_indicator();
        }
#endif

        rgb_matrix_blink_task(led_min, led_max);
        hs_color_test();
        query();
    }

    if (start_paoma_flag) {
        static uint8_t led_start = 0;
        if (led_start < (RGB_MATRIX_LED_COUNT - RGBLED_NUM - WINDOW_SIZE)) {
            rgb_matrix_set_color_all(0x00, 0x00, 0x00); // ÇåÆÁ
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

uint8_t hs_rgb_buff_index[(RGB_MATRIX_LED_COUNT - RGBLED_NUM)] = {
    56,  57,  58,  59,  60,  61,  62,  63,  64,  65,  66,  67,  68,  69,  70,
    5,   4,   3,   2,   1,   0,   55,  54,  53,  52,  51,  50,  49,  48,  47,
    6,   34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,       45,  46,
    33,       32,  31,  30,  29,  28,  27,  26,  25,  24,  23,  22,  21,  20,
    7,   8,   9,             12,                      15,  16,  17,  18,  19
};

void lightUpRainbowWindow(int startIndex) {
    for (int i = 0; i < WINDOW_SIZE; i++) {
        int led_index = hs_rgb_buff_index[startIndex + i];
        uint8_t hue = (startIndex + i) * 10; 
        HSV hsv = { .h = hue % 256, .s = 255, .v = 255 };
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

#ifdef HS_LED_BOOSTING_PIN
    gpio_write_pin_high(HS_LED_BOOSTING_PIN);
#endif

#ifdef HS_ENCODERS_PAD_A
    setPinInputHigh(HS_ENCODERS_PAD_A);
#endif

#ifdef HS_ENCODERS_PAD_B
    setPinInputHigh(HS_ENCODERS_PAD_B);
#endif

    gpio_set_pin_output(HS_MUSIC_MODE_PIN);
    gpio_write_pin_low(HS_MUSIC_MODE_PIN);

    gpio_set_pin_output(HS_MUSIC_CUT_PIN);
    if (confinfo.hs_music_cut) {
        gpio_write_pin_high(HS_MUSIC_CUT_PIN);
        music_cut_flag = false;
    } else {
        gpio_write_pin_low(HS_MUSIC_CUT_PIN);
        rgblight_cut_flag = false;
    }

}

void matrix_init_user(void) {
    
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            if (matrix_is_on(row, col)) {

                if (row == 5 && col == 0) {
                    hs_frequency_test = true; 
                }
            }
        }
    }
}

void hs_color_test(void) {
    if (test_white_light_flag) {
         RGB rgb_test_open;
        switch (test_color_cnt) {
            case 0: rgb_test_open = hsv_to_rgb((HSV){.h = 0, .s = 255, .v = RGB_MATRIX_VAL_STEP * 5}); break;
            case 1: rgb_test_open = hsv_to_rgb((HSV){.h = 85, .s = 255, .v = RGB_MATRIX_VAL_STEP * 5}); break;
            case 2: rgb_test_open = hsv_to_rgb((HSV){.h = 170, .s = 255, .v = RGB_MATRIX_VAL_STEP * 5}); break;
            case 3: rgb_test_open = hsv_to_rgb((HSV){.h = 0, .s = 0, .v = RGB_MATRIX_VAL_STEP * 5}); break;
            default:
                break;
        }
        rgb_matrix_set_color_all(rgb_test_open.r, rgb_test_open.g, rgb_test_open.b);
    }
}

void hs_rl_brightness_switch(void) {
    switch (confinfo.rl_val) {
        case 0: rl_rgb_brightness_val = 8; break;
        case 1: rl_rgb_brightness_val = 17; break;
        case 2: rl_rgb_brightness_val = 30; break;
        case 3: rl_rgb_brightness_val = 42; break;
        case 4: rl_rgb_brightness_val = 55; break;
        default: rl_rgb_brightness_val = 55; break;
    }
}

void hs_rgb_blink_hook_user(void) { 
    if (wireless_get_current_devs() == DEVS_USB) { 
        return;
    } 

    if (*md_getp_state() == MD_STATE_CONNECTED) {
        linker_ok = true;
        linker_time = timer_read32();
    } else {
        if (linker_ok) { 
            linker_blink = true;
            if (timer_elapsed32(linker_time) > 2000) {
                linker_ok = false;
            } 
        } else {
            linker_blink = false;
        }
    }    
}
