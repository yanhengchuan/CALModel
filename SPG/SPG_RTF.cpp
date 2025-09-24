/*============================================================================
// SPG_RTF: Set-point generator (stable state).
/  
/ (c) CMI Industry
/===========================================================================*/
#include "stdafx.h"

/*============================================================================
// SPG_RTF: Set-point generator (stable state). MAIN
/ 
/ Revisions:
/  Created:       2004          P.Dubois
/  Modified: 
/===========================================================================*/
bool SPG_RTF( int i1, int i2, int n_rtf )
{   
   // int i1 : Index for 1st strip
   // int i2 : Index for 2nd strip
   bool status = true;
   string sub_name = "SPG_RTF";
   bool TRC = ns_STD::cl_TRC::TRC_Sub(sub_name);

   float t_t1[MAX_NB_ZON], t_t2[MAX_NB_ZON], power_rtf[MAX_NB_ZON];

   float Tmp_Stp_out_JET[NB_ZON_JET], Pow_JET[NB_ZON_JET], Cooling_rate[NB_ZON_JET];
   float thick1, width1, thick2, width2, spd_max = 0.f, spd_max_jet = 0.f;
   bool roll;

   float t_ref1 = 0.f;
   float t_ref2 = 0.f;
   float tmp_ref_zon1 = 0.f;
   float tmp_ref_zon2 = 0.f;
   float rtf_entry_tmp = 0;

   struct strip_trans_entry data_first;
   struct strip_trans_entry data_second;
   struct strip_trans_exit  data_trans;

   struct coil_data first_coil, second_coil;

   //- Stable state: Computation without roll effect
   //- ---------------------------------------------
   roll = false;

   //- ========
   //- 1st coil
   //- ========
   if (TRC) ("SPG", sub_name, "SPG STABLE 1ST COIL.");
 
   //- Initialization of the furnace with the first strip in steady state
   //- ------------------------------------------------------------------
   RTF_static_init (&rtf1[n_rtf], fur_cfg[n_rtf].nb_zon, rtf_cfg[n_rtf], 
                    s_coil.s_coil_data[i1].N_pdt, s_coil.s_coil_data[i1].N_cla, i1);

   rtf1[n_rtf].spd = s_coil.s_coil_data[i1].spd;
   thick1          = s_coil.s_coil_data[i1].thi;
   width1          = s_coil.s_coil_data[i1].wid;

   //- Calculation of the radiation factors in each element
   //- ----------------------------------------------------
   if (n_rtf == 0)
      rtf_entry_tmp = PRE_HEA_STP_TMP;
   else
      rtf_entry_tmp = rtf1[n_rtf-1].s_elt[rtf1[n_rtf-1].nb_elt-2].s_strip.tmp_out_strip;

   //RTF_dynamic_init( &rtf1[n_rtf], thick1, width1, s_coil.s_coil_data[i1].emi_rtf[n_rtf], 
   //                  fur_cfg[n_rtf].entry_tmp ); 
   RTF_dynamic_init( &rtf1[n_rtf], thick1, width1, s_coil.s_coil_data[i1].emi_rtf[n_rtf], 
                     rtf_entry_tmp );

   //- Compute roof (if necessary) and tube temperature for 1st strip
   //- --------------------------------------------------------------
   //- RTF max speed calculation
   status = SPG_RTF_Max_Spd(false, &rtf1[n_rtf], s_coil.s_coil_data[i1].tmp_tgt[n_rtf], 
                            s_coil.s_coil_data[i1].tmp_in[n_rtf], &spd_max);
   if (!status) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in maximum speed calcul", fur_cfg[RTH + n_rtf].fur_name.c_str());
   if (n_rtf==RTH && USE_JET_SPEED)
   {
      SPG_JET_Max_Spd( s_coil.s_coil_data[i1].tmp_tgt[n_rtf+1], &s_coil.s_coil_data[i1].tmp_tgt[JET],
                       s_coil.s_coil_data[i1].thi, s_coil.s_coil_data[i1].wid, s_coil.s_coil_data[i1].Cool_rate, &spd_max_jet );
      spd_max = min (spd_max, spd_max_jet);
   }
	//- Limit speed with max speed
   s_coil.s_coil_data[i1].spd_max_th[n_rtf] = spd_max;

   // Checking with all upstream furnaces
   for (int i_rtf = 0; i_rtf < n_rtf; i_rtf++)
      spd_max = min (spd_max, s_coil.s_coil_data[i1].spd_max_th[i_rtf]);
   
   spd_max = s_coil.s_coil_data[i1].spd_max_th[RTH];

	s_coil.s_coil_data[i1].spd = s_coil.s_coil_data[i1].spd_max;
   s_coil.s_coil_data[i1].spd = min(s_coil.s_coil_data[i1].spd, spd_max);

	if ( s_coil.s_coil_data[i1].spd_dec != 0.f ) 
        s_coil.s_coil_data[i1].spd = s_coil.s_coil_data[i1].spd_dec;

	s_coil.s_coil_data[i1].spd = max(s_coil.s_coil_data[i1].spd, s_coil.s_coil_data[i1].spd_min);
   rtf1[n_rtf].spd = s_coil.s_coil_data[i1].spd;

   //- Tube temperature calculation
	status = SPG_RTF_Tube_Tmp( &rtf1[n_rtf], roll, s_coil.s_coil_data[i1].tmp_tgt[n_rtf], &t_ref1, t_t1,
                              s_coil.s_coil_data[i1].thi, s_coil.s_coil_data[i1].wid );
   if (!status) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in SPG_RTF_Tube_Tmp (3)", fur_cfg[RTH + n_rtf].fur_name.c_str());

   //- Gas flows calculation for 1st strip
   //- -----------------------------------
   RTF_power( &rtf1[n_rtf], power_rtf );

   //- If JET
   //- ---------------------------------------------------------------------------------------
   bool max_spd = false;
   if ( NB_FUR_JET > 0 && USE_JET_SPEED )
   {
      int pre_fur = fur_cfg[JET].pre_fur;
      status = SPG_JET( s_coil.s_coil_data[i1].tmp_tgt[pre_fur], s_coil.s_coil_data[i1].Bisra, 
                        s_coil.s_coil_data[i1].spd, s_coil.s_coil_data[i1].tmp_tgt[JET], 
                        s_coil.s_coil_data[i1].wid, s_coil.s_coil_data[i1].thi, 
                        s_coil.s_coil_data[i1].Cool_rate, Tmp_Stp_out_JET, 
                        Pow_JET, Cooling_rate, max_spd );

      if (!status) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in SPG_JET", fur_cfg[RTH + n_rtf].fur_name.c_str());

      //- Store calculated set point
      for (int i_zon=0; i_zon < NB_ZON_JET; i_zon++ )
      {
         s_coil.s_coil_data[i1].pow_pct[JET][i_zon] = spg_exit_jet.jet_zon_pow[i_zon];
      }
   } // If JET
   // --------------------------------------------------------------------------------------

   s_coil.s_coil_data[i1].tmp_ref_tub = t_ref1; 
   // For all zones
   for ( int i_zon=0; i_zon < fur_cfg[n_rtf].nb_zon; i_zon++ )
   {
      s_coil.s_coil_data[i1].tmp_zon[n_rtf][i_zon] = t_t1[i_zon];
      s_coil.s_coil_data[i1].pow_pct[n_rtf][i_zon] = power_rtf[i_zon];
		s_coil.s_coil_data[i1].pow_pct[n_rtf][i_zon] = max(s_coil.s_coil_data[i1].pow_pct[n_rtf][i_zon], 0.f);
		s_coil.s_coil_data[i1].pow_pct[n_rtf][i_zon] = min(s_coil.s_coil_data[i1].pow_pct[n_rtf][i_zon], 100.f);
   } // For all zones

	// For all elements
	for ( int i_elt=0; i_elt < rtf_cfg[n_rtf].nb_elt; i_elt++ )
	{
		s_coil.s_coil_data[i1].stp_tmp[n_rtf][i_elt] = rtf1[n_rtf].s_elt[i_elt].s_strip.tmp_out_strip;
	}

   //- ========
   //- 2nd coil
   //- ========
   if (TRC) ns_STD::cl_TRC::TRC_Write("INF", "SPG", sub_name, "SPG STABLE 2ND COIL.");

   //- Static data initialization.
   //- ---------------------------
   RTF_static_init (&rtf2[n_rtf], fur_cfg[n_rtf].nb_zon, rtf_cfg[n_rtf], s_coil.s_coil_data[i2].N_pdt, 
                    s_coil.s_coil_data[i2].N_cla, i2);

   rtf2[n_rtf].spd = s_coil.s_coil_data[i2].spd;
   thick2          = s_coil.s_coil_data[i2].thi;
   width2          = s_coil.s_coil_data[i2].wid;

   //- Dynamic data initialization.
   //- ----------------------------
   if (n_rtf == 0)
      rtf_entry_tmp = PRE_HEA_STP_TMP;
   else
      rtf_entry_tmp = rtf2[n_rtf-1].s_elt[rtf2[n_rtf-1].nb_elt-2].s_strip.tmp_out_strip;

   //RTF_dynamic_init (&rtf2[n_rtf], thick2, width2, s_coil.s_coil_data[i2].emi_rtf[n_rtf], 
   //                  fur_cfg[n_rtf].entry_tmp);
   RTF_dynamic_init (&rtf2[n_rtf], thick2, width2, s_coil.s_coil_data[i2].emi_rtf[n_rtf], 
                     rtf_entry_tmp);

   //- RTF max sped calculation
   status = SPG_RTF_Max_Spd( false, &rtf2[n_rtf], s_coil.s_coil_data[i2].tmp_tgt[n_rtf], 
                             s_coil.s_coil_data[i2].tmp_in[n_rtf], &spd_max);
   if (!status) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in maximum speed calcul", fur_cfg[RTH + n_rtf].fur_name.c_str());
   
   if (n_rtf==RTH && USE_JET_SPEED)
   {
      SPG_JET_Max_Spd( s_coil.s_coil_data[i2].tmp_tgt[n_rtf+1], &s_coil.s_coil_data[i2].tmp_tgt[JET],
                       s_coil.s_coil_data[i2].thi, s_coil.s_coil_data[i2].wid, s_coil.s_coil_data[i2].Cool_rate, &spd_max_jet );
      spd_max = min (spd_max, spd_max_jet);
   }

	//- Limit speed with max speed
   s_coil.s_coil_data[i2].spd_max_th[n_rtf] = spd_max;

   // Checking with all upstream furnaces
   for (int i_rtf = 0; i_rtf < n_rtf; i_rtf++)
      spd_max = min (spd_max, s_coil.s_coil_data[i2].spd_max_th[i_rtf]);

   spd_max = s_coil.s_coil_data[i2].spd_max_th[RTH];

	s_coil.s_coil_data[i2].spd = s_coil.s_coil_data[i2].spd_max;
   s_coil.s_coil_data[i2].spd = min(s_coil.s_coil_data[i2].spd, spd_max);

	if ( s_coil.s_coil_data[i2].spd_dec != 0.f ) 
        s_coil.s_coil_data[i2].spd = s_coil.s_coil_data[i2].spd_dec;

	s_coil.s_coil_data[i2].spd = max(s_coil.s_coil_data[i2].spd, s_coil.s_coil_data[i2].spd_min);
   rtf2[n_rtf].spd = s_coil.s_coil_data[i2].spd;

   //- Tube temperature calculation
	status = SPG_RTF_Tube_Tmp (&rtf2[n_rtf], roll, s_coil.s_coil_data[i2].tmp_tgt[n_rtf], &t_ref2, t_t2,
                              s_coil.s_coil_data[i2].thi, s_coil.s_coil_data[i2].wid);
   if (!status) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in SPG_RTF_Tube_Tmp (7)", fur_cfg[RTH + n_rtf].fur_name.c_str());

   //- Gas flows calculation for 2nd strip
   //- -----------------------------------
   RTF_power(&rtf2[n_rtf], power_rtf);

   //- If JET
   //- -----------------------------------------------------------------------------------------
   if ( NB_FUR_JET > 0 && USE_JET_SPEED )
   {
      int pre_fur = fur_cfg[JET].pre_fur;
      status = SPG_JET( s_coil.s_coil_data[i2].tmp_tgt[pre_fur], s_coil.s_coil_data[i2].Bisra, 
                        s_coil.s_coil_data[i2].spd, s_coil.s_coil_data[i2].tmp_tgt[JET], 
                        s_coil.s_coil_data[i2].wid, s_coil.s_coil_data[i2].thi,
                        s_coil.s_coil_data[i2].Cool_rate, Tmp_Stp_out_JET, 
                        Pow_JET, Cooling_rate, max_spd );

      if (!status) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in SPG_JET", fur_cfg[JET].fur_name.c_str());

      //- Store calculated set point
      for (int i_zon=0; i_zon < NB_ZON_JET; i_zon++ )
      {
         s_coil.s_coil_data[i2].pow_pct[JET][i_zon] = spg_exit_jet.jet_zon_pow[i_zon];
      }
   } // If Jet cooling furnace
   //- -----------------------------------------------------------------------------------------

   //- Storage for SPS calculation
   //- ---------------------------
   s_coil.s_coil_data[i2].tmp_ref_tub = t_ref2;
   // For all zones
   for ( int i_zon=0; i_zon < fur_cfg[n_rtf].nb_zon; i_zon++ )
   {
      s_coil.s_coil_data[i2].tmp_zon[n_rtf][i_zon] = t_t2[i_zon];
      s_coil.s_coil_data[i2].pow_pct[n_rtf][i_zon] = power_rtf[i_zon];
		s_coil.s_coil_data[i2].pow_pct[n_rtf][i_zon] = max(s_coil.s_coil_data[i2].pow_pct[n_rtf][i_zon], 0.f);
		s_coil.s_coil_data[i2].pow_pct[n_rtf][i_zon] = min(s_coil.s_coil_data[i2].pow_pct[n_rtf][i_zon], 100.f);
   } // For all zones

	// For all elements
	for ( int i_elt=0; i_elt < rtf_cfg[n_rtf].nb_elt; i_elt++ )
	{
		s_coil.s_coil_data[i2].stp_tmp[n_rtf][i_elt] = rtf2[n_rtf].s_elt[i_elt].s_strip.tmp_out_strip;
	}
   
   //- =========
   //- Transient
   //- =========
   if (n_rtf == RTH || sps_entry[n_rtf].s_mea.nCurCoil == -1)
   {
      first_coil  = s_coil.s_coil_data[i1];
      second_coil = s_coil.s_coil_data[i2];
      /*if (TRC)*/ ns_STD::cl_TRC::TRC_Write("INF", "SPG", sub_name, "SPG TRANSIENT. - 1");
   }
   else if (i1 == 0)
   {
      first_coil  = s_coil.s_coil_data_last[0];
      second_coil = s_coil.s_coil_data[0];
      /*if (TRC)*/ ns_STD::cl_TRC::TRC_Write("INF", "SPG", sub_name, "SPG TRANSIENT. - 2");
   }
   else
   {
      first_coil  = s_coil.s_coil_data[i1-1];
      second_coil = s_coil.s_coil_data[i2-1];
      /*if (TRC)*/ ns_STD::cl_TRC::TRC_Write("INF", "SPG", sub_name, "SPG TRANSIENT. - 3");
   }

   //- 1st coil data
   //- =============
   data_first.tmp_in  = first_coil.tmp_in[n_rtf];
   data_first.wid     = first_coil.wid;
   data_first.tmp_tgt = first_coil.tmp_tgt[n_rtf];
   data_first.spd     = first_coil.spd;
   data_first.thi     = first_coil.thi;
   data_first.dummy   = first_coil.dummy;
   data_first.tmp_tub_ref = t_ref1;
	data_first.spd_min = first_coil.spd_min;

   // For all elements 
   for ( int i_elt=0; i_elt < rtf_cfg[n_rtf].nb_elt; i_elt++ )
   {
      data_first.stp_rtf[i_elt] = first_coil.stp_tmp[n_rtf][i_elt];
   }

   //- Tube temperatures
   //- -----------------
   // For all zones
   for ( int i_zon=0; i_zon < fur_cfg[n_rtf].nb_zon; i_zon++ ) 
   {
      data_first.tmp_tub[i_zon] = t_t1[i_zon];
      data_first.pow_pct_rtf[i_zon] = first_coil.pow_pct[n_rtf][i_zon];
   } // For all zones

   data_first.tmp_upp = first_coil.tmp_upp[n_rtf];
   data_first.tmp_low = first_coil.tmp_low[n_rtf];

   //- 2nd coil data 
   //- =============
   data_second.tmp_in  = second_coil.tmp_in[n_rtf];
   data_second.wid     = second_coil.wid;
   data_second.tmp_tgt = second_coil.tmp_tgt[n_rtf];
   data_second.spd     = second_coil.spd;
   data_second.thi     = second_coil.thi;
   data_second.dummy   = second_coil.dummy;
   data_second.tmp_tub_ref = t_ref2;
	data_second.spd_min = second_coil.spd_min;

   // For all elements 
   for ( int i_elt=0; i_elt < rtf_cfg[n_rtf].nb_elt; i_elt++ )
   {
      data_second.stp_rtf[i_elt] = second_coil.stp_tmp[n_rtf][i_elt];
   } // For all elements 

   //- Tube temperatures
   //- -----------------
   // For all zones
   for ( int i_zon=0; i_zon < fur_cfg[n_rtf].nb_zon; i_zon++ ) 
   {
      data_second.tmp_tub[i_zon] = t_t2[i_zon];
      data_second.pow_pct_rtf[i_zon] = second_coil.pow_pct[n_rtf][i_zon];
   } // For all zones

   data_second.tmp_upp = second_coil.tmp_upp[n_rtf];
   data_second.tmp_low = second_coil.tmp_low[n_rtf];

   //- ===========
   //- Roll model.
   //- ===========
   if (TRC) ns_STD::cl_TRC::TRC_Write("INF", "SPG", sub_name, "SPG ROLL CALCULATION.");

   //- If use roll calculation (horizontal or vertical furnace)
   if ( USE_ROLL_CAL )
   {
      // Update tube temperature above rolls
      // -----------------------------------
      int n_rol = 0;
      int num_elt_roll[MAX_NB_ROL];
      // For all elements
      for ( int i_elt=0; i_elt < rtf_cfg[n_rtf].nb_elt; i_elt++ )
      {
         if ( rtf1[n_rtf].s_elt[i_elt].i_roll )
         {
            num_elt_roll[n_rol] = i_elt;
            n_rol ++;
         }
      } // For all elements

      int lst_zon = fur_cfg[n_rtf].nb_zon - 1;
      // For all rolls
      for ( int i_rol=0; i_rol < rtf_cfg[n_rtf].nb_rol; i_rol++ )
      {
         // Strip width [m]
         rol_cmp[n_rtf].s_rol[i_rol].s_strip.wid = s_coil.s_coil_data[i1].wid;    

         int num_elt = num_elt_roll[i_rol]; 
         if ( num_elt < 0 ) num_elt = 0;

         // Strip temperature [K] sur le rouleau
         rol_cmp[n_rtf].s_rol[i_rol].s_strip.tmp = rtf1[n_rtf].s_elt[num_elt].s_strip.tmp_mid_strip; 
      } // For all rolls

      int n_rol_cmp = rol_cfg[n_rtf].n_rol_cmp;
      //- Tube temperature [K]
      rol_cmp[n_rtf].s_rol[n_rol_cmp].s_tub.tmp = s_coil.s_coil_data[i1].tmp_zon[n_rtf][lst_zon];

      //- Strip temperature [K]
      rol_cmp[n_rtf].s_rol[n_rol_cmp].s_strip.tmp = s_coil.s_coil_data[i1].tmp_tgt[n_rtf];

      status = SPG_ROL ( &rol_cmp[n_rtf] );
      if (!status) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in SPG_ROL (9)", fur_cfg[RTH + n_rtf].fur_name.c_str());
   } //- If use roll calculation (horizontal or vertical furnace)

   //- =========================================================
   //- Transition type identification and set-point calculation.
   //- =========================================================
   if (TRC) ns_STD::cl_TRC::TRC_Write("INF", "SPG", sub_name, "SPG CALCUL TRANSITOIRE.");
   

   if ( rtf_cfg[n_rtf].trans_calc)
   {   
      //- Specific initialization for transient calculation
      //- -------------------------------------------------
      SPG_Specific::SPG_init_TRS (&rtf1[n_rtf]);

      status = SPG_RTF_Transient( &rtf1[n_rtf], &rtf2[n_rtf], &data_first, &data_second, &data_trans, i1, i2 );
      if (!status) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in SPG_RTF_Transient", fur_cfg[RTH + n_rtf].fur_name.c_str());
   }

   //- If Jet cooling furnace --- Calculate transient data
   //- ------------------------------------------------------------------------------------------
   if ( NB_FUR_JET > 0 && USE_JET_SPEED )
   {
      // No transient state in jet cooling
      // apply set points of next steady state. these set points will need to be applied when the new weld enters the section
      
      int pre_fur = fur_cfg[JET].pre_fur;
      status = SPG_JET(s_coil.s_coil_data[i1].tmp_tgt[pre_fur], s_coil.s_coil_data[i1].Bisra, 
                        data_trans.spd, s_coil.s_coil_data[i1].tmp_tgt[JET], 
                        s_coil.s_coil_data[i1].wid, s_coil.s_coil_data[i1].thi,
                        s_coil.s_coil_data[i1].Cool_rate, Tmp_Stp_out_JET, 
                        Pow_JET, Cooling_rate,max_spd);

      if (!status) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in SPG_JET", fur_cfg[JET].fur_name.c_str());

      ////- Store calculated set point
      for (int i_zon=0; i_zon < NB_ZON_JET; i_zon++ )
      {
         data_trans.pow_pct_jet[i_zon] = s_coil.s_coil_data[i2].pow_pct[JET][i_zon];// spg_exit_jet.jet_zon_pow[i_zon];
      }
   } // If Jet cooling furnace
   //- ------------------------------------------------------------------------------------------

   //- Transient results storage for further use
   //- -----------------------------------------
   trans_result_rtf[n_rtf][i1] = data_trans;
   if (!rtf_cfg[n_rtf].trans_calc)
   {
      // Store anticipation period process values
      trans_result_rtf[n_rtf][i1].tmp_stp_rtf  = first_coil.tmp_tgt[n_rtf];
      trans_result_rtf[n_rtf][i1].tmp_stp_rtf2 = second_coil.tmp_tgt[n_rtf]; 
      trans_result_rtf[n_rtf][i1].spd          = first_coil.spd;
      trans_result_rtf[n_rtf][i1].tmp_tub_ref  = first_coil.tmp_ref_tub;

      // Tube temperatures
      for ( int i_zon=0; i_zon < rtf1[n_rtf].nb_zon; i_zon++ )
      {
			trans_result_rtf[n_rtf][i1].tmp_tub[i_zon]     = data_first.tmp_tub[i_zon];
			trans_result_rtf[n_rtf][i1].pow_pct_rtf[i_zon] = data_first.pow_pct_rtf[i_zon];
		}

      // For all elements
      for ( int i_elt=0; i_elt < rtf1[n_rtf].nb_elt; i_elt++ )
      {
         trans_result_rtf[n_rtf][i1].stp_rtf[i_elt] = rtf1[n_rtf].s_elt[i_elt].s_strip.tmp_out_strip;
      }
   }

   return status;
} // void

