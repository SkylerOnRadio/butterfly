#include "execution.h"
#include "builtin.h"
#include "utility.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int execute(char **args) {
  if (args[0] == NULL)
    return 1;

  int count = 0;
  while (args[count + 1] != 0)
    count++;

  // checking if the command needs to be dettached
  bool disown = false;
  if (strcmp(args[count], "&") == 0) {
    disown = true;
    free(args[count]);
    args[count] = NULL;
  }

  int **pipedIndex = pipeIndex(args);
  if (pipedIndex[0] == NULL) {
    int size = 0;
    while (pipedIndex != NULL)
      ++size;

    for (int i = 0; i < size; ++i) {
      args[*pipedIndex[i]] = NULL;
    }

    pid_t pids[size];
    // each command will have its own pair of read and write file descriptors
    int pipefds[2 * (size - 1)];

    for (int i = 0; i < size; ++i) {
      if (pipe(pipefds + i * 2) < 0) {
        perror("butterfly");
        return 1;
      }
    }

  } else {
    for (int i = 0; i < numOfbuiltin(); ++i) {
      if (strcmp(args[0], builtin[i])) {
        return executeBuiltin(builtinFunc[i], args, disown);
      }

      return executeExternal(args, disown);
    }
  }
}
