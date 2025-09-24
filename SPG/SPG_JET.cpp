/*============================================================================
// SPG_JET.cpp: Set-point generator (stable state).
/  
/ (c) CMI Industry
/===========================================================================*/
#include "stdafx.h"

/*============================================================================
// SPG_JET: Calculation of the JET cooling set points
/
/  entry data: - strip target temperature a the end of the soaking section
/              - strip target temperature at the end of the JET cooling section
/              - Line speed
/              - cooling rate set point. Just to get the cooling rate ratio between zones themselves.
/                the real cooling rate on each zone will be calculate by this model
/
/  exit data:  - fan speed output per zones
/              - strip temperature at the exit of each zones
/              - cooling rate per zones
/
/ the maximum power output has to be adjusted throught ONE the following parameters:
/              - the maximum Hnx pressure in chambers
/              - the maximum Hnx flow of the fans
/ 
/ Revisions:
/  Created:       2008-Sep          J.Naveira
/  Modified:      
/===========================================================================*/
bool SPG_JET (float Tmp_in_jet, int Bisra, float spd, float tmp_tgt, float wid, float thi,
              float cool_rate_spt[], float Tmp_stp_out[], float pow_jet[], float cooling_rate[], bool max_spd)
{
	bool status = true, condition = true, low_cooling_rate = false;
   string sub_name = "SPG_JET";

   float Delta_Tmp, Delta_Tmp_old, Delta_spd, cool_rate_avg_spt, cool_rate_avg_new;
   float cool_rate_spt_new[NB_ZON_JET];
   float Blow_lgt, ratio_cool_rate;
   int iter;

   float Blow_speed_ini = 100.f;

   //- Initialization of cooling section parameters
   //- --------------------------------------------
   // For all jets
   for ( int i_jet=0; i_jet < sps_entry[JET].s_mea.nb_jet; i_jet++ )
   {
      //- Blow temperature [K]
      spg_entry_jet.blw_tmp[i_jet] = 80.f + TK;

      //- H2 rate in blowing gas [%]
      spg_entry_jet.H2_rate[i_jet] = 0.10f;

      //- Cooling length of a Blowstab [m]
      spg_entry_jet.blw_lgt[i_jet] = 8.f;

      //- Control
      spg_entry_jet.jet_on[i_jet] = true;

      // Ventilator maximum speed !?! to be defined during commisionning with a fonctionning point
      // Maximum speed of the fans in each JET zone [m/s]
      if ( i_jet == 0 ) spg_entry_jet.MAX_FAN_SPD[i_jet] = 31.2f;
      else              spg_entry_jet.MAX_FAN_SPD[i_jet] = 16.2f;

      //- Available fan power [W]
      if ( i_jet == 0 ) spg_entry_jet.instal_pow[i_jet] = 300000.f;
      else              spg_entry_jet.instal_pow[i_jet] = 110000.f;

   } // For all jets

   // Security on model convergence
   if ( Tmp_in_jet < tmp_tgt + 10.f )
   {
      // For all JET cooling zone
      for ( int i_jet=0; i_jet < sps_entry[JET].s_mea.nb_jet; i_jet++ )
      {
         pow_jet[i_jet] = 0.f;
         cooling_rate[i_jet] = (Tmp_in_jet - tmp_tgt) / float(sps_entry[JET].s_mea.nb_jet) * 
                               spd / spg_entry_jet.blw_lgt[i_jet];

         return status;
      }
   } // Security on model convergence

   // Initialize exit JET cooling strip exit temperature
   spg_entry_jet.box_tmp[ sps_entry[JET].s_mea.nb_jet - 1 ] = tmp_tgt + 100.f;
  
   // Loop parameters initialization
   Delta_Tmp = spg_entry_jet.box_tmp[ sps_entry[JET].s_mea.nb_jet - 1 ] - tmp_tgt;
   Delta_Tmp_old = Delta_Tmp;
   Delta_spd = 10.f;
   iter = 0;

   //- For all concerned jet sections -- Blowing length calculation
   Blow_lgt = 0.f;
   for ( int i_jet=0; i_jet < sps_entry[JET].s_mea.nb_jet; i_jet++ ) Blow_lgt += spg_entry_jet.blw_lgt[i_jet];

   //- Take into account cooling rate set point if existing
   //- ----------------------------------------------------
   if ( cool_rate_spt[0] > 0.f )
   {
      //- Average cooling rate given by set point
      cool_rate_avg_spt = 0.f;
      for ( int i_jet=0; i_jet<sps_entry[JET].s_mea.nb_jet; i_jet++ ) cool_rate_avg_spt += cool_rate_spt[i_jet];
      cool_rate_avg_spt /= float(sps_entry[JET].s_mea.nb_jet);

      //- Average cooling rate given by temperature set point
      cool_rate_avg_new = (Tmp_in_jet - tmp_tgt) * spd / Blow_lgt;

      //- Cooling rate ratio
      ratio_cool_rate = cool_rate_avg_spt / cool_rate_avg_new;

      //- Correction on cooling rate set point calculation
      for ( int i_jet=0; i_jet<sps_entry[JET].s_mea.nb_jet; i_jet++ )
      {
         cool_rate_spt_new[i_jet] = cool_rate_spt[i_jet] / ratio_cool_rate;
         spg_exit_jet.blw_spd[i_jet] = Blow_speed_ini * ( cool_rate_spt[i_jet] / cool_rate_avg_new );
         spg_exit_jet.jet_on[i_jet] = true;
      }
   } // Take into account cooling rate set point if exists

   //- (else) If no cooling rate setpoint
   //- ----------------------------------
   else
   {
      //- Average cooling rate given by temperature set point
      cool_rate_avg_new = (Tmp_in_jet - tmp_tgt) * spd / Blow_lgt;

      //- Correction on cooling rate set point calculation
      for (int i_jet=0; i_jet<sps_entry[JET].s_mea.nb_jet; i_jet++)
      {
         cool_rate_spt_new[i_jet] = cool_rate_avg_new;
         if (max_spd)
         {
            spg_exit_jet.blw_spd[i_jet] = spg_entry_jet.MAX_FAN_SPD[i_jet];
         }

         else spg_exit_jet.blw_spd[i_jet] = Blow_speed_ini;

         spg_exit_jet.jet_on[i_jet] = true;
      }
   } // no cooling rate imposed
   
   // If we are looking for maximum line speed, speed blow is set
   //============================================================
   if (max_spd)
   {
      //- Calculation of strip exit temperature in JET cooling
      status = JET_STP(Tmp_in_jet, Bisra, spd, wid, thi, spg_exit_jet.blw_spd, Tmp_stp_out);
      if ( !status ) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in JET_STP", fur_cfg[JET].fur_name.c_str());                                       

      //- Initialize box temperature
      for ( int i_jet=0; i_jet < sps_entry[JET].s_mea.nb_jet; i_jet++ ) spg_entry_jet.box_tmp[i_jet] = Tmp_stp_out[i_jet];
   }

   // If we are looking for the speed blow according to the target
   //============================================================
   else
   {
      //- Loop until error on exit JET cooling strip temperature is minimized
      //- ===================================================================
      do
      {
         //- Calculation of strip exit temperature in JET cooling
         status = JET_STP(Tmp_in_jet, Bisra, spd, wid, thi, spg_exit_jet.blw_spd, Tmp_stp_out);
         if ( !status ) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in JET_STP", fur_cfg[JET].fur_name.c_str());                                       

         //- Initialize box temperature
         for ( int i_jet=0; i_jet < sps_entry[JET].s_mea.nb_jet; i_jet++ ) spg_entry_jet.box_tmp[i_jet] = Tmp_stp_out[i_jet];

         //- Error on box temperature
         Delta_Tmp = spg_entry_jet.box_tmp[ sps_entry[JET].s_mea.nb_jet - 1 ] - tmp_tgt;

         //- If the error on JET cooling strip exit temperature has changed of sign
         if ( Delta_Tmp * Delta_Tmp_old < 0.f )
         {
            //- Minimize the speed blowing variation, increase calculation precision
            Delta_spd = Delta_spd / 2.f;
         }
      
         //- Compute Blow speed
         //- ------------------
         //- For all JET cooling zone
         for ( int i_jet=0; i_jet < sps_entry[JET].s_mea.nb_jet; i_jet++ )
         {
            //- If JET section is open
            if ( spg_exit_jet.jet_on[i_jet] )
            {
               //- If error on JET cooling exit temperature is positive
	            if ( Delta_Tmp > 0.f && spg_exit_jet.Cooling_rate[i_jet] < cool_rate_spt_new[i_jet] )
               {
                  //- Increase blow speed
                  spg_exit_jet.blw_spd[i_jet] = spg_exit_jet.blw_spd[i_jet] + Delta_spd;
               }

               //- (else) If error on JET cooling exit temperature is null or negative
	            else if ( Delta_Tmp <= 0.f && spg_exit_jet.Cooling_rate[i_jet] > cool_rate_spt_new[i_jet] )
               {
                  //- Decrease blow speed
                  spg_exit_jet.blw_spd[i_jet] = spg_exit_jet.blw_spd[i_jet] - Delta_spd;
               }
               //- If blow speed is zero
	            if ( spg_exit_jet.blw_spd[i_jet] == 0.f)
               {
                  //- Blow speed is set to 0.1 m/s
                  spg_exit_jet.blw_spd[i_jet] = 0.1f;
               }
               // If blow speed is negative
	            if (spg_exit_jet.blw_spd[i_jet] < 0.f)
               {
                  spg_exit_jet.blw_spd[i_jet] = 2.5f;
	            }
            } // If JET section is open

            //- (else) If JET section is not open
            else
            {
               //- Blow speed is set to zero
               spg_exit_jet.blw_spd[i_jet] = 0.f;
            }

         } // For all JET cooling zone

         // Store error on strip temperature
         Delta_Tmp_old = Delta_Tmp;

	    	condition = true;
	    	for ( int i_jet = 0; i_jet < sps_entry[JET].s_mea.nb_jet; i_jet++ )
         {
	    	   if ( ! (fabs(spg_exit_jet.Cooling_rate[i_jet] - cool_rate_spt_new[i_jet]) < 1.5f ))
            {
	    	    	condition = false;
            }
         }

	    	if ( ! condition ) Delta_spd = max(Delta_spd, 2.f);

         iter ++;
      } while ( ( fabs(Delta_Tmp) > 1.f || !condition) && iter <= 120); 
      /* Loop until error on JET cooling strip exit temperature is minimized*/

      // If too much iteration
      if (iter >= 120) 
      {
		   /* calculation divergence */
		   ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Convergence error", fur_cfg[JET].fur_name.c_str());
      }
   }
   // Store calculation result
   // ========================

   // For all JET cooling zone
   for (int i_jet=0; i_jet<sps_entry[JET].s_mea.nb_jet; i_jet++)
   {
      pow_jet[i_jet] = spg_exit_jet.jet_zon_pow[i_jet];
      cooling_rate[i_jet] = spg_exit_jet.Cooling_rate[i_jet];
   }

   return status;
}

/*============================================================================
// JET_STP: Calculation of strip exit temperature in JET cooling
/ 
/ Revisions:
/  Created:       2008-Sept          J.Naveira
/  Modified: 
/===========================================================================*/
bool JET_STP (float Tmp_in_jet, int Bisra, float spd, float wid, float thi, float Spd_blow[], 
              float Tmp_stp_out[])
{
	
	bool status = true;
	string sub_name = "JET_STP";

	float ro_H2, ro_N2, H2_vis, N2_vis, H2_cond, N2_cond;
	float H2massfrac;
	float MixConductivity, MixViscosity, MixSpecificHeat;
	float Fac12, Fac21;
   float Pstrip;
	float delta_h, old_delta_h, delta_t;           // To iterate
	float z_d;                                     // Ratio Strip-Nozzle distance / Nozzle characteristic length 
	float BlowSurf;                                // Blowing surface 
	float A;                                       // Coeff for Nusselt number calculation
	float T;
	float Tmp_in[NB_ZON_JET];
	float Tmp_out[NB_ZON_JET];
	float max_blow_flow[NB_ZON_JET];

   for (int i_jet=0; i_jet < sps_entry[JET].s_mea.nb_jet ; i_jet++)
   {
      if (i_jet==0) max_blow_flow[i_jet] = 5.55f * 2.f; //2000.f/60.f;
      else          max_blow_flow[i_jet] = 5.76f; //1000.f/60.f;
   }

	for (int i_jet=0; i_jet < sps_entry[JET].s_mea.nb_jet ; i_jet++)
   {

		// Temperature of the strip at the entrance of the box
		if (i_jet == 0) Tmp_in[i_jet] = Tmp_in_jet;
		else            Tmp_in[i_jet] = Tmp_out[i_jet - 1];

		// H2 mass fraction calculation
		ro_H2 = float(M_H2 / 22.4136f * TK / spg_entry_jet.blw_tmp[i_jet]);  // Density at blowing temperature
		ro_N2 = float(M_N2 / 22.4136f * TK / spg_entry_jet.blw_tmp[i_jet]);  // Density at blowing temperature
		H2massfrac = spg_entry_jet.H2_rate[i_jet] * ro_H2 / (spg_entry_jet.H2_rate[i_jet] * ro_H2 + 
                    (1.f - spg_entry_jet.H2_rate[i_jet]) * ro_N2);

		// Enthalpy of the strip at the entrance of the box
		float HE = ns_STD::cl_BIS::BISRA_Enth(Bisra, Tmp_in[i_jet]);

		// Initialisation of iterative calculation
		delta_h = -50.f;
		old_delta_h = -50.f;
		Tmp_out[i_jet] = Tmp_in[i_jet];
		delta_t = 48.f;

		z_d = jet_cfg.d_stp_noz / jet_cfg.noz_dia;
		BlowSurf = float(jet_cfg.nb_holes) * jet_cfg.noz_dia*jet_cfg.noz_dia * PI / 4.f;

	   int loop = 0;
      // Iterations made on delta T°
		while (abs(delta_h) > 1.f && delta_t > 0.002f && spg_entry_jet.jet_on[i_jet] && loop < MAX_ITER) 
      { 
			loop ++;

			if (delta_h * old_delta_h < 0.f) delta_t = delta_t / 2.f;

			if (delta_h < 0.f) Tmp_out[i_jet] = Tmp_out[i_jet] - delta_t;
			else               Tmp_out[i_jet] = Tmp_out[i_jet] + delta_t;

			// Tmp_out must be smaller than Tmp_in
			if (Tmp_out[i_jet] >= Tmp_in[i_jet]) Tmp_out[i_jet] = Tmp_in[i_jet] - 0.0001f; // Avoids divergence

			// Tmp_out must be higher than blowing temperature
			if (Tmp_out[i_jet] <= spg_entry_jet.blw_tmp[i_jet]) Tmp_out[i_jet] = spg_entry_jet.blw_tmp[i_jet]; // Avoids divergence

         // Properties calculation T°
			T = ((Tmp_out[i_jet] + Tmp_in[i_jet]) / 2.f + spg_entry_jet.blw_tmp[i_jet]) / 2.f; 

			H2_cond = -90.f * (T/1000.f * T/1000.f) + 553.f * (T/1000.f) + 27.4f;
			N2_cond = -30.f * (T/1000.f * T/1000.f) +  90.f * (T/1000.f) +  1.1f;

			H2_vis =  -8.f * (T/1000.f * T/1000.f) + 25.f * (T/1000.f) + 2.06f;
			N2_vis = -30.f * (T/1000.f * T/1000.f) + 64.f * (T/1000.f) + 0.98f;

			spg_exit_jet.Mix_density[i_jet] = spg_entry_jet.H2_rate[i_jet] * ro_H2 + (1 - spg_entry_jet.H2_rate[i_jet]) * ro_N2;
			
			float fracHN = M_H2 / M_N2;
			float fracNH = M_N2 / M_H2;
			float visHN = H2_vis/ N2_vis;
			float visNH = N2_vis/ H2_vis;

			Fac12 = powf((8.f * (1.f + fracHN)), (-0.5f)) * powf(1.f + powf(visHN, 0.5f) * powf(fracNH, 0.25f), 2.f);
			Fac21 = powf((8.f * (1.f + fracNH)), (-0.5f)) * powf(1.f + powf(visNH, 0.5f) * powf(fracHN, 0.25f), 2.f);

			MixConductivity = spg_entry_jet.H2_rate[i_jet] * H2_cond/ (spg_entry_jet.H2_rate[i_jet] + 
			                  (1.f - spg_entry_jet.H2_rate[i_jet]) * Fac12) + 
			                  (1.f - spg_entry_jet.H2_rate[i_jet]) * N2_cond / ((1.f - spg_entry_jet.H2_rate[i_jet]) + 
			                  spg_entry_jet.H2_rate[i_jet] * Fac21);

			MixViscosity = spg_entry_jet.H2_rate[i_jet] * H2_vis / (spg_entry_jet.H2_rate[i_jet] + 
			               (1.f - spg_entry_jet.H2_rate[i_jet]) * Fac12) + 
			               (1.f - spg_entry_jet.H2_rate[i_jet]) * N2_vis / ((1.f - spg_entry_jet.H2_rate[i_jet]) + 
			               spg_entry_jet.H2_rate[i_jet] * Fac21);

			MixSpecificHeat = (1.013f * (1.f - H2massfrac) + 14.318f * H2massfrac);

			// Prandtl adimensional number
			float Pr = MixViscosity * MixSpecificHeat / MixConductivity;

			// Reynolds number calculation
			float Re = Spd_blow[i_jet] * jet_cfg.noz_dia * ( spg_exit_jet.Mix_density[i_jet] / MixViscosity ) * 
                    1000000.f;

         // Ratio Nozzle pitch / Nozzle characteristic length 
			float x_d = jet_cfg.pitch / jet_cfg.noz_dia;
			// Nusselt number calculation, based on H Martin 1977 PhD thesis
			A = sqrtf(PI / 4.f / (x_d*x_d));

			float Nu = (powf(Pr, 0.42f) * powf((1.f + powf(((z_d / 0.6f) * A), 6.f)), (-0.05f)) * A * (1.f - (2.2f * A)) / 
                    (1.f + (0.2f * (z_d - 6.f) * A))) * powf(Re, 2.f/3.f);

			// Convection coefficent calculation
			spg_exit_jet.H_cnv[i_jet] = Nu * MixConductivity / jet_cfg.noz_dia / 1000.f;

			// Production en kg/s
			old_delta_h = delta_h;

         float T_m_ln = 0.f;
			if ( Tmp_out[i_jet] != spg_entry_jet.blw_tmp[i_jet] )
         {
				T_m_ln = (Tmp_in[i_jet] - Tmp_out[i_jet]) / logf((Tmp_in[i_jet] - spg_entry_jet.blw_tmp[i_jet]) / 
				         (Tmp_out[i_jet] - spg_entry_jet.blw_tmp[i_jet]));
			}

			// Enthalpy lost by convection
			spg_exit_jet.cnv_pow[i_jet] = spg_exit_jet.H_cnv[i_jet] * (wid * spg_entry_jet.blw_lgt[i_jet]	* 2.f) * T_m_ln;

			// Strip enthalpy at the exit of the box
			float HS = ns_STD::cl_BIS::BISRA_Enth(Bisra,Tmp_out[i_jet]);
			Pstrip = wid * thi * spd * RO_STEEL * (HE - HS);
			delta_h = Pstrip - spg_exit_jet.cnv_pow[i_jet];
		} // End While

		if (loop > MAX_ITER) 
      {
			ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s %d: Error the calculation has not converged", fur_cfg[JET].fur_name.c_str(), i_jet);
			status = false;
		}

		// Blower flow rate
		spg_exit_jet.blw_flw[i_jet] = Spd_blow[i_jet] * BlowSurf * 2.f; // 2 blowing faces

		// N2/H2 return temperature = N2/H2 heat exchanger inlet temperature
		spg_exit_jet.Tmp_N2H2_in[i_jet] = spg_entry_jet.blw_tmp[i_jet] + Pstrip / spg_exit_jet.blw_flw[i_jet] / 
                                        spg_exit_jet.Mix_density[i_jet] / MixSpecificHeat / 1000.f;

      // Blower power in % of ventilator speed
      spg_exit_jet.jet_zon_pow[i_jet] = spg_exit_jet.blw_flw[i_jet] / max_blow_flow[i_jet] * 100.f;

		// Cooling rates
		spg_exit_jet.Cooling_rate[i_jet] = fabs((Tmp_out[i_jet] - Tmp_in[i_jet]) * spd / spg_entry_jet.blw_lgt[i_jet]);

      // Store exit strip temeprature
      Tmp_stp_out[i_jet] = Tmp_out[i_jet];

		if ( A*A >= 0.04f )
      {
         ns_STD::cl_TRC::TRC_Write("WAR", "SPG", sub_name, "%s %d: A coefficient over validity limits.", fur_cfg[JET].fur_name.c_str(), i_jet);
		}       

		else if ( A*A <= 0.004f ) 
      {
         ns_STD::cl_TRC::TRC_Write("WAR", "SPG", sub_name, "%s %d: A coefficient under validity limits.", fur_cfg[JET].fur_name.c_str(), i_jet);
		}   

		if ( z_d > 12.f )
      {
         ns_STD::cl_TRC::TRC_Write("WAR", "SPG", sub_name, "%s %d: Z/D ratio over validity limits.", fur_cfg[JET].fur_name.c_str(), i_jet);
		}      

		else if ( z_d < 2.f )
      {
         ns_STD::cl_TRC::TRC_Write("WAR", "SPG", sub_name, "%s %d: Z/D ratio out of validity limits.", fur_cfg[JET].fur_name.c_str(), i_jet);
		}
	} // For all jet cooling zone

	return status;
}

/*============================================================================
/ SPG_JET_Max_Spd: Compute strip maximum speed in the JET according to the strip 
/     characteristics and the power of the zone.
/ 
/ Revisions:
/  Created:       2009-sep       J. NAVEIRA
/  Modified: 
/*===========================================================================*/
bool SPG_JET_Max_Spd( float Tmp_in_jet, float tmp_tgt[JET], float thi, float wid, float Cool_rate[], float *spd )
{
	bool status = false, strip = false;
	string sub_name = "SPG_JET_Max_Spd";

	float spd1, spd2;
   float Tmp_out_JET[NB_ZON_JET], Cooling_rate[NB_ZON_JET], Pow_JET[NB_ZON_JET];
	float Max_avg_pow_jet = 100.f;
   bool max_spd = true;
   float spd_blow = 0.f;
   int i1 = 0;

	/* Speed initialization at maximum speed */
	spd1 = MAX_SPEED;

	/* Speed initialization at minimum speed */
	spd2 = MIN_SPEED;
	/* ============================================================================================= */
	/*                                   Maximum speed calculation by dichotomy */
	/* ============================================================================================= */
	int iter = 0;
	do
	{
		/* Speed calculation */
		/* ----------------- */
		*spd = (spd1 + spd2) / 2.f;

		/* strip temperature calculation */
		/* ----------------------------- */
		status = SPG_JET(Tmp_in_jet, s_coil.s_coil_data[i1].Bisra, 
                       *spd, s_coil.s_coil_data[i1].tmp_tgt[JET], 
                       s_coil.s_coil_data[i1].wid, s_coil.s_coil_data[i1].thi,
                       s_coil.s_coil_data[i1].Cool_rate, Tmp_out_JET, 
                       Pow_JET, Cooling_rate, max_spd);
		if ( !status ) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in SPG_JET", fur_cfg[JET].fur_name.c_str());
            
      // if the needed power required is higher than the installed power
		if ( Tmp_out_JET[sps_entry[JET].s_mea.nb_jet-1] > s_coil.s_coil_data[i1].tmp_tgt[JET])
		{
			spd1 = *spd;
		}

		else
		{
			spd2 = *spd;
		}

		iter ++;
	} while ( fabs( Tmp_out_JET[sps_entry[JET].s_mea.nb_jet-1] - s_coil.s_coil_data[i1].tmp_tgt[JET] ) > 1.f && 
             iter < MAX_ITER );

   // Alarm treatment "Error over looping". 
	if ( iter >= MAX_ITER ) 
	{
		status = false;
		ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Convergence error maximum speed calculation", fur_cfg[JET].fur_name.c_str());
	}

   return status;
}
