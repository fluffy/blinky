// Copyright (c) 2023 Cullen Jennings
#pragma once

#include <stdint.h>

typedef struct {
  int64_t  localTimeUS;
  int64_t  extTimeUS;
  int64_t  syncTimeUS;
  
} Metrics;

extern Metrics metrics;

void metricsInit();
void metricsSetup();
void metricsRun();
