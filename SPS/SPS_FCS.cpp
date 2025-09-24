/*============================================================================
/ SPS_FCS: FCS sps module
/ 
/ Revisions:
/  Created:       2009-OCT          J. NAVEIRA
/  Modified:      2016-Mar          JC.Mitais / J.Delgas
/  
/ (c) CMI Industry
/===========================================================================*/
#include "stdafx.h"

//===========================================================================
bool SPS_FCS(struct sps_coil_data sps_fin_sp_fcs)
{
   bool status = true;
   string sub_name = "SPS_FCS";

	fba_stp[FCS].pv = sps_entry[FCS].s_mea.tmp_stp;
   fba_stp[FCS].sp = sps_entry[FCS].s_cur_coil.tmp_tgt;
	sps_act_sp[FCS].tmp_stp = fba_stp[FCS].sp;

	if (fba_stp[FCS].sp > 0.f && fba_stp[FCS].sp < 1500.f)
		sps_act_sp[FCS].Valid = true;

	float avg_pow = 0.f;
	// For all zones
	for ( int i_zon=0; i_zon < NB_ZON_FCS; i_zon++ )
	{
		avg_pow += sps_entry[FCS].s_mea.cool_pow_pct[i_zon];
	} // For all zones
	avg_pow /= float(NB_ZON_FCS);


   if ( fba_stp[FCS].sp <= 0.f ) fba_stp[FCS].sp = fba_stp[FCS].pv;

   // If the power is everywhere at max, we don't try to increase it
   fba_stp[FCS].max_exit = 100.f;			// 100.f-avg_pow

   // If the power is everywhere at min, we don't try to decrease it
   fba_stp[FCS].min_exit = 0.f;				// -avg_pow

   sps_act_sp[FCS].ctl_pow = true;
   // PID calculation
   if ( sps_entry[FCS].s_mea.auto_on ) 
   {
      fba_stp[FCS].reset = false;
      status = ns_STD::cl_PID::SPS_PID(&fba_stp[FCS]);
      if ( !status )
      {
         ns_STD::cl_TRC::ERR_Write("FCS", sub_name, "error FCS in SPS_PID");
         return status;
      }
   }

   else
   {
      fba_stp[FCS].reset = true;

      fba_stp[FCS].i_exit = avg_pow  -( - fba_stp[FCS].p * (fba_stp[FCS].sp - fba_stp[FCS].pv));
      fba_stp[FCS].i_exit = max(fba_stp[FCS].i_exit, fba_stp[FCS].min_exit);
      fba_stp[FCS].i_exit = min(fba_stp[FCS].i_exit, fba_stp[FCS].max_exit);
   }

   // Calculate power set point
   // -------------------------
   // for all zones
   for ( int i_zon=0; i_zon < NB_ZON_FCS; i_zon++ )
   {
		if (sps_entry[FCS].s_mea.auto_on)
			sps_act_sp[FCS].cool_pow_pct[i_zon] =  fba_stp[FCS].pid_exit;//avg_pow +
		else
			sps_act_sp[FCS].cool_pow_pct[i_zon] = avg_pow;
      sps_act_sp[FCS].cool_pow_pct[i_zon] = min( sps_act_sp[FCS].cool_pow_pct[i_zon], 100.f );
      sps_act_sp[FCS].cool_pow_pct[i_zon] = max( sps_act_sp[FCS].cool_pow_pct[i_zon],   0.f );
   } // for all zones

   return status;
} // void