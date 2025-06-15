// SPDX-FileCopyrightText: Copyright (c) 2023 Cullen Jennings
// SPDX-License-Identifier: BSD-2-Clause

#include "setting.h"

Setting setting;

void settingInit() {
  setting.blinkMute = 0;       // mutes audio outout
  setting.blinkBlank = 0;      // causes LED to be off
  setting.blinkDispAudio = 0;  // caused audio latency to be displayed on LED
  setting.blinkHaveDisplay = 1;
  setting.blinkPPS = 0;
}

void settingSetup() {
  // nothing to do here
}
