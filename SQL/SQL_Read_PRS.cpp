/*=============================================================================
/ SQL_Read_PRS.cpp: Treatment of data read from MS-SQL SERVER Process Database. 
/ 
/ (c) CMI Industry
//=============================================================================*/
#include "stdafx.h"
#include "SQL.h"

// Declare connection
extern Sql *m_sql1;

// ******************************************************************************************************
// SQL_EMISSIVITY_SELECT: Select records from EMISSIVITY Table. 
// 
// Revisions:
//  Created:      2006-Jun   R.Rabemananjara
//  Modified:     2016-May   JC.Mitais   LTOP 3.0
// ******************************************************************************************************
bool SQL_EMISSIVITY_SELECT(int n_rtf)
{
   bool status = true;
   string sub_name="SQL_EMISSIVITY_SELECT";

   string Table = "EMISSIVITY";
   string Procedure = "[" + (string)BD_PROCESS + "].[dbo].[" + Table + "_SELECT]";

   struct str_PrmArrays PrmArr;
   VariantInit(&PrmArr.var); 

   m_sql1->IntProcArray(&PrmArr, "n_rtf", n_rtf);

   _RecordsetPtr recordset = m_sql1->executeResultProcedure(Procedure, PrmArr.vtArr, PrmArr.prmArr, PrmArr.sizArr);
   while(!recordset->ADOEOF)
	{
      try
      {
         int n_cla = recordset->Fields->GetItem(L"n_CLA")->GetValue().intVal;
         int n_pdt = recordset->Fields->GetItem(L"n_PDT")->GetValue().intVal;

         s_adp[n_rtf].s_pdt_RTF[n_cla][n_pdt].alpha = float(recordset->Fields->GetItem(L"ALPHA")->GetValue().dblVal);
      } // try

      // Error control
      // -------------
      catch(_com_error &e)
      {
         SQL_R_ERR(sub_name, Procedure, (string)e.Description());
         return false;
      }

      recordset->MoveNext();
	} // while(!recordset->ADOEOF)

   return status;
}

// ******************************************************************************************************
// SQL_EFFICIENCY_SELECT: Select records from EFFICIENCY Table. 
// 
// Revisions:
//  Created:      2006-Jun   R.Rabemananjara
//  Modified:     2016-May   JC.Mitais   LTOP 3.0
// ******************************************************************************************************
bool SQL_EFFICIENCY_SELECT(int n_rtf)
{
   bool status = true;
   string sub_name="SQL_EFFICIENCY_SELECT";

   string Table = "EFFICIENCY";
   string Procedure = "[" + (string)BD_PROCESS + "].[dbo].[" + Table + "_SELECT]";

   struct str_PrmArrays PrmArr;
   VariantInit(&PrmArr.var); 

   m_sql1->IntProcArray(&PrmArr, "n_rtf", n_rtf);

   _RecordsetPtr recordset = m_sql1->executeResultProcedure(Procedure, PrmArr.vtArr, PrmArr.prmArr, PrmArr.sizArr);
   try
   {
      // For all RTF zones
      for ( int i_zon=0; i_zon < fur_cfg[n_rtf].nb_zon; i_zon++)
      {
         string VarVal = "EFFI_TUB_Z" + to_string(i_zon);
         s_adp[n_rtf].Effi[i_zon] = float(recordset->Fields->GetItem(VarVal.c_str())->GetValue().dblVal);
      } // For all RTF zones
   } // try

   // Error control
   // -------------
   catch(_com_error &e)
   {
      SQL_R_ERR(sub_name, Procedure, (string)e.Description());
      return false;
   }

   recordset->MoveNext();

   return status;
} // void

// ******************************************************************************************************
// SQL_ADAPTATION_SELECT: Select records from ADAPTATION Table. 
// 
// Revisions:
//  Created:      2006-Jun   R.Rabemananjara
//  Modified:     2016-May   JC.Mitais   LTOP 3.0
// ******************************************************************************************************
bool SQL_ADAPTATION_SELECT(int n_rtf)
{
   bool status = true;
   string sub_name="SQL_ADAPTATION_SELECT";

   string Table = "ADAPTATION";
   string Procedure = "[" + (string)BD_PROCESS + "].[dbo].[" + Table + "_SELECT]";

   struct str_PrmArrays PrmArr;
   VariantInit(&PrmArr.var); 

   m_sql1->IntProcArray(&PrmArr, "n_rtf", n_rtf);

   _RecordsetPtr recordset = m_sql1->executeResultProcedure(Procedure, PrmArr.vtArr, PrmArr.prmArr, PrmArr.sizArr);
   if (!recordset->ADOEOF)
	{
      try
      {
         s_adp[n_rtf].s_exg.Alpha = float(recordset->Fields->GetItem(L"ALPHA_EXC")->GetValue().dblVal);
         s_adp[n_rtf].s_ant.Alpha = float(recordset->Fields->GetItem(L"ALPHA_ANT")->GetValue().dblVal);
      } // try

      // Error control
      // -------------
      catch(_com_error &e)
      {
         SQL_R_ERR(sub_name, Procedure, (string)e.Description());
         return false;
      }
	} // if (!recordset->ADOEOF)

   return status;
} // void

// ******************************************************************************************************
// SQL_PID_SELECT: Select records from PID Table. 
// 
// Revisions:
//  Created:      2006-Jun   R.Rabemananjara
//  Modified:     2016-May   JC.Mitais   LTOP 3.0
// ******************************************************************************************************
bool SQL_PID_SELECT(int n_fur)
{
   bool status = true;
   string sub_name="SQL_PID_SELECT";

   string Table = "PID";
   string Procedure = "[" + (string)BD_PROCESS + "].[dbo].[" + Table + "_SELECT]";

   struct str_PrmArrays PrmArr;
   VariantInit(&PrmArr.var); 

   m_sql1->IntProcArray(&PrmArr, "n_fur", n_fur);

   _RecordsetPtr recordset = m_sql1->executeResultProcedure(Procedure, PrmArr.vtArr, PrmArr.prmArr, PrmArr.sizArr);
   if (!recordset->ADOEOF)
	{
      try
      {
		   fba_stp[n_fur].automatic = (recordset->Fields->GetItem(L"PID_AUTO_STP")->GetValue().boolVal != 0);
		   fba_stp[n_fur].p    = float(recordset->Fields->GetItem(L"KP_STP")->GetValue().dblVal);
		   fba_stp[n_fur].i    = float(recordset->Fields->GetItem(L"KI_STP")->GetValue().dblVal);
		
			// For all zones
         for (int i_zon=0; i_zon < fur_cfg[n_fur].nb_zon; i_zon++)
			{
            string VarVal = "PID_AUTO_Z" + to_string(i_zon);
				fba_zon[n_fur][i_zon].automatic = (recordset->Fields->GetItem(VarVal.c_str())->GetValue().boolVal != 0);

            VarVal = "KP_Z" + to_string(i_zon);
				fba_zon[n_fur][i_zon].p = float(recordset->Fields->GetItem(VarVal.c_str())->GetValue().dblVal);

            VarVal = "KI_Z" + to_string(i_zon);
				fba_zon[n_fur][i_zon].i = float(recordset->Fields->GetItem(VarVal.c_str())->GetValue().dblVal);
			} // For all zones
      } // try
					 
      // Error control
      // -------------
      catch(_com_error &e)
      {
         SQL_R_ERR(sub_name, Procedure, (string)e.Description());
         return false;
      }
	} // if (!recordset->ADOEOF)

   return status;
} // void

// ******************************************************************************************************
// SQL_PRM_SELECT: Select records from PRM Table. 
// 
// Revisions:
//  Created:      2006-Jun   R.Rabemananjara
//  Modified:     2016-May   JC.Mitais   LTOP 3.0
// ******************************************************************************************************
bool SQL_PRM_SELECT(int n_rtf)
{
   bool status = true;
   string sub_name="SQL_PRM_SELECT";

   string Table = "PRM";
   string Procedure = "[" + (string)BD_PROCESS + "].[dbo].[" + Table + "_SELECT]";

   struct str_PrmArrays PrmArr;
   VariantInit(&PrmArr.var); 

   m_sql1->IntProcArray(&PrmArr, "n_rtf", n_rtf);

   _RecordsetPtr recordset = m_sql1->executeResultProcedure(Procedure, PrmArr.vtArr, PrmArr.prmArr, PrmArr.sizArr);
   if (!recordset->ADOEOF)
	{
      try
      {
		   sps_entry[n_rtf].s_mea.restart_risk_lim = float(recordset->Fields->GetItem(L"RISK_LIMIT")->GetValue().dblVal);
      } // try

      // Error control
      // -------------
      catch(_com_error &e)
      {
         SQL_R_ERR(sub_name, Procedure, (string)e.Description());
         return false;
      }
	} // if (!recordset->ADOEOF)

   return status;
} // void

// ==============================================================
// SELECT SQL_CkeckHMI
// ==============================================================
bool SQL_CkeckHMI ()
{
   bool status = true;
   string sub_name="SQl_CheckHMI";

   string Table = "Check_HMI";
   string Procedure = "[" + (string)BD_PROCESS + "].[dbo].[" + Table + "_SELECT]";

   bool KpKi=false, PRM=false;
   //float SpdMax;

   _RecordsetPtr recordset = m_sql1->executeResultProcedure(Procedure);
   if (!recordset->ADOEOF)
	{
      try
      {
         KpKi            = (recordset->Fields->GetItem(L"KpKi_Change")->GetValue().boolVal != 0);
		   s_cur.record_IDX = recordset->Fields->GetItem(L"Record_IDX")->GetValue().intVal;
		   PRM             = (recordset->Fields->GetItem(L"PRM")->GetValue().boolVal != 0);
		   s_spe_PRO.s_mea.Spd_coef_ope = float(recordset->Fields->GetItem(L"SpdMax")->GetValue().dblVal);    // As requested by cutomer this is coefficient (0 --> 1) to reduce the Speed Set Point
      } // try

      // Error control
      // -------------
      catch(_com_error &e)
      {
         SQL_R_ERR(sub_name, Procedure, (string)e.Description());
         return false;
      }
	} // if (!recordset->ADOEOF)

   // Need to read table? 
   // ===================
   if ( KpKi ) for (int i_fur=0; i_fur < MAX_NB_FUR; i_fur++) 
      SQL_PID_SELECT(i_fur);
   if ( PRM ) for (int i_rtf=0; i_rtf < NB_FUR_RTF; i_rtf++) 
      SQL_PRM_SELECT(i_rtf);
   if ( KpKi || PRM ) 
      SQL_Upd_CkeckHMI();

   return status;
} // void

// ******************************************************************************************************
// SQL_SIM_SELECT: Select coils data for Simulation. 
// 
// Revisions:
//  Created:      2017-Apr   Sh. Mechiev
//  Modified:     
// ******************************************************************************************************
bool SQL_SIM_SELECT()
{
   bool status = true;
   string sub_name="SQL_SIM_SELECT";

   string Table = "SIM";
   string Procedure = "[" + (string)BD_PROCESS + "].[dbo].[" + Table + "_SELECT]";
   int i_coil = 0;
   s_sim.nb_coil = 0;

   struct str_PrmArrays PrmArr;
   VariantInit(&PrmArr.var); 

   m_sql1->IntProcArray(&PrmArr, "nb_coils", MAX_NB_COIL);

   _RecordsetPtr recordset = m_sql1->executeResultProcedure(Procedure, PrmArr.vtArr, PrmArr.prmArr, PrmArr.sizArr);
   while (!recordset->ADOEOF)
	{
      try
      {
         s_sim.s_coil_data[i_coil].id     = _bstr_t(recordset->Fields->GetItem(L"ID")->GetValue().bstrVal);
         s_sim.s_coil_data[i_coil].cycle   =        recordset->Fields->GetItem(L"CYCLE")->GetValue().intVal;
         s_sim.s_coil_data[i_coil].dummy   =        recordset->Fields->GetItem(L"DUMMY_COIL")->GetValue().intVal;
         s_sim.s_coil_data[i_coil].lgt     = (float)recordset->Fields->GetItem(L"LGT")->GetValue().dblVal;
         s_sim.s_coil_data[i_coil].thi     = (float)recordset->Fields->GetItem(L"THI")->GetValue().dblVal;
         s_sim.s_coil_data[i_coil].wid     = (float)recordset->Fields->GetItem(L"WID")->GetValue().dblVal;
         s_sim.s_coil_data[i_coil].spd_min = (float)recordset->Fields->GetItem(L"SPD_MIN")->GetValue().dblVal;
         s_sim.s_coil_data[i_coil].spd_max = (float)recordset->Fields->GetItem(L"SPD_MAX")->GetValue().dblVal;
         s_sim.s_coil_data[i_coil].steeltype =      recordset->Fields->GetItem(L"STEEL_TYPE")->GetValue().intVal;
         s_sim.s_coil_data[i_coil].N_cla   = recordset->Fields->GetItem(L"N_CLA")->GetValue().intVal;
         s_sim.s_coil_data[i_coil].N_pdt   = recordset->Fields->GetItem(L"N_PDT")->GetValue().intVal;
         //s_sim.s_coil_data[i_coil].Bisra   =        recordset->Fields->GetItem(L"BISRA")->GetValue().intVal;
		
			// For all sections
         for (int i_fur=0; i_fur < MAX_NB_FUR; i_fur++)
			{
            string VarVal = "TMP_TGT_" + fur_cfg[i_fur].fur_name;
				s_sim.s_coil_data[i_coil].tmp_tgt[i_fur] = (float)recordset->Fields->GetItem(VarVal.c_str())->GetValue().dblVal + TK;

            VarVal = "TMP_LOW_" + fur_cfg[i_fur].fur_name;
				s_sim.s_coil_data[i_coil].tmp_low[i_fur] = (float)recordset->Fields->GetItem(VarVal.c_str())->GetValue().dblVal + TK;

            VarVal = "TMP_UPP_" + fur_cfg[i_fur].fur_name;
				s_sim.s_coil_data[i_coil].tmp_upp[i_fur] = (float)recordset->Fields->GetItem(VarVal.c_str())->GetValue().dblVal + TK;
			} // For all sections

         if (s_sim.s_coil_data[i_coil].id != "")
            s_sim.nb_coil++;

         recordset->MoveNext();
         i_coil++;
      } // try
					 
      // Error control
      // -------------
      catch(_com_error &e)
      {
         SQL_R_ERR(sub_name, Procedure, (string)e.Description());
         return false;
      }
	} // while (!recordset->ADOEOF)

   return status;
} // void