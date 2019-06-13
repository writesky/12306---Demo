/**
 *@desc:	��������log��,log.cpp
 **/

#include"stdafx.h"
#include"Log.h"
#include<stdio.h>
#include<stdarg.h>
#include<time.h>

Log& Log::GetInstance()
{
	static Log Log;
	return Log;
}

bool Log::AddLog(const char* pszLevel, const char* pszFile, int lineNo, const char* pszFuncSig, char* pszFmt, ...)
{
	if (m_file == NULL)
		return false;
	char tmp[8192 * 10] = { 0 };
	va_list va;                        //����һ��va_list�͵ı���,���������ָ�������ָ��.
	va_start(va, pszFmt);              //��va_start���ʼ������,�����ĵڶ��������ǵ�һ���ɱ������ǰһ������,��һ���̶��Ĳ���
	_vsnprintf(tmp, ARRAYSIZE(tmp), pszFmt, va);		//ע��,��Ҫ©��ǰ���_
	va_end(va);

	time_t now = time(NULL);
	struct tm* tmstr = localtime(&now);
	char content[8192 * 10 + 256] = { 0 };
	sprintf_s(content, ARRAYSIZE(content), "[%04d-%02d-%02d %02d:%02d:%02d][%s][0x%04x][%s:%d %s]%s\r\n",
		tmstr->tm_year + 1900,
		tmstr->tm_mon + 1,
		tmstr->tm_mday,
		tmstr->tm_hour,
		tmstr->tm_min,
		tmstr->tm_sec,
		pszLevel,
		GetCurrentThreadId(),
		pszFile,
		lineNo,
		pszFuncSig,
		tmp);
	if (fwrite(content, strlen(content), 1, m_file) != 1)
		return false;
	fflush(m_file);
	return true;
}

Log::Log()
{
	time_t now = time(NULL);
	struct tm* tmstr = localtime(&now);
	char filename[256];
	sprintf_s(filename, ARRAYSIZE(filename), "%04d%02d%02d%02d%02d%02d.log",
		tmstr->tm_year + 1900,
		tmstr->tm_mon + 1,
		tmstr->tm_mday,
		tmstr->tm_hour,
		tmstr->tm_min,
		tmstr->tm_sec);

	m_file = fopen(filename, "at+");
}

Log::~Log()
{
	if (m_file != NULL)
		fclose(m_file);
}