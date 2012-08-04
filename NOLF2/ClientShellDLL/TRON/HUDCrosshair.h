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

typedef enum
{
	RETICLE_NORMAL = 0,
	RETICLE_INNOCENT,
	RETICLE_ENEMY,
	RETICLE_ACTIVATE,
	NUM_RETICLES
} eReticleType;

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

	void	RenderUnarmed();

	void	RenderArmed();
	void	RenderArmed(LTBOOL bMenu, LTIntPt pos);

	void	RenderScope();

	void	Toggle() {m_bEnabled = !m_bEnabled;}

protected:

	LT_POLYGT4		m_Poly;

	CUIFormattedPolyString*	m_pStr;
	LTIntPt			m_StrPos;
	uint8			m_nStrSz;
	uint8			m_nStrJust;
	uint32			m_StrColor;
	uint32			m_StrDisColor;

	CUIFormattedPolyString*	m_pDbgStr;
	LTIntPt			m_DbgPos;
	uint8			m_nDbgSz;
	uint8			m_nDbgJust;
	uint16			m_nDbgWidth;
	uint32			m_DbgColor;

	// Additional information about the currently targeted object i.e. energy required
	CUIFormattedPolyString* m_pInfoStr;
	LTIntPt			m_InfoPos;
	uint8			m_nInfoSz;
	uint8			m_nInfoJust;
	uint16			m_nInfoWidth;
	uint32			m_InfoColor;
	uint32			m_InfoBadColor;

	float			m_fScale;
	float			m_x;
	float			m_y;
	float			m_dbgx;
	float			m_dbgy;

	float			m_infox;
	float			m_infoy;

	//activation data
	HOBJECT		m_hObj;
	uint16		m_nString;

	bool		m_bEnabled;
	bool		m_bArmed;

	LTFLOAT		m_fCrosshairGapMin;
	LTFLOAT		m_fCrosshairGapRange;
	LTFLOAT		m_fCrosshairBarMin;
	LTFLOAT		m_fCrosshairBarRange;

	bool		m_bCamActive;

	HTEXTURE	m_hReticleTex[NUM_RETICLES];
	LT_POLYFT4	m_ReticlePrim;
};

#endif