/*============================================================================
/ Specific.h: All specific common data definition.
/ 
/ Revisions:
/  Created:       2008-Nov       J. NAVEIRA
/  Modified: 
/  
/ (c) CMI Industry
/===========================================================================*/
struct str_fur_mea_spe
{
   float slw_fan_spd[3];
};

// ==========================================
// Specific RTF structure
// ==========================================
struct str_spe_rtf
{
};

// ==========================================
// Specific Adaptation structure
// ==========================================
struct Str_adp_spe
{
};

// ==========================================
// Specific cooling DATA
// ==========================================
struct jet_spe
{
};

// ==========================================
// Specific coil DATA
// ==========================================
struct coil_spe
{
	int	Prev_roll_mill;   // Previous Cold Rolling Mill Number [int]	
   int   weld_code;        // Weld code
   string  cycle;
};

// ==========================================
// Specific function class
// ==========================================
class Specific
{
	public:
      static float Pyrometer (struct rtf_struct *rtf);
      static bool Specific::ThreadMsgTreatment(bool *MsgCoil);

   private:
};

// ==========================================
// Specific function class
// ==========================================
class SPS_Specific
{
	public:
		static bool SPS();
		static bool INIT();
		static bool SPS_getdata ();
		static bool SIM_INI ();
		static void Setpoints();
   private:
};
// ==========================================
// Specific function class
// ==========================================
class SPG_Specific
{
	public:
		static bool SPG_init_TRS (struct rtf_struct *rtf);

   private:
};

//===========================================
// Specific PROJECT structure
//===========================================
struct str_spe_PRO
{
	struct str_mea_PRO
   {
      float pre_hea_stp_tmp;  // Pre heating section strip temperature measurement [K]
      float Spd_max_ope;      // Operator maximum speed [m/s]
      float Spd_coef_ope;     // Speed reduce cofficient set by operator
   } s_mea;
};

extern struct str_spe_PRO s_spe_PRO;
