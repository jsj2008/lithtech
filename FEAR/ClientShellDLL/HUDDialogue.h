// ----------------------------------------------------------------------- //
//
// MODULE  : HUDDialogue.h
//
// PURPOSE : HUDItem to display dialogue icons
//
// CREATED : 05/06/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUDDIALOGUE_H__
#define __HUDDIALOGUE_H__

#include "HUDItem.h"

//******************************************************************************************
//** HUD Ammo display
//******************************************************************************************
class CHUDDialogue : public CHUDItem
{
public:
	CHUDDialogue();
	virtual ~CHUDDialogue() {}

	virtual bool	Init();
	virtual void	Term();

	float GetFadeSpeed() const;

	virtual void	Render();
	virtual void	Update();
	virtual void	UpdateFade();
	virtual void	UpdateFlicker();
	virtual void	EndFlicker();
	virtual void	ScaleChanged();

	virtual void	UpdateLayout();

	virtual void	Show(const char* szIcon, uint32 nClientID);
	virtual void	ShowRecord(HRECORD hRec, uint32 nClientID);
	virtual void	Hide(const char* szIcon);
	virtual void	HideRecord(HRECORD hRec);
	virtual void	HideAll() { Hide(m_sIcon.c_str()); }

protected:

	virtual void	Show(const char* szIcon, HRECORD hRec, uint32 nClientID);
	virtual HRECORD	GetLayout();

private:

	std::string			m_sIcon;
	StopWatchTimer		m_Timer;

	float				m_fFlicker;

	LTVector2n			m_vNameOffset;
	CLTGUIString		m_Name;

	TextureReference	m_hInsignia;
	LTPoly_GT4			m_Insignia;
	LTRect2f			m_rfInsignia;

};

#endif //__HUDDIALOGUE_H__