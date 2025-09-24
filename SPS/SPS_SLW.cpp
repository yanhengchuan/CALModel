/*============================================================================
/ SPS_SLW: slow cooling section sps module
/ 
/ Revisions:
/  Created:       2009-OCT          J. NAVEIRA
/  Modified:      2016-Mar          JC.Mitais / J.Delgas
/  
/ (c) CMI Industry
/===========================================================================*/
#include "stdafx.h"

//===========================================================================
bool SPS_SLW(struct sps_coil_data sps_fin_sp_slw)
{
   bool status = true;
   string sub_name = "SPS_SLW";

	bool iflag_max = true, iflag_min = true;
	// Slow cooling can heat or cool
	// only one exit PID for both controls
	// 0 -> 50% is heating with 0% being maximum heating
	// 50 -> 100% is cooling with 100% being maximum cooling
	// fan speed is kept at 10% minimum output by level 1

	fba_stp[SLW].pv = sps_entry[SLW].s_mea.tmp_stp;
   fba_stp[SLW].sp = sps_entry[SLW].s_cur_coil.tmp_tgt;
	sps_act_sp[SLW].tmp_stp = fba_stp[SLW].sp;

	if (fba_stp[SLW].sp > 0.f && fba_stp[SLW].sp < 1500.f)
		sps_act_sp[SLW].Valid = true;

   if ( fba_stp[SLW].sp <= 0.f ) fba_stp[SLW].sp = fba_stp[SLW].pv;

	float avg_pow = 0.f;
	
	avg_pow = sps_entry[SLW].s_mea.cool_pow_pct[0];

   // For all zones
   for ( int i_zon=0; i_zon < NB_ZON_SLW; i_zon++ )
   {
      sps_fin_sp_slw.pow_pct[i_zon] = avg_pow;
   } // For all zones

   fba_stp[SLW].max_exit = 100.f;// - avg_pow;
   fba_stp[SLW].min_exit = 0.f;//-avg_pow;

   sps_act_sp[SLW].ctl_pow = true;
   // PID calculation
   // ===============
   // If Section in automatic
   if ( sps_entry[SLW].s_mea.auto_on ) 
   {
      fba_stp[SLW].reset = false;
      status = ns_STD::cl_PID::SPS_PID(&fba_stp[SLW]);
      if ( !status )
      {
         sps_act_sp[SLW].Valid = false;
         ns_STD::cl_TRC::ERR_Write("SPS", sub_name, "error in SPS_PID");
         return status;
      }
   } // If Section in automatic

   else // If Section not in automatic
   {
      fba_stp[SLW].reset = true;      
      fba_stp[SLW].i_exit = avg_pow  -(- fba_stp[SLW].p * (fba_stp[SLW].sp - fba_stp[SLW].pv));
      fba_stp[SLW].i_exit = max(fba_stp[SLW].i_exit, fba_stp[SLW].min_exit);
      fba_stp[SLW].i_exit = min(fba_stp[SLW].i_exit, fba_stp[SLW].max_exit);
   } // If Section not in automatic

   // Calculate power set point
   // -------------------------
   // For all zones
   for ( int i_zon=0; i_zon < NB_ZON_SLW; i_zon++ )
   { 
		if(sps_entry[SLW].s_mea.auto_on )
			sps_act_sp[SLW].cool_pow_pct[i_zon] =  fba_stp[SLW].pid_exit;//sps_fin_sp_slw.pow_pct[i_zon] +
		else
			sps_act_sp[SLW].cool_pow_pct[i_zon] = avg_pow;
      sps_act_sp[SLW].cool_pow_pct[i_zon] = min( sps_act_sp[SLW].cool_pow_pct[i_zon], 100.f );
      sps_act_sp[SLW].cool_pow_pct[i_zon] = max( sps_act_sp[SLW].cool_pow_pct[i_zon],   0.f );
   } // For all zones

   return status;
}
