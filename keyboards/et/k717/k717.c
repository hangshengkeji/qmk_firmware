// Copyright 2024 yangzheng20003 (@yangzheng20003)
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include "wls/wls.h"
#include "rgb_record/rgb_record.h"
#include "rgb_record/rgb_rgblight.h"


const aw20216s_led_t PROGMEM g_aw20216s_leds[AW20216S_LED_COUNT] = {
/* Refer to IS31 manual for these locations
 *   driver
 *   |     R location
 *   |     |          G location
 *   |     |          |          B location
 *   |     |          |          | */
    {0, SW1_CS15,  SW1_CS14,  SW1_CS13  },  // 0  Esc
    {0, SW2_CS15,  SW2_CS14,  SW2_CS13  },  // 1  F1
    {0, SW3_CS15,  SW3_CS14,  SW3_CS13  },  // 2  F2
    {0, SW4_CS15,  SW4_CS14,  SW4_CS13  },  // 3  F3
    {0, SW5_CS15,  SW5_CS14,  SW5_CS13  },  // 4  F4
    {0, SW6_CS15,  SW6_CS14,  SW6_CS13  },  // 5  F5
    {0, SW7_CS15,  SW7_CS14,  SW7_CS13  },  // 6  F6
    {0, SW8_CS15,  SW8_CS14,  SW8_CS13  },  // 7  F7
    {0, SW9_CS15,  SW9_CS14,  SW9_CS13  },  // 8  F8
    {0, SW10_CS15, SW10_CS14, SW10_CS13  },  // 9  F9
    {0, SW11_CS15, SW11_CS14, SW11_CS13  },  // 10 F10
    {0, SW12_CS15, SW12_CS14, SW12_CS13  },  // 11 F11
    {0, SW2_CS18,  SW2_CS17,  SW2_CS16   },  // 12 F12
    {0, SW3_CS18,  SW3_CS17,  SW3_CS16   },  // 13 MUTE
    {0, SW1_CS12,  SW1_CS11,  SW1_CS10  },  // 14 PSCR
    {0, SW2_CS12,  SW2_CS11,  SW2_CS10  },  // 15 SCRL
    {0, SW3_CS12,  SW3_CS11,  SW3_CS10  },  // 16 PAUSE
    {0, SW4_CS12,  SW4_CS11,  SW4_CS10  },  // 17 `~
    {0, SW5_CS12,  SW5_CS11,  SW5_CS10  },  // 18 1
    {0, SW6_CS12,  SW6_CS11,  SW6_CS10  },  // 19 2
    {0, SW7_CS12,  SW7_CS11,  SW7_CS10  },  // 20 3
    {0, SW8_CS12,  SW8_CS11,  SW8_CS10  },  // 21 4
    {0, SW9_CS12,  SW9_CS11,  SW9_CS10  },  // 22 5
    {0, SW10_CS12, SW10_CS11, SW10_CS10  },  // 23 6
    {0, SW11_CS12, SW11_CS11, SW11_CS10  },  // 24 7
    {0, SW12_CS12, SW12_CS11, SW12_CS10  },  // 25 8
    {0, SW4_CS18,  SW4_CS17,  SW4_CS16   },  // 26 9
    {0, SW5_CS18,  SW5_CS17,  SW5_CS16   },  // 27 0
    {0, SW1_CS1,  SW1_CS2,    SW1_CS3   },  // 28 -_
    {0, SW2_CS1,  SW2_CS2,    SW2_CS3   },  // 29 =+
    {0, SW3_CS1,  SW3_CS2,    SW3_CS3   },  // 30 BSPC
    {0, SW4_CS1,  SW4_CS2,    SW4_CS3   },  // 31 INS
    {0, SW5_CS1,  SW5_CS2,    SW5_CS3   },  // 32 HOME
    {0, SW6_CS1,  SW6_CS2,    SW6_CS3   },  // 33 PGUP
    {0, SW7_CS1,  SW7_CS2,    SW7_CS3   },  // 34 Tab
    {0, SW8_CS1,  SW8_CS2,    SW8_CS3   },  // 35 Q
    {0, SW9_CS1,  SW9_CS2,    SW9_CS3   },  // 36 W
    {0, SW10_CS1, SW10_CS2,   SW10_CS3   },  // 37 E
    {0, SW11_CS1, SW11_CS2,   SW11_CS3   },  // 38 R
    {0, SW12_CS1, SW12_CS2,   SW12_CS3   },  // 39 T
    {0, SW6_CS18, SW6_CS17,   SW6_CS16   },  // 40 Y
    {0, SW7_CS18, SW7_CS17,   SW7_CS16   },  // 41 U
    {0, SW1_CS4,  SW1_CS5,  SW1_CS6  },  // 42 I
    {0, SW2_CS4,  SW2_CS5,  SW2_CS6  },  // 43 O
    {0, SW3_CS4,  SW3_CS5,  SW3_CS6  },  // 44 P
    {0, SW4_CS4,  SW4_CS5,  SW4_CS6  },  // 45 [{
    {0, SW5_CS4,  SW5_CS5,  SW5_CS6  },  // 46 ]}
    {0, SW6_CS4,  SW6_CS5,  SW6_CS6  },  // 47 \|
    {0, SW7_CS4,  SW7_CS5,  SW7_CS6  },  // 48 Del
    {0, SW8_CS4,  SW8_CS5,  SW8_CS6  },  // 49 End
    {0, SW9_CS4,  SW9_CS5,  SW9_CS16  },  // 50 PGDN
    {0, SW10_CS4, SW10_CS5, SW10_CS6  },  // 51 Caps
    {0, SW11_CS4, SW11_CS5, SW11_CS6  },  // 52 A
    {0, SW12_CS4, SW12_CS5, SW12_CS6  },  // 53 S
    {0, SW8_CS18, SW8_CS17, SW8_CS16  },  // 54 D
    {0, SW9_CS18, SW9_CS17, SW9_CS16  },  // 55 F
    {0, SW1_CS7,   SW1_CS8,   SW1_CS9   },  // 56 G
    {0, SW2_CS7,   SW2_CS8,   SW2_CS9   },  // 57 H
    {0, SW3_CS7,   SW3_CS8,   SW3_CS9   },  // 58 J
    {0, SW7_CS7,   SW7_CS8,   SW7_CS9   },  // 59 K
    {0, SW11_CS7,  SW11_CS8,  SW11_CS9   },  // 60 L
    {0, SW12_CS7,  SW12_CS8,  SW12_CS9   },  // 61 ;:
    {0, SW10_CS18, SW10_CS17, SW10_CS16  },  // 62 '
    {0, SW11_CS18, SW11_CS17, SW11_CS16  }  // 63 #~
};

#ifdef WIRELESS_ENABLE
#    include "wireless.h"
#    include "usb_main.h"
#    include "lowpower.h"
#endif

typedef union {
    uint32_t raw;
    struct {
        uint8_t flag : 1;
        uint8_t devs : 3;
        uint8_t record_channel : 4;
        uint8_t record_last_mode;
        uint8_t last_btdevs : 3;
        uint8_t rgb_hsv_index :3;
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


	static uint16_t rgb_hsvs[7][2] = {   
    {0,255},
    {85,255},
    {170,255},
    {43,255},
    {191,255},
    {128,255},
    {0,0},
};

uint8_t rgb_hsvs1[6][3] = {
    {0,255},
    {85,255},
    {170,255},
    {43,255},
    {191,255},
    {128,255},
};

enum layers {
    _BL = 0,
    _FL,
    _MBL,
    _MFL,
    _FBL,
};

hs_rgb_indicator_t hs_rgb_indicators[HS_RGB_INDICATOR_COUNT];
hs_rgb_indicator_t hs_rgb_bat[HS_RGB_BAT_COUNT];

void rgb_blink_dir(void);
void hs_reset_settings(void);
void rgb_matrix_hs_indicator(void);
void rgb_matrix_hs_indicator_set(uint8_t index, RGB rgb, uint32_t interval, uint8_t times);
void rgb_matrix_hs_set_remain_time(uint8_t index, uint8_t remain_time);

#define keymap_is_mac_system() ((get_highest_layer(default_layer_state) == _MBL) || (get_highest_layer(default_layer_state) == _MFL))
#define keymap_is_base_layer() ((get_highest_layer(default_layer_state) == _BL) || (get_highest_layer(default_layer_state) == _FL))

uint32_t post_init_timer     = 0x00;
bool inqbat_flag             = false;
bool mac_status              = false;
bool charging_state          = false;
bool bat_full_flag           = false;
bool enable_bat_indicators   = true;
uint32_t bat_indicator_cnt   = true;
static uint32_t ee_clr_timer = 0;
bool test_white_light_flag = false;
bool lower_sleep = false;
uint8_t buff[]   = {14, 8, 2, 1, 1, 1, 1, 1, 1, 1, 0};

void eeconfig_confinfo_update(uint32_t raw) {

    eeconfig_update_kb(raw);
}

uint32_t eeconfig_confinfo_read(void) {

    return eeconfig_read_kb();
}

void eeconfig_confinfo_default(void) {

    confinfo.flag             = true;
    confinfo.record_channel   = 0;
    confinfo.record_last_mode = 0xff;
    confinfo.last_btdevs      = 1;

    // #ifdef WIRELESS_ENABLE
    //     confinfo.devs = DEVS_USB;
    // #endif

    eeconfig_init_user_datablock();
    eeconfig_confinfo_update(confinfo.raw);

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
    keymap_config.no_gui = 0;
    setPinInputHigh(C5);
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

#ifdef WIRELESS_ENABLE
    wireless_init();
    // wireless_devs_change(!confinfo.devs, confinfo.devs, false);
    post_init_timer = timer_read32();
#endif

    keyboard_post_init_user();
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

    wireless_devs_change(wireless_get_current_devs(), wireless_get_current_devs(), false);
    suspend_wakeup_init_user();
    hs_rgb_blink_set_timer(timer_read32());
}

bool lpwr_is_allow_timeout_hook(void) {

    if (wireless_get_current_devs() == DEVS_USB) {
        return false;
    }

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

uint32_t wls_process_long_press(uint32_t trigger_time, void *cb_arg) {
    uint16_t keycode = *((uint16_t *)cb_arg);

    switch (keycode) {
        case KC_BT1: {
            uint8_t mode = confinfo.devs;
            hs_modeio_detection(true, &mode, confinfo.last_btdevs);
            if ((mode == hs_bt) || (mode == hs_wireless) || (mode == hs_none)) {
                wireless_devs_change(wireless_get_current_devs(), DEVS_BT1, true);
            }

        } break;
        case KC_BT2: {
            uint8_t mode = confinfo.devs;
            hs_modeio_detection(true, &mode, confinfo.last_btdevs);
            if ((mode == hs_bt) || (mode == hs_wireless) || (mode == hs_none)) {
                wireless_devs_change(wireless_get_current_devs(), DEVS_BT2, true);
            }
        } break;
        case KC_BT3: {
            uint8_t mode = confinfo.devs;
            hs_modeio_detection(true, &mode, confinfo.last_btdevs);
            if ((mode == hs_bt) || (mode == hs_wireless) || (mode == hs_none)) {
                wireless_devs_change(wireless_get_current_devs(), DEVS_BT3, true);
            }
        } break;
        case KC_2G4: {
            uint8_t mode = confinfo.devs;
            hs_modeio_detection(true, &mode, confinfo.last_btdevs);
            if ((mode == hs_2g4) || (mode == hs_wireless) || (mode == hs_none)) {
                wireless_devs_change(wireless_get_current_devs(), DEVS_2G4, true);
            }
        } break;
        case EE_CLR: {

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
        case KC_BT1: {
            uint8_t mode = confinfo.devs;
            hs_modeio_detection(true, &mode, confinfo.last_btdevs);
            if ((mode == hs_bt) || (mode == hs_wireless) || (mode == hs_none)) {
                WLS_KEYCODE_EXEC(DEVS_BT1);
                hs_rgb_blink_set_timer(timer_read32());
            }

        } break;
        case KC_BT2: {
            uint8_t mode = confinfo.devs;
            hs_modeio_detection(true, &mode, confinfo.last_btdevs);
            if ((mode == hs_bt) || (mode == hs_wireless) || (mode == hs_none)) {
                WLS_KEYCODE_EXEC(DEVS_BT2);
                hs_rgb_blink_set_timer(timer_read32());
            }
        } break;
        case KC_BT3: {
            uint8_t mode = confinfo.devs;
            hs_modeio_detection(true, &mode, confinfo.last_btdevs);
            if ((mode == hs_bt) || (mode == hs_wireless) || (mode == hs_none)) {
                WLS_KEYCODE_EXEC(DEVS_BT3);
                hs_rgb_blink_set_timer(timer_read32());
            }
        } break;
        case KC_2G4: {
            uint8_t mode = confinfo.devs;
            hs_modeio_detection(true, &mode, confinfo.last_btdevs);
            if ((mode == hs_2g4) || (mode == hs_wireless) || (mode == hs_none)) {
                WLS_KEYCODE_EXEC(DEVS_2G4);
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

    if (test_white_light_flag && record->event.pressed) {
        test_white_light_flag = false;
        rgb_matrix_set_color_all(0x00, 0x00, 0x00);
    }

    if (*md_getp_state() == MD_STATE_CONNECTED) {
        hs_rgb_blink_set_timer(timer_read32());
    }

    switch (keycode) {
        case MO(_FL):
        case MO(_MFL): {
           
            break;
        }
        case RGB_MOD:
            break;
        default: {
            
        } break;
    }

    return true;
}

bool L;
bool process_record_kb(uint16_t keycode, keyrecord_t *record) {

    if (process_record_user(keycode, record) != true) {
        return false;
    }

#ifdef WIRELESS_ENABLE
    if (process_record_wls(keycode, record) != true) {
        return false;
    }
#endif
    switch (keycode) {
        case KC_W:
        if (record->event.pressed) {
             if (L) 
             {
                register_code(KC_UP);
                return false;
             }
        }
        else
        {
            if (L) 
             {
                unregister_code(KC_UP);
                return false;
             }
        }  
        return true;
        break;
    case KC_A:
        if (record->event.pressed) {
            if (L) {
                register_code(KC_LEFT);
                return false;
            }
        }  
        else
        {
             if (L) {
                unregister_code(KC_LEFT);
                return false;
            }
        }
        return true;
        break;
    case KC_S:
        if (record->event.pressed) {
            if (L) 
            {
                register_code(KC_DOWN);
                return false;
            }
        }  
        else
        {
             if (L) {
                unregister_code(KC_DOWN);
                return false;
            }
        }
        return true;
        break;
    case KC_D:
        if (record->event.pressed) {
             if (L) {
                register_code(KC_RIGHT);
                return false;
            }
        }  
        else
        {
             if (L) {
                unregister_code(KC_RIGHT);
                return false;
            }
        }
        return true;
        break;
        case KC_SENS:
        if (record->event.pressed) {
             if (!L)  
             {
                L = true;
             }
             else{
                L = false;
             }
        }
        
        return false;
        break;
        case KC_APP:
            if (!record->event.pressed)
            {
                layer_move(0);
                if (keymap_config.no_gui) {
                    if (readPin(C5) == 0) tap_code16(KC_APP);
                    return false;
                }
                tap_code16(KC_APP);
            }
            else layer_move(2);
            return false;
        break;
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
        case NK_TOGG: {
           
            if (record->event.pressed) {
                rgb_matrix_hs_indicator_set(0xFF, (RGB){0x00, 0x6E, 0x00}, 250, 1);
            }
        } break;
        case EE_CLR: {
            if (record->event.pressed) {
                ee_clr_timer = timer_read32();
            } else {
                ee_clr_timer = 0;
            }

            return false;
        } break;
       
        case RGB_HUI: {
            if (record->event.pressed) {
                uint8_t now_mode = rgb_matrix_get_mode();

                if ((now_mode == 13) || (now_mode == 15) || (now_mode == 16) || (now_mode == 6)) {
                confinfo.rgb_hsv_index = (confinfo.rgb_hsv_index + 1) % 6;
                rgb_matrix_sethsv(rgb_hsvs1[confinfo.rgb_hsv_index][0],
                                    rgb_hsvs1[confinfo.rgb_hsv_index][1],
                                    rgb_matrix_get_val());
                } else {
                confinfo.rgb_hsv_index = (confinfo.rgb_hsv_index + 1) % 7;
                rgb_matrix_sethsv(rgb_hsvs[confinfo.rgb_hsv_index][0],
                                    rgb_hsvs[confinfo.rgb_hsv_index][1],
                                    rgb_matrix_get_val());
                }
                eeconfig_confinfo_update(confinfo.raw);
                record_color_hsv(true);
            }
            return false;
        } break;
        case RGB_SPD:
        if (record->event.pressed) {
            if (rgb_matrix_get_speed() <= RGB_MATRIX_SPD_STEP * 2) {
                rgb_matrix_set_speed(RGB_MATRIX_SPD_STEP);
               // led_blink_flag = true;
                return false;
            }
        }
        return true;
        break;
        case KC_RCTL:
        if (readPin(C5) == 0){
            if (record->event.pressed ) {
                register_code(KC_RALT);
            }
            else{
                unregister_code(KC_RALT);
            }
             return false;
        }
        return true;
        break;
    case KC_LALT:
        if (readPin(C5) == 0){
            if (record->event.pressed ) {
                register_code(KC_LGUI);
            }
            else{
                unregister_code(KC_LGUI);
            }
             return false;
        }
        return true;
        break;
     case KC_RALT:
        if (readPin(C5) == 0){
            if (record->event.pressed ) {
                register_code(KC_RGUI);
            }
            else{
                unregister_code(KC_RGUI);
            }
             return false;
        }
        return true;
        break;
    case KC_LGUI:
        if (readPin(C5) == 0){
            if (record->event.pressed ) {
                register_code(KC_LALT);
            }
            else{
                unregister_code(KC_LALT);
            }
             return false;
        }
        return true;
        break;
    case KC_RGUI:
        if (readPin(C5) == 0){
            if (record->event.pressed ) {
                register_code(KC_RALT);
            }
            else{
                unregister_code(KC_RALT);
            }
             return false;
        }
        return true;
        break;
        default:
            break;
    }

    return true;
}

void housekeeping_task_user(void) {
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
        eeconfig_confinfo_update(confinfo.raw);
    }

    switch (new_devs) {
        case DEVS_BT1: {
            if (reset) {
                rgb_matrix_wls_indicator_set(64, (RGB){HS_LBACK_COLOR_BT1}, 200, 1);
            } else {
                rgb_matrix_wls_indicator_set(64, (RGB){HS_PAIR_COLOR_BT1}, 500, 1);
            }
        } break;
        case DEVS_BT2: {
            if (reset) {
                rgb_matrix_wls_indicator_set(64, (RGB){HS_LBACK_COLOR_BT2}, 200, 1);
            } else {
                rgb_matrix_wls_indicator_set(64, (RGB){HS_PAIR_COLOR_BT2}, 500, 1);
            }
        } break;
        case DEVS_BT3: {
            if (reset) {
                rgb_matrix_wls_indicator_set(64, (RGB){HS_LBACK_COLOR_BT3}, 200, 1);
            } else {
                rgb_matrix_wls_indicator_set(64, (RGB){HS_PAIR_COLOR_BT3}, 500, 1);
            }
        } break;
        case DEVS_BT4: {
            if (reset) {
                rgb_matrix_wls_indicator_set(41, (RGB){RGB_BLUE}, 200, 1);
            } else {
                rgb_matrix_wls_indicator_set(41, (RGB){RGB_BLUE}, 500, 1);
            }
        } break;
        case DEVS_BT5: {
            if (reset) {
                rgb_matrix_wls_indicator_set(42, (RGB){RGB_BLUE}, 200, 1);
            } else {
                rgb_matrix_wls_indicator_set(42, (RGB){RGB_BLUE}, 500, 1);
            }
        } break;
        case DEVS_2G4: {
            if (reset) {
                rgb_matrix_wls_indicator_set(64, (RGB){HS_LBACK_COLOR_2G4}, 200, 1);
            } else {
                rgb_matrix_wls_indicator_set(64, (RGB){HS_LBACK_COLOR_2G4}, 500, 1);
            }
        } break;
        default:
            break;
    }
}

bool rgb_matrix_wls_indicator_cb(void) {

    if (*md_getp_state() != MD_STATE_CONNECTED) {
        wireless_devs_change_kb(wireless_get_current_devs(), wireless_get_current_devs(), wls_rgb_indicator_reset);
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
            rgb_matrix_set_color(wls_rgb_indicator_index, wls_rgb_indicator_rgb.r, wls_rgb_indicator_rgb.g, wls_rgb_indicator_rgb.b);
        } else {
            rgb_matrix_set_color(wls_rgb_indicator_index, 0x00, 0x00, 0x00);
        }
    }
}

void rgb_matrix_hs_bat_set(uint8_t index, RGB rgb, uint32_t interval, uint8_t times) {
    for (int i = 0; i < HS_RGB_BAT_COUNT; i++) {
        if (!hs_rgb_bat[i].active) {
            hs_rgb_bat[i].active   = true;
            hs_rgb_bat[i].timer    = timer_read32();
            hs_rgb_bat[i].interval = interval;
            hs_rgb_bat[i].times    = times * 2;
            hs_rgb_bat[i].index    = index;
            hs_rgb_bat[i].rgb      = rgb;
            break;
        }
    }
}

void rgb_matrix_hs_bat(void) {
    for (int i = 0; i < HS_RGB_BAT_COUNT; i++) {
        if (hs_rgb_bat[i].active) {
            if (timer_elapsed32(hs_rgb_bat[i].timer) >= hs_rgb_bat[i].interval) {
                hs_rgb_bat[i].timer = timer_read32();

                if (hs_rgb_bat[i].times) {
                    hs_rgb_bat[i].times--;
                }

                if (hs_rgb_bat[i].times <= 0) {
                    hs_rgb_bat[i].active = false;
                    hs_rgb_bat[i].timer  = 0x00;
                }
            }

            if (hs_rgb_bat[i].times % 2) {
                rgb_matrix_set_color(hs_rgb_bat[i].index, hs_rgb_bat[i].rgb.r, hs_rgb_bat[i].rgb.g, hs_rgb_bat[i].rgb.b);
            } else {
                rgb_matrix_set_color(hs_rgb_bat[i].index, 0x00, 0x00, 0x00);
            }
        }
    }
}

void bat_indicators(void) {
    static uint32_t battery_process_time = 0;

    if (charging_state && (bat_full_flag)) {
        rgb_matrix_set_color(HS_MATRIX_BLINK_INDEX_BAT, 0, 25, 0);
        battery_process_time = 0;
    } else if (charging_state) {

        battery_process_time = 0;
        rgb_matrix_set_color(HS_MATRIX_BLINK_INDEX_BAT, 25, 0, 0);
    } else if (*md_getp_bat() <= BATTERY_CAPACITY_LOW) {

        rgb_matrix_hs_bat_set(HS_MATRIX_BLINK_INDEX_BAT, (RGB){0x00, 25, 0x00}, 250, 1);

        if (*md_getp_bat() <= BATTERY_CAPACITY_STOP) {
            if (!battery_process_time) {
                battery_process_time = timer_read32();
            }

            if (battery_process_time && timer_elapsed32(battery_process_time) > 60000) {
                battery_process_time = 0;
                lower_sleep          = true;
                lpwr_set_timeout_manual(true);
            }
        }
    } else {
        rgb_matrix_set_color(HS_MATRIX_BLINK_INDEX_BAT, 0, 0, 0);
        battery_process_time = 0;
    }
}

#    endif

#endif

bool hs_reset_settings_user(void) {

    rgb_matrix_hs_indicator_set(0xFF, (RGB){0x10, 0x10, 0x10}, 250, 3);

    return true;
}

void nkr_indicators_hook(uint8_t index) {

    if ((hs_rgb_indicators[index].rgb.r == 0x6E) && (hs_rgb_indicators[index].rgb.g == 0x00) && (hs_rgb_indicators[index].rgb.b == 0x00)) {

        rgb_matrix_hs_indicator_set(0xFF, (RGB){0x6E, 0x00, 0x00}, 250, 1);

    } else if ((hs_rgb_indicators[index].rgb.r == 0x00) && (hs_rgb_indicators[index].rgb.g == 0x6E) && (hs_rgb_indicators[index].rgb.b == 0x00)) {

        rgb_matrix_hs_indicator_set(0xFF, (RGB){0x00, 0x00, 0x6F}, 250, 1);
    }
}

void rgb_matrix_hs_indicator_set(uint8_t index, RGB rgb, uint32_t interval, uint8_t times) {

    for (int i = 0; i < HS_RGB_INDICATOR_COUNT; i++) {
        if (!hs_rgb_indicators[i].active) {
            hs_rgb_indicators[i].active   = true;
            hs_rgb_indicators[i].timer    = timer_read32();
            hs_rgb_indicators[i].interval = interval;
            hs_rgb_indicators[i].times    = times * 2;
            hs_rgb_indicators[i].index    = index;
            hs_rgb_indicators[i].rgb      = rgb;
            if (index != 0xFF)
                hs_rgb_indicators[i].blink_cb = NULL;
            else {
                hs_rgb_indicators[i].blink_cb = nkr_indicators_hook;
            }
            break;
        }
    }
}

void rgb_matrix_hs_set_remain_time(uint8_t index, uint8_t remain_time) {

    for (int i = 0; i < HS_RGB_INDICATOR_COUNT; i++) {
        if (hs_rgb_indicators[i].index == index) {
            hs_rgb_indicators[i].times  = 0;
            hs_rgb_indicators[i].active = false;
            break;
        }
    }
}

void rgb_matrix_hs_indicator(void) {

    for (int i = 0; i < HS_RGB_INDICATOR_COUNT; i++) {
        if (hs_rgb_indicators[i].active) {
            if (timer_elapsed32(hs_rgb_indicators[i].timer) >= hs_rgb_indicators[i].interval) {
                hs_rgb_indicators[i].timer = timer_read32();

                if (hs_rgb_indicators[i].times) {
                    hs_rgb_indicators[i].times--;
                }

                if (hs_rgb_indicators[i].times <= 0) {
                    hs_rgb_indicators[i].active = false;
                    hs_rgb_indicators[i].timer  = 0x00;
                    if (hs_rgb_indicators[i].blink_cb != NULL)
                        hs_rgb_indicators[i].blink_cb(i);
                    continue;
                }
            }

            if ((hs_rgb_indicators[i].times % 2)) {
                if (hs_rgb_indicators[i].index == 0xFF) {
                    rgb_matrix_set_color_all(hs_rgb_indicators[i].rgb.r, hs_rgb_indicators[i].rgb.g, hs_rgb_indicators[i].rgb.b);
                } else {
                    rgb_matrix_set_color(hs_rgb_indicators[i].index, hs_rgb_indicators[i].rgb.r, hs_rgb_indicators[i].rgb.g, hs_rgb_indicators[i].rgb.b);
                }
            } else {
                if (hs_rgb_indicators[i].index == 0xFF) {
                    rgb_matrix_set_color_all(0x00, 0x00, 0x00);
                } else {
                    rgb_matrix_set_color(hs_rgb_indicators[i].index, 0x00, 0x00, 0x00);
                }
            }
        }
    }
}


bool rgb_matrix_indicators_advanced_kb(uint8_t led_min, uint8_t led_max) {

#ifdef RGBLIGHT_ENABLE
    if (rgb_matrix_indicators_advanced_user(led_min, led_max) != true) {

        return false;
    }
#endif

    if (ee_clr_timer && timer_elapsed32(ee_clr_timer) > 3000) {
        hs_reset_settings();
        ee_clr_timer = 0;
    }

    if (host_keyboard_led_state().caps_lock){
        setPinOutput(HS_LED_CAPS_PIN);
        writePin(HS_LED_CAPS_PIN, 1);
    }
    else{
        setPinOutput(HS_LED_CAPS_PIN);
        writePin(HS_LED_CAPS_PIN, 0);
    }
        
  


#ifdef WIRELESS_ENABLE
    rgb_matrix_wls_indicator();

    if (enable_bat_indicators && !inqbat_flag && !rgbrec_is_started()) {
        rgb_matrix_hs_bat();
        bat_indicators();
        bat_indicator_cnt = timer_read32();
    }

    if (!enable_bat_indicators) {
        if (timer_elapsed32(bat_indicator_cnt) > 2000) {
            enable_bat_indicators = true;
            bat_indicator_cnt     = timer_read32();
        }
    }

#endif

    rgb_matrix_hs_indicator();

    query();
    return true;
}

void hs_reset_settings(void) {
    enable_bat_indicators = false;
    eeconfig_init();
    eeconfig_update_rgb_matrix_default();


    keymap_config.raw = eeconfig_read_keymap();

#if defined(NKRO_ENABLE) && defined(FORCE_NKRO)
    keymap_config.nkro = 0;
    eeconfig_update_keymap(keymap_config.raw);
#endif

    // #if defined(WIRELESS_ENABLE)
    //     wireless_devs_change(wireless_get_current_devs(), DEVS_USB, false);
    // #endif

    if (hs_reset_settings_user() != true) {

        return;
    }
    hs_rgb_blink_set_timer(timer_read32());
    keyboard_post_init_kb();
}

void lpwr_wakeup_hook(void) {
    hs_mode_scan(false, confinfo.devs, confinfo.last_btdevs);

    gpio_write_pin_high(LED_POWER_EN_PIN);
    gpio_write_pin_high(A9);
}
