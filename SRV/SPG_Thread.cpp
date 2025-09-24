/*============================================================================
/ SRV_SPG_Thread: This thread computes the SPG.
/ 
/ Revisions:
/  Created:       2016-Avr.       JC.Mitais
/  Modified: 
/  
/ (c) CMI Industry
//===========================================================================*/
#include "stdafx.h"
extern bool STARTUP;
CRITICAL_SECTION Sec_SPG_Strip;

// ======================================================
//  Constructeur et destructeur de CTSupervisor
// ======================================================
Cl_SPG_Thread::Cl_SPG_Thread(void):m_handle_SPG_Thread(NULL)
{
}

Cl_SPG_Thread::~Cl_SPG_Thread(void)
{
}

// ==========================================================================
//   
// ==========================================================================
void Cl_SPG_Thread::Create_SPG_Thread()
{
	// Create thread 
	UINT uiMainSThreadId = 0;
	HANDLE hmainSThread = (HANDLE)_beginthreadex(NULL,	// Security attributes
												0,	               // stack
												SPG_Proc,	      // Thread proc
												this,	            // Thread param
												CREATE_SUSPENDED,	// creation mode
												&uiMainSThreadId);   // Thread ID
	if ( NULL != hmainSThread)
	{
		ResumeThread( hmainSThread );
		m_handle_SPG_Thread = hmainSThread;

      InitializeCriticalSection(&Sec_SPG_Strip);
	}
}

// ==========================================================================
//   
// ==========================================================================
UINT WINAPI Cl_SPG_Thread::SPG_Proc(LPVOID pParam)
{
	Cl_SPG_Thread* pThis = reinterpret_cast<Cl_SPG_Thread*>( pParam );
	_ASSERTE( pThis != NULL );
	pThis->SPG_Thread();
	return 1L;
} 

// ==========================================================================
//   
// ==========================================================================
void Cl_SPG_Thread::SPG_Thread()
{
	do
	{
      if (s_coil.nb_coil > 0) // test
      {
         // Stable and transient states calculation for RTF
         // -----------------------------------------------
         // RTF stable and transient states set points calculation
         if (!SPG_RTF_Getdata())
         {
            ns_STD::cl_TRC::ERR_Write("SPG", "SPG_Thread", "error in SPG_RTF_Getdata");
         }

         // Post a thread message for SQL treatment
	      PostThreadMessage(HandleSQLThread, MSG_SQL_SPG, 0, 0);
         PostThreadMessage(HandleSQLThread, MSG_SQL_UPD, 0, 0);

         STARTUP = false; 
      }
	   

      if ( !STARTUP )
         WaitForSingleObject( d_eventStopSPG, TIME_PAUSE_SPG );
	} while(true);

   ns_STD::cl_TRC::ERR_Write("ALM", "SPG_Thread", "SPG Thread stopped");
	CloseHandle( d_eventStopSPG );
	d_eventStopSPG = NULL;	
}

// ======================================================
// Stop Supervisor thread
// ======================================================
void Cl_SPG_Thread::Stop_SPG_Thread()
{
	// Kill Thread
	if (NULL != m_handle_SPG_Thread)
	{
		if (WaitForSingleObject(m_handle_SPG_Thread, 5000L) == WAIT_TIMEOUT)
			TerminateThread(m_handle_SPG_Thread, 1L);

		CloseHandle(m_handle_SPG_Thread);
		m_handle_SPG_Thread = NULL;

      DeleteCriticalSection(&Sec_SPG_Strip);
	}
}