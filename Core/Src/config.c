// SPDX-FileCopyrightText: Copyright (c) 2023 Cullen Jennings
// SPDX-License-Identifier: BSD-2-Clause

#include "config.h"

#include <stdio.h>
#include <string.h>

#include "hardware.h"
#include "main.h"
#include "setting.h"

Config config;

void configInit() {
  setting.capture2uSRatioM = 125;
  setting.capture2uSRatioN = 256;
}

void configSetup() {
  // access EEProm
  if (1) {
    char buffer[100];
    const uint16_t i2cAddr = 0x50;
    uint32_t timeout = 256;
    uint8_t eepromMemAddr = 0;
    HAL_StatusTypeDef status;

    status = HAL_I2C_IsDeviceReady(&hI2c, i2cAddr << 1, 2, timeout);
    if (status != HAL_OK) {
      snprintf(buffer, sizeof(buffer), "Error: EEProm not found \r\n");
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

#ifdef FORMAT_EEPROM
    const int writeConfigEEProm = 1;
#else
    // TODO move to seperate program
    const int writeConfigEEProm = 0;
#endif
    if (writeConfigEEProm) {
        snprintf(buffer, sizeof(buffer),
                 "\r\n\r\nPROGRAMMING EEProm CONFIG ONLY \r\n" );
        HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);

      // write config to EEProm
      config.version = 2;
      config.product = 1;  // 1=blink, 2=clock
      config.revMajor = 0;
      config.revMinor = 9;
      config.serialNum = 13; // next serial is 13

      config.usePPS = 0;
      config.future13 = 0;
      config.future14 = 0;
      config.future15 = 0;

      // external osc type ( 0=none, 2= 2.048 MHz, 10=10 MHz)
      config.extOscType = 2;
      config.oscAdj = -535; // TODO - this value seems very high , is this a bug with the 10.5 vs 10

      config.vcoValue = 625;
      // For 10Mhz osc, slope is period goes down 1 ns per about 100 VCO goes up

      status = HAL_I2C_Mem_Write(&hI2c, i2cAddr << 1, eepromMemAddr,
                                 sizeof(eepromMemAddr), (uint8_t *)&config,
                                 (uint16_t)sizeof(config), timeout);
      if (status != HAL_OK) {
        // stat: 0=0k, 1 is HAL_ERROR, 2=busy , 3 = timeout
        snprintf(buffer, sizeof(buffer),
                 "EEProm Write Error:  data hal error %d \r\n", status);
        HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
      }

      // chip has 1.5 ms max page write time when it will not respond
      HAL_Delay(2 /*ms */);
      HAL_Delay(20 /*ms */);
    }

    status = HAL_I2C_Mem_Read(&hI2c, i2cAddr << 1, eepromMemAddr,
                              sizeof(eepromMemAddr), (uint8_t *)&config,
                              (uint16_t)sizeof(config), timeout);
    if (status != HAL_OK) {
      // stat: 0=0k, 1 is HAL_ERROR, 2=busy , 3 = timeout
      snprintf(buffer, sizeof(buffer),
               "EEProm Read Error:  data hal error %d \r\n", status);
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if ((config.version < 1) || (config.version > 10)) {
      snprintf(buffer, sizeof(buffer), "EEProm not initalized: %d \r\n",
               config.version);
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);

      Error_Handler();
    }

    if ( config.version == 1 ) {
      // if it is an old config data, fill in the missing data
      config.usePPS=0;
      config.future13=0;
      config.future14=0;
      config.future15=0;
    }

    if ((config.revMajor == 0) && (config.revMinor == 9)) {
      snprintf(buffer, sizeof(buffer), "  Hardware version: EV9 \r\n");
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);

      setClk(config.extOscType, config.vcoValue, config.oscAdj); // TODO - setup power first

      if (config.product == 1) {
        // This is blink board

        // reconfigure ext_clk to be input buton 2
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        GPIO_InitStruct.Pin = AUX_CLK_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_PULLDOWN;
        HAL_GPIO_Init(AUX_CLK_GPIO_Port, &GPIO_InitStruct);
      }

      if (config.product == 2) { // TODO -move this code to ssetting.c
        // This is clock board

        // turn off options
        setting.blinkHaveDisplay = 0;

        // turn off audio and display
        setting.blinkMute = 1;
        setting.blinkBlank = 1;

        // enable 5V power supply on LED1
        // TODO - make this based on if USB has enough power
        HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
      }

    } else {
      snprintf(buffer, sizeof(buffer), "Unknown hardware version %d.%d\r\n",
               config.revMajor, config.revMinor);
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);

      Error_Handler();
    }

    snprintf(buffer, sizeof(buffer), "  Serial: %d\r\n", config.serialNum);
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);

#if 0
    snprintf(buffer, sizeof(buffer), "  Calibration: %u %d\r\n",
             config.vcoValue, config.oscAdj );
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
#endif
  }
}

void setClk(uint8_t extOscTypeType, uint16_t vcoValue, int16_t oscAdj) {
  char buffer[100];

  if (extOscTypeType == 2) {
    // External CLK is 2.048 Mhz
    __HAL_TIM_SET_AUTORELOAD(&hTimeSync, 2048ul * 1000ul - 1ul);
    setting.capture2uSRatioM = 125;
    setting.capture2uSRatioN = 256;

    snprintf(buffer, sizeof(buffer), "  External clock set to 2.048MHz \r\n");
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
  } else if (extOscTypeType == 10) {
    // External CLK is 10 Mhz
    __HAL_TIM_SET_AUTORELOAD(&hTimeSync, 10ul * 1000ul * 1000ul - 1ul);
    setting.capture2uSRatioM = 1;
    setting.capture2uSRatioN = 10;

    snprintf(buffer, sizeof(buffer), "  External clock set to 10MHz \r\n");
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
  } else if (extOscTypeType == 0) {
    // CLK is internal 84 Mhz
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};

    // oscAdj=-534; // -17 to +2,  delta 90 ,  , serial# 6 Nov 26, 2023
    // oscAdj=-542; // 65,  delta 90 ,  , serial# 3 Nov 26, 2023

    htim2.Init.Prescaler = 8 - 1;
    htim2.Init.Period = 10500ul * 1000ul - 1ul + oscAdj;
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    setting.capture2uSRatioM = 2;
    setting.capture2uSRatioN = 21;

    if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK) {
      Error_Handler();
    }
    if (HAL_TIM_IC_Init(&htim2) != HAL_OK) {
      Error_Handler();
    }

    snprintf(buffer, sizeof(buffer), "  Internal clock set to 10.5MHz + %d Hz \r\n", oscAdj);
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
  }

  // vcoValue = 497; // -8 to 10 ns , delta = 30/-100 = -0.30  , serial# 6 Nov
  // 26, 2023

  HAL_DAC_Start(&hDAC, DAC_CH_OSC_ADJ);
  HAL_DAC_SetValue(&hDAC, DAC_CHANNEL_1, DAC_ALIGN_12B_R, vcoValue);

  snprintf(buffer, sizeof(buffer), "  VCO: %u\r\n", vcoValue);
  HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
}
