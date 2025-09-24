/*============================================================================
/ SPG_ROL.cpp: All Rolls calculation & utilities
/  
/ (c) CMI Industry
/===========================================================================*/
#include "stdafx.h"

/*============================================================================
/ SPG_ROL: Rolls calculation
/ 
/ Revisions:
/  Created:       2004          P.Dubois
/  Modified: 
/===========================================================================*/
bool SPG_ROL (struct roll_profil *rol_cmp)
{
   bool status = true;
   string sub_name = "SPG_ROL";

   float dx, radius;
   int n_rtf = rol_cmp->n_rtf;
   int n_rol = rol_cfg[n_rtf].n_rol_cmp;

   // Initialisation.
   rol_cmp->coef = rtf_cfg[n_rtf].r_coeff;

   // Mesh on half roll length
   dx = rol_cmp->s_rol[n_rol].lgt / 2.f / float(MAX_NB_MESH-2);

   // Strip tension [Pa]
   if ( rol_cmp->s_rol[n_rol].top )
   {
      // Line top roll
      rol_cmp->s_rol[n_rol].s_strip.tension = rol_cmp->s_rol[n_rol].s_strip.tension_basis;
   }
   else
   {
      // Bottom top roll
      rol_cmp->s_rol[n_rol].s_strip.tension = rol_cmp->s_rol[n_rol].s_strip.tension_basis + 
                                                RO_STEEL * rol_cmp->s_fur.hgt * 9.81f;
   }

   // Depending on the strip steel grade
   switch (rol_cmp->s_rol[n_rol].s_strip.steel_charac)
   {
      // Young modulus & elastic limit calculation
      case 1:
      {
         // CQ Steel
         rol_cmp->s_rol[n_rol].s_strip.young  = ns_STD::cl_UTL::Interpolation(ns_STD::cl_PHY::Tab_yo, ns_STD::cl_PHY::Tab_tyo, 
                                                                              rol_cmp->s_rol[n_rol].s_strip.tmp, NB_PTS_YOUNG);
         rol_cmp->s_rol[n_rol].s_strip.lim_el = ns_STD::cl_UTL::Interpolation(ns_STD::cl_PHY::Tab_ecq, ns_STD::cl_PHY::Tab_tecq,
                                                                              rol_cmp->s_rol[n_rol].s_strip.tmp, NB_PTS_YOUNG);
         break;      
      }

      case 2:
      {
         // DQ Steel
         rol_cmp->s_rol[n_rol].s_strip.young  = ns_STD::cl_UTL::Interpolation(ns_STD::cl_PHY::Tab_yo, ns_STD::cl_PHY::Tab_tyo,
                                                                              rol_cmp->s_rol[n_rol].s_strip.tmp, NB_PTS_YOUNG);
         rol_cmp->s_rol[n_rol].s_strip.lim_el = ns_STD::cl_UTL::Interpolation(ns_STD::cl_PHY::Tab_edq, ns_STD::cl_PHY::Tab_tedq,
                                                                              rol_cmp->s_rol[n_rol].s_strip.tmp, NB_PTS_YOUNG);
         break;      
      }

      case 3:
      {
         // DDQ Steel
         rol_cmp->s_rol[n_rol].s_strip.young  = ns_STD::cl_UTL::Interpolation(ns_STD::cl_PHY::Tab_yo, ns_STD::cl_PHY::Tab_tyo,
                                                                              rol_cmp->s_rol[n_rol].s_strip.tmp, NB_PTS_YOUNG);
         rol_cmp->s_rol[n_rol].s_strip.lim_el = ns_STD::cl_UTL::Interpolation(ns_STD::cl_PHY::Tab_eeddq, ns_STD::cl_PHY::Tab_teeddq,
                                                                              rol_cmp->s_rol[n_rol].s_strip.tmp, NB_PTS_YOUNG);
         break;
      }
   }

   // Tube area 
   rol_cmp->s_rol[n_rol].s_tub.area = 5.f * rol_cmp->s_rol[n_rol].dia * 
                                       ( rol_cmp->s_rol[n_rol].lgt - rol_cmp->s_rol[n_rol].s_strip.wid ) / 2.f;

   // Shield area: If indic=0 no shield; if 1 shield under roll; if 2, 2 shields under and between the rolls
   if ( rol_cmp->s_rol[n_rol].indic == 1 )
   {
      rol_cmp->s_rol[n_rol].s_shield.area = rol_cmp->s_rol[n_rol].s_shield.wid * 
                                             ( rol_cmp->s_rol[n_rol].lgt - rol_cmp->s_rol[n_rol].s_strip.wid ) / 2.f;
   }
   else
   {
      rol_cmp->s_rol[n_rol].s_shield.area = 10.f * rol_cmp->s_rol[n_rol].s_shield.wid * 
                                             ( rol_cmp->s_rol[n_rol].lgt - rol_cmp->s_rol[n_rol].s_strip.wid ) / 2.f;
   }

   rol_cmp->s_rol[n_rol].s_shield.tmp = ( rol_cmp->s_rol[n_rol].s_tub.tmp + rol_cmp->s_rol[n_rol].s_strip.tmp ) / 2.f;

   // Roll mesh
   rol_cmp->s_rol[n_rol].s_mesh[0].dx       = dx;    
   rol_cmp->s_rol[n_rol].s_mesh[0].x        = dx / 2.f;
   rol_cmp->s_rol[n_rol].s_mesh[0].heat_exp = 0.f;
   rol_cmp->s_rol[n_rol].s_mesh[0].tmp      = 500.f;

   rol_cmp->s_rol[n_rol].s_mesh[0].cold_prf = 0.f;
   rol_cmp->s_rol[n_rol].s_mesh[0].hot_prf  = 0.f; // [m]

   // For all mesh points
   for ( int i_mesh=1; i_mesh<MAX_NB_MESH; i_mesh++ )
   {
      rol_cmp->s_rol[n_rol].s_mesh[i_mesh].dx = dx;
      rol_cmp->s_rol[n_rol].s_mesh[i_mesh].x = rol_cmp->s_rol[n_rol].s_mesh[i_mesh-1].x + dx;
      rol_cmp->s_rol[n_rol].s_mesh[i_mesh].heat_exp = 0.f;
      rol_cmp->s_rol[n_rol].s_mesh[i_mesh].tmp = 500.f;

      // Depending on the roll type
      switch ( rol_cmp->s_rol[n_rol].indic_roll )
      {
         // ---------------------
         case 1:
            if ( rol_cmp->s_rol[n_rol].s_mesh[i_mesh].x < rol_cmp->s_rol[n_rol].l1 / 2.f )
            {
               rol_cmp->s_rol[n_rol].s_mesh[i_mesh].cold_prf = 0.f;
            }

            else if ( rol_cmp->s_rol[n_rol].s_mesh[i_mesh].x < rol_cmp->s_rol[n_rol].l1/2.f + 
                                                               rol_cmp->s_rol[n_rol].l2 )
            {
               rol_cmp->s_rol[n_rol].s_mesh[i_mesh].cold_prf = - ( rol_cmp->s_rol[n_rol].s_mesh[i_mesh].x - 
                                                                   rol_cmp->s_rol[n_rol].l1/2 ) / 
                                                                   rol_cmp->s_rol[n_rol].l2 * 
                                                                   rol_cmp->s_rol[n_rol].c1;
            }

            else if ( rol_cmp->s_rol[n_rol].s_mesh[i_mesh].x < rol_cmp->s_rol[n_rol].l1/2.f + 
                                                               rol_cmp->s_rol[n_rol].l2 + 
                                                               rol_cmp->s_rol[n_rol].l3 )
            {
               rol_cmp->s_rol[n_rol].s_mesh[i_mesh].cold_prf = - rol_cmp->s_rol[n_rol].c1 - 
                                                               ( rol_cmp->s_rol[n_rol].s_mesh[i_mesh].x - 
                                                                 rol_cmp->s_rol[n_rol].l1/2.f - 
                                                                 rol_cmp->s_rol[n_rol].l2 ) /
                                                                 rol_cmp->s_rol[n_rol].l3 * 
                                                                 rol_cmp->s_rol[n_rol].c2;
            }

            else
            {
               rol_cmp->s_rol[n_rol].s_mesh[i_mesh].cold_prf = - rol_cmp->s_rol[n_rol].c1 - 
                                                                 rol_cmp->s_rol[n_rol].c2;
            }

            rol_cmp->s_rol[n_rol].s_mesh[i_mesh].hot_prf = rol_cmp->s_rol[n_rol].s_mesh[i_mesh].cold_prf;
         break;

         // ---------------------
         case 2:
            if ( rol_cmp->s_rol[n_rol].s_mesh[i_mesh].x < rol_cmp->s_rol[n_rol].l1/2.f )
            {
               rol_cmp->s_rol[n_rol].s_mesh[i_mesh].cold_prf = 0.f;
            }

            else if ( rol_cmp->s_rol[n_rol].s_mesh[i_mesh].x < rol_cmp->s_rol[n_rol].l1/2.f + 
                                                               rol_cmp->s_rol[n_rol].l2 )
            {
               rol_cmp->s_rol[n_rol].s_mesh[i_mesh].cold_prf = - ( rol_cmp->s_rol[n_rol].s_mesh[i_mesh].x - 
                                                                   rol_cmp->s_rol[n_rol].l1/2.f ) / 
                                                                   rol_cmp->s_rol[n_rol].l2 * 
                                                                   rol_cmp->s_rol[n_rol].c1;
            }

            else 
            {
               rol_cmp->s_rol[n_rol].s_mesh[i_mesh].cold_prf = - rol_cmp->s_rol[n_rol].c1 - 
                                                               ( rol_cmp->s_rol[n_rol].s_mesh[i_mesh].x - 
                                                                 rol_cmp->s_rol[n_rol].l1/2.f - 
                                                                 rol_cmp->s_rol[n_rol].l2 ) /
                                                               ( rol_cmp->s_rol[n_rol].lgt - 
                                                                 rol_cmp->s_rol[n_rol].l2 - 
                                                                 rol_cmp->s_rol[n_rol].l1/2.f ) *
                                                                 rol_cmp->s_rol[n_rol].c2;
            }

            rol_cmp->s_rol[n_rol].s_mesh[i_mesh].hot_prf = rol_cmp->s_rol[n_rol].s_mesh[i_mesh].cold_prf;
         break;

         // ---------------------
         case 3:
            if ( rol_cmp->s_rol[n_rol].s_mesh[i_mesh].x < rol_cmp->s_rol[n_rol].l1/2.f )
            {
               rol_cmp->s_rol[n_rol].s_mesh[i_mesh].cold_prf = 0.f;
            }

            else
            {
               rol_cmp->s_rol[n_rol].s_mesh[i_mesh].cold_prf = - ( rol_cmp->s_rol[n_rol].s_mesh[i_mesh].x - 
                                                                   rol_cmp->s_rol[n_rol].l1/2.f ) /
                                                                 ( rol_cmp->s_rol[n_rol].lgt/2.f - 
                                                                   rol_cmp->s_rol[n_rol].l1/2.f ) *
                                                                   rol_cmp->s_rol[n_rol].c1;
            }

            rol_cmp->s_rol[n_rol].s_mesh[i_mesh].hot_prf = rol_cmp->s_rol[n_rol].s_mesh[i_mesh].cold_prf;
         break;

         // ---------------------
         case 4:
   //          radius = ( pow( rol_cmp->s_rol[n_rol].lgt/2.f, 2.f ) + pow( rol_cmp->s_rol[n_rol].c1, 2.f ) ) / 2.f / 
   //                   rol_cmp->s_rol[n_rol].c1;
				//rol_cmp->s_rol[n_rol].s_mesh[i_mesh].cold_prf = sqrtf( powf(radius, 2) - 
				//                                       powf(rol_cmp->s_rol[n_rol].s_mesh[i_mesh].x, 2) ) - radius;
            radius = ( rol_cmp->s_rol[n_rol].lgt / 2.f * rol_cmp->s_rol[n_rol].lgt / 2.f + 
                        rol_cmp->s_rol[n_rol].c1 * rol_cmp->s_rol[n_rol].c1 ) / 2.f /  rol_cmp->s_rol[n_rol].c1;
				rol_cmp->s_rol[n_rol].s_mesh[i_mesh].cold_prf = 
                     sqrtf( radius*radius - 
					               rol_cmp->s_rol[n_rol].s_mesh[i_mesh].x * rol_cmp->s_rol[n_rol].s_mesh[i_mesh].x ) - radius;
         break;
      } // Depending on the roll type
   } // For all mesh points

   // Dynamic initialisation
   ns_STD::cl_ROL::ROL_dynamic_init (&rol_cmp->s_rol[n_rol]);

   // Calculation of the heat radiation inside the roll.
   status = ns_STD::cl_ROL::ROL_profil_calcul (&rol_cmp->s_rol[n_rol], s_Trol[n_rtf], rol_cmp->coef, s_cur.old_ex_t_rol[n_rtf]);
   if ( !status ) ns_STD::cl_TRC::ERR_Write("SPG", sub_name, "%s: Error in SPG_ROL_profil_calcul", fur_cfg[RTH + n_rtf].fur_name.c_str());

   return status;
}