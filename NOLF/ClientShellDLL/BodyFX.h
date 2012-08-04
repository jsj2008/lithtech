// ----------------------------------------------------------------------- //
//
// MODULE  : BodyFX.h
//
// PURPOSE : Body special fx class - Definition
//
// CREATED : 02.01.2000
//
// (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __BODY_FX_H__
#define __BODY_FX_H__

#include "SpecialFX.h"
#include "ModelButeMgr.h"
#include "SharedFXStructs.h"
#include "Animator.h"

class CBodyFX : public CSpecialFX
{
	public :

		// Ctors/Dtors/etc

		CBodyFX();
		~CBodyFX();

        LTBOOL Init(HLOCALOBJ hServObj, HMESSAGEREAD hMessage);
        LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);

        LTBOOL CreateObject(ILTClient* pClientDE);

		// Updates

        LTBOOL Update();

		// Handlers

        LTBOOL OnServerMessage(HMESSAGEREAD hMessage);
		void OnModelKey(HLOCALOBJ hObj, ArgList *pArgs);

		uint8 GetClientId() const { return m_bs.nClientId; }

		virtual uint32 GetSFXID() { return SFX_BODY_ID; }

	protected :

		// Updates

		void UpdateFade();

		void UpdateMarker();
		void CreateMarker(LTVector & vPos, LTBOOL bSame);
		void RemoveMarker();

	protected :

		BODYCREATESTRUCT	m_bs;					// Our createstruct

		LTFLOAT				m_fFaderTime;
		LTFLOAT				m_fFaderTimer;

		HOBJECT				m_hMarker;

		LTAnimTracker		m_TwitchTracker;
};

#endif