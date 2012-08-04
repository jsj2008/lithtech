

#ifndef __ILTOUTPUTREDIR_H__
#define __ILTOUTPUTREDIR_H__


#ifndef __LIBLTINFO_H__
#include "libltinfo.h" // for #defines
#endif

#ifndef __LTMODULE_H__
#include "ltmodule.h"
#endif

#include <stdio.h>
#include <stdarg.h>
#include <ltbasedefs.h>


//	-------------------------------------------------------------------------
//	ENUMS
//	-------------------------------------------------------------------------


/*!
Enumerates the destinations to which text output can be redirected.

Used for: Output.   
*/
typedef enum {
/*!
Output will not be processed.
*/
	OUTPUT_REDIR_NULL = 0,
/*!
Output will be sent to the in-game console.
*/
	OUTPUT_REDIR_CONSOLE = 1,
/*!
Output will be sent to printf() or the system equivalent.
*/
	OUTPUT_REDIR_CONIO = 2,
/*!
Output will be sent to your IDE.
*/
	OUTPUT_REDIR_IDE = 3,
/*!
Output will be sent to the assert() dialog.
*/
	OUTPUT_REDIR_ASSERT = 4,
/*!
Output will be sent to log file 0.
*/
	OUTPUT_REDIR_FILE0 = 5,
/*!
Output will be sent to log file 1.
*/
	OUTPUT_REDIR_FILE1 = 6,
/*!
Output will be sent to log file 2.
*/
	OUTPUT_REDIR_FILE2 = 7,
/*!
Output will be sent to log file 3.
*/
	OUTPUT_REDIR_FILE3 = 8,
/*!
Output will be sent to log file 4.
*/
	OUTPUT_REDIR_FILE4 = 9,
/*!
Represents all possible destinations.  A message can only be sent to one destination.
*/
	OUTPUT_REDIR_ALL = 10
}	
ECOutput_ReDir;


/*!
Enumerates the categories that errors and warnings can be divided into.

Used for: Output.   
*/
typedef enum {
/*!
No Categories.
*/
	OUTPUT_CATEGORY_NONE = 0,
/*!
General informational messages.
*/
	OUTPUT_CATEGORY_GENERAL = 1,
/*!
General errors.
*/
	OUTPUT_CATEGORY_ERROR = 2,
/*!
Messages from or about the I/O system (hard disk, keyboard & mouse).
*/
	OUTPUT_CATEGORY_IO = 3,
/*!
Messages from or about the Network.
*/
	OUTPUT_CATEGORY_NETWORK = 4,
/*!
Messages from or about the Renderer.
*/
	OUTPUT_CATEGORY_RENDERER = 5,
/*!
Messages from or about the Physics Library.
*/
	OUTPUT_CATEGORY_PHYSICS = 6,
/*!
Messages from or about the UI Library.
*/
	OUTPUT_CATEGORY_UI = 7,
/*!
Messages from or about the Client.
*/
	OUTPUT_CATEGORY_CLIENT = 8,
/*!
Messages from or about the Server.
*/
	OUTPUT_CATEGORY_SERVER = 9,
/*!
Messages from or about ESD.
*/
	OUTPUT_CATEGORY_ESD = 10,
/*!
Messages from or about MMP.
*/
	OUTPUT_CATEGORY_MMP = 11,
/*!
Messages from or about the Distributed Objects System - executive operations
*/
	OUTPUT_CATEGORY_DOBSYS_EXEC = 12,
/*!
Messages from or about the Distributed Objects System - view operations
*/
	OUTPUT_CATEGORY_DOBSYS_VIEW = 13,
/*!
Messages from or about the Distributed Objects System - monitor view operations
*/
	OUTPUT_CATEGORY_DOBSYS_MONITOR = 14,
/*!
Messages from or about the Distributed Objects System - admin control view operations
*/
	OUTPUT_CATEGORY_DOBSYS_ACV = 15,
/*!
Messages from or about the Distributed Objects System - ltobject related
*/
	OUTPUT_CATEGORY_DOBSYS_LTOBJECT = 16,

/*!
Messages about HOBJECT id allocation and freeing (client and server both)
*/
	OUTPUT_CATEGORY_HOBJECT_ID_ACTIVITY = 17,
/*!
Messages from or about Custom System 0.
*/
	OUTPUT_CATEGORY_CUSTOM0 = 18,
/*!
Messages from or about Custom System 1.
*/
	OUTPUT_CATEGORY_CUSTOM1 = 19,
/*!
Messages from or about Custom System 2.
*/
	OUTPUT_CATEGORY_CUSTOM2 = 20,
/*!
Messages from or about Custom System 3.
*/
	OUTPUT_CATEGORY_CUSTOM3 = 21,
/*!
Messages from or about Custom System 4.
*/
	OUTPUT_CATEGORY_CUSTOM4 = 22,
/*!
Messages in all categories.
*/
	OUTPUT_CATEGORY_ALL = 23
}	
ECOutput_Category;


/*!
Enumerates the error levels that the Output System will track.  Use these
as guidelines when detemining the severity of your output.

Used for: Output.   
*/
typedef enum {
/*!
This is a marker which represents no error levels.
*/
	OUTPUT_LEVEL_NONE = 0,
/*!
An event that has no effect on the program.  It is purely
informational.
*/
	OUTPUT_LEVEL_INFO = 1,
/*!
An event that has no effect on the program, but is nonetheless
a cause for concern.
*/
	OUTPUT_LEVEL_WARNING = 2,
/*!
An error has occurred.  The program should recover with no lasting effects.
*/
	OUTPUT_LEVEL_ERROR_MINOR = 3,
/*!
An error has occurred.  The program can potentially recover, but with decreased
stability or unpredictable behavior.
*/
	OUTPUT_LEVEL_ERROR_MAJOR = 4,
/*!
An error has occurred.  The program cannot continue.
*/
	OUTPUT_LEVEL_ERROR_CRITICAL = 5,
/*!
This is a marker which represents all error levels.
*/
	OUTPUT_LEVEL_ALL = 6
}	
ECOutput_Level;


/*!
Enumerates the status (enabled or disabled) that apply to Output Destinations.
\see ECOutput_ReDir
Used for: Output.   
*/
typedef enum {
/*!
The output destination is disabled.
*/
	OUTPUT_STATUS_DISABLED = 0,
/*!
The output destination is enabled.
*/
	OUTPUT_STATUS_ENABLED = 1,
/*!
The status of the destination cannot be determined.  This can happen if you try
to query the status of OUTPUT_REDIR_ALL when some destinations are enabled and 
others are not.
*/
	OUTPUT_STATUS_UNDEFINED = 2
}
ECOutput_Status;


/*!
ILTOutputRedir.

This interface keeps track of all output redirection.  It is available through the
interface manager when \b LIBLTINFO_OUTPUT_REDIRECTION is defined in \engine\sdk\inc\libltinfo.h.

ILTOutputRedir allows you to redirect text output to any destination listed in
ECOutput_ReDir.  There are several convenience macros that allow you to send 
messages without specifying extra information, or you can use the LTDBG_OUT
macros for more customized access.

All of the macros require a pointer to the output redir interface as their first
(or only) parameter.  When \b LIBLTINFO_OUTPUT_REDIRECTION is not defined, the
interface manager will return a NULL pointer when asked to retrieve the
output redir interface.  This is normal:  the pointer will never be accessed
by the macros when \b LIBLTINFO_OUTPUT_REDIRECTION is not defined.

Do not call the interface's member functions directly.  Instead: use the convenience
macros so that everything can compiles out if desired.  If you don't you'll get 
unresolved external linker errors.

Some macros end with a bunch of letters.  These are to hint at the parameters that are
required.  Arguments are according to the following key:

	\b C	=	Category
	\b L	=	Level
	\b O	=	Object Name
	\b F	=	File Name
	\b N	=	Line Number
	\b X	=	Color
	\b S	=	String (no variable arguments)
	\b VA	=	Variable Arguments  

Any time \e Category and \e Level are not specified or inferred byt the name of
the macro, it is assumed that they will be \b OUTPUT_CATEGORY_GENERAL and 
\b OUTPUT_LEVEL_INFO, respectively.

You may add to these macros as you need them, but REMEMBER TO ADD EMPTY
\b #defines FOR WHEN \b LT_OUTPUT_REDIRECTION IS NOT DEFINED!

Used for: Output.
*/

class ILTOutputRedir : public IBase
{

#ifndef DOXYGEN_SHOULD_SKIP_THIS

	public:
	
		// add ILTOutputRedir to the interface database	
		interface_version(ILTOutputRedir, 0);

		// virtual destructor
		virtual ~ILTOutputRedir() {};


		// defaults
		virtual void	ResetToDefaults() = 0;

		// redirection
		virtual void 	EnableOutputDestination(ECOutput_ReDir dest, ECOutput_Status enable) = 0;

		// queries
		virtual ECOutput_ReDir GetDestination(ECOutput_Category category, ECOutput_Level level) = 0;
		virtual ECOutput_Status GetDestinationStatus(ECOutput_ReDir dest) = 0;

		// categories (span all levels)
		virtual void 	ClearFilterCategories() = 0;
		virtual void 	AddFilterCategory(ECOutput_Category category, ECOutput_ReDir dest) = 0;
		virtual void 	RemoveFilterCategory(ECOutput_Category category) = 0;

		// error levels (span all categories)
		virtual void 	AddFilterLevel(ECOutput_Level level, ECOutput_ReDir dest) = 0;
		virtual void 	RemoveFilterLevel(ECOutput_Level level) = 0;

		// categories and levels (choose piecemeal)
		virtual void 	ClearFilterLevels(ECOutput_Category category) = 0;
		virtual void 	AddFilterLevelInCategory(ECOutput_Category category, ECOutput_Level level, ECOutput_ReDir dest) = 0;
		virtual void 	RemoveFilterLevelInCategory(ECOutput_Category category, ECOutput_Level level) = 0;

		// logfile management ( OUTPUT_REDIR_FILE0 <= logfile <= OUTPUT_REDIR_FILE4 )
		virtual bool 	OpenLogFile(ECOutput_ReDir logfile, const char* pFilename) = 0;
		virtual bool 	CloseLogFile(ECOutput_ReDir logfile) = 0;

		// buffer
		virtual char*	GetPublicBuffer() = 0;
		virtual uint32	GetPublicBufferSize() = 0;
		
		// output
		virtual void OutputText(ECOutput_Category	category,
							    ECOutput_Level		level,
							    const char*			pObjectName,
							    const char*			pFileName,
							    uint32				lineno,
							    uint32				color,
							    const char*			pFormatStr,
							    ...) = 0;
		// nesting levels
		virtual void IncreaseNestingLevel(const ECOutput_ReDir& channel) = 0;
		virtual void DecreaseNestingLevel(const ECOutput_ReDir& channel) = 0;
		virtual void RootNestingLevel(const ECOutput_ReDir& channel) = 0;
		virtual void SetNestingLevel(const ECOutput_ReDir& channel, uint8 level) = 0;
		virtual uint8 GetCurrentNestingLevel(const ECOutput_ReDir& channel) = 0;
		virtual void SetNestingChar(const ECOutput_ReDir& channel, char nestChar) = 0;
	
	// debugging
		virtual void DumpRedirTable() = 0;	

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

};


// smart nesting guy
// similar to a smart pointer which decrements a ref count when it goes out of scope,
// this little guy stores the current nesting level then restores it when it is destructed
#ifndef DOXYGEN_SHOULD_SKIP_THIS
class LTOutputSmartNestRestorer
{
public:
	LTOutputSmartNestRestorer(ILTOutputRedir* pMgr, const ECOutput_ReDir& channel) 
				: m_Channel(channel),
				  m_pMgr(pMgr)
	{
		m_InitialLevel = pMgr->GetCurrentNestingLevel(channel);
	}

	virtual ~LTOutputSmartNestRestorer()
	{
		ASSERT(m_pMgr);
		m_pMgr->SetNestingLevel(m_Channel, m_InitialLevel);
	}
private:
	uint8			m_InitialLevel;
	ECOutput_ReDir	m_Channel;
	ILTOutputRedir*	m_pMgr;
};
#endif //#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef LIBLTINFO_OUTPUT_REDIRECTION


//	-------------------------------------------------------------------------
//	Convenience Functions & Macros
//	-------------------------------------------------------------------------

// these are macros and c-style functions that allow easier calling of
// the main Output::OutputText() function.  Please feel free to add to 
// this list if you need a more convenient style.  If you add a function 
// to this list, add a corresponding #define below so that it will resolve 
// to nothing when Output Redirection is compiled out.


// SETUP/MAINTENNANCE macros

/*!
\param i ILTOutputRedir interface pointer
Resets all output redirection to the system default
Used for: Output.
*/ 
#define LTDBG_OUT_RESET(i)							(i)->ResetToDefaults()

/*!
\param i ILTOutputRedir interface pointer
\param d destination
Enables an output destination.
\see ECOutput_ReDir
Used for: Output.
*/ 
#define LTDBG_OUT_ENABLE_DESTINATION(i,d)			(i)->EnableOutputDestination(d, OUTPUT_STATUS_ENABLED)

/*!
\param i ILTOutputRedir interface pointer
\param d destination
Disables an output destination.
\see ECOutput_ReDir
Used for: Output.
*/ 
#define LTDBG_OUT_DISABLE_DESTINATION(i,d)			(i)->EnableOutputDestination(d, OUTPUT_STATUS_DISABLED)

/*!
\param i ILTOutputRedir interface pointer
\param d destination
\param f filename
Opens \e filename for log messages sent to \e destination.  \e destination must be one
of \b OUTPUT_REDIR_FILE0 through \b OUTPUT_REDIR_FILE4.
\see ECOutput_ReDir
Used for: Output.
*/ 
#define LTDBG_OUT_OPEN_LOGFILE(i,d,f)				(i)->OpenLogFile(d,f)

/*!
\param i ILTOutputRedir interface pointer
\param d destination
Closes the logfile associated with \e destination.  \e destination must be one
of \b OUTPUT_REDIR_FILE0 through \b OUTPUT_REDIR_FILE4.
\see ECOutput_ReDir
Used for: Output.
*/ 
#define LTDBG_OUT_CLOSE_LOGFILE(i,d)				(i)->CloseLogFile(d)

/*!
\param i ILTOutputRedir interface pointer
\param c category
\param l level
\return output destination
Returns the ECOutput_ReDir destination associated with \e category and \e level.
\see ECOutput_Category
\see ECOutput_Level
\see ECOutput_ReDir
Used for: Output.
*/ 
#define LTDBG_OUT_GET_DESTINATION(i,c,l)			(i)->GetDestination(c,l)

/*!
\param i ILTOutputRedir interface pointer
\param d destination
\return true or false
Returns whether \x destination is enabled or not.  Sending output to a disabled destination is
the same as sending it to \b OUTPUT_REDIR_NULL.
\see ECOutput_ReDir
Used for: Output.
*/ 
#define LTDBG_OUT_DESTINATION_STATUS(i,d)			(i)->GetDestinationStatus(d)

/*!
\param i ILTOutputRedir interface pointer
\param l level
\param d destination
Adds filter \e level to all categories, directed to \e destination.
\see ECOutput_Level
\see ECOutput_ReDir
Used for: Output.
*/ 
#define LTDBG_OUT_ADD_LEVEL(i,l,d)					(i)->AddFilterLevel(l, d)

/*!
\param i ILTOutputRedir interface pointer
\param l level
Removes filter \e level from all categories.
\see ECOutput_Level
Used for: Output.
*/ 
#define LTDBG_OUT_REMOVE_LEVEL(i,l)				(i)->RemoveFilterLevel(l)

/*!
\param i ILTOutputRedir interface pointer
\param c category
Removes all filter levels from \e category.  Currently the same as OUTPUT_REMOVE_CATEGORY.
\see ECOutput_Category
Used for: Output.
*/ 
#define LTDBG_OUT_CLEAR_LEVELS(i,c)				(i)->ClearFilterLevels(c)

/*!
\param i ILTOutputRedir interface pointer
\param c category
\param d destination
Adds filters at every level of \e category, directed to \e destination.
\see ECOutput_Category
\see ECOutput_ReDir
Used for: Output.
*/ 
#define LTDBG_OUT_ADD_CATEGORY(i,c,d)				(i)->AddFilterCategory(c, d)

/*!
\param i ILTOutputRedir interface pointer
\param c category
Removes all filter levels from \e category.  Currently the same as OUTPUT_CLEAR_LEVELS.
\see ECOutput_Category
Used for: Output.
*/ 
#define LTDBG_OUT_REMOVE_CATEGORY(i,c)				(i)->RemoveFilterCategory(c)

/*!
\param i ILTOutputRedir interface pointer
Removes all filters at every level of every category.
Used for: Output.
*/ 
#define LTDBG_OUT_CLEAR_CATEGORIES(i)				(i)->ClearFilterCategories()

/*!
\param i ILTOutputRedir interface pointer
\param c category
\param l level
\param d destination
Adds a filter at \e level of \e category, directed to \e destination.
\see ECOutput_Category
\see ECOutput_Level
\see ECOutput_ReDir
Used for: Output.
*/ 
#define LTDBG_OUT_ADD_FILTER_IN_CATEGORY(i,c,l,d)	(i)->AddFilterLevelInCategory(c,l,d)

/*!
\param i ILTOutputRedir interface pointer
\param c category
\param l level
Removes the filter at \e level of \e category.
\see ECOutput_Category
\see ECOutput_Level
Used for: Output.
*/ 
#define LTDBG_OUT_REMOVE_FILTER_IN_CATEGORY(i,c,l)	(i)->RemoveFilterLevelInCategory(c,l)

/*!
\param i ILTOutputRedir interface pointer
\param d destination
Declares a "smart nester" object which stores the current nesting level
for \e destination.  When this object goes out of scope, it will restore
the original nesting level.

\note You can only use this macro once within the a scope.
*/
#define LTDBG_SMART_NESTING(i, d)					LTOutputSmartNestRestorer z_SmartNester(i, d)

/*!
\param i ILTOutputRedir interface pointer
\param d destination
Increase nesting of \e destination by one level.
*/
#define LTDBG_INCREASE_NESTING(i, d)				(i)->IncreaseNestingLevel(d)

/*!
\param i ILTOutputRedir interface pointer
\param d destination
Decrease nesting of \e destination by one level.
*/
#define LTDBG_DECREASE_NESTING(i, d)				(i)->DecreaseNestingLevel(d)

/*!
\param i ILTOutputRedir interface pointer
\param d destination
Reset nesting of \e destination to zero (root) level.
*/
#define LTDBG_RESET_NESTING(i, d)					(i)->RootNestingLevel(d)

/*!
\param i ILTOutputRedir interface pointer
\param d destination
\param l new nesting level
Reset nesting of \e destination to zero (root) level.
*/
#define LTDBG_SET_NESTING_LEVEL(i, d, l)			(i)->SetNestingLevel(d, l)

/*!
\param i ILTOutputRedir interface pointer
\param d destination
\return current nesting level for destination
Returns current nesting level for \e destination.
*/
#define LTDBG_GET_NESTING_LEVEL(i, d)				(i)->GetCurrentNestingLevel(d)

/*!
\param i ILTOutputRedir interface pointer
\param d destination
\param c character to use for nesting for \e destination.
Sets the character used at the beginning of an output message for \e destination.
X instances of this character will be prepended to the message, where X = current
nesting level for the destination.
*/
#define LTDBG_SET_NESTING_CHAR(i, d, c)				(i)->SetNestingChar(d, c)

// debug
#define LTDBG_OUT_DUMP_TABLE(i)					(i)->DumpRedirTable()


/////////////////////////////////////////////////////
// macros that output text with various parameters //
/////////////////////////////////////////////////////


/*!
\param i ILTOutputRedir interface pointer
\param c category
\param l level
\param s string
Prints a string.
Used for: Output.
*/
#define LTDBG_OUT_CLS(i,c,l,s)		(i)->OutputText(c, l, 0, 0, 0, 0xFFFFFFFF, s)

/*!
\param i ILTOutputRedir interface pointer
\param c category
\param l level
\param x color
\param s string
Prints a colored string if the destination supports color..
Used for: Output.
*/
#define LTDBG_OUT_CLXS(i,c,l,x,s)	(i)->OutputText(c, l, 0, 0, 0, x, s)

/*!
\param i ILTOutputRedir interface pointer
\param s string
Prints a general-info string.
Used for: Output.
*/
#define LTDBG_OUT_S(i,s)			(i)->OutputText(OUTPUT_CATEGORY_GENERAL, OUTPUT_LEVEL_INFO, 0, 0, 0, 0xFFFFFFFF, s)

/*!
\param i ILTOutputRedir interface pointer
\param s string
Prints a colored general-info string.
Used for: Output.
*/
#define LTDBG_OUT_XS(i,x,s)			(i)->OutputText(OUTPUT_CATEGORY_GENERAL, OUTPUT_LEVEL_INFO, 0, 0, 0, x, s)

/*!
\param i ILTOutputRedir interface pointer
\param s string
Prints a general-info string that includes file and line number.
Used for: Output.
*/
#define LTDBG_OUT_FNS(i,f,n,s)		(i)->OutputText(OUTPUT_CATEGORY_GENERAL, OUTPUT_LEVEL_INFO, 0, f, n, 0xFFFFFFFF, s)


/////////////////////////////////////////////////////////
// macros that output a string based on warning level. //
/////////////////////////////////////////////////////////


/*!
\param i ILTOutputRedir interface pointer
\param s string
Prints a general-info string.
Used for: Output.
*/
#define LTDBG_OUT_INFO(i,s)				(i)->OutputText(OUTPUT_CATEGORY_GENERAL, OUTPUT_LEVEL_INFO,			  0, __FILE__, __LINE__, 0xFFFFFFFF, s)

/*!
\param i ILTOutputRedir interface pointer
\param s string
Prints a general-warning string.
Used for: Output.
*/
#define LTDBG_OUT_WARNING(i,s)			(i)->OutputText(OUTPUT_CATEGORY_GENERAL, OUTPUT_LEVEL_WARNING,		  0, __FILE__, __LINE__, 0xFFFFFFFF, s)

/*!
\param i ILTOutputRedir interface pointer
\param s string
Prints a general-minor error string.
Used for: Output.
*/
#define LTDBG_OUT_MINOR_ERROR(i,s)		(i)->OutputText(OUTPUT_CATEGORY_GENERAL, OUTPUT_LEVEL_ERROR_MINOR,	  0, __FILE__, __LINE__, 0xFFFFFFFF, s)

/*!
\param i ILTOutputRedir interface pointer
\param s string
Prints a general-major error string.
Used for: Output.
*/
#define LTDBG_OUT_MAJOR_ERROR(i,s)		(i)->OutputText(OUTPUT_CATEGORY_GENERAL, OUTPUT_LEVEL_ERROR_MAJOR,	  0, __FILE__, __LINE__, 0xFFFFFFFF, s)

/*!
\param i ILTOutputRedir interface pointer
\param s string
Prints a general-critical error string.
Used for: Output.
*/
#define LTDBG_OUT_CRITICAL_ERROR(i,s)	(i)->OutputText(OUTPUT_CATEGORY_GENERAL, OUTPUT_LEVEL_ERROR_CRITICAL, 0, __FILE__, __LINE__, 0xFFFFFFFF, s)


/*!
macro that allows you to specify every output parameter, but does not take a 
variable argument format string.
Used For: Output.
*/
#define	LTDBG_OUT(i,c,l,o,f,n,x,s)		(i)->OutputText(c, l, o, f, n, x, s)

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// version of Output_Text_VA and Output_Text_VA_S that accept a varlist instead of 
// the "..." parameter.
// These are used by the diagnostic front-end which does some preprocessing of stuff
// which it is passed via a "...".  You can't pass the "..." on again, so the varlist
// that was unpacked by the diagnostic preprocessing function is passed to these instead.
inline void Output_Text_VA_S(ILTOutputRedir* i, const char* pFormatStr, va_list args) 
{
	LTVSNPrintF(i->GetPublicBuffer(), i->GetPublicBufferSize(), pFormatStr, args);	
	i->OutputText(OUTPUT_CATEGORY_GENERAL, OUTPUT_LEVEL_INFO, 0, 0, 0, 0xFFFFFFFF, i->GetPublicBuffer());
}

inline void Output_Text_VA(ILTOutputRedir*		i, 
						   ECOutput_Category	category,
						   ECOutput_Level		level,
						   const char*			pObjectName,
						   const char*			pFileName,
						   uint32				lineno,
						   uint32				color,
						   const char*			pFormatStr,
						   va_list				args)
{
	LTVSNPrintF(i->GetPublicBuffer(), i->GetPublicBufferSize(), pFormatStr, args);	
	i->OutputText(category, level, pObjectName, pFileName, lineno, color, i->GetPublicBuffer());
}

#endif // DOXYGEN_SHOULD_SKIP_THIS

/*!
\param i pointer to output redir interface
\param pFormatStr format string
\param ... variable arguments
Simplified output function which takes variable arguments similar to printf().
\b IMPORTANT:  In order to use this variable argument macro, you \e must
call it with double parenthesis.  For example:

OUTPUT_TEXT_VA_S ((i, pFormatStr, arg1, arg2));

Used For: Output.
*/
#define LTDBG_OUT_VA_S(args) Output_Text_VA_S args

#ifndef DOXYGEN_SHOULD_SKIP_THIS

inline void Output_Text_VA_S(ILTOutputRedir* i, const char* pFormatStr, ...) 
{
	va_list args;
	va_start(args, pFormatStr);

	Output_Text_VA_S(i, pFormatStr, args);
//	VSPRINTF(i->GetPublicBuffer(), i->GetPublicBufferSize(), pFormatStr, args);	
//	i->OutputText(OUTPUT_CATEGORY_GENERAL, OUTPUT_LEVEL_INFO, 0, 0, 0, 0xFFFFFFFF, i->GetPublicBuffer());
	
	va_end(args);
}



#endif // DOXYGEN_SHOULD_SKIP_THIS


/*!
\param i pointer to output redir interface
\param category the output category
\param level the output level
\param pObjectName the name of an object associated with this message (can be \b NULL)
\param pFileName the name of the file associated with this message (can be \b NULL)
\param lineno the line number associated with this message
\param color the color to print the message
\param pFormatStr format string
\param ... variable arguments

Gives access to all the parameters of the output text function.  Some notes on usage:

If \e pObjectName is \b NULL, no object name will be printed.  If \e pFileName is \b NULL,
no file name will be printed and \e lineno will be ignored.  Not all output destinations 
support colored output, therefore \e color will be ignored if not applicable.

\b IMPORTANT:  In order to use this variable argument function, you \e must
call it with double parenthesis.  For example:

OUTPUT_TEXT_VA((i, OUTPUT_CATEGORY_GENERAL, OUTPUT_LEVEL_INFO, "CMyObject", __FILE__, __LINE__, 0xFFFF0000, pFormatStr, arg1, arg2));

Used For: Output.
*/
#define LTDBG_OUT_VA(args)	Output_Text_VA args

#ifndef DOXYGEN_SHOULD_SKIP_THIS

inline void Output_Text_VA(ILTOutputRedir*		i, 
						   ECOutput_Category	category,
						   ECOutput_Level		level,
						   const char*			pObjectName,
						   const char*			pFileName,
						   uint32				lineno,
						   uint32				color,
						   const char*			pFormatStr,
						   ...)
{
	va_list args;
	va_start(args, pFormatStr);

	Output_Text_VA(i, category, level, pObjectName, pFileName, lineno, color, pFormatStr, args);

//	VSPRINTF(i->GetPublicBuffer(), i->GetPublicBufferSize(), pFormatStr, args);	
//	i->OutputText(category, level, pObjectName, pFileName, lineno, color, i->GetPublicBuffer());
	
	va_end(args);
}

#endif // DOXYGEN_SHOULD_SKIP_THIS


#else // LIBLTINFO_OUTPUT_REDIRECTION


// if LIBLTINFO_OUTPUT_REDIRECTION is not defined, then all these macros turn into ((void)0).
// this prevents the execution of any functions that may have been passed as parameters to
// these macros.

#ifndef DOXYGEN_SHOULD_SKIP_THIS

// SETUP/MAINTENNANCE macros
#define LTDBG_OUT_RESET(i)							((void)0)

#define LTDBG_OUT_ENABLE_DESTINATION(i,d)			((void)0)
#define LTDBG_OUT_DISABLE_DESTINATION(i,d)			((void)0)

#define LTDBG_OUT_OPEN_LOGFILE(i,d,f)				((void)0)
#define LTDBG_OUT_CLOSE_LOGFILE(i,d)				((void)0)

#define LTDBG_OUT_GET_DESTINATION(i,c,l)			((void)0)
#define LTDBG_OUT_DESTINATION_STATUS(i,d)			((void)0)

#define LTDBG_OUT_ADD_LEVEL(i,l,d)					((void)0)
#define LTDBG_OUT_REMOVE_LEVEL(i,l)					((void)0)
#define LTDBG_OUT_CLEAR_LEVELS(i,c)					((void)0)

#define LTDBG_OUT_ADD_CATEGORY(i,c,d)				((void)0)
#define LTDBG_OUT_REMOVE_CATEGORY(i,c)				((void)0)
#define LTDBG_OUT_CLEAR_CATEGORIES(i)				((void)0)

#define LTDBG_OUT_ADD_FILTER_IN_CATEGORY(i,c,l,d)	((void)0)
#define LTDBG_OUT_REMOVE_FILTER_IN_CATEGORY(i,c,l)	((void)0)

#define LTDBG_SMART_NESTING(i, d)					((void)0)
#define LTDBG_INCREASE_NESTING(i, d)				((void)0)
#define LTDBG_DECREASE_NESTING(i, d)				((void)0)
#define LTDBG_RESET_NESTING(i, d)					((void)0)
#define LTDBG_SET_NESTING_LEVEL(i, d, l)			((void)0)
#define LTDBG_GET_NESTING_LEVEL(i, d)				((void)0)
#define LTDBG_SET_NESTING_CHAR(i, d, c)				((void)0)

#define LTDBG_OUT_DUMP_TABLE(i)						((void)0)

// OUTPUT macros
#define LTDBG_OUT_CLS(i,c,l,s)						((void)0)
#define LTDBG_OUT_CLXS(i,c,l,x,s)					((void)0)
#define LTDBG_OUT_S(i,s)							((void)0)
#define LTDBG_OUT_XS(i,x,s)							((void)0)
#define LTDBG_OUT_FNS(i,f,n,s)						((void)0)

// output by warning level
#define LTDBG_OUT_INFO(i,s)							((void)0)
#define LTDBG_OUT_WARNING(i,s)						((void)0)
#define LTDBG_OUT_MINOR_ERROR(i,s)					((void)0)
#define LTDBG_OUT_MAJOR_ERROR(i,s)					((void)0)
#define LTDBG_OUT_CRITICAL_ERROR(i,s)				((void)0)

// INLINES turned into MACROS
#define	LTDBG_OUT(i,c,l,o,f,n,x,s)					((void)0)
#define	LTDBG_OUT_VA_S(args)						((void)0)
#define	LTDBG_OUT_VA(args)							((void)0)

#endif // DOXYGEN_SHOULD_SKIP_THIS


#endif // LIBLTINFO_OUTPUT_REDIRECTION


#endif // __ILTOUTPUTREDIR_H__
