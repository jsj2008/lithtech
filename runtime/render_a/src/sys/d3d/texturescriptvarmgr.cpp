#include "precompile.h"

#include "texturescriptvarmgr.h"
#include "strtools.h"

//------------------------------------------------------------------------
// Render struct functions. This allows the main engine to access our
// parameters.
//------------------------------------------------------------------------
uint32 d3d_GetTextureEffectVarID(const char* pszName, uint32 nStage)
{
	return CTextureScriptVarMgr::GetID(pszName, nStage);
}

bool d3d_SetTextureEffectVar(uint32 nVarID, uint32 nVar, float fVar)
{
	return CTextureScriptVarMgr::GetSingleton().SetVar(nVarID, nVar, fVar);
}

//---------------------------------------------
// CTextureScriptVarNode
//
//  The actual node in the list of variables
//---------------------------------------------
class CTextureScriptVarNode
{
public:

	//the ID of these variables
	uint32					m_nID;

	//linked list info
	CTextureScriptVarNode*	m_pPrev;
	CTextureScriptVarNode*	m_pNext;

	//the actual variables
	float					m_fVar[CTextureScriptVarMgr::NUM_VARS];
};

//---------------------------------------------
// CTextureScriptVarMgr
//---------------------------------------------

CTextureScriptVarMgr::CTextureScriptVarMgr() :
	m_pHead(NULL)
{
}

CTextureScriptVarMgr::~CTextureScriptVarMgr()
{
	DeleteAllVars();
}

//singleton operators
static CTextureScriptVarMgr g_GlobalTextureScriptVarMgr;

CTextureScriptVarMgr& CTextureScriptVarMgr::GetSingleton()
{
	return g_GlobalTextureScriptVarMgr;
}

//static function that when given a string will return a hashed ID to be used
//for accessing the actual variables
uint32 CTextureScriptVarMgr::GetID(const char* pszName, uint32 nStage)
{
	char pszFullName[256];
	LTSNPrintF(pszFullName, sizeof(pszFullName), "%s%d", pszName, nStage);	

	//make this string lowercase to prevent case issues
	char* pszCurr = pszFullName;
	while(*pszCurr)
	{
		*pszCurr = tolower(*pszCurr);
		pszCurr++;
	}

	// Return the hash code
	return st_GetHashCode(pszFullName);
}

//used to add a new entry into the table. If an array is passed into the function
//it will be assumed to be at least the size specified and any other elements will
//be initialized with 0. Returns false if unable to allocate room
bool CTextureScriptVarMgr::CreateVars(uint32 nID, uint32 nNumDefaults, float* pDefaults)
{
	//make sure that this node doesn't exist (but don't do this in release mode)
	ASSERT((GetNode(nID) == NULL) && "Attempted to add an ID twice to the script variables. Possible hash conflict");

	//create a new node
	CTextureScriptVarNode* pNode;
	LT_MEM_TRACK_ALLOC(pNode = new CTextureScriptVarNode,LT_MEM_TYPE_RENDER_TEXTURESCRIPT);

	//check the allocation
	if(!pNode)
		return false;

	//setup the node
	pNode->m_nID	= nID;
	pNode->m_pPrev	= NULL;
	pNode->m_pNext	= m_pHead;

	//fill in the variables

	//make sure that the user is calling this correctly (if num defualts is greater
	//than 0 they had better have passed in an array)
	ASSERT((nNumDefaults == 0) || (pDefaults != NULL));
	
	//clamp the number of defaults
	nNumDefaults = LTMIN(nNumDefaults, NUM_VARS);

	uint32 nVar = 0;
	//copy over the array
	for(; nVar < nNumDefaults; nVar++)
		pNode->m_fVar[nVar] = pDefaults[nVar];

	//fill in the rest with 0
	for(; nVar < NUM_VARS; nVar++)
		pNode->m_fVar[nVar] = 0.0f;

	//add our node to our list
	m_pHead = pNode;
	
	//success
	return true;
}

//gets the variable list from the specified ID. Returns NULL if the ID is not found
float* CTextureScriptVarMgr::GetVars(uint32 nID)
{
	CTextureScriptVarNode* pNode = GetNode(nID);
	if(!pNode)
		return NULL;

	//found it, return the array
	return pNode->m_fVar;
}

//called to set a specific variable. The return value indicates whether or not
//that ID was found. Assumes the var is in the range of [0..NUM_VARS - 1]
bool CTextureScriptVarMgr::SetVar(uint32 nID, uint32 nVar, float fVal)
{
	//sanity check
	if(nVar >= NUM_VARS)
		return false;

	//find the node
	CTextureScriptVarNode* pNode = GetNode(nID);
	if(!pNode)
		return false;

	//set the value
	pNode->m_fVar[nVar] = fVal;

	return true;
}

//releases an ID, returns true if it found the ID to be released
bool CTextureScriptVarMgr::DeleteVars(uint32 nID)
{
	//find the node
	CTextureScriptVarNode* pNode = GetNode(nID);
	if(!pNode)
		return false;

	//ok, now fix up the links
	if(pNode->m_pPrev)
		pNode->m_pPrev->m_pNext = pNode->m_pNext;
	else
		m_pHead = pNode->m_pNext;

	if(pNode->m_pNext)
		pNode->m_pNext->m_pPrev = pNode->m_pPrev;

	//delete it
	delete pNode;

	return true;
}

//frees all variables
void CTextureScriptVarMgr::DeleteAllVars()
{
	//just remove everything
	CTextureScriptVarNode* pCurr = m_pHead;
	CTextureScriptVarNode* pNext;

	//run through the list deleting away
	while(pCurr)
	{
		pNext = pCurr->m_pNext;
		delete pCurr;
		pCurr = pNext;
	}

	//clean up
	m_pHead = NULL;	
}

//finds the actual node given an ID. Will return NULL if none found
CTextureScriptVarNode* CTextureScriptVarMgr::GetNode(uint32 nID)
{
	//just remove everything
	CTextureScriptVarNode* pCurr = m_pHead;

	//run through the list deleting away
	while(pCurr)
	{
		if(pCurr->m_nID == nID)
			return pCurr;

		pCurr = pCurr->m_pNext;
	}

	//no match
	return NULL;	
}


