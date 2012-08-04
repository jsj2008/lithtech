#include "stdafx.h"
#include "TrackedNode.h"
#include "TrackedNodeMgr.h"

#define DRAWBASIS 0
#if DRAWBASIS
	#ifndef _CLIENTBUILD
		#include "DebugLineSystem.h"
	#endif
#endif
//-----------------------------------------------------------------------------------------
// Math utilities
//-----------------------------------------------------------------------------------------
template <class T>
T Sqr(const T& Val)
{
	return Val * Val;
}


//-----------------------------------------------------------------------------------------
// Matrix utilities
//
// A collection of odd matrix functions that merge together some operations such as transposing
// and multiplication for performance, and also exploit some other features
//-----------------------------------------------------------------------------------------

//build up an inverse matrix from our source. This does a transpose on the
//orientation and a negate on the position. This assumes that the incoming
//matrix is only a composite of rotation and position transforms (no
//shears or scales)
static void BuildInverseMat(const LTMatrix& mIn, LTMatrix& mOut)
{
	LTMatrix mTranslate;
	mTranslate.Identity();
	mTranslate.SetTranslation(-mIn.m[0][3], -mIn.m[1][3], -mIn.m[2][3]);

	mOut.Init(	mIn.m[0][0],	mIn.m[1][0],	mIn.m[2][0],	0.0f,
				mIn.m[0][1],	mIn.m[1][1],	mIn.m[2][1],	0.0f, 
				mIn.m[0][2],	mIn.m[1][2],	mIn.m[2][2],	0.0f, 
				0.0f,			0.0f,			0.0f,			1.0f);

	mOut = mOut * mTranslate;
}

//given two matrices, it will multiply only the UL 3x3 of the matrices and put the result in the
//passed in matrix. This will set the other members to identity
static void MatMul3x3(LTMatrix* pDest, const LTMatrix* pMat1, const LTMatrix* pMat2)
{
    pDest->m[0][0] = pMat1->m[0][0]*pMat2->m[0][0] + pMat1->m[0][1]*pMat2->m[1][0] + pMat1->m[0][2]*pMat2->m[2][0];
    pDest->m[1][0] = pMat1->m[1][0]*pMat2->m[0][0] + pMat1->m[1][1]*pMat2->m[1][0] + pMat1->m[1][2]*pMat2->m[2][0];
    pDest->m[2][0] = pMat1->m[2][0]*pMat2->m[0][0] + pMat1->m[2][1]*pMat2->m[1][0] + pMat1->m[2][2]*pMat2->m[2][0];
    pDest->m[3][0] = 0.0f;

    pDest->m[0][1] = pMat1->m[0][0]*pMat2->m[0][1] + pMat1->m[0][1]*pMat2->m[1][1] + pMat1->m[0][2]*pMat2->m[2][1];
    pDest->m[1][1] = pMat1->m[1][0]*pMat2->m[0][1] + pMat1->m[1][1]*pMat2->m[1][1] + pMat1->m[1][2]*pMat2->m[2][1];
    pDest->m[2][1] = pMat1->m[2][0]*pMat2->m[0][1] + pMat1->m[2][1]*pMat2->m[1][1] + pMat1->m[2][2]*pMat2->m[2][1];
    pDest->m[3][1] = 0.0f;

    pDest->m[0][2] = pMat1->m[0][0]*pMat2->m[0][2] + pMat1->m[0][1]*pMat2->m[1][2] + pMat1->m[0][2]*pMat2->m[2][2];
    pDest->m[1][2] = pMat1->m[1][0]*pMat2->m[0][2] + pMat1->m[1][1]*pMat2->m[1][2] + pMat1->m[1][2]*pMat2->m[2][2];
    pDest->m[2][2] = pMat1->m[2][0]*pMat2->m[0][2] + pMat1->m[2][1]*pMat2->m[1][2] + pMat1->m[2][2]*pMat2->m[2][2];
    pDest->m[3][2] = 0.0f;

    pDest->m[0][3] = 0.0f;
    pDest->m[1][3] = 0.0f;
    pDest->m[2][3] = 0.0f;
    pDest->m[3][3] = 1.0f;
}

//varient of the above with the first matrix needing to be transposed
static void MatMul3Tx3(LTMatrix* pDest, const LTMatrix* pMat1, const LTMatrix* pMat2)
{
    pDest->m[0][0] = pMat1->m[0][0]*pMat2->m[0][0] + pMat1->m[1][0]*pMat2->m[1][0] + pMat1->m[2][0]*pMat2->m[2][0];
    pDest->m[1][0] = pMat1->m[0][1]*pMat2->m[0][0] + pMat1->m[1][1]*pMat2->m[1][0] + pMat1->m[2][1]*pMat2->m[2][0];
    pDest->m[2][0] = pMat1->m[0][2]*pMat2->m[0][0] + pMat1->m[1][2]*pMat2->m[1][0] + pMat1->m[2][2]*pMat2->m[2][0];
    pDest->m[3][0] = 0.0f;

    pDest->m[0][1] = pMat1->m[0][0]*pMat2->m[0][1] + pMat1->m[1][0]*pMat2->m[1][1] + pMat1->m[2][0]*pMat2->m[2][1];
    pDest->m[1][1] = pMat1->m[0][1]*pMat2->m[0][1] + pMat1->m[1][1]*pMat2->m[1][1] + pMat1->m[2][1]*pMat2->m[2][1];
    pDest->m[2][1] = pMat1->m[0][2]*pMat2->m[0][1] + pMat1->m[1][2]*pMat2->m[1][1] + pMat1->m[2][2]*pMat2->m[2][1];
    pDest->m[3][1] = 0.0f;

    pDest->m[0][2] = pMat1->m[0][0]*pMat2->m[0][2] + pMat1->m[1][0]*pMat2->m[1][2] + pMat1->m[2][0]*pMat2->m[2][2];
    pDest->m[1][2] = pMat1->m[0][1]*pMat2->m[0][2] + pMat1->m[1][1]*pMat2->m[1][2] + pMat1->m[2][1]*pMat2->m[2][2];
    pDest->m[2][2] = pMat1->m[0][2]*pMat2->m[0][2] + pMat1->m[1][2]*pMat2->m[1][2] + pMat1->m[2][2]*pMat2->m[2][2];
    pDest->m[3][2] = 0.0f;

    pDest->m[0][3] = 0.0f;
    pDest->m[1][3] = 0.0f;
    pDest->m[2][3] = 0.0f;
    pDest->m[3][3] = 1.0f;
}

//varient of the above with the second matrix needing to be transposed
static void MatMul3x3T(LTMatrix* pDest, const LTMatrix* pMat1, const LTMatrix* pMat2)
{
    pDest->m[0][0] = pMat1->m[0][0]*pMat2->m[0][0] + pMat1->m[0][1]*pMat2->m[0][1] + pMat1->m[0][2]*pMat2->m[0][2];
    pDest->m[1][0] = pMat1->m[1][0]*pMat2->m[0][0] + pMat1->m[1][1]*pMat2->m[0][1] + pMat1->m[1][2]*pMat2->m[0][2];
    pDest->m[2][0] = pMat1->m[2][0]*pMat2->m[0][0] + pMat1->m[2][1]*pMat2->m[0][1] + pMat1->m[2][2]*pMat2->m[0][2];
    pDest->m[3][0] = 0.0f;

    pDest->m[0][1] = pMat1->m[0][0]*pMat2->m[1][0] + pMat1->m[0][1]*pMat2->m[1][1] + pMat1->m[0][2]*pMat2->m[1][2];
    pDest->m[1][1] = pMat1->m[1][0]*pMat2->m[1][0] + pMat1->m[1][1]*pMat2->m[1][1] + pMat1->m[1][2]*pMat2->m[1][2];
    pDest->m[2][1] = pMat1->m[2][0]*pMat2->m[1][0] + pMat1->m[2][1]*pMat2->m[1][1] + pMat1->m[2][2]*pMat2->m[1][2];
    pDest->m[3][1] = 0.0f;

    pDest->m[0][2] = pMat1->m[0][0]*pMat2->m[2][0] + pMat1->m[0][1]*pMat2->m[2][1] + pMat1->m[0][2]*pMat2->m[2][2];
    pDest->m[1][2] = pMat1->m[1][0]*pMat2->m[2][0] + pMat1->m[1][1]*pMat2->m[2][1] + pMat1->m[1][2]*pMat2->m[2][2];
    pDest->m[2][2] = pMat1->m[2][0]*pMat2->m[2][0] + pMat1->m[2][1]*pMat2->m[2][1] + pMat1->m[2][2]*pMat2->m[2][2];
    pDest->m[3][2] = 0.0f;

    pDest->m[0][3] = 0.0f;
    pDest->m[1][3] = 0.0f;
    pDest->m[2][3] = 0.0f;
    pDest->m[3][3] = 1.0f;
}



//-----------------------------------------------------------------------------------------
// Node Utilities
//-----------------------------------------------------------------------------------------

//given a node, it will determine the local position where it needs to track
static LTVector FindTargetInLocalPos(const CTrackedNode* pNode, const LTMatrix& mInvMat, const LTMatrix& mAnimMat)
{
	assert(pNode && pNode->m_pNodeMgr);

	//get the base interface
	ILTCSBase* pILTBase = pNode->m_pNodeMgr->GetBaseInterface();
	assert(pILTBase);

	LTVector vTarget;

	switch(pNode->m_eTrackMode)
	{
	case CTrackedNode::TRACK_NODE:
		{
			//we are tracking a node position, so first find out where that
			//node is...
			LTransform Transform;
			pNode->m_pNodeMgr->GetBaseInterface()->GetModelLT()->GetNodeTransform(pNode->m_hTrackObject, pNode->m_hTrackNode, Transform, true);
			vTarget = mInvMat * (Transform.m_Pos + pNode->m_vTrackOffset);
		}
		break;

	case CTrackedNode::TRACK_OBJECT:
		{
			//find the position of this object
			pILTBase->GetObjectPos(pNode->m_hTrackObject, &vTarget);
			vTarget = mInvMat * (vTarget + pNode->m_vTrackOffset);
		}
		break;

	case CTrackedNode::TRACK_WORLDPOS:
		{
			vTarget = mInvMat * pNode->m_vTrackOffset;
		}
		break;

	case CTrackedNode::TRACK_OBJSPACEPOS:
		{
			//get our object space
			LTVector vModelPos;
			pNode->m_pNodeMgr->GetBaseInterface()->GetObjectPos(pNode->m_hModel, &vModelPos);

			LTRotation rModelOr;
			pNode->m_pNodeMgr->GetBaseInterface()->GetObjectRotation(pNode->m_hModel, &rModelOr);

			//make a transform for this point
			LTMatrix mModelOr;
			rModelOr.ConvertToMatrix(mModelOr);

			//now convert the point into world space, and then into node space
			vTarget = mInvMat * (vModelPos + mModelOr * pNode->m_vTrackOffset);
		}
		break;

	case CTrackedNode::TRACK_LOCALPOS:
		{
			vTarget = pNode->m_vTrackOffset;			
		}
		break;

	case CTrackedNode::TRACK_ANIMATION:
		{
			//get the animation in the binding space
			vTarget = mInvMat * (mAnimMat * pNode->m_vTrackOffset);
		}
		break;

	default:
		{
			assert(false);
		}
		break;
	}

	return vTarget;
}

//given a node, it will update the current time and return the time elapsed since
//the last date in seconds
static float UpdateNodeTime(CTrackedNode* pNode)
{
	clock_t CurrTime = clock();

	//special case when the previous time was 0, indicating that this is the first
	//update

	float fTimeDelta = 0.0f;

	if(pNode->m_LastUpdate != 0)
		fTimeDelta = (float)(CurrTime - pNode->m_LastUpdate) / (float)CLOCKS_PER_SEC;

	//update our time
	pNode->m_LastUpdate = CurrTime;

	return fTimeDelta;
}

//given a node and a time delta, it will return the cosine of the angle that is the
//maximum that the node can move
static float FindMaxAnglularVelocity(const CTrackedNode* pNode, float fFrameTime)
{
	//our total max angle is the nodes max which is in rad/sec, and scale that
	//based upon our time so that it is in radians
	float fMaxAngVel = pNode->m_fMaxAngVel * fFrameTime;

	//now we need to clamp this to be only up to 180 degrees, since beyond we can
	//get all sorts of wrapping issues
	if(fMaxAngVel > MATH_PI)
		fMaxAngVel = MATH_PI;

	//now return the cosine of that value
	return fMaxAngVel;
}

//given the width and height of an ellipse perp to the Z axis that is 1 unit out, and
//a normal for a plane that is to slice it, as well as the desired vector in basis
//space, it will return the constrained vector
static LTVector FindConstrainedVector(const LTVector& vTarget, float fWidth, float fHeight, bool& bOutOfBounds)
{
	//setup
	bOutOfBounds = false;

	//so what we are doing is projecting the look at vector onto the plane, and we can then use
	//that line to determine where it will intersect the ellipse

	//we project by scaling the points to be on the plane Z=1 simply tossing the Z component
	float fX = vTarget.x;
	float fY = vTarget.y;

	//here we need to handle the degenerate ellipses where either the width or height is 0, in
	//these cases we don't want to find the border of the ellipse, but instead handle it as a projection
	//onto a line segment. Note that this will also handle the case of w = h = 0 and just return the Z
	if(fHeight < 0.001f)
	{
		//we need to build two vectors in the plane, and find their dot product with the +Z axis

		//for the constraining vector we can just take a shortcut and find the Z assuming the
		//vector is (X, 0, 1)
		float fConstrainZ = 1.0f / (float)sqrt(fWidth * fWidth + 1.0f);

		//the other vector is formed by projecting onto the XZ plane and finding it's Z value
		float fBindMag = (float)sqrt(fX * fX + vTarget.z * vTarget.z);

		//we need to check and see if it is actually on the up vector, in which case we can't handle
		//it and need to just return
		if(fBindMag < 0.001f)
		{
			bOutOfBounds = true;
			return LTVector(0, 0, 1.0f);
		}

		float fBindZ = vTarget.z / fBindMag;

		//now we take the larger of the two
		if(fBindZ > fConstrainZ)
		{
			//we are in the constraint, return that
			return LTVector(fX / fBindMag, 0.0f, fBindZ);
		}
		//we need to use the constraint (compensating for left/right)
		bOutOfBounds = true;

		if(fX < 0.0f) 
			return LTVector(-fWidth * fConstrainZ, 0.0f, fConstrainZ);
		else
			return LTVector(fWidth * fConstrainZ, 0.0f, fConstrainZ);
	}
	else if(fWidth < 0.001f)
	{
		//we need to build two vectors in the plane, and find their dot product with the +Z axis

		//for the constraining vector we can just take a shortcut and find the Z assuming the
		//vector is (0, Y, 1)
		float fConstrainZ = 1.0f / (float)sqrt(fHeight * fHeight + 1.0f);

		//the other vector is formed by projecting onto the YZ plane and finding it's Z value
		float fBindMag = (float)sqrt(fY * fY + vTarget.z * vTarget.z);

		//we need to check and see if it is actually on the right vector, in which case we can't handle
		//it and need to just return
		if(fBindMag < 0.001f)
		{
			bOutOfBounds = true;
			return LTVector(0, 0, 1.0f);
		}

		float fBindZ = vTarget.z / fBindMag;

		//now we take the larger of the two
		if(fBindZ > fConstrainZ)
		{
			//we are in the constraint, return that
			return LTVector(0.0f, fY / fBindMag, fBindZ);
		}
		//we need to use the constraint (compensating for up/down)
		bOutOfBounds = true;

		if(fY < 0.0f) 
			return LTVector(0.0f, -fHeight * fConstrainZ, fConstrainZ);
		else
			return LTVector(0.0f, fHeight * fConstrainZ, fConstrainZ);	
	}

	//we are at -Z, this must be handled at a higher level function to get any meaningful
	//behavior
	assert(vTarget.z >= -0.99f);

	if(vTarget.z > 0.99f)
	{
		//we are at +Z, assume that it is not at the limits
		return vTarget;
	}

	//what we are doing is building up a plane that contains the two vectors, target and forward.
	//we are then intersecting that plane with the ellipse

	//ok, the checks above guarantee we can generate a normal, and don't have a zero sized ellipse.
	LTVector vNormal = vTarget.Cross(LTVector(0.0f, 0.0f, 1.0f));

	//we solve for Normal . EllipseVec = 0 and the ellipse equation for Y
	float fA = (Sqr(vNormal.y * fHeight) + Sqr(vNormal.x * fWidth)) / Sqr(fHeight);
	float fB = 2.0f * vNormal.y * vNormal.z;
	float fC = (Sqr(vNormal.z) - Sqr(vNormal.x * fWidth));

	//now we determine whether or not we want to use the + or - side since the quadratic equation
	//gives us both
	float fYSignScale = (vTarget.y < 0.0f) ? -1.0f : 1.0f;

	//calculate the descriminant
	float fDescriminant = Sqr(fB) - 4.0f * fA * fC;

	//figure out what the ellipse Y is
	float fEllipseY = (-fB + fYSignScale * (float)sqrt(LTMAX(0.0f, fDescriminant))) / (2.0f * fA);

	//we now have the ellipse Y, lets find the X of the ellipse
	float fXSignScale = (vTarget.x < 0.0f) ? -1.0f : 1.0f;
	float fEllipseX = fXSignScale * (float)sqrt( LTMAX( 0.0f, (1.0f - Sqr(fEllipseY / fHeight))) * Sqr(fWidth));

	//we now have the X and Y components of the ellipse, we can now build up our constrained
	//vector with those
	LTVector vFinal(fEllipseX, fEllipseY, 1.0f);
	vFinal.Normalize();

	//now we can see if it is inside or not
	if(vFinal.z < vTarget.z)
	{
		//we are inside
		return vTarget;
	}
	else
	{
		//flag out of bounds
		bOutOfBounds = true;

		//return the limit that we found
		return vFinal;
	}
}

//given a node along with where it is currently looking and where it wants to look, it will constrain
//it based upon the node's maximum velocity constraints and return the constrained node
static LTVector ConstrainToMaxAngularVel(const CTrackedNode* pNode, float fFrameTime, const LTVector& vCurrLook, const LTVector& vLookGoal, bool& bLookingAt)
{
	//ok, so now we know where we are looking (the actual space) and where we want
	//to look vToTarget, so lets find where we can look given our angular max
	float fActualToTargetAng = vLookGoal.Dot(vCurrLook);

	//get the maximum angle we can move
	float fMaxAngVel	= FindMaxAnglularVelocity(pNode, fFrameTime);
	float fCosMaxAngVel = (float)cos(fMaxAngVel);

	//if we are at 180 degrees difference, much can go wrong, so ensure that we aren't,
	//but if we are, we just want to keep looking in the direction that we currently
	//are
	if(fActualToTargetAng >= fCosMaxAngVel)
	{
		//the look target is within our reach, so just go there
		bLookingAt  = true;

		return vLookGoal;
	}
	else
	{
		//form a right vector that passes through the arc that we are interpolating
		//upon 
		LTVector vRight = vCurrLook - (vLookGoal - vCurrLook) / (fActualToTargetAng - 1.0f);
		vRight.Normalize();

		//now we can get our values based upon that space
		return fCosMaxAngVel * vCurrLook + (float)sin(fMaxAngVel) * vRight;
	}	
}

//this function takes a node and a target look position and determines if it is in discomfort
static bool IsInDiscomfort(const CTrackedNode* pNode, const LTVector& vLookAt)
{
	//we now need to determine if the node is in discomfort
	if((pNode->m_fTanXDiscomfort < 0.01f) || (pNode->m_fTanYDiscomfort < 0.01f))
	{
		//the area is so small that it should always
		//be considered in discomfort
		return true;
	}

	//we can't take any early outs
	bool bDiscomfort = false;
	FindConstrainedVector(vLookAt, pNode->m_fTanXDiscomfort, pNode->m_fTanYDiscomfort, bDiscomfort);

	return bDiscomfort;
}

//given a node, this will take the target vector and apply the animation's inverse orientation on the 
//target space so that it will always try to line up the forward of the model
static LTVector ApplyInverseAnimationOrientation(const CTrackedNode* pNode, const LTMatrix& mAnimSpace, const LTVector vLookAt)
{
	//we do, so first off, get the orientation of the model
	LTRotation ModelRot;
	pNode->m_pNodeMgr->GetBaseInterface()->GetObjectRotation(pNode->m_hModel, &ModelRot);

	//convert that rotation to a matrix
	LTMatrix mModelOr;
	ModelRot.ConvertToMatrix(mModelOr);

	LTMatrix mMappedAnimSpace;
	MatMul3Tx3(&mMappedAnimSpace, &mModelOr, &mAnimSpace);

	//now build up a matrix that is the difference of orientation with respect to the animation
	LTMatrix mAnimDiff;
	MatMul3Tx3(&mAnimDiff, &pNode->m_mInvTargetTransform, &mMappedAnimSpace);

	//now handle the transformation into animation space
	LTVector vTransformed;
	mAnimDiff.Apply3x3(vLookAt, vTransformed);
	return vTransformed;
}

static LTVector FindLookDirectionLS(const CTrackedNode* pNode, const LTMatrix& mBindingSpace,
									const LTMatrix& mAnimSpace, float fFrameTime,
									bool& bDiscomfort, bool& bThreshold, bool& bLookingAt)
{
	//default the looking at to false
	bLookingAt	= false;
	bDiscomfort = false;
	bThreshold	= false;

	//find an inverse matrix for us to use
	LTMatrix mInvMat;
	BuildInverseMat(mBindingSpace, mInvMat);

	//alright, now we need to find the position that we are trying to look at
	//in world space...
	LTVector vTargetPos = FindTargetInLocalPos(pNode, mInvMat, mAnimSpace);
	
	//we may need to bail if we were unable to find the look position
	if(vTargetPos.MagSqr() < 0.01f)
		return pNode->m_vActualForward;

	//see if we need to change the orientation of this based upon the animations position
	if(pNode->m_bOrientFromAnim)
	{
		vTargetPos = ApplyInverseAnimationOrientation(pNode, mAnimSpace, vTargetPos);
	}

	//we now need to reorient the target position so that it will be aligned with the constraint cone
	pNode->m_mInvTargetTransform.Apply3x3(vTargetPos);

	if( vTargetPos.MagSqr( ) < 0.01f )
 	{
		ASSERT( !"FindLookDirectionLS: Invalid target position." );
		return pNode->m_vActualForward;
 	}

	//now we need to find the vector from the current position to the point we want
	//to look at
	LTVector vToTarget = vTargetPos;
	vToTarget.Normalize();

	//see if the player is behind us, if so, just map this into the forward sphere and indicate that this node
	//is in discomfort and at the threshold
	if(vToTarget.z < 0.0f)
	{
		vToTarget.z = -vToTarget.z;
		bDiscomfort = true;
		bThreshold	= true;
	}

	//we now know that this is valid, so constrain it to a cone
	bool bOutOfBounds = false;
	vToTarget = FindConstrainedVector(vToTarget, pNode->m_fTanXThreshold, pNode->m_fTanYThreshold, bOutOfBounds);

	//now constrain the target based upon angular velocity
	vToTarget = ConstrainToMaxAngularVel(pNode, fFrameTime, pNode->m_vActualForward, vToTarget, bLookingAt);

	//if we are looking at the actual position, we need to set threshold to whether or not we constrained earlier
	if(bLookingAt)
		bThreshold = bThreshold || bOutOfBounds;

	//determine if this node is uncomftorable or not
	bDiscomfort = bDiscomfort || bOutOfBounds || IsInDiscomfort(pNode, vToTarget);

	return vToTarget;
}


//Given a node, and the final matrix to setup, this setup the final matrix. It
//assumes that the acutal space is valid from last frame
static void	SetupFinalTransform(CTrackedNode* pNode, const LTVector vLookAt, const LTMatrix &mBindingSpace, LTMatrix* pOutMat)
{
	//always look down the target vector
	pNode->m_vActualForward = vLookAt;

	//generate the right off of the alignment up so it won't roll
	pNode->m_vActualRight	= pNode->m_vActualForward.Cross(pNode->m_vAlignUp);
	pNode->m_vActualRight.Normalize();

	//now generate the up vector
	pNode->m_vActualUp		= pNode->m_vActualRight.Cross(pNode->m_vActualForward);
	pNode->m_vActualUp.Normalize();
	
	//handle the conversion from the internal Z forward space to the arbitrary axis space
	const LTMatrix& m = pNode->m_mInvTargetTransform;
	LTVector vBlendedRight		= m.El(0, 0) * pNode->m_vActualRight + m.El(1, 0) * pNode->m_vActualUp + m.El(2, 0) * pNode->m_vActualForward;
	LTVector vBlendedUp			= m.El(0, 1) * pNode->m_vActualRight + m.El(1, 1) * pNode->m_vActualUp + m.El(2, 1) * pNode->m_vActualForward;
	LTVector vBlendedForward	= vBlendedUp.Cross(vBlendedRight);

	//convert the actuals into world space
	LTVector vFinalRight, vFinalUp, vFinalForward;

	LTMatrix mToGlobalTrans;
	MatMul3x3T(&mToGlobalTrans, &mBindingSpace, &pNode->m_mInvTargetTransform);

	mToGlobalTrans.Apply3x3(vBlendedRight, vFinalRight);
	mToGlobalTrans.Apply3x3(vBlendedUp, vFinalUp);
	mToGlobalTrans.Apply3x3(vBlendedForward, vFinalForward);
	
	//now that we have the actual transform, apply it to the matrix
	pOutMat->SetBasisVectors2(&vFinalRight, &vFinalUp, &vFinalForward);
}

//given an input space, it will convert that to the appropriate space and update the actual vectors
//so that they will match. This reduces popping of animations
static void SyncOrientation(CTrackedNode* pNode, const LTMatrix& mAnimSpace, const LTMatrix& mBindingSpace)
{
	//we need to first convert the orienation from animation space to binding space
	LTMatrix mInBinding;
	MatMul3Tx3(&mInBinding, &mBindingSpace, &mAnimSpace);

	//now we rotate so that our axis are pointing the right way
	LTMatrix mInActual;
	MatMul3x3(&mInActual, &pNode->m_mInvTargetTransform, &mInBinding);

	//we can now extract out the basis vectors for our actual vectors
	LTVector vBlendedRight, vBlendedUp, vBlendedForward;
	mInActual.GetBasisVectors(&vBlendedRight, &vBlendedUp, &vBlendedForward);

	//and now do the vector swapping to build the actual basis space
	LTMatrix& m = pNode->m_mInvTargetTransform;
	pNode->m_vActualRight		= m.El(0, 0) * vBlendedRight + m.El(0, 1) * vBlendedUp + m.El(0, 2) * vBlendedForward;
	pNode->m_vActualUp			= m.El(1, 0) * vBlendedRight + m.El(1, 1) * vBlendedUp + m.El(1, 2) * vBlendedForward;
	pNode->m_vActualForward		= pNode->m_vActualUp.Cross(pNode->m_vActualRight);


	//figure out the vector we should use for alignment to prevent roll
	pNode->m_mInvTargetTransform.GetBasisVectors(&vBlendedRight, &vBlendedUp, &vBlendedForward);
	pNode->m_vAlignUp			= m.El(1, 0) * vBlendedRight + m.El(1, 1) * vBlendedUp + m.El(1, 2) * vBlendedForward;
}


//Handles updating a node that has its orientation cloned
static void UpdateClonedNode(CTrackedNode* pNode, const LTMatrix &mBindingSpace, LTMatrix* pOutMat)
{
	//just grab the cloned node, and steal the data then setup the matrix
	pNode->m_vActualForward		= pNode->m_pMimicNode->m_vActualForward;
	pNode->m_vActualUp			= pNode->m_pMimicNode->m_vActualUp;
	pNode->m_vActualRight		= pNode->m_pMimicNode->m_vActualRight;

	//also copy over the properties
	pNode->m_bInDiscomfort		= pNode->m_pMimicNode->m_bInDiscomfort;
	pNode->m_bAtMaxThreshold	= pNode->m_pMimicNode->m_bAtMaxThreshold;
	pNode->m_bLookingAtTarget	= pNode->m_pMimicNode->m_bLookingAtTarget;

	//handle the conversion from the internal Z forward space to the arbitrary axis space
	const LTMatrix& m = pNode->m_pMimicNode->m_mInvTargetTransform;
	LTVector vBlendedRight		= m.El(0, 0) * pNode->m_vActualRight + m.El(1, 0) * pNode->m_vActualUp + m.El(2, 0) * pNode->m_vActualForward;
	LTVector vBlendedUp			= m.El(0, 1) * pNode->m_vActualRight + m.El(1, 1) * pNode->m_vActualUp + m.El(2, 1) * pNode->m_vActualForward;
	LTVector vBlendedForward	= vBlendedUp.Cross(vBlendedRight);

	//convert the actuals into world space
	LTVector vFinalRight, vFinalUp, vFinalForward;

	LTMatrix mToGlobalTrans;
	MatMul3x3T(&mToGlobalTrans, &mBindingSpace, &pNode->m_mInvTargetTransform);

	mToGlobalTrans.Apply3x3(vBlendedRight, vFinalRight);
	mToGlobalTrans.Apply3x3(vBlendedUp, vFinalUp);
	mToGlobalTrans.Apply3x3(vBlendedForward, vFinalForward);

	//now that we have the actual transform, apply it to the matrix
	pOutMat->SetBasisVectors2(&vFinalRight, &vFinalUp, &vFinalForward);

	//also update our position
	pOutMat->GetTranslation(pNode->m_vActualPos);
}

//Handles updating a node that moves with standard controls
static void UpdateNormalNode(CTrackedNode* pNode, const LTMatrix& mBindingSpace, 
							 float fFrameTime, LTMatrix* pOutMat)
{
	LTVector vLSLookPos;

	//see if we need to sync our orientaion
	if(pNode->m_bSyncOrientation)
	{
		//we do
		SyncOrientation(pNode, *pOutMat, mBindingSpace);
		//we can now clear that flag
		pNode->m_bSyncOrientation = false;
	}

	//see if we have a child in comfort, if we do, we don't need to update, otherwise
	//we do
	if(pNode->m_pChild && !pNode->m_pChild->m_bInDiscomfort)
	{
		//we can just use the last position
		vLSLookPos = pNode->m_vActualForward;
	}
	else
	{
		//find the vector where we are to look in local space
		bool bDiscomfort, bThreshold, bLookingAt;
		vLSLookPos = FindLookDirectionLS(pNode, mBindingSpace, *pOutMat, fFrameTime, bDiscomfort, bThreshold, bLookingAt);

		//now setup the properties of the node
		pNode->m_bInDiscomfort		= bDiscomfort;
		pNode->m_bAtMaxThreshold	= bThreshold;
		pNode->m_bLookingAtTarget	= bLookingAt;
	}

	//now we need to setup the final transform...
	SetupFinalTransform(pNode, vLSLookPos, mBindingSpace, pOutMat);

	//also update our position
	pOutMat->GetTranslation(pNode->m_vActualPos);
}


//the actual node control function for the node tracking
void TrackedNodeControlFn(const NodeControlData& Data, void* pUser)
{
	//sanity checks
	assert(pUser && Data.m_hModel && Data.m_pNodeTransform && (Data.m_hNode != INVALID_MODEL_NODE));

	//first things first, get our tracked node out of this data
	CTrackedNode* pNode = (CTrackedNode*)pUser;

	//update the time of this node 
	float fFrameTime = UpdateNodeTime(pNode);

#if DRAWBASIS
	#ifndef _CLIENTBUILD)
			DebugLineSystem& system = LineSystem::GetSystem(pNode, "ShowBasisVectors");
			system.Clear();
	#endif
#endif

	//try to early out on updating
	if(!pNode->m_bEnabled)
		return;

	if(pNode->m_pMimicNode && !pNode->m_pMimicNode->m_bEnabled)
		return;

	//calculate the binding space from our input parameters
	LTMatrix mBindingSpace = *Data.m_pParentTransform * *Data.m_pFromParentTransform;

	//however, we want the translation to be at the translation of the animated position, so override that
	LTVector vTranslation;
	Data.m_pNodeTransform->GetTranslation(vTranslation);
	mBindingSpace.SetTranslation(vTranslation);


	if(pNode->m_pMimicNode)
	{
		//this node just steals its orientation from another node
		UpdateClonedNode(pNode, mBindingSpace, Data.m_pNodeTransform);
	}
	else
	{
		//See if this node wants to ignore any animation at all for the binding space, this is commonly
		//done when the constraints want to be completely independant
		if(pNode->m_bIgnoreParentAnimation)
		{
			//grab the actual original binding transformation
			LTMatrix mOriginalBinding;
			pNode->m_pNodeMgr->GetBaseInterface()->GetModelLT()->GetBindPoseNodeTransform(pNode->m_hModel, pNode->m_hNode, mOriginalBinding);

			//steal the orientation from that and save it in the binding space
			LTVector vTranslation;
			mBindingSpace.GetTranslation(vTranslation);

			//apply the orientation of the model
			LTRotation ModelRot;
			pNode->m_pNodeMgr->GetBaseInterface()->GetObjectRotation(pNode->m_hModel, &ModelRot);

			//convert that rotation to a matrix
			LTMatrix mModelOr;
			ModelRot.ConvertToMatrix(mModelOr);

			MatMul3x3(&mBindingSpace, &mModelOr, &mOriginalBinding);
			mBindingSpace.SetTranslation(vTranslation);
		}

#if DRAWBASIS
	#ifndef _CLIENTBUILD)
			DebugLineSystem& system = LineSystem::GetSystem(pNode, "ShowBasisVectors");
			LTVector vInitialPos, vUp, vForward, vRight;
			mBindingSpace.GetBasisVectors(&vUp, &vForward, &vRight);
			mBindingSpace.GetTranslation(vInitialPos);
			system << LineSystem::Arrow( vInitialPos, vInitialPos+vUp.Unit()*48,		Color::Green );
			system << LineSystem::Arrow( vInitialPos, vInitialPos+vForward.Unit()*48,	Color::Blue );
			system << LineSystem::Arrow( vInitialPos, vInitialPos+vRight.Unit()*48,		Color::DkRed );
	#endif
#endif
		//we need to actually do full tracking of this node
		UpdateNormalNode(pNode, mBindingSpace, fFrameTime, Data.m_pNodeTransform);
	}

	//handle auto disabling
	if(pNode->m_bAutoDisable)
	{
		//see if we are looking at the destination, if so, disable ourselves
		if(pNode->m_bLookingAtTarget)
			pNode->m_bEnabled = false;

		//clear the auto disable flag now as well
		pNode->m_bAutoDisable = false;
	}

	//all done....
}

