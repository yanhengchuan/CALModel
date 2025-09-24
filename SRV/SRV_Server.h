/*============================================================================
/ SRV_Server.h: main header file for the SERVERSOCKET applications.
/
/ Revisions:
/  Created:       2007 Sep      JC.Mitais
/  Modified: 
/  
/ (c) CMI Industry
/===========================================================================*/
#if !defined(AFX_LTOPSERVER_H__BFD2712F_6DB3_4E82_9FC7_ABEB33496E09__INCLUDED_)
   #define AFX_LTOPSERVER_H__BFD2712F_6DB3_4E82_9FC7_ABEB33496E09__INCLUDED_

   #ifndef __AFXWIN_H__
	   #error include 'stdafx.h' before including this file for PCH
   #endif
#endif // !defined(AFX_LTOPSERVER_H__BFD2712F_6DB3_4E82_9FC7_ABEB33496E09__INCLUDED_)

/////////////////////////////////////////////////////////////////////////////
// CLtopServerApp:
// See LtopServer.cpp for the implementation of this class
class CLtopServerApp : public CWinApp
{
public:
	CLtopServerApp();

	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

   DECLARE_MESSAGE_MAP()

protected:
	//int  m_nLinkMode;
	void ParseCommandLineArgs();
};

