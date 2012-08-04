// ----------------------------------------------------------------------- //
//
// MODULE  : MultiplayerMenu.h
//
// PURPOSE : In-game Multiplayer Menu
//
// CREATED : 3/20/00
//
// ----------------------------------------------------------------------- //

#ifndef __MULTIPLAYER_MENU_H__
#define __MULTIPLAYER_MENU_H__

#include "BaseMenu.h"

class CMultiplayerMenu : public	CBaseMenu
{
public:
	CMultiplayerMenu();

	void Build();
    void Select(uint8 byItem);

};

class CTeamMenu : public CBaseMenu
{
public:
	CTeamMenu();

	void Build();
    void Select(uint8 byItem);

};

class COptionMenu : public CBaseMenu
{
public:
	COptionMenu();

	void Build();
    void Select(uint8 byItem);

};

#endif
