// snow.cpp : main source file for snow.exe
//

#include "stdafx.h"

#include "atlframe.h"
#include "atlctrls.h"
#include "atldlgs.h"

#include "resource.h"


#include "MainFrm.h"

#include <atltypes.h>
#include <time.h>


CAppModule _Module;

int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	CGLMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	CMainFrame wndMain;


	//<fullscreen>
	//������� ���������� � ����� ������� ������
	CRect rect(0, 0, GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN)); 
	ATL::_U_RECT ur(rect);
	//if(wndMain.CreateEx(NULL, rect, WS_POPUP) == NULL)
	//</fullscreen>

	//��� ��������� ����� CreateEx() ��������� �������� ��� ����� �������� ����������
	if(wndMain.CreateEx() == NULL)
	{
		ATLTRACE(_T("Main window creation failed!\n"));
		return 0;
	}

	wndMain.ShowWindow(nCmdShow);

	int nRet = theLoop.Run();

	_Module.RemoveMessageLoop();
	return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
	srand(GetTickCount());

	HRESULT hRes = ::CoInitialize(NULL);
// If you are running on NT 4.0 or higher you can use the following call instead to 
// make the EXE free threaded. This means that calls come in on a random RPC thread.
//	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	ATLASSERT(SUCCEEDED(hRes));

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_BAR_CLASSES);	// add flags to support other controls

	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	int nRet = Run(lpstrCmdLine, nCmdShow);

	_Module.Term();
	::CoUninitialize();

	return nRet;
}
