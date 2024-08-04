#include "shared.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define EXIT_SUCCESS 0
#define EXIT_ERR_TOO_FEW_ARGS 1

struct Reminder {
  unsigned short id;
  char *message;
  // TODO: better type
  char *time;
};

int gen_id() { return rand(); }

int main(int argc, char **argv) {
  srand(time(NULL));

  FILE *file;
  char filePath[1024];

  snprintf(filePath, sizeof(filePath), "%s/%s", getenv("HOME"), REMINDERS_FILE);
  file = fopen(filePath, "a+");

  if (argc < 2) {
    fprintf(stderr, "Usage: %s <message>\n", argv[0]);
    return EXIT_ERR_TOO_FEW_ARGS;
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
    } else {
      struct Reminder r;
      r.message = argv[1];
      r.id = gen_id();
      r.time = argv[2];

      fprintf(file, "%hu === %s === %s\n", r.id, r.message, r.time);
    }
  }

  fclose(file);
  return EXIT_SUCCESS;
}
