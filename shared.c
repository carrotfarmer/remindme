#include "shared.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int delete_reminder(unsigned short id, FILE *file) {
  int success = -1;

  fseek(file, 0, SEEK_END);
  long fsize = ftell(file);
  fseek(file, 0, SEEK_SET);

  char *buf = malloc(fsize + 1);
  fread(buf, fsize, 1, file);

  buf[fsize] = 0;

  char *new_buf = buf;

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
      split = strtok(NULL, "\n");
    }
  }

  free(buf);
  return success;
}
