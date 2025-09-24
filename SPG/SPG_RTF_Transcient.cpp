//*============================================================================
// SPG_RTF_Transient: Transition type identification.
//  
// (c) CMI Industry
//===========================================================================*/
#include "stdafx.h"

/*============================================================================
/ SPG_RTF_Transient: Transition type identification.
/ 
/ Revisions:
/  Created:       2004          P.Dubois
/  Modified: 
/===========================================================================*/
bool SPG_RTF_Transient( struct rtf_struct *rtf_first, struct rtf_struct *rtf_second, 
                        struct strip_trans_entry *data_first, struct strip_trans_entry *data_second, 
                        struct strip_trans_exit *data_trans, int i1, int i2 )
{   
   bool status = true;
   string sub_name = "SPG_RTF_Transient";
   bool TRC=ns_STD::cl_TRC::TRC_Sub( sub_name );

	float tmp_tub[MAX_NB_ZON], tmp_stp=0.f, tmp_stp_rtf2;
   bool roll;

   int n_rtf = rtf_first->n_rtf;

   //memcpy( &rtf[n_rtf], rtf_second, sizeof(struct rtf_struct) );  // ?????
   rtf[n_rtf] = *rtf_second;
   rtf[n_rtf].spd = data_first->spd;  

   // If a dummy
   if ( data_second->dummy == 1 )
   {
      roll = true;

      // Tube temperatures
      for ( int i_zon=0; i_zon < rtf_first->nb_zon; i_zon++ ) tmp_tub[i_zon] = data_first->tmp_tub[i_zon];

      // Roll temperatures initialization
      SPG_RTF_ini_rol_tmp (rtf_first, &rtf[n_rtf], true, data_first->stp_rtf, data_trans);

      // Calculation of the thick strip temperature taking into account the roll exchange
      status = RTF_strip (&rtf[n_rtf], roll, tmp_tub, &tmp_stp_rtf2);
      if ( !status ) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in RTF_strip", fur_cfg[RTH + n_rtf].fur_name.c_str());

      // Store anticipation period process values
      data_trans->tmp_stp_rtf  = data_first->tmp_tgt;
      data_trans->tmp_stp_rtf2 = tmp_stp_rtf2;
      data_trans->spd          = data_first->spd;
      data_trans->tmp_tub_ref  = data_first->tmp_tub_ref;

      // Tube temperatures
      for ( int i_zon=0; i_zon < rtf_first->nb_zon; i_zon++ )
      {
			data_trans->tmp_tub[i_zon]     = data_first->tmp_tub[i_zon] + 10.f;
			data_trans->pow_pct_rtf[i_zon] = data_first->pow_pct_rtf[i_zon] + 10.f;
		}

      // For all elements
      for ( int i_elt=0; i_elt < rtf_first->nb_elt; i_elt++ )
      {
         data_trans->stp_rtf[i_elt] = rtf_first->s_elt[i_elt].s_strip.tmp_out_strip;
      }

      return status;
   } // If a dummy

   // Calculation of strip temperature of the 2nd coil with the tube temperatures
   // of the 1st one to determine which coil is the thinner.
   // ===========================================================================
   roll = false;

   // Tube temperatures
   for ( int i_zon=0; i_zon < rtf_first->nb_zon; i_zon++ ) tmp_tub[i_zon] = data_first->tmp_tub[i_zon];

   // Strip temperature calculation.
   // ------------------------------ 
   // Input: Coil characteristics, speed, strip temperature target.
   //   s_coil.s_coil_data[i1].thi
   //   s_coil.s_coil_data[i1].wid
   //   s_coil.s_coil_data[i1].spd
   //   s_coil.s_coil_data[i1].tmp_tgt[n_rtf]
   // Output: Tubes temperatures.
   //   t_t1
   // If roll = false, strip temperature on the rolls as additionnal output.
   //   strip_roll_t[NB_ROL_RTF]
   // If roll = true, roll temperatures as additionnal input
   status = RTF_strip( &rtf[n_rtf], roll, tmp_tub, &tmp_stp );
   if ( !status ) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in RTF_strip (1)", fur_cfg[RTH + n_rtf].fur_name.c_str());

   // If the temperature target is the same
   // =====================================
   if ( fabs(data_first->tmp_tgt - data_second->tmp_tgt) < 1.f )
   {
      // If the estimated strip temperature is closed to the 1st strip temperature target
      //    the transient state is equal to the first coil steady state
      if ( fabs (tmp_stp - data_second->tmp_tgt) <= 5.f && 
           fabs(data_first->spd - data_second->spd) < 2.f/60.f )
      {
         // Strore anticipation period process values
         data_trans->tmp_stp_rtf   = data_second->tmp_tgt;
         data_trans->tmp_stp_rtf2  = data_second->tmp_tgt;
         data_trans->spd           = data_first->spd;
         data_trans->tmp_tub_ref   = data_first->tmp_tub_ref;

         // Tube temperatures
         for ( int i_zon=0; i_zon < rtf_first->nb_zon; i_zon++ )
         {
            data_trans->tmp_tub[i_zon]     = data_first->tmp_tub[i_zon];
            data_trans->pow_pct_rtf[i_zon] = data_first->pow_pct_rtf[i_zon];
         }

         // For all elements
         for ( int i_elt=0; i_elt<rtf_first->nb_elt; i_elt++ )
            data_trans->stp_rtf[i_elt] = data_first->stp_rtf[i_elt];
      } // If the estimated strip temperature is closed to the 1st strip temperature target

      // If the estimated strip temperature is less than the 1st strip temperature target
      else if ( tmp_stp <= data_first->tmp_tgt + 2.f )
      {
         // The transition is thin to thick.
         if (TRC) ns_STD::cl_TRC::TRC_Write("INF", "SPG", sub_name, "The transition is thin to thick (2)");
         status = SPG_RTF_Thin_Thick ( rtf_first, rtf_second, data_first, data_second, data_trans, i1, i2 );
         if ( !status ) 
         {
            ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in SPG_RTF_Thin_Thick (3)", fur_cfg[RTH + n_rtf].fur_name.c_str());
            return status;
         }
      } // If the estimated strip temperature is less than the 1st strip temperature target

      // Else (estimated strip temperature greater than the 1st strip temperature target)
      else
      {
         // Transition thick to thin
         if (TRC) ns_STD::cl_TRC::TRC_Write("INF", "SPG", sub_name, "The transition is thick to thin (4)");
         status = SPG_RTF_Thick_Thin (rtf_first, rtf_second, data_first, data_second, data_trans, i1, i2);
         if ( !status ) 
         {
            ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in SPG_RTF_Thick_Thin (5)", fur_cfg[RTH + n_rtf].fur_name.c_str());
            return status;
         }
      } // Else (estimated strip temperature greater than the 1st strip temperature target)
   } // If the temperature target is the same

   // Else (the temperature target is not the same)
   // =============================================
   else
   {
      // If the 1st strip temperature target is greater than the 2nd strip one
      if ( data_first->tmp_tgt > data_second->tmp_tgt )
      {
         // If the estimated strip temperature is less than the 1st strip temperature target
         if ( tmp_stp <= data_first->tmp_tgt )
         {
            // The transition is "hot thin" to "cold thick"
            if (TRC) ns_STD::cl_TRC::TRC_Write("INF", "SPG", sub_name, "The transition is hot thin to cold thick (6)");
            status = SPG_RTF_HotThin_ColdThick ( rtf_first, rtf_second, data_first, data_second, data_trans, i1, i2 );
            if ( !status ) 
            {
               ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in SPG_RTF_HotThin_ColdThick (7)", fur_cfg[RTH + n_rtf].fur_name.c_str());
               return status;
            }
         } // If the estimated strip temperature is less than the 1st strip temperature target

         // Else (estimated strip temperature greater than the 1st strip temperature target)
         else
         {
            // The transition is "hot thick" to "cold thin"
            if (TRC) ns_STD::cl_TRC::TRC_Write("INF", "SPG", sub_name, "The transition is hot thick to cold thin (8)");
            status = SPG_RTF_HotThick_ColdThin ( rtf_first, rtf_second, data_first, data_second, data_trans, i1, i2 );
            if ( !status ) 
            {
               ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in SPG_RTF_HotThick_ColdThin (9)", fur_cfg[RTH + n_rtf].fur_name.c_str());
               return status;
            }
         } // Else (estimated strip temperature greater than the 1st strip temperature target)
      } // If the 1st strip temperature target is greater than the 2nd strip one

      // Else (1st strip temperature target lower than the 2nd strip one)
      else
      {
         // If the estimated strip temperature is less than the 1st strip temperature target
         if (tmp_stp <= data_first->tmp_tgt)
         {
            // The transition is "cold thin" to "hot thick"
            if (TRC) ns_STD::cl_TRC::TRC_Write("INF", "SPG", sub_name, "The transition is cold thin to hot thick (10)");
            status = SPG_RTF_ColdThin_HotThick ( rtf_first, rtf_second, data_first, data_second, data_trans, i1, i2 );
            if ( !status ) 
            {
               ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in SPG_RTF_ColdThin_HotThick (11)", fur_cfg[RTH + n_rtf].fur_name.c_str());
               return status;
            }
         } // If the estimated strip temperature is less than the 1st strip temperature target

         // Else (estimated strip temperature greater than the 1st strip temerature target)
         else
         {
            // The transition is "cold thick" to "hot thin"
            if (TRC) ns_STD::cl_TRC::TRC_Write("INF", "SPG", sub_name, "The transition is cold thick to hot thin (12)");
            status = SPG_RTF_ColdThick_HotThin ( rtf_first, rtf_second, data_first, data_second, data_trans, i1, i2 );
            if ( !status ) 
            {
               ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in SPG_RTF_ColdThick_HotThin (13)", fur_cfg[RTH + n_rtf].fur_name.c_str());
               return status;
            }
         } // Else (estimated strip temperature greater than the 1st strip temerature target).
      } // Else (1st strip temperature target lower than the 2nd strip one)
   } // Else (the temperature target is not the same).

   return status;
} // void

/*============================================================================
/ SPG_RTF_ColdThick_HotThin: "Cold thick" to "hot thin" transition.
/ 
/ Revisions:
/  Created:       2004          P.Dubois
/  Modified: 
/===========================================================================*/
bool SPG_RTF_ColdThick_HotThin( struct rtf_struct *rtf_first, struct rtf_struct *rtf_second, 
                                struct strip_trans_entry *data_first, struct strip_trans_entry *data_second, 
                                struct strip_trans_exit *data_trans, int i1, int i2 )
{   
   bool status=true;
   string sub_name = "SPG_RTF_ColdThick_HotThin";

   float tmp_ref_tub=0.f, tmp_stp=0.f, aimed_tmp_rtf;
   float delta_t, spd, spd_max, tmp_tub[MAX_NB_ECZ];
   bool roll, strip;
   float power[MAX_NB_ZON];
   int loop=0;

   int n_rtf = rtf_first->n_rtf;

	/* First strip transcient maximum speed calculation */
   rtf[n_rtf] = *rtf_first;
   status = SPG_RTF_Max_Spd( false, &rtf[n_rtf], data_first->tmp_upp, data_first->tmp_in, &spd_max);

   // Set maximum speed
   spd = max(data_first->spd, data_second->spd);

   // Limit transcient maximum speed with calculated maximum speed
   spd = min (spd, spd_max );

   // Limit transcient maximum speed with calculated maximum speed
   rtf_first->spd = spd;
   rtf_second->spd = spd;

   // Calculation of the 2nd strip temperature with the tube temperatures of the 1st one
   //===================================================================================
   roll = false;
   strip = true;
                                                  
	/* Set first speed for transition */
   rtf_first->spd = spd;

	/* Tube temperature calculation */
	status = SPG_RTF_Tube_Tmp (rtf_first, roll, data_first->tmp_upp, &tmp_ref_tub, tmp_tub, 
                              data_second->thi, data_second->wid); 
   if ( !status ) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in SPG_RTF_Tube_Tmp", fur_cfg[RTH + n_rtf].fur_name.c_str());

	// Calculation of the total gas flow in percentage of the connected inputs
   RTF_power (rtf_first, power);

   rtf[n_rtf] = *rtf_second;
   rtf[n_rtf].spd = spd;
   roll = true;

   // Roll temperatures initialization 
   SPG_RTF_ini_rol_tmp (rtf_first, &rtf[n_rtf], strip, data_first->stp_rtf, data_trans);

   // Calculation of the 2nd strip temperature taking into account the roll exchange
   status = RTF_strip (&rtf[n_rtf], roll, tmp_tub, &tmp_stp);
   if ( !status ) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in RTF_strip", fur_cfg[RTH + n_rtf].fur_name.c_str());

   // If the calculated strip temperature is greater than the 2nd product lower tolerance
   if ( tmp_stp >= data_second->tmp_low || !sps_entry[n_rtf].s_mea.spd_auto )
   {
      // If the calculated strip temperature is lower than the 2nd product upper tolerance
      if ( tmp_stp <= data_second->tmp_upp )
      {
         // The transition is thin to thick.
         status = SPG_RTF_Thin_Thick (rtf_first, rtf_second, data_first, data_second, data_trans, i1, i2);
         if ( !status )
         {
            ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in SPG_RTF_Thin_Thick", fur_cfg[RTH + n_rtf].fur_name.c_str());
            return status;
         }
      } // If the calculated strip temperature is lower than the 2nd product upper tolerance

      // Else (If the calculated strip temperature is greater than the 2nd product upper tolerance)
      else
      {
         // The transition is thick to thin
         status = SPG_RTF_Thick_Thin (rtf_first, rtf_second, data_first, data_second, data_trans, i1, i2);
         if ( !status )
         {
            ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in SPG_RTF_Thick_Thin", fur_cfg[RTH + n_rtf].fur_name.c_str());
            return status;
         }
      } // Else (If the calculated strip temperature is greater than the 2nd product upper tolerance)
   } // If the calculated strip temperature is greater than the 2nd product lower tolerance

   // Else calculation of the tube temperature to be close to the lower temperature of the thick product
   else
   {
      delta_t = 2.f;
      loop = 0;
      spd_max = spd;

      // While the calculated strip temperature is lower than the 2nd product lower temperature
      do
      {
         roll = false;
         strip = true;

         // Increase the target temperature from the upper tolerance
         aimed_tmp_rtf = data_first->tmp_upp + delta_t;

			/* First strip transcient maximum speed calculation */
         rtf[n_rtf] = *rtf_first;
         status = SPG_RTF_Max_Spd( false, &rtf[n_rtf], aimed_tmp_rtf, data_first->tmp_in, &spd_max);
         if ( !status ) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in SPG_RTF_Max_Spd", fur_cfg[RTH + n_rtf].fur_name.c_str());

         // Limit transcient maximum speed with calculated maximum speed
         spd = min (spd, spd_max );
         rtf_first->spd = spd;

			/* Tube temperature calculation for the max temperature of the 1st strip with offset */
			status = SPG_RTF_Tube_Tmp (rtf_first, roll, aimed_tmp_rtf, &tmp_ref_tub, tmp_tub, 
                                    data_second->thi, data_second->wid);
         if ( !status ) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in SPG_RTF_Tube_Tmp", fur_cfg[RTH + n_rtf].fur_name.c_str());
			
         //Calculation of the total gas flow in percentage of the connected inputs
		   RTF_power (rtf_first, power);

         // Store second coil data
         rtf[n_rtf] = *rtf_second;
         rtf[n_rtf].spd = spd;
         roll = true;

         // Roll temperatures initialization
         SPG_RTF_ini_rol_tmp (rtf_first, &rtf[n_rtf], strip, data_first->stp_rtf, data_trans);

         // Calculation of the thin strip temperature taking into account the roll exchange
         status = RTF_strip (&rtf[n_rtf], roll, tmp_tub, &tmp_stp);
         if ( !status ) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in RTF_strip", fur_cfg[RTH + n_rtf].fur_name.c_str());

         // Strip temperature increase
         delta_t = delta_t + 2.f;

         loop++;
		} while ( tmp_stp < data_second->tmp_low && loop < MAX_ITER && delta_t < 12);

      // Loop alarm treatment
      if (loop >= MAX_ITER)
      {
         ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Convergence error", fur_cfg[RTH + n_rtf].fur_name.c_str());
         status = false;
      }

      // Writing the transition set-points after convergence
      data_trans->tmp_stp_rtf  = aimed_tmp_rtf;
      data_trans->tmp_stp_rtf2 = tmp_stp;
      data_trans->spd = spd;
      data_trans->tmp_tub_ref  = tmp_ref_tub;

      // Tubes temperatures
      // ------------------
      // For all zones
      for ( int i_zon=0; i_zon < rtf_first->nb_zon; i_zon++ )
      {
         data_trans->tmp_tub[i_zon] = tmp_tub[i_zon];
         data_trans->pow_pct_rtf[i_zon] = power[i_zon];
      } // For all zones

      // For all elements
      for ( int i_elt=0; i_elt<rtf_first->nb_elt; i_elt++ )
      {
         data_trans->stp_rtf[i_elt] = rtf_first->s_elt[i_elt].s_strip.tmp_out_strip;
      } // For all elements
	} /* Else calculation of the tube temperature to be close to the lower temperature of the thick product */

   return status;
}

/*============================================================================
/ SPG_RTF_ColdThin_HotThick: "Cold thin" to "hot thick" transition.
/ 
/ Revisions:
/  Created:       2004          P.Dubois
/  Modified: 
/===========================================================================*/
bool SPG_RTF_ColdThin_HotThick( struct rtf_struct *rtf_first, struct rtf_struct *rtf_second, 
                                struct strip_trans_entry *data_first, struct strip_trans_entry *data_second, 
                                struct strip_trans_exit *data_trans, int i1, int i2 )
{   
   bool status=true;
   string sub_name = "SPG_RTF_ColdThin_HotThick";

   float tmp_ref_tub=0.f, tmp_stp=0.f, aimed_tmp_rtf;
   float spd, spd_max, delta_t, tmp_tub[NB_ZON_RTH];
   bool roll, strip;
   float power[NB_ZON_RTH];
   int loop=0;

   int n_rtf = rtf_first->n_rtf;

	/* First strip transcient maximum speed calculation */
   rtf[n_rtf] = *rtf_first;
   status = SPG_RTF_Max_Spd( false, &rtf[n_rtf], data_first->tmp_upp, data_first->tmp_in, &spd_max);

   // Set maximum speed
   spd = max(data_first->spd, data_second->spd);

   // Limit transcient maximum speed with calculated maximum speed
   spd = min (spd, spd_max );

   // Limit transcient maximum speed with calculated maximum speed
   rtf_first->spd = spd;
   rtf_second->spd = spd;

   // Calculation of the 2nd strip temperature with the tube temperatures of the 1st
   //===============================================================================
   roll = false;
   strip = true;

   // Set first coil speed for transition
   rtf_first->spd = spd;
	status = SPG_RTF_Tube_Tmp(rtf_first, roll, data_first->tmp_upp, &tmp_ref_tub, tmp_tub, 
                             data_second->thi, data_second->wid);
   if ( !status ) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in SPG_RTF_Tube_Tmp", fur_cfg[RTH + n_rtf].fur_name.c_str());

   RTF_power(rtf_first, power );

   rtf[n_rtf] = *rtf_second;
   rtf[n_rtf].spd = spd;
   roll = true;

   // Roll temperatures initialization
   SPG_RTF_ini_rol_tmp (rtf_first, &rtf[n_rtf], strip, data_first->stp_rtf, data_trans);

   // Calculation of the 2nd strip temperature taking into account the roll exchange
   status = RTF_strip (&rtf[n_rtf], roll, tmp_tub, &tmp_stp);
   if ( !status ) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in RTF_strip", fur_cfg[RTH + n_rtf].fur_name.c_str());

	/* If the calculated strip temperature is greater than the thick strip lower tolerance */
	if ( (tmp_stp >= data_second->tmp_low || !sps_entry[n_rtf].s_mea.spd_auto || 
         fabs(data_first->thi-data_second->thi) >= 0.05f/1000.f) && 
		 data_first->tmp_upp > data_first->tmp_low- 5.f )
   {
      // If the calculated strip temperature is lower than the thick strip upper tolerance
      if ( tmp_stp <= data_second->tmp_upp )
      {
         // The transition is thin to thick
         status = SPG_RTF_Thin_Thick ( rtf_first, rtf_second, data_first, data_second, data_trans, i1, i2 );
         if ( !status )
         {
            ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in SPG_RTF_Thin_Thick", fur_cfg[RTH + n_rtf].fur_name.c_str());
            return status;
         }
      }

      // Else
      else
      {
         // The transition is "thick to thin"
         status = SPG_RTF_Thick_Thin ( rtf_first, rtf_second, data_first, data_second, data_trans, i1, i2 );
         if ( !status )
         {
            ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in SPG_RTF_Thick_Thin", fur_cfg[RTH + n_rtf].fur_name.c_str());
            return status;
         }
      }
   } // If the calculated strip temperature is greater than the thick strip lower tolerance

   // Else (the calculated strip temperature is lower than the thick strip lower tolerance)
   else
   {
      // Calculation of the tube temperature to be close to the lower temperature of the thick product
      delta_t = 8.f;
      loop = 0;
      spd_max = spd;

      // While the calculated strip temperature is less than the thick strip low temperature
      do
      {
         roll = false;
         strip = true;

         // Increase the target temperature from the upper tolerance
         aimed_tmp_rtf = data_first->tmp_upp + delta_t;

			/* First strip transcient maximum speed calculation */
         rtf[n_rtf] = *rtf_first;
         status = SPG_RTF_Max_Spd(false, &rtf[n_rtf], aimed_tmp_rtf, data_first->tmp_in, &spd_max);
		   if ( !status ) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in SPG_RTF_Max_Spd", fur_cfg[RTH + n_rtf].fur_name.c_str());

         // Limit transcient maximum speed with calculated maximum speed
         spd = min (spd, spd_max );

         rtf_first->spd = spd;

			/* Tube temperature calculation for the max temperature of the 1st strip with offset */
			status = SPG_RTF_Tube_Tmp (rtf_first, roll, aimed_tmp_rtf, &tmp_ref_tub, tmp_tub, 
                                    data_second->thi, data_second->wid);
		   if ( !status ) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in SPG_RTF_Tube_Tmp", fur_cfg[RTH + n_rtf].fur_name.c_str());

         RTF_power (rtf_first, power); 

         rtf[n_rtf] = *rtf_second;
         rtf[n_rtf].spd = spd;
         roll = true;

         // Roll temperatures initialization
         SPG_RTF_ini_rol_tmp (rtf_first, &rtf[n_rtf], strip, data_first->stp_rtf, data_trans);

         // Calculation of the thick strip temperature taking into account the roll exchange
         status = RTF_strip (&rtf[n_rtf], roll, tmp_tub, &tmp_stp);
         if ( !status ) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in RTF_strip", fur_cfg[RTH + n_rtf].fur_name.c_str());

			/* Strip temperature increase */
			delta_t = delta_t + 1.f;

         loop ++;
		} while ( tmp_stp < data_second->tmp_low && loop < MAX_ITER && delta_t < 12.f && spd > data_first->spd*0.75f); 

      // Loop alarm treatment
      if (loop >= MAX_ITER)
      {
         status = false;
         ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Convergence error", fur_cfg[RTH + n_rtf].fur_name.c_str());
      }

      // Writing the transition set-points after convergence
      data_trans->tmp_stp_rtf   = aimed_tmp_rtf;
      data_trans->tmp_stp_rtf2  = tmp_stp;
      data_trans->spd           = spd;
      data_trans->tmp_tub_ref   = tmp_ref_tub;

      // Tubes temperatures.
      for ( int i_zon=0; i_zon < rtf_first->nb_zon; i_zon++ )
      {
         data_trans->tmp_tub[i_zon]     = tmp_tub[i_zon];
         data_trans->pow_pct_rtf[i_zon] = power[i_zon];
      }

      // For all elements
      for ( int i_elt=0; i_elt<rtf_first->nb_elt; i_elt++ )
         data_trans->stp_rtf[i_elt] = rtf_first->s_elt[i_elt].s_strip.tmp_out_strip;
	} /* Else (the calculated strip temperature is lower than the thick strip lower tolerance) */

   return status;
}

/*============================================================================
/ SPG_RTF_HotThick_ColdThin: Hot thick to cold thin transition.
/ 
/ Revisions:
/  Created:       2004          P.Dubois
/  Modified: 
/===========================================================================*/
bool SPG_RTF_HotThick_ColdThin( struct rtf_struct *rtf_first, struct rtf_struct *rtf_second, 
                                struct strip_trans_entry *data_first, struct strip_trans_entry *data_second, 
                                struct strip_trans_exit *data_trans, int i1, int i2 )
{   
   bool status = true;
   string sub_name = "SPG_RTF_HotThick_ColdThin";

   float tmp_ref_tub=0.f, tmp_stp=0.f, tmp_stp_rtf2;
   float spd, spd_max, tmp_tub[NB_ZON_RTH];
   bool roll, strip;
   float power[NB_ZON_RTH];
   int loop;

   int n_rtf = rtf_first->n_rtf;

   roll = false;
   strip = true;

	/* First strip transcient maximum speed calculation */
   rtf[n_rtf] = *rtf_first;
   status = SPG_RTF_Max_Spd( false, &rtf[n_rtf], data_first->tmp_low, data_first->tmp_in, &spd_max);

   // Set maximum speed
   spd = max(data_first->spd, data_second->spd);

   // Limit transcient maximum speed with calculated maximum speed
   spd = min (spd, spd_max );

   // Limit transcient maximum speed with calculated maximum speed
   rtf_first->spd = spd;
   rtf_second->spd = spd;

	/* Calculation of the 2nd strip temperature with the tube temperatures of the 1st one */
	/* ================================================================================== */
	status = SPG_RTF_Tube_Tmp (rtf_first, roll, data_first->tmp_low, &tmp_ref_tub, tmp_tub, 
                              data_second->thi, data_second->wid);
   if ( !status ) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in SPG_RTF_Tube_Tmp", fur_cfg[RTH + n_rtf].fur_name.c_str());

   RTF_power (rtf_first, power);

   // Set second coil data
   rtf[n_rtf] = *rtf_second;
   rtf[n_rtf].spd = spd;
   roll = true;

   // Roll temperatures initialization
   SPG_RTF_ini_rol_tmp (rtf_first, &rtf[n_rtf], strip, data_first->stp_rtf, data_trans);

   // Calculation of the 2nd strip temperature taking into account the roll exchange
   status = RTF_strip (&rtf[n_rtf], roll, tmp_tub, &tmp_stp);
   if ( !status ) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in RTF_strip", fur_cfg[RTH + n_rtf].fur_name.c_str());

   // If the calculated strip temperature for the cold product is less than the thick strip upper tolerance
   if ( tmp_stp <= data_second->tmp_upp )
   {
      // If the calculated strip temperature for the cold product is greater than the thick strip lower tolerance
      if (tmp_stp >= data_second->tmp_low)
      {
         // The transition is thick to thin
         status = SPG_RTF_Thick_Thin (rtf_first, rtf_second, data_first, data_second, data_trans, i1, i2);
         if ( !status )
         {
            ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in SPG_RTF_Thick_Thin", fur_cfg[RTH + n_rtf].fur_name.c_str());
            return status;
         }
      }
      // Else (The calculated strip temperature for the cold product is less than the thick strip lower tolerance)
      else
      {
         // The transition is thin to thick.
         status = SPG_RTF_Thin_Thick (rtf_first, rtf_second, data_first, data_second, data_trans, i1, i2);
         if ( !status )
         {
            ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in SPG_RTF_Thin_Thick", fur_cfg[RTH + n_rtf].fur_name.c_str());
            return status;
         }
      } // Else (The calculated strip temperature for the cold product is less than the thick strip lower tolerance
   } // If the calculated strip temperature for the cold product is less than the thick strip upper tolerance

   // Else calculation of the tube temperature to be close to the lower temperature of the thick product
   else
   {
      // Writing the transition set-points after convergence
      loop = 0;
      do
      {
         // decrease the speed
         spd = spd - 5.f/60.f;
         roll = false;
         strip = true;

			// Set first speed for transition
			rtf_first->spd = spd;

			/* Tube temperatures calculation */
			status = SPG_RTF_Tube_Tmp (rtf_first, roll, data_first->tmp_low, &tmp_ref_tub, tmp_tub, 
                                    data_second->thi, data_second->wid);
			if ( !status ) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in SPG_RTF_Tube_Tmp", fur_cfg[RTH + n_rtf].fur_name.c_str());

         RTF_power (rtf_first, power);

         // Set second coil data
         rtf[n_rtf] = *rtf_second;
			rtf[n_rtf].spd = spd;
			roll = true;

         // Roll temperatures initialization
         SPG_RTF_ini_rol_tmp (rtf_first, &rtf[n_rtf], strip, data_first->stp_rtf, data_trans);

         // Calculation of the thick strip temperature taking into account the roll exchange
         status = RTF_strip (&rtf[n_rtf], roll, tmp_tub, &tmp_stp_rtf2);
         if ( !status ) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in RTF_strip", fur_cfg[RTH + n_rtf].fur_name.c_str());

         loop ++;
      } while ( tmp_stp_rtf2 > data_second->tmp_upp && loop < MAX_ITER && 
	    	       (spd > data_first->spd * 0.8f || spd > data_second->spd ) && spd > data_first->spd*0.75f);//MIN_SPEED);
      // while the calculated strip exit temperature of the second coil is higher than the upper tolerance

      // Loop alarm treatment
      if ( loop >= MAX_ITER )
      {
         status = false;
         ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Convergence error", fur_cfg[RTH + n_rtf].fur_name.c_str());
      }

      // Store transient situation set points
      data_trans->tmp_stp_rtf  = data_first->tmp_low;
      data_trans->tmp_stp_rtf2 = tmp_stp;
      data_trans->spd          = spd;
      data_trans->tmp_tub_ref  = tmp_ref_tub;

      // for all RTF zones
      for ( int i_zon=0; i_zon < rtf_first->nb_zon; i_zon++ )
      {
         data_trans->tmp_tub[i_zon] = tmp_tub[i_zon];
         data_trans->pow_pct_rtf[i_zon] = power[i_zon];
      }

      // For all elements
      for ( int i_elt=0; i_elt<rtf_first->nb_elt; i_elt++ )
         data_trans->stp_rtf[i_elt] = rtf_first->s_elt[i_elt].s_strip.tmp_out_strip;
	} /* Else calculation of the tube temperature to be close to the lower temperature of the thick product */

   return status;
}

/*============================================================================
/ SPG_RTF_HotThin_ColdThick: "Hot thin" to "cold thick" transition.
/ 
/ Revisions:
/  Created:       2004          P.Dubois
/  Modified: 
/===========================================================================*/
bool SPG_RTF_HotThin_ColdThick( struct rtf_struct *rtf_first, struct rtf_struct *rtf_second, 
                                struct strip_trans_entry *data_first, struct strip_trans_entry *data_second, 
                                struct strip_trans_exit *data_trans, int i1, int i2 )
{   
   bool status = true;
   string sub_name = "SPG_RTF_HotThin_ColdThick";

   float tmp_ref_tub=0.f, tmp_stp=0.f,tmp_stp_rtf2;
   float spd, tmp_tub[MAX_NB_ECZ], spd_max;
   bool roll, strip;
   float power[NB_ZON_RTH];
   int loop;

   int n_rtf = rtf_first->n_rtf;

   roll = false;
   strip = true;

	/* First strip transcient maximum speed calculation */
   rtf[n_rtf] = *rtf_first;
   status = SPG_RTF_Max_Spd( false, &rtf[n_rtf], data_first->tmp_low, data_first->tmp_in, &spd_max);

   // Set maximum speed
   spd = max(data_first->spd, data_second->spd);

   // Limit transcient maximum speed with calculated maximum speed
   spd = min (spd, spd_max );

   // Limit transcient maximum speed with calculated maximum speed
   rtf_first->spd = spd;
   rtf_second->spd = spd;

	/* Calculation of the 2nd strip temperature with the tube temperature of the 1st. */
	/* ============================================================================== */
	status = SPG_RTF_Tube_Tmp ( rtf_first, roll, data_first->tmp_low, &tmp_ref_tub, tmp_tub, 
                               data_second->thi, data_second->wid);
   if ( !status ) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in SPG_RTF_Tube_Tmp", fur_cfg[RTH + n_rtf].fur_name.c_str());

   RTF_power (rtf_first, power);

   // Set second coil data
   rtf[n_rtf] = *rtf_second;
   rtf[n_rtf].spd = spd;
   roll = true;

   // Roll temperatures initialization.
   SPG_RTF_ini_rol_tmp (rtf_first, &rtf[n_rtf], strip, data_first->stp_rtf, data_trans);

   // Calculation of the 2nd strip temperature taking into account the roll exchange.
   status = RTF_strip (&rtf[n_rtf], roll, tmp_tub, &tmp_stp);
   if ( !status ) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in RTF_strip", fur_cfg[RTH + n_rtf].fur_name.c_str());

   // If the calculated strip temperature is less than the thick strip upper tolerance.
   if ( tmp_stp <= data_second->tmp_upp )
   {
      // If the calculated strip temperature is greater than the thick strip lower tolerance.
      if (tmp_stp >= data_second->tmp_low)
      {
         // The transition is thick to thin.
         status = SPG_RTF_Thick_Thin (rtf_first, rtf_second, data_first, data_second, data_trans, i1, i2);
         if ( !status )
         {
            ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in SPG_RTF_Thick_Thin", fur_cfg[RTH + n_rtf].fur_name.c_str());
            return status;
         }
      }
      // if the calculated strip temperature is lower than the lower tolerance
      else
      {
         // The transition is thin to thick.
         status = SPG_RTF_Thin_Thick (rtf_first, rtf_second, data_first, data_second, data_trans, i1, i2);
         if ( !status )
         {
            ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in SPG_RTF_Thin_Thick", fur_cfg[RTH + n_rtf].fur_name.c_str());
            return status;
         }
      }
   } // If the calculated strip temperature is less than the thick strip upper tolerance.

   // Else (the calculated strip temperature is greater than the thick strip upper tolerance).
   else
   {
      loop = 0;
      do
      {
         // Decrease the speed
         spd = spd - 5.f/60.f;
         roll = false;
         strip = true;

			// Set first speed for transition
			rtf_first->spd = spd;

			/* Tube temperatures calculation. */
			status = SPG_RTF_Tube_Tmp (rtf_first, roll, data_first->tmp_low, &tmp_ref_tub, tmp_tub, 
                                    data_second->thi, data_second->wid);
		   if ( !status ) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in SPG_RTF_Tube_Tmp", fur_cfg[RTH + n_rtf].fur_name.c_str());

		   RTF_power (rtf_first, power);

         // Set second coil data
         rtf[n_rtf] = *rtf_second;
			rtf[n_rtf].spd = spd;
			roll = true;

         // Roll temperatures initialization.
         SPG_RTF_ini_rol_tmp (rtf_first, &rtf[n_rtf], strip, data_first->stp_rtf, data_trans);

         // Calculation of the thick strip temperature taking into account the roll exchange.
         status = RTF_strip (&rtf[n_rtf], roll, tmp_tub, &tmp_stp_rtf2);
         if ( !status ) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in RTF_strip", fur_cfg[RTH + n_rtf].fur_name.c_str());

         loop ++;
      } while ( tmp_stp_rtf2 > data_second->tmp_upp && loop < MAX_ITER && 
	    	       (spd > data_first->spd * 0.8f || spd > data_second->spd ) && spd > MIN_SPEED);
      // while the calculated strip exit temperature of the second coil is higher than the upper tolerance

      // Loop alarm treatment.
      if ( loop >= MAX_ITER )
      {
         status = false;
         ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Convergence error", fur_cfg[RTH + n_rtf].fur_name.c_str());
      }

      // Writing the transition set-points after convergence
      data_trans->tmp_stp_rtf  = data_first->tmp_low;
      data_trans->tmp_stp_rtf2 = tmp_stp;
      data_trans->spd          = spd;
      data_trans->tmp_tub_ref  = tmp_ref_tub;

      // for all RTF zones
      for ( int i_zon=0; i_zon < rtf_first->nb_zon; i_zon++ )
      {
         data_trans->tmp_tub[i_zon] = tmp_tub[i_zon];
         data_trans->pow_pct_rtf[i_zon] = power[i_zon];
      }

      // For all elements
      for ( int i_elt=0; i_elt<rtf_first->nb_ecz; i_elt++ )
         data_trans->stp_rtf[i_elt] = rtf_first->s_elt[i_elt].s_strip.tmp_out_strip;
   }

   return status;
}

/*============================================================================
/ SPG_RTF_Thick_Thin: Thick to thin transition.
/ 
/ Revisions:
/  Created:       2004          P.Dubois
/  Modified: 
/===========================================================================*/
bool SPG_RTF_Thick_Thin( struct rtf_struct *rtf_first, struct rtf_struct *rtf_second, 
                         struct strip_trans_entry *data_first, struct strip_trans_entry *data_second, 
                         struct strip_trans_exit *data_trans, int i1, int i2 )
{   
   bool status = true;
   string sub_name = "SPG_RTF_Thick_Thin";

   float tmp_ref_tub=0.f, tmp_stp=0.f, spd, spd1, spd2, s_t1, tmp_stp_rtf2=0.f;
   float tmp_tub[MAX_NB_ECZ], delta_spd, delta_t, spd_max;
   bool roll, strip;
	float power[NB_ZON_RTH], aimed_tmp;
   int loop = 0;

   int n_rtf = rtf_first->n_rtf;

	// Maximum speed calculation for the first (thick) strip targeting its lower temperature tolerance
   // -------------------------------------------------------------------------------------------------
   // Set first coil data 
   rtf[n_rtf] = *rtf_first;
   // Set Anticipation time target temperature
   aimed_tmp = data_first->tmp_low + 2;

   status = SPG_RTF_Max_Spd( false, &rtf[n_rtf], aimed_tmp, data_first->tmp_in, &spd_max);

   // Set maximum speed
   spd = max(data_first->spd, data_second->spd);

   // Limit transcient maximum speed with calculated maximum speed
   spd = min(spd, spd_max);

   // Tubes temperature calculation targeting first (thick) strip lower temperature for the second (thin) strip
   // -----------------------------------------------------------------------------------------------------------
   roll = false;
   strip = true;
   
   // Set speed set point
   rtf_first->spd = spd;
   rtf_second->spd = spd;

   // Radiant tubes temperature step calculation
	status = SPG_RTF_Tube_Tmp(rtf_first, roll, aimed_tmp, &tmp_ref_tub, tmp_tub,                    // test
                             data_second->thi, data_second->wid);
   if ( !status ) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in SPG_RTF_Tube_Tmp", fur_cfg[RTH + n_rtf].fur_name.c_str());

   // Anticipation time zone power calculation
   RTF_power (rtf_first, power);

   // Second (thin) strip temperature calculation with tubes temperature calculated for the lower
   // first (thick) strip temperature, while taking into acount the exchanges between strip and roll
   // ----------------------------------------------------------------------------------------------
   // Set second coil data 
   rtf[n_rtf] = *rtf_second;
   roll = true;

   // Roll temperatures initialization
   SPG_RTF_ini_rol_tmp (rtf_first, &rtf[n_rtf], strip, data_first->stp_rtf, data_trans);

   // Calculation of the second (thin) strip temperature taking into account the heat exchange between strip and roll
   status = RTF_strip (&rtf[n_rtf], roll, tmp_tub, &tmp_stp);
   if ( !status ) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in RTF_strip", fur_cfg[RTH + n_rtf].fur_name.c_str());

   // If the calculated thin strip temperature is lower than its upper tolerance, we take for the tube
   //     temperature the maximum between the calculated one and the tube temperature of the steady state
   //     of the thin product
   // ======================================================================================================
   if ( tmp_stp <= data_second->tmp_upp || !sps_entry[n_rtf].s_mea.spd_auto )
   {
		// If the tube referance temperature of the thin (second) strip is LOWER than
      // the tube temperature had been calculated for the thick (first) strip
  	  if ( data_second->tmp_tub_ref <= tmp_ref_tub && tmp_stp >= data_second->tmp_tgt )
      {
         // Store process data for anticipation time
         data_trans->tmp_stp_rtf  = aimed_tmp;                                                     // test
         data_trans->tmp_stp_rtf2 = tmp_stp;
         data_trans->spd          = spd;
         data_trans->tmp_tub_ref  = tmp_ref_tub;
  
         // Tube temperatures
         for ( int i_zon=0; i_zon < rtf_first->nb_zon; i_zon++ )
         {
            data_trans->tmp_tub[i_zon] = tmp_tub[i_zon];
            data_trans->pow_pct_rtf[i_zon] = power[i_zon];
         }
  
         // For all elements
         for ( int i_elt=0; i_elt<rtf_first->nb_elt; i_elt++ )
         {
            data_trans->stp_rtf[i_elt] = rtf_first->s_elt[i_elt].s_strip.tmp_out_strip;
         }
      }
  
      // Else calculation of the thick strip temperature with the tube temperature of the thin product
      else
      {
         // Initialise the temperature offset to 1
         delta_t = 2.f;
         loop = 0;

         // Initialisation by the thick strip min temperature, then increase it.
         // While: 
         //      - The calculated thin strip temperature is lower than its upp limit
         //      - The calculated tube temperature is lower than the reference tube temperature for the 
         //          thin strip. 
         //      - The current thick strip temperature is lower than its upp limit
         do
         {
	         // Maximum speed calculation for the first (thick) strip targeting the seeked temperature
            // ----------------------------------------------------------------------------------------
            // Set first coil data 
            rtf[n_rtf] = *rtf_first;
            // Set strip temperature target
            aimed_tmp = data_first->tmp_low + delta_t;

				status = SPG_RTF_Max_Spd(false, &rtf[n_rtf], aimed_tmp, data_first->tmp_in, &spd_max);

            // Set maximum speed
				spd = max(data_first->spd, data_second->spd);

            // Limit transcient maximum speed with calculated maximum speed
            spd = min (spd, spd_max );

            // Tubes temperature calculation targeting the seeked temperature for the second (thin) strip
            // --------------------------------------------------------------------------------------------
            roll = false;
            strip = true;

            // Set speed set point
            rtf_first->spd = spd;
            rtf_second->spd = spd;

            // Radiant tubes temperature step calculation
				status = SPG_RTF_Tube_Tmp (rtf_first, roll, aimed_tmp, &tmp_ref_tub, tmp_tub, 
                                       data_second->thi, data_second->wid);
		      if ( !status ) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in SPG_RTF_Tube_Tmp", fur_cfg[RTH + n_rtf].fur_name.c_str());

            // Anticipation time zone power calculation
	         RTF_power (rtf_first, power);

            // Second (thin) strip temperature calculation with tubes temperature calculated for
            // the seeked temperature, while taking into acount the exchanges between strip and roll
            // ---------------------------------------------------------------------------------------
            // Set second coil data
            rtf[n_rtf] = *rtf_second;
            roll = true;

            // Roll temperatures initialization
            SPG_RTF_ini_rol_tmp( rtf_first, &rtf[n_rtf], strip, data_first->stp_rtf, data_trans );

            // Calculation of the second (thin) strip temperature taking into account the heat exchange between strip and roll
            status = RTF_strip (&rtf[n_rtf], roll, tmp_tub, &tmp_stp);
            if ( !status ) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in RTF_strip", fur_cfg[RTH + n_rtf].fur_name.c_str());

            // new strip delta temperature 
				delta_t += max(fabs(data_second->tmp_tgt - tmp_stp) / 1.5f, 1.f);
	    	   if (data_first->tmp_low + delta_t >= data_first->tmp_upp - 7.f)
            {
               delta_t = data_first->tmp_upp - data_first->tmp_low - 7.f;    
            }
             
            loop ++;
			} while (
                   //(tmp_stp < data_second->tmp_tgt || tmp_stp < data_second->tmp_low)
                   (tmp_stp < data_second->tmp_tgt || tmp_stp < data_second->tmp_low || aimed_tmp < data_second->tmp_low && tmp_stp < data_second->tmp_upp)
                   && aimed_tmp < data_first->tmp_upp - 7.f
                   && loop < MAX_ITER );
         // while the calculated second (thin) strip temperature is lower than its target temeprature

         // Loop alarm treatment
         if (loop >= MAX_ITER)
         {
            status = false;
            ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Convergence error.", fur_cfg[RTH + n_rtf].fur_name.c_str());
         }

         // Store process data for anticipation time
			data_trans->tmp_stp_rtf  = aimed_tmp;
         data_trans->tmp_stp_rtf2 = tmp_stp;
         data_trans->spd          = spd;
         data_trans->tmp_tub_ref  = tmp_ref_tub;

         // Tube temperatures
         for ( int i_zon=0; i_zon < rtf_first->nb_zon; i_zon++ )
         {
            data_trans->tmp_tub[i_zon]     = tmp_tub[i_zon];
            data_trans->pow_pct_rtf[i_zon] = power[i_zon];
         }

         // For all elements
         for ( int i_elt=0; i_elt<rtf_first->nb_elt; i_elt++ )
         {
            data_trans->stp_rtf[i_elt] = rtf_first->s_elt[i_elt].s_strip.tmp_out_strip;
         }
      } // Else (the tube temperature of the thin product is greater than the calculated tube temperature)
   } // If the calculated thin (second) strip temperature is not too high (lower than its upper tolerance)

   // Else: the calculated thin (second) strip temperature is too high (greater than its upper tolerance)
   else
   {
      // Search a lower speed for the dichotomy calculation
      // ==================================================
      loop = 0;
      spd2 = spd;
      spd1 = spd;
      do
      {
         // decrease speed 
			spd2 = spd2 - 10.f/60.f;

         // do not decrease too much the speed
	    	if ( spd2 < data_first->spd *0.8f && spd2 < data_second->spd) 
			{
				spd2 = min(data_first->spd *0.8f,data_second->spd);   
				break;
			}
         roll = false;
         strip = true;

         // Tube temperatures calculation
	 	   rtf_first->spd = spd2;

         // radiant tubes temeprature step calculation
			status = SPG_RTF_Tube_Tmp(rtf_first, roll, data_first->tmp_low, &tmp_ref_tub, tmp_tub, 
                                   data_second->thi, data_second->wid);
		   if ( !status ) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in SPG_RTF_Tube_Tmp", fur_cfg[RTH + n_rtf].fur_name.c_str());

         // Anticipation time zone power calculation
		   RTF_power( rtf_first, power );

         // set second coil data
         rtf_second->spd = spd2;
         rtf[n_rtf] = *rtf_second;   
         roll = true;

         // Roll temperatures initialization
         SPG_RTF_ini_rol_tmp (rtf_first, &rtf[n_rtf], strip, data_first->stp_rtf, data_trans);

         // Calculation of the thick strip temperature taking into account the roll exchange
         status = RTF_strip (&rtf[n_rtf], roll, tmp_tub, &tmp_stp_rtf2);
         if ( !status ) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in RTF_strip", fur_cfg[RTH + n_rtf].fur_name.c_str());

         loop ++;
		} while ( tmp_stp_rtf2 > data_second->tmp_upp && loop < MAX_ITER );
      // while the second strip calculated temperature is higher than its upper tolerance

      // Loop alarm treatment
      if (loop >= MAX_ITER)
      {
         status = false;
         ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Convergence error.", fur_cfg[RTH + n_rtf].fur_name.c_str());
      }
         
      // Calculation of the speed by dichotomy
      loop = 0;

      // calculation of the anticpation period speed by dichotomy
      // ========================================================
      do
      {
         // set speed
         delta_spd = fabs (spd2-spd1) / 2.f;
         spd = spd1 - delta_spd;

         roll = false;
         strip = true;

         // Set anticipation time process speed set point
	 	   rtf_first->spd=spd;

         // radiant tubes temeprature step calculation
			status = SPG_RTF_Tube_Tmp (rtf_first, roll, data_first->tmp_low, &tmp_ref_tub, tmp_tub, 
                                    data_second->thi, data_second->wid);
		   if ( !status ) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in SPG_RTF_Tube_Tmp", fur_cfg[RTH + n_rtf].fur_name.c_str());

         // Anticipation time zone power calculation
		   RTF_power (rtf_first, power);

         // set second coil data
         rtf_second->spd = spd;
         rtf[n_rtf] = *rtf_second;   
         roll = true;

         // Roll temperatures initialization
         SPG_RTF_ini_rol_tmp ( rtf_first, &rtf[n_rtf], strip, data_first->stp_rtf, data_trans );

         // Strip temperature calculation
         status = RTF_strip (&rtf[n_rtf], roll, tmp_tub, &tmp_stp);
         if ( !status ) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in RTF_strip", fur_cfg[RTH + n_rtf].fur_name.c_str());

         // if the second coil strip temperature calculated is higher or equal to the upper tolerance
         if (tmp_stp >= data_second->tmp_upp)
         {
            s_t1 = tmp_stp;
            spd1 = spd;
         }

         else
         {
            tmp_stp_rtf2 = tmp_stp;
            spd2 = spd;
         }

         loop ++;
		} while (fabs(tmp_stp - data_second->tmp_upp) > 1.f && fabs(spd2 - spd1) > 0.005f && loop < MAX_ITER);
      // while the second coil strip temperature calculated is near from the upper tolerance

      // Loop alarm treatment
      if (loop >= MAX_ITER)
      {
         status = false;
         ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Convergence error.", fur_cfg[RTH + n_rtf].fur_name.c_str());
      }

      // Store anticipation period process values
      data_trans->tmp_stp_rtf  = data_first->tmp_low;
      data_trans->tmp_stp_rtf2 = tmp_stp;
      data_trans->spd          = spd;
      data_trans->tmp_tub_ref  = tmp_ref_tub;

      // Tube temperatures
      for ( int i_zon=0; i_zon < rtf_first->nb_zon; i_zon++ )
      {
         data_trans->tmp_tub[i_zon] = tmp_tub[i_zon];
         data_trans->pow_pct_rtf[i_zon] = power[i_zon];
      }

      // For all elements
      for ( int i_elt=0; i_elt<rtf_first->nb_elt; i_elt++ )
         data_trans->stp_rtf[i_elt] = rtf_first->s_elt[i_elt].s_strip.tmp_out_strip;
   } // Else: the calculated thin (second) strip temperature is too high (greater than its upper tolerance)

   return status;
}

/*============================================================================
/ SPG_RTF_Thin_Thick: Thin to thick transition.
/ 
/ Revisions:
/  Created:       2004          P.Dubois
/  Modified: 
/===========================================================================*/
bool SPG_RTF_Thin_Thick( struct rtf_struct *rtf_first, struct rtf_struct *rtf_second, 
                         struct strip_trans_entry *data_first, struct strip_trans_entry *data_second, 
                         struct strip_trans_exit *data_trans, int i1, int i2 )
{   
   bool status = true;
   string sub_name = "SPG_RTF_Thin_Thick";

   float tmp_ref_tub=0.f, tmp_stp=0.f, spd, spd1, spd2, s_t1, tmp_stp_rtf2=0.f;
   float tmp_tub[MAX_NB_ECZ], delta_spd, delta_t, aimed_tmp, spd_max;
   bool roll, strip;
   float power[NB_ZON_RTH];
   int loop = 0;

   int n_rtf = rtf_first->n_rtf;

	/* First strip transient maximum speed calculation */
   rtf[n_rtf] = *rtf_first;
   status = SPG_RTF_Max_Spd( false, &rtf[n_rtf], data_first->tmp_upp, data_first->tmp_in, &spd_max);

   // Set maximum speed
   spd = max(data_first->spd, data_second->spd);

   // Limit transient maximum speed with calculated maximum speed
   spd = min(spd, spd_max);

   // Calculation of thick strip temperature with the tube temperatures of the thin strip
   //====================================================================================
   roll = false;
   strip = true;

   // Set speed
   rtf_first->spd = spd;
   rtf_second->spd = spd;   

   // Set Anticipation time target temperature
   aimed_tmp = data_first->tmp_upp;

   // Radiant tubes temperature step calculation
	status = SPG_RTF_Tube_Tmp (rtf_first, roll, data_first->tmp_upp, &tmp_ref_tub, tmp_tub, 
                              data_second->thi, data_second->wid);
   if ( !status ) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in SPG_RTF_Tube_Tmp", fur_cfg[RTH + n_rtf].fur_name.c_str());

   // Anticipation time zone power calculation
   RTF_power (rtf_first, power);

   // Set Second coil data
   rtf[n_rtf] = *rtf_second;
   roll = true;

   // Calculation of thick strip temperature with the calculated tube temperature for the max thin 
   // strip temperature, while taking into acount the exchanges between strip and roll
   // --------------------------------------------------------------------------------------------
   // Roll temperatures initialization. 
   SPG_RTF_ini_rol_tmp ( rtf_first, &rtf[n_rtf], strip, data_first->stp_rtf, data_trans );

   // Calculation of the thick strip temperature taking into account the roll exchange
   status = RTF_strip ( &rtf[n_rtf], roll, tmp_tub, &tmp_stp );
   if ( !status ) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in RTF_strip", fur_cfg[RTH + n_rtf].fur_name.c_str());

   // If the calculated thick strip temperature is greater than its lower tolerance, we take for the tube
   //     temperature the minimum between the calculated one and the tube temperature of the steady state
   //     of the thick product
   //====================================================================================================
   if ( tmp_stp >= data_second->tmp_low || !sps_entry[n_rtf].s_mea.spd_auto )
   {
		/* If the tube temperature of the thick product is greater than the tube temperature for the upper */
		/* tolerance of the thin product, the set-points are calculated for the upper tolerance thin product */
		if ( data_second->tmp_tub_ref >= tmp_ref_tub && tmp_stp <= data_second->tmp_tgt)
      {
         // Writing the transition set-points after convergence
         data_trans->tmp_stp_rtf  = aimed_tmp;
         data_trans->tmp_stp_rtf2 = tmp_stp;
         data_trans->spd          = spd;
         data_trans->tmp_tub_ref  = tmp_ref_tub;

         // Tube temperatures
         for ( int i_zon=0; i_zon < rtf_first->nb_zon; i_zon++ )
         {
            data_trans->tmp_tub[i_zon] = tmp_tub[i_zon];
            data_trans->pow_pct_rtf[i_zon] = power[i_zon];
         }

         // For all elements
         for ( int i_elt=0; i_elt < rtf_first->nb_elt; i_elt++ )
            data_trans->stp_rtf[i_elt] = rtf_first->s_elt[i_elt].s_strip.tmp_out_strip;
      }

      // Else calculation of the thin strip temperature with the tube temperature of the thick product
      else
      {
         // Initialise the temperature offset to 1
         delta_t = 1.f;
         loop = 0;

         // Initialisation by the thin strip max temperature, then decrease it.
         // While: 
         //      - The calculated thick strip temperature is greater than its low limit
         //      - The calculated tube temperature is greater than the reference tube temperature for the 
         //          thick strip. 
         //      - The current thin strip temperature is greater than its low limit
         do
         {
            roll = false;
            strip = true;
            
            aimed_tmp = data_first->tmp_upp - delta_t;

				/* First strip transcient maximum speed calculation */
            rtf[n_rtf] = *rtf_first;
            status = SPG_RTF_Max_Spd(false, &rtf[n_rtf], aimed_tmp, data_first->tmp_in, &spd_max);
		      if ( !status ) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in SPG_RTF_Max_Spd", fur_cfg[RTH + n_rtf].fur_name.c_str());

            // Set maximum speed
				spd = max(data_first->spd, data_second->spd);

            // Limit transcient maximum speed with calculated maximum speed
            spd = min (spd, spd_max );

            // Set speed set point
            rtf_first->spd = spd;
            rtf_second->spd = spd;

            // Radiant tubes temperature step calculation
				status = SPG_RTF_Tube_Tmp (rtf_first, roll, aimed_tmp, &tmp_ref_tub, tmp_tub, 
                                       data_second->thi, data_second->wid);
		      if ( !status ) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in SPG_RTF_Tube_Tmp", fur_cfg[RTH + n_rtf].fur_name.c_str());

            RTF_power (rtf_first, power);

            // Set second coil data
            rtf[n_rtf] = *rtf_second;
            roll = true;

            // Roll temperatures initialization.
            SPG_RTF_ini_rol_tmp (rtf_first, &rtf[n_rtf], strip, data_first->stp_rtf, data_trans);

            // Calculation of the thick strip temperature taking into account the roll exchange
            status = RTF_strip (&rtf[n_rtf], roll, tmp_tub, &tmp_stp);
            if ( !status ) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in RTF_strip", fur_cfg[RTH + n_rtf].fur_name.c_str());

				/* Increase the offset */
				delta_t += max(fabs(tmp_stp - data_second->tmp_tgt) / 1.5f, 1.f);
	    	   if (data_first->tmp_upp - delta_t <= data_first->tmp_low + 7.f)
            {
               delta_t = data_first->tmp_upp - data_first->tmp_low - 7.f;    
            }

            loop ++;
			} while (
                   //(tmp_stp > data_second->tmp_tgt || tmp_stp > data_second->tmp_upp)
                   (tmp_stp > data_second->tmp_tgt || tmp_stp > data_second->tmp_upp || aimed_tmp > data_second->tmp_upp && tmp_stp > data_second->tmp_low)
                   && aimed_tmp > data_first->tmp_low + 7.f
                   && loop < MAX_ITER ); 

         // Loop alarm treatment
         if (loop >= MAX_ITER)
         {
            status = false;
            ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Convergence error.", fur_cfg[RTH + n_rtf].fur_name.c_str());
            return status;
         }

         // Writing the transition set-points after convergence
         data_trans->tmp_stp_rtf  = aimed_tmp;
         data_trans->tmp_stp_rtf2 = tmp_stp;
         data_trans->spd          = spd;   
         data_trans->tmp_tub_ref  = tmp_ref_tub;

         // Tube temperatures
         for ( int i_zon=0; i_zon < rtf_first->nb_zon; i_zon++ )
         {
            data_trans->tmp_tub[i_zon]     = tmp_tub[i_zon];
            data_trans->pow_pct_rtf[i_zon] = power[i_zon];
         }

         // For all elements
         for ( int i_elt=0; i_elt<rtf_first->nb_elt; i_elt++ )
         {
            data_trans->stp_rtf[i_elt] = rtf_first->s_elt[i_elt].s_strip.tmp_out_strip;
         }
      } // Else calculation of the thin strip temperature with the tube temperature of the thick product
   } // if the second coil strip temperature calculated is higher or equal to the lower tolerance

   // Else: The calculated thick strip temperature is lower than its lower tolerance, the speed is decreased
   //=======================================================================================================
   else
   {
      // Calculation with the minimum speed
      spd1 = spd;
      s_t1 = tmp_stp;
      spd2 = spd;
		roll = true;
      strip = true;
      loop = 0;

      // While the calculated thick strip temperature is lower than its low limit
      do 
      {
			// Speed decrease
			spd2 = spd2  - 10.f/60.f;
			if ( spd2 < data_first->spd * 0.5f && spd2 < data_second->spd ) 
			{
				spd2 = min( data_first->spd * 0.5f, data_second->spd);                 
				break;
			}
         roll = false;
         strip = true;

         // Set process speed set point
         rtf_first->spd = spd2;
	      rtf_second->spd = spd2;

         // Radiant tubes temperature step calculation
			status = SPG_RTF_Tube_Tmp (rtf_first, roll, data_first->tmp_upp, &tmp_ref_tub, tmp_tub, 
                                    data_second->thi, data_second->wid);
		   if ( !status ) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in SPG_RTF_Tube_Tmp", fur_cfg[RTH + n_rtf].fur_name.c_str());

         // Anticipation time zone power calculation
		   RTF_power (rtf_first, power); 

         // Set second coil data
         rtf[n_rtf] = *rtf_second;   
         roll = true;

         // Roll temperatures initialization
         SPG_RTF_ini_rol_tmp (rtf_first, &rtf[n_rtf], strip, data_first->stp_rtf, data_trans);

         // Calculation of the thick strip temperature taking into account the roll exchange
         status = RTF_strip ( &rtf[n_rtf], roll, tmp_tub, &tmp_stp_rtf2 );
         if ( !status ) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in RTF_strip", fur_cfg[RTH + n_rtf].fur_name.c_str());

         loop ++;
		} while ( tmp_stp_rtf2 < data_second->tmp_low && loop < MAX_ITER );
      // while the second coil strip temperature calculated is  lower than its lower tolerance

      // Loop alarm treatment
      if (loop >= MAX_ITER)
      {
         status = false;
         ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Convergence error.", fur_cfg[RTH + n_rtf].fur_name.c_str());
         return status;
      }

      // Calculation of the speed by dichotomy
      loop = 0;

      // calculation of the right process speed set point for the anticipation period
      do
      {
         delta_spd = fabs (spd2-spd1) / 2.f;
         spd = spd1 - delta_spd;
         roll = false;
         strip = true;

         // process speed set point
         rtf_first->spd=spd;
         rtf_second->spd=spd;

         // radiant tubes temperature step calculation
			status = SPG_RTF_Tube_Tmp (rtf_first, roll, data_first->tmp_upp, &tmp_ref_tub, tmp_tub, 
                                    data_second->thi, data_second->wid);
		   if ( !status ) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in SPG_RTF_Tube_Tmp", fur_cfg[RTH + n_rtf].fur_name.c_str());

         // Anticipation time zone power calculation
		   RTF_power (rtf_first, power); 

         // Set second coil data
         rtf[n_rtf] = *rtf_second;   
         roll = true;

         // Roll temperatures initialization
         SPG_RTF_ini_rol_tmp(rtf_first, &rtf[n_rtf], strip, data_first->stp_rtf, data_trans);

         // Calculation of the thick strip temperature taking into account the roll exchange
         status = RTF_strip (&rtf[n_rtf], roll, tmp_tub, &tmp_stp);
         if ( !status ) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in RTF_strip", fur_cfg[RTH + n_rtf].fur_name.c_str());

         // if the second coil strip temeprature calculation is lower or equal to the lower tolerance
         if (tmp_stp <= data_second->tmp_low)
         {
            s_t1 = tmp_stp;
            spd1 = spd;
         }
         else
         {
            tmp_stp_rtf2 = tmp_stp;
            spd2 = spd;
         }

         loop ++;
		} while ( fabs(tmp_stp - data_second->tmp_low) > 1.f && fabs(spd2 - spd1) > 0.005f && loop < MAX_ITER );
      // while the second coil strip temperature calculated is near from the lower tolerance

      // Loop alarm treatment
      if (loop >= MAX_ITER)
      {
         status = false;
         ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Convergence error.", fur_cfg[RTH + n_rtf].fur_name.c_str());
      }

      // Writing the transition set-points after convergence
      data_trans->tmp_stp_rtf  = aimed_tmp;
      data_trans->tmp_stp_rtf2 = tmp_stp;
      data_trans->spd          = spd;
      data_trans->tmp_tub_ref  = tmp_ref_tub;

      // Tube temperatures
      for ( int i_zon=0; i_zon < rtf_first->nb_zon; i_zon++ )
      {
         data_trans->tmp_tub[i_zon]     = tmp_tub[i_zon];
         data_trans->pow_pct_rtf[i_zon] = power[i_zon];
      }

      // For all elements
      for ( int i_elt=0; i_elt<rtf_first->nb_elt; i_elt++ )
         data_trans->stp_rtf[i_elt] = rtf_first->s_elt[i_elt].s_strip.tmp_out_strip;
   } // Else: The calculated thick strip temperature is lower than its lower tolerance, the speed is decreased

   return status;
}

/*============================================================================
/ SPG_RTF_Max_Spd: Compute strip maximum speed in the RTF according to the strip 
/     characteristics, the max tube temperature & the power of the zone.
/ 
/ Revisions:
/  Created:       2007-nov       J. NAVEIRA
/  Modified: 
//===========================================================================*/
bool SPG_RTF_Max_Spd( bool roll, struct rtf_struct *rtf, float tmp_tgt, 
                      float tmp_in, float *spd )
{
   bool status = true;
   string sub_name = "SPG_RTF_Max_Spd";

   bool enough_power, strip = false;
   float spd1, spd2, spd_cur, stp_tmp = 0.f, tmp_tub[MAX_NB_ZON], pow_rtf[MAX_NB_ZON];
   float avg_pow, tmp_tub_ref;
   int iter, iter2;

   float Max_avg_pow_rtf = 100.f;
   float temp_spd;

   int n_rtf = rtf->n_rtf;

   // Store the current speed
   // -----------------------
   spd_cur = rtf->spd; 

   // Initialization of roof temperature at the max roof temperature
   // --------------------------------------------------------------
   for ( int i_zon=0; i_zon < rtf->nb_zon; i_zon++ ) tmp_tub[i_zon] = TUB_MAX_TMP;

   // =============================================================================================
   //                                        Upper limit calculation
   // =============================================================================================
   // Speed initialization at maximum speed
   spd1 = MAX_SPEED;
   rtf->spd = spd1;

   // strip temperature calculation
   // -----------------------------
   status = RTF_strip( rtf, roll, tmp_tub, &stp_tmp );
   if ( !status ) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in RTF_strip", fur_cfg[RTH + n_rtf].fur_name.c_str());
   
   // if Maximum speed not allow to get strip target
   // ----------------------------------------------
   if (stp_tmp > tmp_tgt + 2)
   {
      status = false;
		ns_STD::cl_TRC::TRC_Write("WAR", "SPG", sub_name, "%s: Maximum speed is too low to calculate strip maximum speed", fur_cfg[RTH + n_rtf].fur_name.c_str());
   }

   // =============================================================================================
   //                                        lower limit calculation
   // =============================================================================================
   // Speed initialization at minimum speed
   spd2 = MIN_SPEED;
   rtf->spd = spd2;

   // strip temperature calculation
   // -----------------------------
   status = RTF_strip( rtf, roll, tmp_tub, &stp_tmp );
   if ( !status ) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in RTF_strip", fur_cfg[RTH + n_rtf].fur_name.c_str());
   
   // if Minimum speed not allow to get strip target
   // ----------------------------------------------
   if (stp_tmp < tmp_tgt - 2)
   {
      status = false;
		ns_STD::cl_TRC::TRC_Write("WAR", "SPG", sub_name, "%s: Minimum speed is too high to calculate strip maximum speed", fur_cfg[RTH + n_rtf].fur_name.c_str());
   }

   // =============================================================================================
   //                                   Maximum speed calculation by dichotomy
   // =============================================================================================
   iter = 0;
   do
   {
      // Speed calculation
      // -----------------
      *spd = (spd1 + spd2) / 2.f;
      rtf->spd = *spd;

      tmp_tub_ref = TUB_MAX_TMP;

      // Tube temperature calculation
      // ----------------------------
      SPG_RTF_ratio_tube( tmp_tub_ref, rtf->s_elt[rtf->first_strip_elt].s_strip.wid, 
		                    rtf->s_elt[rtf->first_strip_elt].s_strip.thi, tmp_tub, 
                          rtf->s_elt[rtf->first_strip_elt].s_strip.thi, 
		                    rtf->s_elt[rtf->first_strip_elt].s_strip.wid, rtf->nb_zon, rtf->n_rtf );

      // While we have enough power to heat
      // -----------------------------------
      iter2 = 0;
      do
      {
         // Tube temperature calculation
         // ----------------------------
         SPG_RTF_ratio_tube( tmp_tub_ref, rtf->s_elt[rtf->first_strip_elt].s_strip.wid, 
			                    rtf->s_elt[rtf->first_strip_elt].s_strip.thi, tmp_tub, 
                             rtf->s_elt[rtf->first_strip_elt].s_strip.thi, 
			                    rtf->s_elt[rtf->first_strip_elt].s_strip.wid, rtf->nb_zon, rtf->n_rtf );

         // strip temperature calculation
         // -----------------------------
         status = RTF_strip( rtf, roll, tmp_tub, &stp_tmp );
         if ( !status ) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in RTF_strip", fur_cfg[RTH + n_rtf].fur_name.c_str());

         // RTF power calculation
			//----------------------
         RTF_power( rtf, pow_rtf );

         enough_power = true;

         avg_pow = 0.f;
			for ( int i_zon=0; i_zon<rtf->nb_zon; i_zon++) avg_pow += pow_rtf[i_zon];
			avg_pow /= float(rtf->nb_zon);

			// If lack of power
			if (avg_pow > Max_avg_pow_rtf)
			{
				// Reduce tube temperature to get max power
				tmp_tub_ref -= (avg_pow - Max_avg_pow_rtf) / 2.f + 5.f;
            tmp_tub_ref =max(tmp_tub_ref, rtf_cfg[n_rtf].tub_min_ref);

				enough_power = false;
			}// If lack of power

         iter2 ++;
      } while ( !enough_power && iter2 < MAX_ITER );

      // Alarm treatment "Error over looping".
      if ( iter2 >= MAX_ITER ) 
      {
         status = false;
         ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Convergence error maximum speed calculation", fur_cfg[RTH + n_rtf].fur_name.c_str());
      }

      // if the calculated rtf exit temperature is higher than the target temperature
      if ( stp_tmp > tmp_tgt ) spd2 = *spd;
      else                     spd1 = *spd;

      iter ++;
   } while ( fabs( stp_tmp - tmp_tgt ) > 0.1f && fabs(spd2 - spd1) >0.0001F && iter < MAX_ITER );

   // Alarm treatment "Error over looping".
   if ( iter >= MAX_ITER ) 
   {
      status = false;
      ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Convergence error maximum speed calculation", fur_cfg[RTH + n_rtf].fur_name.c_str());
   }

   temp_spd = 1000.f;
   *spd = min(*spd, temp_spd);

   //*spd = *spd * 0.86f / rtf->s_elt[2].s_strip.wid;       // In case if the calc speed is too high !!!

	/* Restore the current speed */
	/* ------------------------- */
   rtf->spd = spd_cur;

   return status;
} // void

/*============================================================================
/ SPG_RTF_Tube_Tmp: Tube temperature calculation.
/ 
/ Revisions:
/  Created:       2004          P.Dubois
/  Modified: 
/===========================================================================*/
bool SPG_RTF_Tube_Tmp( struct rtf_struct *rtf, bool roll, float rtf_tgt, float *t_t_cal, 
                       float tmp_tub[], float thi2, float wid2 )
{
   bool status = true;
   string sub_name = "SPG_RTF_Tube_Tmp";

   float tube_min=300.f+TK;
   float tmp_tub2, tmp_strip1, tmp_stp_rtf2, tmp_stp, delta_t;

   int n_rtf = rtf->n_rtf;

   float wid = rtf->s_elt[rtf->first_strip_elt].s_strip.wid;
   float thickness = rtf->s_elt[rtf->first_strip_elt].s_strip.thi;
   float tmp_tub1 = TUB_MAX_TMP; // initialization of reference tube temperature (avg temperature in the whole RTF)

   // Calculation of the strip temperature with the tube temperature reference equal to the max tube temperature
   // ==========================================================================================================
   // Calculation of the tube temperature ratio according to the strip width and thickness
   SPG_RTF_ratio_tube( tmp_tub1, wid, thickness, tmp_tub, thi2, wid2, rtf->nb_zon, rtf->n_rtf );

   // Strip temperature calculation
   status = RTF_strip( rtf, roll, tmp_tub, &tmp_strip1 );
   if ( !status ) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in RTF_strip", fur_cfg[RTH + n_rtf].fur_name.c_str());

   // Calculation of the strip temperature with the tube temperature reference equal to the min tube temperature
   // ==========================================================================================================
   tmp_tub2 = rtf_cfg[n_rtf].tub_min_ref;

	/* Calculation of the tube temperature ratio according to the strip width & thickness */
	SPG_RTF_ratio_tube (tmp_tub2, wid, thickness, tmp_tub, thi2, wid2, rtf->nb_zon, rtf->n_rtf );

   // Strip temperature calculation
   status = RTF_strip (rtf, roll, tmp_tub, &tmp_stp_rtf2);
   if ( !status ) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in RTF_strip", fur_cfg[RTH + n_rtf].fur_name.c_str());

   int loop = 0;
   // Calculation of the temperature by dichotomy
   // ===========================================
   do
   {
      // Radiant tube reference temperature
      delta_t = fabs(tmp_tub2 - tmp_tub1) / 2.f;
      *t_t_cal = tmp_tub1 - delta_t;

		/* Calculation of the tube temperature ratio according to the strip width & thickness */
		SPG_RTF_ratio_tube (*t_t_cal, wid, thickness, tmp_tub, thi2, wid2, rtf->nb_zon, rtf->n_rtf );
      if ( !status ) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in SPG_RTF_ratio_tube", fur_cfg[RTH + n_rtf].fur_name.c_str());

      // Strip temperature calculation
      status = RTF_strip (rtf, roll, tmp_tub, &tmp_stp);
      if ( !status ) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in RTF_strip", fur_cfg[RTH + n_rtf].fur_name.c_str());

      // If the strip temperature calculated is higher than the target temperature
      if ( tmp_stp > rtf_tgt )
      {
         tmp_strip1 = tmp_stp;
         tmp_tub1 = *t_t_cal;
      }

      else
      {
         tmp_stp_rtf2 = tmp_stp;
         tmp_tub2 = *t_t_cal;
      }

      loop ++;

   } while ( fabs( tmp_stp - rtf_tgt ) > 0.001f && fabs(tmp_tub1 - tmp_tub2) > 0.01f && loop < MAX_ITER );
   // while the strip tmeperature calculated is near from the targt temeprature

	// Alarm treatment "Error over looping"
   if ( loop >= MAX_ITER ) 
   {
      status = false;
      ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Convergence error.", fur_cfg[RTH + n_rtf].fur_name.c_str());
   }

   return status;
} // void

/*============================================================================
/ SPG_RTF_ini_rol_tmp: Roll temperatures initialization.
/ 
/ Revisions:
/  Created:       2004          P.Dubois
/  Modified:      2009-Jul      J. NAVEIRA
/===========================================================================*/
void SPG_RTF_ini_rol_tmp( struct rtf_struct *rtf_entry, struct rtf_struct *rtf_exit, bool strip,
                          float stp_rtf[], struct strip_trans_exit *data_trans )
{
   int nb_rol = 0;

   // Initialization of the roll enthalpy difference between stable and transient states
   data_trans->DH_roll_zon = 0.f;

   // For all elements
   for ( int i_elt=0; i_elt < rtf_entry->nb_elt; i_elt++ )
   {
      // If the element is a roll one
		if ( rtf_exit->s_elt[i_elt].i_roll )
		{
         // if stationary calculation
			if ( strip ) rtf_exit->s_elt[i_elt].s_rol.r_t_b_c = rtf_entry->s_elt[i_elt].s_strip.tmp_mid_strip;
			else         rtf_exit->s_elt[i_elt].s_rol.r_t_b_c = rtf_entry->s_elt[i_elt].s_rol.r_t_a_c;

			if (i_elt <= rtf_entry->elt_pyro)
			{
				nb_rol ++;

				// Bisra number = 19 for rolls
				data_trans->DH_roll_zon += ns_STD::cl_BIS::BISRA_Enth(19-1, rtf_entry->s_elt[i_elt].s_strip.tmp_mid_strip) - 
					                        ns_STD::cl_BIS::BISRA_Enth(19-1, stp_rtf[i_elt]);
			}
		}
   } // For all elements

   // If a least one roll exists
	if (nb_rol != 0) data_trans->DH_roll_zon /= float(nb_rol);

   return;
} // void

/*============================================================================
/ SPG_RTF_ratio_tube: Calculation of the tube temperature ratio according to 
/     the strip width and the strip thickness.
/ 
/ Revisions:
/  Created:       2004          P.Dubois
/  Modified:      2007-Mar      L. Lâm Ngoc
/===========================================================================*/
void SPG_RTF_ratio_tube( float tmp_tub_ref, float wid, float thi, float tmp_tub[], 
                         float thi2, float wid2, int nb_zon, int n_rtf )
{
   float tub_max_wid, tub_min_wid;
   float tub_max_thi, tub_min_thi, tub_max, tub_min;

   float thi_avg, wid_avg, thi_use, wid_use;

   /* Search thickness and width to use */
   thi_avg = (rtf_cfg[n_rtf].max_thick + rtf_cfg[n_rtf].min_thick) / 2.f;
   wid_avg = (rtf_cfg[n_rtf].max_width + rtf_cfg[n_rtf].min_width) / 2.f;

   // Search the thickness to use the temperature step
   if      ( thi < thi_avg && thi2 < thi_avg ) thi_use = min(thi, thi2);
   else if ( thi > thi_avg && thi2 > thi_avg ) thi_use = max(thi, thi2);
   else thi_use = (thi + thi2) / 2.f;

   // Search the width to use for the temperature step
   if      ( wid < wid_avg && wid2 < wid_avg ) wid_use = min(wid, wid2);
   else if ( wid > wid_avg && wid2 > wid_avg ) wid_use = max(wid, wid2);
   else wid_use = (wid + wid2) / 2.f;

	/* If tube reference temperature is higher than tube min ref temperature */
	/* ===================================================================== */
   if ( tmp_tub_ref >= rtf_cfg[n_rtf].tub_min_ref )
   {
      // For all RTF zones
      for ( int i_zon=0; i_zon < nb_zon; i_zon++ )
      {
         // If the width used is higher than the max width (heat buckles)
			if (wid_use > rtf_cfg[n_rtf].max_width)
         {
            tub_max_wid = rtf_cfg[n_rtf].TUB_MAX_WID[i_zon];
            tub_min_wid = rtf_cfg[n_rtf].TUB_MIN_WID[i_zon];
         }

         else
         {
            // If the width used is lower than the minimum width (mistracking issues)
				if (wid_use < rtf_cfg[n_rtf].min_width)
            {
               tub_max_wid = rtf_cfg[n_rtf].TUB_MAX_NARROW[i_zon];
               tub_min_wid = rtf_cfg[n_rtf].TUB_MIN_NARROW[i_zon];
            }

            else
            {
               tub_max_wid = rtf_cfg[n_rtf].TUB_MAX_NARROW[i_zon] + 
                            (rtf_cfg[n_rtf].TUB_MAX_WID[i_zon] - rtf_cfg[n_rtf].TUB_MAX_NARROW[i_zon]) / 
					             (rtf_cfg[n_rtf].max_width - rtf_cfg[n_rtf].min_width) * 
                            (wid_use - rtf_cfg[n_rtf].min_width);
               tub_min_wid = rtf_cfg[n_rtf].TUB_MIN_NARROW[i_zon] + 
                            (rtf_cfg[n_rtf].TUB_MIN_WID[i_zon] - rtf_cfg[n_rtf].TUB_MIN_NARROW[i_zon]) / 
					             (rtf_cfg[n_rtf].max_width - rtf_cfg[n_rtf].min_width) * 
                            (wid_use - rtf_cfg[n_rtf].min_width);
            }
         } // If the width used is higher than the max width (heat buckles)
         
         // If the thickness used is higher than the maximum define thickness (mistracking issues)
			if (thi_use > rtf_cfg[n_rtf].max_thick)
         {
            tub_max_thi = rtf_cfg[n_rtf].TUB_MAX_THICK[i_zon];
            tub_min_thi = rtf_cfg[n_rtf].TUB_MIN_THICK[i_zon];         
         }

         else
         {
            // If the thickness use is lower than the minimum define thickness (heat buckles issues)
				if (thi_use < rtf_cfg[n_rtf].min_thick)
            {
               tub_max_thi = rtf_cfg[n_rtf].TUB_MAX_THIN[i_zon];
               tub_min_thi = rtf_cfg[n_rtf].TUB_MIN_THIN[i_zon];
            }

            else
            {
               tub_max_thi = rtf_cfg[n_rtf].TUB_MAX_THIN[i_zon] + 
                            (rtf_cfg[n_rtf].TUB_MAX_THICK[i_zon] - rtf_cfg[n_rtf].TUB_MAX_THIN[i_zon]) / 
					             (rtf_cfg[n_rtf].max_thick - rtf_cfg[n_rtf].min_thick) * 
                            (thi_use - rtf_cfg[n_rtf].min_thick);
               tub_min_thi = rtf_cfg[n_rtf].TUB_MIN_THIN[i_zon] + 
                            (rtf_cfg[n_rtf].TUB_MIN_THICK[i_zon] - rtf_cfg[n_rtf].TUB_MIN_THIN[i_zon]) / 
					             (rtf_cfg[n_rtf].max_thick - rtf_cfg[n_rtf].min_thick) * 
                            (thi_use - rtf_cfg[n_rtf].min_thick);
            }
         } // If the thickness used is higher than the maximum define thickness (mistracking issues)

         tub_max = min(tub_max_wid, tub_max_thi);
         tub_min = min(tub_min_wid, tub_min_thi);

         // Calulation of the tube temperature set point in the concerned zone
         tmp_tub[i_zon] = tub_min + (tub_max - tub_min) / 
                                    (rtf_cfg[n_rtf].tub_max_ref - rtf_cfg[n_rtf].tub_min_ref) * 
                                    (tmp_tub_ref - rtf_cfg[n_rtf].tub_min_ref); 

         // security on maximum Radiant Tubes temperature
         tmp_tub[i_zon] = min(tmp_tub[i_zon], TUB_MAX_TMP);
	  } // For all RTF zones
   } // If tube reference temperature is higher than tube min ref temperature

   // If tube reference temperature is <= tube min ref temperature
   // ============================================================
   // JAB:
   // It is a very idea to go in this part of the model
   // Indeed it can generate a non convergence if the model go in this condition and also the previous one
   // that why the TUB_MIN_REF is set equal to TUB_MIN_MIN in ktl_config.h
   // ============================================================
   else 
   {
      // For all RTF zones
      for ( int i_zon=0; i_zon < nb_zon; i_zon++ )
      {
         // If the width used is higher than the max width (heat buckles)
			if (wid_use > rtf_cfg[n_rtf].max_width)
         {
            tub_max_wid = rtf_cfg[n_rtf].TUB_MIN_WID[i_zon];
            tub_min_wid = rtf_cfg[n_rtf].tub_min_min;
         }

         else
         {
            // If the width used is lower than the minimum width (mistracking issues)
				if (wid_use < rtf_cfg[n_rtf].min_width)
            {
               tub_max_wid = rtf_cfg[n_rtf].TUB_MIN_NARROW[i_zon];
               tub_min_wid = rtf_cfg[n_rtf].tub_min_min;
            }

            else
            {
               tub_max_wid = rtf_cfg[n_rtf].TUB_MIN_NARROW[i_zon] + 
                            (rtf_cfg[n_rtf].TUB_MIN_WID[i_zon] - rtf_cfg[n_rtf].TUB_MIN_NARROW[i_zon]) / 
					             (rtf_cfg[n_rtf].max_width - rtf_cfg[n_rtf].min_width) * 
                            (wid_use - rtf_cfg[n_rtf].min_width);
               tub_min_wid = rtf_cfg[n_rtf].tub_min_min; 
            }
         }

         // If the thickness used is higher than the maximum define thickness (mistracking issues)
			if (thi_use > rtf_cfg[n_rtf].max_thick)
         {
            tub_max_thi = rtf_cfg[n_rtf].TUB_MIN_THIN[i_zon];
            tub_min_thi = rtf_cfg[n_rtf].tub_min_min;
         }

         else
         {
            // If the thickness use is lower than the minimum define thickness (heat buckles issues)
				if (thi_use < rtf_cfg[n_rtf].min_thick)
            {
               tub_max_thi = rtf_cfg[n_rtf].TUB_MIN_THICK[i_zon];
               tub_min_thi = rtf_cfg[n_rtf].tub_min_min;
            }

            else
            {
               tub_max_thi = rtf_cfg[n_rtf].TUB_MIN_THICK[i_zon] + 
                            (rtf_cfg[n_rtf].TUB_MIN_THIN[i_zon] - rtf_cfg[n_rtf].TUB_MIN_THICK[i_zon]) / 
					             (rtf_cfg[n_rtf].max_thick - rtf_cfg[n_rtf].min_thick) * 
                            (thi_use - rtf_cfg[n_rtf].min_thick);
               tub_min_thi = rtf_cfg[n_rtf].tub_min_min; 
            }
         } // If the thickness used is higher than the maximum define thickness (mistracking issues)

         tub_max = min(tub_max_wid, tub_max_thi);
         tub_min = min(tub_min_wid, tub_min_thi);

         // Calulation of the tube temperature set point in the concerned zone
         tmp_tub[i_zon] = tub_min + (tub_max - tub_min) / 
                                    (rtf_cfg[n_rtf].tub_min_ref - rtf_cfg[n_rtf].tub_min_min) * 
                                    (tmp_tub_ref - rtf_cfg[n_rtf].tub_min_min); 

         // security on maximum Radiant Tubes temperature
         tmp_tub[i_zon] = min(tmp_tub[i_zon], TUB_MAX_TMP);
		 } // For all RTF zones
   } // If tube reference temperature is <= tube min ref temperature

   return;
}
