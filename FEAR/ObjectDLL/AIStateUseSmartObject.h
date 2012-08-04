// ----------------------------------------------------------------------- //
//
// MODULE  : AIStateUseSmartObject.h
//
// PURPOSE : AIStateUseSmartObject class declaration
//
// CREATED : 2/18/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AI_STATE_USE_SMART_OBJECT_H__
#define __AI_STATE_USE_SMART_OBJECT_H__

#include "AIState.h"
#include "AnimationContext.h"
#include "AnimationProp.h"

// Forward declarations.
class	AINodeSmartObject;
struct  AIDB_SmartObjectRecord;


class CAIStateUseSmartObject : public CAIState
{
	typedef CAIState super;

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( State, CAIStateUseSmartObject, kState_UseSmartObject );

		CAIStateUseSmartObject( );

		virtual void Load( ILTMessage_Read *pMsg );
		virtual void Save( ILTMessage_Write *pMsg );

		// Ctors/Dtors/Etc

		bool Init( CAI* pAI );

		// Update

		void Update();
		void UpdateAnimation();

		// Simple acccessors

		bool			SetNode( AINodeSmartObject* pSmartObjectNode );
		HOBJECT			GetNode() const { return m_hSmartObjectNode; }

		void			SetSmartObject( AIDB_SmartObjectRecord* pSmartObject );
		
		void			SetProp( EnumAnimPropGroup eGroup, EnumAnimProp eProp ) { m_Props.Set( eGroup, eProp ); }
		EnumAnimProp	GetProp( EnumAnimPropGroup eGroup ) { return m_Props.Get( eGroup ); }

		void			SetLooping( bool bLooping ) { m_bLooping = bLooping; }
		void			SetLoopTime( float fMinLoopTime, float fMaxLoopTime );

	protected :

		HOBJECT				m_hSmartObjectNode;

		CAnimationProps		m_Props;
		
		float				m_fMinFidgetTime;
		float				m_fMaxFidgetTime;
		double				m_fNextFidgetTime;
		
		bool				m_bLooping;
		double				m_fLoopTime;
		double				m_fLoopStartTime;

		bool				m_bLockNode;
		bool				m_bFirstUpdate;
};


#endif
