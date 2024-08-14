// remindme client

#include "shared.h"

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define EXIT_SUCCESS 0
#define EXIT_INVALID_ARGS 1
#define EXIT_ERR_OPEN_FILE 2

#define MESSAGE_WIDTH 27
#define TIME_WIDTH 29

void print_help() {
  printf("Usage: remindme [OPTION] [MESSAGE] [TIME]\n");
  printf("Set a reminder for a specific time\n\n");
  printf("Display all reminders:\n");
  printf("  remindme\n\n");
  printf("Options:\n");
  printf("  -d [ID]             Delete a reminder by its ID\n");
  printf("  --clear-all         Delete all reminders\n");
  printf("  --help, -h          Display this help message\n\n");
  printf("Time Formats:\n");
  printf("  [MESSAGE]           The message for the reminder\n");
  printf("  [TIME]              The time to set the reminder\n");
  printf("                      Format: MM/DD/YYYY HH:MM\n");
  printf("                      Example: 12/31/2022 23:59\n");
  printf("                      Relative time formats are also supported:\n");
  printf("                      Example: 1d 1h 30m 15s\n");
}

time_t get_relative_time(int argc, char **argv) {
  int total_seconds = 0;

  for (int i = 2; i < argc; ++i) {
    if (strchr(argv[i], 'd') != NULL) {
      int num_days;
      sscanf(argv[i], "%dy", &num_days);
      total_seconds += (num_days * (24 * 60 * 60));
    } else if (strchr(argv[i], 'h') != NULL) {
      int num_hours;
      sscanf(argv[i], "%dh", &num_hours);
      total_seconds += (num_hours * (60 * 60));
    } else if (strchr(argv[i], 'm') != NULL) {
      int num_minutes;
      sscanf(argv[i], "%dm", &num_minutes);
      total_seconds += (num_minutes * 60);
    } else if (strchr(argv[i], 's') != NULL) {
      int num_seconds;
      sscanf(argv[i], "%ds", &num_seconds);
      total_seconds += num_seconds;
    } else {
      return -1;
    }
  }

  // add seconds to current time
  int raw_time = time(NULL) + total_seconds;

  if (raw_time <= 0) {
    return -1;
  }

  return raw_time;
}

time_t gen_raw_time(char **argv) {
  const char *date_str = argv[2];
  const char *time_str = argv[3];

  struct tm time_info;
  memset(&time_info, 0, sizeof(struct tm));

  // parse date string
  int month, day, year;
  sscanf(date_str, "%d/%d/%d", &year, &month, &day);
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

void display_reminders(struct Reminder *reminders, int count) {
  if (count == 0) {
    return;
  }

  // table header
  printf("+------+------------------------------+-----------------------------+"
         "\n");
  printf("| ID   | Message                      | Time                        "
         "|\n");
  printf("+------+------------------------------+-----------------------------+"
         "\n");

  for (int i = 0; i < count; i++) {
    struct Reminder *r = &reminders[i];

    char *time_str = ctime(&r->time);
    time_str[strcspn(time_str, "\n")] = '\0';

    // truncate the message
    char truncated_message[MESSAGE_WIDTH + 1];
    if (strlen(r->message) > MESSAGE_WIDTH) {
      strncpy(truncated_message, r->message, MESSAGE_WIDTH - 3);
      truncated_message[MESSAGE_WIDTH - 3] =
          '\0'; // Null-terminate the truncated message
      strcat(truncated_message, "...");
    } else {
      snprintf(truncated_message, sizeof(truncated_message), "%s", r->message);
    }

    // print the reminder ID and truncated message
    printf("| %-4d | %-28s | %-27s |\n", r->id, truncated_message, time_str);
  }

  // table footer
  printf("+------+------------------------------+-----------------------------+"
         "\n");
}

int main(int argc, char **argv) {
  srand(time(NULL));

  FILE *file;
  char *file_path = get_file_path();
  file = fopen(file_path, "a+");

  if (file == NULL) {
    fprintf(stderr, "err: failed to open file\n");
    perror("fopen");
    return EXIT_ERR_OPEN_FILE;
  }

  if (argc == 1) {
    struct Reminder *reminders = get_reminders(file);

    if (reminders == NULL) {
      printf("No reminders set\n");
      return EXIT_SUCCESS;
    }

    int reminder_count = get_reminder_count(file);

    printf("Found %d reminders\n\n", reminder_count);
    display_reminders(reminders, reminder_count);

    free(reminders);
  } else if (argc == 2 && strcmp(argv[1], "--clear-all") == 0) {
    fclose(fopen(file_path, "w"));
    printf("cleared all reminders\n");
  } else if (argc == 2 &&
             (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) {
    print_help();
  } else if (argc == 3 && strcmp(argv[1], "-d") == 0) {
    int del_id = atoi(argv[2]);
    int del = delete_reminder(del_id, file);

    if (del == -1) {
      fprintf(stderr, "err: reminder with id %d not found", del_id);
    } else {
      printf("reminder with id %d deleted", del_id);
    }
  } else {
    time_t raw_time;

    // check if argv contains a character
    if (strchr(argv[2], '/') != NULL) {
      raw_time = gen_raw_time(argv);
    } else {
      raw_time = get_relative_time(argc, argv);
    }

    if (raw_time == -1) {
      fprintf(stderr, "err: invalid time format");
      print_help();
      exit(EXIT_INVALID_ARGS);
    }

    int id = get_reminder_count(file) + 1;

    if (fprintf(file, "%hu === %s === %ld\n", id, argv[1], raw_time) < 0) {
      perror("fprintf");
      exit(EXIT_ERR_OPEN_FILE);
    }

    printf("Reminder \"%s\" set for %s %s", argv[1], argv[2], ctime(&raw_time));
  }

  fclose(file);
  return EXIT_SUCCESS;
}
