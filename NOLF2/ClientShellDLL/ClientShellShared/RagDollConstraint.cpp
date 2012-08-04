#include "stdafx.h"
#include "RagDollConstraint.h"
#include "RagDoll.h"
#include "RagDollNode.h"

//------------------------------------------------------------------------------------------------
// Utility functions
//------------------------------------------------------------------------------------------------

//given a weight value, it will determine if it is above 0, and if it is will use that. Otherwise
//it will look at the weighting of each node and use that to determine the value
void DetermineNodeWeights(float fDefaultWeight, const CRagDollNode* pNode1, const CRagDollNode* pNode2, float& fWeight1, float& fWeight2)
{
	if(fDefaultWeight < 0.0f)
	{
		//we want to determine the weighting based upon the weight of each node
		fWeight1 = pNode2->m_fWeight / (pNode1->m_fWeight + pNode2->m_fWeight);
	}
	else
	{
		fWeight1 = fDefaultWeight;
	}
	
	//clamp it to ensure it is valid
	fWeight1 = LTCLAMP(fWeight1, 0.0f, 1.0f);

	//the other weight is simply the compliment of the first
	fWeight2 = 1.0f - fWeight1;
}


//------------------------------------------------------------------------------------------------
// CRagDollDistanceConstraint
//------------------------------------------------------------------------------------------------

CRagDollDistanceConstraint::CRagDollDistanceConstraint(CRagDoll* pRagDoll, HRAGDOLLNODE hNode1, HRAGDOLLNODE hNode2, float fDistance, float fNode1Weight)
{
	m_pNode[0] = NULL;
	m_pNode[1] = NULL;

	if(pRagDoll)
	{
		m_pNode[0] = pRagDoll->GetNode(hNode1);
		m_pNode[1] = pRagDoll->GetNode(hNode2);

		DetermineNodeWeights(fNode1Weight, m_pNode[0], m_pNode[1], m_fNode1Weight, m_fNode2Weight);
		m_fDistance = (fDistance < 0.0f) ? pRagDoll->GetDistance(hNode1, hNode2) : fDistance;
	}
}

CRagDollDistanceConstraint::CRagDollDistanceConstraint(const CRagDollDistanceConstraint& rhs)
{
	m_pNode[0] = rhs.m_pNode[0];
	m_pNode[1] = rhs.m_pNode[1];
	m_fDistance = rhs.m_fDistance;

	m_fNode1Weight = rhs.m_fNode1Weight;
	m_fNode2Weight = rhs.m_fNode2Weight;
}

bool CRagDollDistanceConstraint::IsValid() const
{
	//only valid if both nodes are valid and not the same
	return (m_pNode[0] && m_pNode[1]) && (m_pNode[0] != m_pNode[1]);
}

void CRagDollDistanceConstraint::Apply(uint32 nPosIndex)
{
	//find the distance between them
	LTVector vToOther = m_pNode[1]->m_vPosition[nPosIndex] - 
						m_pNode[0]->m_vPosition[nPosIndex];

	float fDist = vToOther.Mag();
	float fScale = (fDist - m_fDistance) / fDist;
	
	//now offset the vertices
	m_pNode[0]->m_vPosition[nPosIndex] += vToOther * fScale * m_fNode1Weight;
	m_pNode[1]->m_vPosition[nPosIndex] -= vToOther * fScale * m_fNode2Weight;
}

CRagDollConstraint* CRagDollDistanceConstraint::Clone() const
{
	return debug_new1(CRagDollDistanceConstraint, *this);
}

//------------------------------------------------------------------------------------------------
// CRagDollMinDistanceConstraint
//------------------------------------------------------------------------------------------------

CRagDollMinDistanceConstraint::CRagDollMinDistanceConstraint(CRagDoll* pRagDoll, HRAGDOLLNODE hNode1, HRAGDOLLNODE hNode2, float fDistance, float fNode1Weight)
{
	m_fDistance = fDistance;

	m_pNode[0] = NULL;
	m_pNode[1] = NULL;

	if(pRagDoll)
	{
		m_pNode[0] = pRagDoll->GetNode(hNode1);
		m_pNode[1] = pRagDoll->GetNode(hNode2);

		DetermineNodeWeights(fNode1Weight, m_pNode[0], m_pNode[1], m_fNode1Weight, m_fNode2Weight);
		m_fDistance = (fDistance < 0.0f) ? pRagDoll->GetDistance(hNode1, hNode2) : fDistance;
	}
}

CRagDollMinDistanceConstraint::CRagDollMinDistanceConstraint(const CRagDollMinDistanceConstraint& rhs)
{
	m_pNode[0]  = rhs.m_pNode[0];
	m_pNode[1]  = rhs.m_pNode[1];
	m_fDistance = rhs.m_fDistance;

	m_fNode1Weight = rhs.m_fNode1Weight;
	m_fNode2Weight = rhs.m_fNode2Weight;
}

bool CRagDollMinDistanceConstraint::IsValid() const
{
	//only valid if both nodes are valid and not the same
	return (m_pNode[0] && m_pNode[1]) && (m_pNode[0] != m_pNode[1]);
}

void CRagDollMinDistanceConstraint::Apply(uint32 nPosIndex)
{
	//find the distance between them
	LTVector vToOther = m_pNode[1]->m_vPosition[nPosIndex] - 
						m_pNode[0]->m_vPosition[nPosIndex];

	float fDist = vToOther.Mag();

	if(fDist < m_fDistance)
	{
		float fScale = (fDist - m_fDistance) / fDist;
		
		//now offset the vertices
		m_pNode[0]->m_vPosition[nPosIndex] += vToOther * fScale * m_fNode1Weight;
		m_pNode[1]->m_vPosition[nPosIndex] -= vToOther * fScale * m_fNode2Weight;
	}
}

CRagDollConstraint* CRagDollMinDistanceConstraint::Clone() const
{
	return debug_new1(CRagDollMinDistanceConstraint, *this);
}

//------------------------------------------------------------------------------------------------
// CRagDollMaxDistanceConstraint
//------------------------------------------------------------------------------------------------

CRagDollMaxDistanceConstraint::CRagDollMaxDistanceConstraint(CRagDoll* pRagDoll, HRAGDOLLNODE hNode1, HRAGDOLLNODE hNode2, float fDistance, float fNode1Weight)
{
	m_fDistance = fDistance;

	m_pNode[0] = NULL;
	m_pNode[1] = NULL;

	if(pRagDoll)
	{
		m_pNode[0] = pRagDoll->GetNode(hNode1);
		m_pNode[1] = pRagDoll->GetNode(hNode2);

		DetermineNodeWeights(fNode1Weight, m_pNode[0], m_pNode[1], m_fNode1Weight, m_fNode2Weight);
		m_fDistance = (fDistance < 0.0f) ? pRagDoll->GetDistance(hNode1, hNode2) : fDistance;
	}
}

CRagDollMaxDistanceConstraint::CRagDollMaxDistanceConstraint(const CRagDollMaxDistanceConstraint& rhs)
{
	m_pNode[0]  = rhs.m_pNode[0];
	m_pNode[1]  = rhs.m_pNode[1];
	m_fDistance = rhs.m_fDistance;

	m_fNode1Weight = rhs.m_fNode1Weight;
	m_fNode2Weight = rhs.m_fNode2Weight;
}

bool CRagDollMaxDistanceConstraint::IsValid() const
{
	//only valid if both nodes are valid and not the same
	return (m_pNode[0] && m_pNode[1]) && (m_pNode[0] != m_pNode[1]);
}

void CRagDollMaxDistanceConstraint::Apply(uint32 nPosIndex)
{
	//find the distance between them
	LTVector vToOther = m_pNode[1]->m_vPosition[nPosIndex] - 
						m_pNode[0]->m_vPosition[nPosIndex];

	float fDist = vToOther.Mag();

	if(fDist > m_fDistance)
	{
		float fScale = (fDist - m_fDistance) / fDist;
		
		//now offset the vertices
		m_pNode[0]->m_vPosition[nPosIndex] += vToOther * fScale * m_fNode1Weight;
		m_pNode[1]->m_vPosition[nPosIndex] -= vToOther * fScale * m_fNode2Weight;
	}
}

CRagDollConstraint* CRagDollMaxDistanceConstraint::Clone() const
{
	return debug_new1(CRagDollMaxDistanceConstraint, *this);
}


//------------------------------------------------------------------------------------------------
// CRagDollInPlaneConstraint
//------------------------------------------------------------------------------------------------
CRagDollInPlaneConstraint::CRagDollInPlaneConstraint(CRagDoll* pRagDoll, HRAGDOLLNODE hHinge, HRAGDOLLNODE hFirstPlaneCenter, HRAGDOLLNODE hFirstPlaneOther, HRAGDOLLNODE hEndFinalSeg, float fNormalScale, float fTolerance) :
	m_pHinge(NULL),
	m_pPlaneCenter(NULL),
	m_pPlaneOther(NULL),
	m_pConstrain(NULL),
	m_fNormalScale(fNormalScale),
	m_fTolerance(fTolerance)
{
	if(pRagDoll)
	{
		m_pHinge		= pRagDoll->GetNode(hHinge);
		m_pPlaneCenter	= pRagDoll->GetNode(hFirstPlaneCenter);
		m_pPlaneOther	= pRagDoll->GetNode(hFirstPlaneOther);
		m_pConstrain	= pRagDoll->GetNode(hEndFinalSeg);
	}
}

CRagDollInPlaneConstraint::CRagDollInPlaneConstraint(const CRagDollInPlaneConstraint& rhs) :
	m_pHinge(rhs.m_pHinge),
	m_pPlaneCenter(rhs.m_pPlaneCenter),
	m_pPlaneOther(rhs.m_pPlaneOther),
	m_pConstrain(rhs.m_pConstrain),
	m_fNormalScale(rhs.m_fNormalScale),
	m_fTolerance(rhs.m_fTolerance)
{
}

void CRagDollInPlaneConstraint::Apply(uint32 nPosIndex)
{
	//find the plane pivot point (the connection between both points forming the first plane)
	LTVector& vPivot = m_pPlaneCenter->m_vPosition[nPosIndex];

	//find the vector that goes from the pivot point to the hinge point
	LTVector vToHinge = m_pHinge->m_vPosition[nPosIndex] - vPivot;
	LTVector vToOther = m_pPlaneOther->m_vPosition[nPosIndex] - vPivot;

	//find the plane normal
	LTVector vPlaneNormal = vToOther.Cross(vToHinge);
	vPlaneNormal.Normalize();

	vPlaneNormal = vToHinge.Cross(vPlaneNormal) * m_fNormalScale;
	vPlaneNormal.Normalize();

	//move the point into the plane
	LTVector& vConstrain = m_pConstrain->m_vPosition[nPosIndex];

	float fDot = vPlaneNormal.Dot(vConstrain - vPivot);

	if(fDot < -m_fTolerance)
	{
		fDot += m_fTolerance;
		vConstrain -= vPlaneNormal * fDot;
	}
	else if(fDot > m_fTolerance)
	{
		fDot -= m_fTolerance;
		vConstrain -= vPlaneNormal * fDot;
	}

	//success
}

bool CRagDollInPlaneConstraint::IsValid() const
{
	return (m_pHinge && m_pPlaneCenter && m_pPlaneOther && m_pConstrain);
}

CRagDollConstraint* CRagDollInPlaneConstraint::Clone() const
{
	return debug_new1(CRagDollInPlaneConstraint, *this);
}

//------------------------------------------------------------------------------------------------
// CRagDollAbovePlaneConstraint
//------------------------------------------------------------------------------------------------
CRagDollAbovePlaneConstraint::CRagDollAbovePlaneConstraint(CRagDoll* pRagDoll, HRAGDOLLNODE hPtInPlane, HRAGDOLLNODE hPtOnNormal, HRAGDOLLNODE hConstrain, float fNormalScale, float fPlaneOffset) :
	m_pPtInPlane(NULL),
	m_pPtOnNormal(NULL),
	m_pConstrain(NULL),
	m_fNormalScale(fNormalScale),
	m_fPlaneOffset(fPlaneOffset)
{
	if(pRagDoll)
	{
		m_pPtInPlane	= pRagDoll->GetNode(hPtInPlane);
		m_pPtOnNormal	= pRagDoll->GetNode(hPtOnNormal);
		m_pConstrain	= pRagDoll->GetNode(hConstrain);
	}
}

CRagDollAbovePlaneConstraint::CRagDollAbovePlaneConstraint(const CRagDollAbovePlaneConstraint& rhs) :
	m_pPtInPlane(rhs.m_pPtInPlane),
	m_pPtOnNormal(rhs.m_pPtOnNormal),
	m_pConstrain(rhs.m_pConstrain),
	m_fNormalScale(rhs.m_fNormalScale),
	m_fPlaneOffset(rhs.m_fPlaneOffset)
{
}

void CRagDollAbovePlaneConstraint::Apply(uint32 nPosIndex)
{
	const LTVector& vPtInPlane = m_pPtInPlane->m_vPosition[nPosIndex];

	//we first off need to caclulate the normal of the plane
	LTVector vNormal = (m_pPtOnNormal->m_vPosition[nPosIndex] - vPtInPlane) * m_fNormalScale;
	vNormal.Normalize();

	//ok, now we see if the point is above the plane
	LTVector& vConstrain = m_pConstrain->m_vPosition[nPosIndex];
	float fDot = vNormal.Dot(vConstrain - vPtInPlane) - m_fPlaneOffset;

	if(fDot < 0.0f)
	{
		vConstrain -= fDot * vNormal;
	}
}

bool CRagDollAbovePlaneConstraint::IsValid() const
{
	return (m_pPtInPlane && m_pPtOnNormal && m_pConstrain);
}

CRagDollConstraint* CRagDollAbovePlaneConstraint::Clone() const
{
	return debug_new1(CRagDollAbovePlaneConstraint, *this);
}

//------------------------------------------------------------------------------------------------
// CRagDollAbovePlane3Constraint
//------------------------------------------------------------------------------------------------
CRagDollAbovePlane3Constraint::CRagDollAbovePlane3Constraint(CRagDoll* pRagDoll, HRAGDOLLNODE hPt1, HRAGDOLLNODE hPt2, HRAGDOLLNODE hPt3, HRAGDOLLNODE hConstrain, float fNormalScale, float fOffset) :
	m_pPt1(NULL),
	m_pPt2(NULL),
	m_pPt3(NULL),
	m_pConstrain(NULL),
	m_fNormalScale(fNormalScale),
	m_fOffset(fOffset)
{
	if(pRagDoll)
	{
		m_pPt1			= pRagDoll->GetNode(hPt1);
		m_pPt2			= pRagDoll->GetNode(hPt2);
		m_pPt3			= pRagDoll->GetNode(hPt3);
		m_pConstrain	= pRagDoll->GetNode(hConstrain);
	}
}

CRagDollAbovePlane3Constraint::CRagDollAbovePlane3Constraint(const CRagDollAbovePlane3Constraint& rhs) :
	m_pPt1(rhs.m_pPt1),
	m_pPt2(rhs.m_pPt2),
	m_pPt3(rhs.m_pPt3),
	m_pConstrain(rhs.m_pConstrain),
	m_fNormalScale(rhs.m_fNormalScale),
	m_fOffset(rhs.m_fOffset)
{
}

void CRagDollAbovePlane3Constraint::Apply(uint32 nPosIndex)
{
	const LTVector& vPtInPlane = m_pPt1->m_vPosition[nPosIndex];

	//we first off need to caclulate the normal of the plane
	LTVector vNormal = (m_pPt2->m_vPosition[nPosIndex] - vPtInPlane).Cross(m_pPt3->m_vPosition[nPosIndex] - vPtInPlane) * m_fNormalScale;
	vNormal.Normalize();

	//ok, now we see if the point is above the plane
	LTVector& vConstrain = m_pConstrain->m_vPosition[nPosIndex];
	float fDot = vNormal.Dot(vConstrain - vPtInPlane) - m_fOffset;

	if(fDot < 0.0f)
	{
		vConstrain -= fDot * vNormal;
	}
}

bool CRagDollAbovePlane3Constraint::IsValid() const
{
	return (m_pPt1 && m_pPt2 && m_pPt3 && m_pConstrain);
}

CRagDollConstraint* CRagDollAbovePlane3Constraint::Clone() const
{
	return debug_new1(CRagDollAbovePlane3Constraint, *this);
}

//------------------------------------------------------------------------------------------------
// CRagDollAbovePlaneOnEdgeConstraint
//------------------------------------------------------------------------------------------------
CRagDollAbovePlaneOnEdgeConstraint::CRagDollAbovePlaneOnEdgeConstraint(CRagDoll* pRagDoll, HRAGDOLLNODE hPt1, HRAGDOLLNODE hPt2, HRAGDOLLNODE hPt3, HRAGDOLLNODE hConstrain, float fNormalScale) :
	m_pPt1(NULL),
	m_pPt2(NULL),
	m_pPt3(NULL),
	m_pConstrain(NULL),
	m_fNormalScale(fNormalScale)
{
	if(pRagDoll)
	{
		m_pPt1			= pRagDoll->GetNode(hPt1);
		m_pPt2			= pRagDoll->GetNode(hPt2);
		m_pPt3			= pRagDoll->GetNode(hPt3);
		m_pConstrain	= pRagDoll->GetNode(hConstrain);
	}
}

CRagDollAbovePlaneOnEdgeConstraint::CRagDollAbovePlaneOnEdgeConstraint(const CRagDollAbovePlaneOnEdgeConstraint& rhs) :
	m_pPt1(rhs.m_pPt1),
	m_pPt2(rhs.m_pPt2),
	m_pPt3(rhs.m_pPt3),
	m_pConstrain(rhs.m_pConstrain),
	m_fNormalScale(rhs.m_fNormalScale)
{
}

void CRagDollAbovePlaneOnEdgeConstraint::Apply(uint32 nPosIndex)
{
	const LTVector& vPtInPlane = m_pPt1->m_vPosition[nPosIndex];

	LTVector vEdge = m_pPt2->m_vPosition[nPosIndex] - vPtInPlane;

	//we first off need to caclulate the normal of the plane
	LTVector vNormal = vEdge.Cross(m_pPt3->m_vPosition[nPosIndex] - vPtInPlane);
	vNormal.Normalize();

	//now we need to take that plane, and find the perpindicular plane that passes through the first edge
	LTVector vEdgeNormal = vNormal.Cross(vEdge) * m_fNormalScale;
	vEdgeNormal.Normalize();

	//ok, now we see if the point is above the plane
	LTVector& vConstrain = m_pConstrain->m_vPosition[nPosIndex];
	float fDot = vEdgeNormal.Dot(vConstrain - vPtInPlane);

	if(fDot < 0.0f)
	{
		vConstrain -= fDot * vEdgeNormal;
	}
}

bool CRagDollAbovePlaneOnEdgeConstraint::IsValid() const
{
	return (m_pPt1 && m_pPt2 && m_pPt3 && m_pConstrain);
}

CRagDollConstraint* CRagDollAbovePlaneOnEdgeConstraint::Clone() const
{
	return debug_new1(CRagDollAbovePlaneOnEdgeConstraint, *this);
}
