// ----------------------------------------------------------------------- //
//
// MODULE  : CPhysicsConstraintFX.h
//
// PURPOSE : Client side physics simulation constraint
//
// CREATED : 05/26/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PHYSICS_CONSTRAINTS_FX_H__
#define __PHYSICS_CONSTRAINTS_FX_H__

//
// Includes...
//

#include "SpecialFX.h"
#include "SharedFXStructs.h"

class CPhysicsConstraintFX : public CSpecialFX
{
	public: // Methods...

		CPhysicsConstraintFX( );
		virtual ~CPhysicsConstraintFX( );

		// Initialize the client side class...
		virtual bool Init( HLOCALOBJ hServObj, ILTMessage_Read *pMsg );

		// Poll the dependent objects until they become available...
		virtual bool Update( );

		// Retrieve the ID associated with CPhysicsConstraintFX objects...
		virtual uint32 GetSFXID( ) { return SFX_PHYSICS_CONSTRAINT_ID; }


	private: // Methods...

		// Actually create the constraint in the client physics simulation...
		virtual bool CreatePhysicsConstraint( );

		// Create the actual physics constraint object based on the constraint type...
		HPHYSICSCONSTRAINT ConstrainRigidbodies( HPHYSICSRIGIDBODY hBody1, HPHYSICSRIGIDBODY hBody2 );


	private: // Members...

		PHYSICSCONSTRAINTCREATESTRUCT m_cs;

		HPHYSICSCONSTRAINT m_hConstraint;

		// Message used to initialize constraint.
		// This needs to be cached so the dependent objects may be polled for when they are
		// available for use on the client.
		CLTMsgRef_Read m_cInitMsg;

		bool m_bHasDependentObjects;
};

#endif

// EOF
