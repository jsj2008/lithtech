////////////////////////////////////////////////////////////////
//
// keyconfigdlg.h
//
// The implementation for the dialog that handles the list of
// hotkeys and allows the user to manipulate the database. Allows
// for binding keys, and saving and loading of databases. Relies
// somewhat heavily on CBindKeyDlg, which it uses to do actual
// key binding.
//
// Author: John O'Rorke
// Created: 7/10/00
// Modification History:
//
////////////////////////////////////////////////////////////////
#ifndef __KEYCONFIGDLG_H__
#define __KEYCONFIGDLG_H__

#include "stdafx.h"
#include "hotkeydb.h"

class CKeyConfigDlg : 
	public CDialog
{
public:

	//constructor that takes a configuration to use as a default configuration
	//this will be used as the starting values for all the keys. Also takes a 
	//pointer to the tracker of which will be set with the final values if the
	//user presses OK
	CKeyConfigDlg(	const CHotKeyDB& CurrConfig, 
					CUITrackerMgr*	pUITrackerMgr	= NULL,
					CWnd* pParentWnd				= NULL);

	~CKeyConfigDlg();

	//a pointer to the tracker which will be used to associate the values
	//when the user presses OK
	CUITrackerMgr*	m_pUITrackerMgr;

	//currently configured key database
	CHotKeyDB		m_Config;

private:

	//handles to icons used for loading and saving
	HICON			m_hSaveIcon;
	HICON			m_hLoadIcon;

	//gets the list control for the key list
	CListCtrl*		GetKeyList();

	//enables/disables the buttons that perform actions upon a key
	void			EnableActionButtons(BOOL bEnable = TRUE);

	//initializes the list control with all the keys in the current database
	void			InitKeyList();

	//sets up all the information about an item
	void			SetupItem(uint32 nIndex, const CHotKey* pKey);

	//notify messages sent from the key list
	afx_msg void	OnKeyActivated(NMHDR* pmnh, LRESULT* pResult);
	afx_msg void	OnKeySelected(NMHDR* pmnh, LRESULT* pResult);

	//standard button handlers
	void			OnOK();
	void			OnCancel();

	//handle initialization and loading of icons
	BOOL			OnInitDialog();

	//handle buttons for mapping keys
	afx_msg void	OnBind();
	afx_msg void	OnClear();
	afx_msg void	OnReset();

	//handle resetting the database
	afx_msg void	OnResetAll();

	//handle loading and saving configurations
	afx_msg void	OnLoad();
	afx_msg void	OnSave();
	

	DECLARE_MESSAGE_MAP()

};

#endif

