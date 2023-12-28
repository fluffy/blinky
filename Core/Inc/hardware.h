// Copyright (c) 2023 Cullen Jennings
#pragma once

#include <stm32f4xx_ll_tim.h>

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

extern ADC_HandleTypeDef hadc1;
extern DAC_HandleTypeDef hdac;

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
#define TimeLtc_HAL_CH_SYNC_IN2 HAL_TIM_ACTIVE_CHANNEL_1
