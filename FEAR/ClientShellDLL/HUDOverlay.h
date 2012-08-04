// ----------------------------------------------------------------------- //
//
// MODULE  : HUDOverlay.h
//
// PURPOSE : HUDItem to display overlays
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_OVERLAY_H
#define __HUD_OVERLAY_H

#include "HUDItem.h"
#include "HUDAnimation.h"

const char* const OVM_BINOC = "Binoculars";
const char* const OVM_ZOOM_IN = "ZoomIn";
const char* const OVM_ZOOM_OUT = "ZoomOut";
const char* const OVM_DAMAGE = "Damage";

class CHUDOverlay
{
public:
	CHUDOverlay();
	virtual ~CHUDOverlay() {};

	bool	Init(HRECORD hOverlayRec);
	void	InitAnim() { m_Anim.Init(); }

	std::string				m_sName;
	CHUDAnimation			m_Anim;
	bool					m_bExclusive;
	uint32					m_nZOrder;
};

//******************************************************************************************
//** HUD overlay
//******************************************************************************************
class CHUDOverlayMgr : public CHUDItem
{
public:
	CHUDOverlayMgr();
	virtual ~CHUDOverlayMgr() {};
	

	virtual bool		Init();
	virtual void		Term();

	virtual void        Render();
	virtual void        Update();
	virtual void        ScaleChanged();
	virtual void		UpdateLayout() {};

	CHUDOverlay*		GetHUDOverlay( char const* pszName ) const;

	void	Show(CHUDOverlay* pOverlay);
	void	Hide(CHUDOverlay* pOverlay);
	bool	IsVisible(CHUDOverlay* pOverlay);

	HMATERIAL GetMaterialInstance( CHUDOverlay* pOverlay );
	
	// Sets the color property of the overlay.
	uint32	GetColor( CHUDOverlay* pOverlay ) const;
	void	SetColor( CHUDOverlay* pOverlay, uint32 nColor );

protected:
	typedef std::vector< CHUDOverlay*, LTAllocator<CHUDOverlay*, LT_MEM_TYPE_CLIENTSHELL> > OverlayList;
	OverlayList m_vecOverlays;
};

#endif