#ifndef __ILTDIAGNOSTICMGR_H__
#define __ILTDIAGNOSTICMGR_H__

#ifndef __LTMODULE_H__
#include "ltmodule.h"
#endif

#ifndef _STDARG_H
#include "stdarg.h"
#endif

#ifndef __ILTOUTPUTREDIR_H__
#include "libltinfo/iltoutputredir.h"
#endif

//------------------------------------------------------------
//Base class for diagnostic manager..
//------------------------------------------------------------

// (implementations of diagnostic mgrs for specific systems will
// add enums describing different groups)
typedef uint8 LTDiagnosticSubcategory;
const LTDiagnosticSubcategory SUBCATEGORY_RESERVED = 0;

class LTDiagnosticPluginClass;
class LTDiagnosticPluginInstance;

/*!


*/
class ILTDiagnosticMgr : public IBase {
public:
    interface_version(ILTDiagnosticMgr, 0);

	enum { MAX_CATEGORIES = 10, 
		   MAX_SUBCATEGORIES = 10 };	// arbitrary (these do size arrays in the implementation!)


	virtual void SetFirstCategory(ECOutput_Category firstCategory) = 0;

	virtual void SubcategoryOutputOnOff(const ECOutput_Category& category,
										const LTDiagnosticSubcategory& Subcategory,
										bool bOn) = 0;

	// just output the message as given
	/*!
	\param group
	\param Subcategory
	\param level
	\param bForce
	\param fmtString
	\param vaArgs

	\see 

	Used for: Diagnostics.
	*/
	virtual void DiagnosticMessagePlain(LTDiagnosticPluginClass* pClassDiagnostics, 
										 LTDiagnosticPluginInstance* pInstanceDiagnostics,
										 const LTDiagnosticSubcategory& subcategory,
										 const ECOutput_Level level,
										 bool bForce,	// ignore subcategory on/off switch
										 const char* fmtString,
										 va_list args) = 0;

	// output the message with some instance-related info (specific to implementation)
	/*!
	\param group
	\param Subcategory
	\param instanceInfo
	\param infoLevel
	\param errorLevel
	\param bForce
	\param fmtString
	\param vaArgs

	\see 

	Used for: Diagnostics.
	*/
	virtual void DiagnosticMessageWithInfo(LTDiagnosticPluginClass* pClassDiagnostics, 
										 LTDiagnosticPluginInstance* pInstanceDiagnostics,
										 const LTDiagnosticSubcategory& subcategory,
										 const ILTDiagnosticInstanceInfo* pInstanceInfo,
										 const ILTDiagnosticInstanceInfo::InfoLevel& infoLevel,
										 const ECOutput_Level level,
										 bool bForce,	// ignore subcategory on/off switch
										 const char* fmtString,
										 va_list args) = 0;

	// (internal?)
	virtual ECOutput_Level		GetOutputLevel(LTDiagnosticPluginClass*, LTDiagnosticPluginInstance*) = 0;

	virtual ECOutput_Category	GetOutputCategory(LTDiagnosticPluginClass*, LTDiagnosticPluginInstance*) = 0;
};


#endif // __ILTDIAGNOSTICMGR_H__

