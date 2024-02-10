// SPDX-FileCopyrightText: Copyright (c) 2024 Cullen Jennings
// SPDX-License-Identifier: BSD-2-Clause

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stm32f4xx_ll_tim.h>

#include "config.h"
#include "hardware.h"
#include "main.h"
#include "measurement.h"
#include "metrics.h"
#include "setting.h"
#include "status.h"

void buttonsInit(){
  // nothing needed
}

void buttonsSetup() {

  if ( config.product != 1 ) {
    return;
  }

    GPIO_InitTypeDef GPIO_InitStruct = {0};

  GPIO_InitStruct.Pin = BTN3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init( BTN3_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = BTN2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init( BTN2_GPIO_Port, &GPIO_InitStruct);
}

void buttonsRun() {
  static char button1WasPressed = 0;
  static char button2WasPressed = 0;
  static char button3WasPressed = 0;

  char buffer[100];

  uint32_t tick = HAL_GetTick();

  if (config.product == 1) {
    if (!HAL_GPIO_ReadPin(AUX_CLK_GPIO_Port, GPS_PPS_Pin)) {
      if (!button2WasPressed) {
        setting.blinkMute =
            (setting.blinkMute) ? 0 : 1;

        snprintf(buffer, sizeof(buffer), "Mute button pressed. Mute=%d \r\n",
                 (int)setting.blinkMute);
        HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
      }
      button2WasPressed = 1;
    } else {
      button2WasPressed = 0;
    }
  }

  if (HAL_GPIO_ReadPin(BOOT1_GPIO_Port, BOOT1_Pin)) {
    if (!button3WasPressed) {
      if (setting.blinkBlank) {
        setting.blinkDispAudio = 1;
        setting.blinkBlank = 0;
      } else if (setting.blinkDispAudio) {
        setting.blinkDispAudio = 0;
        setting.blinkBlank = 0;
      } else {
        setting.blinkDispAudio = 0;
        setting.blinkBlank = 1;
      }

      snprintf(buffer, sizeof(buffer),
               "Display button pressed. Blank=%d dispAudio=%d \r\n",
               (int)setting.blinkBlank, (int)setting.blinkDispAudio);
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }
    button3WasPressed = 1;
  } else {
    button3WasPressed = 0;
  }

  if ((!HAL_GPIO_ReadPin(BTN1_GPIO_Port, BTN1_Pin))) {
    if (!button1WasPressed) {
      snprintf(buffer, sizeof(buffer), "Sync button pressed\r\n");
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);

      if ((tick > 2000) && (data.gpsCaptureTick + 2000 > tick)) {
        //  had GPS sync in last 2 seconds
        metricsSync(SourceGPS);
        updateStatus(StatusSync);
      } else if ((tick > 2000) && (data.syncCaptureTick + 2000 > tick)) {
        //  had sync in last 2 seconds
        metricsSync(SourceSync);
        updateStatus(StatusSync);
      } else if ((tick > 2000) && (data.extSecondsTick + 2000 > tick)) {
        //  had ext in last 2 seconds
        metricsSync(SourceExternal);
        updateStatus(StatusLostSync);
      } else {
        metricsSync(SourceNone);
        updateStatus(StatusLostSync);
#if 0
        snprintf(buffer, sizeof(buffer),
                 "ERROR: No gps or sync input on button press\r\n");
        HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
#endif
      }
    }
    button1WasPressed = 1;
  } else {
    button1WasPressed = 0;
  }
}
