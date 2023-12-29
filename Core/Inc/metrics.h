// Copyright (c) 2023 Cullen Jennings
#pragma once

#include <stdint.h>

typedef struct {
  uint64_t  localTimeUS;
  uint64_t  extTimeUS;
  uint64_t  syncTimeUS;
  
} Metrics;

extern Metrics metrics;

void metricsInit();
void metricsSetup();
void metricsRun();
