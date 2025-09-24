/*============================================================================
/ SPS_SOA: soaking section sps module
/ 
/ Revisions:
/  Created:       2009-OCT          J. NAVEIRA
/  Modified:      2016-Mar          JC.Mitais / J.Delgas
/
/ Remark:
/ - Depending on "STRIP_SOA" value, the set-points are from the total RTF
/     calculation, or from this one.
/  
/ (c) CMI Industry
/===========================================================================*/
#include "stdafx.h"

//===========================================================================
bool SPS_SOA(struct sps_coil_data sps_fin_sp_soa)
{
   bool status = true;
   string sub_name = "SPS_SOA";

	fba_stp[RTS].pv = sps_entry[RTS].s_mea.tmp_stp;
   fba_stp[RTS].sp = sps_entry[RTS].s_cur_coil.tmp_tgt;

	if (fba_stp[RTS].sp > 0.f && fba_stp[RTS].sp < 1500.f)
		sps_act_sp[RTS].Valid = true;

   if ( fba_stp[RTS].sp <= 0.f ) fba_stp[RTS].sp = fba_stp[RTS].pv;

   // If the power is everywhere at max, we don't try to increase it
   fba_stp[RTS].max_exit = 50.f;

   // If the power is everywhere at min, we don't try to decrease it
   fba_stp[RTS].min_exit = - 50.f;

   sps_act_sp[RTS].ctl_pow = true;
   // PID calculation
   // ===============
   // If Section in automatic
   if ( sps_entry[RTS].s_mea.auto_on ) 
   {
      fba_stp[RTS].reset = false;
      status = ns_STD::cl_PID::SPS_PID(&fba_stp[RTS]);
      if ( !status )
      {
         sps_act_sp[RTS].Valid = false;
         ns_STD::cl_TRC::ERR_Write("SOA", sub_name, "error SOA in SPS_PID");
         return status;
      }
   } // If Section in automatic

   else // If Section not in automatic
   {
      fba_stp[RTS].reset = true;
      float avg_pow = 0.f;
      // For all zones
      for ( int i_zon=0; i_zon < NB_ZON_RTS; i_zon++ )
      {
         avg_pow += sps_entry[RTS].s_mea.heat_pow_pct[i_zon];
      } // For all zones
      avg_pow /= float(NB_ZON_RTS);
      fba_stp[RTS].i_exit = avg_pow - 50.f - fba_stp[RTS].p * (fba_stp[RTS].sp - fba_stp[RTS].pv);
      fba_stp[RTS].i_exit = max(fba_stp[RTS].i_exit, fba_stp[RTS].min_exit);
      fba_stp[RTS].i_exit = min(fba_stp[RTS].i_exit, fba_stp[RTS].max_exit);
   } // If Section not in automatic

   // Calculate power set point
   // -------------------------
   // for all zones
   for ( int i_zon=0; i_zon < NB_ZON_RTS; i_zon++ )
   {
      sps_act_sp[RTS].heat_pow_pct[i_zon] = 50.f + fba_stp[RTS].pid_exit;
      sps_act_sp[RTS].heat_pow_pct[i_zon] = min( sps_act_sp[RTS].heat_pow_pct[i_zon], 100.f );
      sps_act_sp[RTS].heat_pow_pct[i_zon] = max( sps_act_sp[RTS].heat_pow_pct[i_zon],   0.f );

      sps_act_sp[RTS].tmp_zon[i_zon] = sps_entry[RTS].s_mea.tmp_zon[i_zon] + 
                                       fba_stp[RTS].pid_exit / 10.f;
   } // for all zones

   return status;
}