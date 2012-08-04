#ifndef __KEYBOARDMENU_H
#define __KEYBOARDMENU_H

#include "BaseMenu.h"

#define TRACK_BUFFER_SIZE	8

struct MenuEntry
{
	MenuEntry()	{ nStringID = 0; nAction = 0; memset(strControlName, 0, 64); 
				  hSurface = NULL; hSurfaceSelected = LTNULL; szSurface.cx = 0; szSurface.cy = 0; 
				  hSetting = NULL; hSettingSelected = LTNULL; }

	uint32		nStringID;
	int			nAction;
	char		strControlName[64];

	HSURFACE	hSurface;
	HSURFACE	hSurfaceSelected;
	CSize		szSurface;

	HSURFACE	hSetting;
	HSURFACE	hSettingSelected;
};

class CKeyboardMenu : public CBaseMenu
{
public:

	CKeyboardMenu()		{ m_bWaitingForKeypress = LTFALSE; m_fInputPauseTimeLeft = 0.0f; memset (m_pInputArray, 0, sizeof(DeviceInput) * TRACK_BUFFER_SIZE);
						  m_nSecondColumn = 0; m_nSpacing = 0; m_nTopItem = 0; m_nEntries = 0;	}
	~CKeyboardMenu();

	virtual LTBOOL		Init (ILTClient* pClientDE, CRiotMenu* pRiotMenu, CBaseMenu* pParent, int nScreenWidth, int nScreenHeight);
	virtual void		ScreenDimsChanged (int nScreenWidth, int nScreenHeight);
	virtual void		Reset()		{ CBaseMenu::Reset(); m_nTopItem = 0; }
	
	virtual LTBOOL		LoadAllSurfaces()		{ return LoadSurfaces(); }
	virtual void		UnloadAllSurfaces()		{ UnloadSurfaces(); }
	
	virtual void		HandleInput (int vKey);
	virtual void		Up();
	virtual void		Down();
	virtual void		Left()			{}
	virtual void		Right()			{}
	virtual void		PageUp();
	virtual void		PageDown();
	virtual void		Home();
	virtual void		End();
	virtual void		Return();
	virtual void		Esc()			{ if (m_bWaitingForKeypress || m_fInputPauseTimeLeft) return; CBaseMenu::Esc(); }

	virtual void		Draw (HSURFACE hScreen, int nScreenWidth, int nScreenHeight, int nTextOffset = 0);

protected:

	virtual LTBOOL		LoadSurfaces();
	virtual void		UnloadSurfaces();

	virtual void		PostCalculateMenuDims();

	virtual void		CheckSelectionOffMenuTop();
	virtual void		CheckSelectionOffMenuBottom();
	
	LTBOOL				SetCurrentSelection (DeviceInput* pInput);
	void				ClearKBBindings();
	LTBOOL				KeyRemappable (DeviceInput* pInput);

protected:

	LTBOOL				m_bWaitingForKeypress;
	LTFLOAT				m_fInputPauseTimeLeft;
	DeviceInput			m_pInputArray[TRACK_BUFFER_SIZE];

	int					m_nSecondColumn;
	int					m_nSpacing;

	int					m_nEntries;
	MenuEntry			m_pEntries[NUM_COMMANDS + 1];

	GENERIC_ITEM		m_RestoreDefaults;
	GENERIC_ITEM		m_Back;
};

#endif
