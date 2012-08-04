// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionSurveyArea.h
//
// PURPOSE : AIActionSurveyArea class definition
//
// CREATED : 3/27/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_SURVEY_AREA_H__
#define __AIACTION_SURVEY_AREA_H__


// Forward declarations.

class	CAnimationProps;


class CAIActionSurveyArea : public CAIActionAbstract
{
	typedef CAIActionAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionSurveyArea, kAct_SurveyArea );

		CAIActionSurveyArea();

		// CAIActionAbstract members.

		virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual	bool	IsActionComplete( CAI* pAI );
};

// ----------------------------------------------------------------------- //

#endif
