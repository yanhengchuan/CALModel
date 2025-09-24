/*============================================================================
/ SQL.cpp: Communication SQL.
/      
/ (c) CMI Industry
//===========================================================================*/
#include "stdafx.h"
#include "SQL.h"
#include "SRV_Dlg.h"

CRITICAL_SECTION Sec_SQL_Proc0;
CRITICAL_SECTION Sec_SQL_Proc1;
CRITICAL_SECTION Sec_SQL_Proc2;
CRITICAL_SECTION Sec_SQL_Proc3;

/*============================================================================
/ Sql::Sql
/ 
/ Revisions:
/  Created:       2007-Mai       X.Fournaud
/  Modified: 
//===========================================================================*/
Sql::Sql(void)
{
	
}

/*============================================================================
/ Sql::Sql
/ 
/ Revisions:
/  Created:       2007-Mai       X.Fournaud
/  Modified: 
//===========================================================================*/
Sql::Sql(string cnxString, string userName, string password) : m_connectionString(cnxString),
         m_userName(userName), m_password(password)
{
	connect();
}

/*============================================================================
/ _RecordsetPtr Sql::executeQuery
/ 
/ Revisions:
/  Created:       2007-Mai       X.Fournaud
/  Modified: 
//===========================================================================*/
_RecordsetPtr Sql::executeQuery(string query)
{
	_RecordsetPtr pRecordSet;
	try 
	{
      HRESULT hr1 = pRecordSet.CreateInstance(__uuidof(Recordset));
		HRESULT hr2 = pRecordSet->Open(query.c_str(),
				                         pConnection.GetInterfacePtr(),adOpenForwardOnly, adLockReadOnly,adCmdText);
	}

   catch (_com_error &e)
	{
		throw runtime_error( "Impossible to connect to the database: " + e.Description());
	}

	return pRecordSet;
} // void

/*============================================================================
/ void Sql::executeNonQuery
/ 
/ Revisions:
/  Created:       2007-Mai       X.Fournaud
/  Modified: 
//===========================================================================*/
void Sql::executeNonQuery(string query)
{
	_RecordsetPtr pRecordSet;  // recordset object pointer

	try 
	{
      HRESULT hr1 = pRecordSet.CreateInstance(__uuidof(Recordset));

		HRESULT hr2 = pRecordSet->Open(query.c_str(),
				                         pConnection.GetInterfacePtr(),adOpenForwardOnly, adLockReadOnly,adCmdText);

		if( pRecordSet != NULL ) pRecordSet->Close(); 
   }

   catch (_com_error &e)
   {
      MessageBoxA(NULL, e.Description(), NULL, NULL);
	}
} // void

//============================================================================
// CrackStrVariant()  Taken from the CCrack::strVariant method found in the
//      DAOVIEW sample that ships with Visual C++ 4.X/5.0
//  
// Parameters:  var - Reference to an instance of COleVariant.
//
// Returns:     An instance of CString with the value of COleVariant 
//              (if possible).  Otherwise a string containing the type 
//              of data contained by COleVariant.
// 
// Revisions:
//  Created:       2007-Mai       X.Fournaud
//  Modified: 
//===========================================================================
//int variantSize = 0;
DataTypeEnum GetVariantType(const _variant_t&  var)
{
    DataTypeEnum Ret;

    switch(var.vt){
        case VT_EMPTY:
        case VT_NULL:
            Ret = adEmpty;
            break;
        case VT_I2:
            Ret = adSmallInt;
            break;
        case VT_I4:
            Ret = adInteger;
            break;
        case VT_R4:
             Ret = adSingle;
            break;
		case VT_BSTR:
			Ret = adVarChar;
			break;
        case VT_DATE:
           Ret = adDate;
            break;
        case VT_BOOL:
			Ret = adBoolean;
			break;
    }

    return Ret;
} // void

/*============================================================================
/ void Sql::executeNonResultProcedure
/ 
/ Revisions:
/  Created:       2007-Mai       X.Fournaud
/  Modified: 
//===========================================================================*/
void Sql::executeNonResultProcedure(string procedureName)
{
	_CommandPtr pCommand;      // Command object pointer
	_RecordsetPtr pRecordSet;  // recordset object pointer

   EnterCriticalSection(&Sec_SQL_Proc0);	
	try 
	{
		//Create the C++ ADO Command Object
		pCommand.CreateInstance(__uuidof(Command)); 
		pCommand->ActiveConnection = pConnection;

		//Make the ADO C++ command object to accept stored procedure
		pCommand->CommandType = adCmdStoredProc ;

		//Tell the name of the Stored Procedure to the command object
		pCommand->CommandText = procedureName.c_str(); 

		pCommand->Execute( NULL, NULL, adCmdStoredProc ); 
	} // try

	catch (_com_error &e)
	{
      MessageBoxA(NULL, e.Description(), procedureName.c_str(), NULL);
	} // catch

   LeaveCriticalSection(&Sec_SQL_Proc0);
} // void

/*============================================================================
/ void Sql::executeNonResultProcedure
/ 
/ Revisions:
/  Created:       2007-Mai       X.Fournaud
/  Modified: 
//===========================================================================*/
void Sql::executeNonResultProcedure(string procedureName, vector<VARIANT>const& variantArray, 
                                    vector<string>const& parameters, vector<int>const& sizeArray )
{
	_CommandPtr pCommand;      // Command object pointer
	_RecordsetPtr pRecordSet;  // recordset object pointer

	DataTypeEnum adType;
   EnterCriticalSection(&Sec_SQL_Proc1);	
	try 
	{
		//Create the C++ ADO Command Object
		pCommand.CreateInstance(__uuidof(Command)); 
		pCommand->ActiveConnection = pConnection;

		//Make the ADO C++ command object to accept stored procedure
		pCommand->CommandType = adCmdStoredProc ;
      pCommand->NamedParameters = true;

		//Tell the name of the Stored Procedure to the command object
		pCommand->CommandText = procedureName.c_str(); 

		for( int i=0; i < (int)variantArray.size(); i++ )
		{
			adType = GetVariantType( variantArray[i] );
			pCommand->Parameters->Append( pCommand->CreateParameter( parameters[i].c_str(), adType, adParamInput, sizeArray[i], variantArray[i] ) );
		}

		pCommand->Execute( NULL, NULL, adCmdStoredProc ); 
	} // try

	catch (_com_error &e)
	{
      MessageBoxA(NULL, e.Description(), procedureName.c_str(), NULL);
	} // catch

   LeaveCriticalSection(&Sec_SQL_Proc1);
} // void

/*============================================================================
/ _RecordsetPtr Sql::executeResultProcedure
/ 
/ Revisions:
/  Created:       2007-Mai       X.Fournaud
/  Modified: 
//===========================================================================*/
_RecordsetPtr Sql::executeResultProcedure(string procedureName, vector<VARIANT>const& variantArray, 
                                          vector<string>const& parameters, vector<int>const& sizeArray)
{
	_CommandPtr pCommand;      // Command object pointer
	_RecordsetPtr pRecordSet;  // recordset object pointer

	DataTypeEnum adType;
   EnterCriticalSection(&Sec_SQL_Proc2);	
	try 
	{
		//Create the C++ ADO Command Object
		pCommand.CreateInstance(__uuidof(Command)); 
		pCommand->ActiveConnection = pConnection;

		//Make the ADO C++ command object to accept stored procedure
		pCommand->CommandType = adCmdStoredProc ;
      pCommand->NamedParameters = true;

		//Tell the name of the Stored Procedure to the command object
		pCommand->CommandText = procedureName.c_str(); 

		for( int i=0; i < (int)variantArray.size(); i++ )
		{
			adType = GetVariantType( variantArray[i] );
			pCommand->Parameters->Append( pCommand->CreateParameter( parameters[i].c_str(), adType, adParamInput, sizeArray[i], variantArray[i] ) );
		}
		pRecordSet = pCommand->Execute( NULL, NULL, adCmdStoredProc ); 
	}

	catch (_com_error &e)
	{
      MessageBoxA(NULL, e.Description(), procedureName.c_str(), NULL);
	}

   LeaveCriticalSection(&Sec_SQL_Proc2);
	return pRecordSet;
} // void

/*============================================================================
/ _RecordsetPtr Sql::executeResultProcedure
/ 
/ Revisions:
/  Created:       2007-Mai       X.Fournaud
/  Modified: 
//===========================================================================*/
_RecordsetPtr Sql::executeResultProcedure(string procedureName)
{
	_CommandPtr pCommand;      // Command object pointer
	_RecordsetPtr pRecordSet;  // recordset object pointer

   EnterCriticalSection(&Sec_SQL_Proc3);	
	try 
	{
		//Create the C++ ADO Command Object
		pCommand.CreateInstance(__uuidof(Command)); 
		pCommand->ActiveConnection = pConnection;

		//Make the ADO C++ command object to accept stored procedure
		pCommand->CommandType = adCmdStoredProc ;

		//Tell the name of the Stored Procedure to the command object
		pCommand->CommandText = procedureName.c_str(); 

		pRecordSet = pCommand->Execute( NULL, NULL, adCmdStoredProc ); 
	}
	catch (_com_error &e)
	{
      MessageBoxA(NULL, e.Description(), procedureName.c_str(), NULL);
	}

   LeaveCriticalSection(&Sec_SQL_Proc3);
	return pRecordSet;
} // void

/*============================================================================
/ void Sql::FloatProcArray: Store a float in a parameters array for SQL Stored 
/     procedures purpose.
/ 
/ Revisions:
/  Created:       2007-Mai       X.Fournaud
/  Modified: 
//===========================================================================*/
void Sql::FloatProcArray(struct str_PrmArrays *PrmArr, string DBName, float floatVal)
{
   V_VT(&PrmArr->var) = VT_R4;   
   PrmArr->var.fltVal = floatVal;     
   PrmArr->vtArr.push_back(PrmArr->var); 
   PrmArr->sizArr.push_back(L_R);  
   PrmArr->prmArr.push_back("@" + DBName); 

   return;
}

/*============================================================================
/ void Sql::IntProcArray: Store an integer in a parameters array for SQL Stored 
/     procedures purpose.
/ 
/ Revisions:
/  Created:       2007-Mai       X.Fournaud
/  Modified: 
//===========================================================================*/
void Sql::IntProcArray(struct str_PrmArrays *PrmArr, string DBName, int intVal)
{
   V_VT(&PrmArr->var) = VT_I4;   
   PrmArr->var.intVal = intVal;     
   PrmArr->vtArr.push_back(PrmArr->var); 
   PrmArr->sizArr.push_back(L_I);  
   PrmArr->prmArr.push_back("@" + DBName); 

   return;
}

/*============================================================================
/ void Sql::BoolProcArray: Store a boolean in a parameters array for SQL Stored 
/     procedures purpose.
/ 
/ Revisions:
/  Created:       2007-Mai       X.Fournaud
/  Modified: 
//===========================================================================*/
void Sql::BoolProcArray(struct str_PrmArrays *PrmArr, string DBName, bool boolVal)
{
   V_VT(&PrmArr->var) = VT_BOOL;   
   PrmArr->var.boolVal = boolVal;     
   PrmArr->vtArr.push_back(PrmArr->var); 
   PrmArr->sizArr.push_back(L_B);  
   PrmArr->prmArr.push_back("@" + DBName); 
}

/*============================================================================
/ void Sql::StringProcArray: Store a string in a parameters array for SQL Stored 
/     procedures purpose.
/ 
/ Revisions:
/  Created:       2007-Mai       X.Fournaud
/  Modified: 
//===========================================================================*/
void Sql::StringProcArray(struct str_PrmArrays *PrmArr, string DBName, string stringVal)
{
   string loc_string = stringVal;
   if (loc_string == "") loc_string = " ";
   V_VT(&PrmArr->var) = VT_BSTR;  
   V_BSTR(&PrmArr->var) = CComBSTR(loc_string.c_str()).Detach(); 
   PrmArr->vtArr.push_back(PrmArr->var); 
   PrmArr->sizArr.push_back(loc_string.size()); 
   PrmArr->prmArr.push_back("@" + DBName);
}

/*============================================================================
/ void Sql::DateProcArray: Store a time_t in a parameters array for SQL Stored 
/     procedures purpose.
/ 
/ Revisions:
/  Created:       2007-Mai       X.Fournaud
/  Modified: 
//===========================================================================*/
void Sql::DateProcArray(struct str_PrmArrays *PrmArr, string DBName, time_t timeVal)
{
   struct tm newtime;

   localtime_s( &newtime, &timeVal );   // Convert time to struct tm form
   COleDateTime vOleDOB0(newtime.tm_year+1900, newtime.tm_mon+1, newtime.tm_mday, newtime.tm_hour, newtime.tm_min, newtime.tm_sec);
   V_VT(&PrmArr->var)=VT_DATE; 
   PrmArr->var.date = vOleDOB0; 
   PrmArr->vtArr.push_back(PrmArr->var); 
   PrmArr->sizArr.push_back(8); 
   PrmArr->prmArr.push_back("@" + DBName);
}

void Sql::Date2ProcArray(struct str_PrmArrays *PrmArr, string DBName, time_t timeVal, string DBName2, time_t timeVal2)
{
   struct tm newtime;
   struct tm newtime2;

   localtime_s( &newtime, &timeVal );   // Convert time to struct tm form
   localtime_s( &newtime2, &timeVal2 );   // Convert time to struct tm form
  
   COleDateTime vOleDOB0(newtime.tm_year+1900, newtime.tm_mon+1, newtime.tm_mday, newtime.tm_hour, newtime.tm_min, newtime.tm_sec);
   V_VT(&PrmArr->var)=VT_DATE; 

   PrmArr->var.date = vOleDOB0; 
   PrmArr->vtArr.push_back(PrmArr->var); 
   PrmArr->sizArr.push_back(8); 
   PrmArr->prmArr.push_back("@" + DBName);
      
   COleDateTime vOleDOB1(newtime2.tm_year+1900, newtime2.tm_mon+1, newtime2.tm_mday, newtime2.tm_hour, newtime2.tm_min, newtime2.tm_sec);
   V_VT(&PrmArr->var)=VT_DATE;
   PrmArr->var.date = vOleDOB1; 
   PrmArr->vtArr.push_back(PrmArr->var); 
   PrmArr->sizArr.push_back(8); 

   PrmArr->prmArr.push_back(" ,@" + DBName2);
}


/*============================================================================
/ void Sql::connect
/ 
/ Revisions:
/  Created:       2007-Mai       X.Fournaud
/  Modified: 
//===========================================================================*/
void Sql::connect()
{
   HRESULT hr;

	try 
	{
		//CoInitialize(NULL);
		CoInitializeEx(NULL, COINIT_MULTITHREADED);
		hr = pConnection.CreateInstance(__uuidof(Connection));

		// connection
		pConnection->CursorLocation = adUseClient;
		hr = pConnection->Open(m_connectionString.c_str(),m_userName.c_str() ,m_password.c_str(), -1);
	   AddMsgToWindow("SQL connection successful.");
	} // try

	catch (_com_error &e)
	{
      MessageBoxA(NULL, e.Description(), m_connectionString.c_str(), NULL);
	   AddMsgToWindow("Main Thread stopped: No SQL connection.");
      ExitProcess(1);
   }
} // void

/*============================================================================
/ Sql::~Sql
/ 
/ Revisions:
/  Created:       2007-Mai       X.Fournaud
/  Modified: 
//===========================================================================*/
Sql::~Sql(void)
{
	// Close the database connection
	pConnection->Close();

	// Uninitialize COM        
	CoUninitialize();
} // void
