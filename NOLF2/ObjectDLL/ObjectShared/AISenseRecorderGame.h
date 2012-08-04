// ----------------------------------------------------------------------- //
//
// MODULE  : AISenseRecorderGame.h
//
// PURPOSE : AISenseRecorderGame class definition
//
// CREATED : 5/25/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSE_RECORDER_GAME_H__
#define __AISENSE_RECORDER_GAME_H__

#include "AISenseRecorderAbstract.h"

//
// CLASS: CAISenseRecorderAbstract is an abstract base class, 
//		  reusable for other games. CAISenseRecorderGame handles 
//		  stimuli specific to some Game.
//
class CAISenseRecorderGame : public CAISenseRecorderAbstract
{
	typedef CAISenseRecorderAbstract super;

	public : // Public methods
		
		virtual LTBOOL HandleSpecificStimuli(CAIStimulusRecord* pStimulusRecord, 
											 LTFLOAT* pfRateModifier);

	private: // Private methods.
		
		LTBOOL HandleEnemyWeaponImpactVisible(CAIStimulusRecord* pStimulusRecord);
		LTBOOL HandleEnemyFootprintVisible(CAIStimulusRecord* pStimulusRecord);
		LTBOOL HandleEnemyDangerVisible(CAIStimulusRecord* pStimulusRecord);
		LTBOOL HandleEnemyDisturbanceVisible(CAIStimulusRecord* pStimulusRecord);
		LTBOOL HandleEnemyLightDisturbanceVisible(CAIStimulusRecord* pStimulusRecord);
		LTBOOL HandleEnemyAlarmSound(CAIStimulusRecord* pStimulusRecord);
		LTBOOL HandleAllyDisturbanceVisible(CAIStimulusRecord* pStimulusRecord);
		LTBOOL HandleAllySpecialDamageVisible(CAIStimulusRecord* pStimulusRecord);
		LTBOOL HandleAllyDeathVisible(CAIStimulusRecord* pStimulusRecord);
		LTBOOL HandleEnemyVisible(CAIStimulusRecord* pStimulusRecord, LTBOOL bUseSightGrid, LTBOOL bUseInstantSeeDistance, LTFLOAT* pfRateModifier);
		LTBOOL HandleUndeterminedVisible(CAIStimulusRecord* pStimulusRecord, LTFLOAT* pfRateModifier);

	private : // Private member variables

};

#endif
