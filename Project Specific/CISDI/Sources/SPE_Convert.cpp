/*============================================================================
/ SPE_Convert.cpp: Conversion between L-TOP structures and line/furnace HPC structure.
/ 
/ Revisions:
/  Created:       2010-Jan       J. NAVEIRA
/  Modified:      2012-Oct       J.Naveira
/
/ (c) CMI Industry
//===========================================================================*/
#include "stdafx.h"

// *****************************************************************************
// *****************************************************************************
bool MessageSPStoLtop()
{
	bool status = true;
	string sub_name = "MessageSPStoLtop";

	static bool CQ_UPD = true;					// Flag Coils Queue has been Updated

   static float lst_wld_pos = 0;          // memorised previous weld position

   // ********
   // TRACKING
   // ********
   // Line speed
   // ==========
   // For all furnaces
   for (int i_fur=0; i_fur < MAX_NB_FUR; i_fur++) 
   {
      sps_entry[i_fur].s_mea.spd = s_line_trk.Spd_mea / 60.f;
	   sps_entry[i_fur].s_mea.auto_on = s_fur_mea[i_fur].ctl;
	   sps_entry[i_fur].s_mea.spd_auto = s_line_trk.Spd_ctrl;
      //sps_entry[i_fur].s_mea.tim = 10000.f;
   } // For all furnaces

   // If the line is running
	if (sps_entry[RTH].s_mea.spd > 0.f)
   {
		// The weld position sent by level 1 is the distance from the weld detector at the exit of entry looper till
		// the last weld wich had passed this detector. So it is always positive and reset to 0 as soon as the new
		// weld is detected.
		
		if (!SIMULATION)
		{
         // if new weld detected
         if (s_line_trk.weld_pos < 100.f && s_line_trk.weld_pos < lst_wld_pos - 200.f)
         {
            // if the coil appeared to be shorter than expected (5m - accuracy)
            if (lst_wld_pos < s_coil.s_coil_data[0].lgt - 5)
            {
               s_coil.s_coil_data[0].lgt = lst_wld_pos;        // update the length of current coil
               ns_STD::cl_TRC::TRC_Write("WAR", "TRK", sub_name, "Coil %s SHORTer than expected. Detected length: %.0f m", s_coil.s_coil_data[0].id.c_str(), s_coil.s_coil_data[0].lgt);
            }
         }

         lst_wld_pos = s_line_trk.weld_pos;                 // memorise the weld position

         // if the coil appeared to be longer than expected
         if (s_line_trk.weld_pos > s_coil.s_coil_data[0].lgt + 100.f)
         {
            s_coil.s_coil_data[0].lgt = s_line_trk.weld_pos;   // update the length of current coil
            ns_STD::cl_TRC::TRC_Write("WAR", "TRK", sub_name, "Coil %s LONGer than expected. Detected length: %.0f m", s_coil.s_coil_data[0].id.c_str(), s_coil.s_coil_data[0].lgt);
         }

		   // Recalculate weld position related to the entrance of RTH
		   s_line_trk.weld_pos -= PRE_RTH_DST;
		} // if (!SIMULATION)

      // When the detected weld has passed the RTH pyrometer
		if (s_line_trk.weld_pos > fur_cfg[RTH].xPyro)
		{
			// If the received ID has been updated
			if ( s_line_trk.coil_id != s_coil.s_coil_data[0].id && s_line_trk.coil_id == s_coil.s_coil_data[1].id && CQ_UPD )
			{
				// Coils queue needs to be updated
				CQ_UPD = false;
			} // If the received ID has been updated

			// If the coil queue has not been updated yet
			if (!CQ_UPD)
			{
				// Passed coils data update
				for (int i_coil = MAX_NB_COIL - 1; i_coil > 0; i_coil--)
				{
					s_coil.s_coil_data_last[i_coil] = s_coil.s_coil_data_last[i_coil - 1];
				}
				s_coil.s_coil_data_last[0] = s_coil.s_coil_data[0];
				trans_result_old[RTH] = trans_result_rtf[RTH][0];
				trans_result_old[RTS] = trans_result_rtf[RTS][0];

				// Coils queue data update
            if (s_coil.nb_coil > 0)
            {
				   for ( int i_coil=0; i_coil < s_coil.nb_coil - 1; i_coil++ ) 
				   {
					   s_coil.s_coil_data[i_coil] = s_coil.s_coil_data[i_coil + 1];
					   trans_result_rtf[RTH][i_coil] = trans_result_rtf[RTH][i_coil + 1];
					   trans_result_rtf[RTS][i_coil] = trans_result_rtf[RTS][i_coil + 1];
				   }
				   s_coil.s_coil_data[s_coil.nb_coil - 1].thi = 0.f;
				   s_coil.s_coil_data[s_coil.nb_coil - 1].wid = 0.f;
				   s_coil.s_coil_data[s_coil.nb_coil - 1].id = "";
               s_coil.s_coil_data[s_coil.nb_coil - 1].valid = false;

				   // Coils queue old data update
				   for ( int i_coil=0; i_coil < s_coil.nb_coil - 1; i_coil++ ) 
				   {
					   s_coil.s_coil_data_old[i_coil] = s_coil.s_coil_data_old[i_coil + 1];
				   }
				   s_coil.s_coil_data_old[s_coil.nb_coil - 1].thi = 0.f;
				   s_coil.s_coil_data_old[s_coil.nb_coil - 1].wid = 0.f;
				   s_coil.s_coil_data_old[s_coil.nb_coil - 1].id = "";
               s_coil.s_coil_data_old[s_coil.nb_coil - 1].valid = false;

               s_coil.nb_coil--;
            }

				CQ_UPD = true;
			} // If the coil queue has not been updated yet

			// Recalculate the following weld position 
			s_line_trk.weld_pos -= s_coil.s_coil_data[0].lgt;

		} // If the detected weld has passed the RTH pyrometer

		// If  the detected weld has not passed the RTH pyrometer yet
		else if (CQ_UPD)
			CQ_UPD = false;

      // RTF
      // ===
      sps_entry[RTH].s_mea.lgt = fur_cfg[RTH].xPyro - s_line_trk.weld_pos;
      sps_entry[RTH].s_mea.nCurCoil = 0;
      sps_entry[RTH].s_mea.tim = sps_entry[RTH].s_mea.lgt / sps_entry[RTH].s_mea.spd;

      // For the rest of sections
      for (int i_fur=RTH+1; i_fur < MAX_NB_FUR; i_fur++)
         status = SPS_TRK_Section(fur_cfg[i_fur].xPyro, &sps_entry[i_fur].s_mea.nCurCoil,
                           &sps_entry[i_fur].s_mea.lgt, &sps_entry[i_fur].s_mea.tim);

   }  // If the line is running

   // ----------------
   //	SPEED MANAGEMENT
   // ----------------

	// Speed limitation: Maximum operator speed
	s_spe_PRO.s_mea.Spd_max_ope = s_line_trk.Spd_max_ope / 60.f;
	  
	// For all coils
	for ( int i_coil=0; i_coil < s_coil.nb_coil; i_coil++ )
	{
		s_coil.s_coil_data[i_coil].spd_max = (s_coil.s_coil_data[i_coil].spd_max_rec + s_coil.s_coil_data[i_coil].spd_min_rec)/2.f;
		s_coil.s_coil_data[i_coil].spd_min = s_coil.s_coil_data[i_coil].spd_min_rec;
		s_coil.s_coil_data[i_coil].spd_dec= 0.f;
		if (s_spe_PRO.s_mea.Spd_max_ope > 0.f)
      {
			s_coil.s_coil_data[i_coil].spd_max_coil_ope = s_spe_PRO.s_mea.Spd_max_ope;
		   s_coil.s_coil_data[i_coil].spd_max = min (s_coil.s_coil_data[i_coil].spd_max, 
                                                   s_spe_PRO.s_mea.Spd_max_ope); /* Speed lim. operator */

			s_coil.s_coil_data[i_coil].spd_min = min (s_coil.s_coil_data[i_coil].spd_min, 
                                                   s_spe_PRO.s_mea.Spd_max_ope - 5.f/60.f);
      }
		// if speed set in manual
		if ( !sps_entry[RTH].s_mea.spd_auto )
      {
			// If first coil in queue
			if (i_coil < s_coil.nb_coil) 
			{
				if (sps_entry[RTH].s_mea.spd > MIN_SPEED)
				{
					s_coil.s_coil_data[i_coil].spd_max = sps_entry[RTH].s_mea.spd;
					s_coil.s_coil_data[i_coil].spd	  = sps_entry[RTH].s_mea.spd;
					s_coil.s_coil_data[i_coil].spd_dec = sps_entry[RTH].s_mea.spd;
				}

				else
				{
					s_coil.s_coil_data[i_coil].spd	  = s_coil.s_coil_data[i_coil].spd_max;
					s_coil.s_coil_data[i_coil].spd_dec = s_coil.s_coil_data[i_coil].spd_max;
				}

				s_coil.s_coil_data[i_coil].spd_min = min ( s_coil.s_coil_data[i_coil].spd_min, 
                                                       s_coil.s_coil_data[i_coil].spd_max );
			} // If first coil in queue
		} // speed in manuel
	} // For all coils

   // ************
	// MEASUREMENTS
	// ************
   // For all RTF furnaces
   for (int i_rtf=0; i_rtf < NB_FUR_RTF; i_rtf++) 
   {
      // RTF strip tmp. mea.
      sps_entry[i_rtf].s_mea.tmp_stp = s_fur_mea[i_rtf].stp_tmp + TK;		

	   // For all RTF zones, including soaking zones
      for (int i_zon=0; i_zon < fur_cfg[i_rtf].nb_zon; i_zon++)
	   {
		   sps_entry[i_rtf].s_mea.on_zone[i_zon] = true;

		   // RTF power
         sps_entry[i_rtf].s_mea.heat_pow_pct[i_zon] = s_fur_mea[i_rtf].heat_pow_pct[i_zon];
		
		   // RTF tube temperature
         if (  s_fur_mea[i_rtf].zon_tmp[i_zon] + TK > TUB_MAX_TMP + 50.f || s_fur_mea[i_rtf].zon_tmp[i_zon] < 500.f )
            sps_entry[i_rtf].s_mea.tmp_zon[i_zon] = 700.f + TK;
         else
            sps_entry[i_rtf].s_mea.tmp_zon[i_zon] = s_fur_mea[i_rtf].zon_tmp[i_zon] + TK;
	   } // For all RTF zones, including soaking zones

      // RTF entry temperature                                                         // test
      if (i_rtf == 0)
         fur_cfg[i_rtf].entry_tmp = PRE_HEA_STP_TMP;                                   // test
      else
         fur_cfg[i_rtf].entry_tmp = s_coil.s_coil_data[0].tmp_tgt[i_rtf-1];            // test

   } // For all RTF furnaces

   // ---
	// SLW 
   // ---
   // If a SLW
   if (NB_FUR_SLW > 0)
   {
      sps_entry[SLW].s_mea.tmp_stp = s_fur_mea[SLW].stp_tmp + TK;

      // For all zones
      for (int i_zon=0; i_zon < NB_ZON_SLW; i_zon++)
      {
         sps_entry[SLW].s_mea.cool_pow_pct[i_zon] = s_fur_mea[SLW].cool_pow_pct[i_zon];
         sps_entry[SLW].s_mea.tmp_zon[i_zon] = s_fur_mea[SLW].zon_tmp[i_zon] + TK;
      } // For all zones

      //// SPECIFIC YIEH - PHUI / CISDI - Beg
      //s_fur_mea[SLW].cool_pow_pct[0] = 0.f;
      //// For all fans
      //for (int i_fan=0; i_fan < 3; i_fan++)
      //{
      //   s_fur_mea[SLW].cool_pow_pct[0] += s_fur_mea[SLW].s_spe.slw_fan_spd[i_fan];
      //} // For all fans
      //s_fur_mea[SLW].cool_pow_pct[0] /= 3.f;
      //// SPECIFIC YIEH - PHUI / CISDI - End
   } // If a SLW

   // ---
	// JET 
   // ---
   // If a JET
   if (NB_FUR_JET > 0)
   {
	   sps_entry[JET].s_mea.tmp_stp = s_fur_mea[JET].stp_tmp + TK;

      // For all zones
      for (int i_zon=0; i_zon < NB_ZON_JET; i_zon++)
      {
         sps_entry[JET].s_mea.cool_pow_pct[i_zon] = s_fur_mea[JET].cool_pow_pct[i_zon];
		   sps_entry[JET].s_mea.heat_pow_pct[i_zon] = s_fur_mea[JET].heat_pow_pct[i_zon];
		   sps_entry[JET].s_mea.tmp_zon[i_zon] = s_fur_mea[JET].zon_tmp[i_zon] + TK;
      } // For all zones
   } // If a JET

   // ---
	// OVG 
   // ---
   // If a OVG
   if (NB_FUR_OVG > 0)
   {
      sps_entry[OVG].s_mea.tmp_stp = s_fur_mea[OVG].stp_tmp + TK;
      
      // For all zones
	   for (int i_zon=0; i_zon < NB_ZON_OVG; i_zon++)
	   {
         sps_entry[OVG].s_mea.heat_pow_pct[i_zon] = s_fur_mea[OVG].heat_pow_pct[i_zon];
         sps_entry[OVG].s_mea.tmp_zon[i_zon] = s_fur_mea[OVG].zon_tmp[i_zon] + TK;
      } // For all zones
   } // If a OVG

   // ---
	// FCS 
   // ---
   // If a FCS
   if (NB_FUR_FCS > 0)
   {
      sps_entry[FCS].s_mea.tmp_stp = s_fur_mea[FCS].stp_tmp + TK;

      // For all zones
	   for (int i_zon=0; i_zon < NB_ZON_FCS; i_zon++)
	   {
         sps_entry[FCS].s_mea.cool_pow_pct[i_zon] = s_fur_mea[FCS].cool_pow_pct[i_zon];
      } // For all zones
   } // If a FCS

   // If a EXT
   if (NB_FUR_EXT > 0)
   {
      sps_entry[EXT].s_mea.tmp_stp = s_fur_mea[EXT].stp_tmp + TK;
      for (int i_zon=0; i_zon < NB_ZON_EXT; i_zon++)
         sps_entry[EXT].s_mea.tmp_zon[i_zon] = s_fur_mea[EXT].zon_tmp[i_zon] + TK;
   } // If a EXT

   // If a APC
   if (NB_FUR_APC > 0)
   {
      sps_entry[APC].s_mea.tmp_stp = s_fur_mea[APC].stp_tmp + TK;
      for (int i_zon=0; i_zon < NB_ZON_APC; i_zon++)
         sps_entry[APC].s_mea.tmp_zon[i_zon] = s_fur_mea[APC].zon_tmp[i_zon] + TK;
   } // If a APC
	
	return status;
} // void

// *****************************************************************************
// Coil queue
// *****************************************************************************
bool MessageSPGtoLtop()
{
	bool status = true;
	string sub_name = "MessageSPGtoLtop";
	
	int delta = 0;		// error between level 1 coils queue and LTOP coil queue

	// if the weld is around the end of RTH
	if (abs(s_line_trk.weld_pos - PRE_RTH_DST - fur_cfg[RTH].xPyro) < 100
		// and the current coil ID received from line corresponds to the past or the next coil
		&& (s_line_trk.coil_id == s_coil.s_coil_data_last[0].id || s_line_trk.coil_id == s_coil.s_coil_data[1].id || s_coil.s_coil_data_last[0].id == "")
		// or if the first coil in L-TOP is not correct
		&& s_coil.s_coil_data[0].id != "" && s_coil.s_coil_data[0].id != " ")
	{
		// search for the coil corresponding to first coil in L-TOP
		// for all coils
		for ( int i_coil=0; i_coil < s_coil.nb_coil; i_coil++ )
		{
			if (s_line_coil[i_coil].id == s_coil.s_coil_data[0].id)
					delta = i_coil;
		} // For all coils
	}
	// in all other cases
	else
	{
		// search for the coil corresponding to the current coil ID received from line
		// for all coils
		for ( int i_coil=0; i_coil < s_coil.nb_coil; i_coil++ )
		{
			if (s_line_coil[i_coil].id == s_line_trk.coil_id)
					delta = i_coil;
		} // For all coils
	} // in all other cases

   s_coil.nb_coil -= delta;

	// For new coils
	for ( int i_coil=0; i_coil < s_coil.nb_coil; i_coil++ )
	{
		s_coil.s_coil_data[i_coil].id  = s_line_coil[i_coil + delta].id;  // Coil Identity
      s_coil.s_coil_data[i_coil].thi = s_line_coil[i_coil + delta].thi;	// strip thickness
		s_coil.s_coil_data[i_coil].lgt = s_line_coil[i_coil + delta].lgt; // strip length
      s_coil.s_coil_data[i_coil].wid = s_line_coil[i_coil + delta].wid;	// strip width
		s_coil.s_coil_data[i_coil].Bisra = 2;        // Bisra code

      // For all furnaces
      for (int i_fur=0; i_fur < MAX_NB_FUR; i_fur++)
      {
         s_coil.s_coil_data[i_coil].tmp_tgt[i_fur] = s_line_coil[i_coil + delta].tmp_tgt[i_fur] + TK;
         s_coil.s_coil_data[i_coil].tmp_upp[i_fur] = s_line_coil[i_coil + delta].tmp_upp[i_fur] + TK;
         s_coil.s_coil_data[i_coil].tmp_low[i_fur] = s_line_coil[i_coil + delta].tmp_low[i_fur] + TK;

			// If RTF
         if (i_fur < RTH + NB_FUR_RTF)
			{
		      s_coil.s_coil_data[i_coil].emi_rtf[i_fur - RTH] = UTL_Emi_RTF(i_coil, i_fur - RTH);   // Emissivity  !!!!!!!!
			} // If RTF

			if (s_coil.s_coil_data[i_coil].tmp_upp[i_fur] < s_coil.s_coil_data[i_coil].tmp_tgt[i_fur] + 5.f)
				 s_coil.s_coil_data[i_coil].tmp_upp[i_fur] = s_coil.s_coil_data[i_coil].tmp_tgt[i_fur] + 10.f;

			if (s_coil.s_coil_data[i_coil].tmp_low[i_fur] > s_coil.s_coil_data[i_coil].tmp_tgt[i_fur] - 5.f)
				 s_coil.s_coil_data[i_coil].tmp_low[i_fur] = s_coil.s_coil_data[i_coil].tmp_tgt[i_fur] - 10.f;
      } // For all furnaces

      s_coil.s_coil_data[i_coil].s_spe.cycle = s_line_coil[i_coil + delta].cycle;
		s_coil.s_coil_data[i_coil].spd_max     = s_line_coil[i_coil + delta].spd_max / 60.f;   /* Maximum speed */
		s_coil.s_coil_data[i_coil].spd_max_rec = s_line_coil[i_coil + delta].spd_max / 60.f;   /* Maximum speed received to be stored */
		s_coil.s_coil_data[i_coil].spd_min     = s_line_coil[i_coil + delta].spd_min / 60.f;   /* Minimum speed */
		s_coil.s_coil_data[i_coil].spd_max = (s_coil.s_coil_data[i_coil].spd_max_rec + s_coil.s_coil_data[i_coil].spd_min)/2.f;
		s_coil.s_coil_data[i_coil].spd_min_rec = s_coil.s_coil_data[i_coil].spd_min;
		s_coil.s_coil_data[i_coil].dummy       = s_line_coil[i_coil + delta].dummy;
		s_coil.s_coil_data[i_coil].steeltype   = s_line_coil[i_coil + delta].steeltype;

		// For all furnaces
      for (int i_fur=0; i_fur < MAX_NB_FUR; i_fur++)
      {			
		// If RTF
         if (i_fur < RTH + NB_FUR_RTF)
			{
		      s_coil.s_coil_data[i_coil].emi_rtf[i_fur - RTH] = UTL_Emi_RTF(i_coil, i_fur - RTH);   // Emissivity  !!!!!!!!
			} // If RTF

		}
      // entry temperature
		s_coil.s_coil_data[i_coil].tmp_in[0] = s_fur_mea[RTH].pre_hea_stp_tmp + TK;  
      for (int i_rtf=1; i_rtf < NB_FUR_RTF; i_rtf++) 
      {
         s_coil.s_coil_data[i_coil].tmp_in[i_rtf] = s_coil.s_coil_data[i_coil].tmp_tgt[i_rtf - 1];
      } 
	} // For new coils

	// Detect empty coil data
	for ( int i_coil=0; i_coil < s_coil.nb_coil; i_coil ++)
	{
		if(s_coil.s_coil_data[i_coil].id == "Empty")
		{
			s_coil.s_coil_data[i_coil].wid = 0.f;
			s_coil.s_coil_data[i_coil].thi = 0.f;
		}
	}

	// ------------------
	// COIL DATA VALIDITY
	// ------------------
   // For new coils
	for ( int i_coil=0; i_coil < s_coil.nb_coil; i_coil ++)
	{
		// Check minimum speed regarding maximum speed
		s_coil.s_coil_data[i_coil].spd_min = min(s_coil.s_coil_data[i_coil].spd_min, 
                                               s_coil.s_coil_data[i_coil].spd_max);

      s_coil.s_coil_data[i_coil].valid = true;
		// Update coil validity
		if ( (s_coil.s_coil_data[i_coil].spd_max < 0.f && sps_entry[RTH].s_mea.spd != 0) ||
            s_coil.s_coil_data[i_coil].wid <= 0.f ||
		      s_coil.s_coil_data[i_coil].thi <= 0.f )
      {
         ns_STD::cl_TRC::ERR_Write("TRK", sub_name, "Coil %s data is not valid", s_coil.s_coil_data[i_coil].id.c_str());
         s_coil.s_coil_data[i_coil].valid = false;
         for (int i_rtf=0; i_rtf < NB_FUR_RTF; i_rtf++) s_coil.s_coil_data[i_coil].Trs_cal[i_rtf] = false;
      }

      // For all sections
      for (int i_fur=0; i_fur < MAX_NB_FUR; i_fur++)
      {
         
			if (s_coil.s_coil_data[i_coil].tmp_upp[i_fur] < s_coil.s_coil_data[i_coil].tmp_tgt[i_fur] + 5.f ||
             s_coil.s_coil_data[i_coil].tmp_upp[i_fur] > s_coil.s_coil_data[i_coil].tmp_tgt[i_fur] + 100.f)
				 s_coil.s_coil_data[i_coil].tmp_upp[i_fur] = s_coil.s_coil_data[i_coil].tmp_tgt[i_fur] + 10.f;

			if (s_coil.s_coil_data[i_coil].tmp_low[i_fur] > s_coil.s_coil_data[i_coil].tmp_tgt[i_fur] - 5.f ||
             s_coil.s_coil_data[i_coil].tmp_low[i_fur] < s_coil.s_coil_data[i_coil].tmp_tgt[i_fur] - 100.f)
				 s_coil.s_coil_data[i_coil].tmp_low[i_fur] = s_coil.s_coil_data[i_coil].tmp_tgt[i_fur] - 10.f;
      } // For all sections

   } // For new coils

   // For missing coils
	//for (int i_coil=s_coil.nb_coil - delta; i_coil < s_coil.nb_coil; i_coil++)
	for (int i_coil = 0; i_coil < delta; i_coil++)                                // TO BE CHECKED !!!
	{
		s_coil.s_coil_data[s_coil.nb_coil + i_coil].thi = 0.f;
		s_coil.s_coil_data[s_coil.nb_coil + i_coil].wid = 0.f;
		s_coil.s_coil_data[s_coil.nb_coil + i_coil].id = "";
	} // For missing coils

   // For all coils
   for ( int i_coil=0; i_coil < s_coil.nb_coil; i_coil++ )
   {
		s_coil.s_coil_data[i_coil].spd     = min(max(s_coil.s_coil_data[i_coil].spd,     MIN_SPEED) , MAX_SPEED);
		s_coil.s_coil_data[i_coil].spd_max = min(max(s_coil.s_coil_data[i_coil].spd_max, MIN_SPEED) , MAX_SPEED);
		s_coil.s_coil_data[i_coil].spd_min = min(max(s_coil.s_coil_data[i_coil].spd_min, MIN_SPEED) , MAX_SPEED);
	} // For all coils

	//RTS treatment
	//if going hotter
	if(s_coil.s_coil_data[1].tmp_tgt[RTS] >  s_coil.s_coil_data[0].tmp_tgt[RTS] + 5.f)
	{
		s_coil.s_coil_data[0].tmp_tgt[RTS] = s_coil.s_coil_data[0].tmp_upp[RTS] - 2.f;
	}// if going hotter
	// else if going colder
	else if(s_coil.s_coil_data[1].tmp_tgt[RTS] <  s_coil.s_coil_data[0].tmp_tgt[RTS] - 5.f)
	{
		s_coil.s_coil_data[0].tmp_tgt[RTS] = s_coil.s_coil_data[0].tmp_low[RTS] + 2.f;
	}

	// if going thin cold to thick hot increase upper limit of next lower limit + 10
	if(s_coil.s_coil_data[1].tmp_tgt[RTH] >  s_coil.s_coil_data[0].tmp_tgt[RTH] + 5.f
		&& s_coil.s_coil_data[1].thi > s_coil.s_coil_data[0].thi * 1.02f && s_coil.s_coil_data[1].dummy == 0)
	{
		s_coil.s_coil_data[0].tmp_upp[RTH] = max(s_coil.s_coil_data[0].tmp_upp[RTH] + 10.f , s_coil.s_coil_data[1].tmp_low[RTH] + 10.f);
	}

	else if(s_coil.s_coil_data[1].tmp_upp[RTH] <= s_coil.s_coil_data[0].tmp_low[RTH])
	{
		s_coil.s_coil_data[1].tmp_upp[RTH] += 6.f;
	}
	/*else if(s_coil.s_coil_data[0].tmp_upp[RTH] <= s_coil.s_coil_data[1].tmp_low[RTH])
	{
		s_coil.s_coil_data[0].tmp_upp[RTH] += 10.f;
		s_coil.s_coil_data[1].tmp_low[RTH] -= 2.f;
	}
*/
   // =====================
   // Dummy Coils Treatment
   // ===================== 
   int i_dummy = -1; // ?
   int i_reg = -1; // ?
   // For all coils
   for (int i_coil = 0; i_coil < s_coil.nb_coil; i_coil++)
   {
	   // if it is first dummy coil (in the queue or after the regular ones)
	   if (s_coil.s_coil_data[i_coil].dummy == 1 && i_coil > i_reg)
	   {
		   i_dummy = i_coil;
		   for (int i = i_dummy + 1; i < s_coil.nb_coil; i++)
		   {
			   if (s_coil.s_coil_data[i].dummy == 0)
			   {
				   i_reg = i;
				   break;
			   }
		   }

		   // if there is a regular coil in the queue following the dummy
		   if (i_reg > i_dummy)
		   {
			   // for all dummy coils until next regular
			   for (int i = i_dummy; i < i_reg; i++)
			   {
				   for (int i_fur = 0; i_fur < MAX_NB_FUR; i_fur++)
				   {
					   s_coil.s_coil_data[i].tmp_tgt[i_fur] = s_coil.s_coil_data[i_reg].tmp_tgt[i_fur];
					   s_coil.s_coil_data[i].tmp_low[i_fur] = s_coil.s_coil_data[i_reg].tmp_low[i_fur] - 100.f;
					   s_coil.s_coil_data[i].tmp_upp[i_fur] = s_coil.s_coil_data[i_reg].tmp_upp[i_fur] + 100.f;
				   }
			   } // for all dummy coils until next regular
		   } // if there is a regular coil in the queue following the dummy

		   // if there are no regular coils in the queue following the dummy
		   else
		   {
			   // for the rest of coils in the queue
			   for (int i = i_dummy; i < s_coil.nb_coil; i++)
			   {
				   for (int i_fur = 0; i_fur < MAX_NB_FUR; i_fur++)
				   {
					   /*if (i > 0)
					   {*/
						   s_coil.s_coil_data[i].tmp_tgt[i_fur] = s_coil.s_coil_data[i].tmp_tgt[i_fur];//s_coil.s_coil_data[i - 1].tmp_tgt[i_fur];
						   s_coil.s_coil_data[i].tmp_low[i_fur] = s_coil.s_coil_data[i].tmp_low[i_fur];//s_coil.s_coil_data[i - 1].tmp_low[i_fur];
						   s_coil.s_coil_data[i].tmp_upp[i_fur] = s_coil.s_coil_data[i].tmp_upp[i_fur];//s_coil.s_coil_data[i - 1].tmp_upp[i_fur];
					   //}
					   //else if (i == 0 && s_coil.s_coil_data_last[0].tmp_tgt[RTH] > 500.f)
					   //{
						  // s_coil.s_coil_data[i].tmp_tgt[i_fur] = s_coil.s_coil_data_last[0].tmp_tgt[i_fur];
						  // s_coil.s_coil_data[i].tmp_low[i_fur] = s_coil.s_coil_data_last[0].tmp_low[i_fur];
						  // s_coil.s_coil_data[i].tmp_upp[i_fur] = s_coil.s_coil_data_last[0].tmp_upp[i_fur];
					   //}
				   }
			   } // for the rest of coils in the queue

			   i_reg = s_coil.nb_coil;
		   } // if there are no regular coils in the queue following the dummy

	   } // if it is first dummy coil

	} // For all coils

	// For all coils
   for (int i_coil = 0; i_coil < s_coil.nb_coil; i_coil++)
   {  // For all sections
      for (int i_fur=0; i_fur < MAX_NB_FUR; i_fur++)
      {
         // if furnace
         if (i_fur < RTH + NB_FUR_RTF)
         {            		
				if ( s_coil.s_coil_data[i_coil].tmp_tgt[i_fur] <= 300.f + TK )						
				{							
					ns_STD::cl_TRC::ERR_Write("TRK", sub_name, "Coil %s %s tmp_tgt is not valid: %.0f �C",								
						s_coil.s_coil_data[i_coil].id.c_str(), fur_cfg[i_fur].fur_name.c_str(), s_coil.s_coil_data[i_coil].tmp_tgt[i_fur]-TK);
							
					s_coil.s_coil_data[i_coil].valid = false;						
					s_coil.s_coil_data[i_coil].Trs_cal[i_fur] = false;
						
				}
         } // if furnace
			else
         {
			   if (s_coil.s_coil_data[i_coil].tmp_tgt[i_fur] <=  50.f)
            {
               ns_STD::cl_TRC::TRC_Write("WAR", "TRK", sub_name, "Coil %s %s tmp_tgt is not valid: %.0f �C",
                  s_coil.s_coil_data[i_coil].id.c_str(), fur_cfg[i_fur].fur_name.c_str(), s_coil.s_coil_data[i_coil].tmp_tgt[i_fur]-TK);

               if (i_fur == SLW)
				      s_coil.s_coil_data[i_coil].tmp_tgt[i_fur] = 599.f;      // clarify !!!
               else if (i_fur == JET)
				      s_coil.s_coil_data[i_coil].tmp_tgt[i_fur] = 379.f;      // clarify !!!
               else if (i_fur == OVG)
				      s_coil.s_coil_data[i_coil].tmp_tgt[i_fur] = 339.f;      // clarify !!!
               else if (i_fur == FCS)
				      s_coil.s_coil_data[i_coil].tmp_tgt[i_fur] = 139.f;      // clarify !!!
            }
         }
		}
	}	

	int idx_dum = 0;
	int ini_dum = 0;
	bool first_dum = true;
	for (int i = 0; i < 10; i++)
	{
		if (s_coil.s_coil_data[i].dummy == 1)
		{
			idx_dum++;
			if (first_dum)
			{
				ini_dum = i;
				first_dum = false;
			}
		}
	}
	if (idx_dum >= 9)
	{
		s_coil.s_coil_data[ini_dum].dummy = 0;
	}
	
	return status;
} // void

// ====================================
// Setpoints        L-TOP   ---->   PLC
// ====================================
bool MessageFromLtop()
{
	bool status = true;
	string sub_name = "MessageFromLtop";

	bool valid = true; 

   // RTH
   // ---
	float avg_tub_tmp = 0.f;

   if ( sps_act_sp[RTH].tmp_stp < 500.f + TK ||
         sps_act_sp[RTH].tmp_stp > 990.f + TK )
   {
      ns_STD::cl_TRC::ERR_Write(fur_cfg[RTH].fur_name.c_str(), sub_name, "Strip target tmp is not valid: %.0f �C", sps_act_sp[RTH].tmp_stp - TK);
      valid = false; 
   }
   
   // For all zones
	for (int i_zon=0; i_zon < NB_ZON_RTH; i_zon++ )
	{
      avg_tub_tmp += sps_act_sp[RTH].tmp_zon[i_zon];

      if ( sps_act_sp[RTH].tmp_zon[i_zon] < 400.f + TK ||
            sps_act_sp[RTH].tmp_zon[i_zon] > TUB_MAX_TMP + 10.f )
      {
         ns_STD::cl_TRC::ERR_Write(fur_cfg[RTH].fur_name.c_str(), sub_name, "Tmp tube Z%d is not valid: %.0f �C", i_zon, sps_act_sp[RTH].tmp_zon[i_zon] - TK);
         valid = false; 
      }

      if ( sps_act_sp[RTH].heat_pow_pct[i_zon] < 0.f ||
            sps_act_sp[RTH].heat_pow_pct[i_zon] > 101.f )
      {
         ns_STD::cl_TRC::ERR_Write(fur_cfg[RTH].fur_name.c_str(), sub_name, "Pwr tube Z%d is not valid: %.0f", i_zon, sps_act_sp[RTH].heat_pow_pct[i_zon]);
         valid = false; 
      }
	} // For all zones

	avg_tub_tmp /= float(NB_ZON_RTH);
	if ( avg_tub_tmp < 600.f + TK )
   {
      ns_STD::cl_TRC::ERR_Write(fur_cfg[RTH].fur_name.c_str(), sub_name, "Average Pwr tube is not valid: %.0f �C", avg_tub_tmp - TK);
      valid = false; 
   }

   if (!valid)
	{
      sps_act_sp[RTH].Spd_Valid = false;
      sps_act_sp[RTH].Valid = false;
	} // if (!valid)

	// =================================
	// Store set points
	// =================================
	// For all furnaces
   for (int i_fur=0; i_fur < MAX_NB_FUR; i_fur++)
   {
	   s_fur_spt[i_fur].valid   = sps_act_sp[i_fur].Valid;
	   s_fur_spt[i_fur].ctl_mod = sps_act_sp[i_fur].ctl_pow;
	   s_fur_spt[i_fur].stp_tmp = sps_act_sp[i_fur].tmp_stp - TK;

      // For all zones
      for (int i_zon=0; i_zon < fur_cfg[i_fur].nb_zon; i_zon++)
	   {
		   s_fur_spt[i_fur].zon_tmp[i_zon]      = sps_act_sp[i_fur].tmp_zon[i_zon] - TK;
		   s_fur_spt[i_fur].heat_zon_pow[i_zon] = sps_act_sp[i_fur].heat_pow_pct[i_zon];
		   s_fur_spt[i_fur].cool_zon_pow[i_zon] = sps_act_sp[i_fur].cool_pow_pct[i_zon];
	   } // For all zones
   } // For all furnaces

   // Line data security
	if (sps_act_sp[RTH].spd < MIN_SPEED || sps_act_sp[RTH].spd > MAX_SPEED)
   {
      ns_STD::cl_TRC::ERR_Write(fur_cfg[RTH].fur_name.c_str(), sub_name, "Speed is not valid: %.0f m/min", sps_act_sp[RTH].spd * 60.f);
      for (int i_fur = 0; i_fur < MAX_NB_FUR; i_fur++)
      {
	      sps_act_sp[i_fur].Spd_Valid = false;
	      sps_act_sp[i_fur].Valid = false;
      }
   }

	if (sps_entry[RTH].s_mea.spd < MIN_SPEED)
	{
      for (int i_fur = RTH + NB_FUR_RTF; i_fur < MAX_NB_FUR; i_fur++)
      {
	      sps_act_sp[i_fur].Valid = false;
      }
	}

	for (int i_fur = 0; i_fur < MAX_NB_FUR; i_fur++)
	{
		s_fur_spt[i_fur].valid = sps_act_sp[i_fur].Valid;
	}

	if (!sps_entry[RTH].s_mea.auto_on)
	{
		sps_act_sp[RTH].Spd_Valid = false;
	}

   // Line data
	sps_act_sp[RTH].spd = min(max(sps_act_sp[RTH].spd, MIN_SPEED), MAX_SPEED);
	s_line_spt.Spd_spt   = sps_act_sp[RTH].spd * 60.f;
	s_line_spt.spt_valid = sps_act_sp[RTH].Spd_Valid;
   	
   status = PostThreadMessage(HandleSQLThread, MSG_SQL_FUR_SPT, 0, 0) == 1;//SQL2_I_MSG_SPT_HEA();
	status = PostThreadMessage(HandleSQLThread, MSG_SQL_LIN_SPT, 0, 0) == 1;//SQL2_I_MSG_SPT_SPD();

	return status;
} // void

// *********************************************
// Updates the weld location for any section SEC
// *********************************************
bool SPS_TRK_Section(float x_Pyro, int *nCurCoil, float *lgt, float *tim)
{
   // At first consider that the same coil is under RTF & SEC pyrometers
   // ------------------------------------------------------------------
   *nCurCoil = -1;

   // Tail weld of the coil
   float x_coil = s_line_trk.weld_pos;

   // If the front weld of the current rtf coil is before the SEC pyrometer (==> different coils under RTF & SEC pyrometers)
   if ( x_coil + s_coil.s_coil_data[0].lgt < x_Pyro )
   {
      // Add tge current rtf coil length to the tail weld
      x_coil += s_coil.s_coil_data[0].lgt;
      // Update the number of coil last passed RTH
      *nCurCoil = 0;

      // Search which one of the past coils is actually under SEC pyrometer
      // ------------------------------------------------------------------
      // For all past coils
      for (int i_coil=0; i_coil < MAX_NB_COIL; i_coil++)                   // s_coil.nb_coil // test
      {
         // If the front weld of the past coil is beyond the SEC pyrometer
         if ( x_coil + s_coil.s_coil_data_last[i_coil].lgt > x_Pyro )
         {
            *nCurCoil = i_coil;
            break;
         } // If the front weld of the coil is beyond the SEC pyrometer
         else
            // Add tge length of the past coil to the tail weld
            x_coil += s_coil.s_coil_data_last[i_coil].lgt;
      } // For all past coils
   } // If the front weld of the current rtf coil is before the SEC pyrometer

   // Remaining length of the coil before the SEC pyrometer
   *lgt = x_Pyro - x_coil;
   *tim = *lgt / sps_entry[RTH].s_mea.spd;

   return true;
} // void

// ========================================
// Find coil emissivity and classes
// ========================================
float UTL_Emi_RTF (int N_coil, int n_fur)
{
   float emi;
	int i_cla, i_pdt;

	s_coil.s_coil_data[N_coil].N_pdt = 0;

    if ( s_coil.s_coil_data[N_coil].thi < 0.0007f)
   { 
      i_cla = 0;
		s_coil.s_coil_data[N_coil].Steel_code = "DQ";
		if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] >= 810.f + TK)
      {
         i_pdt = 0;
      }
		else if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] >= 790.f + TK)
      {
         i_pdt = 1;
      }
      else if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] > 760.f + TK && s_coil.s_coil_data[N_coil].tmp_tgt[RTH] <= 790.f + TK)
      {
         i_pdt = 2;
      }
      else if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] > 720.f + TK && s_coil.s_coil_data[N_coil].tmp_tgt[RTH] <= 760.f + TK)
      {
         i_pdt = 3;
      }
      else if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] > 650.f + TK && s_coil.s_coil_data[N_coil].tmp_tgt[RTH] <= 720.f + TK)
      {
         i_pdt = 4;
      }
      else if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] <= 650.f + TK )
      {
         i_pdt = 5;
      }
      if (s_coil.s_coil_data[N_coil].dummy == 1)
      {
         i_pdt = 6;
      }

   }
   else if ( s_coil.s_coil_data[N_coil].thi < 0.001f)
   { 
		s_coil.s_coil_data[N_coil].Steel_code = "CQ";

      i_cla = 1;
		if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] >= 810.f + TK)
      {
         i_pdt = 0;
      }
      else if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] >= 790.f + TK)
      {
         i_pdt = 1;
      }
      else if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] > 760.f + TK && s_coil.s_coil_data[N_coil].tmp_tgt[RTH] <= 790.f + TK)
      {
         i_pdt = 2;
      }
      else if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] > 720.f + TK && s_coil.s_coil_data[N_coil].tmp_tgt[RTH] <= 760.f + TK)
      {
         i_pdt = 3;
      }
      else if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] > 650.f + TK && s_coil.s_coil_data[N_coil].tmp_tgt[RTH] <= 720.f + TK)
      {
         i_pdt = 4;
      }
      else if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] <= 650.f + TK )
      {
         i_pdt = 5;
      }
      if (s_coil.s_coil_data[N_coil].dummy == 1)
      {
         i_pdt = 6;
      }

   }
   else if ( s_coil.s_coil_data[N_coil].thi < 0.0015f)
   { 
		s_coil.s_coil_data[N_coil].Steel_code = "DDQ";
      i_cla = 2;
		if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] >= 810.f + TK)
      {
         i_pdt = 0;
      }
      else if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] >= 790.f + TK)
      {
         i_pdt = 1;
      }
      else if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] > 760.f + TK && s_coil.s_coil_data[N_coil].tmp_tgt[RTH] <= 790.f + TK)
      {
         i_pdt = 2;
      }
      else if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] > 720.f + TK && s_coil.s_coil_data[N_coil].tmp_tgt[RTH] <= 760.f + TK)
      {
         i_pdt = 3;
      }
      else if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] > 650.f + TK && s_coil.s_coil_data[N_coil].tmp_tgt[RTH] <= 720.f + TK)
      {
         i_pdt = 4;
      }
      else if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] <= 650.f + TK )
      {
         i_pdt = 5;
      }     
      if (s_coil.s_coil_data[N_coil].dummy == 1)
      {
         i_pdt = 6;
      }

   }
   else if ( s_coil.s_coil_data[N_coil].thi < 0.0019f)
   { 
		s_coil.s_coil_data[N_coil].Steel_code = "EDDQ";
      i_cla = 3;
		if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] >= 810.f + TK)
      {
         i_pdt = 0;
      }
      else if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] >= 790.f + TK)
      {
         i_pdt = 1;
      }
      else if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] > 760.f + TK && s_coil.s_coil_data[N_coil].tmp_tgt[RTH] <= 790.f + TK)
      {
         i_pdt = 2;
      }
      else if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] > 720.f + TK && s_coil.s_coil_data[N_coil].tmp_tgt[RTH] <= 760.f + TK)
      {
         i_pdt = 3;
      }
      else if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] > 650.f + TK && s_coil.s_coil_data[N_coil].tmp_tgt[RTH] <= 720.f + TK)
      {
         i_pdt = 4;
      }
      else if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] <= 650.f + TK )
      {
         i_pdt = 5;
      }
     
      if (s_coil.s_coil_data[N_coil].dummy == 1)
      {
         i_pdt = 6;
      }
   }   
   else if ( s_coil.s_coil_data[N_coil].thi < 0.0023f)
   { 
		s_coil.s_coil_data[N_coil].Steel_code = "IF";
      i_cla = 4;
		if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] >= 810.f + TK)
      {
         i_pdt = 0;
      }
      else if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] >= 790.f + TK)
      {
         i_pdt = 1;
      }
      else if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] > 760.f + TK && s_coil.s_coil_data[N_coil].tmp_tgt[RTH] <= 790.f + TK)
      {
         i_pdt = 2;
      }
      else if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] > 720.f + TK && s_coil.s_coil_data[N_coil].tmp_tgt[RTH] <= 760.f + TK)
      {
         i_pdt = 3;
      }
      else if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] > 650.f + TK && s_coil.s_coil_data[N_coil].tmp_tgt[RTH] <= 720.f + TK)
      {
         i_pdt = 4;
      }
      else if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] <= 650.f + TK )
      {
         i_pdt = 5;
      }
     
      if (s_coil.s_coil_data[N_coil].dummy == 1)
      {
         i_pdt = 6;
      }
   }   
   else if ( s_coil.s_coil_data[N_coil].thi < 0.0027f)
   { 
		s_coil.s_coil_data[N_coil].Steel_code = "HS";
      i_cla = 5;
		if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] >= 810.f + TK)
      {
         i_pdt = 0;
      }
      else if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] >= 790.f + TK)
      {
         i_pdt = 1;
      }
      else if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] > 760.f + TK && s_coil.s_coil_data[N_coil].tmp_tgt[RTH] <= 790.f + TK)
      {
         i_pdt = 2;
      }
      else if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] > 720.f + TK && s_coil.s_coil_data[N_coil].tmp_tgt[RTH] <= 760.f + TK)
      {
         i_pdt = 3;
      }
      else if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] > 650.f + TK && s_coil.s_coil_data[N_coil].tmp_tgt[RTH] <= 720.f + TK)
      {
         i_pdt = 4;
      }
      else if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] <= 650.f + TK )
      {
         i_pdt = 5;
      }
     
      if (s_coil.s_coil_data[N_coil].dummy == 1)
      {
         i_pdt = 6;
      }

   }
   else if ( s_coil.s_coil_data[N_coil].thi < 0.003f)
   { 
		s_coil.s_coil_data[N_coil].Steel_code = "IFS";
      i_cla = 6;
		if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] >= 810.f + TK)
      {
         i_pdt = 0;
      }
      else if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] >= 790.f + TK)
      {
         i_pdt = 1;
      }
      else if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] > 760.f + TK && s_coil.s_coil_data[N_coil].tmp_tgt[RTH] <= 790.f + TK)
      {
         i_pdt = 2;
      }
      else if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] > 720.f + TK && s_coil.s_coil_data[N_coil].tmp_tgt[RTH] <= 760.f + TK)
      {
         i_pdt = 3;
      }
      else if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] > 650.f + TK && s_coil.s_coil_data[N_coil].tmp_tgt[RTH] <= 720.f + TK)
      {
         i_pdt = 4;
      }
      else if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] <= 650.f + TK )
      {
         i_pdt = 5;
      }
     
      if (s_coil.s_coil_data[N_coil].dummy == 1)
      {
         i_pdt = 6;
      }
   }   
  
   else if ( s_coil.s_coil_data[N_coil].thi < 0.0035f)
   { 
		s_coil.s_coil_data[N_coil].Steel_code = "HSLA";
      i_cla = 8;
		if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] >= 810.f + TK)
      {
         i_pdt = 0;
      }
      else if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] >= 790.f + TK)
      {
         i_pdt = 1;
      }
      else if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] > 760.f + TK && s_coil.s_coil_data[N_coil].tmp_tgt[RTH] <= 790.f + TK)
      {
         i_pdt = 2;
      }
      else if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] > 720.f + TK && s_coil.s_coil_data[N_coil].tmp_tgt[RTH] <= 760.f + TK)
      {
         i_pdt = 3;
      }
      else if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] > 650.f + TK && s_coil.s_coil_data[N_coil].tmp_tgt[RTH] <= 720.f + TK)
      {
         i_pdt = 4;
      }
      else if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] <= 650.f + TK )
      {
         i_pdt = 5;
      }
     
      if (s_coil.s_coil_data[N_coil].dummy == 1)
      {
         i_pdt = 6;
      }
   }
	else
   { 
		s_coil.s_coil_data[N_coil].Steel_code = "EDDS";
      i_cla = 9;
		if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] >= 810.f + TK)
      {
         i_pdt = 0;
      }
      else if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] >= 790.f + TK)
      {
         i_pdt = 1;
      }
      else if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] > 760.f + TK && s_coil.s_coil_data[N_coil].tmp_tgt[RTH] <= 790.f + TK)
      {
         i_pdt = 2;
      }
      else if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] > 720.f + TK && s_coil.s_coil_data[N_coil].tmp_tgt[RTH] <= 760.f + TK)
      {
         i_pdt = 3;
      }
      else if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] > 650.f + TK && s_coil.s_coil_data[N_coil].tmp_tgt[RTH] <= 720.f + TK)
      {
         i_pdt = 4;
      }
      else if (s_coil.s_coil_data[N_coil].tmp_tgt[RTH] <= 650.f + TK )
      {
         i_pdt = 5;
      }
     
      if (s_coil.s_coil_data[N_coil].dummy == 1)
      {
         i_pdt = 6;
      }
   } 
   s_coil.s_coil_data[N_coil].N_cla = i_cla;
   s_coil.s_coil_data[N_coil].N_pdt = i_pdt;
	
   emi = s_adp[n_fur].s_pdt_RTF[i_cla][i_pdt].alpha * rtf_cfg[n_fur].emi_stp;

   return emi;
} // void

