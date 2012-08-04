// FolderInventory.h: interface for the CFolderInventory class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _FOLDER_INVENTORY_H_
#define _FOLDER_INVENTORY_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseSelectionFolder.h"
#include "BaseScaleFX.h"
#include "WeaponMgr.h"
#include "MissionData.h"

class CFolderInventory : public CBaseSelectionFolder
{
public:
	CFolderInventory();
	virtual ~CFolderInventory();

	// Build the folder
    LTBOOL	Build();

	// This is called when the folder gets or loses focus
    virtual void    OnFocus(LTBOOL bFocus);
	virtual void	UpdateInterfaceSFX();

    virtual uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

    HSTRING         GetHelpString(uint32 dwHelpId, int nControlIndex);

	virtual	LTBOOL	UpdateSelection();
    virtual LTBOOL	IsAvailable() {return LTTRUE;}

protected:

    void			BuildInventoryList();
	void			SaveInventoryData();
	void			ClearInventoryList();

	virtual void	CreateModelSFX();
	virtual void	RemoveInterfaceSFX();

	void			CalculateAmmo(int nWeaponID);

	void			AddWeapon(int nID);
	void			AddMod(int nWeaponID, int nID);
	void			AddAmmo(int nWeaponID, int nID, LTBOOL bInfinite);
	void			AddGear(int nID);

	CBaseScaleFX	m_Inventory;
	char			m_szModel[WMGR_MAX_FILE_PATH];
	char			m_szSkin[WMGR_MAX_FILE_PATH];
	LTFLOAT			m_fSFXRot;

	int				*m_nAmmo;
	CMissionData	*m_pData;
	CLTGUIFont		*m_pFont;

	int				m_nLastWeapon;

	CLTGUITextItemCtrl *m_pMissionCtrl;

};

#endif // _FOLDER_INVENTORY_H_