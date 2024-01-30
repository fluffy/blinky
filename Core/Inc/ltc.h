// SPDX-FileCopyrightText: Copyright (c) 2023 Cullen Jennings
// SPDX-License-Identifier: BSD-2-Clause

#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  uint8_t hour;
  uint8_t min;
  uint8_t sec;
  uint8_t frame;
  bool valid;
} LtcTimeCode;
void ltcTimeCodeClear(LtcTimeCode*);
void ltcTimeCodeSet(LtcTimeCode*, uint32_t s, uint32_t us);
void ltcTimeCodeSetHMSF(LtcTimeCode*, uint8_t h, uint8_t m, uint8_t s,
                        uint8_t f);
uint32_t ltcTimeCodeSeconds(LtcTimeCode*);
uint32_t ltcTimeCodeMicroSeconds(LtcTimeCode*);
uint32_t ltcTimeCodeDisp(LtcTimeCode*);
int ltcTimeCodeIsValid(LtcTimeCode*);

enum {
  ltcMaxTransitions = 80 * 2 + 1
};  // use enum to get const integer for array size
typedef struct {
  uint32_t transitionTimeUs[ltcMaxTransitions];
  uint16_t numTransitions;
  uint16_t nextTransition;
} LtcTransitionSet;
void LtcTransitionSetClear(LtcTransitionSet* set);
void LtcTransitionSetAdd(LtcTransitionSet* set, uint32_t timeUs);
uint16_t LtcTransitionSetSize(LtcTransitionSet* set);
uint32_t LtcTransitionSetDeltaUs(LtcTransitionSet* set, uint16_t i);

typedef struct {
  uint8_t bits[10];  // 80 bits total
  bool valid;
} Ltc;

void ltcClear(Ltc* ltc);
void ltcSet(Ltc* ltc, LtcTimeCode* time);
void ltcGet(Ltc* ltc, LtcTimeCode* time);
uint8_t ltcParity(Ltc* ltc);
void ppsEncode(Ltc* ltc, LtcTransitionSet* tSet );
void ltcEncode(Ltc* ltc, LtcTransitionSet* tSet, uint8_t fps);
int ltcDecode(Ltc* ltc, LtcTransitionSet* tSet, uint8_t fps);
