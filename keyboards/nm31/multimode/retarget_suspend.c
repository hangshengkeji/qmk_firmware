// Copyright 2024 JoyLee (@itarze)
// SPDX-License-Identifier: GPL-2.0-or-later

#ifdef NO_USB_STARTUP_CHECK

#    include <ch.h>
#    include <hal.h>

#    include "usb_main.h"

/* TMK includes */
#    include "action_util.h"
#    include "mousekey.h"
#    include "print.h"

#    include "suspend.h"
#    include "wait.h"

#    include "quantum.h"
#    include "multimode.h"

#    ifndef USB_SUSPEND_DELAY
#        define USB_SUSPEND_DELAY 7000
#    endif

#    ifndef IDEBUG_INFO
#        define IDEBUG_INFO(fmt, ...)
#    endif

#    define USB_GETSTATUS_REMOTE_WAKEUP_ENABLED (2U)

uint32_t usb_suspend_timer = 0x00;
bool usb_wakeup_host_en    = false;

void rs_updater_usb_suspend_timer(void);
void rs_usb_wakeup_set_host_en(bool state);

void housekeeping_task_kb(void) {

    if (mm_eeconfig.devs == DEVS_USB) {

        if ((USB_DRIVER.state != USB_SUSPENDED) && (USB_DRIVER.state != USB_READY)) {
            usb_suspend_timer  = timer_read32();
            usb_wakeup_host_en = false;
            return;
        }

        // IDEBUG_INFO("USB SUSPEND");

#    ifdef MM_BAT_CABLE_PIN
        if (!readPin(MM_BAT_CABLE_PIN)) {
#        ifndef MM_USB_AUTO_SLEEP_DISABLE
            // USB模式和无线模式一致超时才能休眠
            mm_set_to_sleep();
#        endif
            rs_updater_usb_suspend_timer();
            IDEBUG_INFO("[USB] USB SUSPEND NOT CABLE");
            return;
        }
#    endif

        if (!usb_suspend_timer) {
            rs_updater_usb_suspend_timer();
            return;
        }
        if (!is_keyboard_master()) return;
        // IDEBUG_INFO("USB SUSPEND DURATION: %d", timer_elapsed32(usb_suspend_timer));

#    if USB_SUSPEND_DELAY
        if (usb_suspend_timer && timer_elapsed32(usb_suspend_timer) >= USB_SUSPEND_DELAY) {
            rs_updater_usb_suspend_timer();
            rs_usb_wakeup_set_host_en(true);
            mm_set_to_sleep();
        }
#    endif
    } else {
        rs_updater_usb_suspend_timer();
    }
}

__WEAK bool process_record_usb_suspend(uint16_t keycode, keyrecord_t *record) {

#ifndef USB_WAKEUP_FOLLOW_HOST_ENABLE
    if (mm_eeconfig.devs == DEVS_USB) {
        switch (mm_get_sleep_status()) {
            case mm_state_presleep:
            case mm_state_stop: {
                mm_set_sleep_state(mm_state_wakeup);
                mm_set_sleep_level(mm_sleep_level_timeout);
            } break;
            default:
                break;
        }
    }
#endif

    if ((mm_eeconfig.devs == DEVS_USB) && (USB_DRIVER.state == USB_SUSPENDED)) {
        IDEBUG_INFO("[USB] process_record_usb_suspend: USB_DRIVER.status = %d, usb_wakeup_host_en = %s",
                    USB_DRIVER.status,
                    usb_wakeup_host_en ? "true" : "false");

        if (!(USB_DRIVER.status & USB_GETSTATUS_REMOTE_WAKEUP_ENABLED) || (usb_wakeup_host_en != true)) {

            // 允许非系统键值有效
            if (keycode > 0xFF) {
                return true;
            }

            IDEBUG_INFO("[USB] USB_DRIVER.status = 0x%X", USB_DRIVER.status);
            IDEBUG_INFO("[USB] usb_wakeup_host_en = %d", usb_wakeup_host_en);

            return false;
        }

        switch (mm_get_sleep_status()) {
            case mm_state_normal:
            case mm_state_presleep:
            case mm_state_stop:
            case mm_state_wakeup: {
                rs_usb_wakeup_set_host_en(false);
                rs_updater_usb_suspend_timer();
                usbWakeupHost(&USB_DRIVER);
                restart_usb_driver(&USB_DRIVER);
                IDEBUG_INFO("[USB] USB remote wakeup");
            } break;
            default:
                break;
        }
    }

    return true;
}

void rs_updater_usb_suspend_timer(void) {
    usb_suspend_timer = timer_read32();
    if (mm_eeconfig.devs == DEVS_USB) {
        IDEBUG_INFO("[USB] update usb_suspend_timer");
    }
}

void rs_usb_wakeup_set_host_en(bool state) {
    usb_wakeup_host_en = state;
    IDEBUG_INFO("[USB] rs_usb_wakeup_set_host_en : %s", state ? "true" : "false");
}
#endif
