#include "Stdafx.h"
#include "ConstraintPoint.h"

LINKFROM_MODULE( ConstraintPoint );

BEGIN_CLASS(ConstraintPoint)
END_CLASS(ConstraintPoint, ConstraintBase, "Provides a constraint that will join two rigid bodies together at a point")

CMDMGR_BEGIN_REGISTER_CLASS( ConstraintPoint )
CMDMGR_END_REGISTER_CLASS( ConstraintPoint, ConstraintBase )


//------------------------------------------------------------------------------------
// ConstraintPoint
//------------------------------------------------------------------------------------

ConstraintPoint::ConstraintPoint()
{
	m_vPivotPt1.Init();
	m_vPivotPt2.Init();
}

bool ConstraintPoint::SetupBodySpaceData(const LTRigidTransform& tInvBody1, const LTRigidTransform& tInvBody2)
{
	//get the position of our object which will serve as the constraint point
	LTVector vConstraintPos;
	if(g_pLTServer->GetObjectPos(m_hObject, &vConstraintPos) != LT_OK)
		return false;

	m_vPivotPt1 = tInvBody1 * vConstraintPos;
	m_vPivotPt2 = tInvBody2 * vConstraintPos;

	return true;
}

//called to handle actual construction of this constraint
HPHYSICSCONSTRAINT ConstraintPoint::CreateConstraint(HPHYSICSRIGIDBODY hBody1, HPHYSICSRIGIDBODY hBody2)
{
	//now create the actual constraint
	return g_pLTServer->PhysicsSim()->CreateBallAndSocket(hBody1, hBody2, m_vPivotPt1, m_vPivotPt2);
}

//handles saving and loading all data to a message stream. Derived classes should
//call this first in order to work properly
void ConstraintPoint::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	ConstraintBase::Save(pMsg, dwSaveFlags);

	pMsg->WriteLTVector(m_vPivotPt1);
	pMsg->WriteLTVector(m_vPivotPt2);
}

void ConstraintPoint::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	ConstraintBase::Load(pMsg, dwLoadFlags);

	m_vPivotPt1 = pMsg->ReadLTVector();
	m_vPivotPt2 = pMsg->ReadLTVector();
}

// Write data specific to a point constraint into the create struct
void ConstraintPoint::SetupConstraintCreateStruct( PHYSICSCONSTRAINTCREATESTRUCT &rCS )
{
	rCS.m_eConstraintType = kConstraintType_Point;
	rCS.m_vPivotPt1 = m_vPivotPt1;
	rCS.m_vPivotPt2 = m_vPivotPt2;
}

// EOF
