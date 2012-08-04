// ----------------------------------------------------------------------- //
//
// MODULE  : HUDEnergyTransfer.h
//
// PURPOSE : HUDItem to display an energy transfer in progress
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_ENERGYTRANSFER_H
#define __HUD_ENERGYTRANSFER_H

#include "HUDItem.h"

//******************************************************************************************
//** HUD Ratings display
//******************************************************************************************
class CHUDEnergyTransfer : public CHUDItem
{
public:
	CHUDEnergyTransfer();

    LTBOOL      Init();
	void		Term();

    void        Render();
    void        Update();

    void        UpdateLayout();

private:
	LTIntPt		GetPoint(float fAngle, float fRadius);
	void		BuildPoly(LT_POLYF4 * pPoly, float theta1, float theta2, float r1, float r2);

	float		m_fScale;

	bool		m_bShow;

	LT_POLYF4	m_Connector;
	LT_POLYF4	m_EmptyArc[20];
	LT_POLYF4	m_FullArc[20];

//	LT_POLYF4	m_Base;
//	LT_POLYF4	m_Progress;
};

#endif