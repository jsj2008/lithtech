/****************************************************************************
;
;	MODULE:			LightCycleMgr.cpp
;
;	PURPOSE:		Light Cycle Manager (SERVER) for TRON
;
;	HISTORY:		4/19/2002 [kml] This file was created
;
;	COMMENT:		Copyright (c) 2002, Monolith Productions, Inc.
;
****************************************************************************/

#include "stdafx.h"
#include "LightCycleMgr.h"
#include "MsgIDs.h"

CLightCycleMgr* g_pLightCycleMgr = NULL;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LIGHT_CYCLIST::~LIGHT_CYCLIST
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //
LIGHT_CYCLIST::~LIGHT_CYCLIST()
{
	std::vector<LIGHT_CYCLE_TRAIL*>::iterator iter;
	for(iter=collTrails.begin();iter!=collTrails.end();iter++)
	{
		debug_delete(*iter);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LIGHT_CYCLIST::FindTrail
//
//	PURPOSE:	Looks for a trail in the list by unique ID (word)
//
// ----------------------------------------------------------------------- //
LIGHT_CYCLE_TRAIL* LIGHT_CYCLIST::FindTrail(uint16 wTrailID)
{
	std::vector<LIGHT_CYCLE_TRAIL*>::iterator iter;
	for(iter=collTrails.begin();iter!=collTrails.end();iter++)
	{
		if((*iter)->wID == wTrailID)
			return(*iter);
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::CLightCycleMgr
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
CLightCycleMgr::CLightCycleMgr()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::Init
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //
bool CLightCycleMgr::Init()
{
	g_pLightCycleMgr = this;
	m_bUpdating = true;
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::Term
//
//	PURPOSE:	Cleanup
//
// ----------------------------------------------------------------------- //
void CLightCycleMgr::Term()
{
	std::vector<LIGHT_CYCLIST*>::iterator iter;
	for(iter=m_collCyclists.begin();iter!=m_collCyclists.end();iter++)
	{
		debug_delete(*iter);
	}

	g_pLightCycleMgr = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::AddCyclist
//
//	PURPOSE:	Creats a new cyclist and adds it to the list
//
// ----------------------------------------------------------------------- //
bool CLightCycleMgr::AddCyclist(HOBJECT hObj, bool bUpdate)
{
	// Allocate
	LIGHT_CYCLIST* pCyclist = debug_new(LIGHT_CYCLIST);
	pCyclist->bUpdating = bUpdate;
	pCyclist->hObject = hObj;

	// Add to our list
	m_collCyclists.push_back(pCyclist);

	// Tell the client
	CAutoMessage cMsg;
	cMsg.WriteByte(LCI_ADD_CYCLIST);
	cMsg.WriteObject(hObj);
	cMsg.WriteByte((uint8)bUpdate);
	g_pLTServer->SendToClient(cMsg, MID_LIGHT_CYCLE_INFO, NULL, MESSAGE_GUARANTEED);
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::DerezCyclist
//
//	PURPOSE:	Starts a light cycle derez
//
// ----------------------------------------------------------------------- //
bool CLightCycleMgr::DerezCyclist(HOBJECT hObj)
{
	// Sanity check
	LIGHT_CYCLIST* pCyclist = FindCyclist(hObj);
	if(!pCyclist)
		return false;

	// Maybe we're already derezzing
	if(pCyclist->fStartDerezTime != 0.0f)
	{
		ASSERT(FALSE);
		return true;
	}

	// Start the derez timer
	pCyclist->fStartDerezTime = g_pLTServer->GetTime();

	CAutoMessage cMsg;
	cMsg.WriteByte(LCI_DEREZ_CYCLIST);
	cMsg.WriteObject(hObj);
	g_pLTServer->SendToClient(cMsg, MID_LIGHT_CYCLE_INFO, NULL, MESSAGE_GUARANTEED);
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::RemoveCyclist
//
//	PURPOSE:	Completely removes a light cycle from the manager
//
// ----------------------------------------------------------------------- //
bool CLightCycleMgr::RemoveCyclist(HOBJECT hObj)
{
	// Since we're removing from the list, we need to walk it manually
	std::vector<LIGHT_CYCLIST*>::iterator iter;
	for(iter=m_collCyclists.begin();iter!=m_collCyclists.end();iter++)
	{
		if((*iter)->hObject == hObj)
		{
			// We found him. Killlllll! KIIIILLLLLLLLL!
			LIGHT_CYCLIST* pCyclist = (*iter);
			debug_delete(pCyclist);

			// Remove from list
			m_collCyclists.erase(iter);

			// Notify the client
			CAutoMessage cMsg;
			cMsg.WriteByte(LCI_REMOVE_CYCLIST);
			cMsg.WriteObject(hObj);
			g_pLTServer->SendToClient(cMsg, MID_LIGHT_CYCLE_INFO, NULL, MESSAGE_GUARANTEED);

			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::FindCyclist
//
//	PURPOSE:	Looks for a cyclist in the list by unique ID (dword)
//
// ----------------------------------------------------------------------- //
LIGHT_CYCLIST* CLightCycleMgr::FindCyclist(HOBJECT hObj)
{
	std::vector<LIGHT_CYCLIST*>::iterator iter;
	for(iter=m_collCyclists.begin();iter!=m_collCyclists.end();iter++)
	{
		if((*iter)->hObject == hObj)
			return(*iter);
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::BeginLightCycleTrail
//
//	PURPOSE:	Starts a new light cycle trail
//
// ----------------------------------------------------------------------- //
bool CLightCycleMgr::BeginLightCycleTrail(LIGHT_CYCLIST* pCyclist, LTVector& vPos)
{
	ASSERT(pCyclist);
	if(!pCyclist)
		return false;

	// Create a new trail
	LIGHT_CYCLE_TRAIL* pTrail = debug_new(LIGHT_CYCLE_TRAIL);

	// Set the unique Trail ID and increment it
	pTrail->wID = pCyclist->wNextTrailID++;

	// Add this trail to the light cycle's list
	pCyclist->collTrails.push_back(pTrail);

	// Set our current trail
	pCyclist->pCurTrail = pTrail;

	// Add the point
	if(AddLightCycleTrailPoint(pCyclist, pTrail, vPos) > 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::UpdateLightCycleTrail
//
//	PURPOSE:	Updates a ligt cyclist's trail
//
// ----------------------------------------------------------------------- //
bool CLightCycleMgr::UpdateLightCycleTrail(LIGHT_CYCLIST* pCyclist, LTVector& vPos, int nTrailID)
{
	ASSERT(pCyclist);
	if(!pCyclist)
		return false;

	LIGHT_CYCLE_TRAIL_POINT newPoint;
	LIGHT_CYCLE_TRAIL_POINT *pOldPoint;
	CreateTrailPointFromVector(newPoint,vPos);

	// See which trail we're dealing with
	LIGHT_CYCLE_TRAIL* pTrail;
	if(nTrailID == CURRENT_TRAIL_ID)
	{
		pTrail = pCyclist->pCurTrail;
	}
	else
	{
		pTrail = pCyclist->FindTrail((uint16)nTrailID);
	}

	ASSERT(pTrail);
	if(!pTrail)
	{
		// We tried to update a non-existant trail
		return false;
	}

	int nPoints = pTrail->collTrailPoints.size();

	if(nPoints > 0)
	{
		// Get the last point
		pOldPoint = &(pTrail->collTrailPoints[nPoints-1]);

		// Save the old forward vector
		LTVector vOldForward = pCyclist->vForward;

		// Compute the forward vector
		pCyclist->vForward.x = newPoint.GetX() - pOldPoint->GetX();
		pCyclist->vForward.y = newPoint.GetY() - pOldPoint->GetY();
		pCyclist->vForward.z = newPoint.GetZ() - pOldPoint->GetZ();
		pCyclist->vForward.Normalize();

		if(nPoints > 1)
		{
			// Check for a turn
			float fDot = (float)(fabs(vOldForward.Dot(pCyclist->vForward)));
			
			// Check for a turn
			if(fDot <= MATH_EPSILON)
			{
				// We have a turn, so we have to do two things:
				// 1) Change our last point into the actual turn point
				// 2) Add on our current location point

				LIGHT_CYCLE_TRAIL_POINT turnPoint;

				// Check to see which axis we were travelling on
				if(((float)fabs(vOldForward.z)) > MATH_EPSILON)
				{
					// We were travelling in the Z direction and now we've turned

					// Set the new forward vector
					if(newPoint.GetX() > pOldPoint->GetX())
					{
						pCyclist->vForward.x = 1.0f;
					}
					else
					{
						pCyclist->vForward.x = -1.0f;
					}

					pCyclist->vForward.z = 0.0f;

					// Now calculate the turn point

					/* o = old point, n = new point, t = turn point

						|	o
						|	|\
					   z|	| \
						|	t--n
						|
						+--------
							x 
					*/
					
					// Turn point is the old X and new Z
					turnPoint.SetX(pOldPoint->GetX());
					turnPoint.SetZ(newPoint.GetZ());
				}
				else
				{
					// We were travelling in the X direction and now we've turned

					// Set the new forward vector
					if(newPoint.GetZ() > pOldPoint->GetZ())
					{
						pCyclist->vForward.z = 1.0f;
					}
					else
					{
						pCyclist->vForward.z = -1.0f;
					}

					pCyclist->vForward.x = 0.0f;

					// Now calculate the turn point

					/* o = old point, n = new point, t = turn point

						|	o--t
						|	 \ |
					   z|	  \|
						|	   n
						|
						+-------
							x
					*/
					
					// Turn point is the old Z, and new X
					turnPoint.SetX(newPoint.GetX());
					turnPoint.SetZ(pOldPoint->GetZ());
				}

				turnPoint.SetY(newPoint.GetY());
				if(pOldPoint->GetY() > (newPoint.GetY() + MATH_EPSILON))
				{
					// We shouldn't be going up/down ramps!
					ASSERT(FALSE);
				}

				// Change the last point to the turn point
				pOldPoint->SetX(turnPoint.GetX());
				pOldPoint->SetY(turnPoint.GetY());
				pOldPoint->SetZ(turnPoint.GetZ());

				// Now add on our current location point
				pTrail->collTrailPoints.push_back(newPoint);

				/************************ TEMP */
						// Let's walk his list of points
						std::vector<LIGHT_CYCLE_TRAIL_POINT>::iterator iter;

						// Walk the list
						int nPoint = 0;
						g_pLTServer->CPrint("\n****\n");
						for(iter=pTrail->collTrailPoints.begin();iter!=pTrail->collTrailPoints.end();iter++)
						{
							g_pLTServer->CPrint("Point %d: <%f, %f, %f>\n",nPoint,iter->GetX(),iter->GetY(),iter->GetZ());
							nPoint++;
						}	
						g_pLTServer->CPrint("****\n");
				/************************ TEMP */
			}
			else
			{
				// We're continuing in the same direction, 
				// so we'll just change the last point to our location
				pOldPoint->SetX(newPoint.GetX());
				pOldPoint->SetY(newPoint.GetY());
				pOldPoint->SetZ(newPoint.GetZ());
			}
		}
		else
		{
			// We only have one point on our list (the starting point)
			// so add a new one
			pTrail->collTrailPoints.push_back(newPoint);
		}
	}
	else
	{
		// We shouldn't ever have no points when we update
		ASSERT(FALSE);
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::AddLightCycleTrailPoint
//
//	PURPOSE:	Adds a point onto a light cycle trail
//
// ----------------------------------------------------------------------- //
uint32 CLightCycleMgr::AddLightCycleTrailPoint(LIGHT_CYCLIST* pCyclist, LIGHT_CYCLE_TRAIL* pTrail, LTVector& vPos)
{
	ASSERT(pCyclist);
	if(!pCyclist)
		return 0;

	ASSERT(pTrail);
	if(!pTrail)
		return 0;

	LIGHT_CYCLE_TRAIL_POINT newPoint;
	CreateTrailPointFromVector(newPoint,vPos);

	// Add the new point
	pTrail->collTrailPoints.push_back(newPoint);

	return pTrail->collTrailPoints.size();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::CreateTrailPointFromVector
//
//	PURPOSE:	Creates a trail point from a vector and will do
//				grid-snapping or alignment if we ever need it
//
// ----------------------------------------------------------------------- //
void CLightCycleMgr::CreateTrailPointFromVector(LIGHT_CYCLE_TRAIL_POINT &pt, LTVector &vPos)
{
	// For now, it's just a straight copy
	pt.vPoint = vPos;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::Update
//
//	PURPOSE:	Updates the light cycle mgr
//
// ----------------------------------------------------------------------- //
void CLightCycleMgr::Update()
{
	if(!m_bUpdating)
		return;

	// Vars
	LTVector vPos;
	LTVector vOldForward;
	LIGHT_CYCLIST* pCyclist;
	LIGHT_CYCLE_TRAIL *pTrail;
	std::vector<LIGHT_CYCLIST*>::iterator iter;

	// Walk the list
	for(iter=m_collCyclists.begin();iter!=m_collCyclists.end();iter++)
	{
		pCyclist = (*iter);
		if(pCyclist->bUpdating)
		{
			// Update his trail
			ASSERT(pCyclist->hObject);
			g_pLTServer->GetObjectPos(pCyclist->hObject,&vPos);

			pTrail = pCyclist->pCurTrail;

			// Check to see if we have a trail
			if(pTrail)
			{
				vOldForward = pCyclist->vForward;

				// Update the trail
				UpdateLightCycleTrail(pCyclist, vPos);
			}
			else
			{
				// Simple case: We're starting a new trail
				BeginLightCycleTrail(pCyclist,vPos);
			}

			// Now we check to see if this guy collided
			LightCycleCollisionInfo info;
			if(CheckForCollision(pCyclist,info))
			{
				// Uh-oh. We have a crash. And crashing is bad, m'kay?
				HandleCollision(pCyclist,info);
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::CheckForCollision
//
//	PURPOSE:	Checks for a collision
//
// ----------------------------------------------------------------------- //
bool CLightCycleMgr::CheckForCollision(LIGHT_CYCLIST *pCyclist, LightCycleCollisionInfo &info)
{
	if(!pCyclist)
		return false;

	LIGHT_CYCLE_TRAIL* pTrail = pCyclist->pCurTrail;

	if(!pTrail)
		return false;

	// Let's walk his list of points
	std::vector<LIGHT_CYCLE_TRAIL_POINT>::iterator iter;

	// Walk the list
	int nPoint = 0;
	for(iter=pTrail->collTrailPoints.begin();iter!=pTrail->collTrailPoints.end();iter++)
	{
	}	

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleMgr::HandleCollision
//
//	PURPOSE:	Handles a collision
//
// ----------------------------------------------------------------------- //
void CLightCycleMgr::HandleCollision(LIGHT_CYCLIST *pCyclist, LightCycleCollisionInfo &info)
{
}