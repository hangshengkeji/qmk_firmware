#include "hs_rgblight.h"

uint8_t buff[]   = {9, 8, 2, 1, 1, 1, 1, 1, 1, 1, 0};
rgb_led_t ws2812_hs_led[RGBLIGHT_LED_COUNT];

const uint8_t HS_RGBLED_RAINBOW_SWIRL_INTERVALS[] PROGMEM = {30, 20, 10, 6, 4};
const uint8_t HS_RGBLED_RAINBOW_MOOD_INTERVALS[] PROGMEM = {50, 30, 10, 6, 4};
const uint8_t HS_RGBLED_BREATHING_INTERVALS[] PROGMEM = {50, 30, 10, 6, 4};

static const int hs_table_scale = 256 / sizeof(hs_rgblight_effect_breathe_table);
uint8_t breathe_hsv_index = 0;
extern const uint8_t led_map[];
extern rgblight_config_t rgblight_config;

void ws2812_set_color_hs(int index, uint8_t red, uint8_t green, uint8_t blue);

const rgblight_driver_t rgblight_driver = {
    .init          = ws2812_init,
    .set_color     = ws2812_set_color_hs,
    .set_color_all = ws2812_set_color_all,
    .flush         = ws2812_flush,
};

void hs_rgblight_init(uint8_t speed){
    extern uint16_t interval_time;
    if (rgblight_get_mode() == buff[0]){
        interval_time = pgm_read_byte(&HS_RGBLED_RAINBOW_SWIRL_INTERVALS[speed]);
    } else if (rgblight_get_mode() == buff[1]){
        interval_time = pgm_read_byte(&HS_RGBLED_RAINBOW_MOOD_INTERVALS[speed]);
    } else if (rgblight_get_mode() == buff[2]){
        interval_time = pgm_read_byte(&HS_RGBLED_BREATHING_INTERVALS[speed]);
    }
}

void ws2812_set_color_hs(int index, uint8_t red, uint8_t green, uint8_t blue){
    ws2812_hs_led[pgm_read_byte(&led_map[index])].r = red;
    ws2812_hs_led[pgm_read_byte(&led_map[index])].g = green;
    ws2812_hs_led[pgm_read_byte(&led_map[index])].b = blue;
}

void rgblight_call_driver(rgb_led_t *start_led, uint8_t num_leds) {

    memcpy(ws2812_hs_led, start_led, sizeof(rgb_led_t) * num_leds);
}

void rgblight_set(void) {
    rgb_led_t *start_led;
    uint8_t num_leds = rgblight_ranges.clipping_num_leds;

    if (!rgblight_config.enable) {
        
        for (uint8_t i = rgblight_ranges.effect_start_pos; i < rgblight_ranges.effect_end_pos; i++) {
            ws2812_hs_led[i].r = 0;
            ws2812_hs_led[i].g = 0;
            ws2812_hs_led[i].b = 0;
        }
    }
    rgb_led_t led0[RGBLIGHT_LED_COUNT];
    for (uint8_t i = 0; i < RGBLIGHT_LED_COUNT; i++) {
        led0[i] = ws2812_hs_led[pgm_read_byte(&led_map[i])];
    }
    start_led = led0 + rgblight_ranges.clipping_start_pos;
    rgblight_call_driver(start_led, num_leds);
}


static uint8_t hs_breathe_calc(uint8_t pos) {
    // http://sean.voisen.org/blog/2011/10/breathing-led-with-arduino/
    return pgm_read_byte(&hs_rgblight_effect_breathe_table[pos / hs_table_scale]);
}

void rgblight_effect_breathing(animation_status_t *anim) {
    uint8_t val = hs_breathe_calc(anim->pos);
    extern void rgblight_sethsv_noeeprom_old(uint8_t hue, uint8_t sat, uint8_t val);
    rgblight_sethsv_noeeprom_old(rgb_hsvs[breathe_hsv_index][0], rgb_hsvs[breathe_hsv_index][1], val);
    if (anim->pos == 0xFF) {
        breathe_hsv_index = (breathe_hsv_index + 1) % RGB_HSV_MAX;
    }
    anim->pos = (anim->pos + 1);
}

static uint8_t mode = 0;
void hs_rgblight_increase(void) {
    HSV rgb;
    uint8_t moude;
    if (!rgblight_is_enabled()) mode = 10;
    moude = rgblight_get_mode();
    if (moude == 1) {
        rgb = rgblight_get_hsv();
        if (rgb.h == 0 && rgb.s != 0)
            mode = 3;
        else
            mode = 9;
        switch (rgb.h) {
            case 85: {
                mode = 4;
            } break;
            case 170: {
                mode = 5;
            } break;
            case 43: {
                mode = 6;
            } break;
            case 191: {
                mode = 7;
            } break;
            case 128: {
                mode = 8;
            } break;
            default:
                break;
        }
    }

    mode++;
    if (mode == 11) mode = 0;
    if (mode == 10) {
        rgb = rgblight_get_hsv();
        rgblight_sethsv(0, 255, rgb.v);
        rgblight_disable();
    } else {
        rgblight_enable();
        rgblight_mode(buff[mode]);
    }

    rgb = rgblight_get_hsv();
    switch (mode) {
        case 3: {
            rgblight_sethsv(0, 255, rgb.v);
        } break;
        case 4: {
            rgblight_sethsv(85, 255, rgb.v);
        } break;
        case 5: {
            rgblight_sethsv(170, 255, rgb.v);
        } break;
        case 6: {
            rgblight_sethsv(43, 255, rgb.v);
        } break;
        case 7: {
            rgblight_sethsv(191, 255, rgb.v);
        } break;
        case 8: {
            rgblight_sethsv(128, 255, rgb.v);
        } break;
        case 9: {
            rgblight_sethsv(0, 0, rgb.v);
        } break;
        case 0: {
            rgblight_set_speed(255);
        } break;
        default: {
            rgblight_set_speed(200);
        } break;
    }
}

bool rgb_matrix_indicators_advanced_rgblight(uint8_t led_min, uint8_t led_max) {

    for (uint8_t i = 0; i < RGBLIGHT_LED_COUNT / 2; i++) {
        rgb_matrix_set_color(i + 91, ws2812_hs_led[i].r, ws2812_hs_led[i].g, ws2812_hs_led[i].b); // rgb light
        rgb_matrix_set_color(29- i + 91, ws2812_hs_led[i].r, ws2812_hs_led[i].g, ws2812_hs_led[i].b);
    }
    return true;
}