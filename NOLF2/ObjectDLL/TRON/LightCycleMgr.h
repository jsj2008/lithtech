/****************************************************************************
;
;	MODULE:			LightCycleMgr.h
;
;	PURPOSE:		Light Cycle Manager (SERVER) for TRON
;
;	HISTORY:		4/19/2002 [kml] This file was created
;
;	COMMENT:		Copyright (c) 2002, Monolith Productions, Inc.
;
****************************************************************************/

#ifndef _LIGHTCYCLEMGR_H_
#define _LIGHTCYCLEMGR_H_

// stl vector
#include <vector>

// Defines
#define CURRENT_TRAIL_ID -1

struct LIGHT_CYCLE_TRAIL_POINT
{
	LTVector vPoint;

	// We're defining accessors for this just in case the underlying datatype
	// or architecture changes... That way, the calling code won't have to change
	float GetX() { return vPoint.x; }
	float GetY() { return vPoint.y; }
	float GetZ() { return vPoint.z; }
	void SetX(float x) { vPoint.x = x; }
	void SetY(float y) { vPoint.y = y; }
	void SetZ(float z) { vPoint.z = z; }
};

struct LIGHT_CYCLE_TRAIL
{
	// Collection of wall points
	std::vector<LIGHT_CYCLE_TRAIL_POINT> collTrailPoints;
	
	// Unique ID
	uint16 wID;
};

struct LIGHT_CYCLIST
{
	LIGHT_CYCLIST()
	{
		fStartDerezTime = 0.0f;
		bUpdating = true;
		pCurTrail = NULL; 
		wNextTrailID = 0;
		hObject = NULL;
		vForward.Init(0,0,0);
	}
	~LIGHT_CYCLIST();

	LIGHT_CYCLE_TRAIL* FindTrail(uint16 wTrailID);

	// Collection of trails (normally just 1 except in special cases)
	std::vector<LIGHT_CYCLE_TRAIL*> collTrails;

	// If we die, here's the time we start
	float fStartDerezTime;

	// Are we updating this cyclist?
	bool bUpdating;

	// Which trail we're currently "making"
	LIGHT_CYCLE_TRAIL* pCurTrail;

	// Next trail ID
	uint16 wNextTrailID;

	// The cycle's current forward vector
	LTVector vForward;

	// Object associated with this cyclist
	LTObjRef hObject;
};

struct LightCycleCollisionInfo
{
	LIGHT_CYCLIST* pCyclist;
	LTVector vCollisionPoint;
};

class CLightCycleMgr
{
	public:

		CLightCycleMgr();
		~CLightCycleMgr() { Term(); }

		bool Init();
		void Term();

		bool AddCyclist(HOBJECT hObj, bool bUpdate = true);
		bool DerezCyclist(HOBJECT hObj);
		bool RemoveCyclist(HOBJECT hObj);
		void Update();
		bool BeginLightCycleTrail(LIGHT_CYCLIST* pCyclist, LTVector& vPos);
		bool UpdateLightCycleTrail(LIGHT_CYCLIST* pCyclist, LTVector& vPos, int nTrailID = CURRENT_TRAIL_ID);
		uint32 AddLightCycleTrailPoint(LIGHT_CYCLIST* pCyclist, LIGHT_CYCLE_TRAIL* pTrail, LTVector& vPos);

		void SetUpdating(bool bUpdating) { m_bUpdating = bUpdating; }
	
	private:

		LIGHT_CYCLIST* FindCyclist(HOBJECT hObj);

		void CreateTrailPointFromVector(LIGHT_CYCLE_TRAIL_POINT &pt, LTVector& vPos);

		// Collision check
		bool CheckForCollision(LIGHT_CYCLIST* pCyclist, LightCycleCollisionInfo &info);

		// Collision handler
		void HandleCollision(LIGHT_CYCLIST* pCyclist, LightCycleCollisionInfo &info);

		// Collection of light cyclist data
		std::vector<LIGHT_CYCLIST*> m_collCyclists;

		// Are we updating or not
		bool m_bUpdating;
};

extern CLightCycleMgr* g_pLightCycleMgr;

#endif // _LIGHTCYCLEMGR_H_
