#include <linux/limits.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// TODO:
// string parsing should be proper and not just differentiating with
// whitespace
// add a history
// detect CTRL+C and kill the child process not the main shell
// background processes should be able to be launched using &
// cleanup the disowned processes when they die
// let > mean that the outputs are put into a file
// let < mean that the input come froms the file
// pipe the output of a command to another

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

int launch(char **args) {
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      perror("butterfly");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // forking error
    perror("butterfly");
  } else {
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

int execute(char **args) {
  if (args[0] == NULL)
    return 1;

  for (int i = 0; i < numOfbuiltin(); ++i) {
    if (strcmp(args[0], builtinCmd[i]) == 0)
      return (*builtinFunc[i])(args);
  }

  return launch(args);
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
    tokens = split_line(line);
    exit = execute(tokens);

    free(line);
    free(tokens);
    free(path);
  } while (exit != 0);

  return exit;
}

int main() {

  loop();

  return 0;
}
