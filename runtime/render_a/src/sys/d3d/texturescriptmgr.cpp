#include "precompile.h"

#include "texturescriptmgr.h"
#include "texturescriptinstance.h"
#include "texturescriptevaluator.h"
#include "texturescriptinterpreter.h"

//Interface for the client file manager
#include "client_filemgr.h"		
static IClientFileMgr* g_pIClientFileMgr;
define_holder(IClientFileMgr, g_pIClientFileMgr);

//defines
#define TEXTURE_SCRIPT_GROUP_VERSION		1

//various stage types possible
#define STAGETYPE_DISABLED					0
#define STAGETYPE_OVERRIDDEN				1
#define STAGETYPE_EVALUATED					2

//--------------------------------------------------------
// Node classes
//--------------------------------------------------------
class CTextureScriptNode
{
public:
	//the name of this script
	char						m_pszName[MAX_PATH + 1];

	//the actual evaluator
	ITextureScriptEvaluator*	m_pEvaluator;
};

class CTextureScriptInstanceNode
{
public:
	//the name of this instance/group
	char						m_pszName[MAX_PATH + 1];

	//the actual instance
	CTextureScriptInstance*		m_pInstance;
};


//--------------------------------------------------------
// CTextureScriptMgr
//--------------------------------------------------------
CTextureScriptMgr::CTextureScriptMgr()
{
}

CTextureScriptMgr::~CTextureScriptMgr()
{
	ReleaseAll();
}

//singleton support
static CTextureScriptMgr g_TextureScriptMgrSingleton;

CTextureScriptMgr& CTextureScriptMgr::GetSingleton()
{
	return g_TextureScriptMgrSingleton;
}

//creates a script instance. Note that the returned pointer must be released
//with the function ReleaseInstance, and cannot be deleted
CTextureScriptInstance* CTextureScriptMgr::GetInstance(const char* pszGroupName)
{
	//see if we already have one
	CTextureScriptInstance* pInst = FindInstance(pszGroupName);

	//any luck?
	if(pInst)
	{
		//found one already, return this
		pInst->AddRef();
		return pInst;
	}

	//create our new instance, and node to hold it
	LT_MEM_TRACK_ALLOC(pInst = new CTextureScriptInstance,LT_MEM_TYPE_RENDER_TEXTURESCRIPT);
	CTextureScriptInstanceNode* pNode;
	LT_MEM_TRACK_ALLOC(pNode = new CTextureScriptInstanceNode,LT_MEM_TYPE_RENDER_TEXTURESCRIPT);

	if(!pInst || !pNode)
		return NULL;

	//build up the filename for the group
	char pszFileName[MAX_PATH + 1];
	LTStrCpy(pszFileName, "TextureEffectGroups\\", sizeof(pszFileName));
	LTStrCat(pszFileName, pszGroupName, sizeof(pszFileName));

	//no match, so we now need to load in the new instance
	FileRef Ref; 
	Ref.m_FileType			= TYPECODE_UNKNOWN;
	Ref.m_pFilename			= pszFileName;
	FileIdentifier* pIdent	= g_pIClientFileMgr->GetFileIdentifier(&Ref, TYPECODE_UNKNOWN);

	//open up the stream
	ILTStream* pStream = g_pIClientFileMgr->OpenFile(&Ref);

	//see if the stream was successfully opened
	if (!pStream)
	{
		delete pInst;
		return NULL;
	}

	//now read in the file version
	uint32 nVersion;
	*pStream >> nVersion;

	//make sure that the file version matches
	if(nVersion != TEXTURE_SCRIPT_GROUP_VERSION)
	{
		delete pInst;
		pStream->Release();
		return NULL;
	}

	//ok, now we need to read in how many stages are listed in the file
	uint32 nNumStages;
	*pStream >> nNumStages;

	//clamp it to the maximum number of stages
	nNumStages = LTMIN(nNumStages, CTextureScriptInstance::NUM_STAGES);

	//now read in each stage
	for(uint32 nCurrStage = 0; nCurrStage < nNumStages; nCurrStage++)
	{
		//read in the type of the stage
		uint32 nStageType;
		*pStream >> nStageType;

		//just continue if this stage is disabled
		if(nStageType == STAGETYPE_DISABLED)
			continue;

		//read in the channel that this stage maps to
		uint32 nChannel;
		*pStream >> nChannel;

		if(nStageType == STAGETYPE_OVERRIDDEN)
		{
			//this stage is overridded, load in the stage that is overriding
			uint32 nOverride;
			*pStream >> nOverride;

			if(nOverride < CTextureScriptInstance::NUM_STAGES)
			{
				pInst->SetupStageAsReference(nCurrStage, (ETextureScriptChannel)nChannel, nOverride);
			}
		}
		else if(nStageType == STAGETYPE_EVALUATED)
		{
			//this stage is evaluated, make sure to load in the name of the script
			//and its default values
			char pszScript[MAX_PATH];
			pStream->ReadString(pszScript, MAX_PATH);

			//now read in the defaults
			uint32 nNumDefaults;
			*pStream >> nNumDefaults;

			float fVars[CTextureScriptVarMgr::NUM_VARS];
			for(uint32 nCurrDefault = 0; nCurrDefault < nNumDefaults; nCurrDefault++)
			{
				float fVar;
				*pStream >> fVar;
				
				if(nCurrDefault < CTextureScriptVarMgr::NUM_VARS)
					fVars[nCurrDefault] = fVar;
			}

			//now load our script
			ITextureScriptEvaluator* pEval = LoadEvaluator(pszScript);

			if(!pEval)
				continue;

			//now allocate our variables
			uint32 nVarID = CTextureScriptVarMgr::GetID(pszGroupName, nCurrStage);
			CTextureScriptVarMgr::GetSingleton().CreateVars(nVarID, nNumDefaults, fVars);

			//now setup this stage
			pInst->SetupStage(nCurrStage, nVarID, (ETextureScriptChannel)nChannel, pEval);
		}
	}

	//setup our node to hold this
	LTStrCpy(pNode->m_pszName, pszGroupName, sizeof(pNode->m_pszName));
	pNode->m_pInstance = pInst;

	m_cInstances.push_back(pNode);

	//release our stream
	pStream->Release();

	//add a reference to our instance and give it back to the user
	pInst->AddRef();
	return pInst;
}

//releases a script instance, the pointer should not be used after this call
void CTextureScriptMgr::ReleaseInstance(CTextureScriptInstance* pInstance)
{
	if(!pInstance)
		return;

	//now release our instance
	if(pInstance->Release() == 0)
	{
		//do a search to find the instance
		CTextureScriptInstanceNode* pNode = NULL;

		TInstanceList::iterator iCurrInst = m_cInstances.begin();
		for (; iCurrInst != m_cInstances.end(); ++iCurrInst)
		{
			if((*iCurrInst)->m_pInstance == pInstance)
			{
				pNode = *iCurrInst;
				break;
			}
		}

		//make sure that this was from our list!
		ASSERT(pNode);

		uint32 nCurrStage;

		for(nCurrStage = 0; nCurrStage < CTextureScriptInstance::NUM_STAGES; nCurrStage++)
		{
			CTextureScriptInstanceStage* pStage = &pInstance->m_Stages[nCurrStage];

			//don't bother if it isn't a valid stage or if it is overridded
			if(!pStage->m_bValid || pStage->m_pOverride)
				continue;

			//free the variables associated with the group
			CTextureScriptVarMgr::GetSingleton().DeleteVars(pStage->m_nID);
		}

		for(nCurrStage = 0; nCurrStage < CTextureScriptInstance::NUM_STAGES; nCurrStage++)
		{
			CTextureScriptInstanceStage* pStage = &pInstance->m_Stages[nCurrStage];

			//don't bother if it isn't a valid stage or if it is overridded
			if(!pStage->m_bValid || pStage->m_pOverride)
				continue;

			//make sure to release the reference to the script
			ITextureScriptEvaluator* pEvaluator = pStage->m_pEvaluator;
			ASSERT(pEvaluator);

			//now release the reference to it
			if(pEvaluator->Release() == 0)
			{
				//we need to find this script so we can release it
				TScriptList::iterator iCurrScript = m_cScripts.begin();
				for (; iCurrScript != m_cScripts.end(); ++iCurrScript)
				{
					if((*iCurrScript)->m_pEvaluator == pEvaluator)
					{
						m_cScripts.erase(iCurrScript);
						break;
					}
				}

				//free the memory
				delete pEvaluator;
			}
		}

		//no more outstanding references, we need to delete this
		delete pInstance;

		//remove it from our list
		m_cInstances.erase(iCurrInst);
	}
}

//releases all scripts and instances
void CTextureScriptMgr::ReleaseAll()
{
	//first off run through the instances and remove them
	for (TInstanceList::iterator iCurrInst = m_cInstances.begin(); iCurrInst != m_cInstances.end(); ++iCurrInst)
	{
		//ok, now lets delete this instance
		delete (*iCurrInst)->m_pInstance;

		//delete the node
		delete (*iCurrInst);
	}
	//free the vector
	m_cInstances.resize(0);

	//now run through all the actual script objects and remove them
	for (TScriptList::iterator iCurrScript = m_cScripts.begin(); iCurrScript != m_cScripts.end(); ++iCurrScript)
	{
		//ok, now lets delete this instance
		delete (*iCurrScript)->m_pEvaluator;

		//delete the node
		delete (*iCurrScript);
	}
	//free the vector
	m_cScripts.resize(0);
}

//find the instance with the specified name
CTextureScriptInstance* CTextureScriptMgr::FindInstance(const char* pszGroupName)
{
	//just do a standard search
	for (TInstanceList::iterator iCurrInst = m_cInstances.begin(); iCurrInst != m_cInstances.end(); ++iCurrInst)
	{
		if(stricmp((*iCurrInst)->m_pszName, pszGroupName) == 0)
			return (*iCurrInst)->m_pInstance;
	}

	//no match
	return NULL;
}

//loads up the specified evaluator. Returns NULL if unable to load
ITextureScriptEvaluator* CTextureScriptMgr::LoadEvaluator(const char* pszName)
{
	//lets see if we already have an evaluator that matches this
	for (TScriptList::iterator iCurrEval = m_cScripts.begin(); iCurrEval != m_cScripts.end(); ++iCurrEval)
	{
		if(stricmp((*iCurrEval)->m_pszName, pszName) == 0)
		{
			//found a match, add a reference and return
			(*iCurrEval)->m_pEvaluator->AddRef();
			return (*iCurrEval)->m_pEvaluator;
		}
	}

	//no match was found, we need to load in a new evaluator

	//lets see if we have a hardcoded evaluator for this name
	ITextureScriptEvaluator* pEval = GetHardcodedEvaluator(pszName);

	if(pEval == NULL)
	{
		//no built in one, this must be just a normal script file, so we need to create a script evaluator
		//and have it load its script up.	
		CTextureScriptInterpreter* pInterp;
		LT_MEM_TRACK_ALLOC(pInterp = new CTextureScriptInterpreter,LT_MEM_TYPE_RENDER_TEXTURESCRIPT);

		//failed to allocate
		if(pInterp == NULL)
			return NULL;

		//load up the script
		if(pInterp->LoadScript(pszName) == false)
		{
			//failed to load the script
			delete pInterp;
			return NULL;
		}

		//assign it back to eval
		pEval = pInterp;
	}

	//create a node to hold this information in
	CTextureScriptNode* pNode;
	LT_MEM_TRACK_ALLOC(pNode = new CTextureScriptNode,LT_MEM_TYPE_RENDER_TEXTURESCRIPT);

	if(!pNode)
	{
		delete pEval;
		return NULL;
	}

	LTStrCpy(pNode->m_pszName, pszName, sizeof(pNode->m_pszName));
	pNode->m_pEvaluator = pEval;
	m_cScripts.push_back(pNode);

	//success, add a reference and bail
	pEval->AddRef();

	return pEval;
}

//function that allows for custom overriding of evaluators for performance reasons.
//returns NULL if none match the given name
ITextureScriptEvaluator* CTextureScriptMgr::GetHardcodedEvaluator(const char* pszName)
{
	//this should check the name passed in with all the hard coded names that are currently
	//implemented

	//no matches
	return NULL;
}
