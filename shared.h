#pragma once

#include <stdio.h>

#define REMINDERS_FILE ".remindme"

int delete_reminder(unsigned short id, FILE *file);
