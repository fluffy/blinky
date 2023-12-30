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

  int curr = metrics.nextIndex;

  metrics.nextIndex++;
  if (  metrics.nextIndex >= metricsHistorySize ) {
    metrics.nextIndex=0;
  }
  
  metrics.localTimeUS[curr] = 
    (int64_t)dataPrev.localAtMonSeconds * 1000000 + capture2uS(dataPrev.monCapture);
  metrics.extTimeUS[curr] =
    (int64_t)dataPrev.extAtMonSeconds * 1000000 + extCapture2uS( dataPrev.monAuxCapture ) + dataPrev.extOffsetUS;
  metrics.syncTimeUS[curr] =
    (int64_t)dataPrev.ltcAtMonSeconds * 1000000 + capture2uS( dataPrev.syncCapture );

  metrics.gpsTimeUS[curr] =
    (int64_t)dataPrev.gpsAtMonSeconds * 1000000 + capture2uS( dataPrev.gpsCapture ); // TODO - wrong fix 

  if (dataPrev.localSeconds % 5 == 2) {
    if (1) {
      snprintf(buffer, sizeof(buffer), "\r\nMetrics\r\n");
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (1) {
      snprintf(buffer, sizeof(buffer), "  LocalTime(s) %5lu.%03ld\r\n",
               (uint32_t)(metrics.localTimeUS[curr] / 1000000),
               (uint32_t)(metrics.localTimeUS[curr] % 1000000) / 1000);
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (haveSync) {
      snprintf(buffer, sizeof(buffer), "   SyncTime(s) %5lu.%03ld\r\n",
               (uint32_t)(metrics.syncTimeUS[curr] / 1000000),
               (uint32_t)(metrics.syncTimeUS[curr] % 1000000) / 1000);
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (haveExt) {
      snprintf(buffer, sizeof(buffer), "    ExtTime(s) %5lu.%03ld \r\n",
               (uint32_t)(metrics.extTimeUS[curr] / 1000000),
               (uint32_t)(metrics.extTimeUS[curr] % 1000000) / 1000);
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (haveGps) {
      snprintf(buffer, sizeof(buffer), "    GpsTime(s) %5lu.%03ld UTC\r\n",
               (uint32_t)(metrics.gpsTimeUS[curr] / 1000000),
               (uint32_t)(metrics.gpsTimeUS[curr] % 1000000) / 1000);
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

      
    if (1) {
      snprintf(buffer, sizeof(buffer), "\r\n" );
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }
        
    if (haveSync) {
      int64_t diff =   metrics.syncTimeUS[curr] - metrics.localTimeUS[curr];
      
      snprintf(buffer, sizeof(buffer), "    sync-local(ms) %4ld.%03ld\r\n",
               (int32_t)(diff / 1000),
               (uint32_t)( abs(diff) % 1000) );
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (haveExt) {
      int64_t diff =   metrics.extTimeUS[curr] - metrics.localTimeUS[curr];
      
      snprintf(buffer, sizeof(buffer), "    ext-lcl(ms) %4ld.%03ld\r\n",
               (int32_t)(diff / 1000),
               (uint32_t)( abs(diff) % 1000) );
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (haveGps) {
      int64_t diff =   metrics.gpsTimeUS[curr] - metrics.localTimeUS[curr];
      
      snprintf(buffer, sizeof(buffer), "    gps-lcl(ms) %4ld.%03ld\r\n",
               (int32_t)(diff / 1000),
               (uint32_t)( abs(diff) % 1000) );
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

       
    if (1) {
      snprintf(buffer, sizeof(buffer), "\r\n");
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    int durationS = 10;
    int prev = curr-durationS;
    if ( prev < 0 ) {
      prev += metricsHistorySize;
    }
    
    int64_t localDelta =  metrics.localTimeUS[curr] -  metrics.localTimeUS[prev];
    int64_t extDelta   =  metrics.extTimeUS[curr]   -  metrics.extTimeUS[prev];
    int64_t gpsDelta   =  metrics.gpsTimeUS[curr]   -  metrics.gpsTimeUS[prev];
    int64_t syncDelta  =  metrics.syncTimeUS[curr]  -  metrics.syncTimeUS[prev];

    int64_t ppbMult = 1000 / durationS;
    int64_t gpsPpb = (gpsDelta-localDelta) * ppbMult;
    int64_t extPpb = (extDelta-localDelta) * ppbMult;
    int64_t syncPpb = (syncDelta-localDelta) * ppbMult;
    int64_t syncExtPpb = (syncDelta-extDelta) * ppbMult;


    if (haveGps) {
      snprintf(buffer, sizeof(buffer), "    gpsDift(ppm) %ld.%03ld \r\n",
               (int32_t)( gpsPpb / 1000),
               (uint32_t)( abs(gpsPpb) % 1000) );
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

       
     if (haveExt) {
      snprintf(buffer, sizeof(buffer), "    extDift(ppm) %ld.%03ld \r\n",
               (int32_t)(extPpb / 1000),
               (uint32_t)( abs(extPpb) % 1000) );
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

     if (haveSync) {
      snprintf(buffer, sizeof(buffer), "    syncDift(ppm) %ld.%03ld \r\n",
               (int32_t)(syncPpb / 1000),
               (uint32_t)( abs(syncPpb) % 1000) );
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

     if (haveSync &&haveExt ) {
       snprintf(buffer, sizeof(buffer), "    syncExtDift(ppm) %ld.%03ld \r\n",
                (int32_t)(syncExtPpb / 1000),
                (uint32_t)( abs(syncExtPpb) % 1000) );
       HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
     }
  
  }
}
