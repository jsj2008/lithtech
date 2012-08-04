// ----------------------------------------------------------------------- //
//
// MODULE  : HUDPlayerList.h
//
// PURPOSE : Definition of CHUDPlayerList to display list of teammates
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_PLAYERS_H
#define __HUD_PLAYERS_H

#include "HUDItem.h"
#include "ClientServerShared.h"

//******************************************************************************************
//** HUD Player List
//******************************************************************************************
class CHUDPlayerList : public CHUDItem
{
public:
	CHUDPlayerList();


	virtual bool	Init();
	virtual void	Term();

	virtual void	Render();
	virtual void	Update();
	virtual void	ScaleChanged();

	virtual void    UpdateLayout();


protected:
	bool		m_bVisible;

	uint32		m_cDeadColor;
	uint32		m_cTeamColor[2];

	CLTGUIString	m_Players[MAX_MULTI_PLAYERS];
	uint8			m_nCount;

};



#endif