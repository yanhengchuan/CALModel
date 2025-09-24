/*============================================================================
/ SPS_Dlg: Implementation file.
/
/ Revisions:
/  Created:       2010-Apr       X.Fournaud
/  Modified: 
/  
/ (c) CMI Industry
//===========================================================================*/
#include "stdafx.h"

Cl_SQL_Thread *m_SQL = NULL;
HANDLE d_eventStopSQL;
DWORD HandleSQLThread;

Cl_SPG_Thread *m_SPG = NULL;
HANDLE d_eventStopSPG;

Cl_HMI_Thread *m_HMI = NULL;
HANDLE d_eventStopHMI;

Cl_SIM_Thread *m_SIM = NULL;
HANDLE d_eventStopSIM;

Cl_MAIN_Thread *m_MAIN = NULL;
HANDLE d_eventStopMAIN;

CEdit	*m_ctlMsgListBox;
CMenu* mmenu;

CRITICAL_SECTION Sec_MsgWnd;

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About
class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

   // Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	// ClassWizard generated virtual function overrides
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSPSDlg dialog
CSPSDlg::CSPSDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSPSDlg::IDD, pParent)
{
   m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSPSDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MESSAGE_LIST, m_ctlMsgList);
	DDX_Control(pDX, IDC_PROGRESS2, mProgressCtrlSQL);
}

BEGIN_MESSAGE_MAP(CSPSDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_CLEAR, OnBnClickedBtnClear)
	ON_COMMAND(ID_LTOP_EXIT, &CSPSDlg::OnLtopExit)
	ON_COMMAND(ID_MODE_SIMULATION, &CSPSDlg::OnModeSimulation)
	ON_COMMAND(ID_MODE_PRODUCTION, &CSPSDlg::OnModeProduction)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRtopDlg message handlers
BOOL CSPSDlg::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_KEYDOWN)
	{
		int nVirtKey = (int) pMsg->wParam;
		if (nVirtKey == VK_ESCAPE)
			return TRUE;
		if (nVirtKey == VK_RETURN && (GetFocus()->m_hWnd  == m_ctlMessage.m_hWnd))
		{
			/*if (m_pCurServer->IsOpen())
				OnBtnSend();*/
			return TRUE;
		}
	}
	
	return CDialog::PreTranslateMessage(pMsg);
}

// write message on the Edit Box
void AddMsgToWindow(string strText )
{
   EnterCriticalSection(&Sec_MsgWnd);
	if (NULL == m_ctlMsgListBox)
		return;

   time_t rawtime;
   struct tm * timeinfo;
   char buffer[200];
   time(&rawtime);
   timeinfo = localtime(&rawtime);
   strftime(buffer,80, "%c", timeinfo);
   strcat(buffer, " : ");
   strcat(buffer, (char *)strText.c_str());
   strcat(buffer, "\n");

	HWND hWnd = m_ctlMsgListBox->GetSafeHwnd();
	DWORD dwResult = 0;
	if (SendMessageTimeoutA(hWnd, WM_GETTEXTLENGTH, 0, 0, SMTO_NORMAL, 1000L, &dwResult) != 0)
	{
		int nLen = (int) dwResult;
		if (SendMessageTimeoutA(hWnd, EM_SETSEL, nLen, nLen, SMTO_NORMAL, 1000L, &dwResult) != 0)
		{
			if (SendMessageTimeoutA(hWnd, EM_REPLACESEL, FALSE, (LPARAM)buffer, SMTO_NORMAL, 1000L, &dwResult) != 0)
			{
			}
		}
	}

   LeaveCriticalSection(&Sec_MsgWnd);
   return;
} // void

// external pointer on the edit box and progress bar
void SendMessageWindowControl(CEdit * pMsgCtrl,CProgressCtrl *pCtrlProgressSQL)
{
   m_ctlMsgListBox = pMsgCtrl;
   p_ProgressCtrlSQL = pCtrlProgressSQL;
}

BOOL CSPSDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

   InitializeCriticalSection(&Sec_MsgWnd);

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
			pSysMenu->EnableMenuItem(SC_CLOSE,MF_BYCOMMAND  |MF_GRAYED   ); 
		}
        strAboutMenu.ReleaseBuffer();
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// SPX menu MFC
	mmenu = GetMenu();

   #if defined(SITE)
      mmenu->EnableMenuItem(ID_MODE_SIMULATION, MF_GRAYED);
   #else
      mmenu->EnableMenuItem(ID_MODE_PRODUCTION, MF_GRAYED);
   #endif

   // Progress bar control init value
   mProgressCtrlSQL.SetBarColor(RGB(0,255,0));
   mProgressCtrlSQL.SetRange(0,100);
   mProgressCtrlSQL.SetPos(0);

   // Assign an external pointer on the message list box
   SendMessageWindowControl(& m_ctlMsgList, &mProgressCtrlSQL);
   AddMsgToWindow("L-Top Server is started. Please select one mode: simulation/production.");

   return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSPSDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
void CSPSDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSPSDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

// Clear display message window
void CSPSDlg::OnBnClickedBtnClear()
{
	// Clear display
	m_ctlMsgList.SetWindowText( _T("") );
}

void CSPSDlg::ExitMFCApp()
{
   // same as double-clicking on main window close box
   ASSERT(AfxGetMainWnd() != NULL);
   AfxGetMainWnd()->SendMessage(WM_CLOSE);
}

void CSPSDlg::OnLtopExit()
{
   string sub_name = "CSPSDlg::OnLtopExit";

   int msgboxID = ::MessageBoxA(NULL,  "Are you sure you want to quit the application?", "EXIT",  MB_YESNO);
   switch (msgboxID)
    {
       case IDNO:
          // Do nothing
           break;

       case IDYES:
	      OnModelStop();
	      Sleep(2000);
	      ExitMFCApp();

         ns_STD::cl_TRC::TRC_Write("INF", "SYS", sub_name, "EXIT on user requirement");   
         ExitProcess(1);
         break;
    }
}

void CSPSDlg::OnModeSimulation()
{
   string sub_name = "CSPSDlg::OnModeSimulation";
	bool status;

   AddMsgToWindow("Simulation mode activated.");

	SIMULATION = true;

	// Create SQL Thread 
	d_eventStopSQL = CreateEventA(NULL, FALSE, FALSE, "event_StopSQL");
	m_SQL = new Cl_SQL_Thread;
	m_SQL->Create_SQL_Thread();
	AddMsgToWindow("SQL Thread started.");
	Sleep(1000);
   ns_STD::cl_TRC::TRC_Write("INF", "SYS", sub_name, "START LTOP in Simulation mode");   

	status = INIT();
   if (!status) ns_STD::cl_TRC::ERR_Write("INI", sub_name, "error in INIT at Simulation start");

   status = SIM_Main();
   if (!status) ns_STD::cl_TRC::ERR_Write("INI", sub_name, "error in SIM_Main at Simulation start");

	// Create HMI Thread 
	d_eventStopHMI = CreateEventA(NULL, FALSE, FALSE, "event_StopHMI");
   m_HMI = new Cl_HMI_Thread;
   m_HMI->Create_HMI_Thread();
	AddMsgToWindow("HMI Thread started.");
	Sleep(1000);

	// Create SPG Thread 
	d_eventStopSPG = CreateEventA(NULL, FALSE, FALSE, "event_StopSPG");
   m_SPG = new Cl_SPG_Thread;
   m_SPG->Create_SPG_Thread();
	AddMsgToWindow("SPG Thread started.");
	Sleep(1000);

	// Create MAIN Thread 
	d_eventStopMAIN = CreateEventA(NULL, FALSE, FALSE, "event_StopMainThread");
	m_MAIN = new Cl_MAIN_Thread;
	m_MAIN->Create_MAIN_Thread();
	AddMsgToWindow("MAIN Thread started.");
	Sleep(1000);

	// Create SIM Thread 
   d_eventStopSIM = CreateEventA(NULL, FALSE, FALSE, "event_StopSim");
	m_SIM = new Cl_SIM_Thread;
	m_SIM->Create_SIM_Thread();  
	AddMsgToWindow("SIM Thread started.");
	Sleep(1000);
}

void CSPSDlg::OnModeProduction()
{
   string sub_name = "CSPSDlg::OnModeProduction";
	bool status;

   AddMsgToWindow("Production mode activated.");

	// Stop SIM thread
	SetEvent( d_eventStopSIM );
	SIMULATION = false;

	d_eventStopSQL = CreateEventA(NULL, FALSE, FALSE, "event_StopSQL");
	m_SQL = new Cl_SQL_Thread;
	m_SQL->Create_SQL_Thread();
	Sleep(250);
   ns_STD::cl_TRC::TRC_Write("INF", "SYS", sub_name, "START LTOP in Production mode");   

	status = INIT();
   if (!status) ns_STD::cl_TRC::ERR_Write("INI", sub_name, "error in INIT at Production start");

	d_eventStopHMI = CreateEventA(NULL, FALSE, FALSE, "event_StopHMI");
   m_HMI = new Cl_HMI_Thread;
   m_HMI->Create_HMI_Thread();
	Sleep(250);

	d_eventStopSPG = CreateEventA(NULL, FALSE, FALSE, "event_StopSPG");
   m_SPG = new Cl_SPG_Thread;
   m_SPG->Create_SPG_Thread();
	Sleep(250);

	// Create Main Thread 
	// ===================
	d_eventStopMAIN = CreateEventA(NULL, FALSE, FALSE, "event_StopMainThread");
	m_MAIN = new Cl_MAIN_Thread;
	m_MAIN->Create_MAIN_Thread();
	Sleep(250);
}

void CSPSDlg::OnModelStop()
{
	AddMsgToWindow("Main Thread stopped by the user.");

	/*-- generate event to stop Main Thread --*/
   if (SIMULATION) SetEvent( d_eventStopSIM );
	SetEvent( d_eventStopSQL );
   SetEvent( d_eventStopHMI );
   SetEvent( d_eventStopSPG );
	SetEvent( d_eventStopMAIN );
   DeleteCriticalSection(&Sec_MsgWnd);
}

