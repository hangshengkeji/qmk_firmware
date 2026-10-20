# MCU name
MCU = WB32FQ95

# Bootloader selection
BOOTLOADER = wb32-dfu

# 用来指定当前软件版本, 必须定义
SOFTWARE_VERSION ?= 4.3

# Build Options
#   change yes to no to disable
#
KEYBOARD_SHARED_EP = no
BOOTMAGIC_ENABLE = no         # Enable Bootmagic Lite
MOUSEKEY_ENABLE = yes           # Mouse keys
EXTRAKEY_ENABLE = yes          # Audio control and System control
NKRO_ENABLE = yes              # Enable N-Key Rollover
CONSOLE_ENABLE = no            # Console for debug
COMMAND_ENABLE = no            # Commands for debug and configuration
BACKLIGHT_ENABLE = no          # Enable keyboard backlight functionality
AUDIO_ENABLE = no              # Audio output
HAPTIC_ENABLE = no             # Haptic
RGB_MATRIX_ENABLE = yes        # RGB matrix
RGBLIGHT_ENABLE = no           # Enable keyboard RGB underglow
RGB_RECORD_ENABLE = no        # Light effect recording
ENCODER_ENABLE = yes           # Encoder
LED_BLINK_ENABLE = no          # Led blink
RGB_MATRIX_BLINK_ENABLE = yes  # RGB matrix blink
MULTIMODE_ENABLE = yes         # multimode
RK_COMMON_ENABLE = yes         # rk keyboard

# 需要输出debug信息时打开
# GPIO_UART_ENABLE = yes

# EEPROM驱动方式，固定使用spi flash磨损均衡模拟EEPROM
EEPROM_DRIVER = wear_leveling
WEAR_LEVELING_DRIVER = spi_flash

# 按键灯驱动方式
# RGB_MATRIX_DRIVER = WS2812
RGB_MATRIX_DRIVER = AW20216
# 使用WS2812驱动时，必须使用spi模拟的方式驱动
# WS2812_DRIVER = spi
# 自定义侧灯驱动, 不需要动
# RGBLIGHT_DRIVER = custom

SPLIT_KEYBOARD = yes           # split keyboard
SERIAL_DRIVER = usart
MULTIMODE_DRIVER = uart3

# 单独IO控制旋钮按下时所使用，需要在matrix.c中修改独立IO的名称, 在config.h中MATRIX_ROW_PINS最后增加该IO名称定义
CUSTOM_MATRIX = lite
QUANTUM_SRC += matrix.c
SRC += i2c_master.c  # 根据实际路径调整
SRC += wireless.c
# 使用三模功能
RULES_MK_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
include $(RULES_MK_DIR)/multimode/multimode.mk

ifeq ($(strip $(RK_COMMON_ENABLE)), yes)
    include $(RULES_MK_DIR)/rk_common/rk_common.mk
endif

# RK键盘一些通用逻辑
# ifeq ($(strip $(RK_COMMON_ENABLE)), yes)
#     include $(RULES_MK_DIR)/../rk_common/rk_common.mk
# endif

# # 根据实际芯片型号选择
# MCU_LDSCRIPT = WB32F3C05xB
# USE_CHIBIOS_CONTRIB = yes
# WB32_BOOTLOADER_ADDRESS = 0x8000000
# OPT_DEFS += -DWB32IAP_ENABLE
# OPT_DEFS += -DWB32IAP_ENABLE -DKB_VERSION=$(SOFTWARE_VERSION)
