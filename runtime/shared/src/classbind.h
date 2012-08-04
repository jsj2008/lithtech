
// The ClassBind module just loads up an object module with classes,
// and gets a list of the classes that it supports.

#ifndef __CLASSBIND_H__
#define __CLASSBIND_H__


class CBindModuleType;
struct ClassDef;
struct PropDef;


#define CB_NOERROR			-1
#define CB_CANTFINDMODULE	0
#define CB_NOTCLASSMODULE	1
#define CB_VERSIONMISMATCH	2

class ClassBindModule
{
public:
    CBindModuleType *m_hModule;
    ClassDef    **m_pClassDefs;
    int         m_nClassDefs;
};

// Returns a CB_ status. version is set if it returns CB_VERSIONMISMATCH.
int			cb_LoadModule(const char *pModuleName, bool bTempFile, ClassBindModule& classBindModule, int *version);
void		cb_UnloadModule(ClassBindModule& module);

int			cb_GetNumClassDefs(ClassBindModule *hModule);
ClassDef**	cb_GetClassDefs(ClassBindModule *hModule);

// Helpers..
ClassDef*	cb_FindClass(ClassBindModule *hModule, const char *pClassName);
PropDef*	cb_FindVarFull(ClassBindModule *hModule, ClassDef *pClass, const char *pVarName);
ClassDef *	cb_IsClassFlagSet(ClassBindModule *hModule, ClassDef *pClass, const uint32 dwClassFlag );


#endif  // __CLASSBIND_H__

