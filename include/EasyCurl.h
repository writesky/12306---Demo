#pragma once
/**
 * see: http://blog.csdn.net/hellokandy/article/details/51264225
 * see: http://blog.csdn.net/hellokandy/article/details/53911448
 */

#ifndef _BASE_EASY_CURL_H_
#define _BASE_EASY_CURL_H_

#include<Windows.h>
#include<string>
#include<functional>
using std::string;

class IProgressCallback
{
public:
	virtual bool OnProgressCallback(int nValue) = 0;
};

class EasyCurl
{
	EasyCurl(void);
	~EasyCurl(void);
	typedef std::tr1::function<void(int)> ProgressFunction;

public:
	/// @brief		HTTP POST����
	/// @param[in]	strUrl �������,�����Url��ַ,��:https://www.baidu.com
	/// @param[in]	strParam �������,ʹ�ø�ʽ"name=kandy&pwd=1234"
	/// @param[out]	strResponse �������,���ص�����
	/// @param[in]	pCaPath �������,ΪCA֤���·��.�������ΪNULL,����֤��������֤�����Ч��.
	///	@remark		�����Ƿ�Post�ɹ�
	///	@return		CURLE_OK,�ɹ�!����ʧ��
	int http_post(const string& strUrl, const string& strParam, string& strResponse, const char* pCaPath = NULL);

	/// @brief		HTTPS GET����
	/// @param[in]	strUrl �������,�����Url��ַ,��:https://www.baidu.com
	/// @param[in]	strHttpHeader �������,���ӵ�http�����е�http header��Ϣ
	/// @param[out]	strResponse �������,���ص�����
	/// @param[in]	bReserveHeaders �������,�Ƿ���httpͷ����Ϣ,Ĭ�ϲ�����
	/// @param[in]	pCaPath �������,ΪCA֤���·��.�������ΪNULL,����֤��������֤�����Ч��.
	///	@remark		�����Ƿ�Post�ɹ�
	///	@return		CURLE_OK,�ɹ�!����ʧ��
	bool http_get(const string& strUrl, const string& strHttpHeader, string& strResponse, bool bReserveHeaders = false, const char * pCaPath = NULL);

	/// @brief		�ļ�����
	// @param[in]	url : Ҫ�����ļ���url��ַ
	// @param[in]	outfilename : �����ļ�ָ�����ļ���
	// @remark		
	// @return		����0����ɹ�
	int download_file(const string & strUrl, const string & strFile);

	/// @brief		���ȱ��洦��
	/// @param[in]	func : ������ַ
	///	@remark		
	///	@return		void
	void set_progress_function(ProgressFunction func);

	/// @brief		���ȱ��洦��
	/// @param[in]	pCallback : ����Ķ���
	///	@remark		ʹ�õ���̳���IProgressCallback
	///	@return		void
	void set_progress_callback(IProgressCallback *pCallback);
	//	
public:
	void SetDebug(bool bDebug);

protected:
	static int progress_callback(void *pParam, double dltotal, double dlnow, double ultotal, double ulnow);
private:
	bool m_bDebug;
	ProgressFunction	m_updateProgress;
	IProgressCallback	*m_pHttpCallback;
};
#endif // !_BASE_EASY_CURL_H_
