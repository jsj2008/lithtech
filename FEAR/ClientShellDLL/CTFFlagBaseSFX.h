// ----------------------------------------------------------------------- //
//
// MODULE  : CTFFlagBaseSFX.h
//
// PURPOSE : Client side representation on CTFFlagBase
//
// CREATED : 05/08/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CTFFLAGBASESFX_H__
#define __CTFFLAGBASESFX_H__

//
// Includes...
//

#include "SpecialFX.h"
#include "SharedFXStructs.h"
#include "objectdetector.h"

class CTFFlagSFX;
class FlagBaseSFXStateMachine;

class CTFFlagBaseSFX : public CSpecialFX
{
	public:	// Methods...

		CTFFlagBaseSFX( );
		virtual ~CTFFlagBaseSFX( );

		// Initialize the client side turret class...
		virtual bool Init( HLOCALOBJ hServObj, ILTMessage_Read *pMsg );
        		
		// Update the weapon associated with the turret...
		virtual bool Update( );

		// Retrieve the ID associated with TurretFX objects...
		virtual uint32 GetSFXID( ) { return SFX_CTFFLAGBASE_ID; }

		// Handle a message recieved from the server side Turret object...
		virtual bool OnServerMessage( ILTMessage_Read *pMsg );

		// Sets the flag object.
		void SetFlag( HOBJECT hObject, CTFFlagSFX* pCTFFlagSFX );
		CTFFlagSFX* GetFlag( ) const { return m_pCTFFlagSFX; }

		// Sets whether base has the flag.
		bool SetHasFlag( bool bValue );

		// Accessor to create struct data.
		CTFFLAGBASECREATESTRUCT const& GetCS( ) const { return m_csCTFFlagBase; }

		// Get the complete list of all CTFFlagBaseSFX's.
		typedef std::vector< CTFFlagBaseSFX*, LTAllocator<CTFFlagBaseSFX*, LT_MEM_TYPE_CLIENTSHELL> > CTFFlagBaseSFXList;
		static CTFFlagBaseSFXList const& GetCTFFlagBaseSFXList( ) { return m_lstCTFFlagBaseSFX; }

	private : // Members...

		CTFFLAGBASECREATESTRUCT m_csCTFFlagBase;
		CTFFlagSFX* m_pCTFFlagSFX;

		static CTFFlagBaseSFXList m_lstCTFFlagBaseSFX;

		FlagBaseSFXStateMachine* m_pFlagBaseSFXStateMachine;
};

#endif // __CTFFLAGBASESFX_H__