// ----------------------------------------------------------------------- //
//
// MODULE  : HUDCrosshair.h
//
// PURPOSE : HUDItem to display crosshair
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_CROSSHAIR_H
#define __HUD_CROSSHAIR_H

#include "HUDItem.h"


//******************************************************************************************
//** HUD crosshair
//******************************************************************************************
class CHUDCrosshair : public CHUDItem
{
public:
	CHUDCrosshair();

	virtual bool	Init();
	virtual void	Term();

	virtual void	Render();
	virtual void	Update();
	virtual void	ScaleChanged();	
	virtual void	OnExitWorld() { Enable(true); }
	virtual void	UpdateLayout();


	void	RenderScope();

	void    RenderCrosshair(const LTVector2& vPos, bool bInterface = false);

	bool	IsEnabled() const {return m_bEnabled;}
	void	Enable(bool bEnable) {m_bEnabled = bEnable;}


	void	SetRangeDisplay(bool bDisplay, const LTVector2n* pvDisplayPos = NULL, uint32 cDisplayColor = 0);

protected:

	bool	CanShowCrosshair( );
	
protected:

	LTPoly_GT4		m_Poly;
	uint32			m_cCrosshairColor;


	//Layout info
	uint32			m_StrDisColor;
	uint32			m_TeamColor[2];
	float			m_fHoldTime;

	//activation data
	LTObjRef	m_hObj;
	const char*	m_szStringID;

	bool		m_bEnabled;
	bool		m_bArmed;

	bool		m_bCamActive;

	bool		m_bCanActivateTarget;
	uint8		m_nTargetTeam;
	double		m_fTargetTime;

	bool			m_bRangeEnabled;
	LTVector2n		m_vRangePos;
	CLTGUIString	m_RangeDisplay;

	HTEXTURESTRING	m_hRangeTexture;

	LTVector2	m_vCenter;
	float		m_fBarSize;
	float		m_fMaxSize;

};

#endif