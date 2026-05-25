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

   fba_stp[APC].pv = sps_entry[APC].s_mea.tmp_stp;
   fba_stp[APC].sp = sps_entry[APC].s_cur_coil.tmp_tgt;

	if (fba_stp[APC].sp > 0.f && fba_stp[APC].sp < 1500.f)
		sps_act_sp[APC].Valid = true;

   if ( fba_stp[APC].sp <= 0.f ) fba_stp[APC].sp = fba_stp[APC].pv;

   fba_stp[APC].max_exit = 50.f;
   fba_stp[APC].min_exit = -50.f;

   sps_act_sp[APC].ctl_pow = true;
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
      float avg_tmp = 0.f;
      fba_stp[APC].reset = true;
      for ( int i_zon=0; i_zon < NB_ZON_APC; i_zon++ ) avg_tmp += sps_entry[APC].s_mea.tmp_zon[i_zon];
      avg_tmp /= float(NB_ZON_APC);
      fba_stp[APC].i_exit = avg_tmp - fba_stp[APC].pv - fba_stp[APC].p * (fba_stp[APC].sp - fba_stp[APC].pv);
      fba_stp[APC].i_exit = max(fba_stp[APC].i_exit, fba_stp[APC].min_exit);
      fba_stp[APC].i_exit = min(fba_stp[APC].i_exit, fba_stp[APC].max_exit);
   }

   for ( int i_zon=0; i_zon < NB_ZON_APC; i_zon++ )
   {
      sps_act_sp[APC].tmp_zon[i_zon] = fba_stp[APC].sp + fba_stp[APC].pid_exit;
      sps_act_sp[APC].tmp_zon[i_zon] = min( sps_act_sp[APC].tmp_zon[i_zon], 1000.f );
      sps_act_sp[APC].tmp_zon[i_zon] = max( sps_act_sp[APC].tmp_zon[i_zon], 300.f );
   }

   return status;
} // void