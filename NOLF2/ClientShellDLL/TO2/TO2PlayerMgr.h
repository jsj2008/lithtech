// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerMgr.h
//
// PURPOSE : Definition of class to handle managment of player and camera.
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __TO2_PLAYER_MGR_H__
#define __TO2_PLAYER_MGR_H__

#include "PlayerMgr.h"



class CTO2PlayerMgr : public CPlayerMgr
{
public:
	CTO2PlayerMgr();
	virtual ~CTO2PlayerMgr();

	virtual LTBOOL	Init();

	virtual LTBOOL	OnCommandOn(int command);

	virtual void	OnEnterWorld();

protected:

	int		m_nFlashlightID;
	

};

#endif
