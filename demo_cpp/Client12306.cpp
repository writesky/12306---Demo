/**
*@desc:	封装获取验证码、校验验证码、登录等12306各个请求的类，Client12306.cpp文件
*/

#include "stdafx.h"
#include <string>
#include <sstream>
#include "Log.h"
#include "jsoncpp-0.5.0/json.h"
#include "Utils.h"
#include <curl/curl.h>
#include "Client12306.h"

//#ifdef _DEBUG
//#pragma comment(lib,"libcurl_d.lib")
//#else
#pragma comment(lib,"libcurl.lib")

#define URL_REFERER         "https://kyfw.12306.cn/otn/index/init"
#define URL_LOGIN_INIT      "https://kyfw.12306.cn/otn/login/init"
#define URL_GETPASSCODENEW  "https://kyfw.12306.cn/passport/captcha/captcha-image"
#define URL_CHECKRANDCODEANSYN  "https://kyfw.12306.cn/otn/passcodeNew/checkRandCodeAnsyn"
#define URL_LOGINAYSNSUGGEST "https://kyfw.12306.cn:443/otn/login/loginAysnSuggest"
#define URL_USERLOGIN       "https://kyfw.12306.cn:443/otn/login/userLogin"
#define URL_INITMY12306     "https://kyfw.12306.cn:443/otn/index/initMy12306"
#define URL_QUERY_TICKET1   "https://kyfw.12306.cn/otn/leftTicket/log"
//游客模式下的查票接口是https://kyfw.12306.cn/otn/leftTicket/query?leftTicketDTO.train_date=2017-05-24&leftTicketDTO.from_station=BJP&leftTicketDTO.to_station=SHH&purpose_codes=ADULT
#define URL_QUERY_TICKET2   "https://kyfw.12306.cn/otn/leftTicket/query"
#define URL_CHECK_USER      "https://kyfw.12306.cn/otn/login/checkUser"
#define URL_SUBMIT_ORDER_REQUEST "https://kyfw.12306.cn/otn/leftTicket/submitOrderRequest"
#define URL_INIT_DC         "https://kyfw.12306.cn/otn/confirmPassenger/initDc"
#define URL_GET_PASSENGER_DTO   "https://kyfw.12306.cn/otn/confirmPassenger/getPassengerDTOs"
#define URL_CHECK_ORDER_INFO "https://kyfw.12306.cn/otn/confirmPassenger/checkOrderInfo"
#define URL_GET_QUEUE_COUNT "https://kyfw.12306.cn/otn/confirmPassenger/getQueueCount"
#define URL_CONFIRM_SINGLE_FOR_QUEUE "https://kyfw.12306.cn/otn/confirmPassenger/confirmSingleForQueue"
#define URL_QUERY_ORDER_WAIT_TIME "https://kyfw.12306.cn/otn/confirmPassenger/queryOrderWaitTime"
#define URL_STATION_NAMES   "https://kyfw.12306.cn/otn/resources/js/framework/station_name.js?station_version=1.9053"

#define NOT_EMPTY_EQUAL(src, dest) \
    if (!src.empty()) \
    dest = src; \
    else \
    dest = "--";


static size_t OnWriteData(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
	string* str = dynamic_cast<string*>((string *)lpVoid);
	if (NULL == str || NULL == buffer)
	{
		return -1;
	}

	char* pData = (char*)buffer;
	str->append(pData, size * nmemb);
	return nmemb;
}

Client12306& Client12306::GetInstance()
{
	static Client12306 c;
	return c;
}

Client12306::Client12306()
{
	memset(m_szCurrVCodeName, 0, sizeof(m_szCurrVCodeName));
}

Client12306::~Client12306()
{

}

bool Client12306::ReloadVCodeImage()
{
	if (!loginInit())
	{
		LogError("Login init error");
		return false;
	}

	if (!DownloadVCodeImage("login"))
	{
		LogError("download vcode image error");
		return false;
	}

	return true;
}

bool Client12306::GetVCodeFileName(char* pszDst, int nLength)
{
	if (pszDst == NULL || nLength <= 0)
		return false;

	strcpy_s(pszDst, nLength, m_szCurrVCodeName);
	return true;
}

bool Client12306::GuestQueryTicket(const char* train_date, const char* from_station, const char* to_station, const char* purpose_codes, vector<ticketinfo>& v)
{
	//https://kyfw.12306.cn/otn/leftTicket/query?leftTicketDTO.train_date=2017-05-24&leftTicketDTO.from_station=BJP&leftTicketDTO.to_station=SHH&purpose_codes=ADULT   
	ostringstream osURL;
	osURL << URL_QUERY_TICKET2;
	osURL << "?leftTicketDTO.train_date=";
	osURL << train_date;
	osURL << "&leftTicketDTO.from_station=";
	osURL << from_station;
	osURL << "&leftTicketDTO.to_station=";
	osURL << to_station;
	osURL << "&purpose_codes=";
	osURL << purpose_codes;

	string strResponse;
	string strCookie = "Cookie: ";
	strCookie += m_strCookies;
	if (!HttpRequest(osURL.str().c_str(), strResponse, true, strCookie.c_str(), NULL, false, 10))
	{
		LogError("QueryTickets2 failed");
		return false;
	}
	/*
	"2fXnt0CKAozOCoh9vZkJwPbCoN%2FCiDLQVLZMOuSfR7fnc%2BmpWVAkdJuzcP%2F9eAgI6MPuTOC34SBX%0AisE%2FLaAgRJY3qgBVs1GueU3Ma3KyK2BfN4lvNmRoTBXNNrCz1brJc61jIwFhTziu81RDXi6SxbtF%0AVAW9VbuL%2F1tNzhgCrWkinp8vVgmEj%2F%2BROJ0IH7wpOU3qALFtwrEhTJwJqE90MwxyZuxoSwp4aoTo%0AImEy5aVOvQoG|预订|240000G1010C|G101|VNP|AOH|VNP|AOH|06:44|12:38|05:54|Y|d0o8dwBokIpqvhiqvr%2F1gndff5a15h6bWmYtCTA2neNitEC1|20170524|3|P2|01|11|1|0|||||||||||有|有|18|O0M090|OM9"
	*/
	//|||||||||||一等座|二等座|商务座|
	//|| || || 有 || || | 有 | 18|    |
	/*
	{"validateMessagesShowId":"_validatorMessage","status":true,"httpstatus":200,
	"data":[{"queryLeftNewDTO":{"train_no":"5l0000G10261","station_train_code":"G102",
	"start_station_telecode":"AOH","start_station_name":"上海虹桥","end_station_telecode":"VNP",
	"end_station_name":"北京南","from_station_telecode":"AOH","from_station_name":"上海虹桥",
	"to_station_telecode":"VNP","to_station_name":"北京南","start_time":"24:00",
	"arrive_time":"24:00","day_difference":"0","train_class_name":"","lishi":"99:59",
	"canWebBuy":"IS_TIME_NOT_BUY","lishiValue":"5999","yp_info":"",
	"start_train_date":"20170128","yp_ex":"","train_type_code":"2","start_province_code":"33",
	"start_city_code":"0712","end_province_code":"31","end_city_code":"0357","seat_types":"",
	"location_code":"HZ","from_station_no":"01","to_station_no":"09",
	"control_day":0,"is_support_card":"0","controlled_train_flag":"1",
	"controlled_train_message":"列车运行图调整,暂停发售",
	"gg_num":"--","gr_num":"--","qt_num":"--","rw_num":"--","rz_num":"--",
	"tz_num":"--","wz_num":"--","yb_num":"--","yw_num":"--","yz_num":"--",
	"ze_num":"--","zy_num":"--","swz_num":"--"},"secretStr":"",
	"buttonTextInfo":"列车运行图调整,暂停发售"}}}]
	*/
	Json::Reader JsonReader;
	Json::Value JsonRoot;
	if (!JsonReader.parse(strResponse, JsonRoot))
		return false;

	if (JsonRoot["status"].isNull())
		return false;

	bool bStatus = JsonRoot["status"].asBool();
	if (!bStatus)
		return false;

	if (JsonRoot["httpstatus"].isNull() || JsonRoot["httpstatus"].asInt() != 200)
		return false;

	if (JsonRoot["data"].isNull() || !JsonRoot["data"].isObject())
		return false;

	if (JsonRoot["data"]["result"].isNull() || !JsonRoot["data"]["result"].isArray())
		return false;


	UINT size = JsonRoot["data"]["result"].size();
	vector<string>  ticketInfoStr;
	for (UINT i = 0; i < size; ++i)
	{
		ticketInfoStr.clear();
		split(JsonRoot["data"]["result"][i].asString(), "|", ticketInfoStr);
		//2fXnt0CKAozOCoh9vZkJwPbCoN%2FCiDLQVLZMOuSfR7fnc%2BmpWVAkdJuzcP%2F9eAgI6MPuTOC34SBX%0AisE%2FLaAgRJY3qgBVs1GueU3Ma3KyK2BfN4lvNmRoTBXNNrCz1brJc61jIwFhTziu81RDXi6SxbtF%0AVAW9VbuL%2F1tNzhgCrWkinp8vVgmEj%2F%2BROJ0IH7wpOU3qALFtwrEhTJwJqE90MwxyZuxoSwp4aoTo%0AImEy5aVOvQoG|预订|240000G1010C|G101|VNP|AOH|VNP|AOH|06:44|12:38|05:54|Y|d0o8dwBokIpqvhiqvr%2F1gndff5a15h6bWmYtCTA2neNitEC1|20170524|3|P2|01|11|1|0|||||||||||有|有|18|O0M090|OM9"
		ticketinfo ti;
		ti.secretStr = ticketInfoStr[0];
		ti.buttonTextInfo = ticketInfoStr[1];

		ti.DTO.start_station_telecode = ticketInfoStr[4];
		ti.DTO.end_station_telecode = ticketInfoStr[5];
		ti.DTO.from_station_telecode = ticketInfoStr[6];
		ti.DTO.to_station_telecode = ticketInfoStr[7];
		//ti.DTO.station_train_code = JsonRoot["data"][i]["queryLeftNewDTO"]["station_train_code"].asString();
		//ti.DTO.train_no = JsonRoot["data"][i]["queryLeftNewDTO"]["train_no"].asString();
		//ti.DTO.location_code = JsonRoot["data"][i]["queryLeftNewDTO"]["location_code"].asString();
		//ti.DTO.yp_info = JsonRoot["data"][i]["queryLeftNewDTO"]["yp_info"].asString();

		//车次
		ti.DTO.station_train_code = ticketInfoStr[3];;
		//出发站
		//ti.DTO.from_station_name = ticketInfoStr[6];
		//到达站
		//ti.DTO.to_station_name = ticketInfoStr[7];
		//出发时间
		ti.DTO.start_time = ticketInfoStr[8];
		//到达时间
		ti.DTO.arrive_time = ticketInfoStr[9];
		//历时
		ti.DTO.lishi = ticketInfoStr[10];
		////是否当日到达
		//ti.DTO.day_difference = JsonRoot["data"][i]["queryLeftNewDTO"]["day_difference"].asString();
		////"yz_num":"--","rz_num":"--","yw_num":"--","rw_num":"--",
		////"gr_num":"--","zy_num":"--","ze_num":"--","tz_num":"--","gg_num":"--","yb_num":"--","wz_num":"--","qt_num":"--","swz_num":"--"
		//商务座
		NOT_EMPTY_EQUAL(ticketInfoStr[32], ti.DTO.swz_num)
			//特等座
			NOT_EMPTY_EQUAL(ticketInfoStr[25], ti.DTO.tz_num)
			//一等座
			NOT_EMPTY_EQUAL(ticketInfoStr[31], ti.DTO.zy_num)
			//二等座
			NOT_EMPTY_EQUAL(ticketInfoStr[30], ti.DTO.ze_num)
			////高级软卧gr_num
			//ti.DTO.gr_num = JsonRoot["data"][i]["queryLeftNewDTO"]["gr_num"].asString();
			//软卧rw_num
			NOT_EMPTY_EQUAL(ticketInfoStr[23], ti.DTO.rw_num)
			//硬卧yw_num
			NOT_EMPTY_EQUAL(ticketInfoStr[28], ti.DTO.yw_num)
			////软座rz_num
			//ti.DTO.rz_num = JsonRoot["data"][i]["queryLeftNewDTO"]["rz_num"].asString();
			//硬座
			NOT_EMPTY_EQUAL(ticketInfoStr[26], ti.DTO.yz_num)
			//无座
			NOT_EMPTY_EQUAL(ticketInfoStr[29], ti.DTO.wz_num)
			v.push_back(ti);

		for (size_t j = 0; j < ticketInfoStr.size(); ++j)
		{
			if (j == 0 || j == 3 || j == 12)
				continue;

			if (ticketInfoStr[j].empty())
				::OutputDebugStringA("--");
			else
				::OutputDebugStringA(ticketInfoStr[j].c_str());

			::OutputDebugStringA("  ");
		}

		::OutputDebugStringA(ticketInfoStr[3].c_str());
		::OutputDebugStringA("\r\n");
	}



	return true;
}

bool Client12306::loginInit()
{
	string strResponse;
	if (!HttpRequest(URL_LOGIN_INIT, strResponse, true, "Upgrade-Insecure-Requests: 1", NULL, true, 10))
	{
		LogError("loginInit failed");
		return false;
	}

	if (!GetCookies(strResponse))
	{
		LogError("parse login init cookie error, url=%s", URL_LOGIN_INIT);
		return false;
	}

	return true;
}

bool Client12306::DownloadVCodeImage(const char* module)
{
	if (module == NULL)
	{
		LogError("module is invalid");
		return false;
	}

	//https://kyfw.12306.cn/passport/captcha/captcha-image?login_site=E&module=login&rand=sjrand&0.06851784300754482
	ostringstream osUrl;
	osUrl << URL_GETPASSCODENEW;
	osUrl << "?login_site=E&module=";
	osUrl << module;
	//购票验证码
	if (strcmp(module, "passenger") != 0)
	{
		osUrl << "&rand=sjrand&";
	}
	//登录验证码
	else
	{
		osUrl << "&rand=randp&";
	}
	double d = rand() * 1.000000 / RAND_MAX;
	osUrl.precision(17);
	osUrl << d;

	string strResponse;
	string strCookie = "Cookie: ";
	strCookie += m_strCookies;
	if (!HttpRequest(osUrl.str().c_str(), strResponse, true, strCookie.c_str(), NULL, false, 10))
	{
		LogError("DownloadVCodeImage failed");
		return false;
	}

	//写入文件
	time_t now = time(NULL);
	struct tm* tblock = localtime(&now);
	memset(m_szCurrVCodeName, 0, sizeof(m_szCurrVCodeName));
#ifdef _DEBUG
	sprintf(m_szCurrVCodeName, "vcode%04d%02d%02d%02d%02d%02d.jpg",
		1900 + tblock->tm_year, 1 + tblock->tm_mon, tblock->tm_mday,
		tblock->tm_hour, tblock->tm_min, tblock->tm_sec);
#else
	sprintf(m_szCurrVCodeName, "vcode%04d%02d%02d%02d%02d%02d.v",
		1900 + tblock->tm_year, 1 + tblock->tm_mon, tblock->tm_mday,
		tblock->tm_hour, tblock->tm_min, tblock->tm_sec);
#endif

	FILE* fp = fopen(m_szCurrVCodeName, "wb");
	if (fp == NULL)
	{
		LogError("open file %s error", m_szCurrVCodeName);
		return false;
	}

	const char* p = strResponse.data();
	size_t count = fwrite(p, strResponse.length(), 1, fp);
	if (count != 1)
	{
		LogError("write file %s error", m_szCurrVCodeName);
		fclose(fp);
		return false;
	}

	fclose(fp);

	return true;
}

int Client12306::checkRandCodeAnsyn(const char* vcode)
{
	string param;
	param = "randCode=";
	param += vcode;
	param += "&rand=sjrand";	//passenger:randp

	string strResponse;
	string strCookie = "Cookie: ";
	strCookie += m_strCookies;
	if (!HttpRequest(URL_CHECKRANDCODEANSYN, strResponse, false, strCookie.c_str(), param.c_str(), false, 10))
	{
		LogError("checkRandCodeAnsyn failed");
		return -1;
	}

	///** 成功返回
	//HTTP/1.1 200 OK
	//Date: Thu, 05 Jan 2017 07:44:16 GMT
	//Server: Apache-Coyote/1.1
	//X-Powered-By: Servlet 2.5; JBoss-5.0/JBossWeb-2.1
	//ct: c1_103
	//Content-Type: application/json;charset=UTF-8
	//Content-Length: 144
	//X-Via: 1.1 jiandianxin29:6 (Cdn Cache Server V2.0)
	//Connection: keep-alive
	//X-Cdn-Src-Port: 19153

	//参数无效
	//{"validateMessagesShowId":"_validatorMessage","status":true,"httpstatus":200,"data":{"result":"0","msg":""},"messages":[],"validateMessages":{}}
	//验证码过期
	//{"validateMessagesShowId":"_validatorMessage","status":true,"httpstatus":200,"data":{"result":"0","msg":"EXPIRED"},"messages":[],"validateMessages":{}}
	//验证码错误
	//{"validateMessagesShowId":"_validatorMessage","status":true,"httpstatus":200,"data":{"result":"1","msg":"FALSE"},"messages":[],"validateMessages":{}}
	//验证码正确
	//{"validateMessagesShowId":"_validatorMessage","status":true,"httpstatus":200,"data":{"result":"1","msg":"TRUE"},"messages":[],"validateMessages":{}}
	Json::Reader JsonReader;
	Json::Value JsonRoot;
	if (!JsonReader.parse(strResponse, JsonRoot))
		return -1;
	//{"validateMessagesShowId":"_validatorMessage", "status" : true, "httpstatus" : 200, "data" : {"result":"1", "msg" : "TRUE"}, "messages" : [], "validateMessages" : {}}
	if (JsonRoot["status"].isNull() || JsonRoot["status"].asBool() != true)
		return -1;

	if (JsonRoot["httpstatus"].isNull() || JsonRoot["httpstatus"].asInt() != 200)
		return -1;

	if (JsonRoot["data"].isNull() || !JsonRoot["data"].isObject())
		return -1;

	if (JsonRoot["data"]["result"].isNull())
		return -1;

	if (JsonRoot["data"]["result"].asString() != "1" && JsonRoot["data"]["result"].asString() != "0")
		return -1;

	if (JsonRoot["data"]["msg"].isNull())
		return -1;
	//if (JsonRoot["data"]["msg"].asString().empty())		
	//	return -1;

	if (JsonRoot["data"]["msg"].asString() == "")
		return 0;
	else if (JsonRoot["data"]["msg"].asString() == "FALSE")
		return 1;


	return 1;
}

int Client12306::loginAysnSuggest(const char* user, const char* pass, const char* vcode)
{
	string param = "loginUserDTO.user_name=";
	param += user;
	param += "&userDTO.password=";
	param += pass;
	param += "&randCode=";
	param += vcode;
	string strResponse;
	string strCookie = "Cookie: ";
	strCookie += m_strCookies;
	if (!HttpRequest(URL_LOGINAYSNSUGGEST, strResponse, false, strCookie.c_str(), param.c_str(), false, 10))
	{
		LogError("loginAysnSuggest failed");
		return 2;
	}

	///** 成功返回
	//HTTP/1.1 200 OK
	//Date: Thu, 05 Jan 2017 07:49:53 GMT
	//Server: Apache-Coyote/1.1
	//X-Powered-By: Servlet 2.5; JBoss-5.0/JBossWeb-2.1
	//ct: c1_103
	//Content-Type: application/json;charset=UTF-8
	//Content-Length: 146
	//X-Via: 1.1 f186:10 (Cdn Cache Server V2.0)
	//Connection: keep-alive
	//X-Cdn-Src-Port: 48361

	//邮箱不存在
	//{"validateMessagesShowId":"_validatorMessage","status":true,"httpstatus":200,"data":{},"messages":["该邮箱不存在。"],"validateMessages":{}}
	//密码错误
	//{"validateMessagesShowId":"_validatorMessage","status":true,"httpstatus":200,"data":{},"messages":["密码输入错误。如果输错次数超过4次，用户将被锁定。"],"validateMessages":{}}
	//登录成功
	//{"validateMessagesShowId":"_validatorMessage","status":true,"httpstatus":200,"data":{"otherMsg":"",loginCheck:"Y"},"messages":[],"validateMessages":{}}
	//WCHAR* psz1 = Utf8ToAnsi(strResponse.c_str());
	//wstring str = psz1;
	//delete[] psz1;

	Json::Reader JsonReader;
	Json::Value JsonRoot;
	if (!JsonReader.parse(strResponse, JsonRoot))
		return 2;

	//{"validateMessagesShowId":"_validatorMessage", "status" : true, 
	//"httpstatus" : 200, "data" : {"otherMsg":"", loginCheck : "Y"}, "messages" : [], "validateMessages" : {}}
	if (JsonRoot["status"].isNull())
		return -1;

	bool bStatus = JsonRoot["status"].asBool();
	if (!bStatus)
		return -1;

	if (JsonRoot["httpstatus"].isNull() || JsonRoot["httpstatus"].asInt() != 200)
		return 2;

	if (JsonRoot["data"].isNull() || !JsonRoot["data"].isObject())
		return 2;

	if (JsonRoot["data"]["otherMsg"].isNull() || JsonRoot["data"]["otherMsg"].asString() != "")
		return 2;
	if (JsonRoot["data"]["loginCheck"].isNull() || JsonRoot["data"]["loginCheck"].asString() != "Y")
		return 1;

	return 0;
}

bool Client12306::userLogin()
{
	string strResponse;
	string strCookie = "Cookie: ";
	strCookie += m_strCookies;
	string param = "_json_att=";
	if (!HttpRequest(URL_USERLOGIN, strResponse, false, strCookie.c_str(), param.c_str(), false, 10))
	{
		LogError("userLogin failed");
		return false;
	}

	return true;
}

bool Client12306::initMy12306()
{
	string strResponse;
	string strCookie = "Cookie: ";
	strCookie += m_strCookies;
	if (!HttpRequest(URL_INITMY12306, strResponse, false, strCookie.c_str(), NULL, false, 10))
	{
		LogError("userLogin failed");
		return false;
	}

	return true;
}

bool Client12306::QueryTickets1(const char* train_date, const char* from_station, const char* to_station, const char* purpose_codes)
{
	//https://kyfw.12306.cn/otn/leftTicket/log?leftTicketDTO.train_date=2017-02-08&leftTicketDTO.from_station=SHH&leftTicketDTO.to_station=NJH&purpose_codes=ADULT
	//https://kyfw.12306.cn/otn/leftTicket/query?leftTicketDTO.train_date=2018-05-19&leftTicketDTO.from_station=SHH&leftTicketDTO.to_station=BJP&purpose_codes=ADULT
	ostringstream osURL;
	osURL << URL_QUERY_TICKET1;
	osURL << "?leftTicketDTO.train_date=";
	osURL << train_date;
	osURL << "&leftTicketDTO.from_station=";
	osURL << from_station;
	osURL << "&leftTicketDTO.to_station=";
	osURL << to_station;
	osURL << "&purpose_codes=";
	osURL << purpose_codes;

	string strResponse;
	string strCookie = "Cookie: ";
	strCookie += m_strCookies;
	if (!HttpRequest(osURL.str().c_str(), strResponse, true, strCookie.c_str(), NULL, false, 10))
	{
		LogError("QueryTickets1 failed");
		return false;
	}

	/*
	{“validateMessagesShowId”:”_validatorMessage”,”status”:true,”httpstatus”:200,”messages”:[],”validateMessages”:{}}
	*/
	Json::Reader JsonReader;
	Json::Value JsonRoot;
	if (!JsonReader.parse(strResponse, JsonRoot))
		return false;

	if (JsonRoot["status"].isNull())
		return false;

	bool bStatus = JsonRoot["status"].asBool();
	if (!bStatus)
	{
		LogError("[status] node is not true");
		return false;
	}

	if (JsonRoot["httpstatus"].isNull() || JsonRoot["httpstatus"].asInt() != 200)
	{
		LogError("[httpstatus] node is not 200");
		return false;
	}

	return true;
}

bool Client12306::QueryTickets2(const char* train_date, const char* from_station, const char* to_station, const char* purpose_codes, vector<ticketinfo>& v)
{
	//https://kyfw.12306.cn/otn/leftTicket/queryZ?leftTicketDTO.train_date=2017-01-28&leftTicketDTO.from_station=SHH&leftTicketDTO.to_station=BJP&purpose_codes=ADULT
	ostringstream osURL;
	osURL << URL_QUERY_TICKET2;
	osURL << "?leftTicketDTO.train_date=";
	osURL << train_date;
	osURL << "&leftTicketDTO.from_station=";
	osURL << from_station;
	osURL << "&leftTicketDTO.to_station=";
	osURL << to_station;
	osURL << "&purpose_codes=";
	osURL << purpose_codes;

	string strResponse;
	string strCookie = "Cookie: ";
	strCookie += m_strCookies;
	if (!HttpRequest(osURL.str().c_str(), strResponse, true, strCookie.c_str(), NULL, false, 10))
	{
		LogError("QueryTickets2 failed");
		return false;
	}

	/*
	{"validateMessagesShowId":"_validatorMessage","status":true,"httpstatus":200,
	"data":[{"queryLeftNewDTO":{"train_no":"5l0000G10261","station_train_code":"G102",
	"start_station_telecode":"AOH","start_station_name":"上海虹桥","end_station_telecode":"VNP",
	"end_station_name":"北京南","from_station_telecode":"AOH","from_station_name":"上海虹桥",
	"to_station_telecode":"VNP","to_station_name":"北京南","start_time":"24:00",
	"arrive_time":"24:00","day_difference":"0","train_class_name":"","lishi":"99:59",
	"canWebBuy":"IS_TIME_NOT_BUY","lishiValue":"5999","yp_info":"",
	"start_train_date":"20170128","yp_ex":"","train_type_code":"2","start_province_code":"33",
	"start_city_code":"0712","end_province_code":"31","end_city_code":"0357","seat_types":"",
	"location_code":"HZ","from_station_no":"01","to_station_no":"09",
	"control_day":0,"is_support_card":"0","controlled_train_flag":"1",
	"controlled_train_message":"列车运行图调整,暂停发售",
	"gg_num":"--","gr_num":"--","qt_num":"--","rw_num":"--","rz_num":"--",
	"tz_num":"--","wz_num":"--","yb_num":"--","yw_num":"--","yz_num":"--",
	"ze_num":"--","zy_num":"--","swz_num":"--"},"secretStr":"",
	"buttonTextInfo":"列车运行图调整,暂停发售"}}}]
	*/
	Json::Reader JsonReader;
	Json::Value JsonRoot;
	if (!JsonReader.parse(strResponse, JsonRoot))
		return false;

	if (JsonRoot["status"].isNull())
		return false;

	bool bStatus = JsonRoot["status"].asBool();
	if (!bStatus)
		return false;

	if (JsonRoot["httpstatus"].isNull() || JsonRoot["httpstatus"].asInt() != 200)
		return false;

	if (JsonRoot["data"].isNull() || !JsonRoot["data"].isObject())
		return false;

	if (JsonRoot["data"]["result"].isNull() || !JsonRoot["data"]["result"].isArray())
		return false;


	UINT size = JsonRoot["data"]["result"].size();
	vector<string>  ticketInfoStr;
	for (UINT i = 0; i < size; ++i)
	{
		ticketInfoStr.clear();
		split(JsonRoot["data"]["result"][i].asString(), "|", ticketInfoStr);
		//2fXnt0CKAozOCoh9vZkJwPbCoN%2FCiDLQVLZMOuSfR7fnc%2BmpWVAkdJuzcP%2F9eAgI6MPuTOC34SBX%0AisE%2FLaAgRJY3qgBVs1GueU3Ma3KyK2BfN4lvNmRoTBXNNrCz1brJc61jIwFhTziu81RDXi6SxbtF%0AVAW9VbuL%2F1tNzhgCrWkinp8vVgmEj%2F%2BROJ0IH7wpOU3qALFtwrEhTJwJqE90MwxyZuxoSwp4aoTo%0AImEy5aVOvQoG|预订|240000G1010C|G101|VNP|AOH|VNP|AOH|06:44|12:38|05:54|Y|d0o8dwBokIpqvhiqvr%2F1gndff5a15h6bWmYtCTA2neNitEC1|20170524|3|P2|01|11|1|0|||||||||||有|有|18|O0M090|OM9"
		ticketinfo ti;
		ti.secretStr = ticketInfoStr[0];
		ti.buttonTextInfo = ticketInfoStr[1];

		ti.DTO.start_station_telecode = ticketInfoStr[4];
		ti.DTO.end_station_telecode = ticketInfoStr[5];
		ti.DTO.from_station_telecode = ticketInfoStr[6];
		ti.DTO.to_station_telecode = ticketInfoStr[7];

		ti.DTO.train_no = ticketInfoStr[2];
		//ti.DTO.location_code = JsonRoot["data"][i]["queryLeftNewDTO"]["location_code"].asString();
		//ti.DTO.yp_info = JsonRoot["data"][i]["queryLeftNewDTO"]["yp_info"].asString();

		//车次
		ti.DTO.station_train_code = ticketInfoStr[3];;
		//出发站
		ti.DTO.from_station_name = ticketInfoStr[6];
		//到达站
		ti.DTO.to_station_name = ticketInfoStr[7];
		//出发时间
		ti.DTO.start_time = ticketInfoStr[8];
		//到达时间
		ti.DTO.arrive_time = ticketInfoStr[9];
		//历时
		ti.DTO.lishi = ticketInfoStr[10];
		////是否当日到达
		//ti.DTO.day_difference = JsonRoot["data"][i]["queryLeftNewDTO"]["day_difference"].asString();
		////"yz_num":"--","rz_num":"--","yw_num":"--","rw_num":"--",
		////"gr_num":"--","zy_num":"--","ze_num":"--","tz_num":"--","gg_num":"--","yb_num":"--","wz_num":"--","qt_num":"--","swz_num":"--"
		//商务座
		NOT_EMPTY_EQUAL(ticketInfoStr[32], ti.DTO.swz_num)
			//特等座
			NOT_EMPTY_EQUAL(ticketInfoStr[25], ti.DTO.tz_num)
			//一等座
			NOT_EMPTY_EQUAL(ticketInfoStr[31], ti.DTO.zy_num)
			//二等座
			NOT_EMPTY_EQUAL(ticketInfoStr[30], ti.DTO.ze_num)
			////高级软卧gr_num
			//ti.DTO.gr_num = JsonRoot["data"][i]["queryLeftNewDTO"]["gr_num"].asString();
			//软卧rw_num
			NOT_EMPTY_EQUAL(ticketInfoStr[23], ti.DTO.rw_num)
			//硬卧yw_num
			NOT_EMPTY_EQUAL(ticketInfoStr[28], ti.DTO.yw_num)
			////软座rz_num
			//ti.DTO.rz_num = JsonRoot["data"][i]["queryLeftNewDTO"]["rz_num"].asString();
			//硬座
			NOT_EMPTY_EQUAL(ticketInfoStr[26], ti.DTO.yz_num)
			//无座
			NOT_EMPTY_EQUAL(ticketInfoStr[29], ti.DTO.wz_num)
			v.push_back(ti);

		for (size_t j = 0; j < ticketInfoStr.size(); ++j)
		{
			if (j == 0 || j == 3 || j == 12)
				continue;

			if (ticketInfoStr[j].empty())
				::OutputDebugStringA("--");
			else
				::OutputDebugStringA(ticketInfoStr[j].c_str());

			::OutputDebugStringA("  ");
		}

		::OutputDebugStringA(ticketInfoStr[3].c_str());
		::OutputDebugStringA("\r\n");
	}

	return true;
}

bool Client12306::checkUser()
{
	//https://kyfw.12306.cn/otn/login/checkUser
	string strResponse;
	string strCookie = "Cookie: ";
	strCookie += m_strCookies;
	string param = "_json_att=";
	if (!HttpRequest(URL_CHECK_USER, strResponse, false, strCookie.c_str(), param.c_str(), false, 10))
	{
		LogError("checkUser failed");
		return false;
	}

	/* 应答格式：
	{“validateMessagesShowId”:”_validatorMessage”, ”status” : true, ”httpstatus” : 200, ”data” : {“flag”:true}, ”messages” : [], ”validateMessages” : {}}
	*/
	Json::Reader JsonReader;
	Json::Value JsonRoot;
	if (!JsonReader.parse(strResponse, JsonRoot))
		return false;

	if (JsonRoot["status"].isNull())
		return false;

	bool bStatus = JsonRoot["status"].asBool();
	if (!bStatus)
		return false;

	if (JsonRoot["httpstatus"].isNull() || JsonRoot["httpstatus"].asInt() != 200)
		return false;

	//如果登录成功以后，不再次拉取一下验证码这个flag的值会为false
	if (JsonRoot["data"].isNull() || JsonRoot["data"]["flag"].isNull())
		return false;

	if (JsonRoot["data"]["flag"].asBool() != true)
		return false;

	return true;
}

bool Client12306::submitOrderRequest(const char* secretStr, const char* train_date, const char* back_train_date, const char* tour_flag, const char* purpose_codes, const char* query_from_station_name, const char* query_to_station_name)
{
	//secretStr=secretStr&train_date=2017-01-21&back_train_date=2016-12-23&tour_flag=dc&purpose_codes=ADULT&query_from_station_name=深圳&query_to_station_name=武汉&undefined=
	string param = "secretStr=";
	param += secretStr;
	param += "&train_date=";
	param += train_date;
	param += "&back_train_date=";
	param += back_train_date;
	param += "&tour_flag=";
	param += tour_flag;
	param += "&purpose_codes=";
	param += purpose_codes;
	param += "&query_from_station_name=";
	param += query_from_station_name;
	param += "&query_to_station_name=";
	param += query_to_station_name;
	param += "&undefined=";

	string strResponse;
	string strCookie = "Cookie: ";
	strCookie += m_strCookies;
	if (!HttpRequest(URL_SUBMIT_ORDER_REQUEST, strResponse, false, strCookie.c_str(), param.c_str(), false, 10))
	{
		LogError("submitOrderRequest failed");
		return false;
	}

	///** 成功返回
	//{“validateMessagesShowId”:”_validatorMessage”,”status”:true,”httpstatus”:200,”data”:”N”,”messages”:[],”validateMessages”:{}}
	/*
	{"validateMessagesShowId":"_validatorMessage","status":false,"httpstatus":200,"messages":["您还有未处理的订单，请您到<a href=\"../queryOrder/initNoComplete\">[未完成订单]</a>进行处理!"],"validateMessages":{}}
	*/
	Json::Reader JsonReader;
	Json::Value JsonRoot;
	if (!JsonReader.parse(strResponse, JsonRoot))
		return false;

	if (JsonRoot["status"].isNull())
		return false;

	bool bStatus = JsonRoot["status"].asBool();
	if (!bStatus)
		return false;

	if (JsonRoot["httpstatus"].isNull() || JsonRoot["httpstatus"].asInt() != 200)
		return false;

	return true;
}

bool Client12306::initDc()
{
	string strResponse;
	string strCookie = "Cookie: ";
	strCookie += m_strCookies;
	string param = "_json_att=";
	if (!HttpRequest(URL_INIT_DC, strResponse, false, strCookie.c_str(), param.c_str(), false, 10))
	{
		LogError("initDc failed");
		return false;
	}

	vector<string> result;
	//string globalRepeatSubmitToken;
	string pattern = "var globalRepeatSubmitToken = '";
	int pos = strResponse.find(pattern);
	if (pos == string::npos)
	{
		LogError("empty RepeatSubmitToken");
		return false;
	}

	//32位的md5
	m_strGlobalRepeatSubmitToken = strResponse.substr(pos + pattern.length(), 32);

	pattern = "'key_check_isChange':'";
	pos = strResponse.find(pattern);
	if (pos == string::npos)
	{
		LogError("empty key_check_isChange");
		return false;
	}

	//32位的md5
	m_strKeyCheckIsChange = strResponse.substr(pos + pattern.length(), 56);
	return true;
}

bool Client12306::getPassengerDTOs(vector<passenager>& v)
{
	//_json_att=&REPEAT_SUBMIT_TOKEN=SubmitToken
	string param = "_json_att=";
	param += "&REPEAT_SUBMIT_TOKEN=";
	param += m_strGlobalRepeatSubmitToken;
	string strResponse;
	string strCookie = "Cookie: ";
	strCookie += m_strCookies;
	if (!HttpRequest(URL_GET_PASSENGER_DTO, strResponse, false, strCookie.c_str(), param.c_str(), false, 10))
	{
		LogError("getPassengerDTOs failed");
		return false;
	}

	///** 成功返回
	//{"validateMessagesShowId":"_validatorMessage","status":true,"httpstatus":200,"data":{"isExist":true,"exMsg":"",
	//"two_isOpenClick":["93","95","97","99"],"other_isOpenClick":["91","93","98","99","95","97"],
	//"normal_passengers":[{"code":"8","passenger_name":"寮犺繙榫?,"sex_code":"M","sex_name":"鐢?,"born_date":"1989-12-08 00:00:00","country_code":"CN",
	//"passenger_id_type_code":"1","passenger_id_type_name":"浜屼唬韬唤璇?,"passenger_id_no":"342623198912088150","passenger_type":"1",
	//"passenger_flag":"0","passenger_type_name":"鎴愪汉","mobile_no":"13917043320","phone_no":"","email":"balloonwj@qq.com",
	//"address":"","postalcode":"","first_letter":"","recordCount":"13","total_times":"99","index_id":"0"}
	Json::Reader JsonReader;
	Json::Value JsonRoot;
	if (!JsonReader.parse(strResponse, JsonRoot))
		return false;

	//{"validateMessagesShowId":"_validatorMessage", "status" : true, 
	//"httpstatus" : 200, "data" : {"otherMsg":"", loginCheck : "Y"}, "messages" : [], "validateMessages" : {}}
	if (JsonRoot["status"].isNull())
		return false;

	bool bStatus = JsonRoot["status"].asBool();
	if (!bStatus)
		return false;

	if (JsonRoot["httpstatus"].isNull() || JsonRoot["httpstatus"].asInt() != 200)
		return false;

	if (JsonRoot["data"].isNull() || JsonRoot["data"]["normal_passengers"].isNull())
		return false;

	if (!JsonRoot["data"]["normal_passengers"].isArray())
		return false;

	for (UINT i = 0; i < JsonRoot["data"]["normal_passengers"].size(); ++i)
	{
		passenager p;
		p.code = JsonRoot["data"]["normal_passengers"][i]["code"].asString();  //"8"
		p.passenger_name = JsonRoot["data"]["normal_passengers"][i]["passenger_name"].asString();  //"张远龙"
		p.sex_code = JsonRoot["data"]["normal_passengers"][i]["sex_code"].asString();// "M"
		p.sex_name = JsonRoot["data"]["normal_passengers"][i]["sex_name"].asString();; // "男"
		p.born_date = JsonRoot["data"]["normal_passengers"][i]["born_date"].asString();; //"1989-12-08 00:00:00"
		p.country_code = JsonRoot["data"]["normal_passengers"][i]["country_code"].asString();// "CN"
		p.passenger_id_type_code = JsonRoot["data"]["normal_passengers"][i]["passenger_id_type_code"].asString();//  "1"
		p.passenger_id_type_name = JsonRoot["data"]["normal_passengers"][i]["passenger_id_type_name"].asString(); // "二代身份证"
		p.passenger_id_no = JsonRoot["data"]["normal_passengers"][i]["passenger_id_no"].asString(); // "342623198912088150"
		p.passenger_type = JsonRoot["data"]["normal_passengers"][i]["passenger_type"].asString(); // "1"
		p.passenger_flag = JsonRoot["data"]["normal_passengers"][i]["passenger_flag"].asString(); // "0"
		p.passenger_type_name = JsonRoot["data"]["normal_passengers"][i]["passenger_type_name"].asString(); // "成人"
		p.mobile_no = JsonRoot["data"]["normal_passengers"][i]["mobile_no"].asString(); // "13917043320"
		p.phone_no = JsonRoot["data"]["normal_passengers"][i]["phone_no"].asString();
		p.email = JsonRoot["data"]["normal_passengers"][i]["email"].asString(); // "balloonwj@qq.com"
		p.address = JsonRoot["data"]["normal_passengers"][i]["address"].asString(); //  ""
		p.postalcode = JsonRoot["data"]["normal_passengers"][i]["postalcode"].asString(); // ""
		p.first_letter = JsonRoot["data"]["normal_passengers"][i]["first_letter"].asString();// ""
		p.recordCount = JsonRoot["data"]["normal_passengers"][i]["recordCount"].asString();// "13"
		p.total_times = JsonRoot["data"]["normal_passengers"][i]["total_times"].asString();// "99"
		p.index_id = JsonRoot["data"]["normal_passengers"][i]["index_id"].asString();// "0"
		v.push_back(p);
	}


	return true;
}

bool Client12306::checkOrderInfo(const char* oldPassengerStr, const char* passengerTicketStr, const char* tour_flag, bool& bVerifyVCode)
{
	//_json_att=&REPEAT_SUBMIT_TOKEN=SubmitToken
	string param = "_json_att=";
	param += "&bed_level_order_num=000000000000000000000000000000";
	param += "&cancel_flag=2";
	param += "&oldPassengerStr=";
	param += oldPassengerStr;
	param += "&passengerTicketStr=";
	param += passengerTicketStr;
	param += "&randCode=";
	param += "&REPEAT_SUBMIT_TOKEN=";
	param += m_strGlobalRepeatSubmitToken;
	param += "&tour_flag=";
	param += tour_flag;

	string strResponse;
	string strCookie = "Cookie: ";
	strCookie += m_strCookies;
	if (!HttpRequest(URL_CHECK_ORDER_INFO, strResponse, false, strCookie.c_str(), param.c_str(), false, 10))
	{
		LogError("checkOrderInfo failed");
		return false;
	}

	///** 成功返回
	/*
	应答：{"validateMessagesShowId":"_validatorMessage","status":true,"httpstatus":200,
		 "data":{"ifShowPassCode":"N","canChooseBeds":"N","canChooseSeats":"N","choose_Seats":"MOP9",
		 "isCanChooseMid":"N","ifShowPassCodeTime":"1","submitStatus":true,"smokeStr":""},
		 "messages":[],"validateMessages":{}}

		 {"validateMessagesShowId":"_validatorMessage","status":true,"httpstatus":200,
		 "data":{"errMsg":"对不起，由于您取消次数过多，今日将不能继续受理您的订票请求。2月10日您可继续使用订票功能。",
		 "submitStatus":false},"messages":[],"validateMessages":{}}
	*/
	Json::Reader JsonReader;
	Json::Value JsonRoot;
	OutputDebugStringA("\n");
	OutputDebugStringA(strResponse.c_str());
	OutputDebugStringA("\n");

	if (!JsonReader.parse(strResponse, JsonRoot))
		return false;

	if (JsonRoot["status"].isNull())
		return false;

	bool bStatus = JsonRoot["status"].asBool();
	if (!bStatus)
		return false;

	if (JsonRoot["httpstatus"].isNull() || JsonRoot["httpstatus"].asInt() != 200)
		return false;

	if (JsonRoot["data"].isNull() || JsonRoot["data"]["ifShowPassCode"].isNull())
		return false;

	if (JsonRoot["data"]["ifShowPassCode"].asString() == "N")
		return true;
	else if (JsonRoot["data"]["ifShowPassCode"].asString() == "Y")
	{
		//TODO: 需要校验验证码
		bVerifyVCode = true;
		return true;
	}

	return false;
}

bool Client12306::getQueueCount(const char* fromStationTelecode, const char* leftTicket, const char* purpose_codes, const char* seatType, const char* stationTrainCode, const char* toStationTelecode, const char* train_date, const char* train_location, const char* train_no)
{
	string param = "_json_att=";
	param += "&fromStationTelecode=";
	param += fromStationTelecode;
	param += "&leftTicket=";
	param += leftTicket;
	param += "&purpose_codes=";
	param += purpose_codes;
	param += "&REPEAT_SUBMIT_TOKEN=";
	param += m_strGlobalRepeatSubmitToken;
	param += "&seatType=";
	param += seatType;
	param += "&stationTrainCode=";
	param += stationTrainCode;
	param += "&toStationTelecode=";
	param += toStationTelecode;
	param += "&train_date=";
	param += train_date;
	param += "&train_location=";
	param += train_location;
	param += "&train_no=";
	param += train_no;

	string strResponse;
	string strCookie = "Cookie: ";
	strCookie += m_strCookies;
	if (!HttpRequest(URL_GET_QUEUE_COUNT, strResponse, false, strCookie.c_str(), param.c_str(), false, 10))
	{
		LogError("getQueueCount failed");
		return false;
	}

	///** 成功返回
	/*
	应答：{"validateMessagesShowId":"_validatorMessage","status":true,"httpstatus":200,"data":{"count":"4","ticket":"669","op_2":"false","countT":"0","op_1":"true"},"messages":[],"validateMessages":{}}
	*/
	Json::Reader JsonReader;
	Json::Value JsonRoot;
	if (!JsonReader.parse(strResponse, JsonRoot))
		return false;

	if (JsonRoot["status"].isNull())
		return false;

	bool bStatus = JsonRoot["status"].asBool();
	if (!bStatus)
		return false;

	if (JsonRoot["httpstatus"].isNull() || JsonRoot["httpstatus"].asInt() != 200)
		return false;

	return true;
}

bool Client12306::confirmSingleForQueue(const char* leftTicketStr, const char* oldPassengerStr, const char* passengerTicketStr, const char* purpose_codes, const char* train_location)
{
	string param;
	param += "passengerTicketStr=";
	param += passengerTicketStr;
	param += "&oldPassengerStr=";
	param += oldPassengerStr;
	param += "&randCode=";
	param += "&purpose_codes=";
	param += purpose_codes;
	param += "&key_check_isChange=";
	param += m_strKeyCheckIsChange;
	param += "&leftTicketStr=";
	param += leftTicketStr;
	param += "&train_location=";
	param += train_location;
	param += "&choose_seats=";
	param += "&seatDetailType=000";
	param += "&roomType=00";
	param += "&dwAll=N";
	param += "&_json_att=";
	param += "&REPEAT_SUBMIT_TOKEN=";
	param += m_strGlobalRepeatSubmitToken;

	string strResponse;
	string strCookie = "Cookie: ";
	strCookie += m_strCookies;
	if (!HttpRequest(URL_CONFIRM_SINGLE_FOR_QUEUE, strResponse, false, strCookie.c_str(), param.c_str(), false, 10))
	{
		LogError("confirmSingleForQueue failed");
		return false;
	}

	///** 成功返回
	/*
	应答：{"validateMessagesShowId":"_validatorMessage","status":true,"httpstatus":200,"data":{"submitStatus":true},"messages":[],"validateMessages":{}}
		 {"validateMessagesShowId":"_validatorMessage","status":false,"httpstatus":200,"data":"扣票失败","messages":[],"validateMessages":{}}
	*/
	Json::Reader JsonReader;
	Json::Value JsonRoot;
	if (!JsonReader.parse(strResponse, JsonRoot))
		return false;

	if (JsonRoot["status"].isNull())
		return false;

	bool bStatus = JsonRoot["status"].asBool();
	if (!bStatus)
		return false;

	if (JsonRoot["httpstatus"].isNull() || JsonRoot["httpstatus"].asInt() != 200)
		return false;

	if (JsonRoot["data"].isNull() || JsonRoot["data"]["submitStatus"].isNull())
		return false;

	if (JsonRoot["data"]["submitStatus"].asBool() != true)
		return false;

	return true;
}

bool Client12306::queryOrderWaitTime(const char* tourflag, string& orderId)
{
	//https://kyfw.12306.cn/otn/confirmPassenger/queryOrderWaitTime
	//random=1486368851278&tourFlag=dc&_json_att=&REPEAT_SUBMIT_TOKEN=691c09b5605e46bfb2ec2380ee65de0e
	ostringstream osParam;
	osParam << "random=";
	int d = rand();
	//osURL.precision(17);
	osParam << d;
	osParam << "&tourFlag=";
	osParam << tourflag;
	osParam << "&_json_att=&REPEAT_SUBMIT_TOKEN=";
	osParam << m_strGlobalRepeatSubmitToken;

	string strResponse;
	string strCookie = "Cookie: ";
	strCookie += m_strCookies;
	if (!HttpRequest(URL_QUERY_ORDER_WAIT_TIME, strResponse, false, strCookie.c_str(), osParam.str().c_str(), false, 10))
	{
		LogError("queryOrderWaitTime failed");
		return false;
	}

	///** 成功返回
	/*
	应答：{"validateMessagesShowId":"_validatorMessage","status":true,"httpstatus":200,"data":{"queryOrderWaitTimeStatus":true,"count":0,"waitTime":-1,"requestId":6234282826330508533,"waitCount":0,"tourFlag":"dc","orderId":"E061149209"},"messages":[],"validateMessages":{}}
	*/
	Json::Reader JsonReader;
	Json::Value JsonRoot;
	if (!JsonReader.parse(strResponse, JsonRoot))
		return false;

	if (JsonRoot["status"].isNull())
		return false;

	bool bStatus = JsonRoot["status"].asBool();
	if (!bStatus)
		return false;

	if (JsonRoot["httpstatus"].isNull() || JsonRoot["httpstatus"].asInt() != 200)
		return false;

	if (JsonRoot["data"].isNull() || JsonRoot["data"]["queryOrderWaitTimeStatus"].isNull())
		return false;

	if (JsonRoot["data"]["queryOrderWaitTimeStatus"].asBool() != true)
		return false;

	if (JsonRoot["data"]["orderId"].isNull())
		return false;

	orderId = JsonRoot["data"]["orderId"].asString();

	return true;
}

bool Client12306::GetStationInfo(vector<stationinfo>& si, bool bForceDownload/* = false*/)
{
	FILE* pfile;
	pfile = fopen("station_name.js", "rt+");
	//文件不存在，则必须下载
	if (pfile == NULL)
	{
		bForceDownload = true;
	}
	string strResponse;
	if (bForceDownload)
	{
		if (pfile != NULL)
			fclose(pfile);
		pfile = fopen("station_name.js", "wt+");
		if (pfile == NULL)
		{
			LogError("Unable to create station_name.js");
			return false;
		}

		CURLcode res;
		CURL* curl = curl_easy_init();
		if (NULL == curl)
		{
			fclose(pfile);
			return false;
		}

		//URL_STATION_NAMES
		curl_easy_setopt(curl, CURLOPT_URL, URL_STATION_NAMES);
		//响应结果中保留头部信息
		//curl_easy_setopt(curl, CURLOPT_HEADER, 1);
		curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);
		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
		//设定为不验证证书和HOST
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);

		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);

		res = curl_easy_perform(curl);
		bool bError = false;
		if (res == CURLE_OK)
		{
			int code;
			res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
			if (code != 200)
			{
				bError = true;
				LogError("http response code is not 200, code=%d", code);
			}
		}
		else
		{
			LogError("http request error, error code = %d", res);
			bError = true;
		}

		curl_easy_cleanup(curl);

		if (bError)
		{
			fclose(pfile);
			return !bError;
		}

		if (fwrite(strResponse.data(), strResponse.length(), 1, pfile) != 1)
		{
			LogError("Write data to station_name.js error");
			return false;
		}
		fclose(pfile);
	}
	//直接读取文件
	else
	{
		//得到文件大小
		fseek(pfile, 0, SEEK_END);
		int length = ftell(pfile);
		if (length < 0)
		{
			LogError("invalid station_name.js file");
			fclose(pfile);
		}
		fseek(pfile, 0, SEEK_SET);
		length++;
		char* buf = new char[length];
		memset(buf, 0, length * sizeof(char));
		if (fread(buf, length - 1, 1, pfile) != 1)
		{
			LogError("read station_name.js file error");
			fclose(pfile);
			return false;
		}
		strResponse = buf;
		fclose(pfile);
	}


	/*
	返回结果为一个js文件，
	var station_names = '@bjb|北京北|VAP|beijingbei|bjb|0@bjd|北京东|BOP|beijingdong|bjd|1@bji|北京|BJP|beijing|bj|2"
	*/
	//LogInfo("recv json = %s", strResponse.c_str());
	OutputDebugStringA(strResponse.c_str());

	vector<string> singleStation;
	split(strResponse, "@", singleStation);

	size_t size = singleStation.size();
	for (size_t i = 1; i < size; ++i)
	{
		vector<string> v;
		split(singleStation[i], "|", v);
		if (v.size() < 6)
			continue;

		stationinfo st;
		st.code1 = v[0];
		st.hanzi = v[1];
		st.code2 = v[2];
		st.pingyin = v[3];
		st.simplepingyin = v[4];
		st.no = atol(v[5].c_str());

		si.push_back(st);
	}

	return true;
}

bool Client12306::QueryPassengers(int pageindex/* = 0*/, int pagesize/* = 10*/)
{
	CURLcode res;
	CURL* curl = curl_easy_init();
	if (NULL == curl)
	{
		return false;
	}

	//https://kyfw.12306.cn/otn/passengers/query
	curl_easy_setopt(curl, CURLOPT_URL, "https://kyfw.12306.cn/otn/passengers/query");
	//响应结果中保留头部信息
	//curl_easy_setopt(curl, CURLOPT_HEADER, 1);
	curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
	string strResponse;
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	//设定为不验证证书和HOST
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);

	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);

	struct curl_slist *chunk = NULL;
	/* Add a custom header */
	string strHeader = "Cookie: ";
	strHeader += m_strCookies;
	chunk = curl_slist_append(chunk, strHeader.c_str());
	/* set our custom set of headers */
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

	std::ostringstream osData;
	osData << "pageIndex=" << pageindex << "&pageSize=" << pagesize;
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, osData.str().c_str());

	res = curl_easy_perform(curl);
	bool bError = false;
	if (res == CURLE_OK)
	{
		int code;
		res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
		if (code != 200)
		{
			bError = true;
			LogError("http response code is not 200, code=%d", code);
		}
	}
	else
	{
		LogError("http request error, error code = %d", res);
		bError = true;
	}

	curl_easy_cleanup(curl);

	if (bError)
		return !bError;

	///** 成功返回
	//参数错误
	/*
	{"validateMessagesShowId":"_validatorMessage", "status" : true, "httpstatus" : 200,
		"data" : {"datas":null, "message" : "查询失败", "flag" : false, "pageTotal" : 0},
		"messages" : [], "validateMessages" : {}}
	*/
	//正确响应结果：
	/*
	{"validateMessagesShowId":"_validatorMessage","status":true,"httpstatus":200,
	"data":{"datas":[{"code":"9","passenger_name":"朱效然",
	"sex_code":"M","sex_name":"男","born_date":"2014-12-18 00:00:00",
	"country_code":"CN","passenger_id_type_code":"1",
	"passenger_id_type_name":"二代身份证","passenger_id_no":"342921198302042714",
	"passenger_type":"1","passenger_flag":"0","passenger_type_name":"成人",
	"mobile_no":"18061938073","phone_no":"","email":"","address":"",
	"postalcode":"","first_letter":"ZXR","recordCount":"13",
	"isUserSelf":"N","total_times":"99"},
	{"code":"7","passenger_name":"张远凤","sex_code":"M","sex_name":"男",
	"born_date":"1980-01-01 00:00:00","country_code":"CN","passenger_id_type_code":"1","passenger_id_type_name":"二代身份证","passenger_id_no":"342623199812048121","passenger_type":"1","passenger_flag":"0",
	"passenger_type_name":"成人","mobile_no":"13501734948","phone_no":"",
	"email":"","address":"","postalcode":"","first_letter":"ZYF",
	"recordCount":"13","isUserSelf":"N","total_times":"99"},
	{"code":"8","passenger_name":"张远龙","sex_code":"M","sex_name":"男",
	"born_date":"1989-12-08 00:00:00","country_code":"CN","passenger_id_type_code":"1",
	"passenger_id_type_name":"二代身份证","passenger_id_no":"342623198912088150",
	"passenger_type":"1","passenger_flag":"0","passenger_type_name":"成人",
	"mobile_no":"13917043320","phone_no":"","email":"balloonwj@qq.com",
	"address":"","postalcode":"","first_letter":"","recordCount":"13",
	"isUserSelf":"Y","total_times":"99"}],"flag":true,"pageTotal":2},
	"messages":[],"validateMessages":{}}
	*/
	wstring str = Utf8ToUnicode(strResponse);

	Json::Reader JsonReader;
	Json::Value JsonRoot;
	std::string strText(strResponse);
	LogInfo("recv json = %s", strText.c_str());

	if (!JsonReader.parse(strText, JsonRoot))
		return false;

	//{"validateMessagesShowId":"_validatorMessage", "status" : true, 
	//"httpstatus" : 200, "data" : {"otherMsg":"", loginCheck : "Y"}, "messages" : [], "validateMessages" : {}}
	if (JsonRoot["status"].isNull())
		return false;

	bool bStatus = JsonRoot["status"].asBool();
	if (!bStatus)
		return false;

	if (JsonRoot["httpstatus"].isNull() || JsonRoot["httpstatus"].asInt() != 200)
		return false;

	if (JsonRoot["data"].isNull() || !JsonRoot["data"].isObject())
		return false;

	if (JsonRoot["data"]["otherMsg"].isNull() || JsonRoot["data"]["otherMsg"].asString() != "")
		return false;
	if (JsonRoot["data"]["loginCheck"].isNull() || JsonRoot["data"]["loginCheck"].asString() != "Y")
		return 1;

	return true;
}

bool Client12306::GetCookies(const string& data)
{
	if (data.empty())
	{
		LogError("http data is empty");
		return false;
	}

	//解析http头部
	string str;
	str.append(data.c_str(), data.length());
	size_t n = str.find("\r\n\r\n");
	string header = str.substr(0, n);
	str.erase(0, n + 4);

	//m_cookie.clear();
	//获取http头中的JSESSIONID=21AC68643BBE893FBDF3DA9BCF654E98;
	vector<string> v;
	while (true)
	{
		size_t index = header.find("\r\n");
		if (index == string::npos)
			break;
		string tmp = header.substr(0, index);
		v.push_back(tmp);
		header.erase(0, index + 2);

		if (header.empty())
			break;
	}

	string jsessionid;
	string BIGipServerotn;
	string BIGipServerportal;
	string current_captcha_type;
	size_t m;
	OutputDebugStringA("\nresponse http headers:\n");
	for (size_t i = 0; i < v.size(); ++i)
	{
		OutputDebugStringA(v[i].c_str());
		OutputDebugStringA("\n");
		m = v[i].find("Set-Cookie: ");
		if (m == string::npos)
			continue;

		string tmp = v[i].substr(11);
		Trim(tmp);
		m = tmp.find("JSESSIONID");
		if (m != string::npos)
		{
			size_t comma = tmp.find(";");
			if (comma != string::npos)
				jsessionid = tmp.substr(0, comma);
		}

		m = tmp.find("BIGipServerotn");
		if (m != string::npos)
		{
			size_t comma = tmp.find(";");
			if (comma != string::npos)
				BIGipServerotn = tmp.substr(m, comma);
			else
				BIGipServerotn = tmp;
		}

		m = tmp.find("BIGipServerportal");
		if (m != string::npos)
		{
			size_t comma = tmp.find(";");
			if (comma != string::npos)
				BIGipServerportal = tmp.substr(m, comma);
			else
				BIGipServerportal = tmp;
		}

		m = tmp.find("current_captcha_type");
		if (m != string::npos)
		{
			size_t comma = tmp.find(";");
			if (comma != string::npos)
				current_captcha_type = tmp.substr(m, comma);
			else
				current_captcha_type = tmp;
		}
	}

	if (!jsessionid.empty())
	{
		m_strCookies = jsessionid;
		m_strCookies += "; ";
		m_strCookies += BIGipServerotn;
		if (!BIGipServerportal.empty())
		{
			m_strCookies += "; ";
			m_strCookies += BIGipServerportal;
		}
		m_strCookies += "; ";
		m_strCookies += current_captcha_type;
		return true;
	}

	LogError("jsessionid is empty");
	return false;
}

bool Client12306::HttpRequest(const char* url,
	string& strResponse,
	bool get/* = true*/,
	const char* headers/* = NULL*/,
	const char* postdata/* = NULL*/,
	bool bReserveHeaders/* = false*/,
	int timeout/* = 10*/)
{
	CURLcode res;
	CURL* curl = curl_easy_init();
	if (NULL == curl)
	{
		LogError("curl lib init error");
		return false;
	}

	curl_easy_setopt(curl, CURLOPT_URL, url);

	//响应结果中保留头部信息
	if (bReserveHeaders)
		curl_easy_setopt(curl, CURLOPT_HEADER, 1);
	curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	//设定为不验证证书和HOST
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);

	//设置超时时间
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, timeout);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
	curl_easy_setopt(curl, CURLOPT_REFERER, URL_REFERER);
	//12306早期版本是不需要USERAGENT这个字段的，现在必须了，估计是为了避免一些第三方的非法刺探吧。
	//如果没有这个字段，会返回
	/*
		HTTP/1.0 302 Moved Temporarily
		Location: http://www.12306.cn/mormhweb/logFiles/error.html
		Server: Cdn Cache Server V2.0
		Mime-Version: 1.0
		Date: Fri, 18 May 2018 02:52:05 GMT
		Content-Type: text/html
		Content-Length: 0
		Expires: Fri, 18 May 2018 02:52:05 GMT
		X-Via: 1.0 PSshgqdxxx63:10 (Cdn Cache Server V2.0)
		Connection: keep-alive
		X-Dscp-Value: 0
	 */
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 6.1; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/65.0.3325.146 Safari/537.36");
	//不设置接收的编码格式或者设置为空，libcurl会自动解压压缩的格式，如gzip
	//curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip, deflate, br");


	//添加自定义头信息
	if (headers != NULL)
	{
		//LogInfo("http custom header: %s", headers);
		struct curl_slist *chunk = NULL;
		chunk = curl_slist_append(chunk, headers);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
	}

	if (!get && postdata != NULL)
	{
		//LogInfo("http post data: %s", postdata);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postdata);
	}

	LogInfo("http %s: url=%s, headers=%s, postdata=%s", get ? "get" : "post", url, headers != NULL ? headers : "", postdata != NULL ? postdata : "");

	res = curl_easy_perform(curl);
	bool bError = false;
	if (res == CURLE_OK)
	{
		int code;
		res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
		if (code != 200 && code != 302)
		{
			bError = true;
			LogError("http response code is not 200 or 302, code=%d", code);
		}
	}
	else
	{
		LogError("http request error, error code = %d", res);
		bError = true;
	}

	curl_easy_cleanup(curl);

	LogInfo("http response: %s", strResponse.c_str());

	return !bError;
}