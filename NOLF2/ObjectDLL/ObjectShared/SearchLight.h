// ----------------------------------------------------------------------- //
//
// MODULE  : SearchLight.h
//
// PURPOSE : An object which scans for the player and then sends a message
//
// CREATED : 3/29/99
//
// ----------------------------------------------------------------------- //

#ifndef __SEARCH_LIGHT_H__
#define __SEARCH_LIGHT_H__

#include "ltengineobjects.h"
#include "CharacterAlignment.h"
#include "Scanner.h"
#include "SFXFuncs.h"
#include "Timer.h"

LINKTO_MODULE( SearchLight );

class SearchLight : public CScanner
{
	public :

		SearchLight();
		virtual ~SearchLight();

		void SetTarget(HOBJECT hTargetObject) { Unlink(m_hTargetObject); m_hTargetObject = hTargetObject; Link(hTargetObject); }
		HOBJECT GetTarget() const { return m_hTargetObject; }

	protected :

		enum State
		{
			eStateTurningTo1,
			eStateTurningTo2,
			eStatePausingAt1,
			eStatePausingAt2,
			eStateDetected,
			eStateDestroyed
		};

        virtual uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
        virtual uint32 ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);
		virtual bool OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

	protected :

        LTBOOL  ReadProp(ObjectCreateStruct *pData);
		void	PostPropRead(ObjectCreateStruct* pData);
        LTBOOL  InitialUpdate();

		virtual void	HandleBrokenLink(HOBJECT hObject);

        LTBOOL  Update();
		void	UpdateRotation();

		void	StopSearching();

		virtual DetectState UpdateDetect();

		void	SetState(State eNewState);

        virtual void SetRotation(LTRotation& rRot) { g_pLTServer->SetObjectRotation(m_hObject, &rRot); }
        virtual LTRotation GetRotation() { LTRotation rRot; g_pLTServer->GetObjectRotation(m_hObject, &rRot); return rRot; }

        virtual void SetPosition(LTVector& vPos) { g_pLTServer->SetObjectPos(m_hObject, &vPos); }
        virtual LTVector GetPosition() { LTVector vPos; g_pLTServer->GetObjectPos(m_hObject, &vPos); return vPos; }

		virtual void Save(ILTMessage_Write *pMsg);
		virtual void Load(ILTMessage_Read *pMsg);

		void	FirstUpdate();

        LTVector     GetScanPosition() { return GetPosition(); }
        LTRotation   GetScanRotation() { return GetRotation(); }

        void Link(HOBJECT hObject) { if ( g_pLTServer && hObject ) g_pLTServer->CreateInterObjectLink(m_hObject, hObject); }
        void Unlink(HOBJECT hObject) { if ( g_pLTServer && hObject ) g_pLTServer->BreakInterObjectLink(m_hObject, hObject); }

	protected :

		State	m_eState;
		State	m_ePreviousState;

        LTFLOAT  m_fYaw;
        LTFLOAT  m_fYaw1;
        LTFLOAT  m_fYaw2;
        LTFLOAT  m_fYawSpeed;
        LTFLOAT  m_fYaw1PauseTime;
        LTFLOAT  m_fYaw2PauseTime;
        LTFLOAT  m_fYawPauseTimer;

        LTBOOL   m_bFirstUpdate;
        LTBOOL   m_bOn;

		HSTRING	m_hstrTargetName;
		LTObjRef m_hTargetObject;

		CTimer	m_SearchTimer;

		//** EVERYTHING BELOW HERE DOES NOT NEED SAVING

		// These are all saved by the engine in the special fx message...

        LTFLOAT  m_fBeamLength;
        LTFLOAT  m_fBeamRadius;
        LTFLOAT  m_fBeamAlpha;
        LTFLOAT  m_fBeamRotTime;
        LTFLOAT  m_fLightRadius;
        LTBOOL   m_bBeamAdditive;
        LTVector m_vLightColor;

		LENSFLARE	m_LensInfo;		// Lens flare info
};

class ControlledSearchLight : public SearchLight
{
	public :

		ControlledSearchLight();
		~ControlledSearchLight();

		void SetController(HOBJECT hController) { Unlink(m_hController); m_hController = hController; Link(m_hController); }
		HOBJECT GetController() { return m_hController; }

	protected :

		void HandleBrokenLink(HOBJECT hObject);

        void SetRotation(LTRotation& rRot);
        LTRotation GetRotation();

        void SetPosition(LTVector& vPos);
        LTVector GetPosition();

		void Save(ILTMessage_Write *pMsg);
		void Load(ILTMessage_Read *pMsg);

        void Link(HOBJECT hObject) { if ( g_pLTServer && hObject ) g_pLTServer->CreateInterObjectLink(m_hObject, hObject); }
        void Unlink(HOBJECT hObject) { if ( g_pLTServer && hObject ) g_pLTServer->BreakInterObjectLink(m_hObject, hObject); }

	protected :

		LTObjRef	m_hController;
};

#endif // __SEARCH_LIGHT_H__