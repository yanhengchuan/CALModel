/*============================================================================
/ GLOBAL.cpp: Global common Data Declaration.
/
/ Revisions:
/  Created:       2007 Sep      JC.Mitais
/  Modified: 
/  
/ (c) CMI Industry
/===========================================================================*/
#include "stdafx.h"

// Current data
// ============
str_current s_cur;
str_simulation s_sim;

struct sps_entry_data sps_entry[MAX_NB_FUR];     // measurement structure
struct sps_entry_data sps_entry_old[MAX_NB_FUR]; // old measurement structure
struct sps_exit_data  sps_act_sp[MAX_NB_FUR];    // set points structure
struct sps_coil_data  sps_fin_sp[MAX_NB_FUR];

// ============
struct Str_Adp s_adp[MAX_NB_FUR];   // Self adaptation structure
struct str_alm s_alm;               // Alam structure 

int SIMULATION = 1;
int ErrorComOPC;

// FUEL
// ====
float LHV[NB_FUEL];                       // Lower heating value of each fuel used [J/Nm3]
float VA[NB_FUEL];						      // Air volume for each fuel
float VF[NB_FUEL];						      // waste gazes volume for each fuel used  
float AIR_DEF[NB_FUEL];					      // Air default [%]

// ===============================================================================
// Structures 
// ===============================================================================
// Coils structure
// ==================
struct coils s_coil;       // coils data global structure

struct fur_cfg_struct fur_cfg[MAX_NB_FUR];   // section static configuration
struct rtf_cfg_struct rtf_cfg[NB_FUR_RTF];   // rtf static configuration
struct rol_cfg_struct rol_cfg[NB_FUR_RTF];   // rtf static configuration
struct jet_cfg_struct jet_cfg;               // jet static configuration

// RTF
// ===
struct rtf_struct rtf[NB_FUR_RTF];     // strip and furnace structure
struct rtf_struct rtf1[NB_FUR_RTF];    // strip and furnace structure for spg first coil
struct rtf_struct rtf2[NB_FUR_RTF];    // strip and furnace structure for spg second coil

// JET
// ===
struct spg_entry_data_jet spg_entry_jet;  // JET cooling model entry structure
struct spg_exit_data_jet spg_exit_jet;    // JET cooling model exit struture

// ROLLS
// =====
struct roll_profil rol_cmp[NB_FUR_RTF];      // Roll profile calculation structure for heat buckles risk during steady and transient states
struct roll_profil rol_cmp_risk[NB_FUR_RTF]; // Roll profile calculation structure for heat buckles risk during starting up
struct ns_STD::cl_ROL::str_rolls s_Trol[NB_FUR_RTF];          // Roll thermal


// Transcient
// ==========
struct strip_trans_exit trans_result_rtf[NB_FUR_RTF][MAX_NB_COIL];   // Transient situation calculation results structure
struct strip_trans_exit trans_result_old[NB_FUR_RTF];                // transient situation calculation results structure // ???? utilisé, jms initialisé

struct str_line_coil s_line_coil[MAX_NB_COIL];
struct str_line_trk s_line_trk;
struct str_line_spt s_line_spt;
struct str_fur_mea s_fur_mea[MAX_NB_FUR];
struct str_fur_spt s_fur_spt[MAX_NB_FUR];
struct str_spe_PRO s_spe_PRO;

// ===============================================================================
// Structures & more from "exStruct.h"
// ===============================================================================
struct ns_STD::cl_PID::fba_pid fba_stp[MAX_NB_FUR];                // strip PID structures
struct ns_STD::cl_PID::fba_pid fba_zon[MAX_NB_FUR][MAX_NB_ZON];    // zones PID structures
