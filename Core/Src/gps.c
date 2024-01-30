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
#include "measurement.h"

const int gpsBufferLen = 20;
uint8_t gpsBuffer[20];
uint8_t gpsBufLen = 0;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  if (huart == &hUartGps) {
    if (gpsBufLen == 1) {
      if (gpsBuffer[0] != '$') {
        gpsBufLen = 0;  // keep looking for start of line
      }
    }
    if (gpsBufLen > 1) {
      // found a line
      gpsBuffer[gpsBufLen] = 0;

#if 0
      if (1) {
      char buffer[100];
      snprintf(buffer, sizeof(buffer), "     GPS: %s \r\n", gpsBuffer );
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
      }
#endif
      const char *find = "GPRMC";
      if (memcmp((char *)gpsBuffer + 1, find, strlen(find)) == 0) {
        // found "$GPRMC
        if (gpsBuffer[14] == 'A') {
          char gpsTime[7];  // This will have ASCII chars 123456 to indicate
                            // time is 12:34:56 UTC and be null terminated

          // GPS daa is valid
          strncpy(gpsTime, (char *)gpsBuffer + 7, sizeof(gpsTime) - 1);
          gpsTime[sizeof(gpsTime) - 1] = 0;  // terminate time string

          int hrHigh = gpsTime[0] - '0';
          int hrLow = gpsTime[1] - '0';
          int minHigh = gpsTime[2] - '0';
          int minLow = gpsTime[3] - '0';
          int secHigh = gpsTime[4] - '0';
          int secLow = gpsTime[5] - '0';

          int hr = hrHigh * 10 + hrLow;
          int min = minHigh * 10 + minLow;
          int sec = secHigh * 10 + secLow;

          if (1) {
            uint32_t tick = HAL_GetTick();
            data.gpsSeconds = hr * 3600 + min * 60 + sec;
            data.gpsSecondsTick = tick;

#if 0
          if (1) {
            char buffer[100];
            //snprintf(buffer, sizeof(buffer), "DBG  hr=%d min=%d sec=%d \r\n", hr,min,sec );
            //HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
            
            //snprintf(buffer, sizeof(buffer), "DBG   gps time UTC: %s \r\n", gpsTime );
            //HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
            snprintf(buffer, sizeof(buffer), "DBG2  gps(s): %lu UTC\r\n",  data.gpsSeconds );
            HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
          }
#endif
          }
        }
      }

      gpsBufLen = 0;
    }

    if (gpsBufLen == 0) {
      // look for $ start of line
      gpsBufLen = 1;
      HAL_StatusTypeDef stat =
          HAL_UART_Receive_IT(&hUartGps, gpsBuffer, 1 /* size */);
      if (stat != HAL_UART_ERROR_NONE) {
        Error_Handler();
      }
    } else {
      gpsBufLen =
          16;  // Long enough to get the part of NMEA $GPRMC line with timestamp
      HAL_StatusTypeDef stat = HAL_UART_Receive_IT(&hUartGps, gpsBuffer + 1,
                                                   gpsBufLen - 1 /* size */);
      if (stat != HAL_UART_ERROR_NONE) {
        Error_Handler();
      }
    }
  }
}

void gpsInit() {}

void gpsSetup() {
#if 1
  // start receving for GPS serial
  HAL_StatusTypeDef stat =
      HAL_UART_Receive_IT(&hUartGps, gpsBuffer, 1 /* size */);
  if (stat != HAL_UART_ERROR_NONE) {
    Error_Handler();
  }
#endif
}
