// ----------------------------------------------------------------------- //
//
// MODULE  : ControlPointSFX.h
//
// PURPOSE : Client side representation of ControlPoint object
//
// CREATED : 02/10/06
//
// (c) 2006 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CONTROLPOINTSFX_H__
#define __CONTROLPOINTSFX_H__

//
// Includes...
//

#include "SpecialFX.h"
#include "SharedFXStructs.h"

class ControlPointSFXStateMachine;

class ControlPointSFX : public CSpecialFX
{
	public:	// Methods...

		ControlPointSFX( );
		virtual ~ControlPointSFX( );

		// Initialize the client side turret class...
		virtual bool Init( HLOCALOBJ hServObj, ILTMessage_Read *pMsg );
        		
		// Update the object.
		virtual bool Update( );

		// Retrieve the ID associated with object...
		virtual uint32 GetSFXID( ) { return SFX_CONTROLPOINT_ID; }

		// Handle a message recieved from the server side object...
		virtual bool OnServerMessage( ILTMessage_Read *pMsg );

		// Accessor to create struct data.
		CONTROLPOINTCREATESTRUCT const& GetCS( ) const { return m_csControlPoint; }

		// Get the ETeamId that owns the CP.
		ETeamId GetTeamId( ) const;

		// Get the control level for this CP.
		float GetControlLevel( ) const;

		// Get the complete list of all ControlPointSFX's.
		typedef std::vector< ControlPointSFX*, LTAllocator<ControlPointSFX*, LT_MEM_TYPE_CLIENTSHELL> > ControlPointSFXList;
		static ControlPointSFXList const& GetControlPointSFXList( ) { return m_lstControlPointSFX; }

		//retrieve the control point associated with a particular id
		static ControlPointSFX* GetControlPoint(uint16 nID);

	private : // Members...

		CONTROLPOINTCREATESTRUCT m_csControlPoint;

		static ControlPointSFXList m_lstControlPointSFX;

		friend ControlPointSFXStateMachine;
		ControlPointSFXStateMachine* m_pControlPointSFXStateMachine;
};

#endif // __CONTROLPOINTSFX_H__