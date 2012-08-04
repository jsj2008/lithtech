// ----------------------------------------------------------------------- //
//
// MODULE  : Breakable.h
//
// PURPOSE : A Breakable object
//
// CREATED : 1/14/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __BREAKABLE_H__
#define __BREAKABLE_H__

#include "Door.h"
#include "Timer.h"
#include "Steam.h"

class Breakable : public Door
{
	public:

		Breakable();
		virtual ~Breakable();

		void	Break(HOBJECT hObj) { TouchNotify(hObj); }

	protected:

        uint32  EngineMessageFn(uint32 messageID, void *pData, float fData);
		virtual void TouchNotify(HOBJECT hObj);
		virtual void TriggerMsg(HOBJECT hSender, const char* szMsg);

	private :

        LTFLOAT     m_fBreakTime;
        LTFLOAT     m_fBreakSoundRadius;
        LTFLOAT     m_fImpactSoundRadius;
        LTFLOAT     m_fRotVel;
		HSTRING		m_hstrBreakSound;
		HSTRING		m_hstrImpactSound;
        LTVector    m_vStartingPitchYawRoll;
        LTVector    m_vPitchYawRoll;
        LTVector    m_vTotalDelta;
        LTVector    m_vDelta;
        LTVector    m_vSign;
        LTVector    m_vFinalPos;
        LTVector    m_vShakeAmount;
        LTVector    m_vAdjust;
		LTVector	m_vVel;

        LTBOOL      m_bFalling;
        LTBOOL      m_bStarted;
        LTBOOL      m_bDestroyOnImpact;
        LTBOOL      m_bDestroyAfterBreak;
        LTBOOL      m_bCrushObjects;
        LTBOOL      m_bTouchActivate;

		CTimer		m_BreakTimer;

		HOBJECT		m_hBreakObj;

        LTBOOL   IsStandingOnMe(HOBJECT hObj);

		void	Update();
		void	CacheFiles();
		void	ReadProp(ObjectCreateStruct* pStruct);

        void    StartBreak(HOBJECT hObj=LTNULL);
        LTBOOL  StopBreak();
		void	UpdateBreaking();
		void	UpdateFalling();
		void	Destroy();
		void	CrushObject(HOBJECT hObj);

		void	CreateBreakFX();
		void	CreateImpactFX();

		void	Save(HMESSAGEWRITE hWrite);
		void	Load(HMESSAGEREAD hWrite);
};

#endif // __BREAKABLE_H__