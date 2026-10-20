#include "wireless.h"

extern bts_info_t bts_info;
extern host_driver_t chibios_driver;

host_driver_t wireless_driver = {
    .keyboard_leds = wireless_keyboard_leds,
    .send_keyboard = wireless_send_keyboard,
    .send_mouse    = wireless_send_mouse,
    .send_extra    = wireless_send_extra,
};

#ifdef GPIO_UART_ENABLE
#include "iprint.h"
#include <stdarg.h>
#define DEBUG(fmt, ...) iprintf(fmt, ##__VA_ARGS__)
#else
#define DEBUG(fmt, ...)
#endif

static void md_calc_check_sum(uint8_t *data, uint32_t length) {
    uint8_t sum = 0;

    for (uint32_t i = 0; i < length; i++) {
        sum += data[i];
    }

    data[length] = sum;
}

uint8_t wireless_keyboard_leds(void) {

    if (bts_info.bt_info.paired) {
        return bts_info.bt_info.indicators;
    }

    return 0;
}

void wireless_send_mouse(report_mouse_t *report) {
    
    if(bts_info.bt_info.pairing){
        return;
    }

    typedef struct {
        uint8_t buttons;
        int8_t x;
        int8_t y;
        int8_t z;
        int8_t h;
    } __attribute__((packed)) wls_report_mouse_t;

    wls_report_mouse_t wls_report_mouse = {0};


    if (report != NULL) {
        wls_report_mouse.buttons = report->buttons;
        wls_report_mouse.x       = report->x;
        wls_report_mouse.y       = report->y;
        wls_report_mouse.z       = report->h;
        wls_report_mouse.h       = report->v;
    }

    uint8_t sdata[5 + 2] = {0x00};

    sdata[0] = 0xA8;
    memcpy(&sdata[1], (uint8_t *)&wls_report_mouse, 5);
    md_calc_check_sum(sdata, sizeof(sdata) - 1);
    bts_info.uart_transmit((uint8_t *)&sdata, sizeof(sdata));
}

void wireless_send_extra(report_extra_t *report) {
    if(bts_info.bt_info.pairing){
        return;
    }

    uint16_t usage = 0;

    if (report != NULL) {
        usage = report->usage;

        switch (usage) {
            case 0x81:
            case 0x82:
            case 0x83: { // system usage
                uint8_t sdata[1 + 2] = {0x00};
                sdata[0] = 0xA4;
                sdata[1] = 0x01 << (usage - 0x81);
                md_calc_check_sum(sdata, sizeof(sdata) - 1);
                bts_info.uart_transmit((uint8_t *)&sdata, sizeof(sdata));
                // md_send_system((uint8_t *)&usage);
            } break;
            default: {
                uint8_t sdata[2 + 2] = {0x00};
                sdata[0] = 0xA3;
                // sdata[1] = usage;
                // sdata[2] = &usage;
                memcpy(&sdata[1], &usage, sizeof(sdata) - 2);
                md_calc_check_sum(sdata, sizeof(sdata) - 1);
                bts_info.uart_transmit((uint8_t *)&sdata, sizeof(sdata));
                // md_send_consumer((uint8_t *)&usage);
            } break;
        }
    }
}

void wireless_send_keyboard(report_keyboard_t *report) {
    if(bts_info.bt_info.pairing){
        return;
    }
    uint8_t wls_report_kb[10] = {0};

    if (!keymap_config.nkro){
    
        if (report != NULL) {
            memcpy(&wls_report_kb[1], (uint8_t *)report, 8);
        }
        wls_report_kb[0] = 0xA1;
        DEBUG("wls_report_kb_nkro: %d %d %d\r\n",wls_report_kb[1],REPORT_ID_KEYBOARD,REPORT_ID_NKRO);
        md_calc_check_sum(wls_report_kb, sizeof(wls_report_kb) - 1);
        bts_info.uart_transmit(wls_report_kb,sizeof(wls_report_kb));
    } else {
   
        wireless_send_nkro(report);
    }
}

void wireless_send_nkro(report_keyboard_t *report) {
    static report_keyboard_t temp_report_keyboard = {0};
    uint8_t wls_report_nkro[14]  = {0};
    uint8_t key_count              = 0;

    if(bts_info.bt_info.pairing){
        return;
    }
   
    DEBUG("wireless_send_nkro\r\n");
    if (report != NULL) {
        report_keyboard_t temp_report_nkro = *report;
        

        temp_report_keyboard.mods = temp_report_nkro.nkro.mods;
        for (uint8_t i = 0; i < 14; i++) {
            key_count += __builtin_popcount(temp_report_nkro.nkro.bits[i]);
            wls_report_nkro[i] = temp_report_nkro.nkro.bits[i];
            DEBUG("key_count1 = %d\r\n",key_count);
        }

        // find key up and del it.

        // for (uint8_t i = 0; i < KEYBOARD_REPORT_KEYS && temp_report_keyboard.keys[i]; i++) {
        //     uint8_t usageid = 0x00;
        //     uint8_t n;

        //     for (uint8_t c = 0; c < key_count; c++) {
        //         for (n = 0; n < 14 && !temp_report_nkro.nkro.bits[n]; n++) {}
        //         usageid = (n << 3) | biton(temp_report_nkro.nkro.bits[n]);
        //         // wls_report_nkro[usageid / 8] |= 0x01 << (usageid % 8);
        //         del_key_bit(&temp_report_nkro, usageid);
        //         if (usageid == temp_report_keyboard.keys[i]) {
        //             break;
        //         }
                
        //     }
        //     // wls_report_nkro[i] = report->nkro.bits[i];
        //     if (usageid != temp_report_keyboard.keys[i]) {
        //         temp_report_keyboard.keys[i] = 0x00;
        //     }
        // }

        // /*
        //  * Use NKRO for sending when more than 6 keys are pressed
        //  * to solve the issue of the lack of a protocol flag in wireless mode.
        //  */

        // temp_report_nkro = *report;

        // for (uint8_t i = 0; i < key_count; i++) {
        //     uint8_t usageid;
        //     uint8_t idx, n = 0;

        //     for (n = 0; n < 14 && !temp_report_nkro.nkro.bits[n]; n++) {}
        //     usageid = (n << 3) | biton(temp_report_nkro.nkro.bits[n]);
        //     del_key_bit(&temp_report_nkro, usageid);

        //     for (idx = 0; idx < KEYBOARD_REPORT_KEYS; idx++) {
        //         if (temp_report_keyboard.keys[idx] == usageid) {
        //             break;
        //         }
        //         if (temp_report_keyboard.keys[idx] == 0x00) {
        //             temp_report_keyboard.keys[idx] = usageid;
        //             break;
        //         }
        //     }

        //     if (idx == KEYBOARD_REPORT_KEYS && (usageid < (14 * 8))) {
        //         wls_report_nkro[usageid / 8] |= 0x01 << (usageid % 8);
        //     }
        // }
    } else {
        memset(&temp_report_keyboard, 0, sizeof(temp_report_keyboard));
    }


    uint8_t wls_report_kb[10] = {0};
    wls_report_kb[0] = 0xA1;
    memcpy(&wls_report_kb[1], (uint8_t *)&temp_report_keyboard, 8);
    md_calc_check_sum(wls_report_kb, sizeof(wls_report_kb) - 1);
    bts_info.uart_transmit(wls_report_kb,sizeof(wls_report_kb));

    // if (key_count > 6){
        uint8_t sdata[14 + 2] = {0x00};
        sdata[0] = 0xA2;

        memcpy(&sdata[1], (uint8_t *)&wls_report_nkro, 14);
        md_calc_check_sum(sdata, sizeof(sdata) - 1);
        bts_info.uart_transmit((uint8_t *)&sdata, sizeof(sdata));
    // }
}

void wls_transport_enable(bool enable) {

    if (enable) {
        if (host_get_driver() != &wireless_driver) {
            if (USB_DRIVER.state == USB_ACTIVE) {
                chibios_driver.send_keyboard(NULL);
                // chibios_driver.send_nkro(NULL);
            }
            
            host_set_driver(&wireless_driver);
            extern uint8_t keyboard_protocol;
            keyboard_protocol = true; // default with true
        }
    } else {
        if (bts_info.bt_info.paired) {
            wireless_driver.send_keyboard(NULL);
            // wireless_driver.send_nkro(NULL);
        }
        if (host_get_driver() != &chibios_driver) {
            extern bool last_suspend_state;

            /* This flag is not set to 1 with probability after usb restart */
            last_suspend_state = true;

            host_set_driver(&chibios_driver);
        }
        
    }
}
