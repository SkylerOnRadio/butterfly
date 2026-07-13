#include "execution.h"
#include "backgroundJobs.h"
#include "builtin.h"
#include "utility.h"
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/wait.h>
#include <unistd.h>

/* Handling piping requires us to perform the forking here itself since we
 * need all the commands to run simultaneously, if they were to be done by
 * each calling a function call for each command then it would be sequencial
 */
int handlePipedExecution(int **pipedIndex, char **args, bool disown) {
  // get the numOfPipes of the array containing the number of pipes
  int numOfPipes = 0;
  while (pipedIndex[numOfPipes] != NULL)
    ++numOfPipes;

  int numOfCmd = numOfPipes + 1;

  // set each of the index where there is a | to null
  for (int i = 0; i < numOfPipes; ++i) {
    free(args[*pipedIndex[i]]);
    args[*pipedIndex[i]] = NULL;
  }

  pid_t pids[numOfCmd];
  // each command will have its own pair of read and write file descriptors
  int pipefds[2 * (numOfPipes)];

  /* pipe function takes two file descriptors and then makes the first one the
   * read and the second one the write */
  for (int i = 0; i < numOfPipes; ++i) {
    if (pipe(pipefds + i * 2) < 0) {
      perror("butterfly");
      return 1;
    }
  }

  // for each command, make a child fork to execute the command
  for (int i = 0; i < numOfCmd; i++) {
    pids[i] = fork();

    if (pids[i] == 0) {
      /* if the fork is not the first one then set it input stream to the output
       * stream of the command before */
      if (i != 0)
        dup2(pipefds[(i - 1) * 2], STDIN_FILENO);

      /* if the fork is not the last one, set the output stream to the input
       * stream of the next command */
      if (i != numOfCmd - 1)
        dup2(pipefds[i * 2 + 1], STDOUT_FILENO);

      /* close all the file descriptors (why?) */
      for (int j = 0; j < 2 * numOfPipes; j++) {
        close(pipefds[j]);
      }

      // set the starting point of the current command
      char **current_cmd;
      if (i == 0)
        current_cmd = args;
      else
        current_cmd = &args[*pipedIndex[i - 1] + 1];

      execvp(current_cmd[0], current_cmd);
      /* If the exec worked then the executing function itself will do a
       * return and hence the next lines won't be reached if the command
       * executes */
      perror("butterfly");
      exit(EXIT_FAILURE);

    } else if (pids[i] < 0) {
      perror("butterfly");
    }
  }

  // close the file descriptors for the parent too
  for (int i = 0; i < 2 * numOfPipes; i++)
    close(pipefds[i]);

  int status;

  if (!disown) {
    // wait for each command to finish
    for (int i = 0; i < numOfCmd; i++)
      waitpid(pids[i], &status, 0);
  } else {
    // set each command as its own job
    for (int i = 0; i < numOfCmd; i++) {
      char *arg;
      if (i == 0)
        arg = joinArguments(args);
      else
        arg = joinArguments(&args[*pipedIndex[i - 1] + 1]);

      Job *job = addBackgroundJob(&backgroundJobs, pids[i], arg);
      fprintf(stderr, "[%d]: %d\n", job->job_id, job->process_id);
      free(arg);
    }
  }

  for (int i = 0; i < numOfPipes; i++) {
    free(pipedIndex[i]);
  }
  free(pipedIndex);

  return 1;
}

int execute(char **args) {
  if (args[0] == NULL)
    return 1;

  int count = 0;
  while (args[count + 1] != 0)
    count++;

  // checking if the command needs to be dettached
  bool disown = false;
  if (strcmp(args[count], "&") == 0) {
    disown = true;
    free(args[count]);
    args[count] = NULL;
  }

  int **pipedIndex = pipeIndex(args);

  if (pipedIndex[0] != NULL) {
    return handlePipedExecution(pipedIndex, args, disown);
  } else {
    free(pipedIndex);

    for (int i = 0; i < numOfbuiltin(); ++i) {
      if (strcmp(args[0], builtin[i]) == 0) {
        return executeBuiltin(builtinFunc[i], args, disown);
      }
    }
    return executeExternal(args, disown);
  }
}

int executeExternal(char **args, bool disown) {
  pid_t wpid;
  int status;

  child_pid = fork();
  if (child_pid == 0) {
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
  } else if (child_pid < 0) {
    perror("butterfly");
  } else {
    if (!disown) {
      do {
        wpid = waitpid(child_pid, &status, WUNTRACED);
      } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    } else {
      char *arg = joinArguments(args);
      Job *job = addBackgroundJob(&backgroundJobs, child_pid, arg);
      fprintf(stderr, "[%d]: %d\n", job->job_id, job->process_id);
      free(arg);
    }

    child_pid = -1;
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
      args[index + 1] = NULL;
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
      args[index + 1] = NULL;
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
