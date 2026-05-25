/*============================================================================
/ SPS_Specific: All specific application of a business.
/ 
/ Revisions:
/  Created:       2008-nov       J. NAVEIRA
/  Modified: 
/  
/ (c) CMI Industry
//===========================================================================*/
#include "stdafx.h"
#include "SQL.h"

// ========================================
// Specific treatment of SPS
// ========================================
bool SPS_Specific::SPS()
{
   bool status = true;
   return status;
} // void

// ========================================
// Specific SPS initialization
// ========================================
bool SPS_Specific::INIT()
{
   bool status = true;

	// ===============================
	// Weld position and time - SPECIFIC !!!!
	// ===============================
   // we receive 2 information:
   // - the length between the coil tail and the exit of the exit section
   // - the coil ID of the one currently exiting the reference point
   // Enter length of each section and between section
   // RTH
   // ===
   rtf_cfg[RTH].n_rtf = RTH;
   rtf_cfg[RTH].nb_ecz = NB_ECZ_RTH;
   rtf_cfg[RTH].nb_elt = NB_ELT_RTH;
   rtf_cfg[RTH].nb_rol = NB_ROL_RTH;
   rtf_cfg[RTH].trans_calc = TRANS_CALC_RTH;

   sps_entry[RTH].s_mea.lgt = 0.f;
   s_adp[RTH].use_ada = true;

   fur_cfg[RTH].nb_zon = NB_ZON_RTH;
   fur_cfg[RTH].pre_fur = -1;
   fur_cfg[RTH].stp_lgt = 313.21f; // 291.4
   fur_cfg[RTH].xPyro = fur_cfg[RTH].stp_lgt;
   fur_cfg[RTH].foll_lgt = 0.f;

   rol_cmp[RTH].n_rtf = RTH;
   rol_cmp_risk[RTH].n_rtf = RTH;
   rol_cmp[RTH].nb_roll = NB_ROL_RTH;

   // RTS
   // ===
   rtf_cfg[RTS].n_rtf = RTS;
   rtf_cfg[RTS].nb_ecz = NB_ECZ_RTS;
   rtf_cfg[RTS].nb_elt = NB_ELT_RTS;
   rtf_cfg[RTS].nb_rol = NB_ROL_RTS;
   rtf_cfg[RTS].trans_calc = TRANS_CALC_RTS;

   sps_entry[RTS].s_mea.lgt = 0.f;
   s_adp[RTS].use_ada = true;

   fur_cfg[RTS].nb_zon = NB_ZON_RTS;
   fur_cfg[RTS].pre_fur = RTH;
   fur_cfg[RTS].stp_lgt = 176.52f; // 198.30
   fur_cfg[RTS].xPyro = fur_cfg[RTH].xPyro + fur_cfg[RTH].foll_lgt + fur_cfg[RTS].stp_lgt;
   fur_cfg[RTS].foll_lgt = 2.36f;

   rol_cmp[RTS].n_rtf = RTS;
   rol_cmp_risk[RTS].n_rtf = RTS;
   rol_cmp[RTS].nb_roll = NB_ROL_RTS;

   // SLW
   // ===
   sps_entry[SLW].s_mea.lgt = 0.f;
   s_adp[SLW].use_ada = false;

   fur_cfg[SLW].nb_zon = NB_ZON_SLW;
   fur_cfg[SLW].pre_fur = RTS;
   fur_cfg[SLW].stp_lgt = 49.2f;
   fur_cfg[SLW].xPyro = fur_cfg[RTS].xPyro + fur_cfg[RTS].foll_lgt + fur_cfg[SLW].stp_lgt;
   fur_cfg[SLW].foll_lgt = 6.455f;

   // JET
   // ===
   fur_cfg[JET].nb_zon = NB_ZON_JET;
   fur_cfg[JET].pre_fur = SLW;
   fur_cfg[JET].stp_lgt = 24.6f;
   fur_cfg[JET].xPyro = fur_cfg[SLW].xPyro + fur_cfg[SLW].foll_lgt + fur_cfg[JET].stp_lgt;
   fur_cfg[JET].foll_lgt = 5.96f;

   s_adp[JET].use_ada = false;
   sps_entry[JET].s_mea.lgt = 0.f;

   // OVG
   // ===
   fur_cfg[OVG].nb_zon = NB_ZON_OVG;
   fur_cfg[OVG].pre_fur = JET;
   fur_cfg[OVG].stp_lgt = 15.f * 24.6f + 3.f + 16.f * 24.6f;
   fur_cfg[OVG].xPyro = fur_cfg[JET].xPyro + fur_cfg[JET].foll_lgt + fur_cfg[OVG].stp_lgt;
   fur_cfg[OVG].foll_lgt = 3.f;

   s_adp[OVG].use_ada = false;
   sps_entry[OVG].s_mea.lgt = 0.f;

   // FCS
   // ===
   fur_cfg[FCS].nb_zon = NB_ZON_FCS;
   fur_cfg[FCS].pre_fur = OVG;
   fur_cfg[FCS].stp_lgt = 4.f * 24.6f;
   fur_cfg[FCS].xPyro = fur_cfg[OVG].xPyro + fur_cfg[OVG].foll_lgt + fur_cfg[FCS].stp_lgt;
   fur_cfg[FCS].foll_lgt = 0.f;

   s_adp[FCS].use_ada = false;
   sps_entry[FCS].s_mea.lgt = 0.f;

   // EXT
   // ===
   fur_cfg[EXT].nb_zon = NB_ZON_EXT;
   fur_cfg[EXT].pre_fur = FCS;
   fur_cfg[EXT].stp_lgt = 0.f;
   fur_cfg[EXT].xPyro = fur_cfg[FCS].xPyro + fur_cfg[FCS].foll_lgt + fur_cfg[EXT].stp_lgt;
   fur_cfg[EXT].foll_lgt = 0.f;

   s_adp[EXT].use_ada = false;
   sps_entry[EXT].s_mea.lgt = 0.f;

   // APC
   // ===
   fur_cfg[APC].nb_zon = NB_ZON_APC;
   fur_cfg[APC].pre_fur = EXT;
   fur_cfg[APC].stp_lgt = 0.f;
   fur_cfg[APC].xPyro = fur_cfg[EXT].xPyro + fur_cfg[EXT].foll_lgt + fur_cfg[APC].stp_lgt;
   fur_cfg[APC].foll_lgt = 0.f;

   s_adp[APC].use_ada = false;
   sps_entry[APC].s_mea.lgt = 0.f;

   return status;
} // void

// ========================================
// Specific simulation initialization
// ========================================
bool SPS_Specific::SIM_INI()
{
   bool status = true;
   return status;
}

// ========================================
// Specific SPS initialization
// ========================================
bool SPS_Specific::SPS_getdata()
{
   bool status = true;
   return status; 
}
