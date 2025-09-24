/*============================================================================
/ R-TOP Level 2 System Standard static library
/
/ (c) CMI Industry
//===========================================================================*/
// C General includes
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <memory.h>
#include <string>
#include <algorithm>

#include <iostream>          // for cout
#include <fstream>
#include <iomanip>
#include <sstream>

#include <vector>
#include <ATLComTime.h >

using namespace std;

// GENERAL 
// ==========
#define NMAX_MODULES_TRC   100      // Max nb of modules for traces
#define MAX_ITER        100
#define MSG_LEN         1300
#define SIGMA  (float)  5.675e-8 // Stefan-Boltzman constant
#define TK              273.15f
#define RO_STEEL        7850.f   // Default steel density
#define EMI_CAL         0.8f
#define CPWAT           4185.f
#define PI              3.1415926f

// Physical data
// ===============
#define M_H2 2.016f     // Molecular weight of Hydrogen [g/mol]
#define M_N2 28.0134f   // Molecular weight of Nitrogen [g/mol]

// Variant lengths
// ===============
#define L_I 4     // Integer
#define L_B 4     // Boolean
#define L_R 4     // float

 //ROLLS
 //=====
#define NB_TMP_CHAR_ROL 9
#define NB_TMP_DILA     5
#define MAX_NB_MESH     50    // Rolls mesh
#define NB_PTS_YOUNG    6     // Modules d'Young

 // FUEL
 // ====
#define NB_FUEL 1
extern bool SQL_TRACES_INSERT(string, string, string, string);

namespace ns_STD
{
   // ***
   // UTL
   // ***
   class cl_UTL
   {
   public:
      static float Lin_Interp(float x, float abscissa[], float ordinates[], int n);
      static float Interpolation (float x[], float y[], float yc, int n);
      static float Thales(float x, float X0, float Y0, float X1, float Y1);
      static void ParabolaCoeff(float X0, float Y0, float X1, float Y1, float X2, float Y2,
                                float *A, float *B, float *C);
      static void Range(float y, float data[], int n, int *a, int *b);
      static string timeToDateTime(time_t CurTim);
      static time_t COleDateTimeToTime_t(COleDateTime CurTim);
   }; // class

   // ***
   // RTF
   // ***
   class cl_RTF
   {
   public:

      struct  struct_ecz   // Elementary Control Zone definition structure
      {
         int elt_u;        // Element up  
         int elt_d;        // Element down  
         int i_zon;        // Zone 
         float  fr[3][3];  // Radiation factor 
         float  deter;     // Determinant of the matrice [w/m²]
         float  flux;      // Flow radiation given by tube [W] 

         // Radiant Tube structure
         struct tube_model
         {
            float area;    // Tube area [m²]
            float tmp_tub; // Tube temperature [K]
            float emi;     // Tube emissivity 
            float eff;     // Tube efficiency 
         } s_tub;
      };

      struct  struct_elt
      {   
         bool i_wall;   // True = wall, false = strip 
         bool i_roll;   // True = on a roll, false = not 
         int  ecz_u;    // Element up 
         int  ecz_d;    // Element down 

         // Strip data structure
         struct  strip_model_rtf
         {   
            float  thi;    // Strip thickness [m]
            float  wid;    // Strip width [m]
            float  lgt;    // Length  [m]
            float  emi;    // Strip emissivity 

            float  tmp_in_strip;    // Entry strip temperature [K]
            float  tmp_mid_strip;   // Median strip température [K]
            float  tmp_out_strip;   // Exit strip temperature [K]
         } s_strip; 

         // Roll definition structure
         struct roll_model
         {
            float weigth;     // Roll weigth [Kg]
            float coeff;      // Thermal exchange coefficient between the strip and the roll [W/m²/K]
            float r_t_b_c;    // Roll temperature before calculation [K]
            float r_t_a_c;    // Roll temperature after calculation [K]
         } s_rol; 
      };

      static struct struct_ecz s_ecz;
      static struct  struct_elt  s_elt[];

      static void RTF_Radiation_factor( float e[], float s[], float fr[][3], float *deter, float ff22 );
      static bool RTF_Radiation_calcul( struct struct_ecz s_ecz, struct struct_elt s_elt[], int nb_elt, float *dq1, float *dq2, float *dq3 );
   }; // class

   // ***
   // ROL
   // ***
   class cl_ROL
   {
   public:

      struct shield_car
      {
         float thi;              // thickness  
         float wid;              // width 
         float area;
         float tmp;              // temperature 
         float e;                // emissivity 
      };

      struct strip_car
      {   
         float  thi;             // thickness 
         float  wid;             // width 
         int    steel_charac;    // 1=CQ, 2=DQ, 3=DDQ 
         float  tmp;             // temperature 
         float  tension;         // tension 
         float  tension_basis;   // tension 
         float  young;           // young modulus 
         float  lim_el;          // elastic limit 
      };

      struct tube_car
      {
         float  tmp;             // diameter 
         float  area;
         float  e;
      };

      struct mesh_car
      {
         float  x;               // x position 
         float  dx;              // length of the mesh 
         float  heat_exp;        // heat expansion of the roll 
         float  hot_prf;         // hot profil of the roll 
         float  cold_prf;        // cold profil 
         float  tmp;             // temperature 
         float  heat_out;        // heat exchange received by the mesh from outside 
         float  heat_inp;        // heat exchange received by the mesh from inside 
         float  heat_tot;        // total heat exchange
         float  condu;           // conductivity according to the temperature 
         float  spec_heat;       // specific heat 
      };

      struct roll_car
      {
         // If indic=0 no shield; if 1 shield under roll; if 2, 2 shields under and between the rolls 
         int    indic;
         bool   top;             // roll on top = true, on bottom =false 
         bool   display;         // true the roll profil must be display 
         int    indic_roll;      // indicator for the type of roll  
         float  l1;              // 
         float  l2;              // 
         float  l3;              // 
         float  c1;              // 
         float  c2;              // 
         float  lgt;             // length 
         float  dia;             // diameter 
         float  thi;             // thickness 
         float  deter;           // form factor outside the strip
         float  deter_u;         // Form factor for upper side of the roll -- Delay first roll profil calcul
         float  fr[3][3];        // form factor under the strip
         float  fr_u[3][3];      // form factor for upper side of the roll -- first roll profil calculation when line stop
         float  e;               // 
         float  risk;            // risk of bukle 
         struct shield_car  s_shield;
         struct strip_car   s_strip;
         struct tube_car    s_tub;
         struct mesh_car    s_mesh[MAX_NB_MESH];
         struct mesh_car    s_mesh_risk[2][MAX_NB_MESH];
      };

      struct str_rolls
      {
         float Rol_Cond[NB_TMP_CHAR_ROL];    // Roll profile calulation specific structures
         float Rol_Cp[NB_TMP_CHAR_ROL];      // Roll profile calulation specific structures
         float Rol_Tmp[NB_TMP_CHAR_ROL];     // Roll profile calulation specific structures
         float Rol_Dila[NB_TMP_DILA];        // Roll profile calulation specific structures
         float Rol_Tdil[NB_TMP_DILA];        // Roll profile calulation specific structures
      };

      static struct roll_car s_rol;    // 
      static struct str_rolls s_Trol;   // Roll thermal

      static void ROL_Radiation_factor( float e[3], float s[3], float fr[3][3], float *deter );
      static bool ROL_Radiation_calcul( struct roll_car s_rol, int n_mesh, float *dq1, float *dq2, float *dq3 );
      static void ROL_Radiation_inside( struct roll_car s_rol, int n_mesh, float *dq );
      static void ROL_dynamic_init (struct roll_car *s_rol);
      static void ROL_conduction( struct roll_car *s_rol, struct str_rolls s_Trol, int n_mesh, float *dq );
      static bool ROL_delta_q (struct roll_car *s_rol, struct str_rolls s_Trol, float coef, int n_mesh, float *dq_roll);
      static void ROL_mechanical( struct roll_car *s_rol, struct str_rolls s_Trol );
      static bool ROL_mechanical_risk( struct roll_car *s_rol, struct str_rolls s_Trol );
      static bool ROL_conduction_risk( struct roll_car *s_rol, struct str_rolls s_Trol, int n_top, float *dq , float dt);
      static bool ROL_array_compare ( struct roll_car *s_rol, float old_ex_t_rol[] );
      static bool ROL_thermal( struct roll_car *s_rol, struct str_rolls s_Trol, float coef, float old_ex_t_rol[] );
      static bool ROL_profil_calcul ( struct roll_car *s_rol, struct str_rolls s_Trol, float coef, float old_ex_t_rol[] );
      static bool SPS_DEL_first_roll_thermal( struct roll_car *s_rol, struct str_rolls s_Trol, float dt);
   }; // class

   // ***
   // CNV
   // ***
   class cl_CNV
   {
   public:
      static float tmp_F_K (float input_temp);
      static float tmp_K_C(float input_temp);
      static float tmp_K_F (float input_temp);
      static float dtmp_F_K(float input_temp);
      static float dtmp_K_F(float input_temp);
      static float lgt_mm_m(float input_len);
      static float lgt_m_mm(float input_len);
      static float lgt_in_m(float input_len);
      static float lgt_ft_m(float input_len);
      static float lgt_m_in(float input_len);
      static float lgt_m_ft(float input_len);
      static float air_flw_m3ph_m3ps (float input_flow);
      static float HFO_lph_kgps (float input_flow);
      static float tim_mn_s(float input_time);
   }; // class

   // ***
   // PHY
   // ***
   class cl_PHY
   {
   public:
      struct str_Hottl
      {
         static float    tmp[25];                      // 
         static float    pCO2H2O_L[18];                // 
         static float    emiss_CO2[18][25];            // 
         static float    emiss_H2O[18][25];            // 
         static float    T_correcH2O[5];               // 
         static float    pH2O_L[8];                    // 
         static float    correc_H2O[5][8];             // 
         static float    pCO2H2O_L_correc[10];         // 
         static float    pH20_pH2OCO2_correc[11];      // 
         static float    correc126[10][11];            // 
         static float    correc535[10][11];            // 
         static float    correc940[10][11];            // 
      }; // Hottl tables

      static struct str_Hottl s_Htl;    // Hottl structure concerns gazes characteristics

      static float Tab_tyo    [NB_PTS_YOUNG];
      static float Tab_yo     [NB_PTS_YOUNG];
      static float Tab_tecq   [NB_PTS_YOUNG];
      static float Tab_ecq    [NB_PTS_YOUNG];
      static float Tab_tedq   [NB_PTS_YOUNG];
      static float Tab_edq    [NB_PTS_YOUNG];
      static float Tab_teeddq [NB_PTS_YOUNG];
      static float Tab_eeddq  [NB_PTS_YOUNG];

      static float Tab_Rol_Cond [NB_TMP_CHAR_ROL];
      static float Tab_Rol_Cp [NB_TMP_CHAR_ROL];
      static float Tab_Rol_Tmp [NB_TMP_CHAR_ROL];
   
      static float Tab_Rol_Tdil  [NB_TMP_DILA];
      static float Tab_Rol_Dila [NB_TMP_DILA];
   }; // class

   // ***
   // Waste Gases
   // ***
   class cl_WG
   {
   public:
      static float WG_H[NB_FUEL][28];   // Waste gazes enthalpie for each fuel used [J/Nm^3]
      static float WG_T[28];             // waste gazes temperature attached to the enthalpy above [K]

      // Lacq gas with 5% air excess 
      static float Tab_Wg_Enth[NB_FUEL][28];

      static float AIR_Enth(float Tair);
      static float WG_Enth(int n_fuel, float TWg);
      static float WG_Tmp(int n_fuel, float HWg);
   }; // class

   //******
   // BISRA
   // *****
   class cl_BIS
   {
   public:
      #define NVAL_BIS		28L 		   // Number of values for each BISRA type.
      #define NB_ACIER		22L			// Number of steels in the BISRA tables

      struct str_bisra
      {
	      float	CO[NB_ACIER][NVAL_BIS];	//  Conductivities
	      float	CP[NB_ACIER][NVAL_BIS];	//  Calorific capacities [J/Kg]
	      float	RO[NB_ACIER][NVAL_BIS];	//  Volumic masses [Kg/m^3]
	      float	H[NB_ACIER][NVAL_BIS];	//  Enthalpies [J/Kg/K]
	      float	TB[NVAL_BIS];    	      //  BISRA temperatures
      };

      static struct str_bisra s_BIS;   // Bisra table strucuture

      static float BISRA_Enth(int bisra, float tmp);
      static float BISRA_Tmp(int bisra, float H);
      static float BISRA_Cp(int bisra, float tmp);
      static float BISRA_Co(int bisra, float tmp);
      static bool Bisra_Data(string fil_name);
   }; // class

   // ***
   // TRC
   // ***
   class cl_TRC
   {
   public:
      static void ERR_Write(string Method, string Module, const char *format, ...);
      static void TRC_Write(string LevelTxt, string Method, string Module, const char *format, ...);
      static bool TRC_Sub( string sub_call_name );
      static void TRC_Read_File(char fil_nam[]);
   }; // class

   // ***
   // PID
   // ***
   class cl_PID
   {
   public:
      struct fba_pid
      {   
         float p;             // proportionnal value 
         float i;             // Integral value 
         bool  automatic;     // PID in AUTO mode 
         float pv;            // process value 
         float sp;            // set point value 
         float max_exit;      // maximum exit 
         float min_exit;      // minimum exit 
         float i_exit;        // integral value 
         float pid_exit;      // PID exit 
         bool  reset;         // If true, reset PID 
      };

      static bool SPS_PID (struct fba_pid *pid);
   }; // class

} // namespace
