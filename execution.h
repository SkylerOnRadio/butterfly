#ifndef EXECUTION_H
#define EXECUTION_H

#include <signal.h>
#include <stdbool.h>

extern pid_t child_pid;

int execute(char **args);
int executeBuiltin(int (*func_ptr)(char **), char **args, bool disown);
int executeExternal(char **args, bool disown);

#endif // !EXECUTION_H
