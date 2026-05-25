/*============================================================================
/ SPS_APC: APC sps module
/ 
/ Revisions:
/  Created:       2016-JUL          Sh. Mechiev
/  Modified:
/  
/ (c) CMI Industry
/===========================================================================*/
#include "stdafx.h"

//===========================================================================
bool SPS_APC(struct sps_coil_data sps_fin_sp_apc)
{
   bool status = true;
   string sub_name = "SPS_APC";

   // Initialize pv with strip temperature measurement and sp with temperature target
   fba_stp[APC].pv = sps_entry[APC].s_mea.tmp_stp;
   fba_stp[APC].sp = sps_entry[APC].s_cur_coil.tmp_tgt;

	if (fba_stp[APC].sp > 0.f && fba_stp[APC].sp < 1500.f)
		sps_act_sp[APC].Valid = true;

   if ( fba_stp[APC].sp <= 0.f ) fba_stp[APC].sp = fba_stp[APC].pv;

   // If the power is everywhere at max, we don't try to increase it
   fba_stp[APC].max_exit = 50.f;

   // If the power is everywhere at min, we don't try to decrease it
   fba_stp[APC].min_exit = - 50.f;

   sps_act_sp[APC].ctl_pow = true;
   // PID calculation
   if ( sps_entry[APC].s_mea.auto_on )
   {
      fba_stp[APC].reset = false;
      status = ns_STD::cl_PID::SPS_PID(&fba_stp[APC]);
      if ( !status )
      {
         ns_STD::cl_TRC::ERR_Write("APC", sub_name, "error APC in SPS_PID");
         return status;
      }
   }

   else
   {
      fba_stp[APC].reset = true;
      float avg_pow = 0.f;
      // For all zones
      for ( int i_zon=0; i_ i_zon++ )
      {
         avg_pow += sps_entry[APC].s_mea.cool_pow_pct[i_zon];
      } // For all zones
      avg_pow /= float(NB_ZON_APC);

      fba_stp[APC].i_exit = avg_pow - 50.f - fba_stp[APC].p * (fba_stp[APC].sp - fba_stp[APC].pv);
      fba_stp[APC].i_exit = max(fba_stp[APC].i_exit, fba_stp[APC].min_exit);
      fba_stp[APC].i_exit = min(fba_stp[APC].i_exit, fba_stp[APC].max_exit);
   }
   
   // Storing power set points
   // -------------------------
   // for all zones
   for ( int i_zon=0; i_zon < NB_ZON_APC; i_zon++ )
   {
      sps_act_sp[APC].cool_pow_pct[i_zon] = 50.f + fba_stp[APC].pid_exit;
      sps_act_sp[APC].cool_pow_pct[i_zon] = min( sps_act_sp[APC].cool_pow_pct[i_zon], 100.f );
      sps_act_sp[APC].cool_pow_pct[i_zon] = max( sps_act_sp[APC].cool_pow_pct[i_zon],   0.f );
   } // for all zones

   return status;
} // void