// Copyright 2025 Su
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

enum layers {
    _BL = 0,
    _FL,
    _MBL,
    _MFL,
    _FBL,
    _TEST,
};

enum {
    MD_SND_CMD_DEVCTRL_CHARGING      = 0x64,
    MD_SND_CMD_DEVCTRL_CHARGING_STOP = 0x65,
    MD_SND_CMD_DEVCTRL_CHARGING_DONE = 0x66,
};

enum im_keys {
    HS_MULT = QK_KB_0,
    BT_PRF1,
    BT_PRF2,
    BT_PRF3,
    BT_PRF4,
    BT_PRF5,
    OU_2P4G
};

