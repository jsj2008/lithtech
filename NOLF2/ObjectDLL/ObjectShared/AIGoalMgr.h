// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalMgr.h
//
// PURPOSE : AIGoalMgr class definition
//
// CREATED : 6/7/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_MGR_H__
#define __AIGOAL_MGR_H__

#include "AIClassFactory.h"
#include "AIGoalAbstract.h"

#define GOAL_CMD_PREFIX		"GOAL_"

// Forward declarations.
class  CAIGoalAbstract;
class AIVolume;
struct AISenseRecord;

//
// VECTOR: List of goals.
//
typedef std::vector<CAIGoalAbstract*> AIGOAL_LIST;

//
// VECTOR: List of queued goals.
//
typedef std::vector<EnumAIGoalType> QUEUED_AIGOAL_LIST;
typedef std::vector<HSTRING> QUEUED_AIGOAL_ARG_LIST;

//
// MAP: Map of damage handling goals.
//
typedef std::multimap<uint32, CAIGoalAbstract*, std::greater<uint32> > AIGOAL_DAMAGE_MAP;


//
// GOAL SCRIPT DATA STRUCTURES.
//
struct GOAL_SCRIPT_STRUCT
{
	EnumAIGoalType	eGoalType;
	HSTRING			hstrNameValuePairs;
};
typedef std::vector<GOAL_SCRIPT_STRUCT> GOAL_SCRIPT_LIST;


//
// CLASS: Instance of an AI's goal manager.
//
class CAIGoalMgr : public CAIClassAbstract
{
	public :

		DECLARE_AI_FACTORY_CLASS(CAIGoalMgr);

		CAIGoalMgr( );
		~CAIGoalMgr( );

        virtual void		Save(ILTMessage_Write *pMsg);
        virtual void		Load(ILTMessage_Read *pMsg);

		void				Init(CAI* pAI);
		void				Term(LTBOOL bDestroyAll);

		void				SetGoalSet(const char* szGoalSet, LTBOOL bClearGoals);
		uint32				GetGoalSetIndex() const { return m_iGoalSet; }
		LTFLOAT				GetGoalSetTime() const { return m_fGoalSetTime; }

		CAIGoalAbstract*	AI_FACTORY_NEW_Goal(EnumAIGoalType eGoalType);

		CAIGoalAbstract*	AddGoal(EnumAIGoalType eGoalType, LTFLOAT fImportance, 
									LTFLOAT fTime, LTBOOL bScripted);
		void				DeleteGoalNextUpdate(EnumAIGoalType eGoalType);
		void				SetGoalSetNextUpdate(const char* szGoalSet);
		void				UpdateGoals();
		void				HandleGoalSenseTriggers(AISenseRecord* pSenseRecord);
		bool				HandleCommand(const CParsedMsg &cMsg);
		LTBOOL				HandleDamage(const DamageStruct& damage);
		void				HandleVolumeEnter(AIVolume* pVolume);
		void				HandleVolumeExit(AIVolume* pVolume);

		HMODELANIM			GetAlternateDeathAnimation();

		CAIGoalAbstract*	FindGoalByType(EnumAIGoalType eGoalType);

		LTBOOL				FreezeDecay() const { return ( m_pCurGoal && m_pCurGoal->DoesFreezeDecay() ); }

		// Current goal.

		LTBOOL IsCurGoal(CAIGoalAbstract* pGoal) { return (LTBOOL)(pGoal == m_pCurGoal); }

		// Locking/unlocking.
		// A locked goal does not decay, and can only be replaced by a sense trigger.

		void	LockGoal(CAIGoalAbstract* pGoal);
		void	UnlockGoal(CAIGoalAbstract* pGoal);
		LTBOOL	IsGoalLocked(CAIGoalAbstract* pGoal) { return (LTBOOL)(pGoal == m_pLockedGoal); }
		LTBOOL	HasLockedGoal() { return !!m_pLockedGoal; }

		EnumAIGoalType GetGoalTypeEnumFromName(const char* szGoalType);

		// Dialogue.

		void KillDialogue() { m_bKillDialogue = LTTRUE; }

		// Debugging.

		const CAIGoalAbstract* GetCurrentGoal() const { return m_pCurGoal; }

		AIGOAL_LIST::const_iterator BeginGoals() const { return m_lstGoals.begin(); }
		AIGOAL_LIST::const_iterator EndGoals() const { return m_lstGoals.end(); }

		GOAL_SCRIPT_LIST::const_iterator BeginScript() const { return m_lstScriptGoals.begin(); }
		GOAL_SCRIPT_LIST::const_iterator EndScript()   const { return m_lstScriptGoals.end(); }

		uint32  GetCurrentScriptStep() const { return m_nScriptStep; }

	protected:

		void	AddQueuedGoals();
		void	RunQueuedPrefixGoalCommands();
		void	RemoveGoal(EnumAIGoalType eGoalType);
		void	SetGoalSet(uint32 iGoalSet, LTBOOL bClearGoals);

		void	ChooseCurGoal();
		void	SetNextGoal(CAIGoalAbstract* pGoal);

		void	UpdateGoalTimers(LTFLOAT fTime);

		void	HandleGoalScript();
		void	ClearGoalScript();
		
	protected:

		CAI*				m_pAI;

		AIGOAL_LIST			m_lstGoals;
		CAIGoalAbstract*	m_pNextGoal;
		CAIGoalAbstract*	m_pCurGoal;
		CAIGoalAbstract*	m_pLastGoal;
		CAIGoalAbstract*	m_pTriggeredGoal;
		CAIGoalAbstract*	m_pLockedGoal;
		CAIGoalAbstract*	m_pDeactivatingGoal;		

		AIGOAL_DAMAGE_MAP	m_mapDamageHandlers;

		GOAL_SCRIPT_LIST	m_lstScriptGoals;
		uint32				m_nScriptStep;
		LTBOOL				m_bQueuedGoalScript;
	
		QUEUED_AIGOAL_LIST		m_lstQueuedGoals;
		QUEUED_AIGOAL_ARG_LIST	m_lstQueuedGoalArgs;

		QUEUED_AIGOAL_LIST		m_lstQueuedPrefixCmdGoals;
		QUEUED_AIGOAL_ARG_LIST	m_lstQueuedPrefixCmdGoalArgs;

		uint32				m_iGoalSet;
		LTFLOAT				m_fGoalSetTime;
		uint32				m_iQueuedGoalSet;

		LTBOOL				m_bKillDialogue;
};



#endif
