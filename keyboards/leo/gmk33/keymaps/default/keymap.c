// Copyright 2024 yangzheng20003 (@yangzheng20003)
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    [0] = LAYOUT( /* Base */
        KC_MUTE,   KC_ESC,   KC_GRV,   KC_1,     KC_2,     KC_3,     KC_4,     KC_5,               KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,   KC_BSPC, 
        MC_0,      KC_TAB,   KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,                         KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,     KC_LBRC,  KC_RBRC,  KC_BSLS,    
        MC_1,      KC_CAPS,  KC_A,     KC_S,     KC_D,     KC_F,     KC_G,                         KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,  KC_QUOT,            KC_ENT,         
        MC_2,      KC_LSFT,  KC_Z,     KC_X,     KC_C,     KC_V,     KC_B,               KC_B,     KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,  KC_RSFT,  KC_UP,    KC_DEL,
        MC_3,      KC_LCTL,  MO(1),    KC_LALT,            KC_SPC,                                 KC_SPC,             KC_RALT,  KC_RGUI,            KC_LEFT,  KC_DOWN,  KC_RGHT),

    [1] = LAYOUT( /* Base */
        _______,  _______,  _______,   KC_F1,    KC_F2,    KC_F3,    KC_F4,    KC_F5,              KC_F6,    KC_F7,    KC_F8,    KC_F9,      KC_F10,   KC_F11,    KC_F12,   _______,
        _______,  RGB_MOD,  _______,   _______,  _______,  _______,  _______,                      KC_PSCR,  KC_SCRL,  KC_PAUSE, _______,    _______,  RGB_HUD,   RGB_HUI,  _______,  
        _______,  _______,  _______,   _______,  _______,  _______,  _______,                      KC_HOME,  KC_END,   KC_PGUP,  KC_PGDN,    RGB_SAD,  RGB_SAI,             KC_END, 
        _______,  _______,  _______,   RGB_TOG,  _______,  _______,  _______,            _______,  RGB_HUI,  _______,  _______,  _______,    _______,  _______,   RGB_VAI,  KC_INS,
        _______,  _______,  _______,   _______,            _______,                                _______,            KC_RALT,  GU_TOGG,              RGB_SPD,   RGB_VAD,  RGB_SPI),
};

// clang-format on
