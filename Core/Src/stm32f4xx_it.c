/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @brief   Interrupt Service Routines.
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim7;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
  while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Pre-fetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f4xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles TIM2 global interrupt.
  */
void TIM2_IRQHandler(void)
{
  /* USER CODE BEGIN TIM2_IRQn 0 */

  /* USER CODE END TIM2_IRQn 0 */
  HAL_TIM_IRQHandler(&htim2);
  /* USER CODE BEGIN TIM2_IRQn 1 */

  /* USER CODE END TIM2_IRQn 1 */
}

/**
  * @brief This function handles TIM3 global interrupt.
  */
void TIM3_IRQHandler(void)
{
  /* USER CODE BEGIN TIM3_IRQn 0 */

  /* USER CODE END TIM3_IRQn 0 */
  HAL_TIM_IRQHandler(&htim3);
  /* USER CODE BEGIN TIM3_IRQn 1 */

  /* USER CODE END TIM3_IRQn 1 */
}

/**
  * @brief This function handles TIM7 global interrupt.
  */
void TIM7_IRQHandler(void)
{
  /* USER CODE BEGIN TIM7_IRQn 0 */
  static int tim7Count=0;
  tim7Count++;// counting in ms 

  int dispCount = tim7Count/1000; // TODO remove div 1000
  
  // Low 4 bits of display 
  if ( dispCount % 100  == 0 ) { HAL_GPIO_TogglePin( LEDC_GPIO_Port, LEDC_Pin ); }
  if ( dispCount % 200  == 0 ) { HAL_GPIO_TogglePin( LEDB_GPIO_Port, LEDB_Pin ); }
  if ( dispCount % 400  == 0 ) { HAL_GPIO_TogglePin( LEDE_GPIO_Port, LEDE_Pin ); }
  if ( dispCount % 800  == 0 ) { HAL_GPIO_TogglePin( LEDD_GPIO_Port, LEDD_Pin ); }
  
  // High 4 bits of display 
  if ( dispCount %  1600  == 0 ) { HAL_GPIO_TogglePin( LEDG_GPIO_Port, LEDG_Pin ); }
  if ( dispCount %  3200  == 0 ) { HAL_GPIO_TogglePin( LEDF_GPIO_Port, LEDF_Pin ); }
  if ( dispCount %  6400  == 0 ) { HAL_GPIO_TogglePin( LEDH_GPIO_Port, LEDH_Pin ); }
  if ( dispCount % 12800  == 0 ) { HAL_GPIO_TogglePin( LEDA_GPIO_Port, LEDA_Pin ); }

  if ( 1 ) {
    const int tickTime = 5; 
    int row = 1+ (( dispCount / tickTime ) % 8);
    int col = 1+ (( dispCount / (tickTime*8) ) % 5);
    
    HAL_GPIO_WritePin( ROW1_GPIO_Port, ROW1_Pin, (row==5) ? GPIO_PIN_SET : GPIO_PIN_RESET );
    HAL_GPIO_WritePin( ROW2_GPIO_Port, ROW2_Pin, (row==4) ? GPIO_PIN_SET : GPIO_PIN_RESET );
    HAL_GPIO_WritePin( ROW3_GPIO_Port, ROW3_Pin, (row==3) ? GPIO_PIN_SET : GPIO_PIN_RESET );
    HAL_GPIO_WritePin( ROW4_GPIO_Port, ROW4_Pin, (row==2) ? GPIO_PIN_SET : GPIO_PIN_RESET );
    HAL_GPIO_WritePin( ROW5_GPIO_Port, ROW5_Pin, (row==1) ? GPIO_PIN_SET : GPIO_PIN_RESET );
    HAL_GPIO_WritePin( ROW6_GPIO_Port, ROW6_Pin, (row==6) ? GPIO_PIN_SET : GPIO_PIN_RESET );
    HAL_GPIO_WritePin( ROW7_GPIO_Port, ROW7_Pin, (row==7) ? GPIO_PIN_SET : GPIO_PIN_RESET );
    HAL_GPIO_WritePin( ROW8_GPIO_Port, ROW8_Pin, (row==8) ? GPIO_PIN_SET : GPIO_PIN_RESET );

    HAL_GPIO_WritePin( COL1_GPIO_Port, COL1_Pin, (col==2) ? GPIO_PIN_RESET : GPIO_PIN_SET );
    HAL_GPIO_WritePin( COL2_GPIO_Port, COL2_Pin, (col==1) ? GPIO_PIN_RESET : GPIO_PIN_SET );
    HAL_GPIO_WritePin( COL3_GPIO_Port, COL3_Pin, (col==5) ? GPIO_PIN_RESET : GPIO_PIN_SET );
    HAL_GPIO_WritePin( COL4_GPIO_Port, COL4_Pin, (col==3) ? GPIO_PIN_RESET : GPIO_PIN_SET );
    HAL_GPIO_WritePin( COL5_GPIO_Port, COL5_Pin, (col==4) ? GPIO_PIN_RESET : GPIO_PIN_SET );
  }
  
  /* USER CODE END TIM7_IRQn 0 */
  HAL_TIM_IRQHandler(&htim7);
  /* USER CODE BEGIN TIM7_IRQn 1 */

  /* USER CODE END TIM7_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
