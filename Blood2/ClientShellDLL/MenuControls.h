// MenuControls.h: interface for the CMenuControls class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MENUCONTROLS_H__08A756C2_5C86_11D2_BDA2_0060971BDC6D__INCLUDED_)
#define AFX_MENUCONTROLS_H__08A756C2_5C86_11D2_BDA2_0060971BDC6D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "MenuBase.h"

#define MENU_CONTROLS_NUM_ACTIONS	42	// 44
#define MENU_CONTROLS_NUM_DEVICES	3

class CMenuControls : public CMenuBase  
{
public:
	CMenuControls();
	virtual ~CMenuControls();

	// Initialization/Termination
	DBOOL	Init(CClientDE *pClientDE, CMainMenus *pMainMenus,
				 CMenuBase *pParentMenu, DWORD dwMenuID, int nMenuHeight);
	void	Term();

	// Build the menu
	void	Build();	

	// Handle a keypress
	DBOOL	HandleKeyDown(int key, int rep);

	// Handle device tracking
	DBOOL	HandleDeviceTrack(DeviceInput *pInput, DDWORD dwNumDevices);

	// Intialize the list of controls
	DBOOL	InitControlList();

protected:
	// Adds the columns to the controls
	void	InitColumns();

	// Maps a device type to an index (DEVICETYPE_KEYBOARD=0, DEVICETYPE_MOUSE=1, etc);
	// -1 is returned if the devicetype cannot be mapped.
	int		GetDeviceIndexFromType(DWORD dwDeviceType);
	DWORD	GetDeviceTypeFromIndex(int nIndex);

	// Fills out the binding array for a specific input device.
	void	FillBindingArray(DWORD dwDeviceType);

	// Sets the binding array info for a device based on an action index
	void	SetBindingInfo(int nIndex, char *lpszNewControl, DWORD dwDeviceType);
	
	// Sets the text for the column control at a specific index
	void	SetControlText(int nIndex);

	// This finds lpszString in g_pControlActionName and returns the index to it.
	// -1 is returned if the string cannot be found.
	int		GetActionIndex(char *lpszString);

	// Finds the index of an action that has a specific control assigned to it.
	// -1 is returned if the action cannot be found
	int		GetControlIndex(char *lpszAction, DWORD dwDeviceType);
	
	// This binds a control to an action for a specific device type
	void	BindControlToAction(char *lpszControlName, char *lpszActionName, char *lpszDevice);	

	// Process a message
	DDWORD	OnCommand(DDWORD dwCommand, DDWORD dwParam1, DDWORD dwParam2);

protected:
	char	*m_deviceControlArray[MENU_CONTROLS_NUM_DEVICES][MENU_CONTROLS_NUM_ACTIONS];	
};

#endif // !defined(AFX_MENUCONTROLS_H__08A756C2_5C86_11D2_BDA2_0060971BDC6D__INCLUDED_)
