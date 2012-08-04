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
		double		m_fLastTime;
		bool		m_bUpdateSpin;		// If we stuck on an object then dont update our spin

	protected: // Methods...

		// Engine message handelers

		virtual void	OnSave( ILTMessage_Write *pMsg, uint32 dwSaveFlags );
		virtual void	OnLoad( ILTMessage_Read *pMsg, uint32 dwSaveFlags );

		virtual void	ReadProps( const GenericPropList *pProps );

		// Update state methods...
		
		virtual void	UpdateOn( const double &fCurTime );
		virtual void	UpdatePowerOn( const double &fCurTime );
		virtual void	UpdateOff( const double &fCurTime );
		virtual void	UpdatePowerOff( const double &fCurTime );

		virtual void	SetOn( bool bInitialized = false );
		virtual void	SetPowerOn( float fTime, uint8 nWaveform );

	private:

		PREVENT_OBJECT_COPYING( SpinningWorldModel );

};


#endif // __SPINNING_WORLD_MODEL_H__
