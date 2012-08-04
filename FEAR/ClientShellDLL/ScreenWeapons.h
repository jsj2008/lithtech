// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenWeapons.h
//
// PURPOSE : Interface to set weapon priorities
//
// CREATED : 07/07/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SCREENWEAPONS_H__
#define __SCREENWEAPONS_H__

class CScreenWeapons : public CBaseScreen
{
public:
	CScreenWeapons();
	virtual ~CScreenWeapons();

	// Build the folder
	bool	Build();
	void	OnFocus(bool bFocus);


protected:

	void	SelectWeapon(CLTGUICtrl* pCtrl);
	uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

	void	LoadList();
	void	SaveList();

	uint32 FindIndexOfWeapon(HWEAPON hWpn);

private:

	CLTGUIListCtrl*	m_pWeapons;
	CLTGUIFrame*	m_pCurrentImg;
	CLTGUITextCtrl*	m_pCurrentTxt;

	CLTGUITextureButton*	m_pUp;
	CLTGUITextureButton*	m_pDown;

	CLTGUICtrl*	m_pSelectedWpn;

	uint32		m_cHighlight;

};

#endif  // __SCREENWEAPONS_H__
