/*========================================================================================
/ SRV_Server.cpp (MAIN): Initialization of simulation, HMI communication, SQL connection,
/     and dialog box.
/ 
/  Revisions:
/  Created:       2007-May       X.Fournaud
/  Modified: 
/      
/ (c) CMI Industry
//======================================================================================*/
#include "stdafx.h"
#include "SRV_Server.h"

#define WSA_VERSION MAKEWORD(2,0)
extern bool STARTUP = true;
/////////////////////////////////////////////////////////////////////////////
// CLtopServerApp
BEGIN_MESSAGE_MAP(CLtopServerApp, CWinApp)
   ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLtopServerApp construction
CLtopServerApp::CLtopServerApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CLtopServerApp object
CLtopServerApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CLtopServerApp initialization
BOOL CLtopServerApp::InitInstance()
{
   bool status=true;

	WSADATA WSAData = { 0 };
	if ( 0 != WSAStartup( WSA_VERSION, &WSAData ) )
	{
		// Tell the user that we could not find a usable WinSock DLL.
		if ( LOBYTE( WSAData.wVersion ) != LOBYTE(WSA_VERSION) ||
			 HIBYTE( WSAData.wVersion ) != HIBYTE(WSA_VERSION) )
			 ::MessageBoxA(NULL, "Incorrect version of WS2_32.dll found", "Error", MB_OK);

		WSACleanup( );
		return FALSE;
	}

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need.
   #ifdef _AFXDLL
	   //Enable3dControls();			// Call this when using MFC in a shared DLL
   #else
	   Enable3dControlsStatic();	// Call this when linking to MFC statically
   #endif

	CSPSDlg dlg1; 
	m_pMainWnd = &dlg1; 

	int nResponse = (int)((CDialog*)m_pMainWnd)->DoModal();	// modal managed in MFC dll
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is dismissed with OK
      CoUninitialize();
	}

	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	// application, rather than start the application's message pump.
	return FALSE;
}

int CLtopServerApp::ExitInstance() 
{
	// Terminate use of the WS2_32.DLL
	WSACleanup();
	
	return CWinApp::ExitInstance();
}

void CLtopServerApp::ParseCommandLineArgs()
{
	CString strCmdLine = GetCommandLine();

	if (!strCmdLine.IsEmpty())
	{
		strCmdLine.MakeUpper();

		int nPos = 0;
		do 
      {
			nPos = strCmdLine.Find(TCHAR(' '));
			if (nPos>0)
			{
				strCmdLine.Delete(0, nPos+1);
				CString strCurrent = strCmdLine;
				int nNextPos = strCmdLine.Find(TCHAR(' '));

				if (nNextPos > 0)
					strCurrent = strCmdLine.Left( nNextPos );

				//if (strCurrent == _T("/SERVER") || strCurrent == _T("/S"))
				//	m_nLinkMode = 0;

				//else if (strCurrent == _T("/CLIENT") || strCurrent == _T("/C"))
				//	m_nLinkMode = 1;
			}
		} while( nPos != -1);
	}
}

