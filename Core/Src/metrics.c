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

extern uint32_t dataNextSyncOutPhaseUS;

void metricsInit() { memset(&metrics, 0, sizeof(metrics)); }

void metricsSetup() {}

void metricsSync(  MetricSyncSource syncTo  ) {

  // metricsSync(  250000l /*phaseUS*/ , 0l /*seconds */ );
  //metricsSync(  capture2uS(data.syncCapture) , data.ltcAtMonSeconds );
  //metricsSync(  capture2uS(data.gpsCapture), data.gpsAtMonSeconds );
  
  uint32_t newPhaseUS= 250000l;
  //uint32_t  newSeconds =0;

  if ( syncTo == gps ) {
    newPhaseUS =   capture2uS(data.gpsCapture) ;
    //newSeconds =  data.gpsAtMonSeconds;
  }

  if ( syncTo == sync ) {
    newPhaseUS =  capture2uS(data.syncCapture) ;
    //newSeconds =  data.ltcAtMonSeconds ;
  }
 

#if 0
  if ( 1) {
    char buffer[100];
    snprintf(buffer, sizeof(buffer),
             "  SYNC:  newSeconds(s)=%ld  \r\n",
             newSeconds );
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
  }
#endif

  // TODO - sometimes gps is right ahead of mon, and somtimes right behind, calc might be wrong
  
  int32_t phaseDeltaUS = newPhaseUS - dataNextSyncOutPhaseUS; // change from old phase
  
  int32_t newMainPhase = (int32_t)capture2uS(data.monCapture) + phaseDeltaUS;
  if ( newMainPhase > 1000000 ) { newMainPhase -= 1000000; } if (  newMainPhase < 0 ) { newMainPhase += 1000000; } 
  dataNextSyncOutPhaseUS = newMainPhase;
  data.localOffsetUS = newMainPhase;
  
  int32_t newExtPhase = (int32_t)extCapture2uS(data.monAuxCapture) + phaseDeltaUS;
  if ( newExtPhase > 1000000 ) { newExtPhase -= 1000000; } if (  newExtPhase < 0 ) { newExtPhase += 1000000; }
  data.extOffsetUS = newExtPhase;

  
#if 0
  if ( 1) {
    char buffer[100];
    snprintf(buffer, sizeof(buffer),
             "  SYNC: oldMon(ms)=%lu, newMon(ms)=%ld, oldAuxMon(ms)=%lu, newAuxMon(ms)=%ld\r\n",
             capture2uS(data.monCapture) / 1000l, newMainPhase/1000l, 
             extCapture2uS(data.monAuxCapture) / 1000l,  newExtPhase / 1000l);
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
  }
#endif
    

  metricsAdjust();
  int curr = metrics.nextIndex;

  if ( syncTo == gps ) {
    int64_t delta = (  metrics.gpsTimeUS[curr] -  metrics.localTimeUS[curr] ) / 1000000ll ;
    data.localSeconds += delta;
  }

   if ( syncTo == sync ) {
    int64_t delta = (  metrics.syncTimeUS[curr] -  metrics.localTimeUS[curr] ) / 1000000ll ;
    data.localSeconds += delta;
  }

   if ( syncTo == external ) {
    int64_t delta = (  metrics.extTimeUS[curr] -  metrics.localTimeUS[curr] ) / 1000000ll ;
    data.localSeconds += delta;
  }

   if ( syncTo == none ) {
    data.localSeconds = 0 ;
    data.extSeconds = 0; 
  }
  
#if 0
  if ( 1) {
    char buffer[100];
    int64_t gpsSec =   metrics.gpsTimeUS[curr]/1000000l ;
    int64_t locSec =   metrics.localTimeUS[curr]/1000000l ;
    
    snprintf(buffer, sizeof(buffer),
             "  SYNC: gps(s)=%ld, local(s)=%ld  delta=%ld \r\n",
             (int32_t)gpsSec,  (int32_t)locSec,  (int32_t)gpsDelta
             );
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
  }
#endif


}

void metricsAdjust(){

  // TODO - move to doing this on high priority interupt to minimize
  // the odds odd of something updating data during the copy
  memcpy(&dataPrev, &data, sizeof(dataPrev));

  metrics.haveSync = false;
  metrics.haveExt = false;
  metrics.haveGps = false;

  uint32_t tick = HAL_GetTick();
  if ( ( tick > 2000) && ( dataPrev.syncCaptureTick+1500 > tick) ) {
    metrics.haveSync = true;
  }
  if ( ( tick > 2000) && ( dataPrev.gpsCaptureTick+1500 > tick) ) {
    metrics.haveGps = true;
  }
 if ( ( tick > 2000) && ( dataPrev.extSecondsTick+1500 > tick) ) {
    metrics.haveExt = true;
  }

  if (  dataPrev.gpsAtMonSecondsTick < dataPrev.gpsCaptureTick ) {
    // this GPS capture was from the second before the current pulse, correct for this
    dataPrev.gpsAtMonSeconds += 1;
    dataPrev.gpsAtMonSecondsTick += 1000;
    if (  dataPrev.gpsAtMonSecondsTick < dataPrev.gpsCaptureTick ) {
      // this GPS capture was from way before the second before the current pulse, correct for this
      dataPrev.gpsAtMonSeconds += 1;
      dataPrev.gpsAtMonSecondsTick += 1000; 
    }
  }
  
  if (  dataPrev.ltcAtMonSecondsTick < dataPrev.syncCaptureTick ) {
    // this LTC capture was from the second before the current pulse, correct for this
    dataPrev.ltcAtMonSeconds += 1;
    dataPrev.ltcAtMonSecondsTick += 1000;
    if (  dataPrev.ltcAtMonSecondsTick < dataPrev.syncCaptureTick ) {
      // this LTC capture was from the second before the current pulse, correct for this
      dataPrev.ltcAtMonSeconds += 1;
      dataPrev.ltcAtMonSecondsTick += 1000; 
    }
  }

  int curr = metrics.nextIndex;
  
  metrics.localTimeUS[curr] = (int64_t)dataPrev.localAtMonSeconds * 1000000 +
    capture2uS(dataPrev.monCapture) -
    dataPrev.localOffsetUS;
  
  metrics.extTimeUS[curr] = (int64_t)dataPrev.extAtMonSeconds * 1000000 +
    extCapture2uS(dataPrev.monAuxCapture) -
    dataPrev.extOffsetUS;
  
  int64_t deltaSync =  (int64_t)capture2uS(dataPrev.syncCapture) -  capture2uS(dataPrev.monCapture);
  metrics.syncTimeUS[curr] = (int64_t)dataPrev.ltcAtMonSeconds * 1000000 + deltaSync;

  int64_t deltaGps =  (int64_t)capture2uS(dataPrev.gpsCapture) -  capture2uS(dataPrev.monCapture);
  metrics.gpsTimeUS[curr] = (int64_t)dataPrev.gpsAtMonSeconds * 1000000 + deltaGps ;
}


void metricsRun() {
  char buffer[100];
  if (data.localSeconds == dataPrev.localSeconds) {
    return;
  }

  metricsAdjust();

  int curr = metrics.nextIndex;
  metrics.nextIndex++;
  if (metrics.nextIndex >= metricsHistorySize) {
    metrics.nextIndex = 0;
  }


#if 0
  if (1) {
  int32_t tickDelta = (int32_t) dataPrev.gpsAtMonSecondsTick -  dataPrev.gpsCaptureTick ;
  snprintf(buffer, sizeof(buffer), " \r\nDBG tickDelta(s)=%ld gpsAtMonSecondsTick=%lu gpsCaptureTick=%lu \r\n",
             tickDelta ,
             dataPrev.gpsAtMonSecondsTick, dataPrev.gpsCaptureTick
             );
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
  }
#endif
  
#if 0
  if (1) {
    snprintf(buffer, sizeof(buffer), "DBG gpsSeconds=%lu gpsAtMonSeconds=%lu \r\n",
             dataPrev.gpsSeconds, dataPrev.gpsAtMonSeconds  );
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
  }
#endif
  
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


    if (metrics.haveGps) {
      snprintf(buffer, sizeof(buffer), "    GpsTime(s) %7lu.%03ld,%03ld \r\n",
               (int32_t)(metrics.gpsTimeUS[curr] / 1000000),
               (int32_t)(metrics.gpsTimeUS[curr] % 1000000) / 1000,
               (int32_t)(metrics.gpsTimeUS[curr] % 1000) );
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (metrics.haveSync) {
      snprintf(buffer, sizeof(buffer), "    SyncTime(s) %6lu.%03ld,%03ld \r\n",
               (int32_t)(metrics.syncTimeUS[curr] / 1000000),
               (int32_t)(metrics.syncTimeUS[curr] % 1000000) / 1000,
                (int32_t)(metrics.syncTimeUS[curr] % 1000) );
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (metrics.haveExt) {
      snprintf(buffer, sizeof(buffer), "    ExtTime(s) %7lu.%03ld,%03ld \r\n",
               (int32_t)(metrics.extTimeUS[curr] / 1000000),
               (int32_t)(metrics.extTimeUS[curr] % 1000000) / 1000,
                (int32_t)(metrics.extTimeUS[curr] % 1000) );
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (1) {
      snprintf(buffer, sizeof(buffer), "\r\n");
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (metrics.haveSync) {
      int64_t diff = metrics.syncTimeUS[curr] - metrics.localTimeUS[curr];

      snprintf(buffer, sizeof(buffer), "    syn-lcl(ms) %6ld.%03ld\r\n",
               (int32_t)(diff / 1000), (uint32_t)(abs(diff) % 1000));
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (metrics.haveExt) {
      int64_t diff = metrics.extTimeUS[curr] - metrics.localTimeUS[curr];

      snprintf(buffer, sizeof(buffer), "    ext-lcl(ms) %6ld.%03ld\r\n",
               (int32_t)(diff / 1000), (uint32_t)(abs(diff) % 1000));
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (metrics.haveGps) {
      int64_t diff = metrics.gpsTimeUS[curr] - metrics.localTimeUS[curr];

      snprintf(buffer, sizeof(buffer), "    gps-lcl(ms) %6ld.%03ld\r\n",
               (int32_t)(diff / 1000), (uint32_t)(abs(diff) % 1000));
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (metrics.haveGps && metrics.haveExt) {
      int64_t diff = metrics.gpsTimeUS[curr] - metrics.extTimeUS[curr];

      snprintf(buffer, sizeof(buffer), "    gps-ext(ms) %6ld.%03ld\r\n",
               (int32_t)(diff / 1000), (uint32_t)(abs(diff) % 1000));
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (1) {
      snprintf(buffer, sizeof(buffer), "\r\n");
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    int durationS = 10; // TODO // make sure metricsHistorySize is greater than this 
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

    if (metrics.haveGps) {
      snprintf(buffer, sizeof(buffer), "    gpsLclDrift(ppm) %6ld.%03ld \r\n",
               (int32_t)(gpsPpb / 1000), (uint32_t)(abs(gpsPpb) % 1000));
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (metrics.haveExt) {
      snprintf(buffer, sizeof(buffer), "    extLclDrift(ppm) %6ld.%03ld \r\n",
               (int32_t)(extPpb / 1000), (uint32_t)(abs(extPpb) % 1000));
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (metrics.haveSync) {
      snprintf(buffer, sizeof(buffer), "    synLclDrift(ppm) %6ld.%03ld \r\n",
               (int32_t)(syncPpb / 1000), (uint32_t)(abs(syncPpb) % 1000));
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (metrics.haveSync && metrics.haveExt) {
      snprintf(buffer, sizeof(buffer), "    synExtDrift(ppm) %6ld.%03ld \r\n",
               (int32_t)(syncExtPpb / 1000),
               (uint32_t)(abs(syncExtPpb) % 1000));
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (metrics.haveGps && metrics.haveExt) {
      snprintf(buffer, sizeof(buffer), "    extGpsDrift(ppm) %6ld.%03ld \r\n",
               (int32_t)(extGpsPpb / 1000), (uint32_t)(abs(extGpsPpb) % 1000));
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (1) {
      snprintf(buffer, sizeof(buffer), "\r\n");
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }
  }
}
