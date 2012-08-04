#ifndef __NEWGAMEMENU_H
#define __NEWGAMEMENU_H

#include "BaseMenu.h"

class CNewGameMenu : public CBaseMenu
{
public:

	virtual LTBOOL		Init (ILTClient* pClientDE, CRiotMenu* pRiotMenu, CBaseMenu* pParent, int nScreenWidth, int nScreenHeight);
	virtual void		Reset();
	virtual void		ScreenDimsChanged (int nScreenWidth, int nScreenHeight);

	virtual LTBOOL		LoadAllSurfaces()		{ return LoadSurfaces(); }
	virtual void		UnloadAllSurfaces()		{ UnloadSurfaces(); }

	virtual void		Return();

protected:

	virtual LTBOOL		LoadSurfaces();
	virtual void		UnloadSurfaces();

protected:

};

#endif
