#ifndef EXECUTION_H
#define EXECUTION_H

#import "stdbool.h"

int execute(char **args);
int executeBuiltin(int (*func_ptr)(char **), char **args, bool disown);
int executeExternal(char **args, bool disown);

#endif // !EXECUTION_H
