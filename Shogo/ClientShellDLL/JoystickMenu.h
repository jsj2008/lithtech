#ifndef __JOYSTICKMENU_H
#define __JOYSTICKMENU_H

#include "BaseMenu.h"
#include "Slider.h"

#define NUM_JOYBUTTONS		10

class CJoystickMenu : public CBaseMenu
{
public:

	CJoystickMenu();

	virtual LTBOOL		Init (ILTClient* pClientDE, CRiotMenu* pRiotMenu, CBaseMenu* pParent, int nScreenWidth, int nScreenHeight);
	virtual void		ScreenDimsChanged (int nScreenWidth, int nScreenHeight);
	virtual void		Reset();
	
	virtual LTBOOL		LoadAllSurfaces()		{ return LoadSurfaces(); }
	virtual void		UnloadAllSurfaces()		{ UnloadSurfaces(); }
	
	virtual void		Up();
	virtual void		Down();
	virtual void		Left();
	virtual void		Right();
	virtual void		PageUp();
	virtual void		PageDown();
	virtual void		Home();
	virtual void		End();
	virtual void		Return();
	virtual void		Esc();

	virtual void		Draw (HSURFACE hScreen, int nScreenWidth, int nScreenHeight, int nTextOffset = 0);

	LTBOOL				JoystickEnabled()		{ return m_bJoystickEnabled; }
	LTBOOL				JoystickMenuDisabled()	{ return m_bJoystickMenuDisabled; };

protected:

	virtual LTBOOL		LoadSurfaces();
	virtual void		UnloadSurfaces();

	virtual void		PostCalculateMenuDims();

	LTBOOL				ChangeButtonSettingSurface (int nSelection, int* pSelection, int nChange);
	LTBOOL				BindButtonToCommand (int nButton, int nSelection);
	LTBOOL				ReBindAxis (int nDirection);
	LTBOOL				ImplementJoyLook (ILTClient* pClientDE, LTBOOL bJoyLook);

protected:

	int					m_nSecondColumn;

	LTBOOL				m_bJoystickEnabled;

	LTBOOL				m_bJoystickMenuDisabled;

	LTFLOAT				m_nXAxisMin;
	LTFLOAT				m_nXAxisMax;
	LTFLOAT				m_nYAxisMin;
	LTFLOAT				m_nYAxisMax;

	int					m_nJoyUpSelection;
	int					m_nJoyDownSelection;
	int					m_nJoyLeftSelection;
	int					m_nJoyRightSelection;
	int					m_nButtonSelections[NUM_JOYBUTTONS];

	GENERIC_ITEM		m_JoySettings[7 + NUM_JOYBUTTONS];
};

#endif
