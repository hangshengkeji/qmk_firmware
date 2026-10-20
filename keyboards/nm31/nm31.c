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

#include "quantum.h"
#include "serial_usart.h"


#ifdef RGB_MATRIX_ENABLE

#define ___ NO_LED

const aw_led g_aw_leds[RGB_MATRIX_LED_COUNT] = {
/* Refer to IS31 manual for these locations
 *   driver
 *   |  R location
 *   |  |          G location
 *   |  |          |          B location
 *   |  |          |          | */
    {0, CS18_SW1,  CS17_SW1,  CS16_SW1  },  // 0  Esc
    {0, CS18_SW2,  CS17_SW2,  CS16_SW2  },  // 1  F1
    {0, CS18_SW3,  CS17_SW3,  CS16_SW3  },  // 2  F2
    {0, CS18_SW4,  CS17_SW4,  CS16_SW4  },  // 3  F3
    {0, CS18_SW5,  CS17_SW5,  CS16_SW5  },  // 4  F4
    {0, CS18_SW6,  CS17_SW6,  CS16_SW6  },  // 5  F5
    {0, CS18_SW7,  CS17_SW7,  CS16_SW7  },  // 6  F6

    {0, CS15_SW1,  CS14_SW1,  CS13_SW1  },  // 7  F7
    {0, CS15_SW2,  CS14_SW2,  CS13_SW2  },  // 8  F8
    {0, CS15_SW3,  CS14_SW3,  CS13_SW3  },  // 9  F9
    {0, CS15_SW4,  CS14_SW4,  CS13_SW4  },  // 10 F10
    {0, CS15_SW5,  CS14_SW5,  CS13_SW5  },  // 11 F11
    {0, CS15_SW6,  CS14_SW6,  CS13_SW6  },  // 12 F12
    {0, CS15_SW7,  CS14_SW7,  CS13_SW7  },  // 13 DEL

    {0, CS12_SW1,  CS11_SW1,  CS10_SW1  },  // 14 MUTE
    {0, CS12_SW2,  CS11_SW2,  CS10_SW2  },  // 15 MUTE
    {0, CS12_SW3,  CS11_SW3,  CS10_SW3  },  // 16 MUTE
    {0, CS12_SW4,  CS11_SW4,  CS10_SW4  },  // 17 MUTE
    {0, CS12_SW5,  CS11_SW5,  CS10_SW5  },  // 18 1
    {0, CS12_SW6,  CS11_SW6,  CS10_SW6  },  // 19 2

    {0, CS1_SW1,  CS2_SW1,  CS3_SW1  },  // 20 3
    {0, CS1_SW2,  CS2_SW2,  CS3_SW2  },  // 21 4
    {0, CS1_SW3,  CS2_SW3,  CS3_SW3  },  // 22 5
    {0, CS1_SW4,  CS2_SW4,  CS3_SW4  },  // 23 6
    {0, CS1_SW5,  CS2_SW5,  CS3_SW5  },  // 24 7
    {0, CS1_SW6,  CS2_SW6,  CS3_SW6  },  // 25 8

    {0, CS4_SW1,  CS5_SW1,  CS6_SW1   },  // 26 9
    {0, CS4_SW2,  CS5_SW2,  CS6_SW2   },  // 27 -_
    {0, CS4_SW3,  CS5_SW3,  CS6_SW3   },  // 28 =+
    {0, CS4_SW4,  CS5_SW4,  CS6_SW4   },  // 29 BSPC
    {0, CS4_SW5,  CS5_SW5,  CS6_SW5   },  // 30 INS
    {0, CS4_SW6,  CS5_SW6,  CS6_SW6   },  // 31 HOME
    {0, CS4_SW7,  CS5_SW7,  CS6_SW7   },  // 32 PGUP

    {0, CS7_SW2,  CS8_SW2,  CS9_SW2   },  // 33 BSPC
    {0, CS7_SW3,  CS8_SW3,  CS9_SW3   },  // 34 INS
    {0, CS7_SW4,  CS8_SW4,  CS9_SW4   },  // 35 HOME
    {0, CS7_SW5,  CS8_SW5,  CS9_SW5   },  // 36 PGUP
    {0, CS12_SW7,  CS11_SW7,  CS10_SW7  },  // 19 2  // 36 PGUP
    {0, CS7_SW6,  CS8_SW6,  CS9_SW6  },  // 36 PGUP

   
/*******************************************************************************************************/
    {1, CS18_SW1,  CS17_SW1,  CS16_SW1  },  // 0  Esc
    {1, CS18_SW2,  CS17_SW2,  CS16_SW2  },  // 1  F1
    {1, CS18_SW3,  CS17_SW3,  CS16_SW3  },  // 2  F2
    {1, CS18_SW4,  CS17_SW4,  CS16_SW4  },  // 3  F3
    {1, CS18_SW5,  CS17_SW5,  CS16_SW5  },  // 4  F4
    {1, CS18_SW6,  CS17_SW6,  CS16_SW6  },  // 5  F5
    {1, CS18_SW7,  CS17_SW7,  CS16_SW7  },  // 6  F6
    {1, CS18_SW8,  CS17_SW8,  CS16_SW8  },  // 5  F5
    {1, CS18_SW9,  CS17_SW9,  CS16_SW9  },  // 6  F6
   
    {1, CS15_SW1,  CS14_SW1,  CS13_SW1  },  // 7  F7
    {1, CS15_SW2,  CS14_SW2,  CS13_SW2  },  // 8  F8
    {1, CS15_SW3,  CS14_SW3,  CS13_SW3  },  // 9  F9
    {1, CS15_SW4,  CS14_SW4,  CS13_SW4  },  // 10 F10
    {1, CS15_SW5,  CS14_SW5,  CS13_SW5  },  // 11 F11
    {1, CS15_SW6,  CS14_SW6,  CS13_SW6  },  // 12 F12
    {1, CS15_SW7,  CS14_SW7,  CS13_SW7  },  // 13 DEL
    {1, CS15_SW8,  CS14_SW8,  CS13_SW8  },  // 12 F12
    {1, CS15_SW9,  CS14_SW9,  CS13_SW9  },  // 12 F12

    {1, CS12_SW1,  CS11_SW1,  CS10_SW1  },  // 14 MUTE
    {1, CS12_SW2,  CS11_SW2,  CS10_SW2  },  // 15 MUTE
    {1, CS12_SW3,  CS11_SW3,  CS10_SW3  },  // 16 MUTE
    {1, CS12_SW4,  CS11_SW4,  CS10_SW4  },  // 17 MUTE
    {1, CS12_SW5,  CS11_SW5,  CS10_SW5  },  // 18 1
    {1, CS12_SW6,  CS11_SW6,  CS10_SW6  },  // 19 2
    {1, CS12_SW7,  CS11_SW7,  CS10_SW7  },  // 17 MUTE
    {1, CS12_SW9,  CS11_SW9,  CS10_SW9  },  // 19 2

    {1, CS1_SW1,  CS2_SW1,  CS3_SW1  },  // 20 3
    {1, CS1_SW2,  CS2_SW2,  CS3_SW2  },  // 21 4
    {1, CS1_SW3,  CS2_SW3,  CS3_SW3  },  // 22 5
    {1, CS1_SW4,  CS2_SW4,  CS3_SW4  },  // 23 6
    {1, CS1_SW5,  CS2_SW5,  CS3_SW5  },  // 24 7
    {1, CS1_SW6,  CS2_SW6,  CS3_SW6  },  // 25 8
    {1, CS1_SW7,  CS2_SW7,  CS3_SW7  },  // 24 7
    {1, CS12_SW8,  CS11_SW8,  CS10_SW8  },  // 18 1
    {1, CS1_SW8,  CS2_SW8,  CS3_SW8  },  // 25 8
    

    {1, CS4_SW1,  CS5_SW1,  CS6_SW1   },  // 26 9
    {1, CS4_SW2,  CS5_SW2,  CS6_SW2   },  // 27 -_
    {1, CS4_SW3,  CS5_SW3,  CS6_SW3   },  // 28 =+
    {1, CS4_SW4,  CS5_SW4,  CS6_SW4   },  // 29 BSPC
    {1, CS4_SW5,  CS5_SW5,  CS6_SW5   },  // 30 INS
    {1, CS4_SW6,  CS5_SW6,  CS6_SW6   },  // 31 HOME
    {1, CS4_SW7,  CS5_SW7,  CS6_SW7   },  // 32 PGUP
    {1, CS4_SW8,  CS5_SW8,  CS6_SW8   },  // 32 PGUP

    {1, CS7_SW1,  CS8_SW1,  CS9_SW1   },  // 33 BSPC
    {1, CS7_SW2,  CS8_SW2,  CS9_SW2   },  // 33 BSPC
    {1, CS7_SW3,  CS8_SW3,  CS9_SW3   },  // 35 HOME
    {1, CS7_SW4,  CS8_SW4,  CS9_SW4   },  // 35 HOME

    {1, CS18_SW9,  CS17_SW9,  CS16_SW9  },  // 6  F6


    {1, CS7_SW5,  CS8_SW5,  CS9_SW5   },  // 36 PGUP
    {1, CS7_SW6,  CS8_SW6,  CS9_SW6   },  // 35 HOME
    {1, CS7_SW7,  CS8_SW7,  CS9_SW7   },  // 35 HOME
    {1, CS7_SW8,  CS8_SW8,  CS9_SW8   },  // 36 PGUP

    // {1, CS15_SW9,  CS14_SW9,  CS13_SW9  }  // 13 DEL
  
};

led_config_t g_led_config = {{
    //left hand
 //   0  15   30   45  60  75  90   
    { 0 , 1,  2,  3,  4,  5,  6,   ___, ___,},   //0
    { 7,  8,  9, 10,  11, 12, 13,  ___, ___,},   //13
    { 14, 15, 16, 17, 18,___, 19,  ___, ___,},   //26
    { 20, 21, 22, 23, 24,___, 25,  ___, ___,},   //38
    { 26, ___,28, 29, 30, 31, 32,  ___, ___,},   //51
    { 33, 34, 35, 36,___, 38, ___, ___, ___,},   //64
    //right hand
    // 105 130 145 160 175 190 205 220 224
    {  39, 40, 41, 42, 43, 44, 45, 46, ___,},
    {  48, 49, 50, 51, 52, 53, 54, 55, 56,},
    {  57, 58, 59, 60, 61, 62, 63, ___,64,},
    {  65, 66, 67, 68, 69, 70, 71, 72, 73,},
    {  74, 75, 76, 77, 78, 79, 80, ___,81,},
    {  82, 83, 84, 85, 87, ___,88, 89, 90,}
}, {
    {   0 , 0},  // 0
    {  15 , 0},  // 1
    {  30 , 0},  // 2
    {  45 , 0},  // 3
    {  60 , 0},  // 4
    {  75 , 0},  // 5
    {  90 , 0},  // 6
    {   0 , 13},  // 7
    {  15 , 13},  // 8
    {  30 , 13},  // 9
    {  45 , 13},  // 10
    {  60 , 13},  // 11
    {  75 , 13},  // 12
    {  90 , 13},  // 13
    {   0 , 26},  // 14
    {  15 , 26},  // 15
    {  30 , 26},  // 16
    {  45 , 26},  // 17
    {  60 , 26},  // 18
    {  90 , 26},  // 19
    {   0 , 38},  // 20
    {  15 , 38},  // 21
    {  30 , 38},  // 22
    {  45 , 38},  // 23
    {  60 , 38},  // 24
    {  90 , 38},  // 25
    {   0 , 51},  // 26
    {  15 , 51},  // 27
    {  30 , 51},  // 28
    {  45 , 51},  // 29
    {  60 , 51},  // 30
    {  75 , 51},  // 31
    {  90 , 51},  // 32
    {   0 , 64},  // 33
    {  15 , 64},  // 34
    {  30 , 64},  // 35
    {  45 , 64},  // 35

    {  0 , 0},  // 36
    {  75 , 64},  // 36
// 105 130 145 160 175 190 205 220 224
    { 105 ,  0},  // 37
    { 130 ,  0},  // 38
    { 145 ,  0},  // 39
    { 160 ,  0},  // 40
    { 175 ,  0},  // 41
    { 190 ,  0},  // 42
    { 205 ,  0},  // 43
    { 215 ,  0},  // 44
    { 224 ,  0},  // 45

    { 105 , 13},  // 46
    { 130 , 13},  // 47
    { 145 , 13},  // 48
    { 160 , 13},  // 49
    { 175 , 13},  // 50
    { 190 , 13},  // 51
    { 205 , 13},  // 51
    { 215 , 13},  // 52
    { 224 , 13},  // 53

    { 105 ,  26},  // 54
    { 130 ,  26},  // 55
    { 145 ,  26},  // 56
    { 160 ,  26},  // 57
    { 175 ,  26},  // 58
    { 190 ,  26},  // 59
    { 205 ,  26},  // 60
    { 224 ,  26},  // 62

    { 105 ,  38},  // 63
    { 130 ,  38},  // 64
    { 145 ,  38},  // 65
    { 160 ,  38},  // 66
    { 175 ,  38},  // 67
    { 190 ,  38},  // 68
    { 205 ,  38},  // 51
    { 215 ,  38},  // 69
    { 224 ,  38},  // 70

    { 105 ,  51},  // 71
    { 130 ,  51},  // 72
    { 145 ,  51},  // 73
    { 160 ,  51},  // 74
    { 175 ,  51},  // 75
    { 190 ,  51},  // 75
    { 205 ,  51},  // 76
    { 224 ,  51},  // 77

    { 105 ,  64},  // 78
    { 130 ,  64},  // 78
    { 145 ,  64},  // 79
    { 160 ,  64},  // 80
    { 160 ,  64},  // 84
    { 175 ,  64},  // 81
    { 205 ,  64},  // 82
    { 215 ,  64},  // 83
    { 224 ,  64},  // 84

    

}, {
    4,4,4,4,4,4,4,4,4,4,
    4,4,4,4,4,4,4,4,4,4,
    4,4,4,4,4,4,4,4,4,4,
    4,4,4,4,4,4,4,4,4,4,
    4,4,4,4,4,4,4,4,4,4,
    4,4,4,4,4,4,4,4,4,4,
    4,4,4,4,4,4,4,4,4,4,
    4,4,4,4,4,4,4,4,4,4,
    4,4,4,4,4,4,4,4,4,4,4
} };

#endif


/*usart pin init*/
void usart_init(void) {
    //palSetLineMode(SERIAL_USART_TX_PIN, PAL_MODE_ALTERNATE(SERIAL_USART_TX_PAL_MODE) | PAL_OUTPUT_TYPE_OPENDRAIN);
    palSetLineMode(SERIAL_USART_TX_PIN, PAL_MODE_ALTERNATE(SERIAL_USART_TX_PAL_MODE)  | PAL_OUTPUT_SPEED_HIGHEST);
    palSetLineMode(SERIAL_USART_RX_PIN, PAL_MODE_ALTERNATE(SERIAL_USART_RX_PAL_MODE)  | PAL_OUTPUT_SPEED_HIGHEST);

}

