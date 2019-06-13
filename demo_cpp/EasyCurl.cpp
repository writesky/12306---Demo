#include"stdafx.h"
#include"EasyCurl.h"
#include<iostream>
#include<curl/curl.h>

#pragma comment(lib,"libcurl.lib")

EasyCurl::EasyCurl(void) :m_bDebug(false)
{

}

EasyCurl::~EasyCurl(void)
{

}
static int OnDebug(CURL *, curl_infotype itype, char * pData, size_t size, void *)
{
	if (itype == CURLINFO_TEXT)
	{
		printf("[TEXT]%s\n", pData);
	}
	else if (itype == CURLINFO_HEADER_IN)
	{
		printf("[HEADER_IN]%s\n", pData);
	}
	else if (itype == CURLINFO_HEADER_OUT)
	{
		printf("[HEADER_OUT]%s\n", pData);
	}
	else if (itype == CURLINFO_DATA_IN)
	{
		printf("[DATA_IN]%s\n", pData);
	}
	else if (itype == CURLINFO_DATA_OUT)
	{
		printf("[DATA_OUT]%s\n", pData);
	}
	return 0;
}

static size_t OnWriteData(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
	string * str = dynamic_cast<string*>((string*)lpVoid);
	if (NULL == str || NULL == buffer)
	{
		return -1;
	}

	char* pData = (char*)buffer;
	str->append(pData, size*nmemb);
	return nmemb;
}
/*  libcurl write callback function */
size_t write_data(void* ptr, size_t size, size_t nmemb, FILE* stream)
{
	size_t written = fwrite(ptr, size, nmemb, stream);
	return written;
	//在这里可以把下载到的数据以追加的方式写入文件
	//FILE* fp = NULL;
	//fopen_s(&fp, "c:\\test.dat", "ab+");//一定要有a, 否则前面写入的内容就会被覆盖了
	//size_t nWrite = fwrite(ptr, nSize, nmemb, fp);
	//fclose(fp);
	//return nWrite;
}

int EasyCurl::progress_callback(void *pParam, double dltotal, double dlnow, double ultotal, double ulnow)
{
	EasyCurl* pThis = (EasyCurl*)pParam;
	int nPos = (int)((dlnow / dltotal) * 100);
	//
	if (pThis->m_pHttpCallback)
	{
		pThis->m_pHttpCallback->OnProgressCallback(nPos);
	}
	if (pThis->m_updateProgress)
	{
		pThis->m_updateProgress(nPos);
	}
	return 0;
}
int EasyCurl::http_post(const string & strUrl, const string & strParam, string & strResponse, const char * pCaPath)
{
	CURLcode res;
	CURL* curl = curl_easy_init();
	if (NULL == curl)
	{
		return CURLE_FAILED_INIT;
	}
	if (m_bDebug)
	{
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
	}
	curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strParam.c_str());
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	if (NULL == pCaPath)
	{
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
	}
	else
	{
		//缺省情况就是PEM，所以无需设置，另外支持DER
		//curl_easy_setopt(curl,CURLOPT_SSLCERTTYPE,"PEM");
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, true);
		curl_easy_setopt(curl, CURLOPT_CAINFO, pCaPath);
	}
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	return res;
}
bool EasyCurl::http_get(const string& strUrl, const string& strHttpHeader, string& strResponse, bool bReserveHeaders, const char * pCaPath)
{
	CURLcode res;
	CURL* curl = curl_easy_init();
	if (NULL == curl)
	{
		return false;
	}
	if (m_bDebug)
	{
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
	}
	curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
	if (bReserveHeaders)
		curl_easy_setopt(curl, CURLOPT_HEADER, 1);
	curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	if (NULL == pCaPath)
	{
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);//设定为不验证证书和HOST
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
	}
	else
	{
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, true);
		curl_easy_setopt(curl, CURLOPT_CAINFO, pCaPath);
	}
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
	res = curl_easy_perform(curl);
	bool bError = false;
	if (res != CURLE_OK)
		bError = true;

	int code;
	res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
	if (code != 200)
		bError = true;

	curl_easy_cleanup(curl);

	return !bError;
}

int EasyCurl::download_file(const string & strUrl, const string & strFile)
{
	FILE *fp;
	//调用curl_easy_init()函数得到 easy interface型指针
	CURL *curl = curl_easy_init();
	if (curl)
	{
		fopen_s(&fp, strFile.c_str(), "wb");

		CURLcode res = curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
		if (res != CURLE_OK)
		{
			fclose(fp);
			curl_easy_cleanup(curl);
			return -1;
		}

		res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		if (res != CURLE_OK)
		{
			fclose(fp);
			curl_easy_cleanup(curl);
			return -1;
		}

		res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		if (res != CURLE_OK)
		{
			fclose(fp);
			curl_easy_cleanup(curl);
			return -1;
		}
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
		curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress_callback);//设置进度回调函数
		curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, this);
		//开始执行请求
		res = curl_easy_perform(curl);
		fclose(fp);
		/* Check for errors */
		if (res != CURLE_OK)
		{
			curl_easy_cleanup(curl);
			return -1;
		}
		curl_easy_cleanup(curl);//调用curl_easy_cleanup()释放内存
	}

	return 0;
}
//
void EasyCurl::set_progress_function(ProgressFunction func)
{
	m_updateProgress = func;
}
//
void EasyCurl::set_progress_callback(IProgressCallback *pCallback)
{
	m_pHttpCallback = pCallback;
}
//
void EasyCurl::SetDebug(bool bDebug)
{
	m_bDebug = bDebug;
}