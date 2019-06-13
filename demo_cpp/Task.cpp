#include"stdafx.h"
#include"Client12306.h"
#include"NetThread.h"
#include"AppInfo.h"
#include"Task.h"

CTask::~CTask()
{}

CTask::CTask()
{

}

bool CGetVerificationCodeTask::Run()
{
	Client12306& client12306 = Client12306::GetInstance();
	WPARAM wParam = OPERATION_FAIL;
	if (Client12306::GetInstance().ReloadVCodeImage())
	{
		char szVCodeFileName[256];
		memset(szVCodeFileName, 0, sizeof(szVCodeFileName));
		client12306.GetVCodeFileName(szVCodeFileName, ARRAYSIZE(szVCodeFileName));

		AppInfo::GetInstance().SetLoginVCodeFilename(szVCodeFileName);
		wParam = OPERATION_SUCCESS;
	}
	::PostMessage(m_hwndReflection, WM_LOADVCODE, wParam, 0);
	return true;
}

bool CLoginTask::Run()
{
	Client12306& client12306 = Client12306::GetInstance();
	NetThread& nt = NetThread::GetInstance();
	int ret = client12306.checkRandCodeAnsyn(m_strVCode.c_str());
	if (ret == 1)
	{
		PostMessage(m_hwndReflection, WM_LOGIN, LOGIN_INCORRECT_VCODE, 0);
		//重新加载验证码
		//nt.AddGetVCodeTask();

		return false;
	}
	else if (ret == -1)
	{
		PostMessage(m_hwndReflection, WM_LOGIN, LOGIN_VERIFY_VCODE_ERROR, 0);
		//重新加载验证码
		//nt.AddGetVCodeTask();
		return false;
	}

	ret = client12306.loginAysnSuggest(m_strUrl.c_str(), m_strPassword.c_str(), m_strVCode.c_str());
	if (ret == -1)
	{
		PostMessage(m_hwndReflection, WM_LOGIN, LOGIN_ERROR, 0);
		return false;
	}
	else if (ret == -1)
	{
		PostMessage(m_hwndReflection, WM_LOGIN, LOGIN_ERROR, 0);
		return false;
	}

	//正式登陆
	if (!client12306.initMy12306())
	{
		return false;
	}

	//再次拉取购票验证码
	if (!client12306.DownloadVCodeImage("passenger"))
	{
		return false;
	}

	PostMessage(m_hwndReflection, WM_LOGIN, LOGIN_SUCCESS, 0);
	return true;
}

bool CLoginUserQueryTickTask::Run()
{
	WPARAM wParam = OPERATION_SUCCESS;
	Client12306& client12306 = Client12306::GetInstance();
	if (client12306.QueryTickets1(m_strTrainDate.c_str(), m_strFromStation.c_str(), m_strToStation.c_str(), m_strPurposeCodes.c_str()))
	{
		vector<ticketinfo> info;
		if ((!client12306.QueryTickets2(m_strTrainDate.c_str(), m_strFromStation.c_str(), m_strToStation.c_str(), m_strPurposeCodes.c_str(), info)))
			wParam = OPERATION_FAIL;
		AppInfo::GetInstance().SetQueryTicketInfoResult(info);
	}
	else
		wParam = OPERATION_FAIL;
	PostMessage(m_hwndReflection, WM_QUERYTICKETS, wParam, 0);
	return true;
}

bool CGuestQueryTicketTask::Run()
{
	WPARAM wParam = OPERATION_SUCCESS;
	vector<ticketinfo> info;
	if (Client12306::GetInstance().GuestQueryTicket(m_strTrainDate.c_str(), m_strFromStation.c_str(), m_strToStation.c_str(), m_strPurposeCodes.c_str(), info))
	{
		wParam = OPERATION_SUCCESS;
		AppInfo::GetInstance().SetQueryTicketInfoResult(info);
	}
	else
		wParam = OPERATION_FAIL;
	PostMessage(m_hwndReflection, WM_QUERYTICKETS, wParam, 0);

	return true;
}

bool CGetStationInfoTask::Run()
{
	//TODO: 最好是本地缓存一份，不然由于数据很多，加载和解析会非常慢
	WPARAM wParam = OPERATION_SUCCESS;
	vector<stationinfo> info;

	if (!Client12306::GetInstance().GetStationInfo(info, !m_bUseLocalCache))
		wParam = OPERATION_FAIL;
	AppInfo::GetInstance().SetStationInfo(info);
	PostMessage(m_hwndReflection, WM_GETSTATIONINFO, wParam, 0);

	return true;
}

bool CPreSubmitOrderTask::Run()
{
	AppInfo& appInfo = AppInfo::GetInstance();
	ticketinfo ti;
	appInfo.GetTicketInfoByIndex(ti);
	Client12306& client12306 = Client12306::GetInstance();
	if (!client12306.checkUser())
	{
		PostMessage(m_hwndReflection, WM_PRESUBMIT_ORDER, OPERATION_NEEDRELOGIN, 0);
		return false;
	}

	QueryTicketInfo qti;
	appInfo.GetQueryTicketInfoParams(qti);
	//回程日期暂且设置成当天
	char back_train_date[16] = { 0 };
	time_t now = time(NULL);
	//TODO: 这个函数非线程安全，改成localtime_r
	struct tm* tmstr = localtime(&now);
	sprintf_s(back_train_date, ARRAYSIZE(back_train_date), "%04d-%02d-%02d", tmstr->tm_year + 1900, tmstr->tm_mon + 1, tmstr->tm_mday);
	//提交订单
	if (!client12306.submitOrderRequest(ti.secretStr.c_str(), qti.train_date.c_str(), back_train_date, "dc", qti.purpose_codes.c_str(), ti.DTO.from_station_name.c_str(), ti.DTO.to_station_name.c_str()));
	{
		PostMessage(m_hwndReflection, WM_PRESUBMIT_ORDER, OPERATION_FAIL, 0);
		return false;
	}

	//这步可能要调整到实际提交订单的前面去获取REPEAT_SUBMIT_TOKEN
	if (!client12306.initDc())
	{
		PostMessage(m_hwndReflection, WM_PRESUBMIT_ORDER, OPERATION_FAIL, 0);
		return false;
	}

	vector<passenager> p;
	if (!client12306.getPassengerDTOs(p))
	{
		PostMessage(m_hwndReflection, WM_PRESUBMIT_ORDER, OPERATION_FAIL, 0);
		return false;
	}

	//TODO: 获取验证码GET /otn/passcodeNew/getPassCodeNew?module=passenger&rand=randp&0.7367515801483155 HTTP/1.1
	if (!client12306.DownloadVCodeImage("passenger"))
	{
		PostMessage(m_hwndReflection, WM_PRESUBMIT_ORDER, OPERATION_FAIL, 0);
		return false;
	}

	appInfo.SetPassengerInfo(p);
	PostMessage(m_hwndReflection, WM_PRESUBMIT_ORDER, OPERATION_SUCCESS, 0);

	return true;
}

bool CSubmitOrderTask::Run()
{
	Client12306& client12306 = Client12306::GetInstance();
	bool bVerifyVCode;
	if (!client12306.checkOrderInfo(m_orderInfo.oldPassengerStr.c_str(), m_orderInfo.passengerTicketStr.c_str(), m_orderInfo.tour_flag.c_str(), bVerifyVCode))
	{
		PostMessage(m_hwndReflection, WM_CHECK_ORDER, OPERATION_FAIL, 0);
		return false;
	}

	if (!client12306.getQueueCount(m_orderInfo.fromStationTelecode.c_str(),
		m_orderInfo.leftTicket.c_str(),
		m_orderInfo.purpose_codes.c_str(),
		m_orderInfo.seatType.c_str(),
		m_orderInfo.stationTrainCode.c_str(),
		m_orderInfo.toStationTelecode.c_str(),
		m_orderInfo.train_date.c_str(),
		m_orderInfo.train_location.c_str(),
		m_orderInfo.train_no.c_str()))
	{
		PostMessage(m_hwndReflection, WM_CHECK_ORDER, OPERATION_FAIL, 0);
		return false;
	}
	if (bVerifyVCode)
	{
		//拉取买票验证码
		//https://kyfw.12306.cn/otn/passcodeNew/checkRandCodeAnsyn?randCode=131,117,371,40&rand=randp&_json_att=&REPEAT_SUBMIT_TOKEN=SubmitToken
		return false;
	}

	int count = 10;
	bool bSuccess = false;
	while (count > 0)
	{
		if (client12306.confirmSingleForQueue(m_orderInfo.leftTicket.c_str(),
			m_orderInfo.oldPassengerStr.c_str(),
			m_orderInfo.passengerTicketStr.c_str(),
			"00",
			m_orderInfo.train_location.c_str()))
		{
			bSuccess = true;
			break;
		}
		::Sleep(10);
		count--;
	}
	if (!bSuccess)
	{
		PostMessage(m_hwndReflection, WM_CHECK_ORDER, OPERATION_FAIL, 0);
		return false;
	}

	string orderId;
	count = 3;
	while (count > 0)
	{
		if (!client12306.queryOrderWaitTime("dc", orderId))
		{
			PostMessage(m_hwndReflection, WM_CHECK_ORDER, OPERATION_FAIL, 0);
			break;
		}

		if (!orderId.empty())
			break;
		count--;
	}
	PostMessage(m_hwndReflection, WM_CHECK_ORDER, OPERATION_SUCCESS, 0);

	return true;
}