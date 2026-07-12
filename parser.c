#include <linux/limits.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define ARRAY_SIZE 64
#define ARG_SIZE 1024

char **parseLine(char *line) {
  int tokSize = ARRAY_SIZE;
  int argSize = ARG_SIZE;
  // allocate tokens to hold tokSize pointers of char type
  char **tokens = malloc(tokSize * sizeof(char *));
  char *token;

  if (!tokens) {
    fprintf(stderr, "butterfly: allocation error");
    exit(EXIT_FAILURE);
  }

  int position = 0;
  int i = 0;
  int count = 0;
  int loop = 1;
  bool quoted = false;

  token = malloc(argSize * sizeof(char));
  while (loop != 0) {
    if (line[i] == ' ' && !quoted) {
      if (position <= 0) {
        ++i;
        continue;
      }

      token[position] = '\0';
      tokens[count] = token;
      ++count;
      argSize = ARG_SIZE;
      token = malloc(argSize * sizeof(char));
      position = 0;

      if (count >= tokSize) {
        tokSize += ARRAY_SIZE;
        tokens = realloc(tokens, tokSize * sizeof(char *));

        if (!tokens) {
          perror("butterfly");
          exit(EXIT_FAILURE);
        }
      }
    }

    else if (line[i] == '"') {
      quoted = !quoted;
    }

    else if (line[i] == '\n' && !quoted) {
      if (position <= 0) {
        ++i;
        continue;
      }

      token[position] = '\0';
      tokens[count] = token;
      ++count;
      argSize = ARG_SIZE;
      token = malloc(argSize * sizeof(char));
      position = 0;

      if (count >= tokSize) {
        tokSize += ARRAY_SIZE;
        tokens = realloc(tokens, tokSize * sizeof(char *));

        if (!tokens) {
          perror("butterfly");
          exit(EXIT_FAILURE);
        }
      }
    }

    else if (line[i] == '\0') {
      if (position > 0) {
        token[position] = '\0';
        tokens[count] = token;
        ++count;
      } else {
        free(token);
      }
      if (count >= tokSize) {
        tokSize += ARRAY_SIZE;
        tokens = realloc(tokens, tokSize * sizeof(char *));

        if (!tokens) {
          perror("butterfly");
          exit(EXIT_FAILURE);
        }
      }
      tokens[count] = NULL;
      break;
    }

    else {
      token[position] = line[i];
      ++position;
    }

    // makes sure that the position is never at the last usuable byte of memory
    if (position >= argSize) {
      argSize += ARG_SIZE;
      token = realloc(token, argSize * sizeof(char));

      if (!token) {
        perror("butterfly");
        exit(EXIT_FAILURE);
      }
    }

    ++i;
  }

  return tokens;
}
