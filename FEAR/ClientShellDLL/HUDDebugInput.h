// ----------------------------------------------------------------------- //
//
// MODULE  : HUDDebugInput.h
//
// PURPOSE : Enter debug input
//
// CREATED : 01/06/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUDDEBUGINPUT_H__
#define __HUDDEBUGINPUT_H__


#include "HUDItem.h"
#include "LTGUIMgr.h"

const int kMaxDebugLength = 96;

//******************************************************************************************
//** HUD Debug display
//******************************************************************************************
class CHUDDebugInput : public CHUDItem
{
public:
	CHUDDebugInput();

	virtual bool	Init();
	virtual void	Term();
	virtual void	OnExitWorld();

	virtual void	Render();
	virtual void	Update();
	virtual void	ScaleChanged();		

	virtual void	UpdateLayout();

	// Handles a key press
    bool           HandleKeyDown(int key, int rep);
    bool           HandleChar(wchar_t c);


	void		Show(bool bShow);
	bool		IsVisible() {return m_bVisible;}

private:
	void		Send();


private:

	CLTGUIEditCtrl	m_EditCtrl;

	bool			m_bVisible;

	uint32			m_nInputWidth;

	wchar_t			m_szDebugStr[kMaxDebugLength];

};

#endif  // __HUDDEBUGINPUT_H__
