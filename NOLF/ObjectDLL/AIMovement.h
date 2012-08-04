// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_MOVEMENT_H__
#define __AI_MOVEMENT_H__

class CAIHuman;

// ----------------------------------------------------------------------- //
//
// CLASS   : CAIMovementHuman
//
// PURPOSE : AI Movement manager
//
// ----------------------------------------------------------------------- //

class CAIMovementHuman
{
	public : // Public methods

		// Ctors/Dtors/etc

		void Constructor();
		void Destructor();

        LTBOOL Init(CAIHuman* pAIHuman);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Methods

        void Set(const LTVector& vDest);
		void Clear() { m_eState = eStateUnset; }

        LTBOOL Update();
		void UpdateAnimation();

		// Type

		void SetMovement(CAnimationProp aniMovement) { m_aniMovement = aniMovement; }
		const CAnimationProp& GetMovement() const { return m_aniMovement; }

		// Underwater?

        void SetUnderwater(LTBOOL bUnderwater) { m_bUnderwater = bUnderwater; }

		// Simple accessors

        LTBOOL IsUnset() const { return m_eState == eStateUnset; }
        LTBOOL IsSet() const { return m_eState == eStateSet; }
        LTBOOL IsDone() const { return m_eState == eStateDone; }

	private : // Private methods

		// Simple accessors

		CAIHuman* GetAI() { return m_pAIHuman; }

	private :

		enum State
		{
			eStateUnset,
			eStateSet,
			eStateDone,
		};

	private : // Private member variables

		CAIHuman*		m_pAIHuman;
		CAnimationProp	m_aniMovement;
		State			m_eState;
        LTVector		m_vDest;
        LTBOOL			m_bUnderwater;
};

class AI_Dog;

// ----------------------------------------------------------------------- //
//
// CLASS   : CAIMovementDog
//
// PURPOSE : AI Movement manager
//
// ----------------------------------------------------------------------- //

class CAIMovementDog
{
	public : // Public methods

		// Ctors/Dtors/etc

		void Constructor();
		void Destructor();

        LTBOOL Init(AI_Dog* pAIDog);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Methods

        void Set(const LTVector& vDest);
		void Clear();

        LTBOOL Update();

		// Simple accessors

        LTBOOL IsUnset() const { return m_eState == eStateUnset; }
        LTBOOL IsSet() const { return m_eState == eStateSet; }
        LTBOOL IsDone() const { return m_eState == eStateDone; }

	private : // Private methods

		// Simple accessors

		AI_Dog* GetAI() { return m_pAIDog; }

	private :

		enum State
		{
			eStateUnset,
			eStateSet,
			eStateDone,
		};

	private : // Private member variables

		AI_Dog*		m_pAIDog;
		State		m_eState;
        LTVector     m_vDest;
        LTVector     m_vDestDir;
};

class AI_Poodle;

// ----------------------------------------------------------------------- //
//
// CLASS   : CAIMovementPoodle
//
// PURPOSE : AI Movement manager
//
// ----------------------------------------------------------------------- //

class CAIMovementPoodle
{
	public : // Public methods

		// Ctors/Dtors/etc

		void Constructor();
		void Destructor();

        LTBOOL Init(AI_Poodle* pAIPoodle);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Methods

        void Set(const LTVector& vDest);
		void Clear() { m_eState = eStateUnset; }

        LTBOOL Update();

		// Simple accessors

        LTBOOL IsUnset() const { return m_eState == eStateUnset; }
        LTBOOL IsSet() const { return m_eState == eStateSet; }
        LTBOOL IsDone() const { return m_eState == eStateDone; }

	private : // Private methods

		// Simple accessors

		AI_Poodle* GetAI() { return m_pAIPoodle; }

	private :

		enum State
		{
			eStateUnset,
			eStateSet,
			eStateDone,
		};

	private : // Private member variables

		AI_Poodle*	m_pAIPoodle;
		State		m_eState;
        LTVector     m_vDest;
};

class AI_Shark;

// ----------------------------------------------------------------------- //
//
// CLASS   : CAIMovementShark
//
// PURPOSE : AI Movement manager
//
// ----------------------------------------------------------------------- //

class CAIMovementShark
{
	public : // Public methods

		// Ctors/Dtors/etc

		void Constructor();
		void Destructor();

        LTBOOL Init(AI_Shark* pAIShark);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Methods

        void Set(const LTVector& vDest);
		void Clear();

        LTBOOL Update();

		// Simple accessors

        LTBOOL IsUnset() const { return m_eState == eStateUnset; }
        LTBOOL IsSet() const { return m_eState == eStateSet; }
        LTBOOL IsDone() const { return m_eState == eStateDone; }

	private : // Private methods

		// Simple accessors

		AI_Shark* GetAI() { return m_pAIShark; }

	private :

		enum State
		{
			eStateUnset,
			eStateSet,
			eStateDone,
		};

	private : // Private member variables

		AI_Shark*	m_pAIShark;
		State		m_eState;
        LTVector     m_vDest;
        LTVector     m_vDestDir;
};

class AI_Helicopter;

// ----------------------------------------------------------------------- //
//
// CLASS   : CAIMovementHelicopter
//
// PURPOSE : AI Movement manager
//
// ----------------------------------------------------------------------- //

class CAIMovementHelicopter
{
	public : // Public methods

		// Ctors/Dtors/etc

		void Constructor();
		void Destructor();

        LTBOOL Init(AI_Helicopter* pAIHelicopter);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Methods

        void Set(const LTVector& vDest);
		void Clear();

        LTBOOL Update();

		// Simple accessors

        LTBOOL IsUnset() const { return m_eState == eStateUnset; }
        LTBOOL IsSet() const { return m_eState == eStateSet; }
        LTBOOL IsDone() const { return m_eState == eStateDone; }

	private : // Private methods

		// Simple accessors

		AI_Helicopter* GetAI() { return m_pAIHelicopter; }

	private :

		enum State
		{
			eStateUnset,
			eStateSet,
			eStateDone,
		};

	private : // Private member variables

		AI_Helicopter*	m_pAIHelicopter;
		State		m_eState;
        LTVector     m_vDest;
        LTVector     m_vDestDir;
};


/* 
** H A C K
**
*/

enum MovementData
{
	mdRifle		 = 0,
	mdPistol	 = 1,
	mdCorner	 = 2,
	mdShuffle    = 3,
	mdRoll       = 4,
	mdLeft       = 5,
	mdRight      = 6,
};

LTFLOAT GetMovementData(MovementData mdWeapon, MovementData mdAction, MovementData mdDirection, float fTimePrev, float fTime);

#endif