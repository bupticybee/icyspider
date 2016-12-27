#ifndef _TASK_H
#define _TASK_H

 

 // Base task for Tasks
 // run() should be overloaded and expensive calculations done there
 // showTask() is for debugging and can be deleted if not used
class Task {
public:
Task() {}
virtual ~Task() {}
virtual void run()=0;
virtual void showTask()=0;
};


// Wrapper around std::queue with some mutex protection
class WorkQueue {
public:
WorkQueue(); 
~WorkQueue();
Task *nextTask() ;
// Add a task
void addTask(Task *nt) ;
// Mark the queue finished
void finish() ;
// Check if there's work
bool hasWork() ;
private:
std::queue<Task*> tasks;
bool finished;
pthread_mutex_t qmtx;
pthread_cond_t wcond;
};

// Function that retrieves a task from a queue, runs it and deletes it
void *getWork(void* param) ;

class ThreadPool {
public:
// Allocate a thread pool and set them to work trying to get tasks
ThreadPool(int n);
// Wait for the threads to finish, then delete them
~ThreadPool() ;
// Add a task
void addTask(Task *nt) ;
// Tell the tasks to finish and return
void finish() ;
// Checks if there is work to do
bool hasWork() ;
private:
pthread_t * threads;
int _numThreads;
WorkQueue workQueue;
};

// stdout is a shared resource, so protected it with a mutex
extern static pthread_mutex_t console_mutex ;
extern static pthread_mutex_t crawler_mutex ;
extern static pthread_mutex_t analysis_mutex ;

// Debugging function
void showTask(int n) ;
#endif
