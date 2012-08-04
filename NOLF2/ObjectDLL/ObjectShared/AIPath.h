// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_PATH_H__
#define __AI_PATH_H__

#include "AIVolume.h"
#include "AINode.h"
#include "AIVolumeNeighbor.h"
#include "AIClassFactory.h"

// Vectors.

typedef std::vector<LTVector> POINT_LIST;

// Classes.

class CAI;

class CAIPathWaypoint : public CAIClassAbstract, public ILTObjRefReceiver
{
	public :

		enum Instruction
		{
			eInstructionInvalid,
			eInstructionLockedMoveTo,
			eInstructionMoveTo,
			eInstructionCrawlTo,
			eInstructionClimbUpTo,
			eInstructionClimbDownTo,
			eInstructionFaceLadder,
			eInstructionGetOnLadder,
			eInstructionGetOffLadder,
			eInstructionJumpUpTo,
			eInstructionJumpDownTo,
			eInstructionJumpOver,
			eInstructionFaceJumpLand,
			eInstructionWaitForLift,
			eInstructionEnterLift,
			eInstructionRideLift,
			eInstructionExitLift,
			eInstructionOpenDoors,
			eInstructionWaitForDoors,
			eInstructionFaceDoor,
			eInstructionReleaseGate,
			eInstructionMoveToTeleport,
			eInstructionMoveFromTeleport,
		};

	public :

		DECLARE_AI_FACTORY_CLASS(CAIPathWaypoint);

		CAIPathWaypoint( );
		~CAIPathWaypoint( );

		// Ctors/Dtors/etc

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Methods

		void ReleaseGate();

		// Simple accessors

		Instruction GetInstruction() const { return m_eInstruction; }
        const LTVector& GetArgumentVector1() const { return m_vVector1; }
		ILTBaseClass* GetArgumentObject1() const { return m_pObject1; }
		ILTBaseClass* GetArgumentObject2() const { return m_pObject2; }
		HOBJECT GetArgumentHObject1() const { return m_hObject1; }
		HOBJECT GetArgumentHObject2() const { return m_hObject2; }
		VolumeGate GetArgumentVolumeGate() const { return m_eVolumeGate; }
		uint32 GetWaypointID() const { return m_nWaypointID; }
		uint32 GetControlPointIndex() const { return m_iControlPoint; }
		LTBOOL GetCalculateCurve() const { return m_bCalculateCurve; }
		EnumAnimProp GetAnimProp() { return m_eProp; }

	protected :

		friend class CAIPath;
		friend class CAIPathMgr;

		// Setup

		void SetAI(CAI* pAI) { m_pAI = pAI; }
		void SetInstruction(Instruction eInstruction) { m_eInstruction = eInstruction; }
        void SetArgumentVector1(const LTVector& vVector1) { m_vVector1 = vVector1; }
		void SetArgumentObject1(ILTBaseClass* pObject1);
		void SetArgumentObject2(ILTBaseClass* pObject2);
		void SetArgumentVolumeGate(VolumeGate eVolumeGate) { m_eVolumeGate = eVolumeGate; }
		void SetWaypointID(uint32 id) { m_nWaypointID = id; }
		void SetControlPointIndex(uint32 i) { m_iControlPoint = i; }
		void SetCalculateCurve(LTBOOL b) { m_bCalculateCurve = b; }
		void SetAnimProp(EnumAnimProp eProp) { m_eProp = eProp; }

		void SetWaypoint(CAI* pAI, Instruction eInstruction, const LTVector& vVector1);
		void SetControlPoint(CAI* pAI, Instruction eInstruction, const LTVector& vVector1, uint32 iControlPoint);
		void SetReleaseGatePoint(CAI* pAI, VolumeGate eVolumeGate, ILTBaseClass* pObject1, ILTBaseClass* pObject2);

	private :
		// ILTObjRefReceiver implementation for clearing object pointers if doors get destroyed
		virtual void OnLinkBroken(LTObjRefNotifier* pRef, HOBJECT hObj);

	protected :

		CAI*				m_pAI;
		Instruction			m_eInstruction;
        LTVector			m_vVector1;
		LTObjRefNotifier	m_hObject1;
		ILTBaseClass*		m_pObject1;
		LTObjRefNotifier	m_hObject2;
		ILTBaseClass*		m_pObject2;
		VolumeGate			m_eVolumeGate;
		uint32				m_nWaypointID;
		uint32				m_iControlPoint;
		LTBOOL				m_bCalculateCurve;
		EnumAnimProp		m_eProp;
};


typedef std::vector<CAIPathWaypoint*> AI_WAYPOINT_LIST;

class CAIPath : public CAIClassAbstract
{
	public :

		DECLARE_AI_FACTORY_CLASS(CAIPath);

		CAIPath( );
		~CAIPath( );

		// Ctors/Dtors/etc

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		void Init(CAI* pAI);

		// Waypoint methods

		CAIPathWaypoint* GetWaypoint(int iWaypoint) { return ( iWaypoint < (int)m_lstWaypoints.size() ? m_lstWaypoints[iWaypoint] : LTNULL ); }
		CAIPathWaypoint* GetCurrentWaypoint() { return GetWaypoint( m_iWaypoint ); }
		CAIPathWaypoint* GetNextWaypoint() { return ( HasRemainingWaypoints() ? GetWaypoint( m_iWaypoint + 1 ) : LTNULL ); }

        LTBOOL HasRemainingWaypoints() const { return ( m_iWaypoint < (int)m_lstWaypoints.size() ); }
		int GetCurrentWaypointIndex() const { return m_iWaypoint; }
		int GetNumWaypoints() const { return (int)m_lstWaypoints.size(); }
		AI_WAYPOINT_LIST::iterator Begin() { return m_lstWaypoints.begin(); }
		AI_WAYPOINT_LIST::iterator End() { return m_lstWaypoints.end(); }

		void IncrementWaypointIndex() { m_iWaypoint++; }

		// Path Access

		void GetInitialDir(LTVector* pvDir);
		void GetFinalDir(LTVector* pvDir);
		AIVolume* GetNextVolume(AIVolume* pVolume, AIVolume::EnumVolumeType eVolumeType);
		AIVolume* GetLastVolume(uint32 nOffsetFromLast);

		void Print();

	protected :

		friend class CAIPathMgr;

        LTBOOL HasAI() { return !!m_pAI; }

		// Setup methods

		void AddWaypoint(CAIPathWaypoint* pWaypt);
		AI_WAYPOINT_LIST::iterator RemoveWaypoint(AI_WAYPOINT_LIST::iterator it);
		AI_WAYPOINT_LIST::iterator InsertWaypoint(AI_WAYPOINT_LIST::iterator it, CAIPathWaypoint* pWaypt);
		void ClearWaypoints();

	protected :

		CAI*				m_pAI;
		AI_WAYPOINT_LIST	m_lstWaypoints;
		int					m_iWaypoint;
};

#endif
