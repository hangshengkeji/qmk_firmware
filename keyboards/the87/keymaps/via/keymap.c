// Copyright 2024 Su (@isuua)
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include "rgb_record.h"

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    [_BL] = LAYOUT( /* Base */
        KC_ESC,             KC_F1,    KC_F2,    KC_F3,    KC_F4,    KC_F5,    KC_F6,    KC_F7,    KC_F8,    KC_F9,    KC_F10,   KC_F11,   KC_F12,   KC_PSCR, KC_SCRL,  KC_PAUSE, 
        KC_GRV,   KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,   KC_BSPC,  KC_INS,  KC_HOME,  KC_PGUP,
        KC_TAB,   KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,     KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,     KC_LBRC,  KC_RBRC,  KC_BSLS,  KC_DEL,  KC_END,   KC_PGDN,       
        KC_CAPS,  KC_A,     KC_S,     KC_D,     KC_F,     KC_G,     KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,  KC_QUOT,            KC_ENT,   
        KC_LSFT,  KC_Z,     KC_X,     KC_C,     KC_V,     KC_B,     KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,            KC_RSFT,                     KC_UP,     
        KC_LCTL,  KC_LGUI,  KC_LALT,                      KC_SPC,                                 KC_RALT,  MO(_FL),            KC_APP,   KC_RCTL,  KC_LEFT, KC_DOWN,  KC_RGHT),

    [_FL] = LAYOUT( /* Base */
        EE_CLR,             KC_MYCM,  KC_WHOM,  KC_MAIL,  KC_CALC,  KC_MSEL,  KC_MSTP,  KC_MPRV,  KC_MPLY,  KC_MNXT,  KC_MUTE,  KC_VOLD, KC_VOLU,   _______,  _______,  _______,  
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  MUS_CUT,  MUS_STA,  _______,  _______,
        _______,  HS_PRF1,  HS_PRF2,  HS_PRF3,  HS_2P4G,  _______,  _______,  _______,  _______,  _______,  _______,  RL_MOD,  _______,  UG_NEXT,  _______,  _______,  _______,    
        _______,  _______,  _______,  HS_DEB,   _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            UG_HUEU,                
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  TO(_MBL), _______,  _______,  _______,            MO(_TEST),                    UG_VALU,             
        HS_DKT,   GU_TOGG,  _______,                     HS_BATQ,                                  _______,  _______,            _______,  _______,  UG_SPDD,  UG_VALD,  UG_SPDU ),

    [_MBL] = LAYOUT( /* Base */
        KC_ESC,             KC_F1,    KC_F2,    KC_F3,    KC_F4,    KC_F5,    KC_F6,    KC_F7,    KC_F8,    KC_F9,    KC_F10,   KC_F11,   KC_F12,   KC_PSCR, KC_SCRL,  KC_PAUSE, 
        KC_GRV,   KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,   KC_BSPC,  KC_INS,  KC_HOME,  KC_PGUP,
        KC_TAB,   KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,     KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,     KC_LBRC,  KC_RBRC,  KC_BSLS,  KC_DEL,  KC_END,   KC_PGDN,       
        KC_CAPS,  KC_A,     KC_S,     KC_D,     KC_F,     KC_G,     KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,  KC_QUOT,            KC_ENT,   
        KC_LSFT,  KC_Z,     KC_X,     KC_C,     KC_V,     KC_B,     KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,            KC_RSFT,                     KC_UP,     
        KC_LCTL,  KC_LALT,  KC_LGUI,                      KC_SPC,                                 KC_RGUI,  MO(_MFL),           KC_APP,   KC_RCTL,  KC_LEFT, KC_DOWN,  KC_RGHT),

    [_MFL] = LAYOUT( /* Base */
        EE_CLR,             KC_BRID,  KC_BRIU,  HS_TASK,  KC_MCTL,   KC_NO,    KC_NO,   KC_MPRV,  KC_MPLY,  KC_MNXT,  KC_MUTE,  KC_VOLD, KC_VOLU,  _______,  _______,  _______,    
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______, _______,   MUS_CUT,  MUS_STA, _______,  _______,  
        _______,  HS_PRF1,  HS_PRF2,  HS_PRF3,  HS_2P4G,  _______,  _______,  _______,  _______,  _______,  _______,  RL_MOD,   _______,  UG_NEXT,  _______, _______,  _______, 
        _______,  _______,  _______,  HS_DEB,   _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            UG_HUEU,               
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  TO(_MBL), _______,  _______,  _______,            MO(_TEST),                    UG_VALU,              
        HS_DKT,   _______,  _______,                      HS_BATQ,                                 _______,  _______,            _______,  _______,  UG_SPDD,  UG_VALD,  UG_SPDU),

    [_TEST] = LAYOUT( /* Base */ 
        QK_BOOT,            _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,   _______,  _______,  _______,  _______,    
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,   _______,  _______,  _______,  _______,  
        _______,  _______,  BT_TEST,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,   _______,  _______,  _______,  _______, 
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,             _______,               
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            _______,                       _______,              
        _______,  _______,  _______,                      _______,                                _______,  _______,            _______,   _______,  _______,  _______,  _______),

};

// clang-format on

const uint16_t PROGMEM rgbrec_default_effects[RGBREC_CHANNEL_NUM][MATRIX_ROWS][MATRIX_COLS] = {
    0
};

#ifdef ENCODER_MAP_ENABLE
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][2] = {
    [0] = {ENCODER_CCW_CW(KC_VOLD, KC_VOLU),ENCODER_CCW_CW(KC_VOLD, KC_VOLU)},
    [1] = {ENCODER_CCW_CW(_______, _______),ENCODER_CCW_CW(_______, _______)},
    [2] = {ENCODER_CCW_CW(_______, _______),ENCODER_CCW_CW(_______, _______)},
    [3] = {ENCODER_CCW_CW(_______, _______),ENCODER_CCW_CW(_______, _______)},
    [4] = {ENCODER_CCW_CW(_______, _______),ENCODER_CCW_CW(_______, _______)}};
#endif
// clang-format on
