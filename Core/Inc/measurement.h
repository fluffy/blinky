// SPDX-FileCopyrightText: Copyright (c) 2023 Cullen Jennings
// SPDX-License-Identifier: BSD-2-Clause

#pragma once

#include <stdint.h>

typedef struct {
  // get the phases of inputs relative to main timer
  uint32_t gpsCapture;
  uint32_t gpsCaptureTick;
  uint32_t monCapture;
  uint32_t monCaptureTick;
  uint32_t syncCapture;
  uint32_t syncCaptureTick;

  // get the phases on inputs relative to the aux timer
  uint32_t gpsAuxCapture;
  uint32_t gpsAuxCaptureTick;
  uint32_t monAuxCapture;
  uint32_t monAuxCaptureTick;
  // there is no sync capture for Aux

  uint32_t ltcSeconds;  // time of last LTC pulse
  uint32_t
      ltcSecondsTick;  // time of LTC decode, not the time of the input pulse
  uint32_t
      ltcAtMonSeconds;  // time of last LTC pulse when last Mon pulse happed
  uint32_t ltcAtMonSecondsTick;

  uint32_t ltcGenTick;

  uint32_t gpsSeconds;      // time of last GPS pulse
  uint32_t gpsSecondsTick;  // time at last decode of GPS value, not time of the
                            // pulse
  uint32_t
      gpsAtMonSeconds;  // time of last GPS pulse when last Mon pulse happed
  uint32_t
      gpsAtMonSecondsTick;  // time of last GPS pulse when last Mon pulse happed

  uint32_t localSeconds;  // increments with main timer rollover
  uint32_t localSecondsTick;
  uint32_t localAtMonSeconds;  // localSeconds at last Mon pulse
  uint32_t localAtMonSecondsTick;
  int32_t localOffsetUS;  // starting phase of localTime after last sync

  uint32_t extSeconds;  // increments with aux timer rollover
  uint32_t extSecondsTick;
  uint32_t extAtMonSeconds;      // extSeconds at last Mon pulse
  uint32_t extAtMonSecondsTick;  // extSeconds at last Mon pulse
  int32_t extOffsetUS;           // starting phase of extTime after last sync

  int32_t blinkAudioDelayMs;

} Measurements;

extern Measurements data;
