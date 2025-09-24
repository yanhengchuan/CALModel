/*============================================================================
/ SPS_SPD.cpp: Speed management.
/  
/ (c) CMI Industry
/===========================================================================*/
#include "stdafx.h"

/*============================================================================
/ SPS_RTF_decr_spd: Decrease speed in V-RTF
/ 
/ Revisions:
/  Created:       2005-Aug      P.DUBOIS
/  Modified: 
//===========================================================================*/
bool SPS_RTF_decr_spd( struct sps_coil_data *sps_fin_sp_rtf, struct sps_mea_coil_data *sps_mea_rtf, 
                       struct sps_exit_data *sps_act_sp_rtf, float delta_po_rtf[], int n_rtf)
{   
   bool status = true;
   string sub_name = "SPS_RTF_decr_spd";

   float delta_spd, coef, tmp_limit;

	static int count_spd_dec = 0;


   // Traces treatment.
   bool TRC=ns_STD::cl_TRC::TRC_Sub( sub_name ); 

   // The strip temperature set point is equal to the coil set point temperature
   //sps_act_sp[RTH].tmp_stp = sps_fin_sp_rtf->tmp_tgt;
   sps_act_sp[n_rtf].tmp_stp = sps_fin_sp_rtf->tmp_tgt;                                      // test
   coef = 0.8f / 1000.f / sps_fin_sp_rtf->thi;

   // If the strip temperature measurement is higer than the lower tolerance + 1°C
	if (sps_mea_rtf->tmp_stp > sps_fin_sp_rtf->tmp_low + 1.f)
   {
      // If the speed is close to the set_point
      if (sps_mea_rtf->spd < sps_fin_sp_rtf->spd + 40./60.)
      {
         // For all RTF zones
         for ( int i_zon=0; i_zon < fur_cfg[n_rtf].nb_zon; i_zon++ )
         {
            if (sps_mea_rtf->tmp_zon[i_zon] <= sps_fin_sp_rtf->tmp_zon[i_zon] + 15.f * coef)
               delta_po_rtf[i_zon] = 0.f;
            else if (sps_mea_rtf->tmp_zon[i_zon] <= sps_fin_sp_rtf->tmp_zon[i_zon] + 25.f * coef && delta_po_rtf[i_zon] > -30.f)
               delta_po_rtf[i_zon] = 0.f;//-15.f;
            else if (sps_mea_rtf->tmp_zon[i_zon] > sps_fin_sp_rtf->tmp_zon[i_zon] + 25.f * coef)
               delta_po_rtf[i_zon] = 0.f;//-30.f;
         } // For all RTF zones
      } // If the speed is close to the set_point

		// If the speed is not closed to the final set point the power could be decreased of 60 %
		else
		{
         // For all RTF zones
			for ( int i_zon=0; i_zon < fur_cfg[n_rtf].nb_zon; i_zon++ )
			{
            if (sps_mea_rtf->tmp_zon[i_zon] <= sps_fin_sp_rtf->tmp_zon[i_zon] + 20.f * coef)
               delta_po_rtf[i_zon] = 0.f;
            else if (sps_mea_rtf->tmp_zon[i_zon] <= sps_fin_sp_rtf->tmp_zon[i_zon] + 40.f * coef)
               delta_po_rtf[i_zon] = 0.f;//-10.f;
            else if (sps_mea_rtf->tmp_zon[i_zon] > sps_fin_sp_rtf->tmp_zon[i_zon] + 40.f * coef )
               delta_po_rtf[i_zon] = 0.f;//-20.f;
			} // For all RTF zones
		} // If the speed is not closed to the final set point the power could be decreased of 60 %
	} // If the strip temperature measurement is higer than the lower tolerance + 1°C

   // L-TOP decreases the speed only if the strip temperature is lower than the final strip temperature
	if(sps_entry[n_rtf].s_trans_coil.anticipation_time < sps_entry[n_rtf].s_mea.tim)
		tmp_limit = max(sps_fin_sp_rtf->tmp_tgt, sps_fin_sp_rtf->tmp_upp + 50.f);// min( sps_fin_sp_rtf->tmp_tgt + 4.f, max(sps_fin_sp_rtf->tmp_tgt, sps_fin_sp_rtf->tmp_upp) );
	else
		tmp_limit = min( sps_fin_sp_rtf->tmp_tgt + 4.f, max(sps_fin_sp_rtf->tmp_tgt, sps_fin_sp_rtf->tmp_upp) );

   if ( sps_mea_rtf->tmp_stp <= tmp_limit )
   {
		

		if(sps_entry[n_rtf].s_trans_coil.anticipation_time >= sps_entry[n_rtf].s_mea.tim)
		{
			delta_spd = 0.15f / 60.f * coef;
			count_spd_dec --;
			if (count_spd_dec < 0) count_spd_dec = 0;
			// If the measured strip temperature is lower than the previous strip temperature, decrease a lot the speed
			//if ( sps_mea_rtf->tmp_stp < s_cur.prev_strip_temp_rtf[rtf->n_rtf] - 0.3f) 
			if (count_spd_dec == 0)
			{
				if ( sps_mea_rtf->tmp_stp < s_cur.prev_strip_temp_rtf[n_rtf] - 0.3f)                               // test
					delta_spd = min( max( 1.f*(sps_fin_sp_rtf->tmp_tgt+4.f - sps_mea_rtf->tmp_stp)*coef, 0.05f), 4.f ) / 60.f; //tgt+4.f 0.05f

				// Less speed decrease: depends on the strip temperature measurement regarding the target
				else if (sps_mea_rtf->tmp_stp < sps_fin_sp_rtf->tmp_tgt - 8.f)		
	    			delta_spd = min(  0.35f*coef,   0.35f) / 60.f;
				else if (sps_mea_rtf->tmp_stp < sps_fin_sp_rtf->tmp_tgt - 5.5f)	
	    			delta_spd = min( 0.25f*coef,  0.25f) / 60.f;
				else if (sps_mea_rtf->tmp_stp < sps_fin_sp_rtf->tmp_upp - 0.f)//(sps_mea_rtf->tmp_stp < sps_fin_sp_rtf->tmp_tgt - 0.f)	
	    			delta_spd = min(0.1f*coef, 0.1f) / 60.f;
			}

			count_spd_dec = 6;
		}

		else
		{
			// If the measured strip temperature is lower than the previous strip temperature, decrease a lot the speed
			//if ( sps_mea_rtf->tmp_stp < s_cur.prev_strip_temp_rtf[rtf->n_rtf] - 0.3f)
			delta_spd = 0.2f / 60.f * coef;

			if ( sps_mea_rtf->tmp_stp < s_cur.prev_strip_temp_rtf[n_rtf] - 0.3f)                               // test
				delta_spd = min( max( 1.f*(sps_fin_sp_rtf->tmp_tgt+14.f - sps_mea_rtf->tmp_stp)*coef, 0.65f), 4.f ) / 60.f; //tgt+4.f 0.05f

			// Less speed decrease: depends on the strip temperature measurement regarding the target
			else if (sps_mea_rtf->tmp_stp < sps_fin_sp_rtf->tmp_tgt - 7.f)		
	    		delta_spd = min(  1.f*coef,   1.f) / 60.f;
			else if (sps_mea_rtf->tmp_stp < sps_fin_sp_rtf->tmp_tgt - 3.5f)	
	    		delta_spd = min( 0.3f*coef,  0.3f) / 60.f;
			else if (sps_mea_rtf->tmp_stp < sps_fin_sp_rtf->tmp_upp - 0.f)//(sps_mea_rtf->tmp_stp < sps_fin_sp_rtf->tmp_tgt - 0.f)	
	    		delta_spd = min(0.2f*coef, 0.2f) / 60.f;
		}
      // Calculation of the new speed set points
      //sps_act_sp[RTH].spd = max (sps_act_sp[RTH].spd - delta_spd, sps_fin_sp_rtf->spd);
      sps_act_sp[n_rtf].spd = max (sps_act_sp[n_rtf].spd - delta_spd, sps_fin_sp_rtf->spd);              // test
   }

   // Traces treatment
   if (TRC) ns_STD::cl_TRC::TRC_Write("INF", "SPS", sub_name, "sps_act_sp[%d]: %.0f", n_rtf, sps_act_sp[n_rtf].spd*60.f );

   return status;
} // void

/*============================================================================
/ SPS_RTF_incr_spd: Increase speed in V-RTF
/ 
/ Revisions:
/  Created:       2005-Aug      P.DUBOIS
/  Modified: 
//===========================================================================*/
bool SPS_RTF_incr_spd( struct sps_coil_data *sps_fin_sp_rtf, struct sps_mea_coil_data *sps_mea_rtf, 
                       struct sps_exit_data *sps_act_sp_rtf, float delta_po_rtf[], int n_rtf )
{   
   bool status = true;
   string sub_name = "SPS_RTF_incr_spd";

	float delta_spd, coef, tmp_limit;
	static int count;
	static int count_spd = 0;

   // Traces treatment.
   bool TRC=ns_STD::cl_TRC::TRC_Sub( sub_name );

	//count ++;                   // test

	/* Strip temperature set point is equal to the coil set point temperature */
   //sps_act_sp[RTH].tmp_stp = sps_fin_sp_rtf->tmp_tgt;
   sps_act_sp[n_rtf].tmp_stp = sps_fin_sp_rtf->tmp_tgt;                                                 // test
   coef = 1.f / 1000.f / sps_fin_sp_rtf->thi;

   // ========================================================================================================================================
   // the following condition is usefull for line restart procedure. During a line restart, heat buckles issues may happen. 
   // It is recommended to restart the line quickly, even if the strip temperature is too low.
   // This is the goal of the following condition: detect line starting and perform an accelaration even if the strip temperature is not good
   // a condition has also to be set to not decrease the line speed in the sps, even if the strip temperature is too low.
   // ========================================================================================================================================
   // If the strip temperature measurement is not too high
	if (sps_mea_rtf->tmp_stp < sps_fin_sp_rtf->tmp_upp - 1.f)
   {
      // If the measured line speed is near from the final speed
		if ( sps_mea_rtf->spd > sps_fin_sp_rtf->spd - 20.f/60.f )
		{
			// For each zone if the tube temperature is close to the final set point,
			//    the power is equal to the final power set point
         // For all RTF zones
         for ( int i_zon=0; i_zon < fur_cfg[n_rtf].nb_zon; i_zon++ )
			{
            // if the measured tube temperature is too near from the target
            if      (sps_mea_rtf->tmp_zon[i_zon] >= sps_fin_sp_rtf->tmp_zon[i_zon] - 10.f * coef) 
               delta_po_rtf[i_zon] = 0.f;

            else if (sps_mea_rtf->tmp_zon[i_zon] >= sps_fin_sp_rtf->tmp_zon[i_zon] - 20.f * coef && delta_po_rtf[i_zon] < 30.f)
               delta_po_rtf[i_zon] = 0.f;//15.f;

            // The measured tube temperature is too low
            else if (sps_mea_rtf->tmp_zon[i_zon] <  sps_fin_sp_rtf->tmp_zon[i_zon] - 20.f * coef) 
               delta_po_rtf[i_zon] = 0.f;//30.f;
			} // For all RTF zones
		} // If the measured line speed is near from the final speed

		// If the speed is not closed to the final set point
      else
      {
         // For all RTF zones
         for ( int i_zon=0; i_zon < fur_cfg[n_rtf].nb_zon; i_zon++ )
         {
            // if the measured tube temperature is too near from the target
            if      (sps_mea_rtf->tmp_zon[i_zon] >= sps_fin_sp_rtf->tmp_zon[i_zon] - 20.f * coef)             
               delta_po_rtf[i_zon] = 0.f;

            else if (sps_mea_rtf->tmp_zon[i_zon] >= sps_fin_sp_rtf->tmp_zon[i_zon] - 40.f * coef) 
               delta_po_rtf[i_zon] = 0.f;//10.f;

            // The measured tube temperature is too low
            else if (sps_mea_rtf->tmp_zon[i_zon] <  sps_fin_sp_rtf->tmp_zon[i_zon] - 40.f * coef) 
               delta_po_rtf[i_zon] = 0.f;//20.f;      
         } // For all RTF zones
      } // If the speed is not closed to the final set point
	} // If the strip temperature measurement is not too high

   // If the line speed has not accelerated since 6 times in this function (1 minute)
	if      (count >  6)
 	{
      for (int i_zon=0; i_zon < fur_cfg[n_rtf].nb_zon; i_zon++)
         delta_po_rtf[i_zon] = 0.f;//40.f;
	}
   // If the model has just accelerated
	else if (count == 0)
   {
      // If the strip temperature measurement is higher than the target + 20°C
      if      (sps_mea_rtf->tmp_stp > sps_fin_sp_rtf->tmp_tgt + 20.f)
      {
         for (int i_zon=0; i_zon < fur_cfg[n_rtf].nb_zon; i_zon++)
            delta_po_rtf[i_zon] = 0.f;//-20.f;         
      }
      // If the strip temperature measurement is higher than the target + 10°C
      else if (sps_mea_rtf->tmp_stp > sps_fin_sp_rtf->tmp_tgt + 10.f)
      {
         for (int i_zon=0; i_zon < fur_cfg[n_rtf].nb_zon; i_zon++)
            delta_po_rtf[i_zon] = 0.f;//-10.f;
      }
	} // If the model has just accelerated

	/* Increases the speed only if the strip temperature is higher than the final strip temperature */
   tmp_limit = max( sps_fin_sp_rtf->tmp_tgt - 1.f, min(sps_fin_sp_rtf->tmp_tgt - 1.5f, sps_fin_sp_rtf->tmp_low + 10.f) ); //0.5f //-2.f and tmp_low + 7.f
   if ( (sps_mea_rtf->tmp_stp >= sps_fin_sp_rtf->tmp_upp + 1.5f) ||(sps_mea_rtf->tmp_stp >= tmp_limit && sps_mea_rtf->tmp_stp > s_cur.prev_strip_temp_rtf[n_rtf] - 0.01f) || // before when increasing + 0.08f
	   (sps_mea_rtf->tmp_stp >= sps_fin_sp_rtf->tmp_tgt - 7.f && sps_mea_rtf->tmp_stp > s_cur.prev_strip_temp_rtf[n_rtf] + 0.08f))//-2.5f //before 0.15.f
	{
	   count = -1;
	   count_spd --;
	   if (count_spd < 0) count_spd = 0;
      // default nomimal speed modification
	   if (count_spd == 0)
	   {
		   if (sps_mea_rtf->tmp_stp < s_cur.prev_strip_temp_rtf[n_rtf] - 0.03f  && sps_mea_rtf->tmp_stp < sps_fin_sp_rtf->tmp_upp - 1.f)
		   {
			   delta_spd = 0.f;
		   }
		   else
		   {
		  
			   delta_spd = 0.45f / 60.f * coef;//0.5

			 // If the measured strip temperature is higher than the previous strip temperature, increase a lot the speed, based on the srtip temperatuer measurement
			   if (sps_mea_rtf->tmp_stp > s_cur.prev_strip_temp_rtf[n_rtf] + 0.3f)
				   delta_spd = min(max(1.f*(sps_mea_rtf->tmp_stp - tmp_limit)*coef, 0.05f), 4.f) / 60.f;

			   // Less speed increase: depends on the strip temperature measurement regarding the target
			   else if (sps_mea_rtf->tmp_stp >= sps_fin_sp_rtf->tmp_tgt + 14.f)
				   delta_spd = min(6.f*coef, 1.4f) / 60.f; //6
			   else if (sps_mea_rtf->tmp_stp >= sps_fin_sp_rtf->tmp_tgt + 8.f)
				   delta_spd = min(3.f*coef, 1.2f) / 60.f; //3
			   else if (sps_mea_rtf->tmp_stp >= sps_fin_sp_rtf->tmp_tgt + 4.5f)
				   delta_spd = min(1.5f*coef, 0.7f) / 60.f; //1.5
				else if(sps_mea_rtf->tmp_stp >= sps_fin_sp_rtf->tmp_tgt-1 && sps_mea_rtf->tmp_stp > s_cur.prev_strip_temp_rtf[n_rtf] + 0.07f)
					delta_spd = min(2.f*coef, 0.8f) / 60.f; //1.5
			   else if (sps_mea_rtf->tmp_stp >= sps_fin_sp_rtf->tmp_tgt + 1)
				   delta_spd = min(0.75f*coef, 0.3f) / 60.f; //0.75

			 // Calculation of the new speed set points
			 //sps_act_sp[RTH].spd = min (sps_act_sp[RTH].spd + delta_spd, sps_fin_sp_rtf->spd);
			}
		  sps_act_sp[n_rtf].spd = min (sps_act_sp[n_rtf].spd + delta_spd, sps_fin_sp_rtf->spd);                          // test
		  count_spd = 6;
	   }
   }

	if (TRC) ns_STD::cl_TRC::TRC_Write("INF", "SPS", sub_name, "sps_act_sp[%d]->spd: %.0f m/mn", n_rtf, sps_act_sp[n_rtf].spd*60.f);

   return status;
} // void


