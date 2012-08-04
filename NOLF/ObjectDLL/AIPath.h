// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_PATH_H__
#define __AI_PATH_H__

#include "AIVolume.h"
#include "AINode.h"

class CAI;

class CAIPathWaypoint
{
	public :

		enum Instruction
		{
			eInstructionInvalid,
			eInstructionMoveTo,
			eInstructionCrawlTo,
			eInstructionClimbTo,
			eInstructionJumpTo,
			eInstructionWaitForLift,
			eInstructionEnterLift,
			eInstructionRideLift,
			eInstructionExitLift,
			eInstructionOpenDoors,
			eInstructionWaitForDoors,
			eInstructionFaceDoor,
		};

	public :

		// Ctors/Dtors/etc

		CAIPathWaypoint();

		void Constructor();
		void Destructor();

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Handlers

		void HandleBrokenLink(HOBJECT hObject);

		// Simple accessors

		Instruction GetInstruction() const { return m_eInstruction; }
        const LTVector& GetArgumentVector1() const { return m_vVector1; }
		HOBJECT GetArgumentObject1() const { return m_hObject1; }
		HOBJECT GetArgumentObject2() const { return m_hObject2; }

	protected :

		friend class CAIPath;
		friend class CAIPathMgr;

		// Setup

		void SetAI(CAI* pAI) { m_pAI = pAI; }
		void SetInstruction(Instruction eInstruction) { m_eInstruction = eInstruction; }
        void SetArgumentVector1(const LTVector& vVector1) { m_vVector1 = vVector1; }
		void SetArgumentObject1(HOBJECT hObject1);
		void SetArgumentObject2(HOBJECT hObject2);

	protected :

		CAI*			m_pAI;
		Instruction		m_eInstruction;
        LTVector        m_vVector1;
		HOBJECT			m_hObject1;
		HOBJECT			m_hObject2;
};

class CAIPath : DEFINE_FACTORY_CLASS(CAIPath)
{
	DEFINE_FACTORY_METHODS(CAIPath);

	public :

		// Ctors/Dtors/etc

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		void Init(CAI* pAI);

		// Handlers

		void HandleBrokenLink(HOBJECT hObject);

		// Waypoint methods

		CAIPathWaypoint* GetWaypoint(int iWaypoint) { return &m_aWaypoints[iWaypoint]; }
		CAIPathWaypoint* GetCurrentWaypoint() { return &m_aWaypoints[m_iWaypoint]; }

        LTBOOL HasRemainingWaypoints() const { return m_iWaypoint < m_cWaypoints; }
		int GetCurrentWaypointIndex() const { return m_iWaypoint; }
		int GetNumWaypoints() const { return m_cWaypoints; }

		void IncrementWaypointIndex() { m_iWaypoint++; }

	protected :

		friend class CAIPathMgr;

        LTBOOL HasAI() { return !!m_pAI; }

		// Setup methods

		void AddWaypoint(const CAIPathWaypoint& waypt);
		void ClearWaypoints();

	protected :

		enum Constants
		{
			kMaxWaypoints = 128,
		};

	protected :

		CAI*				m_pAI;
		CAIPathWaypoint		m_aWaypoints[kMaxWaypoints];
		int					m_iWaypoint;
		int					m_cWaypoints;
};

#endif