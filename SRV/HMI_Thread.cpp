/*============================================================================
/ HMI_Thread: This thread reads periodically the Check_HMI PRS table
/     that contains information coming from the HMI.
/ 
/ Revisions:
/  Created:       2009-Jun.       X.Fournaud
/  Modified: 
/  
/ (c) CMI Industry
//===========================================================================*/
#include "stdafx.h"

// ======================================================
//  Constructeur et destructeur de CTSupervisor
// ======================================================
Cl_HMI_Thread::Cl_HMI_Thread(void):m_handle_HMI_Thread(NULL)
{
}

Cl_HMI_Thread::~Cl_HMI_Thread(void)
{
}

// ==========================================================================
//   
// ==========================================================================
void Cl_HMI_Thread::Create_HMI_Thread()
{
	//create thread 
	HANDLE hmainSThread;
	UINT uiMainSThreadId = 0;
	hmainSThread = (HANDLE)_beginthreadex(NULL,	// Security attributes
												0,	// stack
												HMI_Proc,	// Thread proc
												this,	// Thread param
												CREATE_SUSPENDED,	// creation mode
												&uiMainSThreadId);	// Thread ID
	if ( NULL != hmainSThread)
	{
		ResumeThread( hmainSThread );
		m_handle_HMI_Thread = hmainSThread;
	}
} // void

// ==========================================================================
//   
// ==========================================================================
UINT WINAPI Cl_HMI_Thread::HMI_Proc(LPVOID pParam)
{
	Cl_HMI_Thread* pThis = reinterpret_cast<Cl_HMI_Thread*>( pParam );
	_ASSERTE( pThis != NULL );
	pThis->HMI_Thread();
	return 1L;
} // void

// ==========================================================================
// Check data from HMI
// ==========================================================================
void Cl_HMI_Thread::HMI_Thread()
{
	do
	{
		SQL_CkeckHMI();
	} while( WaitForSingleObject( d_eventStopHMI, TIME_PAUSE_HMI ));

   ns_STD::cl_TRC::ERR_Write("ALM", "HMI_Thread", "HMI Thread stopped");

	CloseHandle( d_eventStopHMI );
	d_eventStopHMI = NULL;	
} // void

// ======================================================
// Stop Supervisor thread
// ======================================================
void Cl_HMI_Thread::Stop_HMI_Thread()
{
	// Kill Thread
	if (NULL != m_handle_HMI_Thread)
	{
		if (WaitForSingleObject(m_handle_HMI_Thread, 5000L) == WAIT_TIMEOUT)
			TerminateThread(m_handle_HMI_Thread, 1L);

		CloseHandle(m_handle_HMI_Thread);
		m_handle_HMI_Thread = NULL;
	}
} // void