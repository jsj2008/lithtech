#include "Stdafx.h"
#include "NodeTracker.h"


//--------------------------------------------------------------------------------------------------
// Utility functions
//--------------------------------------------------------------------------------------------------

//given a set of polar coordinates, and limits relative to the Z forward, this will constrain the
//polar coordinates to the limits by finding the closest point in the limits to the provided polar
//point. It will return whether or not the point was in the limits, and the constrained polar point
//itself
static LTPolarCoord ConstrainPolarTargetToLimits(const LTPolarCoord& vPolarPt, const LTRect2f& rLimits, bool& bInLimits)
{
	//assume we are in the limits by default (anyone finding otherwise will set this to false)
	bInLimits = true;

	//the return point
	LTPolarCoord vResult = vPolarPt;

	//we first off want to constrain the Y axis, since it only has a single range and therefore
	//no oddities that need to be handled
	if(vPolarPt.y < rLimits.Top())
	{
		bInLimits = false;
		vResult.y = rLimits.Top();
	}
	if(vPolarPt.y > rLimits.Bottom())
	{
		bInLimits = false;
		vResult.y = rLimits.Bottom();
	}

	//now we need to handle the X, which is trickier since we need to find the closest limit if it is
	//outside of the limits
	if(vPolarPt.x < rLimits.Left())
	{
		bInLimits = false;

		//find the closest point
		float fLeftDist =  rLimits.Left() - vPolarPt.x;
		float fRightDist = (vPolarPt.x + MATH_TWOPI) - rLimits.Right();

		vResult.x = (fLeftDist < fRightDist) ? rLimits.Left() : rLimits.Right();
	}
	if(vPolarPt.x > rLimits.Right())
	{
		bInLimits = false;

		//find the closest point
		float fLeftDist = rLimits.Left() - (vPolarPt.x - MATH_TWOPI);
		float fRightDist = vPolarPt.x - rLimits.Right();

		vResult.x = (fLeftDist < fRightDist) ? rLimits.Left() : rLimits.Right();
	}

	//now give back the result
	return vResult;
}

//this will handle moving from the starting polar position, to the target polar position, up to the maximum
//number of radians allowed. This will return a boolean indicating whether or not the target was within
//the radian range provided, and also return the resulting polar position, restricted to the provided range
static LTPolarCoord MoveTowardsPolarTarget(const LTPolarCoord& vPolarStart, const LTPolarCoord& vPolarTarget, float fMaxRadians, bool& bInRange)
{
	//we want to simply lerp towards the point up to the maximum distance.  We need to use
	// vectors for this, since adding and mulitplying is not a valid polar coordinate operation.
	LTVector2 vToTarget( vPolarTarget.x - vPolarStart.x, vPolarTarget.y - vPolarStart.y );

	//determine the magnitude
	float fMag = vToTarget.Mag();

	//see if it is outside of our range
	bInRange = (fMag < fMaxRadians);

	//if we are in range, simply use the incoming data to ensure stability
	if(bInRange)
	{
		return vPolarTarget;
	}

	//handle degenerate cases
	if(fMag < 0.01f)
		return vPolarStart;

	//interpolate along the target direction
	return vPolarStart.Lerp( vPolarTarget, (fMaxRadians / fMag));
}

//given a vector in aimer space, and the limits in radians in aimer space, this will determine the normalized
//extents of the vector with respect to the limits. This will be in the range of [-1..1] for each axis, and
//will be 0 if the limits are too narrow (nearly equal)
static LTPolarCoord CalculateExtents(const LTPolarCoord& vPolarPt, const LTRect2f& rLimitsRad)
{
	//note that the limits should really be sorted
	LTASSERT(rLimitsRad.IsSorted(), "Error: Invalid limits passed into the extents calculator");

	//Note that the clamping may look unnecessary, but it avoids very minor numerical issues that tend
	//to result in undefined results when near the boundary of the math library functions.
	LTPolarCoord vResult(0.0f, 0.0f);

	//and now convert to the [0..1] range, then to the [-1..1] range
	if(rLimitsRad.GetWidth() > 0.001f)
	{
		vResult.x = (vPolarPt.x - rLimitsRad.Left()) / rLimitsRad.GetWidth();
		vResult.x = (vResult.x - 0.5f) * 2.0f;
	}

	if(rLimitsRad.GetHeight() > 0.001f)
	{
		vResult.y = (vPolarPt.y - rLimitsRad.Top()) / rLimitsRad.GetHeight();
		vResult.y = (vResult.y - 0.5f) * 2.0f;
	}

	return vResult;
}



//--------------------------------------------------------------------------------------------------
// CNodeTracker
//--------------------------------------------------------------------------------------------------

CNodeTracker::CNodeTracker() :
	m_hObject(NULL),

	m_pControlledNodes(NULL),
	m_nNumControlledNodes(0),
	m_fSystemBlendWeight(1.0f),

	m_hAimerNode(INVALID_MODEL_NODE),
	m_rAimerLimits(0.0f, 0.0f, 0.0f, 0.0f),
	m_fMaxSpeedRadPerS(MATH_TWOPI),

	m_eTargetType(eTarget_Aimer),
	m_hTargetObject(NULL),
	m_hTargetNode(INVALID_MODEL_NODE),
	m_vTargetOffset(0.0f, 0.0f, 1.0f),

	m_bDidAimAtTarget(false),
	m_polarAimExtents(0.0f, 0.0f),
	m_vAimDirection(0.0f, 0.0f, 1.0f)
{
	m_rAimerObjectRot.Identity();

	m_hObject.SetReceiver( *this );
}

CNodeTracker::~CNodeTracker()
{
	FreeControlledNodes();
}

//called to set the object associated with this node tracker. This object must be a model
//and this call should be set prior to setting any node information with this tracker.
void CNodeTracker::SetObject(HOBJECT hObject)
{
	//unregister any and all nodes if we have an existing object
	if(m_hObject)
	{
		for(uint32 nCurrNode = 0; nCurrNode < GetNumControlledNodes(); nCurrNode++)
			UnregisterNode(m_hObject, nCurrNode);
	}

	//set this as our actual object
	m_hObject = hObject;

	//and register our nodes for this new object
	if(m_hObject)
	{
		for(uint32 nCurrNode = 0; nCurrNode < GetNumControlledNodes(); nCurrNode++)
			RegisterNode(nCurrNode);
	}
}


//----------------------------------
// Controlled node settings

//called to set the number of controlled nodes that this node tracker controls. Note that
//all settings and nodes currently associated with this tracker will be discarded during
//this operation.
bool CNodeTracker::SetNumControlledNodes(uint32 nNumNodes)
{
	//free our existing nodes
	FreeControlledNodes();

	//now allocate our new list of nodes
	m_pControlledNodes = debug_newa(SControlledNode, nNumNodes);
	if(!m_pControlledNodes)
		return false;

	//run through and set all of our controlled nodes to point back to us so that the node callback
	//can operate properly
	for(uint32 nCurrNode = 0; nCurrNode < nNumNodes; nCurrNode++)
	{
		m_pControlledNodes[nCurrNode].m_pOwner = this;
	}

	m_nNumControlledNodes = nNumNodes;
	return true;
}

//called to set the node that is controlled. The provided index must be in the range
//of [0..Num controlled nodes)
HMODELNODE CNodeTracker::GetControlledNode(uint32 nNode) const
{
	if(nNode >= GetNumControlledNodes())
		return INVALID_MODEL_NODE;
	return m_pControlledNodes[nNode].m_hNode;
}

bool CNodeTracker::SetControlledNode(uint32 nNode, HMODELNODE hNode)
{
	//fail if the node is an invalid index
	if(nNode >= GetNumControlledNodes())
		return false;

	//make sure to unregister the existing node
	UnregisterNode(m_hObject, nNode);

	//and now setup the node that we will belong to
	m_pControlledNodes[nNode].m_hNode = hNode;

	//and register it on the model
	RegisterNode(nNode);

	//success
	return true;
}

//called to set the weight associated with the node. The index should be in the range
//of [0..Num controlled nodes), and the weight should be [0..1] 
float CNodeTracker::GetControlledNodeWeight(uint32 nNode) const
{
	if(nNode > GetNumControlledNodes())
		return 0.0f;
	return m_pControlledNodes[nNode].m_fWeight;
}

bool CNodeTracker::SetControlledNodeWeight(uint32 nNode, float fWeight)
{
	if(nNode > GetNumControlledNodes())
		return false;

	m_pControlledNodes[nNode].m_fWeight = LTCLAMP(fWeight, 0.0f, 1.0f);
	return true;
}

//called to access the blend weight of the entire system. This is in the range
//of [0..1] where 0 means the system is effectively disabled, and 1 is full movement
void CNodeTracker::SetSystemBlendWeight(float fWeight)
{
	m_fSystemBlendWeight = LTCLAMP(fWeight, 0.0f, 1.0f);
}

//----------------------------------
// Aimer node settings

//sets the model node to the provided aimer node
void CNodeTracker::SetAimerNode(HMODELNODE hAimer)
{
	//set this to a new aimer node
	m_hAimerNode = hAimer;

	//and also reset our current state
	m_bDidAimAtTarget = false;
	m_polarAimExtents.Init();
	m_vAimDirection.Init(0.0f, 0.0f, 1.0f);
	m_rAimerObjectRot.Identity();
}

//access to the limits of the aimer node in radians for each direction. The X is limited to
//(-PI..PI), and Y is limited to (-PI/2..PI/2)
void CNodeTracker::SetAimerLimits(const LTRect2f& rLimits)
{
	//determine a rectangle that includes the valid range
    LTRect2f rValidRange(-MATH_PI, -MATH_HALFPI, MATH_PI, MATH_HALFPI);

	//shrink it away from the extremes to avoid stability problems
	rValidRange.Expand(-0.01f);

	//make sure that min < max
	LTRect2f rSortedLimits(rLimits);
	rSortedLimits.Sort();
	m_rAimerLimits = rSortedLimits.GetIntersection(rValidRange);
}

//access to the maximum speed that the aimer can turn at. This is measured in radians per second
//and should be >= 0
void CNodeTracker::SetMaxSpeed(float fMaxSpeed)
{
	m_fMaxSpeedRadPerS = LTMAX(0.0f, fMaxSpeed);
}

//----------------------------------
// Target settings

//clears the targeting information, causing the aimer to simply try to return to the 
//animation's indicated facing
void CNodeTracker::ClearTarget()
{
	SetTargetAimer(LTVector(0.0f, 0.0f, 1.0f));
}

//called to track a local offset in the node tracker space
void CNodeTracker::SetTargetAimer(const LTVector& vOffset)
{
	m_eTargetType	= eTarget_Aimer;
	m_hTargetObject = NULL;
	m_hTargetNode	= INVALID_MODEL_NODE;
	m_vTargetOffset	= vOffset;
}

//this behaves the same as SetTargetAimer, but takes in extents as a parameter and converts
//that to the target position based upon the current extents of the node
void CNodeTracker::SetTargetAimerExtents(const LTPolarCoord& polarExtents)
{
	//convert the extents into polar coordinates. This is done by reversing the equations found
	//in the CalculateExtents
	LTPolarCoord polarDir;
	polarDir.x = m_rAimerLimits.GetWidth() * (polarExtents.x * 0.5f + 0.5f) + m_rAimerLimits.Left();
	polarDir.y = m_rAimerLimits.GetHeight() * (polarExtents.y * 0.5f + 0.5f) + m_rAimerLimits.Top();

	//now convert the polar coordinate version into one that is actually in node space
	LTVector vNodeSpace = polarDir;

	//and target that position in aimer space
	SetTargetAimer(vNodeSpace);
}

//called to set the target to a world space position
void CNodeTracker::SetTargetWorld(const LTVector& vPos)
{
	m_eTargetType	= eTarget_World;
	m_hTargetObject = NULL;
	m_hTargetNode	= INVALID_MODEL_NODE;
	m_vTargetOffset	= vPos;
}

//called to set the target to follow a specified object. If the object is invalid, then nothing
//will be tracked
void CNodeTracker::SetTargetObject(HOBJECT hObj, const LTVector& vOffset)
{
	m_eTargetType	= eTarget_Object;
	m_hTargetObject = hObj;
	m_hTargetNode	= INVALID_MODEL_NODE;
	m_vTargetOffset	= vOffset;
}

//called to set the target to follow a specific model node. If either the model or node is invalid,
//then nothing will be tracked
void CNodeTracker::SetTargetNode(HOBJECT hModel, HMODELNODE hNode, const LTVector& vOffset)
{
	m_eTargetType	= eTarget_Node;
	m_hTargetObject = hModel;
	m_hTargetNode	= (hModel) ? hNode : INVALID_MODEL_NODE;
	m_vTargetOffset	= vOffset;
}

//----------------------------------
// Current status

//called to update the current status of the node controller based upon the update time that
//is provided in seconds. This will return false if the object was in an invalid state
//and could not be updated.
bool CNodeTracker::Update(float fElapsedTimeS)
{
	//first off we need to see if our object is valid, if not, we can't very well update!
	if(!m_hObject || (m_hAimerNode == INVALID_MODEL_NODE))
		return false;

	//get the direction to the target in aimer space
	LTTransform tWorldSpaceAimer;
	g_pLTBase->GetModelLT()->GetNodeTransform(m_hObject, m_hAimerNode, tWorldSpaceAimer, true);
	LTRigidTransform tRigidWorldSpaceAimer(tWorldSpaceAimer.m_vPos, tWorldSpaceAimer.m_rRot);

	//and now get the point in aimer space that we will be targeting
	LTVector vAimerSpaceTarget = GetTargetPosAimerSpace(tRigidWorldSpaceAimer.GetInverse());

	//and normalize it so that way we can use it as a direction
	vAimerSpaceTarget.Normalize();
	
	//convert our aimer space target into polar space coordinates so that we can perform other operations
	//in a stable space
	LTPolarCoord vTargetPolar(vAimerSpaceTarget);

    //we now need to take the vector, and constrain it to the angular limits associated with the
	//aimer node
	bool bInAimerSpace = false;
	LTPolarCoord vConstrainedTargetPolar = ConstrainPolarTargetToLimits(vTargetPolar, m_rAimerLimits, bInAimerSpace);

	//now that we have it constrained to the angular limits, we need to constrain it to the maximum
	//velocity limits
	bool bInAngularVel = false;
	LTPolarCoord polarFinal = MoveTowardsPolarTarget(	LTPolarCoord(m_vAimDirection), vConstrainedTargetPolar, 
													m_fMaxSpeedRadPerS * fElapsedTimeS, bInAngularVel);


	//and now update our status information
	m_bDidAimAtTarget = (bInAimerSpace && bInAngularVel);
	m_polarAimExtents  = CalculateExtents(polarFinal, m_rAimerLimits);
	m_vAimDirection   = polarFinal;


	//update our orientation in object space for use with the node controller functionality
	LTTransform tObjectSpaceAimer;
	g_pLTBase->GetModelLT()->GetNodeTransform(m_hObject, m_hAimerNode, tObjectSpaceAimer, false);
	m_rAimerObjectRot = tObjectSpaceAimer.m_rRot;

	//we now need to run through and dirty all of the nodes that we influence
	for(uint32 nCurrNode = 0; nCurrNode < m_nNumControlledNodes; nCurrNode++)
	{
		//and now dirty this node
		g_pLTBase->GetModelLT()->DirtyNodeTransform(m_hObject, m_pControlledNodes[nCurrNode].m_hNode);
	}

	CLIENT_CODE
	(
		//and also we now need to update the attachments for this object in case any happened to be attached
		//below the point where we are updating
		g_pLTBase->GetModelLT()->UpdateModelAttachments(m_hObject);
	)
    
	//success
	return true;
}

//----------------------------------
// Serialization

//called to save the current state and parameters of this node tracker to the provided stream
void CNodeTracker::Save(ILTMessage_Write* pOutFile)
{
	pOutFile->WriteObject(m_hObject);

	pOutFile->Writeint32(m_nNumControlledNodes);
	for(uint32 nCurrNode = 0; nCurrNode < m_nNumControlledNodes; nCurrNode++)
	{
		pOutFile->Writefloat(m_pControlledNodes[nCurrNode].m_fWeight);
		pOutFile->WriteHMODELNODE(m_pControlledNodes[nCurrNode].m_hNode);
	}
	pOutFile->Writefloat(m_fSystemBlendWeight);

	pOutFile->WriteHMODELNODE(m_hAimerNode);
	pOutFile->WriteLTRect2f(m_rAimerLimits);
	pOutFile->Writefloat(m_fMaxSpeedRadPerS);

	pOutFile->WriteObject(m_hTargetObject);
	pOutFile->WriteHMODELNODE(m_hTargetNode);
	pOutFile->WriteLTVector(m_vTargetOffset);

	pOutFile->Writebool(m_bDidAimAtTarget);
	pOutFile->WriteLTPolarCoord(m_polarAimExtents);
	pOutFile->WriteLTRotation(m_rAimerObjectRot);
	pOutFile->WriteLTVector(m_vAimDirection);
}

//called to load the current state of this node tracker from the provided stream. Note that all
//current status and parameters will be lost during this operation.
bool CNodeTracker::Load(ILTMessage_Read* pInFile)
{
	m_hObject = pInFile->ReadObject();

	//load in the nodes
	uint32 nNumControlled = pInFile->Readint32();

	if(!SetNumControlledNodes(nNumControlled))
		return false;

	for(uint32 nCurrNode = 0; nCurrNode < GetNumControlledNodes(); nCurrNode++)
	{
		m_pControlledNodes[nCurrNode].m_fWeight = pInFile->Readfloat();
		m_pControlledNodes[nCurrNode].m_hNode = pInFile->ReadHMODELNODE();
		RegisterNode(nCurrNode);
	}
	m_fSystemBlendWeight = pInFile->Readfloat();

	m_hAimerNode		= pInFile->ReadHMODELNODE();
	m_rAimerLimits		= pInFile->ReadLTRect2f();
	m_fMaxSpeedRadPerS	= pInFile->Readfloat();

	m_hTargetObject		= pInFile->ReadObject();
	m_hTargetNode		= pInFile->ReadHMODELNODE();
	m_vTargetOffset		= pInFile->ReadLTVector();

	m_bDidAimAtTarget	= pInFile->Readbool();
	m_polarAimExtents	= pInFile->ReadLTPolarCoord();
	m_rAimerObjectRot	= pInFile->ReadLTRotation();
	m_vAimDirection		= pInFile->ReadLTVector();

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CNodeTracker::OnLinkBroken
//
//  PURPOSE:	Implementing classes will have this function called
//				when HOBJECT ref points to gets deleted.
//
// ----------------------------------------------------------------------- //

void CNodeTracker::OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj )
{
	if ( &m_hObject == pRef )
	{
		if( m_pControlledNodes )
		{
			//run through all of our nodes and uninstall them
			for(uint32 nCurrNode = 0; nCurrNode < GetNumControlledNodes(); nCurrNode++)
			{
				UnregisterNode(hObj, nCurrNode);
			}

			//clean up the array
			delete [] m_pControlledNodes;
			m_pControlledNodes = NULL;
		}
		m_nNumControlledNodes = 0;
	}
}


//----------------------------------
// Node Controller Support

//called to handle registering/unregistering a node for the node tracker callback. These
//will do nothing if the specified node is invalid, or there is no associated object
void CNodeTracker::RegisterNode(uint32 nNode)
{
	//we assume that the node is within range
	LTASSERT(nNode < GetNumControlledNodes(), "Error: Invalid node access in CNodeTracker");

	//make sure that we have valid objects to setup our node controller onto
	HMODELNODE hNode = m_pControlledNodes[nNode].m_hNode;
	if(!m_hObject || (hNode == INVALID_MODEL_NODE))
		return;

	//it is valid, so install it
	g_pLTBase->GetModelLT()->AddNodeControlFn(m_hObject, hNode, NodeControllerCB, (void*)&m_pControlledNodes[nNode]);
}

void CNodeTracker::UnregisterNode(HOBJECT hObj, uint32 nNode)
{
	//we assume that the node is within range
	LTASSERT(nNode < GetNumControlledNodes(), "Error: Invalid node access in CNodeTracker");

	//make sure that we have valid objects to setup our node controller onto
	HMODELNODE hNode = m_pControlledNodes[nNode].m_hNode;
	if(!hObj || (hNode == INVALID_MODEL_NODE))
		return;

	//it is valid, so remove it
	g_pLTBase->GetModelLT()->RemoveNodeControlFn(hObj, hNode, NodeControllerCB, (void*)&m_pControlledNodes[nNode]);
}

//the actual node controller function that will be called to apply influence to the nodes
void CNodeTracker::NodeControllerCB(const NodeControlData& NodeData, void* pUserData)
{
	//we should always have user data
	LTASSERT(pUserData, "Error: Invalid user data passed into the node tracker node controller");

	//our user data is really just a controlled node, so extract the data we need out of there
	SControlledNode* pControlled	= (SControlledNode*)pUserData;
	CNodeTracker* pTracker			= pControlled->m_pOwner;

	//determine the weight of this controlled node
	float fWeight = pControlled->m_fWeight * pTracker->GetSystemBlendWeight();

	//if the weight is too close to zero, we can just bail now and save some work. This helps speed
	//up disabled systems significantly
	if(fWeight < 0.01f)
		return;

	//for this node what we want to do is determine how much of the rotation we want to apply in the 
	//node's space, and blend that into our orientation. This is done by moving to the aimer node's space,
	//applying the weighted rotation, and moving back to the node space (but this is only done using
	//orientations, not rotations)

	//Move into aimer space
	LTRotation rInAimerSpace = pTracker->m_rAimerObjectRot.Conjugate() * NodeData.m_pNodeTransform->m_rRot;

	//calculate a stable up vector for the aiming that should be as close to the Y axis in node
	//space as possible. Note that since we know that the movement space does not contain the Y axis
	//we can safely do a cross product without checking for collinear vectors
	LTVector vUp(0.0f, 1.0f, 0.0f);

	//determine the right vector
	LTVector vRight = pTracker->m_vAimDirection.Cross(vUp).GetUnit();
	LTVector vTrueUp = vRight.Cross( pTracker->m_vAimDirection );

	//determine the rotation that we need to apply that is based upon the current aiming direction,
	//and then slerped to there from the default aimer orientation based upon the weight
	LTRotation rAimerTargetOffset;

	//fill in the matrix, and then convert that to an orientation
	LTMatrix3x4 mTemp;
	mTemp.SetBasisVectors(vRight, vTrueUp, pTracker->m_vAimDirection);
	rAimerTargetOffset.ConvertFromMatrix(mTemp);

	LTRotation rIdentity;
	rIdentity.Identity();

	LTRotation rWeightedRotation;
	rWeightedRotation.Slerp(rIdentity, rAimerTargetOffset, fWeight);

	//now we need to rotate the node in aimer space
	LTRotation rInAimerToTarget = rWeightedRotation * rInAimerSpace;

	//now convert back out of aimer space into object space and use that as our new orientation
	NodeData.m_pNodeTransform->m_rRot = pTracker->m_rAimerObjectRot * rInAimerToTarget;
}

//----------------------------------
// Internal utilities

//called to clean up and destroy the controlled node list
void CNodeTracker::FreeControlledNodes()
{
	if( m_pControlledNodes )
	{
		//run through all of our nodes and uninstall them
		for(uint32 nCurrNode = 0; nCurrNode < GetNumControlledNodes(); nCurrNode++)
		{
			UnregisterNode(m_hObject, nCurrNode);
		}

		//clean up the array
		delete [] m_pControlledNodes;
		m_pControlledNodes = NULL;
	}
	m_nNumControlledNodes = 0;
}

//called to get a vector that represents the target orientation in aimer space
LTVector CNodeTracker::GetTargetPosAimerSpace(const LTRigidTransform& tInvAimerSpace) const
{
	//see if we are tracking a model node
	if((m_hTargetNode != INVALID_MODEL_NODE) && m_hTargetObject)
	{
		//try and get the node. If we can't get the node, we will fall back to tracking the object
		LTTransform tNode;
		if(g_pLTBase->GetModelLT()->GetNodeTransform(m_hTargetObject, m_hTargetNode, tNode, true) == LT_OK)
		{
			return tInvAimerSpace * tNode * m_vTargetOffset;
		}
	}

	//see if we are tracking an object. We only check for a valid object since we can fall into here
	//if the node was invalid (still allowing us to track the actual object in place of a node)
	if(m_hTargetObject)
	{
		LTRigidTransform tObject;
		if(g_pLTBase->GetObjectTransform(m_hTargetObject, &tObject) == LT_OK)
		{
			return tInvAimerSpace * tObject * m_vTargetOffset;
		}
	}

	//see if we are tracking a world space position
	if(m_eTargetType == eTarget_World)
	{
		return tInvAimerSpace * m_vTargetOffset;
	}

	//see if we are just tracking an offset in aimer space
	if(m_eTargetType == eTarget_Aimer)
	{
		return m_vTargetOffset;
	}

	//all of the other approaches failed, just target nothing by aiming forward in node space
	return LTVector(0.0f, 0.0f, 1.0f);
}

//----------------------------------
// SControlledNode

CNodeTracker::SControlledNode::SControlledNode() :
	m_pOwner(NULL),
	m_fWeight(0.0f),
	m_hNode(INVALID_MODEL_NODE)
{
}

