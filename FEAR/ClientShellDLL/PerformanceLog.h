//----------------------------------------------------------------------------------
// PerformanceLog.h
//
// This provides the classes and implementation for the performance logging system
// which allows for logging of performance stats and also generating reports and
// monitoring performance information
//
//----------------------------------------------------------------------------------
#ifndef __PERFORMANCELOG_H__
#define __PERFORMANCELOG_H__

#include <string>
#include <vector>
#include <fstream>

//forward declarations
class CFrameRateValue;

//-------------------------------------
// CPerformanceValue
//  This class provides a base interface for all other performance values to implement
// along with some base utility functionality and information tracking
class CPerformanceValue
{
public:

	CPerformanceValue(const char* pszName, const char* pszCategory, const char* pszDescription) :
		m_sName(pszName),
		m_sCategory(pszCategory),
		m_sDescription(pszDescription)
	{}

	virtual ~CPerformanceValue()	{}

	//provides access to the performance value, each implementation must override this accordingly
	virtual double	GetValue() const = 0;

	//the textual name of this performance value
	const char*	GetName() const			{ return m_sName.c_str(); }

	//the category that this performance value belongs to
	const char*	GetCategory() const		{ return m_sCategory.c_str(); }

	//the description for this performance value
	const char* GetDescription() const	{ return m_sDescription.c_str(); }

private:

	//the textual name of this performance value
	std::string		m_sName;

	//the category that this performance value belongs to
	std::string		m_sCategory;

	//the description of this performance value
	std::string		m_sDescription;
};

//-------------------------------------
// CPerformanceValueMonitor
//  This class is responsible for watching a collection of performance values and comparing
// the value against a threshold using a user specified callback to determine if it should
// fire or not
class CPerformanceValueMonitor
{
public:

	//prototype for the callback functions that determine whether or not it should trigger
	typedef bool (*TTriggerCB)(double fAccumulatedValues, double fThreshold);

	CPerformanceValueMonitor();
	~CPerformanceValueMonitor();

	//called to update the performance value monitor by the specified time interval, and also
	//to determine if this monitor should be triggered or not
	bool	Update(double fElapsedTime);

	//called to clear out the list of values that we are tracking
	void	ClearValues();

	//called to add a performance value to the list that should be monitored
	void	AddValue(const CPerformanceValue* pValue);

	//iteration through the currently registered values
	uint32						GetNumValues() const			{ return (uint32)m_Values.size(); }
	const CPerformanceValue*	GetValue(uint32 nValue) const	{ return m_Values[nValue]; }

	//called to set the threshold that should be used for this performance monitor
	void	SetThreshold(double fThreshold)				{ m_fThreshold = fThreshold; }

	//called to set the function for this performance monitor
	void	SetTriggerCB(TTriggerCB pfnCB)				{ m_pfnCB = pfnCB; }

	//called to set the amount of time in seconds that must elapse after a trigger before it is reevaluated 
	//(<0 indicates infinity)
	void	SetTriggerElapse(double fTriggerElapse)		{ m_fTriggerElapse = fTriggerElapse; }

	//the name of this performance value monitor
	std::string			m_sName;

private:

	//our listing of tracked values
	typedef std::vector<const CPerformanceValue*>	TPerformanceValueList;
	TPerformanceValueList	m_Values;

	//our threshold value
	double				m_fThreshold;

	//our trigger elapse value (how much time after a trigger before we can trigger again)
	double				m_fTriggerElapse;

	//how much more time do we have until we can trigger again (<0 = never)
	double				m_fRemainingTriggerElapse;

	//our callback function
	TTriggerCB			m_pfnCB;
};

//-------------------------------------
// CPerformanceValueDB
//  This class provides database management for a listing of performance values
class CPerformanceValueDB
{
public:

	CPerformanceValueDB();
	~CPerformanceValueDB();

	//called to delete and clear out all performance values
	void	Term();

	//called to register a performance value with the database, this database will take ownership
	//of this value and will handle deleting it
	bool	AddPerformanceValue(CPerformanceValue* pPerfValue);

	//called to determine the number of performance values that are currently registered
	uint32	GetNumPerformanceValues() const;

	//called to get the specified performance value by index, this will return NULL if the index is out
	//of range
	const CPerformanceValue*	GetPerformanceValue(uint32 nIndex) const;

	//called to find the performance value by name, this is case insensitive and will return NULL if
	//no match is found
	const CPerformanceValue*	FindPerformanceValue(const char* pszName) const;

private:

	typedef std::vector<CPerformanceValue*>	TPerformanceValueList;
	TPerformanceValueList	m_ValueList;
};

//-------------------------------------
// CPerformanceLog
//  This class is responsible for handling all of the setup of the main performance value database,
// and updating it each frame as well as performing logging, reporting, and monitor updating
class CPerformanceLog
{
public:

	typedef std::vector<CPerformanceValueMonitor*> TMonitorList;

	~CPerformanceLog();

	//singleton support
	static CPerformanceLog&	Singleton();

	//called to initialize the performance values
	void	Init();

	//called to remove all performance values and monitors
	void	Term();

	//called to set the name of the current level, which is used for report information
	void	SetCurrentLevel(const char* pszLevel);

	//called to update all of the performance monitors and logs
	void	Update(double fFrameTimeS);

	//called to obtain the database of performance values for this performance log
	const CPerformanceValueDB&	GetValueDB() const;

	//----------------------------
	// Log support

	//called to start logging the specified listing of performance values. This will close any
	//currently opened logs
	bool	StartLog(const char* pszLogFile, double fSampleFrequencyS, uint32 nNumValues, const CPerformanceValue* const* ppValues);

	//called to stop any currently opened logs
	void	StopLog();

	//called to generate a report to the specified file. This will append the log onto the file if
	//it already exists. An optional listing of performance monitors can be provided in order to 
	//perform coloration of the values that are monitored
	bool	GenerateReport(const char* pszReportFile, const TMonitorList* pMonitorList);

	//called to generate a report based upon the current level
	bool	GenerateReport(const TMonitorList* pMonitorList);

	//----------------------------
	// Monitor support

	//called to register a monitor object with the performance log. This will be updated each frame
	//and when triggered will generate a report. This will take ownership of the monitor.
	bool	AddMonitor(CPerformanceValueMonitor* pMonitor);

	//called to remove all performance monitors with the provided name, this will return whether 
	//or not a performance monitor with that name was found
	bool	RemoveMonitor(const char* pszMonitorName);

private:

	CPerformanceLog();

	//---------------------------------
	// Log support

	//called to update the log based upon the provided time interval
	void	UpdateLog(double fFrameTimeS);

	//---------------------------------
	// Performance Value Creation

	//called to create the common performance values
	void	CreateCommonPerformanceValues();

	//called to create the frame stat performance values
	void	CreateFrameStatPerformanceValues();

	//called to create the memory performance values
	void	CreateMemoryPerformanceValues();

	//called to create the performance monitor performance values
	void	CreatePerfMonPerformanceValues();

	//---------------------------------
	// Report support

	//using the current level name, this will determine the name of the report file that should be used
	void	GetCurrentLevelReportFileName(char* pszBuffer, uint32 nBufferLen);

	//using the current level name and other properties, this will determine the name of the screenshot
	//image that should be used for a report
	void	GetScreenShotFileName(char* pszBuffer, uint32 nBufferLen);

	//given a filename, this will extract the directory out of this file 
	void	ExtractFilePath(const char* pszFilename, char* pszBuffer, uint32 nBufferLen);

	//given a directory, this will make sure that it exists so that files can be created inside of it
	bool	CreateDirectory(const char* pszDirectory);	

	//called to generate the report screenshot, and if successful, write out the screenshot into the
	//table
	void	CreateReportScreenshot(std::ofstream& OutFile, const char* pszDirectory);

	//called to create the base header information for a report, such as the position, build, time, etc
	void	CreateReportHeader(std::ofstream& OutFile);

	//---------------------------------
	// Console Commands
	static void ConsoleCommandHandlerCB(int nArgs, char** ppszArgs);

	//called to handle the provided console command
	void HandleConsoleCommand(uint32 nArgC, const char* const* ppszArgs);

	//called to handle the various commands
	void HandleAddMonitorCommand(uint32 nArgC, const char* const* ppszArgs);
	void HandleRemoveMonitorCommand(uint32 nArgC, const char* const* ppszArgs);
	void HandleLogCommand(uint32 nArgC, const char* const* ppszArgs);
	void HandleStopLogCommand();
	void HandleReportCommand(uint32 nArgC, const char* const* ppszArgs);
	void HandleListCommand(uint32 nArgC, const char* const* ppszArgs);
	void HandleHelpCommand(uint32 nArgC, const char* const* ppszArgs);

	//--------------------------------
	// Help support

	//displays a help title for the provided topic
	void DisplayHelpTitle(const char* pszTopic);

	//given the name of a topic, this will display the associated help, or return false if
	//that topic could not be found
	bool DisplayHelp(const char* pszTopic);

	//called to display the main help menu
	void DisplayMainHelp();


	//the name of the current level
	std::string			m_sCurrentLevel;

	//our listing of performance values
	CPerformanceValueDB	m_ValueDB;

	//our listing of registered performance monitors
	typedef std::vector<CPerformanceValueMonitor*> TMonitorList;
	TMonitorList	m_Monitors;

	//our currently opened log
	std::ofstream	m_LogFile;

	//the listing of the values that we are logging
	typedef std::vector<const CPerformanceValue*> TLogValueList;
	TLogValueList	m_LogValues;

	//the frequency at which we want to log
	double			m_fLogFrequency;

	//the amount of time that has elapsed since our last log
	double			m_fElapsedLogTime;

	//the current log sample
	uint32			m_nCurrLogSample;

	//should we skip the next frame from sampling because we may have altered it by generating a report?
	bool			m_bSkipNextUpdate;

	//our frame rate value that we need to update (note, we don't own this as it is in our value DB)
	CFrameRateValue*	m_pFrameRateValue;
};

#endif
