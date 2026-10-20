THIS_RK_DIR := keyboards/nm31/rk_common
RK_COMMON_DIR := $(THIS_RK_DIR)

RK_COMMON_ENABLE ?= yes
ifeq ($(strip $(RK_COMMON_ENABLE)), yes)
    OPT_DEFS += -DRK_COMMON_ENABLE
    COMMON_VPATH += $(RK_COMMON_DIR)/
    SRC += $(RK_COMMON_DIR)/record.c
endif
