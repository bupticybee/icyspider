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
#include <time.h>
#include <regex>
#include <map>
#include <fstream>
using namespace std;
#define POOL_SIZE 20
string filename;
int linenum = 0;

static pthread_mutex_t fileout_mutex = PTHREAD_MUTEX_INITIALIZER;

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
    void close_sock();
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

void tcp_client::close_sock() 
{
	close(sock);
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
         
        //cout<<"Socket created\n";
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
             
            //cout<<address<<" resolved to "<<inet_ntoa(*addr_list[i])<<endl;
             
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
     
    //cout<<"Connected\n";
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
    //cout<<"Data send\n";
     
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

// Debugging function
void showTask(int n) {
pthread_mutex_lock(&console_mutex);
pthread_mutex_unlock(&console_mutex);
}


// Task to compute fibonacci numbers
// It's more efficient to use an iterative algorithm, but
// the recursive algorithm takes longer and is more interesting
// tha
class CrawerTask : public Task {
public:
CrawerTask(string host,string resource, ThreadPool *crawlerPool) : Task(), _host(host), _resource(resource), _crawlerPool(crawlerPool)   {}
~CrawerTask() {
	// Debug prints
}

bool filter_url(string strlink){
	bool flag = false;
	if (strlink.find("#") != string::npos || strlink.find("javascript") != string::npos)
		flag = true;
	if (strlink.find(" ") != string::npos)	
		flag = true;
	if (strlink.find("\n") != string::npos)	
		flag = true;
	if (strlink.find("\t") != string::npos)	
		flag = true;
	if (strlink.find("shtmls20") != string::npos)	
		flag = true;
	if (strlink.find("mailto") != string::npos)	
		flag = true;
	if (strlink.find("upload") != string::npos)	
		flag = true;
	return not flag;
}

void extraceUrls(string htmlcontent,string host,string resource){
	map<string,int> mapLink;	//容器用于存放抽取出来的链接和计数
	string line;  //一行数据
	string::size_type st1,st2;
	string strlink;  //一条链接
	string baseurl;	//基准url，用于相对路径
	st1=htmlcontent.find("base href=\"");
	st2=htmlcontent.find("\"",st1+11);
	if(st1!=string::npos&&st2!=string::npos)
	{
	  baseurl=htmlcontent.substr(st1+11,st2-(st1+11));
	}
	st1=0;
	while(true)  //抽取出链接
	{
		st1=htmlcontent.find("href=\"",st1);  //找到链接的开始标记href="
		if(st1!=string::npos)	//若存在链接
		{
			st2=htmlcontent.find("\"",st1+6);	//找到链接的结束标记"
			strlink=htmlcontent.substr(st1+6,st2-(st1+6));		  //截取子字符串，即链接
			if(strlink.find("http://")!=0)  //不是以http://开头的链接加上baseurl
			{
				// strategy to clean some useless url
				if( strlink.find("/") == 0)  
				{ 
					strlink=strlink;
				}
				else if( strlink.find("://") == string::npos)  
				{ 
					strlink= resource + strlink;
				}
				else
				{
					strlink.erase();
					st1=st2+1;
					continue;
				}
				if (filter_url(strlink))
					mapLink[strlink]++;  //将链接加入容器，并计数
			}else if(strlink.find("http://" + host) == 0){
				strlink.replace(0,7 + host.size(),"");
				if (filter_url(strlink))
					mapLink[strlink]++;  //将链接加入容器，并计数
			}
			strlink.erase();
			st1=st2+1;
		}
		else
		{
			break;
		}
	}
	for(map<string,int>::iterator it=mapLink.begin();it!=mapLink.end();it++)
	{
		//cout<<it->first<<"---计数:"<<it->second<<endl;
		map < string, int >::iterator iter;
		iter = bloomMap.find(it->first);
		pthread_mutex_lock(&crawler_mutex);
		if (iter != bloomMap.end()){
			continue;
		}
		bloomMap[it->first] = 1;
		pthread_mutex_unlock(&crawler_mutex);
		_crawlerPool->addTask(new CrawerTask(_host,it->first,_crawlerPool));
	}
	
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
	pthread_mutex_lock(&console_mutex);
	fstream fileout;
	fileout.open(filename, ios::app);
	fileout << ++linenum << " " << _host << _resource << " " <<  fullcontent.length() << endl;
	fileout.close();
	pthread_mutex_unlock(&console_mutex);
	extraceUrls(fullcontent,_host,_resource);
	c.close_sock();
	//cout << fullcontent << endl;
	//_crawlerPool->addTask(new CrawerTask(_host,_resource,_crawlerPool));
}
virtual void showTask() {
	// More debug printing
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
map<string,int> bloomMap;
};


int main(int argc, char *argv[]) 
{
	// Create a thread pool
	if (argc != 3){
		cout << "param format : ./spider [starturl] [outfile]" << endl << "[demo]:  ./spider 10.108.84.118/news.sohu.com/ result";
		exit(EXIT_FAILURE);
	}
	ThreadPool *crawlerPool = new ThreadPool(POOL_SIZE);

	// Create work for it
	string starturl = "10.108.84.118";
	string startresource = "/news.sohu.com/";
	filename = "result.txt";

	string fullurl = argv[1];
	int pos = fullurl.find('/');
	starturl = fullurl.substr(0,pos);
	startresource = fullurl.substr(pos,fullurl.length() - 1);
	
	filename = argv[2];

	pthread_mutex_init(&crawler_mutex,0);
	pthread_mutex_init(&fileout_mutex,0);
	
	crawlerPool->addTask(new CrawerTask(starturl,startresource,crawlerPool));
	int iter = 0;
	while(iter ++ < 1 || crawlerPool->hasWork()){
		sleep(3);
	}
	delete crawlerPool;

	printf("\n\n\n\n\nCrawler exiting!\n");
}
