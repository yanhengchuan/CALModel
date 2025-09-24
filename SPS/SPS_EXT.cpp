/*============================================================================
/ SPS_EXT: EXT sps module
/ 
/ Revisions:
/  Created:       2016-JUL          Sh. Mechiev
/  Modified:
/  
/ (c) CMI Industry
/===========================================================================*/
#include "stdafx.h"

//===========================================================================
bool SPS_EXT(struct sps_coil_data sps_fin_sp_ext)
{
   bool status = true;
   string sub_name = "SPS_EXT";

   // Initialize pv with strip temperature measurement and sp with temperature target
   fba_stp[EXT].pv = sps_entry[EXT].s_mea.tmp_stp;
   fba_stp[EXT].sp = sps_entry[EXT].s_cur_coil.tmp_tgt;

	if (fba_stp[EXT].sp > 0.f && fba_stp[EXT].sp < 1500.f)
		sps_act_sp[EXT].Valid = true;

   if ( fba_stp[EXT].sp <= 0.f ) fba_stp[EXT].sp = fba_stp[EXT].pv;

   // If the power is everywhere at max, we don't try to increase it
   fba_stp[EXT].max_exit = 50.f;

   // If the power is everywhere at min, we don't try to decrease it
   fba_stp[EXT].min_exit = - 50.f;

   sps_act_sp[EXT].ctl_pow = true;
   // PID calculation
   if ( sps_entry[EXT].s_mea.auto_on ) 
   {
      fba_stp[EXT].reset = false;
      status = ns_STD::cl_PID::SPS_PID(&fba_stp[EXT]);
      if ( !status )
      {
         ns_STD::cl_TRC::ERR_Write("EXT", sub_name, "error EXT in SPS_PID");
         return status;
      }
   }

   else
   {
      fba_stp[EXT].reset = true;
      float avg_tmp = 0.f;
      // For all zones
      for ( int i_zon=0; i_zon < NB_ZON_EXT; i_zon++ )
      {
         avg_tmp += sps_entry[EXT].s_mea.tmp_zon[i_zon];
      } // For all zones
      avg_tmp /= float(NB_ZON_EXT);

      fba_stp[EXT].i_exit = avg_tmp - fba_stp[EXT].pv - fba_stp[EXT].p * (fba_stp[EXT].sp - fba_stp[EXT].pv);
      fba_stp[EXT].i_exit = max(fba_stp[EXT].i_exit, fba_stp[EXT].min_exit);
      fba_stp[EXT].i_exit = min(fba_stp[EXT].i_exit, fba_stp[EXT].max_exit);
   }
   
   // Storing zone temp set points
   // -------------------------
   // for all zones
   for ( int i_zon=0; i_zon < NB_ZON_EXT; i_zon++ )
   {
      sps_act_sp[EXT].tmp_zon[i_zon] = fba_stp[EXT].sp + fba_stp[EXT].pid_exit;
      sps_act_sp[EXT].tmp_zon[i_zon] = min( sps_act_sp[EXT].tmp_zon[i_zon], 1000.f );
      sps_act_sp[EXT].tmp_zon[i_zon] = max( sps_act_sp[EXT].tmp_zon[i_zon],  300.f );
   } // for all zones

   return status;
} // void