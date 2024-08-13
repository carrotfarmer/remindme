#include "shared.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

char *get_file_path() { return "/etc/.remindme"; }

char *read_to_buf(FILE *file) {
  if (file == NULL) {
    fprintf(stderr, "err: invalid file pointer\n");
    return NULL;
  }

  // get the size of the file
  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  char *buffer = (char *)malloc(file_size + 1); // +1 for null terminator
  if (buffer == NULL) {
    perror("error allocating memory");
    return NULL;
  }

  size_t read_size = fread(buffer, 1, file_size, file);
  if (read_size != file_size) {
    perror("error reading file");
    free(buffer);
    return NULL;
  }

  buffer[file_size] = '\0';

  return buffer;
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
