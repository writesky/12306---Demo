#pragma once
//任务类

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
	//获取车站信息
	ID_GET_STATIONINFO
};

//加载验证码
#define WM_LOADVCODE       WM_USER+0x0001
//登录
#define WM_LOGIN            WM_USER + 0x0002
//查票
#define WM_QUERYTICKETS     WM_USER + 0x0003
//车站信息
#define WM_GETSTATIONINFO   WM_USER + 0x0004
//预提交订单
#define WM_PRESUBMIT_ORDER  WM_USER + 0x0005
//提交订单
#define WM_CHECK_ORDER      WM_USER + 0x0006

enum
{
	OPERATION_SUCCESS,
	OPERATION_FAIL,
	OPERATION_NEEDRELOGIN    //需要重新登陆
};

enum
{
	LOGIN_SUCCESS,
	LOGIN_INCORRECT_VCODE,
	LOGIN_VERIFY_VCODE_ERROR,
	LOGIN_ERROR,
	LOGIN_FAIL
};

//任务基类
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

//拉取验证码任务
class CGetVerificationCodeTask :public CTask
{
public:
	CGetVerificationCodeTask()
	{}

	virtual ~CGetVerificationCodeTask()
	{}

	virtual bool Run() override;
};

//登陆任务
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

//游客查票
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

//登录用户查票
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

//获取车站信息
class CGetStationInfoTask :public CTask
{
public:
	CGetStationInfoTask(bool bUseLocalCache = false) :m_bUseLocalCache(bUseLocalCache) {}

	virtual ~CGetStationInfoTask()
	{
	}

	virtual bool Run() override;
private:
	bool    m_bUseLocalCache;           //是否使用本地缓存的站点信息
};

//预提交订单
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
	int     m_nSelected;                //界面中选中的票行数序号（从0开始）
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
	OrderInfo      m_orderInfo;        //订单信息
};
#endif // !_TASK_H_
