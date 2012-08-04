// ----------------------------------------------------------------------- //
//
// MODULE  : HUDControlPoint.h
//
// PURPOSE : HUDItem to display control point status
//
// CREATED : 01/20/06
//
// (c) 2006 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUDCONTROLPOINT_H__
#define __HUDCONTROLPOINT_H__

#include "HUDItem.h"


class CHUDControlPoint : public CHUDItem
{
public:
	CHUDControlPoint();
	virtual ~CHUDControlPoint() {}

	virtual bool	Init();

	virtual void	Render();
	virtual void	Update();

	virtual void	UpdateLayout();
	virtual void	OnExitWorld() {m_nTeamID = INVALID_TEAM;}

	void	SetIndex(uint16 nIndex);
	void	SetTeam(uint8 nTeamID);



protected:
	uint8	m_nTeamID;
	uint16   m_nIndex;

	uint32				m_cTeamColor[3];
	TextureReference	m_hTeamIcon[3];

};

class CHUDControlPointList : public CHUDItem
{
public:
	CHUDControlPointList();
	virtual ~CHUDControlPointList() {}

	virtual bool	Init();
	virtual void	Term();

	virtual void	Render();
	virtual void	Update();

	virtual void	UpdateLayout();

	virtual void	OnExitWorld();
	virtual void		UpdateFlash();

	virtual void ScaleChanged();


protected:
	// Declare delegate to listen for player team change events.
	static void OnPlayerChangedTeamsEvent( CHUDControlPointList* pCPList, CClientInfoMgr* pClientInfoMgr, EventCaster::NotifyParams& notifyParams )
	{
		g_pHUDMgr->QueueUpdate(kHUDControlPoint);
	}
	Delegate< CHUDControlPointList, CClientInfoMgr, OnPlayerChangedTeamsEvent > m_delegatePlayerChangedTeamsEvent;


	bool			 m_bUpdated;
	uint16			 m_nActivePoints;
	CHUDControlPoint m_ControlPoints[MAX_CONTROLPOINT_OBJECTS];
	LTVector2n		 m_vPointSize;
	uint16			 m_nMaxPerRow;

};


#endif  // __HUDCONTROLPOINT_H__
