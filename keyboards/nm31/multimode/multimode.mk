MULTIMODE_MK_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
MULTIMODE_DIR := $(MULTIMODE_MK_DIR)

RGB_MATRIX_BLINK_ENABLE ?= yes
ifeq ($(strip $(RGB_MATRIX_BLINK_ENABLE)), yes)
    OPT_DEFS += -DRGB_MATRIX_BLINK_ENABLE
    COMMON_VPATH += $(MULTIMODE_DIR)
    SRC += $(MULTIMODE_DIR)/rgb_matrix_blink.c
endif

MULTIMODE_ENABLE ?= yes
ifeq ($(strip $(MULTIMODE_ENABLE)), yes)
    IMMOBILE_ENABLE ?= yes
    OPT_DEFS += -DMULTIMODE_ENABLE
    OPT_DEFS += -DNO_USB_STARTUP_CHECK
    QUANTUM_LIB_SRC += uart.c
    COMMON_VPATH += $(MULTIMODE_DIR)/
    SRC += $(MULTIMODE_DIR)/multimode.c
    SRC += $(MULTIMODE_DIR)/retarget_suspend.c
    SRC += $(MULTIMODE_DIR)/lp_sleep.c
    LDFLAGS += -L $(MULTIMODE_DIR)/ -l_bts
endif

IMMOBILE_ENABLE ?= no
ifeq ($(strip $(IMMOBILE_ENABLE)), yes)
    OPT_DEFS += -DIMMOBILE_ENABLE
    COMMON_VPATH += $(MULTIMODE_DIR)/immobile
    SRC += $(MULTIMODE_DIR)/immobile/immobile.c
endif

RGB_RECORD_ENABLE ?= no
ifeq ($(strip $(RGB_RECORD_ENABLE)), yes)
    RGB_MATRIX_CUSTOM_KB = yes
    OPT_DEFS += -DRGB_RECORD_ENABLE
    COMMON_VPATH += $(MULTIMODE_DIR)/rgb_record
    SRC += $(MULTIMODE_DIR)/rgb_record/rgb_record.c
endif

GPIO_UART_ENABLE ?= no
ifeq ($(strip $(GPIO_UART_ENABLE)), yes)
    OPT_DEFS += -DGPIO_UART_ENABLE
    SRC += $(MULTIMODE_DIR)/iprint.c
endif