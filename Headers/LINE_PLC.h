/*============================================================================
/ LINE_PLC.h: specific structure for converting data from communication between 
/  line PLC and L-TOP.
/ 
/ Revisions:
/  Created:       2011-Jan       J. NAVEIRA
/  Modified:      2012-Oct       J.Naveira
/  
/ (c) CMI Industry
/===========================================================================*/

// ==========================================
// Communication line PLC -> L-TOP: One coil
// ==========================================
struct  str_line_coil
{
	string id;  // Coil identity
	float wid;  // Strip width [inches]
	float thi;  // Strip thickness [inches]
	float lgt;  // Strip length [feet]

	float spd_max;    // Coil maximum line speed 
   float spd_min;    // Coil minimum line speed 
   int   dummy;

   float tmp_tgt[MAX_NB_FUR];   // strip target temperature
   float tmp_upp[MAX_NB_FUR];   // strip upper tolerance 
   float tmp_low[MAX_NB_FUR];   // strip lower tolerance 

   string  cycle;
   int Prev_roll_mill;
   int steeltype;
};

// ==========================================
// Communication line PLC -> L-TOP: tracking
// ==========================================
struct  str_line_trk
{
	string coil_id;         // Coils identity
	float weld_pos;         // Length between the coil tail and the taper cooling section exit [m]
	float Spd_max_ope;      // Operator maximum speed [m/mn]
	bool Spd_ctrl;          // Speed control mode [0=manual, 1=L-TOP]
	float Spd_mea;          // Speed measurement [m/mn]
	float strip_tension;    // Strip Tension
};

// ==========================================
// Communication L-TOP -> line PLC
// ==========================================
struct  str_line_spt
{
	float Spd_spt;    // Line speed set point [m/mn]
	bool spt_valid;   // Speed set point validity [0=non-valid, 1=valid]
	string coil_id;
};

extern struct str_line_coil s_line_coil[MAX_NB_COIL];
extern struct str_line_trk s_line_trk;
extern struct str_line_spt s_line_spt;