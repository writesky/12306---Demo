#include "stdafx.h"
//#include <zlib.h>
#include "httpclientsocket_ssl.h"

#pragma comment(lib,"Ws2_32.lib")
#pragma comment(lib, "libssl.lib")
#pragma  comment(lib, "libcrypto.lib")
//#pragma comment(lib , "zlib.lib")

SSL_CTX* httpclientsocket::ctx = NULL;

httpclientsocket::httpclientsocket() :m_fd(-1), m_timeout(10), m_isConnect(false), isSSL(false), ssl(NULL)
{

}

httpclientsocket::~httpclientsocket()
{
	Close();
}

void httpclientsocket::Close()
{
	if (m_fd != -1)
	{
		if (isSSL == true)
		{
			if (ssl != NULL)
			{
				SSL_shutdown(ssl);
				SSL_free(ssl);
				ssl = NULL;
			}
		}
		closesocket(m_fd);
		m_isConnect = false;
	}
	m_fd = -1;
};

void httpclientsocket::InitSSL()
{
	if (ctx == NULL)
	{
		SSL_library_init();
		OpenSSL_add_all_algorithms();
		SSL_load_error_strings();
		ctx = SSL_CTX_new(SSLv23_client_method());
	}
};
void httpclientsocket::UnInitSSL()
{
	if (ctx != NULL)
	{
		SSL_CTX_free(ctx);
		ctx = NULL;
		ERR_remove_state(0);
		ERR_free_strings();
		EVP_cleanup();
	}
}

int httpclientsocket::GetBufLen(const char* buf, size_t leftbuf)
{
	char* p2 = strstr((char*)buf, "\r\n\r\n");//查找包头是否完整
	if (p2 == NULL)
		return leftbuf + 1024;//包头都不完整，则再接着收吧
	if (p2 - buf + 4 > (int)leftbuf)
		return leftbuf + 1024;//包头都不完整，则再接着收吧
	string head(buf, p2 - buf + 4);
	string::size_type xmllenpos = head.find("Content-Length:", 0);
	if (xmllenpos == string::npos)
	{
		xmllenpos = head.find("Transfer-Encoding: chunked", 0);
		if (xmllenpos == string::npos)
			return -1;//不支持其他表示长度的格式            
		char* pos_start = p2 + 4;//数据起始地址
		char* pos_end = strstr((char*)pos_start, "\r\n");
		size_t len = head.size();
		while (pos_end != NULL)
		{
			if (pos_end - buf > (int)leftbuf)
				return leftbuf + 1024;
			size_t piece_headlen = (pos_end - pos_start + 2);
			size_t piece_datalen = strtol(pos_start, NULL, 16);
			size_t piecelen = piece_headlen + piece_datalen + 2;//计算该数据片的长度
			len += piecelen;//计算完整包总长度
			if (piece_datalen == 0)//该数据片长度为0
				return len;//数据包已经完整了
			if (len >= leftbuf)
				return len + 1024;//数据包还没完整
			pos_start += piecelen;
			pos_end = strstr((char*)pos_start, "\r\n");
		}
		return leftbuf + 1024;
	}
	else
	{
		long xmllen = atol(&head[xmllenpos + strlen("Content-Length:")]);
		return (xmllen + head.size());
	}
	return -1;//不支持其他表示长度的格式
}

int httpclientsocket::RecvData(string& outbuf, int& pkglen)
{
	if (m_fd == -1)
		return -1;
	pkglen = 0;
	char buf[4096] = { 0 };
	time_t tstart = time(NULL);
	int ret = 0;
	while (true)
	{
		if (isSSL == false)
			ret = ::recv(m_fd, buf, 4096, 0);
		else
			ret = SSL_read(ssl, buf, 4096);
		if (ret == 0)
		{
			Close();
			break;//对方关闭socket了
		}
		else if (ret < 0)
		{
			//int n = SSL_get_error(ssl, ret);
			//DWORD dwError = WSAGetLastError();
			if ((isSSL == false && WSAGetLastError() == EWOULDBLOCK) || (isSSL == true && (SSL_get_error(ssl, ret) == SSL_ERROR_WANT_READ)))
			{
				if (time(NULL) - tstart > m_timeout)
				{
					Close();
					break;
				}
				else
					break;
			}
			else
			{
				if (WSAGetLastError() != 0 && WSAGetLastError() != EWOULDBLOCK)
				{
					Close();
					break;//接收出错
				}
				else
					continue;
			}
		}
		outbuf.append(buf, ret);
	}

	pkglen = outbuf.length();
	return pkglen;
}

int httpclientsocket::SendData(const char* inbuf, size_t inbuflen)
{
	OutputDebugStringA("Send data:\n");
	OutputDebugStringA(inbuf);

	if (m_fd == -1)
		return -1;
	int ret = 0;
	size_t sended = 0;
	time_t tstart = time(NULL);
	while (true)
	{
		if (isSSL == false)
			ret = ::send(m_fd, inbuf + sended, inbuflen - sended, 0);
		else
			ret = SSL_write(ssl, inbuf + sended, inbuflen - sended);
		if (ret == 0)
		{
			Close();
			return 0;//对方关闭socket了
		}
		else if (ret < 0)
		{
			if ((isSSL == false && (WSAGetLastError() == EWOULDBLOCK)) || (isSSL == true && (SSL_get_error(ssl, ret) == SSL_ERROR_WANT_WRITE)))
			{
				if (time(NULL) - tstart > m_timeout)
				{
					Close();
					return 0;
				}
				else
					continue;//继续发送
			}
			else
			{
				Close();
				return ret;//发送出错
			}
		}
		sended += ret;
		if (inbuflen == sended)
			break;//全部发送完了
	}
	return sended;
}

void ShowCA(SSL * ssl)
{
	X509 *cert;
	char *line;

	cert = SSL_get_peer_certificate(ssl);
	if (cert != NULL) {
		line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
		//free(line);
		line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
		//free(line);
		X509_free(cert);
	}
	else
		printf("无证书信息！\n");
}

bool httpclientsocket::Connect(const char* pIp, int iPort, int timeout, bool isSSL_)
{
	Close();
	m_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (m_fd == -1)
		return false;
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(iPort);
	server_addr.sin_addr.s_addr = inet_addr(pIp);
	if (server_addr.sin_addr.s_addr == INADDR_NONE)
	{//解析域名
		hostent* myhost = gethostbyname(pIp);
		if (myhost != NULL)
		{
			for(char** pp = myhost->h_aliases;*pp != NULL;pp++)
			{
				char** pp = myhost->h_addr_list;
				while (*pp != NULL)
				{
					server_addr.sin_addr.s_addr = *((unsigned int *)*pp);    //只取第一个ip地址
					pp++;
					break;
				}
			}
		}
	}
	int ret = connect(m_fd, (struct sockaddr *)(&server_addr), sizeof(struct sockaddr));
	if (ret < 0)
	{
		Close();
		return false;
	}
	m_isConnect = true;
	m_timeout = timeout;
	struct timeval tv = { m_timeout , 0 };
	if ((setsockopt(m_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv)) == -1) || (setsockopt(m_fd, SOL_SOCKET, SO_SNDTIMEO, (const char *)&tv, sizeof(tv)) == -1))
	{
		Close();
		return false;
	}

	isSSL = isSSL_;
	if (isSSL_ == true)
	{
		ssl = SSL_new(ctx);
		if (ssl == NULL)
		{
			Close();
			return false;
		}
		if (SSL_set_fd(ssl, m_fd) <= 0)
		{
			Close();
			return false;
		}
		int ret = SSL_connect(ssl);
		if (ret <= 0) {

			int error = SSL_get_error(ssl, ret);
			ERR_print_errors_fp(stderr);
			SSL_state_string_long(ssl);
			Close();
			return false;
		}
		ShowCA(ssl);
	}

	return true;
}

bool httpclientsocket::GetHttpHeadField(string& head, const char* fieldname, string& fieldval)
{
	string::size_type xmllenpos = head.find(fieldname, 0);
	if (xmllenpos == string::npos)
		return false;
	char* pval = (char*)(head.data() + xmllenpos + strlen(fieldname));
	char* p1 = strstr(pval, "\r\n");
	if (p1 == NULL)
		return false;
	fieldval.assign(pval, p1 - pval);
	return true;
}

bool httpclientsocket::GetData(string& xml, size_t& xmllen, const char* buf, size_t buflen, bool getsessionid/* = false*/)
{
	//解析http头部
	string str;
	str.append(buf, buflen);
	size_t n = str.find("\r\n\r\n");
	string header = str.substr(0, n);
	str.erase(0, n + 4);

	string TransferEncoding;
	string ContentLength;
	if (true)
	{
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
		size_t m, n, t;
		bool bChunked = false;
		OutputDebugStringA("response http headers:\n");
		for (size_t i = 0; i < v.size(); i++)
		{
			OutputDebugStringA(v[i].c_str());
			OutputDebugStringA("\n");
			m = v[i].find("Set-Cookie");
			if(m != string::npos)
			{
				string tmp = / v[i].substr(11);
				m = tmp.find("JSESSIONID");
				if (m != string::npos)
				{
					size_t comma = tmp.find(";");
					if (comma != string::npos)
						jsessionid = tmp.substr(12, comma - 12);
				}

				m = tmp.find("BIGipServerotn");
				if (m != string::npos)
				{
					BIGipServerotn = tmp.substr(m, 36);
				}

				m = tmp.find("BIGipServerportal");
				if (m != string::npos)
				{
					BIGipServerportal = tmp.substr(m, 39);
				}

				m = tmp.find("current_captcha_type");
				if (m != string::npos)
				{
					current_captcha_type = tmp.substr(m, 22);
				}
			}

			n = v[i].find("Transfer-Encoding");
			if (n != string::npos)
			{
				TransferEncoding = v[i].substr(n + 18);
			}
			else
			{
				n = v[i].find("transfer-encoding");
				if (n != string::npos)
				{
					TransferEncoding = v[i].substr(n + 18);
				}
			}

			t = v[i].find("Content-Length");
			if (t != string::npos)
			{
				ContentLength = v[i].substr(t + 15);
			}
			else
			{
				t = v[i].find("content-length");
				if (t != string::npos)
				{
					ContentLength = v[i].substr(t + 15);
				}
			}
		}

		if (!jsessionid.empty())
		{
			m_cookie = jsessionid;
			m_cookie += ";";
			m_cookie += BIGipServerotn;
			if (!BIGipServerportal.empty())
			{
				m_cookie += "; ";
				m_cookie += BIGipServerportal;
			}
			m_cookie += "; ";
			m_cookie += current_captcha_type;
		}
	}
	//解析http body
	int nContentLength = 0;
	if (!ContentLength.empty())
	{
		nContentLength = atol(ContentLength.c_str());
		xml.append(str.c_str(), nContentLength);
		xmllen = nContentLength;
		return true;
	}
	if (!TransferEncoding.empty() && TransferEncoding.find("chunked") != string::npos)
	{
		vector<string> v;
		while (true)
		{
			size_t index = str.find("\r\n");
			if (index == string::npos)
				break;
			string tmp = str.substr(0, index);
			v.push_back(tmp);
			str.erase(0, index + 2);

			if (str.empty())
				break;
		}
		//从0开始，奇数项为chunked主体内容
		for (size_t i = 0; i < v.size(); ++i)
		{
			if (i % 2 == 0)
				continue;

			xml.append(v[i].c_str(), v[i].length());
			xmllen += v[i].length();
		}

		return true;
	}
	return false;
}