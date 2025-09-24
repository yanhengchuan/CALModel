/*============================================================================
/ ADA.h: Self adaptation structures.
/ 
/ Revisions:
/  Created:       2007-Jul       J. NAVEIRA
/  Modified: 
/  
/ (c) CMI Industry
/===========================================================================*/

// Product Classe structure for emissivity self adaptation
struct Str_Pdt_Cla
{
   float alpha;               // adaptation coefficient for emissivity
	string  Steel_code;        // Steel code
};

// Strip/rool exchange coeffcient adaptation structure
struct Str_exg
{
   time_t lst_time;           // last data check time
   float Alpha;               // Correction coeffcient
   float Gap_spt;             // Gap calculated on RTF temperature
   float Gap_mea;             // Gap measurment on RTF temperature
   bool adp_calc;             // Adaptation on exchange has been calculated

   struct Str_calcul
   {
      float Tmp_last;         // Last temperature [°C], -1 if not initialize
      int   last_sign;        // Sign of last temperature variation, 0 if not initialize
   } s_calc;
};

// Anticipation adaptation structure
struct Str_ant
{
   time_t last_time;          // Last data check time
   float Alpha;               // Radiant tubes correction coefficient
};

// Salf adaptation structure
struct Str_Adp
{
   bool use_ada;                                         // If true, adaptation is to be done for this furnace
   time_t timer_emi;                                     // emissivity adaptation timer
   struct Str_Pdt_Cla s_pdt_RTF[MAX_NB_CLA][MAX_NB_PDT]; // Product structure for RTF emissivity self adaptation
   float Effi[MAX_NB_ZON];                               // Zone efficacity
   struct Str_exg s_exg;                                 // Strip/roll exchange coefficient structure
   float coeff_chan;                                     // transfert coefficient
   struct Str_ant s_ant;                                 // Anticipation structure
   struct Str_adp_spe   s_spe;                           // Specific adaptation structure
};