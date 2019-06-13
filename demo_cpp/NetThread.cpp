/**
*@desc:    网络线程类，NetThread.cpp
*/
#include"stdafx.h"
#include"Client12306.h"
#include<time.h>
#include"AppInfo.h"
#include"Task.h"
#include"NetThread.h"

extern HANDLE g_hExitEvent;

NetThread& NetThread::GetInstance()
{
	static NetThread nt;
	return nt;
}

void NetThread::Init(int nThreadNum)
{
	if (nThreadNum <= 4)
		nThreadNum = 4;
	m_threads.resize(nThreadNum);
	for (int i = 0; i < nThreadNum; ++i)
	{
		m_threads[i].reset(new thread(std::bind(&NetThread::NetThread::NetThreadProc, this)));
	}
}

void NetThread::Uninit()
{
	size_t nThreadNum = m_threads.size();
	for (size_t i = 0; i < nThreadNum; ++i)
	{
		m_threads[i]->join();
	}
}
void NetThread::SetReflectionWindow(HWND h)
{
	m_hwndReflection = h;
}

NetThread::NetThread()
{
	m_TaskSemaphore = ::CreateSemaphore(NULL, 0, 0xFFFFFF, NULL);
}
NetThread::~NetThread()
{
	::CloseHandle(m_TaskSemaphore);
}
void NetThread::AddTask(CTask* pTask)
{
	{
		lock_guard<mutex> guard(m_mutexTaskQueue);
		m_TaskQueue.push_back(pTask);
	}

	::ReleaseSemaphore(m_TaskSemaphore, 1, NULL);
}
void NetThread::AddGetVCodeTask()
{
	AppInfo::GetInstance().SetLoginVCodeFilename("");
	CGetVerificationCodeTask* pTask = new CGetVerificationCodeTask();
	AddTask(pTask);
}

void NetThread::AddLoginTask(const char* pszUser, const char* pszPassword, const char* pszVCode)
{
	if (pszUser == NULL || pszPassword == NULL || pszVCode == NULL)
		return;

	CLoginTask* pTask = new CLoginTask(pszUser, pszPassword, pszVCode);
	AddTask(pTask);
}
void NetThread::AddGuestQueryTicketTask(const char* train_date, const char* from_station, const char* to_station, const char* purpose_codes)
{
	if (train_date == NULL || from_station == NULL || to_station == NULL || purpose_codes == NULL)
		return;

	CGuestQueryTicketTask* pTask = new CGuestQueryTicketTask(train_date, from_station, to_station, purpose_codes);
	AddTask(pTask);
}

void  NetThread::AddQueryTicketTask(const char* train_date, const char* from_station, const char* to_station, const char* purpose_codes)
{
	if (train_date == NULL || from_station == NULL || to_station == NULL || purpose_codes == NULL)
		return;

	CLoginUserQueryTickTask* pTask = new CLoginUserQueryTickTask(train_date, from_station, to_station, purpose_codes);
	AddTask(pTask);
}

void NetThread::AddGetStationTask(bool bUseLocalCache)
{
	CGetStationInfoTask* pTask = new CGetStationInfoTask(bUseLocalCache);
	AddTask(pTask);
}
void NetThread::AddPreSubmitOrderTask(int nSelected)
{
	CPreSubmitOrderTask* pTask = new CPreSubmitOrderTask(nSelected);
	AddTask(pTask);
}

void NetThread::AddSubmitOrderInfoTask(const OrderInfo& oi)
{
	CSubmitOrderTask* pTask = new CSubmitOrderTask(oi);
	AddTask(pTask);
}
void NetThread::NetThreadProc()
{
	HANDLE handles[2];
	handles[0] = g_hExitEvent;
	handles[1] = m_TaskSemaphore;
	while (true)
	{
		DWORD dwRet = WaitForMultipleObjects(2, handles, FALSE, INFINITE);
		if (dwRet == WAIT_OBJECT_0)
			break;
		else if (dwRet == WAIT_OBJECT_0 + 1)
		{
			DoTask();
		}
	}

}
void NetThread::DoTask()
{
	CTask* pTask = NULL;
	{
		//FIXME: 这里不判断队列是否为空，会不会有问题？
		lock_guard<mutex> guard(m_mutexTaskQueue);
		pTask = m_TaskQueue.front();
		m_TaskQueue.pop_front();
	}

	pTask->SetReflectionWindow(m_hwndReflection);
	pTask->Run();

	delete pTask;
}