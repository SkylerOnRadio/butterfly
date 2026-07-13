#include "backgroundJobs.h"
#include "execution.h"
#include "parser.h"
#include <fcntl.h>
#include <linux/limits.h>
#include <pwd.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static volatile sig_atomic_t gotSigchld = 0;
pid_t child_pid = -1;
Job *backgroundJobs = NULL;

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

char *getDir() {
  char *path = getcwd(NULL, 0);
  if (path == NULL) {
    perror("butterfly");
    return "";
  }

  return path;
}

int loop() {
  int exit = 0;

  char *line;
  char **tokens;
  char *path;

  do {
    if (gotSigchld) {
      gotSigchld = 0;
      pid_t pid;
      int status;
      while ((pid = (waitpid(-1, &status, WNOHANG))) > 0) {
        deleteBackgroundJob(pid, &backgroundJobs);
      }
    }

    path = getDir();

    printf("%s > ", path);
    line = read_line();
    tokens = parseLine(line);
    int count = 0;
    while (tokens[count] != NULL)
      count++;

    exit = execute(tokens);

    free(line);
    for (int i = 0; i < count; ++i) {
      if (tokens[i] != NULL)
        free(tokens[i]);
    }
    free(tokens);
    free(path);

  } while (exit != 0);

  return exit;
}

void handleForceQuit(int sig) {
  if (child_pid != -1) {
    kill(child_pid, SIGKILL);
    child_pid = -1;
  } else {
    printf("\n>");
    fflush(stdout);
  }
}

void handleChildProcessDying(int sig) { gotSigchld = 1; }

void freeAllJobs(Job *head) {
  while (head != NULL) {
    Job *next = head->nextJob;
    free(head->command);
    free(head);
    head = next;
  }
}

int main() {
  child_pid = -1;
  struct sigaction actChild, actInt;

  actInt.sa_handler = handleForceQuit;
  sigemptyset(&actInt.sa_mask);
  actInt.sa_flags = SA_RESTART;

  actChild.sa_handler = handleChildProcessDying;
  sigemptyset(&actChild.sa_mask);
  actChild.sa_flags = SA_RESTART;

  sigaction(SIGINT, &actInt, NULL);
  sigaction(SIGCHLD, &actChild, NULL);

  loop();

  freeAllJobs(backgroundJobs);

  return 0;
}
