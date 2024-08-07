// Copyright 2024 SDK (@sdk66)
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    [0] = LAYOUT( /* Base */
        KC_ESC ,  KC_Q,      KC_W,      KC_E,     KC_R,     KC_T,     KC_Y,     KC_U,     KC_I,     KC_O,      KC_P,     KC_LBRC,  KC_RBRC, KC_BSPC, 
        KC_TAB,   KC_A,      KC_S,      KC_D,     KC_F,     KC_G,     KC_H,     KC_J,     KC_K,     KC_L,      KC_SCLN,  KC_QUOT,           KC_ENT,                           
        KC_LSFT,  KC_Z,      KC_X,      KC_C,     KC_V,     KC_B,     KC_N,     KC_M,     KC_COMM,  KC_DOT,    KC_SLSH,                     KC_RSFT,      
        KC_LCTL,  KC_LCMD,   KC_LALT,   KC_SPC,   KC_SPC,   KC_SPC,             KC_SPC,   KC_RALT,  KC_RCMD,   KC_RCTL,                     MO(1)
        ),  

    [1] = LAYOUT( /* Base */
        KC_ESC ,  KC_F1,     KC_F2,     KC_F3,    KC_F4,    KC_F5,    KC_F6,    KC_F7,    KC_F8,    KC_F9,     KC_F10,   KC_F11,   KC_F12, _______, 
        _______,  _______,   _______,   _______,  _______,  _______,  _______,  _______,  _______,  _______,   _______,  _______,          _______,                           
        _______,  _______,   _______,   _______,  _______,  _______,  _______,  _______,  _______,  _______,   _______,                    _______,      
        _______,  GU_TOGG,   _______,   _______,  _______,  _______,            _______,  _______,  _______,   _______,                    _______ 
        )
};

// clang-format on
