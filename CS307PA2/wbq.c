#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include "wbq.h"

// Do your WorkBalancerQueue implementation here. 
// Implement the 3 given methods here. You can add 
// more methods as you see necessary.

void submitTask(WorkBalancerQueue* q, Task* _task) {
	// TODO: Implement submitTask
    //this function adds a new job/task to the tail end of the queue and can only be called by the owner
    QueueNode *tmp = (QueueNode*)malloc(sizeof(QueueNode));
    assert(tmp != NULL);//making sure that allocation succeedes

    tmp->task = _task;
    tmp->next = NULL;//this node will be the tail, hence the next pointer needs to be NULL

    pthread_mutex_lock(&q->tail_lock);//locking for exclusion

    q->tail->next = tmp;//adding the node to the end of the queue
    q->tail = tmp;//assigning tail to tmp

    pthread_mutex_unlock(&q->tail_lock);


}

void WorkBalancerQueue_Init(WorkBalancerQueue *q){
    QueueNode *tmp = (QueueNode*)malloc(sizeof(QueueNode));//allocating the dummy node
    assert(tmp != NULL);//making sure that the memory allocation is successful

    tmp->task = NULL;//it is a dummy node hence it does not contain any task
    tmp->next = NULL;//it is the only node hence it points to NULL

    q->head = tmp;//head is tmp
    q->tail = tmp;//tail is tmp

    //intializing the mutexes
    pthread_mutex_init(&q->head_lock, NULL);
    pthread_mutex_init(&q->tail_lock, NULL); 

}

int getQueueSize(WorkBalancerQueue *q){
    if(q == NULL){
        return 0;
    }
    int count = 0;
    pthread_mutex_lock(&q->head_lock);//lock the lock to avoid rw issues
    pthread_mutex_lock(&q->tail_lock);
    QueueNode *current = q->head->next;//skip the dummy node
    //count how many elements there are 
    while(current != NULL){
        count++;
        current = current->next;
    }
    pthread_mutex_unlock(&q->tail_lock);
    pthread_mutex_unlock(&q->head_lock);//unlock the lock

    return count;
}


Task* fetchTask(WorkBalancerQueue* q) {
	// TODO: Implement fetchTask
    //this function removes the next available job from the tail end of the queue
    pthread_mutex_lock(&q->head_lock);//locking the head for exclusion

    QueueNode *old_head = q->head;//getting the old head, which is the dummy node
    QueueNode *new_head = old_head->next;//new_head is the node that we are trying to fetch the task from
    if(new_head == NULL){
        //if the queue is empty we return NULL to indicate that there are no elements in the queue
        pthread_mutex_unlock(&q->head_lock);
        return NULL;
    }

    Task* task = new_head->task;
    q->head = new_head;
    pthread_mutex_unlock(&q->head_lock);
    free(old_head);//we need to free back the memory

    return task;

}
/*
Task* fetchTaskFromOthers(WorkBalancerQueue* q) {
	// TODO: Implement fetchTaskFromOthers
    // Lock the head for exclusive access, as this function will be called by non-owner threads
    //the implementation of this function is the same as fetchTask the only difference is the semantics
    pthread_mutex_lock(&q->head_lock);

    QueueNode* old_head = q->head;  // Capture the current head of the queue
    QueueNode* new_head = old_head->next;  // The new head will be the next node

    // If the queue is empty after the current head (only the dummy node is left)
    if (new_head == NULL) {
        pthread_mutex_unlock(&q->head_lock);
        return NULL;  // No tasks to steal, return NULL
    }

    // Retrieve the task from the new head
    Task* task = new_head->task;
    q->head = new_head;  // Update the head pointer to point to the new head

    pthread_mutex_unlock(&q->head_lock);  // Unlock the head lock
    free(old_head);  // Free the old head (the dummy node)

    return task;  // Return the fetched task
}
*/


Task* fetchTaskFromOthers(WorkBalancerQueue* q) {
	// TODO: Implement fetchTaskFromOthers
    // Lock the tail for exclusive access, as this function will be called by non-owner threads
    //the implementation of this function is the same as fetchTask the only difference is we get the node in the tail instead of the head
    pthread_mutex_lock(&q->tail_lock);

    // If there's only the dummy node, there's nothing to fetch
    if (q->head == q->tail) {
        pthread_mutex_unlock(&q->tail_lock);
        return NULL;
    }


    // Traverse the queue to find the second-to-last node
    QueueNode* current = q->head;

    // Traverse until you reach the second-to-last node
    while (current->next != q->tail) {
        current = current->next;
    }

    // 'current' now points to the second-to-last node, 'q->tail' is the last node
    QueueNode* tail_node = q->tail;  //we will remove this node
    Task* task = tail_node->task;    // tail node contains the task to return

    // Update the tail pointer to point to the second-to-last node
    q->tail = current;//the node before the tail is the new tail now
    current->next = NULL;//make the new tail point to NULL

    // Unlock the tail lock
    pthread_mutex_unlock(&q->tail_lock);

    // Free the removed node (the old tail)
    free(tail_node);

    // returning the fetched task
    return task;
}