#include "backgroundJobs.h"
#include "builtin.h"
#include "parser.h"
#include "utility.h"
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

static pid_t child_id = -1;
static volatile sig_atomic_t gotSigchld = 0;
Job *backgroundJobs;

// TODO:
// pipe the output of a command to another
// string parser needs to handle ' quotes within " and " quotes within '
// It also needs to handle \
// add a history

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

int launch(char **args, bool disown) {
  pid_t wpid;
  int status;

  child_id = fork();
  if (child_id == 0) {
    // Child process
    int index = isToBePutToAFile(args);
    if (index != -1) {
      setOutputToFile(args, index);
      args[index] = NULL;
    }
    index = isToBeReadFromAFile(args);
    if (index != -1) {
      setInputFromFile(args, index);
      args[index] = NULL;
    }

    if (execvp(args[0], args) == -1) {
      perror("butterfly");
      exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
  } else if (child_id < 0) {
    // forking error
    perror("butterfly");
  } else {
    if (!disown) {
      do {
        wpid = waitpid(child_id, &status, WUNTRACED);
      } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    } else {
      char *arg = joinArguments(args);
      Job *job = addBackgroundJob(&backgroundJobs, child_id, arg);
      fprintf(stderr, "[%d]: %d\n", job->job_id, job->process_id);
      free(arg);
    }
    child_id = -1;
  }

  return 1;
}

int executeBuiltin(int (*func_ptr)(char **), char **args, bool disown) {
  if (disown) {
    pid_t pid;

    pid = fork();
    if (pid == 0) {
      int index = isToBePutToAFile(args);
      if (index != -1) {
        setOutputToFile(args, index);
        args[index] = NULL;
      }
      index = isToBeReadFromAFile(args);
      if (index != -1) {
        setInputFromFile(args, index);
        args[index] = NULL;
      }

      func_ptr(args);
      exit(EXIT_SUCCESS);
    } else if (pid < 0) {
      perror("butterfly");
    } else {
      char *arg = joinArguments(args);
      Job *job = addBackgroundJob(&backgroundJobs, pid, arg);
      fprintf(stderr, "[%d]: %d\n", job->job_id, job->process_id);
      free(arg);
    }
    return 1;

  } else {
    int index = isToBePutToAFile(args);
    int saved_stdout = -1;
    if (index != -1) {
      int fd;

      if (strcmp(args[index], ">") == 0)
        fd = open(args[index + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
      else
        fd = open(args[index + 1], O_WRONLY | O_CREAT | O_APPEND, 0644);

      if (fd < 0) {
        perror("butterfly");
        return 1;
      }

      saved_stdout = dup(STDOUT_FILENO);

      dup2(fd, STDOUT_FILENO);
      close(fd);

      free(args[index + 1]);
      free(args[index]);

      args[index] = NULL;
    }

    index = isToBeReadFromAFile(args);
    int saved_stdin = -1;
    if (index != -1) {
      int fd = open(args[index + 1], O_RDONLY);
      if (fd < 0) {
        perror("butterfly");
        return 1;
      }

      saved_stdin = dup(STDIN_FILENO);

      dup2(fd, STDIN_FILENO);
      close(fd);

      free(args[index + 1]);
      free(args[index]);

      args[index] = NULL;
    }

    int returnVal = func_ptr(args);

    if (saved_stdout != -1) {
      dup2(saved_stdout, STDOUT_FILENO);
      close(saved_stdout);
    }
    if (saved_stdin != -1) {
      dup2(saved_stdin, STDIN_FILENO);
      close(saved_stdin);
    }

    return returnVal;
  }
}

int execute(char **args) {
  if (args[0] == NULL)
    return 1;

  int i = 0;
  while (args[i + 1] != NULL)
    ++i;

  bool disown = false;
  if (strcmp(args[i], "&") == 0) {
    args[i] = NULL;
    disown = true;
  }

  for (int i = 0; i < numOfbuiltin(); ++i) {
    if (strcmp(args[0], builtin[i]) == 0)
      return executeBuiltin(builtinFunc[i], args, disown);
  }

  return launch(args, disown);
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
    path = getDir();

    printf("%s > ", path);
    line = read_line();
    tokens = parseLine(line);
    exit = execute(tokens);

    free(line);
    for (int i = 0; tokens[i] != NULL; ++i)
      free(tokens[i]);
    free(tokens);
    free(path);

    if (gotSigchld) {
      gotSigchld = 0;
      pid_t pid;
      int status;
      while ((pid = (waitpid(-1, &status, WNOHANG))) > 0) {
        deleteBackgroundJob(pid, &backgroundJobs);
      }
    }
  } while (exit != 0);

  return exit;
}

void handleForceQuit(int sig) {
  if (child_id != -1) {
    kill(child_id, SIGKILL);
    child_id = -1;
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
