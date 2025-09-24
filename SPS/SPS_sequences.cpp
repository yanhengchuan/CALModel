/*============================================================================
/ SPS_sequences.cpp
/ 
/ Sequences required to compute all data and states when an event occurs. 
/ 
/ Revisions:
/  Created:       2007-Sep       X.Fournaud
/  Modified: 
/  
/ (c) CMI Industry
//===========================================================================*/
#include "stdafx.h"

bool SPS_Sequence::SPS_Compute()
{
	bool status=true;
	string sub_name = "LTOP_Compute_SPS";

	// On line set points calculation for RTF
	// --------------------------------------------
	status = SPS_RTF_getdata();

   if (!status) return false;

   // For all non_RTF furnaces
   for (int i_fur=NB_FUR_RTF; i_fur < MAX_NB_FUR; i_fur++)
   {
      // If the current coil is the same than the RTF one
		if (sps_entry[i_fur].s_mea.nCurCoil == -1)
		{
			// test
         sps_entry[i_fur].s_cur_coil.thi   = s_coil.s_coil_data[0].thi;                         // Coil thickness          [m]
			sps_entry[i_fur].s_cur_coil.wid   = s_coil.s_coil_data[0].wid;                         // Coil width              [m]
			sps_entry[i_fur].s_cur_coil.spd   = s_coil.s_coil_data[0].spd;                         // Speed for the coil      [m/s]
			sps_entry[i_fur].s_cur_coil.dummy = s_coil.s_coil_data[0].dummy;                       // Dummy coil
			sps_entry[i_fur].s_cur_coil.valid = s_coil.s_coil_data[0].valid;				            // Data validity
         // test
         sps_entry[i_fur].s_cur_coil.tmp_tgt = s_coil.s_coil_data[0].tmp_tgt[i_fur];            // Aimed temperature       [K]
			sps_entry[i_fur].s_cur_coil.tmp_upp = s_coil.s_coil_data[0].tmp_upp[i_fur];            // Upper limit             [K]
			sps_entry[i_fur].s_cur_coil.tmp_low = s_coil.s_coil_data[0].tmp_low[i_fur];            // Lower limit             [K]
   
         // Power & tube temperatures in all zones                                     // test
         // --------------------------------------
         // For all zones
         for ( int i_zon=0; i_zon < fur_cfg[i_fur].nb_zon; i_zon++ ) 
         {
            sps_entry[i_fur].s_cur_coil.pow_pct[i_zon] = s_coil.s_coil_data[0].pow_pct[i_fur][i_zon];
            sps_entry[i_fur].s_cur_coil.tmp_zon[i_zon] = s_coil.s_coil_data[0].tmp_zon[i_fur][i_zon];
         } // For all zones
		} // If the current coil is the same than the RTF one

		// If the current coil is not the same than the RTF one
		else
		{
         int n_coil = sps_entry[i_fur].s_mea.nCurCoil;
         // test
			sps_entry[i_fur].s_cur_coil.thi     = s_coil.s_coil_data_last[n_coil].thi;             // Coil thickness          [m]
			sps_entry[i_fur].s_cur_coil.wid     = s_coil.s_coil_data_last[n_coil].wid;             // Coil width              [m]
			sps_entry[i_fur].s_cur_coil.spd     = s_coil.s_coil_data_last[n_coil].spd;             // Speed for the coil      [m/s]
			sps_entry[i_fur].s_cur_coil.dummy   = s_coil.s_coil_data_last[n_coil].dummy;           // Dummy coil
			sps_entry[i_fur].s_cur_coil.valid   = s_coil.s_coil_data_last[n_coil].valid;           // Data validity
         // test
			sps_entry[i_fur].s_cur_coil.tmp_tgt = s_coil.s_coil_data_last[n_coil].tmp_tgt[i_fur];  // Aimed temperature       [K]
			sps_entry[i_fur].s_cur_coil.tmp_upp = s_coil.s_coil_data_last[n_coil].tmp_upp[i_fur];  // Upper limit             [K]
			sps_entry[i_fur].s_cur_coil.tmp_low = s_coil.s_coil_data_last[n_coil].tmp_low[i_fur];  // Lower limit             [K]
   
         // Power & tube temperatures in all zones                                        // test
         // --------------------------------------
         // For all zones
         for ( int i_zon=0; i_zon < fur_cfg[i_fur].nb_zon; i_zon++ ) 
         {
            sps_entry[i_fur].s_cur_coil.pow_pct[i_zon] = s_coil.s_coil_data_last[n_coil].pow_pct[i_fur][i_zon];
            sps_entry[i_fur].s_cur_coil.tmp_zon[i_zon] = s_coil.s_coil_data_last[n_coil].tmp_zon[i_fur][i_zon];
         } // For all zones
		} // If the current coil is not the same than the RTF one
   } // For all non_RTF furnaces

   // =======
	// TNL
   // =======
   // if tunnel use
   if (NB_FUR_TNL > 0)
   {
      sps_fin_sp[TNL] = sps_entry[TNL].s_cur_coil;
      sps_act_sp[TNL].tmp_stp = sps_fin_sp[TNL].tmp_tgt;

      // for all zones
      sps_act_sp[TNL].tmp_zon[0] = sps_act_sp[TNL-1].tmp_zon[NB_ZON_RTS-1];
      for (int i_zon = 1; i_zon <NB_ZON_TNL; i_zon++)
      {
         sps_act_sp[TNL].tmp_zon[i_zon] = sps_act_sp[TNL].tmp_zon[i_zon-1];
      }
   } // if tunnel use

   // =======
	// SLW
   // =======
   // if slow cooling section use
   if (NB_FUR_SLW > 0)
   {
      sps_fin_sp[SLW] = sps_entry[SLW].s_cur_coil;
      sps_act_sp[SLW].tmp_stp = sps_fin_sp[SLW].tmp_tgt;
      status = SPS_SLW(sps_fin_sp[SLW]);
		if ( !status )
      {
         ns_STD::cl_TRC::ERR_Write("SPS", sub_name, "error in SPS_RTF_SLW");
         return status;
      }
   } // if slow cooling section use

   // =======
	// JET
   // =======
   // if jet cooling section use
   if (NB_FUR_JET > 0)
   {
      // If the current coil is the same than the RTF one
		if (sps_entry[JET].s_mea.nCurCoil == -1)
		{
			// For all jet cooling zones
			for ( int i_zon=0; i_zon < NB_ZON_JET; i_zon++)
			{
				sps_entry[JET].s_cur_coil.pow_pct[i_zon] = s_coil.s_coil_data[0].pow_pct[JET][i_zon];
				sps_entry[JET].s_trans_coil.pow_pct[i_zon] = s_coil.s_coil_data[0].pow_pct[JET][i_zon];
			}
         if (USE_JET_SPEED)
            sps_entry[JET].s_trans_coil.spd = s_coil.s_coil_data[0].spd_max_th[RTH];
         else
            sps_entry[JET].s_trans_coil.spd = trans_result_rtf[RTH][0].spd;
			   sps_entry[JET].s_trans_coil.tmp_tgt = s_coil.s_coil_data[0].tmp_tgt[JET];
		} // If the current coil is the same than the RTF one

		// If the current coil is not the same than the RTF one
		else
		{
         int n_coil = sps_entry[JET].s_mea.nCurCoil;

			// For all jet cooling zones
			for ( int i_zon=0; i_zon < NB_ZON_JET; i_zon++)
			{
				sps_entry[JET].s_cur_coil.pow_pct[i_zon] = s_coil.s_coil_data_last[n_coil].pow_pct[JET][i_zon];
				sps_entry[JET].s_trans_coil.pow_pct[i_zon] = s_coil.s_coil_data_last[n_coil].pow_pct[JET][i_zon];
			}
          if (USE_JET_SPEED)
             sps_entry[JET].s_trans_coil.spd = s_coil.s_coil_data_last[n_coil].spd_max_th[RTH];
          else
			   sps_entry[JET].s_trans_coil.spd = trans_result_old[RTS].spd;
			   sps_entry[JET].s_trans_coil.tmp_tgt = s_coil.s_coil_data_last[n_coil].tmp_tgt[JET];
		} // If the current coil is not the same than the RTF one

      sps_fin_sp[JET] = sps_entry[JET].s_cur_coil;
      sps_act_sp[JET].tmp_stp = sps_fin_sp[JET].tmp_tgt;
      status = SPS_JET(sps_fin_sp[JET]);
		if ( !status )
      {
         ns_STD::cl_TRC::ERR_Write("SPS", sub_name, "error in SPS_RTF_JET");
         return status;
      }
   } // if jet cooling section use
	
	// ===============
	// OVG
	// ===============
   if ( NB_FUR_OVG > 0) 
   {
      sps_fin_sp[OVG] = sps_entry[OVG].s_cur_coil;
      sps_act_sp[OVG].tmp_stp = sps_fin_sp[OVG].tmp_tgt;
      status = SPS_OVG(sps_fin_sp[OVG]);
		if ( !status )
      {
         ns_STD::cl_TRC::ERR_Write("SPS", sub_name, "error in SPS_RTF_OVG");
         return status;
      }
   }

	// ===================
	// FCS
	// ===================
   if ( NB_FUR_FCS > 0) 
   {
      sps_fin_sp[FCS] = sps_entry[FCS].s_cur_coil;
      sps_act_sp[FCS].tmp_stp = sps_fin_sp[FCS].tmp_tgt;
      status = SPS_FCS(sps_fin_sp[FCS]);
		if ( !status )
      {
         ns_STD::cl_TRC::ERR_Write("SPS", sub_name, "error in SPS_RTF_FCS");
         return status;
      }
   }
   
	// ===================
	// EXT
	// ===================
   if ( NB_FUR_EXT > 0) 
   {
      sps_fin_sp[EXT] = sps_entry[EXT].s_cur_coil;
      sps_act_sp[EXT].tmp_stp = sps_fin_sp[EXT].tmp_tgt;
      status = SPS_EXT(sps_fin_sp[EXT]);
		if ( !status )
      {
         ns_STD::cl_TRC::ERR_Write("SPS", sub_name, "error in SPS_RTF_EXT");
         return status;
      }
   }
      
	// ===================
	// APC
	// ===================
   if ( NB_FUR_APC > 0) 
   {
      sps_fin_sp[APC] = sps_entry[APC].s_cur_coil;
      sps_act_sp[APC].tmp_stp = sps_fin_sp[APC].tmp_tgt;
      status = SPS_APC(sps_fin_sp[APC]);
		if ( !status )
      {
         ns_STD::cl_TRC::ERR_Write("SPS", sub_name, "error in SPS_RTF_APC");
         return status;
      }
   }

   // All SPS update
   // --------------
	PostThreadMessage(HandleSQLThread, MSG_SQL_SPS, 0, 0);

   // History
   // --------
	PostThreadMessage(HandleSQLThread, MSG_SQL_HIS, 0, 0);
   
	static string id_start = "";

	if (s_coil.s_coil_data[0].id != id_start)
	{
		status = SQL_EMI_HIS_INSERT();
		if ( !status ) ns_STD::cl_TRC::ERR_Write("SPS", sub_name, "error in SQL_EMI_HIS_INSERT");

		id_start = s_coil.s_coil_data[0].id;
	}

   // Reports
   // -------
   PostThreadMessage(HandleSQLThread, MSG_SQL_REP, 0, 0);

	return status;
}
