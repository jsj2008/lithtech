// FolderMgr.h: interface for the CFolderMgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FOLDERMGR_H__88EE6E20_1515_11D3_B2DB_006097097C7B__INCLUDED_)
#define AFX_FOLDERMGR_H__88EE6E20_1515_11D3_B2DB_006097097C7B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "InterfaceResMgr.h"

#define MAX_FOLDER_HISTORY 20

class CGameClientShell;
class CBaseFolder;

enum eFolderID
{
	FOLDER_ID_NONE,
	FOLDER_ID_MAIN,
	FOLDER_ID_SINGLE,
	FOLDER_ID_GALLERY,
	FOLDER_ID_ESCAPE,
	FOLDER_ID_OPTIONS,
	FOLDER_ID_DISPLAY,
	FOLDER_ID_AUDIO,
	FOLDER_ID_GAME,
	FOLDER_ID_PERFORMANCE,
	FOLDER_ID_ADVDISPLAY,
	FOLDER_ID_TEXTURE,
	FOLDER_ID_EFFECTS,
	FOLDER_ID_CONTROLS,
	FOLDER_ID_CUST_CONTROLS,
	FOLDER_ID_WPN_CONTROLS,
	FOLDER_ID_CROSSHAIR,
	FOLDER_ID_JOYSTICK,
	FOLDER_ID_MOUSE,
	FOLDER_ID_KEYBOARD,
	FOLDER_ID_CUSTOM_LEVEL,
	FOLDER_ID_FAVORITE_LEVEL,
	FOLDER_ID_NEW,
	FOLDER_ID_DIFFICULTY,
	FOLDER_ID_LOAD,
	FOLDER_ID_SAVE,
	FOLDER_ID_BRIEFING,
	FOLDER_ID_MP_BRIEFING,
	FOLDER_ID_MP_SUMMARY,
	FOLDER_ID_OBJECTIVES,
	FOLDER_ID_WEAPONS,
	FOLDER_ID_GADGETS,
	FOLDER_ID_MODS,
	FOLDER_ID_GEAR,
	FOLDER_ID_INVENTORY,
	FOLDER_ID_VIEW_INV,
	FOLDER_ID_MISSION,
	FOLDER_ID_MULTI_MISSION,
	FOLDER_ID_STATS,
	FOLDER_ID_INTEL,
	FOLDER_ID_SUMMARY,
	FOLDER_ID_AWARDS,
	FOLDER_ID_FAILURE,
	FOLDER_ID_MULTIPLAYER,
	FOLDER_ID_PLAYER,
	FOLDER_ID_JOIN,
	FOLDER_ID_JOIN_LAN,
	FOLDER_ID_HOST,
	FOLDER_ID_HOST_OPTIONS,
	FOLDER_ID_HOST_LEVELS,
	FOLDER_ID_LOADSCREEN_SINGLE,
	FOLDER_ID_LOADSCREEN_MULTI,

	//this must be the last id
	FOLDER_ID_UNASSIGNED,
};



class CFolderMgr
{
public:
	CFolderMgr();
	virtual ~CFolderMgr();
    LTBOOL               Init (ILTClient* pClientDE, CGameClientShell* pClientShell);
	void				Term();

	void				HandleKeyDown (int vkey, int rep);
	void				HandleKeyUp (int vkey);
	void				HandleChar (char c);

	// Mouse messages
	void				OnLButtonDown(int x, int y);
	void				OnLButtonUp(int x, int y);
	void				OnLButtonDblClick(int x, int y);
	void				OnRButtonDown(int x, int y);
	void				OnRButtonUp(int x, int y);
	void				OnRButtonDblClick(int x, int y);
	void				OnMouseMove(int x, int y);

    LTBOOL               ForceFolderUpdate(eFolderID folderID);

	void				OnEnterWorld();
	void				OnExitWorld();

	eFolderID			GetCurrentFolderID()		{return m_eCurrentFolderID;}
	eFolderID			GetLastFolderID()			{return m_eLastFolderID;}
    LTBOOL              SetCurrentFolder(eFolderID folderID);
    LTBOOL              PreviousFolder();
	void				EscapeCurrentFolder();
	void				ExitFolders();

	// Renders the folder to a surface
    LTBOOL               Render(HSURFACE hDestSurf);
	void				UpdateInterfaceSFX();

	CBaseFolder*		GetFolderFromID(eFolderID folderID);
	void				SkipOutfitting(void);

private:

	void				AddFolder(eFolderID folderID);

private:
	//important references
    ILTClient*          m_pClientDE;
	CGameClientShell*	m_pClientShell;


	int				m_nHistoryLen;
	eFolderID		m_eFolderHistory[MAX_FOLDER_HISTORY];
	eFolderID		m_eCurrentFolderID;
	eFolderID		m_eLastFolderID;
	CBaseFolder*	m_pCurrentFolder;		// The current folder
	void			SwitchToFolder(CBaseFolder *pNewFolder, LTBOOL bBack = LTFALSE);

	//folders
	CMoArray<CBaseFolder *>	m_folderArray;			// Pointer to each folder
};

#endif // !defined(AFX_FOLDERMGR_H__88EE6E20_1515_11D3_B2DB_006097097C7B__INCLUDED_)