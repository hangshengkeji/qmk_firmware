/* Copyright 2021 Glorious, LLC <salman@pcgamingrace.com>
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

#include QMK_KEYBOARD_H
#include "immobile.h"
#include "i2c_master.h"
#include "usb_main.h"
#include "eeprom.h"
#include "wireless.h"
#include "record.h"

#ifdef GPIO_UART_ENABLE
#    include "iprint.h"
#    include <stdarg.h>
#    define IDEBUG_USER_INFO IDEBUG_INFO
#else
#    define IDEBUG_USER_INFO(fmt, ...)
#endif
#define LY_WBS TO(_LB)
#define LY_WFN MO(_LF)
#define LY_MBS TO(_LM)
#define LY_MFN MO(_LMF)
static const uint8_t rgbmatrix_buff[] = {13, 15, 16, 24, 25, 26, 29, 37, 33, 34, 35, 43, 2, 5, 6, 9};

static const PWMConfig hs_pwm_config = {
    .frequency = HS_PWM_FREQUENCY,
    .period    = HS_PWM_PERIOD, // Mit dieser Periode wird UDE-Event erzeugt und ein neuer Wert (L?nge WS2812_BIT_N) vom DMA ins CCR geschrieben
    .callback  = NULL,
    .channels =
        {
            [0 ... 3]                = {.mode = PWM_OUTPUT_DISABLED, .callback = NULL},    // Channels default to disabled
            [HS_PWM_CHANNEL_3 - 1]     = {.mode = PWM_OUTPUT_ACTIVE_HIGH, .callback = NULL}, // Turn on the channel we care about
            [HS_PWM_CHANNEL_2 - 1]     = {.mode = PWM_OUTPUT_ACTIVE_HIGH, .callback = NULL}, // Turn on the channel we care about

        },
    .cr2       = 0,
    .dier      = TIM_DIER_UDE, // DMA on update event for next period
};

static const PWMConfig hs_pwm_config1 = {
    .frequency = HS_PWM_FREQUENCY,
    .period    = HS_PWM_PERIOD, // Mit dieser Periode wird UDE-Event erzeugt und ein neuer Wert (L?nge WS2812_BIT_N) vom DMA ins CCR geschrieben
    .callback  = NULL,
    .channels =
        {
            [0 ... 3]                = {.mode = PWM_OUTPUT_DISABLED, .callback = NULL},    // Channels default to disabled
            [HS_PWM_CHANNEL_1 - 1]     = {.mode = PWM_OUTPUT_ACTIVE_HIGH, .callback = NULL}, // Turn on the channel we care about
        },
    .cr2       = 0,
    .dier      = TIM_DIER_UDE, // DMA on update event for next period
};

static bool im_bat_req_flag = false;
 static bool im_bat_req_charging_flag = false;
// static bool ntf = false;
static bool usb_connect_ok_flag = false;
uint8_t led_brightness = 100;
bool FN_status;
static uint8_t i,r,g,b,dir;
int8_t BY6102_ReadSOC(void);
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_LB] = LAYOUT( /* Base */
        KC_ESC,   KC_F1,    KC_F2,    KC_F3,    KC_F4,    KC_F5,    KC_F6,    KC_F7,    KC_F8,    KC_F9,    KC_F10,  KC_F11,   KC_F12,    KC_INS,   KC_DEL,  KC_MUTE,
        KC_GRV,   KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,   KC_INT3,  KC_BSPC, KC_HOME,
        KC_TAB,   KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,     KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,     KC_LBRC,  KC_RBRC,                     KC_PGUP,
        KC_CAPS,  KC_A,     KC_S,     KC_D,     KC_F,     KC_G,     KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,  KC_QUOT,            KC_BSLS,  KC_ENT,  KC_PGDN,
        KC_LSFT,  KC_Z,     KC_X,     KC_C,     KC_V,     KC_B,     KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,  KC_INT1, KC_RSFT,             KC_UP,
        KC_LCTL,  KC_LGUI,  KC_LALT, KC_INT5,   KC_SPC,                                 KC_SPC,   KC_INT4,  KC_INT2,  MO(_LF),  KC_RCTL,  KC_LEFT,  KC_DOWN, KC_RGHT),

    [_LF] = LAYOUT( /* Base */
        KC_LED,  _______,  _______,   _______,  _______,  _______,  _______, KC_MPRV,  KC_MPLY,  KC_MNXT,  KC_MUTE,  KC_VOLD,   KC_VOLU,   LED_BL, _______, RGB_MOD,
        _______, _______,  _______,   _______,  _______,  _______, _______,  IM_BT1,   IM_BT2,   IM_BT3,    IM_2G4,   RGB_MOD,  RGB_TOG,  _______, _______, KC_END,
        _______, _______,  _______,   _______,  HS_SIRI,  _______, KC_CALC,  _______,  KC_UP,    _______,   KC_PSCR,  KC_SCRL,  KC_PAUSE,                   _______,
        _______, _______,  _______,   _______,  _______,  _______, _______,  KC_LEFT,  KC_DOWN,  KC_RGHT,   RGB_HUD,  RGB_HUI,            _______, _______, _______,
        MO(_TEST), KC_APP, _______,   _______,  _______,  NK_TOGG, TO(_LB),  TO(_LM),  _______,   RGB_SAD,  RGB_SAI,  _______,  MM_BATQ,            RGB_VAI,
        _______, HS_TOGG,  _______,  _______,  _______,                                _______,  _______,   _______,  _______,  _______,  RGB_SPD, RGB_VAD, RGB_SPI),

    [_LM] = LAYOUT( /* Base */
        KC_ESC,   KC_BRID,  KC_BRIU,  CONTROL,  HS_LPAD,  KC_NO,   KC_NO,    KC_MPRV,  KC_MPLY,  KC_MNXT,  KC_MUTE,  KC_VOLD,  KC_VOLU,   KC_INS,   KC_DEL,   KC_MUTE,
        KC_GRV,   KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,   KC_INT3,  KC_BSPC,  KC_HOME,
        KC_TAB,   KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,     KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,     KC_LBRC,  KC_RBRC,                      KC_PGUP,
        KC_CAPS,  KC_A,     KC_S,     KC_D,     KC_F,     KC_G,     KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,  KC_QUOT,            KC_BSLS,  KC_ENT,   KC_PGDN,
        KC_LSFT,  KC_Z,     KC_X,     KC_C,     KC_V,     KC_B,     KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,  KC_INT1,  KC_RSFT,            KC_UP,
        KC_LCTL,  KC_LALT,  KC_LGUI, KC_LNG2,   KC_SPC,                                 KC_SPC,   KC_LNG1,  KC_RGUI,  MO(_LMF), KC_RCTL,  KC_LEFT,  KC_DOWN,  KC_RGHT),
     [_LMF] = LAYOUT( /* Base */
        KC_LED,   KC_F1,    KC_F2,     KC_F3,    KC_F4,    KC_F5,   KC_F6,    KC_F7,    KC_F8,   KC_F9,    KC_F10,  KC_F11,   KC_F12,   LED_BL,  _______, RGB_MOD,
        _______, _______,  _______,   _______,  _______,  _______, _______,  IM_BT1,   IM_BT2,  IM_BT3,    IM_2G4,  RGB_MOD,  RGB_TOG,  _______, _______,  KC_END,
        _______, _______,  _______,   _______,  HS_SIRI,  _______, KC_CALC,  _______,  KC_UP,   _______,   KC_PSCR, KC_SCRL,  KC_PAUSE,                    _______,
        _______, _______,  _______,   _______,  _______,  _______, _______,  KC_LEFT,  KC_DOWN, KC_RGHT,   RGB_HUD, RGB_HUI,            _______,  _______, _______,
        MO(_TEST), KC_APP, _______,   _______,  _______,  NK_TOGG, TO(_LB),  TO(_LM),  _______, RGB_SAD,   RGB_SAI,   _______, MM_BATQ,            RGB_VAI,
        _______, _______,  _______,   _______,  _______,                               _______, _______,   _______, _______,  _______,  RGB_SPD,  RGB_VAD, RGB_SPI),
    [_TEST] = LAYOUT( /* Base */
        _______, _______,  _______,   _______,  _______,  _______, _______,  _______,  _______, _______,   _______, _______,  _______,  _______,  _______, _______,
        _______, _______,  _______,   _______,  _______,  _______, _______,  _______,  _______, _______,   _______, _______,  _______,  _______,  _______, _______,
        _______, _______,  _______,   _______,  _______,  _______, _______,  _______,  _______, _______,   _______, _______,  _______,                     _______,
        _______, _______,  _______,   _______,  _______,  _______, _______,  _______,  _______, _______,   _______, _______,            _______,   _______, _______,
        _______, _______,  _______,   _______,  _______,  _______, _______,  _______,  _______, _______,  _______,  _______,  _______,             _______,
        _______, _______,  _______,   _______,  _______,                               _______, _______,  _______, _______,  _______,  _______,  _______, _______),
};

im_lkey_t lkey_define_user[IM_LKEY_COUNT] = {
    {.keycode = HS_SIRI,  .hole_time = 3000},
    {.keycode = HS_MUTE,  .hole_time = 3000},
    {.keycode = BT_TEST,  .hole_time = 3000},
};

#ifdef RGB_MATRIX_BLINK_ENABLE
bool sys_indicator_led = true;
void bat_indicators_hook(uint8_t index);
bool AW20216_read(pin_t cs_pin, uint8_t page, uint8_t reg, uint8_t* data, uint8_t len);
void im_mm_rgb_blink_ind_cb(uint8_t index){
     if (mm_eeconfig.devs == DEVS_2G4) rgb_matrix_blink_set(index);
}

void sys_indicators_hook(uint8_t index){
    if (!sys_indicator_led) {
        rgb_matrix_blink_set(index);
        sys_indicator_led = true;
    }
}
blink_rgb_t blink_rgbs[RGB_MATRIX_BLINK_COUNT] = {
    {.index = IM_MM_RGB_BLINK_INDEX_MIXED, .interval = 250, .times = 1, .color = {.r = 0x00, .g = 0xFE, .b = 0x00}, .blink_cb = im_mm_rgb_blink_cb},
    // {.index = IM_MM_RGB_BLINK_INDEX_BT2, .interval = 250, .times = 1, .color = {.r = 0x00, .g = 0xFE, .b = 0x00}, .blink_cb = im_mm_rgb_blink_cb},
    // {.index = IM_MM_RGB_BLINK_INDEX_BT3, .interval = 250, .times = 1, .color = {.r = 0x00, .g = 0xFE, .b = 0x00}, .blink_cb = im_mm_rgb_blink_cb},
    // {.index = IM_MM_RGB_BLINK_INDEX_2G4, .interval = 250, .times = 1, .color = {.r = 0x00, .g = 0xFE, .b = 0x00}, .blink_cb = im_mm_rgb_blink_cb},
    {.index = IM_MM_RGB_BLINK_INDEX_IND, .interval = 250, .times = 1, .color = {.r = 0x00, .g = 0xFE, .b = 0x00}, .blink_cb = im_mm_rgb_blink_ind_cb},
    {.index = IM_MM_RGB_BLINK_INDEX_CAPS, .interval = 2000, .times = 1, .color = {.r = 0x50, .g = 0x50, .b = 0x50}, .blink_cb = NULL},
    {.index = IM_MM_RGB_BLINK_INDEX_GUI, .interval = 1000, .times = 1, .color = {.r = 0x50, .g = 0x50, .b = 0x50}, .blink_cb = NULL},
    {.index = IM_MM_RGB_BLINK_INDEX_NKRO, .interval = 250, .times = 1, .color = {.r = 0x50, .g = 0x50, .b = 0x50}, .blink_cb = NULL},

    {.index = IM_MM_RGB_BLINK_INDEX_SPI, .interval = 250, .times = 1, .color = {.r = 0x50, .g = 0x50, .b = 0x50}, .blink_cb = NULL},
    {.index = IM_MM_RGB_BLINK_INDEX_SPD, .interval = 250, .times = 1, .color = {.r = 0x50, .g = 0x50, .b = 0x50}, .blink_cb = NULL},
    {.index = IM_MM_RGB_BLINK_INDEX_VAI, .interval = 250, .times = 1, .color = {.r = 0x50, .g = 0x50, .b = 0x50}, .blink_cb = NULL},
    {.index = IM_MM_RGB_BLINK_INDEX_VAD, .interval = 250, .times = 1, .color = {.r = 0x50, .g = 0x50, .b = 0x50}, .blink_cb = NULL},
    {.index = IM_MM_RGB_BLINK_INDEX_WIN, .interval = 2000, .times = 1, .color = {.r = 0x50, .g = 0x50, .b = 0x50}, .blink_cb = sys_indicators_hook},
    {.index = IM_MM_RGB_BLINK_INDEX_MAC, .interval = 2000, .times = 1, .color = {.r = 0x50, .g = 0x50, .b = 0x50}, .blink_cb = sys_indicators_hook},
    {.index = RGB_MATRIX_BLINK_INDEX_BAT, .interval = 250, .times = 3, .color = {.r = 0x00, .g = 0x00, .b = 0x00}, .blink_cb = bat_indicators_hook},
    {.index = RGB_MATRIX_BLINK_INDEX_ALL, .interval = 250, .times = 3, .color = {.r = 0x3, .g = 0x3, .b = 0x3}, .blink_cb = NULL},
};
#endif
void user_sync_mms_slave_handler(uint8_t in_buflen, const void* in_data, uint8_t out_buflen, void* out_data);
#ifdef ENCODER_MAP_ENABLE
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][2] = {
    [0] = { ENCODER_CCW_CW(HS_VOLD, HS_VOLU)},
    [1] = { ENCODER_CCW_CW(RGB_VAD, RGB_VAI)},
    [2] = { ENCODER_CCW_CW(HS_VOLD, HS_VOLU)},
    [3] = { ENCODER_CCW_CW(RGB_VAD, RGB_VAI)},
    [4] = { ENCODER_CCW_CW(HS_VOLD, HS_VOLU)},
};
#endif

typedef union {
    uint32_t raw;
    struct {
        uint8_t flag: 1;
        uint8_t rgb_enable: 1;
        uint8_t no_gui: 1;
        uint8_t layer: 1;
        uint8_t filp: 1;
		uint8_t rgb_hsv_index: 3;
        uint8_t record_last_mode;
        uint8_t encoder : 1;
        uint8_t led_brightness : 7;
        uint8_t left_rgb_enable : 1;
        uint8_t record_channel : 7;
    };
} confinfo_t;
confinfo_t confinfo;

#ifdef RGB_MATRIX_ENABLE

// 此函数不需要改动
bool mm_get_rgb_enable(void) {
// #ifdef RGBLIGHT_ENABLE
//     return confinfo.rgb_enable;
// #else
//     return rgb_matrix_config.enable;
// #endif
    return confinfo.rgb_enable;
}

// 此函数不需要改动
void mm_set_rgb_enable(bool state) {
// #ifdef RGBLIGHT_ENABLE
//     confinfo.rgb_enable = state;
//     eeprom_update_dword((uint32_t *)CONFINFO_EECONFIG_ADDR, confinfo.raw);
// #else
//     rgb_matrix_config.enable = state;
// #endif
    confinfo.rgb_enable = state;
    eeprom_update_dword((uint32_t *)CONFINFO_EECONFIG_ADDR, confinfo.raw);
}

#endif

void eeconfig_confinfo_update(void) {
    eeprom_update_dword((uint32_t *)CONFINFO_EECONFIG_ADDR, confinfo.raw);
}

	// 指示灯闪烁与其回调函数
bool usb_connect_succeed;
bool pair_lback_timeout;
bool hs_sleep = false;
bool im_mm_rgb_blink_hook_user(uint8_t index, mm_linker_rgb_t state) {
    if (!is_keyboard_master()) im_mm_rgb_blink_set_timer(timer_read32());
    #if defined(RGB_MATRIX_BLINK_ENABLE) && defined(MULTIMODE_ENABLE)
        switch (state) {
            case mlrs_lback:
            case mlrs_pair:{
                master_to_slave_t m2s = {0};
                slave_to_master_t s2m = {0};
                m2s.cmd               = 0xD3;
                if (im_mm_rgb_blink_get_timer() == 0x00) transaction_rpc_exec(USER_SYNC_MMS, sizeof(m2s), &m2s, sizeof(s2m), &s2m);
            }break;
            case mlrs_lback_succeed:
            case mlrs_pair_succeed: {
                rgb_matrix_blink_set(index);
                im_mm_rgb_blink_set_state(mlrs_none);
                master_to_slave_t m2s = {0};
                slave_to_master_t s2m = {0};
                m2s.cmd               = 0xD2;
                transaction_rpc_exec(USER_SYNC_MMS, sizeof(m2s), &m2s, sizeof(s2m), &s2m);
                return false;
            } break;
            case mlrs_lback_timeout:
            case mlrs_pair_timeout: {
                im_mm_rgb_blink_set_state(mlrs_none);
                rgb_matrix_blink_set(index);
                if (!mm_eeconfig.charging) mm_set_to_sleep();
                else {
                    pair_lback_timeout = true;
                    hs_sleep = true;
                }
                return false;
            }break;
            default:
                break;
        }
    #endif

    return true;
}

void eeconfig_confinfo_default(void) {
    confinfo.flag = true;
    confinfo.rgb_enable = true;
    confinfo.no_gui = false;
    confinfo.layer = false;
    confinfo.encoder           = false;
    confinfo.record_channel    = 0;
    confinfo.record_last_mode  = 0xff;
    confinfo.left_rgb_enable  = true;
    keymap_config.no_gui = 0;
    confinfo.led_brightness   = 100;
    eeconfig_update_keymap(keymap_config.raw);
    all_color_reset();
    eeprom_update_dword((uint32_t *)CONFINFO_EECONFIG_ADDR, confinfo.raw);

    i = r = g = b = dir = 0;
}
// 初始化一些GPIO PIN 相关的操作
uint8_t bat_full_flag;
bool im_pre_init_user(void) {
#ifdef BAT_FULL_PIN
    setPinInputHigh(BAT_FULL_PIN);
    bat_full_flag = readPin(BAT_FULL_PIN);
#endif
    pair_lback_timeout = false;
    return true;
}
uint8_t pov;
uint32_t start_time = 0;
bool reset = false;
bool led_power_en = true;
// 初始化和参数相关的操作，在恢复出厂设置时此函数会被调用
bool im_init_user(void) {
    // wait_ms(200);

    setPinOutput(C9);
    writePinHigh(C9);
    wait_ms(50);
    writePin(C9, readPin(A10));
    reset = readPin(A10);

    setPinOutput(C5);
    writePinHigh(C5);
    setPinOutput(C4);
    writePinLow(C4);

    i2c_init();

    confinfo.raw = eeprom_read_dword((uint32_t *)CONFINFO_EECONFIG_ADDR);
    if (!confinfo.raw) {
        eeconfig_confinfo_default();
    }
    if(confinfo.layer){
        set_single_persistent_default_layer(_LM);
        layer_move(_LM);
    } else {
        set_single_persistent_default_layer(_LB);
        layer_move(_LB);
    }
    // if (is_keyboard_left() && !is_keyboard_master()) mm_set_rgb_enable(false); //不加关灯时单独插USb会闪灯
    confinfo.rgb_enable = false;
#ifdef RGB_MATRIX_BLINK_INDEX_BAT
    // rgb_matrix_blink_set_interval_times(RGB_MATRIX_BLINK_INDEX_BAT, 200, 0xFF);
    rgb_matrix_blink_set(RGB_MATRIX_BLINK_INDEX_BAT);
#endif
    pov = bts_info.bt_info.pvol;

    // pwm_lld_init();

    // palSetLineMode(LED1_R, PAL_MODE_ALTERNATE(1)| PAL_OUTPUT_TYPE_PUSHPULL | PAL_OUTPUT_SPEED_HIGHEST | PAL_PUPDR_FLOATING);
    // palSetLineMode(LED1_G, PAL_MODE_ALTERNATE(1)| PAL_OUTPUT_TYPE_PUSHPULL | PAL_OUTPUT_SPEED_HIGHEST | PAL_PUPDR_FLOATING);
    // palSetLineMode(LED1_B, PAL_MODE_ALTERNATE(2)| PAL_OUTPUT_TYPE_PUSHPULL | PAL_OUTPUT_SPEED_HIGHEST | PAL_PUPDR_FLOATING);


    // pwm_lld_start(&HS_PWM_DRIVER_2);
    // pwm_lld_start(&HS_PWM_DRIVER_1);

    // pwmStart(&HS_PWM_DRIVER_2, &hs_pwm_config1);
    // pwmStart(&HS_PWM_DRIVER_1, &hs_pwm_config);

    // pwmEnableChannel(&HS_PWM_DRIVER_1, HS_PWM_CHANNEL_3 - 1, 500);
    // pwmEnableChannel(&HS_PWM_DRIVER_1, HS_PWM_CHANNEL_2 - 1, 500);
    // pwmEnableChannel(&HS_PWM_DRIVER_2, HS_PWM_CHANNEL_1 - 1, 50);

    // writePinHigh(LED1_R);
    setPinOutput(RGB_DRIVER_EN_PIN);
    // writePinHigh(RGB_DRIVER_EN_PIN);
    writePinHigh(RGB_DRIVER_EN_PIN);
    start_time = timer_read32();

    // uint8_t data = 0x07;
    // if(is_keyboard_left()) AW20216_write(DRIVER_1_CS,0,0x4D,&data,1);
    // else AW20216_write(DRIVER_2_CS,0,0x4D,&data,1);
    return true;
}

bool im_led_deinit_user(void) {
    setPinOutput(RGB_DRIVER_EN_PIN);
    writePinLow(RGB_DRIVER_EN_PIN);
    // writePinLow(C5);
    // setPinOutput(HS_ENCODERS_PAD_A);
    // writePinLow(HS_ENCODERS_PAD_A);

    // setPinInput(SD1_RX_PIN);
    // setPinOutput(HS_ENCODERS_PAD_B);
    // writePinLow(HS_ENCODERS_PAD_B);
    return true;
}

// bool mm_switch_mode_user(uint8_t last_mode, uint8_t now_mode, uint8_t reset) {
//     if (!is_keyboard_left()){
//         if (now_mode == DEVS_2G4){
//             bts_send_vendor(0xFF);
//         } else {
//             bts_send_vendor(0x80 + 30);
//         }
//     }

//     return true;
// }

bool im_led_init_user()
{
    // setPinOutput(SD1_RX_PIN);
    master_to_slave_t m2s = {0};
    slave_to_master_t s2m = {0};
    if (is_keyboard_master() && (mm_eeconfig.devs == DEVS_USB) && (USB_DRIVER.state == USB_ACTIVE))
    {
        m2s.cmd               = 0xD2;
        if (transaction_rpc_exec(USER_SYNC_MMS, sizeof(m2s), &m2s, sizeof(s2m), &s2m)) {
            if (s2m.resp == 0x01) {}
            IDEBUG_USER_INFO("Slave Sleep OK\n");
        } else {
            IDEBUG_USER_INFO("Slave sync failed!\n");
        }
    }
    m2s.cmd               = 0xCC;
    if (mm_get_sleep_status() != mm_state_normal && is_keyboard_left()) transaction_rpc_exec(USER_SYNC_MMS, sizeof(m2s), &m2s, sizeof(s2m), &s2m);
    // while(s2m.resp != 1);
    IDEBUG_USER_INFO("mm_get_rgb_enable = %d\r\n",mm_get_rgb_enable());
    if (mm_get_rgb_enable()){
        setPinOutput(RGB_DRIVER_EN_PIN);
        writePinHigh(RGB_DRIVER_EN_PIN);
    }
    // setPinOutput(C5);
    // writePinHigh(C5);
    rgb_matrix_enable();
    // setPinInputHigh(HS_ENCODERS_PAD_A);

    // setPinInputHigh(HS_ENCODERS_PAD_B);
    return true;
}
// 恢复出厂设置回调函数
uint32_t rgb_tog_time = 0;
bool im_reset_settings_user(void) {

// #ifdef MULTIMODE_ENABLE
//     eeconfig_update_multimode_default();
// #endif
    master_to_slave_t m2s = {0};
    slave_to_master_t s2m = {0};
    m2s.cmd               = 0xDD;
    m2s.body[0]           = 0x01;
    confinfo.layer = false;
    set_single_persistent_default_layer(_LB);
    layer_move(_LB);
    confinfo.encoder = false;
   confinfo.no_gui = 0;
   keymap_config.no_gui = 1;
   eeconfig_update_keymap(keymap_config.raw);
   eeprom_update_dword((uint32_t *)CONFINFO_EECONFIG_ADDR, confinfo.raw);

   mm_set_rgb_enable(true);
   all_color_reset();
   setPinOutput(RGB_DRIVER_EN_PIN);
   writePinHigh(RGB_DRIVER_EN_PIN);
 if (transaction_rpc_exec(USER_SYNC_MMS, sizeof(m2s), &m2s, sizeof(s2m), &s2m)) {
     if (s2m.resp == 0x00) {}
     IDEBUG_USER_INFO("Slave Sleep OK\n");
 } else {
     IDEBUG_USER_INFO("Slave sync failed!\n");
 }
#ifdef RGB_MATRIX_BLINK_ENABLE
    // rgb_matrix_blink_set_color(IM_MM_RGB_BLINK_INDEX_GUI,0xFF,0x00,0x00);
    // rgb_matrix_blink_set_interval_times(IM_MM_RGB_BLINK_INDEX_GUI, 250, 1);
    // rgb_matrix_blink_set(IM_MM_RGB_BLINK_INDEX_GUI);
    rgb_matrix_blink_set_interval_times(RGB_MATRIX_BLINK_INDEX_ALL, 250, 3);
    rgb_matrix_blink_set(RGB_MATRIX_BLINK_INDEX_ALL);
#endif
    // rgblight_mode(RGBLIGHT_DEFAULT_MODE);

    return true;
}

#ifdef SPLIT_KEYBOARD

bool is_keyboard_master(void) {

    // if (is_keyboard_left()) setPinInput(B8);
    // else setPinInputHigh(B8);
    // return readPin(B8);
    setPinInput(B0);
    return readPin(B0);
}

bool is_keyboard_left(void) {

    setPinInput(SPLIT_HAND_PIN);
    return !readPin(SPLIT_HAND_PIN);
}
bool bat;
bool rgb_effect_status = true;
bool last_rgb_effect_status = true;
uint8_t left_Battery;
void master_sync_mms_slave(uint8_t last_mode, uint8_t now_mode, uint8_t reset)
{
    master_to_slave_t m2s = {0};
    slave_to_master_t s2m = {0};
    m2s.cmd = 0x55;
    m2s.body[0] = last_mode;
    m2s.body[1] = now_mode;
    m2s.body[2] = reset;
    if(transaction_rpc_exec(USER_SYNC_MMS, sizeof(m2s), &m2s, sizeof(s2m), &s2m)) {

        // if (s2m.resp <= 5){
        //     setPinOutput(C4);
        //     writePinLow(C4);
        //     bat = true;
        //     left_Battery = s2m.resp;
        // }
        // else{

        //     if (s2m.resp != 0xFF) {
        //         bat = false;

        //         left_Battery = s2m.resp;
        //         IDEBUG_USER_INFO("C4 = 1\r\n");
        //     }

        // }
        // IDEBUG_USER_INFO("Slave Sleep OK1\n");
        rgb_effect_status = false;

    } else {
        // writePinLow(C4);
        // left_Battery = 0;
        // IDEBUG_USER_INFO("Slave sync failed1!\n");
        rgb_effect_status = true;
    }
}

uint8_t  hs_sat = 0;
void keyboard_post_init_user(void) {
    transaction_register_rpc(USER_SYNC_MMS, user_sync_mms_slave_handler);
    // setPinInput(HS_ENCODERS_PAD_A);
    // setPinInput(HS_ENCODERS_PAD_B);
    hs_sat = rgb_matrix_get_sat();
}

#endif

// 用户主循环

static bool filp_2G4 = true;
bool bat_blink = false;
bool bat_filp;
uint32_t hs_current_time = 0,rgb_sync = 0;
uint8_t hs_now_mode;
uint8_t soc_high = 0;
uint32_t reset_time = 0;
uint8_t clean_count = 0;
static bool pair_lback_timeout_signal;
float soc_bat;
uint8_t hs_rgb_mode = 0,last_hs_rgb_mode = 0;
bool im_loop_user(void) {
    if (mm_get_sleep_status() == mm_state_stop) return true;
    bat_full_flag = readPin(BAT_FULL_PIN);
    static uint8_t data = 0;
    if(is_keyboard_left()) AW20216_read(DRIVER_1_CS,0,0x00,&data,1);
    else AW20216_read(DRIVER_2_CS,0,0x00,&data,1);

    master_to_slave_t m2s = {0};
    slave_to_master_t s2m = {0};
    if (is_keyboard_master() && mm_get_sleep_status() == mm_state_normal) {
        static uint32_t last_sync = 0;
        if (!reset && readPin(A10) != reset && is_keyboard_left())  {
            mcu_reset();
        } else {
            if (start_time && timer_elapsed32(start_time) > 400){
                if (is_keyboard_left() && confinfo.left_rgb_enable && !mm_get_rgb_enable()) {
                    mm_set_rgb_enable(true);
                    IDEBUG_USER_INFO("Left RGB Enable\r\n");
                } else {
                    confinfo.raw = eeprom_read_dword((uint32_t *)CONFINFO_EECONFIG_ADDR);
                }
            }
        }
        if ( !last_sync || timer_elapsed32(last_sync) > 200) {
            last_sync = timer_read32();
            if (BY6102_ReadSOC() != -1){
                // soc_bat = left_Battery * 18 / 48 + soc_high * 30 / 48;
                soc_bat = soc_high;
                bts_send_bat_pvol((uint8_t)soc_bat);
            }
            pov = bts_info.bt_info.pvol;
            if (mm_get_rgb_enable()){
                master_sync_mms_slave(mm_eeconfig.last_devs, mm_eeconfig.devs,bts_info.bt_info.pvol | 0x80);
            } else {
                master_sync_mms_slave(mm_eeconfig.last_devs, mm_eeconfig.devs,bts_info.bt_info.pvol);
            }

            // if (pov <= 5 && readPin(MM_BAT_CABLE_PIN) == 0 && !bat && left_Battery > 5){
            //     if (!readPin(C4)){
            //         wait_ms(70);
            //         setPinOutput(C4);
            //         writePinHigh(C4);
            //     }
            // }
            // else{
            //     setPinOutput(C4);
            //     writePinLow(C4);
            // }
        }

        if (!rgb_sync || timer_elapsed32(rgb_sync) > 20) {
            rgb_sync = timer_read32();
            hs_rgb_mode = rgb_matrix_get_mode();

            if (hs_rgb_mode != last_hs_rgb_mode || rgb_effect_status != last_rgb_effect_status) {
                master_to_slave_t m2s = {0};
                slave_to_master_t s2m = {0};
                m2s.cmd = 0xC4;
                m2s.body[0] = hs_rgb_mode;
                transaction_rpc_exec(USER_SYNC_MMS, sizeof(m2s), &m2s, sizeof(s2m), &s2m);
                last_hs_rgb_mode = hs_rgb_mode;
                last_rgb_effect_status = rgb_effect_status;
            }
        }
        if (pair_lback_timeout){
            if (!pair_lback_timeout_signal){
                m2s.cmd               = 0xD4;
                transaction_rpc_exec(USER_SYNC_MMS, sizeof(m2s), &m2s, sizeof(s2m), &s2m);
            }
            pair_lback_timeout_signal = true;
        }
        else{
            if (pair_lback_timeout_signal){
                m2s.cmd               = 0xD5;
                transaction_rpc_exec(USER_SYNC_MMS, sizeof(m2s), &m2s, sizeof(s2m), &s2m);
            }
            pair_lback_timeout_signal = false;
        }

        if (mm_eeconfig.charging && ( (bat_full_flag))) { // 满电 0x66
            hs_now_mode = v_bat_full;

        } else if (mm_eeconfig.charging) { // 正在充电 0x64
            hs_now_mode = v_bat_charging;

        } else { //正常状态 0x65
            hs_now_mode = v_bat_stop_charging;
        }

        if (hs_current_time == 0) hs_current_time = timer_read32();
        if (timer_elapsed32(hs_current_time) > 1000) {
            hs_current_time = timer_read32();  // 重置时间
            if (is_keyboard_master()) bts_send_vendor(hs_now_mode);  // 发送当前状态：充电和不充电
        }
        if ((mm_eeconfig.devs != DEVS_USB)) {
            usb_connect_ok_flag = true;
        } else if (mm_eeconfig.devs == DEVS_USB && (!mm_eeconfig.charging) && USB_DRIVER.state != USB_ACTIVE) {
            if (usb_connect_ok_flag == true) {
                wait_ms(200);
                // writePinLow(C4);
                usb_connect_ok_flag = false;
            }
        }
    }


    if (mm_eeconfig.devs != DEVS_2G4) {filp_2G4 = true;}
    if (get_highest_layer(default_layer_state) > 1 && keymap_config.no_gui != 0){
        keymap_config.no_gui = 0;
        eeconfig_update_keymap(keymap_config.raw);
    }
    else if(keymap_config.no_gui != confinfo.no_gui && get_highest_layer(default_layer_state) < 2){
        keymap_config.no_gui = confinfo.no_gui;
        eeconfig_update_keymap(keymap_config.raw);
    }

    if (reset_time && timer_elapsed32(reset_time) > 3000) {
        reset_time = 0;
        unregister_code16(KC_LCTL);
        unregister_code16(KC_RCTL);
        im_reset_settings();
    }
    if (is_keyboard_master()){
        if (hs_sleep && !mm_eeconfig.charging) {
            mm_set_to_sleep();
        }

    }
    return true;
}

void suspend_wakeup_init_user(void) {

    if (is_keyboard_master()){
        master_to_slave_t m2s = {0};
        slave_to_master_t s2m = {0};
        m2s.cmd               = 0xCC;
        if (transaction_rpc_exec(USER_SYNC_MMS, sizeof(m2s), &m2s, sizeof(s2m), &s2m)) {
            if (s2m.resp == 0x00) {}
            IDEBUG_USER_INFO("Slave Sleep OK\n");
        } else {
            IDEBUG_USER_INFO("Slave sync failed2!\n");
        }
    }

    if (!mm_get_rgb_enable() && !mm_eeconfig.charging){
        setPinOutput(RGB_DRIVER_EN_PIN);
        writePinLow(RGB_DRIVER_EN_PIN);
    }

}

void suspend_power_down_user(void) {

    if (is_keyboard_master()){
        master_to_slave_t m2s = {0};
        slave_to_master_t s2m = {0};
        m2s.cmd               = 0xBB;
        if (transaction_rpc_exec(USER_SYNC_MMS, sizeof(m2s), &m2s, sizeof(s2m), &s2m)) {
            if (s2m.resp == 0x00) {}
            IDEBUG_USER_INFO("Slave Sleep OK\n");
        } else {
            IDEBUG_USER_INFO("Slave sync failed3!\n");
        }
    }
}

#ifdef RGB_MATRIX_ENABLE

#ifdef RGB_MATRIX_BLINK_INDEX_BAT

typedef enum {
    bat_state_none = 0,
    bat_state_charging,
    bat_state_full,
    bat_state_low,
} bat_state_t;
bat_state_t bat_status = bat_state_none;
static uint16_t bat_full_time = 0;
bool encoder = false;
bool im_lkey_process_user(uint16_t keycode, bool pressed) {
    master_to_slave_t m2s = {0};
    slave_to_master_t s2m = {0};
    if (pressed){
        switch (keycode){
            case HS_MUTE:{
                // if (pressed) {
                //     confinfo.encoder = !confinfo.encoder;
                //     if (confinfo.encoder){
                //         rgb_matrix_blink_set_interval_times(RGB_MATRIX_BLINK_INDEX_ALL, 250, 2);
                //     } else {
                //         rgb_matrix_blink_set_interval_times(RGB_MATRIX_BLINK_INDEX_ALL, 250, 1);
                //     }
                //     rgb_matrix_blink_set(RGB_MATRIX_BLINK_INDEX_ALL);
                //     encoder = false;
                //     eeconfig_confinfo_update();
                // }
                return false;
            }break;
            case IM_2G4 :
                m2s.cmd               = 0xA2;
                if (transaction_rpc_exec(USER_SYNC_MMS, sizeof(m2s), &m2s, sizeof(s2m), &s2m)) {
                    if (s2m.resp == 0x00) {}
                    IDEBUG_USER_INFO("Slave Sleep OK\n");
                } else {
                    IDEBUG_USER_INFO("Slave sync failed!\n");
                }
                break;
            case HS_SIRI:{
                master_to_slave_t m2s = {0};
                slave_to_master_t s2m = {0};
                m2s.cmd               = 0xDD;
                confinfo.no_gui = false;
                eeconfig_confinfo_update();
                rgb_matrix_blink_set_interval_times(RGB_MATRIX_BLINK_INDEX_ALL, 250, 1);
                rgb_matrix_blink_set(RGB_MATRIX_BLINK_INDEX_ALL);
                dynamic_keymap_reset();
                transaction_rpc_exec(USER_SYNC_MMS, sizeof(m2s), &m2s, sizeof(s2m), &s2m);
            }break;
            case BT_TEST:{
                bts_send_vendor(0x62);
                mm_switch_mode(mm_eeconfig.devs, DEVS_BT2, true);
            }break;
            default :
                break;
        }
    }
    return true;
}
uint32_t elapsed_since_phase_start;
uint32_t last_toggle_time;
bool is_active_phase;
void bat_indicators_hook(uint8_t index) {
    static uint32_t battery_process_time;
    mm_eeconfig.charging = readPin(MM_BAT_CABLE_PIN);
    if (mm_eeconfig.charging &&  bat_full_flag) {
         bat_status = bat_state_full;
         battery_process_time = 0;
         dprintf("111\r\n");
    } else if (mm_eeconfig.charging) {
        bat_status = bat_state_charging;
        battery_process_time = 0;
        dprintf("222\r\n");
    } else if (soc_bat <= 15) { // 低电关机 与 低电
        if (is_keyboard_master()){
            if (bat_status != bat_state_low) elapsed_since_phase_start = timer_read32();
            bat_status = bat_state_low;
            if (soc_bat <= 1) {
                if (!battery_process_time) {
                    battery_process_time = timer_read32();
                }

                if (battery_process_time && timer_elapsed32(battery_process_time) > 10000) {
                    battery_process_time = 0;
                    master_to_slave_t m2s = {0};
                    slave_to_master_t s2m = {0};
                    m2s.cmd               = 0xAC;
                    clear_keyboard();
                    transaction_rpc_exec(USER_SYNC_MMS, sizeof(m2s), &m2s, sizeof(s2m), &s2m);
                    wait_ms(100);
                    im_set_lowpower(true);
                }
            }
        }

    } else {
        bat_blink = false;
        // IDEBUG_USER_INFO("444\r\n");
        battery_process_time = 0;
        bat_status = bat_state_none;
        bat_full_time = 0;
    }

    rgb_matrix_blink_set(index);
}

bool rgb_matrix_blink_user(blink_rgb_t *blink_rgb) {
    if (pair_lback_timeout && blink_rgb->index != RGB_MATRIX_BLINK_INDEX_BAT) return false;
    switch (blink_rgb->index) {

        case IM_MM_RGB_BLINK_INDEX_MIXED :{

            if (blink_rgb->flip){
                switch (mm_eeconfig.devs)
                {
                case DEVS_BT1:
                    rgb_matrix_set_color(86,0x00, 0x00, 0xFF * confinfo.led_brightness / 100);
                    break;
                case DEVS_BT2:{
                    HSV hsv = {0x15,0xFF,0x00};

                    hsv.v = confinfo.led_brightness * 255 / 100;
                    RGB rgb = hsv_to_rgb(hsv);
                    rgb_matrix_set_color(86,rgb.r,rgb.g,rgb.b);
                }break;
                case DEVS_BT3:
                    rgb_matrix_set_color(86,0xFF * confinfo.led_brightness / 100, 0x00, 0xFF * confinfo.led_brightness / 100);
                    break;
                case DEVS_2G4:
                    rgb_matrix_set_color(86,0x00,0xFF * confinfo.led_brightness / 100,0x00);

                    break;
                default:
                    break;
                }

            }
            else if(mm_eeconfig.devs != DEVS_USB){
                rgb_matrix_set_color(86,RGB_BLACK);
            }
            rgb_tog_time = timer_read32();
            return false;

        }break;
        case IM_MM_RGB_BLINK_INDEX_CAPS :{
            {
                if (!mm_get_rgb_enable())
                {
                    for (uint8_t i = 0;i < RGB_MATRIX_LED_COUNT; i++){
                        rgb_matrix_set_color(i,0,0,0);
                    }
                    if (!blink_rgb->flip){
                        writePinLow(RGB_DRIVER_EN_PIN);

                    }
                    else{
                        writePinHigh(RGB_DRIVER_EN_PIN);
                    }
                }

                if (!host_keyboard_led_state().caps_lock) {
                    rgb_matrix_blink_set_remain_time(blink_rgb->index,0);
                    if (!mm_get_rgb_enable())  {
                        if (!(((get_highest_layer(default_layer_state) < 2 && confinfo.no_gui) || host_keyboard_led_state().caps_lock || mm_eeconfig.charging) && mm_eeconfig.devs == DEVS_USB))
                        {
                            setPinOutput(RGB_DRIVER_EN_PIN);
                            writePinLow(RGB_DRIVER_EN_PIN);
                        }
                    }
                }

                if(!blink_rgb->flip) return false;
            }
        }break;
        case IM_MM_RGB_BLINK_INDEX_GUI :{
            {
                if (!mm_get_rgb_enable())
                {
                    for (uint8_t i = 0;i < RGB_MATRIX_LED_COUNT; i++){
                        rgb_matrix_set_color(i,0,0,0);
                    }
                }
                if (blink_rgb->flip){
                    writePinHigh(RGB_DRIVER_EN_PIN);

                    rgb_matrix_set_color(34,blink_rgb->color.r,blink_rgb->color.g,blink_rgb->color.b);
                    if (!keymap_config.no_gui) {
                        rgb_matrix_blink_set_remain_time(blink_rgb->index,0);
                        if (!mm_get_rgb_enable())  {
                            if (!(((get_highest_layer(default_layer_state) < 2 && confinfo.no_gui) || host_keyboard_led_state().caps_lock || mm_eeconfig.charging) && mm_eeconfig.devs == DEVS_USB))
                            {
                                setPinOutput(RGB_DRIVER_EN_PIN);
                                writePinLow(RGB_DRIVER_EN_PIN);
                            }
                        }
                    }
                }
                else{
                    // rgb_matrix_set_color(35,0,0,0);
                   if (!mm_get_rgb_enable())  {
                        if (!(((get_highest_layer(default_layer_state) < 2 && confinfo.no_gui) || host_keyboard_led_state().caps_lock || mm_eeconfig.charging) && mm_eeconfig.devs == DEVS_USB))
                        {
                            setPinOutput(RGB_DRIVER_EN_PIN);
                            writePinLow(RGB_DRIVER_EN_PIN);
                        }
                    }
                }
                return false;
            }
        }break;
        case IM_MM_RGB_BLINK_INDEX_MAC :
        case IM_MM_RGB_BLINK_INDEX_WIN :{
            {
                if (blink_rgb->flip){
                    rgb_matrix_set_color(86,0x00,0x00,0x00);
                }
                else{
                    if (sys_indicator_led) {
                        rgb_matrix_blink_set_remain_time(blink_rgb->index,0);
                    }
                    writePinHigh(RGB_DRIVER_EN_PIN);
                    rgb_tog_time = timer_read32();
                    rgb_matrix_set_color(86,0xFF * confinfo.led_brightness / 100, 0xFF * confinfo.led_brightness / 100, 0xFF * confinfo.led_brightness / 100);
                }
                return false;
            }
        }break;
        case IM_MM_RGB_BLINK_INDEX_IND:  {
            if (mm_eeconfig.devs == DEVS_2G4){
                if (blink_rgb->flip) {
                    // rgb_matrix_set_color(IM_MM_RGB_BLINK_INDEX_2G4,0,255,0);

                } else {
                    // rgb_matrix_set_color(IM_MM_RGB_BLINK_INDEX_2G4,0,0,0);
                }
            }
            return false;
        } break;
        case RGB_MATRIX_BLINK_INDEX_ALL:  {

            if (blink_rgb->flip) {

                rgb_matrix_set_color(86,0xFF * confinfo.led_brightness / 100,0xFF* confinfo.led_brightness / 100,0xFF* confinfo.led_brightness / 100);
                rgb_matrix_set_color(37,0xFF * confinfo.led_brightness / 100,0xFF* confinfo.led_brightness / 100,0xFF* confinfo.led_brightness / 100);
            } else {
                rgb_matrix_set_color(86,0x00,0x00,0x00);
                rgb_matrix_set_color(37,0x00,0x00,0x00);
            }
            writePinHigh(RGB_DRIVER_EN_PIN);
            if (!mm_get_rgb_enable()) rgb_tog_time = timer_read32();
            return false;
        }
        case RGB_MATRIX_BLINK_INDEX_BAT:  {
            if (!bat_blink) {
                return false;
            }
            return true;
        } break;
        default:
         break;
    }

    return true;
}
#endif
void limiting_clearance(void){
     rgb_matrix_blink_set_interval_times(IM_MM_RGB_BLINK_INDEX_SPI, 250, 3);
     rgb_matrix_blink_set(IM_MM_RGB_BLINK_INDEX_SPI);
     rgb_matrix_blink_set_interval_times(IM_MM_RGB_BLINK_INDEX_SPD, 250, 3);
     rgb_matrix_blink_set(IM_MM_RGB_BLINK_INDEX_SPD);
     rgb_matrix_blink_set_interval_times(IM_MM_RGB_BLINK_INDEX_VAI, 250, 3);
     rgb_matrix_blink_set(IM_MM_RGB_BLINK_INDEX_VAI);
     rgb_matrix_blink_set_interval_times(IM_MM_RGB_BLINK_INDEX_VAD, 250, 3);
     rgb_matrix_blink_set(IM_MM_RGB_BLINK_INDEX_VAD);
}
void send(void){
    // master_to_slave_t m2s = {0};
    // slave_to_master_t s2m = {0};
    // m2s.cmd               = 0xA3;
    // if (transaction_rpc_exec(USER_SYNC_MMS, sizeof(m2s), &m2s, sizeof(s2m), &s2m)) {
    //     if (s2m.resp == 0x00) {}
    //     IDEBUG_USER_INFO("Slave Sleep OK\n");
    // } else {
    //     IDEBUG_USER_INFO("Slave sync failed!\n");
    // }
    limiting_clearance();
}
uint32_t led_switch_time;
uint32_t led_fn_switch_time;
bool caps_fg = false, gui_fg = false;
bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    // static uint32_t bat_time;
    static bool bat_filp;

    rgb_matrix_set_color(37,RGB_BLACK);
    rgb_matrix_set_color(86,RGB_BLACK);
    if (!led_power_en || !mm_get_rgb_enable() || pov < 5){
        rgb_matrix_set_color_all(0,0,0);
    }

    if (rgb_tog_time && timer_elapsed32(rgb_tog_time) > 200){
        rgb_tog_time = 0;
        if (!mm_get_rgb_enable()) writePinLow(RGB_DRIVER_EN_PIN);
    } else if (rgb_tog_time) {
        if (!mm_get_rgb_enable()) rgb_matrix_set_color_all(0,0,0);
        writePinHigh(RGB_DRIVER_EN_PIN);
    }

    // if (!mm_eeconfig.charging && pair_lback_timeout) mm_set_to_sleep();
    if (pair_lback_timeout){
        rgb_matrix_set_color_all(0x00, 0x00, 0x00);
    }

    if (confinfo.filp && !pair_lback_timeout) rgb_matrix_set_color(28,RGB_MATRIX_MAXIMUM_BRIGHTNESS,RGB_MATRIX_MAXIMUM_BRIGHTNESS,RGB_MATRIX_MAXIMUM_BRIGHTNESS);

    // static bool caps_led_instruction;
    // static bool gui_led_instruction;

    if (im_bat_req_flag) {
        writePinHigh(RGB_DRIVER_EN_PIN);
        if (!mm_get_rgb_enable()) rgb_tog_time = timer_read32();
    }
    if (mm_eeconfig.charging) writePinHigh(RGB_DRIVER_EN_PIN);
    if (host_keyboard_led_state().caps_lock && !pair_lback_timeout) {
        // if(!caps_led_instruction && mm_eeconfig.devs != DEVS_USB) {
        //     if (!mm_get_rgb_enable()){
        //         writePinHigh(RGB_DRIVER_EN_PIN);
        //     }
        //     rgb_matrix_blink_set(IM_MM_RGB_BLINK_INDEX_CAPS);
        // }
        // else if (mm_eeconfig.devs == DEVS_USB) {

            if (!mm_get_rgb_enable()){
                writePinHigh(RGB_DRIVER_EN_PIN);
                for (uint8_t i = 0;i < RGB_MATRIX_LED_COUNT; i++){
                    rgb_matrix_set_color(i,0,0,0);
                }
            }
            rgb_tog_time = 0;
            caps_fg = true;
            rgb_matrix_set_color(IM_MM_RGB_BLINK_INDEX_CAPS,0xFF,0xFF,0xFF);
        // }
        // caps_led_instruction = true;
    }
    else if (!pair_lback_timeout){
        if (!mm_get_rgb_enable()){
            if (!(((get_highest_layer(default_layer_state) < 2 && confinfo.no_gui) || mm_eeconfig.charging) && mm_eeconfig.devs == DEVS_USB))
            {

                if (!rgb_tog_time) rgb_tog_time = timer_read32();
            }
            rgb_matrix_set_color(IM_MM_RGB_BLINK_INDEX_CAPS,0x00,0x00,0x00);
        }
        // caps_led_instruction = false;
        if (caps_fg)  rgb_matrix_set_color(IM_MM_RGB_BLINK_INDEX_CAPS,0x00,0x00,0x00);
        caps_fg = false;
    }
    if (confinfo.no_gui && !pair_lback_timeout) {
        if (get_highest_layer(default_layer_state) < 2 ) {
            // if (!gui_led_instruction && mm_eeconfig.devs != DEVS_USB) {
            //     if (!mm_get_rgb_enable()){
            //         writePinHigh(RGB_DRIVER_EN_PIN);
            //     }
            //     rgb_matrix_blink_set_color(IM_MM_RGB_BLINK_INDEX_GUI,0xFF,0xFF,0xFF);
            //     rgb_matrix_blink_set_interval_times(IM_MM_RGB_BLINK_INDEX_GUI, 1000, 1);
            //     rgb_matrix_blink_set(IM_MM_RGB_BLINK_INDEX_GUI);
            // }
            // else if(mm_eeconfig.devs == DEVS_USB){

                if (!mm_get_rgb_enable()){
                    writePinHigh(RGB_DRIVER_EN_PIN);
                    for (uint8_t i = 0;i < RGB_MATRIX_LED_COUNT; i++){
                        if (i == IM_MM_RGB_BLINK_INDEX_CAPS && host_keyboard_led_state().caps_lock){
                            continue;
                        }
                        rgb_matrix_set_color(i,0,0,0);
                    }
                }
                rgb_tog_time = 0;
                gui_fg = true;
                rgb_matrix_set_color(34,0xFF,0xFF,0xFF);
            // }
            // gui_led_instruction = true;
        }

    }
    else if (!pair_lback_timeout){
        if (!mm_get_rgb_enable()){
            if(!(host_keyboard_led_state().caps_lock)){
                if (!((host_keyboard_led_state().caps_lock || mm_eeconfig.charging)))
                {
                    if (!rgb_tog_time) rgb_tog_time = timer_read32();
                }
            }
        }
        // gui_led_instruction = false;
        if (gui_fg || hs_sat == 0) rgb_matrix_set_color(34,0x00,0x00,0x00);
        if (rgb_matrix_get_sat()) hs_sat = 1;
        gui_fg = false;
    }

    if (im_bat_req_flag) {
        #ifdef IM_RGB_BAT_REQ_CLOSE_ALL
            rgb_matrix_set_color_all(0x00, 0x00, 0x00);
        // #    ifdef RGBLIGHT_ENABLE
        //         for (uint8_t i = 0; i < (RGB_MATRIX_LED_COUNT - RGBLED_NUM); i++) {
        //             rgb_matrix_set_color(i, 0, 0, 0);
        //         }
        // #    else
        //         rgb_matrix_set_color_all(0x00, 0x00, 0x00);
        // #    endif
        #endif
        for (uint8_t i = 0; i < 5; i++) {

            uint8_t mi_index[5] = RGB_MATRIX_BAT_INDEX_MAP;
            if (get_highest_layer(layer_state) == _TEST) {
                for (uint8_t z = 0; z < 5; z++) {
                    mi_index[z] =  mi_index[z] - 39;
                }

                if ((i < ((left_Battery / 20) + 1)) || (i < 1)) {
                    if (left_Battery >= (60)) {
                        rgb_matrix_set_color(mi_index[i], 0,150,0);
                    } else if (left_Battery >= (20)){
                        rgb_matrix_set_color(mi_index[i], 150,150,0);
                    }else {
                        rgb_matrix_set_color(mi_index[i], 150,0,0);
                    }
                } else if((left_Battery < 20) && (left_Battery != 0)){
                    rgb_matrix_set_color(mi_index[0], 150,0,0);
                }else {
                    rgb_matrix_set_color(mi_index[i], 0x00, 0x00, 0x00);
                }

            } else {
                if ((i < ((pov / 20) + 1)) || (i < 1)) {
                    if (pov >= (60)) {
                        rgb_matrix_set_color(mi_index[i], 0,150,0);
                    } else if (pov >= (20)){
                        rgb_matrix_set_color(mi_index[i], 150,150,0);
                    }else {
                        rgb_matrix_set_color(mi_index[i], 150,0,0);
                    }
                } else if((pov < 20) && (pov != 0)){
                    rgb_matrix_set_color(mi_index[0], 150,0,0);
                }else {
                    rgb_matrix_set_color(mi_index[i], 0x00, 0x00, 0x00);
                }
            }
        }
    }
    // rgb_matrix_set_color(86,0x00,0x00,0xFF);
    if (((mm_eeconfig.devs != DEVS_USB && bts_info.bt_info.paired)) || mm_eeconfig.devs == DEVS_USB || is_keyboard_left() || pair_lback_timeout)
    {
        switch (bat_status)
        {
            case bat_state_full:{
                // if (!(!mm_get_rgb_enable() && mm_eeconfig.devs != DEVS_USB))
                    rgb_matrix_set_color(86,0x00,0xFF * confinfo.led_brightness / 100,0x00);  //关灯状态按caps等指示灯，会闪充电灯(关灯不显示指示灯)
                last_toggle_time = 0;
                is_active_phase = true;
            } break;
            case bat_state_charging:{
                // if (timer_elapsed32(bat_time) > 300 || !bat_time) {
                //     bat_time = timer_read32();
                //     bat_filp = !bat_filp;
                // }
                // if (bat_filp) {
                // if (!(!mm_get_rgb_enable() && mm_eeconfig.devs != DEVS_USB))
                    rgb_matrix_set_color(86,0xFF * confinfo.led_brightness / 100,0xFF * confinfo.led_brightness / 100,0x00);//关灯状态按caps等指示灯，会闪充电灯(关灯不显示指示灯)

                // }

                // rgb_matrix_set_color(39,RGB_MATRIX_MAXIMUM_BRIGHTNESS,0,0);
                last_toggle_time = 0;
                is_active_phase = true;
            } break;
            case bat_state_low:{
                if (is_active_phase) {
                    // 2秒活动阶段 - 闪烁
                    if (timer_elapsed32(elapsed_since_phase_start) <= 2000) {
                        if (!last_toggle_time || timer_elapsed32(last_toggle_time) >= 500) {
                            bat_filp = !bat_filp;
                            last_toggle_time = timer_read32();
                        }

                        if (bat_filp) {
                            rgb_matrix_set_color(86,0xFF * confinfo.led_brightness / 100,0x00,0x00);
                            // if (!(!mm_get_rgb_enable() && mm_eeconfig.devs != DEVS_USB)) rgb_matrix_set_color(86,0xFF,0x00,0x00);//关灯状态按caps等指示灯，会闪充电灯(关灯不显示指示灯)
                        } else {
                            rgb_matrix_set_color(86,0x00,0x00,0x00);
                        }
                    } else {
                        // 2秒结束，进入10秒熄灭阶段
                        elapsed_since_phase_start = timer_read32();
                        is_active_phase = false;
                        last_toggle_time = 0;
                        rgb_matrix_set_color(86,0x00,0x00,0x00);
                    }
                } else {
                    // 10秒熄灭阶段
                    if (timer_elapsed32(elapsed_since_phase_start) >= 10000) {
                        // 10秒结束，重新开始2秒活动阶段
                        is_active_phase = true;
                        elapsed_since_phase_start = timer_read32();
                        bat_filp = false;  // 重置状态
                    }
                    rgb_matrix_set_color(86,0x00,0x00,0x00);
                }
            } break;
            default:{
                last_toggle_time = 0;
                is_active_phase = true;
                {
                    rgb_matrix_set_color(86,0x00,0x00,0x00);
                }
            }break;
        }

    }

    // if (ntf) {
    //     RGB_MATRIX_INDICATOR_SET_COLOR(4, RGB_MATRIX_MAXIMUM_BRIGHTNESS, RGB_MATRIX_MAXIMUM_BRIGHTNESS, RGB_MATRIX_MAXIMUM_BRIGHTNESS);
    // }
    // if ((FN_status && ((mm_eeconfig.devs != DEVS_USB && bts_info.bt_info.paired) || ((mm_eeconfig.devs == DEVS_USB) && (USB_DRIVER.state == USB_ACTIVE)))) || (usb_connect_succeed && is_keyboard_left())){
    if (start_time && timer_elapsed32(start_time) < 500){

    }
    else if (!pair_lback_timeout && (led_switch_time || led_fn_switch_time)){
        start_time = 0;
        if (led_switch_time){
            switch (mm_eeconfig.devs)
            {
            case DEVS_BT1:
            case DEVS_BT2:
            case DEVS_BT3:
                rgb_matrix_set_color(37,0x00, 0x00, 0xFF * confinfo.led_brightness / 100);

                break;
            case DEVS_USB:
                rgb_matrix_set_color(37,0xFF * confinfo.led_brightness / 100,0xFF * confinfo.led_brightness / 100,0xFF * confinfo.led_brightness / 100);
                break;
            case DEVS_2G4:
                rgb_matrix_set_color(37,0,0xFF * confinfo.led_brightness / 100,0x00);
                break;
            default :
                rgb_matrix_set_color(37,RGB_BLACK);
                break;
            }
            if (timer_elapsed32(led_switch_time) > 2000) led_switch_time = 0;
        }
        else{
            switch (mm_eeconfig.devs){
                case DEVS_BT1:
                    rgb_matrix_set_color(37,0x00, 0x00, 0xFF * confinfo.led_brightness / 100);
                    break;
                case DEVS_BT2:{
                    HSV hsv = {0x15,0xFF,0x00};

                    hsv.v = confinfo.led_brightness * 255 / 100;
                    RGB rgb = hsv_to_rgb(hsv);
                    rgb_matrix_set_color(37,rgb.r,rgb.g,rgb.b);
                }break;
                case DEVS_BT3:
                    rgb_matrix_set_color(37,0xFF * confinfo.led_brightness / 100, 0x00, 0xFF * confinfo.led_brightness / 100);
                    break;
                default :
                    rgb_matrix_set_color(37,RGB_BLACK);
                    break;
            }
            if (timer_elapsed32(led_fn_switch_time) > 2000) led_fn_switch_time = 0;
        }
    }
    else{
        // rgb_matrix_set_color(0,RGB_BLACK);
        // rgb_matrix_set_color(1,RGB_BLACK);
    }
    if (mm_get_sleep_status() != mm_state_normal){
        rgb_matrix_set_color_all(0x00, 0x00, 0x00);
    }

    return true;
}
#endif

uint8_t buff[] = {14,8,2,1,1,1,1,1,1,1,0};   // 14 21  35
void im_rgblight_increase(void) {
    HSV rgb;
    uint8_t moude;
    static uint8_t mode = 0;
    moude = rgblight_get_mode();
    rgb = rgblight_get_hsv();
    if (moude == 1 && rgb.v != 0){

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
    switch (moude){
        case 14: mode = 0;break;
        case 8: mode = 1;break;
        case 2: mode = 2;break;
        default :if (rgb.v == 0) mode = 10;break;
    }
    mode++;
    dprintf("mode = %d\r\n",mode);
    if(mode == 11) mode=0;
    if (mode == 10) {rgblight_sethsv(0,255,0);}
    else
    {
        rgb = rgblight_get_hsv();
        if(rgb.v == 0) rgblight_sethsv(0,255,RGBLIGHT_LIMIT_VAL);
        rgblight_mode(buff[mode]);

       // if (rgblight_get_mode() == 8) {rgblight_mode(2);mode++;}
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

bool mm_switch_mode_user(uint8_t last_mode, uint8_t now_mode, uint8_t reset) {
    if (now_mode != DEVS_USB) {
        wls_transport_enable(true);
        if (!is_keyboard_left()){
            if (now_mode == DEVS_2G4){
                bts_send_vendor(0xFF);
            } else {
                bts_send_vendor(0x80 + 1);
            }
        }
    } else {
        wls_transport_enable(false);
    }
    return true;
}

bool reset_filp = false;

bool im_process_record_user(uint16_t keycode, keyrecord_t *record) {
    master_to_slave_t m2s = {0};
    slave_to_master_t s2m = {0};
    pair_lback_timeout = false;
    if(!bts_info.bt_info.paired && mm_eeconfig.devs != DEVS_USB){
        if(IS_BASIC_KEYCODE(keycode)){
            return false;
        }
    }

    if ((record->event.key.row == 5 && record->event.key.col == 0) || (record->event.key.row == 11 && record->event.key.col == 4))
    {

        if (record->event.pressed){
            clean_count++;
            if (clean_count == 2){
                reset_time = timer_read32();
                reset_filp = true;
            }
        }
        else{
            clean_count--;
            if (!clean_count) {
                reset_filp = false;
                if (get_highest_layer(default_layer_state) >= 2) {
                    unregister_code16(KC_LCTL);
                    unregister_code16(KC_RCTL);
                }
            }
            reset_time = 0;
        }
    }

    switch (keycode) {
        case LED_BL:{
            if (record->event.pressed){
                if (confinfo.led_brightness == 100){
                    confinfo.led_brightness = 33;
                } else if (confinfo.led_brightness == 33){
                    confinfo.led_brightness = 66;
                } else {
                    confinfo.led_brightness = 100;
                }
                eeconfig_confinfo_update();
                master_to_slave_t m2s = {0};
                slave_to_master_t s2m = {0};


                m2s.cmd               = 0xD8;
                m2s.body[0]        = confinfo.led_brightness;
                transaction_rpc_exec(USER_SYNC_MMS, sizeof(m2s), &m2s, sizeof(s2m), &s2m);
            }
        }break;
        case HS_MUTE:{
            // if (!record->event.pressed && encoder){
            //     encoder = false;
            //     if (!confinfo.encoder){
            //         register_code16(KC_MUTE);
            //         wait_ms(20);
            //         unregister_code16(KC_MUTE);
            //     } else {
            //         if (!mm_get_rgb_enable()) return false;

            //         record_rgbmatrix_mod(&(confinfo.record_last_mode));
            //         master_to_slave_t m2s = {0};
            //         slave_to_master_t s2m = {0};
            //         m2s.cmd               = 0xC4;
            //         m2s.body[0]        = rgb_matrix_get_mode();
            //         transaction_rpc_exec(USER_SYNC_MMS, sizeof(m2s), &m2s, sizeof(s2m), &s2m);

            //         eeconfig_confinfo_update();
            //     }
            // } else {
            //     encoder = true;
            // }
            return false;
        }break;
        case HS_VOLD:{
            if (!confinfo.encoder){
                if (record->event.pressed) {
                    register_code16(KC_VOLD);

                } else {
                    unregister_code16(KC_VOLD);
                }
            } else {
                if (record->event.pressed) {
                    rgb_matrix_decrease_val();
                    if (rgb_matrix_get_val() <= RGB_MATRIX_VAL_STEP && (rgb_matrix_get_val() != 0)) {
                        rgb_matrix_set_color_all(0x00, 0x00, 0x00);
                    }
                }
            }
            return false;
        }break;
        case HS_VOLU:{
            if (!confinfo.encoder){
                if (record->event.pressed) {
                    register_code16(KC_VOLU);

                } else {
                    unregister_code16(KC_VOLU);
                }
            } else {
                if (record->event.pressed) {
                    rgb_matrix_increase_val();
                }
            }
            return false;
        }break;
        case NK_TOGG:{
        if (record->event.pressed){
            master_to_slave_t m2s = {0};
            slave_to_master_t s2m = {0};


            if (keymap_config.nkro){
                m2s.cmd               = 0xD6;
            } else {
                m2s.cmd               = 0xD7;
            }
            transaction_rpc_exec(USER_SYNC_MMS, sizeof(m2s), &m2s, sizeof(s2m), &s2m);

        }
    }break;
    case HS_LPAD :{
        if (record->event.pressed){
            register_code16(KC_LPAD);
        } else {
            unregister_code16(KC_LPAD);
        }
        return false;
    }break;
    case MO(_LMF):
    case MO(_LF):{
        if (record->event.pressed){
            FN_status = true;
        }
        else{
            FN_status = false;
        }
    }break;
    case LSFT(KC_1):{
        if (record->event.pressed){
            register_code16(KC_LSFT);
            register_code16(KC_1);
        }
        else{
            unregister_code16(KC_LSFT);
            unregister_code16(KC_1);
        }
        return false;
    }
    case LSFT(KC_2):{
        if (record->event.pressed){
            register_code16(KC_LSFT);
            register_code16(KC_2);
        }
        else{
            unregister_code16(KC_LSFT);
            unregister_code16(KC_2);
        }
        return false;
    }
    case LSFT(KC_3):{
        if (record->event.pressed){
            register_code16(KC_LSFT);
            register_code16(KC_3);
        }
        else{
            unregister_code16(KC_LSFT);
            unregister_code16(KC_3);
        }
        return false;
    }
    case LSFT(KC_4):{
        if (record->event.pressed){
            register_code16(KC_LSFT);
            register_code16(KC_4);
        }
        else{
            unregister_code16(KC_LSFT);
            unregister_code16(KC_4);
        }
        return false;
    }
    case LSFT(KC_5):{
        if (record->event.pressed){
            register_code16(KC_LSFT);
            register_code16(KC_5);
        }
        else{
            unregister_code16(KC_LSFT);
            unregister_code16(KC_5);
        }
        return false;
    }
    case LSFT(KC_6):{
        if (record->event.pressed){
            register_code16(KC_LSFT);
            register_code16(KC_6);
        }
        else{
            unregister_code16(KC_LSFT);
            unregister_code16(KC_6);
        }
        return false;
    }
    case LSFT(KC_7):{
        if (record->event.pressed){
            register_code16(KC_LSFT);
            register_code16(KC_7);
        }
        else{
            unregister_code16(KC_LSFT);
            unregister_code16(KC_7);
        }
        return false;
    }
    case LSFT(KC_8):{
        if (record->event.pressed){
            register_code16(KC_LSFT);
            register_code16(KC_8);
        }
        else{
            unregister_code16(KC_LSFT);
            unregister_code16(KC_8);
        }
        return false;
    }
    case LSFT(KC_9):{
        if (record->event.pressed){
            register_code16(KC_LSFT);
            register_code16(KC_9);
        }
        else{
            unregister_code16(KC_LSFT);
            unregister_code16(KC_9);
        }
        return false;
    }
    case LSFT(KC_0):{
        if (record->event.pressed){
            register_code16(KC_LSFT);
            register_code16(KC_0);
        }
        else{
            unregister_code16(KC_LSFT);
            unregister_code16(KC_0);
        }
        return false;
    }
    case CONTROL:{
        if (record->event.pressed){
            register_code(KC_LCTL);
            register_code(KC_UP);
        }
        else{
            unregister_code16(KC_LCTL);
            unregister_code16(KC_UP);
        }
        return false;
    }
    case HS_SIRI:{

        return false;
    }break;
    case BT_TEST:{

        return false;
    }break;
    case KC_F1:{
        if(confinfo.filp){
            if (confinfo.layer) {
                // if (record->event.pressed) {
                //     register_code16(KC_BRID);
                // } else {
                //     unregister_code16(KC_BRID);
                // }
                // return false;
            } else {
                if (record->event.pressed) {
                    register_code16(KC_MSEL);
                } else {
                    unregister_code16(KC_MSEL);
                }
                return false;
            }
        }
        return true;
    }break;
    case KC_F2:{
        if(confinfo.filp){
            if (confinfo.layer) {
                // if (record->event.pressed) {
                //     register_code16(KC_BRIU);
                // } else {
                //     unregister_code16(KC_BRIU);
                // }
                // return false;
            } else {
                if (record->event.pressed) {
                    register_code16(KC_VOLD);
                } else {
                    unregister_code16(KC_VOLD);
                }
                return false;
            }
        }
        return true;
    }break;
    case KC_F3:{
        if(confinfo.filp){
            if (confinfo.layer) {
                // if (record->event.pressed) {
                //     register_code(KC_LGUI);
                //     register_code(KC_TAB);
                // } else {
                //     unregister_code(KC_LGUI);
                //     unregister_code(KC_TAB);
                // }
                // return false;
            } else {
                if (record->event.pressed) {
                    register_code16(KC_VOLU);
                } else {
                    unregister_code16(KC_VOLU);
                }
                return false;
            }
        }
        return true;
    }break;
    case KC_F4:{
        if(confinfo.filp){
            if (confinfo.layer) {
                // if (record->event.pressed) {
                //     register_code(KC_LGUI);
                //     register_code(KC_E);
                // } else {
                //     unregister_code(KC_LGUI);
                //     unregister_code(KC_E);
                // }
                // return false;
            } else {
                if (record->event.pressed) {
                    register_code16(KC_MUTE);
                } else {
                    unregister_code16(KC_MUTE);
                }
                return false;
            }
        }
        return true;
    }break;
    case KC_F5:{
        if(confinfo.filp){
            if (confinfo.layer) {
                // if (record->event.pressed) {
                //     rgb_matrix_decrease_val();
                // }
            } else {
                if (record->event.pressed) {
                    register_code16(KC_MSTP);
                } else {
                    unregister_code16(KC_MSTP);
                }
                return false;
            }
        }
        return true;
    }break;
    case KC_F6:{
        if(confinfo.filp){
            if (confinfo.layer) {
                // if (record->event.pressed) {
                //     rgb_matrix_increase_val();
                // }
                // return false;
            } else {
                if (record->event.pressed) {
                    register_code16(KC_MPRV);
                } else {
                    unregister_code16(KC_MPRV);
                }
                return false;
            }
        }
        return true;
    }break;
    case KC_F7:{
        if(confinfo.filp){
            if (confinfo.layer) {
                // if (record->event.pressed) {
                //     register_code(KC_MPRV);
                // } else {
                //     unregister_code(KC_MPRV);
                // }
                // return false;
            } else {
                if (record->event.pressed) {
                    register_code16(KC_MPLY);
                } else {
                    unregister_code16(KC_MPLY);
                }
                return false;
            }
        }
        return true;
    } break;
    case KC_F8:{
       if(confinfo.filp){
            if (confinfo.layer) {
                // if (record->event.pressed) {
                //     register_code(KC_MPLY);
                // } else {
                //     unregister_code(KC_MPLY);
                // }
                // return false;
            } else {
                if (record->event.pressed) {
                    register_code16(KC_MNXT);
                } else {
                    unregister_code16(KC_MNXT);
                }
                return false;
            }
        }
        return true;
    } break;
    case KC_F9:{
        if(confinfo.filp){
            if (confinfo.layer) {
                // if (record->event.pressed) {
                //     register_code(KC_MNXT);
                // } else {
                //     unregister_code(KC_MNXT);
                // }
                // return false;
            } else {
                if (record->event.pressed) {
                    register_code16(KC_MAIL);
                } else {
                    unregister_code16(KC_MAIL);
                }
                return false;
            }
        }
        return true;
    }break;
    case KC_F10:{
        if(confinfo.filp){
            if (confinfo.layer) {
                // if (record->event.pressed) {
                //     register_code(KC_MUTE);
                // } else {
                //     unregister_code(KC_MUTE);
                // }
                // return false;
            } else {
                if (record->event.pressed) {
                    register_code16(KC_WHOM);
                } else {
                    unregister_code16(KC_WHOM);
                }
                return false;
            }
        }
        return true;
    }break;
    case KC_F11:{
        if(confinfo.filp){
            if (confinfo.layer) {
                // if (record->event.pressed) {
                //     register_code(KC_VOLD);
                // } else {
                //     unregister_code(KC_VOLD);
                // }
                // return false;
            } else {
                if (record->event.pressed) {
                    register_code16(KC_CALC);
                } else {
                    unregister_code16(KC_CALC);
                }
                return false;
            }
        }
        return true;
    }break;
    case KC_F12:{
        if(confinfo.filp){
            if (confinfo.layer) {
                // if (record->event.pressed) {
                //     register_code(KC_VOLU);
                // } else {
                //     unregister_code(KC_VOLU);
                // }

                // return false;
            } else {
                if (record->event.pressed) {
                    register_code16(KC_WSCH);
                } else {
                    unregister_code16(KC_WSCH);
                }
                return false;
            }
        }
        return true;
    }break;

    case LWIN_TAB: {
            if (record->event.pressed) {
                register_code(KC_LGUI);
                register_code(KC_TAB);
            }
            else{
                unregister_code(KC_LGUI);
                unregister_code(KC_TAB);
            }
            return false;
        } break;
    case LWIN_E: {
            if (record->event.pressed) {
                register_code(KC_LGUI);
                register_code(KC_E);
            }
            else{
                unregister_code(KC_LGUI);
                unregister_code(KC_E);
            }
            return false;
        } break;
     case KC_RGUI: {
            if((confinfo.no_gui && (get_highest_layer(default_layer_state) < 2)) || reset_filp){
                return false;
            }
        } break;
    case KC_LGUI: {
           if((confinfo.no_gui && (get_highest_layer(default_layer_state) < 2)) || reset_filp){
                return false;
            }

        } break;

     case QK_BOOT: {
            if (record->event.pressed) {
                IDEBUG_USER_INFO("into boot!!!\r\n");
                extern void bootmagic_lite_reset_eeprom(void);
                bootmagic_lite_reset_eeprom();
                bootloader_jump();
            }
        } break;
    case KC_1: {
            if (confinfo.filp){
                if (!confinfo.layer){
                    if (record->event.pressed) {
                        register_code(KC_F1);
                    }
                    else{
                        unregister_code(KC_F1);
                    }
                }
                else{
                    if (record->event.pressed) {
                        register_code(KC_BRID);
                    }
                    else{
                        unregister_code(KC_BRID);
                    }
                }
                return false;
            }
            return true;
        } break;
    case KC_2: {
            if (confinfo.filp){
                if (!confinfo.layer){
                    if (record->event.pressed) {
                        register_code(KC_F2);
                    }
                    else{
                        unregister_code(KC_F2);
                    }
                }
                else{
                    if (record->event.pressed) {
                        register_code(KC_BRIU);
                    }
                    else{
                        unregister_code(KC_BRIU);
                    }
                }
                return false;
            }
            return true;
        } break;
    case KC_3: {
            if (confinfo.filp){
                if (!confinfo.layer){
                    if (record->event.pressed) {
                        register_code(KC_F3);
                    }
                    else{
                        unregister_code(KC_F3);
                    }
                }
                else{
                    if (record->event.pressed) {
                        register_code(KC_LGUI);
                        register_code(KC_TAB);
                    }
                    else{
                        unregister_code(KC_LGUI);
                        unregister_code(KC_TAB);
                    }
                }
                return false;
            }
            return true;
        } break;
    case KC_4: {
           if (confinfo.filp){
                if (!confinfo.layer){
                    if (record->event.pressed) {
                        register_code(KC_F4);
                    }
                    else{
                        unregister_code(KC_F4);
                    }
                }
                else{
                    if (record->event.pressed) {
                        register_code(KC_LGUI);
                        register_code(KC_E);
                    }
                    else{
                         unregister_code(KC_LGUI);
                        unregister_code(KC_E);
                    }
                }
                return false;
            }
            return true;
        } break;
    case KC_5: {
           if (confinfo.filp){
                if (!confinfo.layer){
                    if (record->event.pressed) {
                        register_code(KC_F5);
                    }
                    else{
                        unregister_code(KC_F5);
                    }
                }
                else{
                    if (record->event.pressed) {
                        rgb_matrix_decrease_val();
                    }

                }
                return false;
            }
            return true;
        } break;
    case KC_6: {
            if (confinfo.filp){
                if (!confinfo.layer){
                    if (record->event.pressed) {
                        register_code(KC_F6);
                    }
                    else{
                        unregister_code(KC_F6);
                    }
                }
                else{
                    if (record->event.pressed) {
                        rgb_matrix_increase_val();
                    }
                }
                return false;
            }
            return true;
        } break;
    case KC_7: {
            if (confinfo.filp){
                if (!confinfo.layer){
                    if (record->event.pressed) {
                        register_code(KC_F7);
                    }
                    else{
                        unregister_code(KC_F7);
                    }
                }
                else{
                    if (record->event.pressed) {
                        register_code(KC_MPRV);
                    }
                    else{
                        unregister_code(KC_MPRV);
                    }
                }
                return false;
            }
            return true;
        } break;
    case KC_8: {
            if (confinfo.filp){
                if (!confinfo.layer){
                    if (record->event.pressed) {
                        register_code(KC_F8);
                    }
                    else{
                        unregister_code(KC_F8);
                    }
                }
                else{
                    if (record->event.pressed) {
                        register_code(KC_MPLY);
                    }
                    else{
                        unregister_code(KC_MPLY);
                    }
                }
                return false;
            }
            return true;
        } break;
    case KC_9: {
            if (confinfo.filp){
                if (!confinfo.layer){
                    if (record->event.pressed) {
                        register_code(KC_F9);
                    }
                    else{
                        unregister_code(KC_F9);
                    }
                }
                else{
                    if (record->event.pressed) {
                        register_code(KC_MNXT);
                    }
                    else{
                        unregister_code(KC_MNXT);
                    }
                }
                return false;
            }
            return true;
        } break;
    case KC_0: {
           if (confinfo.filp){
                if (!confinfo.layer){
                    if (record->event.pressed) {
                        register_code(KC_F10);
                    }
                    else{
                        unregister_code(KC_F10);
                    }
                }
                else{
                    if (record->event.pressed) {
                        register_code(KC_MUTE);
                    }
                    else{
                        unregister_code(KC_MUTE);
                    }
                }
                return false;
            }
            return true;
        } break;
    case KC_MINS: {
            if (confinfo.filp){
                if (!confinfo.layer){
                    if (record->event.pressed) {
                        register_code(KC_F11);
                    }
                    else{
                        unregister_code(KC_F11);
                    }
                }
                else{
                    if (record->event.pressed) {
                        register_code(KC_VOLD);
                    }
                    else{
                        unregister_code(KC_VOLD);
                    }
                }
                return false;
            }
            return true;
        } break;
    case KC_EQL: {
            if (confinfo.filp){
                if (!confinfo.layer){
                    if (record->event.pressed) {
                        register_code(KC_F12);
                    }
                    else{
                        unregister_code(KC_F12);
                    }
                }
                else{
                    if (record->event.pressed) {
                        register_code(KC_VOLU);
                    }
                    else{
                        unregister_code(KC_VOLU);
                    }
                }
                return false;
            }
            return true;
        } break;
    case KC_FILP: {
            if (record->event.pressed) {
                confinfo.filp = !confinfo.filp;
                eeprom_update_dword((uint32_t *)CONFINFO_EECONFIG_ADDR, confinfo.raw);
            }
            return false;
        } break;
    case RL_MOD:{ //侧灯模式
            if (record->event.pressed) {
                im_rgblight_increase();
                m2s.cmd               = 0xA6;
                transaction_rpc_exec(USER_SYNC_MMS, sizeof(m2s), &m2s, sizeof(s2m), &s2m);
            }
            return false;
        }break;
    case RGB_SPI:
        if (record->event.pressed) {
            if (!mm_get_rgb_enable()) return false;
            if (record->event.pressed) {
            //     if (rgb_matrix_get_speed() >= (RGB_MATRIX_SPD_STEP * 4)) {
            //         rgb_matrix_blink_set_interval_times(IM_MM_RGB_BLINK_INDEX_SPI, 250, 3);
            //         rgb_matrix_blink_set(IM_MM_RGB_BLINK_INDEX_SPI);
            //     }
            // }
        }
        return true;
    }break;
    case RGB_SPD:

            if (!mm_get_rgb_enable()) return false;
            if (record->event.pressed) {
                if (rgb_matrix_get_speed() <= RGB_MATRIX_SPD_STEP ) {
                    // rgb_matrix_blink_set_interval_times(IM_MM_RGB_BLINK_INDEX_SPD, 250, 3);
                    // rgb_matrix_blink_set(IM_MM_RGB_BLINK_INDEX_SPD);
                    rgb_matrix_set_speed(RGB_MATRIX_SPD_STEP);
                    return false;
                }
            }

        return true;
        break;
    case RGB_VAI:

            if (!mm_get_rgb_enable()) return false;


        return true;
        break;
    case RGB_VAD:

        if (!mm_get_rgb_enable()) return false;
        if (record->event.pressed) {
            if (rgb_matrix_get_val() <= RGB_MATRIX_VAL_STEP) {
                return false;
            }

        }

        return true;
        break;
    case MO(_TEST):{
        // if (record->event.pressed) {
        //     m2s.cmd               = 0xA4;
        //     // im_bat_req_flag =  true;
        // }
        // else{
        //     // im_bat_req_flag =  false;
        //     m2s.cmd               = 0xA5;
        // }
        // // m2s.body[0] = 1;
        // if (transaction_rpc_exec(USER_SYNC_MMS, sizeof(m2s), &m2s, sizeof(s2m), &s2m)) {
        //     if (s2m.resp == 0x00) {}
        //     IDEBUG_USER_INFO("Slave Sleep OK\n");
        // } else {
        //     IDEBUG_USER_INFO("Slave sync failed!\n");
        // }
    }break;
    case MM_BATQ:{
        // if (mm_eeconfig.devs == DEVS_USB) return false;
        if (record->event.pressed) {
            m2s.cmd               = 0xA4;
            im_bat_req_flag =  true;
        }
        else{
            im_bat_req_flag =  false;
            m2s.cmd               = 0xA5;
        }
        // if (get_highest_layer(layer_state) == _TEST) {
        //     m2s.body[0] = 1;
        // }
        if (transaction_rpc_exec(USER_SYNC_MMS, sizeof(m2s), &m2s, sizeof(s2m), &s2m)) {
            if (s2m.resp == 0x00) {}
            IDEBUG_USER_INFO("Slave Sleep OK\n");
        } else {
            IDEBUG_USER_INFO("Slave sync failed!\n");
        }
        return false;
    }break;
    case IM_BATQ:{
         if (record->event.pressed) {
            m2s.cmd               = 0xF0;
            im_bat_req_charging_flag =  true;
        }
        else{
            im_bat_req_charging_flag =  false;
            m2s.cmd               = 0xF1;
        }
        if (transaction_rpc_exec(USER_SYNC_MMS, sizeof(m2s), &m2s, sizeof(s2m), &s2m)) {
                if (s2m.resp == 0x00) {}
                IDEBUG_USER_INFO("Slave Sleep OK\n");
            } else {
                IDEBUG_USER_INFO("Slave sync failed!\n");
            }
        return false;
    }break;

    case LY_WBS:
        if (record->event.pressed) {
            rgb_matrix_blink_set_remain_time(IM_MM_RGB_BLINK_INDEX_MAC, 0); // 添加这句可以让进入这个条件时先停止win系统的指示灯闪烁
            rgb_matrix_blink_set_interval_times(IM_MM_RGB_BLINK_INDEX_WIN, 250, 1);
            rgb_matrix_blink_set(IM_MM_RGB_BLINK_INDEX_WIN);
            sys_indicator_led = false;
            confinfo.layer = false;
            eeprom_update_dword((uint32_t *)CONFINFO_EECONFIG_ADDR, confinfo.raw);
            set_single_persistent_default_layer(_LB);
            layer_move(_LB);
        }
        return false;
        break;
    case LY_MBS:
        if (record->event.pressed) {
            rgb_matrix_blink_set_remain_time(IM_MM_RGB_BLINK_INDEX_WIN, 0); // 添加这句可以让进入这个条件时先停止win系统的指示灯闪烁
            rgb_matrix_blink_set_interval_times(IM_MM_RGB_BLINK_INDEX_MAC, 250, 2);
            rgb_matrix_blink_set(IM_MM_RGB_BLINK_INDEX_MAC);
            sys_indicator_led = false;
            confinfo.layer = true;
            eeprom_update_dword((uint32_t *)CONFINFO_EECONFIG_ADDR, confinfo.raw);
            set_single_persistent_default_layer(_LM);
            layer_move(_LM);
        }
        return false;
        break;
    case HS_TOGG: {
        if(record->event.pressed){
            confinfo.no_gui = !confinfo.no_gui;
            master_to_slave_t m2s = {0};
            slave_to_master_t s2m = {0};
            if (confinfo.no_gui){
                m2s.cmd               = 0xAE;
            }
            else{
                m2s.cmd               = 0xAF;
            }
            transaction_rpc_exec(USER_SYNC_MMS, sizeof(m2s), &m2s, sizeof(s2m), &s2m);
            eeprom_update_dword((uint32_t *)CONFINFO_EECONFIG_ADDR, confinfo.raw);
            keymap_config.no_gui = confinfo.no_gui;
            eeconfig_update_keymap(keymap_config.raw);
        }
        return false;
    }break;
    case KC_LED:{
        if(record->event.pressed){
            master_to_slave_t m2s = {0};
            slave_to_master_t s2m = {0};
            m2s.cmd               = 0xC3;
            transaction_rpc_exec(USER_SYNC_MMS, sizeof(m2s), &m2s, sizeof(s2m), &s2m);
        }
    }break;
    case RGB_TOG: {
        if(record->event.pressed){
            if (is_keyboard_left()){
                confinfo.left_rgb_enable = !confinfo.left_rgb_enable;
                eeprom_update_dword((uint32_t *)CONFINFO_EECONFIG_ADDR, confinfo.raw);
            }
            master_to_slave_t m2s = {0};
            slave_to_master_t s2m = {0};
            mm_set_rgb_enable(!mm_get_rgb_enable());
            if (mm_get_rgb_enable()){  //rgb_matrix_is_enabled
                m2s.cmd               = 0xC1;
                setPinOutput(RGB_DRIVER_EN_PIN);
                writePinHigh(RGB_DRIVER_EN_PIN);
            }
            else{
                if (!(((get_highest_layer(default_layer_state) < 2 && confinfo.no_gui) || host_keyboard_led_state().caps_lock || mm_eeconfig.charging) && mm_eeconfig.devs == DEVS_USB))
                {
                    // setPinOutput(RGB_DRIVER_EN_PIN);
                    // writePinLow(RGB_DRIVER_EN_PIN);
                    rgb_tog_time = timer_read32();
                }
                m2s.cmd               = 0xC2;
            }
            if (transaction_rpc_exec(USER_SYNC_MMS, sizeof(m2s), &m2s, sizeof(s2m), &s2m)) {
                if (s2m.resp == 0x00) {}
                IDEBUG_USER_INFO("Slave Sleep OK\n");
            } else {
                IDEBUG_USER_INFO("Slave sync failed!\n");
            }
         }
        return false;
    }break;
    // case MM_GUI: {
    //     if(!confinfo.no_gui){
    //         if(record->event.pressed){
    //             register_code(KC_RGUI);
    //         }else {
    //             unregister_code(KC_RGUI);
    //         }
    //     }
    //     return true;
    // }break;

    case RGB_MOD: {
        if (record->event.pressed) {
            if (!mm_get_rgb_enable()) return false;
            record_rgbmatrix_mod(&(confinfo.record_last_mode));
            // master_to_slave_t m2s = {0};
            // slave_to_master_t s2m = {0};
            // m2s.cmd               = 0xC4;
            // m2s.body[0]        = rgb_matrix_get_mode();
            // transaction_rpc_exec(USER_SYNC_MMS, sizeof(m2s), &m2s, sizeof(s2m), &s2m);
        }
        eeconfig_confinfo_update();
        return false;
    } break;

    case RGB_SAI: { // 切换主灯效HS
        if (record->event.pressed) {
            if (!mm_get_rgb_enable()) return false;
            uint8_t hue = rgb_matrix_get_hue();
            uint8_t val = rgb_matrix_get_val();
            if (rgb_matrix_get_sat() >= 255 - RGB_MATRIX_SAT_STEP){

                rgb_matrix_sethsv(hue,255,val);
                record_color_write_s_data(255);
                return false;
            }
            if (rgb_matrix_get_sat() == 0){
                rgb_matrix_sethsv(hue,30,val);
                record_color_write_s_data(30);
                return false;
            }

            record_color_write_s_data(rgb_matrix_get_sat() + RGB_MATRIX_SAT_STEP);
            dprintf("record_color_read_s_data = %d\r\n",record_color_read_s_data());

        }
    } break;
    case RGB_SAD: { // 切换主灯效HS
        if (record->event.pressed) {
            if (!mm_get_rgb_enable()) return false;
            uint8_t hue = rgb_matrix_get_hue();
            uint8_t val = rgb_matrix_get_val();
            if (rgb_matrix_get_sat() <= RGB_MATRIX_SAT_STEP + 5){

                rgb_matrix_sethsv(hue,0,val);
                record_color_write_s_data(0);
                return false;
            }
            record_color_write_s_data(rgb_matrix_get_sat() - RGB_MATRIX_SAT_STEP);
            dprintf("record_color_read_s_data = %d\r\n",record_color_read_s_data());
        }
    } break;

    case RGB_HUI: { // 切换主灯效HS
        if (record->event.pressed) {
            if (!mm_get_rgb_enable()) return false;
            uint8_t sat = rgb_matrix_get_sat();
            // if (sat == 0){
            //     sat = 255;
            //     record_color_write_s_data(255);
            // }
            uint8_t val = rgb_matrix_get_val();
            if (rgb_matrix_get_hue() >= 255 - RGB_MATRIX_HUE_STEP){

                if (rgb_matrix_get_hue() != 255) {
                    rgb_matrix_sethsv(255,sat,val);
                    record_color_write_h_data(255);
                } else {
                    rgb_matrix_sethsv(30,sat,val); //30
                    record_color_write_h_data(30); //30
                }
                return false;
            }

            if (rgb_matrix_get_hue() == 0){
                rgb_matrix_sethsv(30,sat,val);
                record_color_write_h_data(30);
                return false;
            }

            record_color_write_h_data(rgb_matrix_get_hue() + RGB_MATRIX_HUE_STEP);
            dprintf("record_color_read_h_data = %d\r\n",record_color_read_h_data());
        }
    } break;
    case RGB_HUD: { // 切换主灯效HS
        if (record->event.pressed) {
            if (!mm_get_rgb_enable()) return false;
            uint8_t sat = rgb_matrix_get_sat();
            // if (sat == 0){
            //     sat = 255;
            //     record_color_write_s_data(255);
            // }
            if (rgb_matrix_get_hue() <= (RGB_MATRIX_HUE_STEP + 5)){
                // uint8_t sat = rgb_matrix_get_sat();
                uint8_t val = rgb_matrix_get_val();
                if (rgb_matrix_get_hue() != 0) {
                    rgb_matrix_sethsv(0,sat,val);
                    record_color_write_h_data(0);
                } else {
                    rgb_matrix_sethsv(230,sat,val);
                    record_color_write_h_data(230);
                }
                return false;
            }
            record_color_write_h_data(rgb_matrix_get_hue() - RGB_MATRIX_HUE_STEP);
            dprintf("record_color_read_h_data = %d\r\n",record_color_read_h_data());
        }
    } break;
    default:
        break;
    }
    return true;
}

void user_sync_mms_slave_handler(uint8_t in_buflen, const void* in_data, uint8_t out_buflen, void* out_data){
    const master_to_slave_t *m2s = (const master_to_slave_t*)in_data;
    slave_to_master_t *s2m = (slave_to_master_t*)out_data;
    IDEBUG_USER_INFO("m2s->cmd  = %d\r\n",m2s->cmd);
    switch(m2s->cmd)
    {
        case 0x55:  //sync multimode
            pov = m2s->body[2] & 0x7F;
            if (m2s->body[2] & 0x80){
                if (!mm_get_rgb_enable()) mm_set_rgb_enable(true);
            } else {
                if (mm_get_rgb_enable()) mm_set_rgb_enable(false);
            }
            if ((m2s->body[1] == DEVS_USB && mm_eeconfig.devs != DEVS_USB) || m2s->body[1] != DEVS_USB) {
                if (mm_eeconfig.devs != m2s->body[1]) {
                    if (!(mm_eeconfig.devs >= DEVS_BT1 && mm_eeconfig.devs <= DEVS_BT3 && m2s->body[1] >= DEVS_BT1 &&  m2s->body[1] <= DEVS_BT3))
                        led_switch_time = timer_read32();
                }
                mm_switch_mode(m2s->body[0], m2s->body[1], false);

            }
            // if (pov <= 5 && readPin(MM_BAT_CABLE_PIN) == 0 && m2s->body[1] != DEVS_USB && left_Battery > 5){

            //     writePinHigh(C4);
            // }
            // else{

            //     writePinLow(C4);
            // }

            // if (BY6102_ReadSOC() != -1){
            //     s2m->resp = soc_high;
            //     left_Battery = soc_high;
            // }
            // else{
            //     s2m->resp = 0xFF;
            // }

            break;
        case 0xAA:
            if(mm_eeconfig.devs != DEVS_USB){
                palSetLineMode(SERIAL_USART_TX_PIN, PAL_OUTPUT_TYPE_OPENDRAIN);
                palSetLineMode(SERIAL_USART_RX_PIN, PAL_OUTPUT_TYPE_OPENDRAIN);
            }
            s2m->resp = 0x00;

            mm_set_to_sleep();
            break;
        case 0xBB:
            im_led_deinit_user();
            IDEBUG_USER_INFO("BB\r\n");
            s2m->resp = 0x00;
            break;
        case 0xCC:
            im_led_init_user();
            IDEBUG_USER_INFO("CC\r\n");
            s2m->resp = 0x00;
            break;
        case 0xD4:
        pair_lback_timeout = true;
        break;
        case 0xD5:
        pair_lback_timeout = false;
        break;
        case 0xD6:{
            rgb_matrix_blink_set_interval_times(IM_MM_RGB_BLINK_INDEX_NKRO, 250, 1);
            rgb_matrix_blink_set(IM_MM_RGB_BLINK_INDEX_NKRO);
        }break;
        case 0xD7:{
            rgb_matrix_blink_set_interval_times(IM_MM_RGB_BLINK_INDEX_NKRO, 250, 3);
            rgb_matrix_blink_set(IM_MM_RGB_BLINK_INDEX_NKRO);
        }break;
         case 0xD8:{
            confinfo.led_brightness = m2s->body[0];
        }break;
        case 0xDD:
            // rgblight_mode(RGBLIGHT_DEFAULT_MODE);
            // rgblight_sethsv(0,255,RGBLIGHT_LIMIT_VAL);

            // extern bool is_rgblight_initialized;
            // extern void rgblight_init(void);
            // is_rgblight_initialized = false;
            // rgblight_init();
            // eeconfig_update_rgblight_default();
            // rgblight_enable();
            if (m2s->body[0]) {
                mm_set_rgb_enable(true);
                setPinOutput(RGB_DRIVER_EN_PIN);
                writePinHigh(RGB_DRIVER_EN_PIN);
                keymap_config.no_gui = 1;
                eeconfig_update_keymap(keymap_config.raw);
                rgb_matrix_blink_set_interval_times(RGB_MATRIX_BLINK_INDEX_ALL, 250, 3);
                // im_reset_settings();
            } else {
                rgb_matrix_blink_set_interval_times(RGB_MATRIX_BLINK_INDEX_ALL, 250, 1);
            }
            rgb_matrix_blink_set(RGB_MATRIX_BLINK_INDEX_ALL);
            confinfo.no_gui = false;
            eeprom_update_dword((uint32_t *)CONFINFO_EECONFIG_ADDR, confinfo.raw);
            i = r = g = b = dir = 0;
            dynamic_keymap_reset();
            eeconfig_update_rgb_matrix_default();
            s2m->resp = 0x00;
            break;
        case 0xA1:
            // rgb_matrix_blink_set_interval_times(IM_MM_RGB_BLINK_INDEX_IND, 500, 1);
            // rgb_matrix_blink_set(IM_MM_RGB_BLINK_INDEX_IND);
            s2m->resp = 0x00;
            break;
        case 0xA2:
            // rgb_matrix_blink_set_interval_times(IM_MM_RGB_BLINK_INDEX_IND, 200, 1);
            // rgb_matrix_blink_set(IM_MM_RGB_BLINK_INDEX_IND);
            s2m->resp = 0x00;
            break;
         case 0xA3:
            limiting_clearance();
            s2m->resp = 0x00;
            break;
        case 0xA4:
            im_bat_req_flag = true;
            s2m->resp = 0x00;
            break;
        case 0xA5:
            im_bat_req_flag = false;
            s2m->resp = 0x00;
            break;
        case 0xA6:
            im_rgblight_increase();
            s2m->resp = 0x00;
            break;
        case 0xAC:
            writePinLow(C4);
            im_set_lowpower(true);
            s2m->resp = 0x00;
            break;
        case 0xC1:
           mm_set_rgb_enable(true);
            setPinOutput(RGB_DRIVER_EN_PIN);
            writePinHigh(RGB_DRIVER_EN_PIN);
           s2m->resp = 0x00;
           break;
        case 0xC2:
            mm_set_rgb_enable(false);
            if (!(((get_highest_layer(default_layer_state) < 2 && confinfo.no_gui) || host_keyboard_led_state().caps_lock || mm_eeconfig.charging) && mm_eeconfig.devs == DEVS_USB)){
                // setPinOutput(RGB_DRIVER_EN_PIN);
                // writePinLow(RGB_DRIVER_EN_PIN);
                rgb_tog_time = timer_read32();
            }
            s2m->resp = 0x00;
            break;
        case 0xC3:
            led_fn_switch_time = timer_read32();
            break;
        case 0xC4:
            rgb_matrix_mode(m2s->body[0]);
            break;
        case 0xC5:
            mm_set_rgb_enable(true);
            writePinHigh(RGB_DRIVER_EN_PIN);
            led_power_en = true;
        break;
        case 0xC6:
            led_power_en = false;
        break;
        case 0xD1:
            im_led_init_user();
            s2m->resp = 0x01;
            break;
        case 0xD2:
            usb_connect_succeed = true;
            s2m->resp = 0x01;
            break;
        case 0xD3:
            usb_connect_succeed = false;
            s2m->resp = 0x01;
            break;
         case 0xF0:
            im_bat_req_charging_flag = true;
            s2m->resp = 0x00;
            break;
        case 0xF1:
             im_bat_req_charging_flag = false;
            s2m->resp = 0x01;
            break;
        case 0xAE:
            keymap_config.no_gui = 1;
            confinfo.no_gui = 1;
            eeconfig_update_keymap(keymap_config.raw);
            eeprom_update_dword((uint32_t *)CONFINFO_EECONFIG_ADDR, confinfo.raw);
            s2m->resp = 0x00;
            break;
        case 0xAF:
            keymap_config.no_gui = 0;
            confinfo.no_gui = 0;
            eeconfig_update_keymap(keymap_config.raw);
            eeprom_update_dword((uint32_t *)CONFINFO_EECONFIG_ADDR, confinfo.raw);
            s2m->resp = 0x00;
            break;
        default:
            break;
    }

}

int8_t BY6102_ReadSOC(void) {

    i2c_status_t status;

    // 读取 0x1C 高位数据

    status = i2c_readReg(0xAB, 0x1C, &soc_high, 1, 10);
    if (status != I2C_STATUS_SUCCESS || soc_high > 100) {
        // dprintf("Failed to read SOC High BY6102\r\n");
        // IDEBUG_USER_INFO("Failed to read SOC High BY6102\r\n");
        return -1;  // 读取失败，返回 0
    }
    // IDEBUG_USER_INFO("read SOC High BY6102 = %d\r\n",soc_high);
    // dprintf("read SOC High BY6102 = %d\r\n",soc_high);
    // 读取 0x1D 低位数据
    // status = i2c_readReg(0xAB, 0x1D, &soc_low, 1, 100);
    // if (status != I2C_STATUS_SUCCESS) {
    //     // DEBUG("Failed to read SOC Low BY6102\r\n");
    //     return 0;  // 读取失败，返回 0
    // }

    // 合并高低位数据
    // uint16_t soc_raw = (soc_high << 8) | soc_low;

    // 返回百分比
    // return (uint8_t)(soc_raw & 0xFF);  // SOC 百分比直接用高 8 位
    // if (soc_high > 100) soc_high = 100;
    return 0;  // SOC 百分比直接用高 8 位
}

// uint8_t get_rgb_matrix_index(void) {
//     uint8_t now_mode_temp = rgb_matrix_get_mode();
//     bool mode_get_flage = false;
//     uint8_t index = 0;
//     for (index = 0; index < sizeof(rgbmatrix_buff) / sizeof(rgbmatrix_buff[0]); index++) {
//         if (now_mode_temp == rgbmatrix_buff[index]) {
//             mode_get_flage = true;
//             break;
//         }
//     }
//     return index;
// }

// // 写入对应灯效的HS所对应的索引
// void record_color_write_data(uint8_t color_index) {
//     uint8_t hs_mode = get_rgb_matrix_index();

//     uint8_t *ptr = (uint8_t *)(((uint32_t)CONFINFO_EECONFIG_ADDR + 4) + hs_mode);
//     eeprom_write_byte(ptr, color_index);
// }

// // 读取对应灯效的HS所对应的索引
// uint8_t record_color_read_data(void) {
//     uint8_t hs_mode = get_rgb_matrix_index();

//     const uint8_t *ptr      = (const uint8_t *)(((uint32_t)CONFINFO_EECONFIG_ADDR + 4) + hs_mode);
//     uint8_t hs_c            = eeprom_read_byte(ptr);

//     if (hs_c > RGB_HSV_MAX) {
//         return 0;
//     } else {
//         return hs_c;
//     }
// }

// void record_rgbmatrix_mod(uint8_t* last_mode) {
//     record_rgbmatrix_increase(last_mode);
//     uint8_t rgb_hsv_index = record_color_read_data();
//     DEBUG("rgb_hsv_index =%d \r\n",rgb_hsv_index);
//     rgb_matrix_sethsv(rgb_hsvs[rgb_hsv_index][0],rgb_hsvs[rgb_hsv_index][1], rgb_matrix_get_val());
// }

#include "spi_master.h"
#define AWINIC_ID 0b1010 << 4
#define MODEIO_TIME 500
bool AW20216_read(pin_t cs_pin, uint8_t page, uint8_t reg, uint8_t* data, uint8_t len) {
    static uint8_t s_spi_transfer_buffer[2] = {0};

    static uint32_t scan_timer = 0x00;
    static uint32_t scan1_timer = 0x00;
    if (scan_timer == 0)  scan_timer = timer_read32();
    if (scan1_timer == 0)  scan1_timer = timer_read32();
    if(cs_pin == DRIVER_1_CS){
        if ((timer_elapsed32(scan_timer) <= (MODEIO_TIME))) {
            return false;
        }
        scan_timer = timer_read32();
        if (!spi_start(cs_pin, false, 0, 4)) {
            spi_stop();
            return false;
        }

        s_spi_transfer_buffer[0] = (AWINIC_ID | page | 1);
        s_spi_transfer_buffer[1] = reg;

        if (spi_transmit(s_spi_transfer_buffer, 2) != SPI_STATUS_SUCCESS) {
            spi_stop();
            return false;
        }

        if (spi_receive(data, len) != SPI_STATUS_SUCCESS) {
            spi_stop();
            return false;
        }

        spi_stop();
        if(!(*data & (0x01))) {
            *data = 0xAE;
            if (!AW20216_write(DRIVER_1_CS,0,0x2F,data,1)) return false;
            wait_ms(5);
            *data = 0b10110000 | 1;
            if (!AW20216_write(DRIVER_1_CS, 0, 0, data,1)) return false;
            wait_ms(5);
            AW20216_init(DRIVER_1_CS,DRIVER_1_EN);

        }
    }else if(cs_pin == DRIVER_2_CS){
        if ((timer_elapsed32(scan1_timer) <= (MODEIO_TIME))) {
            return false;
        }
        scan1_timer = timer_read32();
        if (!spi_start(cs_pin, false, 0, 4)) {
            spi_stop();
            return false;
        }

        s_spi_transfer_buffer[0] = (AWINIC_ID | page | 1);
        s_spi_transfer_buffer[1] = reg;

        if (spi_transmit(s_spi_transfer_buffer, 2) != SPI_STATUS_SUCCESS) {
            spi_stop();
            return false;
        }

        if (spi_receive(data, len) != SPI_STATUS_SUCCESS) {
            spi_stop();
            return false;
        }

        spi_stop();
        if(!(*data & (0x01))) {
            *data = 0xAE;
            if (!AW20216_write(DRIVER_2_CS,0,0x2F,data,1)) return false;
            wait_ms(5);
            *data = 0b10110000 | 1;
            if (!AW20216_write(DRIVER_1_CS, 0, 0, data,1)) return false;
            wait_ms(5);
            AW20216_init(DRIVER_2_CS,DRIVER_2_EN);
        }
    }


    return true;
}
