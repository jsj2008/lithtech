// BaseSelectionFolder.h: interface for the CBaseSelectionFolder class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BASESELECTIONFOLDER_H__C876BDA1_6D07_11D3_B2DB_006097097C7B__INCLUDED_)
#define AFX_BASESELECTIONFOLDER_H__C876BDA1_6D07_11D3_B2DB_006097097C7B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"
#include "FolderMgr.h"
#include "BaseScaleFX.h"
#include "WeaponMgr.h"

#define MAX_SELECTION_SLOTS 5

enum eSelectionType
{
	SEL_NOT_ALLOWED,
	SEL_ALLOWED,
	SEL_DEFAULT,
	SEL_REQUIRED,
	SEL_INTEGRAL,
};


class CBaseSelectionFolder : public CBaseFolder
{
public:
	CBaseSelectionFolder();
	virtual ~CBaseSelectionFolder();

    virtual LTBOOL   Build();
	virtual void	Escape();
	virtual void	Term();

	// Renders the folder to a surface
    virtual LTBOOL   Render(HSURFACE hDestSurf);

	// This is called when the folder gets or loses focus
    virtual void    OnFocus(LTBOOL bFocus);


	// Handles a key press.  Returns FALSE if the key was not processed through this method.
	// Left, Up, Down, Right, and Enter are automatically passed through OnUp(), OnDown(), etc.
    virtual LTBOOL   HandleKeyDown(int key, int rep);

    virtual uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

	// Mouse messages
    virtual LTBOOL   OnLButtonDown(int x, int y);
    virtual LTBOOL   OnLButtonUp(int x, int y);
    virtual LTBOOL   OnLButtonDblClick(int x, int y);
    virtual LTBOOL   OnRButtonDown(int x, int y);
    virtual LTBOOL   OnRButtonUp(int x, int y);
    virtual LTBOOL   OnRButtonDblClick(int x, int y);
    virtual LTBOOL   OnMouseMove(int x, int y);

    int		AddToSlot(int nItemId, int nNameId, LTBOOL bRequired);
    int		AddEmptySlot();
	void	RemoveFromSlot(int nItemId);
	void	ClearSlots();

	int		AddItem(int nItemId, int nNameId);
	void	RemoveItem(int nItemId);


	void	ItemToSlot(int nItemId, int nNameId);
	void	SlotToItem(int nItemId, int nNameId);

	//searches slots and list, returns index of item
	int		FindItemIndex(int nItemId);

	static const int kEmptySlot;

protected:

	// Handle input
    virtual LTBOOL   OnUp();
    virtual LTBOOL   OnDown();
    virtual LTBOOL   OnLeft();
    virtual LTBOOL   OnRight();
    virtual LTBOOL   OnEnter();
    virtual LTBOOL   OnPageUp();
    virtual LTBOOL   OnPageDown();

	virtual LTBOOL	UpdateSelection();
	virtual void	ClearSelection();

	void	UpdateArrows();

	void	SetPhotoBitmap(char *pszPhoto);
	void	AddPhotoBitmap(HSURFACE hSurf);

protected:

	int		m_nNumSlots;
	int		m_nSlotsFilled;
	int		m_nSlotsLocked;
	LTBOOL	m_bSaveSelection;
	int		m_nLastListItem;
	int		m_nFirstSlot;

	CLTGUITextItemCtrl*	m_pName;
	CStaticTextCtrl*	m_pDescription;


    LTBOOL   m_bReadLayout;
    LTIntPt  m_SlotOffset;
    LTRect   m_ListRect;
    LTRect   m_NameRect;
    LTRect   m_DescriptionRect;


	static 	char	m_sSelectStr[32];
	static 	char	m_sUnselectStr[32];
	static 	char	m_sRequiredStr[32];
	static 	char	m_sEquippedStr[32];
	static 	char	m_sUnequippedStr[32];
	static 	char	m_sEmptyStr[32];

	char	m_sPhoto[128];
    LTIntPt  m_PhotoPos;

	// Array of photo surfaces
	CMoArray<HSURFACE>	m_sharedSurfaceArray;
    HSURFACE m_hPhotoSurf;

	CBaseScaleFX	m_ModelSFX;
	char			m_szModel[WMGR_MAX_FILE_PATH];
	char			m_szSkin[WMGR_MAX_FILE_PATH];
	LTFLOAT			m_fSFXRot;
	LTFLOAT			m_fScale;
	LTVector		m_vOffset;


};

//the current folder is passed in rather than calculated, because the function might be called
// before the current folder variable is updated
eFolderID GetNextSelectionFolder(eFolderID eCurrent, int *pnHelpID);
eFolderID GetPreviousSelectionFolder(eFolderID eCurrent);

#endif // !defined(AFX_BASESELECTIONFOLDER_H__C876BDA1_6D07_11D3_B2DB_006097097C7B__INCLUDED_)