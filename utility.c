#include "utility.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int isToBePutToAFile(char **args) {
  for (int i = 0; args[i] != NULL; ++i) {
    if (strcmp(args[i], ">") == 0 || strcmp(args[i], ">>") == 0) {
      return i;
    }
  }
  return -1;
}

int isToBeReadFromAFile(char **args) {
  for (int i = 0; args[i] != NULL; ++i) {
    if (strcmp(args[i], "<") == 0) {
      return i;
    }
  }
  return -1;
}

int **pipeIndex(char **args) {
  int count = 1;
  int **indexes = malloc(sizeof(int *));

  if (indexes == NULL) {
    perror("butterfly");
    exit(EXIT_FAILURE);
  }

  for (int i = 0; args[i] != NULL; ++i) {
    if (strcmp(args[i], "|") == 0) {
      int *index = malloc(sizeof(int));
      if (index == NULL) {
        perror("butterfly");
        exit(EXIT_FAILURE);
      }

      *index = i;
      indexes[count] = index;
      ++count;

      indexes = realloc(indexes, (count + 1) * sizeof(int *));
      if (indexes == NULL || index == NULL) {
        perror("butterfly");
        exit(EXIT_FAILURE);
      }
    }
  }

  indexes[count] = NULL;

  return indexes;
}

int setOutputToFile(char **args, int index) {
  // open is POSIX level file opener, the arguments are write only or create
  // or truncate(wipe out previously existing text to nothing), 0644 is
  // octal number more specifically open requires to know what permissions
  // to give to the file, 0 tells it is a octal number and the number maps
  // to rw-r--r--
  int fd;

  // if > then use truncating file, else use appending  file, since either '>'
  // or '>>' exits for sure
  if (strcmp(args[index], ">") == 0)
    fd = open(args[index + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
  else
    fd = open(args[index + 1], O_WRONLY | O_CREAT | O_APPEND, 0644);

  if (fd < 0) {
    perror("butterfly");
    exit(EXIT_FAILURE);
  }

  dup2(fd, STDOUT_FILENO);

  close(fd);

  free(args[index + 1]);
  args[index + 1] = NULL;
  free(args[index]);
  args[index] = NULL;

  return 1;
}

int setInputFromFile(char **args, int index) {
  int fd = open(args[index + 1], O_RDONLY);
  if (fd < 0) {
    perror("butterfly");
    exit(EXIT_FAILURE);
  }

  dup2(fd, STDIN_FILENO);
  close(fd);

  // setting the indexes to null so that cleaner in the main loop skips these
  // indexs and does not do a double free
  free(args[index + 1]);
  args[index + 1] = NULL;
  free(args[index]);
  args[index] = NULL;

  return 1;
}

// a function to join all the arguments in the argument array
char *joinArguments(char **args) {
  if (args == NULL || args[0] == NULL)
    return NULL;

  char *joined;
  int lenght = 0;

  for (int i = 0; args[i] != NULL; ++i) {
    lenght += strlen(args[i]) + 1;
  }

  joined = malloc(sizeof(char) * lenght);
  if (joined == NULL) {
    perror("butterfly");
    exit(EXIT_FAILURE);
  }

  joined[0] = '\0';

  for (int i = 0; args[i] != NULL; ++i) {
    strcat(joined, args[i]);
    if (args[i + 1] != NULL)
      strcat(joined, " ");
  }

  return joined;
}
