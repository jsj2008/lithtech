#include "Stdafx.h"
#include "ConstraintStiffSpring.h"

LINKFROM_MODULE( ConstraintStiffSpring );

BEGIN_CLASS(ConstraintStiffSpring)
	ADD_STRINGPROP_FLAG(SpringEnd, "", PF_OBJECTLINK, "Indicates the name of the object that will serve as the connection point for the second rigid body.")
END_CLASS(ConstraintStiffSpring, ConstraintBase, "Provides a constraint that will join two rigid bodies together and ensure that the provided points stay a fixed distance away from each other")

CMDMGR_BEGIN_REGISTER_CLASS( ConstraintStiffSpring )
CMDMGR_END_REGISTER_CLASS( ConstraintStiffSpring, ConstraintBase )

//------------------------------------------------------------------------------------
// ConstraintPoint
//------------------------------------------------------------------------------------

ConstraintStiffSpring::ConstraintStiffSpring()
{
	m_vPivotPt1.Init();
	m_vPivotPt2.Init();
	m_fDistance = 0.0f;
}

//virtual function that derived classes must override to handle loading in of
//property data. Derived classes should call this for the parent first.
void ConstraintStiffSpring::ReadConstraintProperties(const GenericPropList *pProps)
{
	ConstraintBase::ReadConstraintProperties(pProps);

	m_sEndPointObject = pProps->GetString("SpringEnd", "");
}

bool ConstraintStiffSpring::SetupBodySpaceData(const LTRigidTransform& tInvBody1, const LTRigidTransform& tInvBody2)
{
	//get the position of our object which will serve as the constraint point
	LTVector vFirstPos;
	if(g_pLTServer->GetObjectPos(m_hObject, &vFirstPos) != LT_OK)
		return false;

	//we now need to find the second point 
	ObjArray<HOBJECT, 1> objArray;
	g_pLTServer->FindNamedObjects( m_sEndPointObject.c_str(), objArray );

	//make sure an object was found
	if( objArray.NumObjects() == 0 )
		return false;

	//and find the position of that object
	LTVector vSecondPos;
	if(g_pLTServer->GetObjectPos(objArray.GetObject(0), &vSecondPos) != LT_OK)
		return false;

	m_vPivotPt1 = tInvBody1 * vFirstPos;
	m_vPivotPt2 = tInvBody2 * vSecondPos;

	m_fDistance = vFirstPos.Dist(vSecondPos);

	return true;
}

//called to handle actual construction of this constraint
HPHYSICSCONSTRAINT ConstraintStiffSpring::CreateConstraint(HPHYSICSRIGIDBODY hBody1, HPHYSICSRIGIDBODY hBody2)
{
	return g_pLTServer->PhysicsSim()->CreateStiffSpring(hBody1, hBody2, m_vPivotPt1, m_vPivotPt2, m_fDistance);
}

//handles saving and loading all data to a message stream. Derived classes should
//call this first in order to work properly
void ConstraintStiffSpring::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	ConstraintBase::Save(pMsg, dwSaveFlags);

	pMsg->WriteLTVector(m_vPivotPt1);
	pMsg->WriteLTVector(m_vPivotPt2);
	pMsg->Writefloat(m_fDistance);
}

void ConstraintStiffSpring::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	ConstraintBase::Load(pMsg, dwLoadFlags);

	m_vPivotPt1 = pMsg->ReadLTVector();
	m_vPivotPt2 = pMsg->ReadLTVector();
	m_fDistance = pMsg->Readfloat();
}

// Write data specific to a stiffspring constraint into the create struct...
void ConstraintStiffSpring::SetupConstraintCreateStruct( PHYSICSCONSTRAINTCREATESTRUCT &rCS )
{
	rCS.m_eConstraintType = kConstraintType_StiffSpring;
	rCS.m_vPivotPt1 = m_vPivotPt1;
	rCS.m_vPivotPt2 = m_vPivotPt2;
	rCS.m_fDistance = m_fDistance;
}

// EOF
