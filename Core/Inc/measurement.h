// Copyright (c) 2023 Cullen Jennings
#pragma once

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

  uint32_t ltcSeconds; // time of last LTC pulse  
  uint32_t ltcSecondsTick;
  uint32_t ltcGenTick;

  // TODO
  // uint32_t gpsSeconds; // time of last GPS pulse 
  // uint32_t gpsSecondsTick;
  
  uint32_t localSeconds; // number of times counter has rolled over
  uint32_t localSecondsTick;
  uint32_t extSeconds; // number of times counter has rolled over 
  uint32_t extSecondsTick; 
  
} Measurements;

extern Measurements data;

