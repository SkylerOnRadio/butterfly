#include "utility.h"
#include <string.h>

int isToBePutToAFile(char **args) {
  for (int i = 0; args[i] != NULL; ++i) {
    if (strcmp(args[i], ">") == 0) {
      args[i] = NULL;
      return i;
    }
  }
  return -1;
}
