// SPDX-FileCopyrightText: Copyright (c) 2023 Cullen Jennings
// SPDX-License-Identifier: BSD-2-Clause


#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stm32f4xx_ll_tim.h>
#include <string.h>

#include "hardware.h"
#include "main.h"
#include "disp.h"
#include "setting.h"
#include "measurement.h"


void dispInit(){
  // nothing to do here
}

void dispSetup(){

  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, GPIO_PIN_RESET);
#if 0  // TODO
  HAL_GPIO_WritePin(LED5_GPIO_Port, LED5_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LED6_GPIO_Port, LED6_Pin, GPIO_PIN_RESET);
#endif
  HAL_GPIO_WritePin(LED7_GPIO_Port, LED7_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LED8_GPIO_Port, LED8_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LED9_GPIO_Port, LED9_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LED10_GPIO_Port, LED10_Pin, GPIO_PIN_RESET);

}

void dispUpdate( int32_t dispUs, int32_t dispS ){
   // called in an interupt - keep this fast
      int32_t ledMs = dispUs / 1000l;

    if (setting.blinkDispAudio) {
      ledMs = data.blinkAudioDelayMs;
    }

    if (ledMs >= 1000) {
      ledMs -= 1000;
    }
    int16_t binCount = (ledMs / 100) % 10;  // 2.5 ms

    int row = 5 - (ledMs / 2) % 5;     // 2 ms across
    int col = 10 - (ledMs / 10) % 10;  // 10 ms down

    if (setting.blinkBlank) {
      binCount = 0x1000;
      row = 255;
      col = 255;
    }

    if (setting.blinkHaveDisplay) {
      HAL_GPIO_WritePin(NCOL1_GPIO_Port, NCOL1_Pin,
                        (col == 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(NCOL2_GPIO_Port, NCOL2_Pin,
                        (col == 2) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(NCOL3_GPIO_Port, NCOL3_Pin,
                        (col == 3) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(NCOL4_GPIO_Port, NCOL4_Pin,
                        (col == 4) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(NCOL5_GPIO_Port, NCOL5_Pin,
                        (col == 5) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(NCOL6_GPIO_Port, NCOL6_Pin,
                        (col == 6) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(NCOL7_GPIO_Port, NCOL7_Pin,
                        (col == 7) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(NCOL8_GPIO_Port, NCOL8_Pin,
                        (col == 8) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(NCOL9_GPIO_Port, NCOL9_Pin,
                        (col == 9) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(NCOL10_GPIO_Port, NCOL10_Pin,
                        (col == 10) ? GPIO_PIN_SET : GPIO_PIN_RESET);

      HAL_GPIO_WritePin(NROW1_GPIO_Port, NROW1_Pin,
                        (row == 1) ? GPIO_PIN_RESET : GPIO_PIN_SET);
      HAL_GPIO_WritePin(NROW2_GPIO_Port, NROW2_Pin,
                        (row == 2) ? GPIO_PIN_RESET : GPIO_PIN_SET);
      HAL_GPIO_WritePin(NROW3_GPIO_Port, NROW3_Pin,
                        (row == 3) ? GPIO_PIN_RESET : GPIO_PIN_SET);
      HAL_GPIO_WritePin(NROW4_GPIO_Port, NROW4_Pin,
                        (row == 4) ? GPIO_PIN_RESET : GPIO_PIN_SET);
      HAL_GPIO_WritePin(NROW5_GPIO_Port, NROW5_Pin,
                        (row == 5) ? GPIO_PIN_RESET : GPIO_PIN_SET);
    }

    if (setting.blinkHaveDisplay) {
      // Low 4 bits of display
      HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin,
                        (binCount & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin,
                        (binCount & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin,
                        (binCount & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin,
                        (binCount & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);

      // High 4 bits of display
#if 0  // TODO
      // problems LED 5,6 input only
      HAL_GPIO_WritePin(LED5_GPIO_Port, LED5_Pin,
                        (binCount & 0x10) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LED6_GPIO_Port, LED6_Pin,
                        (binCount & 0x20) ? GPIO_PIN_SET : GPIO_PIN_RESET);
#endif
      HAL_GPIO_WritePin(LED7_GPIO_Port, LED7_Pin,
                        (binCount & 0x40) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LED8_GPIO_Port, LED8_Pin,
                        (binCount & 0x80) ? GPIO_PIN_SET : GPIO_PIN_RESET);

      HAL_GPIO_WritePin(LED9_GPIO_Port, LED9_Pin,
                        (binCount & 0x100) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LED10_GPIO_Port, LED10_Pin,
                        (binCount & 0x200) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }

}
