// ----------------------------------------------------------------------- //
//
// MODULE  : CDeathScene.h
//
// PURPOSE : Target information encapsulation class
//
// CREATED : 2/16/99
//
// ----------------------------------------------------------------------- //

#ifndef __DEATH_SCENE_H__
#define __DEATH_SCENE_H__

#include "ltengineobjects.h"
#include "CharacterAlignment.h"
#include "Character.h"

class CBody;
class CObjectRelationMgr;

class CDeathScene
{
	public : // Public methods

		CDeathScene();
		virtual ~CDeathScene();

        LTFLOAT GetNoiseVolume();
        LTFLOAT GetNoiseTime();

        LTFLOAT GetLastPainVolume();
        LTFLOAT GetLastPainTime();

        LTVector GetPosition();
		HOBJECT GetObject();
		LTBOOL WasPlayer();

		const RelationData& GetRelationData();
		CharacterDeath GetCharacterDeath();

		void Save(ILTMessage_Write *pMsg);
		void Load(ILTMessage_Read *pMsg);

	protected : // Protected methods

		friend class Body;

		void Set(CCharacter *pCharacter, Body *pBody);
        void SetNoise(LTFLOAT fVolume, LTFLOAT fTime);
        void SetPain(LTFLOAT fVolume, LTFLOAT fTime);

	private : // Private member variables

		CObjectRelationMgr* m_pObjectRelationMgr;
		CharacterDeath	 m_eCharacterDeath;
		LTBOOL		m_bWasPlayer;

		// When a body lingers

		LTObjRef	m_hSceneObject;

		// When there is no body

        LTFLOAT		m_fNoiseVolume;
        LTFLOAT		m_fNoiseTime;
        LTFLOAT		m_fLastPainVolume;
        LTFLOAT		m_fLastPainTime;
};

#endif
