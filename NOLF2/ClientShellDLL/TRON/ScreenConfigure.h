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

#define TRACK_BUFFER_SIZE			8
#define SCREEN_CONTROLS_NUM_DEVICES	3
#define MAX_CONTROLS_PER_ACTION		2


class CScreenConfigure : public CBaseScreen
{
public:
	CScreenConfigure();
	virtual ~CScreenConfigure();

    LTBOOL   Init(int nScreenID);
    LTBOOL   Render(HSURFACE hDestSurf);

	// Build the screen
    LTBOOL   Build();
	virtual void OnFocus(LTBOOL bFocus);

	// Handle input
    virtual LTBOOL   OnUp();
    virtual LTBOOL   OnDown();
    virtual LTBOOL   OnLeft();
    virtual LTBOOL   OnRight();
    virtual LTBOOL   OnEnter();
    virtual LTBOOL   OnLButtonDown(int x, int y);
    virtual LTBOOL   OnLButtonUp(int x, int y);
    virtual LTBOOL   OnLButtonDblClick(int x, int y);
    virtual LTBOOL   OnRButtonDown(int x, int y);
    virtual LTBOOL   OnRButtonUp(int x, int y);
    virtual LTBOOL   OnRButtonDblClick(int x, int y);
    virtual LTBOOL   OnMouseMove(int x, int y);

	virtual void	Escape();


protected:
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
    void	InitControlList();
    void	UpdateControlList();

	void	SetControlText(int nType, int nIndex);
	void	SetControlText(CLTGUICtrl *pCtrl);
    LTBOOL  HandleKeyDown(int key, int rep);
    LTBOOL  KeyRemappable (DeviceInput* pInput);
    uint32  GetControlType(uint32 deviceType, char *controlName);
    LTBOOL  SetCurrentSelection (DeviceInput* pInput);
    LTBOOL  CheckMouseWheel (DeviceInput* pInput);
	int		GetCommand(int nType, int nIndex);
    void    Bind(int nCommand, uint16 diCode, char *lpszControlName, uint32 deviceType);
    void    UnBind(uint16 diCode, char *lpszControlName, uint32 deviceType);

	void	SetCurrentType(int nType);

    LTBOOL          m_bControlChanged;
    LTBOOL          m_bWaitingForKey;
	CLTGUIListCtrl* m_pList[kNumCommandTypes];


    LTFLOAT         m_fInputPauseTimeLeft;
	DeviceInput		m_pInputArray[TRACK_BUFFER_SIZE];

	LTRect			m_ListRect;
	uint8			m_nListFontSize;
	int				m_nActionWidth;
	int				m_nEqualsWidth;
	int				m_nCommandWidth;

	int				m_nType;

	CUserProfile*	m_pProfile;


};

#endif // _SCREEN_CONFIGURE_H_