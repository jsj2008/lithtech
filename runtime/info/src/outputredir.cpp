

#ifndef __LIBLTINFO_H__
#include "libltinfo.h"
#endif


// this whole class in undefined if LT_OUTPUT_REDIRECTION is undefined
#ifdef LIBLTINFO_OUTPUT_REDIRECTION


#ifndef __LTMODULE_H__
#include "ltmodule.h"
#endif

#ifndef __OUTPUTREDIR_H__
#include "outputredir.h"
#endif


// interface database
define_interface(COutputRedir, ILTOutputRedir); 


// having this guy here will make sure that constructor and deconstructor
// get called at startup, shutdown
//static COutputRedir g_COutputRedir;


//	-------------------------------------------------------------------------
COutputRedir::COutputRedir()
{
	// set everything to NULL
	this->ClearFilterCategories();

	// init the logfiles to NULL
	for (int32 i=0; i<MAX_LOG_FILES; i++) {
		m_pLogFiles[i] = 0;
	}

	// enable the defaults
	this->ResetToDefaults();
}


//	-------------------------------------------------------------------------
COutputRedir::~COutputRedir()
{
	// make sure the logfiles get closed
	for (int32 i=0; i<MAX_LOG_FILES; i++) {
		if (m_pLogFiles[i]) {
			CloseLogFile((ECOutput_ReDir)(OUTPUT_REDIR_FILE0 + i));
		}
	}
}


//	-------------------------------------------------------------------------
//	STATIC DATA MEMBERS  												   //
//	-------------------------------------------------------------------------


//	-------------------------------------------------------------------------
//	PUBLIC MEMBER FUNCTIONS												   //
//	-------------------------------------------------------------------------


//	-------------------------------------------------------------------------
void COutputRedir::ResetToDefaults()
{
	// clear all filters
	this->ClearFilterCategories();

	// disable all destinations
	this->EnableOutputDestination(OUTPUT_REDIR_ALL, OUTPUT_STATUS_DISABLED);

	// enable the IDE
	this->EnableOutputDestination(OUTPUT_REDIR_IDE, OUTPUT_STATUS_ENABLED);

	// add all filter categories & levels as IDE
	this->AddFilterCategory(OUTPUT_CATEGORY_ALL, OUTPUT_REDIR_IDE);	

	// reset all nesting levels
	int i;
	for( i = 0; i < OUTPUT_REDIR_ALL; ++i )
	{
		m_NestLevels[i] = 0;
		m_NestChar[i] = '\t';
	}
}


//	-------------------------------------------------------------------------
void COutputRedir::EnableOutputDestination(ECOutput_ReDir dest, ECOutput_Status enable)
{
	// can't set a destination to undefined
	if (enable == OUTPUT_STATUS_UNDEFINED) return;


	if (dest == OUTPUT_REDIR_ALL) {
		// enable all destinations
		memset(m_pDestinations, (uint8)enable, OUTPUT_REDIR_ALL);
	}
	else {
		// enable single destination
		m_pDestinations[dest] = (uint8) enable;
	}
}


//	-------------------------------------------------------------------------
ECOutput_ReDir COutputRedir::GetDestination(ECOutput_Category category, ECOutput_Level level)
{
	// can't query for "all"
	if (category ==	OUTPUT_CATEGORY_ALL) return OUTPUT_REDIR_NULL;

	return (ECOutput_ReDir) m_pFilterTable[category][level];
}


//	-------------------------------------------------------------------------
ECOutput_Status COutputRedir::GetDestinationStatus(ECOutput_ReDir dest)
{
	int32 i;
	int32 count = 0;

	if (dest == OUTPUT_REDIR_ALL) {
		for (i=0; i<OUTPUT_REDIR_ALL; i++) {
			count += m_pDestinations[i];
		}
		if (count == OUTPUT_REDIR_ALL) return OUTPUT_STATUS_ENABLED;
		if (count == 0)				   return OUTPUT_STATUS_DISABLED;
		return OUTPUT_STATUS_UNDEFINED;
	}

	return (ECOutput_Status) m_pDestinations[dest];
}


//	-------------------------------------------------------------------------
void COutputRedir::ClearFilterCategories()
{
	// init the filter table to OUTPUT_REDIR_NULL (zero)
	memset(m_pFilterTable, OUTPUT_REDIR_NULL, 
		   sizeof(uint8) * OUTPUT_CATEGORY_ALL * OUTPUT_LEVEL_ALL);
}


//	-------------------------------------------------------------------------
void COutputRedir::AddFilterCategory(ECOutput_Category category, ECOutput_ReDir dest)
{
	// can't send output to multiple destinations
	if (dest == OUTPUT_REDIR_ALL) dest = OUTPUT_REDIR_NULL;

	if (category == OUTPUT_CATEGORY_ALL) {
		// all categories
		memset(m_pFilterTable, dest, 
		   sizeof(uint8) * OUTPUT_CATEGORY_ALL * OUTPUT_LEVEL_ALL);
	}
	else {
		// a single category
		memset(m_pFilterTable[category], dest, sizeof(uint8) * OUTPUT_LEVEL_ALL);
	}
}


//	-------------------------------------------------------------------------
void COutputRedir::RemoveFilterCategory(ECOutput_Category category)
{
	this->AddFilterCategory(category, OUTPUT_REDIR_NULL);
}


//	-------------------------------------------------------------------------
void COutputRedir::ClearFilterLevels(ECOutput_Category category)
{
	this->RemoveFilterCategory(category);
}


//	-------------------------------------------------------------------------
void COutputRedir::AddFilterLevel(ECOutput_Level level, ECOutput_ReDir dest)
{
	// can't send output to multiple destinations
	if (dest == OUTPUT_REDIR_ALL) dest = OUTPUT_REDIR_NULL;

	if (level == OUTPUT_LEVEL_ALL) {
		// all levels
		memset(m_pFilterTable, dest, 
		   sizeof(uint8) * OUTPUT_CATEGORY_ALL * OUTPUT_LEVEL_ALL);
	}
	else {
		// a single level
		for (int32 cat=0; cat<OUTPUT_CATEGORY_ALL; cat++) {
			m_pFilterTable[cat][level] = dest;
		}
	}
}


//	-------------------------------------------------------------------------
void COutputRedir::RemoveFilterLevel(ECOutput_Level level)
{
	this->AddFilterLevel(level, OUTPUT_REDIR_NULL);
}


//	-------------------------------------------------------------------------
void COutputRedir::AddFilterLevelInCategory(ECOutput_Category category, ECOutput_Level level, ECOutput_ReDir dest)
{
	// can't send output to multiple destinations
	if (dest == OUTPUT_REDIR_ALL) dest = OUTPUT_REDIR_NULL;

	if (category == OUTPUT_CATEGORY_ALL || level == OUTPUT_LEVEL_ALL) {

		// since category and level could be 'all' we might as well run through
		// all of 'em.
		for (int cat=0; cat<OUTPUT_CATEGORY_ALL; cat++) {
			for (int lev=0; lev<OUTPUT_LEVEL_ALL; lev++) {

				if (category == OUTPUT_CATEGORY_ALL || category == cat) {
					if (level == OUTPUT_LEVEL_ALL || level == lev) {
						m_pFilterTable[cat][lev] = dest;
					}
				}
			}
		}
	}
	else {
		m_pFilterTable[category][level] = dest;
	}
}


//	-------------------------------------------------------------------------
void COutputRedir::RemoveFilterLevelInCategory(ECOutput_Category category, ECOutput_Level level)
{
	this->AddFilterLevelInCategory(category, level, OUTPUT_REDIR_NULL);
}


//	-------------------------------------------------------------------------
void COutputRedir::OutputText(ECOutput_Category	category,
					          ECOutput_Level	level,
							  const char*		pObjectName,
							  const char*		pFilename,
							  uint32			lineno,
							  uint32			color,
							  const char*		pFormatStr,
							  ...)
{
	va_list args;

	ECOutput_ReDir dest = (ECOutput_ReDir)m_pFilterTable[category][level];

	// don't do any work if we're redirecting to OUTPUT_REDIR_NULL
	if (dest != OUTPUT_REDIR_NULL) {
		// don't do any work if the destingation isn't enabled
		if (m_pDestinations[dest]) {
			
			// start with an empty buffer
			m_pPrintBuffer[0] = 0;

			// nesting
			const uint8 nestLevel = m_NestLevels[dest];
			if (nestLevel) {
				memset(m_pSecondaryBuffer, m_NestChar[dest], nestLevel);
				m_pSecondaryBuffer[nestLevel] = 0;
				LTStrCat(m_pPrintBuffer, m_pSecondaryBuffer, PRINTBUFFER_SIZE);
			}

			// if there's a filename, add it.  This should be formatted so that
			// in DEVStudio you can click the line and go to the file!
			if (pFilename) {
				sprintf(m_pSecondaryBuffer, "%s(%i) : ", pFilename, lineno); 
				LTStrCat(m_pPrintBuffer, m_pSecondaryBuffer, PRINTBUFFER_SIZE);
			}

			// if there's an object name, add it
			if (pObjectName) {
				sprintf(m_pSecondaryBuffer, "%s: ", pObjectName); 
				LTStrCat(m_pPrintBuffer, m_pSecondaryBuffer, PRINTBUFFER_SIZE);
			}

			// finally, parse and add the va_list
			va_start(args, pFormatStr);
			LTVSNPrintF(m_pSecondaryBuffer, SCRATCHBUFFER_SIZE, pFormatStr, args);
			va_end(args);
			LTStrCat(m_pPrintBuffer, m_pSecondaryBuffer, PRINTBUFFER_SIZE);
		}
		else {
			sprintf(m_pPrintBuffer, "Destination %i NOT ENABLED.\n", dest);
		}
	}

	m_PrintColor = color;

	switch (dest) {

		case OUTPUT_REDIR_CONSOLE:
			this->OutputToConsole();
			break;

		case OUTPUT_REDIR_CONIO:
			this->OutputToCONIO();
			break;

		case OUTPUT_REDIR_IDE:
			this->OutputToIDE();
			break;

		/*
		NOT IMPLEMENTED YET
		case OUTPUT_REDIR_DLL:
			this->OutputToDLL();
			break;
		*/
			
		case OUTPUT_REDIR_ASSERT:
			this->OutputToASSERT();
			break;
			
		case OUTPUT_REDIR_FILE0: // fall through
		case OUTPUT_REDIR_FILE1: // fall through
		case OUTPUT_REDIR_FILE2: // fall through
		case OUTPUT_REDIR_FILE3: // fall through
		case OUTPUT_REDIR_FILE4:
			this->OutputToFile(dest - OUTPUT_REDIR_FILE0);
			break;

		case OUTPUT_REDIR_NULL: // fall through
		default:
			// do nothing!
			break;
	}
}

//	-------------------------------------------------------------------------
void COutputRedir::IncreaseNestingLevel(const ECOutput_ReDir& channel)
{
	if( channel < OUTPUT_REDIR_ALL )
		++(m_NestLevels[channel]);

}

//	-------------------------------------------------------------------------
void COutputRedir::DecreaseNestingLevel(const ECOutput_ReDir& channel)
{
	if( channel < OUTPUT_REDIR_ALL && m_NestLevels[channel] > 0 )
		--(m_NestLevels[channel]);
}

//	-------------------------------------------------------------------------
void COutputRedir::RootNestingLevel(const ECOutput_ReDir& channel)
{
	if( channel < OUTPUT_REDIR_ALL )
		m_NestLevels[channel] = 0;

}
//	-------------------------------------------------------------------------
void COutputRedir::SetNestingLevel(const ECOutput_ReDir& channel, uint8 level)
{
	if( channel < OUTPUT_REDIR_ALL )
		m_NestLevels[channel] = level;
}
//	-------------------------------------------------------------------------
uint8 COutputRedir::GetCurrentNestingLevel(const ECOutput_ReDir& channel)
{
	if( channel < OUTPUT_REDIR_ALL )
		return m_NestLevels[channel];

	return 0;
}

//	-------------------------------------------------------------------------
void COutputRedir::SetNestingChar(const ECOutput_ReDir& channel, char nestChar)
{
	if( channel < OUTPUT_REDIR_ALL )
		m_NestChar[channel] = nestChar;
}


//	-------------------------------------------------------------------------
//	DEBUG
void COutputRedir::DumpRedirTable()
{
	strcpy(m_pPrintBuffer, "\nOutput Redir Table:\n");
	this->OutputToIDE();

	for (int32 i=0; i<OUTPUT_CATEGORY_ALL; i++) {
		
		m_pPrintBuffer[0] = 0;

		for (int32 j=0; j<OUTPUT_LEVEL_ALL; j++) {
			sprintf(m_pSecondaryBuffer, "\t%i", m_pFilterTable[i][j]);
			strcat(m_pPrintBuffer, m_pSecondaryBuffer);			
		}

		strcat(m_pPrintBuffer, "\n");	
		
		this->OutputToIDE();
	}	
}


#endif // LIBLTINFO_OUTPUT_REDIRECTION
