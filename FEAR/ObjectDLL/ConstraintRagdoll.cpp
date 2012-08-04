#include "Stdafx.h"
#include "ConstraintRagdoll.h"

LINKFROM_MODULE( ConstraintRagdoll );

BEGIN_CLASS(ConstraintRagdoll)
	ADD_REALPROP(ConeAngle, 45.0, "The angle of the cone of movement around the forward axis of the constraint")
	ADD_REALPROP(PosConeAngle, 45.0, "The radius of the cone of restriction around the positive X axis of the constraint")
	ADD_REALPROP(NegConeAngle, 45.0, "The radius of the cone of restriction around the negative X axis of the constraint")
	ADD_REALPROP(TwistMin, -45.0, "The minimum angle that the body can twist around the forward axis relative to the up axis. For example -30 means it can rotate counterclockwise up to 30 degrees")
	ADD_REALPROP(TwistMax, 45.0, "The minimum angle that the body can twist around the forward axis relative to the up axis. For example -0 means it can rotate clockwise up to 30 degrees")
	ADD_REALPROP(Friction, 0.0, "The amount of friction to apply as the object moves. This number should be obtained through experimentation, but larger numbers mean more friction")
END_CLASS(ConstraintRagdoll, ConstraintBase, "Provides a constraint that will join two rigid bodies together at a point and allow for a cone of movement with two cones of restriction, and allow for twist around the forward axis of the object")

CMDMGR_BEGIN_REGISTER_CLASS( ConstraintRagdoll )
CMDMGR_END_REGISTER_CLASS( ConstraintRagdoll, ConstraintBase )

//------------------------------------------------------------------------------------
// ConstraintRagdoll
//------------------------------------------------------------------------------------

ConstraintRagdoll::ConstraintRagdoll()
{
	m_vPivotPt1.Init();
	m_vPivotPt2.Init();
	m_vTwist1.Init();
	m_vTwist2.Init();
	m_vPlane1.Init();
	m_vPlane2.Init();

	m_fConeAngle	= MATH_HALFPI;
	m_fPosCone		= MATH_HALFPI;
	m_fNegCone		= MATH_HALFPI;
	m_fTwistMin		= -MATH_HALFPI;
	m_fTwistMax		= MATH_HALFPI;
	m_fFriction		= 0.0f;
}

//virtual function that derived classes must override to handle loading in of
//property data. Derived classes should call this for the parent first.
void ConstraintRagdoll::ReadConstraintProperties(const GenericPropList *pProps)
{
	ConstraintBase::ReadConstraintProperties(pProps);

	m_fConeAngle = MATH_DEGREES_TO_RADIANS(pProps->GetReal("ConeAngle", 45.0f));
	m_fPosCone	= MATH_DEGREES_TO_RADIANS(pProps->GetReal("PosConeAngle", 45.0f));
	m_fNegCone	= MATH_DEGREES_TO_RADIANS(pProps->GetReal("NegConeAngle", 45.0f));
	m_fTwistMin = MATH_DEGREES_TO_RADIANS(pProps->GetReal("TwistMin", -45.0f));
	m_fTwistMax = MATH_DEGREES_TO_RADIANS(pProps->GetReal("TwistMax", 45.0f));
	m_fFriction = pProps->GetReal("Friction", 0.0f);
}

bool ConstraintRagdoll::SetupBodySpaceData(const LTRigidTransform& tInvBody1, const LTRigidTransform& tInvBody2)
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

	m_vTwist1 = tInvBody1.m_rRot.RotateVector(rConstraintOr.Forward());
	m_vTwist2 = tInvBody2.m_rRot.RotateVector(rConstraintOr.Forward());

	m_vPlane1 = tInvBody1.m_rRot.RotateVector(rConstraintOr.Right());
	m_vPlane2 = tInvBody2.m_rRot.RotateVector(rConstraintOr.Right());

	return true;
}

//called to handle actual construction of this constraint
HPHYSICSCONSTRAINT ConstraintRagdoll::CreateConstraint(HPHYSICSRIGIDBODY hBody1, HPHYSICSRIGIDBODY hBody2)
{
	return g_pLTServer->PhysicsSim()->CreateRagdoll(hBody1, hBody2,	m_vPivotPt1, m_vPivotPt2, 
																	m_vTwist1, m_vTwist2, 
																	m_vPlane1, m_vPlane2,
																	m_fConeAngle, m_fPosCone, m_fNegCone,
																	m_fTwistMin, m_fTwistMax, m_fFriction);
}

//handles saving and loading all data to a message stream. Derived classes should
//call this first in order to work properly
void ConstraintRagdoll::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	ConstraintBase::Save(pMsg, dwSaveFlags);

	pMsg->WriteLTVector(m_vPivotPt1);
	pMsg->WriteLTVector(m_vPivotPt2);
	pMsg->WriteLTVector(m_vTwist1);
	pMsg->WriteLTVector(m_vTwist2);
	pMsg->WriteLTVector(m_vPlane1);
	pMsg->WriteLTVector(m_vPlane2);
	pMsg->Writefloat(m_fConeAngle);
	pMsg->Writefloat(m_fPosCone);
	pMsg->Writefloat(m_fNegCone);
	pMsg->Writefloat(m_fTwistMin);
	pMsg->Writefloat(m_fTwistMax);
	pMsg->Writefloat(m_fFriction);
}

void ConstraintRagdoll::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	ConstraintBase::Load(pMsg, dwLoadFlags);

	m_vPivotPt1		= pMsg->ReadLTVector();
	m_vPivotPt2		= pMsg->ReadLTVector();
	m_vTwist1		= pMsg->ReadLTVector();
	m_vTwist2		= pMsg->ReadLTVector();
	m_vPlane1		= pMsg->ReadLTVector();
	m_vPlane2		= pMsg->ReadLTVector();
	m_fConeAngle	= pMsg->Readfloat();
	m_fPosCone		= pMsg->Readfloat();
	m_fNegCone		= pMsg->Readfloat();
	m_fTwistMin		= pMsg->Readfloat();
	m_fTwistMax		= pMsg->Readfloat();
	m_fFriction		= pMsg->Readfloat();
}

// Write data specific to a prismatic constraint into the create struct...
void ConstraintRagdoll::SetupConstraintCreateStruct( PHYSICSCONSTRAINTCREATESTRUCT &rCS )
{
	rCS.m_eConstraintType = kConstraintType_Ragdoll;
	rCS.m_vPivotPt1		= m_vPivotPt1;
	rCS.m_vPivotPt2		= m_vPivotPt2;
	rCS.m_vTwist1		= m_vTwist1;
	rCS.m_vTwist2		= m_vTwist2;
	rCS.m_vPlane1		= m_vPlane1;
	rCS.m_vPlane2		= m_vPlane2;
	rCS.m_fConeAngle	= m_fConeAngle;
	rCS.m_fPosCone		= m_fPosCone;
	rCS.m_fNegCone		= m_fNegCone;
	rCS.m_fTwistMin		= m_fTwistMin;
	rCS.m_fTwistMax		= m_fTwistMax;
	rCS.m_fFriction		= m_fFriction;
}

// EOF
