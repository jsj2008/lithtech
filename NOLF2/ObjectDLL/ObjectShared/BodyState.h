// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __BODY_STATE_H__
#define __BODY_STATE_H__

#include "AIState.h"

class Body;


class CBodyState : public CAIClassAbstract
{
	public :

		DECLARE_AI_FACTORY_CLASS_ABSTRACT_SPECIFIC(State);

		CBodyState( );

		virtual void Init(Body* pBody);
		virtual void InitLoad(Body* pBody);

		virtual void Update() {}

		virtual void HandleTouch(HOBJECT hObject) {}

		virtual void Save(ILTMessage_Write *pMsg) {}
		virtual void Load(ILTMessage_Read *pMsg) {}

		virtual LTBOOL CanDeactivate() { return LTTRUE; }

		// Most of the time the ability to update dims is
		// also the same time we can deactivate.  This is because
		// we can't deactivate as long as we're moving our body around,
		// and we also don't want to update the dims as long as we're moving
		// our body around.
		virtual LTBOOL CanUpdateDims() { return CanDeactivate( ); }

	protected :

		Body*	m_pBody;
};

class CBodyStateNormal : public CBodyState
{
	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CBodyStateNormal, kState_BodyNormal);

		void Init(Body* pBody);
		void InitLoad(Body* pBody);

		void Update();

		LTBOOL CanDeactivate();

		// It's always ok to update dims for normal, because normal isn't
		// doing anything.
		virtual LTBOOL CanUpdateDims() { return true; }

};

class CBodyStateStairs : public CBodyState
{
	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CBodyStateStairs, kState_BodyStairs);

		CBodyStateStairs();

		void Init(Body* pBody);
		void InitLoad(Body* pBody);

		void Update();

		void HandleTouch(HOBJECT hObject);
		void Save(ILTMessage_Write *pMsg);
		void Load(ILTMessage_Read *pMsg);

		LTBOOL CanDeactivate() { return LTFALSE; }

	protected :

        LTVector    m_vDir;
		AIVolume*	m_pVolume;
        LTBOOL      m_bFell;
		HMODELANIM	m_hAniStart;
		HMODELANIM	m_hAniLoop;
		HMODELANIM	m_hAniStop;
};

class CBodyStateLedge : public CBodyState
{
	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CBodyStateLedge, kState_BodyLedge);

		CBodyStateLedge( );

		void Init(Body* pBody);
		void InitLoad(Body* pBody);

		void Update();

		void HandleTouch(HOBJECT hObject);

		void Save(ILTMessage_Write *pMsg);
		void Load(ILTMessage_Read *pMsg);

		LTBOOL CanDeactivate() { return LTFALSE; }

	protected :

		enum State
		{
			eLeaning,
			eFalling,
			eLanding,
		};

	protected :

		State		m_eState;
        LTFLOAT		m_fTimer;
        LTVector	m_vVelocity;
		AIVolume*	m_pVolume;
		HMODELANIM	m_hAniStart;
		HMODELANIM	m_hAniLoop;
		HMODELANIM	m_hAniStop;
};

class CBodyStateLadder : public CBodyState
{
	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CBodyStateLadder, kState_BodyLadder);

		CBodyStateLadder( );

		void Init(Body* pBody);
		void InitLoad(Body* pBody);

		void Update();

		void HandleTouch(HOBJECT hObject);

		void Save(ILTMessage_Write *pMsg);
		void Load(ILTMessage_Read *pMsg);

		LTBOOL CanDeactivate() { return LTFALSE; }

	protected :

		enum State
		{
			eLeaning,
			eFalling,
			eLanding,
		};

	protected :

		State		m_eState;
        LTFLOAT		m_fTimer;
		AIVolume*	m_pVolume;
		HMODELANIM	m_hAniStart;
		HMODELANIM	m_hAniLoop;
		HMODELANIM	m_hAniStop;
};

class CBodyStateUnderwater : public CBodyState
{
	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CBodyStateUnderwater, kState_BodyUnderwater);

		CBodyStateUnderwater( );

		void Init(Body* pBody);
		void InitLoad(Body* pBody);

		void Update();

		void HandleTouch(HOBJECT hObject);

		void Save(ILTMessage_Write *pMsg);
		void Load(ILTMessage_Read *pMsg);

		LTBOOL CanDeactivate() { return LTFALSE; }

	protected :

		HMODELANIM	m_hAniStart;
		HMODELANIM	m_hAniLoop;
		HMODELANIM	m_hAniStop;
        LTBOOL       m_bStop;
};

class CBodyStateLaser : public CBodyState
{
	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CBodyStateLaser, kState_BodyLaser);

		void Init(Body* pBody);
		void InitLoad(Body* pBody);

		void Update();

		void Save(ILTMessage_Write *pMsg);
		void Load(ILTMessage_Read *pMsg);

		LTBOOL CanDeactivate() { return LTFALSE; }

	protected :

		LTFLOAT		m_fRemoveTime;
};

class CBodyStateDecay : public CBodyState
{
	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CBodyStateDecay, kState_BodyDecay);

		void Init(Body* pBody);
		void InitLoad(Body* pBody);

		void Update();

		void Save(ILTMessage_Write *pMsg);
		void Load(ILTMessage_Read *pMsg);

		LTBOOL CanDeactivate() { return LTFALSE; }

	protected :

		LTFLOAT		m_fRemoveTime;
};

class CBodyStateFade : public CBodyState
{
	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CBodyStateFade, kState_BodyFade);

		CBodyStateFade();

		void Init(Body* pBody);
		void InitLoad(Body* pBody);

		void Update();

		void Save(ILTMessage_Write *pMsg);
		void Load(ILTMessage_Read *pMsg);

		LTBOOL CanDeactivate() { return LTFALSE; }

	protected :

		LTFLOAT		m_fRemoveTime;
};

class CBodyStateCrush : public CBodyState
{
	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CBodyStateCrush, kState_BodyCrush);

		void Init(Body* pBody);
		void InitLoad(Body* pBody);
};

class CBodyStateChair : public CBodyState
{
	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CBodyStateChair, kState_BodyChair);

		void Init(Body* pBody);
		void InitLoad(Body* pBody);
};

class CBodyStatePoison : public CBodyState
{
	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CBodyStatePoison, kState_BodyPoison);

		void Init(Body* pBody);
		void InitLoad(Body* pBody);
};

class CBodyStateAcid : public CBodyState
{
	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CBodyStateAcid, kState_BodyAcid);

		void Init(Body* pBody);
		void InitLoad(Body* pBody);
};

class CBodyStateArrow : public CBodyState
{
	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CBodyStateArrow, kState_BodyArrow);

		void Init(Body* pBody);
		void InitLoad(Body* pBody);
};

class CBodyStateExplode : public CBodyState
{
	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CBodyStateExplode, kState_BodyExplode);

		CBodyStateExplode( );

		void Init(Body* pBody);
		void InitLoad(Body* pBody);

		void Update();

		void HandleTouch(HOBJECT hObject);

		void Save(ILTMessage_Write *pMsg);
		void Load(ILTMessage_Read *pMsg);

		LTBOOL CanDeactivate() { return LTFALSE; }

	protected :

		void SetRestingState();

		LTBOOL		m_bLand;
		LTVector	m_vLandPos;

		HMODELANIM	m_hAniStart;
		HMODELANIM	m_hAniLoop;
		HMODELANIM	m_hAniStop;
};

class CBodyStateCarried : public CBodyState
{
	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CBodyStateCarried, kState_BodyCarried);

		void Init(Body* pBody);
		void InitLoad(Body* pBody);

		void Update();

		LTBOOL CanDeactivate() { return LTFALSE; }

	protected :

		HMODELANIM	m_hAniStart;
};

class CBodyStateDropped : public CBodyState
{
	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CBodyStateDropped, kState_BodyDropped);

		void Init(Body* pBody);
		void InitLoad(Body* pBody);

		void Update();

		LTBOOL CanDeactivate() { return LTFALSE; }

	protected :

		HMODELANIM	m_hAniStart;
};


#endif