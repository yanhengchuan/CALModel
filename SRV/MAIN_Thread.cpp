/*============================================================================
/ MAIN_Thread.cpp: Main Thread of the programm.
/ 
/ Revisions:
/  Created:       2010-Jan       X.Fournaud
/  Modified: 
/      
/ (c) CMI Industry
//===========================================================================*/
#include "stdafx.h"
#include <comdef.h>

static time_t lst_cmp_tim=0;
long cmp_Dtim=2;

// ======================================================
//  Constructor & destructor for Cl_MAIN_Thread
// ======================================================
Cl_MAIN_Thread::Cl_MAIN_Thread(void):m_handle_MAIN_Thread(NULL)
{
}

Cl_MAIN_Thread::~Cl_MAIN_Thread(void)
{
}

// ==========================================================================
//   
// ==========================================================================
void Cl_MAIN_Thread::Create_MAIN_Thread()
{
	HANDLE hmainThread;
	UINT uiMainThreadId = 0;
	hmainThread = (HANDLE)_beginthreadex(NULL,	      // Security attributes
												0,             	// stack
												MAIN_Proc,	      // Thread proc
												this,	            // Thread param
												CREATE_SUSPENDED,	// creation mode
												&uiMainThreadId);	// Thread ID
	if ( NULL != hmainThread)
	{
		ResumeThread( hmainThread );
		m_handle_MAIN_Thread = hmainThread;
	}
}

// ==========================================================================
//   
// ==========================================================================
UINT WINAPI Cl_MAIN_Thread::MAIN_Proc(LPVOID pParam)
{
	Cl_MAIN_Thread* pThis = reinterpret_cast<Cl_MAIN_Thread*>( pParam );
	_ASSERTE( pThis != NULL );
	pThis->MAIN_Thread();
	return 1L;
} 

// ==========================================================================
//   
// ==========================================================================
void Cl_MAIN_Thread::MAIN_Thread()
{
   bool status=true;
   string sub_name="MAIN_Thread";
   static bool once = false;

	do
	{
      bool MsgCoil = false;
      // Read messages to LTOP
		status = Specific::ThreadMsgTreatment(&MsgCoil);

      if (MsgCoil)
      {
			MessageSPGtoLtop();
         once = true;
      }

      if ( !once ) return;
      MessageSPStoLtop();

      status = SPS_Sequence::SPS_Compute();

      if (!status)
		{
			for (int i_fur = 0; i_fur < MAX_NB_FUR; i_fur++)
			{
				sps_act_sp[i_fur].ctl_pow = false;
				sps_act_sp[i_fur].Spd_Valid = false;
				sps_act_sp[i_fur].Valid = false;
				if (i_fur < RTH + NB_FUR_RTF)
					for ( int i_zon=0; i_zon < fur_cfg[i_fur].nb_zon; i_zon++ )
						sps_act_sp[i_fur].tmp_zon[i_zon] = TUB_SPT_STOP;
			}
		}

      // Store set points before sending
		MessageFromLtop();

   } while( WaitForSingleObject( d_eventStopMAIN, TIME_PAUSE_MAIN ));

	MessageBoxA(NULL, "Arret du thread MAIN", "Stop", 0x40);
   ns_STD::cl_TRC::ERR_Write("ALM", sub_name, "MAIN Thread stopped");

	CloseHandle( d_eventStopMAIN );
	d_eventStopMAIN = NULL;	
}

// ===============
// Stop Supervisor 
// ===============
void Cl_MAIN_Thread::Stop_MAIN_Thread()
{
	// Kill Thread
	if (NULL != m_handle_MAIN_Thread)
	{
		if (WaitForSingleObject(m_handle_MAIN_Thread, 5000L) == WAIT_TIMEOUT)
			TerminateThread(m_handle_MAIN_Thread, 1L);

		CloseHandle(m_handle_MAIN_Thread);
		m_handle_MAIN_Thread = NULL;
	}
}

// =================
// Computation time
// =================
bool CMP_Cycle(time_t current_time) 
{
   if ( difftime(current_time, lst_cmp_tim) >= cmp_Dtim )
   {
      lst_cmp_tim = current_time;
      return true;
   }

   return false;
}
