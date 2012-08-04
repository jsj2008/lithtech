// ----------------------------------------------------------------------- //
//
// MODULE  : Scanner.h
//
// PURPOSE : An object which scans for the player and then sends a message
//			 (based on old SecurityCamera class)
//
// CREATED : 6/7/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SCANNER_H__
#define __SCANNER_H__

#include "ltengineobjects.h"
#include "CharacterAlignment.h"
#include "Prop.h"
#include "Timer.h"

class CCharacter;

class CScanner : public Prop
{
	public :

		CScanner();
		~CScanner();

        LTBOOL CanSeeObject(ObjectFilterFn fn, HOBJECT hObject);
        LTBOOL CanSeePos(ObjectFilterFn fn, const LTVector& vPos);

		CharacterClass GetCharacterClass() { return BAD; }

        static LTBOOL DefaultFilterFn(HOBJECT hObj, void *pUserData);
        static LTBOOL BodyFilterFn(HOBJECT hObj, void *pUserData);

		enum DetectState { DS_CLEAR, DS_FOCUSING, DS_DETECTED };

	protected :

        virtual uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);

		virtual DetectState	UpdateDetect();
		virtual void		SetLastDetectedEnemy(HOBJECT hObj);

        virtual LTVector GetScanPosition() { LTVector vPos; g_pLTServer->GetObjectPos(m_hObject, &vPos); return vPos; }
        virtual LTRotation   GetScanRotation() { LTRotation rRot; g_pLTServer->GetObjectRotation(m_hObject, &rRot); return rRot; }

        void    SetLastDetectedDeathPos(LTVector vPos) { m_vLastDetectedDeathPos = vPos; }
		void	SetDestroyedModel();

        virtual LTFLOAT  GetFocusTime() { return 0.0f; }

	protected :

        LTFLOAT  m_fFOV;
        LTFLOAT  m_fVisualRange;

		uint8	 m_nPlayerTeamFilter;

        LTVector m_vInitialPitchYawRoll;

		HSTRING	m_hstrDestroyedFilename;
		HSTRING	m_hstrDestroyedSkin;

		HSTRING	m_hstrSpotMessage;
		HSTRING m_hstrSpotTarget;

		HOBJECT	m_hLastDetectedEnemy;
        LTVector m_vLastDetectedDeathPos;

		CTimer	m_FocusTimer;

        LTBOOL   m_bCanProcessDetection;

        // ** EVERYTHING BELOW HERE DOES NOT NEED SAVING

	private :

        LTBOOL   ReadProp(ObjectCreateStruct *pInfo);
		void	Save(HMESSAGEWRITE hWrite);
		void	Load(HMESSAGEREAD hRead);
		void	CacheFiles();
};

#endif // __SCANNER_H__