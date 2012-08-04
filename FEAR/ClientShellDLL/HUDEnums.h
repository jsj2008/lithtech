// ----------------------------------------------------------------------- //
//
// MODULE  : HUDEnums.h
//
// PURPOSE : Definition of HUD Enums.
//
// CREATED : 01/10/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUDENUMS_H__
#define __HUDENUMS_H__

enum eHUDUpdateFlag
{
	kHUDNone			= 0x00000000,
	kHUDFrame			= 0x00000001,
	kHUDHealth			= 0x00000002,
	kHUDArmor			= 0x00000004,
	kHUDDamage			= 0x00000008,
	kHUDAmmo			= 0x00000010,
	kHUDVote			= 0x00000020,
	kHUDGrenade			= 0x00000040,
	kHUDControlPoint	= 0x00000080,
	kHUDDistance		= 0x00000100,
//	unused				= 0x00000200,
//	unused				= 0x00000400,
	kHUDRespawn			= 0x00000800,
	kHUDScores			= 0x00001000,
	kHUDPlayers			= 0x00002000,
	kHUDSwap			= 0x00004000,
	kHUDGear			= 0x00008000,
	kHUDWeapon			= 0x00010000,
	kHUDSpectator		= 0x00020000,
	kHUDAll				= 0xFFFFFFFF,
};

// Layers are rendered at different points in the pipeline.
enum EHUDRenderLayer
{
	eHUDRenderLayer_Back,
	eHUDRenderLayer_Front,
};

#endif//__HUDENUMS_H__

