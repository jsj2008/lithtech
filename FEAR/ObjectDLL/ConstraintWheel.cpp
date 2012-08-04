#include "Stdafx.h"
#include "ConstraintWheel.h"

LINKFROM_MODULE( ConstraintWheel );

BEGIN_CLASS(ConstraintWheel)
	ADD_REALPROP(SuspensionMin, 0.0f, "The lower range of movement along the up axis of this constraint. This should be less than zero and less than the suspension maximum.")
	ADD_REALPROP(SuspensionMax, 0.0f, "The upper range of movement along the up axis of this constraint. This should be more than zero and more than the suspension minimum.")
	ADD_REALPROP(SuspensionStrength, 0.0f, "The strength of the suspension. This should range from zero to one, where zero has no springiness, and one is a very tight spring")
	ADD_REALPROP(SuspensionDamping, 0.0f, "The amount of energy lost by the suspension as the wheel moves along it. This should range from zero to one with zero meaning no energy loss, and one meangin lots of energy loss.")
END_CLASS(ConstraintWheel, ConstraintBase, "Provides a constraint that fixes two rigid bodies together, but allows the first rigid body to rotate freely around the forward axis of the object. In addition it also allows for suspension along the up axis of the object, allowing the first object to move along the suspension axis within the range specified.")

CMDMGR_BEGIN_REGISTER_CLASS( ConstraintWheel )
CMDMGR_END_REGISTER_CLASS( ConstraintWheel, ConstraintBase )

//------------------------------------------------------------------------------------
// ConstraintWheel
//------------------------------------------------------------------------------------

ConstraintWheel::ConstraintWheel()
{
	m_vPivotPt1.Init();
	m_vPivotPt2.Init();
	m_vRotation1.Init();
	m_vRotation2.Init();
	m_vSuspension2.Init();

	m_fSuspensionMin		= 0.0f;
	m_fSuspensionMax		= 0.0f;
	m_fSuspensionStrength	= 0.0f;
	m_fSuspensionDamping	= 0.0f;
}

//virtual function that derived classes must override to handle loading in of
//property data. Derived classes should call this for the parent first.
void ConstraintWheel::ReadConstraintProperties(const GenericPropList *pProps)
{
	ConstraintBase::ReadConstraintProperties(pProps);

	m_fSuspensionMin		= pProps->GetReal("SuspensionMin", 0.0f);
	m_fSuspensionMax		= pProps->GetReal("SuspensionMax", 0.0f);
	m_fSuspensionStrength	= pProps->GetReal("SuspensionStrength", 0.0f);
	m_fSuspensionDamping	= pProps->GetReal("SuspensionDamping", 0.0f);
}

bool ConstraintWheel::SetupBodySpaceData(const LTRigidTransform& tInvBody1, const LTRigidTransform& tInvBody2)
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

	m_vRotation1 = tInvBody1.m_rRot.RotateVector(rConstraintOr.Forward());
	m_vRotation2 = tInvBody2.m_rRot.RotateVector(rConstraintOr.Forward());

	m_vSuspension2 = tInvBody2.m_rRot.RotateVector(rConstraintOr.Up());

	return true;
}

//called to handle actual construction of this constraint
HPHYSICSCONSTRAINT ConstraintWheel::CreateConstraint(HPHYSICSRIGIDBODY hBody1, HPHYSICSRIGIDBODY hBody2)
{
	return g_pLTServer->PhysicsSim()->CreateWheel(hBody1, hBody2,	m_vPivotPt1, m_vPivotPt2, 
																	m_vRotation1, m_vRotation2, 
																	m_vSuspension2,
																	m_fSuspensionMin, m_fSuspensionMax,
																	m_fSuspensionStrength, m_fSuspensionDamping);
}

//handles saving and loading all data to a message stream. Derived classes should
//call this first in order to work properly
void ConstraintWheel::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	ConstraintBase::Save(pMsg, dwSaveFlags);

	pMsg->WriteLTVector(m_vPivotPt1);
	pMsg->WriteLTVector(m_vPivotPt2);
	pMsg->WriteLTVector(m_vRotation1);
	pMsg->WriteLTVector(m_vRotation2);
	pMsg->WriteLTVector(m_vSuspension2);
	pMsg->Writefloat(m_fSuspensionMin);
	pMsg->Writefloat(m_fSuspensionMax);
	pMsg->Writefloat(m_fSuspensionStrength);
	pMsg->Writefloat(m_fSuspensionDamping);
}

void ConstraintWheel::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	ConstraintBase::Load(pMsg, dwLoadFlags);

	m_vPivotPt1				= pMsg->ReadLTVector();
	m_vPivotPt2				= pMsg->ReadLTVector();
	m_vRotation1			= pMsg->ReadLTVector();
	m_vRotation2			= pMsg->ReadLTVector();
	m_vSuspension2			= pMsg->ReadLTVector();
	m_fSuspensionMin		= pMsg->Readfloat();
	m_fSuspensionMax		= pMsg->Readfloat();
	m_fSuspensionStrength	= pMsg->Readfloat();
	m_fSuspensionDamping	= pMsg->Readfloat();
}

// Write data specific to a wheel constraint into the create struct...
void ConstraintWheel::SetupConstraintCreateStruct( PHYSICSCONSTRAINTCREATESTRUCT &rCS )
{
	rCS.m_eConstraintType		= kConstraintType_Wheel;
	rCS.m_vPivotPt1				= m_vPivotPt1;
	rCS.m_vPivotPt2				= m_vPivotPt2;
	rCS.m_vRotation1			= m_vRotation1;
	rCS.m_vRotation2			= m_vRotation2;
	rCS.m_vSuspension2			= m_vSuspension2;
	rCS.m_fSuspensionMin		= m_fSuspensionMin;
	rCS.m_fSuspensionMax		= m_fSuspensionMax;
	rCS.m_fSuspensionStrength	= m_fSuspensionStrength;
	rCS.m_fSuspensionDamping	= m_fSuspensionDamping;
}

// EOF
