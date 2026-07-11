#ifndef BUILTIN_H
#define BUILTIN_H

#include <stdbool.h>

extern char *builtin[];

int cd(char **args);
int help(char **args);
int exitShell(char **args);
int echo(char **args);
int pwd(char **args);

int numOfbuiltin();

extern int (*builtinFunc[])(char **);

#endif // !BUILTIN_H
