//-----------------------------------------------------------------------------
// ConstraintWheel.h
//
// Provides the definition for the wheel constraint that connects two
// rigid bodies together at a point and allows the first rigid body to rotate
// freely around the rotation axis, and move along the suspension axis within
// the range specified
//
//-----------------------------------------------------------------------------

#ifndef __CONSTRAINTWHEEL_H__
#define __CONSTRAINTWHEEL_H__

#ifndef __CONSTRAINTBASE_H__
#	include "ConstraintBase.h"
#endif

LINKTO_MODULE( ConstraintWheel );

class ConstraintWheel :
	public ConstraintBase
{
public:

	ConstraintWheel();

	//virtual function that derived classes must override to handle loading in of
	//property data. Derived classes should call this for the parent first.
	virtual void		ReadConstraintProperties(const GenericPropList *pProps);

	//this function is called only once when the object is created (NOT when it is loaded), and provides
	//the rigid bodies that the constraint will be linked between. This allows for rigid bodies to handle
	//setting up any data that must be specified in the space of the rigid body. This data should then
	//be saved and loaded and used. This is guaranteed to be called before CreateConstraint, and after
	//all other objects have been created. This should return false if it cannot setup the data
	//and therefore should not create the constraint.
	virtual bool		SetupBodySpaceData(const LTRigidTransform& tInvBody1, const LTRigidTransform& tInvBody2);

	//called to handle actual construction of this constraint
	HPHYSICSCONSTRAINT	CreateConstraint(HPHYSICSRIGIDBODY hBody1, HPHYSICSRIGIDBODY hBody2);

	//handles saving and loading all data to a message stream. Derived classes should
	//call this first in order to work properly
	virtual void	Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
	virtual void	Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);
	
	// Write data specific to a wheel constraint into the create struct...
	virtual void	SetupConstraintCreateStruct( PHYSICSCONSTRAINTCREATESTRUCT &rCS );


private:

	//the connection point in both body's spaces
	LTVector	m_vPivotPt1;
	LTVector	m_vPivotPt2;

	//the rotation axis in both rigid bodies spaces
	LTVector	m_vRotation1;
	LTVector	m_vRotation2;

	//the suspension axis, only needed in the second body's space
	LTVector	m_vSuspension2;

	//the limits of the suspension
	float		m_fSuspensionMin;
	float		m_fSuspensionMax;

	//the strength of the suspension (springiness)
	float		m_fSuspensionStrength;

	//the damping of the suspension (loss of energy)
	float		m_fSuspensionDamping;
};

#endif
