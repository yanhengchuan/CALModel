/*============================================================================
/ ADA: All adaptation features.
/  
/ (c) CMI Industry
/===========================================================================*/
#include "stdafx.h"

/*============================================================================
/ SPS_ADA_effi: Coefficient calculation for self emissivity adaptation.
/ 
/ Revisions:
/  Created:       2007-Jul      J. NAVEIRA
/  Modified: 
/
/ RTF incoming structure is current coil in furnace one
/===========================================================================*/
void SPS_ADA_effi(int n_coil, int n_fur)
{
	string sub_name = "SPS_ADA_effi";
   bool status = true;

	bool roll = false;
	float power[MAX_NB_ZON];
   float thick, width, Exit_rtf;
   float ratio_pow;

   // rtf strip entry temperature
   float beg_rtf_tmp = fur_cfg[n_fur].entry_tmp;
   int n_cla = s_coil.s_coil_data[n_coil].N_cla;
   int n_pdt = s_coil.s_coil_data[n_coil].N_pdt;

	// Initialization of the rtf structure
   RTF_static_init (&rtf[n_fur], fur_cfg[n_fur].nb_zon, rtf_cfg[n_fur], n_pdt, n_cla, n_coil);

   rtf[n_fur].spd = sps_entry[n_fur].s_mea.spd;
   thick = s_coil.s_coil_data[n_coil].thi;
   width = s_coil.s_coil_data[n_coil].wid;

   // Calculation of the radiation factors in each element
   float emi = rtf_cfg[n_fur].emi_stp * s_adp[n_fur].s_pdt_RTF[n_cla][n_pdt].alpha;
	RTF_dynamic_init( &rtf[n_fur], thick, width, emi, beg_rtf_tmp );

   // Strip temperature calculation
	status = RTF_strip( &rtf[n_fur], roll, sps_entry[n_fur].s_mea.tmp_zon, &Exit_rtf );
      
   // Initializations
   for ( int i_zon=0; i_zon < fur_cfg[n_fur].nb_zon; i_zon++ ) power[i_zon] = 0.f;

   // Heat flow by zone [W].
   RTF_power (&rtf[n_fur], power);

	// if calcul are good
	if (status)
	{
		// In % of connected input
      for ( int i_zon=0; i_zon < fur_cfg[n_fur].nb_zon; i_zon++ ) 
		{
			if (sps_entry[n_fur].s_mea.heat_pow_pct[i_zon] >= 25.f)
			{
				ratio_pow = power[i_zon] / sps_entry[n_fur].s_mea.heat_pow_pct[i_zon];
				s_adp[n_fur].Effi[i_zon] = 0.99f * s_adp[n_fur].Effi[i_zon] + 
                                       0.01f * s_adp[n_fur].Effi[i_zon] * ratio_pow;
				s_adp[n_fur].Effi[i_zon] = min(max(s_adp[n_fur].Effi[i_zon], 0.05f), 10.f);
			}
		} // In % of connected input

		// Write PID values in a SQL
		// =========================
		status = SQL_EFFICIENCY_UPDATE(n_fur);

	} // if calcul are good

   return;
} // void

/*============================================================================
/ SPS_ADA_Emis: Coefficient calculation for self emissivity adaptation.
/ 
/ Revisions:
/  Created:       2007-Jul      J. NAVEIRA
/  Modified: 
/===========================================================================*/
void SPS_ADA_Emis(int coil_cur, bool roll, int n_fur)
{
	string sub_name = "SPS_ADA_Emis";

   int iter;
	float alpha_est_rtf, tmp_est, tmp_mea, Delta_Tmp;
	float erreur;

	Delta_Tmp = 3.f;

   // Product and classe of current coil
   int n_pdt = s_coil.s_coil_data[coil_cur].N_pdt;
   int n_cla = s_coil.s_coil_data[coil_cur].N_cla;

   // Initialization of estimated Alpha and measured Strip temperature
	alpha_est_rtf = s_adp[n_fur].s_pdt_RTF[n_cla][n_pdt].alpha;

   // calcul of estimated temperature regarding current radiant tube temperature for RTF
   // ----------------------------------------------------------------------------------
   SPS_ADA_Strip_tmp(coil_cur, roll, 1, n_cla, alpha_est_rtf, &tmp_est, n_fur);

   iter = 0;
	tmp_mea = sps_entry[n_fur].s_mea.tmp_stp;

	// Search Apha estimated where Estimated temperature near from measured temperature for RTF
   // ----------------------------------------------------------------------------------------
   //while (((tmp_mea > tmp_est + Delta_Tmp / 2.f) || (tmp_mea < tmp_est - Delta_Tmp / 2.f)) && iter < 60)
   while ((abs(tmp_mea - tmp_est) > Delta_Tmp) && iter < 60)
   {
      // If density flow on strip is under estimated
		//if (tmp_est < tmp_mea)
		//	alpha_est_rtf += 0.001f;
		//else
		//	alpha_est_rtf -= 0.001f;
		alpha_est_rtf += (0.001f * (tmp_mea - tmp_est));

      // Thermal calculation of new estimated temperature
      SPS_ADA_Strip_tmp(coil_cur, roll, 1, n_cla, alpha_est_rtf, &tmp_est, n_fur);

		// Upper security
		if (alpha_est_rtf * rtf_cfg[n_fur].emi_stp > 0.36f)
		{
			alpha_est_rtf = 0.36f / rtf_cfg[n_fur].emi_stp;
         break;
		}

		// lower security
		if (alpha_est_rtf * rtf_cfg[n_fur].emi_stp < 0.05f)
		{
			alpha_est_rtf = 0.05f / rtf_cfg[n_fur].emi_stp;
         break;
		}

      iter ++;
   } // Search Apha estimated where Estimated temperature near from measured temperature for RTF

   // Alarm treatment "Error over looping"
   if ( iter >= 60 )
		ns_STD::cl_TRC::ERR_Write("ADA", sub_name, "%s: Error to find new strip emissivity. n_cla=%d, n_pdt=%d, tmp_est=%.1f, tmp_mea=%.1f",
																	fur_cfg[n_fur + RTH].fur_name.c_str(), n_cla, n_pdt, tmp_est, tmp_mea);

	// Security on alpha calculation
	// =============================
	// Upper security
	if (alpha_est_rtf * rtf_cfg[n_fur].emi_stp > 0.36f)
		alpha_est_rtf = 0.36f / rtf_cfg[n_fur].emi_stp;

	// lower security
	if (alpha_est_rtf * rtf_cfg[n_fur].emi_stp < 0.05f)
		alpha_est_rtf = 0.05f / rtf_cfg[n_fur].emi_stp;

   // Calculation of new alpha for RTF
   // ================================
	erreur = fabs(alpha_est_rtf - s_adp[n_fur].s_pdt_RTF[n_cla][n_pdt].alpha) /
                                 s_adp[n_fur].s_pdt_RTF[n_cla][n_pdt].alpha * 100;

   // The adaptation depends of the error. the bigger the error is the lewest the adaptation is.
	if ( erreur <= 1.f )
      s_adp[n_fur].s_pdt_RTF[n_cla][n_pdt].alpha = 0.8f   * s_adp[n_fur].s_pdt_RTF[n_cla][n_pdt].alpha + 0.2f   * alpha_est_rtf;
	else if ( erreur <= 2.5f )                 
	   s_adp[n_fur].s_pdt_RTF[n_cla][n_pdt].alpha = 0.83f  * s_adp[n_fur].s_pdt_RTF[n_cla][n_pdt].alpha + 0.17f  * alpha_est_rtf;
	else if ( erreur <=  5.f )      
	   s_adp[n_fur].s_pdt_RTF[n_cla][n_pdt].alpha = 0.93f  * s_adp[n_fur].s_pdt_RTF[n_cla][n_pdt].alpha + 0.07f  * alpha_est_rtf;
	else if ( erreur <= 10.f )      
	   s_adp[n_fur].s_pdt_RTF[n_cla][n_pdt].alpha = 0.973f * s_adp[n_fur].s_pdt_RTF[n_cla][n_pdt].alpha + 0.027f * alpha_est_rtf;
	else if ( erreur <= 20.f )      
	   s_adp[n_fur].s_pdt_RTF[n_cla][n_pdt].alpha = 0.99f  * s_adp[n_fur].s_pdt_RTF[n_cla][n_pdt].alpha + 0.01f  * alpha_est_rtf;
	else
	   s_adp[n_fur].s_pdt_RTF[n_cla][n_pdt].alpha = 0.997f * s_adp[n_fur].s_pdt_RTF[n_cla][n_pdt].alpha + 0.003f * alpha_est_rtf;

	s_adp[n_fur].s_pdt_RTF[n_cla][n_pdt].alpha = min(max(s_adp[n_fur].s_pdt_RTF[n_cla][n_pdt].alpha, 0.6f), 1.f);

	// Self adapatation coefficient Writting
	// =====================================

	for (int i_coil = 0; i_coil < s_coil.nb_coil ; i_coil++)
		s_coil.s_coil_data[i_coil].emi_rtf[n_fur] = rtf_cfg[n_fur].emi_stp * s_adp[n_fur].s_pdt_RTF[s_coil.s_coil_data[i_coil].N_cla][s_coil.s_coil_data[i_coil].N_pdt].alpha;
	s_adp[n_fur].s_pdt_RTF[n_cla][n_pdt].Steel_code =  s_coil.s_coil_data[coil_cur].Steel_code;
	SQL_EMISSIVITY_UPDATE(n_fur);

   return;
} // void

/*============================================================================
/ SPS_ADA_Strip_tmp: Strip temperature calculation.
/ 
/ Revisions:
/  Created:       2007-Jul      J. NAVEIRA
/  Modified: 
/===========================================================================*/
bool SPS_ADA_Strip_tmp(int n_coil, bool roll, int n_pdt, int n_cla, float alpha_est, 
                       float *Exit_stp_tmp, int n_fur)
{
	string sub_name = "SPS_ADA_Strip_tmp";
   bool status = true;

   float Exit_rtf_tmp;

   // rtf strip entry temperature
   float beg_rtf_tmp = fur_cfg[n_fur].entry_tmp;

   // Static data initialization
   RTF_static_init (&rtf[n_fur], fur_cfg[n_fur].nb_zon, rtf_cfg[n_fur], n_pdt, n_cla, n_coil );

   // speed
	rtf[n_fur].spd = sps_entry[n_fur].s_mea.spd;

   // Dynamic data initialization
   float emi = rtf_cfg[n_fur].emi_stp * alpha_est;
   RTF_dynamic_init(&rtf[n_fur], s_coil.s_coil_data[n_coil].thi, s_coil.s_coil_data[n_coil].wid, emi, beg_rtf_tmp);

	for (int i_zon = 0; i_zon < fur_cfg[n_fur].nb_zon; i_zon++ )
	{
		if((sps_entry[n_fur].s_mea.heat_pow_pct[i_zon]>98.f) && (sps_entry[n_fur].s_mea.tmp_zon[i_zon] + 40.f < sps_act_sp[n_fur].tmp_zon[i_zon]))
			sps_entry[n_fur].s_mea.tmp_zon_ada[i_zon] = sps_act_sp[n_fur].tmp_zon[i_zon];
		else
			sps_entry[n_fur].s_mea.tmp_zon_ada[i_zon] = sps_entry[n_fur].s_mea.tmp_zon[i_zon];
	}

   // Strip temperature calculation.
   status = RTF_strip( &rtf[n_fur], roll, sps_entry[n_fur].s_mea.tmp_zon_ada, &Exit_rtf_tmp );
   if ( !status )
   {
      ns_STD::cl_TRC::ERR_Write("ADA", sub_name, "error to find rtf strip exit temperature");
      return status;
   }

   // Exit strip temperature
   *Exit_stp_tmp = Exit_rtf_tmp;

   return status;
} // void

/*============================================================================
/ SPS_ADA_exchgeCoeff: Adaptation on strip/roll exchange coeffcient.
/ 
/ Revisions:
/  Created:       2009-Apr      J. NAVEIRA
/  Modified: 
/
/ During Anticipipation time, store RTF temperature gap
/ When weld exit RTF section, look for the gap measurment
/===========================================================================*/
void SPS_ADA_exchgeCoeff(bool Anticipation_time, float Tmp_rtf, int n_fur)
{
	string sub_name = "SPS_ADA_exchgeCoeff";
   time_t current_time;
   time (&current_time);

   // If anticipation time in RTF
   if (Anticipation_time)
   {
      s_adp[n_fur].s_exg.Gap_spt = fabs( trans_result_rtf[n_fur][0].tmp_stp_rtf - 
                                         trans_result_rtf[n_fur][0].tmp_stp_rtf2 );
   } // If anticipation time in RTF

   // If weld has exited from RTF
   else
   {
      s_adp[n_fur].s_exg.adp_calc = true;

      // If last temperature does not exist
      if (s_adp[n_fur].s_exg.s_calc.Tmp_last == -1.f)
      {
         s_adp[n_fur].s_exg.s_calc.Tmp_last = Tmp_rtf;
         s_adp[n_fur].s_exg.Gap_mea = Tmp_rtf;
      }
      // If last sign does not exist
      else if (s_adp[n_fur].s_exg.s_calc.last_sign == 0)
      {
         if (s_adp[n_fur].s_exg.s_calc.Tmp_last - Tmp_rtf > 0.f)
            s_adp[n_fur].s_exg.s_calc.last_sign = -1;
         else if (s_adp[n_fur].s_exg.s_calc.Tmp_last - Tmp_rtf < 0.f)
            s_adp[n_fur].s_exg.s_calc.last_sign = 1;
         else
            s_adp[n_fur].s_exg.s_calc.last_sign = 0;

         s_adp[n_fur].s_exg.s_calc.Tmp_last = Tmp_rtf;
      }

      // Search a sign changement
      else
      {
         // If more than 1 minute since last data check
         if (difftime(current_time, s_adp[n_fur].s_exg.lst_time) > 60.f)
         {
            // The line has stopped, end of self adaptation
            s_adp[n_fur].s_exg.Gap_mea = -1.f;
            s_adp[n_fur].s_exg.Gap_spt = -1.f;
            s_adp[n_fur].s_exg.s_calc.last_sign = 0;
            s_adp[n_fur].s_exg.s_calc.Tmp_last = -1.f;
            return;
         }

         // If positive sign
         if (s_adp[n_fur].s_exg.s_calc.Tmp_last - Tmp_rtf > 0.f)
         {
            // If already the same sign
            if (s_adp[n_fur].s_exg.s_calc.last_sign == -1) s_adp[n_fur].s_exg.s_calc.Tmp_last = Tmp_rtf;
            else
            {
               s_adp[n_fur].s_exg.Gap_mea = fabs(s_adp[n_fur].s_exg.Gap_mea - Tmp_rtf);
               s_adp[n_fur].s_exg.adp_calc = false;
            }
         }
         // If negative sign
         else if (s_adp[n_fur].s_exg.s_calc.Tmp_last - Tmp_rtf < 0.f)
         {
            // If already the same sign
            if (s_adp[n_fur].s_exg.s_calc.last_sign == 1) s_adp[n_fur].s_exg.s_calc.Tmp_last = Tmp_rtf;
            else
            {
               s_adp[n_fur].s_exg.Gap_mea = fabs(s_adp[n_fur].s_exg.Gap_mea - Tmp_rtf);
               s_adp[n_fur].s_exg.adp_calc = false;
            }
         }
         // If no variation
         else
         {
            s_adp[n_fur].s_exg.s_calc.last_sign = 0;
            s_adp[n_fur].s_exg.Gap_mea = fabs(s_adp[n_fur].s_exg.Gap_mea - Tmp_rtf);
            s_adp[n_fur].s_exg.adp_calc = false;
         }
      } // Search a sign changement

      // If sign changement has occured
      if (!s_adp[n_fur].s_exg.adp_calc && s_adp[n_fur].s_exg.Gap_mea != -1.f)
      {
         // Error on gap
         float error = (s_adp[n_fur].s_exg.Gap_spt - s_adp[n_fur].s_exg.Gap_mea) / s_adp[n_fur].s_exg.Gap_spt;

         // Minimize the error
         error /= 10.f;

         // Security on error -- avoid important variation of adaption coefficient
         error = max(min(error, 0.02f), -0.02f);

         // New adaptation parameter
         s_adp[n_fur].s_exg.Alpha = s_adp[n_fur].s_exg.Alpha + error * s_adp[n_fur].s_exg.Alpha;

         // Reset calculation data
         s_adp[n_fur].s_exg.Gap_mea = -1.f;
         s_adp[n_fur].s_exg.Gap_spt = -1.f;
         s_adp[n_fur].s_exg.s_calc.last_sign = 0;
         s_adp[n_fur].s_exg.s_calc.Tmp_last = -1.f;

         // Store calculated adaptation parameter
          SQL_ADAPTATION_UPDATE(n_fur);
      } // If sign changement has occured

      time(&s_adp[n_fur].s_exg.lst_time);

   } // If weld has exited from RTF

   return;
} // void