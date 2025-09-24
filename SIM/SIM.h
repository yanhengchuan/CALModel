/*============================================================================
/ SIM.h: Simulation structures.
/ 
/ Revisions:
/  Created:       2016-Apr       JC.Mitais
/  Modified: 
/  
/ (c) CMI Industry
/===========================================================================*/
extern CRITICAL_SECTION Sec_SIM_Proc0;		

struct str_simulation
{
   int nb_coil;                                // number of coils
   struct coil_data s_coil_data[MAX_NB_COIL];      // basic coil queue

   float weld_pos;   // Weld location

   struct str_line_coil s_line_coil[MAX_NB_COIL];
   struct str_line_trk s_line_trk;
   struct str_fur_mea s_fur_mea[MAX_NB_FUR];

};