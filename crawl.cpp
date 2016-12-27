#include "crawl.h"

CrawerTask::CrawerTask(string host,string resource, ThreadPool *crawlerPool, ThreadPool *analysisPool) : Task(), _host(host), _resource(resource), _crawlerPool(crawlerPool),  _analysisPool(analysisPool) {}
CrawerTask::~CrawerTask() {
	// Debug prints
	pthread_mutex_lock(&console_mutex);
	cout << "deleting job url:" << _resource << endl;
	pthread_mutex_unlock(&console_mutex);		
}
void CrawerTask::run() {
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

	//_analysisPool->addTask(new AnalysisTask(_host,_resource,_crawlerPool,_analysisPool));
}
void CrawerTask::showTask() {
	// More debug printing
	pthread_mutex_lock(&console_mutex);
	cout << "showing job url:" << _resource << endl;
	pthread_mutex_unlock(&console_mutex);		
}
