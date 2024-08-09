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
        fprintf(stderr, "err: reminder with id %d not found\n", del_id);
      } else {
        printf("reminder with id %d deleted\n", del_id);
      }
    }
  } else if (argc == 4) {
    time_t raw_time = gen_raw_time(argv);
    if (raw_time == -1) {
      fprintf(stderr, "err: invalid time format");
      return 1;
    }

    struct Reminder r;
    r.message = argv[1];
    r.id = gen_id();
    r.time = raw_time;

    fprintf(file, "%hu === %s === %ld\n", r.id, r.message, r.time);
    printf("Reminder \"%s\" set for %s %s\n", r.message, argv[2], argv[3]);

    free(r.message);
  } else {
    printf("err: invalid arguments\n\n");
    print_help();

    free(file_path);
    fclose(file);
    exit(EXIT_INVALID_ARGS);
  }

  free(file_path);
  fclose(file);

  return EXIT_SUCCESS;
}
