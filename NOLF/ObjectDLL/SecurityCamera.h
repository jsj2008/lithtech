// ----------------------------------------------------------------------- //
//
// MODULE  : SecurityCamera.h
//
// PURPOSE : An object which scans for the player and then sends a message
//
// CREATED : 3/29/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SECURITYCAMERA_H__
#define __SECURITYCAMERA_H__

#include "ltengineobjects.h"
#include "CharacterAlignment.h"
#include "Scanner.h"
#include "Timer.h"

class SecurityCamera : public CScanner
{
	public :

		SecurityCamera();
		~SecurityCamera();

	protected :

		enum State
		{
			eStateTurningTo1,
			eStateTurningTo2,
			eStatePausingAt1,
			eStatePausingAt2,
			eStateDetected,
			eStateFocusing,
			eStateOff,
			eStateReset,
			eStateDisabled,
			eStateDestroyed
		};

        virtual uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
        virtual uint32 ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

	protected :

        LTBOOL  ReadProp(ObjectCreateStruct *pData);
		void	PostPropRead(ObjectCreateStruct* pData);
        LTBOOL  InitialUpdate();

        LTBOOL  Update();
		void	UpdateRotation();
		void	UpdateSounds(State eStatePrevious);

		virtual DetectState UpdateDetect();
        virtual LTRotation GetScanRotation();

        virtual LTFLOAT  GetFocusTime();

		void	StartLoopSound();
		void	StopLoopSound();
		void	StartFocusingSound();
		void	StopFocusingSound();
		void	PlayDetectedSound();
		void	KillAllSounds();

		void	SetState(State eNewState);
		void	SetupDisabledState();
		void	HandleGadgetMsg(ConParse & parse);

	protected :

		State		m_eState;
		State		m_ePreviousState;

        LTFLOAT		m_fYaw;
        LTFLOAT		m_fYaw1;
        LTFLOAT		m_fYaw2;
        LTFLOAT		m_fYawSpeed;
        LTFLOAT		m_fYaw1PauseTime;
        LTFLOAT		m_fYaw2PauseTime;
        LTFLOAT		m_fYawPauseTimer;

		LTVector	m_vPos;
		HSTRING		m_hstrFocusingSound;
		HSTRING		m_hstrLoopSound;
		HSTRING		m_hstrDetectSound;

        LTFLOAT		m_fSoundRadius;

        HLTSOUND	m_hFocusingSound;
        HLTSOUND	m_hLoopSound;

		HOBJECT		m_hDisablerModel;
		HOBJECT		m_hLight;

        LTBOOL		m_bDisabled;
		LTBOOL		m_bTripped;

		CTimer		m_LightTimer;

	private :

		void	Save(HMESSAGEWRITE hWrite);
		void	Load(HMESSAGEREAD hRead);
		void	CacheFiles();
		void	CreateLight();
		void	UpdateFlashingLight();
};

#endif // __SECURITYCAMERA_H__