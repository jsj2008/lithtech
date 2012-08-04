// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_TARGET_H__
#define __AI_TARGET_H__

class CAITarget : DEFINE_FACTORY_CLASS(CAITarget)
{
	DEFINE_FACTORY_METHODS(CAITarget);

	public : // Public methods

        LTBOOL IsVisibleCompletely() const { return IsVisibleFromEye() && IsVisibleFromWeapon(); }
        LTBOOL IsVisiblePartially() const { return IsVisibleFromEye() || IsVisibleFromWeapon(); }
        LTBOOL IsVisibleFromEye() const { return m_bVisibleFromEye; }
        LTBOOL IsVisibleFromWeapon() const { return m_bVisibleFromWeapon; }

        LTBOOL IsAttacking() const { return m_bAttacking; }

		void UpdateVisibility();

		HOBJECT GetObject() const { return m_hObject; }
        CCharacter* GetCharacter() const { return (CCharacter*)g_pLTServer->HandleToObject(m_hObject); }
        const LTVector& GetPosition() const { return m_vPosition; }
        const LTVector& GetShootPosition() const { return m_vShootPosition; }

	protected : // Protected methods

		friend class CAI;

		void Init(CAI* pAI) { m_pAI = pAI; }
		CAI* GetAI() { return m_pAI; }

        LTBOOL IsValid() { return m_bValid; }
        void SetValid(LTBOOL bValid) { m_bValid = bValid; }
        void SetVisibleFromEye(LTBOOL bVisibleFromEye) { m_bVisibleFromEye = bVisibleFromEye; }
        void SetVisibleFromWeapon(LTBOOL bVisibleFromWeapon) { m_bVisibleFromWeapon = bVisibleFromWeapon; }
		void SetObject(HOBJECT hObject) { m_hObject = hObject; }
        void SetPosition(const LTVector& vPosition) { m_vPosition = vPosition; }

        void SetAttacking(LTBOOL bAttacking) { m_bAttacking = bAttacking; }

        void UpdateShootPosition(const LTVector& vShootPosition, LTFLOAT fError, LTBOOL bNewError = LTTRUE);

		void Save(HMESSAGEWRITE hWrite);
		void Load(HMESSAGEREAD hRead);

	private : // Private member variables

		CAI*		m_pAI;
        LTBOOL		m_bValid;
        LTBOOL		m_bVisibleFromEye;
        LTBOOL		m_bVisibleFromWeapon;
		HOBJECT		m_hObject;
        LTVector	m_vPosition;
        LTVector	m_vShootPosition;
        LTVector	m_vNextShootPosition;
        LTBOOL		m_bAttacking;
		int32		m_nPhase;
		int32		m_nResetPhase;

		LTBOOL		m_bHack;
};

#endif