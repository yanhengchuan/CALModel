/*============================================================================
/ RTF.h: RTF Structures definition.
/ 
/ Revisions:
/  Created:       2005-Aug      P.DUBOIS
/  Modified:      2009-Mar      J. NAVEIRA
/  Modified:      2016-Mar      JC.Mitais
/  
/ (c) CMI Industry
/===========================================================================*/
// ==============
// SPG RTF Structures
// ==============

struct  rtf_struct
{
   int      n_rtf;      // Index in N_FUR_RTF collection
   float    spd;        // Current speed [m/s]
   int      nb_elt;     // Current number element of the heating section
   int      nb_ecz;     // Current number of ecz
   int      first_strip_elt;  //index of the first strip element
   int      nb_zon;
   int      elt_pyro;
   struct   ns_STD::cl_RTF::struct_ecz  s_ecz[MAX_NB_ECZ];
   struct   str_spe_rtf s_spe_rtf;  // Specific RTF structure
   float    Cooling_rate[MAX_NB_ZON]; // JET section cooling rate [°C/s]
   struct   ns_STD::cl_RTF::struct_elt  s_elt[MAX_NB_ELT];
};

// ==============
// rtf CFG Structures
// ==============
struct rtf_cfg_struct
{
   int n_rtf;           // Index in NN_FUR_RTF collection
   int nb_elt;
   int nb_ecz;
   int first_strip_elt;
   int elt_pyro;
   bool trans_calc;     // if true transient calculation is run

   // Rolls
   // -----
   int nb_rol;
   float one_rol_mass;  // 1 roll mass
   float tot_rol_mass;  // Total rolls mass
   float r_coeff;       // Conduction coefficient between the roll and the strip [W/m²/K] 

   // Radiant tubes
   // -------------
   float tot_tub_mass;  // Total radiant tubes mass
   float emi_tub;       // Radiant tubes emissivity
   float emi_wal;       // Walls emissivity
   float emi_stp;       // Initial strip emissivity
   float ff22;          // Form factor

   // Steps
   // -----
   float tub_max_ref;
   float tub_min_ref;
   float tub_min_min;
   float max_width;     // Maximum width
   float min_width;     // Minimum width
   float max_thick;     // Maximum thickness
   float min_thick;     // Minimum thickness

   // Dimensions
   // ----------
   float width;         // Furnace width
   float height;        // Funace height

   int   FIRST_ECZ[MAX_NB_ZON];              // Element index of the first element from each control zone
   int   LAST_ECZ[MAX_NB_ZON];               // Element index of the last element from each control zone
   float MAX_POWER[MAX_NB_ZON];              // Installed power in each control zone [W]

   int   C_ECZ [3][MAX_NB_ECZ];              // Configuration for each elementary control zone: 0: element up, 1: element down, 2: zone
   float TUBE_AREA [MAX_NB_ECZ];             // Area af all the tubes in one elementary control zone [m²]
   float C_ELT [3][MAX_NB_ELT];              // configuration for each element: 0: 1 if a  wall, 0 else 1: 1 if a roll, 0 else 2: length [m]

   // Radiant tube Step
   // =================
   float TUB_MAX_NARROW[MAX_NB_ZON];         // Max tube temperature narrow strip for step calculations [K]
   float TUB_MIN_NARROW[MAX_NB_ZON];         // Min tube temperature narrow strip for step calculations [K]
   float TUB_MAX_WID[MAX_NB_ZON];            // Max tube temperature wide strip for step calculations [K]  
   float TUB_MIN_WID[MAX_NB_ZON];            // Min tube temperature wide strip for step calculations [K]
   float TUB_MAX_THICK[MAX_NB_ZON];          // Max tube temperature thick strip for step calculations [K]
   float TUB_MIN_THICK[MAX_NB_ZON];          // Min tube temperature thick strip for step calculations [K]
   float TUB_MAX_THIN[MAX_NB_ZON];           // Max tube temperature thin strip for step calculations [K] 
   float TUB_MIN_THIN[MAX_NB_ZON];           // Min tube temperature thin strip for step calculations [K]
};