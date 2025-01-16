#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <limits.h>
#include "constants.h"
#include "wbq.h"

extern int stop_threads;//deleted extern
extern int finished_jobs[NUM_CORES];//deleted extern
//extern int flag_array[NUM_CORES];//deleted extern
extern WorkBalancerQueue** processor_queues;//deleted

// Thread function for each core simulator thread
void* processJobs(void* arg) {
    // initalize local variables
    ThreadArguments* my_arg = (ThreadArguments*) arg;
    WorkBalancerQueue* my_queue = my_arg -> q;
    int my_id = my_arg -> id;

    // Main loop, each iteration simulates the processor getting The stop_threads flag
    // is set by the main thread when it concludes all jobs are finished. The bookkeeping 
    // of finished jobs is done in executeJob's example implementation. a task from 
    // its or another processor's queue. After getting a task to execute the thread 
    // should call executeJob to simulate its execution. It is okay if the thread
    //  does busy waiting when its queue is empty and no other job is available
    // outside.
    while (!stop_threads) {
        // TODO: You need to fill in this part
        // .
        // .
        // Use a call to executeJob as stated here.
        Task *task = fetchTask(my_queue);//fetching the task from it's own queue

        if (task == NULL) {
            // If no tasks available, try to fetch a task from other cores
            for (int i = 0; i < NUM_CORES; i++) {
                if (i != my_id) {
                    task = fetchTaskFromOthers(processor_queues[i]);
                    if (task != NULL) {
                        break; // Stop looking once we have fetched a task
                    }
                }
            }

            // If still no task is available, busy wait and continue
            if (task == NULL) {
                continue;
            }
        }


        executeJob(task, my_queue, my_id);//proocessing the fetched task

        //checking if the processsed task has not finished
        if(task->task_duration > 0){
            submitTask(my_queue, task);
        }

        int queue_size = getQueueSize(my_queue);//getting the size of the queue, to check if the size is under threshold or not
        
        //check if the queue size exceeeds the high watermark threshold
        //HW = 20(suggested in the homework document)
        //LW = 10(suggested in the homework document)
        if(queue_size > 20){
            for(int i = 0; i < NUM_CORES; i++){
                if(i != my_id && getQueueSize(processor_queues[i]) < 10){
                    Task* task_to_move = fetchTask(my_queue);
                    if(task_to_move != NULL){
                        submitTask(processor_queues[i], task_to_move);
                        queue_size--;
                    }
                    if(queue_size <= 20){
                        break;
                    }

                }
                
            }
            


        }

        if (queue_size < 10) {
            for (int i = 0; i < NUM_CORES; i++) {
                if (i != my_id && getQueueSize(processor_queues[i]) > 20) {
                    // Fetch a task from another core with many tasks
                    Task* stolen_task = fetchTaskFromOthers(processor_queues[i]);
                    if (stolen_task != NULL) {
                        submitTask(my_queue, stolen_task);
                    }
                }
            }
        }
        
    }
    return NULL;//exiting the thread properly when stop_thread flag is true
}

// Do any initialization of your shared variables here.
// For example initialization of your queues, any data structures 
// you will use for synchronization etc.
void initSharedVariables() {
    // TODO: Fill in this function according to your needs:
    
    stop_threads = 0;//this is the flag that will tell us to stop processing when it is not 0, hence intialize it to 0 to make the keep the threads running

    for(int i = 0; i < NUM_CORES; i++){
        finished_jobs[i] = 0;// initializing all the finished_jobs array to 0 to indicate that none of the cores have finished their jobs
    }
    /*
    for(int i = 0; i < NUM_CORES; i++){
        flag_array[i] = 0;//initializing the flag_array to 0
    }
    */

    processor_queues = (WorkBalancerQueue**)malloc(NUM_CORES * sizeof(WorkBalancerQueue*));//this is an array of queue pointers
    if(processor_queues == NULL){
        fprintf(stderr, "Failed to allocate memory for processor queues");
        exit(1);
    }

    for(int i = 0; i < NUM_CORES; i++){
        processor_queues[i] = (WorkBalancerQueue*)malloc(sizeof(WorkBalancerQueue));
        if (processor_queues[i] == NULL) {
            fprintf(stderr, "Failed to allocate memory for processor queues");
            exit(1);
        }

        WorkBalancerQueue_Init(processor_queues[i]);
    }

}