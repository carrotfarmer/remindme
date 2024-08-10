// remindme client

#include "shared.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define EXIT_SUCCESS 0
#define EXIT_INVALID_ARGS 1
#define EXIT_ERR_OPEN_FILE 2

int gen_id() { return rand(); }

void print_help() {
  printf("Usage:\n");
  printf("remindme <message> <date - mm/dd/yyyy> <time - hh:mm>\n");
  printf("remindme\n");
  printf("remindme -d <id>\n");
  printf("remindme --clear-all\n");

  printf("\n");
  printf("Example: remindme \"hello world\" 08/07/2024 23:59\n");
  printf("This will trigger a notification with \"hello world\" on 08/07/2024 "
         "at 11:59 PM\n");

  printf("\n");
  printf("To list all reminders, run remindme with no arguments\n");
  printf("To delete a reminder, run remindme -d <id>\n");
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

  if (argc == 1 || (argc == 2 && (strcmp(argv[1], "-d") == 0))) {
    struct Reminder *reminders = get_reminders(file);

    if (reminders == NULL) {
      printf("No reminders set\n");
      return EXIT_SUCCESS;
    }

    int reminder_count = get_reminder_count(file);

    printf("Found %d reminders\n\n", reminder_count);
    for (int i = 0; i < reminder_count; i++) {
      printf("id: %hu, message: %s, time: %s", reminders[i].id,
             reminders[i].message, ctime(&reminders[i].time));
    }

    free(reminders);
  } else if (argc == 2 && strcmp(argv[1], "--clear-all") == 0) {
    fclose(fopen(file_path, "w"));
    printf("cleared all reminders\n");
  } else if (argc == 2 &&
             (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) {
    print_help();
  } else if (argc == 3) {
    if (strcmp(argv[1], "-d") == 0) {
      int del_id = atoi(argv[2]);
      int del = delete_reminder(del_id, file);

      if (del == -1) {
        fprintf(stderr, "err: reminder with id %d not found", del_id);
      } else {
        printf("reminder with id %d deleted", del_id);
      }
    } else {
      printf("err: invalid arguments\n\n");
      print_help();
      exit(EXIT_INVALID_ARGS);
    }
  } else if (argc == 4) {
    time_t raw_time = gen_raw_time(argv);
    if (raw_time == -1) {
      fprintf(stderr, "err: invalid time format");
      exit(EXIT_INVALID_ARGS);
    }

    int id = gen_id();

    if (fprintf(file, "%hu === %s === %ld\n", id, argv[1], raw_time) < 0) {
      perror("fprintf");
      exit(EXIT_ERR_OPEN_FILE);
    }

    printf("Reminder \"%s\" set for %s %s", argv[1], argv[2], ctime(&raw_time));
  } else {
    printf("err: invalid arguments\n\n");
    print_help();

    exit(EXIT_INVALID_ARGS);
  }

  fclose(file);

  return EXIT_SUCCESS;
}
