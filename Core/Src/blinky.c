// Copyright (c) 2023 Cullen Jennings

#include <stdio.h>
#include <stm32f4xx_ll_tim.h>
#include <string.h>

#include "blink.h"
#include "detect.h"
#include "main.h"

extern ADC_HandleTypeDef hadc1;

extern DAC_HandleTypeDef hdac;

extern I2C_HandleTypeDef hi2c1;

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim8;

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;

#define hADC hadc1

#define hDAC hdac
#define DAC_CH_OSC_ADJ DAC_CHANNEL_1
#define DAC_CH_AUD_OUT DAC_CHANNEL_2

#define hI2c hi2c1
#define hUartDebug huart1
#define hUartGps huart3

#define hTimePps htim1
#define TimePps_CH_SYNC_OUT TIM_CHANNEL_1
#define TimePps_LL_CH_SYNC_OUT LL_TIM_CHANNEL_CH1

#define hTimeSync htim2
#define TimeSync_CH_SYNC_IN TIM_CHANNEL_2
#define TimeSync_HAL_CH_SYNC_IN HAL_TIM_ACTIVE_CHANNEL_2
#define TimeSync_CH_GPS_PPS TIM_CHANNEL_3
#define TimeSync_HAL_CH_GPS_PPS HAL_TIM_ACTIVE_CHANNEL_3
#define TimeSync_CH_SYNC_MON TIM_CHANNEL_4
#define TimeSync_HAL_CH_SYNC_MON HAL_TIM_ACTIVE_CHANNEL_4

#define hTimeADC htim3

#define hTimeBlink htim4

#define hTimeAux htim5
#define TimeAux_CH_AUX_CLK TIM_CHANNEL_1
#define TimeAux_CH_GPS_PPS TIM_CHANNEL_2
#define TimeAux_HAL_CH_GPS_PPS HAL_TIM_ACTIVE_CHANNEL_3
#define TimeAux_CH_SYNC_MON TIM_CHANNEL_3
#define TimeAux_HAL_CH_SYNC_MON HAL_TIM_ACTIVE_CHANNEL_3

#define hTimeDAC htim6

#define hTimeLtc htim8
#define TimeLtc_CH_SYNC_IN2 TIM_CHANNEL_1

// Uses Semantic versioning https://semver.org/
// major.minor.patch,
// patch=year/month/day
const char *version = "0.80.231126";  


// This structure is saved in EEPROM
// keep size padded to 32 bits
typedef struct {
  uint8_t  version; // 1 
  uint8_t  product; // 1=blink, 2=clock
  uint8_t  revMajor; // 0x1 is Rev A 
  uint8_t  revMinor; // start at 0 

  uint16_t serialNum; // 0 is not valid
  
  int16_t  oscAdj; // offset for intenal oscilator counter   
  uint16_t vcoValue; // value loaded in DAC for VCO
  uint8_t  extOscType; // external osc type ( 2= 2.048 MHz, 10=10 MHz, 0=Internal ) )
} Config;
static Config config;

// #define captureFreqHz 2048000ul
//  next macro must have capture2uS( captureFreqHz ) fit in 32 bit calculation
// #define capture2uS(c) ((c) * 1000ul / 2048ul)

// The main timer counter max value (typically 10 MHz ) * M need to fit in 32
// bits
uint32_t capture2uSRatioM = 125;
uint32_t capture2uSRatioN = 256;
inline uint32_t capture2uS(const uint32_t c) {
  return (c * capture2uSRatioM) / capture2uSRatioN;
}

volatile uint32_t debugAdcCpltCount = 0;
volatile uint32_t debugDacCpltCount = 0;
volatile uint32_t debugDacTimerCnt = 0;
volatile uint32_t debugAdcTimerCnt = 0;

uint8_t blinkMute = 0;  // mutes audio outout 
uint8_t blinkBlank = 0;  // causes LED to be off
uint8_t blinkDispAudio = 0; // caused audio latency to be displayed on LED


uint32_t dataSyncCapture;
uint32_t dataSyncCaptureTick;
uint32_t dataMonCapture;
uint32_t dataMonCaptureTick;
uint32_t dataGpsPpsCapture;
uint32_t dataGpsPpsCaptureTick;

uint32_t dataAuxMonCapture;
uint32_t dataAuxMonCaptureTick;
uint32_t dataAuxGpsPpsCapture;
uint32_t dataAuxGpsPpsCaptureTick;

uint32_t dataExtClkCount;  // counting seconds
uint32_t dataExtClkCountTick;
int32_t dataExtClkCountTickOffset;

uint16_t dataNextSyncOutPhase;
uint16_t dataCurrentPhaseSyncOut;

// dacBuffer has 20 point sin wave center on 1000 with amplitude 500
const int dacBufferLen = 20;
uint32_t dacBuffer[] = {1000, 1155, 1294, 1405, 1476, 1500, 1476,
                        1405, 1294, 1155, 1000, 845,  706,  595,
                        524,  500,  524,  595,  706,  845};
const int adcBufferLen = 20;
uint32_t adcBuffer[20];

const int gpsBufferLen = 20;
uint8_t gpsBuffer[20];
uint8_t gpsBufLen = 0;
char gpsTime[7];  // This will have ASCII chars 123456 to indicate time is
                  // 12:34:56 UTC
uint32_t gpsTimeTick;

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
          // GPS daa is valid
          strncpy(gpsTime, (char *)gpsBuffer + 7, sizeof(gpsTime) - 1);
          gpsTime[sizeof(gpsTime) - 1] = 0;  // terminate time string

          uint32_t tick = HAL_GetTick();
          gpsTimeTick = tick;

#if 0
          if (1) { char buffer[100];
          snprintf(buffer, sizeof(buffer), "   gps time UTC: %s \r\n", gpsTime );
          HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
          }
#endif
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

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc) {
  detectUpdate(&(adcBuffer[0]), adcBufferLen / 2, false);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
  detectUpdate(&(adcBuffer[adcBufferLen / 2]), adcBufferLen / 2, true);

  debugAdcCpltCount++;
}

void HAL_DACEx_ConvHalfCpltCallbackCh2(DAC_HandleTypeDef *hdac) {}

void HAL_DACEx_ConvCpltCallbackCh2(DAC_HandleTypeDef *hdac) {
  debugDacCpltCount++;
}

void HAL_DAC_ErrorCallbackCh1(DAC_HandleTypeDef *hdac) {
  char buffer[100];
  snprintf(buffer, sizeof(buffer), "ERROR hand DAC CH1\r\n");
  HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);

  Error_Handler();
}

void HAL_DAC_ErrorCallbackCh2(DAC_HandleTypeDef *hdac) {
  char buffer[100];
  snprintf(buffer, sizeof(buffer), "ERROR hand DAC CH2\r\n");
  HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);

  Error_Handler();
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
#if 1
   uint32_t tick = HAL_GetTick();
   
  if (htim == &hTimeAux) {
    dataExtClkCount++;
    dataExtClkCountTick = tick;
  }
#endif

#if 1
  if (htim == &hTimeDAC) {
    debugDacTimerCnt++;
  }
#endif

#if 1
  if (htim == &hTimeADC) {
    debugAdcTimerCnt++;
  }
#endif

  if (htim == &hTimeBlink) {
    // This block of code takes 1.9 uS and runs every 1 mS

    uint32_t nowCapture = __HAL_TIM_GetCounter(&hTimeSync);
    uint32_t nowUs = capture2uS(nowCapture);
    uint32_t monUs = capture2uS(dataMonCapture);
    int32_t ledUs = nowUs - monUs;
    if (ledUs < 0) {
      ledUs += 1000000l;
    }
   
    int32_t ledMs =  ledUs / 1000l;

    if ( ledMs >= 1000) {
      ledMs -= 1000;
    }
    int16_t binCount =  (ledMs/100)%10;  // 2.5 ms

    if ( blinkDispAudio ) {
      // TODO 
    }
    
    int row = 5 - (ledMs / 2 ) % 5 ; // 2 ms across
    int col = 10 - (ledMs / 10) % 10; // 10 ms down
    
    if ( blinkBlank ) {
      binCount = 0x1000;
      row=255;
      col=255;
    }
    
    if (1) {
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

    if (1) {
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
  
  if (htim == &hTimeSync) {
    if (htim->Channel ==
        TimeSync_HAL_CH_SYNC_IN) {  // sync in falling edge. falling is rising
                                    // on inverted input
      dataSyncCapture = HAL_TIM_ReadCapturedValue(htim, TimeSync_CH_SYNC_IN);
      dataSyncCaptureTick = tick;
    }

    if (htim->Channel ==
        TimeSync_HAL_CH_SYNC_MON) {  // sync mon falling edge. falling is rising
                                     // on inverted output
      dataMonCapture = HAL_TIM_ReadCapturedValue(htim, TimeSync_CH_SYNC_MON);
      dataMonCaptureTick = tick;
    }

    if (htim->Channel == TimeSync_HAL_CH_GPS_PPS) {  // sync in on falling edge.
                                                     // falling is rising
                                                     // on inverted input
      dataGpsPpsCapture = HAL_TIM_ReadCapturedValue(htim, TimeSync_CH_GPS_PPS);
      dataGpsPpsCaptureTick = tick;
    }
  }

#if 1
  if (htim == &hTimeAux) {
    if (htim->Channel ==
        TimeAux_HAL_CH_SYNC_MON) {  // sync mon falling edge. falling is rising
                                     // on inverted output
      dataAuxMonCapture = HAL_TIM_ReadCapturedValue(htim, TimeAux_CH_SYNC_MON);
      dataAuxMonCaptureTick = tick;
    }

    if (htim->Channel == TimeAux_HAL_CH_GPS_PPS) {  // sync in on falling edge.
                                                     // falling is rising
                                                     // on inverted input
      dataAuxGpsPpsCapture = HAL_TIM_ReadCapturedValue(htim, TimeAux_CH_GPS_PPS);
      dataAuxGpsPpsCaptureTick = tick;
    }
  }
#endif
}

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim == &hTimePps) {
    uint16_t val = __HAL_TIM_GET_COMPARE(&hTimePps, TimePps_CH_SYNC_OUT);
    if (val != dataCurrentPhaseSyncOut) {
      // end of output pulse just happened, set up for next output pulse
      dataCurrentPhaseSyncOut = dataNextSyncOutPhase;
      __HAL_TIM_SET_COMPARE(&hTimePps, TimePps_CH_SYNC_OUT,
                            dataNextSyncOutPhase);
      LL_TIM_OC_SetMode(
          TIM1, TimePps_LL_CH_SYNC_OUT,
          LL_TIM_OCMODE_INACTIVE);  // inverted due to inverting output buffer

      // stop audio output
      HAL_DAC_Stop_DMA(&hDAC, DAC_CHANNEL_2);
    } else {  // val == dataCurrentPhaseSyncOut
      // start of output pulse just started, set up for the end of pulse
      val = dataCurrentPhaseSyncOut + 100 * 10;  // 100 ms wide pulse
      if (val >= 10000) {
        val -= 10000;
      }
      __HAL_TIM_SET_COMPARE(&hTimePps, TimePps_CH_SYNC_OUT, val);
      LL_TIM_OC_SetMode(
          TIM1, TimePps_LL_CH_SYNC_OUT,
          LL_TIM_OCMODE_ACTIVE);  // inverted due to inverting output buffer

      if ( !blinkMute ) {
      // start audio output
      HAL_DAC_Start_DMA(&hDAC, DAC_CHANNEL_2, dacBuffer,
                        dacBufferLen,  //  dacBufferlen is in 32 bit words
                        DAC_ALIGN_12B_R);
      }
    }
  }
}

void blinkInit() {
  dataMonCapture = 0xFFFFffff;
  dataMonCaptureTick = 0;
  dataSyncCapture = 0xFFFFffff;
  dataSyncCaptureTick = 0;
  dataGpsPpsCapture = 0xFFFFffff;
  dataGpsPpsCaptureTick = 0;
  dataExtClkCount = 0;
  dataExtClkCountTickOffset = -1000;
  dataNextSyncOutPhase = 5000;
  dataCurrentPhaseSyncOut = dataNextSyncOutPhase;
  // subFrameCount = 0;
  // subFrameCountOffset = 120;
  gpsTimeTick = 0;
  memset(gpsTime, 0, sizeof(gpsTime));
   
  detectInit(adcBufferLen);
}

int captureDeltaUs(uint32_t pps, uint32_t mon) {
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
  int retMs = diffUs;
  return retMs;
}

void setClk(uint8_t extOscTypeType, uint16_t vcoValue, int16_t oscAdj ) {
  char buffer[100];

  if (extOscTypeType == 2) {
    // External CLK is 2.048 Mhz
    __HAL_TIM_SET_AUTORELOAD(&hTimeSync, 2048ul * 1000ul - 1ul);
    capture2uSRatioM = 125;
    capture2uSRatioN = 256;

    snprintf(buffer, sizeof(buffer), "  External clock set to 2.048 Mhz \r\n");
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
  } else if (extOscTypeType == 10) {
    // External CLK is 10 Mhz
    __HAL_TIM_SET_AUTORELOAD(&hTimeSync, 10ul * 1000ul * 1000ul - 1ul);
    capture2uSRatioM = 1;
    capture2uSRatioN = 10;

    snprintf(buffer, sizeof(buffer), "  External clock set to 10 Mhz \r\n");
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
  } else if (extOscTypeType == 0) {
    // CLK is internal 84 Mhz
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};

    //oscAdj=-534; // -17 to +2,  delta 90 ,  , serial# 6 Nov 26, 2023
    //oscAdj=-542; // 65,  delta 90 ,  , serial# 3 Nov 26, 2023
    
    htim2.Init.Prescaler = 8 - 1;
    htim2.Init.Period =
        10500ul * 1000ul - 1ul +
        oscAdj;  
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    capture2uSRatioM = 2;
    capture2uSRatioN = 21;

    if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK) {
      Error_Handler();
    }
    if (HAL_TIM_IC_Init(&htim2) != HAL_OK) {
      Error_Handler();
    }

    snprintf(buffer, sizeof(buffer), "  Internal clock set to 10.5 MHz \r\n");
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
  }

  //vcoValue = 497; // -8 to 10 ns , delta = 30/-100 = -0.30  , serial# 6 Nov 26, 2023
  
  snprintf(buffer, sizeof(buffer), "  VCO: %u\r\n",  vcoValue);
  HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);

  HAL_DAC_Start(&hDAC, DAC_CH_OSC_ADJ);
  HAL_DAC_SetValue(&hDAC, DAC_CHANNEL_1, DAC_ALIGN_12B_R,  vcoValue );
}

void blinkSetup() {
  //HAL_GPIO_WritePin(LEDMR_GPIO_Port, LEDMR_Pin,
  //                  GPIO_PIN_RESET);  // turn on red error LED
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

  // access EEProm
  if (1) {
    char buffer[100];
    const uint16_t i2cAddr = 0x50;
    uint32_t timeout = 256;
    uint8_t eepromMemAddr = 0;
    HAL_StatusTypeDef status;

    status = HAL_I2C_IsDeviceReady(&hI2c, i2cAddr << 1, 2, timeout);
    if (status != HAL_OK) {
      snprintf(buffer, sizeof(buffer), "Error: EEProm not found \r\n");
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }
  
#ifdef FORMAT_EEPROM  // TODO move to seperate program
    const int writeConfigEEProm = 1;
#else
    const int writeConfigEEProm = 0;
#endif
    if (writeConfigEEProm) {
      // write config to EEProm
      config.version=1;
      config.product = 2; 
      config.revMajor = 0;
      config.revMinor = 8;
      config.serialNum = 5;

      config.extOscType = 0;
      config.oscAdj = -658 ;
      config.vcoValue = 2000;

      status = HAL_I2C_Mem_Write(&hI2c, i2cAddr << 1,
                                 eepromMemAddr, sizeof(eepromMemAddr),
                                 (uint8_t *)&config, (uint16_t)sizeof(config),
                                 timeout);
      if (status != HAL_OK) {
        // stat: 0=0k, 1 is HAL_ERROR, 2=busy , 3 = timeout
        snprintf(buffer, sizeof(buffer),
                 "EEProm Write Error:  data hal error %d \r\n", status);
        HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
      }

      // chip has 1.5 ms max page write time when it will not respond
      HAL_Delay(2 /*ms */);
      HAL_Delay(20 /*ms */);
    }

    status =
        HAL_I2C_Mem_Read(&hI2c, i2cAddr << 1,
                         eepromMemAddr,sizeof(eepromMemAddr),
                         (uint8_t *)&config, (uint16_t)sizeof(config),
                         timeout);
    if (status != HAL_OK) {
      // stat: 0=0k, 1 is HAL_ERROR, 2=busy , 3 = timeout
      snprintf(buffer, sizeof(buffer),
               "EEProm Read Error:  data hal error %d \r\n", status);
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    // config.vcoValue = 111; // TODO remove - test time drift
    // config.oscAdj = -658 ; // TODO remove 
    
    if ( (config.version < 1) || ( config.version > 10 )  ) {
      snprintf(buffer, sizeof(buffer), "EEProm not initalized: %d \r\n",config.version );
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
      
      Error_Handler();
    }
  
    if ( ( config.revMajor == 0 ) && ( config.revMinor == 8 ) ) {
      snprintf(buffer, sizeof(buffer), "  Hardware version: EV8 \r\n");
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);

      setClk( config.extOscType, config.vcoValue, config.oscAdj );
    } else {
      snprintf(buffer, sizeof(buffer), "Unknown hardware version %d.%d\r\n",
               config.revMajor , config.revMinor  );
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);

      Error_Handler();
    }

    snprintf(buffer, sizeof(buffer), "  Serial: %d \r\n", config.serialNum );
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
  }

  HAL_TIM_Base_Start_IT(&hTimeBlink);

  HAL_TIM_Base_Start_IT(&hTimeSync);

  HAL_TIM_OC_Start_IT(&hTimePps, TimePps_CH_SYNC_OUT);  // start sync out

  HAL_TIM_Base_Start_IT(&hTimeSync);

  HAL_TIM_IC_Start_IT(&hTimeSync,
                      TimeSync_CH_SYNC_IN);  // start sync in capture

  HAL_TIM_IC_Start_IT(&hTimeSync,
                      TimeSync_CH_SYNC_MON);  // start sync mon capture

  HAL_TIM_IC_Start_IT(&hTimeSync,
                      TimeSync_CH_GPS_PPS);  // start gps pps capture

#if 1 
  HAL_TIM_Base_Start_IT(&hTimeAux);

  HAL_TIM_IC_Start_IT(&hTimeAux,
                      TimeAux_CH_SYNC_MON );  // start sync mon capture on aux 

   HAL_TIM_IC_Start_IT(&hTimeAux,
                       TimeAux_CH_GPS_PPS);  // start gps pps capture on aux 
#endif
   
   
  // set LED to on but not sync ( yellow, not greeen )
  HAL_GPIO_WritePin(LEDMY_GPIO_Port, LEDMY_Pin,
                    GPIO_PIN_SET);  // turn on yellow assert LED
  HAL_GPIO_WritePin(LEDMG_GPIO_Port, LEDMG_Pin,
                    GPIO_PIN_RESET);  // turn off green ok LED

  if (1) {
    char buffer[100];
    snprintf(buffer, sizeof(buffer), "Setup Done\r\n");
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
  }

#if 1
  // DMA for Audio Out DAC

  HAL_StatusTypeDef err;
  err = HAL_DAC_Start_DMA(&hDAC, DAC_CHANNEL_2, dacBuffer,
                          dacBufferLen,  //  dacBufferlen is in 32 bit words
                          DAC_ALIGN_12B_R);
  if (err != HAL_OK) {
    char buffer[100];
    snprintf(buffer, sizeof(buffer), "HAL DAC error %d r\n", err);
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    Error_Handler();
  }

  HAL_TIM_Base_Start_IT(&hTimeDAC);
#endif

#if 1  // TODO
  // DMA for ADC
  HAL_ADC_Start_DMA(&hADC, adcBuffer, adcBufferLen);

  HAL_TIM_Base_Start_IT(&hTimeADC);
#endif

#if 1
  // start receving for GPS serial
  HAL_StatusTypeDef stat =
      HAL_UART_Receive_IT(&hUartGps, gpsBuffer, 1 /* size */);
  if (stat != HAL_UART_ERROR_NONE) {
    Error_Handler();
  }
#endif
}

void blinkRun() {
  static int loopCount = 0;
  static char button1WasPressed = 0;
  static char button2WasPressed = 0;
  static char button3WasPressed = 0;

  static uint32_t dataSyncCaptureTickPrev = 0;
  static uint32_t dataExtClkCountTickPrev = 0;
  static uint32_t dataGpsPpsCaptureTickPrev = 0;
  static uint32_t dataAuxMonCaptureTickPrev = 0; 
  static uint32_t gpsTimeTickPrev = 0;
  static uint32_t dataExtClkCountMonPrev= 0;
  
  char buffer[100];

  if (loopCount % 100 == 0) {
    snprintf(buffer, sizeof(buffer), "\r\nLoop %d \r\n", loopCount);
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);

#if 0
    snprintf(buffer, sizeof(buffer), "  DAC/ADC Cplt %lu %lu \r\n", debugDacCpltCount, debugAdcCpltCount );
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
#endif

#if 0  // TODO remove 
    while (1) {
      char buf[9];
      HAL_UART_Receive (&huart3, (uint8_t*)buf, sizeof(buf)-1, 500 /*timeout ms*/);
      buf[sizeof(buf)-1]=0;

      snprintf(buffer, sizeof(buffer), "GPS: %s \r\n",
               buf );
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }
#endif
  }

#if 1
  if (!HAL_GPIO_ReadPin(DB3_GPIO_Port, DB3_Pin)) {
    if (!button2WasPressed) {
      blinkMute = (blinkMute) ? 0 : 1;

      snprintf(buffer, sizeof(buffer), "Button 2 press. Mute=%d \r\n",
               (int)blinkMute);
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }
    button2WasPressed = 1;
  } else {
    button2WasPressed = 0;
  }
#endif

#if 1
  if (HAL_GPIO_ReadPin(BOOT1_GPIO_Port, BOOT1_Pin)) {
    if (!button3WasPressed) {
      blinkBlank = (blinkBlank) ? 0 : 1;

      snprintf(buffer, sizeof(buffer), "Button 3 press. Blank=%d \r\n",
               (int)blinkBlank);
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }
    button3WasPressed = 1;
  } else {
    button3WasPressed = 0;
  }
#endif

#if 1
  if (!HAL_GPIO_ReadPin(BTN1_GPIO_Port, BTN1_Pin)) {
    if (!button1WasPressed) {
      uint32_t tick = HAL_GetTick();

      snprintf(buffer, sizeof(buffer), "Button 1 press \r\n");
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);

      dataExtClkCountTickOffset = dataExtClkCountTick;
      dataExtClkCount = 0;

      if ((tick > 2000) && (dataSyncCaptureTick + 2000 > tick)) {
        //  had sync in last 2 seconds
        int32_t deltaPhaseUs =
            capture2uS(dataSyncCapture) - capture2uS(dataMonCapture);
        int32_t deltaPhase =
            deltaPhaseUs /
            100l;  // div 100 for 1MHz to 10KHz counter conversion
        if (deltaPhase < 0) deltaPhase += 10000;

        deltaPhase += 0;  // offset

        uint32_t phase = dataNextSyncOutPhase + deltaPhase;
        if (phase >= 10000) {
          phase -= 10000;
        }

        dataNextSyncOutPhase = phase;

        snprintf(buffer, sizeof(buffer), "  SYCN IN: new phase: %ld\r\n",
                 phase);
        HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
      } else if ((tick > 2000) && (dataGpsPpsCaptureTick + 2000 > tick)) {
        //  had GPS sync in last 2 seconds
        int32_t deltaPhaseUs =
            capture2uS(dataGpsPpsCapture) - capture2uS(dataMonCapture);
        int32_t deltaPhase =
            deltaPhaseUs /
            100l;  // div 100 for 1MHz to 10KHz counter conversion
        if (deltaPhase < 0) deltaPhase += 10000;

        deltaPhase += 0;  // offset

        uint32_t phase = dataNextSyncOutPhase + deltaPhase;
        if (phase >= 10000) {
          phase -= 10000;
        }

        dataNextSyncOutPhase = phase;

        snprintf(buffer, sizeof(buffer), "  GPS: new phase: %ld\r\n", phase);
        HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
      } else {
        snprintf(buffer, sizeof(buffer), "  No sync input\r\n");
        HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
      }
    }
    button1WasPressed = 1;
  } else {
    button1WasPressed = 0;
  }
#endif

#if 0  // prints too much stuff
  if (1) {
    uint32_t val = __HAL_TIM_GetCounter(&hTimeSync);
    snprintf( buffer, sizeof(buffer), "Sync Time val %ld uS\r\n",  capture2uS(val) );
    HAL_UART_Transmit( &hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
  }

   if (1) {
    uint32_t val = __HAL_TIM_GetCounter(&hTimeAux);
    snprintf( buffer, sizeof(buffer), "Aux Time val %ld\r\n",  val );
    HAL_UART_Transmit( &hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
  }
#endif

#if 1
  if (1) {
    static uint32_t prevVal = 0;
    uint32_t val = __HAL_TIM_GetCounter(&hTimeSync);

    if (val <
        prevVal) {  // 1 second loop  // TODO - this this comparison backwards
      float mlpVal;
      uint32_t mltTime;
      detectGetMlpTime(&mltTime, &mlpVal);

      if (mlpVal > 5000.0) {
        const int audioPulseLenMs = 100; // TODO 
        snprintf(buffer, sizeof(buffer), "   Audio delay: %d ms\r\n",
                 captureDeltaUs(mltTime, dataMonCapture) / 1000  - audioPulseLenMs );
        HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
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
#endif

  if (dataSyncCaptureTick != dataSyncCaptureTickPrev) {
    snprintf(buffer, sizeof(buffer), "   sync delta: %d ms\r\n",
             captureDeltaUs(dataSyncCapture, dataMonCapture) / 1000);
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    dataSyncCaptureTickPrev = dataSyncCaptureTick;
  }

  if (dataGpsPpsCaptureTick != dataGpsPpsCaptureTickPrev) {
    snprintf(buffer, sizeof(buffer), "   gps  delta: %d ms\r\n",
             captureDeltaUs(dataGpsPpsCapture, dataMonCapture) / 1000);
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    dataGpsPpsCaptureTickPrev = dataGpsPpsCaptureTick;
  }
  
  if (gpsTimeTick != gpsTimeTickPrev) {
    snprintf(buffer, sizeof(buffer), "   gps time: %s UTC\r\n", gpsTime);
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    gpsTimeTickPrev = gpsTimeTick;
  }

#if 1  // TODO
  if (dataExtClkCountTick != dataExtClkCountTickPrev) {
    snprintf(buffer, sizeof(buffer), "   ext time: %ld s\r\n",
             dataExtClkCount );
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    dataExtClkCountTickPrev = dataExtClkCountTick;
  }
#endif

#if 1 // handle AUX stuff 
  if ( (dataAuxMonCaptureTick != dataAuxMonCaptureTickPrev ) && ( dataExtClkCount != dataExtClkCountMonPrev ) )  {
    snprintf(buffer, sizeof(buffer), "   Aux Mon: external time %lu S %lu.%lu uS\r\n",
             dataExtClkCount,dataAuxMonCapture/10 , dataAuxMonCapture%10 );
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    dataAuxMonCaptureTickPrev = dataAuxMonCaptureTick;
    dataExtClkCountMonPrev = dataExtClkCount;
  }
#endif

  HAL_Delay(10);

  loopCount++;
}
