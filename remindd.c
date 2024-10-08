// remindme Daemon

#include "shared.h"

#include <errno.h>
#include <signal.h>
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

typedef struct {
  int inotify_fd;
  int ievent_status;
  int timer_fd;
  int epoll_fd;
  struct epoll_event ev;
} SignalContext;

static SignalContext context;
static FILE *file;

void init_signal_context(int inotify_fd, int ievent_status, int timer_fd,
                         int epoll_fd, struct epoll_event ev) {
  context.inotify_fd = inotify_fd;
  context.ievent_status = ievent_status;
  context.timer_fd = timer_fd;
  context.epoll_fd = epoll_fd;
  context.ev = ev;
}

void signal_handler(int signal) {
  int inotify_close_status = -1;
  printf("Signal received, cleaning up...\n");

  if (context.ievent_status != -1) {
    inotify_close_status =
        inotify_rm_watch(context.inotify_fd, context.ievent_status);
    if (inotify_close_status == -1) {
      fprintf(stderr, "err: failed to remove inotify watch\n");
    }
  }

  if (context.epoll_fd != -1) {
    if (close(context.epoll_fd) == -1) {
      fprintf(stderr, "err: failed to close epoll instance\n");
    }
  }

  if (context.timer_fd != -1) {
    if (close(context.timer_fd) == -1) {
      fprintf(stderr, "err: failed to close timerfd\n");
    }
  }

  if (context.inotify_fd != -1) {
    close(context.inotify_fd);
  }

  if (file != NULL) {
    fclose(file);
  }

  exit(EXIT_SUCCESS);
}

struct Reminder *get_next_reminder(struct Reminder *reminders) {
  int reminder_count = get_reminder_count(file);
  if (reminder_count == 0) {
    return NULL;
  }

  struct Reminder *reminder = malloc(sizeof(struct Reminder));
  // Initialize reminder with the first reminder
  reminder->id = reminders[0].id;
  reminder->message = reminders[0].message;
  reminder->time = reminders[0].time;

  char *buf = read_to_buf(file);
  char *split = strtok(buf, "\n");
  split = strtok(NULL, "\n");

  while (split != NULL) {
    unsigned short id;
    char *message;
    time_t time;
    sscanf(split, "%hu === %m[^=] === %ld", &id, &message, &time);

    struct Reminder curr_reminder = {id, message, time};

    // Check if current reminder is earlier than reminder
    if (difftime(curr_reminder.time, reminder->time) < 0) {
      reminder->id = curr_reminder.id;
      reminder->message = curr_reminder.message;
      reminder->time = curr_reminder.time;
    }

    split = strtok(NULL, "\n");
  }

  free(buf);
  return reminder;
}

void trigger_notification(char *title, char *message) {
  NotifyNotification *notif_handle;
  if (!notify_init("remindme")) {
    fprintf(stderr, "err: failed to initialize libnotify\n");
    exit(EXIT_ERR_INIT_LIBNOTIFY);
  }

  notif_handle = notify_notification_new(title, message, "dialog-information");
  notify_notification_set_timeout(notif_handle, NOTIFY_EXPIRES_NEVER);
  notify_notification_set_urgency(notif_handle, NOTIFY_URGENCY_CRITICAL);
  notify_notification_set_category(notif_handle, "reminder");
  notify_notification_show(notif_handle, NULL);
}

void load_reminders() {
  fclose(file);
  file = fopen(get_file_path(), "r");
  struct Reminder *reminders = get_reminders(file);

  struct Reminder *next_reminder = get_next_reminder(reminders);

  if (get_reminder_count(file) != 0) {
    printf("Next reminder: %s\n", next_reminder->message);
  } else {
    printf("No reminders found\n");
  }
}

void update_timer(struct Reminder *next_reminder, int timer_fd, int epoll_fd,
                  struct epoll_event ev) {
  struct itimerspec new_value;
  time_t now = time(NULL);
  time_t seconds = next_reminder->time - now;

  new_value.it_value.tv_sec = seconds;
  new_value.it_value.tv_nsec = 0;
  new_value.it_interval.tv_sec = 0;
  new_value.it_interval.tv_nsec = 0;

  if (seconds <= 0) {
    fclose(file);
    file = fopen(get_file_path(), "r");
    trigger_notification("Overdue Reminder!", next_reminder->message);
    delete_reminder(next_reminder->id, file);

    load_reminders();
    if (get_reminder_count(file) != 0) {
      now = time(NULL);
      seconds = next_reminder->time - now;
      new_value.it_value.tv_sec = seconds;
    }
  }

  if (timerfd_settime(timer_fd, 0, &new_value, NULL) == -1 && seconds >= 0) {
    fprintf(stderr, "err: failed to set timerfd\n");
    exit(EXIT_ERR_SET_TIMERFD);
  }

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
}

int main() {
  int inotify_fd = -1;
  int ievent_status = -1;

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

  char *file_path = get_file_path();
  file = fopen(file_path, "r");

  if (file == NULL) {
    file = fopen(file_path, "wb");
    printf("remindd: Reminders file not found, created new file %s\n",
           file_path);
    if (file == NULL) {
      fprintf(stderr, "err: failed to open %s\n", file_path);
      exit(EXIT_FILE_DELETED);
    }
  }

  ievent_status = inotify_add_watch(inotify_fd, file_path, INOTIFY_MASK);
  if (ievent_status == -1) {
    fprintf(stderr, "err: failed to add %s to inotify watch list\n", file_path);
    exit(EXIT_ERR_ADD_WATCH);
  }

  printf("Watching %s for changes\n", file_path);

  struct epoll_event ev;
  ev.events = EPOLLIN;

  // Initialize timer_fd
  int timer_fd = timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK);
  if (timer_fd == -1) {
    fprintf(stderr, "err: failed to create timerfd\n");
    exit(EXIT_INIT_TIMERFD);
  }

  load_reminders();

  if (get_reminder_count(file) != 0) {
    update_timer(get_next_reminder(get_reminders(file)), timer_fd, epoll_fd,
                 ev);
  }

  ev.data.fd = inotify_fd;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, inotify_fd, &ev) == -1) {
    fprintf(stderr, "err: failed to add inotify_fd to epoll\n");
    exit(EXIT_ERR_INIT_EPOLL);
  }

  init_signal_context(inotify_fd, ievent_status, timer_fd, epoll_fd, ev);

  signal(SIGABRT, signal_handler);
  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);

  while (1) {
    struct epoll_event events[2];
    int n = epoll_wait(epoll_fd, events, 2, -1);
    if (n == -1) {
      fprintf(stderr, "err: failed to wait for epoll events\n");
      exit(EXIT_ERR_EPOLL_WAIT);
    }

    bool reload = false;
    for (int i = 0; i < n; i++) {
      if (events[i].data.fd == inotify_fd) {
        // Reload reminders
        printf("Reloading reminders\n");

        // Clear the inotify event
        char buffer[1024];
        read(inotify_fd, buffer, sizeof(buffer));

        reload = true;
      } else if (events[i].data.fd == timer_fd) {
        trigger_notification("Reminder!",
                             get_next_reminder(get_reminders(file))->message);
        uint64_t expirations;
        ssize_t s = read(timer_fd, &expirations, sizeof(expirations));
        if (s == -1) {
          if (errno != EAGAIN) {
            fprintf(stderr, "err: failed to read from timerfd\n");
            exit(EXIT_ERR_SET_TIMERFD);
          }
          continue;
        }
        struct Reminder *next_reminder = get_next_reminder(get_reminders(file));
        delete_reminder(next_reminder->id, file);
        reload = true;
      }
    }

    if (reload) {
      load_reminders();
      if (get_reminder_count(file) != 0) {
        update_timer(get_next_reminder(get_reminders(file)), timer_fd, epoll_fd,
                     ev);
      }
    }
  }

  free(file_path);
  return EXIT_SUCCESS;
}
