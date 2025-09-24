/*=============================================================================
/ INIT: Initializations.
/
/ INIT: Initializations.
/ SPS_INI_Alm: Initialize alarm text in user language.
/  
/ (c) CMI Industry
/===========================================================================*/
#include "stdafx.h"

/*=============================================================================
/ INIT: Initializations.
/
/ Revisions:
/  Created:       2007 Sep      JC.Mitais
/  Modified: 
/===========================================================================*/
bool INIT()
{
   bool status = true;
   string sub_name = "INIT";

   // Read traces definition file (à déplacer)
   // ========================================
   char fil_nam[50];
   strcpy( fil_nam, DIR_CFG );
   strcat( fil_nam, FILE_TRCDAT );
   ns_STD::cl_TRC::TRC_Read_File(fil_nam);
   
   SQL_DIMENSIONS_UPDATE ();

   // Alarm structure initialization
   // ==============================
   status = SPS_INI_Alm();

   // Specific initializations
   // ========================
   SPS_Specific::INIT();

   // Read configuration
   // ==================
   CFG_Read();

   // Rolls mass
   // ----------
   // For all rtf furnaces
   for (int i_rtf=0; i_rtf < NB_FUR_RTF; i_rtf++)
   {
      rtf_cfg[i_rtf].tot_rol_mass = rtf_cfg[i_rtf].one_rol_mass * float(rtf_cfg[i_rtf].nb_rol);
      for (int i_mesh=0; i_mesh < MAX_NB_MESH; i_mesh++) s_cur.old_ex_t_rol[i_rtf][i_mesh] = 0.f;
      for (int i_elt=0; i_elt < rtf_cfg[i_rtf].nb_elt; i_elt++) s_cur.old_ex_t_rtf[i_rtf][i_elt] = 0.f;

      // Check value for roll computation
      if (rol_cfg[i_rtf].n_rol_cmp >= rtf_cfg[i_rtf].nb_rol)
      {
         MessageBoxA(NULL, "Value from CFG file for rol_cmp is too great", NULL, NULL);

         ExitProcess(1);
      } 
   } // For all rtf furnaces

   // Initialize "old" data.
   // ======================
   // For all coils
   for ( int i_coil=0; i_coil < MAX_NB_COIL; i_coil++ )
   {
      s_coil.s_coil_data_old[i_coil].id = "---";
      s_coil.s_coil_data_old[i_coil].valid = true;
      s_coil.s_coil_data_old[i_coil].change = true;
      s_coil.s_coil_data_old[i_coil].thi = 0.f;
      s_coil.s_coil_data_old[i_coil].wid = 0.f;
      s_coil.s_coil_data_old[i_coil].lgt = 0.f;
      s_coil.s_coil_data_old[i_coil].spd = 0.f;

      // For all RTF furnaces
      for (int i_rtf=0; i_rtf < NB_FUR_RTF; i_rtf++)
      {
         s_coil.s_coil_data_old[i_coil].emi_rtf[i_rtf] = 0.f;
         s_coil.s_coil_data_old[i_coil].tmp_tgt[i_rtf] = 0.f;
         s_coil.s_coil_data_old[i_coil].tmp_upp[i_rtf] = 0.f;
         s_coil.s_coil_data_old[i_coil].tmp_low[i_rtf] = 0.f;

         // Emissivity parameters initialization
         // ====================================
         time(&s_cur.time_lst_adapt[i_rtf]);
      } // For all RTF furnaces
   } // For all coils

   // BISRA tables
   // ============
   string fil_name = (string)DIR_DAT + (string)FILE_BISRA;
   status = ns_STD::cl_BIS::Bisra_Data(fil_name);
   if (!status) ns_STD::cl_TRC::ERR_Write("BISRA", sub_name, "error no BISRA data file found");

   // Hottl tables
   // ============
   //status = Hottl_Data();

   // For all furnaces
   for (int i_fur=0; i_fur < MAX_NB_FUR; i_fur++)
   {
      // If adaptation is to be done
      if (s_adp[i_fur].use_ada)
      {
         s_adp[i_fur].s_exg.adp_calc = true;
         s_adp[i_fur].s_exg.s_calc.last_sign = 0;
         s_adp[i_fur].s_exg.s_calc.Tmp_last = -1.f;
         time(&s_adp[i_fur].s_exg.lst_time);
         status = SQL_ADAPTATION_SELECT(i_fur);

         // For all RTF zones
         for ( int i_zon=0; i_zon < fur_cfg[i_fur].nb_zon; i_zon++ )
         {
            s_adp[i_fur].Effi[i_zon] = EFFI_FURNACE;
         } // For all RTF zones

	      // Emissivity coefficient reading
	      // ==============================
         status = SQL_EMISSIVITY_SELECT(i_fur);

         // Read Efficacity values
	      // ======================
         status = SQL_EFFICIENCY_SELECT(i_fur);
      } // If adaptation is to be done

	   // Recover PID parameters
      // ======================
	   SQL_PID_SELECT(i_fur);
      fba_stp[i_fur].automatic = true;
      for (int i_zon=0; i_zon < fur_cfg[i_fur].nb_zon; i_zon++) fba_zon[i_fur][i_zon].automatic = true;
   } // For all furnaces

   // Init simulation if any
   // ======================
   if ( SIMULATION ) SIM_INI();

   // Risk management
   // ===============
   s_cur.Line_running = true;
	s_cur.record_Pt = 0;
   
   // For all RTF furnaces
   for (int i_rtf=0; i_rtf < NB_FUR_RTF; i_rtf++)
   {
      time(&s_cur.time_start[i_rtf]);
      // Compute RTF strip length
      // ========================
      fur_cfg[i_rtf].stp_lgt = 0.f;                                 
      if (i_rtf == 0)
         fur_cfg[i_rtf].xPyro = 0.f;
      else
         fur_cfg[i_rtf].xPyro = fur_cfg[i_rtf-1].xPyro + fur_cfg[i_rtf-1].foll_lgt;                  // test

      // For all ELT
      for ( int i_elt=0; i_elt < rtf_cfg[i_rtf].nb_elt; i_elt++ )
      {
         if ( rtf_cfg[i_rtf].C_ELT[0][i_elt] != 1.f) 
         {
            fur_cfg[i_rtf].stp_lgt += rtf_cfg[i_rtf].C_ELT[2][i_elt]; 
            if ( i_elt <= rtf_cfg[i_rtf].elt_pyro ) fur_cfg[i_rtf].xPyro += rtf_cfg[i_rtf].C_ELT[2][i_elt]; 
         }
      } // For all ELT.

	   // Lengh between zone entry and RTF exit calculation
	   // =================================================
      // For all zones
      for (int i_zon=0; i_zon < fur_cfg[i_rtf].nb_zon; i_zon++)
		{
			float lgt_zon_begin = 0.f;
         int nb_elt;

         int first_ecz = rtf_cfg[i_rtf].FIRST_ECZ[i_zon];
         int idx_elt_d = rtf_cfg[i_rtf].C_ECZ[1][first_ecz];

         // ????????????????????????? c un float !!!!!!!
			//if ( ! rtf_cfg[i_rtf].C_ELT[0][idx_elt_d] )
            nb_elt = rtf_cfg[i_rtf].C_ECZ[1][first_ecz];
			//else
   //         nb_elt = rtf_cfg[i_rtf].C_ECZ[0][first_ecz];

			// For all ELT
			for ( int i_elt=0; i_elt < nb_elt; i_elt++ )
			{
				if (!rtf_cfg[i_rtf].C_ELT[0][i_elt])
               lgt_zon_begin += rtf_cfg[i_rtf].C_ELT[2][i_elt]; 
			} // For all ELT

			s_cur.Lgt_zon_rtf_exit[i_rtf][i_zon] = fur_cfg[i_rtf].stp_lgt - lgt_zon_begin;
		} // For all zones

      // =====================
      // ROLLS
      // =====================
      // Rolls Characteristics
      // ---------------------
      // For all rolls
      for ( int i_rol=0; i_rol < rtf_cfg[i_rtf].nb_rol; i_rol++ )
      {
         // Roll location [ 0=Top; 1=Bottom ]
         rol_cmp[i_rtf].s_rol[i_rol].top = ( i_rol % 2 == 0 );

         // Roll diameter [m]
         rol_cmp[i_rtf].s_rol[i_rol].dia = rol_cfg[i_rtf].dia;

         // Roll thickness [m]
         rol_cmp[i_rtf].s_rol[i_rol].thi = rol_cfg[i_rtf].thick;

         // Roll cold geometry [m]
         rol_cmp[i_rtf].s_rol[i_rol].l1 = rol_cfg[i_rtf].L1;
         rol_cmp[i_rtf].s_rol[i_rol].l2 = rol_cfg[i_rtf].L2;
         rol_cmp[i_rtf].s_rol[i_rol].l3 = rol_cfg[i_rtf].L3;
         rol_cmp[i_rtf].s_rol[i_rol].c1 = rol_cfg[i_rtf].C1;
         rol_cmp[i_rtf].s_rol[i_rol].c2 = rol_cfg[i_rtf].C2;

         // Roll type
         rol_cmp[i_rtf].s_rol[i_rol].indic_roll = rol_cfg[i_rtf].rol_indic;                   

         // Roll length [m]
         rol_cmp[i_rtf].s_rol[i_rol].lgt = rol_cmp[i_rtf].s_rol[i_rol].l1 + 2.f * rol_cmp[i_rtf].s_rol[i_rol].l2 +
                                    2.f * rol_cmp[i_rtf].s_rol[i_rol].l3;

         // Other roll data
         rol_cmp[i_rtf].s_rol[i_rol].s_shield.wid = rol_cfg[i_rtf].shield_wid;
      } // For all rolls

      // Furnace height [m]
      rol_cmp[i_rtf].s_fur.hgt = rtf_cfg[i_rtf].height;

      // Shields
      // -------
      // For all rolls
      for ( int i_rol=0; i_rol < rtf_cfg[i_rtf].nb_rol; i_rol++ )
      {
         // If indic=0 no shield; if 1 shield under roll; if 2 shields under & between the rolls
         rol_cmp[i_rtf].s_rol[i_rol].indic        = rol_cfg[i_rtf].shield_indic;
         rol_cmp[i_rtf].s_rol[i_rol].s_shield.e   = rol_cfg[i_rtf].shield_emi;      // Shield emissivity []
         rol_cmp[i_rtf].s_rol[i_rol].e            = rol_cfg[i_rtf].emi_outside;     // Roll outside emissivity []
         rol_cmp[i_rtf].s_rol[i_rol].s_tub.e      = rtf_cfg[i_rtf].emi_tub;         // Tube emissivity []
         rol_cmp[i_rtf].s_rol[i_rol].s_shield.wid = rol_cfg[i_rtf].shield_wid;      // Shield width [m]
      } // For all rolls

      // Roll steel specific heat and thermal conduction
      // -----------------------------------------------
      for ( int i_tmp=0; i_tmp < NB_TMP_CHAR_ROL; i_tmp++ ) 
      {
         s_Trol[i_rtf].Rol_Cond[i_tmp] = ns_STD::cl_PHY::Tab_Rol_Cond[i_tmp];
         s_Trol[i_rtf].Rol_Cp  [i_tmp] = ns_STD::cl_PHY::Tab_Rol_Cp  [i_tmp];
         s_Trol[i_rtf].Rol_Tmp [i_tmp] = ns_STD::cl_PHY::Tab_Rol_Tmp [i_tmp];
      }

      // Roll steel dilatation
      // ---------------------
      for ( int i_tmp=0; i_tmp < NB_TMP_DILA; i_tmp++ )
      {
         s_Trol[i_rtf].Rol_Dila[i_tmp] = ns_STD::cl_PHY::Tab_Rol_Dila[i_tmp];
         s_Trol[i_rtf].Rol_Tdil[i_tmp] = ns_STD::cl_PHY::Tab_Rol_Tdil[i_tmp];
      }

      // First roll profil calculation during delay
      // ==========================================
      rol_cmp_risk[i_rtf] = rol_cmp[i_rtf];

      SQL_CFG_RTF_UPDATE (i_rtf);
      SQL_ROL_CFG_UPDATE (i_rtf);
   } // For all RTF furnaces

   // For all furnaces
   for (int i_fur=0; i_fur < MAX_NB_FUR; i_fur++)
   {
      SQL_CFG_FUR_UPDATE(i_fur);
   } // For all furnaces

   // JET
   // ===
   sps_entry[JET].s_mea.nb_jet = NB_ZON_JET;

   s_cur.MeaFurRead = false;
   s_cur.MeaTrkRead = false;

   return status;
} // void

/*============================================================================
/ SPS_INI_Alm: Initialize alarm text in user language.
/ 
/ Revisions:
/  Created:       2009-Feb      J. NAVEIRA
/  Modified: 
/===========================================================================*/
bool SPS_INI_Alm ()
{
   bool status = true;
	string sub_name = "SPS_INI_Alm";

   // If french language
   if ( LANGUAGE == 0 )
   {
      // Maximum speed calculation error 
      s_alm.s_rtf_spg.Alm[0] = "Erreur dans le calcul de la vitesse maximale de la bobine: %s (Ep.: %.2f mm, Lg.: %.1f mm, Emis.: %.2f, Vitesse: %.1f m/mn)";
      // Transcient calculation error 
      s_alm.s_rtf_spg.Alm[1] = "Erreur dans le calcul de la transition entre les bobines: %s et %s";
      // Minimum speed too high
      s_alm.s_rtf_Cmp_Max_Spd.Alm[0] = "Vitesse minimum trop grande pour le calcul de la vitesse maximum (Ep.: %.2f mm, Lg.: %.1f mm, Emis.: %.2f, Vitesse: %.1f m/mn)";
      // Not enought instal power in calcul of roof and tube temperature
      s_alm.s_OPC_class.Alm[0] = "La zone %d n est plus sous controle";
   }

   // If English language
   else if ( LANGUAGE == 1 )
   {
      // Maximum speed calculation error 
      s_alm.s_rtf_spg.Alm[0] = "Error in coil: %s maximum speed calculation (Ep.: %.2f mm, Lg.: %.1f mm, Emis.: %.2f, Vitesse: %.1f m/mn)";
      // Transcient calculation error 
      s_alm.s_rtf_spg.Alm[1] = "Calcul error for transient between: %s et %s";
      // Minimum speed too high
      s_alm.s_rtf_Cmp_Max_Spd.Alm[0] = "Minimum speed is too high for maximum speed calculation (Thi.: %.2f mm, Wid.: %.1f mm, Emis.: %.2f, Speed: %.1f m/mn)";
      // Not enought instal power in calcul of roof and tube temperature
      s_alm.s_OPC_class.Alm[0] = "Zone %d in level 1 control";
   }

   // If Russian language
   else if ( LANGUAGE == 2 )
   {
   }

   // If Spanish language
   else if ( LANGUAGE == 3 )
   {
   }

   // If Spanish language
   else if ( LANGUAGE == 4 )
   {
      // Maximum speed calculation error 
      s_alm.s_rtf_spg.Alm[0] = "Erro no cálculo da velocidade da bobina: %s (Ep.: %.2f mm, Lg.: %.1f mm, Emis.: %.2f, velocidade: %.1f m/mn)";
      // Transcient calculation error 
      s_alm.s_rtf_spg.Alm[1] = "Erro no cálculo da transição entre as bobinas: %s et %s";
      // Minimum speed too high
      s_alm.s_rtf_Cmp_Max_Spd.Alm[0] = "Mínima velocidade muito grande para o cálculo da velocidade máxima (Ep.: %.2f mm, Lg.: %.1f mm, Emis.: %.2f, velocidade: %.1f m/mn)";
      // Not enought instal power in calcul of roof and tube temperature
      s_alm.s_OPC_class.Alm[0] = "Zona %d está fora de controle";
   }

   // Language definition error
   else
      status = false;

   return status;
} // void