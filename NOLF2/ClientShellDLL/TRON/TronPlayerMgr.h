// ----------------------------------------------------------------------- //
//
// MODULE  : TronPlayerMgr.h
//
// PURPOSE : Definition of class to handle managment of player and camera.
//			 Derived from CPlayerMgr
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __TRON_PLAYER_MGR_H__
#define __TRON_PLAYER_MGR_H__

#include "PlayerMgr.h"
#include "TronTargetMgr.h"

class CTronPlayerMgr;
extern CTronPlayerMgr* g_pTronPlayerMgr;


class CTronPlayerMgr : public CPlayerMgr
{
public:
	CTronPlayerMgr();
	virtual ~CTronPlayerMgr();

	LTBOOL Init();
	void Term();

	virtual float GetJumpVelocity(float fJumpVel, float fSuperJumpVel);

    LTBOOL	OnMessage(ILTMessage_Read *pMsg);

	LTBOOL	OnCommandOn(int command);
	LTBOOL	OnCommandOff(int command);

	void	Pause(LTBOOL bPause);
	void	Update();

	void	HandleMsgStartEnergyTransfer(ILTMessage_Read * pMsg);

	CTronTargetMgr * GetTronTargetMgr() {return (CTronTargetMgr*)m_pTargetMgr;}
	virtual void InitTargetMgr();
	
	int		GetEnergyRequired() {return (int)m_fEnergyRequired;}
	float	GetPercentEnergyTransferred();

	bool	IsTransferringEnergy() {return m_bTransferringEnergy;}
private:
	bool	m_bTransferringEnergy;
	float	m_fEnergyTransferred;
	float	m_fEnergyRequired;

	LTBOOL				m_bPause;
	float				m_fLastUpdateTime;
};

#endif
