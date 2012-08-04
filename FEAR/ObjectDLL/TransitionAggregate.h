// ----------------------------------------------------------------------- //
//
// MODULE  : TransitionAggregate.h
//
// PURPOSE : The TransitionAggregate object
//
// CREATED : 12/03/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#ifndef __TRANSITION_AGGREGATE_H__
#define __TRANSITION_AGGREGATE_H__

//
// Includes...
//
	
	#include "iaggregate.h"
	#include "BankedList.h"

class CTransitionAggregate : public IAggregate
{
	public : 

		friend class CBankedList<CTransitionAggregate>;

	/* [RP] - NOTE:	We keep the constructor and destructor private to prevent declaring
					an instance of this class.  This forces the use of GameBase::MakeTransitionable()
					on any object you wish to add a CTransitionAggregate to.					
	*/

	private : // Methods...

		CTransitionAggregate( );
		~CTransitionAggregate( );

	
	protected : // Methods...

        uint32	EngineMessageFn( LPBASECLASS pObject, uint32 messageID, void *pData, float lData );
		void	ObjectCreated( LPBASECLASS pObject );
       
		void	Save( ILTMessage_Write *pMsg, uint32 dwSaveFlags );
		void	Load( ILTMessage_Read *pMsg, uint32 dwLoadFlags );

	protected : // Members...

		LTObjRef	m_hObject;		// Object that is associated to this aggregate.

};

#endif // __TRANSITION_AGGREGATE_H__
