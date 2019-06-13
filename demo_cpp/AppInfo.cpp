/**
*@desc:    程序信息类，AppInfo.cpp
*/

#include"pch.h"
#include"stdafx.h"
#include"AppInfo.h"

const char* g_Weekdays[] = { "Sun", "Mon", "Tues", "Wed", "Thur", "Fri", "Sat" };
const char* g_Months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sept", "Oct", "Nov", "Dec" };

AppInfo& AppInfo::GetInstance()
{
	static AppInfo appinfo;
	return appinfo;
}

void AppInfo::SetLoginVCodeFilename(const char* filename)
{
	::EnterCriticalSection(&m_csData);
	m_strLoginVCodeFilename = filename;
	::LeaveCriticalSection(&m_csData);
}

void AppInfo::GetLoginVCodeFilename(string& strFilename)
{
	::EnterCriticalSection(&m_csData);
	strFilename = m_strLoginVCodeFilename;
	::LeaveCriticalSection(&m_csData);
}

void AppInfo::SetLoginInfo(const char* user, const char* password, const char* vcode)
{
	::EnterCriticalSection(&m_csData);
	m_LoginInfo.m_user = user;
	m_LoginInfo.m_password = password;
	m_LoginInfo.m_vcode = vcode;
	::LeaveCriticalSection(&m_csData);
}

void AppInfo::GetLoginInfo(LoginInfo& loginInfo)
{
	::EnterCriticalSection(&m_csData);
	loginInfo.m_user = m_LoginInfo.m_user;
	loginInfo.m_password = m_LoginInfo.m_password;
	loginInfo.m_vcode = m_LoginInfo.m_vcode;
	::LeaveCriticalSection(&m_csData);
}

void AppInfo::SetQueryTicketInfoParams(const char* train_date, const char* from_station, const char* to_station, const char* purpose_codes)
{
	::EnterCriticalSection(&m_csData);
	m_QueryTicketInfo.train_date = train_date;
	m_QueryTicketInfo.from_station = from_station;
	m_QueryTicketInfo.to_station = to_station;
	m_QueryTicketInfo.purpose_codes = purpose_codes;
	::LeaveCriticalSection(&m_csData);
}

void AppInfo::GetQueryTicketInfoParams(QueryTicketInfo& queryTicketInfo)
{
	::EnterCriticalSection(&m_csData);
	queryTicketInfo.train_date = m_QueryTicketInfo.train_date;
	queryTicketInfo.from_station = m_QueryTicketInfo.from_station;
	queryTicketInfo.to_station = m_QueryTicketInfo.to_station;
	queryTicketInfo.purpose_codes = m_QueryTicketInfo.purpose_codes;
	::LeaveCriticalSection(&m_csData);
}

void AppInfo::SetQueryTicketInfoResult(const vector<ticketinfo>& ti)
{
	m_arrTicketInfo = ti;
}

void AppInfo::GetQueryTicketInfoResult(vector<ticketinfo>& ti)
{
	ti = m_arrTicketInfo;
}

void AppInfo::SetBuyTicketInfo(const char* secretStr, const char* train_date, const char* back_train_date, const char* tour_flag, const char* purpose_codes, const char* query_from_station_name, const char* query_to_station_name)
{
	::EnterCriticalSection(&m_csData);
	m_BuyTicketInfo.secretStr = secretStr;
	m_BuyTicketInfo.train_date = train_date;
	m_BuyTicketInfo.back_train_date = back_train_date;
	m_BuyTicketInfo.tour_flag = tour_flag;
	m_BuyTicketInfo.purpose_codes = purpose_codes;
	m_BuyTicketInfo.query_from_station_name = query_from_station_name;
	m_BuyTicketInfo.query_to_station_name = query_to_station_name;
	::LeaveCriticalSection(&m_csData);
}

void AppInfo::GetBuyTicketInfo(BuyTicketInfo& bti)
{
	bti = m_BuyTicketInfo;
}

void AppInfo::SetStationInfo(const vector<stationinfo>& si)
{
	m_arrStationInfo = si;
}

void AppInfo::SetBuyTicketInfoIndex(size_t index)
{
	m_SelectedIndex = index;
}

void AppInfo::GetTicketInfoByIndex(ticketinfo& ti)
{
	if (m_SelectedIndex < 0 || m_SelectedIndex >= m_arrTicketInfo.size())
		return;

	ti = m_arrTicketInfo[m_SelectedIndex];
}

void AppInfo::GetStationInfo(vector<stationinfo>& si)
{
	si = m_arrStationInfo;
}

string AppInfo::GetStationNameByCode(const string& strCode)
{
	size_t size = m_arrStationInfo.size();
	for (size_t i = 0; i < size; ++i)
	{
		if (strCode == m_arrStationInfo[i].code2)
			return m_arrStationInfo[i].hanzi;
	}

	return "";
}

void AppInfo::SetPassengerInfo(const vector<passenager>& p)
{
	m_passengerInfo = p;
}

void AppInfo::GetPassengerInfo(vector<passenager>& p)
{
	p = m_passengerInfo;
}
void AppInfo::SetDate(int year, int month, int day, int weekday)
{
	::EnterCriticalSection(&m_csData);
	m_Year = year;
	m_Month = month;
	m_Day = day;
	m_weekday = weekday;
	::LeaveCriticalSection(&m_csData);
}

void AppInfo::GetDate(int& year, int& month, int& day, int& weekday)
{
	::EnterCriticalSection(&m_csData);
	year = m_Year;
	month = m_Month;
	day = m_Day;
	weekday = m_weekday;
	::LeaveCriticalSection(&m_csData);
}

void AppInfo::SetMode(bool bMode)
{
	::EnterCriticalSection(&m_csData);
	m_bVistorMode = bMode;
	::LeaveCriticalSection(&m_csData);
}

bool AppInfo::GetMode()
{
	return m_bVistorMode;
}

void AppInfo::SetOrderInfo(const OrderInfo& orderInfo)
{
	::EnterCriticalSection(&m_csData);
	m_OrderInfo = orderInfo;
	::LeaveCriticalSection(&m_csData);
}

void AppInfo::GetOrderInfo(OrderInfo& orderInfo)
{
	::EnterCriticalSection(&m_csData);
	orderInfo = m_OrderInfo;
	::LeaveCriticalSection(&m_csData);
}

AppInfo::AppInfo() : m_SelectedIndex(-1), m_Year(0), m_Month(0), m_Day(0), m_weekday(0), m_bVistorMode(true)
{
	::InitializeCriticalSection(&m_csData);
}

AppInfo::~AppInfo()
{
	::DeleteCriticalSection(&m_csData);
}