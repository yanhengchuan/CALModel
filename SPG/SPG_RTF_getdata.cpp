/*============================================================================
/ SPG_RTF_Getdata: Recover required information from external device for LTOP 
/     set-point generator purpose.
/ 
/ Revisions:
/  Created:       2005-Aug      JC.Mitais
/  Modified: 
/  
/ (c) CMI Industry
/===========================================================================*/
#include "stdafx.h"

//===========================================================================
bool SPG_RTF_Getdata ()
{
   bool status = true;
   string sub_name = "SPG_RTF_Getdata";

   // Traces treatment
   bool TRC=ns_STD::cl_TRC::TRC_Sub( sub_name );

   if (TRC) ns_STD::cl_TRC::TRC_Write("INF", "SPG", sub_name, "DATA READING SPG RTF.");

   // Check if coil characteristics have changed
   // ==========================================
   // For all coils
   for ( int i_coil=0; i_coil < s_coil.nb_coil; i_coil++ )
   {
		if(sps_entry[RTH].s_mea.spd_auto && s_coil.s_coil_data[i_coil].spd_max > s_coil.s_coil_data[i_coil].spd_max_coil_ope 
			&& s_coil.s_coil_data[i_coil].spd_max_coil_ope > 0.f)	
		{
			 s_coil.s_coil_data[i_coil].spd_max = s_coil.s_coil_data[i_coil].spd_max_coil_ope;
		}
      // Initialize the the modificationn incdex to 0: no modification
      s_coil.s_coil_data[i_coil].change = 0;

      // if coil is valid
      if ( s_coil.s_coil_data[i_coil].valid )
      {
         if ( s_coil.s_coil_data[i_coil].thi          != s_coil.s_coil_data_old[i_coil].thi )                   s_coil.s_coil_data[i_coil].change += 1;
         if ( s_coil.s_coil_data[i_coil].wid          != s_coil.s_coil_data_old[i_coil].wid )                   s_coil.s_coil_data[i_coil].change += 2;     // 3=1+2
         if ( fabs(s_coil.s_coil_data[i_coil].tmp_in[RTH] - s_coil.s_coil_data_old[i_coil].tmp_in[RTH]) > 40.f) s_coil.s_coil_data[i_coil].change += 4;     // 5=1+4, 6=2+4, 7=1+2+4
         if ( s_coil.s_coil_data[i_coil].tmp_tgt[RTH] != s_coil.s_coil_data_old[i_coil].tmp_tgt[RTH] )          s_coil.s_coil_data[i_coil].change += 1*10;
         if ( s_coil.s_coil_data[i_coil].tmp_upp[RTH] != s_coil.s_coil_data_old[i_coil].tmp_upp[RTH] )          s_coil.s_coil_data[i_coil].change += 2*10;  // 30=10+20
         if ( s_coil.s_coil_data[i_coil].tmp_low[RTH] != s_coil.s_coil_data_old[i_coil].tmp_low[RTH] )          s_coil.s_coil_data[i_coil].change += 4*10;  // 50=10+40, 60=20+40, 70=10+20+40
         if ( fabs(s_coil.s_coil_data[i_coil].spd_max  - s_coil.s_coil_data_old[i_coil].spd_max) > 0.5f/60.f ) s_coil.s_coil_data[i_coil].change += 1*100;
			 //if ( s_coil.s_coil_data[i_coil].spd_dec      != s_coil.s_coil_data_old[i_coil].spd_dec &&
			  //   s_coil.s_coil_data[i_coil].spd_dec      != 0.f )													             s_coil.s_coil_data[i_coil].change += 2*100; // 300=100+200
      }

		if (s_coil.s_coil_data[i_coil].change != 0)
		{
         s_coil.s_coil_data[i_coil].change += i_coil*1000;
         ns_STD::cl_TRC::TRC_Write("INF", "SPG", sub_name, "Data change on coil %s. Code : %04d", 
                     s_coil.s_coil_data[i_coil].id.c_str(), s_coil.s_coil_data[i_coil].change);   
		}
   } // For all coils

   // Check if speed strain on first coil
   if ( s_coil.s_coil_data[0].spd_dec != 0.f && 
        s_coil.s_coil_data[0].spd_dec != s_coil.s_coil_data_old[0].spd_dec )
   {
      // Store speed strain as new speed
	   s_coil.s_coil_data[0].spd = s_coil.s_coil_data[0].spd_dec;  
   }

	// Compute transitions if required
	// ===============================
	// For all coils but first one
   for ( int i_coil = 1; i_coil < s_coil.nb_coil && i_coil < SPG_NB_COIL; i_coil++ )             // test //  && i_coil < SPG_NB_COIL
   {
      // For all RTF furnaces
      for (int i_rtf=0; i_rtf < NB_FUR_RTF; i_rtf++) 
      {
         // If any change
         if (  s_coil.s_coil_data[i_coil-1].valid       && s_coil.s_coil_data[i_coil].valid        &&
            ( (s_coil.s_coil_data[i_coil-1].change != 0 || s_coil.s_coil_data[i_coil].change != 0) || !s_coil.s_coil_data[i_coil-1].Trs_cal[i_rtf] ) )
         {
            // For all rolls
            for ( int i_rol=0; i_rol < rol_cmp[i_rtf].nb_roll; i_rol++ )
            {
               // Strip tension [Pa]
               rol_cmp[i_rtf].s_rol[i_rol].s_strip.tension_basis = STRIP_TENSION;
               //rol_cmp[i_rtf].s_rol[i_rol].s_strip.wid = s_coil.s_coil_data[0].wid;
               rol_cmp[i_rtf].s_rol[i_rol].s_strip.wid = s_coil.s_coil_data[i_coil-1].wid;
            } // For all rolls

            int n_cla = s_coil.s_coil_data[i_coil-1].N_cla;
            int n_pdt = s_coil.s_coil_data[i_coil-1].N_pdt;
            s_coil.s_coil_data[i_coil-1].emi_rtf[i_rtf] = rtf_cfg[i_rtf].emi_stp * 
                                                          s_adp[i_rtf].s_pdt_RTF[n_cla][n_pdt].alpha;

            n_cla = s_coil.s_coil_data[i_coil].N_cla;
            n_pdt = s_coil.s_coil_data[i_coil].N_pdt;
            s_coil.s_coil_data[i_coil].emi_rtf[i_rtf] = rtf_cfg[i_rtf].emi_stp * 
                                                        s_adp[i_rtf].s_pdt_RTF[n_cla][n_pdt].alpha;

            ns_STD::cl_TRC::TRC_Write("INF", "SPG", sub_name, "%s: Start compute transition on coils %s --> %s", fur_cfg[RTH + i_rtf].fur_name.c_str(), 
                      s_coil.s_coil_data[i_coil-1].id.c_str(), s_coil.s_coil_data[i_coil].id.c_str());   

            // Perform the stable and transient situation calculation
            status = SPG_RTF( i_coil-1, i_coil, i_rtf );
            if ( !status )
            {
               ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in SPG_RTF coils %s -> %s", fur_cfg[RTH + i_rtf].fur_name.c_str(), 
                         s_coil.s_coil_data[i_coil-1].id.c_str(), s_coil.s_coil_data[i_coil].id.c_str());
            }

				ns_STD::cl_TRC::TRC_Write("INF", "SPG", sub_name, "%s: End compute transition on coils %s --> %s", fur_cfg[RTH + i_rtf].fur_name.c_str(), 
								s_coil.s_coil_data[i_coil-1].id.c_str(), s_coil.s_coil_data[i_coil].id.c_str());

            // Update roll data
            // ================
            int lst_zon = fur_cfg[i_rtf].nb_zon - 1;
            int n_rol_cmp = rol_cfg[i_rtf].n_rol_cmp;
            // [ 1=CQ, 2=DQ, 3=DDQ ]
            rol_cmp[i_rtf].s_rol[n_rol_cmp].s_strip.steel_charac = s_coil.s_coil_data[0].steel_charac;

            // Tube temperature [K]
            rol_cmp[i_rtf].s_rol[n_rol_cmp].s_tub.tmp = s_coil.s_coil_data[0].tmp_zon[i_rtf][lst_zon];

            // Strip temperature [K]
            rol_cmp[i_rtf].s_rol[n_rol_cmp].s_strip.tmp = s_coil.s_coil_data[0].tmp_tgt[i_rtf];

            // Strip width [m]
            rol_cmp[i_rtf].s_rol[n_rol_cmp].s_strip.wid = s_coil.s_coil_data[0].wid; 

            // Strip thickness [m]
            rol_cmp[i_rtf].s_rol[n_rol_cmp].s_strip.thi = s_coil.s_coil_data[0].thi; 

				s_coil.s_coil_data[i_coil-1].Trs_cal[i_rtf] = true;
            if (i_coil == 1) time(&s_cur.time_lst_adapt[i_rtf]);
         } // If any change
      } // For all RTF furnaces
   } // For all coils but first one

   // Save current coil data for next call.
   // =====================================
   // For all coils in queue
   for ( int i_coil=0; i_coil < s_coil.nb_coil; i_coil++ )
   {
      // Store all the coil queue. This is usefull to check if any data has changed for next calculation step
      s_coil.s_coil_data_old[i_coil] = s_coil.s_coil_data[i_coil];
   } // For all coils in queue

   return status;
}
