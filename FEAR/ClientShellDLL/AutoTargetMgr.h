// ----------------------------------------------------------------------- //
//
// MODULE  : AutoTargetMgr.cpp
//
// PURPOSE : AutoTargetMgr - handle auto targeting for vehicles and easy play
//
// CREATED : 2/28/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __AUTO_TARGET_MGR_H__
#define __AUTO_TARGET_MGR_H__

#include "ltbasetypes.h"
#include "ClientServerShared.h"

// number of character nodes to check for autotargeting
#define MAX_AUTOTARGET_CHARACTERS	(6)
#define	MAX_MODELBMGR_AUTOTARGET_NODES (8)
// max number of potential nodes to check while autotargeting (6 nodes per character * 6 characters) + aim magnets
#define MAX_AUTOTARGET_NODES		((MAX_MODELBMGR_AUTOTARGET_NODES * MAX_AUTOTARGET_CHARACTERS) + MAX_AIM_MAGNETS)

struct AutoTargetNode
{
	LTVector	vPos;
	HOBJECT		hChar;
};

class CAutoTargetMgr
{
protected:

	// Not allowed to create directly.  Use Instance().
	CAutoTargetMgr();
	
	// Copy ctor and assignment operator not implemented and should never be used.
	CAutoTargetMgr( CAutoTargetMgr const &other );
	CAutoTargetMgr& operator=( CAutoTargetMgr const &other );
	
public:

	// Call this to get the singleton instance of the auto-target mgr.
	static CAutoTargetMgr& Instance();

	void	Update();

	bool	IsLockedOn() const {return m_bLockOn;}

	//returns a normalized vector representing our current aim
	LTVector GetTargetVector() const {return m_vCurTarget;}

	//returns a point in screen space where the crosshair should be drawn
	LTVector GetCrosshairPos() const;


	~CAutoTargetMgr();

	

private:
	bool IsPointInCone(const LTVector &vTargetPos);
	void GenerateCharArray();
	void GenerateNodeArray();
	void AddMagnets();
	bool FindNode();
	void InterpolateAim();

	//these members are static so they can be referenced by the static sort function
	static LTVector m_vFirePos;
	static LTVector m_vForward;

	static int CompareTargetNodes(const void* lhs, const void* rhs);

	bool	m_bLockOn;

	LTVector m_vTarget; //a normalized vector to our selected target
	LTVector m_vCurTarget; //a normalized vector to where we're currently aiming
						   //this will interpolate towards our target

	float m_fAngle;
	float m_fRangeSqr;

	typedef std::vector<CCharacterFX*, LTAllocator<CCharacterFX*, LT_MEM_TYPE_CLIENTSHELL> > CharFXArray;
	CharFXArray m_Targets;

	AutoTargetNode m_NodeArray[MAX_AUTOTARGET_NODES];
	uint8 m_nNodeCount;


};

#endif // __AUTO_TARGET_MGR_H__