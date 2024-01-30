// SPDX-FileCopyrightText: Copyright (c) 2023 Cullen Jennings
// SPDX-License-Identifier: BSD-2-Clause


#include "metrics.h"

#include <stdio.h>
#include <string.h>

#include "config.h"
#include "hardware.h"
#include "measurement.h"
#include "status.h"

Metrics metrics;
static Measurements dataPrev;

extern uint32_t dataNextSyncOutPhaseUS;

inline uint32_t us2ms(  int64_t v ) {   uint64_t ms=v/1000l; uint64_t u=ms; uint32_t s=u; return s; };

void metricsInit() { memset(&metrics, 0, sizeof(metrics)); }

void metricsSetup() {}

void metricsSync(MetricSyncSource syncTo) {
  // metricsSync(  250000l /*phaseUS*/ , 0l /*seconds */ );
  // metricsSync(  capture2uS(data.syncCapture) , data.ltcAtMonSeconds );
  // metricsSync(  capture2uS(data.gpsCapture), data.gpsAtMonSeconds );

  uint32_t newPhaseUS = 250000l;
  // uint32_t  newSeconds =0;

  if (syncTo == SourceGPS) {
    newPhaseUS = capture2uS(data.gpsCapture);
    // newSeconds =  data.gpsAtMonSeconds;
  }

  if (syncTo == SourceSync) {
    newPhaseUS = capture2uS(data.syncCapture);
    // newSeconds =  data.ltcAtMonSeconds ;
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

  // TODO - sometimes gps is right ahead of mon, and somtimes right behind, calc
  // might be wrong

  int32_t phaseDeltaUS =
      newPhaseUS - dataNextSyncOutPhaseUS;  // change from old phase

  int32_t newMainPhase = (int32_t)capture2uS(data.monCapture) + phaseDeltaUS;
  if (newMainPhase > 1000000) {
    newMainPhase -= 1000000;
  }
  if (newMainPhase < 0) {
    newMainPhase += 1000000;
  }
  dataNextSyncOutPhaseUS = newMainPhase;
  data.localOffsetUS = newMainPhase;

  int32_t newExtPhase =
      (int32_t)extCapture2uS(data.monAuxCapture) + phaseDeltaUS;
  if (newExtPhase > 1000000) {
    newExtPhase -= 1000000;
  }
  if (newExtPhase < 0) {
    newExtPhase += 1000000;
  }
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

  if (syncTo == SourceGPS) {
    int64_t delta =
        (metrics.gpsTimeUS[curr] - metrics.localTimeUS[curr]) / 1000000ll;
    data.localSeconds += delta;
    //data.localSeconds -= 1; // TODO NO IDEA why this became needed
    data.extSeconds = data.localSeconds;
  }

  if (syncTo == SourceSync) {
    int64_t delta =
        (metrics.syncTimeUS[curr] - metrics.localTimeUS[curr]) / 1000000ll;
    data.localSeconds += delta;
    data.extSeconds = data.localSeconds;
  }

  if (syncTo == SourceExternal) {
    int64_t delta =
        (metrics.extTimeUS[curr] - metrics.localTimeUS[curr]) / 1000000ll;
    data.localSeconds += delta;
  }

  if (syncTo == SourceNone) {
    data.localSeconds = 0;
    data.extSeconds = 0;
  }

  metrics.lastSyncSeconds = data.localSeconds;

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

void metricsAdjust() {
   char __attribute__((__unused__)) buffer[100];

    if (data.localSeconds == dataPrev.localSeconds) {
    return;
  }

  // TODO - move to doing this on high priority interupt to minimize
  // the odds odd of something updating data during the copy
  memcpy(&dataPrev, &data, sizeof(dataPrev));

  metrics.haveSync = false;
  metrics.haveExt = false;
  metrics.haveGps = false;

  uint32_t tick = HAL_GetTick();
  if ((tick > 1500) && (dataPrev.syncCaptureTick + 1500 > tick)) {
    metrics.haveSync = true;
  }
  if ((tick > 1500) && (dataPrev.gpsCaptureTick + 1500 > tick)) {
    metrics.haveGps = true;
  }
  if ((tick > 1500) && (dataPrev.extSecondsTick + 1500 > tick)) {
    metrics.haveExt = true;
  }

#if 0
  snprintf(buffer, sizeof(buffer), " \r\nDBG gpsCaptureTick=%lu gpsAtMonSecondsTick=%lu diff=%lu \r\n",
           dataPrev.gpsCaptureTick , dataPrev.gpsAtMonSecondsTick  ,  dataPrev.gpsCaptureTick - dataPrev.gpsAtMonSecondsTick
           );
  HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
#endif

  if (dataPrev.gpsAtMonSecondsTick < dataPrev.gpsCaptureTick) {
    // this GPS capture was from the second before the current pulse, correct
    // for this
    dataPrev.gpsAtMonSeconds += 1;
    dataPrev.gpsAtMonSecondsTick += 1000;
    if (dataPrev.gpsAtMonSecondsTick < dataPrev.gpsCaptureTick) {
      // this GPS capture was from way before the second before the current
      // pulse, correct for this
      dataPrev.gpsAtMonSeconds += 1;
      dataPrev.gpsAtMonSecondsTick += 1000;
    }
  }

  if (dataPrev.ltcAtMonSecondsTick < dataPrev.syncCaptureTick) {
    // this LTC capture was from the second before the current pulse, correct
    // for this
    dataPrev.ltcAtMonSeconds += 1;
    dataPrev.ltcAtMonSecondsTick += 1000;
    if (dataPrev.ltcAtMonSecondsTick < dataPrev.syncCaptureTick) {
      // this LTC capture was from the second before the current pulse, correct
      // for this
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

  int64_t deltaSync = (int64_t)capture2uS(dataPrev.syncCapture) -
                      capture2uS(dataPrev.monCapture);
  metrics.syncTimeUS[curr] =
      (int64_t)dataPrev.ltcAtMonSeconds * 1000000 + deltaSync;

  int64_t deltaGps = (int64_t)capture2uS(dataPrev.gpsCapture) -
                     capture2uS(dataPrev.monCapture);
  metrics.gpsTimeUS[curr] =
      (int64_t)dataPrev.gpsAtMonSeconds * 1000000 + deltaGps;

#if 0
      snprintf(buffer, sizeof(buffer), "SYNC : syncTime=%lu \r\n",
               capture2uS(dataPrev.syncCapture));
      HAL_UART_Transmit(&hUartDebug, (uint8_t *) buffer, strlen(buffer), 1000);
#endif

  int64_t syncDiffUS = metrics.syncTimeUS[curr] - metrics.localTimeUS[curr];
  if (metrics.haveSync) {
    if ( (syncDiffUS > 2500 ) || (syncDiffUS < -2500 ) ) {

        // wait for metrics to stabilize on new sync
        if (metrics.lastSyncSeconds + 1 < data.localSeconds) {
            // ignore if we have old data ( happens on sync and on unplug of sync )
            if (syncDiffUS > -990000) {

#if 0
                snprintf(buffer, sizeof(buffer), "SYNC LOSS: syncTime=%lu syncDiff=%ld \r\n",
                         capture2uS(dataPrev.syncCapture) , (int32_t) syncDiffUS);
                HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
#endif

                updateStatus(StatusLostSync);
            }
        }
    }
  }

  int64_t gpsDiffUS = metrics.gpsTimeUS[curr] - metrics.localTimeUS[curr];
  if (metrics.haveGps) {
    if ( (gpsDiffUS > 2500 ) || (gpsDiffUS < -2500 ) ) {
        // wait for metrics to stabilize on new sync
        if (metrics.lastSyncSeconds + 1 < data.localSeconds) {
            // ignore if we have old data ( happens on sync and on unplug of sync )
            if (syncDiffUS > -990000) {
                updateStatus(StatusLostSync);
            }
        }
    }
  }

#if 0
  snprintf(buffer, sizeof(buffer), " \r\nDBG gpsDiff(us)=%ld gpsTime(us)=%lu localTime(us)=%l \r\n",
           (int32_t)gpsDiffUS,  (uint32_t)(metrics.gpsTimeUS[curr]) ,  (uint32_t)(metrics.localTimeUS[curr])
           );
  HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);

  int32_t tickDelta = (int32_t) dataPrev.gpsAtMonSecondsTick -  dataPrev.gpsCaptureTick ;
  snprintf(buffer, sizeof(buffer), "DBG tickDelta(s)=%ld gpsAtMonSecondsTick=%lu gpsCaptureTick=%lu \r\n",
           tickDelta ,
           dataPrev.gpsAtMonSecondsTick, dataPrev.gpsCaptureTick
           );
  HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);

  snprintf(buffer, sizeof(buffer), "DBG gpsSeconds=%lu gpsAtMonSeconds=%lu \r\n",
           dataPrev.gpsSeconds, dataPrev.gpsAtMonSeconds  );
  HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);

#endif


  int64_t secondsSinceSync =
      metrics.localTimeUS[curr] / 1000000l - metrics.lastSyncSeconds;

  // TODO - set up the valid time before sync lost - current values are WAG
  uint32_t syncValidTimeSeconds = 20;
  switch (config.extOscType) {
    case 0:
      syncValidTimeSeconds = 500;
      break;  // 1 ms @ 2 ppm
    case 2:
      syncValidTimeSeconds = 40000;
      break;  // 1 ms @ 25 ppb
    case 10:
      syncValidTimeSeconds = 100000;
      break;  // 1 ms @ 10 ppb
  }

  if (secondsSinceSync > syncValidTimeSeconds) {
    updateStatus(StatusLostSync);
  }
}

void metricsRun() {
  char buffer[100];


  metricsAdjust();

  int curr = metrics.nextIndex;
  metrics.nextIndex++;
  if (metrics.nextIndex >= metricsHistorySize) {
    metrics.nextIndex = 0;
  }



    if (dataPrev.localSeconds % 5 == 2) {



#if 1
      snprintf(buffer, sizeof(buffer), "\r\nMetrics\r\n");
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);


    if (1) {
      snprintf(buffer, sizeof(buffer), "    LocalTime(ms) %8ld \r\n",
               us2ms( metrics.localTimeUS[curr] ));
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (metrics.haveGps) {
      updateStatus(StatusCouldSync);
      snprintf(buffer, sizeof(buffer), "    GpsTime(ms) %10ld \r\n",
               us2ms( metrics.gpsTimeUS[curr] ));
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (metrics.haveSync) {
      updateStatus(StatusCouldSync);
      snprintf(buffer, sizeof(buffer), "    SyncTime(ms) %9ld \r\n",
              us2ms( metrics.syncTimeUS[curr] ));
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (metrics.haveExt) {
      snprintf(buffer, sizeof(buffer), "    ExtTime(ms) %10ld \r\n",
             us2ms( metrics.extTimeUS[curr] ));
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    int64_t secondsSinceSync =
        metrics.localTimeUS[curr] / 1000000l - metrics.lastSyncSeconds;

    if (1) {
      snprintf(buffer, sizeof(buffer), "\r\n    lastSync(s) %4lu\r\n",
               (uint32_t)secondsSinceSync);
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (metrics.haveSync) {
      int64_t diff = metrics.syncTimeUS[curr] - metrics.localTimeUS[curr];

      snprintf(buffer, sizeof(buffer), "    syn-lcl(us) %6ld \r\n",
               (int32_t)(diff) );
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (metrics.haveExt) {
      int64_t diff = metrics.extTimeUS[curr] - metrics.localTimeUS[curr];

      snprintf(buffer, sizeof(buffer), "    ext-lcl(us) %6ld \r\n",
               (int32_t)(diff));
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (metrics.haveGps) {
      int64_t diff = metrics.gpsTimeUS[curr] - metrics.localTimeUS[curr];

      snprintf(buffer, sizeof(buffer), "    gps-lcl(us) %6ld \r\n",
               (int32_t)(diff));
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (metrics.haveGps && metrics.haveExt) {
      int64_t diff = metrics.gpsTimeUS[curr] - metrics.extTimeUS[curr];

      snprintf(buffer, sizeof(buffer), "    gps-ext(us) %6ld \r\n",
               (int32_t)(diff));
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (1) {
      snprintf(buffer, sizeof(buffer), "\r\n");
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }
#endif

#if 0
    if ( 1 ) {

    // make sure metricsHistorySize is greater than this
    int durationS = 1000;  // Not really changeable
    int prev = curr - durationS;
    if (prev < 0) {
      prev += metricsHistorySize;
    }

    int64_t lclDelta = metrics.localTimeUS[curr] - metrics.localTimeUS[prev];
    int64_t extDelta = metrics.extTimeUS[curr] - metrics.extTimeUS[prev];
    int64_t gpsDelta = metrics.gpsTimeUS[curr] - metrics.gpsTimeUS[prev];
    int64_t syncDelta = metrics.syncTimeUS[curr] - metrics.syncTimeUS[prev];

    int64_t ppbMult = 1000 / durationS;
    int64_t gpsLclPpb = (gpsDelta - lclDelta) * ppbMult;
    int64_t extLclPpb = (extDelta - lclDelta) * ppbMult;
    int64_t syncLclPpb = (syncDelta - lclDelta) * ppbMult;
    int64_t syncExtPpb = (syncDelta - extDelta) * ppbMult;
    int64_t extGpsPpb = (extDelta - gpsDelta) * ppbMult;

    if ( secondsSinceSync > durationS ) {
    if (metrics.haveGps) {
      snprintf(buffer, sizeof(buffer), "    gpsLclDrift(ppb) %7ld \r\n",
               (int32_t)(gpsLclPpb) );
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (metrics.haveExt) {
      snprintf(buffer, sizeof(buffer), "    extLclDrift(ppb) %7ld \r\n",
               (int32_t)(extLclPpb ));
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (metrics.haveSync) {
      snprintf(buffer, sizeof(buffer), "    synLclDrift(ppb) %7ld \r\n",
               (int32_t)(syncLclPpb ));
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (metrics.haveSync && metrics.haveExt) {
      snprintf(buffer, sizeof(buffer), "    synExtDrift(ppb) %7ld \r\n",
               (int32_t)(syncExtPpb ));
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (metrics.haveGps && metrics.haveExt) {
      snprintf(buffer, sizeof(buffer), "    extGpsDrift(ppb) %7ld \r\n",
               (int32_t)(extGpsPpb ));
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }
    }
    }
#endif


#if 0
    if ( 1 ) {

    // make sure metricsHistorySize is greater than this
    int durationS = 10;  // Not really changeable
    int prev = curr - durationS;
    if (prev < 0) {
      prev += metricsHistorySize;
    }

    int64_t lclDelta = metrics.localTimeUS[curr] - metrics.localTimeUS[prev];
    int64_t extDelta = metrics.extTimeUS[curr] - metrics.extTimeUS[prev];
    int64_t gpsDelta = metrics.gpsTimeUS[curr] - metrics.gpsTimeUS[prev];
    int64_t syncDelta = metrics.syncTimeUS[curr] - metrics.syncTimeUS[prev];

    int64_t ppbMultInv =  durationS ;
    int64_t gpsLclPpm = (gpsDelta - lclDelta) / ppbMultInv;
    int64_t extLclPpm = (extDelta - lclDelta) / ppbMultInv;
    int64_t syncLclPpm = (syncDelta - lclDelta) / ppbMultInv;
    int64_t syncExtPpm = (syncDelta - extDelta) / ppbMultInv;
    int64_t extGpsPpm = (extDelta - gpsDelta) / ppbMultInv;

    if ( secondsSinceSync > durationS ) {
    if (metrics.haveGps) {
      snprintf(buffer, sizeof(buffer), "    gpsLclDrift(ppm) %4ld \r\n",
               (int32_t)(gpsLclPpm) );
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (metrics.haveExt) {
      snprintf(buffer, sizeof(buffer), "    extLclDrift(ppm) %4ld \r\n",
               (int32_t)(extLclPpm ));
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (metrics.haveSync) {
      snprintf(buffer, sizeof(buffer), "    synLclDrift(ppm) %4ld \r\n",
               (int32_t)(syncLclPpm ));
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (metrics.haveSync && metrics.haveExt) {
      snprintf(buffer, sizeof(buffer), "    synExtDrift(ppm) %4ld \r\n",
               (int32_t)(syncExtPpm ));
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (metrics.haveGps && metrics.haveExt) {
      snprintf(buffer, sizeof(buffer), "    extGpsDrift(ppm) %4ld \r\n",
               (int32_t)(extGpsPpm ));
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }
    }
    }
#endif

    if (1) {
      snprintf(buffer, sizeof(buffer), "\r\n");
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }
  }
}
