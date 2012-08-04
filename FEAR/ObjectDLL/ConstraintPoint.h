//-----------------------------------------------------------------------------
// ConstraintPoint.h
//
// Provides the definition for the point to point constraint that connects two
// rigid bodies together at a single point. Most of this functionality comes
// from the base constraint object, and all this needs to handle is loading
// in properties and setting up the actual constraint.
//
//-----------------------------------------------------------------------------

#ifndef __CONSTRAINTPOINT_H__
#define __CONSTRAINTPOINT_H__

#ifndef __CONSTRAINTBASE_H__
#	include "ConstraintBase.h"
#endif

LINKTO_MODULE( ConstraintPoint );

class ConstraintPoint :
	public ConstraintBase
{
public:

	ConstraintPoint();

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

	// Write data specific to a point constraint into the create struct...
	virtual void SetupConstraintCreateStruct( PHYSICSCONSTRAINTCREATESTRUCT &rCS );


private:

	//the connection point in both body's spaces
	LTVector	m_vPivotPt1;
	LTVector	m_vPivotPt2;

};

#endif
