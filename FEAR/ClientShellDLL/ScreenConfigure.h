// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenConfigure.h
//
// PURPOSE : Interface screen for binding keys to commands
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef _SCREEN_CONFIGURE_H_
#define _SCREEN_CONFIGURE_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseScreen.h"
#include "ProfileMgr.h"
#include "BindMgr.h"

class CScreenConfigure : public CBaseScreen
{
public:
	CScreenConfigure();
	virtual ~CScreenConfigure();

    bool   Init(int nScreenID);
    bool   Render();

	// Build the screen
    bool   Build();
	virtual void OnFocus(bool bFocus);

	// Handle input
    virtual bool   OnUp();
    virtual bool   OnDown();
    virtual bool   OnLeft();
    virtual bool   OnRight();
    virtual bool   OnEnter();
    virtual bool   OnLButtonDown(int x, int y);
    virtual bool   OnLButtonUp(int x, int y);
    virtual bool   OnLButtonDblClick(int x, int y);
    virtual bool   OnRButtonDown(int x, int y);
    virtual bool   OnRButtonUp(int x, int y);
    virtual bool   OnRButtonDblClick(int x, int y);
    virtual bool   OnMouseMove(int x, int y);

	virtual void	Escape();


protected:
	typedef std::vector<std::pair<uint32, uint32> > TDeviceObjectList;

protected:
	uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
    void	InitControlList();
    void	UpdateControlList();
    void	AdjustControlFrame();

	void	SetControlText(int nType, int nIndex);
	void	SetControlText(CLTGUICtrl *pCtrl);
    bool	HandleKeyDown(int key, int rep);
    bool	KeyRemappable(uint32 nDevice, uint32 nObject);
    bool	SetCurrentSelection(uint32 nDevice, uint32 nObject, bool bForce);
	int		GetCommand(int nType, int nIndex);

	void	SetCurrentType(int nType);

	void	StartWaitForKeypress();
	void	EndWaitForKeypress();
	void	UpdateWaitForKeypress();
	void	GetButtonsDown(TDeviceObjectList *pResults);

	const wchar_t* GetControlName(const CBindMgr::SBinding& sCurrentBinding);

    bool			m_bWaitingForKey, m_bFirstKeyUpdate;
	CLTGUIListCtrl* m_pList[kNumCommandTypes];


    float         m_fInputPauseTimeLeft;
	TDeviceObjectList m_aMaskedInput;

	LTRect2n		m_ListRect;
	uint8			m_nListFontSize;
	int				m_nActionWidth;
	int				m_nEqualsWidth;
	int				m_nCommandWidth;

	int				m_nType;

	CUserProfile*	m_pProfile;

	uint32			m_nConfirmDevice;
	uint32			m_nConfirmObject;
	float			m_fConfirmValue;
};

#endif // _SCREEN_CONFIGURE_H_