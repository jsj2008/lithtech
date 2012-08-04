#include "stdafx.h"
#include "RagDoll.h"
#include "RagDollNode.h"
#include "RagDollConstraint.h"


//-----------------------------------------------------------------------------------
// Model Node
//-----------------------------------------------------------------------------------
class CModelNode
{
public:

	//This is the ragdoll node that we should look to to grab our base position
	CRagDollNode*	m_pPosNode;

	//This serves as the primary node link in building the basis space
	CRagDollNode*	m_pPrimaryLink;

	//This serves as the secondary link in building the basis space
	CRagDollNode*	m_pSecondaryLink;

	//The matrix to convert the binding space orientation to the current space
	LTMatrix		m_mBasisOrientation;

	//the vector offset from its parent node
	LTVector		m_vParentOffset;

	//the rag doll that we belong to (opaque to this class)
	CRagDoll*		m_pRagDoll;

	//the node that we are tracking this
	HMODELNODE		m_hModelNode;
};


//-----------------------------------------------------------------------------------
// A function that given a model node, it will attempt to build up an orienation
// matrix for the node
//-----------------------------------------------------------------------------------
void BuildModelNodeOrientation(const CModelNode& Node, const LTVector& vDefaultUp, uint32 nCurrPos, LTMatrix& mOutMat)
{
	//we first off make the assumption that the right vector is along the primary link, and that this
	//is the most stable of all the axis
	LTVector vRight = Node.m_pPosNode->m_vPosition[nCurrPos] - Node.m_pPrimaryLink->m_vPosition[nCurrPos];
	vRight.Normalize();

	//determine if we can build a plane out of the basis or not
	LTVector vOther = Node.m_pSecondaryLink->m_vPosition[nCurrPos] - Node.m_pPosNode->m_vPosition[nCurrPos];
	vOther.Normalize();

	LTVector vUp;

	float fDot = vOther.Dot(vRight);

	if(fabs(vOther.Dot(vRight)) > 0.95f)
	{
		//these are too close to forming a line, we have to use the default
		vUp = vDefaultUp;
	}
	else
	{
		//alright, build up a basis space that we can orient off of
		vUp = vRight.Cross(vOther);
		vUp.Normalize();

		//now orthogonalize the basis space
		LTVector vForward = vUp.Cross(vRight);
		vForward.Normalize();

		//install that into a matrix so that we can apply the node offset to it
		LTMatrix mMat;
		mMat.SetBasisVectors(&vRight, &vUp, &vForward);

		//now apply our transformation to it
		mMat = mMat * Node.m_mBasisOrientation;

		//and reget the up out of it
		vUp.Init(mMat.m[0][1], mMat.m[1][1], mMat.m[2][1]);
	}

	//now orthogonalize the basis space
	LTVector vForward = vUp.Cross(vRight);
	vForward.Normalize();

	vUp = vRight.Cross(vForward);

	mOutMat.SetBasisVectors(&vRight, &vUp, &vForward);
}

//-----------------------------------------------------------------------------------
// Node Control function
//-----------------------------------------------------------------------------------

static void RagDollListenNodeControlFn(const NodeControlData& Data, void* pUserData)
{
	//some sanity checks
	assert(Data.m_pFromParentTransform && Data.m_pNodeTransform && Data.m_pParentTransform);

	//see if we have no node assocaited with this data. In that case, we just want to return
	//the animation transform
	if(!pUserData)
		return;

	//our user data is a ragdoll node
	CRagDollNode* pRagNode = (CRagDollNode*)pUserData;

	//now get the handle to the ragdoll that this belongs to
	CRagDoll* pRagDoll = pRagNode->m_pRagDoll;

	//determine which position we need to store this in
	uint32 nStorePos = pRagDoll->GetCurrentPosition();

	//we just need to grab the position from the node
	Data.m_pNodeTransform->GetTranslation(pRagNode->m_vPosition[nStorePos]);
}

static void RagDollNodeControlFn(const NodeControlData& Data, void* pUserData)
{
	//some sanity checks
	assert(Data.m_pFromParentTransform && Data.m_pNodeTransform && Data.m_pParentTransform);

	//see if we have no node assocaited with this data. In that case, we just want to return
	//the animation transform
	if(!pUserData)
		return;

	//save the default animation up, we may need to use that
	LTVector vAnimationUp(Data.m_pNodeTransform->m[0][1], Data.m_pNodeTransform->m[1][1], Data.m_pNodeTransform->m[2][1]);

	//our user data is a ragdoll node
	CModelNode* pModelNode = (CModelNode*)pUserData;

	//figure out the correct positional index
	uint32 nPosIndex = pModelNode->m_pRagDoll->GetCurrentPosition();

	//we just need to steal our node orientation and position
	BuildModelNodeOrientation(*pModelNode, vAnimationUp, nPosIndex, *Data.m_pNodeTransform);

	//override the translation
	Data.m_pNodeTransform->m[0][3] = pModelNode->m_pPosNode->m_vPosition[nPosIndex].x;
	Data.m_pNodeTransform->m[1][3] = pModelNode->m_pPosNode->m_vPosition[nPosIndex].y;
	Data.m_pNodeTransform->m[2][3] = pModelNode->m_pPosNode->m_vPosition[nPosIndex].z;
}

//node control function to handle all the non-simulated nodes in an active rag doll
static void RagDollExtraNodeControlFn(const NodeControlData& Data, void* pUserData)
{
	//all we need to do is to apply our binding position transform on top of our parent's
	//transform and all should be good
	*Data.m_pNodeTransform = *Data.m_pParentTransform * *Data.m_pFromParentTransform;
}

//-----------------------------------------------------------------------------------
// CRagDoll
//-----------------------------------------------------------------------------------

CRagDoll::CRagDoll(HOBJECT hModel, uint32 nMaxNodes, uint32 nMaxConstraints, uint32 nMaxModelNodes) :
	m_fCOR(0.001f),
	m_nNumIterations(5),
	m_fPrevFrameDelta(0.1f),
	m_nCurrentIndex(0),
	m_pNodes(NULL),
	m_nNumNodes(0),
	m_nMaxNodes(0),
	m_pConstraints(NULL),
	m_nNumConstraints(0),
	m_nMaxConstraints(0),
	m_pModelNodes(NULL),
	m_nNumModelNodes(0),
	m_nMaxModelNodes(0),
	m_hModel(hModel),
	m_bFirstUpdate(true),
	m_bValidLastUpdateTime(false),
	m_hMovementNode(INVALIDRAGDOLLNODE),
	m_fDragAmount(0.3f),
	m_fFrictionConstant(1.0f)
{
	m_vAccel.Init(0, 0, 0);

	m_pNodes = debug_newa(CRagDollNode, nMaxNodes);
	if(m_pNodes)
		m_nMaxNodes = nMaxNodes;

	m_pConstraints = debug_newa(CRagDollConstraint*, nMaxConstraints);
	if(m_pConstraints)
		m_nMaxConstraints = nMaxConstraints;

	m_pModelNodes = debug_newa(CModelNode, nMaxModelNodes);
	if(m_pModelNodes)
		m_nMaxModelNodes = nMaxModelNodes;
}

CRagDoll::~CRagDoll()
{
	//clean up
	Free();
}


//this returns the positions of the nodes in the current frame
uint32 CRagDoll::GetCurrentPosition()
{
	return m_nCurrentIndex;
}


//this returns the positions of the nodes from the previous frame
uint32 CRagDoll::GetPreviousPosition()
{
	return (m_nCurrentIndex + 1) % 2;
}


//this tells the doll to swap the status of the positions (so current becomes previous, etc)
void CRagDoll::SwapPositions()
{
	m_nCurrentIndex = GetPreviousPosition();
}


//accesses the coefficient of restitution (fraction of remaining energy after a bounce) of the model
float CRagDoll::GetCOR() const
{
	return m_fCOR;
}

void CRagDoll::SetCOR(float fCOR)
{
	m_fCOR = fCOR;
}


//accesses the number of iterations to perform on this model
uint32 CRagDoll::GetNumIterations() const
{
	return m_nNumIterations;
}

void CRagDoll::SetNumIterations(uint32 nNumIterations)
{
	m_nNumIterations = nNumIterations;
}


//gets the stat that the ragdoll is currently in
CRagDoll::ERagDollState CRagDoll::GetState() const
{
	return m_eState;
}

void CRagDoll::SetState(ERagDollState eState)
{
	m_eState = eState;
}

//accessors for the global acceleration that will be applied on the model each frame
const LTVector& CRagDoll::GetAccel() const
{
	return m_vAccel;
}

void CRagDoll::SetAccel(const LTVector& vAccel)
{
	m_vAccel = vAccel;
}

//creates a link from a model node to a ragdoll node that will use the passed in nodes in order
//to determine its position and orientation
bool CRagDoll::CreateModelNode(const char* pszNodeName, HRAGDOLLNODE hPos, HRAGDOLLNODE hPrimaryLink, HRAGDOLLNODE hSecondaryLink)
{
	//make sure things are valid
	if(!m_hModel || !pszNodeName)
		return false;

	//make sure we have room
	if(m_nNumModelNodes >= m_nMaxModelNodes)
		return false;

	//alright, now make sure that our links are valid for orienation
	if((hPos == INVALIDRAGDOLLNODE) || (hPrimaryLink == INVALIDRAGDOLLNODE) || (hSecondaryLink == INVALIDRAGDOLLNODE))
		return false;

	//try and find the node
	HMODELNODE hNode;
	if(g_pLTClient->GetModelLT()->GetNode(m_hModel, const_cast<char*>(pszNodeName), hNode) != LT_OK)
		return false;

	//alright, everything looks good, create it
	CModelNode* pNode = &m_pModelNodes[m_nNumModelNodes];

	pNode->m_pPosNode			= (hPos == INVALIDRAGDOLLNODE) ? NULL : &m_pNodes[hPos];
	pNode->m_pPrimaryLink		= (hPrimaryLink == INVALIDRAGDOLLNODE) ? NULL : &m_pNodes[hPrimaryLink];
	pNode->m_pSecondaryLink		= (hSecondaryLink == INVALIDRAGDOLLNODE) ? NULL : &m_pNodes[hSecondaryLink];
	pNode->m_pRagDoll			= this;
	pNode->m_hModelNode			= hNode;

	//setup the default transformation matrix
	//we first off make the assumption that the right vector is along the primary link, and that this
	//is the most stable of all the axis
	LTVector vRight = pNode->m_pPosNode->m_vPosition[GetCurrentPosition()] - pNode->m_pPrimaryLink->m_vPosition[GetCurrentPosition()];
	vRight.Normalize();

	//determine if we can build a plane out of the basis or not
	LTVector vOther = pNode->m_pSecondaryLink->m_vPosition[GetCurrentPosition()] - pNode->m_pPosNode->m_vPosition[GetCurrentPosition()];
	vOther.Normalize();

	LTVector vUp;

	//alright, build up a basis space that we can orient off of
	vUp = vRight.Cross(vOther);
	vUp.Normalize();

	//now orthogonalize the basis space
	LTVector vForward = vUp.Cross(vRight);
	vForward.Normalize();

	vUp = vRight.Cross(vForward);

	//install that into a matrix so that we can apply the node offset to it
	LTMatrix mMat;
	mMat.SetBasisVectors(&vRight, &vUp, &vForward);
	mMat.Transpose();

	LTMatrix mBindPoseOr;
	g_pLTClient->GetModelLT()->GetBindPoseNodeTransform(m_hModel, pNode->m_hModelNode, mBindPoseOr);
	mBindPoseOr.SetTranslation(0, 0, 0);

	pNode->m_mBasisOrientation	= mMat * mBindPoseOr;
	
	//increment our count
	m_nNumModelNodes++;

	//success
	return true;
}


//creates a node from the specified model node
HRAGDOLLNODE CRagDoll::CreateNode(const char* pszNodeName, float fRadius, float fWeight)
{
	//make sure things are valid
	if(!m_hModel || !pszNodeName)
		return INVALIDRAGDOLLNODE;

	//make sure we have room
	if(m_nNumNodes >= m_nMaxNodes)
		return INVALIDRAGDOLLNODE;

	//try and find the node
	HMODELNODE hNode;
	if(g_pLTClient->GetModelLT()->GetNode(m_hModel, const_cast<char*>(pszNodeName), hNode) != LT_OK)
		return INVALIDRAGDOLLNODE;

	//get the node we are storing this in
	CRagDollNode* pNode = &m_pNodes[m_nNumNodes];

	//ok, we have the node, initialize it
	pNode->m_hModelNode		= hNode;
	pNode->m_pRagDoll		= this;
	pNode->m_fBSphereRadius = fRadius;
	pNode->m_fWeight		= fWeight;

	//update the position of this node
	LTMatrix mTransform;
	g_pLTClient->GetModelLT()->GetBindPoseNodeTransform(m_hModel, pNode->m_hModelNode, mTransform);
	mTransform.GetTranslation(pNode->m_vPosition[GetCurrentPosition()]);
	pNode->m_vPosition[GetPreviousPosition()] = pNode->m_vPosition[GetCurrentPosition()];

	//we need to add the listener onto the node now so that when the ragdoll is activated we can
	//take over from there
	g_pLTClient->GetModelLT()->AddNodeControlFn(m_hModel, hNode, RagDollListenNodeControlFn, pNode);

	//inc our node count
	m_nNumNodes++;
	
	//the rest will be filled out later after the constraints have been done
	return (HRAGDOLLNODE)(m_nNumNodes - 1);
}

//creates a distance constraint between two nodes
bool CRagDoll::AddConstraint(const CRagDollConstraint& Constraint)
{
	//make sure that we have room
	if(m_nNumConstraints >= m_nMaxConstraints)
		return false;

	m_pConstraints[m_nNumConstraints] = Constraint.Clone();

	if(m_pConstraints[m_nNumConstraints])
	{
		m_nNumConstraints++;
		return true;
	}

	//the cloning failed
	return false;
}

//called to finalize the creation of a ragdoll. This must be called before update is, otherwise
//update will fail
bool CRagDoll::ActivateRagDoll()
{
	//now run through our constraints and allow them to initialize and make sure they are valid
	for(uint32 nCurrConstraint = 0; nCurrConstraint < m_nNumConstraints; nCurrConstraint++)
	{
		CRagDollConstraint* pConstraint = m_pConstraints[nCurrConstraint];

		if(!pConstraint->IsValid())
			return false;
	}

	//now we need to get all the nodes animation position and duplicate those
	g_pLTClient->GetModelLT()->ApplyAnimations(m_hModel);

	//we can now also remove the listener node controls since we are taking over now
	uint32 nCurrNode;
	for(nCurrNode = 0; nCurrNode < m_nNumNodes; nCurrNode++)
	{
		CRagDollNode* pNode = &m_pNodes[nCurrNode];

		//remove the listener
		g_pLTClient->GetModelLT()->RemoveNodeControlFn(m_hModel, pNode->m_hModelNode, RagDollListenNodeControlFn, pNode);
	}

	//we now also need to add th extra node control function to all nodes in the model so that they
	//will not play their animation
	g_pLTClient->GetModelLT()->AddNodeControlFn(m_hModel, RagDollExtraNodeControlFn, NULL);

	//alright, now we need to run through and duplicate the node data so that in
	//the worst case scenario the animation is played for 0 frames and we start out with a 0 velocity
	//on our nodes
	for(nCurrNode = 0; nCurrNode < m_nNumNodes; nCurrNode++)
	{
		CRagDollNode* pNode = &m_pNodes[nCurrNode];
		pNode->m_vPosition[GetPreviousPosition()] = pNode->m_vPosition[GetCurrentPosition()];
	}

	//we now need to hook up all the node controllers
	for(uint32 nCurrModelNode = 0; nCurrModelNode < m_nNumModelNodes; nCurrModelNode++)
	{
		CModelNode* pModelNode = &m_pModelNodes[nCurrModelNode];
		g_pLTClient->GetModelLT()->AddNodeControlFn(m_hModel, pModelNode->m_hModelNode, RagDollNodeControlFn, pModelNode);		

		//since we are actually updating this node, we can remove the extra node tracker from it
		g_pLTClient->GetModelLT()->RemoveNodeControlFn(m_hModel, pModelNode->m_hModelNode, RagDollExtraNodeControlFn, NULL);		
	}


	//ok, we can finally switch into the active state
	SetState(eRagDollState_Active);

	//we are active and live, let us rag this doll
	return true;
}



//called to update this model. This should be called before any accessing of the nodes since it
//will only evaluate itself on the first call of the frame
bool CRagDoll::Update()
{
	//we need to determine what the frame delta is
	float fFrameDelta = g_pLTClient->GetFrameTime();

	//see if we just need to grab the time
	if(!m_bValidLastUpdateTime)
	{
		m_fPrevFrameDelta		= fFrameDelta;
		m_bValidLastUpdateTime	= true;
		return true;
	}

	//check for too short of updates
	if(fabs(fFrameDelta) < 0.001f)
	{
		//we don't need to update
		return true;
	}

	//see what mode we are in...
	bool bRV;
	if(GetState() == eRagDollState_Listen)
	{
		//we are in a listen mode, just grab the new positions
		bRV = UpdateListen();
	}
	else
	{
		//we are in an active state. Update our ragdoll
		bRV = UpdateRagDoll(fFrameDelta);

		//update our position if necessary
		if(m_hMovementNode != INVALIDRAGDOLLNODE)
		{
			CRagDollNode& MovementNode = m_pNodes[m_hMovementNode];
			g_pLTClient->SetObjectPos(m_hModel, &MovementNode.m_vPosition[GetCurrentPosition()], TRUE);
		}
	}

	//update our state to reflect the new times
	m_fPrevFrameDelta = fFrameDelta;

	return bRV;
}

//cleans up all allocated memory
void CRagDoll::Free()
{
	//run through and delete each constraint
	for(uint32 nCurrConstraint = 0; nCurrConstraint < m_nNumConstraints; nCurrConstraint++)
	{
		debug_delete(m_pConstraints[nCurrConstraint]);
	}

	debug_deletea(m_pConstraints);
	m_pConstraints = NULL;

	m_nNumConstraints = 0;
	m_nMaxConstraints = 0;	
	
	//remove all the extra controllers from the model
	g_pLTClient->GetModelLT()->RemoveNodeControlFn(m_hModel, RagDollExtraNodeControlFn, NULL);

	debug_deletea(m_pNodes);
	m_pNodes = NULL;

	m_nNumNodes = 0;
	m_nMaxNodes = 0;

	//we need to run through and remove the node controler from each node
	for(uint32 nCurrModelNode = 0; nCurrModelNode < m_nNumModelNodes; nCurrModelNode++)
	{
		CModelNode* pModelNode = &m_pModelNodes[nCurrModelNode];
		g_pLTClient->GetModelLT()->RemoveNodeControlFn(m_hModel, pModelNode->m_hModelNode, RagDollNodeControlFn, pModelNode);
	}

	debug_deletea(m_pModelNodes);
	m_pModelNodes = NULL;

	m_nNumModelNodes = 0;
	m_nMaxModelNodes = 0;
}

//during a listening state, this will just run through and grab new positions
bool CRagDoll::UpdateListen()
{
	//we need to swap our positions since the node controller will be filling in the current
	SwapPositions();

	//so we just need to update the animation, the node controller will listen and fill 
	//in the positions in the current field...
	g_pLTClient->GetModelLT()->ApplyAnimations(m_hModel);

	//success
	return true;
}

//applies the forces onto each node
bool CRagDoll::ApplyForces(float fNewFrameTime)
{
	uint32 nCurrIndex = GetCurrentPosition();
	uint32 nPrevIndex = GetPreviousPosition();

	//the acceleration scale is dependant upon the square of the frame time
	float fAccelScale = fNewFrameTime * fNewFrameTime;

	//calculate a global acceleration to use
	LTVector vGlobalAccel = m_vAccel * fAccelScale;

	float fVelocityScale = (float)pow(1.0f - m_fDragAmount, fNewFrameTime) * fNewFrameTime / m_fPrevFrameDelta;

	//the position is equal to the old position plus the velocity plus the acceleration
	for(uint32 nCurrNode = 0; nCurrNode < m_nNumNodes; nCurrNode++)
	{
		CRagDollNode* pNode = &m_pNodes[nCurrNode];

		//now update the previous position with the updated position
		pNode->m_vPosition[nPrevIndex] = pNode->m_vPosition[nCurrIndex] + fVelocityScale * (pNode->m_vPosition[nCurrIndex] - pNode->m_vPosition[nPrevIndex]) + vGlobalAccel;
	}

	if(m_bFirstUpdate)
		m_pNodes[0].m_vPosition[nPrevIndex] += LTVector(0, 0, -55.0f);

	if(rand() % 10 == 0)
	{
		LTVector vDir = LTVector(((rand() % 10000) - 5000) / 5000.0f, ((rand() % 10000) - 5000) / 5000.0f, ((rand() % 10000) - 5000) / 5000.0f);
		vDir *= 70.0f;

		//m_pNodes[rand() % m_nNumNodes].m_vPosition[nPrevIndex] += vDir;
	}

	//now we need to swap the indices so we will use the new position, and the old current becomes
	//the previous
	SwapPositions();

	m_bFirstUpdate = false;

	//success
	return true;
}

bool CRagDoll::ApplyInterNodeCollisions()
{
	uint32 nPosIndex = GetCurrentPosition();

	//run through each node
	for(uint32 nNode1 = 0; nNode1 < m_nNumNodes; nNode1++)
	{
		for(uint32 nNode2 = nNode1 + 1; nNode2 < m_nNumNodes; nNode2++)
		{
			//see if these nodes intersect
			float fSumRadius = m_pNodes[nNode1].m_fBSphereRadius + m_pNodes[nNode2].m_fBSphereRadius;

			//find the vector between the center points
			LTVector vThroughCenters = m_pNodes[nNode2].m_vPosition[nPosIndex] - m_pNodes[nNode1].m_vPosition[nPosIndex];

			//find the magnitude
			float fDistSqr = vThroughCenters.MagSqr();

			if(fDistSqr < fSumRadius * fSumRadius)
			{
				//they intersect, we need to find the amount of intersection and push each node back
				//by half that amount
				float fDist = (float)sqrt(fDistSqr);
				float fMoveScale = (fSumRadius - fDist) * 0.5f / fDist;

				//now offset the vertices
				m_pNodes[nNode1].m_vPosition[nPosIndex] -= vThroughCenters * fMoveScale;
				m_pNodes[nNode2].m_vPosition[nPosIndex] += vThroughCenters * fMoveScale;
			}
		}
	}

	return true;
}

//runs through all the constraints and applies them the specified number of times
bool CRagDoll::ApplyConstraints(float fNewFrameTime)
{
	uint32 nPosIndex = GetCurrentPosition();
	uint32 nPrevIndex = GetPreviousPosition();

	static const bool bApplyIntersections = true;

	LTVector vNodeBuff[256];
	for(uint32 nCurrNode = 0; nCurrNode < m_nNumNodes; nCurrNode++)
	{
		vNodeBuff[nCurrNode] = m_pNodes[nCurrNode].m_vPosition[nPrevIndex];
	}

	float fFrictionScale = m_fFrictionConstant * fNewFrameTime / (m_fPrevFrameDelta * m_nNumIterations);
	
	//run through the specified number of times and apply constraints
	for(uint32 nIteration = 0; nIteration < m_nNumIterations; nIteration++)
	{
		for(uint32 nCurrConstraint = 0; nCurrConstraint < m_nNumConstraints; nCurrConstraint++)
		{
			m_pConstraints[nCurrConstraint]->Apply(nPosIndex);
		}

		//we need to handle intersections
		if(bApplyIntersections)
		{
			for(uint32 nCurrNode = 0; nCurrNode < m_nNumNodes; nCurrNode++)
			{
				CRagDollNode& Node = m_pNodes[nCurrNode];

				LTVector vHitPos;
				LTVector vHitNormal;

				if(g_pLTClient->IntersectSweptSphere(vNodeBuff[nCurrNode], Node.m_vPosition[nPosIndex], Node.m_fBSphereRadius, vHitPos, vHitNormal))
				{
					Node.m_vPosition[nPosIndex] = vHitPos;

					/*
					//find the amount of penetration
					float fAmountPen = vDir.Dot(vHitPos - vStart);

					//we need to apply friction on it based upon this penetration depth
					float fFriction = fAmountPen * fFrictionScale;

					//find the vector in which it was heading
					LTVector vCurrVel = Node.m_vPosition[nPosIndex] - vNodeBuff[nCurrNode];
					vCurrVel.Normalize();

					//find out how much friction is going to remove
					LTVector vFriction = fFriction * vCurrVel;

					Node.m_vPosition[nPrevIndex] += vFriction;

					if(vCurrVel.Dot(Node.m_vPosition[nPosIndex] - Node.m_vPosition[nPrevIndex]) <= 0.0f)
					{
						Node.m_vPosition[nPrevIndex] = Node.m_vPosition[nPosIndex];
					}
					*/
				}
			}
		}
	}

	ClampVelocities();

	return true;
}

//this function will actually handle the updating of the ragdoll positions based upon
//the forces
bool CRagDoll::UpdateRagDoll(float fCurrFrameTime)
{
	//phase 1: apply the acceleration to all the nodes
	ApplyForces(fCurrFrameTime);

	//phase 2: recursively apply all the constraints
	ApplyConstraints(fCurrFrameTime);

	return true;
}

//gets a specified node, returns NULL if out of range 
//NOTE: This is only intended for the constraints, this is not intended for general use
CRagDollNode* CRagDoll::GetNode(HRAGDOLLNODE hNode)
{
	if(hNode >= m_nNumNodes)
		return NULL;

	return &m_pNodes[hNode];
}

//gets the distance between two ragdoll nodes. This assumes the nodes have had their positions updated
float CRagDoll::GetDistance(HRAGDOLLNODE hNode1, HRAGDOLLNODE hNode2)
{
	assert(hNode1 < m_nNumNodes);
	assert(hNode2 < m_nNumNodes);

	return (m_pNodes[hNode1].m_vPosition[GetCurrentPosition()] - m_pNodes[hNode2].m_vPosition[GetCurrentPosition()]).Mag();
}

//specifies a node to use as the position for the model. Each update the model will be moved
//to the position of the node for purposes of visibility, hit detection, etc
bool CRagDoll::SetMovementNode(HRAGDOLLNODE hNode)
{
	m_hMovementNode = hNode;
	return true;
}

//runs through the nodes looking for nodes that should have their velocity set to 0 (prevents jitters)
bool CRagDoll::ClampVelocities()
{
	//run through all the nodes, find velocities that should be 0, and clamp them
	static const float kfMinVelocitySqr = 0.01f * 0.01f;

	for(uint32 nCurrNode = 0; nCurrNode < m_nNumNodes; nCurrNode++)
	{
		CRagDollNode& Node = m_pNodes[nCurrNode];

		if((Node.m_vPosition[GetCurrentPosition()] - Node.m_vPosition[GetPreviousPosition()]).MagSqr() <= kfMinVelocitySqr)
		{
			//clamp it to 0
			Node.m_vPosition[GetPreviousPosition()] = Node.m_vPosition[GetCurrentPosition()];
		}
	}

	return true;
}

//the amount of drag on the velocities (ie .3 would have 30% of the velocity energy removed per second)
void CRagDoll::SetDragAmount(float fAmount)
{
	m_fDragAmount = fAmount;
}

//the frictional constant. The higher this is the stronger the force of friction
void CRagDoll::SetFrictionConstant(float fVal)
{
	m_fFrictionConstant = fVal;
}

