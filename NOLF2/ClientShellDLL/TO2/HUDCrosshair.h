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
	

    LTBOOL	Init();
	void	Term();

    void	Render();
    void	Update();

	void	UpdateLayout();

	void	RenderScope();

	void	Toggle() {m_bEnabled = !m_bEnabled;}

	void	SetStyle(uint8 style);

protected:

	void	ScalePolies();


	LTPoly_GT4		m_Poly[3];

	CUIFormattedPolyString*	m_pStr;
	LTIntPt			m_StrPos;
	uint8			m_nStrSz;
	uint8			m_nStrJust;
	uint32			m_StrColor;
	uint32			m_StrDisColor;
	uint32			m_TeamColor[2];


	CUIFormattedPolyString*	m_pDbgStr;
	LTIntPt			m_DbgPos;
	uint8			m_nDbgSz;
	uint8			m_nDbgJust;
	uint16			m_nDbgWidth;
	uint32			m_DbgColor;

	float			m_fScale;
	float			m_x;
	float			m_y;
	float			m_dbgx;
	float			m_dbgy;

	//activation data
	HOBJECT		m_hObj;
	uint16		m_nString;

	bool		m_bEnabled;
	bool		m_bArmed;

	bool		m_bCamActive;

	HTEXTURE	m_hAccurate;
	HTEXTURE	m_hInaccurate;

	float		m_fAccurateSz;
	float		m_fInaccurateSz;

	uint8		m_style;
	
	bool		m_bCanActivateTarget;
	uint8		m_nTargetTeam;

};

#endif