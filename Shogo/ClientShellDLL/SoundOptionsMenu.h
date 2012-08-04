#ifndef __SOUNDOPTIONSMENU_H
#define __SOUNDOPTIONSMENU_H

#include "BaseMenu.h"
#include "Slider.h"

class CSoundOptionsMenu : public CBaseMenu
{
public:

	CSoundOptionsMenu();

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

protected:

	virtual LTBOOL		LoadSurfaces();
	virtual void		UnloadSurfaces();

	virtual void		PostCalculateMenuDims();

protected:

	int					m_nSecondColumn;
	
	char				m_strOrigMusicSource[32];

	GENERIC_ITEM		m_MusicSource;
	GENERIC_ITEM		m_SoundFx;
	GENERIC_ITEM		m_SoundQuality;
	CSlider				m_sliderMusicVolume;
	CSlider				m_sliderSoundVolume;
};

#endif
