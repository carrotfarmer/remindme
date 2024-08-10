#include "shared.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

char *get_file_path() { return "/etc/.remindme"; }

char *read_to_buf(FILE *file) {
  fseek(file, 0, SEEK_END);
  long fsize = ftell(file);
  fseek(file, 0, SEEK_SET);

  char *buf = malloc(fsize + 1);
  fread(buf, fsize, 1, file);

  buf[fsize] = 0;

  return buf;
}

int delete_reminder(unsigned short id, FILE *file) {
  int success = -1;

  char *buf = read_to_buf(file);

  freopen(NULL, "w", file);
  freopen(NULL, "a+", file);

  char *split = strtok(buf, "\n");
  while (split != NULL) {
    char id_str[6];
    snprintf(id_str, sizeof(id_str), "%hu", id);

    if (strstr(split, id_str) != NULL) {
      split = strtok(NULL, "\n");
      success = 0;
    } else {
      fprintf(file, "%s\n", split);
      fflush(file);
      split = strtok(NULL, "\n");
    }
  }

  free(buf);
  return success;
}

time_t gen_raw_time(char **str_args) {
  const char *date_str = str_args[2];
  const char *time_str = str_args[3];

  struct tm time_info;
  memset(&time_info, 0, sizeof(struct tm));

  // parse date string
  int month, day, year;
  sscanf(date_str, "%d/%d/%d", &month, &day, &year);
  time_info.tm_mon = month - 1;
  time_info.tm_mday = day;
  time_info.tm_year = year - 1900;

  // parse time string
  int hour, minute;
  sscanf(time_str, "%d:%d", &hour, &minute);
  time_info.tm_hour = hour - 1;
  time_info.tm_min = minute;

  time_t raw_time = mktime(&time_info);

  if (raw_time <= 0) {
    return -1;
  }

  return raw_time;
}

struct Reminder *get_reminders(FILE *file) {
  struct Reminder *reminders =
      malloc(sizeof(struct Reminder) * get_reminder_count(file));
  int curr_index = 0;

  char *buf = read_to_buf(file);

  char *split = strtok(buf, "\n");
  while (split != NULL) {
    unsigned short id;
    char *message;
    time_t time;
    sscanf(split, "%hu === %m[^=] === %ld", &id, &message, &time);

    struct Reminder r = {id, message, time};
    reminders[curr_index] = r;
    curr_index++;

    split = strtok(NULL, "\n");
  }

  free(buf);
  return reminders;
}

int get_reminder_count(FILE *file) {
  char *buf = read_to_buf(file);
  int count = 0;

  char *split = strtok(buf, "\n");
  while (split != NULL) {
    count++;
    split = strtok(NULL, "\n");
  }

  free(buf);
  return count;
}
