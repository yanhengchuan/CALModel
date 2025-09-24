/*============================================================================
/ exStruct.h: Declaration of external structures.
/
/ Revisions:
/    Created:       2007 Sep      JC.Mitais
/    Modified: 
/ 
/ (c) CMI Industry
===========================================================================*/
// Current data
// ============
extern str_current s_cur;
extern str_simulation s_sim;

extern struct sps_entry_data sps_entry[MAX_NB_FUR];      // measurement structure
extern struct sps_entry_data sps_entry_old[MAX_NB_FUR];  // old measurement structure
extern struct sps_exit_data  sps_act_sp[MAX_NB_FUR];     // set points structure
extern struct sps_coil_data  sps_fin_sp[MAX_NB_FUR];     

// Coils structure
// ==================
extern struct coils s_coil;   // coils data lobal structure

extern struct fur_cfg_struct fur_cfg[MAX_NB_FUR];  // section static configuration
extern struct rtf_cfg_struct rtf_cfg[NB_FUR_RTF];  // rtf static configuration
extern struct rol_cfg_struct rol_cfg[NB_FUR_RTF];  // rtf static configuration
extern struct jet_cfg_struct jet_cfg;              // jet static configuration

// RTF
// ===
extern struct rtf_struct rtf[NB_FUR_RTF];          // strip and furnace structure
extern struct rtf_struct rtf1[NB_FUR_RTF];         // strip and furnace structure for spg first coil
extern struct rtf_struct rtf2[NB_FUR_RTF];         // strip and furnace structure for spg second coil


// JET
// ===
extern struct spg_entry_data_jet spg_entry_jet;    // cooling model entry structure
extern struct spg_exit_data_jet spg_exit_jet;      // cooling model exit struture

// Transcient
// ==========
extern struct strip_trans_exit trans_result_rtf[NB_FUR_RTF][MAX_NB_COIL];  // Transient situation calculation results structure
extern struct strip_trans_exit trans_result_old[NB_FUR_RTF];               // transient situation calculation results structure

// ROLLS
// =====
extern struct roll_profil rol_cmp[NB_FUR_RTF];              // Roll profile calculation structure for heat buckles risk during steady and transient states
extern struct roll_profil rol_cmp_risk[NB_FUR_RTF];         // Roll profile calculation structure for heat buckles risk during starting up
extern struct ns_STD::cl_ROL::str_rolls s_Trol[NB_FUR_RTF]; // Roll thermal

// DIVERS
// ======
//extern struct ns_STD::cl_BIS::str_bisra s_BIS;    // Bisra table strucuture

// ======
extern struct Str_Adp s_adp[MAX_NB_FUR];           // Self adaptation structure
extern struct str_alm s_alm;                       // Alam structure 

// FBA
// ===
extern struct ns_STD::cl_PID::fba_pid fba_stp[MAX_NB_FUR];                // strip PID structures
extern struct ns_STD::cl_PID::fba_pid fba_zon[MAX_NB_FUR][MAX_NB_ZON];    // zones PID structures

// Set-points
// ==========
extern struct str_fur_mea s_fur_mea[MAX_NB_FUR];
extern struct str_fur_spt s_fur_spt[MAX_NB_FUR];

