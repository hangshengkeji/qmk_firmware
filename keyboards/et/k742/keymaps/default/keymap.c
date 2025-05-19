// Copyright 2024 yangzheng20003 (@yangzheng20003)
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include "rgb_record/rgb_record.h"

enum layers {
    _BL = 0,
    _FL,
    _MBL,
    _MFL,
    _FBL,
};

#define ______ HS_BLACK

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    [_BL] = LAYOUT( /* Base */
        KC_ESC,   KC_F1,    KC_F2,    KC_F3,    KC_F4,    KC_F5,    KC_F6,    KC_F7,    KC_F8,    KC_F9,    KC_F10,   KC_F11,   KC_F12,   KC_DEL,   KC_INS,   KC_PGUP,  KC_PGDN,  MM_KEY,  
        KC_GRV,   KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,   KC_BSPC,  KC_NUM,   KC_PSLS,  KC_PAST,  KC_PMNS,           
        KC_TAB,   KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,     KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,     KC_LBRC,  KC_RBRC,  KC_BSLS,  KC_P7,    KC_P8,    KC_P9,    KC_PPLS,           
        KC_CAPS,  KC_A,     KC_S,     KC_D,     KC_F,     KC_G,     KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,  KC_QUOT,  KC_BSLS,  KC_ENTER, KC_P4,    KC_P5,    KC_P6,              
        KC_LSFT,  KC_NUBS,  KC_Z,     KC_X,     KC_C,     KC_V,     KC_B,     KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,  KC_RSFT,  KC_UP,    KC_P1,    KC_P2,    KC_P3,    KC_PENT, 
        KC_LCTL,  KC_LGUI,  KC_LALT,                                KC_SPC,                       KC_RALT,  MO(_FL),  KC_RCTL,  KC_LEFT,  KC_DOWN,  KC_RGHT,  KC_P0,    KC_PDOT    ),

    [_FL] = LAYOUT( /* Base */
        EE_CLR,   KC_BRID,  KC_BRIU,  HS_TASK,  HS_VOICE, HS_DKT,   KC_WSCH,  KC_MPRV,  KC_MPLY,  KC_MNXT,  KC_MUTE,  KC_VOLD,  KC_VOLU,  HS_BATQ,  RGB_MOD,  RGB_HUI,  _______,  MM_FN_KEY,
        _______,  KC_BT1,   KC_BT2,   KC_BT3,  KC_2G4,    _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  RP_P0,    _______,  _______,  _______,  _______,
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  KC_HOME,  KC_END,   KC_SCRL,  _______,  _______,  _______,  _______,  _______,
        KC_ENCW,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  KC_PSCR,  KC_PAUSE, _______,  _______,  _______,  _______,  _______, 
        _______,  _______,  _______,  _______,  _______,  _______,  NK_TOGG,  _______,  _______,  _______,  _______,  _______, MO(_FBL),  RGB_VAI,  _______,  _______,  _______,  _______, 
        _______,  GU_TOGG,  _______,                      _______,                                _______,  _______,  _______,  RGB_SPD,  RGB_VAD,  RGB_SPI,  _______,  _______ ),

    [_MBL] = LAYOUT( /* Base */
        KC_ESC,   KC_BRID,  KC_BRIU,  KC_MCTL,  HS_SIRI,  KC_F11,   HS_WSCH,  KC_MPRV,  KC_MPLY,  KC_MNXT,  KC_MUTE,  KC_VOLD,  KC_VOLU,  KC_DEL,   KC_INS,   KC_PGUP,  KC_PGDN,  MM_KEY,
        KC_GRV,   KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,   KC_BSPC,  KC_NUM,   KC_PSLS,  KC_PAST,  KC_PMNS,           
        KC_TAB,   KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,     KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,     KC_LBRC,  KC_RBRC,  KC_BSLS,  KC_P7,    KC_P8,    KC_P9,    KC_PPLS,           
        KC_CAPS,  KC_A,     KC_S,     KC_D,     KC_F,     KC_G,     KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,  KC_QUOT,  KC_BSLS,  KC_ENTER, KC_P4,    KC_P5,    KC_P6,              
        KC_LSFT,  KC_NUBS,  KC_Z,     KC_X,     KC_C,     KC_V,     KC_B,     KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,  KC_RSFT,  KC_UP,    KC_P1,    KC_P2,    KC_P3,    KC_PENT, 
        KC_LCTL,  KC_LALT,  KC_LGUI,                                KC_SPC,                       KC_RGUI,  MO(_MFL), KC_RALT,  KC_LEFT,  KC_DOWN,  KC_RGHT,  KC_P0,    KC_PDOT  ),

    [_MFL] = LAYOUT( /* Base */
        EE_CLR,   KC_F1,    KC_F2,      KC_F3,    KC_F4,    KC_F5,    KC_F6,    KC_F7,    KC_F8,    KC_F9,   KC_F10,   KC_F11,  KC_F12,   HS_BATQ,  RGB_MOD,  RGB_MOD,  _______, MM_FN_KEY,
        _______,  KC_BT1,   KC_BT2,   KC_BT3,    KC_2G4,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  RP_P0,    _______,  _______,  _______, _______,
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  KC_HOME,  KC_END,   KC_SCRL,  _______,  _______,  _______,  _______, _______, 
        KC_ENCW,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  KC_PSCR,  KC_PAUSE, _______,  _______,  _______,  _______,  _______, 
        _______,  _______,  _______,  _______,  _______,  _______,  NK_TOGG,  _______,  _______,  _______,  _______,  _______,  MO(_FBL), RGB_VAI,  _______,  _______,  _______, _______, 
        _______,  KC_LALT,  KC_LGUI,                      _______,                                _______,  _______,  _______,  RGB_SPD,  RGB_VAD,  RGB_SPI,  _______,  _______),
    [_FBL] = LAYOUT( /* Base */ 
        QK_BOOT,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______, _______, _______, _______, 
        _______,  _______,  BT_TEST,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______, _______, _______, _______, 
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______, _______, _______, _______, 
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______, _______, _______,      
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,_______, _______, _______, 
        _______,  _______,  _______,                      _______,                                _______,  _______,  _______,  _______,  _______,  _______, _______, _______)
};

	const uint16_t PROGMEM rgbrec_default_effects[RGBREC_CHANNEL_NUM][MATRIX_ROWS][MATRIX_COLS] = {
        [0] = LAYOUT(
       HS_GREEN, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________,________, ________, ________,
       ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________,________, ________, ________,
       ________, ________, HS_GREEN, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________,________, ________, ________,
       ________, HS_GREEN, HS_GREEN, HS_GREEN, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________,________, ________,         
       ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, HS_GREEN, ________,________, ________, ________, 
       ________, ________, ________,                     ________,                               ________, ________, ________,  HS_GREEN, HS_GREEN, HS_GREEN,________, ________),

    [1] = LAYOUT(
       HS_GREEN, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________,________, ________, ________,
       ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________,________, ________, ________,
       ________, ________, HS_GREEN, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________,________, ________, ________,
       ________, HS_GREEN, HS_GREEN, HS_GREEN, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________,________, ________,         
       ________, ________,________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, HS_GREEN,________, ________, ________, 
       ________, ________, ________,                     ________,                               ________, ________, ________,  HS_GREEN, HS_GREEN, HS_GREEN,________, ________),

    [2] = LAYOUT(
       HS_GREEN, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________,________, ________, ________,
       ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________,________, ________, ________,
       ________, ________, HS_GREEN, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________,________, ________, ________,
       ________, HS_GREEN, HS_GREEN, HS_GREEN, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________,________, ________,         
       ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, ________, HS_GREEN,________, ________, ________, 
       ________, ________, ________,                     ________,                               ________, ________, ________,  HS_GREEN, HS_GREEN, HS_GREEN,________, ________),
};

#ifdef ENCODER_MAP_ENABLE
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {
    [0] = {ENCODER_CCW_CW(KC_ENCCW, KC_EN)},
    [1] = {ENCODER_CCW_CW(KC_ENCCW, KC_EN)},
    [2] = {ENCODER_CCW_CW(KC_ENCCW, KC_EN)},
    [3] = {ENCODER_CCW_CW(KC_ENCCW, KC_EN)},
    [4] = {ENCODER_CCW_CW(KC_ENCCW, KC_EN)},
};
#endif

const aw_led g_aw_leds[RGB_MATRIX_LED_COUNT] = {
/* Refer to IS31 manual for these locations
 *   driver
 *   |  R location
 *   |  |          G location
 *   |  |          |          B location
 *   |  |          |          | */
    {0, CS18_SW1,  CS17_SW1,  CS16_SW1  },  // 0  Esc
    {0, CS18_SW2,  CS17_SW2,  CS16_SW2  },  // 1  F1
    {0, CS18_SW3,  CS17_SW3,  CS16_SW3  },  // 2  F2
    {0, CS18_SW4,  CS17_SW4,  CS16_SW4  },  // 3  F3
    {0, CS18_SW5,  CS17_SW5,  CS16_SW5  },  // 4  F4
    {0, CS18_SW6,  CS17_SW6,  CS16_SW6  },  // 5  F5
    {0, CS18_SW7,  CS17_SW7,  CS16_SW7  },  // 6  F6
    {0, CS18_SW8,  CS17_SW8,  CS16_SW8  },  // 7  F7
    {0, CS18_SW9,  CS17_SW9,  CS16_SW9  },  // 8  F8
    {0, CS18_SW10, CS17_SW10, CS16_SW10  },  // 9  F9
    {0, CS18_SW11, CS17_SW11, CS16_SW11  },  // 10 F10
    {0, CS18_SW12, CS17_SW12, CS16_SW12  },  // 11 F11
    {1, CS18_SW1,  CS17_SW1,  CS16_SW1   },  // 12 F12
    {1, CS18_SW2,  CS17_SW2,  CS16_SW2   },  // 13 DEL
    {1, CS18_SW4,  CS17_SW4,  CS16_SW4  },  // 14 MUTE
    {1, CS18_SW5,  CS17_SW5,  CS16_SW5  },  // 15 MUTE
    {1, CS18_SW6,  CS17_SW6,  CS16_SW6  },  // 16 MUTE

    {0, CS15_SW1,  CS14_SW1,  CS13_SW1  },  // 17 MUTE
    {0, CS15_SW2,  CS14_SW2,  CS13_SW2  },  // 18 1
    {0, CS15_SW3,  CS14_SW3,  CS13_SW3  },  // 19 2
    {0, CS15_SW4,  CS14_SW4,  CS13_SW4  },  // 20 3
    {0, CS15_SW5,  CS14_SW5,  CS13_SW5  },  // 21 4
    {0, CS15_SW6,  CS14_SW6,  CS13_SW6  },  // 22 5
    {0, CS15_SW7,  CS14_SW7,  CS13_SW7  },  // 23 6
    {0, CS15_SW8,  CS14_SW8,  CS13_SW8  },  // 24 7
    {0, CS15_SW9,  CS14_SW9,  CS13_SW9  },  // 25 8
    {0, CS15_SW10, CS14_SW10, CS13_SW10   },  // 26 9
    {0, CS15_SW11, CS14_SW11, CS13_SW11   },  // 27 0
    {0, CS15_SW12, CS14_SW12, CS13_SW12   },  // 28 -_
    {1, CS15_SW1,  CS14_SW1,  CS13_SW1   },  // 29 =+
    {1, CS15_SW2,  CS14_SW2,  CS13_SW2   },  // 30 BSPC
    {1, CS15_SW4,  CS14_SW4,  CS13_SW4   },  // 31 INS
    {1, CS15_SW5,  CS14_SW5,  CS13_SW5   },  // 32 HOME
    {1, CS15_SW6,  CS14_SW6,  CS13_SW6   },  // 33 PGUP
    {1, CS15_SW7,  CS14_SW7,  CS13_SW7   },  // 34 Tab

    {0, CS12_SW1,  CS11_SW1,   CS10_SW1   },  // 35 Q
    {0, CS12_SW2,  CS11_SW2,   CS10_SW2   },  // 36 W
    {0, CS12_SW3,  CS11_SW3,   CS10_SW3   },  // 37 E
    {0, CS12_SW4,  CS11_SW4,   CS10_SW4   },  // 38 R
    {0, CS12_SW5,  CS11_SW5,   CS10_SW5   },  // 39 T
    {0, CS12_SW6,  CS11_SW6,   CS10_SW6   },  // 40 Y
    {0, CS12_SW7,  CS11_SW7,   CS10_SW7   },  // 41 U
    {0, CS12_SW8,  CS11_SW8,   CS10_SW8  },  // 42 I
    {0, CS12_SW9,  CS11_SW9,   CS10_SW9  },  // 43 O
    {0, CS12_SW10, CS11_SW10,  CS10_SW10  },  // 44 P
    {0, CS12_SW11, CS11_SW11,  CS10_SW11  },  // 45 [{
    {0, CS12_SW12, CS11_SW12,  CS10_SW12  },  // 46 ]}
    {1, CS12_SW1,  CS11_SW1,  CS10_SW1  },  // 47 \|
    {1, CS12_SW2,  CS11_SW2,  CS10_SW2  },  // 48 Del
    {1, CS12_SW3,  CS11_SW3,  CS10_SW3  },  // 49 End
    {1, CS12_SW4,  CS11_SW4,  CS10_SW4  },  // 50 PGDN
    {1, CS12_SW5,  CS11_SW5,  CS10_SW5  },  // 51 Caps
    {1, CS12_SW6,  CS11_SW6,  CS10_SW6  },  // 52 A
    {1, CS12_SW7,  CS11_SW7,  CS10_SW7  },  // 53 S
    
    {0, CS1_SW1,  CS2_SW1,  CS3_SW1  },  // 54 D
    {0, CS1_SW2,  CS2_SW2,  CS3_SW2  },  // 55 F
    {0, CS1_SW3,  CS2_SW3,  CS3_SW3 },  // 56 G
    {0, CS1_SW4,  CS2_SW4,  CS3_SW4 },  // 57 H
    {0, CS1_SW5,  CS2_SW5,  CS3_SW5 },  // 58 J
    {0, CS1_SW6,  CS2_SW6,  CS3_SW6  },  // 59 K
    {0, CS1_SW7,  CS2_SW7,  CS3_SW7   },  // 60 L
    {0, CS1_SW8,  CS2_SW8,  CS3_SW8   },  // 61 ;:
    {0, CS1_SW9,  CS2_SW9,  CS3_SW9  },  // 62 #~
    {0, CS1_SW10, CS2_SW10, CS3_SW10  },  //63
    {0, CS1_SW11, CS2_SW11, CS3_SW11  },  //64
    {0, CS1_SW12, CS2_SW12, CS3_SW12  },  //65
    {1, CS1_SW1, CS2_SW1,  CS3_SW1  },  //66
    {1, CS1_SW2, CS2_SW2,  CS3_SW2  },  //67
    {1, CS1_SW3, CS2_SW3,  CS3_SW3  },  //68
    {1, CS1_SW4, CS2_SW4,  CS3_SW4  },  //69
    {1, CS1_SW5, CS2_SW5,  CS3_SW5  },  //70
    {1, CS1_SW6, CS2_SW6,  CS3_SW6  },  //71

    {0, CS4_SW1,  CS5_SW1,  CS6_SW1  },  //72
    {0, CS4_SW2,  CS5_SW2,  CS6_SW2  },  //73
    {0, CS4_SW3,  CS5_SW3,  CS6_SW3  },  //74
    {0, CS4_SW4,  CS5_SW4,  CS6_SW4  },  //75
    {0, CS4_SW5,  CS5_SW5,  CS6_SW5  },  //76
    {0, CS4_SW6,  CS5_SW6,  CS6_SW6  },  //77
    {0, CS4_SW7,  CS5_SW7,  CS6_SW7  },  //78
    {0, CS4_SW8,  CS5_SW8,  CS6_SW8  },  //79
    {0, CS4_SW9,  CS5_SW9,  CS6_SW9  },  //80
    {0, CS4_SW10, CS5_SW10, CS6_SW10  },  //81
    {0, CS4_SW11, CS5_SW11, CS6_SW11  },  //82
    {0, CS4_SW12, CS5_SW12, CS6_SW12  },  //83
    {1, CS4_SW1, CS5_SW1, CS6_SW1  },  //84
    {1, CS4_SW2, CS5_SW2, CS6_SW2  },  //85
    {1, CS4_SW3, CS5_SW3, CS6_SW3  },  //86
    {1, CS4_SW5, CS5_SW5, CS6_SW5  },  //87
    {1, CS4_SW6, CS5_SW6, CS6_SW6  },  //88
    {1, CS4_SW7, CS5_SW7, CS6_SW7  },  //89

    {0, CS7_SW1,  CS8_SW1,  CS9_SW1  },  //90
    {0, CS7_SW2,  CS8_SW2,  CS9_SW2  },  //91
    {0, CS7_SW3,  CS8_SW3,  CS9_SW3  },  //92
    {0, CS7_SW5,  CS8_SW5,  CS9_SW5  },  //93
    {0, CS7_SW6,  CS8_SW6,  CS9_SW6  },  //94
    {0, CS7_SW7,  CS8_SW7,  CS9_SW7  },  //95
    {0, CS7_SW8,  CS8_SW8,  CS9_SW8  },  //96
    {0, CS7_SW9,  CS8_SW9,  CS9_SW9  },  //97
    {0, CS7_SW10, CS8_SW10, CS9_SW10  },  //98
    {0, CS7_SW11, CS8_SW11, CS9_SW11  },  //99
    {0, CS7_SW12, CS8_SW12, CS9_SW12 },  //100
    {1, CS7_SW1, CS8_SW1, CS9_SW1  },  //101
    {1, CS7_SW2, CS8_SW2, CS9_SW2  },  //102
    {1, CS7_SW3, CS8_SW3, CS9_SW3  },  //103
    {1, CS7_SW5, CS8_SW5, CS9_SW5  },  //104
    {1, CS7_SW6, CS8_SW6, CS9_SW6 },  //105
};
// clang-format on
