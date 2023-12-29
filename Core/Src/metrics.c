// Copyright (c) 2023 Cullen Jennings

#include "metrics.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h> // for abs

#include "hardware.h"
#include "measurement.h"
#include "config.h"

Metrics metrics;
static Measurements dataPrev;

void metricsInit() { memset(&metrics, 0, sizeof(metrics)); }

void metricsSetup() {}

void metricsRun() {
  char buffer[100];
  if (data.localSeconds == dataPrev.localSeconds) {
    return;
  }

  bool haveSync=false;
  bool haveExt=false;
  bool haveGps=false;

  if ( data.ltcAtMonSeconds != dataPrev.ltcAtMonSeconds ) {
    haveSync=true;
  }
  if ( data.extAtMonSeconds != dataPrev.extAtMonSeconds ) {
    haveExt=true;
  }
  if ( data.gpsAtMonSeconds != dataPrev.gpsAtMonSeconds ) {
    haveGps=true;
  }
  
     
  // TODO - move to doing this on high priority interupt to minimize
  // the odds odd of something updating data during the copy
  memcpy(&dataPrev, &data, sizeof(dataPrev));

  metrics.localTimeUS = 
    (int64_t)dataPrev.localAtMonSeconds * 1000000 + capture2uS(dataPrev.monCapture);
  metrics.extTimeUS =
    (int64_t)dataPrev.extAtMonSeconds * 1000000 + extCapture2uS( dataPrev.monAuxCapture ) + dataPrev.extOffsetUS;
  metrics.syncTimeUS =
    (int64_t)dataPrev.ltcAtMonSeconds * 1000000 + capture2uS( dataPrev.syncCapture );

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

    if (haveSync) {
      snprintf(buffer, sizeof(buffer), "   SyncTime(s) %5lu.%03ld\r\n",
               (uint32_t)(metrics.syncTimeUS / 1000000),
               (uint32_t)(metrics.syncTimeUS % 1000000) / 1000);
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (haveExt) {
      snprintf(buffer, sizeof(buffer), "    ExtTime(s) %5lu.%03ld \r\n",
               (uint32_t)(metrics.extTimeUS / 1000000),
               (uint32_t)(metrics.extTimeUS % 1000000) / 1000);
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (1) {
      snprintf(buffer, sizeof(buffer), "\r\n" );
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }
        
    if (haveSync) {
      int64_t diff =   metrics.syncTimeUS - metrics.localTimeUS;
      
      snprintf(buffer, sizeof(buffer), "    sync-local(ms) %4ld.%03ld\r\n",
               (int32_t)(diff / 1000),
               (uint32_t)( abs(diff) % 1000) );
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (haveExt) {
      int64_t diff =   metrics.extTimeUS - metrics.localTimeUS;
      
      snprintf(buffer, sizeof(buffer), "    ext-lcl(ms) %4ld.%03ld\r\n",
               (int32_t)(diff / 1000),
               (uint32_t)( abs(diff) % 1000) );
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (1) {
      snprintf(buffer, sizeof(buffer), "\r\n");
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

  }

  
}
