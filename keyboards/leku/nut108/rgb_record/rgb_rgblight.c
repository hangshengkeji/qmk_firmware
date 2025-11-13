// Copyright 2024 SDK (@sdk66)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "rgb_rgblight.h"

#define RGBLED_START_INDEX (RGB_MATRIX_LED_COUNT - RGBLED_NUM)
#define RGBLED_END_INDEX   RGB_MATRIX_LED_COUNT
#define SIDE_LIGHT_MODE_VAI    4

static const uint8_t brightness_levels[SIDE_LIGHT_MODE_VAI] = {
    (RGBLIGHT_LIMIT_VAL * 25) / 100,
    (RGBLIGHT_LIMIT_VAL * 50) / 100,
    (RGBLIGHT_LIMIT_VAL * 75) / 100,
    RGBLIGHT_LIMIT_VAL
};

static const uint8_t colorful_brightness_levels[SIDE_LIGHT_MODE_VAI] = {
    20,
    32,
    45,
    60
};

static const uint8_t breathe_brightness_levels[SIDE_LIGHT_MODE_VAI] = {
    50,
    80,
    120,
    150
};

static const uint8_t two_color_brightness_levels[SIDE_LIGHT_MODE_VAI] = {
    3,
    5,
    8,
    10
};

static const uint8_t white_brightness_levels[SIDE_LIGHT_MODE_VAI] = {
    3,
    5,
    7,
    9
};

void rgb_light_color_set(uint8_t r, uint8_t g, uint8_t b) {
    for (uint8_t i = RGBLED_START_INDEX; i < RGBLED_END_INDEX; i++) {
        rgb_matrix_set_color(i, r, g, b);
    }
}

static uint8_t neon_color_index = 0;
static bool initialized = false;
static uint8_t last_mode = 0xff;
void hs_rgblight_increase(uint8_t rgblight_mode, uint8_t brightness_level) {

    if (brightness_level > (SIDE_LIGHT_MODE_VAI -1) ) brightness_level = SIDE_LIGHT_MODE_VAI - 1;
    uint8_t rgblight_val = brightness_levels[brightness_level];
    uint8_t colorful_rgblight_val = colorful_brightness_levels[brightness_level];
    uint8_t two_color_rgblight_val = two_color_brightness_levels[brightness_level];

    if (last_mode != rgblight_mode) {
        last_mode = rgblight_mode;
        initialized = false;
    }

    switch (rgblight_mode) {
        case 0: { 
            static uint16_t rainbow_counter = 0;
            static uint32_t last_update = 0;
            uint32_t interval = 20;

            if (timer_elapsed32(last_update) >= interval) {
                last_update = timer_read32();
                rainbow_counter += 3;
            }

            for (uint8_t i = RGBLED_START_INDEX; i < RGBLED_END_INDEX; i++) {
                uint8_t hue = (rainbow_counter + (RGBLED_NUM - 1 - i) * 8) % 255;
                HSV hsv = { .h = hue, .s = 255, .v = colorful_rgblight_val };
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

            for (uint8_t i = RGBLED_START_INDEX; i < RGBLED_END_INDEX; i++) {
                uint8_t hue = (rainbow_counter + i * 8) % 255;
                HSV hsv = { .h = hue, .s = 255, .v = colorful_rgblight_val };
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
            for (uint8_t i = 0; i < (RGBLED_END_INDEX - RGBLED_START_INDEX); i++) {
                uint8_t distance = (i > center) ? (i - center) : (center - i);

                int16_t temp = (int16_t)rainbow_counter - (int16_t)(distance * 12);
                if (temp < 0) temp += 255;
                uint8_t hue = temp % 255;

                HSV hsv = { .h = hue, .s = 255, .v = colorful_rgblight_val };
                RGB rgb = hsv_to_rgb(hsv);
                rgb_matrix_set_color(i + RGBLED_START_INDEX, rgb.r, rgb.g, rgb.b);
            }
        } break;

        case 3: { 
            static uint16_t rainbow_counter = 0;
            if (!initialized) {
                initialized = true;

                static uint32_t random_seed = 12345;
                random_seed ^= (timer_read32() + random_seed * 31);
                rainbow_counter = random_seed % 255;  
            }

            for (uint8_t i = RGBLED_START_INDEX; i < RGBLED_END_INDEX; i++) {
                uint8_t hue = (rainbow_counter + (RGBLED_NUM - 1 - i) * 8) % 255;
                HSV hsv = { .h = hue, .s = 255, .v = colorful_rgblight_val };
                RGB rgb = hsv_to_rgb(hsv);
                rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
            }
        } break;

        case 4: {
            static const RGB neon_colors[] = {
                {RGBLIGHT_LIMIT_VAL, 0, 0},                        
                {RGBLIGHT_LIMIT_VAL, RGBLIGHT_LIMIT_VAL / 4, 0},   
                {RGBLIGHT_LIMIT_VAL, RGBLIGHT_LIMIT_VAL, 0},       
                {0, RGBLIGHT_LIMIT_VAL, 0},                        
                {0, RGBLIGHT_LIMIT_VAL, RGBLIGHT_LIMIT_VAL},       
                {0, 0, RGBLIGHT_LIMIT_VAL},                        
                {RGBLIGHT_LIMIT_VAL, 0, RGBLIGHT_LIMIT_VAL},       
                {RGBLIGHT_LIMIT_VAL, RGBLIGHT_LIMIT_VAL, RGBLIGHT_LIMIT_VAL} 
            };

            static uint8_t neon_brightness = 0;
            static bool neon_increasing = true;
            static uint32_t last_update = 0;

            uint8_t breathe_rgblight_val = breathe_brightness_levels[brightness_level];
            uint32_t interval = 15;
            uint32_t breathe_step = 3;

            switch (brightness_level)
            {
                case 0: interval = 20; breathe_step = 2; break;
                case 1: interval = 15; breathe_step = 2; break;
                case 2: interval = 10; breathe_step = 3; break;
                case 3: interval = 7; breathe_step = 3; break;
                default:interval = 7; breathe_step = 3; break;
            }

            if (timer_elapsed32(last_update) >= interval) {
                last_update = timer_read32();

                uint8_t max_val = breathe_rgblight_val;   
                if (neon_increasing) {
                    if (neon_brightness < max_val) neon_brightness += breathe_step;
                    else neon_increasing = false;
                } else {
                    if (neon_brightness > breathe_step) neon_brightness -= breathe_step;
                    else {
                        neon_increasing = true;
                        neon_color_index = (neon_color_index + 1) % (ARRAY_SIZE(neon_colors));
                    }
                }
            }

            RGB current_color = neon_colors[neon_color_index];
            for (uint8_t i = RGBLED_START_INDEX; i < RGBLED_END_INDEX; i++) {
                rgb_matrix_set_color(
                    i,
                    (current_color.g * neon_brightness) / 255,
                    (current_color.r * neon_brightness) / 255,
                    (current_color.b * neon_brightness) / 255
                );
            }
        } break;

        case 5: rgb_light_color_set(rgblight_val, 0, 0); break;             
        case 6: rgb_light_color_set(rgblight_val, rgblight_val/4, 0); break;
        case 7: rgb_light_color_set(two_color_rgblight_val, two_color_rgblight_val, 0); break;  
        case 8: rgb_light_color_set(0, rgblight_val, 0); break;           
        case 9: rgb_light_color_set(0, two_color_rgblight_val, two_color_rgblight_val); break;  
        case 10: rgb_light_color_set(0, 0, rgblight_val); break;            
        case 11: rgb_light_color_set(two_color_rgblight_val, 0, two_color_rgblight_val); break; 
        case 12: {
            uint8_t white_rgblight_val = white_brightness_levels[brightness_level];
            rgb_light_color_set(white_rgblight_val, white_rgblight_val, white_rgblight_val); break;
        }
        case 13: rgb_light_color_set(0, 0, 0); break; 

        default:
            rgblight_mode = 0;
            break;
    }
}
