/*============================================================================
/ SPG.h: SPG Structures definition.
/ 
/ Revisions:
/  Created:       2007-Apr       L.Lâm Ngoc
/  Modified:      2016-Mar       JC.Mitais
/  
/ (c) CMI Industry
/===========================================================================*/
extern CRITICAL_SECTION Sec_SPG_Strip;

// *********
// RTF
// **********
// Transient
// =========
struct  strip_trans_entry
{
   float  thi;                         // strip thickness               [m]
   float  wid;                         // strip width                   [m]
   float  tmp_in;                      // Strip entry temperature       [K]

   float  tmp_tgt;                     // strip temperature target RTF  [K]
   float  tmp_upp;                     // RTF strip upper temperature   [K]
   float  tmp_low;                     // RTF strip lower temperature   [K]
   float  tmp_tub_ref;                 // tube temperature reference    [K]
   float  tmp_tub[MAX_NB_ZON];         // tube temperature per zone     [K]
   float  pow_pct_rtf[MAX_NB_ZON];     // power per zone                [%]
   float  stp_rtf[MAX_NB_ELT];         // RTF strip temperature for transient state [K]

   float  spd;                         // speed                         [m/s]
   float  spd_min;                     // Minimum speed                 [m/s]
   int    dummy;                       // dummy coil
};

struct  strip_trans_exit
{
   float  tmp_stp_rtf;                 // First  coil strip exit temperature  [K]
   float  tmp_stp_rtf2;                // Second coil strip exit temperature  [K]
   float  tmp_tub_ref;                 // tube temperature reference          [K]
   float  tmp_tub[MAX_NB_ZON];         // Tube temperature set point          [K]
   float  pow_pct_rtf[MAX_NB_ZON];     // power per zone RTF                  [%]
   float  stp_rtf[MAX_NB_ELT];         // RTF strip temperature in element beginning for transient state [K]

   float  pow_pct_jet[NB_ZON_JET];     // power per zone JET                  [%]
   float  spd;                         // speed                               [m/s]
   float  tmp_stp_in;                  // Strip entry temperature             [K]
   float  anticipation_time;           // anticipation time                   [s] 
   float  thermal_inertia;             // thermal inertia                     [J/K]
   float  DH_roll_zon;                 // Roll enthalpy (needed or to give) for anticipation time calculation
   int    dummy;                       // dummy coil, 0 if it is a dummy coil
};

// ====
// JET 
// ====
// JET cooling model data entry sructure
struct  spg_entry_data_jet
{  
   float   H2_rate[NB_ZON_JET];     // Taux d'hydrogene dans JETS (%) between [0 -> 1] // ???????? non initialisé ?????
   float   blw_lgt[NB_ZON_JET];     // Longueur de refroidissement [m]   
   float   blw_tmp[NB_ZON_JET];     // Température du souffle [K]
   float   box_tmp[NB_ZON_JET];     // Température dans la caisse [K]
   float   instal_pow[NB_ZON_JET];  // Available fan power [W]
   float   MAX_FAN_SPD[NB_ZON_JET]; // Maximum speed of the fans in each JET zone [m/s]
   bool    jet_on[NB_ZON_JET];      // true if the jet zone is turned on // Always true, allows to disconnect a given zone
	struct  jet_spe	s_spe;         // Specific jet DATA for the coil
};

// JET cooling model data exit structure
struct  spg_exit_data_jet
{
   float   jet_zon_pow[NB_ZON_JET];    // Setpoint fan jet cooler [%]
   float   blw_spd[NB_ZON_JET];        // Blow speed [m/s]
	float   blw_flw[NB_ZON_JET];        // Débit du souffle [m^3/s]
   float   cnv_pow[NB_ZON_JET];        // Chaleur extraite par convection [W]
   float   H_cnv[NB_ZON_JET];          // Coefficient de convection 
   float   Mix_density[NB_ZON_JET];    // Densité du mélange [kg/m^3]
   float   Tmp_N2H2_in[NB_ZON_JET];    // Température du gaz entrant [K]
   float   Cooling_rate[NB_ZON_JET];   // Cooling rates [°C/s]
   bool    jet_on[NB_ZON_JET];         // Jet section opening [true or false]
	struct  jet_spe	s_spe;            // Specific jet DATA for the coil
};
