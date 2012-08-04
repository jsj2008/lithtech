#ifndef __DETAILSETTINGSMENU_H
#define __DETAILSETTINGSMENU_H

#include "BaseMenu.h"
#include "RiotSettings.h"

class CDetailSettingsMenu : public CBaseMenu
{
public:

	CDetailSettingsMenu();

	virtual LTBOOL		Init (ILTClient* pClientDE, CRiotMenu* pRiotMenu, CBaseMenu* pParent, int nScreenWidth, int nScreenHeight);
	virtual void		ScreenDimsChanged (int nScreenWidth, int nScreenHeight);
	virtual void		Reset();
	
	virtual LTBOOL		LoadAllSurfaces()			{ return LoadSurfaces(); }
	virtual void		UnloadAllSurfaces()			{ UnloadSurfaces(); }
	
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

protected:

	virtual LTBOOL		LoadSurfaces();
	virtual void		UnloadSurfaces();

	virtual void		PostCalculateMenuDims();

	void				AdjustSetting (int nSelection, int nChange);
	int					GetSettingStringID (int nSetting);
	void				ImplementDetailSetting (int nSetting);

protected:

	int					m_nSecondColumn;

	GENERIC_ITEM		m_DetailSetting[RS_SUBDET_LAST + 1];

	LTFLOAT				m_fOriginalTextureDetail;
};

#endif
