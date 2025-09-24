/*============================================================================
/ UTL.cpp:  Utilities.
/  
/ (c) CMI Industry
//===========================================================================*/
#include "stdafx.h"

/*============================================================================
/ CFG_Read: Reading definition file (LTOP_Config.csv).
/  
/ Remarks:
/  - This function reads a .csv file that can be created through MS EXCEL.
/  - The field separator is ";".
/ 
/ Revisions:
/     Created:    2006-Jan       JC.Mitais
/     Modified:   2016-May       JC.Mitais
//===========================================================================*/
bool CFG_Read()
{
   bool status=true, wait=false;
   string sub_name="CFG_Read";

   #define MAX_NB_CHAR 300
   #define MAX_NB_KEYWORD MAX_NB_ELT + 5
   #define LGT_KEYWORD 51

   int n_lin=0, n_fur=-1, n_rtf=-1, n_fuel=-1, nn_fuel=-1, n_set=-1, n_fuel_flow;
   int len_lin, n_key;
   FILE *Config_file;    //file pointer.
   string keyword[MAX_NB_KEYWORD], keyw, keyword_1;
   char line[MAX_NB_CHAR];

   static bool first_time=true;

   // CONFIG file
   // -----------
   string fil_name = (string)DIR_CFG + (string)FILE_CONFIG;

   // Opening Configuration file.
   if ( fopen_s( &Config_file, fil_name.c_str(), "r") == 0 )
   {
      // Set pointer to file begin
      fseek (Config_file, 0L, SEEK_SET);  

      // While not end of file or error
      while ( fgets ( line, 200, Config_file ) != NULL )
      {
         n_lin ++;
         len_lin = (int) strlen(line);
         for ( int i_keyw=0; i_keyw < MAX_NB_KEYWORD; i_keyw++ ) keyword[i_keyw] = "";
         n_key = 0;
         wait = false;
         keyword_1 = "";

         // For all characters of current line
         for ( int i_char=0; i_char < len_lin && n_key < MAX_NB_KEYWORD; i_char++ )
         {
            // If current char is ";" (csv separator)
            if ( line[i_char] == ';' )
            {
               wait = false;
               // If something has already been read
               if ( keyword_1 != "" )
               {
                  // End the keyword
                  //strcpy_s( keyword[n_key], LGT_KEYWORD, keyword_1 );
                  keyword[n_key] = keyword_1;
                  n_key++;
                  keyword_1 = "";
               } // If something has already been read
            } // If current char is ";" (csv separator)

            // Else if current character is a blank or is new_line
            else if ( line[i_char] == ' ' || line[i_char] == '\n' )
            {
               // If something has already been read
               if ( keyword_1 != "" )
               {
                  // End the keyword
                  //strcpy_s( keyword[n_key], LGT_KEYWORD, keyword_1 );
                  keyword[n_key] = keyword_1;
                  n_key ++;
                  keyword_1 = "";
                  wait = true;
               } // If something has already been read
            } // Else if current character is a blank or is new_line

            // Else we don't store the blank characters neither the new_line ones, but the other ones
            else if ( !wait )
            {
               keyword_1 += line[i_char];
            } // Else we don't store the blank characters neither the new_line ones, but the other ones
         } // For all characters of current line

         if ( keyword_1 != "" ) keyword[n_key] = keyword_1;
         else n_key --;

         // ===================
         // "RTF" keyword
         // ===================
         if ( keyword[0] == "RTF" )
         {
            n_fur ++;
            n_rtf ++;
            n_fuel_flow = -1;

            if (n_fur >= MAX_NB_FUR)
            {
               status = false;
               ns_STD::cl_TRC::ERR_Write("CFG", sub_name, "Error keyword '%s': too many RTF lines regarding the max number of furnaces", keyword[0]);
               MessageBoxA(0, "RTF: Max nb of furnaces too low, the application will be closed", 0, 0);
               return status;
            } 

            if (n_rtf >= NB_FUR_RTF)
            {
               status = false;
               ns_STD::cl_TRC::ERR_Write("CFG", sub_name, "Error keyword '%s': too many RTF lines regarding the max number of rtf", keyword[0]);
               MessageBoxA(0, "RTF: Max nb of rtf too low, the application will be closed", 0, 0);
               return status;
            } 

            fur_cfg[n_fur].fur_name = keyword[1];
            sscanf_s( keyword[2].c_str(), "%d", (float *)&rtf_cfg[n_rtf].elt_pyro );
            sscanf_s( keyword[3].c_str(), "%f", (float *)&rtf_cfg[n_rtf].one_rol_mass );
            sscanf_s( keyword[4].c_str(), "%f", (float *)&rtf_cfg[n_rtf].tot_tub_mass );
            sscanf_s( keyword[5].c_str(), "%f", (float *)&rtf_cfg[n_rtf].width );
            sscanf_s( keyword[6].c_str(), "%f", (float *)&rtf_cfg[n_rtf].height );
            sscanf_s( keyword[7].c_str(), "%f", (float *)&rtf_cfg[n_rtf].r_coeff );
         } // "RTF" keyword

         // ===================
         // "ROLLS" keyword
         // ===================
         if ( keyword[0] == "ROLLS" )
         {
            rol_cfg[n_rtf].n_fur = n_rtf;
            sscanf_s( keyword[1].c_str(), "%d", (float *)&rol_cfg[n_rtf].n_rol_cmp );
            sscanf_s( keyword[2].c_str(), "%d", (float *)&rol_cfg[n_rtf].rol_indic );
            sscanf_s( keyword[3].c_str(), "%f", (float *)&rol_cfg[n_rtf].dia );
            sscanf_s( keyword[4].c_str(), "%f", (float *)&rol_cfg[n_rtf].L1 );
            sscanf_s( keyword[5].c_str(), "%f", (float *)&rol_cfg[n_rtf].L2 );
            sscanf_s( keyword[6].c_str(), "%f", (float *)&rol_cfg[n_rtf].L3 );
            sscanf_s( keyword[7].c_str(), "%f", (float *)&rol_cfg[n_rtf].C1 );
            sscanf_s( keyword[8].c_str(), "%f", (float *)&rol_cfg[n_rtf].C2 );
            sscanf_s( keyword[9].c_str(), "%f", (float *)&rol_cfg[n_rtf].emi_outside );
            sscanf_s( keyword[10].c_str(), "%f", (float *)&rol_cfg[n_rtf].thick );

            sscanf_s( keyword[11].c_str(), "%d", (float *)&rol_cfg[n_rtf].shield_indic );
            sscanf_s( keyword[12].c_str(), "%f", (float *)&rol_cfg[n_rtf].shield_emi );
            sscanf_s( keyword[13].c_str(), "%f", (float *)&rol_cfg[n_rtf].shield_wid );

            rol_cfg[n_rtf].n_rol_cmp --;  // 1 to N in CFG file, à to N-1 for index
         } // "ROLLS" keyword

         // ===================
         // "MODEL" keyword
         // ===================
         if ( keyword[0] == "MODEL" )
         {
            sscanf_s( keyword[1].c_str(), "%f", (float *)&rtf_cfg[n_rtf].emi_tub );
            sscanf_s( keyword[2].c_str(), "%f", (float *)&rtf_cfg[n_rtf].emi_wal );
            sscanf_s( keyword[3].c_str(), "%f", (float *)&rtf_cfg[n_rtf].emi_stp );
            sscanf_s( keyword[4].c_str(), "%f", (float *)&rtf_cfg[n_rtf].ff22 );
         } // "MODEL" keyword

         // ===================
         // "STEPS" keyword
         // ===================
         if ( keyword[0] == "STEPS" )
         {
            sscanf_s( keyword[1].c_str(), "%f", (float *)&rtf_cfg[n_rtf].tub_max_ref );
            sscanf_s( keyword[2].c_str(), "%f", (float *)&rtf_cfg[n_rtf].tub_min_ref );
            sscanf_s( keyword[3].c_str(), "%f", (float *)&rtf_cfg[n_rtf].tub_min_min );
            sscanf_s( keyword[4].c_str(), "%f", (float *)&rtf_cfg[n_rtf].max_width );
            sscanf_s( keyword[5].c_str(), "%f", (float *)&rtf_cfg[n_rtf].min_width );
            sscanf_s( keyword[6].c_str(), "%f", (float *)&rtf_cfg[n_rtf].max_thick );
            sscanf_s( keyword[7].c_str(), "%f", (float *)&rtf_cfg[n_rtf].min_thick );

            rtf_cfg[n_rtf].tub_max_ref += TK;
            rtf_cfg[n_rtf].tub_min_ref += TK;
            rtf_cfg[n_rtf].tub_min_min += TK;
            rtf_cfg[n_rtf].max_thick /= 1000.f;
            rtf_cfg[n_rtf].min_thick /= 1000.f;
         } // "STEPS" keyword

         // ===================
         // "FIRST_ECZ" keyword
         // ===================
         else if ( keyword[0] == "FIRST_ECZ" )
         {
            if ( !int_KeyWordTreat(keyword, MAX_NB_ZON, rtf_cfg[n_rtf].FIRST_ECZ) )
            {
               MessageBoxA(0, "FIRST_ECZ: Max nb of zones too low, the application will be closed", 0, 0);
               return status;
            }
         } // "FIRST_ECZ" keyword

         // ===================
         // "LAST_ECZ" keyword
         // ===================
         else if ( keyword[0] == "LAST_ECZ" )
         {
            if ( !int_KeyWordTreat(keyword, MAX_NB_ZON, rtf_cfg[n_rtf].LAST_ECZ) )
            {
               MessageBoxA(0, "LAST_ECZ: Max nb of zones too low, the application will be closed", 0, 0);
               return status;
            }
         } // "LAST_ECZ" keyword

         // ===================
         // "MAX_POWER" keyword
         // ===================
         else if ( keyword[0] == "MAX_POWER" )
         {
            if ( !float_KeyWordTreat(keyword, MAX_NB_ZON, rtf_cfg[n_rtf].MAX_POWER) )
            {
               MessageBoxA(0, "MAX_POWER: Max nb of zones too low, the application will be closed", 0, 0);
               return status;
            }
         } // "MAX_POWER" keyword

         // ===================
         // "TUBE_MAX_NARROW" keyword
         // ===================
         else if ( keyword[0] == "TUBE_MAX_NARROW" )
         {
            if ( !float_KeyWordTreat(keyword, MAX_NB_ZON, rtf_cfg[n_rtf].TUB_MAX_NARROW) )
            {
               MessageBoxA(0, "TUBE_MAX_NARROW: Max nb of zones too low, the application will be closed", 0, 0);
               return status;
            }
         } // "TUBE_MAX_NARROW" keyword

         // ===================
         // "TUBE_MIN_NARROW" keyword
         // ===================
         else if ( keyword[0] == "TUBE_MIN_NARROW" )
         {
            if ( !float_KeyWordTreat(keyword, MAX_NB_ZON, rtf_cfg[n_rtf].TUB_MIN_NARROW) )
            {
               MessageBoxA(0, "TUBE_MIN_NARROW: Max nb of zones too low, the application will be closed", 0, 0);
               return status;
            }
         } // "TUBE_MIN_NARROW" keyword

         // ===================
         // "TUBE_MAX_WIDE" keyword
         // ===================
         else if ( keyword[0] == "TUBE_MAX_WIDE" )
         {
            if ( !float_KeyWordTreat(keyword, MAX_NB_ZON, rtf_cfg[n_rtf].TUB_MAX_WID) )
            {
               MessageBoxA(0, "TUBE_MAX_WIDE: Max nb of zones too low, the application will be closed", 0, 0);
               return status;
            }
         } // "TUBE_MAX_WIDE" keyword

         // ===================
         // "TUBE_MIN_WIDE" keyword
         // ===================
         else if ( keyword[0] == "TUBE_MIN_WIDE" )
         {
            if ( !float_KeyWordTreat(keyword, MAX_NB_ZON, rtf_cfg[n_rtf].TUB_MIN_WID) )
            {
               MessageBoxA(0, "TUBE_MIN_WIDE: Max nb of zones too low, the application will be closed", 0, 0);
               return status;
            }
         } // "TUBE_MIN_WIDE" keyword

         // ===================
         // "TUBE_MAX_THICK" keyword
         // ===================
         else if ( keyword[0] == "TUBE_MAX_THICK" )
         {
            if ( !float_KeyWordTreat(keyword, MAX_NB_ZON, rtf_cfg[n_rtf].TUB_MAX_THICK) )
            {
               MessageBoxA(0, "TUBE_MAX_THICK: Max nb of zones too low, the application will be closed", 0, 0);
               return status;
            }
         } // "TUBE_MAX_THICK" keyword

         // ===================
         // "TUBE_MIN_THICK" keyword
         // ===================
         else if ( keyword[0] == "TUBE_MIN_THICK" )
         {
            if ( !float_KeyWordTreat(keyword, MAX_NB_ZON, rtf_cfg[n_rtf].TUB_MIN_THICK) )
            {
               MessageBoxA(0, "TUBE_MIN_THICK: Max nb of zones too low, the application will be closed", 0, 0);
               return status;
            }
         } // "TUBE_MIN_THICK" keyword

         // ===================
         // "TUBE_MAX_THIN" keyword
         // ===================
         else if ( keyword[0] == "TUBE_MAX_THIN" )
         {
            if ( !float_KeyWordTreat(keyword, MAX_NB_ZON, rtf_cfg[n_rtf].TUB_MAX_THIN) )
            {
               MessageBoxA(0, "TUBE_MIN_THICK: Max nb of zones too low, the application will be closed", 0, 0);
               return status;
            }
         } // "TUBE_MAX_THIN" keyword

         // ===================
         // "TUBE_MIN_THIN" keyword
         // ===================
         else if ( keyword[0] == "TUBE_MIN_THIN" )
         {
            if ( !float_KeyWordTreat(keyword, MAX_NB_ZON, rtf_cfg[n_rtf].TUB_MIN_THIN) )
            {
               MessageBoxA(0, "TUBE_MIN_THIN: Max nb of zones too low, the application will be closed", 0, 0);
               return status;
            }
         } // "TUBE_MIN_THIN" keyword

         // ===================
         // "ecz_elt_u" keyword
         // ===================
         else if ( keyword[0] == "ecz_elt_u" )
         {
            if ( !int_KeyWordTreat(keyword, MAX_NB_ECZ, rtf_cfg[n_rtf].C_ECZ[0]) )
            {
               MessageBoxA(0, "ecz_elt_u: Max nb of ecz too low, the application will be closed", 0, 0);
               return status;
            }
         } // "ecz_elt_u" keyword

         // ===================
         // "ecz_elt_d" keyword
         // ===================
         else if ( keyword[0] == "ecz_elt_d" )
         {
            if ( !int_KeyWordTreat(keyword, MAX_NB_ECZ, rtf_cfg[n_rtf].C_ECZ[1]) )
            {
               MessageBoxA(0, "ecz_elt_d: Max nb of ecz too low, the application will be closed", 0, 0);
               return status;
            }
         } // "ecz_elt_d" keyword

         // ===================
         // "ecz_zone" keyword
         // ===================
         else if ( keyword[0] == "ecz_zone" )
         {
            if ( !int_KeyWordTreat(keyword, MAX_NB_ECZ, rtf_cfg[n_rtf].C_ECZ[2]) )
            {
               MessageBoxA(0, "ecz_zone: Max nb of ecz too low, the application will be closed", 0, 0);
               return status;
            }
         } // "ecz_zone" keyword

         // ===================
         // "TUBE_AREA" keyword
         // ===================
         else if ( keyword[0] == "TUBE_AREA" )
         {
            if ( !float_KeyWordTreat(keyword, MAX_NB_ECZ, rtf_cfg[n_rtf].TUBE_AREA) )
            {
               MessageBoxA(0, "TUBE_AREA: Max nb of ecz too low, the application will be closed", 0, 0);
               return status;
            }
         } // "TUBE_AREA" keyword

         // ===================
         // "elt_wall" keyword
         // ===================
         else if ( keyword[0] == "elt_wall" )
         {
            if ( !float_KeyWordTreat(keyword, MAX_NB_ELT, rtf_cfg[n_rtf].C_ELT[0]) )
            {
               MessageBoxA(0, "elt_wall: Max nb of elt too low, the application will be closed", 0, 0);
               return status;
            }
         } // "elt_wall" keyword

         // ===================
         // "elt_roll" keyword
         // ===================
         else if ( keyword[0] == "elt_roll" )
         {
            if ( !float_KeyWordTreat(keyword, MAX_NB_ELT, rtf_cfg[n_rtf].C_ELT[1]) )
            {
               MessageBoxA(0, "elt_roll: Max nb of elt too low, the application will be closed", 0, 0);
               return status;
            }
         } // "elt_roll" keyword

         // ===================
         // "elt_lgt" keyword
         // ===================
         else if ( keyword[0] == "elt_lgt" )
         {
            if ( !float_KeyWordTreat(keyword, MAX_NB_ELT, rtf_cfg[n_rtf].C_ELT[2]) )
            {
               MessageBoxA(0, "elt_lgt: Max nb of elt too low, the application will be closed", 0, 0);
               return status;
            }
         } // "elt_lgt" keyword

         // ===================
         // "JET" keyword
         // ===================
         if ( keyword[0] == "JET" )
         {
            n_fur ++;

            if (n_fur >= MAX_NB_FUR)
            {
               status = false;
               ns_STD::cl_TRC::ERR_Write("CFG", sub_name, "Error keyword '%s': too many RTF lines regarding the max number of furnaces", keyword[0]);
               MessageBoxA(0, "RTF: Max nb of furnaces too low, the application will be closed", 0, 0);
               return status;
            } 

            jet_cfg.fur_name = keyword[1];
            fur_cfg[n_fur].fur_name = keyword[1];
            sscanf_s( keyword[2].c_str(), "%d", (float *)&jet_cfg.nb_holes );
            sscanf_s( keyword[3].c_str(), "%f", (float *)&jet_cfg.noz_dia );
            sscanf_s( keyword[4].c_str(), "%f", (float *)&jet_cfg.pitch );
            sscanf_s( keyword[5].c_str(), "%f", (float *)&jet_cfg.d_stp_noz );
         } // "JET" keyword

         // ===================
         // "SECTION" keyword
         // ===================
         if ( keyword[0] == "SECTION" )
         {
            n_fur ++;

            if (n_fur >= MAX_NB_FUR)
            {
               status = false;
               ns_STD::cl_TRC::ERR_Write("CFG", sub_name, "Error keyword '%s': too many RTF lines regarding the max number of furnaces", keyword[0]);
               MessageBoxA(0, "SECTION: Max nb of furnaces too low, the application will be closed", 0, 0);
               return status;
            } 

            fur_cfg[n_fur].fur_name = keyword[1];
         } // "SECTION" keyword

         // ===================
         // "FUEL" keyword
         // ===================
         else if ( keyword[0] == "FUEL" )
         {
            n_fuel ++;

            if (n_fuel >= NB_FUEL)
            {
               status = false;
               ns_STD::cl_TRC::ERR_Write("CFG", sub_name, "Error keyword '%s': too many FUEL lines regarding the max number of fuels", keyword[0]);
               MessageBoxA(0, "FUEL: Max nb of fuels too low, the application will be closed", 0, 0);
               return status;
            } 

            sscanf_s( keyword[1].c_str(), "%f", (float *)&LHV[n_fuel] );
            sscanf_s( keyword[2].c_str(), "%f", (float *)&VA[n_fuel] );
            sscanf_s( keyword[3].c_str(), "%f", (float *)&VF[n_fuel] );
            sscanf_s( keyword[4].c_str(), "%f", (float *)&AIR_DEF[n_fuel] );
         } // "FUEL" keyword

      } // While not end of file or error

      // Close the file
      fclose(Config_file);

      // Search the 1st strip element in rtf furnaces
      // --------------------------------------------
      // For all RTF furnaces
      for (int i_rtf=0; i_rtf < NB_FUR_RTF; i_rtf++)
      { 
         // For all elements
         for (int i_elt=0; i_elt < rtf_cfg[i_rtf].nb_elt; i_elt++)
         {
            // If the lement is a strip element
            if (rtf_cfg[i_rtf].C_ELT[0][i_elt] == 0.f && rtf_cfg[i_rtf].C_ELT[1][i_elt] == 0.f)
            {
               rtf_cfg[i_rtf].first_strip_elt = i_elt;
               break;
            } // If the lement is a strip element
         } // For all elements
      } // For all RTF furnaces
   } // The file can be open

   else    // The file cannot be open
   {
      ns_STD::cl_TRC::ERR_Write("CFG", sub_name, "Error Unable to open the configuration file '%s'", fil_name.c_str());

      // If the first_time
      if ( first_time )
      {
         // Generate an alarm
         status = false;
         MessageBoxA(0, "Unable to open the configuration file, the application will be closed", 0, 0);

         ASSERT(AfxGetMainWnd() != NULL);
         AfxGetMainWnd()->SendMessage(WM_CLOSE);
      } // If the first_time

      else
      {
         return true;
      }
   } // The file cannot be open

   first_time = false;

	return status;
} // void

/*============================================================================
/ float_KeyWordTreat: Treat float keywords.
/ 
/ Revisions:
/  Created:       2006-Jan       JC.Mitais
/  Modified: 
//===========================================================================*/
bool float_KeyWordTreat(string keyword[], int MaxVal, float Tab[])
{
   bool status=true;
   string sub_name="float_KeyWordTreat";

   int n_zon = -1;
   int n_key = 1;

   while ( keyword[n_key] != "" && n_key < MAX_NB_KEYWORD )
   {
      n_zon ++;
      if ( n_zon >= MaxVal )
      {
         status = false;
         ns_STD::cl_TRC::ERR_Write("CFG", sub_name, "Error keyword '%s': too many values regarding the max number", keyword[0]);
         MessageBoxA(0, "TUBE_MAX_WIDE: Max nb of values too low, the application will be closed", 0, 0);
         return status;
      }

      sscanf_s( keyword[n_key].c_str(), "%f", (float *)&Tab[n_zon] );
      n_key ++;
   }

   return status;
} // void

/*============================================================================
/ int_KeyWordTreat: Treat int keywords.
/ 
/ Revisions:
/  Created:       2006-Jan       JC.Mitais
/  Modified: 
//===========================================================================*/
bool int_KeyWordTreat(string keyword[], int MaxVal, int Tab[])
{
   bool status=true;
   string sub_name="int_KeyWordTreat";

   int n_zon = -1;
   int n_key = 1;

   while ( keyword[n_key] != "" && n_key < MAX_NB_KEYWORD )
   {
      n_zon ++;
      if ( n_zon >= MaxVal )
      {
         status = false;
         ns_STD::cl_TRC::ERR_Write("CFG", sub_name, "Error keyword '%s': too many values regarding the max number", keyword[0]);
         MessageBoxA(0, "TUBE_MAX_WIDE: Max nb of values too low, the application will be closed", 0, 0);
         return status;
      }

      sscanf_s( keyword[n_key].c_str(), "%d", (int *)&Tab[n_zon] );
      n_key ++;
   }

   return status;
} // void

/*============================================================================
/ STEEL_specific_heat: Calculation of the steel specific heat.
/
/ Revisions:
/  Created:       2004          P.Dubois
/  Modified: 
//===========================================================================*/
float STEEL_specific_heat(float t)
{
   bool status = true;
   string sub_name = "STEEL_specific_heat";

   return CP_STEEL;
} // void
