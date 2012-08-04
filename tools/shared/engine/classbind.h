
// The ClassBind module just loads up an object module with classes,
// and gets a list of the classes that it supports.

#ifndef __CLASSBIND_H__
#define __CLASSBIND_H__

    #ifndef __LTSERVEROBJ_H__
	#include "ltserverobj.h"
    #endif

	
	typedef void* HCLASSMODULE;


	#define CB_NOERROR			-1
	#define CB_CANTFINDMODULE	0
	#define CB_NOTCLASSMODULE	1
	#define CB_VERSIONMISMATCH	2



	// Returns a CB_ status. version is set if it returns CB_VERSIONMISMATCH.
	int			cb_LoadModule(const char *pModuleName, ILTServer *pServerDE, HCLASSMODULE *phModule, int *version);
	void		cb_UnloadModule(HCLASSMODULE hModule);

	int			cb_GetNumClassDefs(HCLASSMODULE hModule);
	ClassDef**	cb_GetClassDefs(HCLASSMODULE hModule);

	// Helpers..
	ClassDef*	cb_FindClass(HCLASSMODULE hModule, const char *pClassName);
	PropDef*	cb_FindVarFull(HCLASSMODULE hModule, ClassDef *pClass, const char *pVarName);
	ClassDef *	cb_IsClassFlagSet( HCLASSMODULE hModule, ClassDef *pClass, const uint32 dwClassFlag );
	bool		cb_IsChildClass( HCLASSMODULE hModule, const char *pChildName, const char *pParentName );


#endif  // __CLASSBIND_H__

