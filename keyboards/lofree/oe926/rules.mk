RGBCTRL_ENABLE = no
RGBLIGHT_CUSTOM_ENABLE = no
RGBM_BLINK_ENABLE = no
WLSIDR_ENABLE = no

include keyboards/wireless/wireless.mk

SRC += $(QUANTUM_DIR)/backlight/backlight_driver_common.c
SRC += backlight_timer.c
SRC += touch_uart.c touch.c
SRC += led_blink.c wlsidr.c
SRC += lpkc.c modip.c

