// ----------------------------------------------------------------------- //
//
// MODULE  : TransitionArea.h
//
// PURPOSE : The TransitionArea object
//
// CREATED : 11/27/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#ifndef __TRANSITION_AREA_H__
#define __TRANSITION_AREA_H__

//
// Includes...
//
	
	#include "GameBase.h"

LINKTO_MODULE( TransitionArea );

class TransitionArea : public GameBase 
{
	public : // Methods...

		TransitionArea( );
		~TransitionArea( );

		int GetTransitionLevel( ) const { return m_nTransLevel; }	
		LTVector GetDims();
		const LTransform& GetWorldTransform( ) const { return m_tfWorld; }

	protected : // Methods...

		uint32	EngineMessageFn( uint32 messageID, void *pData, LTFLOAT fData );
		bool	OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

		void	Save( ILTMessage_Write *pMsg, uint32 dwSaveFlags );
		void	Load( ILTMessage_Read *pMsg, uint32 dwLoadFlags );

		void	ReadProps( ObjectCreateStruct *pOCS );
		void	PostReadProps( ObjectCreateStruct *pOCS );

		void	Update( );

		void	HandleTransition( );

	protected : // Members...

		uint32	m_dwFlags;
		int		m_nTransLevel;

	private : // Members...

		LTransform	m_tfWorld;

		// Set when we get a transition trigger.  Transition happens in update.
		bool		m_bRequestTransition;
};

#endif // __TRANSITION_AREA_H__