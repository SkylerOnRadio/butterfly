#include "backgroundJobs.h"
#include "parser.h"
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
Job *backgroundJobs;

// TODO:
// background processes should be able to be launched using &
// cleanup the disowned processes when they die
// write built-in functions for clear, pwd and kill
// let > mean that the outputs are put into a file
// let < mean that the input come froms the file
// pipe the output of a command to another
// string parser needs to handle ' quotes within " and " quotes within '
// It also needs to handle \
// add a history

char *builtinCmd[] = {"cd", "help", "exit"};

int cd(char **args);
int help(char **args);
int exitShell(char **args);

int (*builtinFunc[])(char **) = {&cd, &help, &exitShell};

int numOfbuiltin() { return sizeof(builtinCmd) / sizeof(char *); }

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

int cd(char **args) {
  // if the user uses ~ go to home dir
  if (args[1] == NULL || strcmp(args[1], "~") == 0) {
    const char *homedir;
    if ((homedir = getenv("HOME")) == NULL)
      /*gets the users id using getuid  and then getpwuid to get the password
       * entry which includes the home directory of the user
       */
      homedir = getpwuid(getuid())->pw_dir;
    if (homedir == NULL)
      fprintf(stderr, "Home directory not found");
    else if (chdir(homedir) != 0)
      perror("butterfly");
  }

  else if (chdir(args[1]) != 0)
    perror("butterfly");

  return 1;
}

int help(char **args) {
  printf("Butterfly Shell\n");
  printf("The following are built in:\n");

  for (int i = 0; i < numOfbuiltin(); ++i) {
    printf("  %s\n", builtinCmd[i]);
  }
  printf("Use the man command for info on other programs.\n");
  return 1;
}

int exitShell(char **args) { return 0; }

int launch(char **args, bool disown) {
  pid_t wpid;
  int status;

  child_id = fork();
  if (child_id == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      perror("butterfly");
    }
    exit(EXIT_FAILURE);
  } else if (child_id < 0) {
    // forking error
    perror("butterfly");
  } else {
    if (!disown) {
      do {
        wpid = waitpid(child_id, &status, WUNTRACED);
      } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    } else {
      Job *job =
          addBackgroundJob(&backgroundJobs, child_id, joinArguments(args));
      fprintf(stderr, "[%d]: %d\n", job->job_id, job->process_id);
    }
    child_id = -1;
  }

  return 1;
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
    if (strcmp(args[0], builtinCmd[i]) == 0)
      return (*builtinFunc[i])(args);
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
    free(tokens);
    free(path);
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

void handleChildProcessDying(int sig) {
  pid_t pid;
  int status;

  while ((pid = waitpid(-1, &status, WNOHANG)) != -1) {
    deleteBackgroundJob(pid, &backgroundJobs);
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

  backgroundJobs = malloc(sizeof(Job));
  backgroundJobs = NULL;

  loop();

  return 0;
}
