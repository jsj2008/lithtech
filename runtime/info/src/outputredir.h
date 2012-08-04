

#ifndef __OUTPUTREDIR_H__
#define __OUTPUTREDIR_H__


#ifndef __LTMODULE_H__
#include "ltmodule.h"
#endif


#ifndef __LIBLTINFO_H__
#include "libltinfo.h"
#endif


#include <stdio.h>
#include <stdarg.h>
#include <ltbasedefs.h>


//	-------------------------------------------------------------------------
//	DEFINES
//	-------------------------------------------------------------------------

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#define PRINTBUFFER_SIZE	512
#define SCRATCHBUFFER_SIZE	512
#define PUBLICBUFFER_SIZE	512

// if you change this, you have to go add more OUTPUT_REDIR enums!
#define	MAX_LOG_FILES		5

#endif // DOXYGEN_SHOULD_SKIP_THIS


//	-------------------------------------------------------------------------
//	Output Class
//	-------------------------------------------------------------------------
#ifdef LIBLTINFO_OUTPUT_REDIRECTION

#ifndef DOXYGEN_SHOULD_SKIP_THIS

// this class keeps track of all the output redirection.  Do not instantiate this
// class directly and do not use the member functions.  Instead: use the convenience
// macros so that everything compiles out.  If you don't you'll get unresolved external
// link errors.

class COutputRedir : public ILTOutputRedir
{

	public:

		// interface database	
		declare_interface(COutputRedir);

	public:
		COutputRedir();
		~COutputRedir();

	public:
	
		// defaults
		void ResetToDefaults();

		// redirection
		void EnableOutputDestination(ECOutput_ReDir dest, ECOutput_Status enable);

		// queries
		ECOutput_ReDir GetDestination(ECOutput_Category category, ECOutput_Level level);
		ECOutput_Status GetDestinationStatus(ECOutput_ReDir dest);

		// categories (span all levels)
		void ClearFilterCategories();
		void AddFilterCategory(ECOutput_Category category, ECOutput_ReDir dest);
		void RemoveFilterCategory(ECOutput_Category category);

		// error levels (span all categories)
		void AddFilterLevel(ECOutput_Level level, ECOutput_ReDir dest);
		void RemoveFilterLevel(ECOutput_Level level);

		// categories and levels (choose piecemeal)
		void ClearFilterLevels(ECOutput_Category category);
		void AddFilterLevelInCategory(ECOutput_Category category, ECOutput_Level level, ECOutput_ReDir dest);
		void RemoveFilterLevelInCategory(ECOutput_Category category, ECOutput_Level level);

		// logfile management ( OUTPUT_REDIR_FILE0 <= logfile <= OUTPUT_REDIR_FILE4 )
		bool OpenLogFile(ECOutput_ReDir logfile, const char* pFilename);
		bool CloseLogFile(ECOutput_ReDir logfile);

		// buffer
		char*  GetPublicBuffer() { return m_pPublicBuffer; }
		uint32 GetPublicBufferSize() { return PUBLICBUFFER_SIZE; }

		// output
		void OutputText(ECOutput_Category	category,
					    ECOutput_Level		level,
						const char*			pObjectName,
						const char*			pFilename,
						uint32				lineno,
						uint32				color,
						const char*			pFormatStr,
						...);

		// nesting
		virtual void IncreaseNestingLevel(const ECOutput_ReDir& channel);
		virtual void DecreaseNestingLevel(const ECOutput_ReDir& channel);
		virtual void RootNestingLevel(const ECOutput_ReDir& channel);
		virtual void SetNestingLevel(const ECOutput_ReDir& channel, uint8 level);
		virtual uint8 GetCurrentNestingLevel(const ECOutput_ReDir& channel);
		virtual void SetNestingChar(const ECOutput_ReDir& channel, char nestChar);

		// debugging
		void DumpRedirTable();
		
	private:

		// these functions are defined in system-specific .cpp files
		void OutputToConsole();
		void OutputToCONIO();
		void OutputToIDE();
		void OutputToDLL();
		void OutputToASSERT();		
		void OutputToFile(uint32 index);
		
	private:

		// the lookup table.  every category-level combo maps to a destination
		uint8		m_pFilterTable[OUTPUT_CATEGORY_ALL][OUTPUT_LEVEL_ALL];
		uint8		m_pDestinations[OUTPUT_REDIR_ALL];

		// the output buffer
		char		m_pPrintBuffer[PRINTBUFFER_SIZE];
		char		m_pSecondaryBuffer[SCRATCHBUFFER_SIZE];
		uint32		m_PrintColor;

		// log file handles
		uint32		m_pLogFiles[MAX_LOG_FILES];

		// nesting levels
		uint8		m_NestLevels[OUTPUT_REDIR_ALL];
		char		m_NestChar[OUTPUT_REDIR_ALL];

	public:

		char		m_pPublicBuffer[PUBLICBUFFER_SIZE];

};

#endif // DOXYGEN_SHOULD_SKIP_THIS


#endif // LIBLTINFO_OUTPUT_REDIRECTION


#endif // __OUTPUTREDIR_H__
