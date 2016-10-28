/** @file libscheduler.h
 */

#ifndef LIBSCHEDULER_H_
#define LIBSCHEDULER_H_

/**
  Stores information making up a job to be scheduled including any statistics.

  You may need to define some global variables or a struct to store your job queue elements.
*/
typedef struct _job_t
{
  int pid;
  int priority;
  int arrivalTime;
  int jobLength;
  int remainingTime;
  int responseTime;
  int lastCheckedTime;
} job_t;

/**
  Constants which represent the different scheduling algorithms
*/
typedef enum {FCFS = 0, SJF, PSJF, PRI, PPRI, RR} scheme_t;

int numCores;
job_t **coreArr;
scheme_t schedule;

int numJobs;
float waitingTime;
float turnaroundTime;
float responseTime;


void  scheduler_start_up               (int cores, scheme_t scheme);
int   scheduler_new_job                (int job_number, int time, int running_time, int priority);
int   scheduler_job_finished           (int core_id, int job_number, int time);
int   scheduler_quantum_expired        (int core_id, int time);
float scheduler_average_turnaround_time();
float scheduler_average_waiting_time   ();
float scheduler_average_response_time  ();
void  scheduler_clean_up               ();
void  scheduler_show_queue             ();

#endif /* LIBSCHEDULER_H_ */
