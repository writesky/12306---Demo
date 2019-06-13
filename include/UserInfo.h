#pragma once
/**
 * �û���Ϣ, UserInfo.h
 */
#ifndef _USER_H_
#define _USER_H_

#include<string>

using namespace std;
enum LOGIN_STATUS
{
	LOGIN_STATUS_NOTLOGIN, //δ��¼
	LOGIN_STATUS_LOGIN
};

class CUserInfo
{
public:
	static CUserInfo& GetInstance();
	
private:
	CUserInfo() :m_nLoginStatus(LOGIN_STATUS_NOTLOGIN)
	{

	}
	~CUserInfo() = default;

	CUserInfo(const CUserInfo& rhs) = delete;
	CUserInfo& operator=(const CUserInfo& rhs) = delete;
	//��Ϣ�����࣬��Ա��������ֱ�ӷ���
private:
	string      m_username;
	string      m_password;
	int         m_nLoginStatus;
};
#endif // !_USER_H_
