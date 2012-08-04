// ----------------------------------------------------------------------- //
//
// MODULE  : HUDItem.h
//
// PURPOSE : Definition of CHUDItem base class
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_ITEM_H
#define __HUD_ITEM_H

#include "ltbasedefs.h"


enum eHUDRenderLevel
{
	kHUDRenderNone = 0,		// always render
	kHUDRenderText,			// render when text is allowed (i.e. cinematics, etc.)
	kHUDRenderDead,			// render when text is allowed (i.e. cinematics, etc.)
	kHUDRenderFull			// render only when full HUD is rendered
};


//******************************************************************************************
//** Base HUD item
//******************************************************************************************
class CHUDItem
{
public:
	CHUDItem() {m_UpdateFlags = 0; m_eLevel = kHUDRenderFull;}
	virtual ~CHUDItem() {Term();}
	

    virtual LTBOOL      Init() {return LTTRUE;}
	virtual void		Term() {}

    virtual void        Render() = 0;
    virtual void        Update() = 0;

    virtual void        UpdateLayout() {}

	virtual uint32		GetUpdateFlags() { return m_UpdateFlags; }

	virtual	eHUDRenderLevel	GetRenderLevel() { return m_eLevel; }

	virtual void		SetRenderState();

protected:
	uint32				m_UpdateFlags;
	eHUDRenderLevel		m_eLevel;

};


#endif