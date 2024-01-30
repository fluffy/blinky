// SPDX-FileCopyrightText: Copyright (c) 2023 Cullen Jennings
// SPDX-License-Identifier: BSD-2-Clause

#include "pps.h"

#include "audio.h"
#include "hardware.h"
#include "ltc.h"
#include "measurement.h"
#include "setting.h"

// extern uint32_t dataLtcGenTick;
extern LtcTransitionSet ltcSendTransitions;
//extern uint8_t blinkMute;

extern uint32_t dataNextSyncOutPhaseUS;
extern uint32_t dataCurrentSyncOutPhaseUS;

void ppsInit() {}

void ppsSetup() {
  HAL_TIM_OC_Start_IT(&hTimePps, TimePps_CH_SYNC_OUT);  // start sync out
}

void ppsStart() {
  // start the output timer pulse
  uint32_t v =
      (ltcSendTransitions.transitionTimeUs[0] + dataCurrentSyncOutPhaseUS) /
      20l;  // convert to 50 KHz timer counts
  if (v >= 50000) {
    v -= 50000;
  }
  __HAL_TIM_SET_COMPARE(&hTimePps, TimePps_CH_SYNC_OUT, v);
  LL_TIM_OC_SetMode(
      TIM1, TimePps_LL_CH_SYNC_OUT,
      LL_TIM_OCMODE_INACTIVE  // inverted due to inverting output buffer
  );

  ltcSendTransitions.nextTransition = 1;
}

void ppsStop() {}

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim) {
  uint32_t tick = HAL_GetTick();

#if 1  // DO LTC
  if (htim == &hTimePps) {
    if (ltcSendTransitions.nextTransition == 1) {
      if (!setting.blinkMute) {
        // start audio output
        audioStart();
      }
    }

    if (ltcSendTransitions.nextTransition ==
        ltcSendTransitions.numTransitions - 1) {
      // stop audio output
      audioStop();
    }

    if (ltcSendTransitions.nextTransition < ltcSendTransitions.numTransitions) {
      uint32_t v = (ltcSendTransitions
                        .transitionTimeUs[ltcSendTransitions.nextTransition] +
                    dataCurrentSyncOutPhaseUS) /
                   20l;  // convert to 50 KHz timer counts
      if (v >= 50000) {
        v -= 50000;
      }

      __HAL_TIM_SET_COMPARE(&hTimePps, TimePps_CH_SYNC_OUT, v);
      LL_TIM_OC_SetMode(
          TIM1, TimePps_LL_CH_SYNC_OUT,
          (ltcSendTransitions.nextTransition % 2 == 0) ? LL_TIM_OCMODE_INACTIVE
                                                       : LL_TIM_OCMODE_ACTIVE
          // inverted due to inverting output buffer
      );

      ltcSendTransitions.nextTransition++;

      if (ltcSendTransitions.nextTransition >=
          ltcSendTransitions.numTransitions) {
        // will restart when new code generated ltcSendTransitions.nextTransition
        // = 0;  // restart
        dataCurrentSyncOutPhaseUS = dataNextSyncOutPhaseUS;
        data.ltcGenTick = tick;
      }
    }
  }
#else  // PPS instead of LTC
  if (htim == &hTimePps) {
    static int alternate = 0;  // TODO
    uint16_t val = __HAL_TIM_GET_COMPARE(&hTimePps, TimePps_CH_SYNC_OUT);
    if ((alternate++) % 2) {  // TODO if (val != dataCurrentSyncOutPhase ) {
      // end of output pulse just happened, set up for next output pulse
      dataCurrentSyncOutPhase = dataNextSyncOutPhase;
      __HAL_TIM_SET_COMPARE(&hTimePps, TimePps_CH_SYNC_OUT,
                            dataCurrentSyncOutPhase);
      LL_TIM_OC_SetMode(
          TIM1, TimePps_LL_CH_SYNC_OUT,
          LL_TIM_OCMODE_INACTIVE);  // inverted due to inverting output buffer

      // stop audio output
      audioStop();
    } else {  // val == dataCurrentSyncOutPhase
              // start of output pulse just started, set up for the end of pulse
      uint16_t v = dataCurrentSyncOutPhase +
                   500l;  // TODOP blinkAudioPulseWidthMs * 50l;
      if (v >= 50000) {
        v -= 50000;
      }
      __HAL_TIM_SET_COMPARE(&hTimePps, TimePps_CH_SYNC_OUT, v);
      LL_TIM_OC_SetMode(
          TIM1, TimePps_LL_CH_SYNC_OUT,
          LL_TIM_OCMODE_ACTIVE);  // inverted due to inverting output buffer

      if (!blinkMute) {
        // start audio output
        audioStart();
      }
    }
  }
#endif
}
