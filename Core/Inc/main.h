/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SYNC_OUT_FX 1000000
#define SYNC_OUT_INTR htim7
#define HSE_IN_Pin GPIO_PIN_0
#define HSE_IN_GPIO_Port GPIOH
#define HSE_OUT_Pin GPIO_PIN_1
#define HSE_OUT_GPIO_Port GPIOH
#define LEDM3_Pin GPIO_PIN_0
#define LEDM3_GPIO_Port GPIOA
#define LEDM1_Pin GPIO_PIN_1
#define LEDM1_GPIO_Port GPIOA
#define LEDM2_Pin GPIO_PIN_2
#define LEDM2_GPIO_Port GPIOA
#define OSC_ADJ_Pin GPIO_PIN_4
#define OSC_ADJ_GPIO_Port GPIOA
#define SYNC_IN_Pin GPIO_PIN_5
#define SYNC_IN_GPIO_Port GPIOA
#define SYNC_OUT_Pin GPIO_PIN_7
#define SYNC_OUT_GPIO_Port GPIOA
#define GPS_PPS_Pin GPIO_PIN_6
#define GPS_PPS_GPIO_Port GPIOC
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWDCLK_Pin GPIO_PIN_14
#define SWDCLK_GPIO_Port GPIOA
#define LEDA_Pin GPIO_PIN_15
#define LEDA_GPIO_Port GPIOA
#define GPS_RX1_Pin GPIO_PIN_10
#define GPS_RX1_GPIO_Port GPIOC
#define GPS_TX1_Pin GPIO_PIN_11
#define GPS_TX1_GPIO_Port GPIOC
#define DB4_Pin GPIO_PIN_12
#define DB4_GPIO_Port GPIOC
#define DB3_Pin GPIO_PIN_2
#define DB3_GPIO_Port GPIOD
#define DB2_Pin GPIO_PIN_4
#define DB2_GPIO_Port GPIOB
#define DB1_Pin GPIO_PIN_5
#define DB1_GPIO_Port GPIOB
#define SCL_Pin GPIO_PIN_8
#define SCL_GPIO_Port GPIOB
#define SDA_Pin GPIO_PIN_9
#define SDA_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
