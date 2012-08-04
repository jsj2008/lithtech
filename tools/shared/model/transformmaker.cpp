#ifndef __PS2
#include <windows.h>
#endif
#include "bdefs.h"
#include "transformmaker.h"

#define MAX_MODELPATH_LEN	64

// ------------------------------------------------------------------------
// SetFromGVPStruct
// set up from a render packet
// ------------------------------------------------------------------------
void TransformMaker::SetupFromGVPStruct(GVPStruct *pStruct)
{
	uint32 i;

	for(i=0; i < pStruct->m_nAnims; i++)
	{
		m_Anims[i] = pStruct->m_Anims[i];
	}

	m_nAnims = pStruct->m_nAnims;
	m_pStartMat = &pStruct->m_BaseTransform;
	
}


bool TransformMaker::IsValid()
{
	uint32 i;
	Model *pModel;
	AnimTimeRef *pAnim;

	if(m_nAnims == 0 || m_nAnims > MAX_GVP_ANIMS)
		return false;

	pModel = m_Anims[0].m_pModel;
	for(i=0; i < m_nAnims; i++)
	{
		pAnim = &m_Anims[i];

		if(!pAnim->IsValid() || pAnim->m_pModel != pModel)
			return false;

		// Make sure the weight set is valid.
		if(i != 0)
		{
			if(pAnim->m_iWeightSet >= pModel->NumWeightSets())
				return false;
		}
	}

	return true;
}


void TransformMaker::CopyTimes(TransformMaker &other)
{
	uint32 i;

	ASSERT(other.m_nAnims <= MAX_GVP_ANIMS);

	for(i=0; i < other.m_nAnims; i++)
		m_Anims[i] = other.m_Anims[i];

	m_nAnims = other.m_nAnims;
}

// ------------------------------------------------------------------------
//
// ------------------------------------------------------------------------
bool TransformMaker::SetupTransforms()
{
	if(!SetupCall()) return false;

	Recurse(m_pModel->GetRootNode()->GetNodeIndex(), m_pStartMat);
	return true;
}


bool TransformMaker::SetupMovementEncoding( void )
{
	if( !SetupCall() )
		return false;

	ModelNode* pMovementNode = m_pModel->GetMovementNode();
	
	if( pMovementNode )
	{
		uint32 movementNode = pMovementNode->GetNodeIndex();

		LTMatrix baseXForm;
		baseXForm.Identity();

		InitTransform( 0, movementNode, m_Quat, m_vTrans );
		for( uint32 i = 1; i < m_nAnims; i++ )
		{
			BlendTransform( i, movementNode );
		}

		m_Quat.ConvertToMatrix( m_mTemp );
		m_mTemp.SetTranslation( m_vTrans );

		*m_pStartMat = m_mTemp * *m_pStartMat;
	}

	return true;
}


bool TransformMaker::GetNodeTransform(uint32 iNode)
{
	uint32 thePath[MAX_MODELPATH_LEN];


	if(!SetupCall())
		return false;

	if(iNode >= m_pModel->NumNodes())
		return false;

	// Build the path from this node to the root.
	m_iCurPath = 0;
	for(;;)
	{
		if(m_iCurPath >= MAX_MODELPATH_LEN)
		{
			ASSERT(false);
			return false;
		}

		thePath[m_iCurPath] = iNode;
		m_iCurPath++;

		iNode = m_pModel->GetNode(iNode)->GetParentNodeIndex();
		if(iNode == NODEPARENT_NONE)
			break;
	}

	m_pRecursePath = thePath;
	m_iCurPath--;
	ASSERT(m_pRecursePath[m_iCurPath] == m_pModel->GetRootNode()->GetNodeIndex());

	Recurse(m_pModel->GetRootNode()->GetNodeIndex(), m_pStartMat);
	return true;
}

// ------------------------------------------------------------------------
// SetupCall()
// This sets up the transform maker, fishes out the result transformation
// array out of the model class and puts it into m_pOutput as a handle that is
// going to be used further on.
// the model is there because the transform maker has been initialized with
// a valid animation before this call.
// ------------------------------------------------------------------------
bool TransformMaker::SetupCall()
{
	AnimInfo *pAnim;


	if(!IsValid())
		return false;

	m_pRecursePath = LTNULL;
	m_pModel = m_Anims[0].m_pModel;

	if(!m_pStartMat)
	{
		Mat_Identity(&m_mIdent);
		m_pStartMat = &m_mIdent;
	}

	if(!m_pOutput)
	{
		m_pOutput = m_pModel->m_Transforms.GetArray();
	}

	pAnim = m_pModel->GetAnimInfo(m_Anims[0].m_Prev.m_iAnim);
	m_pNodeRelation = &pAnim->m_pChildInfo->m_Relation;

	return true;
}

// ------------------------------------------------------------------------
// InitTransform
//
// ------------------------------------------------------------------------
void TransformMaker::InitTransform(	uint32 iAnim,
									uint32 iNode,
									LTRotation &outQuat,
									LTVector &outVec)
{
	AnimTimeRef *pTimeRef;
	ModelAnim *pAnims[2];
	AnimNode *pAnimNodes[2];
	NodeKeyFrame *pKeys[2];
	LTVector *pTrans1, *pTrans2;


	pTimeRef = &m_Anims[iAnim];

	pAnims[0] = m_pModel->GetAnim(pTimeRef->m_Prev.m_iAnim);
	pAnims[1] = m_pModel->GetAnim(pTimeRef->m_Cur.m_iAnim);
	pAnimNodes[0] = pAnims[0]->m_AnimNodes[iNode];
	pAnimNodes[1] = pAnims[1]->m_AnimNodes[iNode];
	pKeys[0] = &pAnimNodes[0]->m_KeyFrames[pTimeRef->m_Prev.m_iFrame];
	pKeys[1] = &pAnimNodes[1]->m_KeyFrames[pTimeRef->m_Cur.m_iFrame];

	outQuat.Slerp(pKeys[0]->m_Quaternion, pKeys[1]->m_Quaternion, pTimeRef->m_Percent);
	outVec = pKeys[0]->m_Translation + (pKeys[1]->m_Translation - pKeys[0]->m_Translation) * pTimeRef->m_Percent;

	// Apply the per-animation translation.
	if(m_pModel->IsRootNode(iNode))
	{
		pTrans1 = &m_pModel->GetAnimInfo(pTimeRef->m_Prev.m_iAnim)->m_vTranslation;
		pTrans2 = &m_pModel->GetAnimInfo(pTimeRef->m_Cur.m_iAnim)->m_vTranslation;
		outVec += *pTrans1 + (*pTrans2 - *pTrans1) * pTimeRef->m_Percent;
	}
}


void TransformMaker::InitTransformAdditive( uint32 iAnim,
											uint32 iNode,
											LTRotation &outQuat,
											LTVector &outVec)
{
	AnimTimeRef *pTimeRef;
	ModelAnim *pAnims[2];
	AnimNode *pAnimNodes[2];
	NodeKeyFrame *pKeys[2], *pBaseKey;


	pTimeRef = &m_Anims[iAnim];

	pAnims[0] = m_pModel->GetAnim(pTimeRef->m_Prev.m_iAnim);
	pAnims[1] = m_pModel->GetAnim(pTimeRef->m_Cur.m_iAnim);
	pAnimNodes[0] = pAnims[0]->m_AnimNodes[iNode];
	pAnimNodes[1] = pAnims[1]->m_AnimNodes[iNode];

	pKeys[0] = &pAnimNodes[0]->m_KeyFrames[pTimeRef->m_Prev.m_iFrame];
	pKeys[1] = &pAnimNodes[1]->m_KeyFrames[pTimeRef->m_Cur.m_iFrame];

	pBaseKey = &pAnimNodes[0]->m_KeyFrames[0];

	outQuat.Slerp(pKeys[0]->m_Quaternion, pKeys[1]->m_Quaternion, pTimeRef->m_Percent);
	outVec = pKeys[0]->m_Translation + (pKeys[1]->m_Translation - pKeys[0]->m_Translation) * pTimeRef->m_Percent;

	//frame = base + offset
	//offset = ~base * frame
	outQuat = ~pBaseKey->m_Quaternion * outQuat;
	outVec = outVec - pBaseKey->m_Translation;
}

void TransformMaker::BlendTransform(uint32 iAnim, uint32 iNode)
{
	LTRotation qTransform, qTemp;
	LTVector vTransform;
	float fPercent;

	// Can we be lazy?
	fPercent = m_pModel->GetWeightSet(m_Anims[iAnim].m_iWeightSet)->m_Weights[iNode];
	if(fPercent == 0.0f)
	{
		return;
	}
	else
	{
		if(fPercent == 2.0f)
		{
			// Add the animation.
			InitTransformAdditive(iAnim, iNode, qTransform, vTransform);

			m_Quat = m_Quat * qTransform;
			m_vTrans = m_vTrans + vTransform;
		}
		else
		{
			InitTransform(iAnim, iNode, qTransform, vTransform);
			qTemp = m_Quat;
			m_Quat.Slerp(qTemp, qTransform, fPercent);
			m_vTrans = m_vTrans + (vTransform - m_vTrans) * fPercent;
		}
	}
}


void TransformMaker::Recurse(uint32 iNode, LTMatrix *pParentT)
{
	uint32 i;
	LTMatrix *pMyGlobal;

	ModelNode *pNode = m_pModel->GetNode(iNode);

	pMyGlobal = &m_pOutput[iNode];
	*pMyGlobal = *pParentT;

	// state the evaluation state for this node. If we need to eval do it.
	// otherwise we're done, just get it from the db.
	// the other state is one where the node is "clear" meaning don't touch it,
	// just set the db to the parent's xform.
	switch( pNode->m_EvalState )
	{
		
	case ModelNode::kEval :
	{
		// Apply animation data (first one inits, the rest are blended in).
		InitTransform(0, iNode, m_Quat, m_vTrans);
		for(i=1; i < m_nAnims; i++)
		{
			BlendTransform(i, iNode);
		}

		m_Quat.ConvertToMatrix(m_mTemp);

		// Use the offset from the parent if this node only uses rotation data
		// from the animation.
		if(pNode->m_Flags & MNODE_ROTATIONONLY)
		{
			m_mTemp.SetTranslation(pNode->m_vOffsetFromParent);
		}
		else
		{
			m_mTemp.SetTranslation(m_vTrans);
		}

		*pMyGlobal = *pMyGlobal * m_mTemp;
		pNode->m_EvalState = ModelNode::kDone ;
	}
	break ;
	case ModelNode::kDone :
		*pMyGlobal = *m_pModel->GetTransform( pNode->m_NodeIndex );
		break;
	}

	// Do the children..
	if(m_pRecursePath)
	{
		if(m_iCurPath > 0)
		{
			m_iCurPath--;
			Recurse(m_pRecursePath[m_iCurPath], pMyGlobal);
		}
	}
	else
	{
		for(i=0; i < pNode->NumChildren(); i++)
		{
			Recurse(pNode->m_Children[i]->GetNodeIndex(), pMyGlobal);
		}
	}
}
