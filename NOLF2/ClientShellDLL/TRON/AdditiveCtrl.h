//-------------------------------------------------------------------------
//
// MODULE  : AdditiveCtrl.h
//
// PURPOSE : GUI element for interacting with the visual representation of
//			 additives
//
// CREATED : 4/4/02 - for TRON
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
//-------------------------------------------------------------------------

#ifndef __ADDITIVE_CTRL_H
#define __ADDITIVE_CTRL_H

#include "ScreenSpriteMgr.h"
#include "BaseScaleFX.h"
#include "TronLayoutMgr.h"
//#include "SubroutineMgr.h"
#include "RatingMgr.h"

class CAdditiveCtrl
{
public:
	CAdditiveCtrl();
	~CAdditiveCtrl();

	void			Term();

	LTBOOL			OnMouseMove(int mx, int my);

	ADDITIVE *		GetHotAdditive() {return m_pHotAdditive;}

private:
	ADDITIVE *		m_pHotAdditive;
};
#endif