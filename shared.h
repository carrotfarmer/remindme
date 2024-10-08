#pragma once

#include <stdio.h>
#include <time.h>

char *get_file_path();
char *read_to_buf(FILE *file);

struct Reminder {
  unsigned short id;
  char *message;
  time_t time;
};

int delete_reminder(unsigned short id, FILE *file);
struct Reminder *get_reminders(FILE *file);
int get_reminder_count(FILE *file);
