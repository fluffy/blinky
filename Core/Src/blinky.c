// Copyright (c) 2023 Cullen Jennings

// #include <math.h>  // for round
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "audio.h"
#include "blink.h"
#include "config.h"
#include "detect.h"
#include "gps.h"
#include "hardware.h"
#include "ltc.h"
#include "main.h"
#include "measurement.h"
#include "metrics.h"
#include "pps.h"
#include "thermo.h"
#include "status.h"
#include "disp.h"
#include "setting.h"


// Uses Semantic versioning. See https://semver.org/
// major.minor.patch,
// patch=year/month/day
const char *version = "0.090.240122";


Measurements data;

uint32_t dataNextSyncOutPhaseUS;  // TODO put in setting struct
uint32_t dataCurrentSyncOutPhaseUS;

const uint32_t blinkAudioPulseWidthMs = 33;

LtcTransitionSet ltcSendTransitions;
LtcTransitionSet ltcRecvTransitions;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  uint32_t tick = HAL_GetTick();

  if (htim == &hTimeSync) {
    data.localSeconds++;
    data.localSecondsTick = tick;
  }

  if (htim == &hTimeAux) {
    data.extSeconds++;
    data.extSecondsTick = tick;
  }

#if 0
  if (htim == &hTimeDAC) {
  }
#endif

#if 0
  if (htim == &hTimeADC) {
  }
#endif

  if (htim == &hTimeBlink) {
    // This block of code takes 1.9 uS and runs every 1 mS

    uint32_t nowCapture = __HAL_TIM_GetCounter(&hTimeSync);
    uint32_t nowUs = capture2uS(nowCapture);
    uint32_t monUs = capture2uS(data.monCapture);
    int32_t ledUs = nowUs - monUs;
    if (ledUs < 0) {
      ledUs += 1000000l;
    }
    if (ledUs >= 1000000l) {
      ledUs -= 1000000l;
    }
    dispUpdate( ledUs , 0 /* TODO seconds */ );
  }
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
#if 0
   if (1) {
    char buffer[100];
    snprintf(buffer, sizeof(buffer), "\r\n* %d\r\n",htim->Channel );
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
  }
#endif

  uint32_t tick = HAL_GetTick();

#if 1
  // LTC Stuff
  if (htim == &hTimeLtc) {
    if (htim->Channel == TimeLtc_HAL_CH_SYNC_IN2) {  // sync on both edges
      uint32_t val;
      val = HAL_TIM_ReadCapturedValue(htim, TimeLtc_CH_SYNC_IN2);
      ltcRecvTransitions.transitionTimeUs[ltcRecvTransitions.numTransitions++] =
          val * 20l;  // convert to uSec, this timer counts at 50 KHz

      if (ltcRecvTransitions.numTransitions >= ltcMaxTransitions) {
        ltcRecvTransitions.numTransitions = 0;
      }
    }
  }
#endif

  if (htim == &hTimeSync) {
    if (htim->Channel ==
        TimeSync_HAL_CH_SYNC_IN) {  // sync in falling edge. falling is rising
                                    // on inverted input

      // supress for 100 ms after seeing first edge
      if ((data.syncCaptureTick < tick) &&
          (tick < data.syncCaptureTick + 100 /*ms*/)) {
        // supress this tick
      } else {
        data.syncCapture = HAL_TIM_ReadCapturedValue(htim, TimeSync_CH_SYNC_IN);
        data.syncCaptureTick = tick;
      }
    }

    if (htim->Channel ==
        TimeSync_HAL_CH_SYNC_MON) {  // sync mon falling edge. falling is rising
                                     // on inverted output

      // supress for 100 ms after seeing first edge
      if ((data.monCaptureTick < tick) &&
          (tick < data.monCaptureTick + 100 /*ms*/)) {
        // supress this tick
      } else {
        data.monCapture = HAL_TIM_ReadCapturedValue(htim, TimeSync_CH_SYNC_MON);
        data.monCaptureTick = tick;

        data.ltcAtMonSeconds = data.ltcSeconds;
        data.ltcAtMonSecondsTick = data.ltcSecondsTick;

        data.gpsAtMonSeconds = data.gpsSeconds;
        data.gpsAtMonSecondsTick = data.gpsSecondsTick;

        data.localAtMonSeconds = data.localSeconds;
        data.localAtMonSecondsTick = data.localSecondsTick;

        data.extAtMonSeconds = data.extSeconds;
        data.extAtMonSecondsTick = data.extSecondsTick;
      }
    }

    if (htim->Channel == TimeSync_HAL_CH_GPS_PPS) {  // sync in on falling edge.
                                                     // falling is rising
                                                     // on inverted input
      data.gpsCapture = HAL_TIM_ReadCapturedValue(htim, TimeSync_CH_GPS_PPS);
      data.gpsCaptureTick = tick;
    }
  }

  if (htim == &hTimeAux) {
    if (htim->Channel ==
        TimeAux_HAL_CH_SYNC_MON) {  // sync mon falling edge. falling is rising
                                    // on inverted output

      // supress for 100 ms
      if ((data.monAuxCaptureTick < tick) &&
          (tick < data.monAuxCaptureTick + 100 /*ms*/)) {
        // supress this tick
      } else {
        data.monAuxCapture =
            HAL_TIM_ReadCapturedValue(htim, TimeAux_CH_SYNC_MON);
        data.monAuxCaptureTick = tick;
      }
    }

    if (htim->Channel == TimeAux_HAL_CH_GPS_PPS) {  // sync in on falling edge.
                                                    // falling is rising
                                                    // on inverted input

      if ((data.gpsAuxCaptureTick < tick) &&
          (tick < data.gpsAuxCaptureTick + 100 /*ms*/)) {
        // supress this tick
      } else {
        data.gpsAuxCapture =
            HAL_TIM_ReadCapturedValue(htim, TimeAux_CH_GPS_PPS);
        data.gpsAuxCaptureTick = tick;
      }
    }
  }
}

void blinkInit() {
  memset(&data, 0, sizeof(data));
  data.gpsAtMonSeconds = 1;
  data.gpsSeconds = 1;

  dataNextSyncOutPhaseUS = 10l * 1000l;
  dataCurrentSyncOutPhaseUS = dataNextSyncOutPhaseUS;

  settingInit();

  configInit();
  gpsInit();
  audioInit();
  ppsInit();
  dispInit();

  LtcTransitionSetClear(&ltcSendTransitions);
  LtcTransitionSetClear(&ltcRecvTransitions);

  metricsInit();
}

int32_t captureDeltaUs(uint32_t pps, uint32_t mon) {
  // Return delta from two capture times in ms
  int32_t ppsUs = capture2uS(pps);
  int32_t monUs = capture2uS(mon);
  int32_t diffUs = ppsUs - monUs;
  if (diffUs <= -500000) {
    diffUs += 1000000;
  }
  if (diffUs > 500000) {
    diffUs -= 1000000;
  }
  int ret = diffUs;
  return ret;
}

void blinkSetup() {
  HAL_GPIO_WritePin(LEDMR_GPIO_Port, LEDMR_Pin,
                    GPIO_PIN_SET);  // turn on red error LED
  HAL_GPIO_WritePin(LEDMB_GPIO_Port, LEDMB_Pin,
                    GPIO_PIN_RESET);  // turn off yellow  LED
  HAL_GPIO_WritePin(LEDMG_GPIO_Port, LEDMG_Pin,
                    GPIO_PIN_RESET);  // turn off green ok LED

  if (1) {
    char buffer[100];
    snprintf(buffer, sizeof(buffer), "\r\nStarting...\r\n");
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    snprintf(buffer, sizeof(buffer), "  Software version: %s\r\n", version);
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
  }

  settingSetup();

  dispSetup();
  configSetup();
  metricsSetup();
  thermoSetup();
  ppsSetup();

  // start aux timer
  HAL_TIM_Base_Start_IT(&hTimeAux);
  HAL_TIM_IC_Start_IT(&hTimeAux,
                      TimeAux_CH_SYNC_MON);  // start sync mon capture on aux
  HAL_TIM_IC_Start_IT(&hTimeAux,
                      TimeAux_CH_GPS_PPS);  // start gps pps capture on aux

#if 0
  HAL_Delay(100); // get diff between main and aux when same clock for testing
#endif

  // start main timer
  HAL_TIM_Base_Start_IT(&hTimeSync);
  HAL_TIM_IC_Start_IT(&hTimeSync,
                      TimeSync_CH_SYNC_IN);  // start sync in capture
  HAL_TIM_IC_Start_IT(&hTimeSync,
                      TimeSync_CH_SYNC_MON);  // start sync mon capture
  HAL_TIM_IC_Start_IT(&hTimeSync,
                      TimeSync_CH_GPS_PPS);  // start gps pps capture

  // start LTC timer
  HAL_TIM_Base_Start_IT(&hTimeLtc);
  HAL_TIM_IC_Start_IT(&hTimeLtc,
                      TimeLtc_CH_SYNC_IN2);  // start sync in capture

  // start display timer
  HAL_TIM_Base_Start_IT(&hTimeBlink);

  audioSetup();

  gpsSetup();

#if 0
  // TODO
  // TODO need to check both channel 7 and 10 for two different sides of connector
  HAL_ADC_Start(&hadc1);
  HAL_ADC_PollForConversion(&hadc1, 1000 /*timeout ms*/);
  uint16_t val = HAL_ADC_GetValue(&hadc1);
  int power=0; // in mA
  if ( ( val > 250) & ( val <= 750 ) ) {
    power = 500;
  }
  if ( ( val > 750) & ( val <= 1500 ) ) {
    power = 1500;
  }
  if ( ( val > 1500) & ( val <= 2500 ) ) {
    power = 3000;
  }
  if (1) {
    char buffer[100];
    snprintf(buffer, sizeof(buffer), "  Power supply: %d.%dA (value=%u)\r\n", power/1000, ( power/100) % 10 ,  val );
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
  }
  // seeing value of 0 if USB connector is fliped wrong way
  // value of 543 = on usb hub = 0.4375 V
  // value of 1138 , 1157 on mac front port = 0.916 V
  // value of 501 = on  usb expander bar

  // the far end has 10k, 22K, 56K resisotr to 5V for 3, 1.5, 0.5 A respectively
  // this end has 5.1K resitor to ground
  // range of ADC is 4096 for 3.3 V
  // < 250 , wrong side CC
  // <  750 , have 0.5 A
  // < 1500 , have 1.5 A
  // < 2500 , have 3 A
#endif

  if ( HAL_GPIO_ReadPin(BTN1_GPIO_Port, BTN1_Pin) ) {
    setting.blinkPPS = 1;
    char buffer[100];
    snprintf(buffer, sizeof(buffer), "Sync button pressed during boot. Set PPS mode\r\n");
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
  }

  if (1) {
    char buffer[100];
    snprintf(buffer, sizeof(buffer), "Setup Done\r\n");
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
  }

 //    assert_param(0 );
 //  assert(0);

  // set LED to on but not sync ( yellow, not green )
  updateStatus( StatusRunning);
}

void blinkRun() {
    static int loopCount = 0;
    static char button1WasPressed = 0;
    static char button2WasPressed = 0;
    static char button3WasPressed = 0;

    static uint32_t __attribute__((__unused__)) syncCaptureTickPrev = 0;
  static uint32_t __attribute__((__unused__)) monCaptureTickPrev = 0;
  static uint32_t __attribute__((__unused__)) gpsCaptureTickPrev = 0;

  static uint32_t __attribute__((__unused__)) monAuxCaptureTickPrev = 0;
  static uint32_t __attribute__((__unused__)) gpsSecondsTickPrev = 0;

  static uint32_t __attribute__((__unused__)) ltcSecondsTickPrev = 0;
  static uint32_t __attribute__((__unused__)) ltcGenTickPrev = 0;

  static uint32_t __attribute__((__unused__)) localSecondsTickPrev = 0;
  static uint32_t __attribute__((__unused__)) extSecondsTickPrev = 0;
  static uint32_t __attribute__((__unused__)) extSecondsPrev = 0;

  static uint32_t __attribute__((__unused__)) gpsAuxCaptureTickPrev = 0;

  char buffer[100];

  uint32_t tick = HAL_GetTick();

  if (loopCount % 3000 == 0) {

    snprintf(
        buffer, sizeof(buffer),
        "Phases(ms) mon=%lu.%03lu sync=%lu.%03lu gps=%lu.%03lu ext=%lu.%03lu "
        "\r\n",
        capture2uS(data.monCapture) / 1000, capture2uS(data.monCapture) % 1000,
        capture2uS(data.syncCapture) / 1000,
        capture2uS(data.syncCapture) % 1000, capture2uS(data.gpsCapture) / 1000,
        capture2uS(data.gpsCapture) % 1000,
        extCapture2uS(data.monAuxCapture) / 1000,
        extCapture2uS(data.monAuxCapture) % 1000);
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
  }

  if (loopCount % 100 == 0) {
#if 0
    snprintf(buffer, sizeof(buffer), "Loop %d\r\n", loopCount);
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
#endif

#if 0
    int16_t tempDeciC = thermoGetTemperatureDeciC();
         snprintf(buffer, sizeof(buffer), "   Temperature: %d.%dC \r\n",
               tempDeciC / 10, tempDeciC % 10);
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
#endif

#if 0
    snprintf(buffer, sizeof(buffer), "  DAC/ADC Cplt %lu %lu \r\n", debugDacCpltCount, debugAdcCpltCount );
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
#endif
  }

  if (config.product == 1) {
    if (!HAL_GPIO_ReadPin(AUX_CLK_GPIO_Port, AUX_CLK_Pin)) {
      if (!button2WasPressed) {
        setting.blinkMute = (setting.blinkMute) ? 0 : 1; // TODO move to setting.c

        snprintf(buffer, sizeof(buffer), "Mute button pressed. Mute=%d \r\n",
                 (int)setting.blinkMute);
        HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
      }
      button2WasPressed = 1;
    } else {
      button2WasPressed = 0;
    }
  }

  if (HAL_GPIO_ReadPin(BOOT1_GPIO_Port, BOOT1_Pin)) { // TODO move to setting.c
    if (!button3WasPressed) {
      if (setting.blinkBlank) {
        setting.blinkDispAudio = 1;
        setting.blinkBlank = 0;
      } else if (setting.blinkDispAudio) {
        setting.blinkDispAudio = 0;
        setting.blinkBlank = 0;
      } else {
        setting.blinkDispAudio = 0;
        setting.blinkBlank = 1;
      }

      snprintf(buffer, sizeof(buffer),
               "Display button pressed. Blank=%d dispAudio=%d \r\n", (int)setting.blinkBlank,
               (int)setting.blinkDispAudio);
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }
    button3WasPressed = 1;
  } else {
    button3WasPressed = 0;
  }

  if ((!HAL_GPIO_ReadPin(BTN1_GPIO_Port, BTN1_Pin))) {
    if (!button1WasPressed) {
      snprintf(buffer, sizeof(buffer), "Sync button pressed\r\n");
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);


      if ((tick > 2000) && (data.gpsCaptureTick + 2000 > tick)) {
        //  had GPS sync in last 2 seconds
        metricsSync( SourceGPS);
        updateStatus( StatusSync);
      } else if ((tick > 2000) && (data.syncCaptureTick + 2000 > tick)) {
        //  had sync in last 2 seconds
        metricsSync( SourceSync );
        updateStatus(StatusSync);
      } else if ((tick > 2000) && (data.extSecondsTick + 2000 > tick)) {
        //  had ext in last 2 seconds
        metricsSync(SourceExternal);
        updateStatus(StatusLostSync);
      } else {
        metricsSync(SourceNone);
        updateStatus(StatusLostSync);
#if 0
        snprintf(buffer, sizeof(buffer),
                 "ERROR: No gps or sync input on button press\r\n");
        HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
#endif
      }
    }
    button1WasPressed = 1;
  } else {
    button1WasPressed = 0;
  }

#if 0  // TODO prints too much stuff
  if (1) {
    uint32_t val = __HAL_TIM_GetCounter(&hTimeSync);
    snprintf( buffer, sizeof(buffer), "Sync time: %lu.%03lums\r\n",  capture2uS(val)/1000l , capture2uS(val)%1000l);
    HAL_UART_Transmit( &hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
  }

  if (0) {
    uint32_t val = __HAL_TIM_GetCounter(&hTimeAux);
    snprintf( buffer, sizeof(buffer), "Aux  time: %lu.%03lums\r\n",  extCapture2uS(val)/1000l , extCapture2uS(val)%1000l );
    HAL_UART_Transmit( &hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
  }
#endif

  if (1) {
    static uint32_t prevVal = 0;
    uint32_t val = __HAL_TIM_GetCounter(&hTimeSync);  // TODO - hate this

    if (val < prevVal) {  // 1 second loop
      float mlpVal;
      uint32_t mltTime;
      detectGetMlpTime(&mltTime, &mlpVal);

      int32_t blinkAudioDelayMs;  // this is the detected value from detect

      if (mlpVal > 5000.0) {
        blinkAudioDelayMs = captureDeltaUs(mltTime, data.monCapture) / 1000l -
                            blinkAudioPulseWidthMs;
        if (blinkAudioDelayMs < 0) {
          blinkAudioDelayMs += 1000;
        }

        snprintf(buffer, sizeof(buffer), "   Audio delay: %d ms\r\n",
                 (int)blinkAudioDelayMs);
        HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
      } else {
        blinkAudioDelayMs = 0;
      }

      data.blinkAudioDelayMs = blinkAudioDelayMs;
#if 0
      float min, max, avg, last;
      detectGetDebug( &min, &max, &avg, &last);

      snprintf(buffer, sizeof(buffer), "  Audio debug %d %d %d %d \r\n",
               (int)min, (int)max, (int)avg, (int)last);
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
#endif

      detectResetMlp();
    }

    detectUpdateMlp(val);
    prevVal = val;
  }

  if (data.syncCaptureTick != syncCaptureTickPrev) {
    syncCaptureTickPrev = data.syncCaptureTick;
#if 0
    snprintf(buffer, sizeof(buffer), "   sync phase=%lums delta2mon=%ldms\r\n",
             capture2uS(data.syncCapture) / 1000l,
             captureDeltaUs(data.syncCapture, data.monCapture) / 1000l);
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
#endif
  }

  if ((tick >= data.monCaptureTick) && (tick <= data.monCaptureTick + 2)) {
#if 1
    snprintf(buffer, sizeof(buffer), "@ %lu\r\n", data.localSeconds);
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
#endif
  }

  if (tick > data.monCaptureTick + 100) {
    if (data.monCaptureTick != monCaptureTickPrev) {
#if 0  // to much print out
    snprintf(buffer, sizeof(buffer), "   mon phase: %lums\r\n",
        capture2uS(data.monCapture )/1000l);
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
#endif

      monCaptureTickPrev = data.monCaptureTick;

      metricsRun();
    }
  }

#if 1
  // TODO - call ppsStart ????
  if (data.ltcGenTick + 2000 < tick) {
    // kick start if get out of sync
    data.ltcGenTick = tick;
  }
#endif

  if ( (data.ltcGenTick != ltcGenTickPrev) && ( tick > data.ltcGenTick+5 ) ) {
    // gen new LTC code
    static Ltc ltc;
    ltcClear(&ltc);
    static LtcTimeCode timeCode;
    ltcTimeCodeClear(&timeCode);
    ltcTimeCodeSet(&timeCode, data.localSeconds + 1, 0 /* us */);

#if 0 // TODO
    //ltcTimeCodeSetHMSF(  &timeCode, 15/*hr*/,31/*min*/,31/*sec*/,15/*frame*/ );
    ltcTimeCodeSetHMSF(  &timeCode, 0/*hr*/, 0/*min*/, 0/*sec*/, 0/*frame*/ );
#endif

    ltcSet(&ltc, &timeCode);
    if (setting.blinkPPS) {
      ppsEncode(&ltc, &ltcSendTransitions);
    } else {
      ltcEncode(&ltc, &ltcSendTransitions, 30 /*fps*/);
    }

    // TODO - some way to kick start if dies
    ppsStart();
#if 0  // TODO
    snprintf(buffer, sizeof(buffer), "   LTC Enc: %lu\r\n", ltcTimeCodeSeconds(&timeCode) );
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
#endif
    ltcGenTickPrev = data.ltcGenTick;
  }

  if (data.syncCaptureTick + 100 /*ms*/ <
      tick) {  // been 100 ms since start of last LTC transition (which flags a
               // sync)
    if (data.syncCaptureTick !=
        ltcSecondsTickPrev) {  // this seems messed up - TODO - think about
      ltcSecondsTickPrev = data.syncCaptureTick;

      // process LTC transition data
      static LtcTimeCode timeCode;
      ltcTimeCodeClear(&timeCode);
      static Ltc ltc;
      ltcClear(&ltc);

      if (ltcRecvTransitions.numTransitions <= 3) {
        // assume in PPS mode not LTC
        data.ltcSeconds++;
        data.ltcSecondsTick = tick;
      } else {
        int err = ltcDecode(&ltc, &ltcRecvTransitions, 30 /*fps */);
        if (err != 0) {
#if 1
          snprintf(buffer, sizeof(buffer), "   LTC decode ERROR: %d\r\n", err);
          HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer),
                            1000);
#endif
          data.ltcSeconds++;
          data.ltcSecondsTick = tick;
        } else {
          ltcGet(&ltc, &timeCode);
          data.ltcSeconds = ltcTimeCodeSeconds(&timeCode);
          data.ltcSecondsTick = tick;
#if 0
          snprintf(buffer, sizeof(buffer), "   LTC Dec: %lu\r\n", data.ltcSeconds);
          HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
#endif
        }
      }
      LtcTransitionSetClear(
          &ltcRecvTransitions);  // reset ltc capture for next cycle
    }
  }

#if 0
  if (data.gpsCaptureTick != gpsCaptureTickPrev) {
    snprintf(buffer, sizeof(buffer), "   gpsPhase(ms): %lu \r\n",
             capture2uS( data.gpsCapture) / 1000);
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    gpsCaptureTickPrev = data.gpsCaptureTick;
  }
#endif

#if 0
  if (data.gpsAuxCaptureTick != gpsAuxCaptureTickPrev) {
    snprintf(buffer, sizeof(buffer), "   gpsAuxPhase(ms): %lu.%03lu \r\n",
             capture2uS( data.gpsAuxCapture) / 1000, capture2uS( data.gpsAuxCapture) % 1000);
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    gpsAuxCaptureTickPrev = data.gpsAuxCaptureTick;
  }
#endif

#if 0
  if ( gpsSecondsTickPrev !=  data.gpsSecondsTick ) {
    snprintf(buffer, sizeof(buffer), "   GPS Dec: %lu UTC\r\n", data.gpsSeconds);
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    gpsSecondsTickPrev = data.gpsSecondsTick;
  }
#endif

#if 0
  if (data.extSecondsTick != extSecondsTickPrev) {
    snprintf(buffer, sizeof(buffer), "   ext time: %lus\r\n", data.extSeconds);
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    extSecondsTickPrev = data.extSecondsTick;
  }
#endif

#if 0
  if (data.localSecondsTick != localSecondsTickPrev) {
    snprintf(buffer, sizeof(buffer), "   local time: %lus\r\n", data.localSeconds);
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    localSecondsTickPrev = data.localSecondsTick;
  }
#endif

  if ((data.monAuxCaptureTick != monAuxCaptureTickPrev) &&
      (data.extSeconds != extSecondsPrev)) {
#if 0
    snprintf(buffer, sizeof(buffer),
             "   Aux mon: external time %lu.%03lums\r\n",
             extCapture2uS(data.monAuxCapture) / 1000l, extCapture2uS(data.monAuxCapture) % 1000l);
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
#endif

    monAuxCaptureTickPrev = data.monAuxCaptureTick;

    extSecondsPrev = data.extSeconds;
  }

  HAL_Delay(1);

  loopCount++;
}
