#include "task.h"
Task::WorkQueue() {
	// Initialize the mutex protecting the queue
	pthread_mutex_init(&qmtx,0);

	// wcond is a condition variable that's signaled
	// when new work arrives
	pthread_cond_init(&wcond, 0);
}

Task::~WorkQueue() {
	// Cleanup pthreads
	pthread_mutex_destroy(&qmtx);
	pthread_cond_destroy(&wcond);
}
// Retrieves the next task from the queue
Task Task::*nextTask() {
	// The return value
	Task *nt = 0;

	// Lock the queue mutex
	pthread_mutex_lock(&qmtx);
	// Check if there's work
	if (finished && tasks.size() == 0) {
		// If not return null (0)
		nt = 0;
	} else {
		// Not finished, but there are no tasks, so wait for
		// wcond to be signalled
		if (tasks.size()==0) {
			pthread_cond_wait(&wcond, &qmtx);
		}
		// get the next task
		nt = tasks.front();
		if(nt){
		tasks.pop();
	}

		// For debugging
		if (nt) nt->showTask();
	}
	// Unlock the mutex and return
	pthread_mutex_unlock(&qmtx);
	return nt;
}
// Add a task
void Task::addTask(Task *nt) {
	// Only add the task if the queue isn't marked finished
	if (!finished) {
		// Lock the queue
		pthread_mutex_lock(&qmtx);
		// Add the task
		tasks.push(nt);
		// signal there's new work
		pthread_cond_signal(&wcond);
		// Unlock the mutex
		pthread_mutex_unlock(&qmtx);
	}
}
// Mark the queue finished
void Task::finish() {
	pthread_mutex_lock(&qmtx);
	finished = true;
	// Signal the condition variable in case any threads are waiting
	pthread_cond_signal(&wcond);
	pthread_mutex_unlock(&qmtx);
}

// Check if there's work
bool Task::hasWork() {
//printf("task queue size is %d\n",tasks.size());
	return (tasks.size()>0);
}
void *getWork(void* param) {
Task *mw = 0;
WorkQueue *wq = (WorkQueue*)param;
while (mw = wq->nextTask()) {
	mw->run();
	delete mw;
}
pthread_exit(NULL);
}

ThreadPool::ThreadPool(int n) : _numThreads(n) {
int rc;
	printf("Creating a thread pool with %d threads\n", n);
	threads = new pthread_t[n];
	for (int i=0; i< n; ++i) {
		rc = pthread_create(&(threads[i]), 0, getWork, &workQueue);
	if (rc){
	 printf("ERROR; return code from pthread_create() is %d\n", rc);
	 exit(-1);
		}
	}
}

// Wait for the threads to finish, then delete them
ThreadPool::~ThreadPool() {
	workQueue.finish();
	//waitForCompletion();
	for (int i=0; i<_numThreads; ++i) {
		pthread_join(threads[i], 0);
	}
	delete [] threads;
}

// Add a task
void ThreadPool::addTask(Task *nt) {
	workQueue.addTask(nt);
}
// Tell the tasks to finish and return
void ThreadPool::finish() {
	workQueue.finish();
}

// Checks if there is work to do
bool ThreadPool::hasWork() {
	return workQueue.hasWork();
}

static pthread_mutex_t console_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t crawler_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t analysis_mutex = PTHREAD_MUTEX_INITIALIZER;

void showTask(int n) {
pthread_mutex_lock(&console_mutex);
pthread_mutex_unlock(&console_mutex);
}
