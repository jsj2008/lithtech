//----------------------------------------------------------------------------
//              
//	MODULE:		AIHumanStateObstruct.h
//              
//	PURPOSE:	CAIHumanStateObstruct declaration
//              
//	CREATED:	12.12.2001
//
//	(c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	-
//              
//              
//----------------------------------------------------------------------------

#ifndef __AIHUMANSTATEOBSTRUCT_H__
#define __AIHUMANSTATEOBSTRUCT_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// Includes
#include "AIHumanState.h"
#include "AINode.h"

// Forward declarations
class AINodeObstruct;

// Globals

// Statics


//----------------------------------------------------------------------------
//              
//	CLASS:		CAIHumanStateObstruct
//              
//	PURPOSE:	State for characters to use Obstruct nodes.  Characters use
//				Obstruct nodes to imped progress along a vector.  For example,
//				an obstruct goal may result in a character running in front of
//				the player, entering a defensive stance, and waiting for any
//				of a number of things to time out of the stance.
//				
//----------------------------------------------------------------------------
class CAIHumanStateObstruct : public CAIHumanState
{
	typedef CAIHumanState super;

	public :

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateObstruct, kState_HumanObstruct);

		CAIHumanStateObstruct( );
		~CAIHumanStateObstruct( );

		// Ctors/dtors/etc

		virtual LTBOOL Init(CAIHuman* pAIHuman);
		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);

		virtual void Update(void);
		virtual void UpdateAnimation(void);
		virtual CMusicMgr::Mood GetMusicMood(void) { return CMusicMgr::eMoodAggressive; }

		// Simple acccessors
		void	SetNode(AINodeObstruct& pUseNode);
		void	SetObjectToObstruct(HOBJECT);
		void	SetAcceptableDistanceToNode(float fDist);

		enum eStateTerminationConditions
		{
			kDC_Invalid	= 0,
			kDC_Time	= 1,
			kDC_Event	= 2,
			kDC_Message	= 3,
			kDC_Damage	= 3,
		};

	protected:

	private:
		void MaybePlayFirstUpdateSound();
		void ClearObstructObject();

		void SetPathNode(AINodeObstruct& ObstructNode);
		void InitPath(CAIHuman* pAIHuman);

		bool IsValidObstructNode(AINodeObstruct& Node);
		bool IsAICloseEnoughToNode(void);
		bool IsNodeStillValid(void);
		bool AttemptSetPathToNode(AINodeObstruct& Node);
		bool CanUpdatePath(void);
		HOBJECT GetObjectToObstruct(void);

		// Pointer to the node to do the obstruction from.  ie, the
		// AI will run to this point and do the obstruction either here
		// or in some proximity to here.
		LTObjRef	m_hNodeToDoObstructAt;

		// How close is close enough to the obstruction node?
		float m_fCloseEnoughDistSqr;

		// Object we are trying to block/obstruct at this node
		LTObjRef	m_hObjectToObstruct;
};

#endif // __AIHUMANSTATESENTRYCHALLENGE_H__

