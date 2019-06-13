#pragma once
#ifndef _HTTPCLIENTSOCKET_H_
#define _HTTPCLIENTSOCKET_H_
//Ïê¼û
//https://blog.csdn.net/xs574924427/article/details/17240793
#include <vector>
#include <string>
#include<time.h>
#include<string>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/conf.h>
#include <openssl/engine.h>
#include <openssl/evp.h>
#include <openssl/md5.h>
#include <openssl/aes.h>
#include <openssl/bio.h>  
#include <openssl/buffer.h> 

using namespace std;

class httpclientsocket
{
public:
	httpclientsocket();
	virtual ~httpclientsocket();
	bool Connect(const char* pIP, int port, int timeout = 5, bool isSSL_ = false);
	bool IsConnect() { return m_isConnect; }
	void Close();
	int SendData(const char *inbuf, size_t inbuflen);
	int RecvData(string& outbuf, int& pkglen);
	bool GetData(string& xml, size_t& xmllen, const char *buf, size_t buflen, bool getsessionid = false);
	string getsessionid() { return m_cookie; }
	void setcookie(const string& cookie) { m_cookie = cookie; }

	static void InitSSL();
	static void UnInitSSL();
private:
	bool GetHttpHeadField(string& head, const char* fieldname, string& fieldval);
	//int Decompress(const char * src, int srcLen, char * dst, int dstLen);
	int GetBufLen(const char* buf, size_t leftbuf);
	int m_fd;
	int m_timeout;
	bool m_isConnect;
	bool isSSL;
	SSL *ssl;
	static SSL_CTX *ctx;
	string	m_cookie;
};
#endif // !_HTTPCLIENTSOCKET_H_
