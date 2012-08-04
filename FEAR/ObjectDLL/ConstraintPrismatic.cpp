#include "Stdafx.h"
#include "ConstraintPrismatic.h"

LINKFROM_MODULE( ConstraintPrismatic );

BEGIN_CLASS(ConstraintPrismatic)
	ADD_REALPROP(MovementMin, 0.0f, "The lower range of movement along the up axis of this constraint. This should be less than zero and less than the movement maximum.")
	ADD_REALPROP(MovementMax, 0.0f, "The upper range of movement along the up axis of this constraint. This should be more than zero and more than the movement minimum.")
	ADD_REALPROP(Friction, 0.0f, "The amount of friction to apply as the rigid body moves along the axis to slow down the movement. Higher values slow the movement down more.")
END_CLASS(ConstraintPrismatic, ConstraintBase, "Provides a constraint that fixes two rigid bodies together, but allows the first rigid body to move along the up axis of the constraint within the range specified.")

CMDMGR_BEGIN_REGISTER_CLASS( ConstraintPrismatic )
CMDMGR_END_REGISTER_CLASS( ConstraintPrismatic, ConstraintBase )

//------------------------------------------------------------------------------------
// ConstraintWheel
//------------------------------------------------------------------------------------

ConstraintPrismatic::ConstraintPrismatic()
{
	m_vPivotPt1.Init();
	m_vPivotPt2.Init();
	m_vAxis1.Init();
	m_vAxis2.Init();
	m_vAxisPerp1.Init();
	m_vAxisPerp2.Init();

	m_fMovementMin	= 0.0f;
	m_fMovementMax	= 0.0f;
	m_fFriction		= 0.0f;
}

//virtual function that derived classes must override to handle loading in of
//property data. Derived classes should call this for the parent first.
void ConstraintPrismatic::ReadConstraintProperties(const GenericPropList *pProps)
{
	ConstraintBase::ReadConstraintProperties(pProps);

	m_fMovementMin	= pProps->GetReal("MovementMin", 0.0f);
	m_fMovementMax	= pProps->GetReal("MovementMax", 0.0f);
	m_fFriction		= pProps->GetReal("Friction", 0.0f);
}

bool ConstraintPrismatic::SetupBodySpaceData(const LTRigidTransform& tInvBody1, const LTRigidTransform& tInvBody2)
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

	m_vAxis1 = tInvBody1.m_rRot.RotateVector(rConstraintOr.Up());
	m_vAxis2 = tInvBody2.m_rRot.RotateVector(rConstraintOr.Up());

	m_vAxisPerp1 = tInvBody1.m_rRot.RotateVector(rConstraintOr.Right());
	m_vAxisPerp2 = tInvBody2.m_rRot.RotateVector(rConstraintOr.Right());

	return true;
}

//called to handle actual construction of this constraint
HPHYSICSCONSTRAINT ConstraintPrismatic::CreateConstraint(HPHYSICSRIGIDBODY hBody1, HPHYSICSRIGIDBODY hBody2)
{
	return g_pLTServer->PhysicsSim()->CreatePrismatic(hBody1, hBody2,	m_vPivotPt1, m_vPivotPt2, 
																	m_vAxis1, m_vAxis2, 
																	m_vAxisPerp1, m_vAxisPerp2,
																	m_fMovementMin, m_fMovementMax,
																	m_fFriction);
}

//handles saving and loading all data to a message stream. Derived classes should
//call this first in order to work properly
void ConstraintPrismatic::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	ConstraintBase::Save(pMsg, dwSaveFlags);

	pMsg->WriteLTVector(m_vPivotPt1);
	pMsg->WriteLTVector(m_vPivotPt2);
	pMsg->WriteLTVector(m_vAxis1);
	pMsg->WriteLTVector(m_vAxis2);
	pMsg->WriteLTVector(m_vAxisPerp1);
	pMsg->WriteLTVector(m_vAxisPerp2);
	pMsg->Writefloat(m_fMovementMin);
	pMsg->Writefloat(m_fMovementMax);
	pMsg->Writefloat(m_fFriction);
}

void ConstraintPrismatic::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	ConstraintBase::Load(pMsg, dwLoadFlags);

	m_vPivotPt1		= pMsg->ReadLTVector();
	m_vPivotPt2		= pMsg->ReadLTVector();
	m_vAxis1		= pMsg->ReadLTVector();
	m_vAxis2		= pMsg->ReadLTVector();
	m_vAxisPerp1	= pMsg->ReadLTVector();
	m_vAxisPerp2	= pMsg->ReadLTVector();
	m_fMovementMin	= pMsg->Readfloat();
	m_fMovementMax	= pMsg->Readfloat();
	m_fFriction		= pMsg->Readfloat();
}

// Write data specific to a prismatic constraint into the create struct...
void ConstraintPrismatic::SetupConstraintCreateStruct( PHYSICSCONSTRAINTCREATESTRUCT &rCS )
{
	rCS.m_eConstraintType = kConstraintType_Prismatic;
	rCS.m_vPivotPt1		= m_vPivotPt1;
	rCS.m_vPivotPt2		= m_vPivotPt2;
	rCS.m_vAxis1		= m_vAxis1;
	rCS.m_vAxis2		= m_vAxis2;
	rCS.m_vAxisPerp1	= m_vAxisPerp1;
	rCS.m_vAxisPerp2	= m_vAxisPerp2;
	rCS.m_fMovementMin	= m_fMovementMin;
	rCS.m_fMovementMax	= m_fMovementMax;
	rCS.m_fFriction		= m_fFriction;
}

// EOF
