// Copyright (c) 2023 Cullen Jennings

#include "config.h"

#include <stdio.h>
#include <string.h>

#include "hardware.h"
#include "main.h"

Config config;
uint32_t capture2uSRatioM = 125;
uint32_t capture2uSRatioN = 256;

void configInit() {}

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

#ifdef FORMAT_EEPROM  // TODO move to seperate program
    const int writeConfigEEProm = 1;
#else
    const int writeConfigEEProm = 0;
#endif
    if (writeConfigEEProm) {
      // write config to EEProm
      config.version = 1;
      config.product = 1;  // 1=blink, 2=clock
      config.revMajor = 0;
      config.revMinor = 9;
      config.serialNum = 8;

      config.extOscType = 2;
      config.oscAdj = -658;
      config.vcoValue = 2000;

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

    if ((config.revMajor == 0) && (config.revMinor == 9)) {
      snprintf(buffer, sizeof(buffer), "  Hardware version: EV9 \r\n");
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);

      setClk(config.extOscType, config.vcoValue, config.oscAdj);

      if (config.product == 1) {
        // This is blink board

        // reconfigure ext_clk to be input buton 2
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        GPIO_InitStruct.Pin = AUX_CLK_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_PULLDOWN;
        HAL_GPIO_Init(AUX_CLK_GPIO_Port, &GPIO_InitStruct);
      }

      if (config.product == 2) {
        // This is clock board

        // turn off options
        blinkHaveDisplay = 0;

        // turn off audio and display
        blinkMute = 1;
        blinkBlank = 1;

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
  }
}

void setClk(uint8_t extOscTypeType, uint16_t vcoValue, int16_t oscAdj) {
  char buffer[100];

  if (extOscTypeType == 2) {
    // External CLK is 2.048 Mhz
    __HAL_TIM_SET_AUTORELOAD(&hTimeSync, 2048ul * 1000ul - 1ul);
    capture2uSRatioM = 125;
    capture2uSRatioN = 256;

    snprintf(buffer, sizeof(buffer), "  External clock set to 2.048MHz \r\n");
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
  } else if (extOscTypeType == 10) {
    // External CLK is 10 Mhz
    __HAL_TIM_SET_AUTORELOAD(&hTimeSync, 10ul * 1000ul * 1000ul - 1ul);
    capture2uSRatioM = 1;
    capture2uSRatioN = 10;

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
    capture2uSRatioM = 2;
    capture2uSRatioN = 21;

    if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK) {
      Error_Handler();
    }
    if (HAL_TIM_IC_Init(&htim2) != HAL_OK) {
      Error_Handler();
    }

    snprintf(buffer, sizeof(buffer), "  Internal clock set to 10.5MHz \r\n");
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
  }

  // vcoValue = 497; // -8 to 10 ns , delta = 30/-100 = -0.30  , serial# 6 Nov
  // 26, 2023

  HAL_DAC_Start(&hDAC, DAC_CH_OSC_ADJ);
  HAL_DAC_SetValue(&hDAC, DAC_CHANNEL_1, DAC_ALIGN_12B_R, vcoValue);

  snprintf(buffer, sizeof(buffer), "  VCO: %u\r\n", vcoValue);
  HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
}
