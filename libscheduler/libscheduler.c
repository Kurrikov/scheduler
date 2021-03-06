/** @file libscheduler.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libscheduler.h"
#include "../libpriqueue/libpriqueue.h"

priqueue_t queue;

int FCFScomp(const void * a, const void * b) {
  // always return 1 so that the job is placed at the back
  return 1;
}
int SJFcomp(const void * a, const void * b) {
  return (((job_t*)a)->remainingTime - ((job_t*)b)->remainingTime);
}
int PRIcomp(const void * a, const void * b) {
  int priority = ((job_t*)a)->priority - ((job_t*)b)->priority;
  if (priority == 0) {
    return ((job_t*)a)->arrivalTime - ((job_t*)b)->arrivalTime;
  }
  return priority;
}

/**
  Initalizes the scheduler.

  Assumptions:
    - You may assume this will be the first scheduler function called.
    - You may assume this function will be called only once.
    - You may assume that cores is a positive, non-zero number.
    - You may assume that scheme is a valid scheduling scheme.

  @param cores the number of cores that is available by the scheduler. These cores will be known as core(id=0), core(id=1), ..., core(id=cores-1).
  @param scheme  the scheduling scheme that should be used. This value will be one of the six enum values of scheme_t
*/
void scheduler_start_up(int cores, scheme_t scheme)
{
  numCores = cores;
  schedule = scheme;
  numJobs = 0;
  waitingTime = 0;
  turnaroundTime = 0;
  responseTime = 0;

  // initialize core array to zeros
  coreArr = calloc(numCores, sizeof(job_t));

  switch (schedule) {
    case FCFS:
    case RR:
      priqueue_init(&queue, FCFScomp);
      break;
    case SJF:
    case PSJF:
      priqueue_init(&queue, SJFcomp);
      break;
    case PRI:
    case PPRI:
      priqueue_init(&queue, PRIcomp);
      break;
  }
}


/**
  Called when a new job arrives.

  If multiple cores are idle, the job should be assigned to the core with the
  lowest id.
  If the job arriving should be scheduled to run during the next
  time cycle, return the zero-based index of the core the job should be
  scheduled on. If another job is already running on the core specified,
  this will preempt the currently running job.
  Assumptions:
    - You may assume that every job wil have a unique arrival time.

  @param job_number a globally unique identification number of the job arriving.
  @param time the current time of the simulator.
  @param running_time the total number of time units this job will run before it will be finished.
  @param priority the priority of the job. (The lower the value, the higher the priority.)
  @return index of core job should be scheduled on
  @return -1 if no scheduling changes should be made.

 */
int scheduler_new_job(int job_number, int time, int running_time, int priority)
{
  // initialize new job
  job_t *newJob = malloc(sizeof(job_t));
  newJob->pid = job_number;
  newJob->arrivalTime = time;
  newJob->jobLength = running_time;
  newJob->remainingTime = running_time;
  newJob->priority = priority;
  newJob->responseTime = 0;
  newJob->lastCheckedTime = time;

  // Find the first idle core
  int idleCore = -1;
  for (int i = 0; i < numCores; ++i) {
    if (coreArr[i] == NULL) {
      idleCore = i;
      break;
    }
  }

  if (idleCore != -1) {
      // an idle core was found
      // place new job on idle core
      coreArr[idleCore] = newJob;
      return idleCore;
  } else if (schedule == PPRI) {
    // preempt based on priority

    // index of the core with the job with the lowest priority
    int lowestPriIdx = 0;
    // find the job with the lowest priority
    for (int i = 0; i < numCores; ++i) {
      // do not preempt jobs that have been scheduled this clock cycle
      if (coreArr[i]->arrivalTime != time) {
        // preempt the job with the lowest priority
        if (coreArr[i]->priority > coreArr[lowestPriIdx]->priority) {
          lowestPriIdx = i;
        } else if (coreArr[i]->priority == coreArr[lowestPriIdx]->priority) {
          // if two jobs have identical priorities, preempt the older one
          if (coreArr[i]->arrivalTime > coreArr[lowestPriIdx]->arrivalTime) {
            lowestPriIdx = i;
          }
        }
      }
    }

    if (coreArr[lowestPriIdx]->priority > newJob->priority) {
      //If the job was just scheduled and we are pre-empting it, reset the response time
      if(coreArr[lowestPriIdx]->responseTime == time - coreArr[lowestPriIdx]->arrivalTime) {
         coreArr[lowestPriIdx]->responseTime = -1;
      }
      // replace job with lowest priority with the new job
      priqueue_offer(&queue, coreArr[lowestPriIdx]);
      coreArr[lowestPriIdx] = newJob;
      return lowestPriIdx;
    }
  } else if (schedule == PSJF) {
    // preempt based on SJF

    // index of the core with the job with the longest remaining time
    int longestTimeIdx = 0;
    // find the job with the longest remaining time
    for (int i = 0; i < numCores; ++i) {
      // update the remaining time of each job on the CPU
      // by reducing it by the time that has elapsed since it was last checked
      coreArr[i]->remainingTime -= (time - coreArr[i]->lastCheckedTime);
      coreArr[i]->lastCheckedTime = time;

      // do not preempt jobs that have been scheduled this clock cycle
      if (coreArr[i]->arrivalTime != time) {
        // preempt the job with the longest remaining time
        if (coreArr[i]->remainingTime > coreArr[longestTimeIdx]->remainingTime) {
          longestTimeIdx = i;
        }
      }
    }

    if (coreArr[longestTimeIdx]->remainingTime > newJob->remainingTime) {
      //If the job was just scheduled and we are pre-empting it, reset the response time
      if(coreArr[longestTimeIdx]->responseTime == time - coreArr[longestTimeIdx]->arrivalTime) {
         coreArr[longestTimeIdx]->responseTime = -1;
      }
      // replace job with the longest remaining time with the new job
      priqueue_offer(&queue, coreArr[longestTimeIdx]);
      coreArr[longestTimeIdx] = newJob;
      return longestTimeIdx;
    }
  }

  // no scheduling changes are being made
  // a responseTime of -1 indicates that a job has not been scheduled yet
  newJob->responseTime = -1;
  // printf("deferring job %d to the wait queue\n", newJob->pid);
  priqueue_offer(&queue, newJob);
  return -1;
}


/**
  Called when a job has completed execution.

  The core_id, job_number and time parameters are provided for convenience. You may be able to calculate the values with your own data structure.
  If any job should be scheduled to run on the core free'd up by the
  finished job, return the job_number of the job that should be scheduled to
  run on core core_id.

  @param core_id the zero-based index of the core where the job was located.
  @param job_number a globally unique identification number of the job.
  @param time the current time of the simulator.
  @return job_number of the job that should be scheduled to run on core core_id
  @return -1 if core should remain idle.
 */
int scheduler_job_finished(int core_id, int job_number, int time)
{
  waitingTime += time - coreArr[core_id]->jobLength - coreArr[core_id]->arrivalTime;
  turnaroundTime += time - coreArr[core_id]->arrivalTime;
  responseTime += coreArr[core_id]->responseTime;
  ++numJobs;

  // free memory held by completed job
  free(coreArr[core_id]);
  coreArr[core_id] = NULL;

  // check waiting queue for a job to schedule
  if (priqueue_size(&queue) != 0) {
    coreArr[core_id] = priqueue_poll(&queue);
    coreArr[core_id]->lastCheckedTime = time;
    // check if the job has not been scheduled before
    if (coreArr[core_id]->responseTime == -1) {
      // set the responseTime to the delay between the job arriving and the job being scheduled
      coreArr[core_id]->responseTime = time - coreArr[core_id]->arrivalTime;
      // printf("job %d response time: %d\n", coreArr[core_id]->pid, coreArr[core_id]->responseTime);
    }
    return coreArr[core_id]->pid;
  }
  // indicate the core is left idle
	return -1;
}


/**
  When the scheme is set to RR, called when the quantum timer has expired
  on a core.

  If any job should be scheduled to run on the core free'd up by
  the quantum expiration, return the job_number of the job that should be
  scheduled to run on core core_id.

  @param core_id the zero-based index of the core where the quantum has expired.
  @param time the current time of the simulator.
  @return job_number of the job that should be scheduled on core cord_id
  @return -1 if core should remain idle
 */
int scheduler_quantum_expired(int core_id, int time)
{
  //Check if there is no job and if the queue is empty
  if (coreArr[core_id] == NULL && priqueue_size(&queue) == 0) {
    //Core should stay idle
    return -1;
  }
  else {
    //Load job onto the back of the queue
    priqueue_offer(&queue, coreArr[core_id]);
  }

  //Pull job from front of the queue
  coreArr[core_id] = priqueue_poll(&queue);
  //Check to see if the job has been scheduled before
  if (coreArr[core_id]->responseTime == -1) {
    //Set the response time
    coreArr[core_id]->responseTime = time - coreArr[core_id]->arrivalTime;
  }
	return coreArr[core_id]->pid;
}


/**
  Returns the average waiting time of all jobs scheduled by your scheduler.
  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average waiting time of all jobs scheduled.
 */
float scheduler_average_waiting_time()
{
  if (numJobs != 0){
    return waitingTime / numJobs;
  }
	return 0.0;
}


/**
  Returns the average turnaround time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average turnaround time of all jobs scheduled.
 */
float scheduler_average_turnaround_time()
{
  if (numJobs != 0){
    return turnaroundTime / numJobs;
  }
	return 0.0;
}


/**
  Returns the average response time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average response time of all jobs scheduled.
 */
float scheduler_average_response_time()
{
  if (numJobs != 0){
    return responseTime / numJobs;
  }
	return 0.0;
}


/**
  Free any memory associated with your scheduler.

  Assumptions:
    - This function will be the last function called in your library.
*/
void scheduler_clean_up()
{
  for (int i = 0; i < numCores; ++i) {
    // free non NULL cores
    if (coreArr[i] != NULL) {
      free(coreArr[i]);
    }
  }
  free(coreArr);
}


/**
  This function may print out any debugging information you choose. This
  function will be called by the simulator after every call the simulator
  makes to your scheduler.

  In our provided output, we have implemented this function to list the jobs in
  the order they are to be scheduled. Furthermore, we have also listed the current
  state of the job (either running on a given core or idle). For example, if we
  have a non-preemptive algorithm and job(id=4) has began running, job(id=2)
  arrives with a higher priority, and job(id=1) arrives with a lower priority,
  the output in our sample output will be:

    2(-1) 4(0) 1(-1)

  This function is not required and will not be graded. You may leave it
  blank if you do not find it useful.
 */
void scheduler_show_queue()
{

}
