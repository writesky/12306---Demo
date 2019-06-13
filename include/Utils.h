#pragma once
#include<string>
#include<vector>

WCHAR * AnsiToUnicode(const CHAR * lpszStr);
CHAR * UnicodeToAnsi(const WCHAR * lpszStr);
CHAR * AnsiToUtf8(const CHAR * lpszStr);
CHAR * Utf8ToAnsi(const CHAR * lpszStr);
CHAR * UnicodeToUtf8(const WCHAR * lpszStr);
WCHAR * Utf8ToUnicode(const CHAR * lpszStr);

BOOL AnsiToUnicode(const CHAR * lpszAnsi, WCHAR * lpszUnicode, int nLen);
BOOL UnicodeToAnsi(const WCHAR * lpszUnicode, CHAR * lpszAnsi, int nLen);
BOOL AnsiToUtf8(const CHAR * lpszAnsi, CHAR * lpszUtf8, int nLen);
BOOL Utf8ToAnsi(const CHAR * lpszUtf8, CHAR * lpszAnsi, int nLen);
BOOL UnicodeToUtf8(const WCHAR * lpszUnicode, CHAR * lpszUtf8, int nLen);
BOOL Utf8ToUnicode(const CHAR * lpszUtf8, WCHAR * lpszUnicode, int nLen);

std::wstring AnsiToUnicode(const std::string& strAnsi);
std::string UnicodeToAnsi(const std::wstring& strUnicode);
std::string AnsiToUtf8(const std::string& strAnsi);
std::string Utf8ToAnsi(const std::string& strUtf8);
std::string UnicodeToUtf8(const std::wstring& strUnicode);
std::wstring Utf8ToUnicode(const std::string& strUtf8);

/**
 * @param: bTrimAll 是否全部修剪，如果为false则只修剪左边的空白，默认修剪左边和右边全部空白
 */

bool Trim(std::string& str, bool bTrimAll = true);

bool split(const std::string& str, const std::string& demiliter, std::vector<std::string>& result);