// SPDX-FileCopyrightText: Copyright (c) 2023 Cullen Jennings
// SPDX-License-Identifier: BSD-2-Clause

#pragma once

#include <stdint.h>


void dispInit();
void dispSetup();

void dispUpdate(int32_t dispUs, int32_t dispS);  // called in an interupt
