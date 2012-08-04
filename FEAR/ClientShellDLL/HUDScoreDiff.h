// ----------------------------------------------------------------------- //
//
// MODULE  : HUDScoreDiff.h
//
// PURPOSE : Definition of CHUDScoreDiff to display player scores
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_SCOREDIFF_H
#define __HUD_SCOREDIFF_H

#include "HUDItem.h"

//******************************************************************************************
//** HUD Score differential
//******************************************************************************************
class CHUDScoreDiff : public CHUDItem
{
public:
	CHUDScoreDiff();
	virtual ~CHUDScoreDiff() {}	

    virtual bool	Init();
	virtual void	Term();

    virtual void	Render();
	virtual void	Update();
	virtual void	ScaleChanged();

	virtual void	UpdateLayout();

protected:

	uint32		m_cWinningTextColor;
	uint32		m_cLosingTextColor;

};



#endif