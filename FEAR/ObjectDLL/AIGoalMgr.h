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
struct AISenseRecord;

//
// VECTOR: List of goals.
//
typedef std::vector<CAIGoalAbstract*, LTAllocator<CAIGoalAbstract*, LT_MEM_TYPE_OBJECTSHELL> > AIGOAL_LIST;


//
// CLASS: Instance of an AI's goal manager.
//
class CAIGoalMgr : public CAIClassAbstract
{
	public :

		DECLARE_AI_FACTORY_CLASS(CAIGoalMgr);

		CAIGoalMgr();
		~CAIGoalMgr();

        void		Save(ILTMessage_Write *pMsg);
        void		Load(ILTMessage_Read *pMsg);

		void				Init(CAI* pAI);
		void				Term(bool bDestroyAll);

		void				SetGoalSet(const char* szGoalSet, const char* szName, bool bClearGoals);
		uint32				GetGoalSetIndex() const { return m_iGoalSet; }
		double				GetGoalSetTime() const { return m_fGoalSetTime; }

		CAIGoalAbstract*	AI_FACTORY_NEW_Goal(EnumAIGoalType eGoalType);

		CAIGoalAbstract*	AddGoal(EnumAIGoalType eGoalType, double fTime);
		void				RemoveGoal(EnumAIGoalType eGoalType);

		CAIGoalAbstract*	FindGoalByType(EnumAIGoalType eGoalType);

		void				SelectRelevantGoal();
		void				UpdateGoal();

		// Current goal.

		bool IsCurGoal( const CAIGoalAbstract* const pGoal ) const { return (bool)(pGoal == m_pCurGoal); }

		// Debugging.

		CAIGoalAbstract* GetCurrentGoal() const { return m_pCurGoal; }

		AIGOAL_LIST::const_iterator BeginGoals() const { return m_lstGoals.begin(); }
		AIGOAL_LIST::const_iterator EndGoals() const { return m_lstGoals.end(); }

	protected:

		void	SetGoalSet(uint32 iGoalSet, const char* szName, bool bClearGoals);

		void				UpdateGoalRelevances( bool bReplan );
		CAIGoalAbstract*	FindMostRelevantGoal( bool bRecalculate );

	protected:

		CAI*				m_pAI;

		AIGOAL_LIST			m_lstGoals;
		CAIGoalAbstract*	m_pCurGoal;

		uint32				m_iGoalSet;
		double				m_fGoalSetTime;
};



#endif
