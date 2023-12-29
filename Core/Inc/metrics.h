// Copyright (c) 2023 Cullen Jennings
#pragma once

#include <stdint.h>

typedef struct {
} Metrics;

extern Metrics metrics;

void metricsInit();
void metricsSetup();
void metricsRun();
