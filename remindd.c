// remindme Daemon

#include "shared.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <sys/inotify.h>

#define EXIT_SUCCESS 0
#define EXIT_ERR_INIT_INOTIFY 1
#define EXIT_ERR_ADD_WATCH 2
#define EXIT_FILE_DELETED 3

int main() {
  int ievent_queue = -1;
  int ievent_status = -1;

  char *file_path = get_file_path();

  const uint32_t INOTIFY_MASK = IN_MODIFY | IN_CREATE | IN_DELETE;
  ievent_queue = inotify_init();
  if (ievent_queue == -1) {
    fprintf(stderr, "err: failed to initialize inotify instance\n");
    exit(EXIT_ERR_INIT_INOTIFY);
  }

  ievent_status = inotify_add_watch(ievent_queue, file_path, INOTIFY_MASK);
  if (ievent_status == -1) {
    fprintf(stderr, "err: failed to add %s to inotify watch list\n", file_path);
    exit(EXIT_ERR_ADD_WATCH);
  }

  FILE *file = fopen(file_path, "r");
  struct Reminder *reminders = get_reminders(file);

  while (1) {
    struct inotify_event event;
    ievent_status = read(ievent_queue, &event, sizeof(event));
    if (ievent_status == -1) {
      fprintf(stderr, "err: failed to read inotify event\n");
      exit(EXIT_ERR_INIT_INOTIFY);
    }

    if (event.mask & IN_MODIFY) {
      printf("file modified...reloading reminders\n");
      reminders = get_reminders(file);
    }

    if (event.mask & IN_CREATE) {
      printf("%s created\n", file_path);
    }

    if (event.mask & IN_DELETE) {
      fprintf(stderr, "err: %s was deleted\n", file_path);
      exit(EXIT_FILE_DELETED);
    }
  }
}
