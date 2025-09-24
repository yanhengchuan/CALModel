/*============================================================================
/ SIM_Thread.cpp
/ 
/ Supervision des communications .
/ 
/ Revisions:
/  Created:       2007-Mai       X.Fournaud
/  Modified: 
/      
/ (c) CMI Industry
//===========================================================================*/
#include "stdafx.h"

// ======================================================
//  Constructeur et destructeur de CSupervisor
// ======================================================
Cl_SIM_Thread::Cl_SIM_Thread(void):m_handle_SIM_Thread(NULL)
{
}

Cl_SIM_Thread::~Cl_SIM_Thread(void)
{
}

// ==========================================================================
// Create thread for com OPC
// ==========================================================================
void Cl_SIM_Thread::Create_SIM_Thread()
{
	HANDLE hSimThread;
	UINT uiSimThreadId = 0;
	hSimThread = (HANDLE)_beginthreadex(NULL,	// Security attributes
												0,	// stack
												SIM_Proc,	// Thread proc
												this,	// Thread param
												CREATE_SUSPENDED,	// creation mode
												&uiSimThreadId);	// Thread ID
	if ( NULL != hSimThread)
	{
		ResumeThread( hSimThread );
		m_handle_SIM_Thread = hSimThread;

      InitializeCriticalSection(&Sec_SIM_Proc0);	
	}
}
// ==========================================================================
//   
// ==========================================================================
UINT WINAPI Cl_SIM_Thread::SIM_Proc(LPVOID pParam)
{
	Cl_SIM_Thread* pThis = reinterpret_cast<Cl_SIM_Thread*>( pParam );
	_ASSERTE( pThis != NULL );
	pThis->SIM_Thread();
	return 1L;
} 

// ==========================================================================
//   
// ==========================================================================
void Cl_SIM_Thread::SIM_Thread()
{
   do
	{
		EnterCriticalSection(&Sec_SIM_Proc0);
		SIM_Main();		
		LeaveCriticalSection(&Sec_SIM_Proc0); 
	} while( WaitForSingleObject( d_eventStopSIM, TIME_PAUSE_SIM ));

	//MessageBoxA(NULL, "Arret du thread SIM", "Stop", 0x40);
   ns_STD::cl_TRC::ERR_Write("ALM", "SIM_Thread", "SIM Thread stopped");
	CloseHandle( d_eventStopSIM );
	d_eventStopSIM = NULL;	
}

// ======================================================
// Stop Supervisor com
// ======================================================
void Cl_SIM_Thread::Stop_SIM_Thread()
{
	// Kill Thread
	if (NULL != m_handle_SIM_Thread)
	{
		if (WaitForSingleObject(m_handle_SIM_Thread, 5000L) == WAIT_TIMEOUT)
			TerminateThread(m_handle_SIM_Thread, 1L);

		CloseHandle(m_handle_SIM_Thread);
		m_handle_SIM_Thread = NULL;

      DeleteCriticalSection(&Sec_SIM_Proc0);
	}
}