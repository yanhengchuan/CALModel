/*============================================================================
/ SQL_Thread.cpp: Get message from MainThread and write the data into the DB.
/ 
/ Revisions:
/  Created:       2010-Feb       X.Fournaud
/  Modified: 
/      
/ (c) CMI Industry
//===========================================================================*/
#include "stdafx.h"
#include "SQL.h"

CProgressCtrl *p_ProgressCtrlSQL;

// ==========================================================================
//   
// ==========================================================================
void Cl_SQL_Thread::SQL_Thread()
{
	MSG msg;
	bool status = false;
	bool status_sql = false;

	string sub_name="SQL_Thread";

	do
	{
		if( PeekMessage(&msg, NULL, WM_USER, WM_USER+2000, PM_REMOVE) )
		{
			// Main SQL Update
			if (msg.message == MSG_SQL_UPD)
			{ 
				SQL_TRACKING_UPDATE();
				status_sql = SQL_COILS_DATA_UPDATE(); 
				if ( !status_sql ) ns_STD::cl_TRC::ERR_Write("SQL", sub_name, "error in SQL_COILS_DATA_UPDATE");
			} // Main SQL Update

			else if (msg.message == MSG_SQL_SPG)
			{ 
            for (int i_rtf=0; i_rtf < NB_FUR_RTF; i_rtf++) SQL_SPG_RTF_UPDATE(i_rtf);
			}

			else if (msg.message == MSG_SQL_SPS)
			{ 
				for (int i_rtf=0; i_rtf < NB_FUR_RTF; i_rtf++) SQL_SPS_RTF_UPDATE(i_rtf);
				for (int i_fur=NB_FUR_RTF; i_fur < MAX_NB_FUR; i_fur++) SQL_SPS_FUR_UPDATE(i_fur);
			}

         else if (msg.message == MSG_SQL_REP)
			{ 
				//status_sql = SQL_Ins_His_Report();
				//if ( !status_sql ) ERR_Write("SQL", sub_name.c_str(), "error in SQL_Ins_His_Report");
			}

 			else if (msg.message == MSG_SQL_HIS)
			{ 
				// Store current measuremenet and set point for history graphic
				SQL_HIS_INSERT();
			}
			else if (msg.message == MSG_SQL_FUR_SPT)
			{
				SQL2_I_MSG_SPT_HEA();
			}
			else if(msg.message == MSG_SQL_LIN_SPT)
			{
				SQL2_I_MSG_SPT_SPD();
			}
      }
	} while( WaitForSingleObject( d_eventStopSQL, TIME_PAUSE_SQL ));

	//MessageBoxA(NULL, "Arret du thread SQL", "Stop", 0x40);
	CloseHandle( d_eventStopSQL );
	d_eventStopSQL = NULL;	

   return;
} // void

// ================================== ====================
//  Constructeur et destructeur de CSupervisor
// ======================================================
Cl_SQL_Thread::Cl_SQL_Thread(void):m_handle_SQL_Thread(NULL)
{
	SQL_ConnectServer();
}

Cl_SQL_Thread::~Cl_SQL_Thread(void)
{
}

// ======================================================
// Create thread for SQL
// ======================================================
void Cl_SQL_Thread::Create_SQL_Thread()
{
	HANDLE hSQLThread;
	UINT uiSQLThreadId = 0;
	hSQLThread = (HANDLE)_beginthreadex(NULL,	   // Security attributes
										0,	               // stack
										SQL_Proc,	      // Thread proc
										this,	            // Thread param
										CREATE_SUSPENDED,	// creation mode
										&uiSQLThreadId);	// Thread ID
	if ( NULL != hSQLThread)
	{
		ResumeThread( hSQLThread );
		m_handle_SQL_Thread = hSQLThread;
		HandleSQLThread = uiSQLThreadId;

      InitializeCriticalSection(&Sec_SQL_Proc0);
      InitializeCriticalSection(&Sec_SQL_Proc1);
      InitializeCriticalSection(&Sec_SQL_Proc2);
      InitializeCriticalSection(&Sec_SQL_Proc3);
	}
}
// ==========================================================================
//   
// ==========================================================================
UINT WINAPI Cl_SQL_Thread::SQL_Proc(LPVOID pParam)
{
	Cl_SQL_Thread* pThis = reinterpret_cast<Cl_SQL_Thread*>( pParam );
	_ASSERTE( pThis != NULL );
	pThis->SQL_Thread();
	return 1L;
} 

// ======================================================
// Stop Supervisor com
// ======================================================
void Cl_SQL_Thread::Stop_SQL_Thread()
{
	// Kill Thread
	if (NULL != m_handle_SQL_Thread)
	{
		if (WaitForSingleObject(m_handle_SQL_Thread, 5000L) == WAIT_TIMEOUT)
			TerminateThread(m_handle_SQL_Thread, 1L);

		CloseHandle(m_handle_SQL_Thread);
		m_handle_SQL_Thread = NULL;

      DeleteCriticalSection(&Sec_SQL_Proc0);
      DeleteCriticalSection(&Sec_SQL_Proc1);
      DeleteCriticalSection(&Sec_SQL_Proc2);
      DeleteCriticalSection(&Sec_SQL_Proc3);
	}
}