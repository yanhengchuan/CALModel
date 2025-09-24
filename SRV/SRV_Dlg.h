/*============================================================================
/ SRV_Dlg: Implementation file.
/
/ Revisions:
/  Created:       2010-Apr       X.Fournaud
/  Modified: 
/  
/ (c) CMI Industry
//===========================================================================*/
#include "resource.h"

extern Cl_SQL_Thread *m_SQL;
extern HANDLE d_eventStopSQL;
extern DWORD HandleSQLThread;

extern Cl_SPG_Thread *m_SPG;
extern HANDLE d_eventStopSPG;

extern Cl_HMI_Thread *m_HMI;
extern HANDLE d_eventStopHMI;

extern Cl_SIM_Thread *m_SIM;
extern HANDLE d_eventStopSIM;

extern Cl_MAIN_Thread *m_MAIN;
extern HANDLE d_eventStopMAIN;

extern CProgressCtrl *p_ProgressCtrlSQL;
extern CEdit *m_ctlMsgListBox;

#if !defined(AFX_LtopDlg_H__BF055235_5494_4FFC_8289_20DBFD9503A8__INCLUDED_)
#define AFX_LtopDlg_H__BF055235_5494_4FFC_8289_20DBFD9503A8__INCLUDED_

class CSPSDlg : public CDialog
{
   // Construction
   public:
	   CSPSDlg(CWnd* pParent = NULL);	// standard constructor

   // Dialog Data
	   enum { IDD = IDD_SERVER_DIALOG };
   
	   CEdit	m_ctlMessage;
	   CEdit	m_ctlMsgList;

	   // ClassWizard generated virtual function overrides
	   public:
	   virtual BOOL PreTranslateMessage(MSG* pMsg);

	   protected:
	   virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

   // Implementation
   protected:
	   HICON m_hIcon;

	   // Generated message map functions
	   virtual BOOL OnInitDialog();
	   afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	   afx_msg void OnPaint();
	   afx_msg HCURSOR OnQueryDragIcon();
	
	   DECLARE_MESSAGE_MAP()
   public:

	   afx_msg void OnBnClickedBtnClear();
	   afx_msg void OnLtopExit();

	   afx_msg void OnModeSimulation();
	   afx_msg void OnModeProduction();
	
	   afx_msg void OnModelStop();

	   CProgressCtrl mProgressCtrlSQL;
	   void ExitMFCApp();
};

#endif // !defined(AFX_LtopDlg_H__BF055235_5494_4FFC_8289_20DBFD9503A8__INCLUDED_)
