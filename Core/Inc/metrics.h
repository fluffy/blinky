// Copyright (c) 2023 Cullen Jennings
#pragma once

#include <stdbool.h>
#include <stdint.h>

enum {
  metricsHistorySize = 1024
};  // use enum to get const integer for array size
typedef struct {
  int nextIndex;
  int64_t localTimeUS[metricsHistorySize];
  int64_t extTimeUS[metricsHistorySize];
  int64_t syncTimeUS[metricsHistorySize];
  int64_t gpsTimeUS[metricsHistorySize];

  int32_t lastSyncSeconds; // rought time of when sync button was last hit 
  
  bool haveSync;
  bool haveExt;
  bool haveGps;

} Metrics;

extern Metrics metrics;

void metricsInit();
void metricsSetup();
void metricsRun();
void metricsAdjust();

typedef enum { SourceNone, SourceGPS, SourceSync, SourceExternal } MetricSyncSource;
void metricsSync(MetricSyncSource syncTo);
