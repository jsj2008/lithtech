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

class CDeathScene
{
	public : // Public methods

		CDeathScene()
		{
			m_eCharacterDeath = CD_NORMAL;
			m_eCharacterClass = UNKNOWN;

            m_hSceneObject = LTNULL;

			m_fNoiseVolume = 0.0f;
			m_fNoiseTime = 0.0f;

			m_fLastPainTime		= -(float)(INT_MAX);
			m_fLastPainVolume	= 0.0f;
		}

        LTFLOAT GetNoiseVolume() { return m_fNoiseVolume; }
        LTFLOAT GetNoiseTime() { return m_fNoiseTime; }

        LTFLOAT GetLastPainVolume() { return m_fLastPainVolume; }
        LTFLOAT GetLastPainTime() { return m_fLastPainTime; }

        LTVector GetPosition()
		{
           LTVector vPos;
           g_pLTServer->GetObjectPos(m_hSceneObject, &vPos);
		   return vPos;
		}

		HOBJECT GetObject() { return m_hSceneObject; }

		CharacterClass GetCharacterClass()
		{
			return m_eCharacterClass;
		}

		CharacterDeath GetCharacterDeath()
		{
			return m_eCharacterDeath;
		}

		void Save(HMESSAGEWRITE hWrite)
		{
            if ( !hWrite || !g_pLTServer ) return;

            g_pLTServer->WriteToMessageDWord(hWrite, m_eCharacterDeath);
            g_pLTServer->WriteToMessageDWord(hWrite, m_eCharacterClass);
            g_pLTServer->WriteToLoadSaveMessageObject(hWrite, m_hSceneObject);
            g_pLTServer->WriteToMessageFloat(hWrite, m_fNoiseVolume);
            g_pLTServer->WriteToMessageFloat(hWrite, m_fNoiseTime);
            g_pLTServer->WriteToMessageFloat(hWrite, m_fLastPainTime);
            g_pLTServer->WriteToMessageFloat(hWrite, m_fLastPainVolume);
		}

		void Load(HMESSAGEREAD hRead)
		{
            if ( !hRead || !g_pLTServer ) return;

            m_eCharacterDeath = (CharacterDeath)g_pLTServer->ReadFromMessageDWord(hRead);
            m_eCharacterClass = (CharacterClass)g_pLTServer->ReadFromMessageDWord(hRead);
            g_pLTServer->ReadFromLoadSaveMessageObject(hRead, &m_hSceneObject);
            m_fNoiseVolume = g_pLTServer->ReadFromMessageFloat(hRead);
            m_fNoiseTime = g_pLTServer->ReadFromMessageFloat(hRead);
            m_fLastPainTime = g_pLTServer->ReadFromMessageFloat(hRead);
            m_fLastPainVolume = g_pLTServer->ReadFromMessageFloat(hRead);
		}

	protected : // Protected methods

		friend class Body;

		void Set(CCharacter *pCharacter, Body *pBody);

        void SetNoise(LTFLOAT fVolume, LTFLOAT fTime)
		{
			m_fNoiseVolume = fVolume;
			m_fNoiseTime = fTime;
		}

        void SetPain(LTFLOAT fVolume, LTFLOAT fTime)
		{
			m_fLastPainTime = fTime;
			m_fLastPainVolume = fVolume;
		}

	private : // Private member variables

		CharacterDeath	 m_eCharacterDeath;
		CharacterClass	 m_eCharacterClass;

		// When a body lingers

		HOBJECT		m_hSceneObject;

		// When there is no body

        LTFLOAT      m_fNoiseVolume;
        LTFLOAT      m_fNoiseTime;
        LTFLOAT      m_fLastPainVolume;
        LTFLOAT      m_fLastPainTime;
};

#include "Body.h"

inline void CDeathScene::Set(CCharacter *pCharacter, Body *pBody)
{
	_ASSERT(pCharacter && pBody);
	if ( !pCharacter || !pBody ) return;

	m_eCharacterDeath = pCharacter->GetDeathType();
	m_eCharacterClass = pCharacter->GetCharacterClass();

	m_hSceneObject = pBody->m_hObject;
}

#endif