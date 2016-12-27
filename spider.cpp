#include <stdio.h>
#include <queue>
#include <unistd.h>
#include <pthread.h>
#include <malloc.h>
#include <stdlib.h>
#include<iostream>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<string.h>
using namespace std;

/**
    TCP Client class
*/
class tcp_client
{
private:
    int sock;
    std::string address;
    int port;
    struct sockaddr_in server;
     
public:
    tcp_client();
    bool conn(string, int);
    bool send_data(string data);
    string receive(int);
};
 
tcp_client::tcp_client()
{
    sock = -1;
    port = 0;
    address = "";
}
 
/**
    Connect to a host on a certain port number
*/
bool tcp_client::conn(string address , int port)
{
    //create socket if it is not already created
    if(sock == -1)
    {
        //Create socket
        sock = socket(AF_INET , SOCK_STREAM , 0);
        if (sock == -1)
        {
            perror("Could not create socket");
        }
         
        cout<<"Socket created\n";
    }
    else    {   /* OK , nothing */  }
     
    //setup address structure
    if(inet_addr(address.c_str()) == -1)
    {
        struct hostent *he;
        struct in_addr **addr_list;
         
        //resolve the hostname, its not an ip address
        if ( (he = gethostbyname( address.c_str() ) ) == NULL)
        {
            //gethostbyname failed
            herror("gethostbyname");
            cout<<"Failed to resolve hostname\n";
             
            return false;
        }
         
        //Cast the h_addr_list to in_addr , since h_addr_list also has the ip address in long format only
        addr_list = (struct in_addr **) he->h_addr_list;
 
        for(int i = 0; addr_list[i] != NULL; i++)
        {
            //strcpy(ip , inet_ntoa(*addr_list[i]) );
            server.sin_addr = *addr_list[i];
             
            cout<<address<<" resolved to "<<inet_ntoa(*addr_list[i])<<endl;
             
            break;
        }
    }
     
    //plain ip address
    else
    {
        server.sin_addr.s_addr = inet_addr( address.c_str() );
    }
     
    server.sin_family = AF_INET;
    server.sin_port = htons( port );
     
    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }
     
    cout<<"Connected\n";
    return true;
}
 
/**
    Send data to the connected host
*/
bool tcp_client::send_data(string data)
{
    //Send some data
    if( send(sock , data.c_str() , strlen( data.c_str() ) , 0) < 0)
    {
        perror("Send failed : ");
        return false;
    }
    cout<<"Data send\n";
     
    return true;
}
 
/**
    Receive data from the connected host
*/
string tcp_client::receive(int size=512)
{
    char buffer[size];
    string reply;
     
    //Receive a reply from the server
    if( recv(sock , buffer , sizeof(buffer) , 0) < 0)
    {
        puts("recv failed");
	string retval = "";
	return retval;
    }
     
    reply = buffer;
    return reply;
}
 

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
WorkQueue() {
	// Initialize the mutex protecting the queue
	pthread_mutex_init(&qmtx,0);

	// wcond is a condition variable that's signaled
	// when new work arrives
	pthread_cond_init(&wcond, 0);
}

~WorkQueue() {
	// Cleanup pthreads
	pthread_mutex_destroy(&qmtx);
	pthread_cond_destroy(&wcond);
}
// Retrieves the next task from the queue
Task *nextTask() {
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
void addTask(Task *nt) {
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
void finish() {
	pthread_mutex_lock(&qmtx);
	finished = true;
	// Signal the condition variable in case any threads are waiting
	pthread_cond_signal(&wcond);
	pthread_mutex_unlock(&qmtx);
}

// Check if there's work
bool hasWork() {
//printf("task queue size is %d\n",tasks.size());
	return (tasks.size()>0);
}

private:
std::queue<Task*> tasks;
bool finished;
pthread_mutex_t qmtx;
pthread_cond_t wcond;
};

// Function that retrieves a task from a queue, runs it and deletes it
void *getWork(void* param) {
Task *mw = 0;
WorkQueue *wq = (WorkQueue*)param;
while (mw = wq->nextTask()) {
	mw->run();
	delete mw;
}
pthread_exit(NULL);
}

class ThreadPool {
public:
// Allocate a thread pool and set them to work trying to get tasks
ThreadPool(int n) : _numThreads(n) {
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
~ThreadPool() {
	workQueue.finish();
	//waitForCompletion();
	for (int i=0; i<_numThreads; ++i) {
		pthread_join(threads[i], 0);
	}
	delete [] threads;
}

// Add a task
void addTask(Task *nt) {
	workQueue.addTask(nt);
}
// Tell the tasks to finish and return
void finish() {
	workQueue.finish();
}

// Checks if there is work to do
bool hasWork() {
	return workQueue.hasWork();
}

private:
pthread_t * threads;
int _numThreads;
WorkQueue workQueue;
};

// stdout is a shared resource, so protected it with a mutex
static pthread_mutex_t console_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t crawler_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t analysis_mutex = PTHREAD_MUTEX_INITIALIZER;

// Debugging function
void showTask(int n) {
pthread_mutex_lock(&console_mutex);
pthread_mutex_unlock(&console_mutex);
}

// Task to compute fibonacci numbers
// It's more efficient to use an iterative algorithm, but
// the recursive algorithm takes longer and is more interesting
// than sleeping for X seconds to show parrallelism
class CrawerTask : public Task {
public:
CrawerTask(string host,string resource, ThreadPool *crawlerPool, ThreadPool *analysisPool) : Task(), _host(host), _resource(resource), _crawlerPool(crawlerPool),  _analysisPool(analysisPool) {}
~CrawerTask() {
	// Debug prints
	pthread_mutex_lock(&console_mutex);
	cout << "deleting job url:" << _resource << endl;
	pthread_mutex_unlock(&console_mutex);		
}
virtual void run() {
	// Note: it's important that this isn't contained in the console mutex lock
	pthread_mutex_lock(&console_mutex);
	cout << "running job url:" << _resource << endl;
	pthread_mutex_unlock(&console_mutex);
	tcp_client c;

	c.conn(_host , 80);
	
	string request = "GET " + _resource + " HTTP/1.1\r\nHost:" + _host + "\r\nConnection:Close\r\n\r\n";
	c.send_data(request);
	string lastcontent = "";
	string fullcontent = "";
	while(true){
		string contentadd = c.receive(1024);
		if (contentadd == lastcontent){
			break;
		}
		lastcontent = contentadd;
		fullcontent += contentadd;
	}
	//cout << fullcontent << endl;

	_analysisPool->addTask(new AnalysisTask(_host,_resource,_crawlerPool,_analysisPool));
}
virtual void showTask() {
	// More debug printing
	pthread_mutex_lock(&console_mutex);
	cout << "showing job url:" << _resource << endl;
	pthread_mutex_unlock(&console_mutex);		
}
private:
// Slow computation of fibonacci sequence
// To make things interesting, and perhaps imporove load balancing, these
// inner computations could be added to the task queue
// Ideally set a lower limit on when that's done
// (i.e. don't create a task for fib(2)) because thread overhead makes it
// not worth it
string _host;
string _resource;
ThreadPool *_crawlerPool;
ThreadPool *_analysisPool;
};

class AnalysisTask : public Task {
public:
AnalysisTask(string host,string resource, ThreadPool *crawlerPool, ThreadPool *analysisPool) : Task(), _host(host), _resource(resource), _crawlerPool(crawlerPool) , _analysisPool(analysisPool) {}
~AnalysisTask() {
	// Debug prints
	pthread_mutex_lock(&console_mutex);
	cout << "deleting job url:" << _resource << endl;
	pthread_mutex_unlock(&console_mutex);		
}
virtual void run() {
	// Note: it's important that this isn't contained in the console mutex lock
	pthread_mutex_lock(&console_mutex);
	cout << "running job url:" << _resource << endl;
	pthread_mutex_unlock(&console_mutex);
}
virtual void showTask() {
	// More debug printing
	pthread_mutex_lock(&console_mutex);
	cout << "showing job url:" << _resource << endl;
	pthread_mutex_unlock(&console_mutex);		
}
private:
// Slow computation of fibonacci sequence
// To make things interesting, and perhaps imporove load balancing, these
// inner computations could be added to the task queue
// Ideally set a lower limit on when that's done
// (i.e. don't create a task for fib(2)) because thread overhead makes it
// not worth it
string _host;
string _resource;
ThreadPool *_crawlerPool;
ThreadPool *_analysisPool;
};

int main(int argc, char *argv[]) 
{
	// Create a thread pool
	ThreadPool *crawlerPool = new ThreadPool(10);
	ThreadPool *analysisPool = new ThreadPool(10);

	// Create work for it
	string starturl = "bbs.hupu.com";
	string startresource = "/";

	crawlerPool->addTask(new CrawerTask(starturl,startresource,crawlerPool,analysisPool));
	delete crawlerPool;

	printf("\n\n\n\n\nCrawler exiting!\n");
}
