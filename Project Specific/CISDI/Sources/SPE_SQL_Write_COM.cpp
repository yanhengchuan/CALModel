/*=============================================================================
/ SQL_Write_COM: Treatment of data send in MS-SQL SERVER Communications Database. 
/  
/ (c) CMI Industry
//=============================================================================*/
#include "stdafx.h"
#include "SQL.h"

extern Sql *m_sql2;

// ****************************************************************************
// SQL2_I_MSG_SPT_HEA: Insert MSG_SPT_HEA msg (Heating setpoints) - *
// 
// Revisions:
//  Created:         2015-Mar       JC.Mitais   LTOP 1.2
//  Modified:        2016-Mar       JC.Mitais
// ****************************************************************************
bool SQL2_I_MSG_SPT_HEA()
{
   string Table1 = "MSG_SPT_FUR";
   string Procedure = "[" + (string)BD_COMMUNICATIONS + "].[dbo].[" + Table1 + "_INSERT]";

   struct str_PrmArrays PrmArr;
   VariantInit(&PrmArr.var); 

   // RTF
   // ===
   m_sql2->BoolProcArray(&PrmArr, "RTF_VALID", s_fur_spt[RTH].valid);
   m_sql2->IntProcArray(&PrmArr, "RTF_CTL_MOD", s_fur_spt[RTH].ctl_mod);
   m_sql2->FloatProcArray(&PrmArr, "RTF_TMP_STRIP", s_fur_spt[RTH].stp_tmp);

   // For all RTF zones
   for (int i_zon = 0; i_zon < NB_ZON_RTH - 1; i_zon++)
   {
      string ZON = to_string(i_zon);
      m_sql2->FloatProcArray(&PrmArr, "RTF_TUB_TMP_SPT_"+ZON, s_fur_spt[RTH].zon_tmp[i_zon]);
      m_sql2->FloatProcArray(&PrmArr, "RTF_ZON_PWR_SPT_"+ZON, s_fur_spt[RTH].heat_zon_pow[i_zon]);
   } // For all RTF zones

   // SOA
   // ===
   m_sql2->BoolProcArray(&PrmArr, "SOA_VALID", s_fur_spt[RTS].valid);
   m_sql2->IntProcArray(&PrmArr, "SOA_CTL_MOD", s_fur_spt[RTS].ctl_mod);
   m_sql2->FloatProcArray(&PrmArr, "SOA_TMP_STRIP", s_fur_spt[RTS].stp_tmp);

   // For all SOA zones
   for (int i_zon = 0; i_zon < NB_ZON_RTS; i_zon++)
   {
      string ZON = to_string(i_zon);
      m_sql2->FloatProcArray(&PrmArr, "SOA_TUB_TMP_SPT_"+ZON, s_fur_spt[RTS].zon_tmp[i_zon]);
      m_sql2->FloatProcArray(&PrmArr, "SOA_ZON_PWR_SPT_"+ZON, s_fur_spt[RTS].heat_zon_pow[i_zon]);
      } // For all SOA zones

   // SLW
   // ===   
   // pow_pct: plusieurs zones   // Unisson mode --> Send average
   float pow_pct_slw = 0.f;

   // For all SLW zones
   for ( int i_zon=0; i_zon < NB_ZON_SLW; i_zon++)
   {
      pow_pct_slw += sps_act_sp[SLW].cool_pow_pct[i_zon];
   } // For all SLW zones
   pow_pct_slw /= float(NB_ZON_SLW);

   m_sql2->BoolProcArray(&PrmArr, "SLW_VALID", s_fur_spt[SLW].valid);
   m_sql2->FloatProcArray(&PrmArr, "SLW_TMP_STRIP", s_fur_spt[SLW].stp_tmp);
   m_sql2->FloatProcArray(&PrmArr, "SLW_ZON_PWR_SPT_0", pow_pct_slw);

   // JET
   // ===   
   m_sql2->BoolProcArray(&PrmArr, "JET_VALID", s_fur_spt[JET].valid);
   m_sql2->FloatProcArray(&PrmArr, "JET_TMP_STRIP", s_fur_spt[JET].stp_tmp);

   // For all JET zones
   for (int i_zon = 0; i_zon < NB_ZON_JET; i_zon++)
   {
      string ZON = to_string(i_zon);
      m_sql2->FloatProcArray(&PrmArr, "JET_ZON_PWR_SPT_"+ZON, s_fur_spt[JET].cool_zon_pow[i_zon]);
   } // For all JET zones

   // OVG
   // ===
   m_sql2->BoolProcArray(&PrmArr, "OVG_VALID", s_fur_spt[OVG].valid);
   m_sql2->FloatProcArray(&PrmArr, "OVG_TMP_STRIP", s_fur_spt[OVG].stp_tmp);

   // For all OVG zones
   for (int i_zon = 0; i_zon < NB_ZON_OVG; i_zon++)
   {
      string ZON = to_string(i_zon);
      m_sql2->FloatProcArray(&PrmArr, "OVG_ZON_TEMP_SPT_"+ZON, s_fur_spt[OVG].zon_tmp[i_zon]);
      m_sql2->FloatProcArray(&PrmArr, "OVG_ZON_PWR_SPT_"+ZON, s_fur_spt[OVG].heat_zon_pow[i_zon]);
   } // For all OVG zones

   // FCS
   // ===
   m_sql2->BoolProcArray(&PrmArr, "FCS_VALID", s_fur_spt[FCS].valid);
   m_sql2->FloatProcArray(&PrmArr, "FCS_TMP_STRIP", s_fur_spt[FCS].stp_tmp);

   // For all FCS zones
   for (int i_zon = 0; i_zon < NB_ZON_FCS; i_zon++)
   {
      string ZON = to_string(i_zon);
      m_sql2->FloatProcArray(&PrmArr, "FCS_ZON_PWR_SPT_"+ZON, s_fur_spt[FCS].cool_zon_pow[i_zon]);
   } // For all FCS zones

   m_sql2->BoolProcArray(&PrmArr, "EXT_VALID", s_fur_spt[EXT].valid);
   m_sql2->FloatProcArray(&PrmArr, "EXT_TMP_STRIP", s_fur_spt[EXT].stp_tmp);
   for (int i_zon = 0; i_zon < NB_ZON_EXT; i_zon++)
   {
      string ZON = to_string(i_zon);
      m_sql2->FloatProcArray(&PrmArr, "EXT_ZON_TEMP_SPT_"+ZON, s_fur_spt[EXT].zon_tmp[i_zon]);
   }

   m_sql2->BoolProcArray(&PrmArr, "APC_VALID", s_fur_spt[APC].valid);
   m_sql2->FloatProcArray(&PrmArr, "APC_TMP_STRIP", s_fur_spt[APC].stp_tmp);
   for (int i_zon = 0; i_zon < NB_ZON_APC; i_zon++)
   {
      string ZON = to_string(i_zon);
      m_sql2->FloatProcArray(&PrmArr, "APC_ZON_TEMP_SPT_"+ZON, s_fur_spt[APC].zon_tmp[i_zon]);
   }

   m_sql2->executeNonResultProcedure(Procedure, PrmArr.vtArr, PrmArr.prmArr, PrmArr.sizArr);

   return true;
} // void

// ****************************************************************************
// SQL2_I_MSG_SPT_HEA: !!!TEST!!! Insert MSG_SPT_HEA msg (Heating setpoints) - *
// 
// Revisions:
//  Created:         2017-May       Sh. Mechiev
//  Modified:        
// ****************************************************************************
//bool SQL2_I_MSG_SPT_HEA()
//{
//   string Table1 = "MSG_SPT_FUR";
//   string Procedure = "[" + (string)BD_COMMUNICATIONS + "].[dbo].[" + Table1 + "_INSERT]";
//
//   struct str_PrmArrays PrmArr;
//   VariantInit(&PrmArr.var); 
//
//   //static int invalid = -1;				// test
//   //s_fur_spt[invalid].valid = false;		// test
//   //static bool pcm = true;					// test
//   //s_fur_spt[RTH].ctl_mod = pcm;			// test
//
//   // RTF
//   // ===
//   m_sql2->BoolProcArray(&PrmArr, "RTF_VALID", s_fur_spt[RTH].valid);
//   m_sql2->IntProcArray(&PrmArr, "RTF_CTL_MOD", s_fur_spt[RTH].ctl_mod);
//   m_sql2->FloatProcArray(&PrmArr, "RTF_TMP_STRIP", s_fur_spt[RTH].stp_tmp);
//
//   // For all RTF zones
//   for (int i_zon = 0; i_zon < NB_ZON_RTH; i_zon++)
//   {
//      string ZON = to_string(i_zon);
//      m_sql2->FloatProcArray(&PrmArr, "RTF_TUB_TMP_SPT_"+ZON, s_fur_spt[RTH].zon_tmp[i_zon] + i_zon);
//      m_sql2->FloatProcArray(&PrmArr, "RTF_ZON_PWR_SPT_"+ZON, s_fur_spt[RTH].heat_zon_pow[i_zon] + i_zon);
//   } // For all RTF zones
//
//   // SOA
//   // ===
//   m_sql2->BoolProcArray(&PrmArr, "SOA_VALID", s_fur_spt[RTS].valid);
//   m_sql2->IntProcArray(&PrmArr, "SOA_CTL_MOD", s_fur_spt[RTS].ctl_mod);
//   m_sql2->FloatProcArray(&PrmArr, "SOA_TMP_STRIP", s_fur_spt[RTS].stp_tmp);
//
//   // For all SOA zones
//   for (int i_zon = 0; i_zon < NB_ZON_RTS; i_zon++)
//   {
//      string ZON = to_string(i_zon);
//      m_sql2->FloatProcArray(&PrmArr, "SOA_TUB_TMP_SPT_"+ZON, s_fur_spt[RTS].zon_tmp[i_zon] + i_zon);
//      m_sql2->FloatProcArray(&PrmArr, "SOA_ZON_PWR_SPT_"+ZON, s_fur_spt[RTS].heat_zon_pow[i_zon] + i_zon);
//      } // For all SOA zones
//
//   // SLW
//   // ===   
//   // pow_pct: plusieurs zones   // Unisson mode --> Send average
//   float pow_pct_slw = 0.f;
//
//   // For all SLW zones
//   for ( int i_zon=0; i_zon < NB_ZON_SLW; i_zon++)
//   {
//      pow_pct_slw += sps_act_sp[SLW].cool_pow_pct[i_zon];
//   } // For all SLW zones
//   pow_pct_slw /= float(NB_ZON_SLW);
//
//   m_sql2->BoolProcArray(&PrmArr, "SLW_VALID", s_fur_spt[SLW].valid);
//   m_sql2->FloatProcArray(&PrmArr, "SLW_TMP_STRIP", s_fur_spt[SLW].stp_tmp);
//   m_sql2->FloatProcArray(&PrmArr, "SLW_ZON_PWR_SPT_0", pow_pct_slw + 5);
//
//   // JET
//   // ===   
//   m_sql2->BoolProcArray(&PrmArr, "JET_VALID", s_fur_spt[JET].valid);
//   m_sql2->FloatProcArray(&PrmArr, "JET_TMP_STRIP", s_fur_spt[JET].stp_tmp);
//
//   // For all JET zones
//   for (int i_zon = 0; i_zon < NB_ZON_JET; i_zon++)
//   {
//      string ZON = to_string(i_zon);
//      m_sql2->FloatProcArray(&PrmArr, "JET_ZON_PWR_SPT_"+ZON, s_fur_spt[JET].cool_zon_pow[i_zon] + i_zon);
//   } // For all JET zones
//
//   // OVG
//   // ===
//   m_sql2->BoolProcArray(&PrmArr, "OVG_VALID", s_fur_spt[OVG].valid);
//   m_sql2->FloatProcArray(&PrmArr, "OVG_TMP_STRIP", s_fur_spt[OVG].stp_tmp);
//
//   // For all OVG zones
//   for (int i_zon = 0; i_zon < NB_ZON_OVG; i_zon++)
//   {
//      string ZON = to_string(i_zon);
//      m_sql2->FloatProcArray(&PrmArr, "OVG_ZON_TEMP_SPT_"+ZON, s_fur_spt[OVG].zon_tmp[i_zon] + i_zon);
//      m_sql2->FloatProcArray(&PrmArr, "OVG_ZON_PWR_SPT_"+ZON, s_fur_spt[OVG].heat_zon_pow[i_zon] + i_zon);
//   } // For all OVG zones
//
//   // FCS
//   // ===
//   m_sql2->BoolProcArray(&PrmArr, "FCS_VALID", s_fur_spt[FCS].valid);
//   m_sql2->FloatProcArray(&PrmArr, "FCS_TMP_STRIP", s_fur_spt[FCS].stp_tmp);
//
//   // For all FCS zones
//   for (int i_zon = 0; i_zon < NB_ZON_FCS; i_zon++)
//   {
//      string ZON = to_string(i_zon);
//      m_sql2->FloatProcArray(&PrmArr, "FCS_ZON_PWR_SPT_"+ZON, s_fur_spt[FCS].cool_zon_pow[i_zon] + i_zon);
//   } // For all FCS zones
//
//   m_sql2->executeNonResultProcedure(Procedure, PrmArr.vtArr, PrmArr.prmArr, PrmArr.sizArr);
//
//   return true;
//} // void

// ****************************************************************************
// SQL2_I_MSG_SPT_SPD: Insert Speed setpoint msg - *
// 
// Revisions:
//  Created:         2015-Mar       JC.Mitais   LTOP 1.2
//  Modified:        2016-Mar       JC.Mitais
// ****************************************************************************
bool SQL2_I_MSG_SPT_SPD()
{
   string Table1 = "MSG_SPT_LINE";
   string Procedure = "[" + (string)BD_COMMUNICATIONS + "].[dbo].[" + Table1 + "_INSERT]";

   struct str_PrmArrays PrmArr;
   VariantInit(&PrmArr.var); 

   m_sql2->FloatProcArray(&PrmArr, "SPD_LINE", s_line_spt.Spd_spt);
   m_sql2->BoolProcArray(&PrmArr, "SPD_VALID", s_line_spt.spt_valid);
   m_sql2->FloatProcArray(&PrmArr, "STRIP_TENSION", s_coil.s_coil_data[0].strip_tension);

   m_sql2->executeNonResultProcedure(Procedure, PrmArr.vtArr, PrmArr.prmArr, PrmArr.sizArr);

   return true;
} // void

// ****************************************************************************
// SQL2_I_MSG_COIL: Insert MSG_COIL msg (Simulation) - *
// 
// Revisions:
//  Created:         2015-Mar       JC.Mitais   LTOP 1.2
//  Modified:        2016-Mar       JC.Mitais
// ****************************************************************************
bool SQL2_I_MSG_COIL()
{
   string Table1 = "MSG_COIL";
   string Procedure = "[" + (string)BD_COMMUNICATIONS + "].[dbo].[" + Table1 + "_UPDATE]";

   struct str_PrmArrays PrmArr;
   VariantInit(&PrmArr.var); 

   // For given coils coils
   for (int i_coil=0; i_coil < s_sim.nb_coil; i_coil++)
   {
      m_sql2->IntProcArray(&PrmArr,   "idx",                                   i_coil);
      m_sql2->StringProcArray(&PrmArr, "COIL_ID",            s_sim.s_coil_data[i_coil].id);
      m_sql2->FloatProcArray(&PrmArr, "THI",                 s_sim.s_coil_data[i_coil].thi * 1000);
      m_sql2->FloatProcArray(&PrmArr, "LGT",                 s_sim.s_coil_data[i_coil].lgt);
      m_sql2->FloatProcArray(&PrmArr, "WID",                 s_sim.s_coil_data[i_coil].wid * 1000);
      m_sql2->FloatProcArray(&PrmArr, "SPD_MAX_RUNNING",     s_sim.s_coil_data[i_coil].spd_max * 60.f);
      m_sql2->FloatProcArray(&PrmArr, "SPD_MIN_RUNNING",     s_sim.s_coil_data[i_coil].spd_min * 60.f);
      m_sql2->IntProcArray(&PrmArr,   "STEEL_TYPE",          s_sim.s_coil_data[i_coil].steeltype);
      m_sql2->IntProcArray(&PrmArr,   "ROLLING_MILL_NUMBER", s_sim.s_coil_data[i_coil].s_spe.Prev_roll_mill);
      m_sql2->IntProcArray(&PrmArr,   "DUMMY_COIL",          s_sim.s_coil_data[i_coil].dummy);

      m_sql2->FloatProcArray(&PrmArr, "TMP_TGT_RTF", s_sim.s_coil_data[i_coil].tmp_tgt[RTH]-TK);
      m_sql2->FloatProcArray(&PrmArr, "TMP_UPP_RTF", s_sim.s_coil_data[i_coil].tmp_upp[RTH]-TK);
      m_sql2->FloatProcArray(&PrmArr, "TMP_LOW_RTF", s_sim.s_coil_data[i_coil].tmp_low[RTH]-TK);
      m_sql2->FloatProcArray(&PrmArr, "TMP_TGT_SOA", s_sim.s_coil_data[i_coil].tmp_tgt[RTS]-TK);
      m_sql2->FloatProcArray(&PrmArr, "TMP_UPP_SOA", s_sim.s_coil_data[i_coil].tmp_upp[RTS]-TK);
      m_sql2->FloatProcArray(&PrmArr, "TMP_LOW_SOA", s_sim.s_coil_data[i_coil].tmp_low[RTS]-TK);
      m_sql2->FloatProcArray(&PrmArr, "TMP_TGT_SLW", s_sim.s_coil_data[i_coil].tmp_tgt[SLW]-TK);
      m_sql2->FloatProcArray(&PrmArr, "TMP_UPP_SLW", s_sim.s_coil_data[i_coil].tmp_upp[SLW]-TK);
      m_sql2->FloatProcArray(&PrmArr, "TMP_LOW_SLW", s_sim.s_coil_data[i_coil].tmp_low[SLW]-TK);
      m_sql2->FloatProcArray(&PrmArr, "TMP_TGT_JET", s_sim.s_coil_data[i_coil].tmp_tgt[JET]-TK);
      m_sql2->FloatProcArray(&PrmArr, "TMP_UPP_JET", s_sim.s_coil_data[i_coil].tmp_upp[JET]-TK);
      m_sql2->FloatProcArray(&PrmArr, "TMP_LOW_JET", s_sim.s_coil_data[i_coil].tmp_low[JET]-TK);
      m_sql2->FloatProcArray(&PrmArr, "TMP_TGT_OVG", s_sim.s_coil_data[i_coil].tmp_tgt[OVG]-TK);
      m_sql2->FloatProcArray(&PrmArr, "TMP_UPP_OVG", s_sim.s_coil_data[i_coil].tmp_upp[OVG]-TK);
      m_sql2->FloatProcArray(&PrmArr, "TMP_LOW_OVG", s_sim.s_coil_data[i_coil].tmp_low[OVG]-TK);
      m_sql2->FloatProcArray(&PrmArr, "TMP_TGT_FCS", s_sim.s_coil_data[i_coil].tmp_tgt[FCS]-TK);
      m_sql2->FloatProcArray(&PrmArr, "TMP_UPP_FCS", s_sim.s_coil_data[i_coil].tmp_upp[FCS]-TK);
      m_sql2->FloatProcArray(&PrmArr, "TMP_LOW_FCS", s_sim.s_coil_data[i_coil].tmp_low[FCS]-TK);
      m_sql2->FloatProcArray(&PrmArr, "TMP_TGT_EXT", s_sim.s_coil_data[i_coil].tmp_tgt[EXT]-TK);
      m_sql2->FloatProcArray(&PrmArr, "TMP_UPP_EXT", s_sim.s_coil_data[i_coil].tmp_upp[EXT]-TK);
      m_sql2->FloatProcArray(&PrmArr, "TMP_LOW_EXT", s_sim.s_coil_data[i_coil].tmp_low[EXT]-TK);
      m_sql2->FloatProcArray(&PrmArr, "TMP_TGT_APC", s_sim.s_coil_data[i_coil].tmp_tgt[APC]-TK);
      m_sql2->FloatProcArray(&PrmArr, "TMP_UPP_APC", s_sim.s_coil_data[i_coil].tmp_upp[APC]-TK);
      m_sql2->FloatProcArray(&PrmArr, "TMP_LOW_APC", s_sim.s_coil_data[i_coil].tmp_low[APC]-TK);
      m_sql2->FloatProcArray(&PrmArr, "TMP_TGT_EXT", s_sim.s_coil_data[i_coil].tmp_tgt[EXT]-TK);
      m_sql2->FloatProcArray(&PrmArr, "TMP_UPP_EXT", s_sim.s_coil_data[i_coil].tmp_upp[EXT]-TK);
      m_sql2->FloatProcArray(&PrmArr, "TMP_LOW_EXT", s_sim.s_coil_data[i_coil].tmp_low[EXT]-TK);
      m_sql2->FloatProcArray(&PrmArr, "TMP_TGT_APC", s_sim.s_coil_data[i_coil].tmp_tgt[APC]-TK);
      m_sql2->FloatProcArray(&PrmArr, "TMP_UPP_APC", s_sim.s_coil_data[i_coil].tmp_upp[APC]-TK);
      m_sql2->FloatProcArray(&PrmArr, "TMP_LOW_APC", s_sim.s_coil_data[i_coil].tmp_low[APC]-TK);

      m_sql2->executeNonResultProcedure(Procedure, PrmArr.vtArr, PrmArr.prmArr, PrmArr.sizArr);
      PrmArr.sizArr.clear();   PrmArr.prmArr.clear();   PrmArr.vtArr.clear();
   } // For given coils coils

   // For missing coils coils
   for (int i_coil=s_sim.nb_coil; i_coil < MAX_NB_COIL; i_coil++)
   {
      m_sql2->IntProcArray(&PrmArr,   "idx",                                   i_coil);
      m_sql2->StringProcArray(&PrmArr, "COIL_ID",            "");
      m_sql2->FloatProcArray(&PrmArr, "THI",                 s_sim.s_coil_data[i_coil].thi * 1000);
      m_sql2->FloatProcArray(&PrmArr, "LGT",                 s_sim.s_coil_data[i_coil].lgt);
      m_sql2->FloatProcArray(&PrmArr, "WID",                 s_sim.s_coil_data[i_coil].wid * 1000);
      m_sql2->FloatProcArray(&PrmArr, "SPD_MAX_RUNNING",     s_sim.s_coil_data[i_coil].spd_max * 60.f);
      m_sql2->FloatProcArray(&PrmArr, "SPD_MIN_RUNNING",     s_sim.s_coil_data[i_coil].spd_min * 60.f);
      m_sql2->IntProcArray(&PrmArr,   "STEEL_TYPE",          s_sim.s_coil_data[i_coil].steeltype);
      m_sql2->IntProcArray(&PrmArr,   "ROLLING_MILL_NUMBER", s_sim.s_coil_data[i_coil].s_spe.Prev_roll_mill);
      m_sql2->IntProcArray(&PrmArr,   "DUMMY_COIL",          s_sim.s_coil_data[i_coil].dummy);

      m_sql2->FloatProcArray(&PrmArr, "TMP_TGT_RTF", s_sim.s_coil_data[i_coil].tmp_tgt[RTH]-TK);
      m_sql2->FloatProcArray(&PrmArr, "TMP_UPP_RTF", s_sim.s_coil_data[i_coil].tmp_upp[RTH]-TK);
      m_sql2->FloatProcArray(&PrmArr, "TMP_LOW_RTF", s_sim.s_coil_data[i_coil].tmp_low[RTH]-TK);
      m_sql2->FloatProcArray(&PrmArr, "TMP_TGT_SOA", s_sim.s_coil_data[i_coil].tmp_tgt[RTS]-TK);
      m_sql2->FloatProcArray(&PrmArr, "TMP_UPP_SOA", s_sim.s_coil_data[i_coil].tmp_upp[RTS]-TK);
      m_sql2->FloatProcArray(&PrmArr, "TMP_LOW_SOA", s_sim.s_coil_data[i_coil].tmp_low[RTS]-TK);
      m_sql2->FloatProcArray(&PrmArr, "TMP_TGT_SLW", s_sim.s_coil_data[i_coil].tmp_tgt[SLW]-TK);
      m_sql2->FloatProcArray(&PrmArr, "TMP_UPP_SLW", s_sim.s_coil_data[i_coil].tmp_upp[SLW]-TK);
      m_sql2->FloatProcArray(&PrmArr, "TMP_LOW_SLW", s_sim.s_coil_data[i_coil].tmp_low[SLW]-TK);
      m_sql2->FloatProcArray(&PrmArr, "TMP_TGT_JET", s_sim.s_coil_data[i_coil].tmp_tgt[JET]-TK);
      m_sql2->FloatProcArray(&PrmArr, "TMP_UPP_JET", s_sim.s_coil_data[i_coil].tmp_upp[JET]-TK);
      m_sql2->FloatProcArray(&PrmArr, "TMP_LOW_JET", s_sim.s_coil_data[i_coil].tmp_low[JET]-TK);
      m_sql2->FloatProcArray(&PrmArr, "TMP_TGT_OVG", s_sim.s_coil_data[i_coil].tmp_tgt[OVG]-TK);
      m_sql2->FloatProcArray(&PrmArr, "TMP_UPP_OVG", s_sim.s_coil_data[i_coil].tmp_upp[OVG]-TK);
      m_sql2->FloatProcArray(&PrmArr, "TMP_LOW_OVG", s_sim.s_coil_data[i_coil].tmp_low[OVG]-TK);
      m_sql2->FloatProcArray(&PrmArr, "TMP_TGT_FCS", s_sim.s_coil_data[i_coil].tmp_tgt[FCS]-TK);
      m_sql2->FloatProcArray(&PrmArr, "TMP_UPP_FCS", s_sim.s_coil_data[i_coil].tmp_upp[FCS]-TK);
      m_sql2->FloatProcArray(&PrmArr, "TMP_LOW_FCS", s_sim.s_coil_data[i_coil].tmp_low[FCS]-TK);

      m_sql2->executeNonResultProcedure(Procedure, PrmArr.vtArr, PrmArr.prmArr, PrmArr.sizArr);
      PrmArr.sizArr.clear();   PrmArr.prmArr.clear();   PrmArr.vtArr.clear();
   } // For missing coils coils

   return true;
} // void

// ****************************************************************************
// SQL2_I_MSG_MEA_HEA: Insert Measured Heating msg (Simulation) - *
// 
// Revisions:
//  Created:         2015-Mar       JC.Mitais   LTOP 1.2
//  Modified:        
// ****************************************************************************
bool SQL2_I_MSG_MEA_HEA()
{
   string Table1 = "MSG_MEA_FUR";
   string Procedure = "[" + (string)BD_COMMUNICATIONS + "].[dbo].[" + Table1 + "_INSERT]";

   struct str_PrmArrays PrmArr;
   VariantInit(&PrmArr.var); 

   // PREHEATING SECTION
   // ==================
   m_sql2->FloatProcArray(&PrmArr, "PRE_HEA_MEA", s_sim.s_fur_mea[RTH].pre_hea_stp_tmp);

   // RTH
   // ===
   m_sql2->BoolProcArray(&PrmArr, "RTF_CTL", s_sim.s_fur_mea[RTH].ctl );
   m_sql2->FloatProcArray(&PrmArr, "RTF_TMP_STRIP_MEA", s_sim.s_fur_mea[RTH].stp_tmp);

   // For all zones
   for (int i_zon = 0; i_zon < NB_ZON_RTH - 1; i_zon++)
   {
      string ZON = to_string(i_zon);
      m_sql2->FloatProcArray(&PrmArr, "RTF_TUB_TMP_MEA_"+ZON, s_sim.s_fur_mea[RTH].zon_tmp[i_zon]);
      m_sql2->FloatProcArray(&PrmArr, "RTF_ZON_PWR_MEA_"+ZON, s_sim.s_fur_mea[RTH].heat_pow_pct[i_zon]);
   } // For all zones

   // RTS
   // ===
   m_sql2->BoolProcArray(&PrmArr, "SOA_CTL", s_sim.s_fur_mea[RTS].ctl);
   m_sql2->FloatProcArray(&PrmArr, "SOA_TMP_STRIP_MEA", s_sim.s_fur_mea[RTS].stp_tmp);

   // For all zones
   for (int i_zon = 0; i_zon < NB_ZON_RTS; i_zon++)
   {
      string ZON = to_string(i_zon);
      m_sql2->FloatProcArray(&PrmArr, "SOA_TUB_TMP_MEA_"+ZON, s_sim.s_fur_mea[RTS].zon_tmp[i_zon]);
      m_sql2->FloatProcArray(&PrmArr, "SOA_ZON_PWR_MEA_"+ZON, s_sim.s_fur_mea[RTS].heat_pow_pct[i_zon]);
   } // For all zones

   // SLW
   // ===
   m_sql2->BoolProcArray(&PrmArr, "SLW_CTL", s_sim.s_fur_mea[SLW].ctl);
   m_sql2->FloatProcArray(&PrmArr, "SLW_TMP_STRIP_MEA", s_sim.s_fur_mea[SLW].stp_tmp);
   m_sql2->FloatProcArray(&PrmArr, "SLW_ZON_TMP_MEA", s_sim.s_fur_mea[SLW].zon_tmp[0]);

   float ave_cool_pct = 0;
   // For all zones
   for (int i_fan = 0; i_fan < 3; i_fan++)
      ave_cool_pct += s_sim.s_fur_mea[SLW].cool_pow_pct[0];

   ave_cool_pct /= 3;

   m_sql2->FloatProcArray(&PrmArr, "SLW_FAN_SPD_MEA_0", ave_cool_pct);

   // JET
   // ===
   m_sql2->BoolProcArray(&PrmArr, "JET_CTL", s_sim.s_fur_mea[JET].ctl);
   m_sql2->IntProcArray(&PrmArr, "JET_FAN_MODE", s_sim.s_fur_mea[JET].fan_mode);
   m_sql2->FloatProcArray(&PrmArr, "JET_TMP_STRIP_MEA", s_sim.s_fur_mea[JET].stp_tmp);

   // For all zones
   for (int i_zon = 0; i_zon < NB_ZON_JET; i_zon++)
   {
      string ZON = to_string(i_zon);
      m_sql2->FloatProcArray(&PrmArr, "JET_ZON_PWR_MEA_"+ZON, s_sim.s_fur_mea[JET].cool_pow_pct[i_zon]);
   } // For all zones

   // OVG
   // ===
   m_sql2->BoolProcArray(&PrmArr, "OVG_CTL", s_sim.s_fur_mea[OVG].ctl);
   m_sql2->FloatProcArray(&PrmArr, "OVG_TMP_STRIP_MEA", s_sim.s_fur_mea[OVG].stp_tmp);

   // For all zones
   for (int i_zon = 0; i_zon < NB_ZON_OVG; i_zon++)
   {
      string ZON = to_string(i_zon);
      m_sql2->FloatProcArray(&PrmArr, "OVG_ZON_TMP_MEA_"+ZON, s_sim.s_fur_mea[OVG].zon_tmp[i_zon]);
      m_sql2->FloatProcArray(&PrmArr, "OVG_ZON_PWR_MEA_"+ZON, s_sim.s_fur_mea[OVG].heat_pow_pct[i_zon]);
   } // For all zones

   // FCS
   // ===
   m_sql2->BoolProcArray(&PrmArr, "FCS_CTL", s_sim.s_fur_mea[FCS].ctl);
   m_sql2->FloatProcArray(&PrmArr, "FCS_TMP_STRIP_MEA", s_sim.s_fur_mea[FCS].stp_tmp);

   // For all zones
   for (int i_zon = 0; i_zon < NB_ZON_FCS; i_zon++)
   {
      string ZON = to_string(i_zon);
      m_sql2->FloatProcArray(&PrmArr, "FCS_ZON_TMP_MEA_"+ZON, s_sim.s_fur_mea[FCS].zon_tmp[i_zon]);
      m_sql2->FloatProcArray(&PrmArr, "FCS_ZON_PWR_MEA_"+ZON, s_sim.s_fur_mea[FCS].cool_pow_pct[i_zon]);
   } // For all zones

   m_sql2->executeNonResultProcedure(Procedure, PrmArr.vtArr, PrmArr.prmArr, PrmArr.sizArr);

   return true;
} // void

// ****************************************************************************
// SQL2_I_MSG_MEA_SPD: Insert Measured Speed msg (Simulation) - *
// 
// Revisions:
//  Created:         2015-Mar       JC.Mitais   LTOP 1.2
//  Modified:        
// ****************************************************************************
bool SQL2_I_MSG_MEA_SPD()
{
   string Table1 = "MSG_MEA_LINE";
   string Procedure = "[" + (string)BD_COMMUNICATIONS + "].[dbo].[" + Table1 + "_INSERT]";

   struct str_PrmArrays PrmArr;
   VariantInit(&PrmArr.var); 

   m_sql2->StringProcArray(&PrmArr, "COIL_ID", s_sim.s_line_trk.coil_id);
   m_sql2->BoolProcArray(&PrmArr, "SPD_CTL", s_sim.s_line_trk.Spd_ctrl);
   m_sql2->FloatProcArray(&PrmArr, "SPD_LINE", s_sim.s_line_trk.Spd_mea*60.f);
   m_sql2->FloatProcArray(&PrmArr, "SPD_MAX_OPE_LINE", 0.f);
   m_sql2->FloatProcArray(&PrmArr, "STRIP_TENSION", s_sim.s_coil_data[0].strip_tension);
   m_sql2->FloatProcArray(&PrmArr, "WEL_POS", s_sim.weld_pos);

   m_sql2->executeNonResultProcedure(Procedure, PrmArr.vtArr, PrmArr.prmArr, PrmArr.sizArr);

   return true;
} // void
