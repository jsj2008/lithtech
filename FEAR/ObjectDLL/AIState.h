// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_STATE_H__
#define __AI_STATE_H__

#include "AIClassFactory.h"
#include "AIEnumStateTypes.h"

// Forward Declarations.
class CAI;

//
// CLASS: State
//
class CAIState : public CAIClassAbstract
{
	public : // Public methods

		DECLARE_AI_FACTORY_CLASS_ABSTRACT_SPECIFIC(State);

		CAIState( );
		virtual ~CAIState( );

		// Ctors/Dtors/etc

		virtual bool Init(CAI* pAI);

		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);

		// Methods

		virtual void Update() {}
		virtual void UpdateAnimation() {}

		// Template Functions
	
		ENUM_AIStateStatus GetStateStatus() { return m_eStateStatus; }
		const char* GetName() { return s_aszStateTypes[GetStateClassType()]; }

	protected : // Private member variables

		CAI*				m_pAI;						// Backpointer to our AI
		ENUM_AIStateStatus	m_eStateStatus;				// What step of the state are we on?
};					

#endif
