/*============================================================================
exFunction.h: Declaration of common external functions.

 Revisions:
  Created:       2007 Sep      X.Fournaud
  Modified: 
  
 (c) CMI Industry
===========================================================================*/
// COMMON
// ======
extern bool JET_STP (float Tmp_in, int Bisra, float spd, float wid, float thi, float Spd_blow[], float pow_jet[]);
extern bool SPG_JET (float Tmp_in, int Bisra, float spd, float tmp_tgt, float wid, float thi, 
                     float cool_rate_spt[], float Tmp_stp_out[], float pow_jet[], float cooling_rate[], bool max_spd);

// RTF calculations
extern bool RTF_strip( struct rtf_struct *rtf, bool roll, float tmp_tub[], float *tmp_stp );
extern void RTF_power( struct rtf_struct *rtf, float power[] );
extern void RTF_ini_array( struct rtf_struct *rtf );
extern void RTF_array_compare( struct rtf_struct *rtf, bool *ok );
extern bool RTF_model_strip( struct rtf_struct *rtf,bool roll );
extern bool RTF_heat_exchange( struct rtf_struct *rtf,int elt, float *e_q );
extern bool RTF_model( struct rtf_struct *rtf, bool roll );
extern void RTF_static_init( struct rtf_struct *rtf, int nb_zon, struct rtf_cfg_struct rtf_cfg, int n_pdt, 
                             int n_cla, int n_coil);
extern void RTF_dynamic_init( struct rtf_struct *rtf, float thi, float wid, float emi, float entry_tmp );

// Utilities
// ---------
extern float STEEL_specific_heat( float t );

// Others
// ======
extern void AddMsgToWindow(string strText );
extern bool CMP_Cycle(time_t current_time);

extern bool MessageSPGtoLtop();
extern bool MessageSPStoLtop();
extern bool MessageFromLtop();
extern float UTL_Emi_RTF(int n_coil, int n_fur);

// SQL
// ====
extern bool SQL_TRACES_INSERT(string LevelTxt, string Module, string Method, string Message);
extern void SQL_R_ERR(string sub_name, string Procedure, string Description);

extern bool SQL_DIMENSIONS_UPDATE ();        // Update general funace information
extern bool SQL_CFG_RTF_UPDATE (int n_rtf);  // Update RTF configuration
extern bool SQL_CFG_FUR_UPDATE (int n_fur);  // Update sections configuration

extern bool SQL_COILS_DATA_UPDATE();         // Update coils queue information
extern void SQL_SPG_RTF_UPDATE(int n_fur);   // Update RTF stable state calculation results for each coils in queue

extern void SQL_SPS_RTF_UPDATE(int n_rtf);   // Update RTF SPS results
extern void SQL_SPS_FUR_UPDATE(int n_fur);   // Update non-RTF SPS results

extern void SQL_TRACKING_UPDATE();        // Update remaining time, position of the next weld
extern bool SQL_PID_SELECT(int n_fur);      // Select regulation parameters
extern bool SQL_PRM_SELECT(int n_rtf);      // Select risk parameter
extern bool SQL_SIM_SELECT();   // Select coils data for Simulation

extern bool SQL_CkeckHMI ();        // Check if the model has to take some information for the SQL database
extern bool SQL_Upd_CkeckHMI ();    // Update the SQL database to give the information that information has been stored
extern bool SQL_HIS_INSERT ();      // Insert one line of history for all process values
extern void SQL_EMISSIVITY_UPDATE(int n_fur);   // Update the emissivity adapted parameters
extern bool SQL_EMISSIVITY_SELECT(int n_fur);   // Select updated information for emissivities classes 
extern bool SQL_EFFICIENCY_UPDATE(int n_fur);   // Update efficacity dapted paramters
extern bool SQL_EFFICIENCY_SELECT(int n_fur);   // Select the efficacity parameters for model initialization
extern bool SQL_ADAPTATION_SELECT(int n_fur);   // Select strip/rolls thermal exchange adaptation parameters
extern bool SQL_ADAPTATION_UPDATE(int n_fur);   // Insert the strip/rolls thermal exchange parameters (calculation not done for the moment)
extern bool SQL_ROL_CFG_UPDATE(int n_rtf);      // Update roll crown information
extern bool SQL_EMI_HIS_INSERT ();       // Insert one point per coil of emissity

extern bool SQL_ROL_HIS_INSERT (int Idx_Rec, int Idx_Pt, float risk, float wid, float thi, float stp_tmp, 
                                float roll_tmp, float tub_tmp, float mesh[], int n_rtf);
extern bool SQL_Upd_CkeckHMI_Record_IDX();      // Update the Index of the last risk history staorage

// Communications
// ==============
extern bool SQL2_I_MSG_SPT();
extern bool SQL2_I_MSG_COIL();
extern bool SQL2_I_MSG_MEA_HEA();
extern bool SQL2_I_MSG_MEA_SPD();
extern bool SQL2_R_MEA_HEA();
extern bool SQL2_R_MEA_SPD();
extern bool SQL2_R_MSG_COIL();
//extern bool SQL2_R_MSG_COIL_START();
extern bool SQL2_I_MSG_SPT_HEA();
extern bool SQL2_I_MSG_SPT_SPD();

// RTF
// ===
extern bool SPS_RTF(int n_fur);
extern bool SPS_RTF_decr_spd( struct sps_coil_data *sps_fin_sp_rtf, struct sps_mea_coil_data *sps_mea_rtf, 
                              struct sps_exit_data *sps_act_sp_rtf, float delta_po_rtf[], int n_fur );
extern bool SPS_RTF_incr_spd( struct sps_coil_data *sps_fin_sp_rtf, struct sps_mea_coil_data *sps_mea_rtf, 
                              struct sps_exit_data *sps_act_sp_rtf, float delta_po_rtf[], int n_fur );
extern bool SPG_RTF_Max_Spd( bool roll, struct rtf_struct *rtf, float tmp_upp, float tmp_in, float *spd );
extern bool SPS_RTF_getdata();

extern bool SPS_JET(struct sps_coil_data sps_fin_sp_jet);
extern bool SPS_SOA(struct sps_coil_data sps_fin_sp_rtf);
extern bool SPS_SLW(struct sps_coil_data sps_fin_sp_slw);
extern bool SPS_OVG(struct sps_coil_data sps_fin_sp_ovg);
extern bool SPS_FCS(struct sps_coil_data sps_fin_sp_fcs);
extern bool SPS_EXT(struct sps_coil_data sps_fin_sp_ext);
extern bool SPS_APC(struct sps_coil_data sps_fin_sp_apc);
extern bool SPS_TRK_Section(float x_Pyro, int *nCurCoil, float *lgt, float *tim);

// Basic
// =====
extern void SPS_RTF_anticipation( float *anticipation_time, float T_ant, float T_stbl, float thi, float wid,
                                  float dt_tub, float power, float dt_roll, int n_rtf);
//extern bool SPS_PID( struct fba_pid *pid );
extern bool INIT();
extern float SPS_DEL_first_roll_prf(bool first_time, float dt, int n_fur);

// SIM
// ===
extern bool SIM_INI();
extern bool SIM_Mea_trk();
extern bool SIM_Mea_rtf();
extern bool SIM_Mea_tnl();
extern bool SIM_Mea_slw();
extern bool SIM_Mea_jet();
extern bool SIM_Mea_ovg();
extern bool SIM_Mea_fcs();
extern bool SIM_Main();
extern bool SIM_Read();

// INIT
// ===
extern bool SPS_INI_Alm ();

// UTL
// ===
extern __inline __int64 FileTimeTo64i (FILETIME ft);
extern void SPS_ADA_effi(int n_coil, int n_fur);
extern void SPS_ADA_Emis(int coil_cur, bool roll, int n_fur);
extern void SPS_ADA_exchgeCoeff(bool Anticipation_time, float Tmp_rtf, int n_fur);
extern bool SPS_ADA_Strip_tmp(int coil_cur, bool roll, int i_pdt, int i_cla, float alpha_est,
                              float *Exit_strip, int n_fur);

// Configuration
// -------------
extern bool CFG_Read();
extern bool float_KeyWordTreat(string keyword[], int MaxVal, float Tab[]);
extern bool int_KeyWordTreat(string keyword[], int MaxVal, int Tab[]);

/*============================================================================
// Declaration of external SPG functions.
===========================================================================*/
// COOLING
// =======
// JET cooling maximum speed calculation function
extern bool SPG_JET_Max_Spd( float Tmp_in_jet, float tmp_tgt[JET], float thi, float wid, float Cool_rate[], float *spd );

// RTF
// ===
extern bool SPG_RTF( int i1, int i2, int n_fur );
extern bool SPG_RTF_Getdata();
extern bool SPG_RTF_Tube_Tmp( struct rtf_struct *rtf, bool roll, float rtf_tar, float *t_t_cal, float tmp_tub[],
                                 float thi2, float wid2  );
extern void SPG_RTF_ini_rol_tmp( struct rtf_struct *rtf_entry, struct rtf_struct *rtf_exit, bool strip,
                                 float stp_rtf[], struct strip_trans_exit *data_trans );
extern bool SPG_RTF_Thin_Thick( struct rtf_struct *rtf_first, struct rtf_struct *rtf_second, 
                                struct strip_trans_entry *data_first, struct strip_trans_entry *data_second, 
                                struct strip_trans_exit *data_trans, int i1, int i2 );
extern bool SPG_RTF_Thick_Thin( struct rtf_struct *rtf_first, struct rtf_struct *rtf_second, 
                                struct strip_trans_entry *data_first, struct strip_trans_entry *data_second, 
                                struct strip_trans_exit *data_trans, int i1, int i2 );
extern bool SPG_RTF_ColdThin_HotThick( struct rtf_struct *rtf_first, struct rtf_struct *rtf_second, 
                                       struct strip_trans_entry *data_first, struct strip_trans_entry *data_second, 
                                       struct strip_trans_exit *data_trans, int i1, int i2 );
extern bool SPG_RTF_HotThin_ColdThick( struct rtf_struct *rtf_first, struct rtf_struct *rtf_second, 
                                       struct strip_trans_entry *data_first, struct strip_trans_entry *data_second, 
                                       struct strip_trans_exit *data_trans, int i1, int i2 );
extern bool SPG_RTF_HotThick_ColdThin( struct rtf_struct *rtf_first, struct rtf_struct *rtf_second, 
                                       struct strip_trans_entry *data_first, struct strip_trans_entry *data_second, 
                                       struct strip_trans_exit *data_trans, int i1, int i2 );
extern bool SPG_RTF_ColdThick_HotThin( struct rtf_struct *rtf_first, struct rtf_struct *rtf_second, 
                                       struct strip_trans_entry *data_first, struct strip_trans_entry *data_second, 
                                       struct strip_trans_exit *data_trans, int i1, int i2 );
extern void SPG_RTF_ratio_tube( float tmp_tub_ref, float wid, float thickness, float tmp_tub[],
                                float thi2, float wid2, int nb_zon, int n_rtf );
extern bool SPG_RTF_Transient   ( struct rtf_struct *rtf_first, struct rtf_struct *rtf_second, 
                                  struct strip_trans_entry *data_first, struct strip_trans_entry *data_second, 
                                  struct strip_trans_exit *data_trans, int i1, int i2 );
extern bool SPG_RTF_Max_Spd( bool roll, struct rtf_struct *rtf, float tmp_upp, float tmp_in, float *spd );

// ROL
// ===
extern bool SPG_ROL( struct roll_profil *rol_cmp );