// Copyright (c) 2023 Cullen Jennings

#include "status.h"

#include <stdio.h>
#include <string.h>

#include "hardware.h"
#include "main.h"

static Status status = StatusNone;
static Status statusPrev = StatusNone;

void updateStatus(Status newStatus) {
  // impelment state machine for status updates
  switch (status) {
    case StatusError: {
      // no way out of error
    } break;

    case StatusBadPower: {
      if (newStatus == StatusError) {
        status = newStatus;
      }
    } break;

    case StatusSync: {
      if (newStatus == StatusRunning) {
        break;
      }
      if (newStatus == StatusCouldSync) {
        break;
      }
      if (newStatus == StatusLostSync) {
        status = StatusRunning;
        break;
      }
      // fall through to default
    }

    case StatusCouldSync: {
      if (newStatus == StatusRunning) {
        break;
      }
      if (newStatus == StatusLostSync) {
        break;
      }
      // fall through to default
    }

    default: {
      if (newStatus == StatusLostSync) {
        status = StatusRunning;
        break;
      }
      status = newStatus;
    } break;

  }  // end swtich

  if (statusPrev != status) {
    HAL_GPIO_WritePin(LEDMR_GPIO_Port, LEDMR_Pin,
                      GPIO_PIN_RESET);  // turn off red error LED
    HAL_GPIO_WritePin(LEDMG_GPIO_Port, LEDMG_Pin,
                      GPIO_PIN_RESET);  // turn off green ok LED
    HAL_GPIO_WritePin(LEDMY_GPIO_Port, LEDMY_Pin,
                      GPIO_PIN_RESET);  // turn off yellow  LED

    switch (status) {
      case StatusError:
        HAL_GPIO_WritePin(LEDMR_GPIO_Port, LEDMR_Pin, GPIO_PIN_SET);
        break;
      case StatusBadPower:
        HAL_GPIO_WritePin(LEDMR_GPIO_Port, LEDMR_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(LEDMY_GPIO_Port, LEDMY_Pin, GPIO_PIN_SET);
        break;
      case StatusSync:
        HAL_GPIO_WritePin(LEDMG_GPIO_Port, LEDMG_Pin, GPIO_PIN_SET);
        break;
      case StatusCouldSync:
        HAL_GPIO_WritePin(LEDMY_GPIO_Port, LEDMY_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(LEDMG_GPIO_Port, LEDMG_Pin, GPIO_PIN_SET);
        break;
      default:
        HAL_GPIO_WritePin(LEDMY_GPIO_Port, LEDMY_Pin, GPIO_PIN_SET);
        break;
    }

    statusPrev = status;
  }
}
