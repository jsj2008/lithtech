// ----------------------------------------------------------------------- //
//
// MODULE  : DefenseGlobeAI.h
//
// PURPOSE : DefenseGlobeAI
//
// CREATED : 12/01/97
//
// ----------------------------------------------------------------------- //

#ifndef __DEFENSE_GLOBAL_AI_H__
#define __DEFENSE_GLOBAL_AI_H__

#include "BaseCharacter.h"
#include "cpp_engineobjects_de.h"
#include "AI_Mgr.h"




class AI_DefenseGlobe : public AI_Mgr
{
	public :

		AI_DefenseGlobe() :    AI_Mgr() {};
		~AI_DefenseGlobe() {}

};

class DefenseGlobeAI : public CBaseCharacter
{
	public :

 		DefenseGlobeAI();

		//R fuzzy relation array
		static float		m_fOutputState[3][NUM_OUPUT_STATES][NUM_INTERNAL_STATES];

		//S fuzzy relation array
		static float		m_fInternalState[NUM_INPUT_STATES][NUM_INTERNAL_STATES][NUM_INTERNAL_STATES];

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);

	private :


		void	PostPropRead(ObjectCreateStruct *pStruct);
		DBOOL	InitialUpdate(DVector *pMovement);

        AI_DefenseGlobe	m_ai;	   // I'm Alive!
 
		//keep only one copy per AI of animations
        static CAnim_Sound	m_Anim_Sound;
};

//static data member initialization
CAnim_Sound	DefenseGlobeAI::m_Anim_Sound;

float		DefenseGlobeAI::m_fOutputState[3][7][7]=
{
	{
		{	1.0f,	0.0f,	0.0f,	0.0f,	0.0f,	0.0f,	0.0f},
		{	0.0f,	1.0f,	0.0f,	0.0f,	0.0f,	0.0f,	0.0f},
		{	0.0f,	0.0f,	1.0f,	0.0f,	0.0f,	0.0f,	0.0f},
		{	0.0f,	0.0f,	0.0f,	1.0f,	0.0f,	0.0f,	0.0f},
		{	0.0f,	0.0f,	0.0f,	0.0f,	1.0f,	0.0f,	0.0f},
		{	0.0f,	0.0f,	0.0f,	0.0f,	0.0f,	1.0f,	0.0f},
		{	0.0f,	0.0f,	0.0f,	0.0f,	0.0f,	0.0f,	1.0f}
	},
	{
		{	1.0f,	0.0f,	0.0f,	0.0f,	0.0f,	0.0f,	0.0f},
		{	0.0f,	1.0f,	0.0f,	0.0f,	0.0f,	0.0f,	0.0f},
		{	0.0f,	0.0f,	1.0f,	0.0f,	0.0f,	0.0f,	0.0f},
		{	0.0f,	0.0f,	0.0f,	1.0f,	0.0f,	0.0f,	0.0f},
		{	0.0f,	0.0f,	0.0f,	0.0f,	1.0f,	0.0f,	0.0f},
		{	0.0f,	0.0f,	0.0f,	0.0f,	0.0f,	1.0f,	0.0f},
		{	0.0f,	0.0f,	0.0f,	0.0f,	0.0f,	0.0f,	1.0f}
	},
	{
		{	1.0f,	0.0f,	0.0f,	0.0f,	0.0f,	0.0f,	0.0f},
		{	0.0f,	1.0f,	0.0f,	0.0f,	0.0f,	0.0f,	0.0f},
		{	0.0f,	0.0f,	1.0f,	0.0f,	0.0f,	0.0f,	0.0f},
		{	0.0f,	0.0f,	0.0f,	1.0f,	0.0f,	0.0f,	0.0f},
		{	0.0f,	0.0f,	0.0f,	0.0f,	1.0f,	0.0f,	0.0f},
		{	0.0f,	0.0f,	0.0f,	0.0f,	0.0f,	1.0f,	0.0f},
		{	0.0f,	0.0f,	0.0f,	0.0f,	0.0f,	0.0f,	1.0f}
	}
};

float		DefenseGlobeAI::m_fInternalState[6][7][7] =
{		
	{//SIGHT_NEAR	
		{	1.00f,	0.20f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f},
		{	1.00f,	0.20f,	0.00f,	0.20f,	0.25f,	0.10f,	0.10f},
		{	0.80f,	0.20f,	0.00f,	0.10f,	0.00f,	0.10f,	0.10f},
		{	1.00f,	0.00f,	0.00f,	0.25f,	0.00f,	0.25f,	0.25f},
		{	1.00f,	0.00f,	0.00f,	0.00f,	1.00f,	0.00f,	0.00f},
		{	1.00f,	0.20f,	0.00f,	0.50f,	0.00f,	0.25f,	0.25f},
		{	1.00f,	0.20f,	0.00f,	0.50f,	0.00f,	0.25f,	0.25f}
	},
	{//SOUND_NEAR	
		{	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f},
		{	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f},
		{	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f},
		{	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f},
		{	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f},
		{	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f},
		{	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f}
	},
	{//SMELL_NEAR	
		{	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f},
		{	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f},
		{	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f},
		{	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f},
		{	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f},
		{	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f},
		{	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f}
	},
	{//SENSE_NEAR	
		{	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f},
		{	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f},
		{	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f},
		{	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f},
		{	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f},
		{	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f},
		{	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f}
	},
	{//HIT_BY_OBJ	
		{	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f},
		{	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f},
		{	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f},
		{	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f},
		{	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f},
		{	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f},
		{	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f}
	},
	{//THREAT	
		{	0.50f,	0.50f,	1.00f,	0.00f,	0.00f,	0.10f,	0.10f},
		{	0.40f,	0.50f,	1.00f,	0.00f,	0.00f,	0.25f,	0.25f},
		{	0.25f,	0.00f,	1.00f,	0.00f,	0.00f,	0.10f,	0.10f},
		{	0.50f,	0.00f,	1.00f,	0.50f,	0.00f,	0.25f,	0.25f},
		{	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f,	0.00f},
		{	0.20f,	0.00f,	1.00f,	0.25f,	0.00f,	0.25f,	0.25f},
		{	0.20f,	0.00f,	1.00f,	0.25f,	0.00f,	0.25f,	0.25f}
	}
};

#endif // __DEFENSE_GLOBAL_AI_H__