#pragma once
//*@desc:	封装获取验证码、校验验证码、登录等12306各个请求的类，Client12306.h文件

#ifndef __CLIENT_12306_H_
#define __CLIENT_12306_H_

#include<vector>
#include<string>

using namespace std;

//车次类型
#define TRAIN_GC    0x00000001
#define TRAIN_D      (0x00000001 << 1)
#define TRAIN_Z      (0x00000001 << 2)
#define TRAIN_T      (0x00000001 << 3)
#define TRAIN_K      (0x00000001 << 4)
#define TRAIN_OTHER  (0x00000001 << 5)
#define TRAIN_ALL    (TRAIN_GC | TRAIN_D | TRAIN_Z | TRAIN_T | TRAIN_K | TRAIN_OTHER)

//票信息
struct queryLeftNewDTO
{
	string  train_no;
	string  station_train_code;
	string  start_station_telecode;     //始发站
	string  start_station_name;
	string  end_station_telecode;       //终点站
	string  end_station_name;
	string  from_station_telecode;      //出发站
	string  from_station_name;          //到达站
	string  to_station_telecode;
	string  to_station_name;
	string  start_time;
	string  arrive_time;
	string  day_difference;
	string  train_class_name;
	string  lishi;
	string  canWebBuy;
	string  lishiValue;
	string  yp_info;
	string  control_train_day;
	string  start_train_date;
	string  seat_feature;
	string  yp_ex;
	string  train_seat_feature;
	string  seat_types;
	string  location_code;
	string  from_station_no;
	string  to_station_no;
	string  control_day;
	string  sale_time;
	string  is_support_card;
	string  controlled_train_flag;
	string  controlled_train_message;
	string  train_type_code;
	string  start_province_code;
	string  start_city_code;
	string  end_province_code;
	string  end_city_code;

	string  swz_num;    //商务座	
	string  rz_num;     //软座
	string  yz_num;     //硬座

	string  gr_num;     //高级软卧
	string  rw_num;     //软卧
	string  yw_num;     //硬卧

	string  tz_num;     //特等座
	string  zy_num;     //一等座
	string  ze_num;     //二等座	
	string  wz_num;     //无座

	string  gg_num;
	string  yb_num;
	string  qt_num;

};

struct ticketinfo
{
	queryLeftNewDTO DTO;
	string secretStr;
	string buttonTextInfo;
};

//var station_names = '@bjb|北京北|VAP|beijingbei|bjb|0@bjd|北京东|BOP|beijingdong|bjd|1@bji|北京|BJP|beijing|bj|2
struct stationinfo
{
	string code1;
	string hanzi;
	string code2;
	string pingyin;
	string simplepingyin;
	int no;
};

struct passenager
{
	string code;   //"8"
	string passenger_name;   //"姓名"
	string sex_code;   //"M"
	string sex_name;  //"男"
	string born_date; //"1989-12-08 00:00:00"
	string country_code;// "CN"
	string passenger_id_type_code;//  "1"
	string passenger_id_type_name; // "二代身份证"
	string passenger_id_no; // "342623198912088150"
	string passenger_type; // "1"
	string passenger_flag; // "0"
	string passenger_type_name; // "成人"
	string mobile_no; // "13917043320"
	string phone_no;
	string email; // "balloonwj@qq.com"
	string address; //  ""
	string postalcode; // ""
	string first_letter;// ""
	string recordCount;// "13"
	string total_times;// "99"
	string index_id;// "0"
};

class Client12306
{
public:
	static Client12306& GetInstance();
private:
	Client12306();
	~Client12306();

private:
	Client12306(const Client12306&);
	Client12306& operator=(const Client12306&);
public:
	bool ReloadVCodeImage();
	/**
	* 游客查票
	* https://kyfw.12306.cn/otn/leftTicket/query?leftTicketDTO.train_date=2017-05-24&leftTicketDTO.from_station=BJP&leftTicketDTO.to_station=SHH&purpose_codes=ADULT
	* 应答：{“validateMessagesShowId”:”_validatorMessage”,”status”:true,”httpstatus”:200,”messages”:[],”validateMessages”:{}}
	*@param: train_date列车发车日期，格式：2017-01-28
	*@param: from_station出发站，格式：SHH 对应上海
	*@parma: to_station到站,格式：BJP 对应北京
	*@param: purpose_codes 票类型，成人票：ADULT 学生票：0X00
	*@param: v 查票结果
	*/
	bool GuestQueryTicket(const char* train_data, const char* from_station, const char* to_station, const char* purpose_codes, vector<ticketinfo>& v);
	/**
	* 初始化session，获取JSESSIONID
	*/
	bool loginInit();
	bool DownloadVCodeImage(const char* module = "login");
	/**
	*@return 0校验成功；1校验失败；2校验出错
	*/
	int checkRandCodeAnsyn(const char* vcode);
	/**
	*@return 0校验成功；1校验失败；2校验出错
	*/
	int loginAysnSuggest(const char* user, const char* pass, const char* vcode);
	/**
	* 正式登录
	*/
	bool userLogin();

	/**
	 * 模拟12306跳转
	 */
	bool initMy12306();

	/**
	 * 拉取乘客买票验证码
	 */
	 //bool GetVCodeImage();

	 /**
	 * 查询余票第一步
	 * https://kyfw.12306.cn/otn/leftTicket/log?leftTicketDTO.train_date=2017-02-08&leftTicketDTO.from_station=SHH&leftTicketDTO.to_station=NJH&purpose_codes=ADULT
	 * 应答：{“validateMessagesShowId”:”_validatorMessage”,”status”:true,”httpstatus”:200,”messages”:[],”validateMessages”:{}}
	 *@param: train_date列车发车日期，格式：2017-01-28
	 *@param: from_station出发站，格式：SHH 对应上海
	 *@parma: to_station到站,格式：BJP 对应北京
	 *@param: purpose_codes 票类型，成人票：ADULT 学生票：0X00
	 */
	bool QueryTickets1(const char* train_date, const char* from_station, const char* to_station, const char* purpose_codes);

	/**
	 * 查询余票第二步
	 * 这几种情形都有可能，所以应该都尝试一下
	 * https://kyfw.12306.cn/otn/leftTicket/queryZ?leftTicketDTO.train_date=2017-02-08&leftTicketDTO.from_station=SHH&leftTicketDTO.to_station=NJH&purpose_codes=ADULT
	 * https://kyfw.12306.cn/otn/leftTicket/queryX?leftTicketDTO.train_date=2017-02-08&leftTicketDTO.from_station=SHH&leftTicketDTO.to_station=NJH&purpose_codes=ADULT
	 * https://kyfw.12306.cn/otn/leftTicket/query?leftTicketDTO.train_date=2017-02-08&leftTicketDTO.from_station=SHH&leftTicketDTO.to_station=NJH&purpose_codes=ADULT
	 * {"status":false,"c_url":"leftTicket/query","c_name":"CLeftTicketUrl"}
	 * {"validateMessagesShowId":"_validatorMessage","status":true,"httpstatus":200,"messages":["非法请求"],"validateMessages":{}}
	 * 应答中含有实际余票信息
	 *@param: train_date列车发车日期，格式：2017-01-28
	 *@param: from_station出发站，格式：SHH 对应上海
	 *@parma: to_station到站,格式：BJP 对应北京
	 *@param: purpose_codes 票类型，成人票：ADULT 学生票：0X00
	 */
	bool QueryTickets2(const char* train_data, const char* from_station, const char* to_station, const char* purpose_codes, vector<ticketinfo>& v);
	/**
	* 检测用户是否登录
	* https://kyfw.12306.cn/otn/login/checkUser POST _json_att=
	* Cookie: JSESSIONID=0A01D967FCD9827FC664E43DEE3C7C6EF950F677C2; __NRF=86A7CBA739653C1CC2C3C3AA7C88A1E3; BIGipServerotn=1742274826.64545.0000; BIGipServerportal=3134456074.17695.0000; current_captcha_type=Z; _jc_save_fromStation=%u4E0A%u6D77%2CSHH; _jc_save_toStation=%u5357%u4EAC%2CNJH; _jc_save_fromDate=2017-01-22; _jc_save_toDate=2017-01-22; _jc_save_wfdc_flag=dc
	* {"validateMessagesShowId":"_validatorMessage","status":true,"httpstatus":200,"data":{"flag":true},"messages":[],"validateMessages":{}}
	*/
	bool checkUser();

	/**
	 * 预提交订单 POST
	 * https://kyfw.12306.cn/otn/leftTicket/submitOrderRequest?secretStr=secretStr&train_date=2017-01-21&back_train_date=2016-12-23&tour_flag=dc&purpose_codes=ADULT&query_from_station_name=深圳&query_to_station_name=武汉&undefined=
	 */
	bool submitOrderRequest(const char* secretStr, const char* train_data, const char* back_train_data, const char* tour_flag, const char* purpose_codes, const char* query_from_station_name, const char* query_to_station_name);
	/**
	 * 模拟跳转页面InitDc，Post
	 */
	bool initDc();
	/**
	 * 拉取常用联系人 POST
	 * https://kyfw.12306.cn/otn/confirmPassenger/getPassengerDTOs?_json_att=&REPEAT_SUBMIT_TOKEN=SubmitToken
	 */
	bool getPassengerDTOs(vector<passenager>& v);
	/**
	 * 购票人确定
	 * https://kyfw.12306.cn/otn/confirmPassenger/checkOrderInfo
	 @param oldPassengerStr	oldPassengerStr组成的格式：乘客名,passenger_id_type_code,passenger_id_no,passenger_type，’_’
							示例： 张远龙,1,342623198912088150,1_
	 @param passengerTicketStr	passengerTicketStr组成的格式：seatType,0,票类型（成人票填1）,乘客名,passenger_id_type_code,passenger_id_no,mobile_no,’N’
							示例： O,0,1,张远龙,1,342623198912088150,13917043320,N	101
	 @tour_flag	dc表示单程票
	 应答：{"validateMessagesShowId":"_validatorMessage","status":true,"httpstatus":200,"data":{"ifShowPassCode":"N","canChooseBeds":"N","canChooseSeats":"N","choose_Seats":"MOP9","isCanChooseMid":"N","ifShowPassCodeTime":"1","submitStatus":true,"smokeStr":""},"messages":[],"validateMessages":{}}
	 */
	bool checkOrderInfo(const char* oldPassengerStr, const char* passengerTicketStr, const char* tour_flag, bool& bVerifyVCode);
	/**
	 * 准备进入排队
	 * https://kyfw.12306.cn/otn/confirmPassenger/getQueueCount
	 _json_att		10
	 fromStationTelecode	VNP	23
	 leftTicket	enu80ehMzuVJlK2Q43c6kn5%2BzQF41LEI6Nr14JuzThrooN57	63
	 purpose_codes	00	16
	 REPEAT_SUBMIT_TOKEN	691c09b5605e46bfb2ec2380ee65de0e	52
	 seatType	O	10
	 stationTrainCode	G5	19
	 toStationTelecode	AOH	21
	 train_date	Fri Feb 10 00:00:00 UTC+0800 2017	50
	 train_location	P2	17
	 train_no	24000000G502	21
	 应答：{"validateMessagesShowId":"_validatorMessage","status":true,"httpstatus":200,"data":{"count":"4","ticket":"669","op_2":"false","countT":"0","op_1":"true"},"messages":[],"validateMessages":{}}
	 */
	bool getQueueCount(const char* fromStationTelecode, const char* leftTicket, const char* perpose_codes, const char* seatType, const char* stationTrainCode, const char* toStationTelecode, const char* train_date, const char* train_location, const char* train_no);
	/**
	 * 确认购买
	 * https://kyfw.12306.cn/otn/confirmPassenger/confirmSingleForQueue
	 _json_att		10
	 choose_seats		13
	 dwAll	N	7
	 key_check_isChange	7503FD317E01E290C3D95CAA1D26DD8CFA9470C3643BA9799D3FB753	75
	 leftTicketStr	enu80ehMzuVJlK2Q43c6kn5%2BzQF41LEI6Nr14JuzThrooN57	66
	 oldPassengerStr	张远龙,1,342623198912088150,1_	73
	 passengerTicketStr	O,0,1,张远龙,1,342623198912088150,13917043320,N	101
	 purpose_codes	00	16
	 randCode		9
	 REPEAT_SUBMIT_TOKEN	691c09b5605e46bfb2ec2380ee65de0e	52
	 roomType	00	11
	 seatDetailType	000	18
	 train_location	P2	17
	 应答：{"validateMessagesShowId":"_validatorMessage","status":true,"httpstatus":200,"data":{"submitStatus":true},"messages":[],"validateMessages":{}}
	 */
	bool confirmSingleForQueue(const char* leftTicketStr, const char* oldPassengerStr, const char* passengerTicketStr, const char* purpose_codes, const char* train_location);
	/**
	 * 查询订单状态： https://kyfw.12306.cn/otn/confirmPassenger/queryOrderWaitTime?random=1486368851278&tourFlag=dc&_json_att=&REPEAT_SUBMIT_TOKEN=691c09b5605e46bfb2ec2380ee65de0e
	 GET
	 _json_att
	 random	1486368851278
	 REPEAT_SUBMIT_TOKEN	691c09b5605e46bfb2ec2380ee65de0e
	 tourFlag	dc
	 响应：{"validateMessagesShowId":"_validatorMessage","status":true,"httpstatus":200,"data":{"queryOrderWaitTimeStatus":true,"count":0,"waitTime":-1,"requestId":6234282826330508533,"waitCount":0,"tourFlag":"dc","orderId":"E061149209"},"messages":[],"validateMessages":{}}
	 */
	bool queryOrderWaitTime(const char* tourflag, string& orderId);
	/**
	 * https://kyfw.12306.cn/otn/confirmPassenger/resultOrderForDcQueue POST
	 _json_att		10
	 orderSequence_no	E061149209	27
	 REPEAT_SUBMIT_TOKEN	691c09b5605e46bfb2ec2380ee65de0e	52
	 {"validateMessagesShowId":"_validatorMessage","status":true,"httpstatus":200,"data":{"submitStatus":true},"messages":[],"validateMessages":{}}
	 */
	 //bool resultOrderForDcQueue();
	/**
	 * 未完成的订单页面 https://kyfw.12306.cn/otn/queryOrder/initNoComplete GET
	 * 获取未完成的订单 https://kyfw.12306.cn/otn/queryOrder/queryMyOrderNoComplete POST _json_att=
	 */
	 /*
	 {
	 "validateMessagesShowId": "_validatorMessage",
	 "status": true,
	 "httpstatus": 200,
	 "data": {
		 "orderDBList": [
			 {
				 "sequence_no": "E079331507",
				 "order_date": "2017-02-09 10:10:55",
				 "ticket_totalnum": 1,
				 "ticket_price_all": 55300,
				 "cancel_flag": "Y",
				 "resign_flag": "4",
				 "return_flag": "N",
				 "print_eticket_flag": "N",
				 "pay_flag": "Y",
				 "pay_resign_flag": "N",
				 "confirm_flag": "N",
				 "tickets": [
					 {
						 "stationTrainDTO": {
							 "trainDTO": {},
							 "station_train_code": "G41",
							 "from_station_telecode": "VNP",
							 "from_station_name": "北京南",
							 "start_time": "1970-01-01 09:16:00",
							 "to_station_telecode": "AOH",
							 "to_station_name": "上海虹桥",
							 "arrive_time": "1970-01-01 14:48:00",
							 "distance": "1318"
						 },
						 "passengerDTO": {
							 "passenger_name": "张远龙",
							 "passenger_id_type_code": "1",
							 "passenger_id_type_name": "二代身份证",
							 "passenger_id_no": "342623198912088150",
							 "total_times": "98"
						 },
						 "ticket_no": "E079331507110008B",
						 "sequence_no": "E079331507",
						 "batch_no": "1",
						 "train_date": "2017-02-11 00:00:00",
						 "coach_no": "10",
						 "coach_name": "10",
						 "seat_no": "008B",
						 "seat_name": "08B号",
						 "seat_flag": "0",
						 "seat_type_code": "O",
						 "seat_type_name": "二等座",
						 "ticket_type_code": "1",
						 "ticket_type_name": "成人票",
						 "reserve_time": "2017-02-09 10:10:55",
						 "limit_time": "2017-02-09 10:10:55",
						 "lose_time": "2017-02-09 10:40:55",
						 "pay_limit_time": "2017-02-09 10:40:55",
						 "ticket_price": 55300,
						 "print_eticket_flag": "N",
						 "resign_flag": "4",
						 "return_flag": "N",
						 "confirm_flag": "N",
						 "pay_mode_code": "Y",
						 "ticket_status_code": "i",
						 "ticket_status_name": "待支付",
						 "cancel_flag": "Y",
						 "amount_char": 0,
						 "trade_mode": "",
						 "start_train_date_page": "2017-02-11 09:16",
						 "str_ticket_price_page": "553.0",
						 "come_go_traveller_ticket_page": "N",
						 "return_deliver_flag": "N",
						 "deliver_fee_char": "",
						 "is_need_alert_flag": false,
						 "is_deliver": "N",
						 "dynamicProp": "",
						 "fee_char": "",
						 "insure_query_no": ""
					 }
				 ],
				 "reserve_flag_query": "p",
				 "if_show_resigning_info": "N",
				 "recordCount": "1",
				 "isNeedSendMailAndMsg": "N",
				 "array_passser_name_page": [
					 "张远龙"
				 ],
				 "from_station_name_page": [
					 "北京南"
				 ],
				 "to_station_name_page": [
					 "上海虹桥"
				 ],
				 "start_train_date_page": "2017-02-11 09:16",
				 "start_time_page": "09:16",
				 "arrive_time_page": "14:48",
				 "train_code_page": "G41",
				 "ticket_total_price_page": "553.0",
				 "come_go_traveller_order_page": "N",
				 "canOffLinePay": "N",
				 "if_deliver": "N",
				 "insure_query_no": ""
			 }
		 ],
		 "to_page": "db"
	 },
	 "messages": [],
	 "validateMessages": {}
	 */

	 /**
	  * 获取全国车站信息
	  *@param si 返回的车站信息
	  *@param bForceDownload 强制从网络上下载，即不使用本地副本
	  */
	bool GetStationInfo(vector<stationinfo>& si, bool bForceDownload = false);

	/**
	 * 查询常用联系人
	 */
	bool QueryPassengers(int pageindex = 2, int pagesize = 10);
	bool GetVCodeFileName(char* pszDst, int nLength);

	private:
		bool GetCookies(const string& data);
		/**
	 * 发送一个http请求
	 *@param url 请求的url
	 *@param strResponse http响应结果
	 *@param get true为GET，false为POST
	 *@param headers 附带发送的http头信息
	 *@param postdata post附带的数据
	 *@param bReserveHeaders http响应结果是否保留头部信息
	 *@param timeout http请求超时时间
	 */
		bool HttpRequest(const char* url, string& strResponse, bool get = true, const char* headers = NULL, const char* postdata = NULL, bool bReserveHeaders = false, int timeout = 10);
		private:
			char                m_szCurrVCodeName[256]; //当前验证码图片的名称
			string              m_strCookies;
			string              m_strGlobalRepeatSubmitToken;
			string              m_strKeyCheckIsChange;
};
#endif // !__CLIENT_12306_H_
