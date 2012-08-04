// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponFXTypes.h
//
// PURPOSE : Weapon FX types contains the different types and enums associated
//			 with the weapon fx class.
//
// CREATED : 2/22/98
//
// (c) 1997-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __WEAPON_FX_TYPES_H__
#define __WEAPON_FX_TYPES_H__

// Weapon fx (impact and fire)...

#define	WFX_MARK		(1<<0)

#define WFX_TINTSCREEN	(1<<2)
#define WFX_BLASTMARK	(1<<3)

#define	WFX_TRACER		(1<<4)
#define WFX_MUZZLE		(1<<5)
#define WFX_SHELL		(1<<6)
#define WFX_LIGHT		(1<<7)
#define WFX_FIRESOUND	(1<<8)
#define WFX_EXITMARK	(1<<9)
#define WFX_EXITDEBRIS	(1<<10)
#define WFX_ALTFIRESND	(1<<11)
#define WFX_SILENCED	(1<<12)
#define WFX_IMPACTONSKY	(1<<13)
#define WFX_IMPACTDING	(1<<14)

#define WFX_EXITSURFACE (WFX_EXITMARK | WFX_EXITDEBRIS)

// Projectile FX...

#define	PFX_SMOKETRAIL	(1<<0)
#define	PFX_FLARE		(1<<1)
#define PFX_LIGHT		(1<<2)
#define PFX_FLYSOUND	(1<<3)

// Particle trail types...
// Bits 0-6 types

#define PT_BLOOD		(1<<0)
#define PT_SMOKE		(1<<1)
#define PT_SMOKE_LONG	(1<<2)
#define PT_SMOKE_BLACK	(1<<3)
#define PT_GIBSMOKE		(1<<4)

#endif // __WEAPON_FX_TYPES_H__