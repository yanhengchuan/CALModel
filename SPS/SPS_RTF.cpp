/*============================================================================
/ SPS_RTF: Set point switching RTF
/ 
/ Revisions:
/  Created:       2005-Aug      P.DUBOIS
/  Modified: 
/
/ Remarks:
/     sps_act_sp.Trans_Type =       -1    --> Error (transition not computed)
/                                   0     --> Line stoppage
/                                   1     --> Transition
/                                   2     --> stable
/                                   10    --> speed decrease
/                                   20    --> speed increase
/                                   30    --> same speed
/                                   100   --> strip temperature is close to the upper limits
/                                   200   --> strip temperature is close to the lower limits
/                                   1000  --> dummy coils
/  
/ (c) CMI Industry
/===========================================================================*/
#include "stdafx.h"

//===============================================================================
bool SPS_RTF(int n_rtf)
{   
   bool status = true;
   string sub_name = "SPS_RTF";

   const float DELTA_SPD = 1.f/60.f;
   const int jcount_max= 5;

   static bool flag_tracking_rtf, flag_pid_rtf, flag_old_pid_rtf;
   static int count_spd_decr = 0;
   float coef = 1.f / 1000.f / sps_entry[n_rtf].s_cur_coil.thi;

	time_t current_time;
	time(&current_time);

   struct sps_mea_coil_data sps_mea_rtf;

	float hot_prf[MAX_NB_MESH];
   float pow_pct, pos_ramp_po_spt, neg_ramp_po_spt, diff_time, risk;

   sps_act_sp[n_rtf].Trans_Type = -1;
   sps_mea_rtf = sps_entry[n_rtf].s_mea;
   sps_act_sp[n_rtf].ctl_pow = true;

	bool last_dum = false;

	bool trans_spd_up = false;

   // Store previous power set-point.
   // -------------------------------
   // For all zones
   for ( int i_zon=0; i_zon < fur_cfg[n_rtf].nb_zon; i_zon++ ) 
   {
      sps_act_sp[n_rtf].heat_pow_old[i_zon] = sps_act_sp[n_rtf].heat_pow_pct[i_zon];
   } // For all zones

   // ============================
	// RTF Power control by default
	// ============================
   // Wait for 3 iterations before controlling
	//if (s_cur.jcount_rtf > 3)  
	if (s_cur.jcount_rtf > 1)  
	{
		sps_act_sp[n_rtf].ctl_pow = true;

		if (s_coil.s_coil_data[0].valid && s_coil.s_coil_data[1].valid)
		{
			sps_act_sp[n_rtf].Spd_Valid = true;
			sps_act_sp[n_rtf].Valid = true;
		}

		else
		{
         sps_act_sp[n_rtf].Spd_Valid = false;
			sps_act_sp[n_rtf].Valid = false;
         sps_act_sp[n_rtf].traction = 0.f;

         // If the speed is in manual
         if ( !sps_entry[n_rtf].s_mea.spd_auto ) sps_act_sp[n_rtf].spd = sps_entry[n_rtf].s_mea.spd;
		}
	} // Wait for 3 iterations before controlling

	else
	{
      sps_act_sp[n_rtf].Spd_Valid = false;
		sps_act_sp[n_rtf].Valid = false;
      sps_act_sp[n_rtf].traction = 0.f;
	}

   // Mistracking
	if ( sps_entry[n_rtf].s_cur_coil.thi >= 1.4f/1000.f && sps_entry[n_rtf].s_cur_coil.wid <= 1.f )
	    sps_act_sp[n_rtf].traction = 5.f;

   // Buckle risks
	if (sps_entry[n_rtf].s_cur_coil.thi <= 0.6f/1000.f && sps_entry[n_rtf].s_cur_coil.wid >= 1.3f )
	    sps_act_sp[n_rtf].traction = -5.f; 

   //flag_tracking_rtf = !sps_entry[n_rtf].s_mea.spd_auto;
   //// If the speed had decreased quickly due to a problem of tracking. !?!
   //if ( flag_tracking_rtf || (!sps_entry[n_rtf].s_mea.spd_auto && sps_entry[n_rtf].s_cur_coil.thi >= 1.f/1000.f && 
   //                           sps_entry[n_rtf].s_cur_coil.wid <= 1.f) )
   //{
   //   flag_tracking_rtf = true;
   //   return status;
   //}

	// =========================
	// Line stoppage - LINE STOP 
	// =========================
	// If the measured speed is < MIN_SPEED, switching to zone temperature control and 
   //    all set-points initialized to 600°C
	//if ( SIMULATION && sps_entry[n_rtf].s_mea.spd < MIN_SPEED ||
 //      !SIMULATION && sps_entry[n_rtf].s_mea.spd < MIN_SPEED + 1.f / 60.f )
	if ( sps_entry[n_rtf].s_mea.spd < MIN_SPEED )                                          // test
	{
		if (n_rtf == RTH)
		{
			if (s_cur.Line_running)
			{
				s_cur.record_IDX ++;
				if (s_cur.record_IDX > 3000) s_cur.record_IDX = 0;

				SQL_Upd_CkeckHMI_Record_IDX();      

            ns_STD::cl_TRC::TRC_Write("INF", "TRK", sub_name, "%s: Measured speed is not valid: %.0f m/min", fur_cfg[n_rtf + RTH].fur_name.c_str(),
               sps_entry[n_rtf].s_mea.spd * 60.f);
			}

			float dt = (float)difftime(current_time, s_cur.time_start[n_rtf]);
			if (s_coil.s_coil_data[0].valid && dt > 0.f)
			{
				risk = SPS_DEL_first_roll_prf(s_cur.Line_running, dt, n_rtf ); 
				sps_act_sp[n_rtf].restart_OK = (risk < sps_entry[n_rtf].s_mea.restart_risk_lim);                   
			}
			else
				sps_act_sp[n_rtf].restart_OK = true;

			float rol_tmp=0.f;
			for (int i_mesh=0; i_mesh < MAX_NB_MESH; i_mesh++)        
			{                     
				rol_tmp += rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[0][i_mesh].tmp - TK;
				hot_prf[i_mesh] = rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[0][i_mesh].hot_prf;             
			}
			rol_tmp /= float(MAX_NB_MESH);     

			if (rol_cmp_risk[n_rtf].s_rol[0].s_strip.tmp > TK)
			{
				SQL_ROL_HIS_INSERT( s_cur.record_IDX, s_cur.record_Pt, risk, sps_entry[n_rtf].s_cur_coil.wid, 
										  sps_entry[n_rtf].s_cur_coil.thi, rol_cmp_risk[n_rtf].s_rol[0].s_strip.tmp, rol_tmp, 
										  sps_entry[n_rtf].s_mea.tmp_zon[0], hot_prf, n_rtf);
			}
			 s_cur.Line_running = false;
		} // if (n_rtf == RTH)	  

      sps_act_sp[n_rtf].ctl_pow = false;
		sps_act_sp[n_rtf].Spd_Valid = false;
		sps_act_sp[n_rtf].Valid = false;

      for ( int i_zon=0; i_zon < fur_cfg[n_rtf].nb_zon; i_zon++ ) sps_act_sp[n_rtf].tmp_zon[i_zon] = TUB_SPT_STOP;

      sps_act_sp[n_rtf].Trans_Type = 0;
      time(&s_cur.time_start[n_rtf]);
		s_cur.record_Pt ++;

      return status;
	} // If the measured speed is < MIN_SPEED

   else
      sps_act_sp[n_rtf].restart_OK = false;

   s_cur.Line_running = true;
   time(&s_cur.time_start[n_rtf]);
	s_cur.record_Pt = 0;

	// anticipation time treatment for similar coils 
	float ratio_thi = s_coil.s_coil_data[1].thi / sps_entry[n_rtf].s_cur_coil.thi;
	float ratio_wid = s_coil.s_coil_data[1].wid / sps_entry[n_rtf].s_cur_coil.wid;

	if(sps_entry[n_rtf].s_cur_coil.thi == s_coil.s_coil_data[1].thi && sps_entry[n_rtf].s_cur_coil.wid == s_coil.s_coil_data[1].wid 
			&& s_coil.s_coil_data[0].tmp_tgt[n_rtf] == s_coil.s_coil_data[1].tmp_tgt[n_rtf])
	{
		sps_entry[n_rtf].s_trans_coil.anticipation_time = 0.01f;
	}
	else if((ratio_thi > 0.96f && ratio_thi < 1.04f) && (ratio_wid > 0.9f && ratio_wid < 1.1f) 
				&& s_coil.s_coil_data[0].tmp_tgt[n_rtf] == s_coil.s_coil_data[1].tmp_tgt[n_rtf])
	{
		sps_entry[n_rtf].s_trans_coil.anticipation_time = 30.f;
	}

	if (s_coil.s_coil_data[0].dummy == 1 && s_coil.s_coil_data[1].dummy == 1)
	{
		sps_entry[n_rtf].s_trans_coil.anticipation_time = 0.1f;
	}
	else if (s_coil.s_coil_data[1].dummy == 1)
	{
		sps_entry[n_rtf].s_trans_coil.anticipation_time = 0.1f;
	}

	sps_entry[n_rtf].s_trans_coil.anticipation_time = min(sps_entry[n_rtf].s_trans_coil.anticipation_time, (s_coil.s_coil_data[0].lgt/3)/ sps_act_sp[n_rtf].spd);
	
	if(sps_entry[n_rtf].s_cur_coil.dummy == 1 &&  s_coil.s_coil_data[1].dummy == 0)
		last_dum = true;

	if (last_dum)
	{
		sps_entry[n_rtf].s_cur_coil.dummy = 0;
		s_coil.s_coil_data[0].dummy == 0;
	}
	if (n_rtf == RTS)
	{
		if (sps_entry[RTH].s_mea.tim < sps_entry[RTS].s_mea.tim)
		{
			sps_entry[n_rtf].s_trans_coil.anticipation_time = sps_entry[RTH].s_trans_coil.anticipation_time;
			sps_entry[n_rtf].s_trans_coil.last_time = sps_entry[n_rtf].s_trans_coil.anticipation_time;
		}
		else
		{
			sps_entry[n_rtf].s_trans_coil.anticipation_time = sps_entry[n_rtf].s_trans_coil.last_time;
		}
	}

	// ================================================================
	// If we are inside anticipation time (transition) -  RTF TRANSIENT
	// ================================================================
   if ( sps_entry[n_rtf].s_trans_coil.anticipation_time >= sps_entry[n_rtf].s_mea.tim )
   {
      sps_act_sp[n_rtf].Trans_Type = 1;
      sps_fin_sp[n_rtf] = sps_entry[n_rtf].s_cur_coil;
      for ( int i_zon=0; i_zon < fur_cfg[n_rtf].nb_zon; i_zon++ ) s_cur.delta_po_rtf[n_rtf][i_zon] = 0.f;

      sps_fin_sp[n_rtf].tmp_tgt = sps_entry[n_rtf].s_trans_coil.tmp_tgt;
		if(n_rtf == RTS)
			sps_fin_sp[n_rtf].tmp_tgt = sps_entry[n_rtf].s_cur_coil.nxt_tgt;

      sps_act_sp[n_rtf].tmp_stp = sps_fin_sp[n_rtf].tmp_tgt;

	  // only for rth decrease the speed if the measurement is too far from the target when in transition
	  if (n_rtf == RTH )
	  { 
		  count_spd_decr--;
		  if (count_spd_decr < 0) count_spd_decr = 0;

		  // Decrease the line speed if the temperature is too low
		  if (count_spd_decr == 0 &&
			  (sps_entry[n_rtf].s_mea.tmp_stp < sps_act_sp[n_rtf].tmp_stp - 8.f && sps_entry[n_rtf].s_mea.tim < 4.5f * 60.f ||		// 5.f * 60.f and 3.f * 60.f
				  sps_entry[n_rtf].s_mea.tmp_stp < sps_act_sp[n_rtf].tmp_stp - 5.f && sps_entry[n_rtf].s_mea.tim < 2.5f * 60.f)
				  && sps_entry[n_rtf].s_trans_coil.anticipation_time > 90.f)	// before 6.f *60.f and 3.5f * 60.f
		  {
			  sps_act_sp[n_rtf].spd -= 0.85f / 60.f * coef;																			// 1.f / 60.f
			  sps_act_sp[n_rtf].spd = max(sps_act_sp[n_rtf].spd, sps_entry[n_rtf].s_trans_coil.spd * 0.88f);
			  count_spd_decr = 6;
		  }
	  }

		// Check if weld has exited a given zone
      // -------------------------------------
      // For all zones
      for ( int i_zon=0; i_zon < fur_cfg[n_rtf].nb_zon; i_zon++ )
      {
			if (sps_entry[n_rtf].s_mea.lgt > s_cur.Lgt_zon_rtf_exit[n_rtf][i_zon])
			{
				sps_fin_sp[n_rtf].tmp_zon[i_zon] = sps_entry[n_rtf].s_trans_coil.tmp_zon[i_zon];// + fba_stp[n_rtf].pid_exit;
				sps_fin_sp[n_rtf].pow_pct[i_zon] = sps_entry[n_rtf].s_trans_coil.pow_pct[i_zon];
			}

			else
			{
				sps_fin_sp[n_rtf].tmp_zon[i_zon] = s_coil.s_coil_data[1].tmp_zon[n_rtf][i_zon];// + fba_stp[n_rtf].pid_exit;
				sps_fin_sp[n_rtf].pow_pct[i_zon] = s_coil.s_coil_data[1].pow_pct[n_rtf][i_zon];
			}
      } // For all zones

      // If the speed must decrease
      // --------------------------
      if ( sps_entry[n_rtf].s_trans_coil.spd < sps_act_sp[n_rtf].spd - DELTA_SPD * coef && 
           sps_entry[n_rtf].s_mea.spd_auto && n_rtf == RTH)
      {
         sps_act_sp[n_rtf].Trans_Type += 10;
         sps_fin_sp[n_rtf].spd = sps_entry[n_rtf].s_trans_coil.spd;

         status = SPS_RTF_decr_spd (&sps_fin_sp[n_rtf], &sps_mea_rtf, &sps_act_sp[n_rtf], s_cur.delta_po_rtf[n_rtf], n_rtf);
         if ( !status ) ns_STD::cl_TRC::ERR_Write("SPS", sub_name, "error in SPS_RTF_decr_spd");
      } // If the speed must decrease

      // If the speed must increase
      // --------------------------
      else if (sps_entry[n_rtf].s_trans_coil.spd > sps_act_sp[n_rtf].spd + DELTA_SPD * coef && 
               sps_entry[n_rtf].s_mea.spd_auto && n_rtf == RTH)
      {
         sps_act_sp[n_rtf].Trans_Type += 20;
         sps_fin_sp[n_rtf].spd = max(sps_entry[n_rtf].s_trans_coil.spd, sps_act_sp[n_rtf].spd, sps_entry[n_rtf].s_cur_coil.spd * 0.82f);

			// we go either lighter or colder so bool is set to true
			trans_spd_up = true;

         status = SPS_RTF_incr_spd (&sps_fin_sp[n_rtf], &sps_mea_rtf, &sps_act_sp[n_rtf], s_cur.delta_po_rtf[n_rtf], n_rtf);
         if ( !status ) ns_STD::cl_TRC::ERR_Write("SPS", sub_name, "error in SPS_RTF_incr_spd");
      } // If the speed must increase

      // The speed is constant 
      // ---------------------
      else 
      {
         sps_act_sp[n_rtf].Trans_Type += 30;

         if ( sps_entry[n_rtf].s_mea.tmp_stp >= sps_entry[n_rtf].s_cur_coil.tmp_low + 5.f || 
              sps_entry[n_rtf].s_mea.tmp_stp >= sps_act_sp[n_rtf].tmp_stp )
         {
            sps_act_sp[n_rtf].spd = min(sps_act_sp[n_rtf].spd,sps_entry[n_rtf].s_trans_coil.spd);
         }

         // For all RTF zones
         for ( int i_zon=0; i_zon < fur_cfg[n_rtf].nb_zon; i_zon++ )
         {
            // if the strip temperature is far away from the target,  Set power offset
            if (fabs(sps_entry[n_rtf].s_mea.tmp_stp - sps_entry[n_rtf].s_trans_coil.tmp_tgt) > 4.f)
            {
               // if tube temperature measure is lower than the set point
               if ( sps_entry[n_rtf].s_mea.tmp_zon[i_zon] <= sps_fin_sp[n_rtf].tmp_zon[i_zon] )
               {
                  if ( sps_mea_rtf.tmp_zon[i_zon] >= sps_fin_sp[n_rtf].tmp_zon[i_zon] - 8.f * coef)  
                  {
                     s_cur.delta_po_rtf[n_rtf][i_zon] = 0.f;
                  }

	               else if ( sps_mea_rtf.tmp_zon[i_zon] >= sps_fin_sp[n_rtf].tmp_zon[i_zon] - 15.f * coef )
                  {
                     //s_cur.delta_po_rtf[n_rtf][i_zon] = 9.6f;
                  }

                  else if ( sps_mea_rtf.tmp_zon[i_zon] >= sps_fin_sp[n_rtf].tmp_zon[i_zon] - 25.f * coef && 
                            s_cur.delta_po_rtf[n_rtf][i_zon] <= 15.f)                      
                  {
                     //s_cur.delta_po_rtf[n_rtf][i_zon] = 15.f;
                  }

                  else if ( sps_mea_rtf.tmp_zon[i_zon] < sps_fin_sp[n_rtf].tmp_zon[i_zon] - 25.f * coef ) 
                  {
                     //s_cur.delta_po_rtf[n_rtf][i_zon] = 30.f;
                  }
               } // if the strip temperature is far away from the target

               // if tube temperature measure is higher than the set point
               else if ( sps_entry[n_rtf].s_mea.tmp_zon[i_zon] >= sps_fin_sp[n_rtf].tmp_zon[i_zon] )
               {
                  if ( sps_mea_rtf.tmp_zon[i_zon] <= sps_fin_sp[n_rtf].tmp_zon[i_zon] + 8.f * coef)   
                  {
                     s_cur.delta_po_rtf[n_rtf][i_zon] = 0.f;
                  }

	               else if ( sps_mea_rtf.tmp_zon[i_zon] <= sps_fin_sp[n_rtf].tmp_zon[i_zon] + 15.f * coef )     
                  {
                     //s_cur.delta_po_rtf[n_rtf][i_zon] = -7.5f;
                  }

                  else if ( sps_mea_rtf.tmp_zon[i_zon] <= sps_fin_sp[n_rtf].tmp_zon[i_zon] + 25.f * coef && 
                            s_cur.delta_po_rtf[n_rtf][i_zon] >= -15.f) 
                  {
                     //s_cur.delta_po_rtf[n_rtf][i_zon] = -15.f;
                  }

                  else if ( sps_mea_rtf.tmp_zon[i_zon] > sps_fin_sp[n_rtf].tmp_zon[i_zon] + 25.f * coef )
                  {
                     //s_cur.delta_po_rtf[n_rtf][i_zon] = -30.f;
                  }
               }
            } // Set power offset if the strip temperature is far away from the target

            else // No power offset if the strip temperature is near the target
               s_cur.delta_po_rtf[n_rtf][i_zon] = 0.f;   

            // If strip temperature is lower than the target
            if ( sps_entry[n_rtf].s_trans_coil.tmp_tgt > sps_entry[n_rtf].s_cur_coil.tmp_tgt + 5.f &&
                 sps_entry[n_rtf].s_mea.tmp_stp <= sps_entry[n_rtf].s_trans_coil.tmp_tgt - 7.f) 
            {
               if (sps_entry[n_rtf].s_mea.tmp_stp <= sps_entry[n_rtf].s_trans_coil.tmp_tgt - 15.f)                                                                          
                  s_cur.delta_po_rtf[n_rtf][i_zon] = 0.f; //max(s_cur.delta_po_rtf[n_rtf][i_zon], 40.f);                   
               else
                  s_cur.delta_po_rtf[n_rtf][i_zon] = 0.f;//15.f;                           
            } // If strip temperature is lower than the target

            // If strip temperature is higher than the target
            if ( sps_entry[n_rtf].s_trans_coil.tmp_tgt < sps_entry[n_rtf].s_cur_coil.tmp_tgt - 5.f &&
                 sps_entry[n_rtf].s_mea.tmp_stp >= sps_entry[n_rtf].s_trans_coil.tmp_tgt + 7.f) 
            {
               // Si on est pas sur un format à risque de déport
               // if ((sps_entry[n_rtf].s_cur_coil.thi <= 1.3f/1000.f && sps_entry[n_rtf].s_cur_coil.wid >= 1.2f)
               //         || (i_zon != 0 && i_zon != 1))
               s_cur.delta_po_rtf[n_rtf][i_zon] = 0.f; // min(s_cur.delta_po_rtf[n_rtf][i_zon], -15.f);                                  
            } // If strip temperature is higher than the target
         } // For all RTF zones
      } // The speed is constant 

      // For all RTF zones
      for ( int i_zon=0; i_zon < fur_cfg[n_rtf].nb_zon; i_zon++ )
      {
			if (sps_entry[n_rtf].s_mea.lgt > s_cur.Lgt_zon_rtf_exit[n_rtf][i_zon])
				sps_fin_sp[n_rtf].tmp_zon[i_zon] = sps_entry[n_rtf].s_trans_coil.tmp_zon[i_zon];
			else
				sps_fin_sp[n_rtf].tmp_zon[i_zon] = s_coil.s_coil_data[1].tmp_zon[n_rtf][i_zon];
      }
   } // If we are inside anticipation time

	// ============================================================
	// If we are not inside anticipation time (stable) - RTF STABLE
	// ============================================================
   else
   {
      sps_act_sp[n_rtf].Trans_Type = 2;
      sps_act_sp[n_rtf].tmp_stp = sps_entry[n_rtf].s_cur_coil.tmp_tgt;
      sps_fin_sp[n_rtf] = sps_entry[n_rtf].s_cur_coil;

      // For all RTF zones
      for ( int i_zon=0; i_zon < fur_cfg[n_rtf].nb_zon; i_zon++ )
      {
         s_cur.delta_po_rtf[n_rtf][i_zon] = 0.f;
			sps_fin_sp[n_rtf].tmp_zon[i_zon] = sps_entry[n_rtf].s_cur_coil.tmp_zon[i_zon];// + fba_stp[n_rtf].pid_exit;
      } // For all RTF zones

      // If the speed must decrease
      // --------------------------
      if ( sps_entry[n_rtf].s_cur_coil.spd < sps_act_sp[n_rtf].spd - DELTA_SPD * coef && 
           sps_entry[n_rtf].s_mea.spd_auto && n_rtf == RTH)
      {
			sps_act_sp[n_rtf].Trans_Type += 100;
			
         status = SPS_RTF_decr_spd( &sps_fin_sp[n_rtf], &sps_mea_rtf, &sps_act_sp[n_rtf], s_cur.delta_po_rtf[n_rtf], n_rtf );
         if (!status) ns_STD::cl_TRC::ERR_Write("SPS", sub_name, "error in SPS_RTF_decr_spd");
      } // If the speed must decrease

      // If the speed must increase
      // --------------------------
      else if ( sps_entry[n_rtf].s_cur_coil.spd > sps_act_sp[n_rtf].spd + DELTA_SPD * coef && 
                sps_entry[n_rtf].s_mea.spd_auto && n_rtf == RTH)
      {
			sps_act_sp[n_rtf].Trans_Type += 200;

         status = SPS_RTF_incr_spd( &sps_fin_sp[n_rtf], &sps_mea_rtf, &sps_act_sp[n_rtf], s_cur.delta_po_rtf[n_rtf], n_rtf );
         if ( !status ) ns_STD::cl_TRC::ERR_Write("SPS", sub_name, "error in SPS_RTF_incr_spd");
      } // If the speed must increase

      // If the speed is constant
      // ------------------------
      else
      { 
			sps_act_sp[n_rtf].Trans_Type += 300;
   //      // For all RTF zones
   //      for ( int i_zon=0; i_zon < fur_cfg[n_rtf].nb_zon; i_zon++ ) 
   //      {
   //         s_cur.delta_po_rtf[n_rtf][i_zon] = 0.f;
	//		   sps_fin_sp[n_rtf].tmp_zon[i_zon] = sps_entry[n_rtf].s_cur_coil.tmp_zon[i_zon];
   //      } // For all RTF zones

         if ( sps_entry[n_rtf].s_cur_coil.spd < sps_act_sp[n_rtf].spd ||
              sps_entry[n_rtf].s_mea.tmp_stp >= sps_entry[n_rtf].s_cur_coil.tmp_low+ 5.f )
         {
            sps_act_sp[n_rtf].spd = sps_entry[n_rtf].s_cur_coil.spd;
         }
		} // If the speed is constant
	  for (int i_zon = 0; i_zon < fur_cfg[n_rtf].nb_zon; i_zon++)
	  {
		  s_cur.delta_po_rtf[n_rtf][i_zon] = 0.f;
	  } // For all RTF zones
	} // If we are not inside anticipation time (stable)

	// =================================
	// Auto adaptation - AUTO ADAPTATION
	// =================================
   diff_time = (float)difftime(current_time, s_cur.time_lst_adapt[n_rtf]);

   sps_entry[n_rtf].s_mea.tonnage = sps_entry[n_rtf].s_mea.spd * sps_entry[n_rtf].s_cur_coil.thi *
                                    sps_entry[n_rtf].s_cur_coil.wid * 3600.f * RO_STEEL / 1000.f;

      // Emissivity adaptation
   if ( sps_entry[n_rtf].s_mea.tonnage > 35.f && diff_time > 120.f && 
        sps_entry[n_rtf].s_mea.spd > MIN_SPEED + 20.f/60.f
		  && n_rtf == RTH
        //&& !SIMULATION
      )
   {
      SPS_ADA_Emis(0, false, n_rtf);
		time(&s_cur.time_lst_adapt[n_rtf]);
	}

	// Zone efficiency self adaptation
	if (SIMULATION == 0) SPS_ADA_effi(0, n_rtf);

	// =========================================================================
	// If the strip temperature is close to the upper limits - OUT TOLERANCE UPP
	// =========================================================================

	// =========================================================================
	// If the strip temperature is close to the lower limits - OUT TOLERANCE LOW
	// =========================================================================
   if ( (sps_entry[n_rtf].s_mea.tmp_stp <= sps_entry[n_rtf].s_cur_coil.tmp_low + 3.f) && 
		  (sps_entry[n_rtf].s_mea.tmp_stp <= sps_act_sp[n_rtf].tmp_stp - 0.f) 
	   || (sps_entry[n_rtf].s_mea.tmp_stp <= (sps_act_sp[n_rtf].tmp_stp -2.f) 
		   &&  sps_entry[n_rtf].s_mea.tmp_stp < (s_cur.prev_strip_temp_rtf[n_rtf] - 0.35f))
	   && n_rtf == RTH)
	{
      // if strip temperature is near from lower tolerance
  //    if ( sps_entry[n_rtf].s_mea.tmp_stp <= sps_entry[n_rtf].s_cur_coil.tmp_low+ 3.f )    
		//{
		//   sps_act_sp[n_rtf].Trans_Type += 2000;

         //fba_stp[n_rtf].p = fba_stp[n_rtf].p * 1.25f;
		//	fba_stp[n_rtf].i = fba_stp[n_rtf].i / 1.25f;  
  //    }  // if strip temperature is near from lower tolerance

      // If the strip temperature is too close to the lower limits, the speed is decreased.
      if (sps_entry[n_rtf].s_cur_coil.dummy == 0 && !last_dum)
      {
         if (
                sps_entry[n_rtf].s_mea.tmp_stp <=     sps_entry[n_rtf].s_cur_coil.tmp_low+1.0f || //without 2.0
				  ( sps_entry[n_rtf].s_mea.tmp_stp <= max(sps_entry[n_rtf].s_cur_coil.tmp_low + 3.f, sps_act_sp[n_rtf].tmp_stp - 6.5f ) &&  //min before
                sps_entry[n_rtf].s_mea.tmp_stp < s_cur.prev_strip_temp_rtf[n_rtf] - 0.1f ) || //-0.25f before 0.08
			 (sps_entry[n_rtf].s_mea.tmp_stp <= sps_act_sp[n_rtf].tmp_stp - 1.f && sps_entry[n_rtf].s_mea.tmp_stp < s_cur.prev_strip_temp_rtf[n_rtf] - 0.2f) //0.3 before
			 && n_rtf == RTH )//0.3            
         {
			 if (sps_entry[n_rtf].s_mea.tmp_stp > s_cur.prev_strip_temp_rtf[n_rtf] + 0.05f 
				 && sps_entry[n_rtf].s_mea.tmp_stp >= sps_entry[n_rtf].s_cur_coil.tmp_low - 2.5f)
			 {
				 sps_act_sp[n_rtf].spd = max(min(sps_act_sp[n_rtf].spd, MAX_SPEED), MIN_SPEED + 2.f / 60.f);
			 }
			 else
			 {
				 if (sps_entry[n_rtf].s_mea.tim > (150.f/sps_entry[n_rtf].s_mea.spd ))
				 {
					 if (sps_entry[n_rtf].s_trans_coil.anticipation_time < sps_entry[n_rtf].s_mea.tim
						 || (sps_entry[n_rtf].s_trans_coil.anticipation_time >= sps_entry[n_rtf].s_mea.tim && sps_entry[n_rtf].s_mea.tmp_stp < sps_entry[n_rtf].s_cur_coil.tmp_low))
					 {
						 sps_act_sp[n_rtf].spd -= min(max(0.25f * (sps_entry[n_rtf].s_cur_coil.tmp_low + 3.f -
							 sps_entry[n_rtf].s_mea.tmp_stp)*coef, 0.05f), 0.4f) / 60.f; // 0.05f and 0.84

					 }
				 }
				 else
				 {
					 sps_act_sp[n_rtf].spd -= min(max(0.15f * (sps_entry[n_rtf].s_cur_coil.tmp_low + 2.f - sps_entry[n_rtf].s_mea.tmp_stp)*coef, 0.05f), 0.2f) / 60.f;
				 }
				 sps_act_sp[n_rtf].spd = max(sps_act_sp[n_rtf].spd, sps_entry[n_rtf].s_cur_coil.spd * 0.88f);
				 sps_act_sp[n_rtf].spd = max(min(sps_act_sp[n_rtf].spd, MAX_SPEED), MIN_SPEED + 2.f / 60.f);
			 }
         }
      }
   } // If the strip temperature is close to the lower limits

   // ==========
   // Dummy coil
   // ==========
   if ( sps_entry[n_rtf].s_cur_coil.dummy == 1 ) 
   {
		sps_act_sp[n_rtf].Trans_Type += 10000;
	   if (sps_entry[n_rtf].s_cur_coil.spd <= sps_act_sp[n_rtf].spd) 
         sps_act_sp[n_rtf].spd = max( sps_act_sp[n_rtf].spd - 2.f/60.f, sps_entry[n_rtf].s_cur_coil.spd);
	   else 
         sps_act_sp[n_rtf].spd = min( sps_act_sp[n_rtf].spd + 2.f/60.f, sps_entry[n_rtf].s_cur_coil.spd);
   }

	// ==============================================
	// Correction of the exit temperature - RTF STRIP
	// ==============================================
  // if ( s_coil.s_coil_data[0].thi >= 0.0015 && sps_mea_rtf.spd < 1.f || 
  //      sps_entry[n_rtf].s_mea.tonnage < 65.f && sps_act_sp[n_rtf].spd < 1.16f || 
  //      sps_entry[n_rtf].s_mea.tonnage < 40.f)
  // {
  //    fba_stp[n_rtf].p = fba_stp[n_rtf].p / 1.5f;
		//fba_stp[n_rtf].i = fba_stp[n_rtf].i * 1.25f;  
  // }

  // else if ( s_coil.s_coil_data[0].thi >= 0.0015f && sps_mea_rtf.spd < 1.16f || 
  //           sps_entry[n_rtf].s_mea.tonnage < 75.f && sps_act_sp[n_rtf].spd < 1.33f || 
  //           sps_entry[n_rtf].s_mea.tonnage < 60.f)
  // {
  //    fba_stp[n_rtf].p = fba_stp[n_rtf].p / 1.2f;
		//fba_stp[n_rtf].i = fba_stp[n_rtf].i * 1.10f;  
  // }
	 
	// Store PID value
	// ---------------	   
	float P_stp_lst = fba_stp[n_rtf].p;
   float I_stp_lst = fba_stp[n_rtf].i;

	if ( s_coil.s_coil_data[0].thi >= 0.002 && sps_entry[n_rtf].s_mea.tonnage > 69.f )
   {
      fba_stp[n_rtf].p = fba_stp[n_rtf].p * 1.2f;
		fba_stp[n_rtf].i = fba_stp[n_rtf].i / 1.1f;  
   }

	fba_stp[n_rtf].pv = min (max (sps_mea_rtf.tmp_stp, sps_act_sp[n_rtf].tmp_stp-100.f), 
                           sps_act_sp[n_rtf].tmp_stp + 100.f); 
   fba_stp[n_rtf].sp = sps_act_sp[n_rtf].tmp_stp;
   if ( fba_stp[n_rtf].sp <= 0.f ) fba_stp[n_rtf].sp += 10.f;

	// PID reset or not
	if ( !sps_entry[n_rtf].s_mea.auto_on ) fba_stp[n_rtf].reset = true;
   else                                   fba_stp[n_rtf].reset = false;

	// Sécurité sur la divergence de la sortie intégrale du PID bande
	if ( sps_entry[n_rtf].s_trans_coil.anticipation_time < sps_entry[n_rtf].s_mea.tim )
   {
		//if (sps_entry[n_rtf].s_mea.tmp_stp > sps_act_sp[n_rtf].tmp_stp + 10.f)
		//	 fba_stp[n_rtf].i_exit = min(fba_stp[n_rtf].i_exit, +10.f);           
		//else if (sps_entry[n_rtf].s_mea.tmp_stp > sps_act_sp[n_rtf].tmp_stp + 5.f)
		//	 fba_stp[n_rtf].i_exit = min(fba_stp[n_rtf].i_exit, +20.f);                                                 
		//else if(sps_entry[n_rtf].s_mea.tmp_stp < sps_act_sp[n_rtf].tmp_stp - 10.f)
		//	 fba_stp[n_rtf].i_exit = max(fba_stp[n_rtf].i_exit, -10.f);
		//else if(sps_entry[n_rtf].s_mea.tmp_stp < sps_act_sp[n_rtf].tmp_stp - 5.f)
		//	 fba_stp[n_rtf].i_exit = max(fba_stp[n_rtf].i_exit, -20.f);
		if (sps_entry[n_rtf].s_mea.tmp_stp < sps_act_sp[n_rtf].tmp_stp - 15.f && fba_stp[n_rtf].i_exit < -15.f)
			 fba_stp[n_rtf].i_exit = -15.f;
		if (sps_entry[n_rtf].s_mea.tmp_stp > sps_act_sp[n_rtf].tmp_stp + 20.f && fba_stp[n_rtf].i_exit > 15.f)
			fba_stp[n_rtf].i_exit = 15.f;
	}
              
   bool iflag_max = true;
   bool iflag_min = true;
	float rts_pow_ave = 0;
	// Increase the tube temperatue only if the power are not at the maximum or decrease
	//    the tube temperature only if the power is greater than the minimum
   // --------------------------------------------------------------------------------
   // For all zones
   for ( int i_zon=0; i_zon < fur_cfg[n_rtf].nb_zon; i_zon++ )
   {
      if (sps_fin_sp[n_rtf].pow_pct[i_zon]  + fba_zon[n_rtf][i_zon].pid_exit < 99.3f )//|| 
			      //sps_entry[n_rtf].s_mea.tmp_stp <= sps_act_sp[n_rtf].tmp_stp - 25.f) 
      { 
         iflag_max = false;
      }

      //if (sps_fin_sp[n_rtf].pow_pct[i_zon] + s_cur.delta_po_rtf[n_rtf][i_zon] + fba_zon[n_rtf][i_zon].pid_exit >  0.f || 
		    //  sps_entry[n_rtf].s_mea.tmp_stp >= sps_act_sp[n_rtf].tmp_stp + 25.f) 
      //{
         iflag_min = false;
      //}
   } // For all zones

	if (n_rtf == RTS)
	{
		for (int i_zon = 0; i_zon < NB_ZON_RTS; i_zon++)
		{
			sps_fin_sp[RTS].pow_pct[i_zon] = min(sps_fin_sp[RTS].pow_pct[i_zon],50.f);
		}
	}
   // If the power is everywhere at max, we don't try to increase it
   int lst_zon = fur_cfg[n_rtf].nb_zon - 1;
   if ( iflag_max || sps_fin_sp[n_rtf].tmp_zon[lst_zon] + fba_stp[n_rtf].pid_exit >= TUB_MAX_TMP + 10.f ) 
      fba_stp[n_rtf].max_exit = fba_stp[n_rtf].pid_exit;
   else 
	{
		if (n_rtf == RTH)
		{
			if(sps_entry[n_rtf].s_cur_coil.thi < 0.0018f)
				fba_stp[n_rtf].max_exit = 25.f; // 15.f;
			else
				fba_stp[n_rtf].max_exit = 40.f;
		}
		else if (n_rtf == RTS)
		{
			for (int i_zon = 0; i_zon < NB_ZON_RTS; i_zon++)
			{
				rts_pow_ave += sps_fin_sp[RTS].pow_pct[i_zon];
			}
			rts_pow_ave /= float(NB_ZON_RTS);

			fba_stp[n_rtf].max_exit = 100.f - rts_pow_ave;
		}
	}

   // If the power is everywhere at min, we don't try to decrease it
   if ( iflag_min ) 
      fba_stp[n_rtf].min_exit = fba_stp[n_rtf].pid_exit;
   else           
	{
      if (n_rtf == RTH)
			fba_stp[n_rtf].min_exit = -30.f; //-15.f;  
		else if (n_rtf == RTS)
			fba_stp[n_rtf].min_exit = -rts_pow_ave + 5.f;
	}

   if ( fba_stp[n_rtf].max_exit == 0.f && fba_stp[n_rtf].min_exit == 0.f )
   {
      fba_stp[n_rtf].max_exit = +10.f;
      fba_stp[n_rtf].min_exit = -10.f;
   }

   if (sps_entry[n_rtf].s_cur_coil.spd > sps_act_sp[n_rtf].spd + DELTA_SPD * coef &&
	   sps_entry[n_rtf].s_mea.spd_auto && n_rtf == RTH && !trans_spd_up)
   {
	   if (sps_entry[n_rtf].s_trans_coil.anticipation_time >= sps_entry[n_rtf].s_mea.tim)
	   {
		   if(sps_entry[n_rtf].s_trans_coil.spd > sps_act_sp[n_rtf].spd + DELTA_SPD * coef)
			   fba_stp[n_rtf].i_exit += 0.1f;		// += 0.15f += 0.2
	   }
	   else
		fba_stp[n_rtf].i_exit += 0.1f;		//+= 0.15f // += 0.2
   }

	// Strip PID calculation
	if ( sps_entry[n_rtf].s_cur_coil.dummy == 0 || n_rtf == RTS)  //sps_entry[n_rtf].s_mea.tmp_stp < sps_entry[n_rtf].s_cur_coil.tmp_low + 3.f 
   {
      status = ns_STD::cl_PID::SPS_PID(&fba_stp[n_rtf]);
	   if ( !status ) ns_STD::cl_TRC::ERR_Write("SPS", sub_name, "error in SPS_PID");
   }

	if (fba_stp[n_rtf].i_exit > fba_stp[n_rtf].max_exit)
		fba_stp[n_rtf].i_exit = fba_stp[n_rtf].max_exit - 1.f;
	else if (fba_stp[n_rtf].i_exit < fba_stp[n_rtf].min_exit)
		fba_stp[n_rtf].i_exit = fba_stp[n_rtf].min_exit + 1.f;

   // =========================================
   // Correction of power set points - RTF TUBE
   // =========================================
   flag_pid_rtf = false;
   // For all RTF zones
   for ( int i_zon=0; i_zon < fur_cfg[n_rtf].nb_zon; i_zon++ )
   {
		fba_zon[n_rtf][i_zon].reset = !sps_entry[n_rtf].s_mea.auto_on;
		if (sps_entry[n_rtf].s_cur_coil.dummy == 1 && n_rtf == RTH)
			sps_act_sp[n_rtf].tmp_zon[i_zon] = min(sps_fin_sp[n_rtf].tmp_zon[i_zon] + 8.f,TUB_MAX_TMP);
		else
		{
			//if(sps_entry[n_rtf].s_cur_coil.spd > sps_act_sp[n_rtf].spd + DELTA_SPD * coef && i_zon > 5)
			if(i_zon > 5)
				sps_act_sp[n_rtf].tmp_zon[i_zon] = min(sps_fin_sp[n_rtf].tmp_zon[i_zon] + fba_stp[n_rtf].pid_exit, 
                                        TUB_MAX_TMP + 10.f);

			else
				sps_act_sp[n_rtf].tmp_zon[i_zon] = min(sps_fin_sp[n_rtf].tmp_zon[i_zon] + fba_stp[n_rtf].pid_exit, 
                                        TUB_MAX_TMP);
		}

		// Only for RTH
		if(n_rtf == RTH)
		{
		   // Initializations
		   fba_zon[n_rtf][i_zon].pv = sps_entry[n_rtf].s_mea.tmp_zon[i_zon];
		   if ( sps_act_sp[n_rtf].heat_pow_pct[i_zon] <= 0.f ) 
			   sps_act_sp[n_rtf].heat_pow_pct[i_zon] = sps_entry[n_rtf].s_mea.heat_pow_pct[i_zon];

		   // Tube temperature set point
		   fba_zon[n_rtf][i_zon].sp = sps_act_sp[n_rtf].tmp_zon[i_zon];

		   // Tube temperature measurement
		   if ( fba_zon[n_rtf][i_zon].sp <= 0.f ) fba_zon[n_rtf][i_zon].sp = fba_zon[n_rtf][i_zon].pv + 10.f;

		   fba_zon[n_rtf][i_zon].max_exit = 100.f - (sps_fin_sp[n_rtf].pow_pct[i_zon] + s_cur.delta_po_rtf[n_rtf][i_zon]);

		   //if (sps_entry[n_rtf].s_mea.tmp_stp >= sps_act_sp[n_rtf].tmp_stp - 2.f)
		   //	fba_zon[n_rtf][i_zon].max_exit = min( fba_zon[n_rtf][i_zon].max_exit , 30.f);

		   fba_zon[n_rtf][i_zon].min_exit = 5.f - (sps_fin_sp[n_rtf].pow_pct[i_zon] + s_cur.delta_po_rtf[n_rtf][i_zon]);

		   //if (sps_entry[n_rtf].s_mea.tmp_stp <= sps_act_sp[n_rtf].tmp_stp + 2.f)
		   //	fba_zon[n_rtf][i_zon].min_exit = max( fba_zon[n_rtf][i_zon].min_exit , -30.f);

		   // PID reset or not
		   if (!sps_entry[n_rtf].s_mea.auto_on) 
			   fba_zon[n_rtf][i_zon].reset = true;
		   else                                 
			   fba_zon[n_rtf][i_zon].reset = false;

		   // PID calculation
		   if ( fabs(s_cur.delta_po_rtf[n_rtf][i_zon]) < 4.f && sps_entry[n_rtf].s_mea.on_zone[i_zon] )
		   {
			   status = ns_STD::cl_PID::SPS_PID (&fba_zon[n_rtf][i_zon]);
			   if ( !status ) ns_STD::cl_TRC::ERR_Write("SPS", sub_name, "error in SPS_PID");
		   } // PID calculation

		   /* Tube PID integral exit limitation */
		   if (fba_zon[n_rtf][i_zon].i_exit > fba_zon[n_rtf][i_zon].max_exit)
			   fba_zon[n_rtf][i_zon].i_exit = fba_zon[n_rtf][i_zon].max_exit - 1.f;
		   else if(fba_zon[n_rtf][i_zon].i_exit < fba_zon[n_rtf][i_zon].min_exit)
			   fba_zon[n_rtf][i_zon].i_exit = fba_zon[n_rtf][i_zon].min_exit + 1.f;

		   //fba_zon[n_rtf][i_zon].i_exit = min ( max ( fba_zon[n_rtf][i_zon].i_exit , -80.f), 80.f);

		   if ( fabs(s_cur.delta_po_rtf[n_rtf][i_zon]) >= 4.f )
		   {
			   pow_pct = sps_fin_sp[n_rtf].pow_pct[i_zon] + s_cur.delta_po_rtf[n_rtf][i_zon];// +	fba_zon[n_rtf][i_zon].i_exit;
		   }

		   else
		   {
			   pow_pct = sps_fin_sp[n_rtf].pow_pct[i_zon] + s_cur.delta_po_rtf[n_rtf][i_zon] + 
							   fba_zon[n_rtf][i_zon].pid_exit;
		   } 
		} // Only for RTH

		// For RTS
		else
		{
			sps_act_sp[n_rtf].tmp_zon[i_zon] = min(sps_fin_sp[n_rtf].tmp_zon[i_zon] + 45.f, 
                                             TUB_MAX_TMP);

      
			pow_pct = sps_fin_sp[n_rtf].pow_pct[i_zon] + fba_stp[n_rtf].pid_exit;
		} // For RTS

		pow_pct = min ( pow_pct, 100.f );
		pow_pct = max ( pow_pct,   5.f );

		if ( !sps_entry[n_rtf].s_mea.on_zone[i_zon] || 
				(s_cur.jcount_rtf <= jcount_max && 
				sps_entry[n_rtf].s_mea.tmp_stp >= sps_entry[n_rtf].s_cur_coil.tmp_low) )
		{
			fba_zon[n_rtf][i_zon].i_exit = 0.f;
			sps_act_sp[n_rtf].heat_pow_pct[i_zon] = 5.f;
	    	pow_pct = 5.f;
			flag_pid_rtf = true;
		}

		sps_act_sp[n_rtf].heat_pow_pct[i_zon] = pow_pct;

	   // If not in automatic
		if (!sps_entry[n_rtf].s_mea.auto_on)
		{
			sps_act_sp[n_rtf].heat_pow_pct[i_zon] = sps_entry[n_rtf].s_mea.heat_pow_pct[i_zon];
			//if(n_rtf == RTH)
			//{
				fba_zon[n_rtf][i_zon].i_exit = sps_entry[n_rtf].s_mea.heat_pow_pct[i_zon] - 
					                            sps_fin_sp[n_rtf].pow_pct[i_zon] - fba_zon[n_rtf][i_zon].p * 
						                         (fba_zon[n_rtf][i_zon].sp - fba_zon[n_rtf][i_zon].pv);
			//}
			//else
			//{
			//	fba_stp[n_rtf].i_exit = sps_entry[n_rtf].s_mea.heat_pow_pct[i_zon] -
			//		                            sps_fin_sp[n_rtf].pow_pct[i_zon] - fba_stp[n_rtf].p * 
			//			                         (fba_stp[n_rtf].sp - fba_stp[n_rtf].pv);
			//}
		} // If not in automatic
	} // For all RTF zones

   s_cur.jcount_rtf ++;
   if (s_cur.jcount_rtf > 1000) s_cur.jcount_rtf = 500;

	if (last_dum)
	{
		sps_entry[n_rtf].s_cur_coil.dummy = 1;
		s_coil.s_coil_data[0].dummy == 1;
	}

	// Store back PID value
	// ====================
	fba_stp[n_rtf].p = P_stp_lst;
	fba_stp[n_rtf].i = I_stp_lst;

   //if (SIMULATION)
   //   sps_act_sp[n_rtf].spd = max(min (sps_act_sp[n_rtf].spd,320.f/60),100.f/60.f);

   // Store current strip temperature mesurement
   if ( fabs( s_cur.prev_strip_temp_rtf[n_rtf] - sps_entry[n_rtf].s_mea.tmp_stp ) > 0.3f ) 
      s_cur.prev_strip_temp_rtf[n_rtf] = (float)sps_entry[n_rtf].s_mea.tmp_stp;

   // If we recover a zone, reset the strip PID
   if ( !flag_pid_rtf && flag_old_pid_rtf ) fba_stp[n_rtf].i_exit = 0.f;
   flag_old_pid_rtf = flag_pid_rtf;

   // If the speed is in manual
   if ( !sps_entry[n_rtf].s_mea.spd_auto ) sps_act_sp[n_rtf].spd = sps_entry[n_rtf].s_mea.spd;

	sps_act_sp[n_rtf].spd = max(sps_act_sp[RTH].spd, sps_entry[RTH].s_cur_coil.spd * 0.71f);
   sps_act_sp[n_rtf].spd = min(max(sps_act_sp[n_rtf].spd, MIN_SPEED + 0.1f),320.f/60.f);

   return status;
} // void

/*============================================================================
/ SPS_RTF_anticipation: Calculation of the anticipation time in a RTF section.
/ 
/ Revisions:
/  Created:       2005-Aug      JC.Mitais
/  Modified:      2009-Feb       J. NAVEIRA
/
/ ro.P = µ.Cp0.DT0 + ( m.Cp1.DT1 ) / t    ==>     1/t = ( ro.P - µ.Cp0.DT0 ) / ( m.Cp1.DT1 )
/          strip     radiant tubes
/ µ strip massic flow, ro furnace efficiency
/===========================================================================*/
void SPS_RTF_anticipation( float *anticipation_time, float T_ant, float T_stbl, float thi, float wid,
                           float dH_tub, float power, float dH_roll, int n_rtf)
{
	float DH_stp;
	float limit_rate_thi_easy = 2, limit_var_temp_easy = 15;
	float limit_rate_thi_norm = 10, limit_var_temp_norm = 50;

   // ===================================
   // |	CALCULATION WHEN AN RTF EXISTS	|
   // ===================================
   //  anticipation_time: [s]
   //  thermal_inertia: [J/kg/K]
   // Calculation of the strip enthlpy to give or take from the strip 
	if (sps_entry[n_rtf].s_trans_coil.tmp_tgt > sps_entry[n_rtf].s_cur_coil.tmp_tgt)
   {
		DH_stp = ns_STD::cl_BIS::BISRA_Enth( 2, sps_entry[n_rtf].s_trans_coil.tmp_tgt + 2.f) - 
               ns_STD::cl_BIS::BISRA_Enth( 2, sps_entry[n_rtf].s_cur_coil.tmp_tgt);
   }

	else
   {
		DH_stp = ns_STD::cl_BIS::BISRA_Enth( 2, sps_entry[n_rtf].s_trans_coil.tmp_tgt - 4.f) - 
               ns_STD::cl_BIS::BISRA_Enth( 2, sps_entry[n_rtf].s_cur_coil.tmp_tgt);
   }

	float Delta_E = ns_STD::cl_BIS::BISRA_Enth( 2, T_ant) - ns_STD::cl_BIS::BISRA_Enth( 2, T_stbl);
	float Power_E = Delta_E * thi * wid * RO_STEEL * sps_entry[n_rtf].s_cur_coil.spd;//sps_entry[n_rtf].s_mea.spd;

   // if speed is decreased or power is decreased during anticipation time
	if (power < 0.f || sps_entry[n_rtf].s_trans_coil.spd < sps_entry[n_rtf].s_cur_coil.spd)
	{
      // Same target temperature
		if (sps_entry[n_rtf].s_trans_coil.tmp_tgt == sps_entry[n_rtf].s_cur_coil.tmp_tgt)
      {
         if      (sps_entry[n_rtf].s_cur_coil.spd != sps_entry[n_rtf].s_trans_coil.spd)
            *anticipation_time = 120.f;
         else if (sps_entry[n_rtf].s_cur_coil.thi != s_coil.s_coil_data[1].thi)
				*anticipation_time = 75.f;
         else
 				*anticipation_time = 45.f;
      }

      else
		{
	      float deb_mat = sps_entry[n_rtf].s_trans_coil.spd * thi * wid * RO_STEEL;

			// strip and rolls has to be heated
			if (DH_stp > 0 && dH_roll > 0)
			{
				// strip flow will heat the rolls
				*anticipation_time = rtf_cfg[n_rtf].tot_rol_mass * (s_coil.s_coil_data[0].wid / rtf_cfg[n_rtf].width) * 
                                 dH_roll / (deb_mat * DH_stp);

				// Tube reactivity
				//*anticipation_time += 120.f;                                                                              // test

            //*anticipation_time = max(min(*anticipation_time/50.f, 500),120.f);                                        // test
            // test
            // ====
		      if (Power_E + power != 0.f)
            {
			      *anticipation_time =  1.2f * (
                  ( rtf_cfg[n_rtf].tot_tub_mass + rtf_cfg[n_rtf].tot_rol_mass * (1.f - s_coil.s_coil_data[0].wid / rtf_cfg[n_rtf].width) ) * dH_tub
                                                                              +
								                          rtf_cfg[n_rtf].tot_rol_mass *      ( s_coil.s_coil_data[0].wid / rtf_cfg[n_rtf].width)   * dH_roll
                                            ) / (Power_E + power);
            }

		      else
            {
			      *anticipation_time = 1800.f;
            }
            // ====
            // test
         }

			// strip and rolls have to be cooled down
			else if (DH_stp < 0 && dH_roll < 0)
			{
				// strip flow has to get the colories from the rolls
				*anticipation_time = 1.3f *
                              rtf_cfg[n_rtf].tot_rol_mass * (s_coil.s_coil_data[0].wid / rtf_cfg[n_rtf].width) * dH_roll
                                      / (deb_mat * DH_stp);

				// Tube reactivity
				//*anticipation_time += 240.f;                                                                              // test
			}

			// if strip has to be heated and rolls to be cool down or vise and versa
			else
			{
				if (Power_E + power != 0.f)
            {
					*anticipation_time =  1.2f * (
                  ( rtf_cfg[n_rtf].tot_tub_mass + rtf_cfg[n_rtf].tot_rol_mass * (1 - s_coil.s_coil_data[0].wid / rtf_cfg[n_rtf].width ) ) * dH_tub
                                                                              +
						                                rtf_cfg[n_rtf].tot_rol_mass *    ( s_coil.s_coil_data[0].wid / rtf_cfg[n_rtf].width )   * dH_roll
                                            ) / (Power_E + power);
            }

				else
					*anticipation_time = 1800.f;
			}

			// If speed is decreased: take into account the speed decrease
			if ( sps_entry[n_rtf].s_cur_coil.spd > 0.f)
			{
				*anticipation_time = sps_entry[n_rtf].s_trans_coil.spd / sps_entry[n_rtf].s_cur_coil.spd * 
                                 *anticipation_time;
			}

			else
            *anticipation_time = 0.f;
		}

		if (*anticipation_time <= 0.f)
         *anticipation_time = 300.f;
	}  // if speed is decreased or power is decreased during anticipation time

   // if we increase or keep the same speed during anticipation time, we anticipate a little bit more
	else
	{
		if (Power_E + power != 0.f)
      {//before 0.3f
			*anticipation_time =  0.7f * (	
            ( rtf_cfg[n_rtf].tot_tub_mass + rtf_cfg[n_rtf].tot_rol_mass * (1.f - s_coil.s_coil_data[0].wid / rtf_cfg[n_rtf].width) ) * dH_tub
                                                                        +
								                    rtf_cfg[n_rtf].tot_rol_mass *      ( s_coil.s_coil_data[0].wid / rtf_cfg[n_rtf].width)   * dH_roll
                                      ) / (Power_E + power);
      }

		else
      {
			*anticipation_time = min(520.f,(s_coil.s_coil_data[0].lgt/(sps_entry[RTH].s_cur_coil.spd * 3.f)));//1800.f;
      }
		*anticipation_time = min(520.f,abs(*anticipation_time));
		if(sps_entry[RTH].s_cur_coil.thi < 0.0017f)
			*anticipation_time = min(520.f,(s_coil.s_coil_data[0].lgt/(sps_entry[RTH].s_cur_coil.spd * 3.f)));
		else if(sps_entry[RTH].s_cur_coil.thi < 0.002f)
			*anticipation_time = min(520.f,(s_coil.s_coil_data[0].lgt/(sps_entry[RTH].s_cur_coil.spd * 3.f)));
		else
			*anticipation_time = min(520.f,(s_coil.s_coil_data[0].lgt/(sps_entry[RTH].s_cur_coil.spd * 3.f)));

		*anticipation_time = min(max(*anticipation_time, 140.f),520.f);// + 180.f;
		//*anticipation_time = min( fabs(*anticipation_time), 1800.f );            // test
	} // if we increase or keep the same speed during anticipation time, we anticipate a little bit more

	//*anticipation_time = 75;                                                 // test

   return;
}

/*============================================================================
/ SPS_DEL_first_roll_prf: first roll profil calculation after an accidental shutdown.
/  calculate the risk of heat buckles creation on the first roller of the furnace.
/ 
/ Revisions:
/  Created:       2007-Jun       J. NAVEIRA
/  Modified: 
/
/ a faire température de tube en entrée
/ lors du premier passage, initialiser la température de bande à la température du rouleau
/ repiquer sur le lsin alpha qui est la sortie de ce programme
/
/ Exit: roll_calcul.roll[0].risk
/
/ Incoming: roll_calcul->roll[iroll].tube.temp
/           rol_cmp_risk.roll[0].strip.temp
/           sps_entry.mea.t_t[0]
/
/                                                                                               
/   middle of the roll                                                                       
/        ___________________________________________                                            
/       |             i-1    i    i+1    STRIP     |                                           
/       |___________________________________________________________________                    
/       |              |     |     |                                        |                    
/       |              |     |     |                                        |                    
/                      |     |     |                                        |                    
/       |___________________________________________________________________|                   
/                            ^                                                                  
/       |                    | Flux du bas provenant de tt mesh                                
/       |                    |                                                                  
/       |                                                                                       
/       |                                                                                       
/                            |                                                                  
/       |                    | Flux du haut provenant de ttes mesh                   
/                            V                                                                   
/       |__________________________________________________________________                     
/       |              |     |     |                                       |                    
/       |              |     |     |                                       |                    
/       |              |     |     |                                       |                    
/        __________________________________________________________________|                  
/                     i-1  ^ i    i+1                                                           
/                          |                                                                    
/                          |                                                                    
/                          | Flux du tube et écrans                                                       
/===========================================================================*/
float SPS_DEL_first_roll_prf(bool first_time, float dt, int n_rtf)
{
   bool status = true;
   string sub_name = "SPS_DEL_first_roll_prf";

   int N_pdt, N_cla;
   int n_roll = 0;
   int N_roll[MAX_NB_ROL], N_elmt=0;
   float radius, dx, tmp_stp;
	float e[3], s[3];
	int count;

   float beg_rtf_tmp = fur_cfg[n_rtf].entry_tmp;

   // Product and class of current coil
   N_pdt = s_coil.s_coil_data[0].N_pdt;
   N_cla = s_coil.s_coil_data[0].N_cla;

   // Static data initialization
   RTF_static_init (&rtf[n_rtf], fur_cfg[n_rtf].nb_zon, rtf_cfg[n_rtf], N_pdt, N_cla, 0 );

	/* The given speed is not important 
   /* The rtf structure is just usefull to know the number of the first roller element */
	rtf[n_rtf].spd = s_coil.s_coil_data[0].spd_max_th[n_rtf];

   // Dynamic data initialization
   RTF_dynamic_init( &rtf[n_rtf], s_coil.s_coil_data[0].thi, s_coil.s_coil_data[0].wid, 
                     s_coil.s_coil_data[0].emi_rtf[n_rtf], beg_rtf_tmp );

   // Update roll index in element table
   // ----------------------------------
   // For all elements
   for ( int i_elt=0; i_elt < rtf_cfg[n_rtf].nb_elt; i_elt++ )
   {
      // If the element is a roll one
      if ( rtf[n_rtf].s_elt[i_elt].i_roll ) 
      {
         N_roll[n_roll] = i_elt;
         n_roll ++;
      }
   } // For all elements

   // Tube temperature [k]  ---  Initialization with tube temperature measurement
   // ---------------------------------------------------------------------------
   rol_cmp_risk[n_rtf].s_rol[0].s_tub.tmp = sps_entry[n_rtf].s_mea.tmp_zon[0];

   // ==================================================================
   // Initialization
   // ==================================================================
	// Mesh on half roll length
   dx = rol_cmp_risk[n_rtf].s_rol[0].lgt / 2.f / float(MAX_NB_MESH-2);

	/* If first time ----- Accidental shutdown has just occured */
   if (first_time)
   {
      // Calculate the strip temperature --> Get the first roller temperature
      status = RTF_strip( &rtf[n_rtf], false, sps_entry[n_rtf].s_mea.tmp_zon, &tmp_stp );

      /* Strip caracteristics */
      rol_cmp_risk[n_rtf].s_rol[0].s_strip.wid = s_coil.s_coil_data[0].wid;
      rol_cmp_risk[n_rtf].s_rol[0].s_strip.steel_charac = 1;
      rol_cmp_risk[n_rtf].s_rol[0].s_strip.tension_basis = s_coil.s_coil_data[0].strip_tension;
      rol_cmp_risk[n_rtf].s_rol[0].s_strip.thi = s_coil.s_coil_data[0].thi;
      rol_cmp_risk[n_rtf].s_rol[0].s_strip.tension_basis = 300.f / 
                     (rol_cmp_risk[n_rtf].s_rol[0].s_strip.thi * rol_cmp_risk[n_rtf].s_rol[0].s_strip.wid);

      N_elmt = N_roll[0];
      if ( N_elmt < 0 ) N_elmt = 0;

      // Strip temperature [K]   ---  Initialization with roll temperature
      rol_cmp_risk[n_rtf].s_rol[0].s_strip.tmp = rtf[RTH].s_elt[N_elmt].s_strip.tmp_mid_strip;

      // Tube caracteristics
      rol_cmp_risk[n_rtf].s_rol[0].s_tub.area = 5.f * rol_cmp_risk[n_rtf].s_rol[0].dia * 
                                                rol_cmp_risk[n_rtf].s_rol[0].lgt / 2.f;

      // Shield caracteristics
      // Shield area: If indic=0 no shield; if 1 shield under roll; if 2, 
      //    2 shields under and between the rolls
      if ( rol_cmp_risk[n_rtf].s_rol[0].indic == 1 )
      {
         rol_cmp_risk[n_rtf].s_rol[0].s_shield.area = rol_cmp_risk[n_rtf].s_rol[0].s_shield.wid * 
                                                      rol_cmp_risk[n_rtf].s_rol[0].lgt / 2.f;
      }

      else
      {
         rol_cmp_risk[n_rtf].s_rol[0].s_shield.area = 10.f * rol_cmp_risk[n_rtf].s_rol[0].s_shield.wid * 
                                                      rol_cmp_risk[n_rtf].s_rol[0].lgt / 2.f;
      }

      // Initialization of mesh temperature  ---  Initialization with roll temperature
      // -----------------------------------------------------------------------------
      // For top and bot (roll face in contact with strip and roll face which received 
      //    radiation flow of shield and tube)
      for ( int i_top=0; i_top < 2; i_top++)
      {
         rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[0][0].dx = dx;    
         rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[0][0].x = dx / 2.f;

         // For all mesh but first one
         for ( int i_mesh = 1; i_mesh < MAX_NB_MESH; i_mesh++ )
         {
            rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[0][i_mesh].dx = dx;
            rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[0][i_mesh].x = 
                        rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[0][i_mesh-1].x + dx;
         } // For all mesh but first one

         // For all mesh points
         for ( int i_mesh = 0; i_mesh < MAX_NB_MESH; i_mesh++ )
         {
            if (rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[0][i_mesh].x > rol_cmp_risk[n_rtf].s_rol[0].s_strip.wid/2.f)
            {
               rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[i_top][i_mesh].tmp = 
                                 min(rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[i_top][i_mesh-1].tmp + 100.f, 
                                     sps_entry[RTH].s_mea.tmp_zon[0] - 10.f);
            }

            else
            {
               rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[i_top][i_mesh].tmp = 
                                          rol_cmp_risk[n_rtf].s_rol[0].s_strip.tmp + 200.f;
            }
         } // For all mesh points
		} // For top bottom points

      // Cond coefficient betwen roll and strip
      rol_cmp_risk[n_rtf].coef = rtf_cfg[n_rtf].r_coeff; 
   }// If first time

   // Security
   if (rol_cmp_risk[n_rtf].s_rol[0].s_strip.thi == 0.f)
   {
      ns_STD::cl_TRC::ERR_Write("SPS", sub_name, "Wid = %f, thi = %f", rol_cmp_risk[n_rtf].s_rol[0].s_strip.wid, rol_cmp_risk[n_rtf].s_rol[0].s_strip.thi);
      return 100000000;
   }

   // Mesh on half roll length
   dx = rol_cmp_risk[n_rtf].s_rol[0].lgt / 2.f / float(MAX_NB_MESH-2);

   // Strip tension [Pa]
   if ( !rol_cmp_risk[n_rtf].s_rol[0].top )
   {
      // Line top roll
      rol_cmp_risk[n_rtf].s_rol[0].s_strip.tension = 300.f / 
                                  rol_cmp_risk[n_rtf].s_rol[0].s_strip.thi * rol_cmp_risk[n_rtf].s_rol[0].s_strip.wid;
   }

   else
   {
      rol_cmp_risk[n_rtf].s_rol[0].s_strip.tension = 300.f / 
                  (rol_cmp_risk[n_rtf].s_rol[0].s_strip.thi * rol_cmp_risk[n_rtf].s_rol[0].s_strip.wid) + 
                              RO_STEEL * 9.81f * rol_cmp_risk[n_rtf].s_fur.hgt * 
                              rol_cmp_risk[n_rtf].s_rol[0].s_strip.wid * 
                              rol_cmp_risk[n_rtf].s_rol[0].s_strip.thi /
                              rol_cmp_risk[n_rtf].s_rol[0].lgt /
                              rol_cmp_risk[n_rtf].s_rol[0].dia;
   }

   // Depending on the strip steel grade
   switch (rol_cmp_risk[n_rtf].s_rol[0].s_strip.steel_charac)
   {
      // Young modulus & elastic limit calculation
      case 1:
      {
         // CQ Steel
         rol_cmp_risk[n_rtf].s_rol[0].s_strip.young = ns_STD::cl_UTL::Interpolation( ns_STD::cl_PHY::Tab_yo,  ns_STD::cl_PHY::Tab_tyo, 
                                                      rol_cmp_risk[n_rtf].s_rol[0].s_strip.tmp, NB_PTS_YOUNG);
         rol_cmp_risk[n_rtf].s_rol[0].s_strip.lim_el = ns_STD::cl_UTL::Interpolation(ns_STD::cl_PHY::Tab_ecq, ns_STD::cl_PHY::Tab_tecq, 
                                                      rol_cmp_risk[n_rtf].s_rol[0].s_strip.tmp, NB_PTS_YOUNG);
         break;      
      }

      case 2:
      {
         // DQ Steel
         rol_cmp_risk[n_rtf].s_rol[0].s_strip.young = ns_STD::cl_UTL::Interpolation( ns_STD::cl_PHY::Tab_yo,  ns_STD::cl_PHY::Tab_tyo, 
                                                      rol_cmp_risk[n_rtf].s_rol[0].s_strip.tmp, NB_PTS_YOUNG);
         rol_cmp_risk[n_rtf].s_rol[0].s_strip.lim_el = ns_STD::cl_UTL::Interpolation(ns_STD::cl_PHY::Tab_edq, ns_STD::cl_PHY::Tab_tedq, 
                                                      rol_cmp_risk[n_rtf].s_rol[0].s_strip.tmp, NB_PTS_YOUNG);
         break;      
      }

      case 3:
      {
         // DDQ Steel
         rol_cmp_risk[n_rtf].s_rol[0].s_strip.young = ns_STD::cl_UTL::Interpolation( ns_STD::cl_PHY::Tab_yo, ns_STD::cl_PHY::Tab_tyo, 
                                                      rol_cmp_risk[n_rtf].s_rol[0].s_strip.tmp, NB_PTS_YOUNG);
         rol_cmp_risk[n_rtf].s_rol[0].s_strip.lim_el = ns_STD::cl_UTL::Interpolation(ns_STD::cl_PHY::Tab_eeddq, ns_STD::cl_PHY::Tab_teeddq, 
                                                      rol_cmp_risk[n_rtf].s_rol[0].s_strip.tmp, NB_PTS_YOUNG);
         break;
      }
   } // Depending on the strip steel grade

   // Tube area         
   rol_cmp_risk[n_rtf].s_rol[0].s_tub.area = 5.f * rol_cmp_risk[n_rtf].s_rol[0].dia * 
                                             rol_cmp_risk[n_rtf].s_rol[0].lgt / 2.f;

   // Shield temperature
   rol_cmp_risk[n_rtf].s_rol[0].s_shield.tmp = rol_cmp_risk[n_rtf].s_rol[0].s_tub.tmp - 30.f;

   // Shield area: If indic=0 no shield; if 1 shield under roll; if 2, 
   //    2 shields under and between the rolls
   if (rol_cmp_risk[n_rtf].s_rol[0].indic == 1)
   {
      rol_cmp_risk[n_rtf].s_rol[0].s_shield.area = rol_cmp_risk[n_rtf].s_rol[0].s_shield.wid * 
                                                   rol_cmp_risk[n_rtf].s_rol[0].lgt / 2.f;
   }

   else if ( rol_cmp_risk[n_rtf].s_rol[0].indic == 2 )
   {
      rol_cmp_risk[n_rtf].s_rol[0].s_shield.area = 10.f * rol_cmp_risk[n_rtf].s_rol[0].s_shield.wid * 
                                                   rol_cmp_risk[n_rtf].s_rol[0].lgt / 2.f;
   }

   else if ( rol_cmp_risk[n_rtf].s_rol[0].indic == 0 )
   {
      rol_cmp_risk[n_rtf].s_rol[0].s_shield.area = 0.f;
      rol_cmp_risk[n_rtf].s_rol[0].s_shield.tmp = 0.f;
   }

   // =========================================================================
   // First roll cold profil calcul
   // =========================================================================
   rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[0][0].dx = dx;    
   rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[0][0].x = dx / 2.f;
   rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[0][0].heat_exp = 0.f;
   rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[0][0].cold_prf = 0.f;
   rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[0][0].hot_prf = 0.f;
   rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[1][0].dx = dx;    
   rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[1][0].x = dx / 2.f;

   // For all mesh elements
   for ( int imesh = 1; imesh < MAX_NB_MESH; imesh++ )
   {
      rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[0][imesh].dx = dx;
      rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[0][imesh].x = rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[0][imesh-1].x + dx;
      rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[1][imesh].dx = dx;
      rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[1][imesh].x = rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[1][imesh-1].x + dx;
      rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[0][imesh].heat_exp = 0.f;

      switch (rol_cmp_risk[n_rtf].s_rol[0].indic_roll)
      {
         case 1:
            if ( rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[0][imesh].x < rol_cmp_risk[n_rtf].s_rol[0].l1 / 2.f )
            {
               rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[0][imesh].cold_prf = 0.f;
            }

            else if ( rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[0][imesh].x < 
                      rol_cmp_risk[n_rtf].s_rol[0].l1/2.f + rol_cmp_risk[n_rtf].s_rol[0].l2 )
            {
               rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[0][imesh].cold_prf = 
                                       - ( rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[0][imesh].x - 
                                                              rol_cmp_risk[n_rtf].s_rol[0].l1/2 ) / 
                                                              rol_cmp_risk[n_rtf].s_rol[0].l2 * 
                                                              rol_cmp_risk[n_rtf].s_rol[0].c1;
            }

            else if ( rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[0][imesh].x < 
                      rol_cmp_risk[n_rtf].s_rol[0].l1/2.f + rol_cmp_risk[n_rtf].s_rol[0].l2 + 
                                                     rol_cmp_risk[n_rtf].s_rol[0].l3 )
            {
               rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[0][imesh].cold_prf = - rol_cmp_risk[n_rtf].s_rol[0].c1 - 
                                        ( rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[0][imesh].x - 
                                          rol_cmp_risk[n_rtf].s_rol[0].l1/2.f - 
                                          rol_cmp_risk[n_rtf].s_rol[0].l2 ) /
                                          rol_cmp_risk[n_rtf].s_rol[0].l3 * 
                                          rol_cmp_risk[n_rtf].s_rol[0].c2;
            }

            else
            {
               rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[0][imesh].cold_prf = - rol_cmp_risk[n_rtf].s_rol[0].c1 - 
                                                                        rol_cmp_risk[n_rtf].s_rol[0].c2;
            }

            rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[0][imesh].hot_prf = 
                     rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[0][imesh].cold_prf;
         break;

         case 2:
            if ( rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[0][imesh].x < rol_cmp_risk[n_rtf].s_rol[0].l1/2.f )
            {
               rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[0][imesh].cold_prf = 0.f;
            }

            else if ( rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[0][imesh].x < 
                      rol_cmp_risk[n_rtf].s_rol[0].l1/2.f + rol_cmp_risk[n_rtf].s_rol[0].l2 )
            {
               rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[0][imesh].cold_prf = 
                                       - ( rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[0][imesh].x - 
                                                                 rol_cmp_risk[n_rtf].s_rol[0].l1/2.f ) / 
                                                                 rol_cmp_risk[n_rtf].s_rol[0].l2 * 
                                                                 rol_cmp_risk[n_rtf].s_rol[0].c1;
            }

            else 
            {
               rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[0][imesh].cold_prf = - rol_cmp_risk[n_rtf].s_rol[0].c1 - 
                                          ( rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[0][imesh].x - 
                                            rol_cmp_risk[n_rtf].s_rol[0].l1/2.f - 
                                            rol_cmp_risk[n_rtf].s_rol[0].l2 ) /
                                           ( rol_cmp_risk[n_rtf].s_rol[0].lgt/2.f - 
                                             rol_cmp_risk[n_rtf].s_rol[0].l2 - 
                                             rol_cmp_risk[n_rtf].s_rol[0].l1/2.f ) *
                                             rol_cmp_risk[n_rtf].s_rol[0].c2;
            }

            rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[0][imesh].hot_prf = 
                     rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[0][imesh].cold_prf;
         break;

         case 3:
            if ( rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[0][imesh].x < rol_cmp_risk[n_rtf].s_rol[0].l1/2.f )
            {
               rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[0][imesh].cold_prf = 0.f;
            }

            else
            {
               rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[0][imesh].cold_prf = 
                                       - ( rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[0][imesh].x - 
                                                              rol_cmp_risk[n_rtf].s_rol[0].l1/2.f ) /
                                                            ( rol_cmp_risk[n_rtf].s_rol[0].lgt/2.f - 
                                                              rol_cmp_risk[n_rtf].s_rol[0].l1/2.f ) *
                                                              rol_cmp_risk[n_rtf].s_rol[0].c1;
            }

            rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[0][imesh].hot_prf = 
                     rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[0][imesh].cold_prf;
         break;

         case 4:
            radius = ( rol_cmp_risk[n_rtf].s_rol[0].lgt / 2.f * rol_cmp_risk[n_rtf].s_rol[0].lgt / 2.f + 
                       rol_cmp_risk[n_rtf].s_rol[0].c1 * rol_cmp_risk[n_rtf].s_rol[0].c1 ) / 2.f / rol_cmp_risk[n_rtf].s_rol[0].c1;
            rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[0][imesh].cold_prf = 
               sqrt( radius*radius -
                     rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[0][imesh].x * rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[0][imesh].x ) -
                     radius;
         break;
      }
   } // For all mesh elements

   /* ================================================================== */
   /* Dynamic initialisation for radiation factor calculation */
   /* ================================================================== */
   /* Upper form factor */
   /* ----------------- */
   e[0] = rol_cmp_risk[n_rtf].s_rol[0].e;
   e[1] = rol_cmp_risk[n_rtf].s_rol[0].s_shield.e;
   e[2] = rol_cmp_risk[n_rtf].s_rol[0].s_tub.e;

   // s[0] area for a half roll
   s[0] = PI * rol_cmp_risk[n_rtf].s_rol[0].dia / 2.f * ( rol_cmp_risk[n_rtf].s_rol[0].lgt ) / 2.f;
   s[1] = rol_cmp_risk[n_rtf].s_rol[0].s_shield.area;
   s[2] = rol_cmp_risk[n_rtf].s_rol[0].s_tub.area;

   ns_STD::cl_ROL::ROL_Radiation_factor ( e, s, rol_cmp_risk[n_rtf].s_rol[0].fr_u, &rol_cmp_risk[n_rtf].s_rol[0].deter_u );

   /* ================================================================== */
   /* Calculation of the heat radiation inside the roll */
   /* ================================================================== */
   status = ns_STD::cl_ROL::SPS_DEL_first_roll_thermal( &rol_cmp_risk[n_rtf].s_rol[0], s_Trol[n_rtf], dt );
   if (!status) return 10000000;
  
   /* ================================================================== */
   /* Calculation of heat roll profil and strip risk */
   /* ================================================================== */
   status = ns_STD::cl_ROL::ROL_mechanical_risk( &rol_cmp_risk[n_rtf].s_rol[0], s_Trol[rol_cmp_risk[n_rtf].n_rtf]);
   if (!status) return 10000000;
         
   // ==================================================================
   // Strip temperature  -----  for next calcul
   // ==================================================================
   /* for all mesh */
   rol_cmp_risk[n_rtf].s_rol[1].s_strip.tmp = 0.f;
   count = 0;
   for ( int i_mesh=1; i_mesh < MAX_NB_MESH; i_mesh++)
   {
      if ( rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[1][i_mesh].x < rol_cmp_risk[n_rtf].s_rol[0].s_strip.wid / 2.f )
      {
         rol_cmp_risk[n_rtf].s_rol[0].s_strip.tmp += rol_cmp_risk[n_rtf].s_rol[0].s_mesh_risk[1][i_mesh].tmp;
         count ++;
      }
      else break;
   } /* for all mesh */

   // Average strip temperature for next calculation step
   if (count > 0) rol_cmp_risk[n_rtf].s_rol[0].s_strip.tmp /= float(count);

   return rol_cmp_risk[n_rtf].s_rol[0].risk;
} // void