#ifndef UTILITY_H
#define UTILITY_H

int isToBePutToAFile(char **args);
int isToBeReadFromAFile(char **args);
int **pipeIndex(char **args);
int setOutputToFile(char **args, int index);
int setInputFromFile(char **args, int index);

char *joinArguments(char **args);

#endif // !UTILITY_H
