// SPDX-FileCopyrightText: Copyright (c) 2023 Cullen Jennings
// SPDX-License-Identifier: BSD-2-Clause

#pragma once


#include <stdbool.h>
#include <stdint.h>


typedef struct {
  bool  blinkMute;       // mutes audio outout
  bool blinkBlank;      // causes LED to be off
  bool blinkDispAudio;  // caused audio latency to be displayed on LED
  
  bool blinkHaveDisplay;
  
  bool blinkPPS; // do PPS instead of LTC out output
  
} Setting;

extern Setting setting;

  
void settingInit();
void settingSetup();

