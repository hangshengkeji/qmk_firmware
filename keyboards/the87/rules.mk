RGBCTRL_ENABLE = no
RGBLIGHT_CUSTOM_ENABLE = no
RGBM_BLINK_ENABLE = no
WLSIDR_ENABLE = no
RGB_MATRIX_CUSTOM_USER = yes
RGBLIGHT_DRIVER = custom
include keyboards/the87/wireless/wireless.mk

SRC += rgb_matrix_blink.c wlsidr.c hs_rgblight.c
SRC += lpkc.c modip.c
SRC += rgb_record.c
# SRC += i2c_master.c

# 以下内容调试使用, 不公开
# CONSOLE_ENABLE = yes
# KEYBOARD_SHARED_EP = yes

# OPT ?= 0
# DEBUG_ENABLE = yes
