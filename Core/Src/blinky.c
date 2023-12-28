// Copyright (c) 2023 Cullen Jennings

//#include <math.h>  // for round
#include <stdio.h>
#include <string.h>

#include "audio.h"
#include "blink.h"
#include "detect.h"
#include "gps.h"
#include "pps.h"
#include "ltc.h"
#include "thermo.h"
#include "main.h"

#include "hardware.h"
#include "config.h"


// Uses Semantic versioning https://semver.org/
// major.minor.patch,
// patch=year/month/day
const char *version = "0.90.231227";

// The main timer counter max value (typically 10 MHz ) * M need this cacluation
// to fit in 32 bits

uint8_t blinkMute = 1;       // mutes audio outout
uint8_t blinkBlank = 1;      // causes LED to be off
uint8_t blinkDispAudio = 0;  // caused audio latency to be displayed on LED

uint8_t blinkHaveDisplay = 1;

typedef struct {
  
  uint32_t gpsCapture;
  uint32_t gpsCaptureTick;
  uint32_t monCapture;
  uint32_t monCaptureTick;
  uint32_t syncCapture;
  uint32_t syncCaptureTick;

  uint32_t gpsAuxCapture;
  uint32_t gpsAuxCaptureTick;
  uint32_t monAuxCapture;
  uint32_t monAuxCaptureTick;
  // there is no sync captue for Aux

} Measurements;

Measurements data;

uint32_t dataLtcCapture;
uint32_t dataLtcCaptureTick;
uint32_t dataLtcGenTick;

//uint32_t dataAuxMonCapture;
//uint32_t dataAuxMonCaptureTick;
//uint32_t dataAuxGpsPpsCapture;
//uint32_t dataAuxGpsPpsCaptureTick;

uint32_t dataExtClkCount;  // counting seconds
uint32_t dataExtClkCountTick;
int32_t dataExtClkCountTickOffset;

uint32_t dataNextSyncOutPhaseUS;
uint32_t dataCurrentSyncOutPhaseUS;

int32_t blinkAudioDelayMs; // this is the detected value from detect
const uint32_t blinkAudioPulseWidthMs = 33;

LtcTransitionSet ltcSendTransitions;
LtcTransitionSet ltcRecvTransitions;

uint32_t blinkLocalSeconds;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  uint32_t tick = HAL_GetTick();

  if (htim == &hTimeSync) {
    blinkLocalSeconds++;
  }

  if (htim == &hTimeAux) {
    dataExtClkCount++;
    dataExtClkCountTick = tick;
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
    int32_t ledMs = ledUs / 1000l;

    if (blinkDispAudio) {
      ledMs = blinkAudioDelayMs;
    }

    if (ledMs >= 1000) {
      ledMs -= 1000;
    }
    int16_t binCount = (ledMs / 100) % 10;  // 2.5 ms

    int row = 5 - (ledMs / 2) % 5;     // 2 ms across
    int col = 10 - (ledMs / 10) % 10;  // 10 ms down

    if (blinkBlank) {
      binCount = 0x1000;
      row = 255;
      col = 255;
    }

    if (blinkHaveDisplay) {
      HAL_GPIO_WritePin(NCOL1_GPIO_Port, NCOL1_Pin,
                        (col == 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(NCOL2_GPIO_Port, NCOL2_Pin,
                        (col == 2) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(NCOL3_GPIO_Port, NCOL3_Pin,
                        (col == 3) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(NCOL4_GPIO_Port, NCOL4_Pin,
                        (col == 4) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(NCOL5_GPIO_Port, NCOL5_Pin,
                        (col == 5) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(NCOL6_GPIO_Port, NCOL6_Pin,
                        (col == 6) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(NCOL7_GPIO_Port, NCOL7_Pin,
                        (col == 7) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(NCOL8_GPIO_Port, NCOL8_Pin,
                        (col == 8) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(NCOL9_GPIO_Port, NCOL9_Pin,
                        (col == 9) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(NCOL10_GPIO_Port, NCOL10_Pin,
                        (col == 10) ? GPIO_PIN_SET : GPIO_PIN_RESET);

      HAL_GPIO_WritePin(NROW1_GPIO_Port, NROW1_Pin,
                        (row == 1) ? GPIO_PIN_RESET : GPIO_PIN_SET);
      HAL_GPIO_WritePin(NROW2_GPIO_Port, NROW2_Pin,
                        (row == 2) ? GPIO_PIN_RESET : GPIO_PIN_SET);
      HAL_GPIO_WritePin(NROW3_GPIO_Port, NROW3_Pin,
                        (row == 3) ? GPIO_PIN_RESET : GPIO_PIN_SET);
      HAL_GPIO_WritePin(NROW4_GPIO_Port, NROW4_Pin,
                        (row == 4) ? GPIO_PIN_RESET : GPIO_PIN_SET);
      HAL_GPIO_WritePin(NROW5_GPIO_Port, NROW5_Pin,
                        (row == 5) ? GPIO_PIN_RESET : GPIO_PIN_SET);
    }

    if (blinkHaveDisplay) {
      // Low 4 bits of display
      HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin,
                        (binCount & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin,
                        (binCount & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin,
                        (binCount & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin,
                        (binCount & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);

      // High 4 bits of display
#if 0  // TODO
      // problems LED 5,6 input only
      HAL_GPIO_WritePin(LED5_GPIO_Port, LED5_Pin,
                        (binCount & 0x10) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LED6_GPIO_Port, LED6_Pin,
                        (binCount & 0x20) ? GPIO_PIN_SET : GPIO_PIN_RESET);
#endif
      HAL_GPIO_WritePin(LED7_GPIO_Port, LED7_Pin,
                        (binCount & 0x40) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LED8_GPIO_Port, LED8_Pin,
                        (binCount & 0x80) ? GPIO_PIN_SET : GPIO_PIN_RESET);

      HAL_GPIO_WritePin(LED9_GPIO_Port, LED9_Pin,
                        (binCount & 0x100) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LED10_GPIO_Port, LED10_Pin,
                        (binCount & 0x200) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
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

      dataLtcCapture = val;
      dataLtcCaptureTick = tick;

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

      if ((data.monAuxCaptureTick < tick) &&
          (tick < data.monAuxCaptureTick + 100 /*ms*/)) {
        // supress this tick
      } else {
        data.monAuxCapture = HAL_TIM_ReadCapturedValue(htim, TimeAux_CH_SYNC_MON);
        data.monAuxCaptureTick = tick;
      }
    }

    if (htim->Channel == TimeAux_HAL_CH_GPS_PPS) {  // sync in on falling edge.
                                                    // falling is rising
                                                    // on inverted input
      data.gpsAuxCapture =
          HAL_TIM_ReadCapturedValue(htim, TimeAux_CH_GPS_PPS);
      data.gpsAuxCaptureTick = tick;
    }
  }
}

void blinkInit() {

  memset( &data, 0, sizeof( data ) );
  
  data.monCapture = 0;
  data.monCaptureTick = 0;
  data.syncCapture = 0;
  data.syncCaptureTick = 0;
  data.gpsCapture = 0;
  data.gpsCaptureTick = 0;
  dataExtClkCount = 0;
  dataExtClkCountTickOffset = -1000;  // TODO
  dataNextSyncOutPhaseUS = 10l * 1000l;
  dataCurrentSyncOutPhaseUS = dataNextSyncOutPhaseUS;
  // subFrameCount = 0;
  // subFrameCountOffset = 120;

  configInit();
  
  gpsInit();

  blinkLocalSeconds = 0;

  audioInit();
  ppsInit();
  
  LtcTransitionSetClear(&ltcSendTransitions);
  LtcTransitionSetClear(&ltcRecvTransitions);

  data.syncCaptureTick = 0;
  dataLtcGenTick = 0;
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
  // HAL_GPIO_WritePin(LEDMR_GPIO_Port, LEDMR_Pin,
  //                   GPIO_PIN_RESET);  // turn on red error LED
  HAL_GPIO_WritePin(LEDMY_GPIO_Port, LEDMY_Pin,
                    GPIO_PIN_SET);  // turn on yellow  LED
  HAL_GPIO_WritePin(LEDMG_GPIO_Port, LEDMG_Pin,
                    GPIO_PIN_SET);  // turn on green ok LED

  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, GPIO_PIN_RESET);

#if 0  // TODO
  HAL_GPIO_WritePin(LED5_GPIO_Port, LED5_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LED6_GPIO_Port, LED6_Pin, GPIO_PIN_RESET);
#endif
  HAL_GPIO_WritePin(LED7_GPIO_Port, LED7_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LED8_GPIO_Port, LED8_Pin, GPIO_PIN_RESET);

  HAL_GPIO_WritePin(LED9_GPIO_Port, LED9_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LED10_GPIO_Port, LED10_Pin, GPIO_PIN_RESET);

  if (1) {
    char buffer[100];
    snprintf(buffer, sizeof(buffer), "\r\nStarting...\r\n");
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    snprintf(buffer, sizeof(buffer), "  Software version: %s\r\n", version);
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
  }

  configSetup();
  
  thermoSetup();

  ppsSetup();
  
  HAL_TIM_Base_Start_IT(&hTimeBlink);

  HAL_TIM_Base_Start_IT(&hTimeSync);

 
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

  HAL_TIM_Base_Start_IT(&hTimeAux);

  HAL_TIM_IC_Start_IT(&hTimeAux,
                      TimeAux_CH_SYNC_MON);  // start sync mon capture on aux

  HAL_TIM_IC_Start_IT(&hTimeAux,
                      TimeAux_CH_GPS_PPS);  // start gps pps capture on aux

  // set LED to on but not sync ( yellow, not green )
  HAL_GPIO_WritePin(LEDMY_GPIO_Port, LEDMY_Pin,
                    GPIO_PIN_SET);  // turn on blue assert LED
  HAL_GPIO_WritePin(LEDMG_GPIO_Port, LEDMG_Pin,
                    GPIO_PIN_RESET);  // turn off green ok LED

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

  if (1) {
    char buffer[100];
    snprintf(buffer, sizeof(buffer), "Setup Done\r\n");
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
  }
}

void blinkRun() {
  static int loopCount = 0;
  static char button1WasPressed = 0;
  static char button2WasPressed = 0;
  static char button3WasPressed = 0;

  static uint32_t syncCaptureTickPrev = 0;
  static uint32_t monCaptureTickPrev = 0;
  static uint32_t gpsCaptureTickPrev = 0;

  
  static uint32_t dataLtcCaptureTickPrev = 0;
  static uint32_t dataLtcGenTickPrev = 0;
  //static uint32_t dataExtClkCountTickPrev = 0;
  static uint32_t monAuxCaptureTickPrev = 0;
  static uint32_t gpsTimeTickPrev = 0;
  static uint32_t dataExtClkCountMonPrev = 0;

  char buffer[100];

  uint32_t tick = HAL_GetTick();

  if (loopCount % 100 == 0) {
    snprintf(buffer, sizeof(buffer), "Loop %d\r\n", loopCount);
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);

#if 1
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
        blinkMute = (blinkMute) ? 0 : 1;

        snprintf(buffer, sizeof(buffer), "Button 2 press. Mute=%d\r\n",
                 (int)blinkMute);
        HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
      }
      button2WasPressed = 1;
    } else {
      button2WasPressed = 0;
    }
  }

  if (HAL_GPIO_ReadPin(BOOT1_GPIO_Port, BOOT1_Pin)) {
    if (!button3WasPressed) {
      if (blinkBlank) {
        blinkDispAudio = 1;
        blinkBlank = 0;
      } else if (blinkDispAudio) {
        blinkDispAudio = 0;
        blinkBlank = 0;
      } else {
        blinkDispAudio = 0;
        blinkBlank = 1;
      }

      snprintf(buffer, sizeof(buffer),
               "Button 3 press. Blank=%d dispAudio=%d\r\n", (int)blinkBlank,
               (int)blinkDispAudio);
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }
    button3WasPressed = 1;
  } else {
    button3WasPressed = 0;
  }

  if (!HAL_GPIO_ReadPin(BTN1_GPIO_Port, BTN1_Pin)) {
    if (!button1WasPressed) {  
      uint32_t tick = HAL_GetTick();

      snprintf(buffer, sizeof(buffer), "Button 1 press\r\n");
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);

      dataExtClkCountTickOffset = dataExtClkCountTick;
      dataExtClkCount = 0;

      if ((tick > 2000) && (data.syncCaptureTick + 2000 > tick)) {
        //  had sync in last 2 seconds
        int32_t deltaPhaseUS = capture2uS(data.syncCapture) - capture2uS(data.monCapture);
        if ( deltaPhaseUS < 0) {
          deltaPhaseUS += 1000000l;
        }
        if (deltaPhaseUS >= 1000000l) {
          deltaPhaseUS -= 1000000l;
        }
        
        uint32_t prePhaseUS = dataNextSyncOutPhaseUS;  // TODO REMOVE
        uint32_t phaseUS = dataNextSyncOutPhaseUS + deltaPhaseUS;
        if ( phaseUS < 0) {
          phaseUS += 1000000l;
        }
        if ( phaseUS >= 1000000l) {
          phaseUS -= 1000000l;
        }
        dataNextSyncOutPhaseUS = phaseUS;
        
        snprintf(buffer, sizeof(buffer),
                 "  SYNC IN: sync phase=%lums, mon phase=%lums, oldPhase=%lums, newPhase=%ldms\r\n",
                 capture2uS(data.syncCapture)/1000l, capture2uS(data.monCapture)/1000l,
                 prePhaseUS/1000l, phaseUS/1000l  );
        HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
        
      }
      else if ((tick > 2000) && (data.gpsCaptureTick + 2000 > tick))  {
        //  had GPS sync in last 2 seconds
        int32_t deltaPhaseUS = capture2uS(data.gpsCapture) - capture2uS(data.monCapture);
        if ( deltaPhaseUS < 0) {
          deltaPhaseUS += 1000000l;
        }
        if (deltaPhaseUS >= 1000000l) {
          deltaPhaseUS -= 1000000l;
        }
        
        uint32_t prePhaseUS = dataNextSyncOutPhaseUS;  // TODO REMOVE
        uint32_t phaseUS = dataNextSyncOutPhaseUS + deltaPhaseUS;
        if ( phaseUS < 0) {
          phaseUS += 1000000l;
        }
        if ( phaseUS >= 1000000l) {
          phaseUS -= 1000000l;
        }
        dataNextSyncOutPhaseUS = phaseUS;
        
        snprintf(buffer, sizeof(buffer),
                 "  SYNC GPS: gpsPhase=%lums, moPhase=%lums, oldPhase=%lums, newPhase=%ldms\r\n",
                 capture2uS(data.gpsCapture)/1000l, capture2uS(data.monCapture)/1000l,
                 prePhaseUS/1000l, phaseUS/1000l  );
        HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
      }
       else
        {
        snprintf(buffer, sizeof(buffer), "ERROR: No gps or sync input on button press\r\n");
        HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
      }
 
    }
    button1WasPressed = 1;
  } else {
    button1WasPressed = 0;
  }

#if 0 // TODO prints too much stuff
  if (1) {
    uint32_t val = __HAL_TIM_GetCounter(&hTimeSync);
    snprintf( buffer, sizeof(buffer), "Sync time: %lu.%03lums\r\n",  capture2uS(val)/1000l , capture2uS(val)%1000l);
    HAL_UART_Transmit( &hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
  }

  if (1) {
    uint32_t val = __HAL_TIM_GetCounter(&hTimeAux);
    snprintf( buffer, sizeof(buffer), "Aux  time: %lu.%03lums\r\n",  extCapture2uS(val)/1000l , extCapture2uS(val)%1000l );
    HAL_UART_Transmit( &hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
  }
#endif

   if (1) { 
    static uint32_t prevVal = 0;
    uint32_t val = __HAL_TIM_GetCounter(&hTimeSync); // TODO - hate this 

    if (val < prevVal) {  // 1 second loop
      float mlpVal;
      uint32_t mltTime;
      detectGetMlpTime(&mltTime, &mlpVal);

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
      
      snprintf(buffer, sizeof(buffer), "   sync phase=%lums delta2mon=%ldms\r\n",
        capture2uS(data.syncCapture)/1000l, captureDeltaUs(data.syncCapture, data.monCapture)/1000l );
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }
  

  if (data.monCaptureTick != monCaptureTickPrev) {
#if 0 // to much print out 
    snprintf(buffer, sizeof(buffer), "   mon phase: %lums\r\n",
        capture2uS(data.monCapture )/1000l);
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
#endif
    
    monCaptureTickPrev = data.monCaptureTick;
  }

#if 1
  // TODO - call ppsStart ????
  if (dataLtcGenTick + 2000 < tick) {
    // kick start if get out of sync
    dataLtcGenTick = tick;
  }
#endif

  if (dataLtcGenTick != dataLtcGenTickPrev) {
    // gen new LTC code
    static Ltc ltc;
    ltcClear(&ltc);
    static LtcTimeCode timeCode;
    LtcTimeCodeClear(&timeCode);
    LtcTimeCodeSet(&timeCode, blinkLocalSeconds, 0 /* us */);

    // LtcTimeCodeSetHMSF(  &timeCode, 1,1,1,1 );

    ltcSet(&ltc, &timeCode);
    ltcEncode(&ltc, &ltcSendTransitions, 30 /*fps*/);

    // TODO - some way to kick start if dies 
    ppsStart();
    
    snprintf(buffer, sizeof(buffer), "   LTC gen %lus\r\n", LtcTimeCodeSeconds(&timeCode) );
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);

    dataLtcGenTickPrev = dataLtcGenTick;
  }


  if (dataLtcCaptureTick + 100 /*ms*/ <
      tick) {  // been 100 ms since lask LTC transition
    if (dataLtcCaptureTick != dataLtcCaptureTickPrev) {
      dataLtcCaptureTickPrev = dataLtcCaptureTick;

      // process LTC transition data
      static LtcTimeCode timeCode;
      LtcTimeCodeClear(&timeCode);
      static Ltc ltc;
      ltcClear(&ltc);
      int err = ltcDecode(&ltc, &ltcRecvTransitions, 30 /*fps */);
      if (err != 0) {
        snprintf(buffer, sizeof(buffer), "   LTC decode ERROR: %d\r\n", err);
        HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
      }

      ltcGet(&ltc, &timeCode);
      uint32_t ltcSeconds = LtcTimeCodeSeconds(&timeCode);

      snprintf(buffer, sizeof(buffer), "   LTC decode: %lus\r\n",
               ltcSeconds);
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);

      LtcTransitionSetClear(
          &ltcRecvTransitions);  // reset ltc capture for next cycle
    }
  }

  if (data.gpsCaptureTick != gpsCaptureTickPrev) {
    snprintf(buffer, sizeof(buffer), "   gps delta: %ldms\r\n",
             captureDeltaUs(data.gpsCapture, data.monCapture) / 1000);
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    gpsCaptureTickPrev = data.gpsCaptureTick;
  }

  if (gpsTimeTick != gpsTimeTickPrev) {
    snprintf(buffer, sizeof(buffer), "   gps time: %s UTC\r\n", gpsTime);
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    gpsTimeTickPrev = gpsTimeTick;
  }

#if 0
  if (dataExtClkCountTick != dataExtClkCountTickPrev) {
    snprintf(buffer, sizeof(buffer), "   ext time: %lus\r\n", dataExtClkCount);
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    dataExtClkCountTickPrev = dataExtClkCountTick;
  }
#endif

#if 1
  if ((data.monAuxCaptureTick != monAuxCaptureTickPrev) &&
      (dataExtClkCount != dataExtClkCountMonPrev)) {
    snprintf(buffer, sizeof(buffer),
             "   Aux mon: external time %lu.%03lums\r\n", 
             extCapture2uS(data.monAuxCapture) / 1000l, extCapture2uS(data.monAuxCapture) % 1000l);
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    monAuxCaptureTickPrev = data.monAuxCaptureTick;
    
    dataExtClkCountMonPrev = dataExtClkCount;
  }
#endif
  
  HAL_Delay(10);

  loopCount++;
}
