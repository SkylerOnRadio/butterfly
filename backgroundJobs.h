#ifndef BACKGROUNDJOBS_H
#define BACKGROUNDJOBS_H

#include <signal.h>

typedef struct Job {
  int job_id;
  pid_t process_id;
  char *command;
  int isRunning;
  struct Job *nextJob;
} Job;

extern Job *backgroundJobs;

Job *addBackgroundJob(Job **head, pid_t process_id, char *command);
void deleteBackgroundJob(pid_t process_id, Job **head);

#endif // !BACKGROUNDJOBS_H
