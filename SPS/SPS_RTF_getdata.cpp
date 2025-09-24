/*============================================================================
/ SPS_RTF_getdata: Recover required information from external device for LTOP use (RTF).
/ 
/ Revisions:
/  Created:       2005-Aug      P.Dubois
/  Modified: 
/  
/ (c) CMI Industry
/===========================================================================*/
#include "stdafx.h"

//============================================================================
bool SPS_RTF_getdata ()
{
   bool status = true;
   string sub_name = "SPS_RTF_getdata";

   bool TRC=ns_STD::cl_TRC::TRC_Sub( sub_name );

   if ( !s_cur.MeaTrkRead || !s_cur.MeaFurRead ) return false;
   float anticipation_time=0.f;

   if ( !s_coil.s_coil_data[0].valid ) return false;

   // For all RTF furnaces
   for (int i_rtf=0; i_rtf < NB_FUR_RTF; i_rtf++)
   {
      // Store the previous measurements
      // ===============================
      sps_entry_old[i_rtf] = sps_entry[i_rtf];

      if (TRC) ns_STD::cl_TRC::TRC_Write("INF", "SPS", sub_name, "SPS DATA READING.");

      // Initialization of the upstream coil currently in the RTF; if 2 coils are currently in the RTF,
      //      take into account the data of the one that is currently under the pyrometer.
      // ==============================================================================================
      if (i_rtf == RTH || sps_entry[i_rtf].s_mea.nCurCoil == -1)
      {
         sps_entry[i_rtf].s_cur_coil.thi     = s_coil.s_coil_data[0].thi;                    // Coil thickness             [m] 
         sps_entry[i_rtf].s_cur_coil.wid     = s_coil.s_coil_data[0].wid;                    // Coil width                 [m] 
         sps_entry[i_rtf].s_cur_coil.spd     = s_coil.s_coil_data[0].spd;                    // Speed for the coil         [m/s] 
         sps_entry[i_rtf].s_cur_coil.dummy   = s_coil.s_coil_data[0].dummy;                  // Dummy coil 
		   sps_entry[i_rtf].s_cur_coil.valid   = s_coil.s_coil_data[0].valid;          // test // Data validity

         sps_entry[i_rtf].s_cur_coil.tmp_tgt = s_coil.s_coil_data[0].tmp_tgt[i_rtf];         // Aimed temperature          [K] 
         sps_entry[i_rtf].s_cur_coil.tmp_upp = s_coil.s_coil_data[0].tmp_upp[i_rtf];         // Upper limit                [K] 
         sps_entry[i_rtf].s_cur_coil.tmp_low = s_coil.s_coil_data[0].tmp_low[i_rtf];         // Lower limit                [K] 
   
			sps_entry[i_rtf].s_cur_coil.nxt_tgt = s_coil.s_coil_data[1].tmp_tgt[i_rtf];         // Aimed temperature          [K] 


         // Power & tube temperatures in all RTF zones
         // ------------------------------------------
         // For all zones
         for ( int i_zon=0; i_zon < fur_cfg[i_rtf].nb_zon; i_zon++ ) 
         {
            sps_entry[i_rtf].s_cur_coil.pow_pct[i_zon] = s_coil.s_coil_data[0].pow_pct[i_rtf][i_zon];
            sps_entry[i_rtf].s_cur_coil.tmp_zon[i_zon] = s_coil.s_coil_data[0].tmp_zon[i_rtf][i_zon];
         } // For all zones

			// TEST
			// ****

			int i_reg = -1; // ?
						 
			// if it is dummy coil
			if (s_coil.s_coil_data[0].dummy == 1)
			{
				for (int i = 1; i < s_coil.nb_coil; i++)
				{
					if (s_coil.s_coil_data[i].dummy == 0)
					{
						i_reg = i;
						break;
					}
				}

				// if there is a regular coil in the queue
				if (i_reg > 0)
				{
					//sps_entry[i_rtf].s_cur_coil.spd = min(s_coil.s_coil_data[i_reg].spd,s_coil.s_coil_data[i_reg].spd * 0.92f * s_coil.s_coil_data[i_reg].thi / sps_entry[i_rtf].s_cur_coil.thi);  // Speed for the coil [m/s]
					sps_entry[i_rtf].s_cur_coil.spd = s_coil.s_coil_data[i_reg].spd * 0.94f * s_coil.s_coil_data[i_reg].thi / sps_entry[i_rtf].s_cur_coil.thi;
					sps_entry[i_rtf].s_cur_coil.spd = min(sps_entry[i_rtf].s_cur_coil.spd,s_coil.s_coil_data[0].spd_max_coil_ope);
					// Power & tube temperatures in all RTF zones
					// ------------------------------------------
					// For all zones
					for (int i_zon = 0; i_zon < fur_cfg[i_rtf].nb_zon; i_zon++)
					{
						// copy from the next regular coil in the queue
						sps_entry[i_rtf].s_cur_coil.pow_pct[i_zon] = s_coil.s_coil_data[i_reg].pow_pct[i_rtf][i_zon];
						sps_entry[i_rtf].s_cur_coil.tmp_zon[i_zon] = s_coil.s_coil_data[i_reg].tmp_zon[i_rtf][i_zon];
						
							//sps_entry[i_rtf].s_cur_coil.tmp_zon[i_zon] = s_coil.s_coil_data[i_reg].tmp_zon[i_rtf][i_zon] - ((sps_entry[i_rtf].s_cur_coil.thi / s_coil.s_coil_data[i_reg].thi*100.f) - 100.f);
							sps_entry[i_rtf].s_cur_coil.tmp_zon[i_zon] = min(sps_entry[i_rtf].s_cur_coil.tmp_zon[i_zon] , TUB_MAX_TMP);
						
						sps_entry[i_rtf].s_trans_coil.pow_pct[i_zon] = s_coil.s_coil_data[i_reg].pow_pct[i_rtf][i_zon];
						sps_entry[i_rtf].s_trans_coil.tmp_zon[i_zon] = s_coil.s_coil_data[i_reg].tmp_zon[i_rtf][i_zon];

					} // For all zones
				} // if there is a regular coil in the queue

				// if there are no regular coils in the queue
				else
				{
					sps_entry[i_rtf].s_cur_coil.spd = min(sps_entry[i_rtf].s_cur_coil.spd,s_coil.s_coil_data_last[0].spd * 0.94f  * s_coil.s_coil_data_last[0].thi / sps_entry[i_rtf].s_cur_coil.thi);  // Speed for the coil [m/s]
					sps_entry[i_rtf].s_cur_coil.spd = min(sps_entry[i_rtf].s_cur_coil.spd,s_coil.s_coil_data[0].spd_max_coil_ope);

					// Power & tube temperatures in all RTF zones
					// ------------------------------------------
					// For all zones
					for (int i_zon = 0; i_zon < fur_cfg[i_rtf].nb_zon; i_zon++)
					{
						// copy from the previous coil
						sps_entry[i_rtf].s_cur_coil.pow_pct[i_zon] = s_coil.s_coil_data_last[0].pow_pct[i_rtf][i_zon];		// ???
						sps_entry[i_rtf].s_cur_coil.tmp_zon[i_zon] = s_coil.s_coil_data_last[0].tmp_zon[i_rtf][i_zon];		// ???
					} // For all zones
				} // if there are no regular coils in the queue

			 } // if it is dummy coil

			// ****
			// TEST

      }
      else
      {
         int n_coil = sps_entry[i_rtf].s_mea.nCurCoil;
         // test
			sps_entry[i_rtf].s_cur_coil.thi     = s_coil.s_coil_data_last[n_coil].thi;             // Coil thickness          [m]
			sps_entry[i_rtf].s_cur_coil.wid     = s_coil.s_coil_data_last[n_coil].wid;             // Coil width              [m]
			sps_entry[i_rtf].s_cur_coil.spd     = s_coil.s_coil_data_last[n_coil].spd;             // Speed for the coil      [m/s]
			sps_entry[i_rtf].s_cur_coil.dummy   = s_coil.s_coil_data_last[n_coil].dummy;           // Dummy coil
			sps_entry[i_rtf].s_cur_coil.valid   = s_coil.s_coil_data_last[n_coil].valid;  // test  // Data validity
         // test
			sps_entry[i_rtf].s_cur_coil.tmp_tgt = s_coil.s_coil_data_last[n_coil].tmp_tgt[i_rtf];  // Aimed temperature       [K]
			sps_entry[i_rtf].s_cur_coil.tmp_upp = s_coil.s_coil_data_last[n_coil].tmp_upp[i_rtf];  // Upper limit             [K]
			sps_entry[i_rtf].s_cur_coil.tmp_low = s_coil.s_coil_data_last[n_coil].tmp_low[i_rtf];  // Lower limit             [K]

						
			sps_entry[i_rtf].s_cur_coil.nxt_tgt = s_coil.s_coil_data[0].tmp_tgt[i_rtf];         // Aimed temperature          [K] 

   
         // Power & tube temperatures in all zones                                        // test
         // --------------------------------------
         // For all zones
         for ( int i_zon=0; i_zon < fur_cfg[i_rtf].nb_zon; i_zon++ ) 
         {
            sps_entry[i_rtf].s_cur_coil.pow_pct[i_zon] = s_coil.s_coil_data_last[n_coil].pow_pct[i_rtf][i_zon];
            sps_entry[i_rtf].s_cur_coil.tmp_zon[i_zon] = s_coil.s_coil_data_last[n_coil].tmp_zon[i_rtf][i_zon];
         } // For all zones

			// TEST
			// ****

			int i_reg = -1; // ?

			// if it is dummy coil
			if (s_coil.s_coil_data_last[n_coil].dummy == 1)
			{
				for (int i = 1; i < n_coil + s_coil.nb_coil + 1; i++)
				{
					int i_coil = n_coil - i;
					if (i_coil < 0)
					{
						i_coil = i - n_coil - 1;
						if (s_coil.s_coil_data[i_coil].dummy == 0)
						{
							i_reg = i;
							break;
						}
					}
					else
					{
						if (s_coil.s_coil_data_last[i_coil].dummy == 0)
						{
							i_reg = i;
							break;
						}
					}
				}

				// if there is a regular coil in the queue
				if (i_reg > 0)
				{
					int i_coil = n_coil - i_reg;
					if (i_coil < 0)
					{
						i_coil = i_reg - n_coil - 1;

						//sps_entry[i_rtf].s_cur_coil.spd = min(s_coil.s_coil_data[i_coil].spd,s_coil.s_coil_data[i_coil].spd * 0.92f * s_coil.s_coil_data[i_coil].thi / sps_entry[i_rtf].s_cur_coil.thi);  // Speed for the coil [m/s]
						sps_entry[i_rtf].s_cur_coil.spd = s_coil.s_coil_data[i_coil].spd * 0.94f * s_coil.s_coil_data[i_coil].thi / sps_entry[i_rtf].s_cur_coil.thi;  // Speed for the coil [m/s]
						sps_entry[i_rtf].s_cur_coil.spd = min(sps_entry[i_rtf].s_cur_coil.spd,s_coil.s_coil_data[i_coil].spd_max_coil_ope);

						// Power & tube temperatures in all RTF zones
						// ------------------------------------------
						// For all zones
						for (int i_zon = 0; i_zon < fur_cfg[i_rtf].nb_zon; i_zon++)
						{
							// copy from the next regular coil in the queue
							sps_entry[i_rtf].s_cur_coil.pow_pct[i_zon] = s_coil.s_coil_data[i_coil].pow_pct[i_rtf][i_zon];
							sps_entry[i_rtf].s_cur_coil.tmp_zon[i_zon] = s_coil.s_coil_data[i_coil].tmp_zon[i_rtf][i_zon];
							sps_entry[i_rtf].s_trans_coil.pow_pct[i_zon] = s_coil.s_coil_data[i_coil].pow_pct[i_rtf][i_zon];
							sps_entry[i_rtf].s_trans_coil.tmp_zon[i_zon] = s_coil.s_coil_data[i_coil].tmp_zon[i_rtf][i_zon];

						} // For all zones
					}
					else
					{
						sps_entry[i_rtf].s_cur_coil.spd = min(sps_entry[i_rtf].s_cur_coil.spd,s_coil.s_coil_data_last[i_coil].spd * 0.94f * s_coil.s_coil_data_last[i_coil].thi / sps_entry[i_rtf].s_cur_coil.thi);  // Speed for the coil [m/s]
						sps_entry[i_rtf].s_cur_coil.spd = min(sps_entry[i_rtf].s_cur_coil.spd,s_coil.s_coil_data[i_coil].spd_max_coil_ope);

						// Power & tube temperatures in all RTF zones
						// ------------------------------------------
						// For all zones
						for (int i_zon = 0; i_zon < fur_cfg[i_rtf].nb_zon; i_zon++)
						{
							// copy from the next regular coil in the queue
							sps_entry[i_rtf].s_cur_coil.pow_pct[i_zon] = s_coil.s_coil_data_last[i_coil].pow_pct[i_rtf][i_zon];
							sps_entry[i_rtf].s_cur_coil.tmp_zon[i_zon] = s_coil.s_coil_data_last[i_coil].tmp_zon[i_rtf][i_zon];
						} // For all zones
					}
				} // if there is a regular coil in the queue

				// if there are no regular coils in the queue
				else
				{
					if (n_coil + 1 < MAX_NB_COIL)
					{
						sps_entry[i_rtf].s_cur_coil.spd = s_coil.s_coil_data_last[n_coil + 1].spd * 0.94f * s_coil.s_coil_data_last[n_coil + 1].thi / sps_entry[i_rtf].s_cur_coil.thi;  // Speed for the coil [m/s]
						sps_entry[i_rtf].s_cur_coil.spd = min(sps_entry[i_rtf].s_cur_coil.spd,s_coil.s_coil_data[0].spd_max_coil_ope);

						// Power & tube temperatures in all RTF zones
						// ------------------------------------------
						// For all zones
						for (int i_zon = 0; i_zon < fur_cfg[i_rtf].nb_zon; i_zon++)
						{
							// copy from the previous coil
							sps_entry[i_rtf].s_cur_coil.pow_pct[i_zon] = s_coil.s_coil_data_last[n_coil + 1].pow_pct[i_rtf][i_zon];		// ???
							sps_entry[i_rtf].s_cur_coil.tmp_zon[i_zon] = s_coil.s_coil_data_last[n_coil + 1].tmp_zon[i_rtf][i_zon];		// ???
						} // For all zones
					}
				} // if there are no regular coils in the queue

			} // if it is dummy coil

			// ****
			// TEST

		} // If the current coil is not the same than the RTF one

      //if ( !s_coil.s_coil_data[0].Trs_cal[i_rtf] ) break;

	   /* Transient between upstream & downstream coils */
	   /* ============================================= */
      if (rtf_cfg[i_rtf].trans_calc && s_coil.s_coil_data[0].dummy==0)
      {
         if (i_rtf == RTH || sps_entry[i_rtf].s_mea.nCurCoil == -1 && s_coil.s_coil_data[0].Trs_cal[i_rtf])
         {
	         /* Transition data */
	         sps_entry[i_rtf].s_trans_coil.tmp_tgt = min(max(trans_result_rtf[i_rtf][0].tmp_stp_rtf,sps_entry[i_rtf].s_cur_coil.tmp_low + 1.5f),sps_entry[i_rtf].s_cur_coil.tmp_upp - 1.5f);  // Aimed temperature [K] calculated by transient
	         sps_entry[i_rtf].s_trans_coil.spd     = trans_result_rtf[i_rtf][0].spd;          // Speed [m/s]

            // For all RTF zones
            for ( int i_zon=0; i_zon < fur_cfg[i_rtf].nb_zon; i_zon++ )
            {
               sps_entry[i_rtf].s_trans_coil.pow_pct[i_zon] = trans_result_rtf[i_rtf][0].pow_pct_rtf[i_zon];
               sps_entry[i_rtf].s_trans_coil.tmp_zon[i_zon] = trans_result_rtf[i_rtf][0].tmp_tub[i_zon];
            } // For all RTF zones
         }
         else
         {
	         /* Transition data */
	         sps_entry[i_rtf].s_trans_coil.tmp_tgt = trans_result_old[i_rtf].tmp_stp_rtf;  // Aimed temperature [K] calculated by transient
	         sps_entry[i_rtf].s_trans_coil.spd     = trans_result_old[i_rtf].spd;          // Speed [m/s]

            // For all RTF zones
            for ( int i_zon=0; i_zon < fur_cfg[i_rtf].nb_zon; i_zon++ )
            {
               sps_entry[i_rtf].s_trans_coil.pow_pct[i_zon] = trans_result_old[i_rtf].pow_pct_rtf[i_zon];
               sps_entry[i_rtf].s_trans_coil.tmp_zon[i_zon] = trans_result_old[i_rtf].tmp_tub[i_zon];
            } // For all RTF zones
         }
      }
      else
      {
	      /* Transition data */
			if (s_coil.s_coil_data[0].dummy==1 && i_rtf == RTH)
				sps_entry[i_rtf].s_trans_coil.tmp_tgt = min(max(sps_entry[i_rtf].s_cur_coil.tmp_tgt + (trans_result_rtf[i_rtf][0].tmp_stp_rtf - trans_result_rtf[i_rtf][1].tmp_stp_rtf),sps_entry[i_rtf].s_cur_coil.tmp_low + 1.5f),sps_entry[i_rtf].s_cur_coil.tmp_upp - 1.5f);  // Aimed temperature [K] calculated by transient
			else
				sps_entry[i_rtf].s_trans_coil.tmp_tgt = sps_entry[i_rtf].s_cur_coil.tmp_tgt;     // Aimed temperature [K] calculated by transient
	      sps_entry[i_rtf].s_trans_coil.spd     = sps_entry[i_rtf].s_cur_coil.spd;         // Speed [m/s]

         // For all RTF zones
         for ( int i_zon=0; i_zon < fur_cfg[i_rtf].nb_zon; i_zon++ )
         {
            sps_entry[i_rtf].s_trans_coil.pow_pct[i_zon] = sps_entry[i_rtf].s_cur_coil.pow_pct[i_zon];
            sps_entry[i_rtf].s_trans_coil.tmp_zon[i_zon] = sps_entry[i_rtf].s_cur_coil.tmp_zon[i_zon];
         } // For all RTF zones
      }

		if (sps_entry[RTH].s_cur_coil.dummy == 1 && sps_entry[RTS].s_cur_coil.dummy == 0) // if dummy coil has still not fully enter RTS 
		{
			sps_entry[RTH].s_cur_coil.spd = sps_entry[RTS].s_cur_coil.spd;
		}

      float dH_rt = 0.f;
      // For all RTF zones
      for (int i_zon=0; i_zon < fur_cfg[i_rtf].nb_zon; i_zon++)
      {
         dH_rt += ns_STD::cl_BIS::BISRA_Enth(2, sps_entry[i_rtf].s_trans_coil.tmp_zon[i_zon]) -
                  ns_STD::cl_BIS::BISRA_Enth(2, s_coil.s_coil_data[0].tmp_zon[i_rtf][i_zon]);
      } // For all RTF zones
      dH_rt /= float(fur_cfg[i_rtf].nb_zon);

      // Total power used
      // ----------------
      float power_total = 0.f;
      float pow_min_rtf = 0.f;
      // For all RTF zones
      for ( int i_zon=0; i_zon < fur_cfg[i_rtf].nb_zon; i_zon++ )
      {
         power_total += min((100.f - trans_result_rtf[i_rtf][0].pow_pct_rtf[i_zon]), 30.f) / 
                        100.f * rtf_cfg[i_rtf].MAX_POWER[i_zon] * s_adp[i_rtf].Effi[i_zon] / 2.f;

		   pow_min_rtf += 5.f / 100.f * rtf_cfg[i_rtf].MAX_POWER[i_zon] * s_adp[i_rtf].Effi[i_zon];
      }

      //pow_min_rtf *= -1.f;           // test

      // Anticipation time calculation [s]
      // ---------------------------------
      if (dH_rt >= 200.f)
	   {
         SPS_RTF_anticipation( &anticipation_time, 20.f + TK, 20.f + TK, s_coil.s_coil_data[0].thi,
                               s_coil.s_coil_data[0].wid, dH_rt, power_total,
										 trans_result_rtf[i_rtf][0].DH_roll_zon, i_rtf ); 
	   }

      else
	   {
         SPS_RTF_anticipation( &anticipation_time, 20.f + TK, 20.f + TK, s_coil.s_coil_data[0].thi,
                               s_coil.s_coil_data[0].wid, dH_rt, pow_min_rtf,
										trans_result_rtf[i_rtf][0].DH_roll_zon, i_rtf );
	   }

      // Store anticipation time calculation
      sps_entry[i_rtf].s_trans_coil.anticipation_time = anticipation_time;

      if (TRC) 
      {
         int lst_zon = fur_cfg[i_rtf].nb_zon - 1;
         ns_STD::cl_TRC::TRC_Write("INF", "SPS", sub_name, "tmp_tub[NB_ZON_RTF-1] : %.0f", sps_entry[i_rtf].s_mea.tmp_zon[lst_zon]-TK);
         ns_STD::cl_TRC::TRC_Write("INF", "SPS", sub_name, "pow_pct[NB_ZON_RTF-1] : %.0f", sps_entry[i_rtf].s_mea.heat_pow_pct[lst_zon] );
         ns_STD::cl_TRC::TRC_Write("INF", "SPS", sub_name, "spd                   : %.0f", sps_entry[i_rtf].s_mea.spd );
         ns_STD::cl_TRC::TRC_Write("INF", "SPS", sub_name, "tmp_stp               : %.0f", sps_entry[i_rtf].s_mea.tmp_stp-TK);
      }

	   // Store PID value
	   // ---------------
	   float P_stp_lst = fba_stp[i_rtf].p;
      float I_stp_lst = fba_stp[i_rtf].i;

      // Store Tubes PID
      // ----------------
	   float P_tub_lst[MAX_NB_ZON], I_tub_lst[MAX_NB_ZON];

      // For all RTF zones
      for ( int i_zon=0; i_zon < fur_cfg[i_rtf].nb_zon; i_zon++ ) 
      {
         P_tub_lst[i_zon] = fba_zon[i_rtf][i_zon].p;
         I_tub_lst[i_zon] = fba_zon[i_rtf][i_zon].i;
	   } // For all RTF zones

      // Specific SPS initialization
      // ---------------------------
      SPS_Specific::SPS_getdata();

      // Set-points calculation
      // ======================
      status = SPS_RTF(i_rtf);
      if ( !status )
      {
         ns_STD::cl_TRC::ERR_Write("SPS", sub_name, "error in SPS_RTF");
         return status;
      }

	   // Store back PID value
	   // ====================
	   fba_stp[i_rtf].p = P_stp_lst;
      fba_stp[i_rtf].i = I_stp_lst;

      // Store back Tubes PID
      // ====================
      for ( int i_zon=0; i_zon < fur_cfg[i_rtf].nb_zon; i_zon++ ) 
      {
         fba_zon[i_rtf][i_zon].p = P_tub_lst[i_zon];
         fba_zon[i_rtf][i_zon].i = I_tub_lst[i_zon];
      } // For all RTF zones
   } // For all RTF furnaces

   return status;
}
