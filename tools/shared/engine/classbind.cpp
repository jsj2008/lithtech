
#include "bdefs.h"
#include "classbind.h"
#include "bindmgr.h"
#include "ltobjectcreate.h"


struct ClassBindModule
{
	HBINDMODULE	m_hModule;
	ClassDef	**m_pClassDefs;
	int			m_nClassDefs;
};


typedef ClassDef** (*ObjectDLLSetupFn)(int *nDefs, void *pServer, int *version);


static void cb_VerifyClassDefProperties(ClassBindModule *pModule)
{
	int i, j;
	PropDef *pProp;

	for(i=0; i < pModule->m_nClassDefs; i++)
	{
		for(j=0; j < pModule->m_pClassDefs[i]->m_nProps; j++)
		{
			pProp = &pModule->m_pClassDefs[i]->m_Props[j];

			if(pProp->m_PropType >= NUM_PROPERTYTYPES || pProp->m_PropType < 0)
				pProp->m_PropType = NUM_PROPERTYTYPES;
		}
	}
}


#ifdef __PS2
extern ClassDef** ObjectDLLSetup(int *nDefs, ILTServer *pServer, int *version);
#endif

int cb_LoadModule(const char *pModuleName, ILTServer *pServer, HCLASSMODULE *phModule, int *version)
{
	ClassBindModule *pModule;
	// this is alloced here and then returned :(
	pModule = (ClassBindModule*)malloc(sizeof(ClassBindModule));
	HBINDMODULE hModule;

	// platforms that use static binding should just call the funciton.
	// Otherwise figure out which function and call that

	#ifdef __PS2
	pModule->m_hModule = NULL;
	// FIX THIS MJS - should not hard code to a name with "DLL" in it
	pModule->m_pClassDefs = ObjectDLLSetup(&pModule->m_nClassDefs, 
	                                       pServer, version);

	#else
	ObjectDLLSetupFn theFunction;
	int status;

	
	status = bm_BindModule(pModuleName, &hModule);
	if(status == BIND_CANTFINDMODULE)
		return CB_CANTFINDMODULE;

	// Get the function.
	theFunction = (ObjectDLLSetupFn)bm_GetFunctionPointer(hModule, "ObjectDLLSetup");
	if(!theFunction)
	{
		bm_UnbindModule(hModule);
		return CB_NOTCLASSMODULE;
	}

	// Ok.. setup the HCLASSMODULE.
	pModule->m_hModule = hModule;
	pModule->m_pClassDefs = theFunction(&pModule->m_nClassDefs, pServer, version);

	if(*version != SERVEROBJ_VERSION)
	{
		free(pModule);
		bm_UnbindModule(hModule);
		return CB_VERSIONMISMATCH;
	}
	#endif

	// Verify all properties!
	cb_VerifyClassDefProperties(pModule);

	*phModule = pModule;
	return CB_NOERROR;
}


void cb_UnloadModule(HCLASSMODULE hModule)
{
	ClassBindModule *pModule;

	pModule = (ClassBindModule*)hModule;
	bm_UnbindModule(pModule->m_hModule);

	free(pModule);
}


int cb_GetNumClassDefs(HCLASSMODULE hModule)
{
	return ((ClassBindModule*)hModule)->m_nClassDefs;
}


ClassDef** cb_GetClassDefs(HCLASSMODULE hModule)
{
	return ((ClassBindModule*)hModule)->m_pClassDefs;
}


ClassDef* cb_FindClass(HCLASSMODULE hModule, const char *pClassName)
{
	ClassBindModule *pModule;
	int i;

	pModule = (ClassBindModule*)hModule;
	for(i=0; i < pModule->m_nClassDefs; i++)
		if(strcmp(pModule->m_pClassDefs[i]->m_ClassName, pClassName) == 0)
			return pModule->m_pClassDefs[i];

	return LTNULL;
}


PropDef* cb_FindVarFull(HCLASSMODULE hModule, ClassDef *pClass, const char *pVarName)
{
	int i;

	while(pClass)
	{
		for(i=0; i < pClass->m_nProps; i++)
			if(strcmp(pClass->m_Props[i].m_PropName, pVarName) == 0)
				return &pClass->m_Props[i];
	
		pClass = pClass->m_ParentClass;
	}

	return LTNULL;
}


// IsClassFlagSet
//
// Finds if class flags are set in this or any parent class...
ClassDef *cb_IsClassFlagSet( HCLASSMODULE hModule, ClassDef *pClass, const uint32 dwClassFlag )
{
	while(pClass)
	{
		if( pClass->m_ClassFlags & dwClassFlag )
			return pClass;
	
		pClass = pClass->m_ParentClass;
	}

	return LTNULL;
}


bool cb_IsChildClass( HCLASSMODULE hModule, const char *pChildName, const char *pParentName )
{
	ClassDef *pClassDef = cb_FindClass(hModule, pChildName);

	// Walk up the heirarchy looking for the parent name
	while (pClassDef)
	{
		// Did we find the parent?
		if (strcmp(pClassDef->m_ClassName, pParentName) == 0)
			return true;

		// Go up the heirarchy
		pClassDef = pClassDef->m_ParentClass;
	}

	// We didn't find the parent...
	return false;
}