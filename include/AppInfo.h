//程序信息类，AppInfo.h

#pragma once
#ifndef _APPINFO_H_
#define _APPINFO_H_

#include<string>
#include<vector>
#include"Client12306.h"

using namespace std;

struct LoginInfo
{
	string m_user;
	string m_password;
	string m_vcode;
};

struct QueryTicketInfo
{
	string train_date;
	string from_station;
	string to_station;
	string purpose_codes;
};

struct BuyTicketInfo
{
	string secretStr;
	string train_date;
	string back_train_date;
	string tour_flag;
	string purpose_codes;
	string query_from_station_name;
	string query_to_station_name;
};

struct OrderInfo
{
	string oldPassengerStr;
	string passengerTicketStr;
	string tour_flag;
	string fromStationTelecode;
	string leftTicket;
	string purpose_codes;
	string seatType;
	string stationTrainCode;
	string toStationTelecode;
	string train_date;
	string train_location;
	string train_no;
};

extern const char* g_Weekdays[];
extern const char* g_Months[];
//‘硬卧’ = > ‘3’,
//‘软卧’ = > ‘4’,
//‘二等座’ = > ‘O’,
//‘一等座’ = > ‘M’,
//‘硬座’ = > ‘1’,

class AppInfo
{
public:
	static AppInfo& GetInstance();

	void SetLoginVCodeFilename(const char* filename);
	void GetLoginVCodeFilename(string& strFilename);

	void SetLoginInfo(const char* user, const char* password, const char* vcode);
	void GetLoginInfo(LoginInfo& loginInfo);

	void SetQueryTicketInfoParams(const char* train_data, const char* from_statsion, const char* to_station, const char* purpose_codes);
	void GetQueryTicketInfoParams(QueryTicketInfo& queryTicketInfo);
	
	void SetQueryTicketInfoResult(const vector<ticketinfo>& ti);
	void GetQueryTicketInfoResult(vector<ticketinfo>& ti);

	void SetBuyTicketInfo(const char* secretStr, const char* train_data, const char* back_train_date, const char* tour_flag, const char*purpose_codes, const char* query_from_station_name, const char* query_to_station_name);
	void GetBuyTicketInfo(BuyTicketInfo& bti);
	void SetBuyTicketInfoIndex(size_t index);
	void GetTicketInfoByIndex(ticketinfo& ti);

	void SetStationInfo(const vector<stationinfo>& si);
	void GetStationInfo(vector<stationinfo>& si);
	
	string GetStationNameByCode(const string& strCode);
	string GetStationCode(int stationNo);

	void SetPassengerInfo(const vector<passenager>& p);
	void GetPassengerInfo(vector<passenager>& p);

	void SetDate(int year, int month, int day, int weekday);
	void GetDate(int& year, int& month, int& day, int& weekday);

	void SetMode(bool bMode);
	bool GetMode();

	void SetOrderInfo(const OrderInfo& orderInfo);
	void GetOrderInfo(OrderInfo& orderInfo);

private:
	AppInfo();
	~AppInfo();
	AppInfo(const AppInfo&);
	AppInfo& operator=(const AppInfo);
private:
	LoginInfo           m_LoginInfo;
	QueryTicketInfo     m_QueryTicketInfo;
	BuyTicketInfo       m_BuyTicketInfo;
	size_t              m_SelectedIndex;
	string              m_strLoginVCodeFilename;
	OrderInfo           m_OrderInfo;

	vector<ticketinfo>  m_arrTicketInfo;
	vector<stationinfo> m_arrStationInfo;
	vector<passenager>  m_passengerInfo;

	int                 m_Year;
	int                 m_Month;
	int                 m_Day;
	int                 m_weekday;

	bool                m_bVistorMode;

	CRITICAL_SECTION    m_csData;
};

#endif // !_APPINFO_H_

