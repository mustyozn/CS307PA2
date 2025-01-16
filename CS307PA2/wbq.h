#ifndef WBQ_H
#define WBQ_H
#include<pthread.h>
// Structs and methods for WorkBalancerQueue, you can use additional structs 
// and data structures ON TOP OF the ones provided here.

// **********************************************************

typedef struct WorkBalancerQueue WorkBalancerQueue;

typedef struct ThreadArguments {
    WorkBalancerQueue* q;
    int id;
} ThreadArguments;

typedef struct Task {
    char* task_id;
    int task_duration;
	double cache_warmed_up;
	WorkBalancerQueue* owner;
} Task;

// TODO: You can modify this struct and add any 
// fields you may need

typedef struct QueueNode{
    Task *task;
    struct QueueNode *next;
}QueueNode;


struct WorkBalancerQueue {
    //this is a collectioon data structure that will be implemented by each core thread to keep track of it's jobs
    //each core will insert and remove jobs to it's Work balancer queue
    //the implementation will be michael and scott queue, if needed we can modify it anytime we want

	QueueNode* head;
	QueueNode* tail;
    pthread_mutex_t head_lock;//this is the lock for the head
    pthread_mutex_t tail_lock;//this is the lock for the tail

};

// **********************************************************

// WorkBalancerQueue API **********************************************************
void submitTask(WorkBalancerQueue* q, Task* _task);
Task* fetchTask(WorkBalancerQueue* q);
Task* fetchTaskFromOthers(WorkBalancerQueue* q);
void WorkBalancerQueue_Init(WorkBalancerQueue* q);//this is a function to initialize the queue
int getQueueSize(WorkBalancerQueue* q);  // This function returns the size of the queue

// You can add more methods to Queue API
// .
// . 
// **********************************************************


// Your simulator threads should call this function to simulate execution. 
// Don't change the function signature, you can use the provided implementation of 
// this function. We will use potentially different implementations while testing.
void executeJob(Task* task, WorkBalancerQueue* my_queue, int my_id );

void* processJobs(void* arg);
void initSharedVariables();
#endif