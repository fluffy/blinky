// Copyright (c) 2023 Cullen Jennings
#pragma once

#include <stdbool.h>
#include <stdint.h>

void detectInit(int cycleLen);
void detectUpdate(uint32_t* pData, int len, bool invert);

void detectUpdateMlp(uint32_t time);
void detectGetMlpTime(uint32_t* timeP, float* valP);
void detectResetMlp();

void detectGetDebug(float* minP, float* maxP, float* avgP, float* lastP);
