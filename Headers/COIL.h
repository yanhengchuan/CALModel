/*============================================================================
/ COIL.H: Structures definition for waiting coils.
/ 
/ Revisions:
/  Created:       2007-Sep      JC.Mitais
/  Modified:      2009-Mar      J. NAVEIRA
/  
/ (c) CMI Industry
/===========================================================================*/
struct coil_chemical
{
   float C;                            // Carbon %
   float Al;                           // aluminum %
   float B;                            // brome %
   float Ce;                           // 
   float Cr;                           // chrome %
   float Cu;                           // copper %
   float Mn;                           // Manganese %
   float Mo;                           // 
   float N;                            // azote %
   float Nb;                           //
   float Ni;                           // Nickel %
   float P;                            //
   float S;                            //
   float Si;                           // silicium %
   float Sn;                           //
   float Ti;                           // titanium
   float V;                            //
   float W;                            //
};

struct coil_data
{
   bool    valid;                      // True if coil is valid
   bool    Trs_cal[NB_FUR_RTF];			// True if next transcient has been calculated
   string  id;                         // coil id 
   float   thi;                        // strip thickness 
   float   wid;                        // strip width 
   int     dummy;                      // dummy coil 
   float   lgt;                        // length  
	float   emi_rtf[NB_FUR_RTF];        // strip emissivity for RTF

   float   tmp_tgt[MAX_NB_FUR];  // Temperature target 
   float   tmp_upp[MAX_NB_FUR];  // Upper temperature  
   float   tmp_low[MAX_NB_FUR];  // Lower temperature  

   float   spd;                        // speed 
   float   spd_dec;                    // decided coil speed [m/s] from Level 3; if 0, use max speed
	float	  spd_max_rec;						// received maximum speed by level 1 for tracking purpose
	float		spd_min_rec;					// received minimum speed by level 1 for tracking purpose
   float   spd_max;                    // maximum coil speed [m/s]
   float   spd_max_th[NB_FUR_RTF];                 // maximum thermal coil speed [m/s]
	float   spd_max_coil_ope;				// Minimum speed between coil speed and operator speed limitation [m/s]
   float   spd_min;                    // Minimum coil speed [m/s]
   float   tmp_in[NB_FUR_RTF];         // Entry temperature
   int     steel_charac;               // 1=CQ, 2=DQ, 3=DDQ 
   int     change;                     // if !=0, a change occured between 2 calls. 

   float   tmp_ref_tub;

   float   tmp_zon[MAX_NB_FUR][MAX_NB_ZON];  // tube temperatures for steady state 
   float   pow_pct[MAX_NB_FUR][MAX_NB_ZON];  // power for steady state 
   float   stp_tmp[NB_FUR_RTF][MAX_NB_ELT];  // RTF strip temperature for steady state [K]

   float   Cool_rate[NB_ZON_JET];      // Cooling rate set point for JET cooling section zone (°C/s)

   int     steeltype;                  // 0=Défaut,1=Choix Tôle Acier,2=Tôle fer blanc,3=DP,4=TRIP
   string  Steel_code;				      // Coil steel code
   int     cycle;                      // thermal cycle
   int     N_pdt;                      // Product number for emissivity
   int     N_cla;                      // Classe number for emissivity
   float   strip_tension;              // Strip Tension
   int     Bisra;                      // Bisra code
	struct  coil_spe	s_spe;				// Specific DATA for the coil
   struct  coil_chemical   s_chem;     // Steel chemical structure
};

struct coils
{
   int      nb_coil;                                // number of coils
   struct   coil_data    s_coil_data[MAX_NB_COIL];      // basic coil queue
   struct   coil_data    s_coil_data_old[MAX_NB_COIL];  // old coil data 
   struct   coil_data    s_coil_data_last[MAX_NB_COIL];   // Last coil data structure // Repère les bobines passées
};
