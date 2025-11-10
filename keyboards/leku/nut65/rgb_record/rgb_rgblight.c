// Copyright 2024 SDK (@sdk66)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "rgb_rgblight.h"

void rgb_light_color_set(uint8_t r, uint8_t g, uint8_t b) {
    for(uint8_t i = 0; i < RGBLED_NUM; i++) {
        rgb_matrix_set_color(i, r, g, b);
    }
}

void hs_rgblight_increase(uint8_t rgblight_mode, uint8_t rl_rgb_brightness_val, uint8_t rl_val) {

    switch (rgblight_mode)
    {
        case 0: { 
            static uint16_t rainbow_counter = 0;
            static uint32_t last_update = 0;
            uint32_t interval = 15;

            if (timer_elapsed32(last_update) >= interval) {
                last_update = timer_read32();
                rainbow_counter += 3;
            }

            for (uint8_t i = 0; i < RGBLED_NUM; i++) {
                uint8_t led_index = RGB_MATRIX_LED_COUNT - RGBLED_NUM + i;
               
                uint8_t hue = (rainbow_counter + (RGBLED_NUM - 1 - i) * 8) % 255;
                HSV hsv = { .h = hue, .s = 255, .v = (rl_rgb_brightness_val+(rl_val*5)) };
                RGB rgb = hsv_to_rgb(hsv);
                rgb_matrix_set_color(led_index, rgb.r, rgb.g, rgb.b);
            }
        } break;

        case 1: { 
            static uint16_t rainbow_counter = 0;
            static uint32_t last_update = 0;
            uint32_t interval = 15;

            if (timer_elapsed32(last_update) >= interval) {
                last_update = timer_read32();
                rainbow_counter += 3;
            }

            for (uint8_t i = 0; i < RGBLED_NUM; i++) {
                uint8_t led_index = RGB_MATRIX_LED_COUNT - RGBLED_NUM + i;
                uint8_t hue = (rainbow_counter + i * 8) % 255;
                HSV hsv = { .h = hue, .s = 255, .v = (rl_rgb_brightness_val+(rl_val*5)) };
                RGB rgb = hsv_to_rgb(hsv);
                rgb_matrix_set_color(led_index, rgb.r, rgb.g, rgb.b);
            }
        } break;

        case 2: { 
            static uint16_t rainbow_counter = 0;
            static uint32_t last_update = 0;
            uint32_t interval = 15;

            if (timer_elapsed32(last_update) >= interval) {
                last_update = timer_read32();
                rainbow_counter += 3;
            }

            uint8_t center = RGBLED_NUM / 2;
            for (uint8_t i = 0; i < RGBLED_NUM; i++) {
                uint8_t led_index = RGB_MATRIX_LED_COUNT - RGBLED_NUM + i;
                uint8_t distance = (i > center) ? (i - center) : (center - i);
                uint8_t hue = (rainbow_counter + distance * 12) % 255;  
                HSV hsv = { .h = hue, .s = 255, .v = (rl_rgb_brightness_val+(rl_val*5)) };
                RGB rgb = hsv_to_rgb(hsv);
                rgb_matrix_set_color(led_index, rgb.r, rgb.g, rgb.b);
            }
        } break;

        case 3: { 
            RGB neon_colors[8]; 

            neon_colors[0] = (RGB){rl_rgb_brightness_val, 0, 0};
            neon_colors[1] = (RGB){rl_rgb_brightness_val, rl_rgb_brightness_val / 5, 0}; 
            neon_colors[2] = (RGB){
                rl_rgb_brightness_val - (rl_val * 2),
                rl_rgb_brightness_val - (rl_val * 2),
                0}; 
            neon_colors[3] = (RGB){0, rl_rgb_brightness_val, 0}; 
            neon_colors[4] = (RGB){
                0,
                rl_rgb_brightness_val - (rl_val * 2),
                rl_rgb_brightness_val - (rl_val * 2)}; 
            neon_colors[5] = (RGB){0, 0, rl_rgb_brightness_val}; 
            neon_colors[6] = (RGB){
                rl_rgb_brightness_val - (rl_val * 2),
                0,
                rl_rgb_brightness_val - (rl_val * 2)}; 
            neon_colors[7] = (RGB){
                rl_rgb_brightness_val / 2,
                rl_rgb_brightness_val / 2,
                rl_rgb_brightness_val / 2};

            static uint8_t neon_color_index = 0;
            static uint8_t neon_brightness = 0;
            static bool neon_increasing = true;

            static uint32_t last_update = 0;
            uint32_t interval = 15;

            switch (rl_val) {
                case 0: interval = 45; break;
                case 1: interval = 20; break;
                case 2: interval = 29; break;
                case 3: interval = 20; break;
                case 4: interval = 15; break;
                default: break;
            }

            if (timer_elapsed32(last_update) >= interval) {
                last_update = timer_read32();

                uint8_t max_val = rl_rgb_brightness_val;
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
                        neon_color_index = (neon_color_index + 1) % 8;
                    }
                }
            }

            RGB current_color = neon_colors[neon_color_index];
            for (uint8_t i = RGB_MATRIX_LED_COUNT - RGBLED_NUM; i < RGB_MATRIX_LED_COUNT; i++) {
                rgb_matrix_set_color(i,
                    (current_color.g * neon_brightness) / rl_rgb_brightness_val,
                    (current_color.r * neon_brightness) / rl_rgb_brightness_val,
                    (current_color.b * neon_brightness) / rl_rgb_brightness_val);
            }
        } break;

        case 4: 
            for (uint8_t i = RGB_MATRIX_LED_COUNT - RGBLED_NUM; i < RGB_MATRIX_LED_COUNT; i++) {
                rgb_matrix_set_color(i, rl_rgb_brightness_val, 0, 0);
            } break;

        case 5: 
            for (uint8_t i = RGB_MATRIX_LED_COUNT - RGBLED_NUM; i < RGB_MATRIX_LED_COUNT; i++) {
                rgb_matrix_set_color(i, rl_rgb_brightness_val, rl_rgb_brightness_val/5,  0);
            } break;

        case 6: 
            for (uint8_t i = RGB_MATRIX_LED_COUNT - RGBLED_NUM; i < RGB_MATRIX_LED_COUNT; i++) {
                rgb_matrix_set_color(i, rl_rgb_brightness_val - (rl_val*4), rl_rgb_brightness_val - (rl_val*4), 0);
            } break;

        case 7: 
            for (uint8_t i = RGB_MATRIX_LED_COUNT - RGBLED_NUM; i < RGB_MATRIX_LED_COUNT; i++) {
                rgb_matrix_set_color(i, 0, rl_rgb_brightness_val, 0);
            } break;

        case 8: 
            for (uint8_t i = RGB_MATRIX_LED_COUNT - RGBLED_NUM; i < RGB_MATRIX_LED_COUNT; i++) {
                rgb_matrix_set_color(i, 0, rl_rgb_brightness_val, rl_rgb_brightness_val);
            } break;

        case 9: 
            for (uint8_t i = RGB_MATRIX_LED_COUNT - RGBLED_NUM; i < RGB_MATRIX_LED_COUNT; i++) {
                rgb_matrix_set_color(i, 0, 0, rl_rgb_brightness_val);
            } break;

        case 10: 
            for (uint8_t i = RGB_MATRIX_LED_COUNT - RGBLED_NUM; i < RGB_MATRIX_LED_COUNT; i++) {
                rgb_matrix_set_color(i, rl_rgb_brightness_val - (rl_val*4), 0, rl_rgb_brightness_val - (rl_val*4));
            } break;

        case 11: 
            for (uint8_t i = RGB_MATRIX_LED_COUNT - RGBLED_NUM; i < RGB_MATRIX_LED_COUNT; i++) {
                rgb_matrix_set_color(i, rl_rgb_brightness_val/2, rl_rgb_brightness_val/2, rl_rgb_brightness_val/2);
            } break;

        case 12: 
            for (uint8_t i = RGB_MATRIX_LED_COUNT - RGBLED_NUM; i < RGB_MATRIX_LED_COUNT; i++) {
                rgb_matrix_set_color(i, 0, 0, 0);
            } break;

        default:
            break;
    }
}