//*@desc:    网络线程类，NetThread.h
#pragma once
#ifndef _NETTHREAD_H_
#define _NETTHREAD_H_

#include<list>
#include<vector>
#include<thread>
#include<memory>
#include<mutex>
using namespace std;

class CTask;
struct OrderInfo;

class NetThread
{
public:
	static NetThread& GetInstance();

	void Init(int nThreadNum);
	void Uninit();

	void SetReflectionWindow(HWND h);

	void AddTask(CTask* pTask);

	void AddGetVCodeTask();
	void AddLoginTask(const char* pszUser, const char* pszPassword, const char* pszVCode);
	void AddGuestQueryTicketTask(const char* train_date, const char* from_station, const char* to_station, const char* purpose_codes);
	void AddQueryTicketTask(const char* train_data, const char* from_station, const char* to_station, const char* purpose_codes);
	void AddGetStationTask(bool bUseLocalCache);
	void AddPreSubmitOrderTask(int nSelected);
	void AddSubmitOrderInfoTask(const OrderInfo& oi);

private:
	NetThread();
	~NetThread();
	NetThread(const NetThread&) = delete;
	NetThread& operator=(const NetThread&) = delete;

	void DoTask();

public:
	void NetThreadProc();
private:
	list<CTask*>           m_TaskQueue;
	mutex                  m_mutexTaskQueue;    //多线程取任务时保护任务队列的互斥体
	HANDLE                 m_TaskSemaphore;
	HWND                   m_hwndReflection;
	vector<shared_ptr<thread>>        m_threads;
};
#endif // !_NETTHREAD_H_
