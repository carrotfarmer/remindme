// remindme client

#include "shared.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define EXIT_SUCCESS 0
#define EXIT_ERR_TOO_FEW_ARGS 1

int gen_id() { return rand(); }

int main(int argc, char **argv) {
  srand(time(NULL));

  FILE *file;
  char *filePath = get_file_path();
  file = fopen(filePath, "a+");

  // if (argc < 2) {
  // fprintf(stderr, "Usage: %s <message>\n", argv[0]);
  // return EXIT_ERR_TOO_FEW_ARGS;
  //}

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
  }

  if (argc == 3) {
    if (strcmp(argv[1], "-d") == 0) {
      int del_id = atoi(argv[2]);
      int del = delete_reminder(del_id, file);

      if (del == -1) {
        fprintf(stderr, "err: reminder with id %d not found\n", del_id);
      } else {
        printf("reminder with id %d deleted\n", del_id);
      }
    }
  }

  if (argc == 4) {
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
  }

  fclose(file);
  return EXIT_SUCCESS;
}
