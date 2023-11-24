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
#define BTN1_Pin GPIO_PIN_13
#define BTN1_GPIO_Port GPIOC
#define NCOL10_Pin GPIO_PIN_14
#define NCOL10_GPIO_Port GPIOC
#define NROW1_Pin GPIO_PIN_15
#define NROW1_GPIO_Port GPIOC
#define HSE_IN_Pin GPIO_PIN_0
#define HSE_IN_GPIO_Port GPIOH
#define HSE_OUT_Pin GPIO_PIN_1
#define HSE_OUT_GPIO_Port GPIOH
#define AUD_IN_Pin GPIO_PIN_0
#define AUD_IN_GPIO_Port GPIOC
#define NROW2_Pin GPIO_PIN_1
#define NROW2_GPIO_Port GPIOC
#define NROW3_Pin GPIO_PIN_2
#define NROW3_GPIO_Port GPIOC
#define NROW4_Pin GPIO_PIN_3
#define NROW4_GPIO_Port GPIOC
#define AUX_CLK_Pin GPIO_PIN_0
#define AUX_CLK_GPIO_Port GPIOA
#define AUX_GPS_PPS_Pin GPIO_PIN_1
#define AUX_GPS_PPS_GPIO_Port GPIOA
#define AUX_SYNC_MON_Pin GPIO_PIN_2
#define AUX_SYNC_MON_GPIO_Port GPIOA
#define LED4_Pin GPIO_PIN_3
#define LED4_GPIO_Port GPIOA
#define OSC_ADJ_Pin GPIO_PIN_4
#define OSC_ADJ_GPIO_Port GPIOA
#define AUD_OUT_Pin GPIO_PIN_5
#define AUD_OUT_GPIO_Port GPIOA
#define LED1_Pin GPIO_PIN_6
#define LED1_GPIO_Port GPIOA
#define LED2_Pin GPIO_PIN_7
#define LED2_GPIO_Port GPIOA
#define LED7_Pin GPIO_PIN_4
#define LED7_GPIO_Port GPIOC
#define LED8_Pin GPIO_PIN_5
#define LED8_GPIO_Port GPIOC
#define LED9_Pin GPIO_PIN_0
#define LED9_GPIO_Port GPIOB
#define LED10_Pin GPIO_PIN_1
#define LED10_GPIO_Port GPIOB
#define BOOT1_Pin GPIO_PIN_2
#define BOOT1_GPIO_Port GPIOB
#define SYNC_MON_Pin GPIO_PIN_11
#define SYNC_MON_GPIO_Port GPIOB
#define NCOL8_Pin GPIO_PIN_12
#define NCOL8_GPIO_Port GPIOB
#define NCOL7_Pin GPIO_PIN_13
#define NCOL7_GPIO_Port GPIOB
#define NCOL6_Pin GPIO_PIN_14
#define NCOL6_GPIO_Port GPIOB
#define NCOL5_Pin GPIO_PIN_15
#define NCOL5_GPIO_Port GPIOB
#define SYNC_IN2_Pin GPIO_PIN_6
#define SYNC_IN2_GPIO_Port GPIOC
#define NCOL4_Pin GPIO_PIN_7
#define NCOL4_GPIO_Port GPIOC
#define NCOL3_Pin GPIO_PIN_8
#define NCOL3_GPIO_Port GPIOC
#define NCOL2_Pin GPIO_PIN_9
#define NCOL2_GPIO_Port GPIOC
#define SYNC_OUT_Pin GPIO_PIN_8
#define SYNC_OUT_GPIO_Port GPIOA
#define NCOL1_Pin GPIO_PIN_9
#define NCOL1_GPIO_Port GPIOA
#define LEDMY_Pin GPIO_PIN_10
#define LEDMY_GPIO_Port GPIOA
#define LEDMG_Pin GPIO_PIN_11
#define LEDMG_GPIO_Port GPIOA
#define LEDMR_Pin GPIO_PIN_12
#define LEDMR_GPIO_Port GPIOA
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWDCLK_Pin GPIO_PIN_14
#define SWDCLK_GPIO_Port GPIOA
#define CLK_Pin GPIO_PIN_15
#define CLK_GPIO_Port GPIOA
#define NCOL9_Pin GPIO_PIN_12
#define NCOL9_GPIO_Port GPIOC
#define DB3_Pin GPIO_PIN_2
#define DB3_GPIO_Port GPIOD
#define SYNC_IN_Pin GPIO_PIN_3
#define SYNC_IN_GPIO_Port GPIOB
#define LED3_Pin GPIO_PIN_4
#define LED3_GPIO_Port GPIOB
#define NROW5_Pin GPIO_PIN_5
#define NROW5_GPIO_Port GPIOB
#define USB_RX_Pin GPIO_PIN_6
#define USB_RX_GPIO_Port GPIOB
#define USB_TX_Pin GPIO_PIN_7
#define USB_TX_GPIO_Port GPIOB
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
