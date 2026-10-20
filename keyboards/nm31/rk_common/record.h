
#pragma once

void record_rgbmatrix_increase(uint8_t* last_mode);
uint8_t record_led_channel(uint8_t record_channel ,uint8_t* last_mode);
void all_color_reset(void);
void record_color_write_h_data(uint8_t color_index);
void record_color_write_s_data(uint8_t color_index);
uint8_t record_color_read_h_data(void);
uint8_t record_color_read_s_data(void);
void record_color_hsv(void);
void record_rgbmatrix_mod(uint8_t* last_mode);
void rgb_breathing_indicator(HSV hsv, uint8_t speed, uint8_t index);

#ifdef RGBLIGHT_ENABLE
uint8_t record_rgblight_mod(uint8_t rgblight_num);
#endif

bool rgb_matrix_indicators_advanced_rk(uint8_t led_min, uint8_t led_max);

// 使用层级区分实现WIN/MAC双系统功能
#ifndef KEYMAP_MAC_DISABLE
#    ifdef keymap_is_mac
#        undef keymap_is_mac
#    endif
#    ifdef keymap_mac_toggle
#        undef  keymap_mac_toggle
#    endif

#    define keymap_is_mac() (get_highest_layer(default_layer_state) == _LM)
#    define keymap_mac_toggle()                                          \
        do {                                                             \
            if (get_highest_layer(default_layer_state) == _LB) {         \
                set_single_persistent_default_layer(_LM);                \
                layer_move(0);                                           \
            } else if (get_highest_layer(default_layer_state) == _LM) {  \
                set_single_persistent_default_layer(_LB);                \
                layer_move(0);                                           \
            }                                                            \
        } while (0)
#endif
