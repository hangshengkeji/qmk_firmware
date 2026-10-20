/* Copyright (C) 2023 Westberry Technology (ChangZhou) Corp., Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#define BTS_LIB_VERSION 2.8.2

typedef enum {
  DEVS_USB = 0,
  DEVS_BT1,
  DEVS_BT2,
  DEVS_BT3,
  DEVS_BT4,
  DEVS_BT5,
  DEVS_2G4,
} devs_t;

typedef enum {
  v_usb               = 0x11, // USB模式
  v_2g4               = 0x30, // 2.4G模式
  v_bt1               = 0x31, // 蓝牙1
  v_bt2               = 0x32, // 蓝牙2
  v_bt3               = 0x33, // 蓝牙3
  v_bt4               = 0x34, // 蓝牙4
  v_bt5               = 0x35, // 蓝牙5
  v_pair              = 0x51, // 无线配对
  v_clear             = 0x52, // 清除配对信息
  v_query_vol         = 0x53, // 查询电量信息
  v_en_sleep_bt       = 0x55, // 允许休眠蓝牙30分钟
  v_dis_sleep_bt      = 0x56, // 禁止蓝牙休眠
  v_en_sleep_wl       = 0x57, // 允许休眠2.4G30分钟
  v_dis_sleep_wl      = 0x58, // 禁止2.4G休眠
  v_bat_charging      = 0x64, // 正在充电
  v_bat_stop_charging = 0x65, // 停止充电
  v_bat_full          = 0x66, // 电量充满
  v_get_version       = 0x70, // 获取模块固件版本号
  v_ota               = 0x81, // 无线空中升级
  v_fixed_freq        = 0x82, // 进入定频模式
} vbs_t;

typedef struct {
  bool sleeped;           // 已休眠
  bool low_vol;           // 低电压
  bool low_vol_offed;     // 低电压关机
  bool normal_vol;        // 正常电压
  bool pairing;           // 正在配对
  bool paired;            // 已配对
  bool come_back;         // 进入回连状态
  bool come_back_err;     // 回连失败
  bool no_pair_info;      // 蓝牙无配对信息
  bool mode_switched;     // 模式已切换
  bool active;            // 键盘处于活动状态
  uint8_t pvol;           // 电压百分比
  uint8_t indicators;     // LED指示灯状态
  uint8_t version;        // Module FW version
  uint8_t pcstate;        // PC状态 0: suspend 1: resume
} bt_info_t;

/**
  * @brief  BTS Init structure definition
  */
typedef struct
{
  const char *bt_name[5];
  bt_info_t bt_info;
  void (*uart_init)(uint32_t);
  uint8_t (*uart_read)(void);
  void (*uart_transmit)(const uint8_t *, uint16_t);
  void (*uart_receive)(uint8_t *, uint16_t);
  bool (*uart_available)(void);
  uint32_t (*timer_read32)(void);
  void (*bts_pcstate_cb)(uint8_t);
} bts_info_t;

void bts_init(bts_info_t *info);
void bts_task(devs_t dev_state);
bool bts_process_keys(uint16_t keycode, bool pressed, devs_t dev_state, bool no_gui, bool nkro_en);

bool bts_clear_all_keys(void);
bool bts_send_fn(bool pressed);
bool bts_send_name(devs_t host);
bool bts_send_vendor(vbs_t cmd);
bool bts_send_bat_pvol(uint8_t pvol);
bool bts_send_mouse_report(uint8_t *report);
void bts_test_report_rate_task(void);
char* bts_get_version(void);
uint8_t bts_is_busy(void);

bool bts_send_manufacturer(const char *str, uint8_t strlen);
bool bts_send_product(const char *str, uint8_t strlen);
bool bts_send_vpid(uint16_t vid, uint16_t pid);

bool bts_via_task(void);
bool bts_send_via(uint8_t *data, uint8_t length);

bool bts_rf_send_modulate(uint8_t channel, uint8_t payload_type, uint8_t tx_power, uint8_t phy);
bool bts_rf_send_burst(uint8_t channel, uint8_t payload_type, uint8_t tx_power, uint8_t phy, uint8_t len);
bool bts_rf_send_carrier(uint8_t channel, uint8_t tx_power, uint8_t phy);
bool bts_rf_send_stop(void);
