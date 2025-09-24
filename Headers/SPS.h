/*============================================================================
/ SPS.h: SPS Structures definition.
/ 
/ Revisions:
/  Created:       2007-Apr       L.Lâm Ngoc
/  Modified:      2016-Mar       JC.Mitais
/  
/ (c) CMI Industry
/===========================================================================*/
// ==============
// Fuel Structure
// ==============
struct str_fuel      // For each fuel
{
   float    pCO2;       // Partial pressure CO2
   float    pH2O;       // Partial pressure H2O
   float    lhv;        // LHV [J/Nm3] or [J/kg]
   float    va;         // Air factor
   float    vf;         // Waste gas factor
   bool     liquid;     // If true, liquid fuel; else gazeous fuel
   float    rho;        // fuel density [kg/m3]
   string   fName;      // Name of fuel
};

// ----------------------------------------------------------------------------------------
struct  sps_coil_data
{
   bool    valid;       // Set points validity
   float   thi;         // strip thickness [m]
   float   wid;         // strip width [m]
   float   tmp_tgt;     // temperature target [K]
	float	  nxt_tgt;		// next coil temperature target [K]
   float   tmp_upp;     // upper temperature [K]
   float   tmp_low;     // lower temperature [K]
   float   spd;         // speed [m/s]
   int     dummy;       // dummy coil, transition coil 
   int     steeltype;   // 0=Défaut, 1=Choix Tôle Acier, 2=Tôle fer blanc, 3=DP, 4=TRIP 

   float   tmp_zon[MAX_NB_ZON];  // tube temperature [K]
   float   pow_pct[MAX_NB_ZON];  // Power [%]
};

// ----------------------------------------------------------------------------------------
struct  sps_trans_coil_data
{
   float   anticipation_time;
	float		last_time;
   float   tmp_tgt;              // temperature target 
   float   spd;                  // speed

   float   tmp_zon[MAX_NB_ZON];  // zone (tube) temperature [K]
   float   pow_pct[MAX_NB_ZON];  // Power [%]
};

// ----------------------------------------------------------------------------------------
struct  sps_mea_coil_data
{
   bool spd_auto;    // speed mode [1: speed in automatic(ltop); 0: speed in manual]
   bool auto_on;     // section control mode [1: furnace in automatic; 0: furnace in manual]
   float spd;        // speed [m/s]
   float tonnage;    // Furnace tonnage production per hour
   float tim;        // time before the weld exit the section [s] 
   float lgt;        // length before the weld exit the section [m]
   float tmp_stp;    // strip temperature measurement (pyro) [K]
   int nCurCoil;     // Index of the coil currently under the pyrometer

   bool    on_zone[MAX_NB_ZON];     // zone on [1: zone is open; 0: zone is closed]
   float   fuel_flow[MAX_NB_ZON];   // Measured fuel flow [Nm3/s] 
   float   tmp_zon[MAX_NB_ZON];     // zone (tube) temperature [K]
	float   tmp_zon_ada[MAX_NB_ZON];     // zone (tube) temperature [K]
   float   heat_pow_pct[MAX_NB_ZON];   // Power [%]
	float	  cool_pow_pct[MAX_NB_ZON];   // heat demand [%]

   int     nb_jet;            // number active jet cooling zones

   float   restart_risk_lim;  // Restarting risk limit value, read from SQL Database
	float   spd_dec;           // manual speed decided [m/s]
};

// ----------------------------------------------------------------------------------------
struct  sps_entry_data
{
   struct sps_coil_data       s_cur_coil;   // characteristic of the current coil 
   struct sps_trans_coil_data s_trans_coil; // transition set point 
   struct sps_mea_coil_data   s_mea;        // measurement 
};

// ----------------------------------------------------------------------------------------
struct  sps_exit_data
{
   bool    Valid;             // Set point validity [1: set points are valid, 0: set points invalid]
   bool    Spd_Valid;         // Speed Set point validity [1: set points are valid, 0: set points invalid]
   float   tmp_stp;           // strip temperature set point [K]
   float   spd;               // speed, vitesse [m/s]
   bool    ctl_pow;           // if true control by power; else by tube temperature 
   bool    restart_OK;        // If the line is able to restart without buckles after a shutdown [1: ok, to restart; 0: not OK]
   float   Risk;              // Risk value in case of restart

   int     Trans_Type;        // Transition type
   float   traction;          // Traction offset [%]

   float   tmp_zon[MAX_NB_ZON];        // zone/tube temperature [K]
   float   heat_pow_pct[MAX_NB_ZON];   // power per zone [%]
	float   cool_pow_pct[MAX_NB_ZON];   // Heating demand [%]
   float   heat_pow_old[MAX_NB_ZON];   // power per zone at previous calculation [%]
   float   cool_pow_old[MAX_NB_ZON];   // power per zone at previous calculation [%]
};

// *********
// SEQUENCES
// *********
class SPS_Sequence
{
   public:
      static bool SPS_Compute();
   private:
};