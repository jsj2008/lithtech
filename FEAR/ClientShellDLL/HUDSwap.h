// ----------------------------------------------------------------------- //
//
// MODULE  : HUDSwap.h
//
// PURPOSE : HUDItem to display weapon swap info
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_SWAP_H
#define __HUD_SWAP_H

#include "HUDItem.h"

class CHUDSwap : public CHUDItem
{
public:
	CHUDSwap();
	virtual bool	Init();
	virtual void	Term();

	virtual void	Render();
	virtual void	Update();
	virtual void	ScaleChanged();

	virtual void	UpdateLayout();

	static void		UpdateTriggerName();


private:
	bool		m_bDraw;

	static std::wstring	m_wsTrigger;

	LTObjRef	m_hLastIconTarget;
	bool		m_bShowIcon;

	uint32		m_cSwapColor;
	uint32		m_cSavedTextColor;
	uint32		m_cSavedIconColor;

};

#endif