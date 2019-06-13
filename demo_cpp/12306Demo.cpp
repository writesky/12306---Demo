/**
 * 12306ˢƱ����
 */

#include "stdafx.h"
#include <GdiPlus.h>
#include <time.h>
#include <vector>
#include <sstream>
#include <ocidl.h> 
#include <olectl.h>
#include "Log.h"
#include "Utils.h"
#include "AppInfo.h"
#include "NetThread.h"
#include "Client12306.h"
#include "Task.h"
#include "UserInfo.h"
#include "12306Demo.h"

#pragma comment(lib, "Comctl32.lib")
#pragma comment(lib, "gdiplus.lib")


using namespace std;

#define ID_PASSENGERBTN_BASE 0x1000

//��֤��ͼƬ���������������ʾ�ļ�֮��ļ�϶����λ�����أ�
#define VCODE_SLICE_GAP      40

//��֤��״̬
enum VCODE_STATUS
{
	VCODE_STATUS_LOADING,       //������
	VCODE_STATUS_LOADED,        //�������
	VCODE_STATUS_LOADFAILED,    //���س���
	VCODE_STATUS_LOGINING       //��¼��
};

//ˢ����֤�� ��¼״̬�µ���֤�봫�롱randp�����ǵ�¼���롱sjrand�� ����ο�ԭotsweb�еĴ������
struct VCodePosition
{
	int x;
	int y;
};

const VCodePosition g_pos[] =
{
	{ 39, 40 },
	{ 114, 43 },
	{ 186, 42 },
	{ 252, 47 },
	{ 36, 120 },
	{ 115, 125 },
	{ 194, 125 },
	{ 256, 120 }
};

//��֤��ͼƬ�˸������λ��
struct VCODE_SLICE_POS
{
	int xLeft;
	int xRight;
	int yTop;
	int yBottom;
};

const VCODE_SLICE_POS g_VCodeSlicePos[] =
{
	{0,   70,  0,  70},
	{71,  140, 0,  70 },
	{141, 210, 0,  70 },
	{211, 280, 0,  70 },
	{ 0,  70,  70, 140 },
	{71,  140, 70, 140 },
	{141, 210, 70, 140 },
	{211, 280, 70, 140 }
};

//8����֤������������״̬
bool g_bVodeSlice1Pressed[8] = { false, false, false, false, false, false, false, false };

#ifdef _DEBUG
TCHAR* g_szTicketTitle[] = { L"0����", L"1����վ->����վ", L"2����ʱ��-����ʱ��",
							L"3��ʱ", L"4������", L"5�ص���", L"6һ����", L"7������", L"8�߼�����",
							L"9����", L"10Ӳ��", L"11����", L"12Ӳ��", L"13����",
							L"14����", L"15��ע" };
#else
TCHAR* g_szTicketTitle[] = { L"����", L"����վ->����վ", L"����ʱ��-����ʱ��",
							L"��ʱ", L"������", L"�ص���", L"һ����", L"������", L"�߼�����",
							L"����", L"Ӳ��", L"����", L"Ӳ��", L"����",
							L"����", L"��ע" };
#endif
// Global Variables:
HINSTANCE g_hInst = NULL;

//�����˳��ź�
HANDLE  g_hExitEvent = NULL;
//���������߳�
HANDLE  g_hNetThread = NULL;

HWND g_hMainDlg = NULL;

VCODE_STATUS g_nVCodeStatus = VCODE_STATUS_LOADING;

//����Ʊ����ѡ��
int g_nTicketsFilterIndex = 0;

//��������
int g_nTrainType = TRAIN_ALL;

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	srand((int)time(NULL));
	g_hInst = hInstance;

	INITCOMMONCONTROLSEX ic = { 0 };
	ic.dwSize = sizeof(ic);
	ic.dwICC = ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&ic);

	ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	//WORD wVersionRequested = MAKEWORD(2, 2);
	//WSADATA wsaData;
	//int nErrorID = ::WSAStartup(wVersionRequested, &wsaData);
	//if (nErrorID != 0)
	//    return -1;

	//if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wHighVersion) != 2)
	//{
	//    WSACleanup();
	//    return -1;
	//}

	ClearExpiredLog();

	g_hExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	NetThread::GetInstance().Init(4);

	INT_PTR ret = DialogBox(hInstance, MAKEINTRESOURCE(IDD_LOGIN), NULL, LoginDlgProc);
	if (ret == IDOK || ret == IDC_VISITORLOGIN)
	{
		DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, MainDlgProc);
	}

	//�����˳������ź�
	SetEvent(g_hExitEvent);
	NetThread::GetInstance().Uninit();

	CloseHandle(g_hExitEvent);

	//WSACleanup();
	Gdiplus::GdiplusShutdown(gdiplusToken);

	return 0;
}

void ClearExpiredLog()
{
	TCHAR szPath[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szPath, MAX_PATH);
	for (int i = _tcslen(szPath) - 1; i >= 0; --i)
	{
		if (szPath[i] == L'\\')
		{
			szPath[i] = 0;
			break;
		}
	}

	int clearrunlog = GetPrivateProfileInt(L"app", L"clearrunlog", 0, L"./cfg.ini");

	WIN32_FIND_DATA data = { 0 };
	HANDLE hFindFile;
	if (clearrunlog != 0)
	{
		//ɾ��log�ļ�
		wstring strLogFileName = szPath;
		strLogFileName += L"\\*.log";

		hFindFile = FindFirstFile(strLogFileName.c_str(), &data);
		if (hFindFile != INVALID_HANDLE_VALUE)
		{
			BOOL bRet;
			do
			{
				DeleteFile(data.cFileName);
				bRet = FindNextFile(hFindFile, &data);
			} while (bRet);
		}

		FindClose(hFindFile);
	}


	int clearvcode = GetPrivateProfileInt(L"app", L"clearvcode", 0, L"./cfg.ini");
	if (clearvcode != 0)
	{
		//ɾ����֤��ͼƬ�ļ�
		wstring strVCodeFileName = szPath;
#ifdef _DEBUG
		strVCodeFileName += L"\\*.jpg";
#else
		strVCodeFileName += L"\\*.v";
#endif

		hFindFile = FindFirstFile(strVCodeFileName.c_str(), &data);
		if (hFindFile != INVALID_HANDLE_VALUE)
		{
			BOOL bRet;
			do
			{
				DeleteFile(data.cFileName);
				bRet = FindNextFile(hFindFile, &data);
			} while (bRet);
		}

		FindClose(hFindFile);
	}
}

// Message handler for LoginDlg box.
INT_PTR CALLBACK LoginDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	static HWND hVCode;
	static HWND hLoginButton;
	//��֤��ؼ�����
	static RECT rtVCode = { 112, 130, 410, 327 };
	switch (msg)
	{
	case WM_INITDIALOG:
		CenterWindow(hDlg);

#ifdef _DEBUG
		SetWindowTextA(GetDlgItem(hDlg, IDC_USER), "zxf125302");
		SetWindowTextA(GetDlgItem(hDlg, IDC_PASS), "zxf125302");
#endif
		hLoginButton = GetDlgItem(hDlg, IDC_LOGIN);
		EnableWindow(hLoginButton, FALSE);
		hVCode = GetDlgItem(hDlg, IDC_VCODE);
		SetWindowText(hVCode, L"��֤�������...");
		EnableWindow(hVCode, FALSE);

		NetThread::GetInstance().SetReflectionWindow(hDlg);
		NetThread::GetInstance().AddGetVCodeTask();

		return (INT_PTR)TRUE;

	case WM_PAINT:
	{
		PAINTSTRUCT ps = { 0 };
		HDC hdc = BeginPaint(hDlg, &ps);
		//DrawVCodeImage(hVCode, 293, 190, g_pPic);            
		HDC hdcVCodeCtrl = GetWindowDC(hVCode);
		Gdiplus::Graphics g(hdcVCodeCtrl);
		string strVCodeFilename;
		AppInfo::GetInstance().GetLoginVCodeFilename(strVCodeFilename);
		//����ʾ�ļ�����֤��ͼƬ
		if (g_nVCodeStatus == VCODE_STATUS_LOADING)
		{
			Gdiplus::FontFamily fontFamily(_T("΢���ź�"));
			Gdiplus::Font font(&fontFamily, 12);
			RECT rtRectVCode;
			GetWindowRect(hVCode, &rtRectVCode);
			Gdiplus::RectF rectf(0, (rtRectVCode.bottom - rtRectVCode.top) / 2, rtRectVCode.right - rtRectVCode.left, rtRectVCode.bottom - rtRectVCode.top);
			Gdiplus::SolidBrush blackBrush(Gdiplus::Color(255, 0, 0, 0));
			Gdiplus::StringFormat format;
			format.SetAlignment(Gdiplus::StringAlignmentCenter);
			g.DrawString(_T("��֤�������..."), -1, &font, rectf, &format, &blackBrush);
		}
		else if (g_nVCodeStatus == VCODE_STATUS_LOADED)
		{
			wstring strVCodeFilenameW = AnsiToUnicode(strVCodeFilename);
			Gdiplus::Image vcodeImage(strVCodeFilenameW.c_str());
			g.DrawImage(&vcodeImage, 0, 0);

			//TODO: �ĳ��ó���·����ȡ
			Gdiplus::Image markImage(_T("mark.png"));
			//��ѡ����֤����
			for (int i = 0; i < ARRAYSIZE(g_bVodeSlice1Pressed); ++i)
			{
				if (g_bVodeSlice1Pressed[i])
				{
					g.DrawImage(&markImage,
						g_VCodeSlicePos[i].xLeft + (g_VCodeSlicePos[i].xRight - g_VCodeSlicePos[i].xLeft) / 2 - 13,
						g_VCodeSlicePos[i].yTop + VCODE_SLICE_GAP + (g_VCodeSlicePos[i].yBottom - g_VCodeSlicePos[i].yTop) / 2 - 13);
				}
			}
		}
		else if (g_nVCodeStatus == VCODE_STATUS_LOADFAILED)
		{
			Gdiplus::FontFamily fontFamily(_T("΢���ź�"));
			Gdiplus::Font font(&fontFamily, 12);
			RECT rtRectVCode;
			GetWindowRect(hVCode, &rtRectVCode);
			Gdiplus::RectF rectf(0, (rtRectVCode.bottom - rtRectVCode.top) / 2, rtRectVCode.right - rtRectVCode.left, rtRectVCode.bottom - rtRectVCode.top);
			Gdiplus::SolidBrush blackBrush(Gdiplus::Color(255, 255, 0, 0));
			Gdiplus::StringFormat format;
			format.SetAlignment(Gdiplus::StringAlignmentCenter);
			g.DrawString(_T("��֤����س�����������������"), -1, &font, rectf, &format, &blackBrush);
		}
		else if (g_nVCodeStatus == VCODE_STATUS_LOGINING)
		{
			Gdiplus::FontFamily fontFamily(_T("΢���ź�"));
			Gdiplus::Font font(&fontFamily, 12);
			RECT rtRectVCode;
			GetWindowRect(hVCode, &rtRectVCode);
			Gdiplus::RectF rectf(0, (rtRectVCode.bottom - rtRectVCode.top) / 2, rtRectVCode.right - rtRectVCode.left, rtRectVCode.bottom - rtRectVCode.top);
			Gdiplus::SolidBrush blackBrush(Gdiplus::Color(255, 0, 255, 0));
			Gdiplus::StringFormat format;
			format.SetAlignment(Gdiplus::StringAlignmentCenter);
			g.DrawString(_T("���ڵ�¼�����Ե�..."), -1, &font, rectf, &format, &blackBrush);
		}

		EndPaint(hDlg, &ps);
	}
	break;


	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_VISITORLOGIN || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		else if (LOWORD(wParam) == IDC_LOGIN)
		{
			char user[32] = { 0 };
			char pass[32] = { 0 };
			GetWindowTextA(GetDlgItem(hDlg, IDC_USER), user, 32);
			GetWindowTextA(GetDlgItem(hDlg, IDC_PASS), pass, 32);
			if (strlen(user) == 0)
			{
				MessageBox(hDlg, L"�û�������Ϊ��", L"����", MB_OK | MB_ICONERROR);
				return (INT_PTR)FALSE;
			}

			if (strlen(pass) < 6)
			{
				MessageBox(hDlg, L"���볤�Ȳ���С��6λ", L"����", MB_OK | MB_ICONERROR);
				return (INT_PTR)FALSE;
			}

			ostringstream pos;
			for (size_t i = 0; i < ARRAYSIZE(g_bVodeSlice1Pressed); ++i)
			{
				if (g_bVodeSlice1Pressed[i])
				{
					pos << g_pos[i].x << "," << g_pos[i].y;
					if (i != ARRAYSIZE(g_bVodeSlice1Pressed) - 1)
						pos << ",";
				}
			}

			if (pos.str().empty())
			{
				MessageBox(hDlg, L"����ѡ����֤��", L"��ʾ", MB_OK | MB_ICONINFORMATION);
				return (INT_PTR)FALSE;
			}

			EnableWindow(hLoginButton, FALSE);
			CUserInfo::GetInstance().m_username = user;
			CUserInfo::GetInstance().m_password = pass;
			NetThread::GetInstance().AddLoginTask(user, pass, pos.str().c_str());

			g_nVCodeStatus = VCODE_STATUS_LOGINING;
			InvalidateRect(hDlg, &rtVCode, TRUE);
		}
		else if (LOWORD(wParam) == IDC_VCODE && HIWORD(wParam) == STN_DBLCLK)
		{
			g_nVCodeStatus = VCODE_STATUS_LOADING;
			EnableWindow(hVCode, FALSE);
			EnableWindow(hLoginButton, FALSE);
			//NetThread::GetInstance().SetReflectionWindow(hDlg);
			NetThread::GetInstance().AddGetVCodeTask();
			ResetVCodeSliceUnmarked();

			InvalidateRect(hDlg, &rtVCode, TRUE);
		}
		else if (LOWORD(wParam) == IDC_VCODE && HIWORD(wParam) == STN_CLICKED)
		{
			if (g_nVCodeStatus != VCODE_STATUS_LOADED)
				break;
			//��Ļ��������λ��
			POINT ptMousePos;
			GetCursorPos(&ptMousePos);
			//ת�����������֤��ؼ��Ŀͻ������������
			ScreenToClient(hVCode, &ptMousePos);
			for (int i = 0; i < ARRAYSIZE(g_VCodeSlicePos); ++i)
			{
				RECT rt = { g_VCodeSlicePos[i].xLeft, VCODE_SLICE_GAP + g_VCodeSlicePos[i].yTop, g_VCodeSlicePos[i].xRight, VCODE_SLICE_GAP + g_VCodeSlicePos[i].yBottom };
				if (PtInRect(&rt, ptMousePos))
				{
					g_bVodeSlice1Pressed[i] = !g_bVodeSlice1Pressed[i];
					InvalidateRect(hDlg, &rtVCode, TRUE);
					break;
				}
			}
		}

		break;

		//���ص�¼��֤��������
	case WM_LOADVCODE:
	{
		EnableWindow(hVCode, TRUE);
		EnableWindow(hLoginButton, TRUE);
		if (wParam == OPERATION_FAIL)
			g_nVCodeStatus = VCODE_STATUS_LOADFAILED;
		else
			g_nVCodeStatus = VCODE_STATUS_LOADED;

		InvalidateRect(hDlg, &rtVCode, TRUE);
	}

	break;

	//��¼�������
	case WM_LOGIN:
	{
		EnableWindow(hLoginButton, TRUE);
		if (wParam == LOGIN_INCORRECT_VCODE)
		{
			ResetVCodeSliceUnmarked();
			MessageBox(hDlg, L"��֤�벻��ȷ�������ԣ�", L"����", MB_OK | MB_ICONERROR);
			g_nVCodeStatus = VCODE_STATUS_LOADING;
			InvalidateRect(hDlg, &rtVCode, TRUE);
			NetThread::GetInstance().AddGetVCodeTask();

			return (INT_PTR)FALSE;
		}
		else if (wParam == LOGIN_VERIFY_VCODE_ERROR)
		{
			ResetVCodeSliceUnmarked();
			MessageBox(hDlg, L"У����֤�����", L"����", MB_OK | MB_ICONERROR);
			g_nVCodeStatus = VCODE_STATUS_LOADING;
			InvalidateRect(hDlg, &rtVCode, TRUE);
			NetThread::GetInstance().AddGetVCodeTask();
			return (INT_PTR)FALSE;
		}
		else if (wParam == LOGIN_ERROR)
		{
			ResetVCodeSliceUnmarked();
			MessageBox(hDlg, L"��¼��������", L"����", MB_OK | MB_ICONERROR);
			g_nVCodeStatus = VCODE_STATUS_LOADING;
			InvalidateRect(hDlg, &rtVCode, TRUE);
			NetThread::GetInstance().AddGetVCodeTask();
			return (INT_PTR)FALSE;
		}
		else if (wParam == LOGIN_FAIL)
		{
			ResetVCodeSliceUnmarked();
			MessageBox(hDlg, L"��¼ʧ�ܣ�", L"����", MB_OK | MB_ICONERROR);
			g_nVCodeStatus = VCODE_STATUS_LOADING;
			InvalidateRect(hDlg, &rtVCode, TRUE);
			NetThread::GetInstance().AddGetVCodeTask();
			return (INT_PTR)FALSE;
		}

		ResetVCodeSliceUnmarked();
		CUserInfo::GetInstance().m_nLoginStatus = LOGIN_STATUS_LOGINON;
		//��¼�ɹ����رյ�¼�Ի�����ʾ���Ի���
		EndDialog(hDlg, IDOK);

	}
	break;
	}// end switch


	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK MainDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	static HMENU hMainMenu;
	static HWND hTickDetailList;
	static HWND hFromStation;
	static HWND hToStation;
	static HWND hSetoutDate;
	static HWND hTicketType1;
	static HWND hTicketType2;
	static HWND hTicketsFilter;

	static HWND hCheckGC;
	static HWND hCheckD;
	static HWND hCheckZ;
	static HWND hCheckT;
	static HWND hCheckK;
	static HWND hCheckOTHER;
	static HWND hStatusInfo;
	switch (msg)
	{
	case WM_INITDIALOG:
	{
		g_hMainDlg = hDlg;
		hMainMenu = LoadMenu(NULL, MAKEINTRESOURCE(IDM_MAIN));
		SetMenu(hDlg, hMainMenu);

		hTickDetailList = GetDlgItem(hDlg, IDC_TICKDETAIL);
		InitTicketDetailList(hTickDetailList);

		hFromStation = GetDlgItem(hDlg, IDC_FROMSTATION);
		hToStation = GetDlgItem(hDlg, IDC_TOSTATION);

		hCheckGC = GetDlgItem(hDlg, IDC_CHECKGC);
		hCheckD = GetDlgItem(hDlg, IDC_CHECKD);
		hCheckZ = GetDlgItem(hDlg, IDC_CHECKZ);
		hCheckT = GetDlgItem(hDlg, IDC_CHECKT);
		hCheckK = GetDlgItem(hDlg, IDC_CHECKK);
		hCheckOTHER = GetDlgItem(hDlg, IDC_CHECKOTHER);

		//�ײ�״̬��ʾ��
		hStatusInfo = GetDlgItem(hDlg, IDC_STATUS);

		CheckDlgButton(hDlg, IDC_CHECKGC, BST_CHECKED);
		CheckDlgButton(hDlg, IDC_CHECKD, BST_CHECKED);
		CheckDlgButton(hDlg, IDC_CHECKZ, BST_CHECKED);
		CheckDlgButton(hDlg, IDC_CHECKT, BST_CHECKED);
		CheckDlgButton(hDlg, IDC_CHECKK, BST_CHECKED);
		CheckDlgButton(hDlg, IDC_CHECKOTHER, BST_CHECKED);

		EnableWindow(hFromStation, FALSE);
		EnableWindow(hToStation, FALSE);

		hSetoutDate = GetDlgItem(hDlg, IDC_SETOUT);

		int nYear = GetPrivateProfileInt(L"history", L"lastselectyear", 0, L"./cfg.ini");
		int nMonth = GetPrivateProfileInt(L"history", L"lastselectmonth", 0, L"./cfg.ini");
		int nDay = GetPrivateProfileInt(L"history", L"lastselectday", 0, L"./cfg.ini");
		if (nYear > 0 && nMonth > 0 && nDay > 0)
		{
			SYSTEMTIME st = { 0 };
			GetLocalTime(&st);
			if (nYear * 10000 + nMonth * 100 + nDay > st.wYear * 10000 + st.wMonth * 100 + st.wDay)
			{
				st.wYear = nYear;
				st.wMonth = nMonth;
				st.wDay = nDay;
			}
			DateTime_SetSystemtime(hSetoutDate, GDT_VALID, &st);
		}

		hTicketType1 = GetDlgItem(hDlg, IDC_TICKETTYPE1);
		hTicketType2 = GetDlgItem(hDlg, IDC_TICKETTYPE2);
		CheckDlgButton(hDlg, IDC_TICKETTYPE1, BST_CHECKED);

		hTicketsFilter = GetDlgItem(hDlg, IDC_TICKETSFILTER);
		InitTicketsFilter(hTicketsFilter);

		CenterWindow(hDlg);

		if (CUserInfo::GetInstance().m_nLoginStatus == LOGIN_STATUS_NOTLOGIN)
		{
			SetWindowText(hStatusInfo, _T("δ��¼"));
			SetWindowText(hDlg, _T("12306��Ʊϵͳ - δ��¼"));
		}
		else
		{
			char szUserInfo[32] = { 0 };
			sprintf_s(szUserInfo, ARRAYSIZE(szUserInfo), "�û�[%s]�Ѿ���¼", CUserInfo::GetInstance().m_username.c_str());
			SetWindowTextA(hStatusInfo, szUserInfo);
			SetWindowText(hDlg, _T("12306��Ʊϵͳ"));
		}

		NetThread::GetInstance().SetReflectionWindow(hDlg);
		bool bUseLocalCity = GetPrivateProfileInt(L"app", L"uselocalcity", 0, L"./cfg.ini") != 0 ? true : false;
		NetThread::GetInstance().AddGetStationTask(bUseLocalCity);


	}
	return (INT_PTR)TRUE;

	case WM_SIZE:
	{
		int cxList = LOWORD(lParam);
		int cyList = HIWORD(lParam);

		MoveWindow(hTickDetailList, 10, 170, cxList - 20, cyList - 200, TRUE);
		MoveWindow(hStatusInfo, 10, cyList - 20, cxList - 20, 16, TRUE);
	}
	return (INT_PTR)TRUE;

	//���ƴ�����С��С
	case WM_GETMINMAXINFO:
	{
		MINMAXINFO* pMinMaxInfo = reinterpret_cast<MINMAXINFO*>(lParam);
		pMinMaxInfo->ptMinTrackSize.x = 480;
		pMinMaxInfo->ptMinTrackSize.y = 320;
	}
	return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDCANCEL || LOWORD(wParam) == IDM_EXIT)
		{
#ifndef _DEBUG
			if (MessageBox(hDlg, L"ȷ��Ҫ�˳���", L"��ʾ", MB_YESNO | MB_ICONINFORMATION) != IDYES)
				return (INT_PTR)FALSE;
#endif

			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		//�ǳ�
		else if (LOWORD(wParam) == IDM_LOGOUT)
		{
			CUserInfo::GetInstance().m_nLoginStatus = LOGIN_STATUS_NOTLOGIN;
			CUserInfo::GetInstance().m_username.clear();
			CUserInfo::GetInstance().m_password.clear();

			SetWindowText(hStatusInfo, _T("δ��¼"));
			SetWindowText(hDlg, _T("12306��Ʊϵͳ - δ��¼"));
		}
		//�л��û�
		else if (LOWORD(wParam) == IDM_SWITCHUSER)
		{
			CUserInfo::GetInstance().m_nLoginStatus = LOGIN_STATUS_NOTLOGIN;
			CUserInfo::GetInstance().m_username.clear();
			CUserInfo::GetInstance().m_password.clear();

			EndDialog(hDlg, IDCANCEL);
			DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_LOGIN), NULL, LoginDlgProc);
		}
		//��Ʊ
		else if (LOWORD(wParam) == IDC_QUERYTICKET)
		{
			//
			SYSTEMTIME st = { 0 };
			DateTime_GetSystemtime(hSetoutDate, &st);
			char timestr[16] = { 0 };
			sprintf_s(timestr, ARRAYSIZE(timestr), "%04d-%02d-%02d", st.wYear, st.wMonth, st.wDay);
			AppInfo::GetInstance().SetDate(st.wYear, st.wMonth, st.wDay, st.wDayOfWeek);

			int nIndex = SendMessage(hFromStation, CB_GETCURSEL, 0, 0);
			int no = SendMessage(hFromStation, CB_GETITEMDATA, nIndex, 0);
			string strFromStation = AppInfo::GetInstance().GetStationCode(no);
			nIndex = SendMessage(hToStation, CB_GETCURSEL, 0, 0);
			no = SendMessage(hToStation, CB_GETITEMDATA, nIndex, 0);
			string strToStation = AppInfo::GetInstance().GetStationCode(no);
			string purposecode = "ADULT";
			if (IsDlgButtonChecked(hDlg, IDC_TICKETTYPE2))
				purposecode = "0X00";

			AppInfo::GetInstance().SetQueryTicketInfoParams(timestr, strFromStation.c_str(), strToStation.c_str(), purposecode.c_str());
			if (CUserInfo::GetInstance().m_nLoginStatus == LOGIN_STATUS_NOTLOGIN)
				NetThread::GetInstance().AddGuestQueryTicketTask(timestr, strFromStation.c_str(), strToStation.c_str(), purposecode.c_str());
			else
				NetThread::GetInstance().AddQueryTicketTask(timestr, strFromStation.c_str(), strToStation.c_str(), purposecode.c_str());

			EnableWindow(hTickDetailList, FALSE);
		}
		//����Ʊѡ�����
		else if (LOWORD(wParam) == IDC_TICKETSFILTER)
		{
			if (HIWORD(wParam) == CBN_SELCHANGE)
			{
				int nCurSel = SendMessage(hTicketsFilter, CB_GETCURSEL, 0, 0);
				if (g_nTicketsFilterIndex == nCurSel)
					return (INT_PTR)FALSE;
				g_nTicketsFilterIndex = nCurSel;
				FillTicketsInfo(hTickDetailList);
				InvalidateRect(hTicketsFilter, NULL, FALSE);
			}
		}
		//��Ʊ
		else if (LOWORD(wParam) == IDC_BUYTICKET)
		{
			if (CUserInfo::GetInstance().m_nLoginStatus == LOGIN_STATUS_NOTLOGIN)
			{
				if (MessageBox(hDlg, L"��Ʊ��Ҫ���ȵ�¼���Ƿ����ھ�ȥ��¼", L"��ʾ", MB_YESNO | MB_ICONINFORMATION) != IDYES)
					return (INT_PTR)FALSE;
				EndDialog(hDlg, IDCANCEL);
				DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_LOGIN), NULL, LoginDlgProc);
				return (INT_PTR)FALSE;
			}

			int iSelected = ListView_GetNextItem(hTickDetailList, -1, LVNI_SELECTED);
			if (iSelected != -1)
			{
				ticketinfo ti;
				AppInfo::GetInstance().SetBuyTicketInfoIndex(iSelected);
				NetThread::GetInstance().AddPreSubmitOrderTask(iSelected);
			}
			else
				MessageBox(hDlg, L"����ѡ��Ʊ", L"��ʾ", MB_OK | MB_ICONINFORMATION);
		}
		else if (LOWORD(wParam) == IDM_ABOUT)
		{
			DialogBox(NULL, MAKEINTRESOURCE(IDD_ABOUTBOX), hDlg, AboutDlgProc);
		}
		//�����г��ͺ�
		else if (LOWORD(wParam) == IDC_CHECKGC ||
			LOWORD(wParam) == IDC_CHECKD ||
			LOWORD(wParam) == IDC_CHECKZ ||
			LOWORD(wParam) == IDC_CHECKT ||
			LOWORD(wParam) == IDC_CHECKK ||
			LOWORD(wParam) == IDC_CHECKOTHER)
		{
			if (IsDlgButtonChecked(hDlg, IDC_CHECKGC))
				g_nTrainType |= TRAIN_GC;
			else
				g_nTrainType &= ~TRAIN_GC;

			if (IsDlgButtonChecked(hDlg, IDC_CHECKD))
				g_nTrainType |= TRAIN_D;
			else
				g_nTrainType &= ~TRAIN_D;

			if (IsDlgButtonChecked(hDlg, IDC_CHECKZ))
				g_nTrainType |= TRAIN_Z;
			else
				g_nTrainType &= ~TRAIN_Z;

			if (IsDlgButtonChecked(hDlg, IDC_CHECKT))
				g_nTrainType |= TRAIN_T;
			else
				g_nTrainType &= ~TRAIN_T;

			if (IsDlgButtonChecked(hDlg, IDC_CHECKK))
				g_nTrainType |= TRAIN_K;
			else
				g_nTrainType &= ~TRAIN_K;

			if (IsDlgButtonChecked(hDlg, IDC_CHECKOTHER))
				g_nTrainType |= TRAIN_OTHER;
			else
				g_nTrainType &= ~TRAIN_OTHER;


			FillTicketsInfo(hTickDetailList);
			InvalidateRect(hTicketsFilter, NULL, FALSE);
		}

		break;
		//��Ʊ���
	case WM_QUERYTICKETS:
	{
		EnableWindow(hTickDetailList, TRUE);
		if (wParam == OPERATION_FAIL)
		{
			MessageBox(hDlg, L"��Ʊʧ��������", L"��ʾ", MB_OK | MB_ICONINFORMATION);
			return (INT_PTR)FALSE;
		}

		FillTicketsInfo(hTickDetailList);
	}
	break;

	//���س�վ��Ϣ���
	case WM_GETSTATIONINFO:
	{
		if (wParam == OPERATION_FAIL)
		{
			MessageBox(hDlg, L"���س�վ��Ϣʧ��", L"��ʾ", MB_OK | MB_ICONERROR);
			return (INT_PTR)FALSE;
		}

		FillStationInfo(hFromStation, hToStation);

		EnableWindow(hFromStation, TRUE);
		EnableWindow(hToStation, TRUE);
	}
	break;

	//Ԥ�ύ�������
	case  WM_PRESUBMIT_ORDER:
	{
		if (wParam == OPERATION_FAIL)
		{
			MessageBox(hDlg, L"Ԥ�ύ����ʧ��", L"��ʾ", MB_OK | MB_ICONERROR);
			return (INT_PTR)FALSE;
		}
		else if (wParam == OPERATION_NEEDRELOGIN)
		{
			if (MessageBox(hDlg, L"�Ự�ѹ��ڣ���Ҫ���µ�¼�����ھ����µ�¼��", L"��ʾ", MB_YESNO | MB_ICONINFORMATION) != IDYES)
				return (INT_PTR)FALSE;

			EndDialog(hDlg, IDCANCEL);
			DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_LOGIN), NULL, LoginDlgProc);
		}

		if (DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_CONFIRMTICKET), hDlg, ConfirmTicketDlgProc) != IDOK)
			break;

		//TODO: �ύ����

	}
	break;

	case WM_CHECK_ORDER:
	{
		if (wParam == OPERATION_FAIL)
		{
			MessageBox(hDlg, L"�ύ����ʧ�ܣ�������", L"��ʾ", MB_OK | MB_ICONERROR);
			return (INT_PTR)FALSE;
		}

		MessageBox(hDlg, L"��Ʊ�ɹ�������30�����ڵ�¼12306����֧��", L"��ʾ", MB_OK | MB_ICONINFORMATION);
	}
	break;

	case WM_DESTROY:
		SaveToHistory(hFromStation, hToStation, hSetoutDate);
		DestroyMenu(hMainMenu);
		break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK ConfirmTicketDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	static HWND passengerButton[50] = { 0 };
	switch (msg)
	{
	case WM_INITDIALOG:
	{
		//CenterWindow(hDlg, g_hMainDlg);
		//HWND hPassengerInfo = GetDlgItem(hDlg, IDC_PASSENGERINFO);
		vector<passenager> p;
		AppInfo::GetInstance().GetPassengerInfo(p);
		size_t size = p.size();
		for (size_t i = 0; i < size; ++i)
		{
			wstring strData = Utf8ToUnicode(p[i].passenger_name);
			int index = i / 4;
			passengerButton[i] = CreateWindow(L"BUTTON", strData.c_str(), WS_CHILD | BS_AUTOCHECKBOX, 25 + (i % 4) * 70, index * 20 + 60, 70, 20, hDlg, (HMENU)(ID_PASSENGERBTN_BASE + i), g_hInst, NULL);
			ShowWindow(passengerButton[i], SW_SHOW);
		}

		ticketinfo ti;
		AppInfo::GetInstance().GetTicketInfoByIndex(ti);

		wstring strTicketInfo;
		//����վ-����վ
		string strStation = ti.DTO.station_train_code;
		strStation += " ";
		strStation += AppInfo::GetInstance().GetStationNameByCode(ti.DTO.from_station_telecode);
		strStation += "->";
		strStation += AppInfo::GetInstance().GetStationNameByCode(ti.DTO.to_station_telecode);
		TCHAR szData[256] = { 0 };
		_tcscpy_s(szData, ARRAYSIZE(szData), Utf8ToUnicode(strStation).c_str());
		strTicketInfo = szData;

		int wYear;
		int wMonth;
		int wDay;
		int wWeekday;
		AppInfo::GetInstance().GetDate(wYear, wMonth, wDay, wWeekday);
		memset(szData, 0, sizeof(szData));
		wsprintfW(szData, L"%04d-%02d-%02d", wYear, wMonth, wDay);
		strTicketInfo += L" ";
		strTicketInfo += szData;

		//����ʱ�䡫����ʱ��
		memset(szData, 0, sizeof(szData));
		string strTime = ti.DTO.start_time;
		strTime += "~";
		strTime += ti.DTO.arrive_time;
		_tcscpy_s(szData, ARRAYSIZE(szData), Utf8ToUnicode(strTime).c_str());
		strTicketInfo += L" ";
		strTicketInfo += szData;
		SetWindowText(GetDlgItem(hDlg, IDC_TICKETINFO), strTicketInfo.c_str());

	}
	return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, IDCANCEL);
			return (INT_PTR)TRUE;
		}
		else if (LOWORD(wParam) == IDOK)
		{
			//��װ������Ϣ
			vector<passenager> p;
			AppInfo::GetInstance().GetPassengerInfo(p);
			OrderInfo oi;
			ostringstream oldPassengerStr;

			int size = ARRAYSIZE(passengerButton);
			int count = 0;
			for (int i = 0; i < size; ++i)
			{
				if (!IsDlgButtonChecked(hDlg, i + ID_PASSENGERBTN_BASE))
					continue;

				count++;
			}

			ostringstream passengerTicketStr;

			for (int i = 0; i < size; ++i)
			{
				if (!IsDlgButtonChecked(hDlg, i + ID_PASSENGERBTN_BASE))
					continue;

				oldPassengerStr << p[i].passenger_name << ",";
				oldPassengerStr << p[i].passenger_id_type_code << ",";
				oldPassengerStr << p[i].passenger_id_no << ",";
				oldPassengerStr << p[i].passenger_type << "_";

				//������,0,����Ʊ
				passengerTicketStr << "O,0,1," << p[i].passenger_name << ",";
				passengerTicketStr << p[i].passenger_id_type_code << ",";
				passengerTicketStr << p[i].passenger_id_no << ",";
				passengerTicketStr << p[i].mobile_no << ",N";
				if (count != 1)
					passengerTicketStr << "_";
			}

			string strEncode;
			UrlEncode(oldPassengerStr.str(), strEncode);
			oi.oldPassengerStr = strEncode;

			string strTemp = passengerTicketStr.str();
			if (count != 1)
			{
				strTemp = strTemp.substr(0, strTemp.length() - 1);
			}
			//string strEncode;
			strEncode.clear();
			UrlEncode(strTemp, strEncode);
			oi.passengerTicketStr = strEncode;
			oi.tour_flag = "dc";

			ticketinfo ti;
			AppInfo::GetInstance().GetTicketInfoByIndex(ti);
			oi.fromStationTelecode = ti.DTO.from_station_telecode;
			oi.toStationTelecode = ti.DTO.to_station_telecode;
			oi.leftTicket = ti.DTO.yp_info;
			oi.train_location = ti.DTO.location_code;
			oi.purpose_codes = "00";
			oi.seatType = "O";
			oi.train_no = ti.DTO.train_no;
			oi.stationTrainCode = ti.DTO.station_train_code;


			int wYear;
			int wMonth;
			int wDay;
			int wWeekday;
			AppInfo::GetInstance().GetDate(wYear, wMonth, wDay, wWeekday);
			ostringstream train_date;
			wstring str = L"(�й���׼ʱ��)";
			char szData[32] = { 0 };
			string strUtf8 = UnicodeToUtf8(str);
			train_date << g_Weekdays[wWeekday] << " " << g_Months[wMonth - 1] << " " << wDay << " " << wYear << " 00:00:00 GMT+0800"/* << strUtf8*/;
			string train_date_encode;
			UrlEncode(train_date.str(), train_date_encode);
			oi.train_date = train_date_encode;
			AppInfo::GetInstance().SetOrderInfo(oi);

			NetThread::GetInstance().AddSubmitOrderInfoTask(oi);
			EndDialog(hDlg, IDOK);
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK AboutDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (msg)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

void CenterWindow(HWND hTarget, HWND hParent)
{
	if (!::IsWindow(hTarget))
		return;

	if (!::IsWindow(hParent))
		hParent = GetDesktopWindow();

	RECT rtTarget = { 0 };
	GetWindowRect(hTarget, &rtTarget);
	RECT rtParent = { 0 };
	GetWindowRect(hParent, &rtParent);
	MoveWindow(hTarget,
		(rtParent.right - rtTarget.right) / 2,
		(rtParent.bottom - rtTarget.bottom) / 2,
		rtTarget.right - rtTarget.left,
		rtTarget.bottom - rtTarget.top,
		FALSE);
}

void ResetVCodeSliceUnmarked()
{
	for (int i = 0; i < ARRAYSIZE(g_bVodeSlice1Pressed); ++i)
	{
		g_bVodeSlice1Pressed[i] = false;
	}
}

bool InitTicketDetailList(HWND hList)
{
	ListView_SetExtendedListViewStyle(hList, LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);

	LVCOLUMN lvc = { 0 };
	// Initialize the LVCOLUMN structure.
	// The mask specifies that the format, width, text,
	// and subitem members of the structure are valid.
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	wstring str;
	for (int iCol = 0; iCol < ARRAYSIZE(g_szTicketTitle); ++iCol)
	{
		//��������
		lvc.iSubItem = iCol;
		lvc.pszText = g_szTicketTitle[iCol];
		//TODO: ÿһ�г�����Ϣд�ڽṹ������
		lvc.cx = 100;

		// Insert the columns into the list view.
		if (ListView_InsertColumn(hList, iCol, &lvc) == -1)
			return false;
	}

	ListView_SetColumnWidth(hList, 0, 70);
	ListView_SetColumnWidth(hList, 1, 130);
	ListView_SetColumnWidth(hList, 2, 120);


	return true;
}

bool InitTicketsFilter(HWND hTicketsFilter)
{
	TCHAR* pszFilter[] = { _T("��ʾȫ��"),
							_T("����ʾ��������"),
							_T("����ʾ����������"),
							_T("����ʾ��������"),
							_T("����ʾ����Ӳ��"),
							_T("����ʾ����߼�����"),
							_T("����ʾ��������"),
							_T("����ʾ����Ӳ��"),
							_T("����ʾ�����ص���"),
							_T("����ʾ����һ����"),
							_T("����ʾ���������"),
							_T("����ʾ��������")
	};

	for (size_t i = 0; i < ARRAYSIZE(pszFilter); ++i)
	{
		SendMessage(hTicketsFilter, CB_ADDSTRING, 0, (LPARAM)pszFilter[i]);
		SendMessage(hTicketsFilter, CB_SETITEMDATA, i, (LPARAM)i);
	}

	//Ĭ��ѡ�е�һ��
	SendMessage(hTicketsFilter, CB_SETCURSEL, 0, 0);

	return true;
}

bool FillTicketsInfo(HWND hList)
{
	if (ListView_DeleteAllItems(hList) == -1)
		return false;

	vector<ticketinfo> info;
	AppInfo::GetInstance().GetQueryTicketInfoResult(info);
	//���ݹ�������ɸѡƱ
	for (auto iter = info.begin(); iter != info.end(); )
	{
		//����ʾ��������
		if (g_nTicketsFilterIndex == 1)
		{
			if (iter->DTO.swz_num == "--" &&
				iter->DTO.rz_num == "--" &&
				iter->DTO.yz_num == "--" &&
				iter->DTO.gr_num == "--" &&
				iter->DTO.rw_num == "--" &&
				iter->DTO.yw_num == "--" &&
				iter->DTO.tz_num == "--" &&
				iter->DTO.zy_num == "--" &&
				iter->DTO.ze_num == "--"  &&
				iter->DTO.wz_num == "--")
			{
				iter = info.erase(iter);
			}
			else
				iter++;
		}
		//����ʾ����������
		else if (g_nTicketsFilterIndex == 2)
		{
			if (iter->DTO.swz_num == "--")
			{
				iter = info.erase(iter);
			}
			else
				iter++;
		}
		//����ʾ��������
		else if (g_nTicketsFilterIndex == 3)
		{
			if (iter->DTO.rz_num == "--")
			{
				iter = info.erase(iter);
			}
			else
				iter++;
		}
		//����ʾ����Ӳ��
		else if (g_nTicketsFilterIndex == 4)
		{
			if (iter->DTO.yz_num == "--")
			{
				iter = info.erase(iter);
			}
			else
				iter++;
		}
		//����ʾ����߼�����
		else if (g_nTicketsFilterIndex == 5)
		{
			if (iter->DTO.gr_num == "--")
			{
				iter = info.erase(iter);
			}
			else
				iter++;
		}
		//����ʾ��������
		else if (g_nTicketsFilterIndex == 6)
		{
			if (iter->DTO.rw_num == "--")
			{
				iter = info.erase(iter);
			}
			else
				iter++;
		}
		//����ʾ����Ӳ��
		else if (g_nTicketsFilterIndex == 7)
		{
			if (iter->DTO.yw_num == "--")
			{
				iter = info.erase(iter);
			}
			else
				iter++;
		}
		//����ʾ�����ص���
		else if (g_nTicketsFilterIndex == 8)
		{
			if (iter->DTO.tz_num == "--")
			{
				iter = info.erase(iter);
			}
			else
				iter++;
		}
		//����ʾ����һ����
		else if (g_nTicketsFilterIndex == 9)
		{
			if (iter->DTO.zy_num == "--")
			{
				iter = info.erase(iter);
			}
			else
				iter++;
		}
		//����ʾ���������
		else if (g_nTicketsFilterIndex == 10)
		{
			if (iter->DTO.ze_num == "--")
			{
				iter = info.erase(iter);
			}
			else
				iter++;
		}
		//����ʾ��������
		else if (g_nTicketsFilterIndex == 11)
		{
			if (iter->DTO.wz_num == "--")
			{
				iter = info.erase(iter);
			}
			else
				iter++;
		}
		else
			iter++;
	}


	//�����г��ͺŹ��˳���
	for (auto iter = info.begin(); iter != info.end();)
	{
		//&��������ȼ���==�ͣ�������������
		//����ʾ����Ʊ
		if ((g_nTrainType & TRAIN_GC) == 0)
		{
			if (iter->DTO.station_train_code[0] == 'G')
			{
				iter = info.erase(iter);
				continue;
			}
		}
		//����ʾ����Ʊ
		if ((g_nTrainType & TRAIN_D) == 0)
		{
			if (iter->DTO.station_train_code[0] == 'D')
			{
				iter = info.erase(iter);
				continue;
			}
		}
		//����ʾֱ��Ʊ
		if ((g_nTrainType & TRAIN_Z) == 0)
		{
			if (iter->DTO.station_train_code[0] == 'Z')
			{
				iter = info.erase(iter);
				continue;
			}
		}
		//����ʾZƱ
		if ((g_nTrainType & TRAIN_T) == 0)
		{
			if (iter->DTO.station_train_code[0] == 'T')
			{
				iter = info.erase(iter);
				continue;
			}
		}
		//����ʾKƱ
		if ((g_nTrainType & TRAIN_K) == 0)
		{
			if (iter->DTO.station_train_code[0] == 'K')
			{
				iter = info.erase(iter);
				continue;
			}
		}
		//����ʾ����Ʊ
		if ((g_nTrainType & TRAIN_OTHER) == 0)
		{
			if (iter->DTO.station_train_code[0] != 'G' &&
				iter->DTO.station_train_code[0] != 'D' &&
				iter->DTO.station_train_code[0] != 'Z' &&
				iter->DTO.station_train_code[0] != 'T' &&
				iter->DTO.station_train_code[0] != 'K')
			{
				iter = info.erase(iter);
				continue;
			}
		}


		iter++;
	}


	size_t size = info.size();
	size_t index = 0;
	LVITEM lvi = { 0 };
	lvi.mask = LVIF_TEXT;
	TCHAR szData[64];
	AppInfo& appInfo = AppInfo::GetInstance();
	for (size_t index = 0; index < size; ++index)
	{
		//����
		memset(szData, 0, sizeof(szData));
		_tcscpy_s(szData, ARRAYSIZE(szData), Utf8ToUnicode(info[index].DTO.station_train_code).c_str());
		lvi.pszText = szData;
		lvi.cchTextMax = ARRAYSIZE(szData);

		//������������������
		//�к�
		lvi.iItem = index;
		//�к�
		lvi.iSubItem = 0;
		// Insert items into the list.
		ListView_InsertItem(hList, &lvi);

		//����վ-����վ
		memset(szData, 0, sizeof(szData));
		string strFromStation = appInfo.GetStationNameByCode(info[index].DTO.from_station_telecode);
		wstring strFromStationW = Utf8ToUnicode(strFromStation);
		//��ʼվ���ڳ���վ���ǹ�·��
		if (info[index].DTO.from_station_telecode != info[index].DTO.start_station_telecode)
			strFromStationW += _T("(��)");

		string strToStation = appInfo.GetStationNameByCode(info[index].DTO.to_station_telecode);
		wstring strToStationW = Utf8ToUnicode(strToStation);
		//��ʼվ���ڳ���վ���ǹ�·��s
		if (info[index].DTO.to_station_telecode != info[index].DTO.end_station_telecode)
			strToStationW += _T("(��)");

		wsprintfW(szData, _T("%s->%s"), strFromStationW.c_str(), strToStationW.c_str());
		lvi.iSubItem = 1;
		ListView_SetItem(hList, &lvi);

		//����ʱ�䡫����ʱ��
		memset(szData, 0, sizeof(szData));
		string strTime = info[index].DTO.start_time;
		strTime += "~";
		strTime += info[index].DTO.arrive_time;
		_tcscpy_s(szData, ARRAYSIZE(szData), Utf8ToUnicode(strTime).c_str());
		OutputDebugString(L"\n");
		OutputDebugString(szData);
		lvi.pszText = szData;
		lvi.iSubItem = 2;
		ListView_SetItem(hList, &lvi);

		//��ʱ
		memset(szData, 0, sizeof(szData));
		string strHours = info[index].DTO.lishi;
		_tcscpy_s(szData, ARRAYSIZE(szData), Utf8ToUnicode(strHours).c_str());
		OutputDebugString(L"\t");
		OutputDebugString(szData);
		lvi.iSubItem = 3;
		//lvi.lParam = atol(strHours.substr(0, 2).c_str()) * 60 + atol(strHours.substr(3).c_str());
		ListView_SetItem(hList, &lvi);

		//������
		memset(szData, 0, sizeof(szData));
		_tcscpy_s(szData, ARRAYSIZE(szData), Utf8ToUnicode(info[index].DTO.swz_num).c_str());
		OutputDebugString(L"\t");
		OutputDebugString(szData);
		lvi.iSubItem = 4;
		ListView_SetItem(hList, &lvi);

		//�ص���
		memset(szData, 0, sizeof(szData));
		_tcscpy_s(szData, ARRAYSIZE(szData), Utf8ToUnicode(info[index].DTO.tz_num).c_str());
		OutputDebugString(L"\t");
		OutputDebugString(szData);
		lvi.iSubItem = 5;
		ListView_SetItem(hList, &lvi);

		//һ����
		memset(szData, 0, sizeof(szData));
		_tcscpy_s(szData, ARRAYSIZE(szData), Utf8ToUnicode(info[index].DTO.zy_num).c_str());
		OutputDebugString(L"\t");
		OutputDebugString(szData);
		lvi.iSubItem = 6;
		ListView_SetItem(hList, &lvi);

		//������
		memset(szData, 0, sizeof(szData));
		_tcscpy_s(szData, ARRAYSIZE(szData), Utf8ToUnicode(info[index].DTO.ze_num).c_str());
		OutputDebugString(L"\t");
		OutputDebugString(szData);
		lvi.iSubItem = 7;
		ListView_SetItem(hList, &lvi);

		//�߼�����
		memset(szData, 0, sizeof(szData));
		_tcscpy_s(szData, ARRAYSIZE(szData), Utf8ToUnicode(info[index].DTO.gr_num).c_str());
		OutputDebugString(L"\t");
		OutputDebugString(szData);
		lvi.iSubItem = 8;
		ListView_SetItem(hList, &lvi);

		//����
		memset(szData, 0, sizeof(szData));
		_tcscpy_s(szData, ARRAYSIZE(szData), Utf8ToUnicode(info[index].DTO.rw_num).c_str());
		OutputDebugString(L"\t");
		OutputDebugString(szData);
		lvi.iSubItem = 9;
		ListView_SetItem(hList, &lvi);

		//Ӳ��
		memset(szData, 0, sizeof(szData));
		_tcscpy_s(szData, ARRAYSIZE(szData), Utf8ToUnicode(info[index].DTO.yw_num).c_str());
		OutputDebugString(L"\t");
		OutputDebugString(szData);
		lvi.iSubItem = 10;
		ListView_SetItem(hList, &lvi);

		//����
		memset(szData, 0, sizeof(szData));
		_tcscpy_s(szData, ARRAYSIZE(szData), Utf8ToUnicode(info[index].DTO.rz_num).c_str());
		OutputDebugString(L"\t");
		OutputDebugString(szData);
		lvi.iSubItem = 11;
		ListView_SetItem(hList, &lvi);

		//Ӳ��
		memset(szData, 0, sizeof(szData));
		_tcscpy_s(szData, ARRAYSIZE(szData), Utf8ToUnicode(info[index].DTO.yz_num).c_str());
		OutputDebugString(L"\t");
		OutputDebugString(szData);
		lvi.iSubItem = 12;
		ListView_SetItem(hList, &lvi);

		//����
		memset(szData, 0, sizeof(szData));
		_tcscpy_s(szData, ARRAYSIZE(szData), Utf8ToUnicode(info[index].DTO.wz_num).c_str());
		OutputDebugString(L"\t");
		OutputDebugString(szData);
		lvi.iSubItem = 13;
		ListView_SetItem(hList, &lvi);

		//��ע
		memset(szData, 0, sizeof(szData));
		_tcscpy_s(szData, ARRAYSIZE(szData), Utf8ToUnicode(info[index].buttonTextInfo).c_str());
		lvi.iSubItem = 15;
		ListView_SetItem(hList, &lvi);
	}

	ListView_SetColumnWidth(hList, 0, 70);
	ListView_SetColumnWidth(hList, 1, 130);
	ListView_SetColumnWidth(hList, 2, 120);

	//ListView_SetTextBkColor(hList, RGB(255, 0, 0));
	//ListView_SortItems(hList, CompareFunc, 0);

	return true;
}

bool FillStationInfo(HWND hFromStation, HWND hToStation)
{
	vector<stationinfo> v;
	AppInfo::GetInstance().GetStationInfo(v);

	int nlastfromstationno = GetPrivateProfileInt(L"history", L"lastfromstationno", 0, L"./cfg.ini");
	int nlasttostationno = GetPrivateProfileInt(L"history", L"lasttostationno", 0, L"./cfg.ini");

	size_t size = v.size();
	for (size_t i = 0; i < size; ++i)
	{
		wstring strData = Utf8ToUnicode(v[i].hanzi);
		int n = v[i].no;

		SendMessage(hFromStation, CB_ADDSTRING, 0, (LPARAM)strData.data());
		SendMessage(hFromStation, CB_SETITEMDATA, i, (LPARAM)n);

		SendMessage(hToStation, CB_ADDSTRING, 0, (LPARAM)strData.data());
		SendMessage(hToStation, CB_SETITEMDATA, i, (LPARAM)n);

		if (n == nlastfromstationno)
		{
			SendMessage(hFromStation, CB_SETCURSEL, i, 0);
		}

		if (n == nlasttostationno)
		{
			SendMessage(hToStation, CB_SETCURSEL, i, 0);
		}
	}

	return true;
}

void SaveToHistory(HWND hFromStation, HWND hToStation, HWND hSetoutDate)
{
	vector<stationinfo> v;
	AppInfo::GetInstance().GetStationInfo(v);

	int nlastfromstationno = 0;
	int nlasttostationno = 0;

	int nFrom = SendMessage(hFromStation, CB_GETCURSEL, 0, 0);
	int nTo = SendMessage(hToStation, CB_GETCURSEL, 0, 0);

	size_t size = v.size();
	for (size_t i = 0; i < size; ++i)
	{
		if (i == nFrom)
		{
			nlastfromstationno = v[i].no;
		}

		if (i == nTo)
		{
			nlasttostationno = v[i].no;
		}
	}

	TCHAR szData[8] = { 0 };
	wsprintfW(szData, L"%d", nFrom);
	WritePrivateProfileString(L"history", L"lastfromstationno", szData, L"./cfg.ini");
	memset(szData, 0, sizeof(szData));
	wsprintfW(szData, L"%d", nTo);
	WritePrivateProfileString(L"history", L"lasttostationno", szData, L"./cfg.ini");

	SYSTEMTIME st = { 0 };
	DateTime_GetSystemtime(hSetoutDate, &st);

	memset(szData, 0, sizeof(szData));
	wsprintfW(szData, L"%d", st.wYear);
	WritePrivateProfileString(L"history", L"lastselectyear", szData, L"./cfg.ini");

	memset(szData, 0, sizeof(szData));
	wsprintfW(szData, L"%d", st.wMonth);
	WritePrivateProfileString(L"history", L"lastselectmonth", szData, L"./cfg.ini");

	memset(szData, 0, sizeof(szData));
	wsprintfW(szData, L"%d", st.wDay);
	WritePrivateProfileString(L"history", L"lastselectday", szData, L"./cfg.ini");
}

bool UrlEncode(const string& src, string& dst)
{
	if (src.empty())
		return false;

	char hex[] = "0123456789ABCDEF";
	size_t size = src.size();
	for (size_t i = 0; i < size; ++i)
	{
		unsigned char cc = src[i];
		if (isascii(cc))
		{
			if (cc == ' ')
			{
				dst += "%20";
			}
			else if (cc == ',')
			{
				dst += "%2C";
			}
			else if (cc == '(')
			{
				dst += "%28";
			}
			else if (cc == ')')
			{
				dst += "%29";
			}
			else if (cc == ':')
			{
				dst += "%3A";
			}
			else if (cc == '+')
			{
				dst += "%2B";
			}
			else
				dst += cc;
		}
		else
		{
			unsigned char c = static_cast<unsigned char>(src[i]);
			dst += '%';
			dst += hex[c / 16];
			dst += hex[c % 16];
		}
	}
	return true;
}

int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	if (lParam1 < lParam2)
		return -1;
	if (lParam1 == lParam2)
		return 0;
	else
		return 1;
}