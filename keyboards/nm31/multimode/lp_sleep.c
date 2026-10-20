// Copyright 2024 JoyLee (@itarze)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "quantum.h"
#include "multimode.h"
#ifdef LP_UART_WAKEUP
#    include "uart.h"
#endif

#ifdef SPLIT_KEYBOARD
#    include "serial_usart.h"
#endif

#ifndef WB32_EXTI_REQUIRED
#    error "Must be define WB32_EXTI_REQUIRED"
#endif

#ifndef WB32_USB_WAKEUP_DISABLE
#    define WB32_USB_WAKEUP_DISABLE
// #    error "Must be define WB32_USB_WAKEUP_DISABLE"
#endif

// #define WAKEUP_RESET

#define LP_MATRIX_ROWS                 MATRIX_ROWS
#define LP_MATRIX_COLS                 MATRIX_COLS

#define MATRIX_SPLIT_IS_MASTER()       true
#define MATRIX_SPLIT_IS_MASTER_RIGHT() true

#ifdef SPLIT_KEYBOARD
#    undef LP_MATRIX_ROWS
#    define LP_MATRIX_ROWS (MATRIX_ROWS / 2)
#    undef MATRIX_SPLIT_IS_MASTER
#    define MATRIX_SPLIT_IS_MASTER() is_keyboard_master()
#endif

#if defined(MATRIX_ROW_PINS_RIGHT) && defined(MATRIX_COL_PINS_RIGHT)
#    undef MATRIX_SPLIT_IS_MASTER_RIGHT
#    define MATRIX_SPLIT_IS_MASTER_RIGHT() is_keyboard_master()
#endif

static ioline_t row_pins[LP_MATRIX_ROWS] = MATRIX_ROW_PINS;
static ioline_t col_pins[LP_MATRIX_COLS] = MATRIX_COL_PINS;
#if defined(MATRIX_ROW_PINS_RIGHT) && defined(MATRIX_COL_PINS_RIGHT)
static ioline_t row_pins_r[LP_MATRIX_ROWS] = MATRIX_ROW_PINS_RIGHT;
static ioline_t col_pins_r[LP_MATRIX_COLS] = MATRIX_COL_PINS_RIGHT;
#endif

static const uint32_t pre_lp_code[] = {553863175u, 554459777u, 1208378049u, 4026624001u, 688390415u, 554227969u, 3204472833u, 1198571264u, 1073807360u, 1073808388u};
#define PRE_LP() ((void (*)(void))((unsigned int)(pre_lp_code) | 0x01))()

static const uint32_t post_lp_code[] = {553863177u, 554459777u, 1208509121u, 51443856u, 4026550535u, 1745485839u, 3489677954u, 536895496u, 673389632u, 1198578684u, 1073807360u, 536866816u, 1073808388u};
#define POST_LP() ((void (*)(void))((unsigned int)(post_lp_code) | 0x01))()

void lp_recovery_hook(void);
bool lp_exti_init_user(void);
void lp_exti_init_kb(void);
static void stop_mode_entry(void);
static void exti_init(mm_sleep_level_t sleep_level);
#ifdef WAKEUP_RESET
void mcu_reset(void);
#endif

extern void matrix_init_pins(void);

void lp_system_sleep(mm_sleep_level_t sleep_level) {
    extern void __early_init(void);

#ifdef WAKEUP_RESET
    PWR->GPREG0 = 0x5AA5;
#endif

    chSysLock();
    exti_init(sleep_level);
    chSysUnlock();

    chSysDisable();

    stop_mode_entry();

    RCC->APB1PRE |= RCC_APB1PRE_SRCEN;
    RCC->APB1ENR |= RCC_APB1ENR_EXTIEN;

#ifdef WAKEUP_RESET
    mcu_reset();
#else
    __early_init();

#    if defined(WB32_EXTI_REQUIRED) && !defined(WB32_USB_WAKEUP_DISABLE)
    /* Unlocks write to ANCTL registers */
    PWR->ANAKEY1 = 0x03;
    PWR->ANAKEY2 = 0x0C;
    ANCTL->USBPCR &= ~(ANCTL_USBPCR_DMSTEN | ANCTL_USBPCR_DPSTEN);
    /* Locks write to ANCTL registers */
    PWR->ANAKEY1 = 0x00;
    PWR->ANAKEY2 = 0x00;

    /* Enable SFM clock */
    RCC->AHBENR1 |= RCC_AHBENR1_CRCSFMEN;

    /* Enable USB peripheral clock */
    RCC->AHBENR1 |= RCC_AHBENR1_USBEN;

    /* Configure USB FIFO clock source */
    RCC->USBFIFOCLKSRC = RCC_USBFIFOCLKSRC_USBCLK;

    /* Enable USB FIFO clock */
    RCC->USBFIFOCLKENR = RCC_USBFIFOCLKENR_CLKEN;

    /* Configure and enable USBCLK */
#        if (WB32_USBPRE == WB32_USBPRE_DIV1P5)
    RCC->USBCLKENR = RCC_USBCLKENR_CLKEN;
    RCC->USBPRE    = RCC_USBPRE_SRCEN;
    RCC->USBPRE |= RCC_USBPRE_RATIO_1_5;
    RCC->USBPRE |= RCC_USBPRE_DIVEN;
#        elif (WB32_USBPRE == WB32_USBPRE_DIV1)
    RCC->USBCLKENR = RCC_USBCLKENR_CLKEN;
    RCC->USBPRE    = RCC_USBPRE_SRCEN;
    RCC->USBPRE |= 0x00;
#        elif (WB32_USBPRE == WB32_USBPRE_DIV2)
    RCC->USBCLKENR = RCC_USBCLKENR_CLKEN;
    RCC->USBPRE    = RCC_USBPRE_SRCEN;
    RCC->USBPRE |= RCC_USBPRE_RATIO_2;
    RCC->USBPRE |= RCC_USBPRE_DIVEN;
#        elif (WB32_USBPRE == WB32_USBPRE_DIV3)
    RCC->USBCLKENR = RCC_USBCLKENR_CLKEN;
    RCC->USBPRE    = RCC_USBPRE_SRCEN;
    RCC->USBPRE |= RCC_USBPRE_RATIO_3;
    RCC->USBPRE |= RCC_USBPRE_DIVEN;
#        else
#            error "invalid WB32_USBPRE value specified"
#        endif
#    endif

    matrix_init_pins();

#    ifdef LP_UART_WAKEUP
    palSetLineMode(SD1_RX_PIN, PAL_MODE_ALTERNATE(SD1_RX_PAL_MODE) | PAL_OUTPUT_TYPE_PUSHPULL | PAL_OUTPUT_SPEED_HIGHEST);
#    endif

#    ifdef SPLIT_KEYBOARD
    palSetLineMode(SERIAL_USART_TX_PIN, PAL_MODE_ALTERNATE(SERIAL_USART_TX_PAL_MODE) | PAL_OUTPUT_TYPE_PUSHPULL | PAL_OUTPUT_SPEED_HIGHEST);
    palSetLineMode(SERIAL_USART_RX_PIN, PAL_MODE_ALTERNATE(SERIAL_USART_RX_PAL_MODE) | PAL_OUTPUT_TYPE_PUSHPULL | PAL_OUTPUT_SPEED_HIGHEST);
#    endif

#    if WB32_SERIAL_USE_UART1
    rccEnableUART1();
#    endif
#    if WB32_SERIAL_USE_UART2
    rccEnableUART2();
#    endif
#    if WB32_SERIAL_USE_UART3
    rccEnableUART3();
#    endif
#    if WB32_SPI_USE_QSPI
    rccEnableQSPI();
#    endif
#    if WB32_SPI_USE_SPIM2
    rccEnableSPIM2();
#    endif
#    if WB32_GPT_USE_TIM1 || WB32_ICU_USE_TIM1 || WB32_PWM_USE_TIM1
    rccEnableTIM1();
#    endif
#    if WB32_ST_USE_TIM2 || WB32_GPT_USE_TIM2 || WB32_ICU_USE_TIM2 || WB32_PWM_USE_TIM2
    rccEnableTIM2();
#    endif
#    if WB32_ST_USE_TIM3 || WB32_GPT_USE_TIM3 || WB32_ICU_USE_TIM3 || WB32_PWM_USE_TIM3
    rccEnableTIM3();
#    endif
#    if WB32_ST_USE_TIM4 || WB32_GPT_USE_TIM4 || WB32_ICU_USE_TIM4 || WB32_PWM_USE_TIM4
    rccEnableTIM4();
#    endif
    lp_recovery_hook();
#endif

    chSysEnable();

    rccResetEXTI();
    rccDisableEXTI();
}

__WEAK void lp_recovery_hook(void) {
    /*
     * User implementation
     * related configuration and clock recovery
     */
}

#ifdef WAKEUP_RESET
void mcu_reset(void) {
    NVIC_SystemReset();
    while (1)
        ;
}
#endif

void _pal_lld_enablepadevent(ioportid_t port,
                             iopadid_t pad,
                             ioeventmode_t mode) {
    uint32_t padmask, cridx, croff, crmask, portidx;

    /* Enable EXTI clock.*/
    rccEnableEXTI();

    /* Mask of the pad.*/
    padmask = 1U << (uint32_t)pad;

    /* Multiple channel setting of the same channel not allowed, first
       disable it. This is done because on WB32 the same channel cannot
       be mapped on multiple ports.*/
    osalDbgAssert(((EXTI->RTSR & padmask) == 0U) &&
                      ((EXTI->FTSR & padmask) == 0U),
                  "channel already in use");

    /* Index and mask of the SYSCFG CR register to be used.*/
    cridx  = (uint32_t)pad >> 2U;
    croff  = ((uint32_t)pad & 3U) * 4U;
    crmask = ~(0xFU << croff);

    /* Port index is obtained assuming that GPIO ports are placed at  regular
       0x400 intervals in memory space. So far this is true for all devices.*/
    portidx = (((uint32_t)port - (uint32_t)GPIOA) >> 10U) & 0xFU;

    /* Port selection in SYSCFG.*/
    AFIO->EXTICR[cridx] = (AFIO->EXTICR[cridx] & crmask) | (portidx << croff);

    /* Programming edge registers.*/
    if (mode & PAL_EVENT_MODE_RISING_EDGE) {
        EXTI->RTSR |= padmask;
    } else {
        EXTI->RTSR &= ~padmask;
    }

    if (mode & PAL_EVENT_MODE_FALLING_EDGE) {
        EXTI->FTSR |= padmask;
    } else {
        EXTI->FTSR &= ~padmask;
    }

    EXTI->PR = padmask;

    /* Programming interrupt and event registers.*/
    EXTI->IMR |= padmask;
    EXTI->EMR &= ~padmask;
}

void wb32_exti_nvic_init(iopadid_t pad) {

    switch (pad) {
        case 0:
            nvicEnableVector(EXTI0_IRQn, WB32_IRQ_EXTI0_PRIORITY);
            break;
        case 1:
            nvicEnableVector(EXTI1_IRQn, WB32_IRQ_EXTI1_PRIORITY);
            break;
        case 2:
            nvicEnableVector(EXTI2_IRQn, WB32_IRQ_EXTI2_PRIORITY);
            break;
        case 3:
            nvicEnableVector(EXTI3_IRQn, WB32_IRQ_EXTI3_PRIORITY);
            break;
        case 4:
            nvicEnableVector(EXTI4_IRQn, WB32_IRQ_EXTI4_PRIORITY);
            break;
        case 5 ... 9:
            nvicEnableVector(EXTI9_5_IRQn, WB32_IRQ_EXTI5_9_PRIORITY);
            break;
        case 10 ... 15:
            nvicEnableVector(EXTI15_10_IRQn, WB32_IRQ_EXTI10_15_PRIORITY);
            break;
    }
}

__WEAK bool lp_exti_init_user(void) {
    return true;
}

__WEAK void lp_exti_init_kb(void) {

    if (lp_exti_init_user() != true) {
        return;
    }

#if defined(WB32_EXTI_REQUIRED) && !defined(WB32_USB_WAKEUP_DISABLE)
    /* Unlocks write to ANCTL registers */
    PWR->ANAKEY1  = 0x03;
    PWR->ANAKEY2  = 0x0C;
    ANCTL->USBPCR = ANCTL_USBPCR_DMSTEN | ANCTL_USBPCR_DPSTEN;
    /* Locks write to ANCTL registers */
    PWR->ANAKEY1 = 0x00;
    PWR->ANAKEY2 = 0x00;

    RCC->AHBENR1 |= RCC_AHBENR1_CRCSFMEN;

    SFM->USBPSDCSR = 0xF00;
    SFM->USBPSDCSR &= 0xF00; // Clear Flags

    extiEnableLine(18, PAL_EVENT_MODE_RISING_EDGE);
    nvicEnableVector(USBP_WKUP_IRQn, 6);
#endif

#ifdef MM_BAT_CABLE_PIN
#    ifndef LP_BAT_CABLE_PAL_EVENT_MODE
#        define LP_BAT_CABLE_PAL_EVENT_MODE PAL_EVENT_MODE_RISING_EDGE
#    endif
    _pal_lld_enablepadevent(PAL_PORT(MM_BAT_CABLE_PIN), PAL_PAD(MM_BAT_CABLE_PIN), LP_BAT_CABLE_PAL_EVENT_MODE);
    wb32_exti_nvic_init(PAL_PAD(MM_BAT_CABLE_PIN));
#endif

#ifdef ENCODER_ENABLE
#    include "encoder.h"
#    ifndef ENCODERS_PAD_A_INT_DISABLE
    static pin_t encoders_pad_a[NUM_ENCODERS_MAX_PER_SIDE] = ENCODERS_PAD_A_RIGHT;
#    endif // !ENCODERS_PAD_A_INT_DISABLE
#    ifndef ENCODERS_PAD_B_INT_DISABLE
    static pin_t encoders_pad_b[NUM_ENCODERS_MAX_PER_SIDE] = ENCODERS_PAD_B_RIGHT;
#    endif

    if (mm_get_sleep_level() == mm_sleep_level_timeout) {
        for (uint8_t i = 0; i < NUM_ENCODERS; i++) {
#    ifndef ENCODERS_PAD_A_INT_DISABLE
            _pal_lld_enablepadevent(PAL_PORT(encoders_pad_a[i]), PAL_PAD(encoders_pad_a[i]), PAL_EVENT_MODE_BOTH_EDGES);
            wb32_exti_nvic_init(PAL_PAD(encoders_pad_a[i]));
#    endif
#    ifndef ENCODERS_PAD_B_INT_DISABLE
            _pal_lld_enablepadevent(PAL_PORT(encoders_pad_b[i]), PAL_PAD(encoders_pad_b[i]), PAL_EVENT_MODE_BOTH_EDGES);
            wb32_exti_nvic_init(PAL_PAD(encoders_pad_b[i]));
#    endif
        }
    }
#endif

#ifdef MM_MODE_SW_PIN
    if (MATRIX_SPLIT_IS_MASTER()) {
        _pal_lld_enablepadevent(PAL_PORT(MM_MODE_SW_PIN), PAL_PAD(MM_MODE_SW_PIN), PAL_EVENT_MODE_BOTH_EDGES);
        wb32_exti_nvic_init(PAL_PAD(MM_MODE_SW_PIN));
    }
#elif defined(MM_BT_DEF_PIN) && defined(MM_2G4_DEF_PIN)
    if (MATRIX_SPLIT_IS_MASTER()) {
        _pal_lld_enablepadevent(PAL_PORT(MM_BT_DEF_PIN), PAL_PAD(MM_BT_DEF_PIN), PAL_EVENT_MODE_BOTH_EDGES);
        wb32_exti_nvic_init(PAL_PAD(MM_BT_DEF_PIN));
        _pal_lld_enablepadevent(PAL_PORT(MM_2G4_DEF_PIN), PAL_PAD(MM_2G4_DEF_PIN), PAL_EVENT_MODE_BOTH_EDGES);
        wb32_exti_nvic_init(PAL_PAD(MM_2G4_DEF_PIN));
    }
#endif
}

static void exti_init(mm_sleep_level_t sleep_level) {

#ifndef LP_DIODE_DIRECTION
#    define LP_DIODE_DIRECTION DIODE_DIRECTION
#endif

#ifndef LP_ACTIVE_STATE
#    define LP_ACTIVE_STATE 0
#endif

#ifndef LP_PAL_EVENT_MODE
#    define LP_PAL_EVENT_MODE PAL_EVENT_MODE_BOTH_EDGES
#endif

#if (LP_ACTIVE_STATE != 0) && (LP_ACTIVE_STATE != 1)
#    error "LP_ACTIVE_STATE must be one of 0 or 1!"
#endif

    lp_exti_init_kb();

#if LP_DIODE_DIRECTION == ROW2COL
    for (int col = 0; col < LP_MATRIX_COLS; col++) {
        if ((sleep_level < mm_sleep_level_shutdown)
#    ifdef LP_COL_PINS
            || ((sleep_level == mm_sleep_level_shutdown) && (LP_COL_PINS == col_pins[col]))
#    endif
        ) {
            if (MATRIX_SPLIT_IS_MASTER_RIGHT()) {
                if (col_pins[col] != NO_PIN) {
#    if LP_ACTIVE_STATE == 0
                    setPinOutputOpenDrain(col_pins[col]);
                    writePinLow(col_pins[col]);
#    elif LP_ACTIVE_STATE == 1
                    setPinOutput(col_pins[col]);
                    writePinHigh(col_pins[col]);
#    endif
                }
            } else {
#    if defined(MATRIX_ROW_PINS_RIGHT) && defined(MATRIX_COL_PINS_RIGHT)
                if (col_pins_r[col] != NO_PIN) {
#        if LP_ACTIVE_STATE == 0
                    setPinOutputOpenDrain(col_pins_r[col]);
                    writePinLow(col_pins_r[col]);
#        elif LP_ACTIVE_STATE == 1
                    setPinOutput(col_pins_r[col]);
                    writePinHigh(col_pins_r[col]);
#        endif
                }
#    endif
            }
        } else {
            if (MATRIX_SPLIT_IS_MASTER_RIGHT()) {
                if (col_pins[col] != NO_PIN) {
#    if LP_ACTIVE_STATE == 1
                    setPinInputLow(col_pins[col]);
#    elif LP_ACTIVE_STATE == 0
                    setPinInputHigh(col_pins[col]);
#    endif
                }
            } else {
#    if defined(MATRIX_ROW_PINS_RIGHT) && defined(MATRIX_COL_PINS_RIGHT)
                if (col_pins_r[col] != NO_PIN) {
#        if LP_ACTIVE_STATE == 1
                    setPinInputLow(col_pins_r[col]);
#        elif LP_ACTIVE_STATE == 0
                    setPinInputHigh(col_pins_r[col]);
#        endif
                }
#    endif
            }
        }
    }

    for (int row = 0; row < LP_MATRIX_ROWS; row++) {
        if ((sleep_level < mm_sleep_level_shutdown)
#    ifdef LP_ROW_PINS
            || ((sleep_level == mm_sleep_level_shutdown) && (LP_ROW_PINS == row_pins[row]))
#    endif
        ) {
            if (MATRIX_SPLIT_IS_MASTER_RIGHT()) {
                if (row_pins[row] != NO_PIN) {
#    if LP_ACTIVE_STATE == 0
                    setPinInputHigh(row_pins[row]);
#    elif LP_ACTIVE_STATE == 1
                    setPinInputLow(row_pins[row]);
#    endif
                    waitInputPinDelay();
                    _pal_lld_enablepadevent(PAL_PORT(row_pins[row]), PAL_PAD(row_pins[row]), LP_PAL_EVENT_MODE);
                    wb32_exti_nvic_init(PAL_PAD(row_pins[row]));
                }
            } else {
#    if defined(MATRIX_ROW_PINS_RIGHT) && defined(MATRIX_COL_PINS_RIGHT)
                if (row_pins_r[row] != NO_PIN) {
#        if LP_ACTIVE_STATE == 0
                    setPinInputHigh(row_pins_r[row]);
#        elif LP_ACTIVE_STATE == 1
                    setPinInputLow(row_pins_r[row]);
#        endif
                    waitInputPinDelay();
                    _pal_lld_enablepadevent(PAL_PORT(row_pins_r[row]), PAL_PAD(row_pins_r[row]), LP_PAL_EVENT_MODE);
                    wb32_exti_nvic_init(PAL_PAD(row_pins_r[row]));
                }
#    endif
            }
        } else {
            if (MATRIX_SPLIT_IS_MASTER_RIGHT()) {
                if (row_pins[row] != NO_PIN) {
#    if LP_ACTIVE_STATE == 1
                    setPinInputHigh(row_pins[row]);
#    elif LP_ACTIVE_STATE == 0
                    setPinInputLow(row_pins[row]);
#    endif
                }
            } else {
#    if defined(MATRIX_ROW_PINS_RIGHT) && defined(MATRIX_COL_PINS_RIGHT)
                if (row_pins_r[row] != NO_PIN) {
#        if LP_ACTIVE_STATE == 1
                    setPinInputHigh(row_pins_r[row]);
#        elif LP_ACTIVE_STATE == 0
                    setPinInputLow(row_pins_r[row]);
#        endif
                }
#    endif
            }
        }
#elif LP_DIODE_DIRECTION == COL2ROW
    for (int row = 0; row < LP_MATRIX_ROWS; row++) {
        if ((sleep_level < mm_sleep_level_shutdown)
#    ifdef LP_ROW_PINS
            || ((sleep_level == mm_sleep_level_shutdown) && (LP_ROW_PINS == row_pins[row]))
#    endif
        ) {
            if (row_pins[row] != NO_PIN) {
#    if LP_ACTIVE_STATE == 0
                setPinOutputOpenDrain(row_pins[row]);
                writePinLow(row_pins[row]);
#    elif LP_ACTIVE_STATE == 1
                setPinOutput(row_pins[row]);
                writePinHigh(row_pins[row]);
#    endif
            }
        } else {
            if (row_pins[row] != NO_PIN) {
#    if LP_ACTIVE_STATE == 1
                setPinInputLow(row_pins[row]);
#    elif LP_ACTIVE_STATE == 0
                setPinInputHigh(row_pins[row]);
#    endif
            }
        }
    }

    for (int col = 0; col < LP_MATRIX_COLS; col++) {
        if ((sleep_level < mm_sleep_level_shutdown)
#    ifdef LP_COL_PINS
            || ((sleep_level == mm_sleep_level_shutdown) && (LP_COL_PINS == col_pins[col]))
#    endif
        ) {
            if (col_pins[col] != NO_PIN) {
#    if LP_ACTIVE_STATE == 0
                setPinInputHigh(col_pins[col]);
#    elif LP_ACTIVE_STATE == 1
                setPinInputLow(col_pins[col]);
#    endif
                waitInputPinDelay();
                _pal_lld_enablepadevent(PAL_PORT(col_pins[col]), PAL_PAD(col_pins[col]), LP_PAL_EVENT_MODE);
                wb32_exti_nvic_init(PAL_PAD(col_pins[col]));
            }
        } else {
            if (col_pins[col] != NO_PIN) {
#    if LP_ACTIVE_STATE == 1
                setPinInputHigh(col_pins[col]);
#    elif LP_ACTIVE_STATE == 0
                setPinInputLow(col_pins[col]);
#    endif
            }
        }
#else
#    error LP_DIODE_DIRECTION must be one of COL2ROW or ROW2COL!
#endif
    }

#ifdef LP_UART_WAKEUP
    setPinInput(SD1_RX_PIN);
    waitInputPinDelay();
    _pal_lld_enablepadevent(PAL_PORT(SD1_RX_PIN), PAL_PAD(SD1_RX_PIN), PAL_EVENT_MODE_BOTH_EDGES);
    wb32_exti_nvic_init(PAL_PAD(SD1_RX_PIN));
#endif

#ifdef SPLIT_KEYBOARD
    setPinInput(SERIAL_USART_RX_PIN);
    waitInputPinDelay();
    _pal_lld_enablepadevent(PAL_PORT(SERIAL_USART_RX_PIN), PAL_PAD(SERIAL_USART_RX_PIN), PAL_EVENT_MODE_BOTH_EDGES);
    wb32_exti_nvic_init(PAL_PAD(SERIAL_USART_RX_PIN));
#endif
}

__WEAK bool lp_isr_code_user(uint8_t channel) {
    return true;
}

__WEAK bool lp_isr_code_kb(uint8_t channel) {
    return true;
}

static void lp_serve_irq(uint32_t pr, uint8_t channel) {
    if ((pr) & (1U << (channel))) {
        switch (channel) {
            case (18): { // exti line 18
                mm_set_sleep_wakeupcd(mm_wakeup_usb);
            } break;
#ifdef MM_BAT_CABLE_PIN
            case PAL_PAD(MM_BAT_CABLE_PIN): {
                mm_set_sleep_wakeupcd(mm_wakeup_cable);
            } break;
#endif
#if defined(LP_UART_WAKEUP) && defined(SD1_RX_PIN)
            case PAL_PAD(SD1_RX_PIN): {
                mm_set_sleep_wakeupcd(mm_wakeup_uart);
            } break;
#endif
#if defined(MM_MODE_SW_PIN)
            case PAL_PAD(MM_MODE_SW_PIN): {
                mm_set_sleep_wakeupcd(mm_wakeup_switch);
            } break;
#endif
#if defined(MM_BT_DEF_PIN)
            case PAL_PAD(MM_BT_DEF_PIN): {
                mm_set_sleep_wakeupcd(mm_wakeup_switch);
            } break;
#endif
#if defined(MM_2G4_DEF_PIN)
            case PAL_PAD(MM_2G4_DEF_PIN): {
                mm_set_sleep_wakeupcd(mm_wakeup_switch);
            } break;
#endif
#if LP_DIODE_DIRECTION == ROW2COL
#    if defined(LP_ROW_PINS)
            case PAL_PAD(LP_ROW_PINS): {
                mm_set_sleep_wakeupcd(mm_wakeup_onekey);
            } break;
#    endif
#elif LP_DIODE_DIRECTION == COL2ROW
#    if defined(LP_COL_PINS)
            case PAL_PAD(LP_COL_PINS): {
                mm_set_sleep_wakeupcd(mm_wakeup_onekey);
            } break;
#    endif
#endif
            default: {
                mm_set_sleep_wakeupcd(mm_wakeup_matrix);
            } break;
        }
        if (lp_isr_code_user(channel) != false) {
            lp_isr_code_kb(channel);
        }
    }
}

static void stop_mode_entry(void) {

    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;

#if 1
    EXTI->PR = 0x7FFFF;
    for (uint8_t i = 0; i < 8; i++) {
        for (uint8_t j = 0; j < 32; j++) {
            if (NVIC->ISPR[i] & (0x01UL < j)) {
                NVIC->ICPR[i] = (0x01UL < j);
            }
        }
    }
    SCB->ICSR |= SCB_ICSR_PENDSTCLR_Msk; // Clear Systick IRQ Pending
#endif

    /* Clear all bits except DBP and FCLKSD bit */
    PWR->CR0 &= 0x09U;

    // STOP LP4 MODE S32KON
    PWR->CR0 |= 0x3B004U;
    PWR->CFGR = 0x3B3;

    PRE_LP();

    /* Set SLEEPDEEP bit of Cortex System Control Register */
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

    /* Request Wait For Interrupt */
    __WFI();

    POST_LP();

    /* Clear SLEEPDEEP bit of Cortex System Control Register */
    SCB->SCR &= (~SCB_SCR_SLEEPDEEP_Msk);

    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
}

OSAL_IRQ_HANDLER(WB32_EXTI0_IRQ_VECTOR) {
    uint32_t pr;

    OSAL_IRQ_PROLOGUE();

    pr = EXTI->PR & EXTI_PR_PR0;

    if ((pr & EXTI_PR_PR0) != 0) {
        EXTI->IMR &= ~(EXTI_IMR_MR0);
        lp_serve_irq(pr, 0);
    }

    NVIC_DisableIRQ(EXTI0_IRQn);

    EXTI->PR = EXTI_PR_PR0;

    OSAL_IRQ_EPILOGUE();
}

OSAL_IRQ_HANDLER(WB32_EXTI1_IRQ_VECTOR) {
    uint32_t pr;

    OSAL_IRQ_PROLOGUE();

    pr = EXTI->PR & EXTI_PR_PR1;

    if ((pr & EXTI_PR_PR1) != 0) {
        EXTI->IMR &= ~(EXTI_IMR_MR1);
        lp_serve_irq(pr, 1);
    }

    NVIC_DisableIRQ(EXTI1_IRQn);

    EXTI->PR = EXTI_PR_PR1;

    OSAL_IRQ_EPILOGUE();
}

OSAL_IRQ_HANDLER(WB32_EXTI2_IRQ_VECTOR) {
    uint32_t pr;

    OSAL_IRQ_PROLOGUE();

    pr = EXTI->PR & EXTI_PR_PR2;

    if ((pr & EXTI_PR_PR2) != 0) {
        EXTI->IMR &= ~(EXTI_IMR_MR2);
        lp_serve_irq(pr, 2);
    }

    NVIC_DisableIRQ(EXTI2_IRQn);

    EXTI->PR = EXTI_PR_PR2;

    OSAL_IRQ_EPILOGUE();
}

OSAL_IRQ_HANDLER(WB32_EXTI3_IRQ_VECTOR) {
    uint32_t pr;

    OSAL_IRQ_PROLOGUE();

    pr = EXTI->PR & EXTI_PR_PR3;

    if ((pr & EXTI_PR_PR3) != 0) {
        EXTI->IMR &= ~(EXTI_IMR_MR3);
        lp_serve_irq(pr, 3);
    }

    NVIC_DisableIRQ(EXTI3_IRQn);

    EXTI->PR = EXTI_PR_PR3;

    OSAL_IRQ_EPILOGUE();
}

OSAL_IRQ_HANDLER(WB32_EXTI4_IRQ_VECTOR) {
    uint32_t pr;

    OSAL_IRQ_PROLOGUE();

    pr = EXTI->PR & EXTI_PR_PR4;

    if ((pr & EXTI_PR_PR4) != 0) {
        EXTI->IMR &= ~(EXTI_IMR_MR4);
        lp_serve_irq(pr, 4);
    }

    NVIC_DisableIRQ(EXTI4_IRQn);

    EXTI->PR = EXTI_PR_PR4;

    OSAL_IRQ_EPILOGUE();
}

OSAL_IRQ_HANDLER(WB32_EXTI9_5_IRQ_VECTOR) {
    uint32_t pr;

    OSAL_IRQ_PROLOGUE();

    pr = EXTI->PR & (EXTI_PR_PR5 | EXTI_PR_PR6 | EXTI_PR_PR7 | EXTI_PR_PR8 | EXTI_PR_PR9);

    if ((pr & EXTI_PR_PR5) != 0) {
        EXTI->IMR &= ~(EXTI_IMR_MR5);
        lp_serve_irq(pr, 5);
    }

    if ((pr & EXTI_PR_PR6) != 0) {
        EXTI->IMR &= ~(EXTI_IMR_MR6);
        lp_serve_irq(pr, 6);
    }

    if ((pr & EXTI_PR_PR7) != 0) {
        EXTI->IMR &= ~(EXTI_IMR_MR7);
        lp_serve_irq(pr, 7);
    }

    if ((pr & EXTI_PR_PR8) != 0) {
        EXTI->IMR &= ~(EXTI_IMR_MR8);
        lp_serve_irq(pr, 8);
    }

    if ((pr & EXTI_PR_PR9) != 0) {
        EXTI->IMR &= ~(EXTI_IMR_MR9);
        lp_serve_irq(pr, 9);
    }

    NVIC_DisableIRQ(EXTI9_5_IRQn);

    EXTI->PR = EXTI_PR_PR5 |
               EXTI_PR_PR6 |
               EXTI_PR_PR7 |
               EXTI_PR_PR8 |
               EXTI_PR_PR9;

    OSAL_IRQ_EPILOGUE();
}

OSAL_IRQ_HANDLER(WB32_EXTI15_10_IRQ_VECTOR) {
    uint32_t pr;

    OSAL_IRQ_PROLOGUE();

    pr = EXTI->PR & (EXTI_PR_PR10 | EXTI_PR_PR11 | EXTI_PR_PR12 | EXTI_PR_PR13 | EXTI_PR_PR14 | EXTI_PR_PR15);

    if ((pr & EXTI_PR_PR10) != 0) {
        EXTI->IMR &= ~(EXTI_IMR_MR10);
        lp_serve_irq(pr, 10);
    }

    if ((pr & EXTI_PR_PR11) != 0) {
        EXTI->IMR &= ~(EXTI_IMR_MR11);
        lp_serve_irq(pr, 11);
    }

    if ((pr & EXTI_PR_PR12) != 0) {
        EXTI->IMR &= ~(EXTI_IMR_MR12);
        lp_serve_irq(pr, 12);
    }

    if ((pr & EXTI_PR_PR13) != 0) {
        EXTI->IMR &= ~(EXTI_IMR_MR13);
        lp_serve_irq(pr, 13);
    }

    if ((pr & EXTI_PR_PR14) != 0) {
        EXTI->IMR &= ~(EXTI_IMR_MR14);
        lp_serve_irq(pr, 14);
    }

    if ((pr & EXTI_PR_PR15) != 0) {
        EXTI->IMR &= ~(EXTI_IMR_MR15);
        lp_serve_irq(pr, 15);
    }

    NVIC_DisableIRQ(EXTI15_10_IRQn);

    EXTI->PR = EXTI_PR_PR10 |
               EXTI_PR_PR11 |
               EXTI_PR_PR12 |
               EXTI_PR_PR13 |
               EXTI_PR_PR14 |
               EXTI_PR_PR15;

    OSAL_IRQ_EPILOGUE();
}

OSAL_IRQ_HANDLER(WB32_USBP1_WKUP_IRQ_VECTOR) {
    uint32_t pr;

    OSAL_IRQ_PROLOGUE();

    pr = EXTI->PR & (EXTI_PR_PR18);

    if ((pr & EXTI_PR_PR18) != 0) {
        EXTI->IMR &= ~(EXTI_IMR_MR18);
        lp_serve_irq(pr, 18);
    }

    NVIC_DisableIRQ(USBP_WKUP_IRQn);

    EXTI->PR = EXTI_PR_PR18;

    OSAL_IRQ_EPILOGUE();
}

#if defined(WAKEUP_RESET) && defined(BOOTMAGIC_LITE)
void bootmagic_lite(void) {

    if (PWR->GPREG0 != 0x5AA5) {

        // We need multiple scans because debouncing can't be turned off.
        matrix_scan();
#    if defined(DEBOUNCE) && DEBOUNCE > 0
        wait_ms(DEBOUNCE * 2);
#    else
        wait_ms(30);
#    endif
        matrix_scan();

        // If the configured key (commonly Esc) is held down on power up,
        // reset the EEPROM valid state and jump to bootloader.
        // This isn't very generalized, but we need something that doesn't
        // rely on user's keymaps in firmware or EEPROM.
        uint8_t row = BOOTMAGIC_LITE_ROW;
        uint8_t col = BOOTMAGIC_LITE_COLUMN;

#    if defined(SPLIT_KEYBOARD) && defined(BOOTMAGIC_LITE_ROW_RIGHT) && defined(BOOTMAGIC_LITE_COLUMN_RIGHT)
        if (!is_keyboard_left()) {
            row = BOOTMAGIC_LITE_ROW_RIGHT;
            col = BOOTMAGIC_LITE_COLUMN_RIGHT;
        }
#    endif

        if (matrix_get_row(row) & (1 << col)) {
            eeconfig_disable();

            // Jump to bootloader.
            bootloader_jump();
        }
    }

    PWR->GPREG0 = 0x5AA5;
}
#endif
