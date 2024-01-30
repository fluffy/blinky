// SPDX-FileCopyrightText: Copyright (c) 2023 Cullen Jennings
// SPDX-License-Identifier: BSD-2-Clause

#pragma once

typedef enum {
  StatusNone = 0,
  StatusError,
  StatusBadPower,
  StatusRunning,
  StatusCouldSync,
  StatusSync,
  StatusLostSync
} Status;
void updateStatus(Status newStatus);
