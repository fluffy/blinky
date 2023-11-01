// Copyright (c) 2023 Cullen Jennings

#ifndef __DETECT_H
#define __DETECT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void detectInit(int cycleLen);
void detectUpdate(uint32_t* pData, int len, bool invert);

void detectUpdateMlp(uint32_t time);
void detectGetMlpTime(uint32_t* timeP, float* valP);
void detectResetMlp();

void detectGetDebug(float* minP, float* maxP, float* avgP, float* lastP);

#ifdef __cplusplus
}
#endif

#endif /* __DETECT_H */
