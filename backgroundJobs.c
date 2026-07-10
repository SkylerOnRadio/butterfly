#include "backgroundJobs.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Job *addBackgroundJob(Job **head, pid_t process_id, char *command) {
  // create the background job
  Job *job = malloc(sizeof(Job));

  if (job == NULL) {
    perror("butterfly");
    exit(EXIT_FAILURE);
  }

  // change states since structs are public, job id will be +1 of the last job
  // id in the linked list
  job->process_id = process_id;
  job->command = strdup(command);
  job->isRunning = true;
  job->nextJob = NULL;

  if (*head == NULL) {
    job->job_id = 1;
    *head = job;
    return *head;
  }

  Job *tmp = *head;
  while (tmp->nextJob != NULL) {
    tmp = tmp->nextJob;
  }

  job->job_id = tmp->job_id + 1;

  tmp->nextJob = job;

  return tmp->nextJob;
}

void deleteBackgroundJob(pid_t process_id, Job **head) {
  if (*head == NULL) {
    fprintf(stderr,
            "Background process %d was asked to be deleted but there are no "
            "background tasks at all.",
            process_id);
    return;
  }

  Job *tmp = *head;
  Job *prev = NULL;

  while (tmp != NULL && tmp->process_id != process_id) {
    prev = tmp;
    tmp = tmp->nextJob;
  }

  if (tmp == NULL) {
    fprintf(stderr,
            "Background process %d was asked to be deleted but there are no "
            "background tasks with that process id",
            process_id);
    return;
  }

  fprintf(stderr, "\n[%d]: Done %s &\n", tmp->job_id, tmp->command);
  if (prev == NULL) {
    *head = tmp->nextJob;
  } else {
    prev->nextJob = tmp->nextJob;
  }

  if (tmp->command != NULL)
    free(tmp->command);

  free(tmp);
}

// a function to join all the arguments in the argument array
char *joinArguments(char **args) {
  if (args == NULL || args[0] == NULL)
    return NULL;

  char *joined;
  int lenght = 0;

  for (int i = 0; args[i] != NULL; ++i) {
    lenght += strlen(args[i]) + 1;
  }

  joined = malloc(sizeof(char) * lenght);
  if (joined == NULL) {
    perror("butterfly");
    exit(EXIT_FAILURE);
  }

  joined[0] = '\0';

  for (int i = 0; args[i] != NULL; ++i) {
    strcat(joined, args[i]);
    if (args[i + 1] != NULL)
      strcat(joined, " ");
  }

  return joined;
}
