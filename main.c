#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

#define TOK_BUFSIZE 64
#define TOK_DELIM " \t\a\n\a"
char **split_line(char *line) {
  int bufsize = TOK_BUFSIZE;
  char **tokens = malloc(bufsize * sizeof(char *));
  char *token;

  if (!tokens) {
    fprintf(stderr, "butterfly: allocation error\n");
    exit(EXIT_FAILURE);
  }

  int position = 0;
  token = strtok(line, TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    ++position;

    if (position >= bufsize) {
      bufsize += TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize);
      if (!tokens) {
        fprintf(stderr, "butterfly: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, TOK_DELIM);
  }

  tokens[position] = NULL;
  return tokens;
}

int loop() {
  int exit = 0;

  char *line;
  char **tokens;

  while (exit != -1) {
    printf(">");
    line = read_line();
    tokens = split_line(line);
  }

  return exit;
}

int main() {
  printf("This is a shell for linux.\n");

  loop();

  return 0;
}
