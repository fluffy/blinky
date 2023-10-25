// Copyright (c) 2023 Cullen Jennings

#include <stdio.h>
#include <stm32f4xx_ll_tim.h>
#include <string.h>

#include "blink.h"
#include "main.h"

extern ADC_HandleTypeDef hadc1;

extern DAC_HandleTypeDef hdac;

extern I2C_HandleTypeDef hi2c1;

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim5;
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

#define hTimeBlink htim4

#define hTimeAux htim5
// #define TimeAux_CH_AUX_CLK TIM_CHANNEL_1
// #define TimeAux_CH_AUX_GPS_PPS TIM_CHANNEL_2
#define TimeAux_CH_AUX_SYNC_MON TIM_CHANNEL_3

#define hTimeLtc htim8
#define TimeLtc_CH_SYNC_IN2 TIM_CHANNEL_1

const char *version = "0.60.231023";  // major , minor, year/month/day

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

uint32_t dataMonCapture;
uint32_t dataMonCaptureTick;
uint32_t dataSyncCapture;
uint32_t dataSyncCaptureTick;
uint32_t dataGpsPpsCapture;
uint32_t dataGpsPpsCaptureTick;

uint32_t dataExtClkCount;  // counting seconds
uint32_t dataExtClkCountTick;
int32_t dataExtClkCountTickOffset;

uint16_t dataNextSyncOutPhase;
uint16_t dataCurrentPhaseSyncOut;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  uint32_t tick = HAL_GetTick();

#if 0  // TODO 
  if (htim == &hTimeSync) {
    // HAL_GPIO_TogglePin(DB1_GPIO_Port, DB1_Pin);  // toggle DB1 LED

    dataExtClkCount++;
    dataExtClkCountTick = tick;
  }
#endif

  if (htim == &hTimeBlink) {
    // This block of code takes 1.9 uS and runs every 1 mS
    // HAL_GPIO_WritePin(DB2_GPIO_Port, DB2_Pin, GPIO_PIN_SET);

    uint32_t mainCapture = __HAL_TIM_GetCounter(&hTimeSync);
    uint32_t mainUs = capture2uS(mainCapture);
    uint32_t monUs = capture2uS(dataMonCapture);
    int32_t ledUs = mainUs - monUs;
    if (ledUs < 0) {
      ledUs += 1000000ul;
    }

    // uS to 240 Hz
    uint32_t subFrameCount = (240ul * ledUs) / (1000ul * 1000ul);

    if (subFrameCount >= 240) {
      subFrameCount -= 240;
    }

    int16_t gridCount =
        subFrameCount;  // counting up in 1/8 of 30 frames per second
    int16_t binCount = subFrameCount / 8;  // counting up in frames at 30 fps

    if (1) {
      int row = 1 + ((gridCount) % 8);
      int col = 1 + ((gridCount / 8) % 5);

      HAL_GPIO_WritePin(ROW1_GPIO_Port, ROW1_Pin,
                        (row == 5) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(ROW2_GPIO_Port, ROW2_Pin,
                        (row == 4) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(ROW3_GPIO_Port, ROW3_Pin,
                        (row == 3) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(ROW4_GPIO_Port, ROW4_Pin,
                        (row == 2) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(ROW5_GPIO_Port, ROW5_Pin,
                        (row == 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(ROW6_GPIO_Port, ROW6_Pin,
                        (row == 6) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(ROW7_GPIO_Port, ROW7_Pin,
                        (row == 7) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(ROW8_GPIO_Port, ROW8_Pin,
                        (row == 8) ? GPIO_PIN_SET : GPIO_PIN_RESET);

      HAL_GPIO_WritePin(COL1_GPIO_Port, COL1_Pin,
                        (col == 2) ? GPIO_PIN_RESET : GPIO_PIN_SET);
      HAL_GPIO_WritePin(COL2_GPIO_Port, COL2_Pin,
                        (col == 1) ? GPIO_PIN_RESET : GPIO_PIN_SET);
      HAL_GPIO_WritePin(COL3_GPIO_Port, COL3_Pin,
                        (col == 5) ? GPIO_PIN_RESET : GPIO_PIN_SET);
      HAL_GPIO_WritePin(COL4_GPIO_Port, COL4_Pin,
                        (col == 3) ? GPIO_PIN_RESET : GPIO_PIN_SET);
      HAL_GPIO_WritePin(COL5_GPIO_Port, COL5_Pin,
                        (col == 4) ? GPIO_PIN_RESET : GPIO_PIN_SET);
    }

    if (1) {
      // Low 4 bits of display
      HAL_GPIO_WritePin(LEDC_GPIO_Port, LEDC_Pin,
                        (binCount & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LEDB_GPIO_Port, LEDB_Pin,
                        (binCount & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LEDE_GPIO_Port, LEDE_Pin,
                        (binCount & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LEDD_GPIO_Port, LEDD_Pin,
                        (binCount & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);

      // High 4 bits of display
      HAL_GPIO_WritePin(LEDG_GPIO_Port, LEDG_Pin,
                        (binCount & 0x10) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LEDF_GPIO_Port, LEDF_Pin,
                        (binCount & 0x20) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LEDH_GPIO_Port, LEDH_Pin,
                        (binCount & 0x40) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LEDA_GPIO_Port, LEDA_Pin,
                        (binCount & 0x80) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }

    // HAL_GPIO_WritePin(DB2_GPIO_Port, DB2_Pin, GPIO_PIN_RESET);
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
    } else {                        // val == dataCurrentPhaseSyncOut
      // start of output pulse just started, set up for the end of pulse
      val = dataCurrentPhaseSyncOut + 200;  // 20 ms wide pulse
      if (val >= 10000) {
        val -= 10000;
      }
      __HAL_TIM_SET_COMPARE(&hTimePps, TimePps_CH_SYNC_OUT, val);
      LL_TIM_OC_SetMode(
          TIM1, TimePps_LL_CH_SYNC_OUT,
          LL_TIM_OCMODE_ACTIVE);  // inverted due to inverting output buffer
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
}

void setClk(uint8_t clk, uint8_t adj) {
  char buffer[100];

  if (clk == 2) {
    // External CLK is 2.048 Mhz
    __HAL_TIM_SET_AUTORELOAD(&hTimeSync, 2048ul * 1000ul - 1ul);
    capture2uSRatioM = 125;
    capture2uSRatioN = 256;

    snprintf(buffer, sizeof(buffer), "  External clock set to 2.048 Mhz \r\n");
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
  } else if (clk == 10) {
    // External CLK is 10 Mhz
    __HAL_TIM_SET_AUTORELOAD(&hTimeSync, 10ul * 1000ul * 1000ul - 1ul);
    capture2uSRatioM = 1;
    capture2uSRatioN = 10;

    snprintf(buffer, sizeof(buffer), "  External clock set to 10 Mhz \r\n");
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
  } else if (clk == 0) {
    // CLK is internal 84 Mhz
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};

    int32_t freqOffset = (int32_t)(adj)-100l;
    htim2.Init.Prescaler = 8 - 1;
    htim2.Init.Period = 10500ul * 1000ul - 1ul 
                        + freqOffset*10 ;  // 60+100 is abouut manual correction TOOO put in eeprom
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

  int16_t vcoOffset = (int16_t)(adj)-100;
  snprintf(buffer, sizeof(buffer), "  VCO offset: %d\r\n", vcoOffset);
  HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);

  HAL_DAC_Start(&hDAC, DAC_CH_OSC_ADJ);
  // TODO - compute slope 
  // -15 gave +118 ns on period
  // -115 gave +714 ns
  // -5 gave +63 ns
  // +5 gave +15
  // +8 gave -10
  // +6 gave +5
  // +7 gave +2 ns with range -9 to +14
  // vcoOffset = 8;
  uint16_t dacValue = 10000 + vcoOffset;
  HAL_DAC_SetValue(&hDAC, DAC_CHANNEL_1, DAC_ALIGN_12B_R, dacValue);
}

void blinkSetup() {
  HAL_GPIO_WritePin(LEDM2_GPIO_Port, LEDM2_Pin,
                    GPIO_PIN_SET);  // turn on red error LED
  HAL_GPIO_WritePin(LEDM3_GPIO_Port, LEDM3_Pin,
                    GPIO_PIN_RESET);  // turn off yellow assert LED
  HAL_GPIO_WritePin(LEDM1_GPIO_Port, LEDM1_Pin,
                    GPIO_PIN_RESET);  // turn off green ok LED

  HAL_GPIO_WritePin(LEDA_GPIO_Port, LEDA_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LEDB_GPIO_Port, LEDB_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LEDC_GPIO_Port, LEDC_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LEDD_GPIO_Port, LEDD_Pin, GPIO_PIN_RESET);

  HAL_GPIO_WritePin(LEDE_GPIO_Port, LEDE_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LEDF_GPIO_Port, LEDF_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LEDG_GPIO_Port, LEDG_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LEDH_GPIO_Port, LEDH_Pin, GPIO_PIN_RESET);

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
    uint8_t data[16];
    for (int i = 0; i < sizeof(data); i++) {
      data[i] = 0;
    }
    uint32_t timeout = 2;
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
      data[0] = 6;        // hardware version
      data[1] = 2;        // osc speed ( 2= 2.048 MHz, 10=10 MHz, 0=Internal ) )
      data[2] = 100 + 8;  // VCO voltage offset

      status = HAL_I2C_Mem_Write(&hI2c, i2cAddr << 1, eepromMemAddr,
                                 sizeof(eepromMemAddr), data,
                                 (uint16_t)sizeof(data), timeout);
      if (status != HAL_OK) {
        // stat: 0=0k, 1 is HAL_ERROR, 2=busy , 3 = timeout
        snprintf(buffer, sizeof(buffer),
                 "EEProm Write Error:  data hal error %d \r\n", status);
        HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
      }
      HAL_Delay(
          2);  // chip has 1.5 ms max page write time when it will not respond
    }

    // TODO put sleep here

    status =
        HAL_I2C_Mem_Read(&hI2c, i2cAddr << 1, eepromMemAddr,
                         sizeof(eepromMemAddr), data, (uint16_t)(3), timeout);
    if (status != HAL_OK) {
      // stat: 0=0k, 1 is HAL_ERROR, 2=busy , 3 = timeout
      snprintf(buffer, sizeof(buffer),
               "EEProm Read Error:  data hal error %d \r\n", status);
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

#if 0
        snprintf( buffer, sizeof(buffer), "EEProm: data=%d %d %d\r\n", data[0] , data[1], data[3] );
        HAL_UART_Transmit( &hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
#endif

    if (data[0] == 6) {
      // This is V6 hardware
      snprintf(buffer, sizeof(buffer), "  Hardware version: V6 \r\n");
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);

      setClk(data[1], data[2]);
    } else {
      snprintf(buffer, sizeof(buffer), "Unknown Hardware version %d \r\n",
               data[0]);
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);

      Error_Handler();
    }
  }

  HAL_TIM_Base_Start_IT(&hTimeBlink);

#if 1
  HAL_TIM_Base_Start_IT(&hTimeSync);
#endif

#if 1
  HAL_TIM_OC_Start_IT(&hTimePps, TimePps_CH_SYNC_OUT);  // start sync out
#endif

  HAL_TIM_Base_Start_IT(&hTimeSync);

#if 1
  HAL_TIM_IC_Start_IT(&hTimeSync,
                      TimeSync_CH_SYNC_IN);  // start sync in capture
#endif

#if 1
  HAL_TIM_IC_Start_IT(&hTimeSync,
                      TimeSync_CH_SYNC_MON);  // start sync mon capture
#endif

#if 0 
    HAL_TIM_IC_Start_IT( &hTmeSync, TimeSync_CH_GPS_PPS  ); // start gps pps capture
#endif

  // set LED to on but not sync ( yellow, not greeen )
  HAL_GPIO_WritePin(LEDM3_GPIO_Port, LEDM3_Pin,
                    GPIO_PIN_SET);  // turn on yellow assert LED
  HAL_GPIO_WritePin(LEDM1_GPIO_Port, LEDM1_Pin,
                    GPIO_PIN_RESET);  // turn off green ok LED

  if (1) {
    char buffer[100];
    snprintf(buffer, sizeof(buffer), "Setup Done\r\n");
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
  }
}

void blinkRun() {
  static int loopCount = 0;
  static char buttonWasPressed = 0;

  static uint32_t dataMonCaptureTickPrev = 0;
  static uint32_t dataSyncCaptureTickPrev = 0;
  static uint32_t dataExtClkCountTickPrev = 0;
  static uint32_t dataGpsPpsCaptureTickPrev = 0;

  char buffer[100];

  if (loopCount % 10 == 0) {
    snprintf(buffer, sizeof(buffer), "\r\nLoop %d \r\n", loopCount);
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
  }

#if 1
  if (!HAL_GPIO_ReadPin(BTN1_GPIO_Port, BTN1_Pin)) {
    if (!buttonWasPressed) {
      uint32_t tick = HAL_GetTick();

      snprintf(buffer, sizeof(buffer), "BTN1 press \r\n");
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);

      dataExtClkCountTickOffset = dataExtClkCountTick;
      dataExtClkCount = 0;

      if ((tick > 2000) && (dataSyncCaptureTick + 2000 >
                            tick)) {  // if had sync in last 2 seconds
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

        snprintf(buffer, sizeof(buffer), "  new phase: %ld\r\n", phase);
        HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
      } else {
        snprintf(buffer, sizeof(buffer), "  No sync input\r\n");
        HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
      }
    }
    buttonWasPressed = 1;
  } else {
    buttonWasPressed = 0;
  }
#endif

#if 0  // prints too much stuff 
    uint32_t val = __HAL_TIM_GetCounter(&hTimeSync);
    snprintf( buffer, sizeof(buffer), "Sync Time val %ld uS\r\n",  capture2uS(val) );
    HAL_UART_Transmit( &hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
#endif

  if (dataMonCaptureTick != dataMonCaptureTickPrev) {
    snprintf(buffer, sizeof(buffer), "   mon : %ld ms\r\n",
             capture2uS(dataMonCapture) / 1000);
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    dataMonCaptureTickPrev = dataMonCaptureTick;
  }

  if (dataSyncCaptureTick != dataSyncCaptureTickPrev) {
    snprintf(buffer, sizeof(buffer), "   sync: %ld ms\r\n",
             capture2uS(dataSyncCapture) / 1000);
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    dataSyncCaptureTickPrev = dataSyncCaptureTick;
  }

#if 0  // TODO 
  if (dataGpsPpsCaptureTick != dataGpsPpsCaptureTickPrev) {
    snprintf(buffer, sizeof(buffer), "   gpsPPS: %ld ms\r\n",
             capture2uS( dataGpsPpsCapture ) / 1000);
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    dataGpsPpsCaptureTickPrev = dataGpsPpsCaptureTick;
  }
#endif

#if 0  // TODO 
  if (dataExtClkCountTick != dataExtClkCountTickPrev) {
    uint32_t val = __HAL_TIM_GetCounter(&hTimeSync);
    int32_t err = dataExtClkCountTick - dataExtClkCountTickOffset -
                  dataExtClkCount * 1000l;
    snprintf(buffer, sizeof(buffer), "   time: %ld s %ld ms err: %ld ms\r\n",
             dataExtClkCount, val, err);
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    dataExtClkCountTickPrev = dataExtClkCountTick;
  }
#endif

  HAL_Delay(100);

  loopCount++;
}
