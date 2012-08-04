// ----------------------------------------------------------------------- //
//
// MODULE  : CTFFlagSFX.h
//
// PURPOSE : Client side representation on CTFFlag
//
// CREATED : 05/08/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CTFFLAGSFX_H__
#define __CTFFLAGSFX_H__

//
// Includes...
//

#include "SpecialFX.h"
#include "SharedFXStructs.h"

class FlagSFXStateMachine;
class CTFFlagBaseSFX;

class CTFFlagSFX : public CSpecialFX
{
	public:	// Methods...

		CTFFlagSFX( );
		virtual ~CTFFlagSFX( );

		// Initialize the client side turret class...
		virtual bool Init( HLOCALOBJ hServObj, ILTMessage_Read *pMsg );
        		
		// Update the weapon associated with the turret...
		virtual bool Update( );

		// Retrieve the ID associated with TurretFX objects...
		virtual uint32 GetSFXID( ) { return SFX_CTFFLAG_ID; }

		// Handle a message recieved from the server side Turret object...
		virtual bool OnServerMessage( ILTMessage_Read *pMsg );

		// Sets the flag base object.
		void SetFlagBase( HOBJECT hObject, CTFFlagBaseSFX* pCTFFlagBaseSFX );
		CTFFlagBaseSFX* GetFlagBase( ) const { return m_pCTFFlagBaseSFX; }

		// Accessor to create struct data.
		CTFFLAGCREATESTRUCT const& GetCS( ) const { return m_csCTFFlag; }

		// Checks if local player can grab this flag.
		bool CanGrabFlag( ) const;

		// Checks if flag is in the base.
		bool IsFlagInBase( ) const;

	private : // Members...

		CTFFLAGCREATESTRUCT m_csCTFFlag;
		CTFFlagBaseSFX* m_pCTFFlagBaseSFX;

		// Registered with CPlayerMgr::m_PickupObjectDetector;
		ObjectDetectorLink	m_iObjectDetectorLink;

		// State machine object.
		friend FlagSFXStateMachine;
		FlagSFXStateMachine* m_pFlagSFXStateMachine;

		// Flag carrier object.
		HOBJECT m_hFlagCarrier;
};

#endif // __CTFFLAGSFX_H__