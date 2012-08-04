#ifndef __LOADSAVEDLEVELMENU_H
#define __LOADSAVEDLEVELMENU_H

#include "BaseMenu.h"

#define MAXLOADLEVELS		(__min (MAX_GENERIC_ITEMS, 12))

class CLoadSavedLevelMenu : public CBaseMenu
{
public:

	virtual LTBOOL		Init (ILTClient* pClientDE, CRiotMenu* pRiotMenu, CBaseMenu* pParent, int nScreenWidth, int nScreenHeight);
	virtual void		ScreenDimsChanged (int nScreenWidth, int nScreenHeight);
	virtual void		Reset();

	virtual LTBOOL		LoadAllSurfaces()		{ return LoadSurfaces(); }
	virtual void		UnloadAllSurfaces()		{ UnloadSurfaces(); }

	virtual void		Up();
	virtual void		Down();
	virtual void		End();

	virtual void		Return();
	virtual void		Esc();

	virtual void		Draw (HSURFACE hScreen, int nScreenWidth, int nScreenHeight, int nTextOffset = 0);

protected:

	virtual LTBOOL		LoadSurfaces();
	virtual void		UnloadSurfaces();

	virtual void		PostCalculateMenuDims();

protected:

	int					m_nSecondColumn;

	GENERIC_ITEM		m_DateTime[MAXLOADLEVELS];
};

#endif
