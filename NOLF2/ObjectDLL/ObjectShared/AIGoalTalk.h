// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalTalk.h
//
// PURPOSE : AIGoalTalk class definition
//
// CREATED : 7/24/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_TALK_H__
#define __AIGOAL_TALK_H__

#include "AIGoalGuard.h"

// Forward Declarations.

enum EnumAnimProp;


class CAIGoalTalk : public CAIGoalGuard
{
	typedef CAIGoalGuard super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalTalk, kGoal_Talk);

		 CAIGoalTalk( );
		~CAIGoalTalk( );

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Activation.

		virtual void ActivateGoal();
		virtual void DeactivateGoal();

		// Updating.

		void UpdateGoal();
		virtual void RecalcImportance();

		// Guard Node.

		virtual void	FindGuardNode();

		// Sense Handling.

		virtual LTBOOL HandleGoalSenseTrigger(AISenseRecord* pSenseRecord);

		// Command Handling.

		virtual LTBOOL HandleNameValuePair(const char *szName, const char *szValue);

		// Dialog object access.

		LTBOOL	HasTalkNode() const { return !!m_hGuardNode; }
		LTBOOL	IsInTalkPosition() const { return m_bInTalkPosition; }
		LTBOOL	RequestDialogue(HOBJECT hDialogue);
		void	StartDialogue();
		void	StartTalking();
		void	StopTalking();
		void	ResetDialogue();

	protected:

		// State Handling.

		virtual void	HandleStateIdle();
		virtual void	HandleStateGoto();
		virtual void	HandleStateTalk();
		void			SetStateTalk();

		void			HandleInRadius();

		virtual void	GotoNode();

	protected:

		EnumAnimProp	m_eMood;
		EnumAnimProp	m_eMovement;
		HSTRING			m_hstrGesture;
		LTFLOAT			m_fFaceTime;
		LTBOOL			m_bStartedDialog;
		LTBOOL			m_bTalking;
		LTBOOL			m_bInTalkPosition;
		LTBOOL			m_bRetriggerDialogue;
		LTBOOL			m_bDisposableDialogue;
		LTObjRef		m_hDialogue;
};


#endif
