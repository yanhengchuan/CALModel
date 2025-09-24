/*============================================================================
/ SPS_JET: Jet cooling section sps module
/ 
/ Revisions:
/  Created:       2009-OCT          J. NAVEIRA
/  Modified:      2016-Mar          JC.Mitais / J.Delgas
/  
/ (c) CMI Industry
/===========================================================================*/
#include "stdafx.h"

//===========================================================================
bool SPS_JET(struct sps_coil_data sps_fin_sp_jet)
{
   bool status = true;
   string sub_name = "SPS_JET";

	float hst = 5.f;	            // hysterisis for cascade mode of the fans
	static int nb_zon_on = 1;		// number of zones ON in cascade mode

	fba_stp[JET].pv = sps_entry[JET].s_mea.tmp_stp;
   fba_stp[JET].sp =  sps_entry[JET].s_cur_coil.tmp_tgt; 
	sps_act_sp[JET].tmp_stp = fba_stp[JET].sp;
   if ( fba_stp[JET].sp <= 0.f ) fba_stp[JET].sp = fba_stp[JET].pv;

   if (fba_stp[JET].sp > 0.f && fba_stp[JET].sp < 1500.f)
	   sps_act_sp[JET].Valid = true;


	float avg_pow = 0.f, sec_pow = 0.f, tot_pow = 0.f;
	// For all zones
	for ( int i_zon=0; i_zon < NB_ZON_JET; i_zon++ )
	{
		avg_pow += sps_entry[JET].s_mea.cool_pow_pct[i_zon]; //s_coil.s_coil_data[0].pow_pct[JET][i_zon];
	} // For all zones
	avg_pow /= float(NB_ZON_JET);

   // For all zones
   for ( int i_zon=0; i_zon < NB_ZON_JET; i_zon++ )
   {
      sps_fin_sp_jet.pow_pct[i_zon] = avg_pow;

   } // For all zones

	fba_stp[JET].max_exit = 100.f;

	fba_stp[JET].min_exit = 0.f;

   if ( fba_stp[JET].max_exit == 0.f && fba_stp[JET].min_exit == 0.f )
   {
      fba_stp[JET].max_exit = +30.f;
      fba_stp[JET].min_exit = -30.f;
   }

   sps_act_sp[JET].ctl_pow = true;
   // PID calculation
   // ===============
   // If Section in automatic
   if ( sps_entry[JET].s_mea.auto_on ) 
   {
      fba_stp[JET].reset = false;
      status = ns_STD::cl_PID::SPS_PID(&fba_stp[JET]);
      if ( !status )
      {
         sps_act_sp[JET].Valid = false;
         ns_STD::cl_TRC::ERR_Write("SPS", sub_name, "error in SPS_PID");
         return status;
      }

      //sec_pow = max(min( avg_pow + fba_stp[JET].pid_exit, 100.f ),   0.f );	//sps_fin_sp_jet.pow_pct[i_zon] + 
      sec_pow = max(min( fba_stp[JET].pid_exit, 100.f ),   0.f );	//sps_fin_sp_jet.pow_pct[i_zon] + 
	   tot_pow = sec_pow * float(NB_ZON_JET);

      // Calculate power set point
      // -------------------------
		// If fans in Cascade mode
		if (s_fur_mea[JET].fan_mode)
		{
         if ((tot_pow > (50 + hst / 2.f) * nb_zon_on        && nb_zon_on < NB_ZON_JET) ||    // if one more zone to be started
             (tot_pow < (50 - hst / 2.f) * (nb_zon_on - 1)  && nb_zon_on > 1         ) )     // if one zone to be stopped
         {
            nb_zon_on = int(tot_pow / 50.f) + 1;
            nb_zon_on = min(nb_zon_on, NB_ZON_JET);
         }

         // For all zones
         for ( int i_zon=0; i_zon < NB_ZON_JET; i_zon++ )
         {
			   if (i_zon >= NB_ZON_JET - nb_zon_on)
               sps_act_sp[JET].cool_pow_pct[i_zon] = tot_pow / (float)nb_zon_on;
            else
               sps_act_sp[JET].cool_pow_pct[i_zon] = 0;
         } // For all zones
		} // If fans in Cascade mode

		// If fans in Unisson mode
      else
      {
         // For all zones
         for ( int i_zon=0; i_zon < NB_ZON_JET; i_zon++ )
				sps_act_sp[JET].cool_pow_pct[i_zon] = sec_pow;
      } // If fans in Unisson mode

   } // If Section in automatic


   else // If Section not in automatic
   {
	   fba_stp[JET].reset = true;

	   fba_stp[JET].i_exit = avg_pow - (-fba_stp[JET].p * (fba_stp[JET].sp - fba_stp[JET].pv));
	   fba_stp[JET].i_exit = max(fba_stp[JET].i_exit, fba_stp[JET].min_exit);
	   fba_stp[JET].i_exit = min(fba_stp[JET].i_exit, fba_stp[JET].max_exit);
	   for (int i_zon = 0; i_zon < NB_ZON_JET; i_zon++)
	   {
			sps_act_sp[JET].cool_pow_pct[i_zon] = avg_pow;
		}
   } // If Section not in automatic

   return status;
}
