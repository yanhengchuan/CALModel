/*============================================================================
/ FURNACE_PLC.h: structure for communication between furnace PLC and L-TOP.
/ 
/ Revisions:
/  Created:       2011-Jan       J. NAVEIRA
/  Modified:      2012-Oct       J.Naveira
/  
/ (c) CMI Industry
/===========================================================================*/

// ==========================================
// Communication Furnace PLC -> L-TOP
// ==========================================
struct str_fur_mea
{
   float pre_hea_stp_tmp;     // Pre heating section strip temperature measurement [°C]

   bool ctl;                  // control mode [0=manual, 1=L-TOP]
   bool zon_ctl[MAX_NB_ZON];  // specific control mode for each zone [0=manual, 1=L-TOP]
   bool fan_mode;					// fans mode [0=Unisson, 1=Cascade]
	float stp_tmp;             // strip temperature measurement [°C]
	float heat_pow_pct[MAX_NB_ZON]; // zone power measurement [%]  
	float cool_pow_pct[MAX_NB_ZON]; // zone power measurement [%]  
	float zon_tmp[MAX_NB_ZON]; // zone temperature measurement [°C], 

   struct str_fur_mea_spe s_spe;
};

// ==========================================
// Communication L-TOP -> Furnace PLC
// ==========================================
struct str_fur_spt
{
	bool ctl_mod;                // control mode [1=power, 0=temperature]
	bool valid;                  // data validity [1=valid, 0=non-valid]
	float zon_tmp[MAX_NB_ZON];   // tube temperature set point [°C]
	float heat_zon_pow[MAX_NB_ZON];   // zone power set point [%]
	float cool_zon_pow[MAX_NB_ZON];   // zone power set point [%]
	float stp_tmp;               // strip temperature set point [°C]
};

