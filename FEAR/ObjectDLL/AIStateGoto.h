// ----------------------------------------------------------------------- //
//
// MODULE  : AIStateGoto.h
//
// PURPOSE : AIStateGoto class declaration
//
// CREATED : 2/18/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AI_STATE_GOTO_H__
#define __AI_STATE_GOTO_H__

#include "AIState.h"


class CAIStateGoto : public CAIState
{
	typedef CAIState super;

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( State, CAIStateGoto, kState_Goto );

		CAIStateGoto( );
		~CAIStateGoto( );

		virtual void Load( ILTMessage_Read *pMsg );
		virtual void Save( ILTMessage_Write *pMsg );

		// Ctors/Dtors/Etc

		bool Init(CAI* pAI);

		// Update

		virtual void Update();
		void UpdateAnimation();

		// Simple acccessors

		void SetDest( const LTVector& vDest );
		void SetDestNode( HOBJECT hDestNode );
		void SetDestObject( HOBJECT hObj, const LTVector& vOffset );
		void SetDynamicRepathDistance(float flDist);
		void SetMovementProp(EnumAnimProp eMovement) { m_eMovement = eMovement; }
		void SetActivityProp(EnumAnimProp eActivity) { m_eActivity = eActivity; }
		void SetDirectionProp(EnumAnimProp eDir) { m_eDir = eDir; }

	protected :

		void		ArriveAtNode();
		void		GetObjectNMPosition( HOBJECT hObj, LTVector* pvPos );
		void		UpdateCollisionResponse();

	protected :

		LTVector		m_vDest;
		LTObjRef		m_hDestNode;
		LTObjRef		m_hDestObject;
		LTVector		m_vDestObjectOffset;

		EnumAnimProp	m_eMovement;
		EnumAnimProp	m_eActivity;
		EnumAnimProp	m_eDir;

		bool			m_bDestinationBlockedLastUpdate;
};


#endif
