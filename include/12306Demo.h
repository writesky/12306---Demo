#pragma once
#include "resource.h"

// Forward declarations of functions included in this code module:

void ClearExpiredLog();

INT_PTR CALLBACK LoginDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK MainDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ConfirmTicketDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AboutDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
void CenterWindow(HWND hTarget, HWND hParent = NULL);

void ResetVCodeSliceUnmarked();

bool InitTicketDetailList(HWND hList);
bool InitTicketsFilter(HWND hTicketsFilter);

bool FillTicketsInfo(HWND hList);
bool FillStationInfo(HWND hFromStation, HWND hToStation);

void SaveToHistory(HWND hFromStation, HWND hToStation, HWND hSetoutDate);
bool UrlEncode(const string& src, string& dst);

int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);