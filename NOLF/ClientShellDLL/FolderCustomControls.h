// FolderCustomControls.h: interface for the CFolderCustomControls class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _FOLDER_CUSTOM_CONTROLS_H_
#define _FOLDER_CUSTOM_CONTROLS_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"
#include "ClientUtilities.h"

#define TRACK_BUFFER_SIZE			8
#define FOLDER_CONTROLS_NUM_DEVICES	3
#define MAX_CONTROLS_PER_ACTION		2

struct FolderEntry
{
	FolderEntry()	{ nStringID = 0; nAction = 0; memset(strControlName, 0, sizeof(strControlName)); }

    uint32      nStringID;
	int			nAction;
	char		strControlName[FOLDER_CONTROLS_NUM_DEVICES][64];
};

class CFolderCustomControls : public CBaseFolder
{
public:
	CFolderCustomControls();
	virtual ~CFolderCustomControls();

    LTBOOL   Init(int nFolderID);
    LTBOOL   Render(HSURFACE hDestSurf);

	// Build the folder
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
    LTBOOL  InitControlList();
    void	FillControlList();
	void	SetControlText(int nIndex);
	void	InitColumns(int nFilter,int nFirstEntry,int nEntries);
    LTBOOL  HandleKeyDown(int key, int rep);
    LTBOOL  KeyRemappable (DeviceInput* pInput);
    uint32  GetControlType(uint32 deviceType, char *controlName);
    LTBOOL  SetCurrentSelection (DeviceInput* pInput);
    LTBOOL  CheckMouseWheel (DeviceInput* pInput);
    void    Bind(int commandIndex, char *lpszControlName, uint32 deviceType, uint8 diCode = 0);
    void    UnBind(int commandIndex, uint32 deviceType);
	CLTGUIColumnTextCtrl *GetControlFromEntryIndex(int nIndex);
	void	RebindWheel(int nUpAction,int nDownAction);

    LTBOOL          m_bControlChanged;
    LTBOOL          m_bWaitingForKey;
	int				m_nEntries;
	FolderEntry		m_pEntries[50]; // This must be at least g_kNumCommands + 1 in size

    LTFLOAT         m_fInputPauseTimeLeft;
	DeviceInput		m_pInputArray[TRACK_BUFFER_SIZE];

	LTIntPt			m_FilterPos;
	int				m_nActionWidth;
	int				m_nBufferWidth;
	int				m_nEqualsWidth;


};

#endif // _FOLDER_CUSTOM_CONTROLS_H_