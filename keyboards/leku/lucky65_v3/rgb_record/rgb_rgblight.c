// Copyright 2024 SDK (@sdk66)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "rgb_rgblight.h"

void rgb_light_color_set(uint8_t r, uint8_t g, uint8_t b) {
    for(uint8_t i = 0; i < RGBLED_NUM; i++) {
        rgb_matrix_set_color(i, r, g, b);
    }
}

static uint8_t neon_color_index = 0;
void hs_rgblight_increase(uint8_t rgblight_mode) {

    switch (rgblight_mode)
    {
        case 0: {
            static uint16_t rainbow_counter = 0;
            static uint32_t last_update = 0;
            uint32_t interval = 20;

            if (timer_elapsed32(last_update) >= interval) {
                last_update = timer_read32();
                rainbow_counter += 3;
            }

            for (uint8_t i = 0; i < RGBLED_NUM; i++) {
                uint8_t hue = (rainbow_counter + (RGBLED_NUM - 1 - i) * 8) % 255;
                HSV hsv = { .h = hue, .s = 255, .v = (RGBLIGHT_LIMIT_VAL) };
                RGB rgb = hsv_to_rgb(hsv);
                rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
            }
        } break;

        case 1: {
            static uint16_t rainbow_counter = 0;
            static uint32_t last_update = 0;
            uint32_t interval = 20;

            if (timer_elapsed32(last_update) >= interval) {
                last_update = timer_read32();
                rainbow_counter += 3;
            }

            for (uint8_t i = 0; i < RGBLED_NUM; i++) {
                uint8_t hue = (rainbow_counter + i * 8) % 255;
                HSV hsv = { .h = hue, .s = 255, .v = (RGBLIGHT_LIMIT_VAL) };
                RGB rgb = hsv_to_rgb(hsv);
                rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
            }
        } break;

        case 2: {
            static uint16_t rainbow_counter = 0;
            static uint32_t last_update = 0;
            uint32_t interval = 20;
            neon_color_index = 0;

            if (timer_elapsed32(last_update) >= interval) {
                last_update = timer_read32();
                rainbow_counter += 3;
            }

            uint8_t center = RGBLED_NUM / 2;
            for (uint8_t i = 0; i < RGBLED_NUM; i++) {
                uint8_t distance = (i > center) ? (i - center) : (center - i);
                uint8_t hue = (rainbow_counter + distance * 12) % 255;  
                HSV hsv = { .h = hue, .s = 255, .v = (RGBLIGHT_LIMIT_VAL) };
                RGB rgb = hsv_to_rgb(hsv);
                rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
            }
        } break;

        case 3: { 
            static const RGB neon_colors[] = {
                {RGBLIGHT_LIMIT_VAL, 0, 0},        
                {RGBLIGHT_LIMIT_VAL, RGBLIGHT_LIMIT_VAL/5, 0},   
                {RGBLIGHT_LIMIT_VAL-10, RGBLIGHT_LIMIT_VAL-10, 0},  
                {0, RGBLIGHT_LIMIT_VAL, 0},         
                {0, RGBLIGHT_LIMIT_VAL-10, RGBLIGHT_LIMIT_VAL-10},   
                {0, 0, RGBLIGHT_LIMIT_VAL},        
                {RGBLIGHT_LIMIT_VAL-10, 0, RGBLIGHT_LIMIT_VAL-10},   
                {RGBLIGHT_LIMIT_VAL-10, RGBLIGHT_LIMIT_VAL-10, RGBLIGHT_LIMIT_VAL-10}  
            };

            static uint8_t neon_brightness = 0;
            static bool neon_increasing = true;

            static uint32_t last_update = 0;
            uint32_t interval = 15;  

            if (timer_elapsed32(last_update) >= interval) {
                last_update = timer_read32();

                uint8_t max_val = RGBLIGHT_LIMIT_VAL - 30;
                if (neon_increasing) {
                    neon_brightness++;
                    if (neon_brightness >= max_val) {
                        neon_brightness = max_val;
                        neon_increasing = false;
                    }
                } else {
                    if (neon_brightness > 0) {
                        neon_brightness--;
                    } else {
                        neon_increasing = true;
                        neon_color_index = (neon_color_index + 1) % (sizeof(neon_colors) / sizeof(neon_colors[0]));
                    }
                }
            }

            RGB current_color = neon_colors[neon_color_index];
            for (uint8_t i = 0; i < RGBLED_NUM; i++) { 
                rgb_matrix_set_color(i,
                    (current_color.g * neon_brightness) / (RGBLIGHT_LIMIT_VAL - 30),
                    (current_color.r * neon_brightness) / (RGBLIGHT_LIMIT_VAL - 30),
                    (current_color.b * neon_brightness) / (RGBLIGHT_LIMIT_VAL - 30));
            }
        } break;

        case 4: 
            neon_color_index = 0;
            rgb_light_color_set(RGBLIGHT_LIMIT_VAL, 0x00, 0x00); break;

        case 5: 
            rgb_light_color_set(RGBLIGHT_LIMIT_VAL - 30, ((RGBLIGHT_LIMIT_VAL-30)/5),  0); break;

        case 6: 
            rgb_light_color_set(RGBLIGHT_LIMIT_VAL - 30, RGBLIGHT_LIMIT_VAL-30, 0); break;

        case 7: 
            rgb_light_color_set(0, RGBLIGHT_LIMIT_VAL, 0); break;

        case 8: 
            rgb_light_color_set(0, RGBLIGHT_LIMIT_VAL - 30, RGBLIGHT_LIMIT_VAL - 30); break;

        case 9: 
            rgb_light_color_set(0, 0, RGBLIGHT_LIMIT_VAL); break;

        case 10: 
            rgb_light_color_set(RGBLIGHT_LIMIT_VAL - 30, 0, RGBLIGHT_LIMIT_VAL - 30); break;

        case 11: 
            rgb_light_color_set(RGBLIGHT_LIMIT_VAL - 50, RGBLIGHT_LIMIT_VAL - 50, RGBLIGHT_LIMIT_VAL - 50); break;

        case 12:
            rgb_light_color_set(0, 0, 0); break;
    }
}
