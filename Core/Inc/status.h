// Copyright (c) 2024 Cullen Jennings
#pragma once

typedef enum { StatusNone=0, StatusError, StatusBadPower, StatusRunning, StatusCouldSync , StatusSync, StatusLostSync } Status;
void updateStatus( Status newStatus );

