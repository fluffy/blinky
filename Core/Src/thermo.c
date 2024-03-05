// SPDX-FileCopyrightText: Copyright (c) 2023 Cullen Jennings
// SPDX-License-Identifier: BSD-2-Clause

#include "thermo.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "hardware.h"

void thermoInit() {}

void thermoSetup() {
  char buffer[100];
  const uint16_t i2cAddr = 0x48;
  uint32_t timeout = 256;
  uint8_t regAddr = 0x1;
  uint16_t reg = 0;
  HAL_StatusTypeDef status;

  if (config.product == 3) {
    return;
  }

  status = HAL_I2C_IsDeviceReady(&hI2c, i2cAddr << 1, 2, timeout);
  if (status != HAL_OK) {
    snprintf(buffer, sizeof(buffer),
             "Error: Temperature sensor not found \r\n");
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
  }

  reg = 0xA060;    // power up default
  reg &= ~0x1000;  // turn off extended mode (  only needed for temps above
                   // 128 C )
  uint16_t convRate = 1;                    // 0=0.25Hz, 1=1Hz, 2=4Hz, 3=8Hz
  reg |= (reg & 0x3FFF) | (convRate << 6);  // set coversion rate

  status = HAL_I2C_Mem_Write(&hI2c, i2cAddr << 1, regAddr, sizeof(regAddr),
                             (uint8_t *)&reg, (uint16_t)sizeof(reg), timeout);
  if (status != HAL_OK) {
    // stat: 0=0k, 1 is HAL_ERROR, 2=busy , 3 = timeout
    snprintf(buffer, sizeof(buffer),
             "Temperature Write Error:  data hal error %d \r\n", status);
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
  }

  HAL_Delay(1100 /*ms */);  // TODO remove
}

int16_t thermoGetTemperatureDeciC() {
  char buffer[100];
  const uint16_t i2cAddr = 0x48;
  uint32_t timeout = 256;
  uint8_t tempAddr = 0x0;
  uint16_t temp = 0;
  HAL_StatusTypeDef status;

  if (config.product == 3) {
    return 0;
  }

  status = HAL_I2C_Mem_Read(&hI2c, i2cAddr << 1, tempAddr, sizeof(tempAddr),
                            (uint8_t *)&temp, (uint16_t)sizeof(temp), timeout);
  if (status != HAL_OK) {
    // stat: 0=0k, 1 is HAL_ERROR, 2=busy , 3 = timeout
    snprintf(buffer, sizeof(buffer),
             "Temperature Read Error:  data hal error %d \r\n", status);
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);

    return 0x7FFF;
  }

  temp = ((temp & 0x00FF) << 8) | ((temp & 0xFF00) >> 8);  // swap bytes
  temp = temp >> 4;  // shift to be 12 bits

  float tempC;

  if (temp & 0x800) {
    // negative temp
    float count = temp & 0x7FF;  // bottom 11 bits
    tempC = -0.0625 * count;
  } else {
    // positive temp
    float count = temp & 0x7FF;  // bottom 11 bits
    tempC = 0.0625 * count;
  }

  int16_t tempDeciC = round(tempC * 10.0);

#if 0  // TODO
      snprintf(buffer, sizeof(buffer), "  Temperature: %d.%dC \r\n",
               tempDeciC / 10, tempDeciC % 10);
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
#endif

  return tempDeciC;
}
