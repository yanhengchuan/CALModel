/*============================================================================
/ SRV_Threads.h: All threads declaration.
/ 
/ Revisions:
/  Created:       2010-Jan       X.Fournaud
/  Modified:      2016-Apr       JC.Mitais
/      
/ (c) CMI Industry
//===========================================================================*/
#define TIME_PAUSE_MAIN 2000
#define TIME_PAUSE_SQL 100
#define TIME_PAUSE_HMI 5000
#define TIME_PAUSE_SPG 10000
#define TIME_PAUSE_SIM 900

/*============================================================================
/ Cl_MAIN_Thread: MAIN thread class.
//===========================================================================*/
class Cl_MAIN_Thread
{
public:
	Cl_MAIN_Thread(void);	// standard constructor
	~Cl_MAIN_Thread(void);	// standard destructor
	void Create_MAIN_Thread();
	void MAIN_Thread();
	void Stop_MAIN_Thread();
	
protected:
	HANDLE m_handle_MAIN_Thread;
	static UINT WINAPI MAIN_Proc(LPVOID pParam);
};

/*============================================================================
/ Cl_HMI_Thread: This thread reads periodically the Check_HMI PROCESS table
/     that contains information coming from the HMI.
//===========================================================================*/
class Cl_HMI_Thread
{
public:
	Cl_HMI_Thread(void);	   // standard constructor
	~Cl_HMI_Thread(void);	// standard destructor
	void Create_HMI_Thread();
	void HMI_Thread();
	void Stop_HMI_Thread();
	
protected:
	HANDLE m_handle_HMI_Thread;
	static UINT WINAPI HMI_Proc(LPVOID pParam);
};

/*============================================================================
/ Cl_SQL_Thread: Thread to write in DB.
//===========================================================================*/
class Cl_SQL_Thread
{
public:
	Cl_SQL_Thread(void);	   // standard constructor
	~Cl_SQL_Thread(void);	// standard destructor
	void Create_SQL_Thread();
	void SQL_Thread();
	void Stop_SQL_Thread();
	
protected:
	HANDLE m_handle_SQL_Thread;		// Thread handle
	static UINT WINAPI SQL_Proc(LPVOID pParam);
};

/*============================================================================
/ Cl_SPG_Thread: This thread computes the SPG.
//===========================================================================*/
class Cl_SPG_Thread
{
public:
	Cl_SPG_Thread(void);	   // standard constructor
	~Cl_SPG_Thread(void);	// standard destructor
	void Create_SPG_Thread();
	void SPG_Thread();
	void Stop_SPG_Thread();
	
protected:
	HANDLE m_handle_SPG_Thread;
	static UINT WINAPI SPG_Proc(LPVOID pParam);
};

/*============================================================================
/ Cl_SIM_Thread : Simulation Thread Supervisor
//===========================================================================*/
class Cl_SIM_Thread
{
public:
	Cl_SIM_Thread(void);	   // standard constructor
	~Cl_SIM_Thread(void);	// standard destructor
	void Create_SIM_Thread();
	void SIM_Thread();
	void Stop_SIM_Thread();
	
protected:
	HANDLE m_handle_SIM_Thread;
	static UINT WINAPI SIM_Proc(LPVOID pParam);
};
