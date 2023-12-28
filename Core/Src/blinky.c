// Copyright (c) 2023 Cullen Jennings

#include <math.h>  // for round
#include <stdio.h>

#include <string.h>

#include "audio.h"
#include "blink.h"
#include "detect.h"
#include "gps.h"
#include "pps.h"
#include "ltc.h"
#include "main.h"

#include "hardware.h"


// Uses Semantic versioning https://semver.org/
// major.minor.patch,
// patch=year/month/day
const char *version = "0.90.231227";

// This structure is saved in EEPROM
// keep size padded to 32 bits
typedef struct {
  uint8_t version;   // 1
  uint8_t product;   // 1=blink, 2=clock
  uint8_t revMajor;  // 0x1 is Rev A
  uint8_t revMinor;  // start at 0

  uint16_t serialNum;  // 0 is not valid

  int16_t oscAdj;      // offset for intenal oscilator counter
  uint16_t vcoValue;   // value loaded in DAC for VCO
  uint8_t extOscType;  // external osc type ( 2= 2.048 MHz, 10=10 MHz,
                       // 0=Internal ) )
} Config;
static Config config;

// #define captureFreqHz 2048000ul
//  next macro must have capture2uS( captureFreqHz ) fit in 32 bit calculation
// #define capture2uS(c) ((c) * 1000ul / 2048ul)

// The main timer counter max value (typically 10 MHz ) * M need this cacluation
// to fit in 32 bits
uint32_t capture2uSRatioM = 125;
uint32_t capture2uSRatioN = 256;
inline uint32_t capture2uS(const uint32_t c) {
  return (c * capture2uSRatioM) / capture2uSRatioN;
}

inline uint32_t extCapture2uS(const uint32_t c) {
  return c / 10l;
}

uint8_t blinkMute = 1;       // mutes audio outout
uint8_t blinkBlank = 1;      // causes LED to be off
uint8_t blinkDispAudio = 0;  // caused audio latency to be displayed on LED

uint8_t blinkHaveDisplay = 1;

uint32_t dataSyncCapture;
uint32_t dataSyncCaptureTick;
uint32_t dataLtcCapture;
uint32_t dataLtcCaptureTick;
uint32_t dataLtcGenTick;

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
    uint32_t monUs = capture2uS(dataMonCapture);
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
      if ((dataSyncCaptureTick < tick) &&
          (tick < dataSyncCaptureTick + 100 /*ms*/)) {
        // supress this tick
      } else {
        dataSyncCapture = HAL_TIM_ReadCapturedValue(htim, TimeSync_CH_SYNC_IN);
        dataSyncCaptureTick = tick;
       }
    }

    if (htim->Channel ==
        TimeSync_HAL_CH_SYNC_MON) {  // sync mon falling edge. falling is rising
                                     // on inverted output

      // supress for 100 ms after seeing first edge
      if ((dataMonCaptureTick < tick) &&
          (tick < dataMonCaptureTick + 100 /*ms*/)) {
        // supress this tick
      } else {
        dataMonCapture = HAL_TIM_ReadCapturedValue(htim, TimeSync_CH_SYNC_MON);
        dataMonCaptureTick = tick;
      }
    }

    if (htim->Channel == TimeSync_HAL_CH_GPS_PPS) {  // sync in on falling edge.
                                                     // falling is rising
                                                     // on inverted input
      dataGpsPpsCapture = HAL_TIM_ReadCapturedValue(htim, TimeSync_CH_GPS_PPS);
      dataGpsPpsCaptureTick = tick;
    }
  }

  if (htim == &hTimeAux) {
    if (htim->Channel ==
        TimeAux_HAL_CH_SYNC_MON) {  // sync mon falling edge. falling is rising
                                    // on inverted output

      if ((dataAuxMonCaptureTick < tick) &&
          (tick < dataAuxMonCaptureTick + 100 /*ms*/)) {
        // supress this tick
      } else {
        dataAuxMonCapture = HAL_TIM_ReadCapturedValue(htim, TimeAux_CH_SYNC_MON);
        dataAuxMonCaptureTick = tick;
      }
    }

    if (htim->Channel == TimeAux_HAL_CH_GPS_PPS) {  // sync in on falling edge.
                                                    // falling is rising
                                                    // on inverted input
      dataAuxGpsPpsCapture =
          HAL_TIM_ReadCapturedValue(htim, TimeAux_CH_GPS_PPS);
      dataAuxGpsPpsCaptureTick = tick;
    }
  }
}

void blinkInit() {
  dataMonCapture = 0;
  dataMonCaptureTick = 0;
  dataSyncCapture = 0;
  dataSyncCaptureTick = 0;
  dataGpsPpsCapture = 0;
  dataGpsPpsCaptureTick = 0;
  dataExtClkCount = 0;
  dataExtClkCountTickOffset = -1000;  // TODO
  dataNextSyncOutPhaseUS = 10l * 1000l;
  dataCurrentSyncOutPhaseUS = dataNextSyncOutPhaseUS;
  // subFrameCount = 0;
  // subFrameCountOffset = 120;

  gpsInit();

  blinkLocalSeconds = 0;

  audioInit();
  ppsInit();
  
  LtcTransitionSetClear(&ltcSendTransitions);
  LtcTransitionSetClear(&ltcRecvTransitions);

  dataSyncCaptureTick = 0;
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

void setClk(uint8_t extOscTypeType, uint16_t vcoValue, int16_t oscAdj) {
  char buffer[100];

  if (extOscTypeType == 2) {
    // External CLK is 2.048 Mhz
    __HAL_TIM_SET_AUTORELOAD(&hTimeSync, 2048ul * 1000ul - 1ul);
    capture2uSRatioM = 125;
    capture2uSRatioN = 256;

    snprintf(buffer, sizeof(buffer), "  External clock set to 2.048MHz \r\n");
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
  } else if (extOscTypeType == 10) {
    // External CLK is 10 Mhz
    __HAL_TIM_SET_AUTORELOAD(&hTimeSync, 10ul * 1000ul * 1000ul - 1ul);
    capture2uSRatioM = 1;
    capture2uSRatioN = 10;

    snprintf(buffer, sizeof(buffer), "  External clock set to 10MHz \r\n");
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
  } else if (extOscTypeType == 0) {
    // CLK is internal 84 Mhz
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};

    // oscAdj=-534; // -17 to +2,  delta 90 ,  , serial# 6 Nov 26, 2023
    // oscAdj=-542; // 65,  delta 90 ,  , serial# 3 Nov 26, 2023

    htim2.Init.Prescaler = 8 - 1;
    htim2.Init.Period = 10500ul * 1000ul - 1ul + oscAdj;
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    capture2uSRatioM = 2;
    capture2uSRatioN = 21;

    if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK) {
      Error_Handler();
    }
    if (HAL_TIM_IC_Init(&htim2) != HAL_OK) {
      Error_Handler();
    }

    snprintf(buffer, sizeof(buffer), "  Internal clock set to 10.5MHz \r\n");
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
  }

  // vcoValue = 497; // -8 to 10 ns , delta = 30/-100 = -0.30  , serial# 6 Nov
  // 26, 2023

  HAL_DAC_Start(&hDAC, DAC_CH_OSC_ADJ);
  HAL_DAC_SetValue(&hDAC, DAC_CHANNEL_1, DAC_ALIGN_12B_R, vcoValue);

  snprintf(buffer, sizeof(buffer), "  VCO: %u\r\n", vcoValue);
  HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
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

  // access Temperature Sensor
  if (1) {
    char buffer[100];
    const uint16_t i2cAddr = 0x48;
    uint32_t timeout = 256;
    uint8_t tempAddr = 0x0;
    uint16_t temp = 0;

    uint8_t configAddr = 0x1;
    uint16_t config = 0;
    HAL_StatusTypeDef status;

    status = HAL_I2C_IsDeviceReady(&hI2c, i2cAddr << 1, 2, timeout);
    if (status != HAL_OK) {
      snprintf(buffer, sizeof(buffer),
               "Error: Temperature sensor not found \r\n");
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    config = 0xA060;    // power up default
    config &= ~0x1000;  // turn off extended mode (  only needed for temps above
                        // 128 C )
    uint16_t convRate = 1;  // 0=0.25Hz, 1=1Hz, 2=4Hz, 3=8Hz
    config |= (config & 0x3FFF) | (convRate << 6);  // set coversion rate

    status = HAL_I2C_Mem_Write(&hI2c, i2cAddr << 1, configAddr,
                               sizeof(configAddr), (uint8_t *)&config,
                               (uint16_t)sizeof(config), timeout);
    if (status != HAL_OK) {
      // stat: 0=0k, 1 is HAL_ERROR, 2=busy , 3 = timeout
      snprintf(buffer, sizeof(buffer),
               "Temperature Write Error:  data hal error %d \r\n", status);
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    HAL_Delay(1100 /*ms */);  // TODO remove

    status =
        HAL_I2C_Mem_Read(&hI2c, i2cAddr << 1, tempAddr, sizeof(tempAddr),
                         (uint8_t *)&temp, (uint16_t)sizeof(temp), timeout);
    if (status != HAL_OK) {
      // stat: 0=0k, 1 is HAL_ERROR, 2=busy , 3 = timeout
      snprintf(buffer, sizeof(buffer),
               "Temperature Read Error:  data hal error %d \r\n", status);
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    } else {
      temp = ((temp & 0x00FF) << 8) | ((temp & 0xFF00) >> 8);  // swap bytes
      temp = temp >> 4;  // shift to be 12 bits

      float tempC;

      if (temp & 0x800) {
        // negative temp
        float count = temp & 0x7FF;  // bottom 11 bits
        tempC = -0.0625 * count;
      } else {
        // positive temp
        float count = temp & 0x7FF;  // bottom 11 bits
        tempC = 0.0625 * count;
      }

      int tempDeciC = round(tempC * 10.0);
      snprintf(buffer, sizeof(buffer), "  Temperature: %d.%dC \r\n",
               tempDeciC / 10, tempDeciC % 10);
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }
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
      config.version = 1;
      config.product = 2;  // 1=blink, 2=clock
      config.revMajor = 0;
      config.revMinor = 9;
      config.serialNum = 8;

      config.extOscType = 0;
      config.oscAdj = -658;
      config.vcoValue = 2000;

      status = HAL_I2C_Mem_Write(&hI2c, i2cAddr << 1, eepromMemAddr,
                                 sizeof(eepromMemAddr), (uint8_t *)&config,
                                 (uint16_t)sizeof(config), timeout);
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

    status = HAL_I2C_Mem_Read(&hI2c, i2cAddr << 1, eepromMemAddr,
                              sizeof(eepromMemAddr), (uint8_t *)&config,
                              (uint16_t)sizeof(config), timeout);
    if (status != HAL_OK) {
      // stat: 0=0k, 1 is HAL_ERROR, 2=busy , 3 = timeout
      snprintf(buffer, sizeof(buffer),
               "EEProm Read Error:  data hal error %d \r\n", status);
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if ((config.version < 1) || (config.version > 10)) {
      snprintf(buffer, sizeof(buffer), "EEProm not initalized: %d \r\n",
               config.version);
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);

      Error_Handler();
    }

    if ((config.revMajor == 0) && (config.revMinor == 9)) {
      snprintf(buffer, sizeof(buffer), "  Hardware version: EV9 \r\n");
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);

      setClk(config.extOscType, config.vcoValue, config.oscAdj);

      if (config.product == 1) {
        // This is blink board

        // reconfigure ext_clk to be input buton 2
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        GPIO_InitStruct.Pin = AUX_CLK_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_PULLDOWN;
        HAL_GPIO_Init(AUX_CLK_GPIO_Port, &GPIO_InitStruct);
      }

      if (config.product == 2) {
        // This is clock board

        // turn off options
        blinkHaveDisplay = 0;

        // turn off audio and display
        blinkMute = 1;
        blinkBlank = 1;

        // enable 5V power supply on LED1
        // TODO - make this based on if USB has enough power
        HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
      }

    } else {
      snprintf(buffer, sizeof(buffer), "Unknown hardware version %d.%d\r\n",
               config.revMajor, config.revMinor);
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);

      Error_Handler();
    }

    snprintf(buffer, sizeof(buffer), "  Serial: %d\r\n", config.serialNum);
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
  }

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
  // seeing value of 0 if fliped wrong way
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

  static uint32_t dataSyncCaptureTickPrev = 0;
  static uint32_t dataMonCaptureTickPrev = 0;
  static uint32_t dataLtcCaptureTickPrev = 0;
  static uint32_t dataLtcGenTickPrev = 0;
  //static uint32_t dataExtClkCountTickPrev = 0;
  static uint32_t dataGpsPpsCaptureTickPrev = 0;
  static uint32_t dataAuxMonCaptureTickPrev = 0;
  static uint32_t gpsTimeTickPrev = 0;
  static uint32_t dataExtClkCountMonPrev = 0;

  char buffer[100];

  uint32_t tick = HAL_GetTick();

  if (loopCount % 100 == 0) {
    snprintf(buffer, sizeof(buffer), "Loop %d\r\n", loopCount);
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);

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

      if ((tick > 2000) && (dataSyncCaptureTick + 2000 > tick)) {
        //  had sync in last 2 seconds
        int32_t deltaPhaseUS = capture2uS(dataSyncCapture) - capture2uS(dataMonCapture);
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
                 capture2uS(dataSyncCapture)/1000l, capture2uS(dataMonCapture)/1000l,
                 prePhaseUS/1000l, phaseUS/1000l  );
        HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
        
      }
      else if ((tick > 2000) && (dataGpsPpsCaptureTick + 2000 > tick))  {
        //  had GPS sync in last 2 seconds
        int32_t deltaPhaseUS = capture2uS(dataGpsPpsCapture) - capture2uS(dataMonCapture);
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
                 capture2uS(dataGpsPpsCapture)/1000l, capture2uS(dataMonCapture)/1000l,
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
        blinkAudioDelayMs = captureDeltaUs(mltTime, dataMonCapture) / 1000l -
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
   
   if (dataSyncCaptureTick != dataSyncCaptureTickPrev) {
      dataSyncCaptureTickPrev = dataSyncCaptureTick;
      
      snprintf(buffer, sizeof(buffer), "   sync phase=%lums delta2mon=%ldms\r\n",
        capture2uS(dataSyncCapture)/1000l, captureDeltaUs(dataSyncCapture, dataMonCapture)/1000l );
      HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    }
  

  if (dataMonCaptureTick != dataMonCaptureTickPrev) {
#if 0 // to much print out 
    snprintf(buffer, sizeof(buffer), "   mon phase: %lums\r\n",
        capture2uS(dataMonCapture )/1000l);
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
#endif
    
    dataMonCaptureTickPrev = dataMonCaptureTick;
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

  if (dataGpsPpsCaptureTick != dataGpsPpsCaptureTickPrev) {
    snprintf(buffer, sizeof(buffer), "   gps delta: %ldms\r\n",
             captureDeltaUs(dataGpsPpsCapture, dataMonCapture) / 1000);
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    dataGpsPpsCaptureTickPrev = dataGpsPpsCaptureTick;
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
  if ((dataAuxMonCaptureTick != dataAuxMonCaptureTickPrev) &&
      (dataExtClkCount != dataExtClkCountMonPrev)) {
    snprintf(buffer, sizeof(buffer),
             "   Aux mon: external time %lu.%03lums\r\n", 
             extCapture2uS(dataAuxMonCapture) / 1000l, extCapture2uS(dataAuxMonCapture) % 1000l);
    HAL_UART_Transmit(&hUartDebug, (uint8_t *)buffer, strlen(buffer), 1000);
    dataAuxMonCaptureTickPrev = dataAuxMonCaptureTick;
    dataExtClkCountMonPrev = dataExtClkCount;
  }
#endif
  
  HAL_Delay(10);

  loopCount++;
}
