/*============================================================================
/ ROL.h: Roll structures
/ 
/ Revisions:
/  Created:       2004          P.Dubois
/  Modified: 
/  
/ (c) CMI Industry
/===========================================================================*/
struct furnace_car
{
   float  hgt;
   float  tmp[MAX_NB_ZON];      // temp 
};

struct roll_profil
{
   int n_rtf;

   float  coef;
   int    nb_roll;
   bool   indic_mechanical;
   bool   indic_thermal;
   struct ns_STD::cl_ROL::roll_car s_rol[MAX_NB_ROL];
   struct furnace_car s_fur;
};
