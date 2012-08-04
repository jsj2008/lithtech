// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenHostWeapons.h
//
// PURPOSE : Interface screen for choosing levels for a hosted game
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef _SCREENHOSTWEAPONS_H_
#define _SCREENHOSTWEAPONS_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseScreen.h"


class CScreenHostWeapons : public CBaseScreen
{
public:
	CScreenHostWeapons();
	virtual ~CScreenHostWeapons();

	// Build the screen
    LTBOOL  Build();
	void	Escape();

    void    OnFocus(LTBOOL bFocus);


protected:
	enum eItemTypes
	{
		kWeaponType,
		kAmmoType,
		kGearType,
	};

    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
    LTBOOL  FillAvailList();
	void	LoadItemList();
	void	MakeDefaultItemList();
	void	SaveItemList();
	void	AddItemToList(int nId, bool bSelected, eItemTypes eType);
	void	UpdateButtons();


	CLTGUITextCtrl*			m_pAdd;
	CLTGUITextCtrl*			m_pRemove;
	CLTGUITextCtrl*			m_pAddAll;
	CLTGUITextCtrl*			m_pRemoveAll;
	CLTGUIListCtrl*			m_pAvailItems;
	CLTGUIListCtrl*			m_pSelItems;

};

#endif // _SCREENHOSTWEAPONS_H_