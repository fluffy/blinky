// Copyright (c) 2023 Cullen Jennings

#include "metrics.h"

#include <stdio.h>
#include <string.h>

#include "hardware.h"
#include "measurement.h"

Metrics metrics;
static Measurements dataPrev;

void metricsInit() { memset(&metrics, 0, sizeof(metrics)); }

void metricsSetup() {}

void metricsRun() {
  char buffer[100];
  if (data.localSeconds == dataPrev.localSeconds) {
    return;
  }

  // TODO - move to doing this on high priority interupt to minimize
  // the odds odd of something updating data during the copy
  memcpy(&dataPrev, &data, sizeof(dataPrev));

  metrics.localTimeUS =
      (uint64_t)dataPrev.localSeconds * 1000000 + dataPrev.monCapture;
  metrics.extTimeUS =
      (uint64_t)dataPrev.extSeconds * 1000000 + dataPrev.monAuxCapture;
  metrics.syncTimeUS =
      (uint64_t)dataPrev.ltcSeconds * 1000000 + dataPrev.syncCapture;

  if (dataPrev.localSeconds % 5 == 2) {
    if (1) {
      snprintf(buffer, sizeof(buffer), "\r\nMetrics\r\n");
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (1) {
      snprintf(buffer, sizeof(buffer), "  LocalTime(s) %5lu.%03ld\r\n",
               (uint32_t)(metrics.localTimeUS / 1000000),
               (uint32_t)(metrics.localTimeUS % 1000000) / 1000);
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (1) {
      snprintf(buffer, sizeof(buffer), "    ExtTime(s) %5lu.%03ld\r\n",
               (uint32_t)(metrics.extTimeUS / 1000000),
               (uint32_t)(metrics.extTimeUS % 1000000) / 1000);
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (1) {
      snprintf(buffer, sizeof(buffer), "   SyncTime(s) %5lu.%03ld\r\n",
               (uint32_t)(metrics.syncTimeUS / 1000000),
               (uint32_t)(metrics.syncTimeUS % 1000000) / 1000);
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }
  }
}
