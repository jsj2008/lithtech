// ----------------------------------------------------------------------- //
//
// MODULE  : HUDActivate.h
//
// PURPOSE : HUD Item to display activation icon
//
// CREATED : 01/03/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUDACTIVATE_H__
#define __HUDACTIVATE_H__


//******************************************************************************************
//** HUD Activation display
//******************************************************************************************
class CHUDActivate : public CHUDItem
{
public:
	CHUDActivate();

	virtual bool		Init();

	virtual void        Render();
	virtual void        Update();

	virtual void        UpdateLayout();

private:
	bool	m_bDraw;

};


#endif  // __HUDACTIVATE_H__
