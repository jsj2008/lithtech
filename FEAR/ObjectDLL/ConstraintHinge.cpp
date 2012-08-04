#include "Stdafx.h"
#include "ConstraintHinge.h"

LINKFROM_MODULE( ConstraintHinge );

BEGIN_CLASS(ConstraintHinge)
END_CLASS(ConstraintHinge, ConstraintBase, "Provides a constraint that will join two rigid bodies together at a point and allow for rotation around the forward axis")

CMDMGR_BEGIN_REGISTER_CLASS( ConstraintHinge )
CMDMGR_END_REGISTER_CLASS( ConstraintHinge, ConstraintBase )

//------------------------------------------------------------------------------------
// ConstraintHinge
//------------------------------------------------------------------------------------

ConstraintHinge::ConstraintHinge()
{
	m_vPivotPt1.Init();
	m_vPivotPt2.Init();
	m_vPivotDir1.Init();
	m_vPivotDir2.Init();
}

bool ConstraintHinge::SetupBodySpaceData(const LTRigidTransform& tInvBody1, const LTRigidTransform& tInvBody2)
{
	//get the position of our object which will serve as the constraint point
	LTVector vConstraintPos;
	if(g_pLTServer->GetObjectPos(m_hObject, &vConstraintPos) != LT_OK)
		return false;

	LTRotation rConstraintOr;
	if(g_pLTServer->GetObjectRotation(m_hObject, &rConstraintOr) != LT_OK)
		return false;

	m_vPivotPt1 = tInvBody1 * vConstraintPos;
	m_vPivotPt2 = tInvBody2 * vConstraintPos;

	m_vPivotDir1 = tInvBody1.m_rRot.RotateVector(rConstraintOr.Forward());
	m_vPivotDir2 = tInvBody2.m_rRot.RotateVector(rConstraintOr.Forward());

	return true;
}

//called to handle actual construction of this constraint
HPHYSICSCONSTRAINT ConstraintHinge::CreateConstraint(HPHYSICSRIGIDBODY hBody1, HPHYSICSRIGIDBODY hBody2)
{
	return g_pLTServer->PhysicsSim()->CreateHinge(hBody1, hBody2, m_vPivotPt1, m_vPivotPt2, m_vPivotDir1, m_vPivotDir2);
}

//handles saving and loading all data to a message stream. Derived classes should
//call this first in order to work properly
void ConstraintHinge::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	ConstraintBase::Save(pMsg, dwSaveFlags);

	pMsg->WriteLTVector(m_vPivotPt1);
	pMsg->WriteLTVector(m_vPivotPt2);
	pMsg->WriteLTVector(m_vPivotDir1);
	pMsg->WriteLTVector(m_vPivotDir2);
}

void ConstraintHinge::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	ConstraintBase::Load(pMsg, dwLoadFlags);

	m_vPivotPt1 = pMsg->ReadLTVector();
	m_vPivotPt2 = pMsg->ReadLTVector();
	m_vPivotDir1 = pMsg->ReadLTVector();
	m_vPivotDir2 = pMsg->ReadLTVector();
}

// Write data specific to a hinge constraint into the create struct...
void ConstraintHinge::SetupConstraintCreateStruct( PHYSICSCONSTRAINTCREATESTRUCT &rCS )
{
	rCS.m_eConstraintType = kConstraintType_Hinge;
	rCS.m_vPivotPt1 = m_vPivotPt1;
	rCS.m_vPivotPt2 = m_vPivotPt2;
	rCS.m_vPivotDir1 = m_vPivotDir1;
	rCS.m_vPivotDir2 = m_vPivotDir2;
}
