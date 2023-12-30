// Copyright (c) 2023 Cullen Jennings
#pragma once

#include <stdint.h>

typedef struct {
  uint32_t gpsCapture;
  uint32_t gpsCaptureTick;
  uint32_t monCapture;
  uint32_t monCaptureTick;
  uint32_t syncCapture;
  uint32_t syncCaptureTick;

  uint32_t gpsAuxCapture;
  uint32_t gpsAuxCaptureTick;
  uint32_t monAuxCapture;
  uint32_t monAuxCaptureTick;
  // there is no sync captue for Aux

  uint32_t ltcSeconds;  // time of last LTC pulse
  uint32_t ltcSecondsTick;
  uint32_t ltcAtMonSeconds;  // time of last LTC pulse when last Mon pulse happed
  uint32_t ltcGenTick;

  uint32_t gpsSeconds; // time of last GPS pulse
  uint32_t gpsAtMonSeconds;  // time of last GPS pulse when last Mon pulse happed
  uint32_t gpsSecondsTick;

  uint32_t localSeconds;  // number of times counter has rolled over
  uint32_t localAtMonSeconds;  // localSeconds at last Mon pulse 
  uint32_t localSecondsTick;
  
  uint32_t extSeconds;  // number of times counter has rolled over
  uint32_t extAtMonSeconds;  // extSeconds at last Mon pulse 
  uint32_t extSecondsTick;
  int32_t extOffsetUS;  // offset to make local = ext + offset at time of sync 

} Measurements;

extern Measurements data;
