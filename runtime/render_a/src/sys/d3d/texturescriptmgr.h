#ifndef __TEXTURESCRIPTMGR_H__
#define __TEXTURESCRIPTMGR_H__

//forward declarations
class CTextureScriptInstance;
class CTextureScriptNode;
class CTextureScriptInstanceNode;
class ITextureScriptEvaluator;

#include <vector>
using namespace std;

class CTextureScriptMgr
{
public:

	CTextureScriptMgr();
	~CTextureScriptMgr();

	//singleton support
	static CTextureScriptMgr& GetSingleton();

	//creates a script instance. Note that the returned pointer must be released
	//with the function ReleaseInstance, and cannot be deleted
	CTextureScriptInstance* GetInstance(const char* pszGroupName);

	//releases a script instance, the pointer should not be used after this call
	void ReleaseInstance(CTextureScriptInstance* pInstance);

	//releases all scripts and instances
	void ReleaseAll();

private:

	//loads up the specified evaluator. Returns NULL if unable to load
	ITextureScriptEvaluator* LoadEvaluator(const char* pszName);

	//function that allows for custom overriding of evaluators for performance reasons.
	//returns NULL if none match the given name
	ITextureScriptEvaluator* GetHardcodedEvaluator(const char* pszName);

	//find the instance with the specified name
	CTextureScriptInstance* FindInstance(const char* pszGroupName);

	//prevent copying
	CTextureScriptMgr(const CTextureScriptMgr&)		{}
	void operator=(const CTextureScriptMgr&)		{}

	typedef vector<CTextureScriptNode*>				TScriptList;
	typedef vector<CTextureScriptInstanceNode*>		TInstanceList;

	TScriptList		m_cScripts;
	TInstanceList	m_cInstances;

};

#endif
