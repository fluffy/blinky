// SPDX-FileCopyrightText: Copyright (c) 2024 Cullen Jennings
// SPDX-License-Identifier: BSD-2-Clause

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "hardware.h"
#include "ltc.h"
#include "hardware.h"
#include "power.h"
#include "config.h"


void powerInit(){
  // nothing to do
}

void powerSetup(){
  // TODO

  if ( config.product == 2) {
  // TODO need to check both channel 7 and 10 for two different sides of connector
  HAL_ADC_Start(&hadc1);
  HAL_ADC_PollForConversion(&hadc1, 1000 /*timeout ms*/);
  uint16_t val = HAL_ADC_GetValue(&hadc1);
  int power=0; // in mA
  if ( ( val > 250) & ( val <= 750 ) ) {
    power = 500;
  }
  if ( ( val > 750) & ( val <= 1500 ) ) {
    power = 1500;
  }
  if ( ( val > 1500) & ( val <= 2500 ) ) {
    power = 3000;
  }
  if (1) {
    char buffer[100];
    snprintf(buffer, sizeof(buffer), "  Power supply1: %d.%dA (value=%u)\r\n", power/1000, ( power/100) % 10 ,  val );
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
  }
  // seeing value of 0 if USB connector is fliped wrong way
  // value of 543 = on usb hub = 0.4375 V
  // value of 1138 , 1157 on mac front port = 0.916 V
  // value of 501 = on  usb expander bar

  // the far end has 10k, 22K, 56K resisotr to 5V for 3, 1.5, 0.5 A respectively
  // this end has 5.1K resitor to ground
  // range of ADC is 4096 for 3.3 V
  // < 250 , wrong side CC
  // <  750 , have 0.5 A
  // < 1500 , have 1.5 A
  // < 2500 , have 3 A

   HAL_ADC_Start(&hadc2);
  HAL_ADC_PollForConversion(&hadc2, 1000 /*timeout ms*/);
  val = HAL_ADC_GetValue(&hadc2);
  power=0; // in mA
  if ( ( val > 250) & ( val <= 750 ) ) {
    power = 500;
  }
  if ( ( val > 750) & ( val <= 1500 ) ) {
    power = 1500;
  }
  if ( ( val > 1500) & ( val <= 2500 ) ) {
    power = 3000;
  }
  if (1) {
    char buffer[100];
    snprintf(buffer, sizeof(buffer), "  Power supply2: %d.%dA (value=%u)\r\n", power/1000, ( power/100) % 10 ,  val );
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
  }

  // TODO - if power, set status LED and error
  // TODO - if power good, enable 5V output
  }

}
