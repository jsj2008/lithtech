// ----------------------------------------------------------------------- //
//
// MODULE  : AIStateAnimate.h
//
// PURPOSE : AIStateAnimate class declaration
//
// CREATED : 2/18/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AI_STATE_ANIMATE_H__
#define __AI_STATE_ANIMATE_H__

#include "AIState.h"
#include "AnimationContext.h"


enum AISPECIAL_ANIMATION_TYPE
{
	kSpecial_None	= 0x00,
	kSpecial_Props	= 0x01,
	kSpecial_String	= 0x02,
	kSpecial_Handle	= 0x04,
};


class CAIStateAnimate : public CAIState
{
	typedef CAIState super;

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( State, CAIStateAnimate, kState_Animate );

		// Ctors/Dtors/Etc

		CAIStateAnimate();
		~CAIStateAnimate();

		// Load/Save

		void Load( ILTMessage_Read *pMsg );
		void Save( ILTMessage_Write *pMsg );

		// Update

		void Update();
		void UpdateAnimation();

		// Data access.

		void SetAnimation( HMODELANIM hAni, bool bLoop );
		void SetAnimation( const char* strAnim, bool bLoop );
		void SetAnimation( const CAnimationProps& animProps, bool bLoop );

		void DepartNode();
		void CheckDepartedNode( bool bCheck ) { m_bCheckDepartedNode = bCheck; }

		void GetAnimationProps( CAnimationProps* pProps );

	protected :

		void PlaySpecial();

	protected :

		AISPECIAL_ANIMATION_TYPE	m_eSpecialType;

		CAnimationProps m_animProps;
		bool			m_bLoop;

		bool			m_bCheckDepartedNode;
		LTVector		m_vInitialPos;
};


#endif
