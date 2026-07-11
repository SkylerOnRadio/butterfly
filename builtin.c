#include "builtin.h"
#include <linux/limits.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

char *builtin[] = {"cd", "help", "exit"};

int numOfbuiltin() { return sizeof(builtin) / sizeof(char *); }

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
    printf("  %s\n", builtin[i]);
  }
  printf("Use the man command for info on other programs.\n");
  return 1;
}

int exitShell(char **args) { return 0; }

int (*builtinFunc[])(char **) = {&cd, &help, &exitShell};
