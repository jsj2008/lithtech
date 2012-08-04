//-----------------------------------------------------------------------------
// ConstraintPrismatic.h
//
// Provides the definition for the prismatic constraint that connects two
// rigid bodies together at a point and allows the first rigid body to move
// along a single axis within a provided range.
//
//-----------------------------------------------------------------------------

#ifndef __CONSTRAINTPRISMATIC_H__
#define __CONSTRAINTPRISMATIC_H__

#ifndef __CONSTRAINTBASE_H__
#	include "ConstraintBase.h"
#endif

LINKTO_MODULE( ConstraintPrismatic );

class ConstraintPrismatic :
	public ConstraintBase
{
public:

	ConstraintPrismatic();

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

	// Write data specific to a prismatic constraint into the create struct...
	virtual void	SetupConstraintCreateStruct( PHYSICSCONSTRAINTCREATESTRUCT &rCS );

private:

	//the connection point in both body's spaces
	LTVector	m_vPivotPt1;
	LTVector	m_vPivotPt2;

	//the movement axis in both rigid bodies spaces
	LTVector	m_vAxis1;
	LTVector	m_vAxis2;

	//vector perpendicular to the movement axis in both rigid bodies spaces
	LTVector	m_vAxisPerp1;
	LTVector	m_vAxisPerp2;

	//the limits of the movement
	float		m_fMovementMin;
	float		m_fMovementMax;

	//the strength of the friction as the body moves
	float		m_fFriction;
};

#endif
