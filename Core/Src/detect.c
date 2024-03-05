// SPDX-FileCopyrightText: Copyright (c) 2023 Cullen Jennings
// SPDX-License-Identifier: BSD-2-Clause

#include "detect.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#define maxCycleLen 20
static float dataMin;
static float dataMax;
static float dataSum[maxCycleLen];
static float dataAvg100;
static float maxSum;
static float maxSumTime;
static float lastVal;

void detectInit(int cycleLen) {
  assert(cycleLen <= maxCycleLen);

  for (int i = 0; i < maxCycleLen; i++) {
    dataSum[i] = 0.0;
  }
  dataMin = 1e6;
  dataMax = -1e6;
  dataAvg100 = 0.0;

  lastVal = 1e6;
  maxSum = 0.0;
  maxSumTime = 0;
}

void detectUpdate(uint32_t* pData, int len, bool invert) {
  assert(len <= maxCycleLen);

  for (int i = 0; i < len; i++) {
    uint32_t vInt = pData[i];
    float f = vInt;
    if (f < dataMin) {
      dataMin = f;
    }
    if (f > dataMax) {
      dataMax = f;
    }

    dataAvg100 = dataAvg100 * 0.99 + 0.01 * f;
    float v = f - dataAvg100;

    lastVal = v;

    if (invert) {
      dataSum[i] = dataSum[i] * 0.95 - v;
    } else {
      dataSum[i] = dataSum[i] * 0.95 + v;
    }
  }
}

void detectUpdateMlp(uint32_t time) {
  for (int i = 0; i < maxCycleLen; i++) {
    float sum = (dataSum[i] >= 0.0) ? (dataSum[i]) : (-dataSum[i]);
    if (sum > maxSum) {
      maxSum = sum;
      maxSumTime = time;
    }
  }
}

void detectGetMlpTime(uint32_t* timeP, float* valP) {
  *timeP = maxSumTime;
  *valP = maxSum;
}

void detectResetMlp() {
  dataMin = 1e6;
  dataMax = -1e6;
  lastVal = 1e6;
  maxSum = 0.0;
  maxSumTime = 0;
}

void detectGetDebug(float* minP, float* maxP, float* avgP, float* lastP) {
  *minP = dataMin;
  *maxP = dataMax;
  *avgP = dataAvg100;
  *lastP = lastVal;
}
