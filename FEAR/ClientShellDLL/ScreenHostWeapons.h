// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenHostWeapons.h
//
// PURPOSE : Interface screen for choosing server weapon restrictions
//
// CREATED : 11/12/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SCREENHOSTWEAPONS_H__
#define __SCREENHOSTWEAPONS_H__


class CScreenHostWeapons : public CBaseScreen
{
public:
	CScreenHostWeapons();
	virtual ~CScreenHostWeapons();

	// Build the screen
	virtual bool	Build();

	virtual void    OnFocus(bool bFocus);


protected:
	enum eItemTypes
	{
		kWeaponType,
		kAmmoType,
		kGearType,
	};

	uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
	bool	FillAvailList();
	void	LoadItemList();
	void	MakeDefaultItemList();
	void	SaveItemList();
	void	AddItemToList(uint32 nId, bool bSelected, eItemTypes eType);
	void	UpdateButtons();


	CLTGUITextCtrl*			m_pAdd;
	CLTGUITextCtrl*			m_pRemove;
	CLTGUITextCtrl*			m_pRemoveAll;
	CLTGUIListCtrl*			m_pAvailItems;
	CLTGUIListCtrl*			m_pSelItems;

	StringSet m_setRestrictedWeapons;
	StringSet m_setRestrictedGear;


};

#endif  // __SCREENHOSTWEAPONS_H__
