// ScreenProfile.h: interface for the ScreenProfile class.
//
//////////////////////////////////////////////////////////////////////

#ifndef SCREEN_PROFILE_H
#define SCREEN_PROFILE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseScreen.h"

#pragma warning (disable : 4503)
#pragma warning( disable : 4786 )
#include <vector>
#include <string>

const int MAX_PROFILENAME_LEN = 16;

class CScreenProfile : public CBaseScreen
{
public:
	
	CScreenProfile();
	virtual ~CScreenProfile();

	// Build the screen
    bool   Build();

    virtual uint32	OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
	void			Escape();
	void			OnFocus(bool bFocus);
	bool			OnRButtonUp(int x, int y);
	bool			OnLButtonUp(int x, int y);

private:
    void	HandleDlgCommand(uint32 dwCommand, uint16 nIndex);
	void	CreateProfileList();
	void	UpdateProfileName();

	wchar_t	m_szProfile[MAX_PROFILENAME_LEN];
	wchar_t	m_szOldProfile[MAX_PROFILENAME_LEN];

	CLTGUITextCtrl	*m_pCurrent;
	CLTGUITextCtrl	*m_pLoad;
	CLTGUITextCtrl	*m_pDelete;
	CLTGUITextCtrl	*m_pRename;


	ProfileArray		m_ProfileList;
	CLTGUIListCtrl*		m_pListCtrl;
	CLTGUIWindow*		m_pDlg;

	CUserProfile*		m_pProfile;

};

#endif // SCREEN_PROFILE_H