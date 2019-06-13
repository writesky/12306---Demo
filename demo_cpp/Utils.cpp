#include"stdafx.h"
#include"Utils.h"
WCHAR * AnsiToUnicode(const CHAR * lpszStr)
{
	WCHAR * lpUnicode;
	int nLen;

	if (NULL == lpszStr)
		return NULL;

	nLen = ::MultiByteToWideChar(CP_ACP, 0, lpszStr, -1, NULL, 0);
	if (0 == nLen)
		return NULL;

	lpUnicode = new WCHAR[nLen + 1];
	if (NULL == lpUnicode)
		return NULL;

	memset(lpUnicode, 0, sizeof(WCHAR) * (nLen + 1));
	nLen = ::MultiByteToWideChar(CP_ACP, 0, lpszStr, -1, lpUnicode, nLen);
	if (0 == nLen)
	{
		delete[]lpUnicode;
		return NULL;
	}

	return lpUnicode;
}

CHAR * UnicodeToAnsi(const WCHAR * lpszStr)
{
	CHAR * lpAnsi;
	int nLen;

	if (NULL == lpszStr)
		return NULL;

	nLen = ::WideCharToMultiByte(CP_ACP, 0, lpszStr, -1, NULL, 0, NULL, NULL);
	if (0 == nLen)
		return NULL;

	lpAnsi = new CHAR[nLen + 1];
	if (NULL == lpAnsi)
		return NULL;

	memset(lpAnsi, 0, nLen + 1);
	nLen = ::WideCharToMultiByte(CP_ACP, 0, lpszStr, -1, lpAnsi, nLen, NULL, NULL);
	if (0 == nLen)
	{
		delete[]lpAnsi;
		return NULL;
	}

	return lpAnsi;
}

CHAR * AnsiToUtf8(const CHAR * lpszStr)
{
	WCHAR * lpUnicode;
	CHAR * lpUtf8;
	int nLen;

	if (NULL == lpszStr)
		return NULL;

	nLen = ::MultiByteToWideChar(CP_ACP, 0, lpszStr, -1, NULL, NULL);
	if (0 == nLen)
		return NULL;

	lpUnicode = new WCHAR[nLen + 1];
	if (NULL == lpUnicode)
		return NULL;

	memset(lpUnicode, 0, sizeof(WCHAR) * (nLen + 1));
	nLen = ::MultiByteToWideChar(CP_ACP, 0, lpszStr, -1, lpUnicode, nLen);
	if (0 == nLen)
	{
		delete[]lpUnicode;
		return NULL;
	}

	nLen = ::WideCharToMultiByte(CP_UTF8, 0, lpUnicode, -1, NULL, 0, NULL, NULL);
	if (0 == nLen)
	{
		delete[]lpUnicode;
		return NULL;
	}

	lpUtf8 = new CHAR[nLen + 1];
	if (NULL == lpUtf8)
	{
		delete[]lpUnicode;
		return NULL;
	}

	memset(lpUtf8, 0, nLen + 1);
	nLen = ::WideCharToMultiByte(CP_UTF8, 0, lpUnicode, -1, lpUtf8, nLen, NULL, NULL);
	if (0 == nLen)
	{
		delete[]lpUnicode;
		delete[]lpUtf8;
		return NULL;
	}

	delete[]lpUnicode;

	return lpUtf8;
}

CHAR * Utf8ToAnsi(const CHAR * lpszStr)
{
	WCHAR * lpUnicode;
	CHAR * lpAnsi;
	int nLen;

	if (NULL == lpszStr)
		return NULL;

	nLen = ::MultiByteToWideChar(CP_UTF8, 0, lpszStr, -1, NULL, NULL);
	if (0 == nLen)
		return NULL;

	lpUnicode = new WCHAR[nLen + 1];
	if (NULL == lpUnicode)
		return NULL;

	memset(lpUnicode, 0, sizeof(WCHAR) * (nLen + 1));
	nLen = ::MultiByteToWideChar(CP_UTF8, 0, lpszStr, -1, lpUnicode, nLen);
	if (0 == nLen)
	{
		delete[]lpUnicode;
		return NULL;
	}

	nLen = ::WideCharToMultiByte(CP_ACP, 0, lpUnicode, -1, NULL, 0, NULL, NULL);
	if (0 == nLen)
	{
		delete[]lpUnicode;
		return NULL;
	}

	lpAnsi = new CHAR[nLen + 1];
	if (NULL == lpAnsi)
	{
		delete[]lpUnicode;
		return NULL;
	}

	memset(lpAnsi, 0, nLen + 1);
	nLen = ::WideCharToMultiByte(CP_ACP, 0, lpUnicode, -1, lpAnsi, nLen, NULL, NULL);
	if (0 == nLen)
	{
		delete[]lpUnicode;
		delete[]lpAnsi;
		return NULL;
	}

	delete[]lpUnicode;

	return lpAnsi;
}

CHAR * UnicodeToUtf8(const WCHAR * lpszStr)
{
	CHAR * lpUtf8;
	int nLen;

	if (NULL == lpszStr)
		return NULL;

	nLen = ::WideCharToMultiByte(CP_UTF8, 0, lpszStr, -1, NULL, 0, NULL, NULL);
	if (0 == nLen)
		return NULL;

	lpUtf8 = new CHAR[nLen + 1];
	if (NULL == lpUtf8)
		return NULL;

	memset(lpUtf8, 0, nLen + 1);
	nLen = ::WideCharToMultiByte(CP_UTF8, 0, lpszStr, -1, lpUtf8, nLen, NULL, NULL);
	if (0 == nLen)
	{
		delete[]lpUtf8;
		return NULL;
	}

	return lpUtf8;
}

WCHAR * Utf8ToUnicode(const CHAR * lpszStr)
{
	WCHAR * lpUnicode;
	int nLen;

	if (NULL == lpszStr)
		return NULL;

	nLen = ::MultiByteToWideChar(CP_UTF8, 0, lpszStr, -1, NULL, 0);
	if (0 == nLen)
		return NULL;

	lpUnicode = new WCHAR[nLen + 1];
	if (NULL == lpUnicode)
		return NULL;

	memset(lpUnicode, 0, sizeof(WCHAR) * (nLen + 1));
	nLen = ::MultiByteToWideChar(CP_UTF8, 0, lpszStr, -1, lpUnicode, nLen);
	if (0 == nLen)
	{
		delete[]lpUnicode;
		return NULL;
	}

	return lpUnicode;
}

BOOL AnsiToUnicode(const CHAR * lpszAnsi, WCHAR * lpszUnicode, int nLen)
{
	int nRet = ::MultiByteToWideChar(CP_ACP, 0, lpszAnsi, -1, lpszUnicode, nLen);
	return (0 == nRet) ? FALSE : TRUE;
}

BOOL UnicodeToAnsi(const WCHAR * lpszUnicode, CHAR * lpszAnsi, int nLen)
{
	int nRet = ::WideCharToMultiByte(CP_ACP, 0, lpszUnicode, -1, lpszAnsi, nLen, NULL, NULL);
	return (0 == nRet) ? FALSE : TRUE;
}

BOOL AnsiToUtf8(const CHAR * lpszAnsi, CHAR * lpszUtf8, int nLen)
{
	WCHAR * lpszUnicode = AnsiToUnicode(lpszAnsi);
	if (NULL == lpszUnicode)
		return FALSE;

	int nRet = UnicodeToUtf8(lpszUnicode, lpszUtf8, nLen);

	delete[]lpszUnicode;

	return (0 == nRet) ? FALSE : TRUE;
}

BOOL Utf8ToAnsi(const CHAR * lpszUtf8, CHAR * lpszAnsi, int nLen)
{
	WCHAR * lpszUnicode = Utf8ToUnicode(lpszUtf8);
	if (NULL == lpszUnicode)
		return FALSE;

	int nRet = UnicodeToAnsi(lpszUnicode, lpszAnsi, nLen);

	delete[]lpszUnicode;

	return (0 == nRet) ? FALSE : TRUE;
}

BOOL UnicodeToUtf8(const WCHAR * lpszUnicode, CHAR * lpszUtf8, int nLen)
{
	int nRet = ::WideCharToMultiByte(CP_UTF8, 0, lpszUnicode, -1, lpszUtf8, nLen, NULL, NULL);
	return (0 == nRet) ? FALSE : TRUE;
}

BOOL Utf8ToUnicode(const CHAR * lpszUtf8, WCHAR * lpszUnicode, int nLen)
{
	int nRet = ::MultiByteToWideChar(CP_UTF8, 0, lpszUtf8, -1, lpszUnicode, nLen);
	return (0 == nRet) ? FALSE : TRUE;
}

std::wstring AnsiToUnicode(const std::string& strAnsi)
{
	std::wstring strUnicode;

	WCHAR * lpszUnicode = AnsiToUnicode(strAnsi.c_str());
	if (lpszUnicode != NULL)
	{
		strUnicode = lpszUnicode;
		delete[]lpszUnicode;
	}

	return strUnicode;
}
std::string UnicodeToAnsi(const std::wstring& strUnicode)
{
	std::string strAnsi;

	CHAR * lpszAnsi = UnicodeToAnsi(strUnicode.c_str());
	if (lpszAnsi != NULL)
	{
		strAnsi = lpszAnsi;
		delete[]lpszAnsi;
	}

	return strAnsi;
}

std::string AnsiToUtf8(const std::string& strAnsi)
{
	std::string strUtf8;

	CHAR * lpszUtf8 = AnsiToUtf8(strAnsi.c_str());
	if (lpszUtf8 != NULL)
	{
		strUtf8 = lpszUtf8;
		delete[]lpszUtf8;
	}

	return strUtf8;
}

std::string Utf8ToAnsi(const std::string& strUtf8)
{
	std::string strAnsi;

	CHAR * lpszAnsi = Utf8ToAnsi(strUtf8.c_str());
	if (lpszAnsi != NULL)
	{
		strAnsi = lpszAnsi;
		delete[]lpszAnsi;
	}

	return strAnsi;
}

std::string UnicodeToUtf8(const std::wstring& strUnicode)
{
	std::string strUtf8;

	CHAR * lpszUtf8 = UnicodeToUtf8(strUnicode.c_str());
	if (lpszUtf8 != NULL)
	{
		strUtf8 = lpszUtf8;
		delete[]lpszUtf8;
	}

	return strUtf8;
}

std::wstring Utf8ToUnicode(const std::string& strUtf8)
{
	std::wstring strUnicode;

	WCHAR * lpszUnicode = Utf8ToUnicode(strUtf8.c_str());
	if (lpszUnicode != NULL)
	{
		strUnicode = lpszUnicode;
		delete[]lpszUnicode;
	}

	return strUnicode;
}


bool Trim(std::string& str, bool bTrimAll/* = true*/)
{
	if (str.empty())
		return false;

	std::string tmp;
	char c;
	for (size_t i = 0; i < str.length(); ++i)
	{
		c = str[i];
		if (c != ' ')
		{
			tmp = str.substr(i);
			break;
		}
	}

	str = tmp;
	if (bTrimAll)
	{
		for (size_t i = str.length() - 1; i >= 0; ++i)
		{
			c = str[i];
			if (c != ' ')
			{
				tmp = str.substr(0, i);
				break;
			}
		}
	}

	str = tmp;

	return true;
}

bool split(const std::string& str, const std::string& demiliter, std::vector<std::string>& result)
{
	std::string tmp = str;
	while (true)
	{
		size_t n = tmp.find(demiliter);
		if (n == std::string::npos)
		{
			result.push_back(tmp);
			break;
		}

		result.push_back(tmp.substr(0, n));

		tmp.erase(0, n + 1);
	}

	return true;
}