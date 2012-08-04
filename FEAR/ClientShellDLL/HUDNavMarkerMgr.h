// ----------------------------------------------------------------------- //
//
// MODULE  : HUDNavMarkerMgr.h
//
// PURPOSE : HUDItem to manage display of multiple NavMarkers
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_NAV_MARKER_MGR_H
#define __HUD_NAV_MARKER_MGR_H

#include "HUDItem.h"

class CHUDNavMarker;

class CHUDNavMarkerMgr : public CHUDItem
{
public:
	CHUDNavMarkerMgr();
	virtual ~CHUDNavMarkerMgr();

	virtual bool	Init();
	virtual void	Term();

	virtual void	Render();
	virtual void	Update();
	virtual void	ScaleChanged();
	virtual void	UpdateLayout() {};
	virtual void	UpdateFlicker();

	virtual void	OnExitWorld() {RemoveAllMarkers();}

	void	AddMarker(CHUDNavMarker* pHUDNM);
	void	RemoveMarker(CHUDNavMarker* pHUDNM);

	void	RemoveAllMarkers();

	HTEXTURESTRING GetRangeTextureString() const { return m_hRangeTexture; }

	bool	MultiplayerFilter() const { return m_bMultiplayerFilter; }
	void	SetMultiplayerFilter(bool bOn);

private:

	typedef std::vector<CHUDNavMarker*, LTAllocator<CHUDNavMarker*, LT_MEM_TYPE_CLIENTSHELL> > MarkerArray;
	MarkerArray	m_ActiveMarkers;

	CHUDNavMarker* m_pArrowMarker;

	HTEXTURESTRING	m_hRangeTexture;

	bool	m_bMultiplayerFilter;
	bool	m_bFirstUpdate;


};

#endif
