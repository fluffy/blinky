// Copyright (c) 2023 Cullen Jennings
#pragma once

#include <stdint.h>
#include <stdbool.h>

// This structure is saved in EEPROM
// keep size padded to 32 bits
typedef struct {
  uint8_t version;   // 1
  uint8_t product;   // 1=blink, 2=clock
  uint8_t revMajor;  // 0x1 is Rev A
  uint8_t revMinor;  // start at 0

  uint16_t serialNum;  // 0 is not valid

  int16_t oscAdj;      // offset for intenal oscilator counter
  uint16_t vcoValue;   // value loaded in DAC for VCO
  uint8_t extOscType;  // external osc type ( 2= 2.048 MHz, 10=10 MHz,
                       // 0=Internal ) )
} Config;
extern Config config;

extern uint32_t capture2uSRatioM;
extern uint32_t capture2uSRatioN;

inline uint32_t capture2uS(const uint32_t c) {
  return (c * capture2uSRatioM) / capture2uSRatioN;
}

inline uint32_t extCapture2uS(const uint32_t c) {
  return c / 10l;
}

extern uint8_t blinkMute;       // mutes audio outout
extern uint8_t blinkBlank;      // causes LED to be off
extern uint8_t blinkDispAudio;  // caused audio latency to be displayed on LED
extern uint8_t blinkHaveDisplay; // TODO - make bool with stdbool

void configInit();
void configSetup();

void setClk(uint8_t extOscTypeType, uint16_t vcoValue, int16_t oscAdj);
