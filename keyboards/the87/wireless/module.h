// Copyright 2024 Su (@isuua)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#define LIBMODULE_VERSION 0.2.6

// device index
enum {
    DEVS_USB = 0,
    DEVS_BT1,
    DEVS_BT2,
    DEVS_BT3,
    DEVS_BT4,
    DEVS_BT5,
    DEVS_2G4,
};

enum {
    MD_STATE_NONE = 0,
    MD_STATE_PAIRING,
    MD_STATE_CONNECTED,
    MD_STATE_DISCONNECTED,
    MD_STATE_UNPAIRED,
    MD_STATE_REJECT,
};

typedef struct {
    uint32_t (*timer_read32)(void);
    uint8_t (*uart_read)(void);
    void (*uart_transmit)(const uint8_t *, uint16_t);
    bool (*uart_available)(void);
    void (*host_state_cb)(bool resume);
    void (*raw_receive_cb)(uint8_t *pdata, uint8_t len);
    bool (*pkt_receive_cb)(uint8_t *pdata, uint8_t len);
} md_parms_t;

void md_init(md_parms_t parms);
void md_main_task(uint8_t devs);
void md_send_kb(uint8_t *data);
void md_send_nkro(uint8_t *data, uint8_t bits);
void md_send_consumer(uint8_t *data);
void md_send_system(uint8_t *data);
void md_send_fn(uint8_t *data);
void md_send_mouse(uint8_t *data);
void md_send_devctrl(uint8_t cmd);
void md_send_bat(uint8_t pvol);
void md_switch_to_usb(void);
void md_sleep_bt(bool enable);
void md_sleep_2g4(bool enable);
void md_switch_to_bt1(bool reset, char *name);
void md_switch_to_bt2(bool reset, char *name);
void md_switch_to_bt3(bool reset, char *name);
void md_switch_to_bt4(bool reset, char *name);
void md_switch_to_bt5(bool reset, char *name);
void md_switch_to_2g4(bool reset, char *manufacture, uint8_t mlen, char *product, uint8_t plen, uint16_t vid, uint16_t pid, bool new);
void md_send_manufacturer(char *str, uint8_t len);
void md_send_product(char *str, uint8_t len);
void md_send_vpid(uint16_t vid, uint16_t pid);
void md_send_raw(uint8_t *data, uint8_t length);
bool md_send_pkt(uint8_t *data, uint32_t len);
bool md_send_smsg_pkt(uint8_t *data, uint32_t len);
bool md_inquire_bat(void);
bool md_inquire_qbat(void);
bool md_is_busy(void);
bool md_is_full(void);
bool md_is_free(void);
uint8_t md_get_devs(void);
uint8_t *md_getp_bat(void);
uint8_t *md_getp_qbat(void);
uint8_t md_get_version(void);
uint8_t *md_getp_state(void);
uint8_t *md_getp_indicator(void);
char *md_get_libmodule_version(void);
