// ------------------------------------------------------------------------------------------ //
//
// MODULE  : HUDFocus.h
//
// PURPOSE : Definitition of HUD Focus display
//
// CREATED : 06/15/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ------------------------------------------------------------------------------------------ //

#ifndef __HUDFOCUS_H__
#define __HUDFOCUS_H__

#ifndef __HUDITEM_H__
#include "HUDItem.h"
#endif//__HUDITEM_H__

// ****************************************************************************************** //

class CHUDFocus : public CHUDItem
{
	public:

		CHUDFocus();

		virtual bool		Init();
		virtual void		Term();

		virtual void        Render();
		virtual void        Update();

		virtual void        UpdateLayout();
};

//******************************************************************************************

#endif//__HUDFOCUS_H__

