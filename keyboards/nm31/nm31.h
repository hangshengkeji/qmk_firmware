/* Copyright (C) 2023 Westberry Technology Corp., Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "quantum.h"

#ifdef MULTIMODE_ENABLE
#    include "multimode.h"
#endif

#ifdef RGB_MATRIX_BLINK_ENABLE
#    include "rgb_matrix_blink.h"
#endif

#ifdef IMMOBILE_ENABLE
#    include "immobile.h"
#endif

#ifdef SPLIT_KEYBOARD
#include "transactions.h"

typedef struct _master_to_slave_t {
    uint8_t cmd;
    uint8_t body[4];
} master_to_slave_t;

typedef struct _slave_to_master_t {
    uint8_t resp;
    uint8_t body[4];
} slave_to_master_t;

#endif

#define ___ KC_NO

// clang-format on
#define LAYOUT( \
   k00, k01, k02, k03, k04, k05, k06, k09, k0a, k0b, k0c, k0d, k0e, k0f, k0g, k0i, \
   k10, k11, k12, k13, k14, k15, k16, k19, k1a, k1b, k1c, k1d, k1e, k1f, k1g, k1i, \
   k20, k21, k22, k23,      k25, k26, k29, k2a, k2b, k2c, k2d, k2e,      k2g, k2i, \
   k30, k31, k32, k33,      k35, k36, k39, k3a, k3b, k3c, k3d, k3e, k3f, k3g, k3i, \
   k40, k41, k42, k43, k44, k45,      k49, k4a, k4b, k4c, k4d,      k4f, k4g, k4i,     \
   k50, k51, k52, k53,      k55,      k59, k5a, k5b, k5c, k5d,      k5f, k5g, k5i \
) \
{ \
  {k00, k01, k02, k03, k04, k05, k06, ___, ___}, \
  {k10, k11, k12, k13, k14, k15, k16, ___, ___}, \
  {k20, k21, k22, k23, k25, ___, k26, ___, ___}, \
  {k30, k31, k32, k33, k35, ___, k36, ___, ___}, \
  {k40, ___, k41, k42, k43, k44, k45, ___, ___,}, \
  {k50, k51, k52, k53, ___, k55, ___, ___, ___}, \
  {k09, k0a, k0b, k0c, k0d, k0e, k0f, k0g, k0i}, \
  {k19, k1a, k1b, k1c, k1d, k1e, k1f, k1g, k1i}, \
  {k29, k2a, k2b, k2c, k2d, k2e, k2g, ___, k2i}, \
  {k39, k3a, k3b, k3c, k3d, k3e, k3f, k3g, k3i}, \
  {k49, k4a, k4b, k4c, k4d, k4f, k4g, ___, k4i}, \
  {k59, k5a, k5b, k5c, k5d, ___, k5f, k5g, k5i}  \
}

enum safe_key {
    RP_P0 = IM_USER, // 灯效录制通道0
    RP_P1,           // 灯效录制通道1
    RP_P2,           // 灯效录制通道2
    RP_TOG,          // 灯效录制开关翻转
    KC_ONEW,         // 一键白光
    KC_BTPR,         // BT免密配对
    KC_FILP,
    LWIN_TAB,
    LWIN_E,
    HS_MUTE,
    HS_VOLD,
    HS_VOLU,
    LED_BL,
    BT_TEST
};
enum layers {
    _LB = 0,
    _LF,
    _LM,
    _LMF,
    _TEST,
};

