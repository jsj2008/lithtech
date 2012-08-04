#ifndef __LTDIAGNOSTICPLUGINS_H__
#define __LTDIAGNOSTICPLUGINS_H__

#ifndef _INC_STDARG
#include "stdarg.h"
#endif

// static plugin object (one per class)
/*!
Plugin class added to a diagnostic-using class by macro interface.
Only one instance of the plugin is added to the class (it's a static member).

This plugin stores a class name, output redirection category membership, and minimum output level.

*/
class LTDiagnosticPluginClass
{
public:

	/*!
	\param className	String name of class employing diagnostics.
	\param category		Output redirection enum designating what group this class belongs to.
	\param level		Output redirection enum setting the minimum level for output.

	Constructor.

	Used for: Diagnostics.
	*/
	LTDiagnosticPluginClass(const char* className, 
							 const ECOutput_Category& category,
						 	 const ECOutput_Level level) 
							:	m_Category(category),
								m_Level(level)
	{
		SetClassName(className);
	}

	enum { CLASS_NAME_LENGTH = 50 };

	/*!
	\param name String name of class employing diagnostics.

	\see GetClassName

	Used for: Diagnostics.
	*/
	void SetClassName(const char* name)
	{
		if( name )
			strncpy(m_ClassName, name, CLASS_NAME_LENGTH - 1);
		else
			m_ClassName[0] = 0;
	}

	/*!
	\return String name of class.  Useful for output in diagnostic messages.

	\see SetClassName

	Used for: Diagnostics.
	*/
	const char* GetClassName() const	{ return m_ClassName; }

	/*!
	\param level

	\see 

	Used for: Diagnostics.
	*/
	void SetLevel(ECOutput_Level level)
	{ m_Level = level; }

	/*!
	\return 

	\see 

	Used for: Diagnostics.
	*/
	ECOutput_Level GetLevel() const
	{ return m_Level; }

	/*!
	\param category		Output redirection enum designating what group this class belongs to.

	\see GetDiagnosticGroup

	Used for: Diagnostics.
	*/
	void SetCategory(const ECOutput_Category& category)
	{ 
			m_Category = category; 
	}

	/*!
	\return The current output redirection category this class belongs to

	\see SetDiagnosticGroup

	Used for: Diagnostics.
	*/
	ECOutput_Category GetCategory() const
	{ return m_Category; }

//	virtual ILTDiagnosticMgr* GetDiagnosticMgr() const = 0;
	
protected:
	ECOutput_Category	m_Category;
	ECOutput_Level		m_Level;	// only display this level or higher
	char				m_ClassName[CLASS_NAME_LENGTH];
};

// instance-level plugin object
/*!


*/
class LTDiagnosticPluginInstance
{
public:
	LTDiagnosticPluginInstance() : m_Category(OUTPUT_CATEGORY_NONE), m_Level(OUTPUT_LEVEL_NONE)
	{}

	/*!
	\param pInfo

	\see 

	Used for: Diagnostics.
	*/
	void SetInstanceInfo(ILTDiagnosticInstanceInfo* pInfo)
	{	m_pInstanceInfo = pInfo; }

	/*!
	\return 

	\see 

	Used for: Diagnostics.
	*/
	const ILTDiagnosticInstanceInfo* GetInstanceInfo() const
	{	return m_pInstanceInfo; }

	/*!
	\return 

	\see 

	Used for: Diagnostics.
	*/
	ILTDiagnosticInstanceInfo* GetInstanceInfo() 
	{	return m_pInstanceInfo; }

	/*!
	\param level

	\see 

	Used for: Diagnostics.
	*/
	void SetLevel(ECOutput_Level level)
	{ m_Level = level; }

	/*!
	\return 

	\see 

	Used for: Diagnostics.
	*/
	ECOutput_Level GetLevel() const
	{ return m_Level; }

	/*!
	\param group

	\see 

	Used for: Diagnostics.
	*/
	void SetCategory(ECOutput_Category category)
	{ 	m_Category = category; 	}

	/*!
	\return 

	\see 

	Used for: Diagnostics.
	*/
	ECOutput_Category GetCategory() const
	{ return m_Category; }


protected:
	ECOutput_Category			m_Category; // overrides the class group if set
	ECOutput_Level				m_Level; // overrides the class level if set
	ILTDiagnosticInstanceInfo*	m_pInstanceInfo;
};



// macros
/*!
\defgroup Diagnostic_macros Diagnostic macro interface.
Add these macros to your custom class to use Autoviews to distribute their state.

\note Classes using Autoview must derive from \b BaseClass or \b BaseClassClient.

Used for: Diagnostics.
*/


//----------------------------------------
// System-wide setup and initialization
//----------------------------------------

// use DECLARE_DIAGNOSTIC_SUBSYSTEM and DEFINE_SYSTEM_CLASS_PLUGIN once per 
// instance of a diagnostic mgr.
// DECLARE_DIAGNOSTIC_SUBSYSTEM belongs in your subsystem-specific header.
#define DECLARE_DIAGNOSTIC_SUBSYSTEM(mgrName)										\
	extern ILTDiagnosticMgr* diagnostic_mgr_##mgrName;								\
	extern ILTOutputRedir*	 outputredir_##mgrName;									\

// Use DEFINE_DIAGNOSTIC_SUBSYSTEM in a .cpp file.
// (DEFINE_DIAGNOSTIC_SUBSYSTEM must appear in all modules - EXE, ObjectLTO, etc. -
//  where the diagnostic subsystem is active.)
#define DEFINE_DIAGNOSTIC_SUBSYSTEM(mgrName)										\
	ILTDiagnosticMgr* diagnostic_mgr_##mgrName;										\
	ILTOutputRedir*	  outputredir_##mgrName;										\
	define_holder_to_instance(ILTDiagnosticMgr, diagnostic_mgr_##mgrName##, mgrName); \
	define_holder(ILTOutputRedir, outputredir_##mgrName);

// Place INITIALIZE_DIAGNOSTIC_SUBSYSTEM in a constructor or other spot which
// will be called before any of the diagnostic subsystem's functionality will be used.
// "firstCategory" is the first ECOutput_Category (from iltoutputredir.h) of the set
// of categories this subsystem will manage.  (The full set must be continguous!)
#define INITIALIZE_DIAGNOSTIC_SUBSYSTEM(mgrName, firstCategory)						\
	diagnostic_mgr_##mgrName->SetFirstCategory(firstCategory)


// -------------------------------------
// Class-level setup and initialization
// -------------------------------------

// use DECLARE_DIAGNOSTICS in the base class of a hierarchy
// (derived classes can use all the functionality without having a DECLARE_DIAGNOSTICS)
#define DECLARE_DIAGNOSTICS(mgrName)												\
protected:																			\
	static LTDiagnosticPluginClass z_ClassDiagnostics;								\
	LTDiagnosticPluginInstance z_InstanceDiagnostics;								\
	void   z_DiagOutInfo(const LTDiagnosticSubcategory& subcategory,				\
						 const ILTDiagnosticInstanceInfo::InfoLevel& infoLevel,		\
						 const ECOutput_Level level,								\
						 const char* fmtString,										\
						 ...)	{													\
		va_list args;																\
		va_start(args, fmtString);													\
		diagnostic_mgr_##mgrName##->DiagnosticMessageWithInfo(&z_ClassDiagnostics,	\
					&z_InstanceDiagnostics, subcategory,							\
					z_InstanceDiagnostics.GetInstanceInfo(), infoLevel, level,		\
					false, fmtString, args);										\
		va_end(args); }																\
	void   z_DiagOut(const LTDiagnosticSubcategory& subcategory,					\
					 const ECOutput_Level level,									\
					 const char* fmtString,											\
					 ...)	{														\
		va_list args;																\
		va_start(args, fmtString);													\
		diagnostic_mgr_##mgrName##->DiagnosticMessagePlain(&z_ClassDiagnostics,		\
					&z_InstanceDiagnostics, subcategory, level,	false, fmtString,	\
					args);															\
		va_end(args); }																

// Place DEFINE_DIAGNOSTICS at file scope in the class .cpp file.
#define DEFINE_DIAGNOSTICS(mgrName, klas, group, level)								\
	LTDiagnosticPluginClass klas::z_ClassDiagnostics(#klas, group, level); 


// -------------------------------------
// Diagnostic functionality macros
// -------------------------------------
#define DIAG_SUBCATEGORY_ON_OFF(mgrName, cat, subcat, onoff)						\
	diagnostic_mgr_##mgrName##->SubcategoryOutputOnOff(cat, subcat, onoff)

#define DIAG_SET_INSTANCE_CATEGORY(cat)												\
	z_InstanceDiagnostics.SetCategory(cat)

#define DIAG_SET_CLASS_CATEGORY(cat)												\
	z_ClassDiagnostics.SetCategory(cat)

#define DIAG_GET_INSTANCE_CATEGORY()												\
	(z_InstanceDiagnostics.GetCategory())

#define DIAG_GET_CLASS_CATEGORY()													\
	(z_ClassDiagnostics.GetCategory())

#define DIAG_SET_INSTANCE_OUTPUT_LEVEL(level)										\
	z_InstanceDiagnostics.SetLevel(level)

#define DIAG_SET_CLASS_OUTPUT_LEVEL(level)											\
	z_ClassDiagnostics.SetLevel(level)

#define DIAG_GET_INSTANCE_OUTPUT_LEVEL()											\
	(z_InstanceDiagnostics.GetLevel())

#define DIAG_GET_CLASS_OUTPUT_LEVEL()												\
	(z_ClassDiagnostics.GetLevel())

#define DIAG_SET_INSTANCE_INFO(info)												\
	z_InstanceDiagnostics.SetInstanceInfo(info)

#define DIAG_GET_INSTANCE_INFO()													\
	(z_InstanceDiagnostics.GetInstanceInfo())

#define DIAG_MGR_PTR(mgrName)														\
	diagnostic_mgr_##mgrName

// -------------------------------------
// Pass-throughs to ILTOutputRedir
// -------------------------------------
#define DIAG_OUTPUTREDIR_PTR(mgrName)												\
	outputredir_##mgrName

#define DIAG_SET_CAT_CHANNEL(mgrName, cat, channel)									\
	LTDBG_OUT_ADD_CATEGORY(outputredir_##mgrName, cat, channel)

#define DIAG_SMART_NESTING(mgrName)													\
	LTDBG_SMART_NESTING(outputredir_##mgrName,										\
						outputredir_##mgrName##->GetDestination(					\
							diagnostic_mgr_##mgrName##->GetOutputCategory(			\
								&z_ClassDiagnostics, &z_InstanceDiagnostics),		\
							OUTPUT_LEVEL_ALL))

#define DIAG_INCREASE_NESTING(mgrName)												\
	LTDBG_INCREASE_NESTING(outputredir_##mgrName,									\
						   outputredir_##mgrName##->GetDestination(					\
								diagnostic_mgr_##mgrName##->GetOutputCategory(		\
									&z_ClassDiagnostics, &z_InstanceDiagnostics),	\
								OUTPUT_LEVEL_ALL))

#define DIAG_DECREASE_NESTING(mgrName)												\
	LTDBG_DECREASE_NESTING(outputredir_##mgrName,									\
						   outputredir_##mgrName##->GetDestination(					\
								diagnostic_mgr_##mgrName##->GetOutputCategory(		\
									&z_ClassDiagnostics, &z_InstanceDiagnostics),	\
								OUTPUT_LEVEL_ALL))

#define DIAG_SET_NESTING_LEVEL(mgrName, level)										\
	LTDBG_SET_NESTING_LEVEL(outputredir_##mgrName,									\
						    outputredir_##mgrName##->GetDestination(				\
								diagnostic_mgr_##mgrName##->GetOutputCategory(		\
									&z_ClassDiagnostics, &z_InstanceDiagnostics),	\
								OUTPUT_LEVEL_ALL), level)

#define DIAG_GET_NESTING_LEVEL(mgrName)												\
	LTDBG_GET_NESTING_LEVEL(outputredir_##mgrName,									\
						    outputredir_##mgrName##->GetDestination(				\
								diagnostic_mgr_##mgrName##->GetOutputCategory(		\
									&z_ClassDiagnostics, &z_InstanceDiagnostics),	\
								OUTPUT_LEVEL_ALL))

#define DIAG_NESTING_TO_ROOT(mgrName)												\
	LTDBG_RESET_NESTING(outputredir_##mgrName,										\
						outputredir_##mgrName##->GetDestination(					\
							diagnostic_mgr_##mgrName##->GetOutputCategory(			\
								&z_ClassDiagnostics, &z_InstanceDiagnostics),		\
							OUTPUT_LEVEL_ALL))

// -------------------------------------
// Convenience/internal macros
// -------------------------------------
#define CLASS_DIAGNOSTICS_REF z_ClassDiagnostics

#define INSTANCE_DIAGNOSTICS_REF z_InstanceDiagnostics

#endif //__LTDIAGNOSTICPLUGINS_H__

