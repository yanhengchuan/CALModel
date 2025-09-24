/*=============================================================================
/ SQL_Read_COM.cpp: Treatment of data read in MS-SQL SERVER Communications Database. 
/ 
/ (c) CMI Industry
//=============================================================================*/
#include "stdafx.h"
#include "SQL.h"

// Declare connection
extern Sql *m_sql2;

// ******************
// Read COIL messages
// ******************
bool SQL2_R_MSG_COIL()
{
   bool status = true;
   string sub_name="SQL2_R_MSG_COIL";

   string Table = "MSG_COIL";
   string Procedure = "[" + (string)BD_COMMUNICATIONS + "].[dbo].[" + Table + "_SELECT]";
   _RecordsetPtr recordset = m_sql2->executeResultProcedure(Procedure);
   int nb_records = recordset->RecordCount;

   COleDateTime Coil_Ole_tim;
   int i_coil=0, n_coils = 0;

	while (!recordset->ADOEOF && i_coil < nb_records)
	{
      try
      {
         Coil_Ole_tim =                       recordset->Fields->GetItem(L"msg_tim")->GetValue().date;
         s_line_coil[i_coil].id =     _bstr_t(recordset->Fields->GetItem(L"COIL_ID")->GetValue().bstrVal);

         s_line_coil[i_coil].thi     = (float)recordset->Fields->GetItem(L"THI")->GetValue().dblVal / 1000.f;
         s_line_coil[i_coil].wid     = (float)recordset->Fields->GetItem(L"WID")->GetValue().dblVal / 1000.f;
         s_line_coil[i_coil].lgt     = (float)recordset->Fields->GetItem(L"LGT")->GetValue().dblVal;
         s_line_coil[i_coil].spd_max = (float)recordset->Fields->GetItem(L"SPD_MAX_RUNNING")->GetValue().dblVal;
         s_line_coil[i_coil].spd_min = (float)recordset->Fields->GetItem(L"SPD_MIN_RUNNING")->GetValue().dblVal;
         s_line_coil[i_coil].steeltype      = recordset->Fields->GetItem(L"STEEL_TYPE")->GetValue().intVal;
         s_line_coil[i_coil].Prev_roll_mill = recordset->Fields->GetItem(L"ROLLING_MILL_NUMBER")->GetValue().intVal;
         s_line_coil[i_coil].dummy          = recordset->Fields->GetItem(L"DUMMY_COIL")->GetValue().intVal;

         s_line_coil[i_coil].tmp_tgt[RTH] = (float)recordset->Fields->GetItem(L"TMP_TGT_RTF")->GetValue().dblVal;
			s_line_coil[i_coil].tmp_upp[RTH] = (float)recordset->Fields->GetItem(L"TMP_UPP_RTF")->GetValue().dblVal;
         s_line_coil[i_coil].tmp_low[RTH] = (float)recordset->Fields->GetItem(L"TMP_LOW_RTF")->GetValue().dblVal;
         s_line_coil[i_coil].tmp_tgt[RTS] = (float)recordset->Fields->GetItem(L"TMP_TGT_SOA")->GetValue().dblVal;
         s_line_coil[i_coil].tmp_upp[RTS] = (float)recordset->Fields->GetItem(L"TMP_UPP_SOA")->GetValue().dblVal;
         s_line_coil[i_coil].tmp_low[RTS] = (float)recordset->Fields->GetItem(L"TMP_LOW_SOA")->GetValue().dblVal;
         s_line_coil[i_coil].tmp_tgt[SLW] = (float)recordset->Fields->GetItem(L"TMP_TGT_SLW")->GetValue().dblVal;
         s_line_coil[i_coil].tmp_upp[SLW] = (float)recordset->Fields->GetItem(L"TMP_UPP_SLW")->GetValue().dblVal;
         s_line_coil[i_coil].tmp_low[SLW] = (float)recordset->Fields->GetItem(L"TMP_LOW_SLW")->GetValue().dblVal;
         s_line_coil[i_coil].tmp_tgt[JET] = (float)recordset->Fields->GetItem(L"TMP_TGT_JET")->GetValue().dblVal;
         s_line_coil[i_coil].tmp_upp[JET] = (float)recordset->Fields->GetItem(L"TMP_UPP_JET")->GetValue().dblVal;
         s_line_coil[i_coil].tmp_low[JET] = (float)recordset->Fields->GetItem(L"TMP_LOW_JET")->GetValue().dblVal;
         s_line_coil[i_coil].tmp_tgt[OVG] = (float)recordset->Fields->GetItem(L"TMP_TGT_OVG")->GetValue().dblVal;
         s_line_coil[i_coil].tmp_upp[OVG] = (float)recordset->Fields->GetItem(L"TMP_UPP_OVG")->GetValue().dblVal;
         s_line_coil[i_coil].tmp_low[OVG] = (float)recordset->Fields->GetItem(L"TMP_LOW_OVG")->GetValue().dblVal;
         s_line_coil[i_coil].tmp_tgt[FCS] = (float)recordset->Fields->GetItem(L"TMP_TGT_FCS")->GetValue().dblVal;
         s_line_coil[i_coil].tmp_upp[FCS] = (float)recordset->Fields->GetItem(L"TMP_UPP_FCS")->GetValue().dblVal;
         s_line_coil[i_coil].tmp_low[FCS] = (float)recordset->Fields->GetItem(L"TMP_LOW_FCS")->GetValue().dblVal;
      } // try

      // Error control
      // -------------
      catch(_com_error &e)
      {
         SQL_R_ERR(sub_name, Procedure, (string)e.Description());
         return false;
      }

      if (s_line_coil[i_coil].id != "" && s_line_coil[i_coil].id != " ")
         n_coils++;

      i_coil++;

      recordset->MoveNext();
	} // while (!recordset->ADOEOF)

   s_coil.nb_coil = n_coils;

   return status;
} // void

// ******************************
// Read measured heating messages
// ******************************
bool SQL2_R_MEA_HEA()
{
   bool status = true;
   string sub_name="SQL2_R_MEA_HEA";

   string Table = "MSG_MEA_FUR";
   string Procedure = "[" + (string)BD_COMMUNICATIONS + "].[dbo].[" + Table + "_SELECT]";
   _RecordsetPtr recordset = m_sql2->executeResultProcedure(Procedure);

	if (!recordset->ADOEOF)
	{
      try
      {
         s_fur_mea[RTH].pre_hea_stp_tmp = (float)recordset->Fields->GetItem(L"PRE_HEA_MEA")->GetValue().dblVal; 

         // RTF
         // ===
         s_fur_mea[RTH].ctl = (recordset->Fields->GetItem(L"RTF_CTL")->GetValue().boolVal != 0);
         s_fur_mea[RTH].stp_tmp = (float)recordset->Fields->GetItem(L"RTF_TMP_STRIP_MEA")->GetValue().dblVal; 

         // For all RTF zones
         for (int i_zon = 0; i_zon < NB_ZON_RTH - 1; i_zon++)
         {
            string VarVal = "RTF_TUB_TMP_MEA_" + to_string(i_zon);
            s_fur_mea[RTH].zon_tmp[i_zon] = (float)recordset->Fields->GetItem(VarVal.c_str())->GetValue().dblVal; 

            VarVal = "RTF_ZON_PWR_MEA_" + to_string(i_zon);
            s_fur_mea[RTH].heat_pow_pct[i_zon] = (float)recordset->Fields->GetItem(VarVal.c_str())->GetValue().dblVal; 
         } // For all RTF zones

         // SOA
         // ===
         s_fur_mea[RTS].ctl = (recordset->Fields->GetItem(L"SOA_CTL")->GetValue().boolVal != 0);
         s_fur_mea[RTS].stp_tmp = (float)recordset->Fields->GetItem(L"SOA_TMP_STRIP_MEA")->GetValue().dblVal; 

         // For all SOA zones
         for (int i_zon = 0; i_zon < NB_ZON_RTS; i_zon++)
         {
            string VarVal = "SOA_TUB_TMP_MEA_" + to_string(i_zon);
            s_fur_mea[RTS].zon_tmp[i_zon] = (float)recordset->Fields->GetItem(VarVal.c_str())->GetValue().dblVal; 

            VarVal = "SOA_ZON_PWR_MEA_" + to_string(i_zon);
            s_fur_mea[RTS].heat_pow_pct[i_zon] = (float)recordset->Fields->GetItem(VarVal.c_str())->GetValue().dblVal; 
         } // For all SOA zones

         s_fur_mea[RTH].zon_tmp[NB_ZON_RTH - 1] = s_fur_mea[RTS].zon_tmp[0]; 
         s_fur_mea[RTH].heat_pow_pct[NB_ZON_RTH - 1] = s_fur_mea[RTS].heat_pow_pct[0]; 

         // SLW
         // ===
         s_fur_mea[SLW].ctl = (recordset->Fields->GetItem(L"SLW_CTL")->GetValue().boolVal != 0);
         s_fur_mea[SLW].stp_tmp = (float)recordset->Fields->GetItem(L"SLW_TMP_STRIP_MEA")->GetValue().dblVal;
         s_fur_mea[SLW].zon_tmp[0] = (float)recordset->Fields->GetItem(L"SLW_ZON_TMP_MEA")->GetValue().dblVal;

         // For all SLW zones
         for (int i_zon = 0; i_zon < NB_ZON_SLW; i_zon++)			// test
         {
            string VarVal = "SLW_FAN_SPD_MEA_" + to_string(i_zon);
				s_fur_mea[SLW].cool_pow_pct[i_zon]		 = (float)recordset->Fields->GetItem(VarVal.c_str())->GetValue().dblVal; 
            s_fur_mea[SLW].s_spe.slw_fan_spd[i_zon] = (float)recordset->Fields->GetItem(VarVal.c_str())->GetValue().dblVal; 
         } // For all SLW zones

         // JET
         // ===
         s_fur_mea[JET].ctl = (recordset->Fields->GetItem(L"JET_CTL")->GetValue().boolVal != 0);
         s_fur_mea[JET].stp_tmp = (float)recordset->Fields->GetItem(L"JET_TMP_STRIP_MEA")->GetValue().dblVal;
			s_fur_mea[JET].fan_mode = (recordset->Fields->GetItem(L"JET_FAN_MODE")->GetValue().intVal == 1);

         // For all JET zones
         for (int i_zon = 0; i_zon < NB_ZON_JET; i_zon++)
         {
            string VarVal = "JET_ZON_PWR_MEA_" + to_string(i_zon);
            s_fur_mea[JET].cool_pow_pct[i_zon] = (float)recordset->Fields->GetItem(VarVal.c_str())->GetValue().dblVal ; 
         } // For all JET zones

         // OVG
         // ===
         s_fur_mea[OVG].ctl = (recordset->Fields->GetItem(L"OVG_CTL")->GetValue().boolVal != 0);
         s_fur_mea[OVG].stp_tmp = (float)recordset->Fields->GetItem(L"OVG_TMP_STRIP_MEA")->GetValue().dblVal;

         // For all zones
         for (int i_zon = 0; i_zon < NB_ZON_OVG; i_zon++)
         {
            string VarVal = "OVG_ZON_TMP_MEA_" + to_string(i_zon);
            s_fur_mea[OVG].zon_tmp[i_zon] = (float)recordset->Fields->GetItem(VarVal.c_str())->GetValue().dblVal;
					 
				VarVal = "OVG_ZON_PWR_MEA_" + to_string(i_zon);
            s_fur_mea[OVG].heat_pow_pct[i_zon] = (float)recordset->Fields->GetItem(VarVal.c_str())->GetValue().dblVal; 
         } // For all zones

         // FCS
         // ===
         s_fur_mea[FCS].ctl = (recordset->Fields->GetItem(L"FCS_CTL")->GetValue().boolVal != 0);
         s_fur_mea[FCS].stp_tmp = (float)recordset->Fields->GetItem(L"FCS_TMP_STRIP_MEA")->GetValue().dblVal;

         // For all zones
         for (int i_zon = 0; i_zon < NB_ZON_FCS; i_zon++)
         {
            string VarVal = "FCS_ZON_TMP_MEA_" + to_string(i_zon);
            s_fur_mea[FCS].zon_tmp[i_zon] = (float)recordset->Fields->GetItem(VarVal.c_str())->GetValue().dblVal; 

            VarVal = "FCS_ZON_PWR_MEA_" + to_string(i_zon);
            s_fur_mea[FCS].cool_pow_pct[i_zon] = (float)recordset->Fields->GetItem(VarVal.c_str())->GetValue().dblVal; 
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

   // Specific data
   // =============
   //SQL2_Read_MeaSpe();

   if (status) s_cur.MeaFurRead = true;

   return status;
} // void

// *************************
// Read measured SPD message
// *************************
bool SQL2_R_MEA_SPD()
{
   bool status = true;
   string sub_name="SQL2_R_MEA_SPD";

   string Table = "MSG_MEA_LINE";
   string Procedure = "[" + (string)BD_COMMUNICATIONS + "].[dbo].[" + Table + "_SELECT]";
   _RecordsetPtr recordset = m_sql2->executeResultProcedure(Procedure);

	COleDateTime Coil_Ole_tim;
   int n_coil=0;

	if (!recordset->ADOEOF)
	{
      try
      {
			Coil_Ole_tim = recordset->Fields->GetItem(L"msg_tim")->GetValue().date;
			s_line_trk.coil_id = _bstr_t(recordset->Fields->GetItem(L"COIL_ID")->GetValue().bstrVal);
			s_line_trk.Spd_mea = (float)recordset->Fields->GetItem(L"SPD_LINE")->GetValue().dblVal;
			s_line_trk.Spd_max_ope = (float)recordset->Fields->GetItem(L"SPD_MAX_OPE_LINE")->GetValue().dblVal;
         s_line_trk.Spd_ctrl = (recordset->Fields->GetItem(L"SPD_CTL")->GetValue().boolVal != 0); 
			s_line_trk.weld_pos = (float)recordset->Fields->GetItem(L"WEL_POS")->GetValue().dblVal;
			s_line_trk.strip_tension = (float)recordset->Fields->GetItem(L"STRIP_TENSION")->GetValue().dblVal;
      } // try

      // Error control
      // -------------
      catch(_com_error &e)
      {
         SQL_R_ERR(sub_name, Procedure, (string)e.Description());
         return false;
      }
	} // if (!recordset->ADOEOF)

   if (status) s_cur.MeaTrkRead = true;

   return status;
} // void

