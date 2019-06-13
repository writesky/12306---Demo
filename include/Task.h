#pragma once
//������

#ifndef _TASK_H_
#define _TASK_H_

#include<string>
#include"AppInfo.h"

using namespace std;

enum TaskId
{
	ID_LOAD_VCODE = 100,
	ID_LOGIN,
	ID_QUERY_TICKET,
	ID_PRESUBMIT_ORDER,
	ID_CHECK_ORDER,
	//��ȡ��վ��Ϣ
	ID_GET_STATIONINFO
};

//������֤��
#define WM_LOADVCODE       WM_USER+0x0001
//��¼
#define WM_LOGIN            WM_USER + 0x0002
//��Ʊ
#define WM_QUERYTICKETS     WM_USER + 0x0003
//��վ��Ϣ
#define WM_GETSTATIONINFO   WM_USER + 0x0004
//Ԥ�ύ����
#define WM_PRESUBMIT_ORDER  WM_USER + 0x0005
//�ύ����
#define WM_CHECK_ORDER      WM_USER + 0x0006

enum
{
	OPERATION_SUCCESS,
	OPERATION_FAIL,
	OPERATION_NEEDRELOGIN    //��Ҫ���µ�½
};

enum
{
	LOGIN_SUCCESS,
	LOGIN_INCORRECT_VCODE,
	LOGIN_VERIFY_VCODE_ERROR,
	LOGIN_ERROR,
	LOGIN_FAIL
};

//�������
class CTask
{
public:
	CTask();
	virtual ~CTask();

	void SetReflectionWindow(HWND hwnd)
	{
		m_hwndReflection = hwnd;
	}

	virtual bool Run() = 0;

protected:
	HWND m_hwndReflection;
	string m_strUrl;
};

//��ȡ��֤������
class CGetVerificationCodeTask :public CTask
{
public:
	CGetVerificationCodeTask()
	{}

	virtual ~CGetVerificationCodeTask()
	{}

	virtual bool Run() override;
};

//��½����
class CLoginTask :public CTask
{
public:
	CLoginTask(const char* pszUser, const char* pszPassword, const char* pszVCode) :
		m_strUser(pszUser), m_strPassword(pszPassword), m_strVCode(pszVCode) {}

	virtual ~CLoginTask() {}


	virtual bool Run() override;

private:
	string m_strUser;
	string m_strPassword;
	string m_strVCode;
};

//�οͲ�Ʊ
class CGuestQueryTicketTask :public CTask
{
public:
	CGuestQueryTicketTask(const char* train_date, const char* from_station, const char* to_station, const char* purpose_codes) :
		m_strTrainDate(train_date),
		m_strFromStation(from_station),
		m_strToStation(to_station),
		m_strPurposeCodes(purpose_codes)
	{

	}

	virtual ~CGuestQueryTicketTask() {}

	virtual bool Run() override;

private:
	string      m_strTrainDate;
	string      m_strFromStation;
	string      m_strToStation;
	string      m_strPurposeCodes;
};

//��¼�û���Ʊ
class CLoginUserQueryTickTask : public CTask
{
public:
	CLoginUserQueryTickTask(const char* train_date, const char* from_station, const char* to_station, const char* purpose_codes) :
		m_strTrainDate(train_date),
		m_strFromStation(from_station),
		m_strToStation(to_station),
		m_strPurposeCodes(purpose_codes)
	{

	}

	virtual ~CLoginUserQueryTickTask()
	{
	}

	virtual bool Run() override;

private:
	string      m_strTrainDate;
	string      m_strFromStation;
	string      m_strToStation;
	string      m_strPurposeCodes;
};

//��ȡ��վ��Ϣ
class CGetStationInfoTask :public CTask
{
public:
	CGetStationInfoTask(bool bUseLocalCache = false) :m_bUseLocalCache(bUseLocalCache) {}

	virtual ~CGetStationInfoTask()
	{
	}

	virtual bool Run() override;
private:
	bool    m_bUseLocalCache;           //�Ƿ�ʹ�ñ��ػ����վ����Ϣ
};

//Ԥ�ύ����
class CPreSubmitOrderTask : public CTask
{
public:
	CPreSubmitOrderTask(int nSelected) : m_nSelected(nSelected)
	{
	}

	virtual ~CPreSubmitOrderTask()
	{
	}

	virtual bool Run() override;

private:
	int     m_nSelected;                //������ѡ�е�Ʊ������ţ���0��ʼ��
};
class CSubmitOrderTask : public CTask
{
public:
	CSubmitOrderTask(const OrderInfo& oi) : m_orderInfo(oi)
	{
	}

	virtual ~CSubmitOrderTask()
	{
	}

	virtual bool Run() override;

private:
	OrderInfo      m_orderInfo;        //������Ϣ
};
#endif // !_TASK_H_
