#include <stdio.h>
#include <stdlib.h>

char *read_line() {
  char *line = NULL;
  size_t bufsize = 0;

  // gets the character from stdin and adds them to the lineptr, and reallocates
  // like realloc if neccessary
  if (getline(&line, &bufsize, stdin) == -1) {
    // feof checks if the end of a given stream has been reached
    if (feof(stdin)) {
      exit(EXIT_SUCCESS);
    } else {
      perror("readline");
      exit(EXIT_FAILURE);
    }
  }

  return line;
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
