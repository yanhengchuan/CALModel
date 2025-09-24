/*============================================================================
/ SIM_MAIN: Simulation.
/  
/ (c) CMI Industry
/===========================================================================*/
#include "stdafx.h"

CRITICAL_SECTION Sec_SIM_Proc0;		

/*============================================================================
/ SIM_MAIN: Simulation main program.
/ 
/ Revisions:
/  Created:       2007-Feb      JC.Mitais
/  Modified: 
/===========================================================================*/
bool SIM_Main()
{
   bool status = true;

	static bool once = true;
     
	if (once)
	{
		SQL2_I_MSG_COIL();
		once = false;
	}

   SIM_Mea_trk();
   if ( NB_FUR_RTF > 0 ) SIM_Mea_rtf();
   if ( NB_FUR_TNL > 0 ) SIM_Mea_tnl();
   if ( NB_FUR_SLW > 0 ) SIM_Mea_slw();
   if ( NB_FUR_JET > 0 ) SIM_Mea_jet();
   if ( NB_FUR_OVG > 0 ) SIM_Mea_ovg();
   if ( NB_FUR_FCS > 0 ) SIM_Mea_fcs();
	
	SQL2_I_MSG_MEA_HEA();
	SQL2_I_MSG_MEA_SPD();

   return status;
}

/*============================================================================
/ SIM_Mea_trk: Tracking simulation.
/ 
/ Revisions:
/  Created:       2016-Mar      JC.Mitais
/  Modified: 
/
/ In this version, 0 is the RTF entry, the "current" coil is the one under 
/     the RTF pyrometer
/===========================================================================*/
bool SIM_Mea_trk()
{
   bool status = true;

   time_t current_time;
   time(&current_time);

   static time_t previous_time;

   // Strip speed
	static bool once = true;
   if (once || sps_act_sp[RTH].spd <= 0.f )
   {
      //s_sim.s_line_trk.Spd_mea = min(MAX_SPEED, s_sim.s_coil_data[0].spd) - 20.f/60.f;    // test
      //s_sim.s_line_trk.Spd_mea = s_coil.s_coil_data[0].spd_max_th[0] - 20.f/60.f;
      s_sim.s_line_trk.Spd_mea = 30.f/60.f;
      s_sim.s_line_trk.Spd_ctrl = false;     // Speed control (ON/OFF)
      previous_time = current_time - 4;
      once = false;
   }

   else
   {
      s_sim.s_line_trk.Spd_mea = sps_act_sp[RTH].spd;
      //if ( s_sim.s_line_trk.Spd_mea < MIN_SPEED ) s_sim.s_line_trk.Spd_mea = s_coil.s_coil_data[0].spd;
      if ( s_sim.s_line_trk.Spd_mea < MIN_SPEED ) s_sim.s_line_trk.Spd_mea = MIN_SPEED;
      s_sim.s_line_trk.Spd_ctrl = true;     // Speed control (ON/OFF)
   }

   // incrementation of weld position
   s_sim.weld_pos += float(current_time - previous_time) * s_sim.s_line_trk.Spd_mea;

   // If the line is running
	if (s_sim.s_line_trk.Spd_mea > 0.f)
   {
      s_sim.s_line_trk.Spd_ctrl = true;
      s_sim.s_fur_mea[RTH].ctl = true;

      //// If the weld passes the RTF pyrometer
      //if ( s_sim.weld_pos >= fur_cfg[RTH].xPyro )
      //{
      //   s_sim.weld_pos = fur_cfg[RTH].xPyro - s_sim.s_coil_data[1].lgt;

      //   for (int i_coil=MAX_NB_COIL - 1; i_coil > 0; i_coil--)                           // test
      //   {
      //      s_coil.s_coil_data_last[i_coil] = s_coil.s_coil_data_last[i_coil - 1];
      //   }
      //   s_coil.s_coil_data_last[0] = s_coil.s_coil_data[0];

      //   // Permutation of coil queue
      //   coil_data tmp_coil = s_sim.s_coil_data[0];
      //   for (int i_coil=1; i_coil < s_sim.nb_coil; i_coil++) s_sim.s_coil_data[i_coil - 1] = s_sim.s_coil_data[i_coil];
      //   s_sim.s_coil_data[s_sim.nb_coil - 1] = tmp_coil;
      //   s_sim.s_line_trk.coil_id = s_sim.s_coil_data[0].id;
      //   //s_coil.s_coil_data_last[0] = s_coil.s_coil_data[0];

      //   // Update Coils Data
      //   SQL2_I_MSG_COIL();
      //} // The weld is after the pyrometer

      // The weld is after the pyrometer
      if ( s_sim.weld_pos < fur_cfg[RTH].xPyro )
         s_sim.s_line_trk.coil_id = s_sim.s_coil_data[0].id;
      else
         s_sim.s_line_trk.coil_id = s_sim.s_coil_data[1].id;

      // if whole length of the coil passed the reference point
      if ( s_sim.weld_pos >= s_sim.s_coil_data[0].lgt )
      {
         // reset weld position
         s_sim.weld_pos = 0;

         // Permutation of coil queue
         coil_data tmp_coil = s_sim.s_coil_data[0];
         for (int i_coil=1; i_coil < s_sim.nb_coil; i_coil++) s_sim.s_coil_data[i_coil - 1] = s_sim.s_coil_data[i_coil];
         s_sim.s_coil_data[s_sim.nb_coil - 1] = tmp_coil;

         // generate new ID
         static int idx = 1;
         s_sim.s_coil_data[s_sim.nb_coil - 1].id = "ZZ_" + to_string(idx);
         idx++;

         s_sim.s_coil_data[s_sim.nb_coil - 1].id = "";      // TEST

         //// Update Coils Data
         //SQL2_I_MSG_COIL();
      } // if whole length of the coil passed the reference point
   } // If the line is running

   previous_time = current_time;

   return status;
} // void

/*============================================================================
/ SIM_Mea_rtf: Measurements simulation.
/ 
/ Revisions:
/  Created:       2007-Feb      JC.Mitais
/  Modified: 
/===========================================================================*/
bool SIM_Mea_rtf()
{
   bool status = true;
   string sub_name = "SIM_Mea_rtf";

   float tmp_tub[MAX_NB_ZON];
   float tmp_stp;
   struct rtf_struct sim_rtf[NB_FUR_RTF];     // RTH strip and furnace structure
   bool roll = false;

	static bool once = true;

   s_sim.s_fur_mea[RTH].pre_hea_stp_tmp = PRE_HEA_STP_TMP - TK;
   //fur_cfg[RTH].entry_tmp = PRE_HEA_STP_TMP;                                         // test
   //fur_cfg[RTS].entry_tmp = s_coil.s_coil_data[0].tmp_tgt[RTH];                      // test

   for (int i_rtf=0; i_rtf < NB_FUR_RTF; i_rtf++) sim_rtf[i_rtf] = rtf[i_rtf];

   if (once || sps_act_sp[RTH].tmp_zon[0] < 10.f)
   {
      // For all RTF furnaces
      for (int i_rtf=0; i_rtf < NB_FUR_RTF; i_rtf++)
      {
         s_sim.s_fur_mea[i_rtf].ctl = false;    // heat control (ON/OFF)
	      s_sim.s_fur_mea[i_rtf].stp_tmp = s_sim.s_coil_data[0].tmp_tgt[i_rtf] - TK;

         // For all RTF zones
         for ( int i_zon=0; i_zon < fur_cfg[i_rtf].nb_zon; i_zon++ ) 
         {
            s_sim.s_fur_mea[i_rtf].zon_tmp[i_zon] = s_sim.s_coil_data[0].tmp_upp[i_rtf] + 230.f - TK;
            s_sim.s_fur_mea[i_rtf].heat_pow_pct[i_zon] = 35.f;

            sps_entry_old[i_rtf].s_mea.tmp_zon[i_zon] = s_sim.s_coil_data[0].tmp_upp[i_rtf] + 230.f;
        } // For all RTH zones

         once = false;
      } // For all RTF furnaces
   } // if (once || sps_act_sp[RTH].tmp_tub[0] < 10.f)

   else
   {
      sim_rtf[RTH].spd = s_sim.s_line_trk.Spd_mea;
      // For all RTF furnaces
      for (int i_rtf=0; i_rtf < NB_FUR_RTF; i_rtf++)
      {
         s_sim.s_fur_mea[i_rtf].ctl = true;    // RTF heat control (ON/OFF)

         // Fuel flows & powers in RTH zones
         // ================================
         // For all zones
         for ( int i_zon=0; i_zon < fur_cfg[i_rtf].nb_zon; i_zon++ ) 
         {
            s_sim.s_fur_mea[i_rtf].zon_tmp[i_zon] = 0.9f * sps_entry_old[i_rtf].s_mea.tmp_zon[i_zon] +
                                                    0.1f * sps_act_sp[i_rtf].tmp_zon[i_zon] - TK;
            //if (i_rtf == 0)
            //   s_sim.s_fur_mea[i_rtf].zon_tmp[i_zon] = sps_act_sp[i_rtf].tmp_zon[i_zon];                       // test // to set PID parameters

            tmp_tub[i_zon] = s_sim.s_fur_mea[i_rtf].zon_tmp[i_zon] + TK;

            s_sim.s_fur_mea[i_rtf].heat_pow_pct[i_zon] = 0.65f * sps_entry_old[i_rtf].s_mea.heat_pow_pct[i_zon] +
                                                         0.35f * sps_act_sp[i_rtf].heat_pow_pct[i_zon];
         } // For all zones

         // RTF structure initialization
         // ============================
         // Static data initialization.
         // ---------------------------
         RTF_static_init (&sim_rtf[i_rtf], fur_cfg[i_rtf].nb_zon, rtf_cfg[i_rtf], 
                          s_coil.s_coil_data[0].N_pdt, s_coil.s_coil_data[0].N_cla, 0);

         // Dynamic data initialization.
         // ----------------------------
         RTF_dynamic_init (&sim_rtf[i_rtf], s_coil.s_coil_data[0].thi, s_coil.s_coil_data[0].wid, 
                           s_coil.s_coil_data[0].emi_rtf[i_rtf], fur_cfg[i_rtf].entry_tmp);

         sim_rtf[i_rtf].s_elt[ sim_rtf[i_rtf].first_strip_elt ].s_strip.tmp_in_strip = fur_cfg[i_rtf].entry_tmp;

         // Strip temperature calculation
         status = RTF_strip( &sim_rtf[i_rtf], roll, tmp_tub, &tmp_stp );
         if ( !status ) ns_STD::cl_TRC::ERR_Write("SIM", sub_name, "%s: Error in RTF_strip 1", fur_cfg[RTH + i_rtf].fur_name.c_str());

	      s_sim.s_fur_mea[i_rtf].stp_tmp = 0.95f * s_sim.s_fur_mea[i_rtf].stp_tmp + 0.05f * (tmp_stp - TK);
      } // For all RTF furnaces
   } // else

   return status;
} // void

/*============================================================================
/ SIM_Mea_tnl: Measurements simulation for the TNL.
/ 
/ Revisions:
/  Created:       2016-Fev      Sh. Mechiev
/  Modified: 
/===========================================================================*/
bool SIM_Mea_tnl()
{
   bool status = true;
   string sub_name = "SIM_Mea_tnl";

   s_sim.s_fur_mea[TNL].ctl = true;
	s_sim.s_fur_mea[TNL].stp_tmp = s_sim.s_fur_mea[TNL-1].stp_tmp;

   // For all zones
   for ( int i_zon=0; i_zon < NB_ZON_TNL; i_zon++ ) 
   {
      s_sim.s_fur_mea[TNL].zon_tmp[i_zon] = s_sim.s_fur_mea[TNL-1].zon_tmp[i_zon] +
         0.2f * (s_coil.s_coil_data[0].tmp_tgt[TNL] - TK - s_sim.s_fur_mea[TNL].zon_tmp[i_zon]) -
         0.1f * fba_stp[TNL].pid_exit;
   } // For all zones

   return status;
}

/*============================================================================
/ SIM_Mea_slw: Measurements simulation for the SLW.
/ 
/ Revisions:
/  Created:       2007-Apr      L.Lâm Ngoc
/  Modified: 
/===========================================================================*/
bool SIM_Mea_slw()
{
   bool status = true;
   string sub_name = "SIM_Mea_slw";

	static bool once = true;
   if (once || s_sim.s_fur_mea[SLW].stp_tmp <= s_sim.s_coil_data[0].tmp_tgt[SLW] - 200.f - TK )
   {
      s_sim.s_fur_mea[SLW].ctl = false;
      s_sim.s_fur_mea[SLW].stp_tmp = s_sim.s_coil_data[0].tmp_tgt[SLW] + 11.f - TK;

      // For all zones
      for ( int i_zon=0; i_zon < NB_ZON_SLW; i_zon++ ) 
      {
         s_sim.s_fur_mea[SLW].zon_tmp[i_zon] = s_sim.s_coil_data[0].tmp_tgt[SLW] + float(i_zon) - TK;
         s_sim.s_fur_mea[SLW].cool_pow_pct[i_zon] = 50.f + float(i_zon);
      } // For all zones

      once = false;
   }

   else
   {
      s_sim.s_fur_mea[SLW].ctl = true;
	   s_sim.s_fur_mea[SLW].stp_tmp = 0.9f * s_sim.s_fur_mea[SLW].stp_tmp + 0.1f * (sps_act_sp[SLW].tmp_stp - TK) -
                                    ( s_sim.s_fur_mea[SLW].cool_pow_pct[0] - 50.f) / 5.f;

      // For all zones
      for ( int i_zon=0; i_zon < NB_ZON_SLW; i_zon++ ) 
      {
         s_sim.s_fur_mea[SLW].zon_tmp[i_zon] = s_sim.s_fur_mea[SLW].zon_tmp[i_zon] +
            0.2f * (s_coil.s_coil_data[0].tmp_tgt[SLW] - TK - s_sim.s_fur_mea[SLW].zon_tmp[i_zon]) -
            0.1f * fba_stp[SLW].pid_exit;

         s_sim.s_fur_mea[SLW].cool_pow_pct[i_zon] = 0.9f * s_sim.s_fur_mea[SLW].cool_pow_pct[i_zon] + 
                                                    0.1f * sps_act_sp[SLW].cool_pow_pct[i_zon];
         s_sim.s_fur_mea[SLW].cool_pow_pct[i_zon] = min(s_sim.s_fur_mea[SLW].cool_pow_pct[i_zon], 100.f);
         s_sim.s_fur_mea[SLW].cool_pow_pct[i_zon] = max(s_sim.s_fur_mea[SLW].cool_pow_pct[i_zon], 0.f);
      } // For all zones
   }

   // For all fans
   for ( int i_fan=0; i_fan < 3; i_fan++ ) 
   {
      s_fur_mea[SLW].s_spe.slw_fan_spd[i_fan] = s_fur_mea[SLW].cool_pow_pct[0] + float(i_fan);
   } // For all zones

   return status;
}

/*============================================================================
/ SIM_Mea_jet: Measurements simulation for the JET.
/ 
/ Revisions:
/  Created:       2007-Apr      L.Lâm Ngoc
/  Modified: 
/===========================================================================*/
bool SIM_Mea_jet()
{
   bool status = true;
   string sub_name = "SIM_Mea_jet";

   s_sim.s_fur_mea[JET].fan_mode = true;

	static bool once = true;
   if (once || s_sim.s_fur_mea[JET].stp_tmp <= s_sim.s_coil_data[0].tmp_tgt[JET] - 200.f - TK)
   {
      s_sim.s_fur_mea[JET].ctl = false;
      s_sim.s_fur_mea[JET].stp_tmp = s_sim.s_coil_data[0].tmp_tgt[JET] + 11.f - TK;

      // For all zones
      for ( int i_zon=0; i_zon < NB_ZON_JET; i_zon++ ) 
      {
         s_sim.s_fur_mea[JET].cool_pow_pct[i_zon] = 50.f + float(i_zon);
      } // For all zones

      once = false;
   }

   else
   {
      s_sim.s_fur_mea[JET].fan_mode = true;
      s_sim.s_fur_mea[JET].ctl = true;
	   s_sim.s_fur_mea[JET].stp_tmp = 0.9f * s_sim.s_fur_mea[JET].stp_tmp + 0.1f * (sps_act_sp[JET].tmp_stp - TK) -
                                    ( s_sim.s_fur_mea[JET].cool_pow_pct[0] - 50.f) / 5.f;

      // For all zones
      for ( int i_zon=0; i_zon < NB_ZON_JET; i_zon++ ) 
      {
         s_sim.s_fur_mea[JET].cool_pow_pct[i_zon] = 0.9f * s_sim.s_fur_mea[JET].cool_pow_pct[i_zon] + 
                                               0.1f * sps_act_sp[JET].cool_pow_pct[i_zon];
         s_sim.s_fur_mea[JET].cool_pow_pct[i_zon] = min(s_sim.s_fur_mea[JET].cool_pow_pct[i_zon], 100.f);
         s_sim.s_fur_mea[JET].cool_pow_pct[i_zon] = max(s_sim.s_fur_mea[JET].cool_pow_pct[i_zon], 0.f);
      } // For all zones
   } // else

   return status;
} // void

/*============================================================================
/ SIM_Mea_ovg: Measurements simulation for the OVG.
/ 
/ Revisions:
/  Created:       2007-Apr      L.Lâm Ngoc
/  Modified: 
/===========================================================================*/
bool SIM_Mea_ovg()
{
   bool status = true;
   string sub_name = "SIM_Mea_ovg";

	static bool once = true;
   if (once || s_sim.s_fur_mea[OVG].stp_tmp <= s_sim.s_coil_data[0].tmp_tgt[OVG] - 200.f - TK )
   {
      s_sim.s_fur_mea[OVG].ctl = false;
      s_sim.s_fur_mea[OVG].stp_tmp = s_sim.s_coil_data[0].tmp_tgt[OVG] + 11.f - TK;

      // For all zones
      for ( int i_zon=0; i_zon < NB_ZON_OVG; i_zon++ ) 
      {
         s_sim.s_fur_mea[OVG].zon_tmp[i_zon] = s_sim.s_coil_data[0].tmp_tgt[OVG] + 11.f + float(i_zon) - TK;
         s_sim.s_fur_mea[OVG].heat_pow_pct[i_zon] = 50.f  + float(i_zon);
      } // For all zones

      once = false;
   }

   else
   {
      s_sim.s_fur_mea[OVG].ctl = true;
	   s_sim.s_fur_mea[OVG].stp_tmp = 0.9f * s_sim.s_fur_mea[OVG].stp_tmp + 0.1f * (sps_act_sp[OVG].tmp_stp - TK) +
                                    ( s_sim.s_fur_mea[OVG].heat_pow_pct[0] - 50.f) / 5.f;

      // For all zones
      for ( int i_zon=0; i_zon < NB_ZON_OVG; i_zon++ ) 
      {
         s_sim.s_fur_mea[OVG].zon_tmp[i_zon] = s_sim.s_fur_mea[OVG].zon_tmp[i_zon] +
            0.2f * (s_coil.s_coil_data[0].tmp_tgt[OVG] - TK - s_sim.s_fur_mea[OVG].zon_tmp[i_zon]) -
                                              0.1f * fba_stp[OVG].pid_exit;

         s_sim.s_fur_mea[OVG].heat_pow_pct[i_zon] = 0.9f * s_sim.s_fur_mea[OVG].heat_pow_pct[i_zon] + 
                                                    0.1f * sps_act_sp[OVG].heat_pow_pct[i_zon];
         s_sim.s_fur_mea[OVG].heat_pow_pct[i_zon] = min(s_sim.s_fur_mea[OVG].heat_pow_pct[i_zon], 100.f);
         s_sim.s_fur_mea[OVG].heat_pow_pct[i_zon] = max(s_sim.s_fur_mea[OVG].heat_pow_pct[i_zon], 0.f);
      } // For all zones
   }

   return status;
} // void

/*============================================================================
/ SIM_Mea_fcs: Measurements simulation for the FCS.
/ 
/ Revisions:
/  Created:       2007-Apr      L.Lâm Ngoc
/  Modified: 
/===========================================================================*/
bool SIM_Mea_fcs()
{
   bool status = true;
   string sub_name = "SIM_Mea_fcs";

	static bool once = true;
   if (once || s_sim.s_fur_mea[FCS].stp_tmp <= s_sim.s_coil_data[0].tmp_tgt[FCS] - 200.f - TK)
   {
      s_sim.s_fur_mea[FCS].ctl = false;
      s_sim.s_fur_mea[FCS].stp_tmp = s_sim.s_coil_data[0].tmp_tgt[FCS] + 11.f - TK;

      // For all zones
      for ( int i_zon=0; i_zon < NB_ZON_FCS; i_zon++ ) 
      {
         s_sim.s_fur_mea[FCS].zon_tmp[i_zon] = s_sim.s_coil_data[0].tmp_tgt[FCS] + 11.f + float(i_zon) - TK;
         s_sim.s_fur_mea[FCS].cool_pow_pct[i_zon] = 50.f + float(i_zon);
      } // For all zones

      once = false;
   }

   else
   {
      s_sim.s_fur_mea[FCS].ctl = true;
	   s_sim.s_fur_mea[FCS].stp_tmp = 0.9f * s_sim.s_fur_mea[FCS].stp_tmp + 0.1f * (sps_act_sp[FCS].tmp_stp - TK) -
                                    ( s_sim.s_fur_mea[FCS].cool_pow_pct[0] - 50.f) / 5.f;

      // For all zones
      for ( int i_zon=0; i_zon < NB_ZON_FCS; i_zon++ ) 
      {
         s_sim.s_fur_mea[FCS].zon_tmp[i_zon] = s_sim.s_fur_mea[FCS].zon_tmp[i_zon] +
            0.2f * (s_coil.s_coil_data[0].tmp_tgt[FCS] - TK - s_sim.s_fur_mea[FCS].zon_tmp[i_zon]) -
                                              0.1f * fba_stp[FCS].pid_exit;


         s_sim.s_fur_mea[FCS].cool_pow_pct[i_zon] = 0.9f * s_sim.s_fur_mea[FCS].cool_pow_pct[i_zon] + 
                                                    0.1f * sps_act_sp[FCS].cool_pow_pct[i_zon];
         s_sim.s_fur_mea[FCS].cool_pow_pct[i_zon] = min(s_sim.s_fur_mea[FCS].cool_pow_pct[i_zon], 100.f);
         s_sim.s_fur_mea[FCS].cool_pow_pct[i_zon] = max(s_sim.s_fur_mea[FCS].cool_pow_pct[i_zon], 0.f);
      } // For all zones
   }

   return status;
} // void

// **************************************************************************
// SIM_INI: Values for simulation initialization
// 
// Revisions:
//  Created:       2007-Sep      JC.Mitais
//  Modified: 
// **************************************************************************
bool SIM_INI()
{
   bool status = true;
   string sub_name = "SIM_INI";

   // Read simulation file
   //SIM_Read();

   // Read simulation table
   if (!SQL_SIM_SELECT())
   {
      // Scheduled coils
      // ===============
      // Number of coils
      s_sim.nb_coil = MAX_NB_COIL;

      // For all coils
      for (int i_coil=0; i_coil < s_sim.nb_coil; i_coil++)
      {
         s_sim.s_coil_data[i_coil].id = "C-" + to_string(i_coil + 1);
      } // For all coils
 
      // thickness
      s_sim.s_coil_data[0].thi = 1.95f / 1000.f;
      s_sim.s_coil_data[1].thi = 1.5f / 1000.f;
      s_sim.s_coil_data[2].thi = 1.8f / 1000.f;
      s_sim.s_coil_data[3].thi = 1.6f / 1000.f;
      s_sim.s_coil_data[4].thi = 1.95f / 1000.f;
      s_sim.s_coil_data[5].thi = 1.5f / 1000.f;
      s_sim.s_coil_data[6].thi = 1.8f / 1000.f;
      s_sim.s_coil_data[7].thi = 1.6f / 1000.f;
      s_sim.s_coil_data[8].thi = 1.8f / 1000.f;
      s_sim.s_coil_data[9].thi = 1.6f / 1000.f;

      // speed
      // For all coils
      for (int i_coil=0; i_coil < s_sim.nb_coil; i_coil++)
      {
         //s_sim.s_coil_data[i_coil].spd_max = 320.f / 60.f;
         s_sim.s_coil_data[i_coil].spd = s_sim.s_coil_data[i_coil].spd_max = 320.f / 60.f;
      } // For all coils
		
      // Length
      s_sim.s_coil_data[0].lgt = 500.f;
      s_sim.s_coil_data[1].lgt = 1000.f;
      s_sim.s_coil_data[2].lgt = 1500.f;
      s_sim.s_coil_data[3].lgt = 2000.f;
      s_sim.s_coil_data[4].lgt = 500.f;
      s_sim.s_coil_data[5].lgt = 1000.f;
      s_sim.s_coil_data[6].lgt = 1500.f;
      s_sim.s_coil_data[7].lgt = 2000.f;
      s_sim.s_coil_data[8].lgt = 1500.f;
      s_sim.s_coil_data[9].lgt = 2000.f;

      // Width
      s_sim.s_coil_data[0].wid = 1.160f;
      s_sim.s_coil_data[1].wid = 1.372f;
      s_sim.s_coil_data[2].wid = 1.285f;
      s_sim.s_coil_data[3].wid = 1.190f;
      s_sim.s_coil_data[4].wid = 1.160f;
      s_sim.s_coil_data[5].wid = 1.372f;
      s_sim.s_coil_data[6].wid = 1.285f;
      s_sim.s_coil_data[7].wid = 1.190f;
      s_sim.s_coil_data[8].wid = 1.285f;
      s_sim.s_coil_data[9].wid = 1.190f;

      // For all coils
      for ( int i_coil=0; i_coil < s_sim.nb_coil; i_coil++ )
      {
         s_sim.s_coil_data[i_coil].N_cla = 0;
         s_sim.s_coil_data[i_coil].N_pdt = 0;
         for (int i_rtf=0; i_rtf < NB_FUR_RTF; i_rtf++) s_sim.s_coil_data[i_coil].Trs_cal[i_rtf] = false;
         s_sim.s_coil_data[i_coil].valid = true;
         s_sim.s_coil_data[i_coil].spd_min = MIN_SPEED;

         s_sim.s_coil_data[i_coil].strip_tension = 60000.f;

         // RTH
		   if      (i_coil == 0) s_sim.s_coil_data[i_coil].tmp_tgt[RTH] = 780.f + TK;
		   else if (i_coil == 1) s_sim.s_coil_data[i_coil].tmp_tgt[RTH] = 780.f + TK;
		   else if (i_coil == 2) s_sim.s_coil_data[i_coil].tmp_tgt[RTH] = 760.f + TK;
		   else if (i_coil == 3) s_sim.s_coil_data[i_coil].tmp_tgt[RTH] = 790.f + TK;
		   else if (i_coil == 4) s_sim.s_coil_data[i_coil].tmp_tgt[RTH] = 780.f + TK;
		   else if (i_coil == 5) s_sim.s_coil_data[i_coil].tmp_tgt[RTH] = 780.f + TK;
		   else if (i_coil == 6) s_sim.s_coil_data[i_coil].tmp_tgt[RTH] = 760.f + TK;
		   else if (i_coil == 7) s_sim.s_coil_data[i_coil].tmp_tgt[RTH] = 790.f + TK;
		   else if (i_coil == 8) s_sim.s_coil_data[i_coil].tmp_tgt[RTH] = 760.f + TK;
		   else if (i_coil == 9) s_sim.s_coil_data[i_coil].tmp_tgt[RTH] = 790.f + TK;

         s_sim.s_coil_data[i_coil].tmp_upp[RTH] = s_sim.s_coil_data[i_coil].tmp_tgt[RTH] + 30.f;
         s_sim.s_coil_data[i_coil].tmp_low[RTH] = s_sim.s_coil_data[i_coil].tmp_tgt[RTH] - 15.f;
         s_sim.s_coil_data[i_coil].emi_rtf[RTH] = rtf_cfg[RTH].emi_stp;

         // RTS
		   if      (i_coil == 0) s_sim.s_coil_data[i_coil].tmp_tgt[RTS] = 790.f + TK;
		   else if (i_coil == 1) s_sim.s_coil_data[i_coil].tmp_tgt[RTS] = 785.f + TK;
		   else if (i_coil == 2) s_sim.s_coil_data[i_coil].tmp_tgt[RTS] = 765.f + TK;
		   else if (i_coil == 3) s_sim.s_coil_data[i_coil].tmp_tgt[RTS] = 800.f + TK;
		   else if (i_coil == 4) s_sim.s_coil_data[i_coil].tmp_tgt[RTS] = 790.f + TK;
		   else if (i_coil == 5) s_sim.s_coil_data[i_coil].tmp_tgt[RTS] = 785.f + TK;
		   else if (i_coil == 6) s_sim.s_coil_data[i_coil].tmp_tgt[RTS] = 765.f + TK;
		   else if (i_coil == 7) s_sim.s_coil_data[i_coil].tmp_tgt[RTS] = 800.f + TK;
		   else if (i_coil == 8) s_sim.s_coil_data[i_coil].tmp_tgt[RTS] = 765.f + TK;
		   else if (i_coil == 9) s_sim.s_coil_data[i_coil].tmp_tgt[RTS] = 800.f + TK;

         s_sim.s_coil_data[i_coil].tmp_upp[RTS] = s_sim.s_coil_data[i_coil].tmp_tgt[RTS] + 30.f;
         s_sim.s_coil_data[i_coil].tmp_low[RTS] = s_sim.s_coil_data[i_coil].tmp_tgt[RTS] - 15.f;
         s_sim.s_coil_data[i_coil].emi_rtf[RTS] = rtf_cfg[RTS].emi_stp;

         // If a TNL
         if (NB_FUR_TNL > 0)
         {
            s_sim.s_coil_data[i_coil].tmp_tgt[TNL] = s_sim.s_coil_data[i_coil].tmp_tgt[RTS];                   // test
            s_sim.s_coil_data[i_coil].tmp_upp[TNL] = s_sim.s_coil_data[i_coil].tmp_tgt[TNL] + 10.f;
            s_sim.s_coil_data[i_coil].tmp_low[TNL] = s_sim.s_coil_data[i_coil].tmp_tgt[TNL] - 10.f;
         } // If a TNL

         // If a SLW
         if (NB_FUR_SLW > 0)
         {
            s_sim.s_coil_data[i_coil].tmp_tgt[SLW] = 660.f + TK;
            s_sim.s_coil_data[i_coil].tmp_upp[SLW] = s_sim.s_coil_data[i_coil].tmp_tgt[SLW] + 10.f;
            s_sim.s_coil_data[i_coil].tmp_low[SLW] = s_sim.s_coil_data[i_coil].tmp_tgt[SLW] - 10.f;
         } // If a SLW

         // If a JET
         if (NB_FUR_JET > 0)
         {
            s_sim.s_coil_data[i_coil].tmp_tgt[JET] = 460.f + TK;
            s_sim.s_coil_data[i_coil].tmp_upp[JET] = s_sim.s_coil_data[i_coil].tmp_tgt[JET] + 10.f;
            s_sim.s_coil_data[i_coil].tmp_low[JET] = s_sim.s_coil_data[i_coil].tmp_tgt[JET] - 10.f;

            for (int i_zon=0; i_zon < NB_ZON_JET; i_zon++)
            {
               s_sim.s_coil_data[i_coil].Cool_rate[i_zon] = 20.f + float(i_zon); // ??????????????
            }
         } // If a JET

         // If a OVG
         if (NB_FUR_OVG > 0)
         {
            s_sim.s_coil_data[i_coil].tmp_tgt[OVG] = 420.f + TK;
            s_sim.s_coil_data[i_coil].tmp_upp[OVG] = s_sim.s_coil_data[i_coil].tmp_tgt[OVG] + 10.f;
            s_sim.s_coil_data[i_coil].tmp_low[OVG] = s_sim.s_coil_data[i_coil].tmp_tgt[OVG] - 10.f;
         } // If a OVG

         // If a FCS
         if (NB_FUR_FCS > 0)
         {
            s_sim.s_coil_data[i_coil].tmp_tgt[FCS] = 200.f + TK;
            s_sim.s_coil_data[i_coil].tmp_upp[FCS] = s_sim.s_coil_data[i_coil].tmp_tgt[FCS] + 10.f;
            s_sim.s_coil_data[i_coil].tmp_low[FCS] = s_sim.s_coil_data[i_coil].tmp_tgt[FCS] - 10.f;
         } // If a FCS

         // If a EXT
         if (NB_FUR_EXT > 0)
         {
            s_sim.s_coil_data[i_coil].tmp_tgt[EXT] = 460.f + TK;
            s_sim.s_coil_data[i_coil].tmp_upp[EXT] = s_sim.s_coil_data[i_coil].tmp_tgt[EXT] + 10.f;
            s_sim.s_coil_data[i_coil].tmp_low[EXT] = s_sim.s_coil_data[i_coil].tmp_tgt[EXT] - 10.f;
         } // If a EXT

         // If a APC
         if (NB_FUR_APC > 0)
         {
            s_sim.s_coil_data[i_coil].tmp_tgt[APC] = 270.f + TK;
            s_sim.s_coil_data[i_coil].tmp_upp[APC] = s_sim.s_coil_data[i_coil].tmp_tgt[APC] + 10.f;
            s_sim.s_coil_data[i_coil].tmp_low[APC] = s_sim.s_coil_data[i_coil].tmp_tgt[APC] - 10.f;
         } // If a APC
      } // For all coils
   }
   else
   {
      // For all coils
      for ( int i_coil=0; i_coil < s_sim.nb_coil; i_coil++ )
      {
         s_sim.s_coil_data[i_coil].valid = true;
         s_sim.s_coil_data[i_coil].spd = s_sim.s_coil_data[i_coil].spd_max;
         s_sim.s_coil_data[i_coil].strip_tension = 60000.f;

         for (int i_fur=RTH; i_fur < RTH + NB_FUR_RTF; i_fur++)
         {
            s_sim.s_coil_data[i_coil].Trs_cal[i_fur] = false;
            s_sim.s_coil_data[i_coil].emi_rtf[i_fur] = rtf_cfg[i_fur].emi_stp;
         }

         //// NOF
         //if (NB_FUR_NOF > 0)
         //   s_sim.s_coil_data[i_coil].emi[NOF] = EMI_COIL;                                            // rtf_cfg[NOF].emi_stp - not initialised

         // JET
         if (NB_FUR_JET > 0)
            for (int i_zon=0; i_zon < NB_ZON_JET; i_zon++)
               s_sim.s_coil_data[i_coil].Cool_rate[i_zon] = 20.f + float(i_zon); // ??????????????
      } // For all coils
   }

   // RTF
   // ===
   // Measured speed
   // --------------
   s_sim.s_line_trk.Spd_mea = s_sim.s_coil_data[0].spd_max ;   // Measured speed [m/s]
   s_sim.s_line_trk.Spd_ctrl = false;                          // Speed control (ON/OFF)

   // For all RTF furnaces
   for (int i_rtf=0; i_rtf < NB_FUR_RTF; i_rtf++) 
   {
      // Strip temperature measurement at RTF exit (pyro)
      s_sim.s_fur_mea[i_rtf].stp_tmp = s_sim.s_coil_data[0].tmp_tgt[i_rtf] - TK; 
      s_sim.s_fur_mea[i_rtf].ctl = true;    // RTF heat control (ON/OFF)

      // RTF zones
      // ---------
      for (int i_zon = 0; i_zon < fur_cfg[i_rtf].nb_zon; i_zon++)
      {
         s_sim.s_fur_mea[i_rtf].zon_tmp[i_zon] = 700.f;                    // Tube temperature measurement
         s_sim.s_fur_mea[i_rtf].heat_pow_pct[i_zon] = 50.f + float(i_rtf); // Measured power [%]
      } // RTF zones
   } // For all RTF furnaces

   // If a TNL
   if (NB_FUR_TNL > 0)
   {
      s_sim.s_fur_mea[TNL].ctl = true;
      s_sim.s_fur_mea[TNL].stp_tmp = s_sim.s_coil_data[0].tmp_tgt[TNL] - TK;

      // For all zones
      for ( int i_zon=0; i_zon < NB_ZON_TNL; i_zon++ )
      {
         s_sim.s_fur_mea[TNL].zon_tmp[i_zon] = s_sim.s_fur_mea[RTS].zon_tmp[NB_ZON_RTS-1];               // test
      } // For all zones
   } // If a TNL

   // If a SLW
   if (NB_FUR_SLW > 0)
   {
      s_sim.s_fur_mea[SLW].ctl = true;
      s_sim.s_fur_mea[SLW].stp_tmp = s_sim.s_coil_data[0].tmp_tgt[SLW] - TK;

      // For all zones
      for ( int i_zon=0; i_zon < NB_ZON_SLW; i_zon++ )
      {
         s_sim.s_fur_mea[SLW].cool_pow_pct[i_zon] = 50.f;
         s_sim.s_fur_mea[SLW].zon_tmp[i_zon] = 500.f;
      } // For all zones

      // For all fans
      for ( int i_fan=0; i_fan < 3; i_fan++ )
      {
         s_sim.s_fur_mea[SLW].s_spe.slw_fan_spd[i_fan] = 60.f;
      } // For all fans
   } // If a SLW

   // If a JET
   if (NB_FUR_JET > 0)
   {
      s_sim.s_fur_mea[JET].ctl = true;
      s_sim.s_fur_mea[JET].stp_tmp = s_sim.s_coil_data[0].tmp_tgt[JET] - TK;
      s_sim.s_fur_mea[JET].fan_mode = true;

      // For all zones
      for ( int i_zon=0; i_zon < NB_ZON_JET; i_zon++ )
      {
         s_sim.s_fur_mea[JET].cool_pow_pct[i_zon] = 60.f;
      } // For all zones
   } // If a JET

   // If a OVG
   if (NB_FUR_OVG > 0)
   {
      s_sim.s_fur_mea[OVG].ctl = true;
      s_sim.s_fur_mea[OVG].stp_tmp = s_sim.s_coil_data[0].tmp_tgt[OVG] - TK;

      // For all zones
      for ( int i_zon=0; i_zon < NB_ZON_OVG; i_zon++ )
      {
         s_sim.s_fur_mea[OVG].zon_tmp[i_zon] = 500.f;
         s_sim.s_fur_mea[OVG].heat_pow_pct[i_zon] = 55.f;
      } // For all zones
   } // If a OVG

   // If a FCS
   if (NB_FUR_FCS > 0)
   {
      s_sim.s_fur_mea[FCS].ctl = true;
      s_sim.s_fur_mea[FCS].stp_tmp = s_sim.s_coil_data[0].tmp_tgt[FCS] - TK;

      // For all zones
      for ( int i_zon=0; i_zon < NB_ZON_FCS; i_zon++ )
      {
         s_sim.s_fur_mea[FCS].zon_tmp[i_zon] = 400.f;
         s_sim.s_fur_mea[FCS].cool_pow_pct[i_zon] = 60.f;
      } // For all zones
   } // If a FCS

   // If a EXT
   if (NB_FUR_EXT > 0)
   {
      s_sim.s_fur_mea[EXT].ctl = true;
      s_sim.s_fur_mea[EXT].stp_tmp = s_sim.s_coil_data[0].tmp_tgt[EXT] - TK;

      // For all zones
      for ( int i_zon=0; i_zon < NB_ZON_EXT; i_zon++ )
      {
         s_sim.s_fur_mea[EXT].zon_tmp[i_zon] = 400.f;
         s_sim.s_fur_mea[EXT].cool_pow_pct[i_zon] = 60.f;
      } // For all zones
   } // If a EXT

   // If a APC
   if (NB_FUR_APC > 0)
   {
      s_sim.s_fur_mea[APC].ctl = true;
      s_sim.s_fur_mea[APC].stp_tmp = s_sim.s_coil_data[0].tmp_tgt[APC] - TK;

      // For all zones
      for ( int i_zon=0; i_zon < NB_ZON_APC; i_zon++ )
      {
         s_sim.s_fur_mea[APC].zon_tmp[i_zon] = 250.f;
         s_sim.s_fur_mea[APC].cool_pow_pct[i_zon] = 60.f;
      } // For all zones
   } // If a APC

   // Specific simulation initialization
   // ==================================
   SPS_Specific::SIM_INI ();

   //s_sim.weld_pos = -s_sim.s_coil_data[1].lgt + 1.f;
   s_sim.weld_pos = -s_sim.s_coil_data[0].lgt + 1.f;                             // test
   s_sim.s_line_trk.coil_id = s_sim.s_coil_data[0].id;

   s_coil.s_coil_data_last[0] = s_sim.s_coil_data[0];                            // test

   s_coil.s_coil_data_last[0].id = "UNKNOWN";
   s_coil.s_coil_data_last[0].valid = false;

   return status;
} // void

/*=============================================================================
/ SIM_Read: Read parameters in file. 
/ 
/ Revisions:
/  Created:       2007-oct.       X.FOURNAUD
/  Modified:      
//=============================================================================*/
bool SIM_Read()
{
   bool status=true;
   string sub_name="SIM_Read";

   FILE *SIM_File;
   #define L_rline 128
	char rline[L_rline];
	char fil_name[L_FIL_NAM];
   int  n_coil = 0;
   char word0[L_FIL_NAM], word1[L_FIL_NAM];
   float fword[4];

   // PRM file
   // --------
   strcpy_s( fil_name, DIR_CFG );
   strcat_s( fil_name, FILE_SIM_SCH );

   // if Opening PRM file is OK
   // =========================
   if ( fopen_s( &SIM_File, fil_name, "r") == 0 )
   {
      // Loop reading file lines
	   while ( fgets( rline, L_rline, SIM_File ) != NULL )
		{    
         sscanf(rline, "%s %s ", word0, word1);
         fword[0] = (float)atof(word1);
        
         // Comment treatment
			if ( rline[0] == '#' ) continue;

         // EOF treatment
         if ( strcmp(word0 , "EOF\n" ) == 0) break;

         // Keyword treatment
			if ( strcmp(word0, "MAX_NB_COIL") == 0 )
         {
            s_sim.nb_coil = int(fword[0]);
         }

         else if ( strcmp(word0, "CYCLE") == 0 )
         {
            s_sim.s_coil_data[n_coil].cycle = int(fword[0]);
         }
         
			else if ( strcmp(word0, "DUMMY") == 0 )
         {
           s_sim.s_coil_data[n_coil].dummy = int(fword[0]);
         }

			else if ( strcmp(word0, "LENGTH") == 0 )
         {
            s_sim.s_coil_data[n_coil].lgt= fword[0];
         }

			else if ( strcmp(word0, "THICK") == 0 )
         {
            s_sim.s_coil_data[n_coil].thi = fword[0]/1000.f;
         }

			else if ( strcmp(word0, "WIDTH") == 0 )
         {
            s_sim.s_coil_data[n_coil].wid = fword[0];
         }

			else if ( strcmp(word0, "TMP_TGT_RTF") == 0 )
         {
             s_sim.s_coil_data[n_coil].tmp_tgt[RTH]= fword[0]+TK;
         }

			else if ( strcmp(word0, "TMP_TGT_LOW") == 0 )
         {
             s_sim.s_coil_data[n_coil].tmp_low[RTH]= fword[0]+TK;
         }

         else if ( strcmp(word0, "TMP_TGT_UPP") == 0 )
         {
            s_sim.s_coil_data[n_coil].tmp_upp[RTH] = fword[0]+TK;
         }

         else if ( strcmp(word0, "TMP_TGT_JET") == 0 )
         {
             s_sim.s_coil_data[n_coil].tmp_tgt[JET]= fword[0]+TK;
         }

			else if ( strcmp(word0, "TMP_TGT_JET_LOW") == 0 )
         {
             s_sim.s_coil_data[n_coil].tmp_low[JET]= fword[0]+TK;
         }

         else if ( strcmp(word0, "TMP_TGT_JET_UPP") == 0 )
         {
            s_sim.s_coil_data[n_coil].tmp_upp[JET] = fword[0]+TK;
         }

         else if ( strcmp(word0, "SPEED") == 0 )
         {
            s_sim.s_coil_data[n_coil].spd_max = fword[0]/60.f;
         }

          else if ( strcmp(word0, "STEELTYPE") == 0 )
         {
            s_sim.s_coil_data[n_coil].steeltype = int(fword[0]);
         }

         else if ( strcmp(word0, "NCLA") == 0 )
         {
            s_sim.s_coil_data[n_coil].N_cla = int(fword[0]);
         }

         else if ( strcmp(word0, "NPDT") == 0 )
         {
            s_sim.s_coil_data[n_coil].N_pdt = int(fword[0]);

            n_coil ++;
         }
      } // Loop reading file lines

      // Close the file
      // --------------
      fclose(SIM_File);
   } // if Opening  file is OK

   else
   {
      status = false;
      ns_STD::cl_TRC::ERR_Write("SIM", sub_name, " %s: Unable to open simulation file %s\n", fil_name );
   }

   s_sim.nb_coil = n_coil;

   return status;
}