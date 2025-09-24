/*============================================================================
/ RTF_UTL: RTF Utilities.
/  
/ (c) CMI Industry
/===========================================================================*/
#include "stdafx.h" 

/*============================================================================
/ RTF_static_init: Static data initialization.
/
/ Revisions:
/  Created:       2004          P.Dubois
/  Modified: 
/===========================================================================*/
void RTF_static_init( struct rtf_struct *rtf, int nb_zon, struct rtf_cfg_struct rtf_cfg, int n_pdt, int n_cla, 
                      int n_coil )
{
   //int ecz_d[MAX_NB_ELT], ecz_u[MAX_NB_ELT];
   //bool wall[MAX_NB_ELT], roll[MAX_NB_ELT];

   rtf->n_rtf = rtf_cfg.n_rtf;
   rtf->nb_elt = rtf_cfg.nb_elt;
   rtf->nb_ecz = rtf_cfg.nb_ecz; 
   rtf->nb_zon = nb_zon;
   rtf->first_strip_elt = rtf_cfg.first_strip_elt;
   rtf->elt_pyro = rtf_cfg.elt_pyro;

   // For all ELT (strip element, wall and roll element)
   for ( int i_elt=0 ; i_elt < rtf->nb_elt; i_elt++ )
   {
      rtf->s_elt[i_elt].i_wall       = ( rtf_cfg.C_ELT[0][i_elt] == 1.f );
      rtf->s_elt[i_elt].i_roll       = ( rtf_cfg.C_ELT[1][i_elt] == 1.f );
      rtf->s_elt[i_elt].s_strip.lgt  = rtf_cfg.C_ELT[2][i_elt];
      rtf->s_elt[i_elt].s_rol.coeff  = rtf_cfg.r_coeff;
      rtf->s_elt[i_elt].s_rol.weigth = rtf_cfg.one_rol_mass;
      rtf->s_elt[i_elt].ecz_d        = -1;
      rtf->s_elt[i_elt].ecz_u        = -1;
      rtf->s_elt[i_elt].s_strip.emi  = rtf_cfg.emi_stp * 
                                       s_adp[rtf->n_rtf].s_pdt_RTF[n_cla][n_pdt].alpha;
      rtf->s_elt[i_elt].s_strip.thi  = s_coil.s_coil_data[n_coil].thi;
      rtf->s_elt[i_elt].s_strip.wid  = s_coil.s_coil_data[n_coil].wid;
   }    // For all ELT (strip element)

   // For all ECZ (elementary control zone)
   for ( int i_ecz=0 ; i_ecz < rtf->nb_ecz; i_ecz++ )
   {
      rtf->s_ecz[i_ecz].elt_u           = rtf_cfg.C_ECZ[0][i_ecz];
      rtf->s_ecz[i_ecz].elt_d           = rtf_cfg.C_ECZ[1][i_ecz];
      rtf->s_elt[rtf_cfg.C_ECZ[0][i_ecz]].ecz_d = i_ecz;
      rtf->s_elt[rtf_cfg.C_ECZ[1][i_ecz]].ecz_u = i_ecz;
      rtf->s_ecz[i_ecz].s_tub.area      = rtf_cfg.TUBE_AREA[i_ecz];
      rtf->s_ecz[i_ecz].s_tub.emi       = rtf_cfg.emi_tub;
      rtf->s_ecz[i_ecz].i_zon           = rtf_cfg.C_ECZ[2][i_ecz];
   } // For all ECZ (elementary control zone)

   //for ( int i_elt=0 ; i_elt < rtf->nb_elt; i_elt++ )
   //{
   //   ecz_d[i_elt] = rtf->s_elt[i_elt].ecz_d;
   //   ecz_u[i_elt] = rtf->s_elt[i_elt].ecz_u;
   //   wall[i_elt] = rtf->s_elt[i_elt].i_wall;
   //   roll[i_elt] = rtf->s_elt[i_elt].i_roll;
   //}

   return;
} // void

/*============================================================================
/ RTF_dynamic_init: Dynamic RTF data initialization.
/
/ Revisions:
/  Created:       2004          P.Dubois
/  Modified: 
/===========================================================================*/
void RTF_dynamic_init(struct rtf_struct *rtf, float thi, float wid, float emi, float entry_tmp)
{
   float e[3], s[3];
	int elt_u, elt_d;

   int n_rtf = rtf->n_rtf;
   rtf->s_elt[rtf->first_strip_elt].s_strip.tmp_in_strip = entry_tmp;
 
   // For all ELT
   // RTH
   for ( int i_elt=0; i_elt < rtf->nb_elt ; i_elt++ )
   {
      // If the element is a wall.
      if ( rtf->s_elt[i_elt].i_wall )
      {
         rtf->s_elt[i_elt].s_strip.wid = rtf_cfg[n_rtf].width ;
         rtf->s_elt[i_elt].s_strip.emi = rtf_cfg[n_rtf].emi_wal;
      }

      else // Else: The ELT is not a wall
      {
         rtf->s_elt[i_elt].s_strip.thi = thi;
         rtf->s_elt[i_elt].s_strip.wid = wid;

         // If the element is not a roll
         if ( !rtf->s_elt[i_elt].i_roll ) rtf->s_elt[i_elt].s_strip.emi = emi; ;
      } // Else: The ELT is not a wall
   } // For all ELT

   // For all ECZ
   for ( int i_ecz=0; i_ecz<rtf->nb_ecz; i_ecz++ )
   {
      elt_u = rtf->s_ecz[i_ecz].elt_u;
      elt_d = rtf->s_ecz[i_ecz].elt_d;
      e[0] = rtf->s_elt[elt_u].s_strip.emi;
      s[0] = 1.f * rtf->s_elt[elt_u].s_strip.lgt;
      e[1] = rtf->s_ecz[i_ecz].s_tub.emi;
      s[1] = rtf->s_ecz[i_ecz].s_tub.area;
      e[2] = rtf->s_elt[elt_d].s_strip.emi;        
      s[2] = 1.f * rtf->s_elt[elt_d].s_strip.lgt;

      // Calculation of the radiation factors according to the form factor, the emissivity 
      //    and the surface of each element.
      // ---------------------------------------------------------------------------------
      ns_STD::cl_RTF::RTF_Radiation_factor ( e, s, rtf->s_ecz[i_ecz].fr, &rtf->s_ecz[i_ecz].deter, rtf_cfg[n_rtf].ff22 );
   } // For all ECZ
      
   return;
} // void

/*============================================================================
/ RTF_power: Calculation of the total gas flow in percentage of the connected inputs.
/ 
/ Revisions:
/  Created:       2004          P.Dubois
/  Modified: 
/===========================================================================*/
void RTF_power( struct rtf_struct *rtf, float power[] )
{
   // Initializations
   for ( int i_zon=0; i_zon < rtf->nb_zon; i_zon++ ) power[i_zon] = 0.f;

   // Heat flow by zone [W].
   for ( int i_ecz=0; i_ecz < rtf->nb_ecz; i_ecz++ )
   {
      // Sum up of all radiative flows given by the tubes
      power[ rtf->s_ecz[i_ecz].i_zon ] += rtf->s_ecz[i_ecz].flux;
   }
      
   
   // For all zones
   for ( int i_zon=0; i_zon < rtf->nb_zon; i_zon++ ) 
   {
      // Power in % of connected input
      power[i_zon] = power[i_zon] /rtf_cfg[rtf->n_rtf].MAX_POWER[i_zon] * 100.f / s_adp[rtf->n_rtf].Effi[i_zon];
      power[i_zon] = min(power[i_zon], 100.f);
      power[i_zon] = max(power[i_zon],   0.f);
   } // For all zones

   return;
} // void

/*============================================================================
/ RTF_strip: Strip temperature calculation at pyrometer location.
/ 
/ Revisions:
/  Created:       2004          P.Dubois
/  Modified: 
/===========================================================================*/
bool RTF_strip( struct rtf_struct *rtf, bool roll, float tmp_tub[], float *tmp_stp )
{   
   EnterCriticalSection(&Sec_SPG_Strip);     // TEST // To avoid this function to be called from SPS and APG at the same time

   bool status = true;
   string sub_name = "RTF_strip";

   // for all ECZ
   //RTF
   for ( int i_ecz=0; i_ecz< rtf->nb_ecz; i_ecz++ )
   {
      rtf->s_ecz[i_ecz].s_tub.tmp_tub = tmp_tub[rtf->s_ecz[i_ecz].i_zon ];
   }

   // Compute strip temperature over whole furnace
   // --------------------------------------------
   status = RTF_model( rtf, roll );
   if ( !status ) ns_STD::cl_TRC::ERR_Write(fur_cfg[rtf->n_rtf + RTH].fur_name.c_str(), sub_name, "Error in RTF_model");

   // If the number of soaking pass may change
   //if (SOA_PASS_CHANGE) *tmp_stp = Specific::Pyrometer (rtf);
   //else                 *tmp_stp = rtf->s_elt[ELT_PYRO_RTH].s_strip.tmp_out_strip;
   
   *tmp_stp = rtf->s_elt[rtf->elt_pyro].s_strip.tmp_out_strip;

   LeaveCriticalSection(&Sec_SPG_Strip);

   return status;
} // void

/*============================================================================
/ RTF_model: Calculation of the strip temperature.
/
/ Revisions:
/  Created:       2004          P.Dubois
/  Modified: 
/===========================================================================*/
bool RTF_model( struct rtf_struct *rtf, bool roll )
{
   bool status = true;
   string sub_name = "RTF_model";

   // Initialization of the strip temperatures
   RTF_ini_array (rtf);

   int loop = 0;
   bool ok = false;

   // Calculation of the strip temperatures until no change
   do  
   {
      // calculation of the strip temperature on all the elements
      status = RTF_model_strip( rtf, roll );
      if ( !status ) ns_STD::cl_TRC::ERR_Write(fur_cfg[rtf->n_rtf + RTH].fur_name.c_str(), sub_name, "Error in RTF_model_strip");

      // Check if the calculation has converge
      RTF_array_compare( rtf, &ok );
      
      loop ++;
   } while ( !ok && loop < MAX_ITER);

   // Alarm treatment "Error over looping"
   //if ( loop >= MAX_ITER ) 
   if ( loop >= 40 )			// to avoid extra iterations - explored that when it goes after 40 in 86% of cases it reaches 100
   {
      status = false;
      ns_STD::cl_TRC::ERR_Write(fur_cfg[rtf->n_rtf + RTH].fur_name.c_str(), sub_name, "Convergence error in RTF_model");
   }
   
   //ns_STD::cl_TRC::TRC_Write("INF", "RTF_model", sub_name, "Number of iterations = %d", loop);

   return status;
} // void

/*============================================================================
/ RTF_array_compare: Compare strip exit temperature at current and previous steps.
/ 
/ Revisions:
/  Created:       2004          P.Dubois
/  Modified: 
/===========================================================================*/
void RTF_array_compare( struct rtf_struct *rtf, bool *flag )
{
	const float MAX_GAP = 2.7f;
   int n_elt=0;

   *flag = true;
   do
   {
		*flag = !( fabs( rtf->s_elt[n_elt].s_strip.tmp_out_strip - 
                       s_cur.old_ex_t_rtf[rtf->n_rtf][n_elt]) >= MAX_GAP );
      n_elt++;
   } while ( n_elt < rtf->nb_elt && *flag );

   for ( int i_elt=0; i_elt < rtf->nb_elt; i_elt++ ) 
   {
      s_cur.old_ex_t_rtf[rtf->n_rtf][i_elt] = rtf->s_elt[i_elt].s_strip.tmp_out_strip;
   }

   return;
} // void

/*============================================================================
/ RTF_model_strip: Calculation of the strip temperature.
/
/ Revisions:
/  Created:       2004           P.Dubois
/  Modified:      2016-May       JC.Mitais   Use of last_tmp_in
/===========================================================================*/
bool RTF_model_strip (struct rtf_struct *rtf, bool roll)
{
   bool status = true;
   string sub_name = "RTF_model_strip";

   float t1, t2, e_q1, e_q2, e_q, r_q, cp, delta_t;
   float last_tmp_in = fur_cfg[rtf->n_rtf].entry_tmp;

   // For all elements
   for ( int i_elt=0; i_elt < rtf->nb_elt; i_elt++ )
   {
      // If not first element
      if ( i_elt>0 )
      {
         // if the previous and the pre-previous are not wall
         if ( !rtf->s_elt[i_elt].i_wall && !rtf->s_elt[i_elt-1].i_wall ) 
         {
            rtf->s_elt[i_elt].s_strip.tmp_in_strip = rtf->s_elt[i_elt-1].s_strip.tmp_out_strip;
         }

         else if ( i_elt-2 > 0 && !rtf->s_elt[i_elt-2].i_wall )
         {
            rtf->s_elt[i_elt].s_strip.tmp_in_strip = rtf->s_elt[i_elt-2].s_strip.tmp_out_strip;
         }

         else
         {
            rtf->s_elt[i_elt].s_strip.tmp_in_strip = last_tmp_in;
         } 
      } // If not first element

      // If the strip is not on a roll, calculation of its temperature by calculation of the heat exchange
      // =================================================================================================
      if ( !rtf->s_elt[i_elt].i_roll ) 
      {
         // Calculation of the heat balance for 2 temperatures
         t1 = rtf->s_elt[i_elt].s_strip.tmp_mid_strip;

         // Calculation of the heat exchange between the tube and the element (strip or wall)
         status = RTF_heat_exchange( rtf, i_elt, &e_q1 );
         if ( !status ) ns_STD::cl_TRC::ERR_Write(fur_cfg[rtf->n_rtf + RTH].fur_name.c_str(), sub_name, "Error in RTF_heat_exchange (1)");

         t2 = t1;
         e_q2 = e_q1;

         int loop = 0;
         // while we have found a second temperature where the exchange is reversed
         do 
         {
            if ( e_q1 > 0 )
            {
               t2 = t2 - 50.f;
					t2 = max( t2, 0.f);
               rtf->s_elt[i_elt].s_strip.tmp_mid_strip = t2;
               status = RTF_heat_exchange( rtf, i_elt, &e_q2 );
            }

            else
            {
               t1 = t1 + 50.f;
               rtf->s_elt[i_elt].s_strip.tmp_mid_strip = t1;
               status = RTF_heat_exchange (rtf, i_elt, &e_q1);
            }

            loop ++;
         } while ( e_q2*e_q1 >= 0.0001f && t2 > 0.f && loop < MAX_ITER );

         // Alarm treatment "Error over looping"
         if ( loop >= MAX_ITER ) 
         {
            status = false;
            ns_STD::cl_TRC::ERR_Write(fur_cfg[rtf->n_rtf + RTH].fur_name.c_str(), sub_name, "Error convergence loop 1 (2)");
         }
         
         // ----------------------------------------------------------------------------------
         loop = 0;
         // Calculation of the temperature by dichotomy
         do
         {
            delta_t = fabs ( t2 - t1 ) / 2.f;
            rtf->s_elt[i_elt].s_strip.tmp_mid_strip = t1 - delta_t;

            // exchange coming to the strip
            status = RTF_heat_exchange (rtf, i_elt, &e_q);
            if ( !status ) ns_STD::cl_TRC::ERR_Write(fur_cfg[rtf->n_rtf + RTH].fur_name.c_str(), sub_name, "Error in RTF_heat_exchange (3)");

            // positive flow
            if (e_q >0)
            {
               e_q1 = e_q;
               t1 = rtf->s_elt[i_elt].s_strip.tmp_mid_strip;
            }

            else
            {
               e_q2 = e_q;
               t2 = rtf->s_elt[i_elt].s_strip.tmp_mid_strip;
            }

            loop ++;
         } while (fabs(e_q) > 0.1f && delta_t > 0.001 && loop < MAX_ITER);
         // Calculation of the temperature by dichotomy

         // Alarm treatment "Error over looping".
         //if ( loop >= MAX_ITER && delta_t > 0.1)
         if ( loop >= 40 && delta_t > 0.1)			// to avoid extra iterations - explored that when it goes after 40 in 86% of cases it reaches 100
         {
            status = false;
            ns_STD::cl_TRC::ERR_Write(fur_cfg[rtf->n_rtf + RTH].fur_name.c_str(), sub_name, "Error convergence loop (4)");
         }

         //ns_STD::cl_TRC::TRC_Write("INF", "Temp calc", sub_name, "Number of iterations = %d", loop);

      } // If the strip is not on a roll

      // Else, the i_elt is on a roll, we calculate the temperature drop of the strip exiting of the roll.
      // ===============================================================================================
      else
      {
         // Only if the model has to take into account the roll.
         if ( roll )
         {
            // Roll steel calorific value.
            cp = STEEL_specific_heat( rtf->s_elt[i_elt].s_strip.tmp_mid_strip );
            if ( cp <= 0.f ) 
            {
               status = false;
               ns_STD::cl_TRC::ERR_Write(fur_cfg[rtf->n_rtf + RTH].fur_name.c_str(), sub_name, "Cp not valid: %f (5)", cp);
               return status;
            }

            // ========================================================================================
            // JAB:
            // exchange recieved by the strip 
            // normaly the calculation should use the middle temperature of the strip.
            // but use this temeperature may make the model not cenvergent with array compare, when the 
            // thermal exchange between the strip and the rolls is udge 
            // That why the model use the entry temperature of the strip.
            // it is no problematic, because the the thermal exchange is adapted on each plant
            // ========================================================================================
            r_q = rtf->s_elt[i_elt].s_strip.wid * rtf->s_elt[i_elt].s_strip.lgt * 
                  rtf->s_elt[i_elt].s_rol.coeff *
                  (rtf->s_elt[i_elt].s_strip.tmp_in_strip - rtf->s_elt[i_elt].s_rol.r_t_b_c) / 2.f;
            // ========================================================================================
            // Check speed value validity
            if ( rtf->spd <= 0.f ) 
            {
               status = false;
               ns_STD::cl_TRC::ERR_Write(fur_cfg[rtf->n_rtf + RTH].fur_name.c_str(), sub_name, "Speed not valid: %f (6)", rtf->spd);
               return status;
            }

            // Check strip width value validity
            if ( rtf->s_elt[i_elt].s_strip.wid <= 0.f ) 
            {
               status = false;
               ns_STD::cl_TRC::ERR_Write(fur_cfg[rtf->n_rtf + RTH].fur_name.c_str(), sub_name, "Strip width not valid: %f (7)", rtf->s_elt[i_elt].s_strip.wid);
               return status;
            }

            // Check strip thickness value validity
            if ( rtf->s_elt[i_elt].s_strip.thi <= 0.f ) 
            {
               status = false;
               ns_STD::cl_TRC::ERR_Write(fur_cfg[rtf->n_rtf + RTH].fur_name.c_str(), sub_name, "Strip thickness not valid: %f (8)", rtf->s_elt[i_elt].s_strip.thi);
               return status;
            }

            // calculation of the strip exit temperature on the roll
            rtf->s_elt[i_elt].s_strip.tmp_out_strip = ns_STD::cl_BIS::BISRA_Tmp(2, 
                                 ns_STD::cl_BIS::BISRA_Enth(2, rtf->s_elt[i_elt].s_strip.tmp_in_strip) - 
                                 r_q / rtf->spd / rtf->s_elt[i_elt].s_strip.wid / rtf->s_elt[i_elt].s_strip.thi / 
                                 RO_STEEL );
 
            // calculation of the middle temperature on the roll
            rtf->s_elt[i_elt].s_strip.tmp_mid_strip = ( rtf->s_elt[i_elt].s_strip.tmp_in_strip + 
                                                        rtf->s_elt[i_elt].s_strip.tmp_out_strip ) / 2.f;

            // if strip enter temperature on the roller is higher than roller temperature, mid and
            //    exit can't be lower
            if (rtf->s_elt[i_elt].s_strip.tmp_in_strip > rtf->s_elt[i_elt].s_rol.r_t_b_c)
            {
               rtf->s_elt[i_elt].s_strip.tmp_out_strip = max(rtf->s_elt[i_elt].s_strip.tmp_out_strip, 
                                                             rtf->s_elt[i_elt].s_rol.r_t_b_c);
               rtf->s_elt[i_elt].s_strip.tmp_mid_strip = max(rtf->s_elt[i_elt].s_strip.tmp_mid_strip, 
                                                             rtf->s_elt[i_elt].s_rol.r_t_b_c);
               rtf->s_elt[i_elt].s_strip.tmp_out_strip = min(rtf->s_elt[i_elt].s_strip.tmp_out_strip, 
                                                             rtf->s_elt[i_elt].s_strip.tmp_in_strip);
               rtf->s_elt[i_elt].s_strip.tmp_mid_strip = min(rtf->s_elt[i_elt].s_strip.tmp_mid_strip, 
                                                             rtf->s_elt[i_elt].s_strip.tmp_in_strip);
            }

            // if strip enter temperature on the roller is lower than roller temperature, mid and 
            //    exit can't be higher
            else
            {
               rtf->s_elt[i_elt].s_strip.tmp_out_strip = min(rtf->s_elt[i_elt].s_strip.tmp_out_strip, 
                                                             rtf->s_elt[i_elt].s_rol.r_t_b_c);
               rtf->s_elt[i_elt].s_strip.tmp_mid_strip = min(rtf->s_elt[i_elt].s_strip.tmp_mid_strip, 
                                                             rtf->s_elt[i_elt].s_rol.r_t_b_c);
               rtf->s_elt[i_elt].s_strip.tmp_out_strip = max(rtf->s_elt[i_elt].s_strip.tmp_out_strip, 
                                                             rtf->s_elt[i_elt].s_strip.tmp_in_strip);
               rtf->s_elt[i_elt].s_strip.tmp_mid_strip = max(rtf->s_elt[i_elt].s_strip.tmp_mid_strip, 
                                                             rtf->s_elt[i_elt].s_strip.tmp_in_strip);
            }

            // calculation of the roll average temperature after this calculation
            // Imply a philosophy of transient calculation, but not implemented (this temperatue 
            //    will not be used after)
            rtf->s_elt[i_elt].s_rol.r_t_a_c = rtf->s_elt[i_elt].s_rol.r_t_b_c + 
                                              r_q / cp / rtf->s_elt[i_elt].s_rol.weigth / 
                                                rtf->spd * rtf->s_elt[i_elt].s_strip.wid;
         } // Only if the model has to take into account the roll

         else // Else the model has not to take into account the roll
         {
            rtf->s_elt[i_elt].s_strip.tmp_out_strip = rtf->s_elt[i_elt].s_strip.tmp_in_strip;
            rtf->s_elt[i_elt].s_strip.tmp_mid_strip = rtf->s_elt[i_elt].s_strip.tmp_in_strip;
            rtf->s_elt[i_elt].s_rol.r_t_a_c = rtf->s_elt[i_elt].s_strip.tmp_mid_strip;
         } // Else the model has not to take into account the roll
      } // Else, the i_elt is on a roll

      if (!rtf->s_elt[i_elt].i_wall) last_tmp_in = rtf->s_elt[i_elt].s_strip.tmp_out_strip;
   } // For all elements

   return status;
} // void

/*============================================================================
/ RTF_heat_exchange: Calculation of the heat exchange between the tube and 
/     the element (strip or wall) or between the roll and the strip if this 
/     calculation is requested.
/
/ Revisions:
/  Created:       2004          P.Dubois
/  Modified: 
/===========================================================================*/
bool RTF_heat_exchange( struct rtf_struct *rtf, int n_elt, float *e_q )
{
   bool status = true;
   string sub_name = "RTF_heat_exchange";

   float q=0.f;
   int ecz_u, ecz_d;
   float dq1, dq2, dq3, cp;

   ecz_u = rtf->s_elt[n_elt].ecz_u;
   ecz_d = rtf->s_elt[n_elt].ecz_d;

   // If the upstream ecz exists, calculation of the heat exchange
   // ------------------------------------------------------------
   if (ecz_u >= 0)
   {
		// Calculation of the heat flow density according to the temperature, 
      //    the emissivity and the surface of each element
      status = ns_STD::cl_RTF::RTF_Radiation_calcul (rtf->s_ecz[ecz_u], rtf->s_elt, rtf->nb_elt, &dq1, &dq2, &dq3);
      if ( !status ) ns_STD::cl_TRC::ERR_Write(fur_cfg[rtf->n_rtf + RTH].fur_name.c_str(), sub_name, "Error in RTF_radiation_calcul");

      q = q - dq3 * rtf->s_elt[n_elt].s_strip.wid * rtf->s_elt[n_elt].s_strip.lgt;
		
		// flow radiation given by tube
      rtf->s_ecz[ecz_u].flux = dq2 * rtf->s_ecz[ecz_u].s_tub.area;   
   } // If the upstream ecz exists

   // If the downstream ecz exists, calculation of the heat exchange
   // --------------------------------------------------------------
   if (ecz_d >= 0)
   {
		// Calculation of the heat flow density according to the temperature, 
      //    the emissivity and the surface of each element
      status = ns_STD::cl_RTF::RTF_Radiation_calcul (rtf->s_ecz[ecz_d], rtf->s_elt, rtf->nb_elt, &dq1, &dq2, &dq3);
      if ( !status )
      {
         ns_STD::cl_TRC::ERR_Write(fur_cfg[rtf->n_rtf + RTH].fur_name.c_str(), sub_name, "Error in RTF_radiation_calcul");
         //return status;
      }

      q = q - dq1 * rtf->s_elt[n_elt].s_strip.wid * rtf->s_elt[n_elt].s_strip.lgt;
   } // If the downstream ecz exists

   // If the element is a wall we consider the wall as adiabatic
   // ----------------------------------------------------------
   if ( rtf->s_elt[n_elt].i_wall ) *e_q = -q;

   // Else: The element is a strip -> calculation between the heat exchange given by the tube and 
   //    the heat taken by the strip.
   // -------------------------------------------------------------------------------------------
   else
   {
      // Out strip temperature calculation
      rtf->s_elt[n_elt].s_strip.tmp_out_strip = rtf->s_elt[n_elt].s_strip.tmp_in_strip + 
                                              2.f * ( rtf->s_elt[n_elt].s_strip.tmp_mid_strip - 
                                              rtf->s_elt[n_elt].s_strip.tmp_in_strip );

      // Steel specific heat calculation
      cp = STEEL_specific_heat( rtf->s_elt[n_elt].s_strip.tmp_mid_strip );

      // calculation of the heat exchange recieved by the strip
      *e_q = rtf->spd * rtf->s_elt[n_elt].s_strip.wid * rtf->s_elt[n_elt].s_strip.thi * RO_STEEL * cp * 
             ( rtf->s_elt[n_elt].s_strip.tmp_out_strip - rtf->s_elt[n_elt].s_strip.tmp_in_strip ) - q;
   } // Else: The element is a strip

   return status;
} // void

/*============================================================================
/ RTF_ini_array: Initialize the wall and strip elements temperature for the first iteration
/ 
/ Revisions:
/  Created:       2004          P.Dubois
/  Modified: 
/===========================================================================*/
void RTF_ini_array( struct rtf_struct *rtf )
{
   //const float DELTA_T = (50.f) / float(rtf_cfg[ rtf->n_rtf].nb_elt) / 2.f;
	float tmp_in = rtf->s_elt[rtf->first_strip_elt].s_strip.tmp_in_strip;
	float tmp_out = rtf->s_elt[rtf->nb_elt-2].s_strip.tmp_out_strip;
   const float DELTA_T = abs(tmp_out - tmp_in) / float(rtf->nb_elt) / 2.f;

   const float INT_WALL_T = 700.f + TK;

   // For all elements
   for ( int i_elt=0; i_elt < rtf->nb_elt; i_elt++ )
   {
      // If the element is a wall
      if ( rtf->s_elt[i_elt].i_wall )
      {
         rtf->s_elt[i_elt].s_strip.tmp_mid_strip = INT_WALL_T;
      } // If the element is a wall

      else // If the element is not a wall
      {
         // If not first strip element
         if ( i_elt > rtf->first_strip_elt ) 
         {
            rtf->s_elt[i_elt].s_strip.tmp_in_strip = rtf->s_elt[i_elt-1].s_strip.tmp_out_strip;
         }

         rtf->s_elt[i_elt].s_strip.tmp_mid_strip = rtf->s_elt[i_elt].s_strip.tmp_in_strip + DELTA_T;
         rtf->s_elt[i_elt].s_strip.tmp_out_strip = rtf->s_elt[i_elt].s_strip.tmp_mid_strip + DELTA_T;
      } // If the element is not a wall
   } // For all elements

   return;
} // void
