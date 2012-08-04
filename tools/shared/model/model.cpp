// ------------------------------------------------------------------------
// model.cpp
// implementation details of models
// ------------------------------------------------------------------------
#include "bdefs.h"
#include "model.h"
#include "model_ops.h"
#include "transformmaker.h"

#include <set>


#pragma warning (disable:4786)

#define MAX_SKELETON_NODES	256
#define NUM_RADIUS_TWEENS	25
#define DEFAULT_MODEL_VIS_RADIUS	50.0f

#define DEFAULT_MODEL_AMBIENT_LIGHT	0.25f
#define DEFAULT_MODEL_DIR_LIGHT		0.75f


// ------------------------------------------------------------------------
// globals, and console vars 
// ------------------------------------------------------------------------

// Used so we can track if the filename is allocated or not.
static char *g_pNoModelFilename = "";
float g_CV_DefaultDrawIndexedDist = 500.0f;


// Global value to track model memory usage
unsigned long g_ModelMemory = 0;

// ------------------------------------------------------------------------
// File scope classes 
// ------------------------------------------------------------------------


// Used in ParseCommandString.
class SimpleFloatVar
{
public:
	char	*m_pName;
	float	*m_pVal;
};


class TransformMap
{
public:
				TransformMap() {}

				TransformMap(ModelNode *pNode, uint32 transform)
				{
					m_ScratchData = pNode->ScratchData();
					m_OldTransform = m_NewTransform = transform;
				}

	uint32		m_ScratchData;
	uint32		m_OldTransform;
	uint32		m_NewTransform;
};


// ------------------------------------------------------------------------ //
// Helpers.
// ------------------------------------------------------------------------ //

static TransformMap* FindTransformMapByNode(CMoArray<TransformMap> &tMap, ModelNode *pNode)
{
	uint32 i;
	for(i=0; i < tMap.GetSize(); i++)
	{
		if(tMap[i].m_ScratchData == pNode->ScratchData())
			return &tMap[i];
	}
	return LTNULL;
}

static TransformMap* FindTransformMapByOldIndex(CMoArray<TransformMap> &tMap, uint32 index)
{
	uint32 i;
	for(i=0; i < tMap.GetSize(); i++)
	{
		if(tMap[i].m_OldTransform == index)
			return &tMap[i];
	}
	return LTNULL;
}

// Applies pMat to pRoot and adjusts pRoot's children so they end up the same as they were.
static void ApplyMatrixToKeyframes(AnimNode *pRoot, LTMatrix transform, uint32 iKey)
{
	LTMatrix outMat, myRot;
	uint32 i;
	AnimNode *pChild;
	LTVector right, up, forward, pos;
	NodeKeyFrame *pKeyframe;

	// Apply it to the root.
	pRoot->m_KeyFrames[iKey].m_Quaternion.ConvertToMatrix(myRot);
	MatMul(&outMat, &transform, &myRot);
	pRoot->m_KeyFrames[iKey].m_Quaternion.ConvertFromMatrix(outMat);

	for(i=0; i < pRoot->m_pNode->NumChildren(); i++)
	{
		pChild = pRoot->m_Children[i];
		pKeyframe = &pChild->m_KeyFrames[iKey];

		pKeyframe->m_Quaternion.ConvertToMatrix(myRot);
		MatMul(&outMat, &transform, &myRot);
		pKeyframe->m_Quaternion.ConvertFromMatrix(outMat);

		Mat_GetBasisVectors(&myRot, &right, &up, &forward);
		pos.Init();
		pos += right * pKeyframe->m_Translation.x;
		pos += up * pKeyframe->m_Translation.y;
		pos += forward * pKeyframe->m_Translation.z;

		Mat_GetBasisVectors(&outMat, &right, &up, &forward);
		pKeyframe->m_Translation.x = right.Dot(pos);
		pKeyframe->m_Translation.y = up.Dot(pos);
		pKeyframe->m_Translation.z = forward.Dot(pos);
	}
}


static LTBOOL CreateDummyAnimNodes(Model *pModel, ModelAnim *pAnim, 
	ModelNode *pNode, AnimNode *pAnimNode, AnimNode *pParentAnimNode)
{
	uint32 i;
	AnimNode *pNewNode;

	pAnimNode->m_pNode = pNode;
	pAnimNode->m_pParentNode = pParentAnimNode;
	
	pAnimNode->m_KeyFrames.SetSize(1);
	pAnimNode->m_KeyFrames[0].m_Translation.Init();
	pAnimNode->m_KeyFrames[0].m_Quaternion.Init();

	for(i=0; i < pNode->NumChildren(); i++)
	{
		pNewNode = new AnimNode(pAnim, pAnimNode);
		pAnimNode->m_Children.Append(pNewNode);

		if(!CreateDummyAnimNodes(pModel, pAnim, pNode->m_Children[i], 
			pNewNode, pAnimNode))
		{
			return false;
		}
	}

	return true;
}


static void _ChangeVertexReferences(uint32 from, uint32 to, PieceLOD *pNode)
{
	ModelTri *pTri;
	uint32 i, j;

	for(i=0; i < pNode->m_Tris; i++)
	{
		pTri = &pNode->m_Tris[i];

		for(j=0; j < 3; j++)
		{
			if(pTri->m_Indices[j] == from)
				pTri->m_Indices[j] = (uint16)to;
			
			// Decrement if the vertex is greater than the one we're removing since the 
			// array will slide down.
			if(pTri->m_Indices[j] > from)
				pTri->m_Indices[j]--;
		}
	}

}


static void _ChangeVertexWeightReferences(Model *pModel, 
	PieceLOD *pPiece, 
	uint32 iVert,
	uint32 iReplaceWith)
{
	uint32 iNode;
	
	for(iNode=0; iNode < pModel->NumNodes(); iNode++)
	{
		pModel->GetNode(iNode)->ChangeVertexReferences(pPiece, iVert, iReplaceWith);		
	}
}


// Convert the transforms from local to global space.
void _TransformsLtoG(AnimNode *pAnimNode, uint32 iFrame, LTMatrix *pParentTransform)
{
	LTMatrix localTransform, globalTransform;
	uint32 i;

	pAnimNode->m_KeyFrames[iFrame].ConvertToMatrix(localTransform);
	MatMul(&globalTransform, pParentTransform, &localTransform);
	pAnimNode->m_KeyFrames[iFrame].ConvertFromMatrix(globalTransform);

	for(i=0; i < pAnimNode->m_Children; i++)
	{
		_TransformsLtoG(pAnimNode->m_Children[i], iFrame, &globalTransform);
	}
}


// Convert the transforms from global to local space.
// Global = Parent * Local so:
// Local = ~Parent * Global
void _TransformsGtoL(AnimNode *pAnimNode, uint32 iFrame, LTMatrix *pParentTransform)
{
	LTMatrix inverseParent, localTransform, globalTransform;
	uint32 i;

	pAnimNode->m_KeyFrames[iFrame].ConvertToMatrix(globalTransform);
	Mat_InverseTransformation(pParentTransform, &inverseParent);
	MatMul(&localTransform, &inverseParent, &globalTransform);	

	pAnimNode->m_KeyFrames[iFrame].ConvertFromMatrix(localTransform);

	for(i=0; i < pAnimNode->m_Children; i++)
	{
		_TransformsGtoL(pAnimNode->m_Children[i], iFrame, &globalTransform);
	}
}


// Sets the given node's keyframe to identity.
void _ResetNodeKeyFrames(AnimNode *pAnimNode, ModelNode *pNode, uint32 iFrame)
{
	uint32 i;

	if(pAnimNode->m_pNode == pNode)
	{
		pAnimNode->m_KeyFrames[iFrame].Identity();
	}

	for(i=0; i < pAnimNode->m_Children; i++)
	{
		_ResetNodeKeyFrames(pAnimNode->m_Children[i], pNode, iFrame);
	}
}


LTRESULT DefaultLoadChildFn(ModelLoadRequest *pRequest, Model **pModel)
{
	return LT_NOCHANGE;
}


void UpdateDistSqr(CMoArray< LTVector > &verts, float &maxDistSqr)
{
	uint32 i;
	LTVector *pCurVert;
	float testDist;

	pCurVert = verts.GetArray();
	for(i=0; i < verts; i++)
	{
		testDist = pCurVert->MagSqr();
		if(testDist > maxDistSqr)
		{
			maxDistSqr = testDist;
		}
	
		pCurVert++;
	}
}


LTBOOL VerifyChildModel_R(ModelNode *pParentNode, ModelNode *pChildNode, ModelNode* &pErrNode)
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



// ------------------------------------------------------------------------ //
// Exposed helpers.
// ------------------------------------------------------------------------ //

char* model_FindExporterToken(char *pName, char *pToken)
{
	char *pCur, *pCurName, *pCurToken;
	bool bSame;

	pCur = pName;
	while(*pCur)
	{
		if(pCur[0] == '_' && pCur[1] == 'z')
		{
			bSame = true;
			pCurName = &pCur[2];
			pCurToken = pToken;
			while(*pCurToken)
			{
				if(toupper(*pCurToken) != toupper(*pCurName))
				{
					bSame = false;
					break;
				}

				pCurToken++;
				pCurName++;
			}

			if(bSame)
				return &pCur[2];
		}
	
		pCur++;
	}
	
	return LTNULL;
}


// ------------------------------------------------------------------------ //
// ModelStringList.
// ------------------------------------------------------------------------ //

ModelStringList::ModelStringList(LAlloc *pAlloc)
{
	m_pAlloc = pAlloc;
	m_StringList = LTNULL;
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
	m_StringList = LTNULL;
}


char* ModelStringList::AddString(const char *pString)
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

	strcpy(pRet->m_String, pString);
	return pRet->m_String;
}


bool ModelStringList::SetAlloc(LAlloc *pAlloc)
{
	if(m_StringList)
	{
		ASSERT(false);
		return false;
	}

	m_pAlloc = pAlloc;
	return true;
}



// ------------------------------------------------------------------------ //
// AnimTimeRef.
// ------------------------------------------------------------------------ //

bool AnimTimeRef::IsValid()
{
	return m_pModel && 
		m_Prev.m_iAnim < m_pModel->m_Anims && 
		m_Cur.m_iAnim < m_pModel->m_Anims &&
		m_Prev.m_iFrame < m_pModel->GetAnim(m_Prev.m_iAnim)->m_KeyFrames &&
		m_Cur.m_iFrame < m_pModel->GetAnim(m_Cur.m_iAnim)->m_KeyFrames &&
		m_Percent >= 0.0f && m_Percent <= 1.0f;
}

ModelAnim* AnimTimeRef::GetAnim1()
{
	ASSERT(m_Prev.m_iAnim < m_pModel->m_Anims);
	return m_pModel->GetAnim(m_Prev.m_iAnim);
}

ModelAnim* AnimTimeRef::GetAnim2()
{
	ASSERT(m_Cur.m_iAnim < m_pModel->m_Anims);
	return m_pModel->GetAnim(m_Cur.m_iAnim);
}



// ------------------------------------------------------------------------ //
// NewVertexWeight.
// ------------------------------------------------------------------------ //

NewVertexWeight::NewVertexWeight()
{
	m_Vec[0] = m_Vec[1] = m_Vec[2] = m_Vec[3] = 0.0f;
	m_iNode = 0;
}


// ------------------------------------------------------------------------ //
// ModelVert.
// ------------------------------------------------------------------------ //

ModelVert::ModelVert()
{
	m_Weights = LTNULL;
	m_nWeights = 0;
	m_Vec.Init();
	m_Normal.Init();
}


// note :
// this allocates memory that may not be freed.
// in modeledit, the lod builder uses this method 
// memory created here from that is not properly dealloc'd
NewVertexWeight* ModelVert::AddWeight()
{
	NewVertexWeight *pNewList;
	uint32 i;

	pNewList = new NewVertexWeight[m_nWeights+1];
	if(!pNewList)
		return LTNULL;

	for(i=0; i < m_nWeights; i++)
		pNewList[i] = m_Weights[i];

	delete m_Weights;
	m_Weights = pNewList;
	m_nWeights++;

	return &m_Weights[m_nWeights-1];
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
CDefVertexLst::CDefVertexLst( int size, LAlloc *alloc )
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
void CDefVertexLst::setSize( int size , LAlloc *alloc )
{
	/* possible weirdness :
		if m_vVertexList is alloced already and alloc is different from m_pAlloc.
	*/
	m_pAlloc = alloc ;
	m_vVertexList.SetSize2( size, alloc );
}

// get value at index (copy of data )
// index is unchecked
void CDefVertexLst::getValue( int index, float val[3] )    
{
	defVert & dv = m_vVertexList[index];
	memcpy(val, dv.pos, sizeof(float)*3);
}

// get value at index (pointer to actual data )
// note: don't use pointer as the head of an array, defVert may 
// change..
float * CDefVertexLst::getValue( int index )
{
	defVert & dv = m_vVertexList[index];
	return (float*)&dv.pos;
}

// add values at the end of the list
void CDefVertexLst::append( float val[3] )
{
	defVert dv ;
	for( int i = 0 ;i < 3 ; i++ )
		dv.pos[i] = val[i];
	//memcpy( dv.pos, val, sizeof(float)*3);
	//int lsize = m_vVertexList.GetSize();
	//m_vVertexList.Insert2(lsize, dv , m_pAlloc );
	m_vVertexList.Append( dv );
}

// append 
void CDefVertexLst::append( float x, float y, float z)
{
	defVert dv;
	dv.pos[0] = x ;dv.pos[1] = y ;dv.pos[2] = z ;
	//int lsize = m_vVertexList.GetSize();
	//m_vVertexList.Insert2(lsize, dv, m_pAlloc );
	m_vVertexList.Append( dv );
}


// ------------------------------------------------------------------------ //
// ChildModel.
// ------------------------------------------------------------------------ //

ChildInfo::ChildInfo()
{
	m_AnimOffset = 0;
	m_pFilename = "";
	m_pModel = LTNULL;
	m_pParentModel = LTNULL;
	m_bBoundRadiusValid = false;
	m_bTreeValid = true;
}

ChildInfo::~ChildInfo()
{
	Term();
}

// Note this does not seem to delete the child model.
void ChildInfo::Term()
{
	m_pModel = LTNULL;
	m_pFilename = LTNULL;
	m_Relation.Term(GetAlloc());
}


// ------------------------------------------------------------------------ //
// AnimKeyFrame.
// ------------------------------------------------------------------------ //

AnimKeyFrame::AnimKeyFrame()
{
	m_Time = 0;
	m_pString = LTNULL;
	m_KeyType = KEYTYPE_POSITION;
	m_Callback = LTNULL;
	m_pUser1 = LTNULL;
}


// ------------------------------------------------------------------------ //
// AnimNode.
// 
// ------------------------------------------------------------------------ //

// ------------------------------------------------------------------------
// Constructor
// ------------------------------------------------------------------------
AnimNode::AnimNode()
	:m_iPiece(-1){
m_LastKF = 0;
	Clear();
}

AnimNode::AnimNode(ModelAnim *pAnim, AnimNode *pParent)
	:m_iPiece(-1)
{
	Clear();
	m_pAnim = pAnim;
	m_pParentNode = pParent;
	m_LastKF = 0;
}

AnimNode::~AnimNode()
{
	Term();
}

void AnimNode::Clear()
{
	m_pAnim = LTNULL;
	m_pParentNode = LTNULL;
	m_pNode = LTNULL;
	m_KeyFrames = LTNULL;
}

void AnimNode::Term()
{
	m_KeyFrames.Term(GetAlloc());
	DeleteAndClearArray(m_Children, GetAlloc());
}


void AnimNode::SetAnim(ModelAnim *pAnim)
{
	m_pAnim = pAnim;
}


AnimNode* AnimNode::AllocateSameKind(ModelAnim *pAnim, AnimNode *pParent)
{
	return new AnimNode(pAnim, pParent);
}


bool AnimNode::FillAnimNodeList(uint32 &curNodeIndex)
{
	unsigned long i;

	if(!m_pAnim->m_AnimNodes || curNodeIndex >= GetModel()->NumNodes())
	{
		ASSERT(false);
		return false;
	}

	m_pAnim->m_AnimNodes[curNodeIndex] = this;
	++curNodeIndex;

	for(i=0; i < NumChildren(); i++)
	{
		if(!m_Children[i]->FillAnimNodeList(curNodeIndex))
			return false;
	}

	return true;
}


bool AnimNode::SetNodePointers(ModelNode *pNode)
{
	uint32 i;

	m_pNode = pNode;

	if(m_Children.GetSize() != pNode->m_Children.GetSize())
	{
		ASSERT(false);
		return false;
	}

	for(i=0; i < m_Children; i++)
	{
		m_Children[i]->SetNodePointers(pNode->m_Children[i]);
	}

	return true;
}


Model* AnimNode::GetModel()
{
	return m_pAnim->GetModel();
}


bool AnimNode::RemoveKeyFrame(uint32 iKeyFrame)
{
	uint32 i;

	ASSERT(iKeyFrame < NumKeyFrames());

	// Slide our keyframes back one.
	m_KeyFrames.Remove(iKeyFrame);

	// Recurse.
	for(i=0; i < m_Children; i++)
	{
		if(!m_Children[i]->RemoveKeyFrame(iKeyFrame))
			return false;
	}

	return true;
}


uint32 AnimNode::NumKeyFrames()
{
	return m_pAnim->NumKeyFrames();
}


// ---------------------------------------------------------------------------
// GetVertexAnimMemSize();
// returns the total memory (in bytes) size of the vertex animation held in 
// in this node.
// return 0 if there's nothing.
// ---------------------------------------------------------------------------
int		AnimNode::GetVertexAnimMemSize()         
{
	int size =0;
	
	// quick check to see if this is a vertex anim node.
	if( !isVertexAnim() )
		return 0 ;

	for( uint32 i = 0 ;i < m_KeyFrames.GetSize() ; i++ )
	{
		if( m_KeyFrames[i].m_pDefVertexLst != NULL )
				size += m_KeyFrames[i].m_pDefVertexLst->mem_size();
			
	}
	return size ;
}

// ----------------------------------------------------------------------------
// size = GetTotalVertexAnimMemSize()
// 
//  calc the size for all children.
// ----------------------------------------------------------------------------
int	AnimNode::GetTotalVertexAnimMemSize()
{
	int size = 0;

	size = GetVertexAnimMemSize();

	for(uint32 i=0; i < m_Children.GetSize(); i++)
	{
		size += m_Children[i]->GetTotalVertexAnimMemSize();
		
	}
	return size ;
}

bool AnimNode::CopyKeyFrame(uint32 iKeyFrame, AnimNode *pDest)
{
	uint32 i;
	AnimNode *pNewChild;


	if(iKeyFrame >= NumKeyFrames())
	{
		ASSERT(false);
		return false;
	}


	// Add the keyframe.
	ASSERT(!pDest->m_KeyFrames);
	pDest->m_KeyFrames.SetSize(1);
	if(!pDest->m_KeyFrames)
		return false;

	pDest->m_KeyFrames[0] = m_KeyFrames[iKeyFrame];

	// Recurse.
	for(i=0; i < m_Children; i++)
	{
		pNewChild = AllocateSameKind(m_pAnim, pDest);
		if(!pNewChild)
			return false;

		if(!m_Children[i]->CopyKeyFrame(iKeyFrame, pNewChild) || !pDest->m_Children.Append(pNewChild))
		{
			delete pNewChild;
			return false;
		}
	}

	return true;
}


bool AnimNode::PasteKeyFrame(AnimNode *pSrc, uint32 iBefore)
{
	uint32 i;


	// Make sure we're compatible.
	if(!pSrc->m_KeyFrames || pSrc->m_Children.GetSize() != m_Children.GetSize())
		return false;

	//clamp the position to insert to the number of keys
	uint32 nPasteLocation = LTMIN(iBefore, m_KeyFrames.GetSize());

	// Add to the keyframe array.
	m_KeyFrames.Insert(nPasteLocation, pSrc->m_KeyFrames[0]);

	// Recurse.
	for(i=0; i < m_Children; i++)
	{
		if(!m_Children[i]->PasteKeyFrame(pSrc->m_Children[i], iBefore))
			return false;
	}

	return true;
}

NodeKeyFrame *AnimNode::GetKeyFrame( uint32 time )
{
	/*
	if time is greater than or equal to last-kf and less then last-kf +1 then 
	return last-kf 
	else if time greater or equal to last-kf +1
		while
			if time is equal or greater then last-kf +1 and less then last-kf +2 then 
			last-kf = last-kf +1 
			return last-kf ;
	else if time less then last-kf 
		set last-kf = 0 ;
		search forward for proximate key.
 

// naive search for keyf.
	uint32 cur_key=1;
	uint32 size = m_KeyFrames.GetSize();

	if( size >= 2)
	{
		NodeKeyFrame *pNKF0 = m_KeyFrames[0];
		
		while(cur_key<= (size-1))
		{
			NodeKeyFrame *pNKF1 = m_KeyFrames[cur_key];

			if( pNKF0->m_Time >= time && pNKF1->m_Time < time )
				break;
			else
			{
				cur_key++ ;
				pNKF0 = pNKF1 ;
				pNKF1 = m_KeyFrames[ cur_key ] ;
			}
		}

		return pNKF0;
	}
 */
	
	return NULL ;
}

// ------------------------------------------------------------------------ //
// ModelAnim functions.
// ------------------------------------------------------------------------ //

ModelAnim::ModelAnim(Model *pModel)
{
	m_pModel = pModel;
	m_pName = "";
	m_AnimNodes = LTNULL;
	m_ModelEditWeightSet = INVALID_MODEL_WEIGHTSET;
	m_InterpolationMS = 200;
	m_CompressionType =NONE;

	m_pRootNode = &m_DefaultRootNode;
	m_pRootNode->SetAnim(this);
}


ModelAnim::~ModelAnim()
{
	Term();
}


void ModelAnim::Term()
{
	if(m_AnimNodes)
	{
		LDelete_Array(GetAlloc(), m_AnimNodes, m_pModel->NumNodes());
		m_AnimNodes = LTNULL;
	}

	m_KeyFrames.Term(GetAlloc());
	m_DefaultRootNode.Term();
	DeleteRootNode();
}


void ModelAnim::SetRootNode(AnimNode *pNode)
{
	DeleteRootNode();

	m_pRootNode = pNode;
}


void ModelAnim::SetName(const char *pName)
{
	m_pName = m_pModel->AddString(pName);
}


void ModelAnim::SetModel(Model *pModel)
{
	m_pName = pModel->AddString(m_pName);
	m_pModel = pModel;

	// Resetup the AnimNode's ModelNode pointers.
	GetRootNode()->SetNodePointers(m_pModel->GetRootNode());
}


ModelAnim* ModelAnim::AllocateSameKind(Model *pModel)
{
	return new ModelAnim(pModel);
}


void ModelAnim::DeleteRootNode()
{
	if(m_pRootNode && m_pRootNode != &m_DefaultRootNode)
	{
		delete m_pRootNode;
	}

	m_pRootNode = &m_DefaultRootNode;
}


bool ModelAnim::PrecalcNodeLists(bool bRebuild)
{
	return BuildAnimNodeList(bRebuild);
}


bool ModelAnim::BuildAnimNodeList(bool bRebuild)
{
	uint32 curNodeIndex;

	if(!bRebuild && m_AnimNodes)
		return true;

	if(m_AnimNodes)
	{
		LDelete_Array(GetAlloc(), m_AnimNodes, GetModel()->NumNodes());
	}

	m_AnimNodes = LNew_Array(GetAlloc(), AnimNode*, m_pModel->NumNodes(), false);
	if(!m_AnimNodes)
		return false;

	curNodeIndex = 0;
	return GetRootNode()->FillAnimNodeList(curNodeIndex);
}


ModelAnim* ModelAnim::CopyKeyFrame(uint32 iKeyFrame)
{
	ModelAnim *pRet;

	pRet = LTNULL;

	if(iKeyFrame >= NumKeyFrames())
	{
		ASSERT(false);
		goto Error;
	}

	pRet = AllocateSameKind(m_pModel);
	if(!pRet)
		goto Error;

	// Copy our keyframe.
	if(!pRet->m_KeyFrames.Append(m_KeyFrames[iKeyFrame]))
	{
		goto Error;
	}

	// Recursively add keyframes.
	pRet->SetRootNode(GetRootNode()->AllocateSameKind(pRet, LTNULL)); // So it's the same class..
	if(!GetRootNode()->CopyKeyFrame(iKeyFrame, pRet->GetRootNode()))
		goto Error;

	return pRet;

// Errors go here.
Error:;

	if(pRet)
		delete pRet;

	return LTNULL;
}


bool ModelAnim::PasteKeyFrame(ModelAnim *pFrame, uint32 iBefore)
{
	if(!pFrame || pFrame->NumKeyFrames() == 0)
		return false;

	if(!GetRootNode()->PasteKeyFrame(pFrame->GetRootNode(), iBefore))
		return false;

	uint32 nPasteLocation = LTMIN(iBefore, m_KeyFrames.GetSize());

	// NOTE: it MUST do this last so the AnimNodes have a correct keyframe count
	// while they do their paste.
	if(!m_KeyFrames.Insert(nPasteLocation, pFrame->m_KeyFrames[0]))
		return false;
	
	return true;
}


ModelAnim* ModelAnim::CutKeyFrame(uint32 iKeyFrame)
{
	ModelAnim *pRet;

	pRet = CopyKeyFrame(iKeyFrame);
	if(pRet)
	{
		if(RemoveKeyFrame(iKeyFrame))
		{
			return pRet;
		}
		else
		{
			delete pRet;
		}
	}

	return false;
}


bool ModelAnim::RemoveKeyFrame(uint32 iKeyFrame)
{
	uint32 i, nShortenedBy;

	// there must be at least two keyframes in an animation
	if(m_KeyFrames.GetSize() <= 2)
	{
		return false;
	}

	// if it's the first or last frame, adjust the animation time
	if(iKeyFrame == 0)
	{
		nShortenedBy = m_KeyFrames[1].m_Time;
		for(i = 1; i < m_KeyFrames; i++)
		{
			m_KeyFrames[i].m_Time -= nShortenedBy;
		}
	}
	
	// now remove the actual translation and rotation keyframe from each model node
	if(!GetRootNode()->RemoveKeyFrame(iKeyFrame))
		return false;

	// Remove it from our list of keyframes.
	// This MUST come last so the AnimNodes have a correct keyframe count while
	// they remove their keyframes.
	m_KeyFrames.Remove(iKeyFrame);

	return true;
}

uint32 ModelAnim::GetAnimTime()
{
	if(m_KeyFrames.GetSize() > 0)
		return m_KeyFrames.Last().m_Time;
	else
		return 0;
}


bool ModelAnim::CopyAnimNode( AnimNode* pSrc, AnimNode* pDest )
{
	ASSERT( !pDest->m_KeyFrames );
	if( !pDest->m_KeyFrames.SetSize2( NumKeyFrames(), GetAlloc() ) )
		return false;

	pDest->m_iPiece = pSrc->m_iPiece;

	for( uint32 curFrame = 0; curFrame < NumKeyFrames(); curFrame++ )
	{
		pDest->m_KeyFrames[curFrame] = pSrc->m_KeyFrames[curFrame];

		if( pSrc->m_KeyFrames[curFrame].m_pDefVertexLst )
		{
			uint32 size = pSrc->m_KeyFrames[curFrame].m_pDefVertexLst->size();

			CDefVertexLst* pDefVertLst = new CDefVertexLst;
			pDefVertLst->setSize( size, GetAlloc() );
		
			memcpy( pDefVertLst->getArray(), pSrc->m_KeyFrames[curFrame].m_pDefVertexLst->getArray(), sizeof(float) * 3 * size );

			pDest->m_KeyFrames[curFrame].m_pDefVertexLst = pDefVertLst;
		}
		else
			pDest->m_KeyFrames[curFrame].m_pDefVertexLst = NULL;
	}
	
	// recurse through children
	if( !pDest->m_Children.SetSizeInit4( pDest->m_pNode->NumChildren(), LTNULL, GetAlloc() ) )
		return false;

	for( uint32 curChild = 0; curChild < pDest->m_pNode->NumChildren(); curChild++ )
	{
		AnimNode* pChild = LNew_2P( GetAlloc(), AnimNode, pDest->GetAnim(), pDest );
		if( !pChild )
			return false;

		pChild->m_pNode = pDest->m_pNode->m_Children[curChild];

		if( !CopyAnimNode( pSrc->m_Children[curChild], pChild ) )
		{
			LDelete( GetAlloc(), pChild );
			DeleteAndClearArray2( pDest->m_Children, GetAlloc() );
			return false;
		}

		pDest->m_Children[curChild] = pChild;
	}

	return true;
}


bool ModelAnim::CopyAnim( ModelAnim* pSrc )
{
	Term();

	// copy the name of the animation
	if( !(m_pName = m_pModel->AddString( pSrc->m_pName )) )
		return false;

	m_ModelEditWeightSet = pSrc->m_ModelEditWeightSet;
	m_InterpolationMS = pSrc->m_InterpolationMS;

	// copy keyframes
	uint32 numKeyFrames = pSrc->m_KeyFrames.GetSize();

	if( !m_KeyFrames.SetSize2( numKeyFrames, GetAlloc() ) )
		return false;

	for( uint32 curKeyFrame = 0; curKeyFrame < numKeyFrames; curKeyFrame++ )
	{
		AnimKeyFrame* pThisKeyFrame = &m_KeyFrames[curKeyFrame];
		AnimKeyFrame* pSrcKeyFrame = &pSrc->m_KeyFrames[curKeyFrame];
		pThisKeyFrame->m_Time = pSrcKeyFrame->m_Time;
		if( !(pThisKeyFrame->m_pString = m_pModel->AddString( pSrcKeyFrame->m_pString )) )
			return false;
	}

	// copy the transform tree
	GetRootNode()->m_pNode = m_pModel->GetRootNode();

	CopyAnimNode( pSrc->GetRootNode(), GetRootNode() );

	if( !PrecalcNodeLists() )
		return false;

	return true;
};



// ------------------------------------------------------------------------ //
// AnimInfo.
// ------------------------------------------------------------------------ //

AnimInfo::AnimInfo()
{
	m_pAnim = LTNULL;
	m_pChildInfo = LTNULL;
	
	// Init to a wierd number.. animators should always set it.
	m_vDims.Init(128.0f, 16.0f, 16.0f);
	m_vTranslation.Init();
}
		
		
Model* AnimInfo::GetAnimOwner()
{
	if(m_pChildInfo->m_pModel == m_pAnim->GetModel())
	{
		return m_pChildInfo->m_pModel;
	}
	else
	{
		ASSERT(false);
		return LTNULL;
	}
}



// ------------------------------------------------------------------------ //
// ModelNode.
// ------------------------------------------------------------------------ //

ModelNode::ModelNode()
{
	Clear();
	m_pModel = LTNULL;
	
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
	DisableOBB();
	m_EvalState = kIgnore ;
}


void ModelNode::SetName(char *pName)
{
	m_pName = m_pModel->AddString(pName);
}


ModelNode* ModelNode::AllocateSameKind(Model *pModel)
{
	return new ModelNode(pModel);
}


void ModelNode::SetModel(Model *pModel)
{
	uint32 i;


	m_pModel = pModel;
	m_pName = m_pModel->AddString(m_pName);

	for(i=0; i < m_Children; i++)
	{
		m_Children[i]->SetModel(pModel);
	}
}


uint32 ModelNode::CalcNumNodes()
{
	int total;
	unsigned long i;

	total = 1;
	for(i=0; i < NumChildren(); i++)
	{
		total += m_Children[i]->CalcNumNodes();
	}

	return total;
}


bool ModelNode::FillNodeList(uint32 &curNodeIndex)
{
	unsigned long i;

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


void ModelNode::SetOBB( const SOBB &new_obb )
{
	m_OBB = new_obb;
	m_OBB.m_Size.x = fabsf( m_OBB.m_Size.x );
	m_OBB.m_Size.y = fabsf( m_OBB.m_Size.y );
	m_OBB.m_Size.z = fabsf( m_OBB.m_Size.z );
}



// ------------------------------------------------------------------------ //
// PieceLOD.
// ------------------------------------------------------------------------ //

PieceLOD::PieceLOD(Model *pModel)
{					 
	Init(pModel);
}


PieceLOD::PieceLOD()
{					 
	Init(LTNULL);
}


void PieceLOD::Init(Model *pModel)
{
	m_pModel= pModel;

	m_iRenderStyle = 0;
	m_nNumTextures = 1;
	m_nRenderPriority = 0;

	m_iTextures[0] = 0;m_iTextures[1] = 1;
	m_iTextures[2] = 2;m_iTextures[3] = 3;

}

PieceLOD::~PieceLOD()
{	
	// its possible that there are no model back pointer, in cases
	// where the piecelod is a temporary.
	if( m_pModel != NULL )
	{
		m_Tris.Term(GetAlloc());
		m_Verts.Term(GetAlloc());
	}
	else 
	{
		m_Tris.Term();
		m_Verts.Term();
	}
}

// copy the material info from another piece lod
bool PieceLOD::SetMaterialInfo( const PieceLOD& sourceLOD )
{
	m_iRenderStyle		= sourceLOD.m_iRenderStyle;
	m_nRenderPriority	= sourceLOD.m_nRenderPriority;
	m_nNumTextures		= sourceLOD.m_nNumTextures;
	m_iTextures[0]		= sourceLOD.m_iTextures[0];
	m_iTextures[1]		= sourceLOD.m_iTextures[1];
	m_iTextures[2]		= sourceLOD.m_iTextures[2];
	m_iTextures[3]		= sourceLOD.m_iTextures[3];

	return true;
}

// ------------------------------------------------------------------------
// CalcUsedNodeList()
// Generate a list of nodes used by this geometry. These nodes must be the 
// most distal nodes in the hierarchy. These distal or the leaves in the sub
// tree used by the geometry is all we need to form an evaluation path between
// the root and the leaf.
// ------------------------------------------------------------------------
void PieceLOD::CalcUsedNodeList()
{
	uint32 iVert, iWeight ;
	ModelVert *pVert ;

	std::set<uint32>			node_set ;

	for(iVert=0; iVert < m_Verts; iVert++)
	{
		pVert = &m_Verts[iVert];

		for(iWeight=0; iWeight < pVert->m_nWeights; iWeight++)
			node_set.insert(pVert->m_Weights[iWeight].m_iNode);
	}

	std::set<uint32>::iterator set_it = node_set.begin() ;
	
	// create set of terminal nodes for finding paths.
	for( ; set_it != node_set.end() ; set_it++ )
	{
		uint32 iNode  = *set_it ;
		ModelNode *pModelNode = m_pModel->GetNode(iNode);
	
		// check if children are in the set.
		// if none of the children are in the set, add the node to the final list.
		uint32 nChildren = pModelNode->m_Children.GetSize();
		uint32 iChild ;
		bool   IsTerminalNode = true ;
		for( iChild = 0 ; iChild < nChildren ; iChild++ )
		{		
			// if we find a child that is in the set, quit the search.
			if( node_set.find( pModelNode->m_Children[iChild]->m_NodeIndex ) != node_set.end() )
			{	
				IsTerminalNode = false  ;
				continue ;
			}
		}
		
		// if all the children didn't have a parent in the mesh's bone list, add this node
		// as a terminal node.
		if(IsTerminalNode)
		{
			m_UsedNodeList.push_back(*set_it);	
		}
	}	
}

// ------------------------------------------------------------------------ //
// ModelPiece.
// ------------------------------------------------------------------------ //

ModelPiece::ModelPiece(Model *pModel)
{
	m_pModel = pModel;
	m_Name[0] = 0;
	m_LODWeight = 1.0f;

	m_isVA = 0;
	m_vaAnimNodeIdx = -1 ;
	m_MinLODOffset =  m_MaxLODOffset = 0;


}


ModelPiece::~ModelPiece()
{
	Term();
}


void ModelPiece::Term()
{
	m_LODs.Term(GetAlloc());
}


void ModelPiece::SetName(const char *pName)
{
	SAFE_STRCPY(m_Name, pName);
}


// ------------------------------------------------------------------------
// AddNullLOD
// adds a new LOD that has no associated shape. For more info see AddLOD
//
// return 1 on success, 0 on fail.
// it will fail if at_dist is < 0
// ------------------------------------------------------------------------
uint32 ModelPiece::AddNullLOD(float at_dist, bool append_only )
{
	PieceLOD LOD(GetModel());
	LOD.SetName("NULL");
	LOD.m_nNumTextures = 0;

	return AddLOD(LOD, at_dist, append_only);
}


// ------------------------------------------------------------------------
// AddLOD
// puts the new lod at the distance specified, if the dist is beyond the last
// one, append it.
// if append_only is true (defaults to false) then the lod is appended no
// matter what the distance is (must call SortLODs() before use)
//
// return 1 on success, 0 on fail.
// it will fail if at_dist is < 0
// ------------------------------------------------------------------------
uint32 ModelPiece::AddLOD( PieceLOD &pPiece , float at_dist, bool append_only )
{
	if( at_dist < 0 ) 
		return 0;

	if( !append_only )
	{
		for( uint32 i = 0 ; i < m_LODDists.GetSize() ; ++i )
		{
			if(at_dist <= m_LODDists[i] )
			{  
				m_LODs.Insert(i, pPiece);
				m_LODDists.Insert(i,at_dist);
				return 1;
			}
		}
	}

	// ok then, append it to the end.
	PieceLOD lod ;
	m_LODs.Append(lod);
	m_LODs.Last() = pPiece ;
	m_LODDists.Append(at_dist);
	m_MaxLODOffset = m_LODDists.GetSize() - 1;

	return 1;
}


// removes the specified LOD and slides higher LODs down
bool ModelPiece::RemoveLOD( uint32 iLOD )
{
	int numLODs = m_LODs.GetSize();

	// only one remaining LOD or the specified LOD was out of range
	if( (numLODs <= 1) || (iLOD >= m_LODs.GetSize()) )
		return false;

	ASSERT( m_LODs.GetSize() == m_LODDists.GetSize() );
	m_LODs.Remove( iLOD );
	m_LODDists.Remove( iLOD );

	return true;
}


// sets the distance for the specified lod
// caller should call SortLODs after setting LOD distances to make
// sure that the lods are in the correct order
// returns false if the lod or dist aren't valid
bool ModelPiece::SetLODDist( uint32 lod, float dist )
{
	if( (dist < 0.0f) || (lod >= m_LODs.GetSize()) )
		return false;

	m_LODDists[lod] = dist;

	return true;
}


// sorts lod pieces by distance
bool ModelPiece::SortLODs( void )
{
	CMoArray<PieceLOD, NoCache>	sorted;
	CMoArray<float, NoCache> sortedDists; 

	for( uint32 i = 0; i < m_LODs.GetSize(); i++ )
	{
		bool inserted = false;
		float dist = m_LODDists[i];
		ASSERT( dist >= 0.0f );

		for( uint32 j = 0; j < sortedDists.GetSize(); j++ )
		{
			if( dist <= sortedDists[j] )
			{
				sorted.Insert( j, m_LODs[i] );
				sortedDists.Insert( j, dist );
				inserted = true;
				break;
			}
		}

		// append it to the end if not inserted
		if( !inserted )
		{
			PieceLOD lod;
			sorted.Append( lod );
			sorted.Last() = m_LODs[i];
			sortedDists.Append( dist );
		}
	}

	m_LODs.CopyArray( sorted );
	m_LODDists.CopyArray( sortedDists );

	return true;
}


// T.F ModelPiece // no longer valid Move this to PIECE-LOD
bool ModelPiece::FindTriByPosition(LTVector *pVertPositions, 
									 LTVector *pVert1, 
									 LTVector *pVert2, 
									 LTVector *pVert3, 
									 uint32 &triIndex)
{
		// T.F ModelPiece 
	ASSERT(FALSE && " this is no longer valid " );
#if(0)
	uint32 i;
	ModelTri *pTri;

	for(i=0; i < m_Tris; i++)
	{
		pTri = &m_Tris[i];

		if(	pVertPositions[pTri->m_Indices[0]].NearlyEquals(*pVert1, 0.1f) && 
			pVertPositions[pTri->m_Indices[1]].NearlyEquals(*pVert2, 0.1f) &&
			pVertPositions[pTri->m_Indices[2]].NearlyEquals(*pVert3, 0.1f))
		{
			triIndex = i;
			return true;
		}
	}
#endif
	return true;
}


bool ModelPiece::CopyUVs(ModelPiece *pSrc)
{
	// T.F ModelPiece Move To PieceLOD
	ASSERT(FALSE );

#if(0)
	uint32 i, j;

	// Make sure the nodes match.
	if(pSrc->m_Tris.GetSize() != m_Tris.GetSize())
	{
		return false;
	}

	for(i=0; i < m_Tris; i++)
	{
		for(j=0; j < 3; j++)
		{
			m_Tris[i].m_UVs[j] = pSrc->m_Tris[i].m_UVs[j];
		}
	}
#endif
	return true;
}


bool PieceLOD::ReplaceVertex(uint32 iVert, uint32 iReplaceWith)
{
	// T.F ModelPiece 
	ASSERT(FALSE);

	if(iVert >= m_Verts.GetSize() || iReplaceWith >= m_Verts.GetSize())
		return false;

	_ChangeVertexReferences(iVert, iReplaceWith, this);
	
	// (in case this is in an exporter, we have to update the EnvelopeInfo vertex refs).
	_ChangeVertexWeightReferences(GetModel(), this, iVert, iReplaceWith);
	
	m_Verts.Remove(iVert);
	return true;
}

// remap node indices to match the node indices of destModel
// returns false if matching nodes aren't found
bool PieceLOD::RemapNodeIndices( Model* destModel )
{
	uint32 numNodes, newIndex, i, j;
	bool retVal = true;

	numNodes = m_pModel->NumNodes();
	ASSERT( numNodes );

	// array mapping node indices in one model to those in the destination model
	uint32* mapping = new uint32[numNodes];

	for( i = 0; i < numNodes; i++ )
	{
		ModelNode* node = m_pModel->GetNode( i );
		char* nodeName = node->GetName();

		// check for the matching node in the destination model
		if( destModel->FindNode( nodeName, &newIndex ) )
			mapping[i] = newIndex;		// good mapping found
		else
			mapping[i] = 0xffffffff;	// no mapping found
	}

	for( i = 0; retVal && (i < m_Verts); i++ )
	{
		for( j = 0; j < m_Verts[i].m_nWeights; j++ )
		{
			// remap the node index
			uint32& weightNode = m_Verts[i].m_Weights[j].m_iNode;
			ASSERT( weightNode < numNodes );
			weightNode = mapping[weightNode];

			// found a node that doesn't match any in the destination
			if( weightNode == 0xffffffff )
			{
				retVal = false;
				break;
			}
		}
	}

	delete [] mapping;

	return retVal;
}


// ------------------------------------------------------------------------ //
// WeightSet.
// ------------------------------------------------------------------------ //

WeightSet::WeightSet(Model *pModel)
{
	m_Name[0] = 0;
	m_pModel = pModel;
}

WeightSet::~WeightSet()
{
	m_Weights.Term(GetAlloc());
}

void WeightSet::SetName(const char *pName)
{
	SAFE_STRCPY(m_Name, pName);
}

bool WeightSet::InitWeights(uint32 nWeights)
{
	return m_Weights.SetSizeInit2(nWeights, 0.0f) == LTTRUE;
}


// ------------------------------------------------------------------------ //
// ModelSocket
// ------------------------------------------------------------------------ //

ModelSocket::ModelSocket()
{
	m_Name[0] = 0;
	m_iNode = 0;
	m_Pos.Init();
	m_Rot.Init();
	m_Scale.Init(1.0f,1.0f,1.0f);
	m_pAttachment = LTNULL;
}

void ModelSocket::SetName(const char *pName)
{
	SAFE_STRCPY(m_Name, pName);
}




// ------------------------------------------------------------------------ //
// Model functions.
// ------------------------------------------------------------------------ //

Model::Model(LAlloc *pAlloc, LAlloc *pDefAlloc) :
	m_StringList(pAlloc)
{
	m_pAlloc = pAlloc;
	m_pDefAlloc = pDefAlloc;

	m_GlobalRadius = 96.0f;
	m_Link.m_pData = this;

	m_pFilename = g_pNoModelFilename;

	m_nTotalVerts = 0;
	m_Flags = 0;
	m_RefCount=0;
	m_FlatNodeList = LTNULL;

	m_nDWordNodes = 0;
	
	m_VisRadius = DEFAULT_MODEL_VIS_RADIUS;
	m_bNoAnimation = false;
	m_AmbientLight = DEFAULT_MODEL_AMBIENT_LIGHT;
	m_DirLight = DEFAULT_MODEL_DIR_LIGHT;
	m_bFOVOffset = false;
	m_bSpecularEnable = false;
	
	m_bShadowEnable = false;
	m_fShadowProjectLength = 200.0f;
	m_fShadowLightDist = 200.0f;
	m_fShadowSizeX = 50.0f;
	m_fShadowSizeY = 50.0f;
	m_vShadowCenterOffset.Init();

    m_bRigid = false;
        
	m_iNormalRefNode = INVALID_MODEL_NODE;
	m_iNormalRefAnim = INVALID_MODEL_ANIM;
	m_bNormalRef = false;

	m_Transforms = LTNULL;
//	m_DDTransforms = LTNULL;
	m_CommandString = "";

	m_pRootNode = &m_DefaultRootNode;
	m_pRootNode->SetModel(this);

	m_pMovementNode = NULL;

	m_nChildModels = 1;
	m_ChildModels[0] = &m_SelfChildModel;
	GetSelfChildModel()->m_pModel = GetSelfChildModel()->m_pParentModel = this;

	SetSaveIndex((uint32)rand());

	// Memory tracking
	g_ModelMemory += sizeof(Model);

	m_CompressionType  = 0;
	m_bExcludeGeom = false;
		
}


Model::~Model()
{
	ASSERT(m_RefCount == 0);
	Term(true);

	// Memory tracking
	g_ModelMemory -= sizeof(Model);
	g_ModelMemory -= m_BlockAlloc.GetBlockSize();
}
						  

void Model::Term(bool bDeleteChildModels)
{
	if(m_pRootNode != &m_DefaultRootNode)
	{
		LDelete(GetAlloc(), m_pRootNode);
	}
	m_DefaultRootNode.Term();
	m_pRootNode = &m_DefaultRootNode;
	DeleteAndClearArray2(m_Pieces, GetAlloc());

	m_pMovementNode = NULL;

	TermAnims();
	TermChildModels(bDeleteChildModels);
	DeleteAndClearArray2(m_Sockets, GetAlloc());
	DeleteAndClearArray2(m_WeightSets, GetAlloc());
	m_Transforms.Term(GetAlloc());

	m_VertexWeights.Term(GetAlloc());

	m_FlatNodeList.Term(GetAlloc());

	// dfree all the strings.
	m_StringList.Term();

	FreeFilename();


	m_nTotalVerts = 0;
}


void Model::TermAnims()
{
	uint32 i;
	ModelAnim *pAnim;

	// Delete all the anims we own.
	for(i=0; i < m_Anims; i++)
	{
		pAnim = GetAnim(i);

		if(pAnim->GetModel() == this)
		{
			LDelete(GetAlloc(), pAnim);
		}
	}

	m_Anims.Term(GetDefAlloc());
}


void Model::TermChildModels(bool bDeleteChildModels)
{
	uint32 i;
	ChildInfo *pChildModel;

	for(i=0; i < NumChildModels(); i++)
	{
		pChildModel = m_ChildModels[i];

		if(!pChildModel)
			continue;

		// Possibly free the model itself.
		if(pChildModel != GetSelfChildModel())
		{
			if(pChildModel->m_pModel && pChildModel->m_pModel != this)
			{

				if(bDeleteChildModels && pChildModel->m_pModel->IsFreeable())
					delete pChildModel->m_pModel;
			}
		}

		pChildModel->Term();

		// Free the ChildModel object.
		if(pChildModel != GetSelfChildModel())
		{
			if(pChildModel != &m_SelfChildModel)
			{
				LDelete(GetAlloc(), pChildModel);
			}
		}
	}

	m_nChildModels = 1;
	m_ChildModels[0] = &m_SelfChildModel;
}

void Model::AddTextureName( uint32 index, const char *filename )
{
	m_TextureIndexMap[ index ]  = filename ;
}

// returns false if we could not find the index.
bool Model::GetTextureName( uint32 index , std::string & name) 
{
	name = m_TextureIndexMap[ index ];

	if( m_TextureIndexMap[ index ].size() == 0 )
		return false ;

	return true ;
}

void Model::SetRootNode(ModelNode *pRoot)
{
	// Delete the old one.
	if(m_pRootNode && m_pRootNode != &m_DefaultRootNode)
		delete m_pRootNode;

	m_pRootNode = pRoot;
}


bool Model::AddWeightSet(WeightSet *pSet)
{
	return m_WeightSets.Append(pSet) == LTTRUE;
}


void Model::RemoveWeightSet(uint32 index)
{
	if(index >= NumWeightSets())
	{
		ASSERT(false);
		return;
	}

	delete m_WeightSets[index];
	m_WeightSets.Remove(index);
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
	uint32 i, curOffset;
	ModelAnim *pAnim;

	
	// BuildTransformList must be called first.
	if(!BuildTransformList(bRebuild) || !BuildFlatNodeList(bRebuild))
	{
		return false;
	}

	// find the movement node if it exists
	uint32 numRootChildren;
	m_pMovementNode = NULL;
	ModelNode* pRootNode = GetRootNode();
	if( pRootNode && (numRootChildren = pRootNode->NumChildren()) )
	{
		for( i = 0; i < numRootChildren; i++ )
		{
			// try to find an immediate child named movement
			ModelNode* pCurChild = pRootNode->GetChild( i );
			if( stricmp( pCurChild->GetName(), "movement" ) == 0 )
			{
				// movement node must not have any children
				if( pCurChild->NumChildren() == 0 )
				{
					m_pMovementNode = pCurChild;
					break;
				}
			}
		}
	}

	// Precalculate ModelNode::m_VertOffset.
	curOffset = 0;
	for(i=0; i < NumPieces(); i++)
	{
		GetPiece(i)->m_VertOffset = curOffset;
		curOffset += GetPiece(i)->GetLOD(uint32(0))->m_Verts.GetSize();
	}

	for(i=0; i < m_Anims; i++)
	{
		pAnim = GetAnim(i);

		if(pAnim->GetModel() == this)
		{
			if(!pAnim->PrecalcNodeLists(bRebuild))
				return false;
		}
	}

	m_nTotalVerts = CalcNumVerts();
	m_nTotalTris = CalcNumTris();
	return true;
}


bool Model::BuildTransformList(bool bRebuild)
{
	uint32 nNodes;

	if(!bRebuild && m_Transforms.GetSize() == GetRootNode()->CalcNumNodes())
		return true;

	m_Transforms.Term();


	nNodes = GetRootNode()->CalcNumNodes();
	if (!m_Transforms.SetSize2(nNodes, GetAlloc(), true))	// Note: force quad word align so that we can use SimD instructinos...
		return false;
	
	return true;
}


bool Model::BuildFlatNodeList(bool bRebuild)
{
	uint32 nNodes, curNodeIndex;


	if(!bRebuild && m_FlatNodeList.GetSize() == NumNodes())
		return true;
			
	m_FlatNodeList.Term(GetAlloc());

	// Figure out how many there are.
	nNodes = NumNodes();
	if(!m_FlatNodeList.SetSizeInit4(nNodes, LTNULL, GetAlloc()))
		return false;

	// Fill in the list.
	curNodeIndex = 0;
	if(!GetRootNode()->FillNodeList(curNodeIndex))
		return false;

	// Precalculate m_nDWordNodes;
	m_nDWordNodes = nNodes >> 2;
	if(nNodes % 4 != 0)
		++m_nDWordNodes;
	
	GetRootNode()->SetParent_R(NODEPARENT_NONE);
	return true;
}

void Model::ResetNodeEvalState()
{
	for( uint32 i = 0 ; i < m_FlatNodeList.GetSize() ; i++ )
	{
		m_FlatNodeList[i]->m_EvalState = ModelNode::kIgnore;
	}
}


void Model::SetupNodeEvalStateFromPieces(float lod_dist)
{
	uint32 iPiece, iParent ;
	ModelPiece *pPiece ;

	for( iPiece = 0 ; iPiece < NumPieces() ; iPiece++ )
	{
		pPiece = GetPiece(iPiece);

		PieceLOD *pLOD = pPiece->GetLOD(lod_dist);

		for( uint32 iNodeCnt = 0 ; iNodeCnt < pLOD->m_UsedNodeList.size() ; iNodeCnt++)
		{

			uint32 iNode = pLOD->m_UsedNodeList[ iNodeCnt ];
			m_FlatNodeList[ iNode ]->m_EvalState = ModelNode::kEval ;

			iParent = m_FlatNodeList[ iNode ]->m_iParentNode;
			
			while(iParent != NODEPARENT_NONE)
			{
				if( m_FlatNodeList[ iParent ]->m_EvalState != ModelNode::kEval  ) 
				{
					m_FlatNodeList[ iParent ]->m_EvalState = ModelNode::kEval ;
					iParent = m_FlatNodeList[ iParent ]->m_iParentNode ;
				}
				else
					break;	
			}
		}
	}
}

void Model::SetupNodeEvalStateFromNode( uint32 iNode )
{
	if( m_FlatNodeList[ iNode ]->m_EvalState == ModelNode::kEval ) 
		return ;

	uint32 iParent ;
	m_FlatNodeList[ iNode ]->m_EvalState = ModelNode::kEval ;

	iParent = m_FlatNodeList[ iNode ]->m_iParentNode ;
	while(iParent != NODEPARENT_NONE)
	{
		// if the parent node hasn't been evaled or asked to be evaled, evaluate it.
		if( m_FlatNodeList[ iParent ]->m_EvalState == ModelNode::kIgnore )
		{
			m_FlatNodeList[ iParent ]->m_EvalState = ModelNode::kEval ;
			iParent = m_FlatNodeList[ iParent ]->m_iParentNode ;
		}
		else
			break;	
	}
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

	return LTNULL;
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

	return LTNULL;
}


bool Model::SwapPieces(uint32 iPiece1, uint32 iPiece2)
{
	ModelPiece *pTemp;

	// Validate..
	if(iPiece1 >= NumPieces() || iPiece2 >= NumPieces())
		return false;

	// Swap them.
	pTemp = m_Pieces[iPiece1];
	m_Pieces[iPiece1] = m_Pieces[iPiece2];
	m_Pieces[iPiece2] = pTemp;

	// TODO :
	// alert anims that a change in organization has occured...
	return true;
}



char* Model::AddString(const char *pString)
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

	return LTNULL;
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
	
	return LTNULL;
}


void Model::UnrotateAnimNode(ModelAnim *pAnim, AnimNode *pAnimNode)
{
	uint32 j;
	LTVector pos, *pUp, *pRight, *pForward;
	LTMatrix mat;
	NodeKeyFrame *pKeyframe;

	for(j=0; j < pAnim->m_KeyFrames; j++)
	{
		pKeyframe = &pAnimNode->m_KeyFrames[j];
		pKeyframe->m_Quaternion.ConvertToMatrix(mat);
		
		// Adjust the translation to compensate..
		pRight = (LTVector*)&mat.m[0][0];
		pUp = (LTVector*)&mat.m[1][0];
		pForward = (LTVector*)&mat.m[2][0];
		pos.Init();
		pos += *pRight * pKeyframe->m_Translation.x;
		pos += *pUp * pKeyframe->m_Translation.y;
		pos += *pForward * pKeyframe->m_Translation.z;
		
		// Apply this matrix to all the other nodes on this keyframe.
		ApplyMatrixToKeyframes(pAnimNode, mat, j);

		// Reset the root node's rotation.
		//THIS SEEMS TO MESS UP THE TRANSLATIONS..??! VEC_COPY(pKeyframe->m_Translation, pos);
		pKeyframe->m_Quaternion[0] = pKeyframe->m_Quaternion[1] = pKeyframe->m_Quaternion[2] = 0.0f;
		pKeyframe->m_Quaternion[3] = 1.0f;
	}
}


void Model::UnrotateNode(ModelNode *pNode)
{
	uint32 i;

	for(i=0; i < m_Anims; i++)
	{
		ModelAnim *pAnim = GetAnimIfOwner(i);
		if(pAnim)
		{
			UnrotateAnimNode(pAnim, pAnim->m_AnimNodes[pNode->GetNodeIndex()]);
		}
	}
}


bool Model::RemoveNode(ModelNode *pNode)
{
	ModelAnim *pAnim;
	uint32 i, j, k, nodeIndex, oldTransformIndex;
	AnimNode *pAnimNode;
	ModelNode *pParent;
	AnimNode *pParentAnimNode, *pChild;
	CMoArray<TransformMap> tMap;
	TransformMap *pMap;
	LTMatrix parentTransform;
	ModelPiece *pPiece;
	ModelVert *pVert;
	NewVertexWeight *pWeight;

	
	// Must be a null node and can't be the root node.
	if(!pNode || 
		pNode == GetRootNode() || 
		!(pNode->m_Flags & MNODE_REMOVABLE) ||
		AnyVertsWeightedToNode(pNode->m_NodeIndex))
	{
		return false;
	}

	oldTransformIndex = pNode->GetNodeIndex();

	pParent = FindParent(pNode, GetRootNode(), &nodeIndex);
	if(!pParent)
		return false;

	Mat_Identity(&parentTransform);


	// Go Local->Global (it converts back to local below).
	// This effectively preserves the transforms for all the nodes.
	for(i=0; i < m_Anims; i++)
	{
		pAnim = GetAnimIfOwner(i);
		if(pAnim)
		{
			for(j=0; j < pAnim->NumKeyFrames(); j++)
			{
				_TransformsLtoG(pAnim->GetRootNode(), j, &parentTransform);
			}
		}
	}

	
	// Backup the old transform indices of the nodes.
	for(i=0; i < NumNodes(); i++)
	{
		m_FlatNodeList[i]->ScratchData() = (uint8)i; // Give it its identifier.
		tMap.Append(TransformMap(m_FlatNodeList[i], m_FlatNodeList[i]->GetNodeIndex()));
	}


	// Update the AnimNode trees.
	for(i=0; i < m_Anims; i++)
	{
		pAnim = GetAnimIfOwner(i);
		if(pAnim)
		{
			pAnimNode = pAnim->m_AnimNodes[pNode->GetNodeIndex()];
			pParentAnimNode = pAnim->m_AnimNodes[pParent->GetNodeIndex()];

			// Redo the children's parent pointers and add the node's children to its parent.
			for(j=0; j < pNode->NumChildren(); j++)
			{
				pChild = pAnimNode->m_Children[j];

				pParentAnimNode->m_Children.Append(pChild);
				pChild->m_pParentNode = pParentAnimNode;
			}

			// Clear its list (but don't actually delete them since they're in the parent now).
			pAnimNode->m_Children.Term();

			// Delete the node and remove it from its parent's list.
			pParentAnimNode->m_Children.Remove(pParentAnimNode->m_Children.FindElement(pAnimNode));
			delete pAnimNode;
		}
	}

	// Add its children to its parent.
	for(i=0; i < pNode->NumChildren(); i++)
	{
		pParent->m_Children.Append(pNode->m_Children[i]);
	}

	// Clear its list (but don't actually delete them since they're in the parent now).
	pNode->m_Children.Term();

	// Remove the node itself from its parent.
	pParent->m_Children.Remove(pParent->m_Children.FindElement(pNode));
	delete pNode;


	// Redo the node lists.
	PrecalcNodeLists();


	// Convert the transforms back to local space for the nodes.
	for(i=0; i < m_Anims; i++)
	{
		pAnim = GetAnimIfOwner(i);
		if(pAnim)
		{
			for(j=0; j < pAnim->NumKeyFrames(); j++)
			{
				_TransformsGtoL(pAnim->GetRootNode(), j, &parentTransform);
			}
		}
	}


	// Finish the transform map.
	for(i=0; i < NumNodes(); i++)
	{
		pMap = FindTransformMapByNode(tMap, m_FlatNodeList[i]);
		if(pMap)
		{
			pMap->m_NewTransform = m_FlatNodeList[i]->GetNodeIndex();
		}
		else
		{
			// Since we just removed a node, it should always find the nodes.
			ASSERT(false);
		}
	}

	
	// Now remap all the vert weight indices.
	for(i=0; i < NumPieces(); i++)
	{
		pPiece = GetPiece(i);
		uint32 num_lods = pPiece->NumLODs();
		for( uint32 iLods = 0 ; iLods < num_lods ; ++iLods )
		{
			PieceLOD *pLOD = pPiece->GetLOD(iLods);
			for(j=0; j < pLOD->m_Verts; j++)
			{
				pVert = &pLOD->m_Verts[j];

				for(k=0; k < pVert->m_nWeights; k++)
				{
					pWeight = &pVert->m_Weights[k];
				
					pWeight->m_iNode = tMap[pWeight->m_iNode].m_NewTransform;
				}
			}
		}
	}

	// Rebuild our relation to ourself.
	if(!SetupChildModelRelation(GetSelfChildModel(), this, false))
		return false;
	
	return true;
}


bool Model::RemoveRemovableNodes()
{
	uint32 i;
	ModelNode *pNode;
	bool bRet;


	bRet = true;
	for(i=0; i < NumNodes(); i++)
	{
		pNode = GetNode(i);

		if(pNode->m_Flags & MNODE_REMOVABLE)
		{
			if(RemoveNode(pNode))
			{
				// Start iteration over..
				i = (uint32)-1;
			}
			else
			{
				bRet = false;
			}
		}
	}

	return bRet;
}


AnimInfo* Model::AddAnim(ModelAnim *pAnim, ChildInfo *pInfo)
{
	AnimInfo info, *pNewAnim;
	uint32 offset;

	info.m_pAnim = pAnim;
	info.m_pChildInfo = pInfo;

	if(pInfo->m_pModel)
		offset = pInfo->m_AnimOffset + pInfo->m_pModel->CalcNumParentAnims();
	else
		offset = pInfo->m_AnimOffset;

	if(m_Anims.Insert(offset, info))
	{
		pNewAnim = &m_Anims[offset];
		ASSERT(pNewAnim->m_pAnim == pAnim);
		return pNewAnim;
	}
	else
	{
		return LTNULL;
	}
}


bool Model::AddDummyAnimation(char *pName)
{
	ModelAnim *pAnim;
	AnimKeyFrame frame;
	AnimInfo *pInfo;


	pAnim = new ModelAnim(this);
	if(!pAnim)
		return false;

	pInfo = AddAnim(pAnim, &m_SelfChildModel);
	if(!pInfo)
	{
		delete pAnim;
		return false;
	}

	pAnim->SetName(pName);
	
	frame.m_Time = 1;
	pAnim->m_KeyFrames.Append(frame);

	pAnim->SetRootNode(new AnimNode(pAnim, LTNULL));
	if(!CreateDummyAnimNodes(this, pAnim, GetRootNode(), pAnim->GetRootNode(), LTNULL))
		return false;

	// Update other data.
	pAnim->PrecalcNodeLists();

	return true;
}


ModelAnim* Model::FindAnim(char *pName, uint32 *index, AnimInfo **ppInfo)
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

	return LTNULL;
}


AnimInfo* Model::FindAnimInfo(char *pAnimName, Model *pOwner, uint32 *index)
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

	return LTNULL;
}


bool Model::GetAnimInfoIndex(AnimInfo *pInfo, uint32 &index)
{
	uint32 i;

	for(i=0; i < m_Anims; i++)
	{
		if(&m_Anims[i] == pInfo)
		{
			index = i;
			return true;
		}
	}

	return false;
}


bool Model::ForceAnimOrder()
{
	CMoArray<AnimInfo,NoCache> newAnimList;
	uint32 i, iOut;

	if(!newAnimList.SetSize(m_Anims.GetSize()))
		return false;

	// Make sure our animations are the first in the list.
	iOut = 0;
	for(i=0; i < m_Anims; i++)
	{
		if(m_Anims[i].IsParentModel())
		{
			newAnimList[iOut] = m_Anims[i];
			iOut++;
		}
	}

	for(i=0; i < m_Anims; i++)
	{
		if(!m_Anims[i].IsParentModel())
		{
			newAnimList[iOut] = m_Anims[i];
			iOut++;
		}
	}
	
	return m_Anims.CopyArray(newAnimList) == LTTRUE;
}


bool Model::CopyUVs(Model *pSrc)
{
	uint32 i;

	if(NumPieces() != pSrc->NumPieces())
		return false;

	// Go thru all the nodes and copy UVs.
	for(i=0; i < NumPieces(); i++)
	{
		if(!GetPiece(i)->CopyUVs(pSrc->GetPiece(i)))
			return false;
	}
	
	return true;
}


inline void _MatVMul_Deform(LTVector *pDest, LTMatrix *pMat, LTVector *pSrc)
{
	// Making this assumption makes the matrix multiply faster.
	ASSERT(fabs(pMat->m[3][0]) < 0.0001 && fabs(pMat->m[3][1]) < 0.0001 && 
		fabs(pMat->m[3][2]) < 0.0001 && fabs(1.0 - pMat->m[3][3]) < 0.0001);
	
	pDest->x = (pMat->m[0][0]*pSrc->x + pMat->m[0][1]*pSrc->y + pMat->m[0][2]*pSrc->z + pMat->m[0][3]);
	pDest->y = (pMat->m[1][0]*pSrc->x + pMat->m[1][1]*pSrc->y + pMat->m[1][2]*pSrc->z + pMat->m[1][3]);
	pDest->z = (pMat->m[2][0]*pSrc->x + pMat->m[2][1]*pSrc->y + pMat->m[2][2]*pSrc->z + pMat->m[2][3]);
}


bool Model::MakeUniqueAnimName(char *pBase, char *pName)
{
	uint32 i;

	if(!FindAnim(pBase))
	{
		strcpy(pName, pBase);
		return true;
	}

	for(i=0; i < 32768; i++)
	{
		sprintf(pName, "%s%d", pBase, i);
		if(!FindAnim(pName))
		{
			return true;
		}
	}

	return false;
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

// ------------------------------------------------------------------------
// BOOL GetVertexPositions( GVPStruct , bool setup-transforms, weight )
//
// This is the 'reference' function for how to get the vertex positions on a given 
// frame of animation.
// ------------------------------------------------------------------------	
bool Model::GetVertexPositions(GVPStruct *pStruct, bool bSetupTransforms, bool bWeight)
{
	uint32 i, j, k;
	ModelPiece *pPiece;
	uint8 *pCurOut;
	TransformMaker tMaker;
	float vTemp[4];
	ModelVert *pVert;
	LTMatrix *pTransforms;
	NewVertexWeight *pWeight;
	PieceLOD *pLOD;

	// Validate..
	if(!pStruct->m_Vertices)
		return false;

	if(pStruct->m_nAnims > MAX_GVP_ANIMS)
		return false;

	if(bSetupTransforms)
	{
		tMaker.SetupFromGVPStruct(pStruct);
		if(!tMaker.SetupTransforms())
			return false;
	}

	pTransforms = m_Transforms.GetArray();
	
	// Copy the vertex positions over.
	pCurOut = (uint8*)pStruct->m_Vertices;
	for(i=0; i < NumPieces(); i++)
	{
		pPiece = GetPiece(i);
		pLOD = pPiece->GetLOD(pStruct->m_iLOD);

		for(j=0; j < pLOD->m_Verts; j++)
		{
			pVert = &pLOD->m_Verts[j];

			// Apply all the weights.
			if(pVert->m_nWeights > 0)
			{
				vTemp[0] = vTemp[1] = vTemp[2] = vTemp[3] = 0.0f;
				for(k=0; k < pVert->m_nWeights; k++)
				{
					pWeight = &pVert->m_Weights[k];
					ASSERT(pWeight->m_iNode < NumNodes());
					MatVMul_Add(vTemp, &pTransforms[pWeight->m_iNode], pWeight->m_Vec);
				}

				// Divide by w.
				vTemp[3] = 1.0f / vTemp[3];
				((LTVector*)pCurOut)->x = vTemp[0] * vTemp[3];
				((LTVector*)pCurOut)->y = vTemp[1] * vTemp[3];
				((LTVector*)pCurOut)->z = vTemp[2] * vTemp[3];
			}
			else
			{
				((LTVector*)pCurOut)->Init();
			}

			pCurOut += pStruct->m_VertexStride;
		}
	}

	return true;
}


uint32 Model::CalcNumTris(uint32 iLOD)
{
	uint32 i, total;
	PieceLOD *pLOD;

	total = 0;
	for(i=0; i < NumPieces(); i++)
	{
		pLOD = GetPiece(i)->GetLOD(iLOD);
		if(pLOD)
		{
			total += pLOD->m_Tris;
		}
		else
		{
			ASSERT(false);
		}
	}

	return total;
}

uint32 Model::CalcNumTris(float fDistLOD)
{
	uint32 i, total;
	PieceLOD *pLOD;

	total = 0;
	for(i=0; i < NumPieces(); i++)
	{
		pLOD = GetPiece(i)->GetLOD(fDistLOD);
		if(pLOD)
		{
			total += pLOD->m_Tris;
		}
		else
		{
			ASSERT(false);
		}
	}

	return total;
}



uint32 Model::CalcNumVerts()
{
	uint32 i,j, total;

	total = 0;
	for(i=0; i < NumPieces(); i++)
		for(j=0; j< GetPiece(i)->NumLODs() ; j++ )
		total += GetPiece(i)->GetLOD(j)->m_Verts.GetSize();

	return total;
}


uint32 Model::CalcNumVertexWeights()
{
	uint32 i, j, iLOD, total;
	ModelPiece *pPiece;
	PieceLOD *pLOD;
	
	total = 0;
	for(i=0; i < NumPieces(); i++)
	{
		pPiece = GetPiece(i);

		for(iLOD=0; iLOD < pPiece->NumLODs(); iLOD++)
		{
			pLOD = pPiece->GetLOD(iLOD);
			
			for(j=0; j < pLOD->m_Verts; j++)
			{
				total += pLOD->m_Verts[j].m_nWeights;
			}
		}
	}
	
	return total;
}


uint32 Model::CalcNumChildModelAnims(bool bIncludeSelf)
{
	uint32 i, total;
	ChildInfo *pChildModel;

	total = 0;
	for(i=0; i < NumChildModels(); i++)
	{
		pChildModel = GetChildModel(i);

		if(!pChildModel->m_pModel)
			continue;

		if(pChildModel == GetSelfChildModel() && !bIncludeSelf)
			continue;

		total += pChildModel->m_pModel->m_Anims.GetSize();
	}

	return total;
}


uint32 Model::CalcNumParentAnims()
{
	uint32 i, total;

	total = 0;
	for(i=0; i < m_Anims; i++)
	{
		if(GetAnim(i)->m_pModel == this)
			++total;
	}

	return total;
}


void Model::ParseCommandString()
{
	ConParse parse;
	TransformMaker tMaker;
	uint32 i;
	SimpleFloatVar floatVars[] =
	{
		"VisRadius", &m_VisRadius,
		"AmbientLight", &m_AmbientLight,
		"DirLight", &m_DirLight,
		"ShadowProjectLength", &m_fShadowProjectLength,
		"ShadowLightDist", &m_fShadowLightDist,
		"ShadowSizeX", &m_fShadowSizeX,
		"ShadowSizeY", &m_fShadowSizeY
	};


	
	if(!m_CommandString)
		return;

	m_bFOVOffset = false;

	// Parse for level of detail settings...
	parse.Init(m_CommandString);
	while(parse.Parse())
	{
		if(parse.m_nArgs == 2)
		{
			// Read in the easy ones.
			for(i=0; i < (sizeof(floatVars) / sizeof(floatVars[0])); i++)
			{
				if(stricmp(floatVars[i].m_pName, parse.m_Args[0]) == 0)
					*floatVars[i].m_pVal = (float)atof(parse.m_Args[1]);
			}

			// Check for non-trivial ones..
			if(stricmp("FovXOffset", parse.m_Args[0]) == 0)
			{
				m_xFOVOffset = (float)atof(parse.m_Args[1]);
				m_xFOVOffset = MATH_DEGREES_TO_RADIANS(m_xFOVOffset);
				m_bFOVOffset = true;
			}
			else if(stricmp("FovYOffset", parse.m_Args[0]) == 0)
			{
				m_yFOVOffset = (float)atof(parse.m_Args[1]);
				m_yFOVOffset = MATH_DEGREES_TO_RADIANS(m_yFOVOffset);
				m_bFOVOffset = true;
			}
		}
		else if(parse.m_nArgs == 1)
		{
			if(stricmp("NoAnimation", parse.m_Args[0]) == 0)
			{
				m_bNoAnimation = true;
			}
			else if(stricmp("ShadowEnable", parse.m_Args[0]) == 0)
			{
				m_bShadowEnable = true;
			}
			else if(stricmp("SpecularEnable", parse.m_Args[0]) == 0)
			{
				m_bSpecularEnable = true;
			}
            else if(stricmp("Rigid", parse.m_Args[0]) == 0)
			{
				m_bRigid = true;
			}

		}
		else if(parse.m_nArgs >= 3)
		{
			if(stricmp("NormalRef", parse.m_Args[0]) == 0)
			{
				m_bNormalRef = false;
				m_iNormalRefNode = INVALID_MODEL_NODE;
				if(FindNode(parse.m_Args[1], &m_iNormalRefNode))
				{
					m_iNormalRefAnim = INVALID_MODEL_ANIM;
					if(FindAnim(parse.m_Args[2], &m_iNormalRefAnim))
					{
						tMaker.m_Anims[0].Init(this, 
							m_iNormalRefAnim, 0, 
							m_iNormalRefAnim, 0,
							0.0f);
						tMaker.m_nAnims = 1;
												
						if(tMaker.SetupTransforms())
						{
							m_mNormalRef = m_Transforms[m_iNormalRefNode];
							m_bNormalRef = true;
						}							
					}					
				}
			}
		}
		else if(parse.m_nArgs >= 4)
		{
			if(stricmp("ShadowCenterOffset", parse.m_Args[0]) == 0)
			{
				m_vShadowCenterOffset.x = (float)atof(parse.m_Args[1]);
				m_vShadowCenterOffset.y = (float)atof(parse.m_Args[2]);
				m_vShadowCenterOffset.z = (float)atof(parse.m_Args[3]);
			}
		}
	}
}

const char* Model::GetFilename()
{
	return m_pFilename;
}


bool Model::SetFilename(const char *pInFilename)
{
	char *pFilename;

	FreeFilename();

	pFilename = new char[strlen(pInFilename) + 1];
	if(!pFilename)
	{
		return false;
	}

	strcpy(pFilename, pInFilename);
	m_pFilename = pFilename;
	return true;
}


void Model::FreeFilename()
{
	if(m_pFilename != g_pNoModelFilename)
	{
		delete m_pFilename;
	}

	m_pFilename = g_pNoModelFilename;
}


bool Model::VerifyChildModelTree(Model *pChild, ModelNode* &pErrNode)
{
	return VerifyChildModel_R(GetRootNode(), pChild->GetRootNode(), pErrNode) == LTTRUE;
}


// 
// N = O * X
// N * ~O = X
// Global = Parent * Local so:
// Local = ~Parent * Global
void _SetupChildModelRelation_R(ChildInfo *pInfo, 
	LTMatrix *pParentT1, LTMatrix *pParentT2,
	ModelNode *pNode1, ModelNode *pNode2)
{
	LTMatrix inverse, local1, local2, mFinal;
	uint32 i;


	// Get the local transforms.
	Mat_InverseTransformation(pParentT1, &inverse);
	MatMul(&local1, &inverse, &pNode1->GetGlobalTransform());

	Mat_InverseTransformation(pParentT2, &inverse);
	MatMul(&local2, &inverse, &pNode2->GetGlobalTransform());

	// Setup the relation.
	Mat_InverseTransformation(&local2, &inverse);
	MatMul(&mFinal, &inverse, &local1);
	
	pInfo->m_Relation[pNode1->GetNodeIndex()].ConvertFromMatrix(mFinal);

	for(i=0; i < pNode1->NumChildren(); i++)
	{
		_SetupChildModelRelation_R(pInfo, 
			&pNode1->GetGlobalTransform(), &pNode2->GetGlobalTransform(),
			pNode1->GetChild(i), pNode2->GetChild(i));
	}
}


bool Model::SetupChildModelRelation(ChildInfo *pInfo, Model *pChild, bool bScaleSkeleton)
{
	LTMatrix ident;
	uint32 i;
	
	if(!pInfo->m_Relation.SetSize(NumNodes()))
		return false;

	Mat_Identity(&ident);
	if(bScaleSkeleton)
	{
		_SetupChildModelRelation_R(pInfo, &ident, &ident, GetRootNode(), pChild->GetRootNode());
	}
	else
	{
		for(i=0; i < pInfo->m_Relation; i++)
		{
			pInfo->m_Relation[i].ConvertFromMatrix(ident);
		}
	}

	return true;
}


ChildInfo* Model::AddChildModel(

				Model *pChild,         // ptr to loaded child model
				char *pFilename,	   // name of on-disk file.
				char *pErrStr,         // return error string to caller (must have space)
				bool bScaleSkeleton)
{
	ChildInfo *pChildModel;
	AnimInfo *pAnim;
	uint32 i;
	ModelNode *pErrNode;

	
	if(FindChildModel(pChild))
	{
		// ooooo you dirty rat %#$@#!
		ASSERT(false);
		sprintf(pErrStr, "Child Model already exists.");
		return LTNULL;
	}

	// Can't have too many.
	if(m_nChildModels >= MAX_CHILD_MODELS)
	{
		sprintf(pErrStr, "Child model limit reached.");
		return LTNULL;
	}

	// Make sure it's valid.
	if(pChild)
	{
		if(!VerifyChildModelTree(pChild, pErrNode))
		{
			sprintf(pErrStr, "Node trees differ at node %s.", pErrNode->GetName());
			return false;
		}
	}

	// Add it..
	pChildModel = new ChildInfo;
	if(!pChildModel)
		return LTNULL;

	if(!SetupChildModelRelation(pChildModel, pChild, bScaleSkeleton))
	{
		sprintf(pErrStr, "SetupChildModelRelation returned false.");
		delete pChildModel;
		return LTNULL;
	}

	pChildModel->m_pFilename = AddString(pFilename);
	if(!pChildModel->m_pFilename)
	{
		delete pChildModel;
		return LTNULL;
	}

	// Add it to the list.
	m_ChildModels[m_nChildModels] = pChildModel;
	m_nChildModels++;

	// Set it up.
	pChildModel->m_pParentModel = this;
	pChildModel->m_pModel = pChild;
	pChildModel->m_AnimOffset = 0;

	if(pChild)
	{
		pChild->AddRef();

		pChildModel->m_AnimOffset = m_Anims.GetSize();
		if(!m_Anims.NiceSetSize(m_Anims.GetSize() + pChild->m_Anims.GetSize()))
		{
			sprintf(pErrStr, "Out of memory.");
			return LTNULL;
		}

		for(i=0; i < pChild->m_Anims; i++)
		{
			pAnim = GetAnimInfo(pChildModel->m_AnimOffset+i);
			*pAnim = pChild->m_Anims[i];
			pAnim->m_pChildInfo = pChildModel;
		}
	}

	return pChildModel;
}

// ------------------------------------------------------------------------
// Add ChildModel as References 
// ------------------------------------------------------------------------
ChildInfo* Model::AddChildModelRef( char *pFilename, 
								char *pErrStr 
							   )
							   	
{
	ChildInfo *pChildModel;
	uint32 index ;

	
	// Check if there's already a child by filenames.
	if(FindChildModelByFilename(pFilename, &index) != LTNULL)
	{
		ASSERT(false);
		sprintf(pErrStr, "Child Model already exists.");
		return LTNULL;
	}


	// Can't have too many.
	if(m_nChildModels >= MAX_CHILD_MODELS)
	{
		sprintf(pErrStr, "Child model limit reached.");
		return LTNULL;
	}

	// Make sure it's valid.
#if(0)
	// we can't there's no real data.
	if(pChild)
	{
		if(!VerifyChildModelTree(pChild, pErrNode))
		{
			sprintf(pErrStr, "Node trees differ at node %s.", pErrNode->GetName());
			return false;
		}
	}
#endif

	// Add it..
	pChildModel = new ChildInfo;
	if(!pChildModel)
		return LTNULL;

#if(0)
	// ?? 
	if(!SetupChildModelRelation(pChildModel, pChild, bScaleSkeleton))
	{
		sprintf(pErrStr, "SetupChildModelRelation returned false.");
		delete pChildModel;
		return LTNULL;
	}
#endif

	pChildModel->m_pFilename = AddString(pFilename);
	if(!pChildModel->m_pFilename)
	{
		delete pChildModel;
		return LTNULL;
	}

	// Add it to the list.
	m_ChildModels[m_nChildModels] = pChildModel;
	m_nChildModels++;

	// Set it up.
	pChildModel->m_pParentModel = this;
	pChildModel->m_pModel = LTNULL; // danger here, the child model is technically broken.
	pChildModel->m_AnimOffset = 0;

#if(0)
	if(pChild)
	{
		pChild->AddRef();

		pChildModel->m_AnimOffset = m_Anims.GetSize();
		if(!m_Anims.NiceSetSize(m_Anims.GetSize() + pChild->m_Anims.GetSize()))
		{
			sprintf(pErrStr, "Out of memory.");
			return LTNULL;
		}

		for(i=0; i < pChild->m_Anims; i++)
		{
			pAnim = GetAnimInfo(pChildModel->m_AnimOffset+i);
			*pAnim = pChild->m_Anims[i];
			pAnim->m_pChildInfo = pChildModel;
		}
	}
#endif

	return pChildModel;
}

// ------------------------------------------------------------------------
// RemoveChildModel
// ------------------------------------------------------------------------
bool Model::RemoveChildModel(uint32 index)
{
	uint32 i;
	ChildInfo *pInfo;

	if(index >= m_nChildModels || index == 0)
		return false;

	pInfo = m_ChildModels[index];

	// Remove any animations referencing this child model.
	for(i=0; i < m_Anims; i++)
	{
		if(m_Anims[i].m_pChildInfo == pInfo)
		{
			m_Anims.Remove(i);
			--i;
		}
	}

	// Delete and remove it from the list.
	delete pInfo;
	
	// re arrange our static memory to accomodate the missing childmodel
	for( i = index ; i < m_nChildModels-1; i++ )
	{
		m_ChildModels[i] = m_ChildModels[i+1];	
	}
	
	for( ; i < MAX_CHILD_MODELS; i++ )
		m_ChildModels[i] = NULL ;

	m_nChildModels = m_nChildModels -1 ;
	
	return true;
}


bool Model::InitChildInfo(uint32 index, ChildInfo *pChildModel, Model *pModel, char *pFilename)
{
	if(index >= MAX_CHILD_MODELS)
		return false;

	pChildModel->m_pFilename = pFilename;
	pChildModel->m_pParentModel = this;
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


ChildInfo* Model::FindChildModel(Model *pModel)
{
	uint32 i;

	for(i=0; i < NumChildModels(); i++)
	{
		if(GetChildModel(i)->m_pModel == pModel)
			return GetChildModel(i);
	}
	
	return LTNULL;
}


ChildInfo* Model::FindChildModelByFilename(char *pFilename, uint32 *index)
{
	uint32 i;
	ChildInfo *pInfo;

	for(i=0; i < NumChildModels(); i++)
	{
		pInfo = GetChildModel(i);

		if(strcmp(pInfo->m_pFilename, pFilename) == 0)
		{
			if(index)
				*index = i;

			return pInfo;
		}
	}
	
	return LTNULL;
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

	return LTNULL;
}


bool Model::AddSocket(ModelSocket *pSocket)
{
	return m_Sockets.Append(pSocket) == LTTRUE;
}


bool Model::RemoveSocket(uint32 index)
{
	if(index >= m_Sockets.GetSize())
		return false;

	delete m_Sockets[index];
	m_Sockets.Remove(index);
	return true;
}


bool Model::GetSocketTransform(ModelSocket *pSocket, LTMatrix *pSocketTransform)
{
	LTMatrix offsetTransform, *pTransform;
	ModelNode *pNode;


	if(pSocket->m_iNode >= NumNodes())
		return false;

	pNode = GetNode(pSocket->m_iNode);
	if(!pNode)
		return false;

	pTransform = GetTransform(pNode->GetNodeIndex());
	pSocket->ConvertToMatrix(offsetTransform);

	MatMul(pSocketTransform, pTransform, &offsetTransform);
	return true;
}


bool Model::AnyVertsWeightedToNode(uint32 iNode)
{
	uint32 iPiece, iLOD, iVert, iWeight;
	ModelPiece *pPiece;
	ModelVert *pVert;

	for(iPiece=0; iPiece < NumPieces(); iPiece++)
	{
		pPiece = GetPiece(iPiece);
		for( iLOD = 0 ; iLOD < pPiece->NumLODs() ; iLOD++ )
		{
			PieceLOD *pLOD = GetPiece(iPiece)->GetLOD(iLOD);
			for(iVert=0; iVert < pLOD->m_Verts; iVert++)
			{
				pVert = &pLOD->m_Verts[iVert];

				for(iWeight=0; iWeight < pVert->m_nWeights; iWeight++)
					if(pVert->m_Weights[iWeight].m_iNode == iNode)
						return true;
			}
		}
	}

	return false;
}


