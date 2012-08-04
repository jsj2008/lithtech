// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponFXTypes.h
//
// PURPOSE : Weapon FX types contains the different types and enums associated
//			 with the weapon fx class.
//
// CREATED : 2/22/98
//
// (c) 1997-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __WEAPON_FX_TYPES_H__
#define __WEAPON_FX_TYPES_H__

// Weapon fx (impact and fire)...

#define	WFX_MARK		(1<<0)
#define	WFX_TRACER		(1<<1) 
#define WFX_MUZZLE		(1<<2)
#define WFX_SHELL		(1<<3)
#define WFX_FIRESOUND	(1<<4)
#define WFX_EXITMARK	(1<<5)
#define WFX_ALTFIRESND	(1<<6)
#define WFX_SILENCED	(1<<7)
#define WFX_IMPACTDING	(1<<8)	// Can be figured out on clients (If MPGame and HitCharacter and NotLocalImpact)


// Projectile FX...

#define PFX_FLYSOUND	(1<<0)

#endif // __WEAPON_FX_TYPES_H__
