// ----------------------------------------------------------------------- //
//
// MODULE  : TronHUDMgr.h
//
// PURPOSE : Definition of derived CHUDMgr class
//
// CREATED : 11/5/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __TRONHUDMGR_H
#define __TRONHUDMGR_H

#include "HUDMgr.h"		// base class

#include "TRONPlayerStats.h"

// TO2 dependencies
#include "HUDCrosshair.h"
#include "HUDDamage.h"
#include "HUDWeapons.h"

// Tron dependencies
#include "HUDPermissions.h"
#include "HUDVersion.h"
#include "HUDHealthEnergy.h"
#include "HUDArmor.h"
#include "HUDEnergyTransfer.h"
#include "HUDProgress.h"
#include "HUDChooser.h"
#include "HUDWeapon.h"

//******************************************************************************************
//** Tron HUD Manager
//******************************************************************************************
class CTronHUDMgr: public CHUDMgr
{
public:

	CTronHUDMgr();
	~CTronHUDMgr();

	LTBOOL		Init();

protected:

	CHUDCrosshair		m_Crosshair;
	CHUDWeapons			m_Weapons;

	// New Tron items
	CHUDPermissions		m_Permissions;
	CHUDVersion			m_Version;
	CHUDHealthEnergy	m_HealthEnergy;
	CHUDArmor			m_Armor;
	CHUDEnergyTransfer	m_EnergyTrans;
	CHUDProgress		m_Progress;
	CHUDWpnChooser		m_WpnChooser;
	CHUDAmmoChooser		m_AmmoChooser;
	CHUDWeapon			m_Weapon;
};
extern CHUDCrosshair*		g_pCrosshair;

#endif // __TRONHUDMGR_H
