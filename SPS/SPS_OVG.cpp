/*============================================================================
/ SPS_OVG: OVG sps module
/ 
/ Revisions:
/  Created:       2009-OCT          J. NAVEIRA
/  Modified:      2016-Mar          JC.Mitais / J.Delgas
/  
/ (c) CMI Industry
/===========================================================================*/
#include "stdafx.h"

//===========================================================================
bool SPS_OVG(struct sps_coil_data sps_fin_sp_ovg)
{
   bool status = true;
   string sub_name = "SPS_OVG";

	fba_stp[OVG].pv = sps_entry[OVG].s_mea.tmp_stp;
   fba_stp[OVG].sp = sps_entry[OVG].s_cur_coil.tmp_tgt;

   if (fba_stp[OVG].sp > 0.f && fba_stp[OVG].sp < 1500.f)
	   sps_act_sp[OVG].Valid = true;

	// For all zones
	for ( int i_zon=0; i_zon < fur_cfg[OVG].nb_zon; i_zon++ )
	{
		fba_zon[OVG][i_zon].p = fba_stp[OVG].p;
		fba_zon[OVG][i_zon].i = fba_stp[OVG].i;

		fba_zon[OVG][i_zon].pv = sps_entry[OVG].s_mea.tmp_zon[i_zon];																		//fba_stp[OVG].pv;
		if (i_zon < 5 )
			fba_zon[OVG][i_zon].sp = (sps_entry[JET].s_cur_coil.tmp_tgt+sps_entry[OVG].s_cur_coil.tmp_tgt)/2 + 5.f;	//fba_stp[OVG].sp;
		else
			fba_zon[OVG][i_zon].sp = sps_entry[OVG].s_cur_coil.tmp_tgt;

		fba_zon[OVG][i_zon].max_exit = 100.f - sps_entry[OVG].s_mea.heat_pow_pct[i_zon];
		fba_zon[OVG][i_zon].min_exit = -sps_entry[OVG].s_mea.heat_pow_pct[i_zon];
	}

   if ( fba_stp[OVG].sp <= 0.f ) fba_stp[OVG].sp = fba_stp[OVG].pv;

	float avg_pow = 0.f;
	// For all zones
	for ( int i_zon=0; i_zon < NB_ZON_OVG; i_zon++ )
	{
		avg_pow += sps_entry[OVG].s_mea.heat_pow_pct[i_zon];
	} // For all zones
	avg_pow /= float(NB_ZON_OVG);

   // If the power is everywhere at max, we don't try to increase it
   fba_stp[OVG].max_exit = 100.f - avg_pow;

   // If the power is everywhere at min, we don't try to decrease it
   fba_stp[OVG].min_exit = -avg_pow;

   sps_act_sp[OVG].ctl_pow = true;
   // PID calculation
   // ===============
   // If Section in automatic
   if ( sps_entry[OVG].s_mea.auto_on ) 
   {
      fba_stp[OVG].reset = false;
		// Strip PID calculation
      status = ns_STD::cl_PID::SPS_PID(&fba_stp[OVG]);
      if ( !status )
      {
         sps_act_sp[OVG].Valid = false;
         ns_STD::cl_TRC::ERR_Write("OVG", sub_name, "error OVG in SPS_PID");
         return status;
      }

		// Zone PID calculation
		for ( int i_zon=0; i_zon < fur_cfg[OVG].nb_zon; i_zon++ )
		{
			fba_zon[OVG][i_zon].reset = false;
			status = ns_STD::cl_PID::SPS_PID (&fba_zon[OVG][i_zon]);
			if ( !status ) ns_STD::cl_TRC::ERR_Write("SPS", sub_name, "error in SPS_PID");
		}
   } // If Section in automatic

   else // If Section not in automatic
   {
      fba_stp[OVG].reset = true;
      fba_stp[OVG].i_exit = avg_pow - fba_stp[OVG].p * (fba_stp[OVG].sp - fba_stp[OVG].pv);
      fba_stp[OVG].i_exit = max(fba_stp[OVG].i_exit, fba_stp[OVG].min_exit);
      fba_stp[OVG].i_exit = min(fba_stp[OVG].i_exit, fba_stp[OVG].max_exit); 
		// for all zones
		for ( int i_zon=0; i_zon < NB_ZON_OVG; i_zon++ )
		{
			fba_zon[OVG][i_zon].reset = true;
			fba_zon[OVG][i_zon].i_exit = sps_entry[OVG].s_mea.heat_pow_pct[i_zon] - fba_zon[OVG][i_zon].p * (fba_zon[OVG][i_zon].sp - fba_zon[OVG][i_zon].pv);
			fba_zon[OVG][i_zon].i_exit = max(fba_zon[OVG][i_zon].i_exit, fba_zon[OVG][i_zon].min_exit);
			fba_zon[OVG][i_zon].i_exit = min(fba_zon[OVG][i_zon].i_exit, fba_zon[OVG][i_zon].max_exit);

			sps_act_sp[OVG].heat_pow_pct[i_zon] = sps_entry[OVG].s_mea.heat_pow_pct[i_zon];
			sps_act_sp[OVG].tmp_zon[i_zon] = sps_entry[OVG].s_mea.tmp_zon[i_zon];
		}
	} // If Section not in automatic

   // Calculate power set point
   // -------------------------
   // for all zones
   for ( int i_zon=0; i_zon < NB_ZON_OVG; i_zon++ )
   {
		if ( sps_entry[OVG].s_mea.auto_on ) 
		{
	      sps_act_sp[OVG].heat_pow_pct[i_zon] = fba_zon[OVG][i_zon].pid_exit;
		}

      sps_act_sp[OVG].heat_pow_pct[i_zon] = min( sps_act_sp[OVG].heat_pow_pct[i_zon], 100.f );
      sps_act_sp[OVG].heat_pow_pct[i_zon] = max( sps_act_sp[OVG].heat_pow_pct[i_zon],   0.f );

		// To distribute temperature set points between two parts of OVG
		if (i_zon < 5 )
			sps_act_sp[OVG].tmp_zon[i_zon] = (sps_entry[JET].s_cur_coil.tmp_tgt+fba_stp[OVG].sp)/2 + 5.f;
		else
			sps_act_sp[OVG].tmp_zon[i_zon] = fba_stp[OVG].sp;
   } // for all zones

   return status;
} // void