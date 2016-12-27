#ifndef _CRAWL_H
#define _CRAWL_H
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
#include "task.h"
#include "tcp.h"

using namespace std;
class CrawerTask : public Task {
public:
CrawerTask(string host,string resource, ThreadPool *crawlerPool, ThreadPool *analysisPool) ;
~CrawerTask(); 
virtual void run();
virtual void showTask();
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

#endif
