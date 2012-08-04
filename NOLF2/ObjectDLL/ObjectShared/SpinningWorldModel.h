// ----------------------------------------------------------------------- //
//
// MODULE  : SpinningWorldModel.h
//
// PURPOSE : The SpinningWorldModel object
//
// CREATED : 5/22/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SPINNING_WORLD_MODEL_H__
#define __SPINNING_WORLD_MODEL_H__

//
// Includes...
//

	#include "ActiveWorldModel.h"

LINKTO_MODULE( SpinningWorldModel );

//
// Structs...
//

class SpinningWorldModel : public ActiveWorldModel
{
	public: // Methods...

		SpinningWorldModel( );
		virtual ~SpinningWorldModel( );

	protected: // Members...

		LTVector	m_vVelocity;		// Velocity we have
		LTVector	m_vFinalVelocity;	// Velocity we should end up with	
		LTFLOAT		m_fLastTime;
		LTBOOL		m_bUpdateSpin;		// If we stuck on an object then dont update our spin

	protected: // Methods...

		// Engine message handelers

		virtual void	OnSave( ILTMessage_Write *pMsg, uint32 dwSaveFlags );
		virtual void	OnLoad( ILTMessage_Read *pMsg, uint32 dwSaveFlags );

		virtual void	ReadProps( ObjectCreateStruct *pOCS );

		// Update state methods...
		
		virtual void	UpdateOn( const LTFLOAT &fCurTime );
		virtual void	UpdatePowerOn( const LTFLOAT &fCurTime );
		virtual void	UpdateOff( const LTFLOAT &fCurTime );
		virtual void	UpdatePowerOff( const LTFLOAT &fCurTime );

		virtual void	SetOn( LTBOOL bInitialized = LTFALSE );

};


#endif // __SPINNING_WORLD_MODEL_H__