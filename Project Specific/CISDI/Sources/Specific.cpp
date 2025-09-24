/*==========================================================================
/ Specific:  All specific application of a business.
/
/ Revisions:
/  Created:       2007-Apr      L. Lâm Ngoc
/  Modified: 
/  
/ (c) CMI Industry
/===========================================================================*/
#include "stdafx.h"
#include "SQL.h"

// ========================================
// Read MESSAGE flags
// ========================================
bool Specific::ThreadMsgTreatment(bool *MsgCoil)
{
	bool status=true;
	string sub_name="ThreadMsgTreatment";

   // Declare connection
   extern Sql *m_sql2;

   static bool restart=true;

	_RecordsetPtr pRecordSet;  // recordset object pointer
	bool coil_queue = false;

	string ProcedureR = "SELECT TOP 1 [Ack] FROM [dbo].[MSG_COIL] ORDER BY [msg_tim] DESC";
	pRecordSet = m_sql2->executeQuery(ProcedureR);
	if ( pRecordSet != NULL )
	{
		coil_queue = (pRecordSet->Fields->GetItem(L"Ack")->GetValue().boolVal == 0);
	}

	status = SQL2_R_MEA_HEA();
	status = SQL2_R_MEA_SPD();

   if (restart || coil_queue)
   {
      status = SQL2_R_MSG_COIL();

	   string ProcedureW = "UPDATE [dbo].[MSG_COIL] SET [Ack] = 1 WHERE [Ack] = 0";
		pRecordSet = m_sql2->executeQuery(ProcedureW);
      restart = false;
      *MsgCoil = true;

   }

   // For all RTF furnaces
   for (int i_fur=0; i_fur < MAX_NB_FUR; i_fur++)
   {
      for (int i_zon=0; i_zon < fur_cfg[i_fur].nb_zon; i_zon++) 
         sps_entry[i_fur].s_mea.on_zone[i_zon] = s_fur_mea[i_fur].ctl;
   } // For all RTF furnaces
   
	return status;
} // void

// ========================================
// Specific number of soaking pass
// ========================================
float Specific::Pyrometer (struct rtf_struct *rtf)
{
   string sub_name = "Specific::Pyrometer";
   float Tmp_Pyro = 0.f;

   return Tmp_Pyro;
}

