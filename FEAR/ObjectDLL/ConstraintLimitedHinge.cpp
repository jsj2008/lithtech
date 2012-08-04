#include "Stdafx.h"
#include "ConstraintLimitedHinge.h"

LINKFROM_MODULE( ConstraintLimitedHinge );

BEGIN_CLASS(ConstraintLimitedHinge)
	ADD_REALPROP(MinAngle, -45.0, "The minimum angle that the hinge can rotate around the forward axis relative to the up axis. For example -30 means it can rotate counterclockwise up to 30 degrees")
	ADD_REALPROP(MaxAngle, 45.0, "The maximum angle that the hinge can rotate around the forward axis relative to the up axis. For example 30 means it can rotate clockwise up to 30 degrees")
	ADD_REALPROP(Friction, 0.0, "The amount of friction to apply as the object goes around the hinge. This number should be obtained through experimentation, but larger numbers mean more friction")
END_CLASS(ConstraintLimitedHinge, ConstraintBase, "Provides a constraint that will join two rigid bodies together at a point and allow for rotation around the forward axis")

CMDMGR_BEGIN_REGISTER_CLASS( ConstraintLimitedHinge )
CMDMGR_END_REGISTER_CLASS( ConstraintLimitedHinge, ConstraintBase )

//------------------------------------------------------------------------------------
// ConstraintLimitedHinge
//------------------------------------------------------------------------------------

ConstraintLimitedHinge::ConstraintLimitedHinge()
{
	m_vPivotPt1.Init();
	m_vPivotPt2.Init();
	m_vPivotDir1.Init();
	m_vPivotDir2.Init();
	m_vPivotPerp1.Init();
	m_vPivotPerp2.Init();

	m_fAngleMin = -MATH_HALFPI;
	m_fAngleMax = MATH_HALFPI;
	m_fFriction = 0.0f;
}

//virtual function that derived classes must override to handle loading in of
//property data. Derived classes should call this for the parent first.
void ConstraintLimitedHinge::ReadConstraintProperties(const GenericPropList *pProps)
{
	ConstraintBase::ReadConstraintProperties(pProps);

	m_fAngleMin = MATH_DEGREES_TO_RADIANS(pProps->GetReal("MinAngle", -45.0f));
	m_fAngleMax = MATH_DEGREES_TO_RADIANS(pProps->GetReal("MaxAngle", -45.0f));
	m_fFriction = pProps->GetReal("Friction", 0.0f);
}

bool ConstraintLimitedHinge::SetupBodySpaceData(const LTRigidTransform& tInvBody1, const LTRigidTransform& tInvBody2)
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

	m_vPivotPerp1 = tInvBody1.m_rRot.RotateVector(rConstraintOr.Up());
	m_vPivotPerp2 = tInvBody2.m_rRot.RotateVector(rConstraintOr.Up());

	return true;
}

//called to handle actual construction of this constraint
HPHYSICSCONSTRAINT ConstraintLimitedHinge::CreateConstraint(HPHYSICSRIGIDBODY hBody1, HPHYSICSRIGIDBODY hBody2)
{
	return g_pLTServer->PhysicsSim()->CreateLimitedHinge(hBody1, hBody2,	m_vPivotPt1, m_vPivotPt2, 
																			m_vPivotDir1, m_vPivotDir2, 
																			m_vPivotPerp1, m_vPivotPerp2,
																			m_fAngleMin, m_fAngleMax, m_fFriction);
}

//handles saving and loading all data to a message stream. Derived classes should
//call this first in order to work properly
void ConstraintLimitedHinge::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	ConstraintBase::Save(pMsg, dwSaveFlags);

	pMsg->WriteLTVector(m_vPivotPt1);
	pMsg->WriteLTVector(m_vPivotPt2);
	pMsg->WriteLTVector(m_vPivotDir1);
	pMsg->WriteLTVector(m_vPivotDir2);
	pMsg->WriteLTVector(m_vPivotPerp1);
	pMsg->WriteLTVector(m_vPivotPerp2);
	pMsg->Writefloat(m_fAngleMin);
	pMsg->Writefloat(m_fAngleMax);
	pMsg->Writefloat(m_fFriction);
}

void ConstraintLimitedHinge::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	ConstraintBase::Load(pMsg, dwLoadFlags);

	m_vPivotPt1 = pMsg->ReadLTVector();
	m_vPivotPt2 = pMsg->ReadLTVector();
	m_vPivotDir1 = pMsg->ReadLTVector();
	m_vPivotDir2 = pMsg->ReadLTVector();
	m_vPivotPerp1 = pMsg->ReadLTVector();
	m_vPivotPerp2 = pMsg->ReadLTVector();
	m_fAngleMin = pMsg->Readfloat();
	m_fAngleMax = pMsg->Readfloat();
	m_fFriction = pMsg->Readfloat();
}

// Write data specific to a limited hinge constraint into the create struct...
void ConstraintLimitedHinge::SetupConstraintCreateStruct( PHYSICSCONSTRAINTCREATESTRUCT &rCS )
{
	rCS.m_eConstraintType = kConstraintType_LimitedHinge;
	rCS.m_vPivotPt1 = m_vPivotPt1;
	rCS.m_vPivotPt2 = m_vPivotPt2;
	rCS.m_vPivotDir1 = m_vPivotDir1;
	rCS.m_vPivotDir2 = m_vPivotDir2;
	rCS.m_vPivotPerp1 = m_vPivotPerp1;
	rCS.m_vPivotPerp2 = m_vPivotPerp2;
	rCS.m_fAngleMin = m_fAngleMin;
	rCS.m_fAngleMax = m_fAngleMax;
	rCS.m_fFriction = m_fFriction;
}

// EOF
