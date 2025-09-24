/*=============================================================================
/ SQL_Server.cpp: SQL Server.
/ 
/ (c) CMI Industry
//=============================================================================*/
#include "stdafx.h"
#include "SQL.h"

Sql *m_sql1;
Sql *m_sql2;

/*=============================================================================
/ SQL_ConnectServer: Treatment of data received/send from/to HMI. 
/ 
/ Revisions:
/  Created:          2007-oct       X.Fournaud
/  Modified:         2008-Dec       JC.Mitais
//=============================================================================*/
void SQL_ConnectServer()
{	
	try
	{
      string user = "HMI_Admin";
      string mdp = "cmi";

	   string serverPRO = (string)SQL_ConnectionPRO + (string)BD_PROCESS + ";";
      m_sql1  = new Sql(serverPRO, user, mdp);

	   string serverCOM = (string)SQL_ConnectionCOM + (string)BD_COMMUNICATIONS + ";";
      m_sql2  = new Sql(serverCOM, user, mdp);
	}

	catch(_com_error &e)
	{
		//std::cout << "Impossible de se connecter au serveur distant";
		MessageBoxA(0, (LPCSTR) e.Description(), 0, 0);
	}

	//catch(...)
	//{
	//	MessageBoxA(0, "Connection DB  impossible", 0, 0);
	//};  

	return;
}

/*=============================================================================
/ SQL_R_ERR: Read error treatment 
/ 
/ Revisions:
/  Created:          2008-Dec       JC.Mitais
/  Modified:         
//=============================================================================*/
void SQL_R_ERR(string sub_name, string Procedure, string Description)
{
   string AlmTxt = sub_name + ": " + Procedure + ": " + Description;

   ns_STD::cl_TRC::ERR_Write("SQL",  sub_name, "ERROR in %s Procedure: %s",Procedure.c_str(), Description.c_str());
   MessageBoxA(0, LPCSTR(Description.c_str()), LPCSTR(sub_name.c_str()), 0);

   return;
}

