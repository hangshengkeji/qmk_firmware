#include "rgblight.h"
#include "rgb_record.h"
#include QMK_KEYBOARD_H

#define RGBLIGHT_BREATHE_TABLE_SIZE 128

static const uint8_t hs_rgblight_effect_breathe_table[] = {
      0,  12,  18,  24,  28,  32,  36,  40,  43,  46,  49,  52,  55,  58,  60,  63,
     65,  68,  70,  73,  75,  77,  79,  81,  84,  86,  88,  90,  92,  94,  96,  98,
     99, 101, 103, 105, 107, 108, 110, 112, 114, 115, 117, 119, 120, 122, 124, 125,
    127, 129, 130, 132, 133, 135, 136, 138, 139, 141, 142, 144, 145, 147, 148, 150,
    150, 148, 147, 145, 144, 142, 141, 139, 138, 136, 135, 133, 132, 130, 129, 127,
    125, 124, 122, 120, 119, 117, 115, 114, 112, 110, 108, 107, 105, 103, 101,  99,
     98,  96,  94,  92,  90,  88,  86,  84,  81,  79,  77,  75,  73,  70,  68,  65,
     63,  60,  58,  55,  52,  49,  46,  43,  40,  36,  32,  28,  24,  18,  12,   0,
};
extern const uint8_t rgb_hsvs[RGB_HSV_MAX][2];

bool rgb_matrix_indicators_advanced_rgblight(uint8_t led_min, uint8_t led_max);
void hs_rgblight_increase(void);
void hs_rgblight_init(uint8_t speed);
