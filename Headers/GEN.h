/*============================================================================
/ GEN.h: All general data definition.
/ 
/ Revisions:
/  Created:       2016-Mar       JC.Mitais
/  Modified:      
/  
/ (c) CMI Industry
/===========================================================================*/
// ==============
// Section CFG Structures
// ==============
struct fur_cfg_struct
{
   string fur_name;

   int nb_zon;
   int pre_fur;            // previous furnace on the line; -1 if none
   float entry_tmp;

   float stp_lgt;          //
   float xPyro;            //
   float foll_lgt;         // Strip length between current furnace and following one
};

// ==============
// jet CFG Structures
// ==============
struct jet_cfg_struct
{
   string fur_name;

   int n_fur;           // Index in furnaces collection
   int nb_holes;        // holes number
   float noz_dia;       // nozzle diameter
   float pitch;         // pitch between each tube
   float d_stp_noz;     // strip nozzle distance
}; 

// ==============
// rolls CFG Structures
// ==============
struct rol_cfg_struct
{
   int n_fur;           // Index in furnaces collection
   int n_rol_cmp;       // Index of roll for calculation
   int  rol_indic;          // Indicator for the type of roll 
   float dia;           // pitch between each tube
   float L1;            // 
   float L2;            // 
   float L3;            // 
   float C1;            // 
   float C2;            // 
   float emi_outside;   // Roll outside emissivity
   float thick;         // Roll thickness

   int shield_indic;    // If indic=0, no shield; if 1, 1 shield under roll; if 2, 2 shields under & between the rolls 
   float shield_emi;    // Shield emissivity
   float shield_wid;    // Shield width
}; 

struct str_current
{
   // ROLL
   // ====
   float old_ex_t_rol[NB_FUR_RTF][MAX_NB_MESH];  // Roll profile mesh structure

   // RTF
   float old_ex_t_rtf[NB_FUR_RTF][MAX_NB_ELT];  // Temporary old element temperature [K]

   int jcount_rtf;      // RTF line stoppage counter

   float delta_po_rtf[NB_FUR_RTF][MAX_NB_ZON];     // RTF power offsets
   float	prev_strip_temp_rtf[NB_FUR_RTF];          // RTF previous strip temperature measurement [K]

   // SPS parameters
   // ==============
   float Lgt_zon_rtf_exit[NB_FUR_RTF][MAX_NB_ZON];    // Length of strip in each control zone [m] 

   // Risk calculation
   // ================
   time_t time_start[NB_FUR_RTF];   // time since last risk calcul
   int record_IDX;         // Current Record Index
   int record_Pt;          // Current Record point

   time_t time_lst_adapt[NB_FUR_RTF];  // Time since last emissivity adaptation
   bool Line_running;      // True if line is running

   bool MeaFurRead;
   bool MeaTrkRead;
};

