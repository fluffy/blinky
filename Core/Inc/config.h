// Copyright (c) 2023 Cullen Jennings
#pragma once

#include <stdbool.h>
#include <stdint.h>

// This structure is saved in EEPROM
// keep size padded to 32 bits
typedef struct {
  uint8_t version;   // 2 - version of config format
  uint8_t product;   // 1=blink, 2=clock
  uint8_t revMajor;  // 0x1 is Rev A - board major version
  uint8_t revMinor;  // start at 0 - board minor version

  uint16_t serialNum;  // 0 is not valid

  int16_t oscAdj;      // offset for intenal oscilator counter
  uint16_t vcoValue;   // value loaded in DAC for VCO
  uint8_t extOscType;  // external osc type ( 2= 2.048 MHz, 10=10 MHz,
                       // 0=Internal ) )
  uint8_t usePPS; // 0
  uint8_t future13; // 0
  uint8_t future14; // 0
  uint8_t future15; // 0
} Config;
extern Config config;

extern uint32_t capture2uSRatioM;
extern uint32_t capture2uSRatioN;

inline uint32_t capture2uS(const uint32_t c) {
  return (c * capture2uSRatioM) / capture2uSRatioN;
}

inline uint32_t extCapture2uS(const uint32_t c) { return c / 10l; }

void configInit();
void configSetup();

void setClk(uint8_t extOscTypeType, uint16_t vcoValue, int16_t oscAdj);
