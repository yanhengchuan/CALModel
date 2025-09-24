/*============================================================================
/ ALM.h: Structures definition for alarms.
/ 
/ Revisions:
/  Created:       2009-feb      J. NAVEIRA
/  Modified: 
/  
/ (c) CMI Industry
/===========================================================================*/
struct str_alm_prg
{
   string Alm[5];
};

struct str_alm
{
   struct str_alm_prg s_rtf_spg;                   // SPG_RTF
   struct str_alm_prg s_rtf_Cmp_Max_Spd;           // SPG_RTF_Max_Spd
	struct str_alm_prg s_OPC_class;						// OPC_class
};
