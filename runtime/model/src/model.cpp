// ------------------------------------------------------------------------
// model.cpp
// monolith.
// ------------------------------------------------------------------------
#include "bdefs.h"
#include "model.h"
#include "model_ops.h"


#ifdef DE_CLIENT_COMPILE		// RenderStruct for Destroying Render Objects...
	#include "render.h"
#endif
#ifdef DE_RENDER_COMPILE		// or D3D_Device for Destroying Render Objects...
	#include "d3d_device.h"
#endif


#define MAX_SKELETON_NODES	256
#define DEFAULT_MODEL_VIS_RADIUS	50.0f


// ------------------------------------------------------------------------
// globals, and console vars
// ------------------------------------------------------------------------

// Used so we can track if the filename is allocated or not.
static char *g_pNoModelFilename = "no-filename";
float g_CV_DefaultDrawIndexedDist = 500.0f;


// Global value to track model memory usage
uint32 g_ModelMemory = 0;






LTRESULT DefaultLoadChildFn(ModelLoadRequest *pRequest, Model **pModel)
{
	return LT_NOCHANGE;
}



bool VerifyChildModel_R(ModelNode *pParentNode, ModelNode *pChildNode, ModelNode* &pErrNode)
{
	uint32 i;

	if(pParentNode->NumChildren() != pChildNode->NumChildren())
	{
		pErrNode = pParentNode;
		return false;
	}

	for(i=0; i < pParentNode->NumChildren(); i++)
	{
		if(!VerifyChildModel_R(pParentNode->GetChild(i), pChildNode->GetChild(i), pErrNode))
			return false;
	}

	return true;
}


// ------------------------------------------------------------------------
// input channel singleton accessors
// ------------------------------------------------------------------------
const IAnimPosChannel*	NULLPOSChannel::GetSingleton()
{
	static NULLPOSChannel s_Singleton;
	return &s_Singleton;
}

const IAnimQuatChannel*	NULLQUATChannel::GetSingleton()
{
	static NULLQUATChannel s_Singleton;
	return &s_Singleton;
}

const IAnimPosChannel*	POSChannel::GetSingleton()
{
	static POSChannel s_Singleton;
	return &s_Singleton;
}

const IAnimQuatChannel*	QUATChannel::GetSingleton()
{
	static QUATChannel s_Singleton;
	return &s_Singleton;
}

const IAnimPosChannel*	SinglePOSChannel::GetSingleton()
{
	static SinglePOSChannel s_Singleton;
	return &s_Singleton;
}

const IAnimQuatChannel*	SingleQUATChannel::GetSingleton()
{
	static SingleQUATChannel s_Singleton;
	return &s_Singleton;
}

const IAnimPosChannel*	POS16Channel::GetSingleton()
{
	static POS16Channel s_Singleton;
	return &s_Singleton;
}

const IAnimQuatChannel*	QUAT16Channel::GetSingleton()
{
	static QUAT16Channel s_Singleton;
	return &s_Singleton;
}

const IAnimPosChannel*	SinglePOS16Channel::GetSingleton()
{
	static SinglePOS16Channel s_Singleton;
	return &s_Singleton;
}

const IAnimQuatChannel*	SingleQUAT16Channel::GetSingleton()
{
	static SingleQUAT16Channel s_Singleton;
	return &s_Singleton;
}


// ------------------------------------------------------------------------ //
// ModelStringList.
// ------------------------------------------------------------------------ //

ModelStringList::ModelStringList(LAlloc *pAlloc)
{
	m_pAlloc = pAlloc;
	m_StringList = NULL;
}


ModelStringList::~ModelStringList()
{
	Term();
}


void ModelStringList::Term()
{
	ModelString *pCurString, *pNextString;

	pCurString = m_StringList;
	while(pCurString)
	{
		pNextString = pCurString->m_pNext;
		LDelete(GetAlloc(), pCurString);
		pCurString = pNextString;
	}
	m_StringList = NULL;
}


const char* ModelStringList::AddString(const char *pString)
{
	ModelString *pCur, *pRet;
	int len;
	uint32 dwSize;

	// Quick exit..
	if(!pString || pString[0] == 0)
		return "";

	pCur = m_StringList;
	while(pCur)
	{
		if(strcmp(pCur->m_String, pString) == 0)
			return pCur->m_String;

		pCur = pCur->m_pNext;
	}

	// Ok, add a new string.
	len = strlen(pString) + 1;
	dwSize = sizeof(ModelString) - 1 + len;
	pRet = (ModelString*)GetAlloc()->Alloc(dwSize);
	if(!pRet)
		return "";

	pRet->m_AllocSize = dwSize;
	pRet->m_pNext = m_StringList;
	m_StringList = pRet;

	LTStrCpy(pRet->m_String, pString, len);
	return pRet->m_String;
}


bool ModelStringList::SetAlloc(LAlloc *pAlloc)
{
	if(m_StringList)
	{
		assert(false);
		return false;
	}

	m_pAlloc = pAlloc;
	return true;
}






// ------------------------------------------------------------------------
// CDefVertexList
/* CDefVertexList's data :

		struct defVert {
			float pos[3];
		};
		// data array
		CMoArray<defVert, NoCache> m_vVertexList ;
		LAlloc                    *m_pAlloc ;
*/
// ------------------------------------------------------------------------

// default constructor
CDefVertexLst::	CDefVertexLst()
{
	//nothing
}

// array create constructor
CDefVertexLst::CDefVertexLst( uint32 size, LAlloc *alloc )
{
	m_pAlloc = alloc ;
	m_vVertexList.SetSize2( size, alloc );
}

CDefVertexLst::~CDefVertexLst()
{
	m_vVertexList.Term(m_pAlloc);
	m_pAlloc = NULL ;
}

// pre set the size of the array
void CDefVertexLst::setSize( uint32 size , LAlloc *alloc )
{
	/* possible weirdness :
		if m_vVertexList is alloced already and alloc is different from m_pAlloc.
	*/
	m_pAlloc = alloc ;
	m_vVertexList.SetSize2( size, alloc );
}

// get value at index (pointer to actual data )
// note: don't use pointer as the head of an array, defVert may
// change..
const float * CDefVertexLst::getValue( uint32 index ) const
{
	defVert & dv = m_vVertexList[index];
	return (const float*)&dv.pos;
}

// ------------------------------------------------------------------------ //
// ChildModel.
// ------------------------------------------------------------------------ //

ChildInfo::ChildInfo()
{
	m_AnimOffset = 0;
	m_pModel = NULL;
}

ChildInfo::~ChildInfo()
{
	Term();
}

void ChildInfo::Term()
{
	m_pModel = NULL;
}

// ------------------------------------------------------------------------ //
// AnimNode.
// like a hierarchy node but contains animationdata.
// ------------------------------------------------------------------------ //
AnimNode::AnimNode()
{
	m_pPosChannel = NULL ;
	m_pQuatChannel = NULL ;
	m_pVertexChannel = NULL ;
}


AnimNode::~AnimNode()
{
	Term();
}

void AnimNode::Term()
{
	m_pPosChannel	= NULL;
	m_pPosData		= NULL;

	m_pQuatChannel	= NULL;
	m_pQuatData		= NULL;

	if( m_pVertexChannel )
	{
		delete [] m_pVertexChannel ;
		m_pVertexChannel = NULL ;
	}
}

// ------------------------------------------------------------------------ //
// ModelAnim functions.
// ------------------------------------------------------------------------ //

ModelAnim::ModelAnim(Model *pModel)
{
	m_pModel = pModel;
	m_pName = "";
	m_pAnimNodes = NULL;
	m_InterpolationMS = 200;
}


ModelAnim::~ModelAnim()
{
	Term();
}


void ModelAnim::Term()
{
	if(m_pAnimNodes)
	{
		LDelete_Array(GetAlloc(), m_pAnimNodes, m_pModel->NumNodes());
		m_pAnimNodes = NULL;
	}

	m_KeyFrames.Term(GetAlloc());
}

uint32 ModelAnim::GetAnimTime() const
{
	if(m_KeyFrames.GetSize() > 0)
		return m_KeyFrames.Last().m_Time;
	else
		return 0;
}

// ------------------------------------------------------------------------ //
// AnimInfo.
// ------------------------------------------------------------------------ //

AnimInfo::AnimInfo()
{
	m_pAnim = NULL;

	// Init to a wierd number.. animators should always set it.
	m_vDims.Init(128.0f, 16.0f, 16.0f);
	m_vTranslation.Init();
}

// ------------------------------------------------------------------------ //
// ModelNode.
// ------------------------------------------------------------------------ //

ModelNode::ModelNode()
{
	Clear();
	m_pModel = NULL;
}

ModelNode::ModelNode(Model *pModel)
{
	Clear();
	m_pModel = pModel;
}

ModelNode::~ModelNode()
{
	Term();
}


void ModelNode::Term()
{
	DeleteAndClearArray(m_Children, GetAlloc());
	Clear();
}


void ModelNode::Clear()
{
	m_pName = "";
	m_NodeIndex = 0;
	m_Flags = 0;
	m_vOffsetFromParent.Init();
	m_mGlobalTransform.Identity();
	m_mInvGlobalTransform.Identity();
	m_mFromParentTransform.Identity();
}

uint32 ModelNode::CalcNumNodes() const
{
	uint32 total = 1;
	for(uint32 i=0; i < NumChildren(); i++)
	{
		total += m_Children[i]->CalcNumNodes();
	}

	return total;
}


bool ModelNode::FillNodeList(uint32 &curNodeIndex)
{
	uint32 i;

	if(curNodeIndex >= m_pModel->m_FlatNodeList.GetSize())
		return false;

	m_NodeIndex = (unsigned short)curNodeIndex;
	m_pModel->m_FlatNodeList[curNodeIndex] = this;
	++curNodeIndex;

	for(i=0; i < NumChildren(); i++)
	{
		if(!m_Children[i]->FillNodeList(curNodeIndex))
			return false;
	}

	return true;
}


void ModelNode::SetParent_R(uint32 iParent)
{
	uint32 i;

	m_iParentNode = iParent;

	for(i=0; i < NumChildren(); i++)
	{
		GetChild(i)->SetParent_R(m_NodeIndex);
	}
}

// ------------------------------------------------------------------------ //
// ModelPiece.
// ------------------------------------------------------------------------ //
ModelPiece::ModelPiece(Model* pModel) :
	m_pModel(pModel)
{
	//we need to have an associated model
	assert(m_pModel);

	m_nLODs				= 0;
	m_pLODDists			= NULL;
	m_pRenderObjects	= NULL;
}


ModelPiece::~ModelPiece()
{
	Term();
}

void ModelPiece::Term()
{
    // Delete/release renderobjects.
	if (m_pRenderObjects)
	{
		for (uint32 i = 0; i < m_nLODs; ++i)
		{
#ifdef DE_CLIENT_COMPILE
			if (m_pRenderObjects[i] && r_GetRenderStruct()->m_bLoaded) r_GetRenderStruct()->DestroyRenderObject(m_pRenderObjects[i]);
#endif
#ifdef DE_RENDER_COMPILE
			if (m_pRenderObjects[i]) g_Device.DestroyRenderObject(m_pRenderObjects[i]);
#endif
#ifdef DE_SERVER_COMPILE
			if (m_pRenderObjects[i])
			{
				// On the server side, empty objects are allocated. Delete these.
				delete m_pRenderObjects[i];
				m_pRenderObjects[i] = NULL;
			}
#endif
		}
	}

	// release the dist values ;
	if (m_pLODDists != NULL )
	{
		LDelete_Array(m_pModel->GetAlloc(), m_pLODDists, m_nLODs);
		m_pLODDists = NULL;
	}

	//now release the render object array
	if (m_pLODDists != NULL )
	{
		LDelete_Array(m_pModel->GetAlloc(), m_pRenderObjects, m_nLODs);
		m_pRenderObjects = NULL;
	}

	//we no longer have any LOD's
	m_nLODs = 0;
}

// ------------------------------------------------------------------------ //
// WeightSet.
// ------------------------------------------------------------------------ //

WeightSet::WeightSet(Model *pModel)
{
	m_pName	 = NULL;
	m_pModel = pModel;
}

WeightSet::~WeightSet()
{
	m_Weights.Term(GetAlloc());
}

bool WeightSet::InitWeights(uint32 nWeights)
{
	return m_Weights.SetSizeInit2(nWeights, 0.0f) ? true : false;
}


// ------------------------------------------------------------------------ //
// ModelSocket
// ------------------------------------------------------------------------ //

ModelSocket::ModelSocket()
{
	m_pName = NULL;
	m_iNode = 0;
	m_Pos.Init();
	m_Rot.Init();
}

// ------------------------------------------------------------------------ //
// Model functions.
// ------------------------------------------------------------------------ //

Model::Model(LAlloc *pAlloc, LAlloc *pDefAlloc) :
	m_StringList(pAlloc)
{
	m_pAlloc = pAlloc;
	m_pDefAlloc = pDefAlloc;

	m_pAnimData = NULL;

	m_FileID = 0xFFFFFFFF;

	m_pFilename = g_pNoModelFilename;

	m_Flags = 0;
	m_RefCount = 0;

	m_VisRadius = DEFAULT_MODEL_VIS_RADIUS;

	m_bShadowEnable = false;
	m_CommandString = "";

	m_pRootNode = &m_DefaultRootNode;
	m_pRootNode->m_pModel = this;

	m_nChildModels = 1;
	m_ChildModels[0] = &m_SelfChildModel;
	for( int i = 1 ; i < MAX_CHILD_MODELS ; i++ ) m_ChildModels[i] = NULL ;

	GetSelfChildModel()->m_pModel = this;

	// Memory tracking
	g_ModelMemory += sizeof(Model);

	DEBUG_MODEL_REZ(("model-rez: model constr [%d]", this) );

	g_ModelMgr.Add(this);

#if(MODEL_OBB)
	m_NumOBBs	= 0;
	m_ModelOBBs	=NULL;
#endif
}


Model::~Model()
{
	assert(m_RefCount == 0);

	// remove from instance list.
	g_ModelMgr.Remove(this) ;

	Term();

	DEBUG_MODEL_REZ(("model-rez: model release [%d] %s ", this, GetFilename()) );
	FreeFilename();

	// Memory tracking
	g_ModelMemory -= sizeof(Model);
	g_ModelMemory -= m_BlockAlloc.GetBlockSize();
}


void Model::Term()
{
	if(m_pRootNode != &m_DefaultRootNode)
	{
		LDelete(GetAlloc(), m_pRootNode);
	}

	//free up our animation data
	LDelete(GetAlloc(), m_pAnimData);
	m_pAnimData = NULL;

	m_DefaultRootNode.Term();
	m_pRootNode = &m_DefaultRootNode;
	DeleteAndClearArray2(m_Pieces, GetAlloc());

	TermAnims();
	TermChildModels();
	DeleteAndClearArray2(m_Sockets, GetAlloc());
	DeleteAndClearArray2(m_WeightSets, GetAlloc());

	m_FlatNodeList.Term(GetAlloc());

	// dfree all the strings.
	m_StringList.Term();

#if(MODEL_OBB)
	m_NumOBBs = 0 ;
	if( m_ModelOBBs )
	{
		delete [] m_ModelOBBs ;
		m_ModelOBBs = NULL ;
	}
#endif
}


void Model::TermAnims()
{
	ModelAnim *pAnim;

	// Delete all the anims we own.
	for(uint32 i = 0; i < m_Anims; i++)
	{
		pAnim = GetAnim(i);

		if(pAnim->GetModel() == this)
		{
			LDelete(GetAlloc(), pAnim);
		}
	}

	m_Anims.Term(GetDefAlloc());
}


void Model::TermChildModels()
{
	uint32 i;
	ChildInfo *pChildModel;

	for(i=0; i < NumChildModels(); i++)
	{
		pChildModel = m_ChildModels[i];

		if(!pChildModel)
			continue;

		// release only real children.
		if(pChildModel != GetSelfChildModel())
		{
			if(pChildModel->m_pModel && pChildModel->m_pModel != this)
			{
				pChildModel->m_pModel->Release();
				pChildModel->m_pModel = NULL ;
			}
		}

		pChildModel->Term();

		// Free the ChildModel object.
		if(pChildModel != GetSelfChildModel())
		{
			if(pChildModel != &m_SelfChildModel)
			{
				delete pChildModel;
			}
		}
	}

	m_nChildModels = 1;
	m_ChildModels[0] = &m_SelfChildModel;
}

// ------------------------------------------------------------------------
// PrecalcNodeLists( )
// returns false on fure anywhere in list below
// 1. resizes m_Transforms ;
// 2. builds the flat node list
// 3. sets an offset value for every piece.
// 4. calls PrecalcNodeListys on all animation nodes.
// 5. finds the number of verts and num tris the model contains.
// ------------------------------------------------------------------------
bool Model::PrecalcNodeLists(bool bRebuild)
{
	return BuildFlatNodeList(bRebuild);
}


bool Model::BuildFlatNodeList(bool bRebuild)
{
	uint32 nNodes, curNodeIndex;

	// Figure out how many there are.
	nNodes = GetRootNode()->CalcNumNodes();

	// early out, do it? or done?
	if(!bRebuild && m_FlatNodeList.GetSize() == nNodes)
		return true;

	m_FlatNodeList.Term(GetAlloc());


	if(!m_FlatNodeList.SetSizeInit4(nNodes, NULL, GetAlloc()))
		return false;

	// Fill in the list.
	curNodeIndex = 0;
	if(!GetRootNode()->FillNodeList(curNodeIndex))
		return false;


	GetRootNode()->SetParent_R(NODEPARENT_NONE);
	return true;
}


ModelPiece* Model::FindPiece(const char *pName, uint32 *index)
{
	uint32 i;
	ModelPiece *pPiece;

	for(i=0; i < NumPieces(); i++)
	{
		pPiece = GetPiece(i);

		if(stricmp(pName, pPiece->GetName()) == 0)
		{
			if(index)
				*index = i;

			return pPiece;
		}
	}

	return NULL;
}


WeightSet* Model::FindWeightSet(const char *pName, uint32 *index)
{
	uint32 i;
	WeightSet *pSet;

	for(i=0; i < NumWeightSets(); i++)
	{
		pSet = GetWeightSet(i);

		if(stricmp(pName, pSet->GetName()) == 0)
		{
			if(index)
				*index = i;

			return pSet;
		}
	}

	return NULL;
}

const char* Model::AddString(const char *pString)
{
	return m_StringList.AddString(pString);
}


ModelNode* Model::FindNode(const char *pName, uint32 *index)
{
	uint32 i;

	for(i=0; i < NumNodes(); i++)
	{
		if(stricmp(m_FlatNodeList[i]->GetName(), pName) == 0)
		{
			if(index)
				*index = i;

			return m_FlatNodeList[i];
		}
	}

	return NULL;
}


ModelNode* Model::FindParent(ModelNode *pNode, ModelNode *pRoot, uint32 *index)
{
	uint32 i;
	ModelNode *pTest;

	for(i=0; i < pRoot->NumChildren(); i++)
	{
		if(pRoot->m_Children[i] == pNode)
		{
			if(index)
				*index = i;

			return pRoot;
		}

		pTest = FindParent(pNode, pRoot->m_Children[i], index);
		if(pTest)
			return pTest;
	}

	return NULL;
}


ModelAnim* Model::FindAnim(const char *pName, uint32 *index, AnimInfo **ppInfo)
{
	uint32 i;
	ModelAnim *pAnim;

	for(i=0; i < m_Anims; i++)
	{
		pAnim = GetAnim(i);

		if(stricmp(pAnim->m_pName, pName) == 0)
		{
			if(index)
				*index = i;

			if(ppInfo)
				*ppInfo = &m_Anims[i];

			return pAnim;
		}
	}

	return NULL;
}


AnimInfo* Model::FindAnimInfo(const char *pAnimName, Model *pOwner, uint32 *index)
{
	uint32 i;
	AnimInfo *pInfo;


	for(i=0; i < m_Anims; i++)
	{
		pInfo = &m_Anims[i];

		if(pInfo->m_pAnim->GetModel() == pOwner)
		{
			if(stricmp(pInfo->m_pAnim->GetName(), pAnimName) == 0)
			{
				if(index)
					*index = i;

				return pInfo;
			}
		}
	}

	return NULL;
}

void Model::SetNodeParentOffsets()
{
	uint32 i;
	ModelNode *pNode, *pParent;
	LTMatrix mLocal;


	for(i=0; i < NumNodes(); i++)
	{
		pNode = GetNode(i);

		if(pNode->m_iParentNode < NumNodes())
		{
			pParent = GetNode(pNode->m_iParentNode);
			mLocal = pParent->GetGlobalTransform().MakeInverseTransform() * pNode->GetGlobalTransform();
			mLocal.GetTranslation(pNode->m_vOffsetFromParent);
		}
		else
		{
			pNode->m_vOffsetFromParent.Init();
		}
	}
}

uint32 Model::CalcNumChildModelAnims()
{
	uint32 i, total;
	ChildInfo *pChildModel;

	total = 0;
	for(i=0; i < NumChildModels(); i++)
	{
		pChildModel = GetChildModel(i);

		if(!pChildModel->m_pModel)
			continue;

		if(pChildModel == GetSelfChildModel())
			continue;

		total += pChildModel->m_pModel->m_Anims.GetSize();
	}

	return total;
}


uint32 Model::CalcNumParentAnims() const
{
	uint32 total = 0;
	for(uint32 i=0; i < m_Anims; i++)
	{
		if(m_Anims[i].m_pAnim->m_pModel == this)
			++total;
	}

	return total;
}


void Model::ParseCommandString()
{
	if(!m_CommandString)
		return;

	// Parse for level of detail settings...
	ConParse parse;
	parse.Init(m_CommandString);
	while(parse.Parse())
	{
		if(parse.m_nArgs == 1)
		{
			if(stricmp("ShadowEnable", parse.m_Args[0]) == 0)
			{
				m_bShadowEnable = true;
			}
		}
	}
}


const char* Model::GetFilename() const
{
	return m_pFilename;
}


bool Model::SetFilename(const char *pInFilename)
{
	char *pFilename;

	FreeFilename();
	uint32 nStrLen = strlen(pInFilename);

	LT_MEM_TRACK_ALLOC(pFilename = new char[nStrLen + 1],LT_MEM_TYPE_MODEL);
	if(!pFilename)
	{
		return false;
	}

	//make sure that this filename will match other filenames
	CHelpers::FormatFilename(pInFilename, pFilename, nStrLen + 1);

	m_pFilename = pFilename;
	return true;
}


void Model::FreeFilename()
{
	if(m_pFilename != g_pNoModelFilename)
	{
		if( m_pFilename )
		delete [] m_pFilename;
	}

	m_pFilename = g_pNoModelFilename;
}


bool Model::VerifyChildModelTree(Model *pChild, ModelNode* &pErrNode)
{
	return VerifyChildModel_R(GetRootNode(), pChild->GetRootNode(), pErrNode);
}




ChildInfo* Model::AddChildModel(Model *pChild, 
								const char *pFilename, 
								char *pErrStr, uint32 nErrStrLen)
{
	ChildInfo *pChildModel;
	AnimInfo *pAnim;
	uint32 i, index;
	ModelNode *pErrNode;
	
	
	if(FindChildModel(pChild,index))
	{
		LTStrCpy(pErrStr, "Model already exists.", nErrStrLen);
		return NULL;
	}

	// Can't have too many.
	if(m_nChildModels >= MAX_CHILD_MODELS)
	{
		LTStrCpy(pErrStr, "Child model limit reached.", nErrStrLen);
		return NULL;
	}

	// Make sure it's valid.
	if(pChild)
	{
		if(!VerifyChildModelTree(pChild, pErrNode))
		{
			LTSNPrintF(pErrStr, nErrStrLen, "Node trees differ at node %s.", pErrNode->GetName());
			return false;
		}
	}

	// Add it..
	LT_MEM_TRACK_ALLOC(pChildModel = new ChildInfo,LT_MEM_TYPE_MODEL);
	if(!pChildModel)
		return NULL;

	// Add it to the list.
	m_ChildModels[m_nChildModels] = pChildModel;
	m_nChildModels++;

	// Set it up.
	pChildModel->m_pModel = pChild;
	pChildModel->m_AnimOffset = 0;

	if(pChild)
	{
		pChild->AddRef();

		pChildModel->m_AnimOffset = m_Anims.GetSize();
		LTBOOL bSetSizeResult;
		LT_MEM_TRACK_ALLOC(bSetSizeResult = m_Anims.NiceSetSize(m_Anims.GetSize() + pChild->m_Anims.GetSize()), LT_MEM_TYPE_MODEL);
		if(!bSetSizeResult)
		{
			LTStrCpy(pErrStr, "Out of memory.", nErrStrLen);
			return NULL;
		}

		for(i=0; i < pChild->m_Anims; i++)
		{
			pAnim = GetAnimInfo(pChildModel->m_AnimOffset+i);
			*pAnim = pChild->m_Anims[i];
		}
	}

	//make sure that our visible radius will encompass that of the child model
	m_VisRadius = LTMAX(m_VisRadius, pChild->m_VisRadius);

	return pChildModel;
}

bool Model::InitChildInfo(uint32 index, ChildInfo *pChildModel, Model *pModel, const char *pFilename)
{
	if(index >= MAX_CHILD_MODELS)
		return false;

	pChildModel->m_pModel = pModel;
	pChildModel->m_AnimOffset = 0;

	if(pModel)
	{
		if(pModel != this)
		{
			pModel->AddRef();
		}
	}

	m_ChildModels[index] = pChildModel;
	return true;
}


ChildInfo* Model::FindChildModel(Model *pModel, uint32 & index )
{
	uint32 i;

	for(i=0; i < NumChildModels(); i++)
	{
		index = i ;
		if(GetChildModel(i)->m_pModel == pModel)
			return GetChildModel(i);
	}
	
	return NULL;
}


ModelSocket* Model::FindSocket(const char *pName, uint32 *index)
{
	uint32 i;
	ModelSocket *pSocket;


	for(i=0; i < NumSockets(); i++)
	{
		pSocket = GetSocket(i);

		if(stricmp(pSocket->GetName(), pName) == 0)
		{
			if(index)
				*index = i;

			return pSocket;
		}
	}

	return NULL;
}


// Dummy load function (for the server mostly) - just skips all render dependant data.
bool CDIModelDrawable::Load(ILTStream& file, LTB_Header &)
{
	uint32 iObjSize; file.Read(&iObjSize,sizeof(iObjSize));
	uint32 iPos; file.GetPos(&iPos); file.SeekTo(iPos + iObjSize);
	return true;
}


#if(MODEL_OBB)

	// GetCopyOfOBBSet copies the current set in to preallocated array.
uint32	Model::GetNumOBB()
{
	return m_NumOBBs;
}

void Model::GetCopyOfOBBSet( ModelOBB *RetModelOBB )
{
	for(uint32 i = 0 ; i < m_NumOBBs ; i++ )
	{
		RetModelOBB[i] = m_ModelOBBs[i];
	}
}

#endif

using namespace std;

// ------------------------------------------------------------------------
// CModelMgr 
// ------------------------------------------------------------------------
CModelMgr g_ModelMgr ;
// ------------------------------------------------------------------------
CModelMgr::CModelMgr()
{

}

CModelMgr::~CModelMgr()
{
	// report potential errors.
	std::set<Model*>::iterator it;
	
	it = m_Models.begin() ;
	
	for( ; it != m_Models.end() ; it++ )
	{
		DEBUG_MODEL_REZ( ("model-rez: unfreed model [%d] %s", (*it), (*it)->GetFilename()) );
	}
}

Model* CModelMgr::Find( const char *filename )
{
	if(strlen(filename))
	{
		char pszFilename[_MAX_PATH + 1];
		CHelpers::FormatFilename(filename, pszFilename, _MAX_PATH + 1);

		std::set<Model*>::iterator it;
		
		it = m_Models.begin();
		
		for( ; it != m_Models.end() ; it++ )
			if( stricmp( pszFilename, (*it)->GetFilename() ) == 0 )
				return *it ;
	}
	
	return NULL;
}

Model*	CModelMgr::Find( uint16 file_id )
{
	std::set<Model*>::iterator it;
	
	it = m_Models.begin();
	
	for( ; it != m_Models.end() ; it++ )
		if( (*it)->m_FileID == file_id  )
			return *it ;
	return NULL;
}

// ------------------------------------------------------------------------
// ForEach( function, user-data )
//
// ForEach iterates the function fn over all models, passing the model and user data to 
// the function. This is like a lisp map function.
// ------------------------------------------------------------------------
void CModelMgr::ForEach( void (*fn)(const Model &Model, void *user_data), void *user_data)
{
	std::set<Model*>::iterator it;
	
	it = m_Models.begin();
	
	for( ; it != m_Models.end() ;  )
	{
		Model *pModel = *it;
		it++;
		fn( *pModel, user_data );
	}		
}

bool CModelMgr::Add( Model *pModel )
{
	// returns true on successful insertion.
	return m_Models.insert(pModel).second;	
}

// fail if nothing to erase... 
bool	CModelMgr::Remove( Model *pModel )
{
	assert( m_Models.find(pModel) != m_Models.end() );
	return m_Models.erase(pModel) > 0;
}

static bool UncacheServerModel(const Model & model )
{

	return false;
}


void CModelMgr::UncacheServerModels()
{
	std::set<Model*>::iterator it;
	
	it = m_Models.begin();
	
	for( ; it != m_Models.end() ;  )
	{
		Model *pModel = *it ;
		if(( pModel->m_Flags & MODELFLAG_CACHED ) != 0)
		{
			DEBUG_MODEL_REZ(("model-rez: server uncache %s ref(%d)", pModel->GetFilename(), pModel->GetRefCount()));

			pModel->m_Flags &= ~MODELFLAG_CACHED ;
			pModel->Release();

			it = m_Models.begin( );
		}
		else
		{
			it++;
		}
	}		
}

void CModelMgr::UncacheClientModels()
{
	std::set<Model*>::iterator it;
	
	it = m_Models.begin();
	
	for( ; it != m_Models.end() ;  )
	{
		Model *pModel = *it ;
		if(( pModel->m_Flags & MODELFLAG_CACHED_CLIENT ) != 0)
		{
			DEBUG_MODEL_REZ( ("model-rez: client uncachemodel [%d] %s", pModel, pModel->GetFilename()));

			pModel->m_Flags &= ~MODELFLAG_CACHED_CLIENT ;
			pModel->Release();

			it = m_Models.begin( );
		}
		else
		{
			it++;
		}
	}		
}





// the basement
#if(MODEL_OBB)

// ------------------------------------------------------------------------
// RayIntersect( ray-origin, ray-direction (normalize), hit-parameter)
// returns true if intersection happened.
// Test a ray against the obb.
// source : Real-time rendering moller&haines
// ------------------------------------------------------------------------
bool RayIntersect( const ModelOBB &mobb, const LTVector &origin, const LTVector &dir, float &t )
{
	float tmin = -1000;
	float tmax = 1000;

	LTVector p = mobb.m_Pos - origin ;
	float e ;
	float f;
	float hi;
	float t1,t2;

	// 1/2 len vectors
	LTVector axis[3] = { mobb.m_Basis[0] *.5f , mobb.m_Basis[1] *.5f, mobb.m_Basis[2] *.5f } ;


	for( int i = 0 ; i < 3 ; i ++ )
	{
		e = axis[i].Dot(p);
		f = axis[i].Dot(dir);		
		hi= mobb.m_Size[i] * 0.5f ; // get the half size.

		if( fabs(f) > 0.00015 )
		{
			t1 = ( e + hi ) / f ;
			t2 = ( e - hi ) / f ;
			if( t1 > t2 ) {float v = t2 ; t2 = t1 ; t1 = v ; }
			if(t1 > tmin ) tmin = t1 ;
			if(t2 < tmax ) tmax = t2 ;
			if(tmin > tmax ) { 
				 return false ; }
			if(tmax < 0  ) { 
				 return false ; }
			// according to moller & hains this is supposed jump out when the 
			// ray is paralell with the slab... 
			//if( ((-e - hi) > 0 ) || ( (-e + hi) < 0 )) { t = 0 ; return false ; }
		}
	}

	if( tmin > 0 ) { t = tmin ;  return true ; }
	else { t = tmax ;  return true ;}
}



// code taken from M.Gomez's gamasutra article.
// not tested...
///check if two oriented bounding boxes overlap
inline bool obb_overlap
(
 //A
 const LTVector& a, //extents
 const LTVector& Pa, //position
 const LTVector* A, //orthonormal basis
 //B
 const LTVector& b, //extents
 const LTVector& Pb, //position
 const LTVector* B //orthonormal basis
)
{

  //translation, in parent frame
  LTVector v = Pb - Pa;
  //translation, in A's frame
  LTVector T( v.Dot(A[0]), v.Dot(A[1]), v.Dot(A[2]) );
  
  //B's basis with respect to A's local frame
  float R[3][3];
  float ra, rb, t;
  long i, k;
  
  //calculate rotation matrix
  for( i=0 ; i<3 ; i++ )
    for( k=0 ; k<3 ; k++ )
      R[i][k] = A[i].Dot(B[k]); 
  /*ALGORITHM: Use the separating axis test for all 15 potential 
    separating axes. If a separating axis could not be found, the two 
    boxes overlap. */
  
  //A's basis vectors
  for( i=0 ; i<3 ; i++ )  {
    ra = a[i];
    rb = b[0]*fabsf(R[i][0]) + b[1]*fabsf(R[i][1]) + b[2]*fabsf(R[i][2]);
    t = fabsf( T[i] );
    if( t > ra + rb ) 
      return false;
  }
  //B's basis vectors
  for( k=0 ; k<3 ; k++ )  {
    ra = 
      a[0]*fabsf(R[0][k]) + a[1]*fabsf(R[1][k]) + a[2]*fabsf(R[2][k]);
    
    rb = b[k];
    
    t = 
      fabsf( T[0]*R[0][k] + T[1]*R[1][k] + 
     T[2]*R[2][k] );
    
    if( t > ra + rb )
      return false;
  }
  
  //9 cross products
  //L = A0 x B0
  ra = 
    a[1]*fabsf(R[2][0]) + a[2]*fabsf(R[1][0]);
  
  rb = 
    b[1]*fabsf(R[0][2]) + b[2]*fabsf(R[0][1]);
  
  t = 
    fabsf( T[2]*R[1][0] - 
   T[1]*R[2][0] );
  
  if( t > ra + rb )
    return false;
  
  //L = A0 x B1
  ra = 
    a[1]*fabsf(R[2][1]) + a[2]*fabsf(R[1][1]);
  
  rb = 
    b[0]*fabsf(R[0][2]) + b[2]*fabsf(R[0][0]);
  
  t = 
    fabsf( T[2]*R[1][1] - 
   T[1]*R[2][1] );
  
  if( t > ra + rb )
    return false;
  
  //L = A0 x B2
  ra = 
    a[1]*fabsf(R[2][2]) + a[2]*fabsf(R[1][2]);
  
  rb = 
    b[0]*fabsf(R[0][1]) + b[1]*fabsf(R[0][0]);
  
  t = 
    fabsf( T[2]*R[1][2] - 
   T[1]*R[2][2] );
  
  if( t > ra + rb )
    return false;
  
  //L = A1 x B0
  ra = 
    a[0]*fabsf(R[2][0]) + a[2]*fabsf(R[0][0]);
  
  rb = 
    b[1]*fabsf(R[1][2]) + b[2]*fabsf(R[1][1]);
  
  t = 
    fabsf( T[0]*R[2][0] - 
   T[2]*R[0][0] );
  
  if( t > ra + rb )
    return false;
  
  //L = A1 x B1
  ra = 
    a[0]*fabsf(R[2][1]) + a[2]*fabsf(R[0][1]);
  
  rb = 
    b[0]*fabsf(R[1][2]) + b[2]*fabsf(R[1][0]);
  
  t = 
    fabsf( T[0]*R[2][1] - 
   T[2]*R[0][1] );
  
  if( t > ra + rb )
    return false;
//L = A1 x B2
  ra = 
      a[0]*fabsf(R[2][2]) + a[2]*fabsf(R[0][2]);
  rb = 
      b[0]*fabsf(R[1][1]) + b[1]*fabsf(R[1][0]);
  t = 
      fabsf( T[0]*R[2][2] - 
            T[2]*R[0][2] );
  if( t > ra + rb )
      return false;
//L = A2 x B0
  ra = 
      a[0]*fabsf(R[1][0]) + a[1]*fabsf(R[0][0]);
  rb = 
      b[1]*fabsf(R[2][2]) + b[2]*fabsf(R[2][1]);
  t = 
      fabsf( T[1]*R[0][0] - 
            T[0]*R[1][0] );
  if( t > ra + rb )
      return false;
//L = A2 x B1
  ra = 
      a[0]*fabsf(R[1][1]) + a[1]*fabsf(R[0][1]);
  rb = 
      b[0] *fabsf(R[2][2]) + b[2]*fabsf(R[2][0]);
  t = 
      fabsf( T[1]*R[0][1] - 
            T[0]*R[1][1] );
  if( t > ra + rb )
      return false;
//L = A2 x B2
  ra = 
      a[0]*fabsf(R[1][2]) + a[1]*fabsf(R[0][2]);
  rb = 
      b[0]*fabsf(R[2][1]) + b[1]*fabsf(R[2][0]);
  t = 
      fabsf( T[1]*R[0][2] - 
            T[0]*R[1][2] );
  if( t > ra + rb )
      return false;
/*no separating axis found,
  the two boxes overlap */
  return true;
}

bool ModelOBBOverlap(const ModelOBB &me , const ModelOBB &other )
{
	return obb_overlap( me.m_Size, me.m_Pos, me.m_Basis, 
						other.m_Size, other.m_Pos, other.m_Basis);
}

#endif

















