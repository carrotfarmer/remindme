// remindme Daemon

#include "shared.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/epoll.h>
#include <sys/inotify.h>
#include <sys/timerfd.h>

#include <libnotify/notify.h>

#define EXIT_SUCCESS 0
#define EXIT_ERR_INIT_EPOLL 1
#define EXIT_ERR_INIT_INOTIFY 2
#define EXIT_ERR_ADD_WATCH 3
#define EXIT_FILE_DELETED 4
#define EXIT_ERR_GET_CURR_TIME 5
#define EXIT_INIT_TIMERFD 6
#define EXIT_ERR_EPOLL_WAIT 7
#define EXIT_ERR_SET_TIMERFD 8
#define EXIT_ERR_INIT_LIBNOTIFY 9

struct Reminder *get_next_reminder(FILE *file, struct Reminder *reminders) {
  int reminder_count = get_reminder_count(file);
  if (reminder_count == 0) {
    return NULL;
  }

  struct Reminder *reminder = malloc(sizeof(struct Reminder));
  // initialize reminder with first reminder
  reminder->id = reminders[0].id;
  reminder->message = reminders[0].message;
  reminder->time = reminders[0].time;
  printf("initial reminder: %s\n", reminder->message);

  char *buf = read_to_buf(file);
  char *split = strtok(buf, "\n");
  split = strtok(NULL, "\n");

  while (split != NULL) {
    unsigned short id;
    char *message;
    time_t time;
    sscanf(split, "%hu === %m[^=] === %ld", &id, &message, &time);

    struct Reminder curr_reminder = {id, message, time};

    // check if current reminder is earlier than reminder
    printf("comparing %s to %s\n", curr_reminder.message, reminder->message);
    if (difftime(curr_reminder.time, reminder->time) < 0) {
      reminder->id = curr_reminder.id;
      reminder->message = curr_reminder.message;
      reminder->time = curr_reminder.time;

      printf("new reminder: %s\n", reminder->message);
    }

    split = strtok(NULL, "\n");
  }

  free(buf);
  return reminder;
}

void trigger_notification(char *message) {
  NotifyNotification *notif_handle;
  int libnotify_status = notify_init("Reminder!");
  if (libnotify_status == 0) {
    fprintf(stderr, "err: failed to initialize libnotify\n");
    exit(EXIT_ERR_INIT_LIBNOTIFY);
  }

  notif_handle =
      notify_notification_new("Reminder!", message, "dialog-critical");
  notify_notification_show(notif_handle, NULL);
}

void load_reminders(FILE *file, int timer_fd) {
  freopen(NULL, "r", file);
  struct Reminder *reminders = get_reminders(file);

  struct Reminder *next_reminder = get_next_reminder(file, reminders);

  if (get_reminder_count(file) != 0) {
    printf("Next reminder: %s\n", next_reminder->message);
  } else {
    printf("No reminders found\n");
  }
}

void update_timer(int timer_fd, struct Reminder *next_reminder,
                  struct epoll_event ev, int epoll_fd, FILE *file) {
  struct itimerspec new_value;
  time_t now = time(NULL);
  time_t seconds = next_reminder->time - now;

  new_value.it_value.tv_sec = seconds;
  new_value.it_value.tv_nsec = 0;
  new_value.it_interval.tv_sec = 0; // Set to 0 for one-shot timer
  new_value.it_interval.tv_nsec = 0;

  printf("seconds: %ld\n", seconds);
  printf("OEIJFEWOIJ");

  if (seconds <= 0) {
    char notif_msg[256];
    snprintf(notif_msg, sizeof(notif_msg), "Overdue reminder! %s",
             next_reminder->message);
    trigger_notification(notif_msg);
    delete_reminder(next_reminder->id, NULL);
    load_reminders(file, timer_fd);
  }

  if (timerfd_settime(timer_fd, 0, &new_value, NULL) == -1 && seconds >= 0) {
    fprintf(stderr, "err: failed to set timerfd\n");
    exit(EXIT_ERR_SET_TIMERFD);
  }

  // check if timer_fd already exists in epoll
  ev.data.fd = timer_fd;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, timer_fd, &ev) == -1) {
    if (errno == EEXIST) {
      if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, timer_fd, &ev) == -1) {
        fprintf(stderr, "err: failed to modify timer event\n");
        perror("epoll_ctl");
        exit(EXIT_ERR_INIT_EPOLL);
      }
    } else {
      fprintf(stderr, "err: failed to add timer_fd to epoll\n");
      perror("epoll_ctl");
      exit(EXIT_ERR_INIT_EPOLL);
    }
  }

  printf("Timer set to %ld seconds\n", seconds);
}

int main() {
  int inotify_fd = -1;
  int ievent_status = -1;

  char *file_path = get_file_path();

  int epoll_fd = epoll_create1(0);
  if (epoll_fd == -1) {
    fprintf(stderr, "err: failed to create epoll instance\n");
    exit(EXIT_ERR_INIT_EPOLL);
  }

  const uint32_t INOTIFY_MASK = IN_MODIFY | IN_CREATE | IN_DELETE;
  inotify_fd = inotify_init1(IN_NONBLOCK);
  if (inotify_fd == -1) {
    fprintf(stderr, "err: failed to initialize inotify instance\n");
    exit(EXIT_ERR_INIT_INOTIFY);
  }

  ievent_status = inotify_add_watch(inotify_fd, file_path, INOTIFY_MASK);
  if (ievent_status == -1) {
    fprintf(stderr, "err: failed to add %s to inotify watch list\n", file_path);
    exit(EXIT_ERR_ADD_WATCH);
  }

  printf("Watching %s for changes\n", file_path);

  FILE *file = fopen(file_path, "r");

  // add timer_fd and ievent_queue to epoll
  struct epoll_event ev;
  ev.events = EPOLLIN;

  // initialize timer_fd
  int timer_fd = timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK);
  if (timer_fd == -1) {
    fprintf(stderr, "err: failed to create timerfd\n");
    exit(EXIT_INIT_TIMERFD);
  }

  load_reminders(file, timer_fd);

  if (get_reminder_count(file) != 0) {
    freopen(NULL, "r", file);
    update_timer(timer_fd, get_next_reminder(file, get_reminders(file)), ev,
                 epoll_fd, file);
  }

  ev.data.fd = inotify_fd;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, inotify_fd, &ev) == -1) {
    fprintf(stderr, "err: failed to add inotify_fd to epoll\n");
    exit(EXIT_ERR_INIT_EPOLL);
  }

  while (1) {
    struct epoll_event events[2];
    int n = epoll_wait(epoll_fd, events, 2, -1);
    if (n == -1) {
      fprintf(stderr, "err: failed to wait for epoll events\n");
      exit(EXIT_ERR_EPOLL_WAIT);
    }

    int reload = false;
    for (int i = 0; i < n; i++) {
      if (events[i].data.fd == inotify_fd) {
        // reload reminders
        printf("Reloading reminders\n");

        // Clear the inotify event
        char buffer[1024];
        read(inotify_fd, buffer, sizeof(buffer));

        reload = true;
      } else if (events[i].data.fd == timer_fd) {
        trigger_notification(
            get_next_reminder(file, get_reminders(file))->message);
        uint64_t expirations;
        ssize_t s = read(timer_fd, &expirations, sizeof(expirations));
        if (s == -1) {
          if (errno != EAGAIN) {
            fprintf(stderr, "err: failed to read from timerfd\n");
            exit(EXIT_ERR_SET_TIMERFD);
          }
          continue;
        }
        struct Reminder *next_reminder =
            get_next_reminder(file, get_reminders(file));
        printf("Reminder: %s\n", next_reminder->message);
        delete_reminder(next_reminder->id, file);
        reload = true;
      }
    }

    if (reload) {
      freopen(NULL, "r", file);
      load_reminders(file, timer_fd);
      if (get_reminder_count(file) != 0) {
        update_timer(timer_fd, get_next_reminder(file, get_reminders(file)), ev,
                     epoll_fd, file);
      }
      reload = false;
    }
  }
}
