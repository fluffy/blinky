/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stm32f4xx_ll_tim.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
DAC_HandleTypeDef hdac;

I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim8;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */
const char *version = "0.2.230822";  // major , minor, year/moth/day

uint32_t dataMonCapture;
uint32_t dataMonCaptureTick;
uint32_t dataSyncCapture;
uint32_t dataSyncCaptureTick;
uint32_t dataGpsPpsCapture;
uint32_t dataGpsPpsCaptureTick;

uint32_t dataExtClkCount;
uint32_t dataExtClkCountTick;
int32_t dataExtClkCountTickOffset;

uint16_t dataNextSyncOutPhase;
uint16_t dataCurrentPhaseSyncOut;

uint32_t subFrameCount;  // counts at 240 sub frames per second, reset to 0
                         // every second
uint32_t subFrameCountOffset;  // count in subFames into the second time when
                               // the syncOut pulse happens

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM8_Init(void);
static void MX_DAC_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM3_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_TIM4_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  uint32_t tick = HAL_GetTick();

  if (htim == &htim1) {
    HAL_GPIO_TogglePin(DB1_GPIO_Port, DB1_Pin);  // toggle DB1 LED
    dataExtClkCount++;
    dataExtClkCountTick = tick;

    subFrameCount = 240 - subFrameCountOffset;
  }
  if (htim == &htim4) {
#if 1  // This block of code takes 1.9 uS and runs every 1 mS
    HAL_GPIO_WritePin(DB2_GPIO_Port, DB2_Pin, GPIO_PIN_SET);
    subFrameCount++;  // counting at rate 240 Hz
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

    HAL_GPIO_WritePin(DB2_GPIO_Port, DB2_Pin, GPIO_PIN_RESET);
#endif
  }
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
  uint32_t tick = HAL_GetTick();
  if (htim == &htim2) {
    if (htim->Channel ==
        HAL_TIM_ACTIVE_CHANNEL_1) {  // sync in falling edge. falling is rising
                                     // on inverted input
      dataSyncCapture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
      dataSyncCaptureTick = tick;
    }
    if (htim->Channel ==
        HAL_TIM_ACTIVE_CHANNEL_4) {  // sync mon falling edge. falling is rising
                                     // on inverted output
      dataMonCapture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_4);
      dataMonCaptureTick = tick;
    }
  }
#if 1
  if (htim == &htim8) {
    if (htim->Channel ==
        HAL_TIM_ACTIVE_CHANNEL_1) {  // sync in falling edge. falling is rising
                                     // on inverted input
      dataGpsPpsCapture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
      dataGpsPpsCaptureTick = tick;
    }
  }
#endif
}

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim == &htim3) {
    // HAL_GPIO_TogglePin(LEDM3_GPIO_Port, LEDM3_Pin ); // toggle ok LED

    uint16_t val = __HAL_TIM_GET_COMPARE(&htim3, TIM_CHANNEL_2);
    if (val != dataCurrentPhaseSyncOut) {
      // end of output pulse just happened, set up for next output pulse
      dataCurrentPhaseSyncOut = dataNextSyncOutPhase;
      __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, dataNextSyncOutPhase);
      LL_TIM_OC_SetMode(
          TIM3, LL_TIM_CHANNEL_CH2,
          LL_TIM_OCMODE_INACTIVE);  // inverted due to inverting output buffer
    } else {                        // val == dataCurrentPhaseSyncOut
      // start of output pulse just started, set up for the end of pulse
      val = dataCurrentPhaseSyncOut + 200;  // 20 ms wide pulse
      if (val >= 10000) {
        val -= 10000;
      }
      __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, val);
      LL_TIM_OC_SetMode(
          TIM3, LL_TIM_CHANNEL_CH2,
          LL_TIM_OCMODE_ACTIVE);  // inverted due to inverting output buffer
    }
  }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
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
  subFrameCount = 0;
  subFrameCountOffset = 120;

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM2_Init();
  MX_TIM8_Init();
  MX_DAC_Init();
  MX_I2C1_Init();
  MX_TIM1_Init();
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  MX_USART3_UART_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */

  // HAL_GPIO_WritePin(GPIOC, ROW3_Pin, GPIO_PIN_SET);
  // HAL_GPIO_WritePin(GPIOC, ROW4_Pin, GPIO_PIN_SET);
  // HAL_GPIO_WritePin(GPIOB, COL3_Pin, GPIO_PIN_RESET);

  HAL_GPIO_WritePin(LEDM1_GPIO_Port, LEDM1_Pin,
                    GPIO_PIN_SET);  // turn off error LED
  HAL_GPIO_WritePin(LEDM2_GPIO_Port, LEDM2_Pin,
                    GPIO_PIN_SET);  // turn off assert LED
  HAL_GPIO_WritePin(LEDM3_GPIO_Port, LEDM3_Pin,
                    GPIO_PIN_SET);  // turn off ok LED

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
    HAL_UART_Transmit(&huart1, (uint8_t *)buffer, strlen(buffer), 1000);
    snprintf(buffer, sizeof(buffer), "  Software version: %s\r\n", version);
    HAL_UART_Transmit(&huart1, (uint8_t *)buffer, strlen(buffer), 1000);
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

    status = HAL_I2C_IsDeviceReady(&hi2c1, i2cAddr << 1, 2, timeout);
    if (status != HAL_OK) {
      snprintf(buffer, sizeof(buffer), "Error: EEProm not found \r\n");
      HAL_UART_Transmit(&huart1, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    const int writeConfigEEProm = 0;
    if (writeConfigEEProm) {
      // write config to EEProm
      data[0] = 4;   // hardware version
      data[1] = 10;  // osc speed ( 2= 2.048 MHx, 10=10 MHz )

      status = HAL_I2C_Mem_Write(&hi2c1, i2cAddr << 1, eepromMemAddr,
                                 sizeof(eepromMemAddr), data,
                                 (uint16_t)sizeof(data), timeout);
      if (status != HAL_OK) {
        // stat: 0=0k, 1 is HAL_ERROR, 2=busy , 3 = timeout
        snprintf(buffer, sizeof(buffer),
                 "EEProm Write Error:  data hal error %d \r\n", status);
        HAL_UART_Transmit(&huart1, (uint8_t *)buffer, strlen(buffer), 1000);
      }
      HAL_Delay(
          2);  // chip has 1.5 ms max page write time when it will not respond
    }

    status =
        HAL_I2C_Mem_Read(&hi2c1, i2cAddr << 1, eepromMemAddr,
                         sizeof(eepromMemAddr), data, (uint16_t)3, timeout);
    if (status != HAL_OK) {
      // stat: 0=0k, 1 is HAL_ERROR, 2=busy , 3 = timeout
      snprintf(buffer, sizeof(buffer),
               "EEProm Read Error:  data hal error %d \r\n", status);
      HAL_UART_Transmit(&huart1, (uint8_t *)buffer, strlen(buffer), 1000);
    }

#if 0
        snprintf( buffer, sizeof(buffer), "EEProm: data=%d %d %d\r\n", data[0] , data[1], data[3] );
        HAL_UART_Transmit( &huart1, (uint8_t *)buffer, strlen(buffer), 1000);
#endif

    if (data[0] == 4) {
      // This is V4 hardware
      snprintf(buffer, sizeof(buffer), "  Hardware version: V4 \r\n");
      HAL_UART_Transmit(&huart1, (uint8_t *)buffer, strlen(buffer), 1000);

      if (data[1] == 2) {
        // External CLK is 2.048 Mhz
        __HAL_TIM_SET_AUTORELOAD(&htim1, 8192 - 1);

        snprintf(buffer, sizeof(buffer),
                 "  External clock set to 2.048 Mhz \r\n");
        HAL_UART_Transmit(&huart1, (uint8_t *)buffer, strlen(buffer), 1000);
      } else if (data[1] == 10) {
        // External CLK is 10 Mhz
        __HAL_TIM_SET_AUTORELOAD(&htim1, 40000 - 1);

        snprintf(buffer, sizeof(buffer), "  External clock set to 10 Mhz \r\n");
        HAL_UART_Transmit(&huart1, (uint8_t *)buffer, strlen(buffer), 1000);
      }
    } else {
      snprintf(buffer, sizeof(buffer), "Unknown Hardware version %d \r\n",
               data[0]);
      HAL_UART_Transmit(&huart1, (uint8_t *)buffer, strlen(buffer), 1000);

      Error_Handler();
    }
  }

  HAL_TIM_Base_Start_IT(&htim4);
  HAL_TIM_Base_Start_IT(&htim1);
  //__HAL_TIM_ENABLE_IT( &htim1, TIM_IT_UPDATE );
#if 0
    // this does not seem to be needed
    HAL_TIM_Base_Start_IT(&htim2);
    HAL_TIM_Base_Start_IT(&htim3);
    HAL_TIM_Base_Start_IT(&htim8);
#endif

  HAL_TIM_OC_Start_IT(&htim3, TIM_CHANNEL_2);  // start sync out

  // HAL_TIM_Base_Start_IT(&htim2);
  HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);  // start sync in capture
  HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_4);  // start sync mon capture
#if 0
    // starting this send capture intruts into solid loop - TODO FIX
    HAL_TIM_IC_Start_IT( &htim8, TIM_CHANNEL_1 ); // start gps pps capture
#endif

    HAL_DAC_Start( &hdac , DAC_CHANNEL_1 );
    uint16_t dacValue = 10000-15; 
    HAL_DAC_SetValue(&hdac,DAC_CHANNEL_1,DAC_ALIGN_12B_R,dacValue);

    /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  HAL_GPIO_WritePin(LEDM3_GPIO_Port, LEDM3_Pin,
                    GPIO_PIN_RESET);  // turn on ok LED

  int loopCount = 0;
  char buttonWasPressed = 0;
  uint32_t dataMonCaptureTickPrev = 0;
  uint32_t dataSyncCaptureTickPrev = 0;
  uint32_t dataExtClkCountTickPrev = 0;
  uint32_t dataGpsPpsCaptureTickPrev = 0;

  while (1) {
    char buffer[100];

    if (loopCount % 10 == 0) {
      snprintf(buffer, sizeof(buffer), "\r\nLoop %d \r\n", loopCount);
      HAL_UART_Transmit(&huart1, (uint8_t *)buffer, strlen(buffer), 1000);
    }

    if (!HAL_GPIO_ReadPin(BTN1_GPIO_Port, BTN1_Pin)) {
      if (!buttonWasPressed) {
        uint32_t tick = HAL_GetTick();

        snprintf(buffer, sizeof(buffer), "BTN1 press \r\n");
        HAL_UART_Transmit(&huart1, (uint8_t *)buffer, strlen(buffer), 1000);

        dataExtClkCountTickOffset = dataExtClkCountTick;
        dataExtClkCount = 0;

        if ((tick > 2000) && (dataSyncCaptureTick + 2000 >
                              tick)) {  // if had sync in last 2 seconds
          int32_t deltaPhaseUs = dataSyncCapture - dataMonCapture;
          int32_t deltaPhase =
              deltaPhaseUs /
              100l;  // div 100 for 1MHz to 10KHz counter conversion
          if (deltaPhase < 0) deltaPhase += 10000;
          uint32_t phase = dataNextSyncOutPhase + deltaPhase;
          if (phase >= 10000) {
            phase -= 10000;
          }

          dataNextSyncOutPhase = phase;

          snprintf(buffer, sizeof(buffer), "  new phase: %ld\r\n", phase);
          HAL_UART_Transmit(&huart1, (uint8_t *)buffer, strlen(buffer), 1000);
        } else {
          snprintf(buffer, sizeof(buffer), "  No sync input\r\n");
          HAL_UART_Transmit(&huart1, (uint8_t *)buffer, strlen(buffer), 1000);
        }

        // set offset for the  LED  ( dataNextSyncOutPhase update 10KHz,
        // subFrameCountOffset update 240 Hx )
        subFrameCountOffset =
            ((uint32_t)dataNextSyncOutPhase * 3l) / 125l;  // ratio of 240/10000
        subFrameCountOffset += 2;
        if (subFrameCountOffset < 0) {
          subFrameCountOffset += 240;
        }
        if (subFrameCountOffset >= 240) {
          subFrameCountOffset -= 240;
        }
      }
      buttonWasPressed = 1;
    } else {
      buttonWasPressed = 0;
    }

    // uint32_t val = __HAL_TIM_GetCounter(&htim2);
    // snprintf( buffer, sizeof(buffer), "val %ld \r\n", val/1000 );
    // HAL_UART_Transmit( &huart1, (uint8_t *)buffer, strlen(buffer), 1000);

    if (dataMonCaptureTick != dataMonCaptureTickPrev) {
      snprintf(buffer, sizeof(buffer), "   mon : %ld ms\r\n",
               dataMonCapture / 1000);
      HAL_UART_Transmit(&huart1, (uint8_t *)buffer, strlen(buffer), 1000);
      dataMonCaptureTickPrev = dataMonCaptureTick;
    }

    if (dataSyncCaptureTick != dataSyncCaptureTickPrev) {
      snprintf(buffer, sizeof(buffer), "   sync: %ld ms\r\n",
               dataSyncCapture / 1000);
      HAL_UART_Transmit(&huart1, (uint8_t *)buffer, strlen(buffer), 1000);
      dataSyncCaptureTickPrev = dataSyncCaptureTick;
    }

#if 1
    if (dataGpsPpsCaptureTick != dataGpsPpsCaptureTickPrev) {
      snprintf(buffer, sizeof(buffer), "   gpsPPS: %ld ms\r\n",
               dataGpsPpsCapture / 10);
      HAL_UART_Transmit(&huart1, (uint8_t *)buffer, strlen(buffer), 1000);
      dataSyncCaptureTickPrev = dataSyncCaptureTick;
    }
#endif

#if 1
    if (dataExtClkCountTick != dataExtClkCountTickPrev) {
      uint32_t val = __HAL_TIM_GetCounter(&htim1);
      int32_t err = dataExtClkCountTick - dataExtClkCountTickOffset -
                    dataExtClkCount * 1000l;
      snprintf(buffer, sizeof(buffer), "   time: %ld s %ld ms err: %ld ms\r\n",
               dataExtClkCount, val, err);
      HAL_UART_Transmit(&huart1, (uint8_t *)buffer, strlen(buffer), 1000);
      dataExtClkCountTickPrev = dataExtClkCountTick;

      // snprintf( buffer, sizeof(buffer), "   gridCount: %ld offset=%ld \r\n",
      // dataGridCount,dataGridCountOffset); HAL_UART_Transmit( &huart1,
      // (uint8_t *)buffer, strlen(buffer), 1000);
    }
#endif

    HAL_Delay(100);

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    loopCount++;
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief DAC Initialization Function
  * @param None
  * @retval None
  */
static void MX_DAC_Init(void)
{

  /* USER CODE BEGIN DAC_Init 0 */

  /* USER CODE END DAC_Init 0 */

  DAC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN DAC_Init 1 */

  /* USER CODE END DAC_Init 1 */

  /** DAC Initialization
  */
  hdac.Instance = DAC;
  if (HAL_DAC_Init(&hdac) != HAL_OK)
  {
    Error_Handler();
  }

  /** DAC channel OUT1 config
  */
  sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  if (HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DAC_Init 2 */

  /* USER CODE END DAC_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_SlaveConfigTypeDef sSlaveConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 500-1;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 40000-1;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sSlaveConfig.SlaveMode = TIM_SLAVEMODE_EXTERNAL1;
  sSlaveConfig.InputTrigger = TIM_TS_TI1F_ED;
  sSlaveConfig.TriggerFilter = 0;
  if (HAL_TIM_SlaveConfigSynchro(&htim1, &sSlaveConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_SlaveConfigTypeDef sSlaveConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_IC_InitTypeDef sConfigIC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 84-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 1000000-1;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_IC_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sSlaveConfig.SlaveMode = TIM_SLAVEMODE_DISABLE;
  sSlaveConfig.InputTrigger = TIM_TS_ITR0;
  if (HAL_TIM_SlaveConfigSynchro(&htim2, &sSlaveConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_SlaveConfigTypeDef sSlaveConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 8400-1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 15000-1;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_OC_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sSlaveConfig.SlaveMode = TIM_SLAVEMODE_RESET;
  sSlaveConfig.InputTrigger = TIM_TS_ITR0;
  if (HAL_TIM_SlaveConfigSynchro(&htim3, &sSlaveConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_TOGGLE;
  sConfigOC.Pulse = 2000;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_OC_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_SlaveConfigTypeDef sSlaveConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 3500-1;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 100-1;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sSlaveConfig.SlaveMode = TIM_SLAVEMODE_RESET;
  sSlaveConfig.InputTrigger = TIM_TS_ITR0;
  if (HAL_TIM_SlaveConfigSynchro(&htim4, &sSlaveConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

}

/**
  * @brief TIM8 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM8_Init(void)
{

  /* USER CODE BEGIN TIM8_Init 0 */

  /* USER CODE END TIM8_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_SlaveConfigTypeDef sSlaveConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_IC_InitTypeDef sConfigIC = {0};

  /* USER CODE BEGIN TIM8_Init 1 */

  /* USER CODE END TIM8_Init 1 */
  htim8.Instance = TIM8;
  htim8.Init.Prescaler = 8400-1;
  htim8.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim8.Init.Period = 15000-1;
  htim8.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim8.Init.RepetitionCounter = 0;
  htim8.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim8) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim8, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_IC_Init(&htim8) != HAL_OK)
  {
    Error_Handler();
  }
  sSlaveConfig.SlaveMode = TIM_SLAVEMODE_RESET;
  sSlaveConfig.InputTrigger = TIM_TS_ITR0;
  if (HAL_TIM_SlaveConfigSynchro(&htim8, &sSlaveConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim8, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if (HAL_TIM_IC_ConfigChannel(&htim8, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM8_Init 2 */

  /* USER CODE END TIM8_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, ROW6_Pin|ROW7_Pin|ROW8_Pin|ROW5_Pin
                          |ROW3_Pin|ROW2_Pin|ROW1_Pin|DB4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LEDM3_Pin|LEDM1_Pin|LEDM2_Pin|LEDD_Pin
                          |LEDC_Pin|LEDB_Pin|LEDA_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(COL3_GPIO_Port, COL3_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, COL5_Pin|COL4_Pin|COL2_Pin|COL1_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LEDH_Pin|LEDG_Pin|LEDF_Pin|ROW4_Pin
                          |DB2_Pin|DB1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LEDE_GPIO_Port, LEDE_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(DB3_GPIO_Port, DB3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : BTN1_Pin */
  GPIO_InitStruct.Pin = BTN1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(BTN1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : ROW6_Pin ROW7_Pin ROW8_Pin ROW5_Pin
                           COL3_Pin ROW3_Pin ROW2_Pin ROW1_Pin */
  GPIO_InitStruct.Pin = ROW6_Pin|ROW7_Pin|ROW8_Pin|ROW5_Pin
                          |COL3_Pin|ROW3_Pin|ROW2_Pin|ROW1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : LEDM3_Pin LEDM1_Pin LEDM2_Pin LEDE_Pin
                           LEDD_Pin LEDC_Pin LEDB_Pin LEDA_Pin */
  GPIO_InitStruct.Pin = LEDM3_Pin|LEDM1_Pin|LEDM2_Pin|LEDE_Pin
                          |LEDD_Pin|LEDC_Pin|LEDB_Pin|LEDA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : COL5_Pin COL4_Pin COL2_Pin COL1_Pin
                           LEDH_Pin LEDG_Pin LEDF_Pin ROW4_Pin */
  GPIO_InitStruct.Pin = COL5_Pin|COL4_Pin|COL2_Pin|COL1_Pin
                          |LEDH_Pin|LEDG_Pin|LEDF_Pin|ROW4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : BOOT1_Pin */
  GPIO_InitStruct.Pin = BOOT1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BOOT1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : DB4_Pin */
  GPIO_InitStruct.Pin = DB4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(DB4_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : DB3_Pin */
  GPIO_InitStruct.Pin = DB3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(DB3_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : DB2_Pin DB1_Pin */
  GPIO_InitStruct.Pin = DB2_Pin|DB1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  HAL_GPIO_WritePin(LEDM1_GPIO_Port, LEDM1_Pin,
                    GPIO_PIN_RESET);  // turn on error LED
  HAL_GPIO_WritePin(LEDM2_GPIO_Port, LEDM2_Pin,
                    GPIO_PIN_SET);  // turn off assert LED
  HAL_GPIO_WritePin(LEDM3_GPIO_Port, LEDM3_Pin,
                    GPIO_PIN_SET);  // turn off ok LED
  while (1) {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line
     number, ex: printf("Wrong parameters value: file %s on line %d\r\n", file,
     line) */
  __disable_irq();
  HAL_GPIO_WritePin(LEDM1_GPIO_Port, LEDM1_Pin,
                    GPIO_PIN_SET);  // turn off error LED
  HAL_GPIO_WritePin(LEDM2_GPIO_Port, LEDM2_Pin,
                    GPIO_PIN_RESET);  // turn on assert LED
  HAL_GPIO_WritePin(LEDM3_GPIO_Port, LEDM3_Pin,
                    GPIO_PIN_SET);  // turn off ok LED
  while (1) {
  }
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
