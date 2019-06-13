/**
 * 用户信息, UserInfo.h
 */
#include"stdafx.h"
#include"UserInfo.h"

CUserInfo& CUserInfo::GetInstance()
{
	static CUserInfo u;
	return u;
}