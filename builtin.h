#ifndef BUILTIN_H
#define BUILTIN_H

extern char *builtin[];

int cd(char **args);
int help(char **args);
int exitShell(char **args);

int numOfbuiltin();

extern int (*builtinFunc[])(char **);

#endif // !BUILTIN_H
