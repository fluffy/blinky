// Copyright (c) 2023 Cullen Jennings
#pragma once

#include <stdint.h>

enum {
  metricsHistorySize = 120
};  // use enum to get const integer for array size
typedef struct {
  int nextIndex;
  int64_t  localTimeUS[metricsHistorySize];
  int64_t  extTimeUS[metricsHistorySize];
  int64_t  syncTimeUS[metricsHistorySize];
  
} Metrics;

extern Metrics metrics;

void metricsInit();
void metricsSetup();
void metricsRun();