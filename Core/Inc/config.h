// SPDX-FileCopyrightText: Copyright (c) 2023 Cullen Jennings
// SPDX-License-Identifier: BSD-2-Clause

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "setting.h"

// This structure is saved in EEPROM
// keep size padded to 32 bits
typedef struct {
  uint8_t version;   // 2 - version of config format
  uint8_t product;   // 1=blink, 2=clock, 3=gps
  uint8_t revMajor;  // board major version, 0 is prototypes
  uint8_t revMinor;  // start at 1 - board minor version

  uint16_t serialNum;  // 0 is not valid

  int16_t oscAdj;      // offset for intenal oscilator counter
  uint16_t vcoValue;   // value loaded in DAC for VCO
  uint8_t extOscType;  // external osc type ( 2= 2.048 MHz, 10=10 MHz,
                       // 0=Internal ) )
  uint8_t usePPS;      // 0
  uint8_t future13;    // 0
  uint8_t future14;    // 0
  uint8_t future15;    // 0
} Config;

extern Config config;

inline uint32_t capture2uS(const uint32_t c) {
  // The main timer counter max value (typically 10 MHz ) * M need this
  // cacluation to fit in 32 bits
  return (c * setting.capture2uSRatioM) / setting.capture2uSRatioN;
}

inline uint32_t extCapture2uS(const uint32_t c) { return c / 10l; }

void configInit();
void configSetup();

void setClk(uint8_t extOscTypeType, uint16_t vcoValue, int16_t oscAdj);
