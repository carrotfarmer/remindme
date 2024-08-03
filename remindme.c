#include <stdbool.h>
#include <stdio.h>

#define EXIT_SUCCESS 0
#define EXIT_ERR_TOO_FEW_ARGS 1

struct Reminder {
  unsigned short id;
  char *message;
  // TODO: better type
  char *time;
};

int main(int argc, char **argv) {
  bool create = false;

  if (argc < 2) {
    fprintf(stderr, "Usage: %s <message>\n", argv[0]);
    return EXIT_ERR_TOO_FEW_ARGS;
  }

  if (argc == 3) {
    create = true;

    struct Reminder r;
    r.message = argv[1];
    r.id = 0;
    r.time = argv[2];

    printf("Reminder: %s\n", r.message);
    printf("Time: %s\n", r.time);
    printf("ID: %d\n", r.id);
  }

  return EXIT_SUCCESS;
}
