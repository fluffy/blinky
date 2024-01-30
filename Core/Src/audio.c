// SPDX-FileCopyrightText: Copyright (c) 2023 Cullen Jennings
// SPDX-License-Identifier: BSD-2-Clause

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stm32f4xx_ll_tim.h>
#include <string.h>

#include "detect.h"
#include "hardware.h"
#include "main.h"

// dacBuffer has 20 point sin wave center on 1000 with amplitude 500
const int dacBufferLen = 20;
uint32_t dacBuffer[] = {1000, 1155, 1294, 1405, 1476, 1500, 1476,
                        1405, 1294, 1155, 1000, 845,  706,  595,
                        524,  500,  524,  595,  706,  845};
const int adcBufferLen = 20;
uint32_t adcBuffer[20];

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc) {
  detectUpdate(&(adcBuffer[0]), adcBufferLen / 2, false);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
  detectUpdate(&(adcBuffer[adcBufferLen / 2]), adcBufferLen / 2, true);
}

void HAL_DACEx_ConvHalfCpltCallbackCh2(DAC_HandleTypeDef *hdac) {}

void HAL_DACEx_ConvCpltCallbackCh2(DAC_HandleTypeDef *hdac) {}

void HAL_DAC_ErrorCallbackCh1(DAC_HandleTypeDef *hdac) {
  char buffer[100];
  snprintf(buffer, sizeof(buffer), "ERROR hand DAC CH1\r\n");
  HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);

  Error_Handler();
}

void HAL_DAC_ErrorCallbackCh2(DAC_HandleTypeDef *hdac) {
  char buffer[100];
  snprintf(buffer, sizeof(buffer), "ERROR hand DAC CH2\r\n");
  HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);

  Error_Handler();
}

void audioSetup() {
#if 1
  // DMA for Audio Out DAC
  HAL_StatusTypeDef err;
  err = HAL_DAC_Start_DMA(&hDAC, DAC_CHANNEL_2, dacBuffer,
                          dacBufferLen,  //  dacBufferlen is in 32 bit words
                          DAC_ALIGN_12B_R);
  if (err != HAL_OK) {
    char buffer[100];
    snprintf(buffer, sizeof(buffer), "HAL DAC error %d r\n", err);
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    Error_Handler();
  }

  HAL_TIM_Base_Start_IT(&hTimeDAC);
#endif

#if 1
  // DMA for ADC
  HAL_ADC_Start_DMA(&hADC, adcBuffer, adcBufferLen);

  HAL_TIM_Base_Start_IT(&hTimeADC);
#endif
}

void audioStart() {
  HAL_DAC_Start_DMA(&hDAC, DAC_CHANNEL_2, dacBuffer,
                    dacBufferLen,  //  dacBufferlen is in 32 bit words
                    DAC_ALIGN_12B_R);
}

void audioStop() { HAL_DAC_Stop_DMA(&hDAC, DAC_CHANNEL_2); }

void audioInit() { detectInit(adcBufferLen); }
