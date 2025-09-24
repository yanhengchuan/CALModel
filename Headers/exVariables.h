/*============================================================================
/ exVariables.h: Declaration of external variables.
/
/ Revisions:
/  Created:       2007 Sep      JC.Mitais
/  Modified: 
/  
/ (c) CMI Industry
/===========================================================================*/
extern int SIMULATION;
extern int ErrorComOPC;

// FUEL
// ====
extern float LHV[NB_FUEL];                      // Low heating value of each fuel used [J/Nm3]
extern float VA[NB_FUEL];						      // Air volume for each fuel
extern float VF[NB_FUEL];						      // waste gazes volume for each fuel used  
extern float AIR_DEF[NB_FUEL];					   // Air default [%]