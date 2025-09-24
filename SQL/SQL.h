/*============================================================================
/ SQL.cpp: Communication SQL.
/      
/ (c) CMI Industry
//===========================================================================*/
#import "C:\\Program Files (x86)\\Common Files\\System\\ado\\msado15.dll" no_namespace rename("EOF","ADOEOF") // Windows 7 
//#import "C:\Program Files\Common Files\System\ado\msado15.dll" no_namespace rename("EOF","ADOEOF") // Windows XP

extern CRITICAL_SECTION Sec_SQL_Proc0;
extern CRITICAL_SECTION Sec_SQL_Proc1;
extern CRITICAL_SECTION Sec_SQL_Proc2;
extern CRITICAL_SECTION Sec_SQL_Proc3;

struct str_PrmArrays
{
   VARIANT var; 
   vector<VARIANT> vtArr; 
   vector<string> prmArr; 
   vector<int> sizArr;
}; // struct

class Sql
{
public:
	Sql(void);
	Sql(string connectionString, string userName, string password);
	~Sql(void);

   void FloatProcArray(struct str_PrmArrays *PrmArr, string DBName, float floatVal);
   void IntProcArray(struct str_PrmArrays *PrmArr, string DBName, int intVal);
   void BoolProcArray(struct str_PrmArrays *PrmArr, string DBName, bool boolVal);
   void StringProcArray(struct str_PrmArrays *PrmArr, string DBName, string stringVal);
   void DateProcArray(struct str_PrmArrays *PrmArr, string DBName, time_t timeVal);
   void Date2ProcArray(struct str_PrmArrays *PrmArr, string DBName, time_t timeVal, string DBName2, time_t timeVal2);

	_RecordsetPtr executeQuery(string query);
	void executeNonQuery(string query);
	_RecordsetPtr executeResultProcedure(string procedureName);
	_RecordsetPtr executeResultProcedure(string procedureName, vector<VARIANT>const& variantArray, vector<string>const& parameters, vector<int>const& sizeArray);
	void executeNonResultProcedure(string procedureName);
	void executeNonResultProcedure(string procedureName,vector<VARIANT>const& variantArray,vector<string>const& parameters, vector<int>const& sizeArray);

	void connect();
	//void connect(string connectionString, string userName, string password);
	void disconnect();

private:
	string const m_moduleName;
	string const m_methodName;

	string const m_connectionString;
	string const m_userName;
	string const m_password; 

	_ConnectionPtr pConnection; // connection object pointer
};

extern void SQL_ConnectServer(); // Connection to the SQL database server


