#include "host_driver.h"
#include "bts_lib.h"
#include "host.h"
#include "usb_main.h"
#include "quantum.h"

void wls_transport_enable(bool enable);
uint8_t wireless_keyboard_leds(void);
void wireless_send_extra(report_extra_t *report);
void wireless_send_keyboard(report_keyboard_t *report);
void wireless_send_mouse(report_mouse_t *report);

void wireless_send_nkro(report_keyboard_t *report);
void wireless_send_nkro(report_keyboard_t *report);