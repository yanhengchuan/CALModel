/*============================================================================
/ SPE_config.h: CISDI YIEH PHUI Installation Configuration.
/
/ Revisions:
/  Created:       2007 Jan      JC.Mitais
/  Modified:      2008 Mai      X.Fournaud
/  
/ (c) CMI Industry
/===========================================================================*/
// Directories and files  
// ---------------------
#define BD_PROCESS "LTOP_CISDI_PROCESS"
#define BD_COMMUNICATIONS "LTOP_CISDI_COMMUNICATIONS"

#define SITE 
#if defined(SITE)
   // SQL Database
   #define SQL_ConnectionPRO "Provider=SQLNCLI11.0;Server=.\\;Database="
   #define SQL_ConnectionCOM "Provider=SQLNCLI11.0;Server=.\\;Database="

   // Directories
   #define DIR_DAT            "Z:\\Project Specific\\CISDI\\DATA\\"
      #define FILE_BISRA         "BISRA.data"
      #define FILE_HOTTL         "Hottl.data"

   #define DIR_SAV            "C:\\LTOP\\CISDI\\INSTAL\\save\\"
      #define FILE_SAVE          "LTOP_Save.L_TOP"
      #define FILE_PRM_SAVE      "LTOP_Prm_Save.L_TOP"

   #define DIR_CFG            "C:\\LTOP\\CISDI\\INSTAL\\config\\"
      #define FILE_CONFIG        "LTOP_Config.csv"
      #define FILE_PRM           "ktl_Param.dat"
      #define FILE_SIM_SCH       "ktl_Simulation.txt"
	   #define FILE_EMIS          "ktl_Emissivity.txt"
	   #define FILE_PID           "ktl_Pid.txt"
      #define FILE_TRCDAT        "ktl_Traces_Def.dat"

   #define DIR_TRC            "C:\\LTOP\\CISDI\\INSTAL\\trace\\"
      #define FILE_SIM           "LTOP_Sim.L_TOP"
#else
   // SQL Database
   #define SQL_ConnectionPRO "Provider=SQLNCLI11.0;Server=FRC1367;Database="
   #define SQL_ConnectionCOM "Provider=SQLNCLI11.0;Server=FRC1367;Database="

   // Directories
   #define DIR_DAT            "Z:\\Project Specific\\CISDI\\DATA\\"
      #define FILE_BISRA         "BISRA.data"
      #define FILE_HOTTL         "Hottl.data"

   #define DIR_SAV            "C:\\LTOP\\CISDI\\INSTAL\\save\\"
      #define FILE_SAVE          "LTOP_Save.L_TOP"
      #define FILE_PRM_SAVE      "LTOP_Prm_Save.L_TOP"

   #define DIR_CFG            "C:\\LTOP\\CISDI\\INSTAL\\config\\"
      #define FILE_CONFIG        "LTOP_Config.csv"
      #define FILE_PRM           "ktl_Param.dat"
      #define FILE_SIM_SCH       "ktl_Simulation.txt"
	   #define FILE_EMIS          "ktl_Emissivity.txt"
	   #define FILE_PID           "ktl_Pid.txt"
      #define FILE_TRCDAT        "ktl_Traces_Def.dat"

   #define DIR_TRC            "C:\\LTOP\\CISDI\\INSTAL\\trace\\"
      #define FILE_SIM           "LTOP_Sim.L_TOP"
#endif

// Section index
// =============
// In any case, first index ("0") is for RTF/RTH/RTS, not NOF
enum SEC
{
   RTH = 0,
   RTS = 1,
   TNL = -1,
   SLW = 2,
   JET = 3,
   OVG = 4,
   FCS = 5,
   EXT = 6,
   APC = 7,
};

// GENERAL 
// ==========
#define LANGUAGE        1           // Language, 0->Fr, 1->En, 2->Ru, 3->Sp
#define NMAX_MODULES_TRC   100      // Max nb of modules for traces
#define L_FIL_NAM          100      // Max length for file names
#define USE_ROLL_CAL       true     // If horizontal rtf furnace don't use roll calulation
#define MAX_ITER           100

#define MAX_NB_FUR 8

#define NB_FUR_RTF   2     // Number of RTH sections
#define NB_FUR_TNL   0     // Number of TNL sections
#define NB_FUR_SLW   1     // Number of SLW sections
#define NB_FUR_JET   1     // Number of JET sections
#define NB_FUR_OVG	1     // Number of OVG sections
#define NB_FUR_FCS	1     // Number of FCS sections
#define NB_FUR_EXT	1     // Number of EXT sections
#define NB_FUR_APC	1     // Number of APC sections

// PREHEATING
// ==========
#define PRE_HEAT_SECTION   true        // True if a preheating section exists
#define PRE_HEA_STP_TMP  160.0 + TK    // Used for simulation or if no tmp measurement reading

#define PRE_RTH_DST  100.0	// Distance from the weld detector to the entrance of RTH [m]

// RTH
// ===
#define NB_ECZ_RTH   15    // Number of Elementary Control Zones
#define NB_ELT_RTH   30    // Number of Elements
#define NB_ZON_RTH   15    // Number of control zones
#define NB_ROL_RTH   14    // Number of rolls
#define TRANS_CALC_RTH  true 

// RTS
// ===
#define NB_ECZ_RTS   9     // Number of Elementary Control Zones    
#define NB_ELT_RTS   18    // Number of Elements
#define NB_ZON_RTS   5     // Number of control zones
#define NB_ROL_RTS   8     // Number of rolls
#define TRANS_CALC_RTS  false     // test 

// Maximum values
// --------------
#define MAX_NB_ZON   max(NB_ZON_RTH, NB_ZON_RTS)
#define MAX_NB_ELT   max(NB_ELT_RTH, NB_ELT_RTS)
#define MAX_NB_ECZ   max(NB_ECZ_RTH, NB_ECZ_RTS)
#define MAX_NB_ROL   max(NB_ROL_RTH, NB_ROL_RTS)

// RTF
#define TUB_MAX_TMP     950.f + TK  //   
#define TUB_SPT_STOP    700.f + TK  // 
#define EFFI_FURNACE    0.62f
#define STRIP_TENSION   60000.f     // [Pa] 

// TNL
// ===
#define NB_ZON_TNL   1     // TNL zone control number

// SLW
// ===
#define NB_ZON_SLW   1     // slow cooling zone control number

// JET
// ===
#define NB_ZON_JET   3     // Jets cooling zone control number
#define USE_JET_SPEED   false // If true, jet calculated max speed is run and used

// OVG
// ===
#define NB_ZON_OVG	9

// FCS
// ===
#define NB_ZON_FCS	8

// EXT
// ===
#define NB_ZON_EXT	3

// APC
// ===
#define NB_ZON_APC	4

// Model
// =====
#define RO_STEEL     7850.f
#define CP_STEEL     450.f       // [J/kg/K] 

// SPD
// ===
#define MAX_SPEED  320.f / 60.f
#define MIN_SPEED   30.f / 60.f
   
 // COIL QUEUE
 // ==========
#define MAX_NB_COIL   7
#define SPG_NB_COIL   5				// Max number of coils on which SPG calculations are performed

 //ROLLS
 //=====
//#define MAX_NB_MESH        50    // Rolls mesh

// Adaptation
// ==========
#define MAX_NB_PDT   10    // Maximum number of products
#define MAX_NB_CLA   10    // Maximum number of classes for products

// Communications --> 1300 !
// ==============
#define MSG_COM_HANDLING_MEA   	WM_USER+100
#define MSG_COM_HEATING_MEA   	WM_USER+200
#define MSG_COM_HEATING_SPT		WM_USER+300

// SQL 
#define MSG_SQL_UPD  WM_USER+400
#define MSG_SQL_SPG  WM_USER+500
#define MSG_SQL_SPS  WM_USER+600
#define MSG_SQL_SPE  WM_USER+700
#define MSG_SQL_HIS  WM_USER+800
#define MSG_SQL_REP  WM_USER+900
#define MSG_SQL_FUR_SPT  WM_USER+1000
#define MSG_SQL_LIN_SPT  WM_USER+1100


#define CORRECT_TIMER		1.f			// for L(H)-TOP business
