#if !defined(__LINUX)
#include <windows.h>
#endif
#include "bdefs.h"
#include "transformmaker.h"
#include "de_objects.h"

#define MAX_MODELPATH_LEN	64


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



// ------------------------------------------------------------------------
//
// ------------------------------------------------------------------------
bool TransformMaker::SetupTransforms()
{
	if(!SetupCall()) 
		return false;

	Recurse(m_pModel->GetRootNode()->GetNodeIndex(), m_pStartMat);
	return true;
}

bool TransformMaker::SetupTransformsWithPath()
{
	uint32 iRootNode = m_pInstance->GetModelDB()->GetRootNode()->GetNodeIndex() ;
	
	// leave if there's nothing to eval.
	if( !m_pInstance->ShouldEvaluateNode( iRootNode ) )
		return true ;

	if(!SetupCall()) 
		return false ;

	RecurseWithPath( iRootNode, m_pStartMat);

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
	if(!IsValid())
		return false;

	if(!m_pStartMat || !m_pOutput)
	{
		// We need a root node and something to copy into.
		ASSERT(false);
		return false;
	}

	m_pRecursePath = LTNULL;
	m_pModel = m_Anims[0].m_pModel;

	// Cache the information that's going to be used while recursing
	for(uint32 i=0; i < m_nAnims; i++)
	{
		// Note: The first weightset is invalid, and therefore unused
		m_WeightSets[i] = (i > 0) ? m_pModel->GetWeightSet(m_Anims[i].m_iWeightSet) : NULL;
		m_pAnimPrev[i]	= m_pModel->GetAnim(m_Anims[i].m_Prev.m_iAnim);
		m_pAnimCur[i]	= m_pModel->GetAnim(m_Anims[i].m_Cur.m_iAnim);
	}

	return LTTRUE;
}


// ------------------------------------------------------------------------
// InitTransform
//
// ------------------------------------------------------------------------
inline
void TransformMaker::InitTransform(	uint32 iAnim,
									uint32 iNode,
									LTRotation &outQuat,
									LTVector &outVec)
{
	AnimTimeRef *pTimeRef;
	
	LTVector p1,p2;
	LTRotation q1,q2;
	LTVector *pTrans1, *pTrans2;
	

	pTimeRef = &m_Anims[iAnim];
	float inter_frame_param  = pTimeRef->m_Percent ;

	// we don't want to interpolate the movement node between two different
	// anims. When the two anims are different, use the next anim's first 
	// frame during the interpolation time.
	if( iNode == m_iMoveHintNode )
	{
		if( pTimeRef->m_Prev.m_iAnim != pTimeRef->m_Cur.m_iAnim )
		{
			 inter_frame_param = 1.0f; // take the next anims first frame.
		}
	}

	m_pAnimPrev[iAnim]->GetAnimNode(iNode)->GetData(pTimeRef->m_Prev.m_iFrame, p1, q1);;
	m_pAnimCur[iAnim]->GetAnimNode(iNode)->GetData(pTimeRef->m_Cur.m_iFrame, p2, q2);;
	
	outQuat.Slerp( q1, q2, inter_frame_param);
	outVec = p1 + (p2 - p1) * inter_frame_param ;

	// Apply the per-animation translation.
	if(m_pModel->IsRootNode(iNode))
	{
		pTrans1 = &m_pModel->GetAnimInfo(pTimeRef->m_Prev.m_iAnim)->m_vTranslation;
		pTrans2 = &m_pModel->GetAnimInfo(pTimeRef->m_Cur.m_iAnim)->m_vTranslation;
		outVec += *pTrans1 + (*pTrans2 - *pTrans1) * inter_frame_param;
	}
}

// ------------------------------------------------------------------------
// 
// ------------------------------------------------------------------------
inline
void TransformMaker::InitTransformAdditive(
	uint32 iAnim, 
	uint32 iNode, 
	LTRotation &outQuat, 
	LTVector &outVec)
{
	AnimTimeRef *pTimeRef;
	LTVector p1,p2,basepos;
	LTRotation q1,q2, basequat;

	pTimeRef = &m_Anims[iAnim];

	// get transforms
	m_pAnimPrev[iAnim]->GetAnimNode(iNode)->GetData(pTimeRef->m_Prev.m_iFrame, p1, q1);;
	m_pAnimPrev[iAnim]->GetAnimNode(iNode)->GetData(0,basepos,basequat);
	m_pAnimCur [iAnim]->GetAnimNode(iNode)->GetData(pTimeRef->m_Cur.m_iFrame, p2, q2);

	outQuat.Slerp(q1, q2, pTimeRef->m_Percent );
	outVec = p1 + ( p2 - p1 ) * pTimeRef->m_Percent;

	//frame = base + offset
	outQuat = ~basequat * outQuat ; // take out base pos's rotation
	outVec  = outVec - basepos ;
}
 
// ------------------------------------------------------------------------
// 
// ------------------------------------------------------------------------
inline 
void TransformMaker::BlendTransform(uint32 iAnim, uint32 iNode)
{
	float fPercent = m_WeightSets[iAnim]->m_Weights[iNode];

	// skip this blend if the weight for this anim is zero.
	if(fPercent == 0.0f)
	{
		return;
	}
	else
	{
		LTRotation qTransform, qTemp;
		LTVector vTransform;

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
	ModelNode *pNode;
	static LTMatrix mScratchMat;

	for(;;)
	{
		pMyGlobal = &m_pOutput[iNode];

		//cache our node reference
		pNode = m_pModel->GetNode(iNode);

		// Apply animation data (first one inits, the rest are blended in).
		InitTransform(0, iNode, m_Quat, m_vTrans);
		for(i=1; i < m_nAnims; i++)
		{
			BlendTransform(i, iNode);
		}

		// Update the global matrix.
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

		MatMul(pMyGlobal, pParentT, &m_mTemp);

		if(m_pInstance && m_pInstance->HasNodeControlFn(iNode))
		{
			//setup the node control data
			NodeControlData Data;
			Data.m_hModel					= (HOBJECT)m_pInstance;
			Data.m_hNode					= iNode;
			Data.m_pFromParentTransform		= &pNode->GetFromParentTransform();
			Data.m_pNodeTransform			= pMyGlobal;
			Data.m_pParentTransform			= pParentT;

			m_pInstance->ApplyNodeControl(Data);
		}


		// Do the children..
		if(m_pRecursePath)
		{
			if(m_iCurPath > 0)
			{
				m_iCurPath--;
				// Start over the loop at the next point in the path
				iNode = m_pRecursePath[m_iCurPath];
				pParentT = pMyGlobal;
			}
			else
				break;
		}
		else
		{
			uint32 nNumChildren = pNode->NumChildren();
			if (nNumChildren)
			{
				// Recurse for the beginning of the child list
				--nNumChildren;
				for(i=0; i < nNumChildren; i++)
				{
					Recurse(pNode->m_Children[i]->GetNodeIndex(), pMyGlobal);
				}
				// Iterate for the final child
				m_iCurPath = 1;
				iNode = pNode->m_Children[nNumChildren]->GetNodeIndex();
				pParentT = pMyGlobal;
			}
			else
				// Jump out of the list..  No children.
				break;
		}
	}
}


// ------------------------------------------------------------------------
// RecurseWithPath( node-index, parent's-matrix )
// evaluate the transform hierarchy only traversing nodes on a "path".
// the path is determined by the set of requested nodes either from geometry
// or socket transform requests.
// if a node has already been evaluated don't do it again.
// ------------------------------------------------------------------------
void TransformMaker::RecurseWithPath(uint32 iNode, LTMatrix *pParentT)
{
	uint32 i;
	LTMatrix *pMyGlobal;
	ModelNode *pNode;

	// get current transform to work on (from current modelinstance xform cache).
	pMyGlobal = &m_pOutput[iNode];

	//get the hierarchy node we are working with.
	pNode = m_pModel->GetNode(iNode);

	// if we need to evaluate this node, do so.
	if( !m_pInstance->IsNodeEvaluated( iNode ) )
	{
		// Apply animation data (first one inits, the rest are blended in).
		InitTransform(0, iNode, m_Quat, m_vTrans);
		for(i=1; i < m_nAnims; i++)
		{
			BlendTransform(i, iNode);
		}

		// Update the global matrix.
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

		// final global pos = parent transform * local-evaluated-animation
		MatMul(pMyGlobal, pParentT, &m_mTemp);

		// tell node we've evaluated it.  Mark it done before the nodecontrolfn
		// since it could end up calling back into this model to update it's transforms
		// which would cause an infinite recursion.  This can happen if 2 ai
		// are tracking each other.
		m_pInstance->SetNodeEvaluated( iNode, true);

		if(m_pInstance && m_pInstance->HasNodeControlFn(iNode))
		{
			//setup the node control data
			NodeControlData Data;
			Data.m_hModel					= (HOBJECT)m_pInstance;
			Data.m_hNode					= iNode;
			Data.m_pFromParentTransform		= &pNode->GetFromParentTransform();
			Data.m_pNodeTransform			= pMyGlobal;
			Data.m_pParentTransform			= pParentT;

			m_pInstance->ApplyNodeControl(Data);
		}
	}
	
	// evaluate the children..
	uint32 nNumChildren = pNode->NumChildren();
	for( i = 0 ; i < nNumChildren; i++ )
	{
		uint32 iChild  = pNode->m_Children[i]->GetNodeIndex();

		// if this node is on the evaluation path, recurse
		if( m_pInstance->ShouldEvaluateNode( iChild ))
		{
			RecurseWithPath( iChild , pMyGlobal );

			// erase the path as we go along.
			m_pInstance->SetShouldEvaluateNode(iChild, false);
		}
	}
}

