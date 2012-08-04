// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenPreload.h
//
// PURPOSE : Interface screen to be displayed  before loading a level
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#if !defined(_SCREEN_PRELOAD_H_)
#define _SCREEN_PRELOAD_H_

#include "BaseScreen.h"

class CScreenPreload : public CBaseScreen
{
public:
	CScreenPreload();
	virtual ~CScreenPreload();

	// Build the screen
    virtual bool	Build();
    virtual void	OnFocus(bool bFocus);
	virtual void	Escape();

	virtual bool	UpdateInterfaceSFX();

protected:

	virtual void	CreateInterfaceSFX();

	void			FirstUpdate( );

	HRECORD			m_layout;
	bool			m_bFirstUpdate;

};

#endif // !defined(_SCREEN_PRELOAD_H_)