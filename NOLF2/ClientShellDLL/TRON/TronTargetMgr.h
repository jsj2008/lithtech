// ----------------------------------------------------------------------- //
//
// MODULE  : TronTargetMgr.h
//
// PURPOSE : Definition of class to handle tracking what the player is aimed at.
//           Derived from CTargetMgr
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __TRON_TARGET_MGR_H__
#define __TRON_TARGET_MGR_H__

#include "TargetMgr.h"

class CTronTargetMgr : public CTargetMgr
{
public:
	CTronTargetMgr();
	virtual ~CTronTargetMgr();

	void Update();
	void HandleTargetPropertiesMessage(ILTMessage_Read *pMsg);

	bool IsEnergyRequired() {return m_bEnergyRequired;}
	int		GetEnergyRequired() {return m_nEnergyRequired;}
private:
	bool		m_bEnergyRequired;
	int			m_nEnergyRequired;
};

#endif