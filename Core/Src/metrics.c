// Copyright (c) 2023 Cullen Jennings

#include "metrics.h"

#include <stdio.h>
#include <stdlib.h>  // for abs
#include <string.h>

#include "config.h"
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

  bool haveSync = false;
  bool haveExt = false;
  bool haveGps = false;

  // TODO - move to doing this on high priority interupt to minimize
  // the odds odd of something updating data during the copy
  memcpy(&dataPrev, &data, sizeof(dataPrev));
  
  
  uint32_t tick = HAL_GetTick();
  if ( ( tick > 2000) && ( dataPrev.syncCaptureTick+1500 > tick) ) {
    haveSync = true;
  }
  if ( ( tick > 2000) && ( dataPrev.gpsCaptureTick+1500 > tick) ) {
    haveGps = true;
  }
 if ( ( tick > 2000) && ( dataPrev.extSecondsTick+1500 > tick) ) {
    haveExt = true;
  }

  int curr = metrics.nextIndex;

  metrics.nextIndex++;
  if (metrics.nextIndex >= metricsHistorySize) {
    metrics.nextIndex = 0;
  }

  metrics.localTimeUS[curr] = (int64_t)dataPrev.localAtMonSeconds * 1000000 +
                              capture2uS(dataPrev.monCapture);
  metrics.extTimeUS[curr] = (int64_t)dataPrev.extAtMonSeconds * 1000000 +
                            extCapture2uS(dataPrev.monAuxCapture) +
                            dataPrev.extOffsetUS;
  metrics.syncTimeUS[curr] = (int64_t)dataPrev.ltcAtMonSeconds * 1000000 +
                             capture2uS(dataPrev.syncCapture);

  metrics.gpsTimeUS[curr] =
      (int64_t)dataPrev.gpsAtMonSeconds * 1000000 +
      capture2uS(dataPrev.gpsCapture) -  capture2uS(dataPrev.monCapture); ;  // TODO - wrong fix

  if (dataPrev.localSeconds % 5 == 2) {
    if (1) {
      snprintf(buffer, sizeof(buffer), "\r\nMetrics\r\n");
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (1) {
      snprintf(buffer, sizeof(buffer), "    LocalTime(s) %5lu.%03ld,%03ld \r\n",
               (int32_t)(metrics.localTimeUS[curr] / 1000000),
               (int32_t)(metrics.localTimeUS[curr] % 1000000) / 1000,
               (int32_t)(metrics.localTimeUS[curr] % 1000));
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (haveSync) {
      snprintf(buffer, sizeof(buffer), "    SyncTime(s) %6lu.%03ld,%03ld \r\n",
               (int32_t)(metrics.syncTimeUS[curr] / 1000000),
               (int32_t)(metrics.syncTimeUS[curr] % 1000000) / 1000,
                (int32_t)(metrics.syncTimeUS[curr] % 1000) );
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (haveExt) {
      snprintf(buffer, sizeof(buffer), "    ExtTime(s) %7lu.%03ld,%03ld \r\n",
               (int32_t)(metrics.extTimeUS[curr] / 1000000),
               (int32_t)(metrics.extTimeUS[curr] % 1000000) / 1000,
                (int32_t)(metrics.extTimeUS[curr] % 1000) );
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (haveGps) {
      snprintf(buffer, sizeof(buffer), "    GpsTime(s) %7lu.%03ld,%03ld \r\n",
               (int32_t)(metrics.gpsTimeUS[curr] / 1000000),
               (int32_t)(metrics.gpsTimeUS[curr] % 1000000) / 1000,
               (int32_t)(metrics.gpsTimeUS[curr] % 1000) );
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (1) {
      snprintf(buffer, sizeof(buffer), "\r\n");
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (haveSync) {
      int64_t diff = metrics.syncTimeUS[curr] - metrics.localTimeUS[curr];

      snprintf(buffer, sizeof(buffer), "    syn-lcl(ms) %6ld.%03ld\r\n",
               (int32_t)(diff / 1000), (uint32_t)(abs(diff) % 1000));
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (haveExt) {
      int64_t diff = metrics.extTimeUS[curr] - metrics.localTimeUS[curr];

      snprintf(buffer, sizeof(buffer), "    ext-lcl(ms) %6ld.%03ld\r\n",
               (int32_t)(diff / 1000), (uint32_t)(abs(diff) % 1000));
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (haveGps) {
      int64_t diff = metrics.gpsTimeUS[curr] - metrics.localTimeUS[curr];

      snprintf(buffer, sizeof(buffer), "    gps-lcl(ms) %6ld.%03ld\r\n",
               (int32_t)(diff / 1000), (uint32_t)(abs(diff) % 1000));
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (haveGps && haveExt) {
      int64_t diff = metrics.gpsTimeUS[curr] - metrics.extTimeUS[curr];

      snprintf(buffer, sizeof(buffer), "    gps-ext(ms) %6ld.%03ld\r\n",
               (int32_t)(diff / 1000), (uint32_t)(abs(diff) % 1000));
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (1) {
      snprintf(buffer, sizeof(buffer), "\r\n");
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    int durationS = 10;
    int prev = curr - durationS;
    if (prev < 0) {
      prev += metricsHistorySize;
    }

    int64_t localDelta = metrics.localTimeUS[curr] - metrics.localTimeUS[prev];
    int64_t extDelta = metrics.extTimeUS[curr] - metrics.extTimeUS[prev];
    int64_t gpsDelta = metrics.gpsTimeUS[curr] - metrics.gpsTimeUS[prev];
    int64_t syncDelta = metrics.syncTimeUS[curr] - metrics.syncTimeUS[prev];

    int64_t ppbMult = 1000 / durationS;
    int64_t gpsPpb = (gpsDelta - localDelta) * ppbMult;
    int64_t extPpb = (extDelta - localDelta) * ppbMult;
    int64_t syncPpb = (syncDelta - localDelta) * ppbMult;
    int64_t syncExtPpb = (syncDelta - extDelta) * ppbMult;
    int64_t extGpsPpb = (extDelta - gpsDelta) * ppbMult;

    if (haveGps) {
      snprintf(buffer, sizeof(buffer), "    gpsLclDift(ppm) %6ld.%03ld \r\n",
               (int32_t)(gpsPpb / 1000), (uint32_t)(abs(gpsPpb) % 1000));
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (haveExt) {
      snprintf(buffer, sizeof(buffer), "    extLclDift(ppm) %6ld.%03ld \r\n",
               (int32_t)(extPpb / 1000), (uint32_t)(abs(extPpb) % 1000));
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (haveSync) {
      snprintf(buffer, sizeof(buffer), "    synLclDift(ppm) %6ld.%03ld \r\n",
               (int32_t)(syncPpb / 1000), (uint32_t)(abs(syncPpb) % 1000));
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (haveSync && haveExt) {
      snprintf(buffer, sizeof(buffer), "    synExtDift(ppm) %6ld.%03ld \r\n",
               (int32_t)(syncExtPpb / 1000),
               (uint32_t)(abs(syncExtPpb) % 1000));
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (haveGps && haveExt) {
      snprintf(buffer, sizeof(buffer), "    extGpsDift(ppm) %6ld.%03ld \r\n",
               (int32_t)(extGpsPpb / 1000), (uint32_t)(abs(extGpsPpb) % 1000));
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (1) {
      snprintf(buffer, sizeof(buffer), "\r\n");
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }
  }
}
