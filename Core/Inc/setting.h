// Copyright (c) 2023 Cullen Jennings
#pragma once


#include <stdbool.h>
#include <stdint.h>


typedef struct {
  uint8_t blinkMute;       // mutes audio outout
  uint8_t blinkBlank;      // causes LED to be off
  uint8_t blinkDispAudio;  // caused audio latency to be displayed on LED
  
  uint8_t blinkHaveDisplay;
  
  uint8_t blinkPPS; // do PPS instead of LTC out output
  
} Setting;

extern Setting setting;

  
void settingInit();
void settingSetup();

