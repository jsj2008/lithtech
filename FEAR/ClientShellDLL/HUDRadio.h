// ----------------------------------------------------------------------- //
//
// MODULE  : HUDRadio.h
//
// PURPOSE : Definition of CHUDRadio to display transmission messages
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_RADIO_H
#define __HUD_RADIO_H

#include "HUDItem.h"
#include "ClientServerShared.h"

#define MAX_RADIO_CHOICES 9
//******************************************************************************************
//** HUD Radio dialog
//******************************************************************************************
class CHUDRadio : public CHUDItem
{
public:
	CHUDRadio();
	

    virtual bool      Init();
	virtual void		Term();

    virtual void        Render();
    virtual void        Update();
	virtual void		ScaleChanged();

	virtual void        UpdateLayout();

	//hide or show
	void	Show(bool bShow);
	bool	IsVisible() {return m_bVisible;}

	bool	Choose(uint8 nChoice);

	void	ResetText();

	void	SetBindings(bool bSet);
	bool	AreBindingsSet() const { return m_bBindingsSet; }

protected:
	void	SetText(HRECORD hSet);
	

	uint8		m_nNumChoices;
	//runtime info
	bool		m_bVisible;

	//set of weapons we are carrying
	typedef std::vector<HRECORD, LTAllocator<HRECORD, LT_MEM_TYPE_GAMECODE> > RecordArray;
	RecordArray	m_vecHistory;
	HRECORD		m_hCurrent;

	uint16		m_nWidth;
	
	bool		m_bBindingsSet;

	CLTGUIWindow	    m_Dlg;
	CLTGUIColumnCtrl*	m_pText[MAX_RADIO_CHOICES+1];
	std::string			m_sBroadcast[MAX_RADIO_CHOICES];
	HRECORD				m_hSubset[MAX_RADIO_CHOICES];

	uint32				m_nSavedCommands[MAX_RADIO_CHOICES+1];

};



#endif