// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __BODY_STATE_H__
#define __BODY_STATE_H__

class Body;

class CBodyState
{
	DEFINE_ABSTRACT_FACTORY_METHODS(CBodyState);

	public :

		virtual void Init(Body* pBody);
		virtual void InitLoad(Body* pBody);

		virtual void Update() {}

		virtual void HandleTouch(HOBJECT hObject) {}

		virtual void Save(HMESSAGEWRITE hWrite) {}
		virtual void Load(HMESSAGEREAD hRead) {}

		virtual LTBOOL CanDeactivate() { return LTTRUE; }

	protected :

		Body*	m_pBody;
};

class CBodyStateNormal : DEFINE_FACTORY_CLASS(CBodyStateNormal), public CBodyState
{
	DEFINE_FACTORY_METHODS(CBodyStateNormal);

	public :

		void Init(Body* pBody);
		void InitLoad(Body* pBody);

		void Update();

		LTBOOL CanDeactivate();
};

class CBodyStateStairs : DEFINE_FACTORY_CLASS(CBodyStateStairs), public CBodyState
{
	DEFINE_FACTORY_METHODS(CBodyStateStairs);

	public :

		void Init(Body* pBody);
		void InitLoad(Body* pBody);

		void Update();

		void HandleTouch(HOBJECT hObject);
		void Save(HMESSAGEWRITE hWrite);
		void Load(HMESSAGEREAD hRead);

		LTBOOL CanDeactivate() { return LTFALSE; }

	protected :

        LTVector    m_vDir;
		int			m_iVolume;
        LTBOOL      m_bFell;
		HMODELANIM	m_hAniStart;
		HMODELANIM	m_hAniLoop;
		HMODELANIM	m_hAniStop;
};

class CBodyStateLedge : DEFINE_FACTORY_CLASS(CBodyStateLedge), public CBodyState
{
	DEFINE_FACTORY_METHODS(CBodyStateLedge);

	public :

		void Init(Body* pBody);
		void InitLoad(Body* pBody);

		void Update();

		void HandleTouch(HOBJECT hObject);

		void Save(HMESSAGEWRITE hWrite);
		void Load(HMESSAGEREAD hRead);

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
        LTFLOAT      m_fTimer;
        LTVector     m_vVelocity;
		int			m_iVolume;
		HMODELANIM	m_hAniStart;
		HMODELANIM	m_hAniLoop;
		HMODELANIM	m_hAniStop;
};

class CBodyStateUnderwater : DEFINE_FACTORY_CLASS(CBodyStateUnderwater), public CBodyState
{
	DEFINE_FACTORY_METHODS(CBodyStateUnderwater);

	public :

		void Init(Body* pBody);
		void InitLoad(Body* pBody);

		void Update();

		void HandleTouch(HOBJECT hObject);

		void Save(HMESSAGEWRITE hWrite);
		void Load(HMESSAGEREAD hRead);

		LTBOOL CanDeactivate() { return LTFALSE; }

	protected :

		HMODELANIM	m_hAniStart;
		HMODELANIM	m_hAniLoop;
		HMODELANIM	m_hAniStop;
        LTBOOL       m_bStop;
};

class CBodyStateLaser : DEFINE_FACTORY_CLASS(CBodyStateLaser), public CBodyState
{
	DEFINE_FACTORY_METHODS(CBodyStateLaser);

	public :

		void Init(Body* pBody);
		void InitLoad(Body* pBody);

		void Update();

		void Save(HMESSAGEWRITE hWrite);
		void Load(HMESSAGEREAD hRead);

		LTBOOL CanDeactivate() { return LTFALSE; }

	protected :

		LTFLOAT		m_fRemoveTime;
};

class CBodyStateDecay : DEFINE_FACTORY_CLASS(CBodyStateDecay), public CBodyState
{
	DEFINE_FACTORY_METHODS(CBodyStateDecay);

	public :

		void Init(Body* pBody);
		void InitLoad(Body* pBody);

		void Update();

		void Save(HMESSAGEWRITE hWrite);
		void Load(HMESSAGEREAD hRead);

		LTBOOL CanDeactivate() { return LTFALSE; }

	protected :

		LTFLOAT		m_fRemoveTime;
};

class CBodyStateFade : DEFINE_FACTORY_CLASS(CBodyStateFade), public CBodyState
{
	DEFINE_FACTORY_METHODS(CBodyStateFade);

	public :

		void Init(Body* pBody);
		void InitLoad(Body* pBody);

		void Update();

		void Save(HMESSAGEWRITE hWrite);
		void Load(HMESSAGEREAD hRead);

		LTBOOL CanDeactivate() { return LTFALSE; }

	protected :

		LTFLOAT		m_fRemoveTime;
};

class CBodyStateCrush : DEFINE_FACTORY_CLASS(CBodyStateCrush), public CBodyState
{
	DEFINE_FACTORY_METHODS(CBodyStateCrush);

	public :

		void Init(Body* pBody);
		void InitLoad(Body* pBody);
};

class CBodyStateChair : DEFINE_FACTORY_CLASS(CBodyStateChair), public CBodyState
{
	DEFINE_FACTORY_METHODS(CBodyStateChair);

	public :

		void Init(Body* pBody);
		void InitLoad(Body* pBody);
};

class CBodyStatePoison : DEFINE_FACTORY_CLASS(CBodyStatePoison), public CBodyState
{
	DEFINE_FACTORY_METHODS(CBodyStatePoison);

	public :

		void Init(Body* pBody);
		void InitLoad(Body* pBody);
};

class CBodyStateAcid : DEFINE_FACTORY_CLASS(CBodyStateAcid), public CBodyState
{
	DEFINE_FACTORY_METHODS(CBodyStateAcid);

	public :

		void Init(Body* pBody);
		void InitLoad(Body* pBody);
};

class CBodyStateArrow : DEFINE_FACTORY_CLASS(CBodyStateArrow), public CBodyState
{
	DEFINE_FACTORY_METHODS(CBodyStateArrow);

	public :

		void Init(Body* pBody);
		void InitLoad(Body* pBody);
};

class CBodyStateExplode : DEFINE_FACTORY_CLASS(CBodyStateExplode), public CBodyState
{
	DEFINE_FACTORY_METHODS(CBodyStateExplode);

	public :

		void Init(Body* pBody);
		void InitLoad(Body* pBody);

		void Update();

		void HandleTouch(HOBJECT hObject);

		void Save(HMESSAGEWRITE hWrite);
		void Load(HMESSAGEREAD hRead);

		LTBOOL CanDeactivate() { return LTFALSE; }

	protected :

		LTBOOL		m_bLand;
		LTVector	m_vLandPos;

		HMODELANIM	m_hAniStart;
		HMODELANIM	m_hAniLoop;
		HMODELANIM	m_hAniStop;
};

#endif