// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef _AI_NUDEGE_H_
#define _AI_NUDEGE_H_

class CAIHuman;

class CNudge
{
	public :

		enum Priority
		{
			ePriorityLow,
			ePriorityHigh,
		};

	public : // Public methods

		// Ctors/Dtors/etc

		CNudge(CAIHuman* pAI);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Updates

		void Update(LTBOOL bMoving);

		// Nudge

		LTBOOL IsNudging() const { return m_eState == eStateNudge; }
		const LTVector& GetNudge() const { return m_vNudge; }

		// Priority

		void SetPriority(Priority ePriority) { m_ePriority = ePriority; }
		Priority GetPriority() const { return m_ePriority; }

	protected :

		enum State
		{
			eStateNoNudge,
			eStateNudge,
		};

	protected : // Protected member variables

		CAIHuman*		m_pAI;

		State			m_eState;
		Priority		m_ePriority;

		LTVector		m_vNudge;
};

#endif
