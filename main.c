#include <stdio.h>
#include <stdlib.h>

#define RL_BUF_SIZE 1024
char *read_line() {
  int bufsize = RL_BUF_SIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    fprintf(stderr, "butterfly: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    c = getchar();

    if (c == EOF || c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }

    ++position;

    if (position >= bufsize) {
      bufsize += RL_BUF_SIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "butterfly: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

int loop() {
  int exit = 0;

  char *line;

  while (exit != -1) {
    printf(">");
    line = read_line();
    if (line == "exit")
      exit = -1;
    printf("%s\n", line);
  }

  return exit;
}

int main() {
  printf("This is a shell for linux.\n");

  loop();

  return 0;
}
