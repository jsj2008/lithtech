////////////////////////////////////////////////////////////////
//
// bindkeydlg.h
//
// Implementation for dialog that allows editing the associated
// events for a specific hot key. 
//
// Author: John O'Rorke
// Created: 7/10/00
// Modification History:
//
////////////////////////////////////////////////////////////////
#ifndef __BINDKEYDLG_H__
#define __BINDKEYDLG_H__

#include "stdafx.h"
#include "hotkey.h"

class CHotKeyDB;

class CBindKeyDlg : 
	public CDialog
{
public:

	//constructor that takes a hotkey to initialize its data to
	CBindKeyDlg(	const CHotKey* HotKey			= NULL,
					CHotKeyDB* pDB					= NULL,
					CWnd* pParentWnd				= NULL);

	//the current hot key that is being edited by the dialog
	CHotKey			m_HotKey;

	//converts a hotkey to a string
	static CString			HotKeyToString(const CHotKey& Key);
	//converts a key to a text equivelent
	static CString			EventToString(const CUIEvent& Event);

private:

	//the database that needs to be checked for existing hot key conflicts
	CHotKeyDB*		m_pDB;

	//a backup of the original hotkey in case the user wants to reset back
	//to the original key
	CHotKey			m_OriginalHotKey;

	//sets the specified check box control to checked if the current hotkey
	//has the specified key included
	void			SetCheckIfKeyUsed(int nControl, uint32 nKey);

	//fills the specified dropdown box with all the keys possible to select
	void			FillDropDown(int nControl);

	//standard button handlers
	void			OnOK();
	void			OnCancel();

	//handle initialization and loading of icons
	BOOL			OnInitDialog();


	DECLARE_MESSAGE_MAP()

};

#endif

