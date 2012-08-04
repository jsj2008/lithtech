// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponFXTypes.h
//
// PURPOSE : Weapon FX types contains the different types and enums associated
//			 with the weapon fx class.
//
// CREATED : 2/22/98
//
// ----------------------------------------------------------------------- //

#ifndef __WEAPON_FX_TYPES_H__
#define __WEAPON_FX_TYPES_H__


#define	WFX_MARK		(1<<0)
#define	WFX_SPARKS		(1<<1)
#define	WFX_SMOKE		(1<<2)
#define	WFX_TRACER		(1<<3)
#define WFX_MUZZLE		(1<<4)
#define WFX_SHELL		(1<<5)
#define WFX_LIGHT		(1<<6)
#define WFX_FIRESOUND	(1<<7)

// Particle trail types...
// Bits 0-6 types
// Bit 7 is small indicator
#define PT_SMOKE		(1<<0)
#define PT_BLOOD		(1<<1)
#define PT_GIBSMOKE		(1<<2)
#define PT_SMALL		(1<<7)

#endif // __WEAPON_FX_TYPES_H__