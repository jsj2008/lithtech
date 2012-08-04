//----------------------------------------------------------
//
// MODULE  : WeaponDefs.cpp
//
// PURPOSE : Weapon definition tables.
//
// CREATED : 9/31/98
//
//----------------------------------------------------------

// Includes....
#include "server_de.h"
#include "WeaponDefs.h"
#include "ClientRes.h"



// initial values for standard weapons
WeaponData g_WeaponDefaults[WEAP_MAXWEAPONTYPES] = 
{
	// Beretta (Pistol)
	{
		"Models\\Weapons\\Beretta_pv.abc",	// View Model
		"Models\\Weapons\\lh_Beretta_pv.abc",	// View Model
		"Skins\\Weapons\\C_Beretta_pv_t.dtx",	// View Skin
		"Models\\Powerups\\beretta_pu.abc",	// 3rd person model
		"Skins\\Powerups\\beretta_pu.dtx",	// 3rd person skin
		WEAP_BERETTA,				// Type
		TYPE_PISTOL,				// Fire type
		AMMO_BULLET,				// Ammo type
		1,							// Ammo Use
		1,							// Alt Ammo Use
		8.0f,						// Min Damage
		12.0f,						// Max Damage
		10.0f,						// Min Alt damage
		20.0f,						// Max Alt damage
		0.33f,						// Reload time
		0.5f,						// Alt reload time
		{75.0f, 25.0f},				// Spread
		{250.0f, 150.0f},			// Alt Spread
		0.0f,						// Projectile Velocity
		0.0f,						// Alt Projectile Velocity
		3500.0f,					// Range
		3500.0f,					// Alt Range
		1,							// Shots per fire
		1,							// Alt Shots per fire
		1,							// Strength or Magic required
		2,							// Two-handed Strength or Magic required
		0,							// Damage Radius (projectiles)
		0,							// Alt Damage Radius (projectiles)
		DFALSE,						// Alt fire zoom
		DTRUE,						// Semi-auto
		"Sounds\\weapons\\beretta\\fire.wav",	// Fire sound
		"Sounds\\weapons\\beretta\\fire.wav",	// Fire sound for alt fire
		"",		// Empty weapon sound
		"",		// Alt Empty weapon sound
		NULL,						// Projectile class
		NULL,						// Alt Projectile class
		100,						// Muzzle Flash radius
		{1.0f, 0.68f, 0.35f},		// Muzzle Flash color
		// Client info
		"Beretta 92F",				// Name
		IDS_WEAPON_BERETTA,			// Name ID (resource)
		"Sprites\\pistolflare.spr",	// flash sprite name
		"Sprites\\pistolflare.spr",	// alt flash sprite name
		0.01f,						// Flash duration
		0.12f,						// Flash scale
		{0.0f, 0.0f, 0.0f},			// 3rd person gun offset
		{0.6f, -0.6f, 1.9f},		// Gun Offset
		{0.15f, 0.1f, 2.0f},		// Muzzle Offset
		{0.03f, 0.03f, 2.55f},		// Recoil
		{7.5f, -5.6f, 28.0f},		// Flash Offset
		0.625f,						// Eject interval
		0.0f,						// View kick
		DFALSE,						// View kick is Cumulative
		DFALSE,						// Animation looping
		DTRUE,						// Alt anim looping
		"interface\\statusbar\\weapons\\beretta.pcx",	// Icon for status bar
		"interface\\statusbar\\weapons\\beretta_h.pcx",	// Highlighted icon for status bar
	},
	// Ingram Mac-10 Sub-machine Gun
	{
		"Models\\Weapons\\Mac10_pv.abc",	// Model
		"Models\\Weapons\\lh_Mac10_pv.abc",	// Model
		"Skins\\Weapons\\C_Mac10_pv_t.dtx",	// Skin
		"Models\\Powerups\\Mac10_pu.abc",	// 3rd person model
		"Skins\\Powerups\\Mac10_pu.dtx",	// 3rd person skin
		WEAP_SUBMACHINEGUN,			// Type
		TYPE_PISTOL,				// Fire type
		AMMO_BULLET,				// Ammo type
		1,							// Ammo Use
		1,							// Alt Ammo Use
		8.0f,						// Min Damage
		12.0f,						// Max Damage
		8.0f,						// Min Alt damage
		12.0f,						// Max Alt damage
		0.1f,						// Reload time
		0.5f,						// Alt reload time
		{200.0f, 100.0f},			// Spread
		{75.0f, 25.0f},				// Alt Spread
		0.0f,						// Projectile Velocity
		0.0f,						// Alt Projectile Velocity
		3500.0f,					// Range
		3500.0f,					// Alt Range
		1,							// Shots per fire
		1,							// Alt Shots per fire
		2,							// Strength or Magic required
		3,							// Two-handed Strength or Magic required
		0,							// Damage Radius (projectiles)
		0,							// Alt Damage Radius (projectiles)
		DFALSE,						// Alt fire zoom
		DFALSE,						// Semi-auto
		"Sounds\\weapons\\mac10\\fire.wav",	// Fire sound
		"Sounds\\weapons\\mac10\\alt.wav",	// Alt Fire sound
		"",	// Empty weapon sound
		"",	// Alt Empty weapon sound
		NULL,						// Projectile class
		NULL,						// Alt Projectile class
		100,						// Muzzle Flash radius
		{1.0f, 0.68f, 0.35f},		// Muzzle Flash color
		"Ingram Mac-10",			// Name
		IDS_WEAPON_MAC10,			// Name ID (resource)
		"Sprites\\mac10flare.spr",	// flash sprite name
		"Sprites\\mac10flare.spr",	// alt flash Sprite name
		0.01f,						// Flash duration
		0.11f,						// Flash scale
		{0.0f, 0.0f, 0.0f},			// 3rd person gun offset
		{0.8f, -0.7f, 2.0f},		// Gun Offset
		{1.0f, 0.0f, 1.0f},			// Muzzle Offset
		{0.03f, 0.03f, 0.15f},		// Recoil
		{8.0f, -5.75f, 28.0f},		// Flash position
		0.625f,						// Eject interval
		0.0f,						// View kick
		DFALSE,						// View kick is Cumulative
		DTRUE,						// Animation looping
		DTRUE,						// Alt anim looping
		"interface\\statusbar\\weapons\\mac10.pcx",		// Icon for status bar
		"interface\\statusbar\\weapons\\mac10_h.pcx",	// Highlighted icon for status bar
	},
	// Flaregun
	{
		"Models\\Weapons\\Flarepistol_pv.abc",	// Model
		"Models\\Weapons\\lh_Flarepistol_pv.abc",	// Model
		"Skins\\Weapons\\C_Flare_pv_t.dtx",	// Skin
		"Models\\Powerups\\Flarepistol_pu.abc",	// 3rd person model
		"Skins\\Powerups\\Flarepistol_pu.dtx",	// 3rd person skin
		WEAP_FLAREGUN,			// Type
		TYPE_PISTOL,			// Fire type
		AMMO_FLARE,				// Ammo type
		1,						// Ammo Use
		8,						// Alt Ammo Use
		10.0f,					// Min Damage
		15.0f,					// Max Damage
		20.0f,					// Min Alt damage
		30.0f,					// Max Alt damage
		0.75f,					// Reload time
		1.5f,					// Alt reload time
		{0.0f, 0.0f},			// Spread
		{0.0f, 0.0f},			// Alt Spread
		2000.0f,				// Projectile Velocity
		2000.0f,				// Alt Projectile Velocity
		5000.0f,				// Range
		5000.0f,				// Alt Range
		1,						// Shots per fire
		1,						// Alt Shots per fire
		2,						// Strength or Magic required
		3,						// Two-handed Strength or Magic required
		100,					// Damage Radius (projectiles)
		100,					// Alt Damage Radius (projectiles)
		DFALSE,					// Alt fire zoom
		DFALSE,					// Semi-auto
		"Sounds\\Weapons\\flare\\fire.wav",	// Fire sound
		"Sounds\\Weapons\\flare\\alt.wav",	// Alt Fire sound
		"",	// Empty weapon sound
		"",	// Alt Empty weapon sound
		"CFlareProjectile",		// Projectile class
		"CFlareAltProjectile",	// Alt Projectile class
		150,					// Muzzle Flash radius
		{1.0f, 0.68f, 0.35f},	// Muzzle Flash color
		"Flare Gun",			// Name
		IDS_WEAPON_FLAREGUN,	// Name ID (resource)
		"Sprites\\FlaregunFlare.spr",	// flash sprite name
		"Sprites\\FlaregunFlare.spr",	// alt flash Sprite name
		0.05f,					// Flash duration
		0.175f,					// Flash scale
		{0.0f, 0.0f, 0.0f},		// 3rd person gun offset
		{0.8f, -0.7f, 1.6f},	// Gun Offset
		{0.0f, 0.275f, 1.0f},	// Muzzle Offset
		{0.03f, 0.03f, 0.15f},	// Recoil
		{6.5f, -4.5f, 28.0f},	// Flash position
		0.625f,					// Eject interval
		0.0f,					// View kick
		DFALSE,					// View kick is Cumulative
		DFALSE,					// Animation looping
		DFALSE,					// Alt anim looping
		"interface\\statusbar\\weapons\\flaregun.pcx",		// Icon for status bar
		"interface\\statusbar\\weapons\\flaregun_h.pcx",	// Highlighted icon for status bar
	},
	// Shotgun
	{
		"Models\\Weapons\\Shotgun_pv.abc",	// Model
		"Models\\Weapons\\Shotgun_pv.abc",	// Model
		"Skins\\Weapons\\C_Shot_pv_t.dtx",	// Skin
		"Models\\Powerups\\Shotgun_pu.abc",	// 3rd person model
		"Skins\\Powerups\\Shotgun_pu.dtx",	// 3rd person skin
		WEAP_SHOTGUN,				// Type
		TYPE_PISTOL,				// Fire type
		AMMO_SHELL,					// Ammo type
		1,							// Ammo Use
		2,							// Alt Ammo Use
		4.0f,						// Min Damage
		6.0f,						// Max Damage
		8.0f,						// Min Alt damage
		12.0f,						// Max Alt damage
		0.5f,						// Reload time
		1.0f,						// Alt reload time
		{200.0f, 100.0f},			// Spread
		{200.0f, 100.0f},			// Alt Spread
		0.0f,						// Projectile Velocity
		0.0f,						// Alt Projectile Velocity
		3500.0f,					// Range
		3500.0f,					// Alt Range
		16,							// Shots per fire
		16,							// Alt Shots per fire
		3,							// Strength or Magic required
		4,							// Two-handed Strength or Magic required
		0,							// Damage Radius (projectiles)
		0,							// Alt Damage Radius (projectiles)
		DFALSE,						// Alt fire zoom
		DFALSE,						// Semi-auto
		"Sounds\\Weapons\\shotgun\\fire.wav",	// Fire sound
		"Sounds\\Weapons\\shotgun\\alt.wav",	// Alt Fire sound
		"",	// Empty weapon sound
		"",	// Alt Empty weapon sound
		NULL,						// Projectile class
		NULL,						// Alt Projectile class
		125,						// Muzzle Flash radius
		{1.0f, 0.68f, 0.35f},		// Muzzle Flash color
		"Sawed-off Shotgun",		// Name
		IDS_WEAPON_SHOTGUN,			// Name ID (resource)
		"Sprites\\shotgun1.spr",	// flash sprite name
		"Sprites\\shotgun2.spr",	// alt flash Sprite name
		0.05f,						// Flash duration
		0.19f,						// Flash scale
		{0.0f, 0.0f, 0.0f},			// 3rd person gun offset
		{0.5f, -0.5f, 0.9f},		// Gun Offset
		{0.08f, 0.5f, 2.0f},		// Muzzle Offset
		{0.03f, 0.03f, 0.15f},		// Recoil
		{5.0f, -5.0f, 30.0f},		// Flash position
		0.625f,						// Eject interval
		2.5f,						// View kick
		DFALSE,						// View kick is Cumulative
		DFALSE,						// Animation looping
		DFALSE,						// Alt anim looping
		"interface\\statusbar\\weapons\\shotgun.pcx",		// Icon for status bar
		"interface\\statusbar\\weapons\\shotgun_h.pcx",	// Highlighted icon for status bar
	},
	// Barrett .50 BMG Sniper Rifle
	{
		"Models\\Weapons\\SniperRifle_pv.abc",	// Model
		"Models\\Weapons\\SniperRifle_pv.abc",	// Model
		"Skins\\Weapons\\C_Sniper_pv_t.dtx",	// Skin
		"Models\\Powerups\\SniperRifle_pu.abc",	// 3rd person model
		"Skins\\Powerups\\SniperRifle_pu.dtx",	// 3rd person skin
		WEAP_SNIPERRIFLE,			// Type
		TYPE_RIFLE,					// Fire type
		AMMO_BMG,					// Ammo type
		1,							// Ammo Use
		0,							// Alt Ammo Use
		100.0f,						// Min Damage
		125.0f,						// Max Damage
		0.0f,						// Min Alt damage
		0.0f,						// Max Alt damage
		1.0f,						// Reload time
		1.0f,						// Alt reload time
		{0.0f, 0.0f},				// Spread
		{0.0f, 0.0f},				// Alt Spread
		0.0f,						// Projectile Velocity
		0.0f,						// Alt Projectile Velocity
		10000.0f,					// Range
		10000.0f,					// Alt Range
		1,							// Shots per fire
		0,							// Alt Shots per fire
		4,							// Strength or Magic required
		0,							// Two-handed Strength or Magic required
		0,							// Damage Radius (projectiles)
		0,							// Alt Damage Radius (projectiles)
		DTRUE,						// Alt fire zoom
		DFALSE,						// Semi-auto
		"Sounds\\Weapons\\sniper\\fire.wav",	// Fire sound
		"Sounds\\Weapons\\sniper\\zoom.wav",		// Alt Fire sound
		"",		// Empty weapon sound
		"",		// Alt Empty weapon sound
		NULL,						// Projectile class
		NULL,						// Alt Projectile class
		75,							// Muzzle Flash radius
		{1.0f, 0.68f, 0.35f},		// Muzzle Flash color
		"BMG Sniper Rifle",			// Name
		IDS_WEAPON_SNIPER,			// Name ID (resource)
		"Sprites\\sniperflare.spr",	// flash sprite name
		"Sprites\\sniperflare.spr",	// alt flash Sprite name
		0.01f,						// Flash duration
		0.25f,						// Flash scale
		{0.0f, 0.0f, 0.0f},			// 3rd person gun offset
		{0.6f, -0.8f, 1.9f},		// Gun Offset
		{-0.25f, 0.260f, 3.0f},		// Muzzle Offset
		{1.0f, 1.0f, 0.5f},			// Recoil
		{1.5f, -2.5f, 25.0f},		// Flash Offset
		0.6333f,					// Eject interval
		2.0f,						// View kick
		DFALSE,						// View kick is Cumulative
		DFALSE,						// Animation looping
		DFALSE,						// Alt anim looping
		"interface\\statusbar\\weapons\\sniper.pcx",		// Icon for status bar
		"interface\\statusbar\\weapons\\sniper_h.pcx",	// Highlighted icon for status bar
	},
	// Howitzer
	{
		"Models\\Weapons\\Howitzer_pv.abc",	// Model
		"Models\\Weapons\\Howitzer_pv.abc",	// Model
		"Skins\\Weapons\\C_Howitzer_pv_t.dtx",	// Skin
		"Models\\Powerups\\Howitzer_pu.abc",	// 3rd person model
		"Skins\\Powerups\\Howitzer_pu.dtx",	// 3rd person skin
		WEAP_HOWITZER,			// Type
		TYPE_RIFLE,				// Fire type
		AMMO_HOWITZER,			// Ammo type
		1,						// Ammo Use
		5,						// Alt Ammo Use
		30.0f,					// Min Damage
		50.0f,					// Max Damage
		50.0f,					// Min Alt damage
		75.0f,					// Max Alt damage
		1.0f,					// Reload time
		1.0f,					// Alt reload time
		{20.0f, 20.0f},			// Spread
		{100.0f, 100.0f},		// Alt Spread
		1000.0f,				// Projectile Velocity
		1000.0f,				// Alt Projectile Velocity
		3500.0f,				// Range
		3500.0f,				// Alt Range
		1,						// Shots per fire
		1,						// Alt Shots per fire
		4,						// Strength or Magic required
		0,						// Two-handed Strength or Magic required
		150,					// Damage Radius (projectiles)
		200,					// Alt Damage Radius (projectiles)
		DFALSE,					// Alt fire zoom
		DFALSE,					// Semi-auto
		"Sounds\\Weapons\\howitzer\\fire.wav",	// Fire sound
		"Sounds\\Weapons\\howitzer\\alt.wav",	// Alt Fire sound
		"",	// Empty weapon sound
		"",	// Alt Empty weapon sound
		"CHowitzerShell",		// Projectile class
		"CHowitzerAltShell",	// Alt Projectile class
		200,					// Muzzle Flash radius
		{1.0f, 0.68f, 0.35f},	// Muzzle Flash color
		"50mm Pack Howitzer",	// Name
		IDS_WEAPON_HOWITZER,	// Name ID (resource)
		"Sprites\\howitzer.spr",// flash sprite name
		"Sprites\\howitzer.spr",// alt flash Sprite name
		0.25f,					// Flash duration
		0.25f,					// Flash scale
		{0.0f, 0.0f, 0.0f},		// 3rd person gun offset
		{0.7f, -0.8f, 2.0f},	// Gun Offset
		{0.0f, 0.275f, 1.0f},	// Muzzle Offset
		{0.03f, 0.03f, 0.15f},	// Recoil
		{1.1f, -1.80f, 18.0f},	// Flash position
		0.625f,					// Eject interval
		2.0f,					// View kick
		DFALSE,					// View kick is Cumulative
		DFALSE,					// Animation looping
		DFALSE,					// Alt anim looping
		"interface\\statusbar\\weapons\\howitzer.pcx",		// Icon for status bar
		"interface\\statusbar\\weapons\\howitzer_h.pcx",	// Highlighted icon for status bar
	},
	// Napalm Cannon
	{
		"Models\\Weapons\\NapalmCannon_pv.abc",	// Model
		"Models\\Weapons\\lh_NapalmCannon_pv.abc",	// Model
		"Skins\\Weapons\\C_Napalm_pv_t.dtx",	// Skin
		"Models\\Powerups\\NapalmCannon_pu.abc",	// 3rd person model
		"Skins\\Powerups\\NapalmCannon_pu.dtx",	// 3rd person skin
		WEAP_NAPALMCANNON,		// Type
		TYPE_RIFLE,				// Fire type
		AMMO_FUEL,				// Ammo type
		1,						// Ammo Use
		10,						// Alt Ammo Use
		60.0f,					// Min Damage
		80.0f,					// Max Damage
		30.0f,					// Min Alt damage
		40.0f,					// Max Alt damage
		0.75f,					// Reload time
		1.0f,					// Alt reload time
		{50.0f, 50.0f},			// Spread
		{50.0f, 50.0f},			// Alt Spread
		1250.0f,				// Projectile Velocity
		900.0f,					// Alt Projectile Velocity
		5000.0f,				// Range
		5000.0f,				// Alt Range
		1,						// Shots per fire
		1,						// Alt Shots per fire
		3,						// Strength or Magic required
		0,						// Two-handed Strength or Magic required
		300,					// Damage Radius (projectiles)
		250,					// Alt Damage Radius (projectiles)
		DFALSE,					// Alt fire zoom
		DFALSE,					// Semi-auto
		"Sounds\\Weapons\\napalm\\fire.wav",	// Fire sound
		"Sounds\\Weapons\\napalm\\alt.wav",	// Alt Fire sound
		"",	// Empty weapon sound
		"",	// Alt Empty weapon sound
		"CNapalmProjectile",	// Projectile class
		"CNapalmAltProjectile",	// Alt Projectile class
		175,					// Muzzle Flash radius
		{1.0f, 0.68f, 0.35f},	// Muzzle Flash color
		"Napalm Cannon",		// Name
		IDS_WEAPON_NAPALM,		// Name ID (resource)
		"",						// flash sprite name
		"",						// alt flash Sprite name
		0.25f,					// Flash duration
		0.10f,					// Flash scale
		{0.0f, 0.0f, 0.0f},		// 3rd person gun offset
		{0.9f, -0.8f, 2.1f},	// Gun Offset
		{6.0f, -6.0f, 40.2f},	// Muzzle Offset
		{0.03f, 0.03f, 0.15f},	// Recoil
		{9.0f, -3.0f, 25.0f},	// Flash Offset
		0.625f,					// Eject interval
		0.0f,					// View kick
		DFALSE,					// View kick is Cumulative
		DFALSE,					// Animation looping
		DFALSE,					// Alt anim looping
		"interface\\statusbar\\weapons\\napalm.pcx",		// Icon for status bar
		"interface\\statusbar\\weapons\\napalm_h.pcx",	// Highlighted icon for status bar
	},
	// Singularity
	{
		"Models\\Weapons\\Singularity_pv.abc",	// Model
		"Models\\Weapons\\Singularity_pv.abc",	// Model
		"Skins\\Weapons\\C_Single_pv_t.dtx",	// Skin
		"Models\\Powerups\\Singularity_pu.abc",	// 3rd person model
		"Skins\\Powerups\\Singularity_pu.dtx",	// 3rd person skin
		WEAP_SINGULARITY,			// Type
		TYPE_RIFLE,					// Fire type
		AMMO_BATTERY,				// Ammo type
		50,							// Ammo Use
		50,							// Alt Ammo Use
		5.0f,						// Min Damage
		10.0f,						// Max Damage
		5.0f,						// Min Alt damage
		10.0f,						// Max Alt damage
		5.0f,						// Reload time
		5.0f,						// Alt reload time
		{0.0f, 0.0f},				// Spread
		{0.0f, 0.0f},				// Alt Spread
		0.0f,						// Projectile Velocity
		0.0f,						// Alt Projectile Velocity
		3500.0f,					// Range
		3500.0f,					// Alt Range
		1,							// Shots per fire
		1,							// Alt Shots per fire
		5,							// Strength or Magic required
		0,							// Two-handed Strength or Magic required
		500,						// Damage Radius (projectiles)
		500,						// Alt Damage Radius (projectiles)
		DFALSE,						// Alt fire zoom
		DFALSE,						// Semi-auto
		"Sounds\\Weapons\\singularity\\fire.wav",	// Fire sound
		"Sounds\\Weapons\\singularity\\fire.wav",	// Alt Fire sound
		"",		// Empty weapon sound
		"",		// Alt Empty weapon sound
		"CSingularityProjectile",	// Projectile class
		"CSingularityProjectile",	// Alt Projectile class
		0,							// Muzzle Flash radius
		{0.0f, 0.0f, 0.0f},			// Muzzle Flash color
		"Singularity Cannon",		// Name
		IDS_WEAPON_SINGULARITY,		// Name ID (resource)
		"",							// flash sprite name
		"",							// alt flash Sprite name
		0.10f,						// Flash duration
		0.13f,						// Flash scale
		{0.0f, 0.0f, 0.0f},			// 3rd person gun offset
		{1.4f, -1.9f, 3.4f},		// Gun Offset
		{0.0f, 0.0f, 0.0f},			// Muzzle Offset
		{0.03f, 0.03f, 0.15f},		// Recoil
		{4.25f, -4.25f, 25.0f},		// Flash position
		0.625f,						// Eject interval
		0.2f,						// View kick
		DFALSE,						// View kick is Cumulative
		DFALSE,						// Animation looping
		DFALSE,						// Alt anim looping
		"interface\\statusbar\\weapons\\singularity.pcx",		// Icon for status bar
		"interface\\statusbar\\weapons\\singularity_h.pcx",	// Highlighted icon for status bar
	},	
	// M16 Assault Rifle
	{
		"Models\\Weapons\\M16_pv.abc",	// Model
		"Models\\Weapons\\lh_M16_pv.abc",	// Model
		"Skins\\Weapons\\C_M16_pv_t.dtx",	// Skin
		"Models\\Powerups\\M16_pu.abc",	// 3rd person model
		"Skins\\Powerups\\M16_pu.dtx",	// 3rd person skin
		WEAP_ASSAULTRIFLE,			// Type
		TYPE_AUTORIFLE,				// Fire type
		AMMO_BULLET,				// Ammo type
		1,							// Ammo Use
		5,							// Alt Ammo Use
		10.0f,						// Min Damage
		16.0f,						// Max Damage
		100.0f,						// Min Alt damage
		140.0f,						// Max Alt damage
		0.1f,						// Reload time
		0.0f,						// Alt reload time
		{75.0f, 25.0f},				// Spread
		{50.0f, 50.0f},				// Alt Spread
		0.0f,						// Projectile Velocity
		1500.0f,					// Alt Projectile Velocity
		3500.0f,					// Range
		400.0f,						// Alt Range
		1,							// Shots per fire
		1,							// Alt Shots per fire
		3,							// Strength or Magic required
		0,							// Two-handed Strength or Magic required
		0,							// Damage Radius (projectiles)
		200,						// Alt Damage Radius (projectiles)
		DFALSE,						// Alt fire zoom
		DFALSE,						// Semi-auto
		"Sounds\\Weapons\\assault\\fire.wav",	// Fire sound
		"Sounds\\Weapons\\assault\\alt.wav",	// Alt Fire sound
		"",	// Empty weapon sound
		"",	// Alt Empty weapon sound
		NULL,						// Projectile class
		"CGrenade",					// Alt Projectile class
		100,						// Muzzle Flash radius
		{1.0f, 0.68f, 0.35f},		// Muzzle Flash color
		"M16 Assault Rifle",		// Name
		IDS_WEAPON_M16,				// Name ID (resource)
		"Sprites\\m16flare.spr",	// flash sprite name
		"Sprites\\m16altflare.spr",	// alt flash Sprite name
		0.1f,						// Flash duration
		0.125f,						// Flash scale
		{0.0f, 0.0f, 0.0f},			// 3rd person gun offset
		{0.7f, -0.9f, 1.7f},		// Gun Offset
		{0.0f, 0.275f, 3.0f},		// Muzzle Offset
		{0.03f, 0.03f, 0.15f},		// Recoil
		{5.25f, -5.8f, 30.0f},		// Flash position
		0.625f,						// Eject interval
		0.5f,						// View kick
		DFALSE,						// View kick is Cumulative
		DTRUE,						// Animation looping
		DFALSE,						// Alt anim looping
		"interface\\statusbar\\weapons\\m16.pcx",		// Icon for status bar
		"interface\\statusbar\\weapons\\m16_h.pcx",	// Highlighted icon for status bar
	},
	// Bug Spray Canister
	{
		"Models\\Weapons\\BugSpray_pv.abc",		// Model
		"Models\\Weapons\\BugSpray_pv.abc",		// Model
		"Skins\\Weapons\\C_BugSpray_pv_t.dtx",	// Skin
		"Models\\Powerups\\BugSpray_pu.abc",	// 3rd person model
		"Skins\\Powerups\\BugSpray_pu.dtx",		// 3rd person skin
		WEAP_BUGSPRAY,			// Type
		TYPE_AUTORIFLE,			// Fire type
		AMMO_DIEBUGDIE,			// Ammo type
		1,						// Ammo Use
		1,						// Alt Ammo Use
		40.0f,					// Min Damage
		60.0f,					// Max Damage
		60.0f,					// Min Alt damage
		80.0f,					// Max Alt damage
		0.25f,					// Reload time
		1.0f,					// Alt reload time
		{0.0f, 0.0f},			// Spread
		{0.0f, 0.0f},			// Alt Spread
		1500.0f,				// Projectile Velocity
		1500.0f,				// Alt Projectile Velocity
		500.0f,					// Range
		500.0f,					// Alt Range
		1,						// Shots per fire
		1,						// Alt Shots per fire
		2,						// Strength or Magic required
		0,						// Two-handed Strength or Magic required
		0,						// Damage Radius (projectiles)
		0,						// Alt Damage Radius (projectiles)
		DFALSE,					// Alt fire zoom
		DFALSE,					// Semi-auto
		"Sounds\\Weapons\\bugbuster\\fire.wav",	// Fire sound
		"Sounds\\Weapons\\bugbuster\\alt.wav",	// Alt Fire sound
		"",	// Empty weapon sound
		"",	// Alt Empty weapon sound
		"CBugSprayProjectile",	// Projectile class
		"CBugSprayAltProjectile",	// Alt Projectile class
		0,						// Muzzle Flash radius
		{0.0f, 0.0f, 0.0f},		// Muzzle Flash color
		"Bug Spray Canister",	// Name
		IDS_WEAPON_BUGSPRAY,	// Name ID (resource)
		"",						// flash sprite name
		"",						// alt flash Sprite name
		0.25f,					// Flash duration
		0.10f,					// Flash scale
		{0.0f, 0.0f, 0.0f},		// 3rd person gun offset
		{1.5f, -2.1f, 4.0f},	// Gun Offset
		{0.0f, 0.275f, 1.0f},	// Muzzle Offset
		{0.03f, 0.03f, 0.15f},	// Recoil
		{4.5f, -3.5f, 20.0f},	// Flash position
		0.625f,					// Eject interval
		0.0f,					// View kick
		DFALSE,					// View kick is Cumulative
		DFALSE,					// Animation looping
		DTRUE,					// Alt anim looping
		"interface\\statusbar\\weapons\\bugspray.pcx",		// Icon for status bar
		"interface\\statusbar\\weapons\\bugspray_h.pcx",	// Highlighted icon for status bar
	},
	// Minigun
	{
		"Models\\Weapons\\Minigun_pv.abc",	// Model
		"Models\\Weapons\\Minigun_pv.abc",	// Model
		"Skins\\Weapons\\C_Minigun_pv_t.dtx",	// Skin
		"Models\\Powerups\\Minigun_pu.abc",	// 3rd person model
		"Skins\\Powerups\\Minigun_pu.dtx",	// 3rd person skin
		WEAP_MINIGUN,			// Type
		TYPE_AUTORIFLE,			// Fire type
		AMMO_BULLET,			// Ammo type
		1,						// Ammo Use
		1,						// Alt Ammo Use
		8.0f,					// Min Damage
		16.0f,					// Max Damage
		8.0f,					// Min Alt damage
		16.0f,					// Max Alt damage
		0.0666667f,				// Reload time
		0.0666667f,				// Alt Reload time
		{150.0f, 75.0f},		// Spread
		{150.0f, 75.0f},		// Alt Spread
		0.0f,					// Projectile Velocity
		0.0f,					// Alt Projectile Velocity
		3500.0f,				// Range
		3500.0f,				// Alt Range
		4,						// Shots per fire
		4,						// Alt Shots per fire
		5,						// Strength or Magic required
		0,						// Two-handed Strength or Magic required
		0,						// Damage Radius (projectiles)
		0,						// Alt Damage Radius (projectiles)
		DFALSE,					// Alt fire zoom
		DFALSE,					// Semi-auto
		"",						// Fire sound
		"",						// Fire sound
		"",	// Empty weapon sound
		"",	// Alt Empty weapon sound
		NULL,					// Projectile class
		NULL,					// Alt Projectile class
		125,					// Muzzle Flash radius
		{1.0f, 0.68f, 0.35f},	// Muzzle Flash color
		"Vulcan Cannon",		// Name
		IDS_WEAPON_MINIGUN,		// Name ID (resource)
		"Sprites\\minigun.spr",	// flash sprite name
		"Sprites\\minigun.spr",	// alt flash Sprite name
		0.05f,					// Flash duration
		0.225f,					// Flash scale
		{0.0f, 0.0f, 0.0f},		// 3rd person gun offset
		{0.6f, -1.3f, 2.0f},	// Gun Offset
		{0.0f, 0.275f, 1.0f},	// Muzzle Offset
		{0.03f, 0.03f, 0.15f},	// Recoil
		{0.0f, 0.0f, 0.1f},		// Flash position
		0.625f,					// Eject interval
		0.0f,					// View kick
		DFALSE,					// View kick is Cumulative
		DTRUE,					// Animation looping
		DTRUE,					// Alt anim looping
		"interface\\statusbar\\weapons\\minigun.pcx",		// Icon for status bar
		"interface\\statusbar\\weapons\\minigun_h.pcx",	// Highlighted icon for status bar
	},
    // Death-Ray
   	{
		"Models\\Weapons\\DeathRay_pv.abc",		// Model
		"Models\\Weapons\\DeathRay_pv.abc",		// Model
		"Skins\\Weapons\\C_DeathRay_pv_t.dtx",	// Skin
		"Models\\Powerups\\DeathRay_pu.abc",	// 3rd person model
		"Skins\\Powerups\\DeathRay_pu.dtx",		// 3rd person skin
		WEAP_DEATHRAY,			// Type
		TYPE_AUTORIFLE,			// Fire type
		AMMO_BATTERY,			// Ammo type
		1,						// Ammo Use
		2,						// Alt Ammo Use
		50.0f,					// Min Damage
		60.0f,					// Max Damage
		50.0f,					// Min Alt damage
		75.0f,					// Max Alt damage
		0.2f,					// Reload time
		0.05f,					// Alt reload time
		{0.0f, 0.0f},			// Spread
		{0.0f, 0.0f},			// Alt Spread
		0.0f,					// Projectile Velocity
		0.0f,					// Alt Projectile Velocity
		3500.0f,				// Range
		1.0f,					// Alt Range
		1,						// Shots per fire
		1,						// Alt Shots per fire
		3,						// Strength or Magic required
		0,						// Two-handed Strength or Magic required
		0,						// Damage Radius (projectiles)
		0,						// Alt Damage Radius (projectiles)
		DFALSE,					// Alt fire zoom
		DFALSE,					// Semi-auto
		"Sounds\\Weapons\\laser\\fire.wav",	// Fire sound
		"Sounds\\Weapons\\laser\\alt.wav",	// Alt Fire sound
		"",	// Empty weapon sound
		"",	// Alt Empty weapon sound
		"CDeathRayProjectile",	// Projectile class
		DNULL,					// Alt Projectile class
		100,					// Muzzle Flash radius
		{0.0f, 1.0f, 0.0f},		// Muzzle Flash color
		"Cabalco Death-Ray",	// Name
		IDS_WEAPON_DEATHRAY,	// Name ID (resource)
		"Sprites\\laser.spr",	// flash sprite name
		"Sprites\\laser.spr",	// alt flash Sprite name
		0.25f,					// Flash duration
		0.175f,					// Flash scale
		{0.0f, 0.0f, 0.0f},		// 3rd person gun offset
		{0.7f, -0.9f, 1.9f},	// Gun Offset
		{0.0f, 0.0f, 0.0f},		// Muzzle Offset
		{0.03f, 0.03f, 0.15f},	// Recoil
		{4.25f, -4.25f, 25.0f},	// Flash position
		0.625f,					// Eject interval
		0.0f,					// View kick
		DFALSE,					// View kick is Cumulative
		DFALSE,					// Animation looping
		DTRUE,					// Alt anim looping
		"interface\\statusbar\\weapons\\deathray.pcx",		// Icon for status bar
		"interface\\statusbar\\weapons\\deathray_h.pcx",	// Highlighted icon for status bar
	},
	// Tesla Cannon
	{
		"Models\\Weapons\\TeslaRod_pv.abc",		// Model
		"Models\\Weapons\\lh_TeslaRod_pv.abc",		// Model
		"Skins\\Weapons\\C_Tesla_pv_t.dtx",	// Skin
		"Models\\Powerups\\TeslaCannon_pu.abc",	// 3rd person model
		"Skins\\Powerups\\TeslaCannon_pu.dtx",	// 3rd person skin
		WEAP_TESLACANNON,		// Type
		TYPE_AUTORIFLE,			// Fire type
		AMMO_BATTERY,			// Ammo type
		2,						// Ammo Use
		25,						// Alt Ammo Use
		30.0f,					// Min Damage
		50.0f,					// Max Damage
		100.0f,					// Min Alt damage
		150.0f,					// Max Alt damage
		0.175f,					// Reload time
		1.0f,					// Alt reload time
		{20.0f, 20.0f},			// Spread
		{0.0f, 0.0f},			// Alt Spread
		1000.0f,				// Projectile Velocity
		1000.0f,				// Alt Projectile Velocity
		3500.0f,				// Range
		3500.0f,				// Alt Range
		1,						// Shots per fire
		1,						// Alt Shots per fire
		4,						// Strength or Magic required
		0,						// Two-handed Strength or Magic required
		100,					// Damage Radius (projectiles)
		150,					// Alt Damage Radius (projectiles)
		DFALSE,					// Alt fire zoom
		DFALSE,					// Semi-auto
		"Sounds\\Weapons\\Tesla\\fire.wav",	// Fire sound
		"Sounds\\Weapons\\Tesla\\alt.wav",	// Alt Fire sound
		"",	// Empty weapon sound
		"",	// Alt Empty weapon sound
		"CTeslaProjectile",		// Projectile class
		"CTeslaBallProjectile",	// Alt Projectile class
		150.0f,					// Muzzle Flash radius
		{0.2f, 0.2f, 1.0f},		// Muzzle Flash color
		"Tesla Cannon",			// Name
		IDS_WEAPON_TESLA,		// Name ID (resource)
		"Sprites\\teslamuzzle.spr",	// flash sprite name
		"Sprites\\teslaflash.spr",	// alt flash Sprite name
		0.25f,					// Flash duration
		0.1f,					// Flash scale
		{0.0f, 0.0f, 0.0f},		// 3rd person gun offset
		{0.5f, -0.9f, 1.4f},	// Gun Offset
		{0.15f, 0.3f, 5.0f},	// Muzzle Offset
		{0.03f, 0.03f, 0.15f},	// Recoil
		{5.0f, -5.0f, 30.0f},	// Flash Offset
		0.625f,					// Eject interval
		0.0f,					// View kick
		DFALSE,					// View kick is Cumulative
		DTRUE,					// Animation looping
		DFALSE,					// Alt anim looping
		"interface\\statusbar\\weapons\\tesla.pcx",		// Icon for status bar
		"interface\\statusbar\\weapons\\tesla_h.pcx",	// Highlighted icon for status bar
	},
	// Voodoo doll
	{
		"Models\\Weapons\\Voodoo_pv.abc",	// Model
		"Models\\Weapons\\Voodoo_pv.abc",	// Model
		"Skins\\Weapons\\C_Voodoo_pv_t.dtx",	// Skin
		"Models\\Powerups\\Voodoo_pu.abc",	// 3rd person model
		"Skins\\Powerups\\Voodoo_pu.dtx",	// 3rd person skin
		WEAP_VOODOO,			// Type
		TYPE_MELEE,				// Fire type
		AMMO_FOCUS,				// Ammo type
		5,						// Ammo Use
		100,					// Alt Ammo Use
		10.0f,					// Min Damage
		20.0f,					// Max Damage
		40.0f,					// Min Alt damage
		60.0f,					// Max Alt damage
		0.33f,					// Reload time
		1.0f,					// Alt reload time
		{0.0f, 0.0f},			// Spread
		{0.0f, 0.0f},			// Alt Spread
		0.0f,					// Projectile Velocity
		0.0f,					// Alt Projectile Velocity
		3500.0f,				// Range
		3500.0f,				// Alt Range
		1,						// Shots per fire
		1,						// Alt Shots per fire
		1,						// Strength or Magic required
		0,						// Two-handed Strength or Magic required
		0,						// Damage Radius (projectiles)
		0,						// Alt Damage Radius (projectiles)
		DFALSE,					// Alt fire zoom
		DFALSE,					// Semi-auto
		"Sounds\\Weapons\\voodoo\\fire.wav",// Fire sound
		"Sounds\\Weapons\\voodoo\\alt.wav",	// Alt Fire sound
		"",	// Empty weapon sound
		"",	// Alt Empty weapon sound
		NULL,					// Projectile class
		NULL,					// Alt Projectile class
		0,						// Muzzle Flash radius
		{0.0f, 0.0f, 0.0f},		// Muzzle Flash color
		"Voodoo Doll",			// Name
		IDS_WEAPON_VOODOO,		// Name ID (resource)
		"",						// flash sprite name
		"",						// alt flash Sprite name
		0.0f,					// Flash duration
		0.0f,					// Flash scale
		{0.0f, 0.0f, 0.0f},		// 3rd person gun offset
		{-0.2f, -0.2f, 1.8f},	// Gun Offset
		{0.0f, 0.275f, 1.0f},	// Muzzle Offset
		{0.03f, 0.03f, 0.15f},	// Recoil
		{0.0f, 0.0f, 0.1f},		// Flash position
		0.625f,					// Eject interval
		0.0f,					// View kick
		DFALSE,					// View kick is Cumulative
		DFALSE,					// Animation looping
		DFALSE,					// Alt anim looping
		"interface\\statusbar\\weapons\\voodoo.pcx",		// Icon for status bar
		"interface\\statusbar\\weapons\\voodoo_h.pcx",	// Highlighted icon for status bar
	},
	// The Orb
	{
		"Models\\Weapons\\Orb_pv.abc",			// Model
		"Models\\Weapons\\Orb_pv.abc",			// Model
		"Skins\\Weapons\\C_Orb_pv_t.dtx",		// Skin
		"Models\\Powerups\\Orb_pu.abc",	// 3rd person model
		"Skins\\Powerups\\Orb_pu.dtx",	// 3rd person skin
		WEAP_ORB,				// Type
		TYPE_PISTOL,			// Fire type
		AMMO_FOCUS,				// Ammo type
		20,						// Ammo Use
		200,					// Alt Ammo Use
		10.0f,					// Min Damage
		15.0f,					// Max Damage
		10.0f,					// Min Alt damage
		15.0f,					// Max Alt damage
		1.0f,					// Reload time
		5.0f,					// Alt reload time
		{0.0f, 0.0f},			// Spread
		{0.0f, 0.0f},			// Alt Spread
		500.0f,					// Projectile Velocity
		500.0f,					// Alt Projectile Velocity
		3500.0f,				// Range
		3500.0f,				// Alt Range
		1,						// Shots per fire
		1,						// Alt Shots per fire
		1,						// Strength or Magic required
		0,						// Two-handed Strength or Magic required
		0,						// Damage Radius (projectiles)
		0,						// Alt Damage Radius (projectiles)
		DFALSE,					// Alt fire zoom
		DFALSE,					// Semi-auto
		"",						// Fire sound
		"",						// Alt Fire sound
		"",	// Empty weapon sound
		"",	// Alt Empty weapon sound
		"COrbProjectile",		// Projectile class
		"COrbAltProjectile",	// Alt Projectile class
		0,						// Muzzle Flash radius
		{0.0f, 0.0f, 0.0f},		// Muzzle Flash color
		"The Orb",				// Name
		IDS_WEAPON_ORB,			// Name ID (resource)
		"",						// flash sprite name
		"",						// alt flash Sprite name
		0.0f,					// Flash duration
		0.0f,					// Flash scale
		{0.0f, 0.0f, 0.0f},		// 3rd person gun offset
		{0.0f, -0.5f, 2.4f},	// Gun Offset
		{0.0f, 0.0f, 0.0f},		// Muzzle Offset
		{0.03f, 0.03f, 0.15f},	// Recoil
		{0.0f, -5.0f, 20.0f},	// Flash position
		0.625f,					// Eject interval
		0.0f,					// View kick
		DFALSE,					// View kick is Cumulative
		DFALSE,					// Animation looping
		DFALSE,					// Alt anim looping
		"interface\\statusbar\\weapons\\orb.pcx",		// Icon for status bar
		"interface\\statusbar\\weapons\\orb_h.pcx",	// Highlighted icon for status bar
	},    
	// Life Leech
	{
		"Models\\Weapons\\Lifeleech_pv.abc",	// Model
		"Models\\Weapons\\Lifeleech_pv.abc",	// Model
		"Skins\\Weapons\\C_Lifeleech_pv_t.dtx",	// Skin
		"Models\\Powerups\\Lifeleech_pu.abc",	// 3rd person model
		"Skins\\Powerups\\Lifeleech_pu.dtx",	// 3rd person skin
		WEAP_LIFELEECH,			// Type
		TYPE_MELEE,				// Fire type
		AMMO_FOCUS,				// Ammo type
		2,						// Ammo Use
		100,					// Alt Ammo Use
		20.0f,					// Min Damage
		30.0f,					// Max Damage
		75.0f,					// Min Alt damage
		100.0f,					// Max Alt damage
		0.15f,					// Reload time
		1.0f,					// Alt reload time
		{0.0f, 0.0f},			// Spread
		{0.0f, 0.0f},			// Alt Spread
		1000.0f,				// Projectile Velocity
		0.0f,					// Alt Projectile Velocity
		3500.0f,				// Range
		3500.0f,				// Alt Range
		1,						// Shots per fire
		1,						// Alt Shots per fire
		1,						// Strength or Magic required
		0,						// Two-handed Strength or Magic required
		100,					// Damage Radius (projectiles)
		0,						// Alt Damage Radius (projectiles)
		DFALSE,					// Alt fire zoom
		DFALSE,					// Semi-auto
		"",						// Fire sound
		"Sounds\\Weapons\\lifeleech\\alt.wav",	// Alt Fire sound
		"",	// Empty weapon sound
		"",	// Alt Empty weapon sound
		"CLeechPrimeProjectile",// Projectile class
		"CLeechAltProjectile",	// Alt Projectile class
		0,						// Muzzle Flash radius
		{0.0f, 0.0f, 0.0f},		// Muzzle Flash color
		"Life Leech",			// Name
		IDS_WEAPON_LIFELEECH,	// Name ID (resource)
		"",						// flash sprite name
		"",						// alt flash Sprite name
		0.25f,					// Flash duration
		0.10f,					// Flash scale
		{0.0f, 0.0f, 0.0f},		// 3rd person gun offset
		{-1.2f, -1.0f, 3.3f},	// Gun Offset
		{0.0f, 0.0f, 0.0f},		// Muzzle Offset
		{0.03f, 0.03f, 0.15f},	// Recoil
		{1.5f, -4.5f, 25.0f},	// Flash position
		0.625f,					// Eject interval
		0.0f,					// View kick
		DFALSE,					// View kick is Cumulative
		DTRUE,					// Animation looping
		DFALSE,					// Alt anim looping
		"interface\\statusbar\\weapons\\lifeleech.pcx",		// Icon for status bar
		"interface\\statusbar\\weapons\\lifeleech_h.pcx",	// Highlighted icon for status bar
	},            
	// Melee Hand Weapon
	{
		"Models\\Weapons\\c_knife_pv.abc",	// Model
		"Models\\Weapons\\c_knife_pv.abc",	// Model
		"Skins\\Weapons\\C_knife_pv_t.dtx",	// Skin
		"Models\\Powerups\\knifec_pu.abc",	// Model
		"Skins\\Powerups\\knifec_pu.dtx",	// Skin
		WEAP_MELEE,	    		// Type
		TYPE_MELEE,				// Fire type
		AMMO_NONE,	    		// Ammo type
		1,						// Ammo Use
		1,						// Alt Ammo Use
		15.0f,					// Min Damage
		25.0f,					// Max Damage
		35.0f,					// Min Alt damage
		60.0f,					// Max Alt damage
		0.0f,					// Reload time
		0.0f,					// Alt reload time
		{0.0f, 0.0f},			// Spread
		{0.0f, 0.0f},			// Alt Spread
		0.0f,					// Projectile Velocity
		0.0f,					// Alt Projectile Velocity
		75.0f,		    		// Range
		75.0f,	    			// Alt Range
		1,						// Shots per fire
		1,						// Alt Shots per fire
		1,						// Strength or Magic required
		0,						// Two-handed Strength or Magic required
		0,						// Damage Radius (projectiles)
		0,						// Alt Damage Radius (projectiles)
		DFALSE,					// Alt fire zoom
		DFALSE,					// Semi-auto
		"Sounds\\Weapons\\Knife\\fire.wav",	// Fire sound
		"Sounds\\Weapons\\Knife\\alt.wav",	// Alt Fire sound
		"",						// Empty weapon sound
		"",						// Alt Empty weapon sound
		NULL,					// Projectile class
		NULL,					// Alt Projectile class
		0,						// Muzzle Flash radius
		{0.0f, 0.0f, 0.0f},		// Muzzle Flash color
		"Chosen Knife",			// Name
		IDS_WEAPON_KNIFE,		// Name ID (resource)
		"",						// flash sprite name
		"",						// alt flash Sprite name
		0.0f,					// Flash duration
		0.0f,					// Flash scale
		{0.0f, 0.0f, 0.0f},		// 3rd person gun offset
		{-0.4f, -1.2f, 0.1f},	// 1st person gun Offset
		{0.0f, 0.0f, 0.0f},		// Muzzle Offset
		{0.03f, 0.03f, 0.15f},	// Recoil
		{0.0f, 0.0f, 0.1f},		// Flash position
		0.625f,					// Eject interval
		0.0f,					// View kick
		DFALSE,					// View kick is Cumulative
		DFALSE,					// Animation looping
		DFALSE,					// Alt anim looping
		"interface\\statusbar\\weapons\\c_knife.pcx",		// Icon for status bar
		"interface\\statusbar\\weapons\\c_knife_h.pcx",	// Highlighted icon for status bar
	},
#ifdef _ADD_ON
	// Combat Shotgun
	{
		"Models_ao\\Weapons_ao\\CombatShotgun_pv.abc",	// Model
		"Models_ao\\Weapons_ao\\CombatShotgun_pv.abc",	// LH Model
		"Skins_ao\\Weapons_ao\\C_ComShot_pv_t.dtx",	// Skin
		"Models_ao\\Powerups_ao\\CombatShotgun_pu.abc",	// 3rd person model
		"Skins_ao\\Powerups_ao\\CombatShotgun_pu.dtx",	// 3rd person skin
		WEAP_COMBATSHOTGUN,				// Type
		TYPE_RIFLE,					// Fire type
		AMMO_SHELL,					// Ammo type
		2,							// Ammo Use
		5,							// Alt Ammo Use
		4.0f,						// Min Damage
		6.0f,						// Max Damage
		50.0f,						// Min Alt damage
		70.0f,						// Max Alt damage
		0.33f,						// Reload time
		2.0f,						// Alt reload time
		{200.0f, 100.0f},			// Spread
		{0.0f, 0.0f},				// Alt Spread
		0.0f,						// Projectile Velocity
		1500.0f,					// Alt Projectile Velocity
		3500.0f,					// Range
		400.0f,						// Alt Range
		16,							// Shots per fire
		1,							// Alt Shots per fire
		3,							// Strength or Magic required
		0,							// Two-handed Strength or Magic required
		0,							// Damage Radius (projectiles)
		200,						// Alt Damage Radius (projectiles)
		DFALSE,						// Alt fire zoom
		DTRUE,						// Semi-auto
		"Sounds_ao\\Weapons\\combatshotgun\\fire.wav",	// Fire sound
		"Sounds_ao\\Weapons\\combatshotgun\\alt.wav",	// Alt Fire sound
		"",							// Empty weapon sound
		"",							// Alt Empty weapon sound
		NULL,						// Projectile class
		"CGasGrenade",				// Alt Projectile class
		125,						// Muzzle Flash radius
		{1.0f, 0.68f, 0.35f},		// Muzzle Flash color
		"Combat Shotgun",			// Name
		IDS_WEAPON_COMBATSHOTGUN,	// Name ID (resource)
		"Sprites\\shotgun2.spr",	// flash sprite name
		"Sprites\\m16altflare.spr",	// alt flash Sprite name
		0.05f,						// Flash duration
		0.22f,						// Flash scale
		{0.0f, 0.0f, 0.0f},			// 3rd person gun offset
		{0.6f, -0.6f, 0.7f},		// Gun Offset
		{0.08f, 0.5f, 2.0f},		// Muzzle Offset
		{0.03f, 0.03f, 0.15f},		// Recoil
		{5.0f, -3.5f, 30.0f},		// Flash position
		0.625f,						// Eject interval
		2.5f,						// View kick
		DFALSE,						// View kick is Cumulative
		DFALSE,						// Animation looping
		DFALSE,						// Alt anim looping
		"interface_ao\\hammericon.pcx",		// Icon for status bar
		"interface_ao\\hammericonH.pcx",	// Highlighted icon for status bar
	},
	// Flayer
	{
		"Models_ao\\Weapons_ao\\Flayer_pv.abc",	// Model
		"Models_ao\\Weapons_ao\\Flayer_pv.abc",	// Model
		"Skins_ao\\Weapons_ao\\C_Flayer_pv_t.dtx",	// Skin
		"Models_ao\\Powerups_ao\\Flayer_pu.abc",	// 3rd person model
		"Skins_ao\\Powerups_ao\\Flayer_pu.dtx",	// 3rd person skin
		WEAP_FLAYER,			// Type
		TYPE_MELEE,				// Fire type
		AMMO_FOCUS,				// Ammo type
		25,						// Ammo Use
		100,					// Alt Ammo Use
		8.0f,					// Min Damage
		12.0f,					// Max Damage
		8.0f,					// Min Alt damage
		12.0f,					// Max Alt damage
		0.15f,					// Reload time
		1.0f,					// Alt reload time
		{0.0f, 0.0f},			// Spread
		{0.0f, 0.0f},			// Alt Spread
		500.0f,					// Projectile Velocity
		0.0f,					// Alt Projectile Velocity
		3500.0f,				// Range
		0.0f,					// Alt Range
		1,						// Shots per fire
		1,						// Alt Shots per fire
		1,						// Strength or Magic required
		0,						// Two-handed Strength or Magic required
		100,					// Damage Radius (projectiles)
		0,						// Alt Damage Radius (projectiles)
		DFALSE,					// Alt fire zoom
		DFALSE,					// Semi-auto
		"Sounds_ao\\Weapons\\flayer\\fire.wav",// Fire sound
		"Sounds_ao\\Weapons\\flayer\\fire.wav",	// Alt Fire sound
		"",						// Empty weapon sound
		"",						// Alt Empty weapon sound
		"CFlayerChain",			// Projectile class
		"CFlayerPortal",		// Alt Projectile class
		0,						// Muzzle Flash radius
		{0.0f, 0.0f, 0.0f},		// Muzzle Flash color
		"Flayer",				// Name
		IDS_WEAPON_FLAYER,		// Name ID (resource)
		"",						// flash sprite name
		"",						// alt flash Sprite name
		0.25f,					// Flash duration
		0.10f,					// Flash scale
		{0.0f, 0.0f, 0.0f},		// 3rd person gun offset
		{1.0f, -1.0f, 2.5f},	// Gun Offset
		{0.0f, 0.0f, 0.0f},		// Muzzle Offset
		{0.03f, 0.03f, 0.15f},	// Recoil
		{0.0f, -10.0f, 25.0f},	// Flash position
		0.625f,					// Eject interval
		0.0f,					// View kick
		DFALSE,					// View kick is Cumulative
		DFALSE,					// Animation looping
		DFALSE,					// Alt anim looping
		"interface_ao\\flayericon.pcx",		// Icon for status bar
		"interface_ao\\flayericonH.pcx",	// Highlighted icon for status bar
	},            
#endif
	// Shikari Hand Claw
	{
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Skins\\Weapons\\knifec_pv_t.dtx",	// Skin
		DNULL,								// Model
		DNULL,								// Skin
		WEAP_SHIKARI_CLAW,	  	// Type
		TYPE_MELEE,				// Fire type
		AMMO_NONE,	    		// Ammo type
		1,						// Ammo Use
		1,						// Alt Ammo Use
		30.0f,					// Min Damage
		60.0f,					// Max Damage
		30.0f,					// Min Alt damage
		60.0f,					// Max Alt damage
		1.00f,					// Reload time
		0.0f,					// Alt reload time
		{0.0f, 0.0f},			// Spread
		{0.0f, 0.0f},			// Alt Spread
		0.0f,					// Projectile Velocity
		0.0f,					// Alt Projectile Velocity
		100.0f,		    		// Range
		100.0f,	    			// Alt Range
		1,						// Shots per fire
		1,						// Alt Shots per fire
		1,						// Strength or Magic required
		0,						// Two-handed Strength or Magic required
		0,						// Damage Radius (projectiles)
		0,						// Alt Damage Radius (projectiles)
		DFALSE,					// Alt fire zoom
		DFALSE,					// Semi-auto
		"Sounds\\Weapons\\Knife\\fire.wav",	// Fire sound
		"Sounds\\Weapons\\Knife\\alt.wav",	// Alt Fire sound
		"",	// Empty weapon sound
		"",	// Alt Empty weapon sound
		NULL,					// Projectile class
		NULL,					// Alt Projectile class
		0,						// Muzzle Flash radius
		{0.0f, 0.0f, 0.0f},		// Muzzle Flash color
		"Shikari_Claw",			// Name
		0,						// Name ID (resource)
		"",						// flash sprite name
		"",						// alt flash Sprite name
		0.25f,					// Flash duration
		0.10f,					// Flash scale
		{0.0f, 0.0f, 0.0f},		// 3rd person gun offset
		{0.0f, -1.4f, 0.3f},	// 1st person gun Offset
		{0.0f, 0.0f, 0.0f},		// Muzzle Offset
		{0.03f, 0.03f, 0.15f},	// Recoil
		{0.0f, 0.0f, 0.1f},		// Flash position
		0.625f,					// Eject interval
		0.0f,					// View kick
		DFALSE,					// View kick is Cumulative
		DFALSE,					// Animation looping
		DFALSE,					// Alt anim looping
		DNULL,					// Icon for status bar
		DNULL,					// Highlighted icon for status bar
	},
    // Shikari Spit
	{
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Skins\\Weapons\\knifec_pv_t.dtx",	// Skin
		DNULL,								// Model
		DNULL,								// Skin
		WEAP_SHIKARI_SPIT, 		// Type
		TYPE_RIFLE,				// Fire type
		AMMO_NONE,	    		// Ammo type
		1,						// Ammo Use
		1,						// Alt Ammo Use
		30.0f,					// Min Damage
		50.0f,					// Max Damage
		30.0f,					// Min Alt damage
		50.0f,					// Max Alt damage
		1.00f,					// Reload time
		0.0f,					// Alt reload time
		{0.0f, 0.0f},			// Spread
		{0.0f, 0.0f},			// Alt Spread
		1000.0f,					// Projectile Velocity
		1000.0f,					// Alt Projectile Velocity
		75.0f,		    		// Range
		75.0f,	    			// Alt Range
		1,						// Shots per fire
		1,						// Alt Shots per fire
		1,						// Strength or Magic required
		0,						// Two-handed Strength or Magic required
		0,						// Damage Radius (projectiles)
		0,						// Alt Damage Radius (projectiles)
		DFALSE,					// Alt fire zoom
		DFALSE,					// Semi-auto
		"Sounds\\Enemies\\Shikari\\sh_spit_3.wav",	// Fire sound
		"Sounds\\Enemies\\Shikari\\sh_spit_3.wav",	// Alt Fire sound
		"Sounds\\Enemies\\Shikari\\sh_spit_1.wav",	// Empty Fire sound
		"Sounds\\Enemies\\Shikari\\sh_spit_1.wav",	// Alt EmptyFire sound
		"CShikariLoogieProjectile",	// Projectile class
		"CShikariLoogieProjectile",	// Alt Projectile class
		0,						// Muzzle Flash radius
		{0.0f, 0.0f, 0.0f},		// Muzzle Flash color
		"Shikari_Spit",			// Name
		0,						// Name ID (resource)
		"",						// flash sprite name
		"",						// alt flash Sprite name
		0.25f,					// Flash duration
		0.10f,					// Flash scale
		{0.0f, 0.0f, 0.0f},		// 3rd person gun offset
		{0.0f, -1.4f, 0.3f},	// 1st person gun Offset
		{0.0f, 0.0f, 0.0f},		// Muzzle Offset
		{0.03f, 0.03f, 0.15f},	// Recoil
		{0.0f, 0.0f, 0.1f},		// Flash position
		0.625f,					// Eject interval
		0.0f,					// View kick
		DFALSE,					// View kick is Cumulative
		DFALSE,					// Animation looping
		DFALSE,					// Alt anim looping
		DNULL,					// Icon for status bar
		DNULL,					// Highlighted icon for status bar
	},
	// SoulDrudge Crowbar
	{
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Skins\\Weapons\\knifec_pv_t.dtx",	// Skin
		"Models\\Powerups\\crowbar_pu.abc",	// Model
		"Skins\\Powerups\\crowbar_pu.dtx",	// Skin
		WEAP_SOUL_CROWBAR,		// Type
		TYPE_MELEE,				// Fire type
		AMMO_NONE,	    		// Ammo type
		1,						// Ammo Use
		1,						// Alt Ammo Use
		10.0f,					// Min Damage
		20.0f,					// Max Damage
		10.0f,					// Min Alt damage
		20.0f,					// Max Alt damage
		1.00f,					// Reload time
		0.0f,					// Alt reload time
		{0.0f, 0.0f},			// Spread
		{0.0f, 0.0f},			// Alt Spread
		0.0f,					// Projectile Velocity
		0.0f,					// Alt Projectile Velocity
		75.0f,		    		// Range
		75.0f,	    			// Alt Range
		1,						// Shots per fire
		1,						// Alt Shots per fire
		1,						// Strength or Magic required
		0,						// Two-handed Strength or Magic required
		0,						// Damage Radius (projectiles)
		0,						// Alt Damage Radius (projectiles)
		DFALSE,					// Alt fire zoom
		DFALSE,					// Semi-auto
		"Sounds\\Weapons\\Knife\\fire.wav",	// Fire sound
		"Sounds\\Weapons\\Knife\\alt.wav",	// Alt Fire sound
		"",	// Empty weapon sound
		"",	// Alt Empty weapon sound
		NULL,					// Projectile class
		NULL,					// Alt Projectile class
		0,						// Muzzle Flash radius
		{0.0f, 0.0f, 0.0f},		// Muzzle Flash color
		"Soul_Crowbar",			// Name
		0,						// Name ID (resource)
		"",						// flash sprite name
		"",						// alt flash Sprite name
		0.25f,					// Flash duration
		0.10f,					// Flash scale
		{0.0f, 0.0f, 0.0f},		// 3rd person gun offset
		{0.0f, -1.4f, 0.3f},	// 1st person gun Offset
		{0.0f, 0.0f, 0.0f},		// Muzzle Offset
		{0.03f, 0.03f, 0.15f},	// Recoil
		{0.0f, 0.0f, 0.1f},		// Flash position
		0.625f,					// Eject interval
		0.0f,					// View kick
		DFALSE,					// View kick is Cumulative
		DFALSE,					// Animation looping
		DFALSE,					// Alt anim looping
		DNULL,					// Icon for status bar
		DNULL,					// Highlighted icon for status bar
	},
	// SoulDrudge Axe
	{
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Skins\\Weapons\\knifec_pv_t.dtx",	// Skin
		"Models\\Powerups\\fireaxe_pu.abc",	// Model
		"Skins\\Powerups\\fireaxe_pu.dtx",	// Skin
		WEAP_SOUL_AXE,		// Type
		TYPE_MELEE,				// Fire type
		AMMO_NONE,	    		// Ammo type
		1,						// Ammo Use
		1,						// Alt Ammo Use
		20.0f,					// Min Damage
		30.0f,					// Max Damage
		20.0f,					// Min Alt damage
		30.0f,					// Max Alt damage
		1.00f,					// Reload time
		0.0f,					// Alt reload time
		{0.0f, 0.0f},			// Spread
		{0.0f, 0.0f},			// Alt Spread
		0.0f,					// Projectile Velocity
		0.0f,					// Alt Projectile Velocity
		75.0f,		    		// Range
		75.0f,	    			// Alt Range
		1,						// Shots per fire
		1,						// Alt Shots per fire
		1,						// Strength or Magic required
		0,						// Two-handed Strength or Magic required
		0,						// Damage Radius (projectiles)
		0,						// Alt Damage Radius (projectiles)
		DFALSE,					// Alt fire zoom
		DFALSE,					// Semi-auto
		"Sounds\\Weapons\\Knife\\fire.wav",	// Fire sound
		"Sounds\\Weapons\\Knife\\alt.wav",	// Alt Fire sound
		"",	// Empty weapon sound
		"",	// Alt Empty weapon sound
		NULL,					// Projectile class
		NULL,					// Alt Projectile class
		0,						// Muzzle Flash radius
		{0.0f, 0.0f, 0.0f},		// Muzzle Flash color
		"Soul_Axe",			// Name
		0,						// Name ID (resource)
		"",						// flash sprite name
		"",						// alt flash Sprite name
		0.25f,					// Flash duration
		0.10f,					// Flash scale
		{0.0f, 0.0f, 0.0f},		// 3rd person gun offset
		{0.0f, -1.4f, 0.3f},	// 1st person gun Offset
		{0.0f, 0.0f, 0.0f},		// Muzzle Offset
		{0.03f, 0.03f, 0.15f},	// Recoil
		{0.0f, 0.0f, 0.1f},		// Flash position
		0.625f,					// Eject interval
		0.0f,					// View kick
		DFALSE,					// View kick is Cumulative
		DFALSE,					// Animation looping
		DFALSE,					// Alt anim looping
		DNULL,					// Icon for status bar
		DNULL,					// Highlighted icon for status bar
	},
	// SoulDrudge Pipe
	{
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Skins\\Weapons\\knifec_pv_t.dtx",	// Skin
		"Models\\Powerups\\pipe_pu.abc",	// Model
		"Skins\\Powerups\\pipe_pu.dtx",	// Skin
		WEAP_SOUL_PIPE,		// Type
		TYPE_MELEE,				// Fire type
		AMMO_NONE,	    		// Ammo type
		1,						// Ammo Use
		1,						// Alt Ammo Use
		10.0f,					// Min Damage
		20.0f,					// Max Damage
		10.0f,					// Min Alt damage
		20.0f,					// Max Alt damage
		1.00f,					// Reload time
		0.0f,					// Alt reload time
		{0.0f, 0.0f},			// Spread
		{0.0f, 0.0f},			// Alt Spread
		0.0f,					// Projectile Velocity
		0.0f,					// Alt Projectile Velocity
		75.0f,		    		// Range
		75.0f,	    			// Alt Range
		1,						// Shots per fire
		1,						// Alt Shots per fire
		1,						// Strength or Magic required
		0,						// Two-handed Strength or Magic required
		0,						// Damage Radius (projectiles)
		0,						// Alt Damage Radius (projectiles)
		DFALSE,					// Alt fire zoom
		DFALSE,					// Semi-auto
		"Sounds\\Weapons\\Knife\\fire.wav",	// Fire sound
		"Sounds\\Weapons\\Knife\\alt.wav",	// Alt Fire sound
		"",	// Empty weapon sound
		"",	// Alt Empty weapon sound
		NULL,					// Projectile class
		NULL,					// Alt Projectile class
		0,						// Muzzle Flash radius
		{0.0f, 0.0f, 0.0f},		// Muzzle Flash color
		"Soul_Pipe",			// Name
		0,						// Name ID (resource)
		"",						// flash sprite name
		"",						// alt flash Sprite name
		0.25f,					// Flash duration
		0.10f,					// Flash scale
		{0.0f, 0.0f, 0.0f},		// 3rd person gun offset
		{0.0f, -1.4f, 0.3f},	// 1st person gun Offset
		{0.0f, 0.0f, 0.0f},		// Muzzle Offset
		{0.03f, 0.03f, 0.15f},	// Recoil
		{0.0f, 0.0f, 0.1f},		// Flash position
		0.625f,					// Eject interval
		0.0f,					// View kick
		DFALSE,					// View kick is Cumulative
		DFALSE,					// Animation looping
		DFALSE,					// Alt anim looping
		DNULL,					// Icon for status bar
		DNULL,					// Highlighted icon for status bar
	},
	// DrudgeLord Hook
	{
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Skins\\Weapons\\knifec_pv_t.dtx",	// Skin
		DNULL,								// Model
		DNULL,								// Skin
		WEAP_SOUL_HOOK,	   		// Type
		TYPE_MELEE,				// Fire type
		AMMO_NONE,	    		// Ammo type
		1,						// Ammo Use
		1,						// Alt Ammo Use
		50.0f,					// Min Damage
		80.0f,					// Max Damage
		50.0f,					// Min Alt damage
		80.0f,					// Max Alt damage
		1.00f,					// Reload time
		0.0f,					// Alt reload time
		{0.0f, 0.0f},			// Spread
		{0.0f, 0.0f},			// Alt Spread
		0.0f,					// Projectile Velocity
		0.0f,					// Alt Projectile Velocity
		100.0f,		    		// Range
		100.0f,	    			// Alt Range
		1,						// Shots per fire
		1,						// Alt Shots per fire
		1,						// Strength or Magic required
		0,						// Two-handed Strength or Magic required
		0,						// Damage Radius (projectiles)
		0,						// Alt Damage Radius (projectiles)
		DFALSE,					// Alt fire zoom
		DFALSE,					// Semi-auto
		"Sounds\\Weapons\\Knife\\fire.wav",	// Fire sound
		"Sounds\\Weapons\\Knife\\alt.wav",	// Alt Fire sound
		"",	// Empty weapon sound
		"",	// Alt Empty weapon sound
		NULL,					// Projectile class
		NULL,					// Alt Projectile class
		0,						// Muzzle Flash radius
		{0.0f, 0.0f, 0.0f},		// Muzzle Flash color
		"Soul_Hook",			// Name
		0,						// Name ID (resource)
		"",						// flash sprite name
		"",						// alt flash Sprite name
		0.25f,					// Flash duration
		0.10f,					// Flash scale
		{0.0f, 0.0f, 0.0f},		// 3rd person gun offset
		{0.0f, -1.4f, 0.3f},	// 1st person gun Offset
		{0.0f, 0.0f, 0.0f},		// Muzzle Offset
		{0.03f, 0.03f, 0.15f},	// Recoil
		{0.0f, 0.0f, 0.1f},		// Flash position
		0.625f,					// Eject interval
		0.0f,					// View kick
		DFALSE,					// View kick is Cumulative
		DFALSE,					// Animation looping
		DFALSE,					// Alt anim looping
		DNULL,					// Icon for status bar
		DNULL,					// Highlighted icon for status bar
	},
	// Behemoth Hand Claw
	{
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Skins\\Weapons\\knifec_pv_t.dtx",	// Skin
		DNULL,								// Model
		DNULL,								// Skin
		WEAP_BEHEMOTH_CLAW,		// Type
		TYPE_MELEE,				// Fire type
		AMMO_NONE,	    		// Ammo type
		1,						// Ammo Use
		1,						// Alt Ammo Use
		100.0f,					// Min Damage
		120.0f,					// Max Damage
		100.0f,					// Min Alt damage
		120.0f,					// Max Alt damage
		1.00f,					// Reload time
		0.0f,					// Alt reload time
		{0.0f, 0.0f},			// Spread
		{0.0f, 0.0f},			// Alt Spread
		0.0f,					// Projectile Velocity
		0.0f,					// Alt Projectile Velocity
		100.0f,		    		// Range
		100.0f,	    			// Alt Range
		1,						// Shots per fire
		1,						// Alt Shots per fire
		1,						// Strength or Magic required
		0,						// Two-handed Strength or Magic required
		0,						// Damage Radius (projectiles)
		0,						// Alt Damage Radius (projectiles)
		DFALSE,					// Alt fire zoom
		DFALSE,					// Semi-auto
		"Sounds\\Weapons\\Knife\\fire.wav",	// Fire sound
		"Sounds\\Weapons\\Knife\\alt.wav",	// Alt Fire sound
		"",	// Empty weapon sound
		"",	// Alt Empty weapon sound
		NULL,					// Projectile class
		NULL,					// Alt Projectile class
		0,						// Muzzle Flash radius
		{0.0f, 0.0f, 0.0f},		// Muzzle Flash color
		"Behemoth_Claw",		// Name
		0,						// Name ID (resource)
		"",						// flash sprite name
		"",						// alt flash Sprite name
		0.25f,					// Flash duration
		0.10f,					// Flash scale
		{0.0f, 0.0f, 0.0f},		// 3rd person gun offset
		{0.0f, -1.4f, 0.3f},	// 1st person gun Offset
		{0.0f, 0.0f, 0.0f},		// Muzzle Offset
		{0.03f, 0.03f, 0.15f},	// Recoil
		{0.0f, 0.0f, 0.1f},		// Flash position
		0.625f,					// Eject interval
		0.0f,					// View kick
		DFALSE,					// View kick is Cumulative
		DFALSE,					// Animation looping
		DFALSE,					// Alt anim looping
		DNULL,					// Icon for status bar
		DNULL,					// Highlighted icon for status bar
	},
	// Zealot heal
	{
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Skins\\Weapons\\knifec_pv_t.dtx",	// Skin
		DNULL,								// Model
		DNULL,								// Skin
		WEAP_ZEALOT_HEAL,  		// Type
		TYPE_MELEE,				// Fire type
		AMMO_NONE,	    		// Ammo type
		1,						// Ammo Use
		1,						// Alt Ammo Use
		20.0f,					// Min Damage
		40.0f,					// Max Damage
		20.0f,					// Min Alt damage
		40.0f,					// Max Alt damage
		1.00f,					// Reload time
		0.0f,					// Alt reload time
		{0.0f, 0.0f},			// Spread
		{0.0f, 0.0f},			// Alt Spread
		0.0f,					// Projectile Velocity
		0.0f,					// Alt Projectile Velocity
		100.0f,		    		// Range
		100.0f,	    			// Alt Range
		1,						// Shots per fire
		1,						// Alt Shots per fire
		1,						// Strength or Magic required
		0,						// Two-handed Strength or Magic required
		0,						// Damage Radius (projectiles)
		0,						// Alt Damage Radius (projectiles)
		DFALSE,					// Alt fire zoom
		DFALSE,					// Semi-auto
		"Sounds\\Enemies\\Zealot\\healing.wav",	// Fire sound
		"Sounds\\Enemies\\Zealot\\healing.wav",	// Alt Fire sound
		"Sounds\\Enemies\\Zealot\\healing.wav",	// Empty weapon sound
		"Sounds\\Enemies\\Zealot\\healing.wav",	// Alt Empty weapon sound
		NULL,					// Projectile class
		NULL,					// Alt Projectile class
		0,						// Muzzle Flash radius
		{0.0f, 0.0f, 0.0f},		// Muzzle Flash color
		"Heal",					// Name
		0,						// Name ID (resource)
		"",						// flash sprite name
		"",						// alt flash Sprite name
		0.25f,					// Flash duration
		0.10f,					// Flash scale
		{0.0f, 0.0f, 0.0f},		// 3rd person gun offset
		{0.0f, -1.4f, 0.3f},	// 1st person gun Offset
		{0.0f, 0.0f, 0.0f},		// Muzzle Offset
		{0.03f, 0.03f, 0.15f},	// Recoil
		{0.0f, 0.0f, 0.1f},		// Flash position
		0.625f,					// Eject interval
		0.0f,					// View kick
		DFALSE,					// View kick is Cumulative
		DFALSE,					// Animation looping
		DFALSE,					// Alt anim looping
		DNULL,					// Icon for status bar
		DNULL,					// Highlighted icon for status bar
	},
	// Zealot shield
	{
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Skins\\Weapons\\knifec_pv_t.dtx",	// Skin
		DNULL,								// Model
		DNULL,								// Skin
		WEAP_ZEALOT_SHIELD,		// Type
		TYPE_MELEE,				// Fire type
		AMMO_NONE,	    		// Ammo type
		1,						// Ammo Use
		1,						// Alt Ammo Use
		20.0f,					// Min Damage
		40.0f,					// Max Damage
		20.0f,					// Min Alt damage
		40.0f,					// Max Alt damage
		1.00f,					// Reload time
		0.0f,					// Alt reload time
		{0.0f, 0.0f},			// Spread
		{0.0f, 0.0f},			// Alt Spread
		0.0f,					// Projectile Velocity
		0.0f,					// Alt Projectile Velocity
		1.0f,		    		// Range
		1.0f,	    			// Alt Range
		1,						// Shots per fire
		1,						// Alt Shots per fire
		1,						// Strength or Magic required
		0,						// Two-handed Strength or Magic required
		0,						// Damage Radius (projectiles)
		0,						// Alt Damage Radius (projectiles)
		DFALSE,					// Alt fire zoom
		DFALSE,					// Semi-auto
		"Sounds\\Enemies\\Zealot\\shield.wav",	// Fire sound
		"Sounds\\Enemies\\Zealot\\shield.wav",	// Alt Fire sound
		"Sounds\\Enemies\\Zealot\\shield.wav",	// Empty weapon sound
		"Sounds\\Enemies\\Zealot\\shield.wav",	// Alt Empty weapon sound
		NULL,					// Projectile class
		NULL,					// Alt Projectile class
		0,						// Muzzle Flash radius
		{0.0f, 0.0f, 0.0f},		// Muzzle Flash color
		"Shield",				// Name
		0,						// Name ID (resource)
		"",						// flash sprite name
		"",						// alt flash Sprite name
		0.25f,					// Flash duration
		0.10f,					// Flash scale
		{0.0f, 0.0f, 0.0f},		// 3rd person gun offset
		{0.0f, -1.4f, 0.3f},	// 1st person gun Offset
		{0.0f, 0.0f, 0.0f},		// Muzzle Offset
		{0.03f, 0.03f, 0.15f},	// Recoil
		{0.0f, 0.0f, 0.1f},		// Flash position
		0.625f,					// Eject interval
		0.0f,					// View kick
		DFALSE,					// View kick is Cumulative
		DFALSE,					// Animation looping
		DFALSE,					// Alt anim looping
		DNULL,					// Icon for status bar
		DNULL,					// Highlighted icon for status bar
	},
	// Zealot energy blast
	{
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Skins\\Weapons\\knifec_pv_t.dtx",	// Skin
		DNULL,								// Model
		DNULL,								// Skin
		WEAP_ZEALOT_ENERGYBLAST,// Type
		TYPE_MELEE,				// Fire type
		AMMO_NONE,	    		// Ammo type
		1,						// Ammo Use
		1,						// Alt Ammo Use
		25.0f,					// Min Damage
		50.0f,					// Max Damage
		25.0f,					// Min Alt damage
		50.0f,					// Max Alt damage
		0.50f,					// Reload time
		0.50f,					// Alt reload time
		{0.0f, 0.0f},			// Spread
		{0.0f, 0.0f},			// Alt Spread
		1000.0f,				// Projectile Velocity
		1000.0f,				// Alt Projectile Velocity
		1000.0f,	    		// Range
		1000.0f,    			// Alt Range
		1,						// Shots per fire
		1,						// Alt Shots per fire
		1,						// Strength or Magic required
		0,						// Two-handed Strength or Magic required
		175,						// Damage Radius (projectiles)
		175,						// Alt Damage Radius (projectiles)
		DFALSE,					// Alt fire zoom
		DFALSE,					// Semi-auto
		"Sounds\\Enemies\\Zealot\\distfire.wav",// Fire sound
		"Sounds\\Enemies\\Zealot\\distfire.wav",	// Alt Fire sound
		"Sounds\\Enemies\\Zealot\\distfire.wav",	// Empty weapon sound
		"Sounds\\Enemies\\Zealot\\distfire.wav",	// Alt Empty weapon sound
		"CEnergyBlastProjectile",	// Projectile class
		"CEnergyBlastProjectile",	// Alt Projectile class
		20,						// Muzzle Flash radius
		{0.0f, 0.0f, 1.0f},		// Muzzle Flash color
		"Energy_Blast",			// Name
		0,						// Name ID (resource)
		"",						// flash sprite name
		"",						// alt flash Sprite name
		0.25f,					// Flash duration
		0.40f,					// Flash scale
		{0.0f, 0.0f, 0.0f},		// 3rd person gun offset
		{0.0f, -1.4f, 0.3f},	// 1st person gun Offset
		{0.0f, 0.0f, 0.0f},		// Muzzle Offset
		{0.03f, 0.03f, 0.15f},	// Recoil
		{0.0f, 0.0f, 0.1f},		// Flash position
		0.625f,					// Eject interval
		0.0f,					// View kick
		DFALSE,					// View kick is Cumulative
		DFALSE,					// Animation looping
		DFALSE,					// Alt anim looping
		DNULL,					// Icon for status bar
		DNULL,					// Highlighted icon for status bar
	},
	// Zealot Ground fire attack
	{
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Skins\\Weapons\\knifec_pv_t.dtx",	// Skin
		DNULL,								// Model
		DNULL,								// Skin
		WEAP_ZEALOT_GROUNDFIRE, // Type
		TYPE_MELEE,				// Fire type
		AMMO_NONE,	    		// Ammo type
		1,						// Ammo Use
		1,						// Alt Ammo Use
		30.0f,					// Min Damage
		60.0f,					// Max Damage
		30.0f,					// Min Alt damage
		60.0f,					// Max Alt damage
		0.50f,					// Reload time
		0.50f,					// Alt reload time
		{0.0f, 0.0f},			// Spread
		{0.0f, 0.0f},			// Alt Spread
		1000.0f,				// Projectile Velocity
		1000.0f,				// Alt Projectile Velocity
		1000.0f,	    		// Range
		1000.0f,    			// Alt Range
		1,						// Shots per fire
		1,						// Alt Shots per fire
		1,						// Strength or Magic required
		0,						// Two-handed Strength or Magic required
		20,						// Damage Radius (projectiles)
		20,						// Alt Damage Radius (projectiles)
		DFALSE,					// Alt fire zoom
		DFALSE,					// Semi-auto
		"Sounds\\Enemies\\Zealot\\fireball.wav",// Fire sound
		"Sounds\\Enemies\\Zealot\\fireball.wav",	// Alt Fire sound
		"Sounds\\Enemies\\Zealot\\fireball.wav",	// Empty weapon sound
		"Sounds\\Enemies\\Zealot\\fireball.wav",	// Alt Empty weapon sound
		"CGroundStrikeProjectile",	// Projectile class
		"CGroundStrikeProjectile",	// Alt Projectile class
		20,						// Muzzle Flash radius
		{0.0f, 0.0f, 1.0f},		// Muzzle Flash color
		"Ground_Fire",			// Name
		0,						// Name ID (resource)
		"",						// flash sprite name
		"",						// alt flash Sprite name
		0.25f,					// Flash duration
		0.40f,					// Flash scale
		{0.0f, 0.0f, 0.0f},		// 3rd person gun offset
		{0.0f, -1.4f, 0.3f},	// 1st person gun Offset
		{0.0f, 0.0f, 0.0f},		// Muzzle Offset
		{0.03f, 0.03f, 0.15f},	// Recoil
		{0.0f, 0.0f, 0.1f},		// Flash position
		0.625f,					// Eject interval
		0.0f,					// View kick
		DFALSE,					// View kick is Cumulative
		DFALSE,					// Animation looping
		DFALSE,					// Alt anim looping
		DNULL,					// Icon for status bar
		DNULL,					// Highlighted icon for status bar
	},
	// Zealot Shockwave
	{
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Skins\\Weapons\\knifec_pv_t.dtx",	// Skin
		DNULL,								// Model
		DNULL,								// Skin
		WEAP_ZEALOT_SHOCKWAVE,	// Type
		TYPE_MELEE,				// Fire type
		AMMO_NONE,	    		// Ammo type
		1,						// Ammo Use
		1,						// Alt Ammo Use
		20.0f,					// Min Damage
		40.0f,					// Max Damage
		20.0f,					// Min Alt damage
		40.0f,					// Max Alt damage
		0.50f,					// Reload time
		0.50f,					// Alt reload time
		{0.0f, 0.0f},			// Spread
		{0.0f, 0.0f},			// Alt Spread
		0.0f,					// Projectile Velocity
		0.0f,					// Alt Projectile Velocity
		100.0f,		    		// Range
		100.0f,	    			// Alt Range
		1,						// Shots per fire
		1,						// Alt Shots per fire
		1,						// Strength or Magic required
		0,						// Two-handed Strength or Magic required
		0,						// Damage Radius (projectiles)
		0,						// Alt Damage Radius (projectiles)
		DFALSE,					// Alt fire zoom
		DFALSE,					// Semi-auto
		"Sounds\\Enemies\\Zealot\\magic.wav",	// Fire sound
		"Sounds\\Enemies\\Zealot\\magic.wav",	// Alt Fire sound
		"Sounds\\Enemies\\Zealot\\magic.wav",	// Empty weapon sound
		"Sounds\\Enemies\\Zealot\\magic.wav",	// Alt Empty weapon sound
		"CShockwaveProjectile",	// Projectile class
		"CShockwaveProjectile",	// Alt Projectile class
		0,						// Muzzle Flash radius
		{0.0f, 0.0f, 0.0f},		// Muzzle Flash color
		"Shockwave",			// Name
		0,						// Name ID (resource)
		"",						// flash sprite name
		"",						// alt flash Sprite name
		0.25f,					// Flash duration
		0.10f,					// Flash scale
		{0.0f, 0.0f, 0.0f},		// 3rd person gun offset
		{0.0f, -1.4f, 0.3f},	// 1st person gun Offset
		{0.0f, 0.0f, 0.0f},		// Muzzle Offset
		{0.03f, 0.03f, 0.15f},	// Recoil
		{0.0f, 0.0f, 0.1f},		// Flash position
		0.625f,					// Eject interval
		0.0f,					// View kick
		DFALSE,					// View kick is Cumulative
		DFALSE,					// Animation looping
		DFALSE,					// Alt anim looping
		DNULL,					// Icon for status bar
		DNULL,					// Highlighted icon for status bar
	},
	// Drudge Fireball
	{
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Skins\\Weapons\\knifec_pv_t.dtx",	// Skin
		DNULL,								// Model
		DNULL,								// Skin
		WEAP_DRUDGE_FIREBALL,	// Type
		TYPE_RIFLE,				// Fire type
		AMMO_NONE,	    		// Ammo type
		1,						// Ammo Use
		1,						// Alt Ammo Use
		40.0f,					// Min Damage
		60.0f,					// Max Damage
		40.0f,					// Min Alt damage
		60.0f,					// Max Alt damage
		0.50f,					// Reload time
		0.50f,					// Alt reload time
		{0.0f, 0.0f},			// Spread
		{0.0f, 0.0f},			// Alt Spread
		750.0f,					// Projectile Velocity
		750.0f,					// Alt Projectile Velocity
		1000.0f,	    		// Range
		1000.0f,	   			// Alt Range
		1,						// Shots per fire
		1,						// Alt Shots per fire
		1,						// Strength or Magic required
		0,						// Two-handed Strength or Magic required
		300,					// Damage Radius (projectiles)
		300,					// Alt Damage Radius (projectiles)
		DFALSE,					// Alt fire zoom
		DFALSE,					// Semi-auto
		"Sounds\\Weapons\\Napalm\\fire.wav",// Fire sound
		"Sounds\\Weapons\\Napalm\\fire.wav",	// Alt Fire sound
		"",	// Empty weapon sound
		"",	// Alt Empty weapon sound
		"CNapalmProjectile",	// Projectile class
		"CNapalmProjectile",	// Alt Projectile class
		20,						// Muzzle Flash radius
		{0.8f, 0.3f, 0.0f},		// Muzzle Flash color
		"Fireball",				// Name
		0,						// Name ID (resource)
		"",						// flash sprite name
		"",						// alt flash Sprite name
		0.25f,					// Flash duration
		0.40f,					// Flash scale
		{0.0f, 0.0f, 0.0f},		// 3rd person gun offset
		{0.0f, -1.4f, 0.3f},	// 1st person gun Offset
		{0.0f, 0.0f, 0.0f},		// Muzzle Offset
		{0.03f, 0.03f, 0.15f},	// Recoil
		{0.0f, 0.0f, 0.1f},		// Flash position
		0.625f,					// Eject interval
		0.0f,					// View kick
		DFALSE,					// View kick is Cumulative
		DFALSE,					// Animation looping
		DFALSE,					// Alt anim looping
		DNULL,					// Icon for status bar
		DNULL,					// Highlighted icon for status bar
	},
	// Drudge Lightning
	{
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Skins\\Weapons\\knifec_pv_t.dtx",	// Skin
		DNULL,								// Model
		DNULL,								// Skin
		WEAP_DRUDGE_LIGHTNING,	// Type
		TYPE_RIFLE,				// Fire type
		AMMO_NONE,	    	// Ammo type
		1,						// Ammo Use
		1,						// Alt Ammo Use
		20.0f,					// Min Damage
		40.0f,					// Max Damage
		20.0f,					// Min Alt damage
		40.0f,					// Max Alt damage
		0.50f,					// Reload time
		0.50f,					// Alt reload time
		{0.0f, 0.0f},			// Spread
		{0.0f, 0.0f},			// Alt Spread
		500.0f,					// Projectile Velocity
		500.0f,					// Alt Projectile Velocity
		3000.0f,		    	// Range
		3000.0f,	    		// Alt Range
		1,						// Shots per fire
		1,						// Alt Shots per fire
		1,						// Strength or Magic required
		0,						// Two-handed Strength or Magic required
		200,					// Damage Radius (projectiles)
		200,					// Alt Damage Radius (projectiles)
		DFALSE,					// Alt fire zoom
		DFALSE,					// Semi-auto
		"Sounds\\Weapons\\Tesla\\fire.wav",// Fire sound
		"Sounds\\Weapons\\Tesla\\fire.wav",	// Alt Fire sound
		"",	// Empty weapon sound
		"",	// Alt Empty weapon sound
		DNULL,					// Projectile class
		DNULL,					// Alt Projectile class
		20,						// Muzzle Flash radius
		{0.8f, 0.3f, 0.0f},		// Muzzle Flash color
		"Lightning",			// Name
		0,						// Name ID (resource)
		"",						// flash sprite name
		"",						// alt flash Sprite name
		0.25f,					// Flash duration
		0.40f,					// Flash scale
		{0.0f, 0.0f, 0.0f},		// 3rd person gun offset
		{0.0f, -1.4f, 0.3f},	// 1st person gun Offset
		{0.0f, 0.0f, 0.0f},		// Muzzle Offset
		{0.03f, 0.03f, 0.15f},	// Recoil
		{0.0f, 0.0f, 0.1f},		// Flash position
		0.625f,					// Eject interval
		0.0f,					// View kick
		DFALSE,					// View kick is Cumulative
		DFALSE,					// Animation looping
		DFALSE,					// Alt anim looping
		DNULL,					// Icon for status bar
		DNULL,					// Highlighted icon for status bar
	},
	// Hand Squeeze
	{
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Skins\\Weapons\\knifec_pv_t.dtx",	// Skin
		DNULL,								// Model
		DNULL,								// Skin
		WEAP_HAND_SQUEEZE, 		// Type
		TYPE_MELEE,				// Fire type
		AMMO_NONE,	    		// Ammo type
		0,						// Ammo Use
		0,						// Alt Ammo Use
		4.0f,					// Min Damage
		8.0f,					// Max Damage
		4.0f,					// Min Alt damage
		8.0f,					// Max Alt damage
		1.00f,					// Reload time
		0.0f,					// Alt reload time
		{0.0f, 0.0f},			// Spread
		{0.0f, 0.0f},			// Alt Spread
		0.0f,					// Projectile Velocity
		0.0f,					// Alt Projectile Velocity
		100.0f,		    		// Range
		100.0f,	    			// Alt Range
		1,						// Shots per fire
		1,						// Alt Shots per fire
		1,						// Strength or Magic required
		0,						// Two-handed Strength or Magic required
		0,						// Damage Radius (projectiles)
		0,						// Alt Damage Radius (projectiles)
		DFALSE,					// Alt fire zoom
		DFALSE,					// Semi-auto
		"Sounds\\Weapons\\Knife\\fire.wav",	// Fire sound
		"Sounds\\Weapons\\Knife\\alt.wav",	// Alt Fire sound
		"",	// Empty weapon sound
		"",	// Alt Empty weapon sound
		NULL,					// Projectile class
		NULL,					// Alt Projectile class
		0,						// Muzzle Flash radius
		{0.0f, 0.0f, 0.0f},		// Muzzle Flash color
		"Hand_Squeeze",			// Name
		0,						// Name ID (resource)
		"",						// flash sprite name
		"",						// alt flash Sprite name
		0.25f,					// Flash duration
		0.10f,					// Flash scale
		{0.0f, 0.0f, 0.0f},		// 3rd person gun offset
		{0.0f, -1.4f, 0.3f},	// 1st person gun Offset
		{0.0f, 0.0f, 0.0f},		// Muzzle Offset
		{0.03f, 0.03f, 0.15f},	// Recoil
		{0.0f, 0.0f, 0.1f},		// Flash position
		0.625f,					// Eject interval
		0.0f,					// View kick
		DFALSE,					// View kick is Cumulative
		DFALSE,					// Animation looping
		DFALSE,					// Alt anim looping
		DNULL,					// Icon for status bar
		DNULL,					// Highlighted icon for status bar
	},
	// Bone Leech Suck
	{
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Skins\\Weapons\\knifec_pv_t.dtx",	// Skin
		DNULL,								// Model
		DNULL,								// Skin
		WEAP_BONELEECH_SUCK, 		// Type
		TYPE_MELEE,				// Fire type
		AMMO_NONE,	    		// Ammo type
		0,						// Ammo Use
		0,						// Alt Ammo Use
		4.0f,					// Min Damage
		8.0f,					// Max Damage
		4.0f,					// Min Alt damage
		8.0f,					// Max Alt damage
		1.00f,					// Reload time
		0.0f,					// Alt reload time
		{0.0f, 0.0f},			// Spread
		{0.0f, 0.0f},			// Alt Spread
		0.0f,					// Projectile Velocity
		0.0f,					// Alt Projectile Velocity
		100.0f,		    		// Range
		100.0f,	    			// Alt Range
		1,						// Shots per fire
		1,						// Alt Shots per fire
		1,						// Strength or Magic required
		0,						// Two-handed Strength or Magic required
		0,						// Damage Radius (projectiles)
		0,						// Alt Damage Radius (projectiles)
		DFALSE,					// Alt fire zoom
		DFALSE,					// Semi-auto
		"Sounds\\Weapons\\Knife\\fire.wav",	// Fire sound
		"Sounds\\Weapons\\Knife\\alt.wav",	// Alt Fire sound
		"",	// Empty weapon sound
		"",	// Alt Empty weapon sound
		NULL,					// Projectile class
		NULL,					// Alt Projectile class
		0,						// Muzzle Flash radius
		{0.0f, 0.0f, 0.0f},		// Muzzle Flash color
		"Boneleech_Suck",		// Name
		0,						// Name ID (resource)
		"",						// flash sprite name
		"",						// alt flash Sprite name
		0.25f,					// Flash duration
		0.10f,					// Flash scale
		{0.0f, 0.0f, 0.0f},		// 3rd person gun offset
		{0.0f, -1.4f, 0.3f},	// 1st person gun Offset
		{0.0f, 0.0f, 0.0f},		// Muzzle Offset
		{0.03f, 0.03f, 0.15f},	// Recoil
		{0.0f, 0.0f, 0.1f},		// Flash position
		0.625f,					// Eject interval
		0.0f,					// View kick
		DFALSE,					// View kick is Cumulative
		DFALSE,					// Animation looping
		DFALSE,					// Alt anim looping
		DNULL,					// Icon for status bar
		DNULL,					// Highlighted icon for status bar
	},
	// Thief Suck
	{
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Skins\\Weapons\\knifec_pv_t.dtx",	// Skin
		DNULL,								// Model
		DNULL,								// Skin
		WEAP_THIEF_SUCK, 		// Type
		TYPE_MELEE,				// Fire type
		AMMO_NONE,	    		// Ammo type
		0,						// Ammo Use
		0,						// Alt Ammo Use
		4.0f,					// Min Damage
		8.0f,					// Max Damage
		4.0f,					// Min Alt damage
		8.0f,					// Max Alt damage
		1.00f,					// Reload time
		0.0f,					// Alt reload time
		{0.0f, 0.0f},			// Spread
		{0.0f, 0.0f},			// Alt Spread
		0.0f,					// Projectile Velocity
		0.0f,					// Alt Projectile Velocity
		100.0f,		    		// Range
		100.0f,	    			// Alt Range
		1,						// Shots per fire
		1,						// Alt Shots per fire
		1,						// Strength or Magic required
		0,						// Two-handed Strength or Magic required
		0,						// Damage Radius (projectiles)
		0,						// Alt Damage Radius (projectiles)
		DFALSE,					// Alt fire zoom
		DFALSE,					// Semi-auto
		"Sounds\\Weapons\\Knife\\fire.wav",	// Fire sound
		"Sounds\\Weapons\\Knife\\alt.wav",	// Alt Fire sound
		"",	// Empty weapon sound
		"",	// Alt Empty weapon sound
		NULL,					// Projectile class
		NULL,					// Alt Projectile class
		0,						// Muzzle Flash radius
		{0.0f, 0.0f, 0.0f},		// Muzzle Flash color
		"Thief_Suck",			// Name
		0,						// Name ID (resource)
		"",						// flash sprite name
		"",						// alt flash Sprite name
		0.25f,					// Flash duration
		0.10f,					// Flash scale
		{0.0f, 0.0f, 0.0f},		// 3rd person gun offset
		{0.0f, -1.4f, 0.3f},	// 1st person gun Offset
		{0.0f, 0.0f, 0.0f},		// Muzzle Offset
		{0.03f, 0.03f, 0.15f},	// Recoil
		{0.0f, 0.0f, 0.1f},		// Flash position
		0.625f,					// Eject interval
		0.0f,					// View kick
		DFALSE,					// View kick is Cumulative
		DFALSE,					// Animation looping
		DFALSE,					// Alt anim looping
		DNULL,					// Icon for status bar
		DNULL,					// Highlighted icon for status bar
	},
	// Nightmare
	{
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Skins\\Weapons\\knifec_pv_t.dtx",	// Skin
		DNULL,								// Model
		DNULL,								// Skin
		WEAP_MELEE,	    		// Type
		TYPE_MELEE,				// Fire type
		AMMO_NONE,	    		// Ammo type
		1,						// Ammo Use
		1,						// Alt Ammo Use
		40.0f,					// Min Damage
		80.0f,					// Max Damage
		40.0f,					// Min Alt damage
		80.0f,					// Max Alt damage
		1.00f,					// Reload time
		0.0f,					// Alt reload time
		{0.0f, 0.0f},			// Spread
		{0.0f, 0.0f},			// Alt Spread
		0.0f,					// Projectile Velocity
		0.0f,					// Alt Projectile Velocity
		100.0f,		    		// Range
		100.0f,	    			// Alt Range
		1,						// Shots per fire
		1,						// Alt Shots per fire
		1,						// Strength or Magic required
		0,						// Two-handed Strength or Magic required
		0,						// Damage Radius (projectiles)
		0,						// Alt Damage Radius (projectiles)
		DFALSE,					// Alt fire zoom
		DFALSE,					// Semi-auto
		"Sounds\\Weapons\\Knife\\fire.wav",	// Fire sound
		"Sounds\\Weapons\\Knife\\alt.wav",	// Alt Fire sound
		"",	// Empty weapon sound
		"",	// Alt Empty weapon sound
		NULL,					// Projectile class
		NULL,					// Alt Projectile class
		0,						// Muzzle Flash radius
		{0.0f, 0.0f, 0.0f},		// Muzzle Flash color
		"Nightmare_Bite",		// Name
		0,						// Name ID (resource)
		"",						// flash sprite name
		"",						// alt flash Sprite name
		0.25f,					// Flash duration
		0.10f,					// Flash scale
		{0.0f, 0.0f, 0.0f},		// 3rd person gun offset
		{0.0f, -1.4f, 0.3f},	// 1st person gun Offset
		{0.0f, 0.0f, 0.0f},		// Muzzle Offset
		{0.03f, 0.03f, 0.15f},	// Recoil
		{0.0f, 0.0f, 0.1f},		// Flash position
		0.625f,					// Eject interval
		0.0f,					// View kick
		DFALSE,					// View kick is Cumulative
		DFALSE,					// Animation looping
		DFALSE,					// Alt anim looping
		DNULL,					// Icon for status bar
		DNULL,					// Highlighted icon for status bar
	},
	// Behemoth Shockwave
	{
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Skins\\Weapons\\knifec_pv_t.dtx",	// Skin
		DNULL,								// Model
		DNULL,								// Skin
		WEAP_MELEE,	    		// Type
		TYPE_MELEE,				// Fire type
		AMMO_NONE,	    		// Ammo type
		1,						// Ammo Use
		1,						// Alt Ammo Use
		20.0f,					// Min Damage
		40.0f,					// Max Damage
		20.0f,					// Min Alt damage
		40.0f,					// Max Alt damage
		0.50f,					// Reload time
		0.50f,					// Alt reload time
		{0.0f, 0.0f},			// Spread
		{0.0f, 0.0f},			// Alt Spread
		0.0f,					// Projectile Velocity
		0.0f,					// Alt Projectile Velocity
		800.0f,		    		// Range
		800.0f,	    			// Alt Range
		1,						// Shots per fire
		1,						// Alt Shots per fire
		1,						// Strength or Magic required
		0,						// Two-handed Strength or Magic required
		0,						// Damage Radius (projectiles)
		0,						// Alt Damage Radius (projectiles)
		DFALSE,					// Alt fire zoom
		DFALSE,					// Semi-auto
		"",						// Fire sound
		"",						// Alt Fire sound
		"",						// Empty weapon sound
		"",						// Alt Empty weapon sound
		"CBehemothShockwaveProjectile",	// Projectile class
		"CBehemothShockwaveProjectile",	// Alt Projectile class
		0,						// Muzzle Flash radius
		{0.0f, 0.0f, 0.0f},		// Muzzle Flash color
		"Behemoth_Shockwave",	// Name
		0,						// Name ID (resource)
		"",						// flash sprite name
		"",						// alt flash Sprite name
		0.25f,					// Flash duration
		0.10f,					// Flash scale
		{0.0f, 0.0f, 0.0f},		// 3rd person gun offset
		{0.0f, -1.4f, 0.3f},	// 1st person gun Offset
		{0.0f, 0.0f, 0.0f},		// Muzzle Offset
		{0.03f, 0.03f, 0.15f},	// Recoil
		{0.0f, 0.0f, 0.1f},		// Flash position
		0.625f,					// Eject interval
		0.0f,					// View kick
		DFALSE,					// View kick is Cumulative
		DFALSE,					// Animation looping
		DFALSE,					// Alt anim looping
		DNULL,					// Icon for status bar
		DNULL,					// Highlighted icon for status bar
	},
	// Deathshroud zap
	{
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Skins\\Weapons\\knifec_pv_t.dtx",	// Skin
		DNULL,								// Model
		DNULL,								// Skin
		WEAP_DEATHSHROUD_ZAP,	// Type
		TYPE_MELEE,				// Fire type
		AMMO_NONE,		    	// Ammo type
		1,						// Ammo Use
		1,						// Alt Ammo Use
		20.0f,					// Min Damage
		40.0f,					// Max Damage
		20.0f,					// Min Alt damage
		40.0f,					// Max Alt damage
		0.50f,					// Reload time
		0.50f,					// Alt reload time
		{0.0f, 0.0f},			// Spread
		{0.0f, 0.0f},			// Alt Spread
		500.0f,					// Projectile Velocity
		500.0f,					// Alt Projectile Velocity
		3000.0f,		    	// Range
		3000.0f,	    		// Alt Range
		1,						// Shots per fire
		1,						// Alt Shots per fire
		1,						// Strength or Magic required
		0,						// Two-handed Strength or Magic required
		20,						// Damage Radius (projectiles)
		20,						// Alt Damage Radius (projectiles)
		DFALSE,					// Alt fire zoom
		DFALSE,					// Semi-auto
		"Sounds\\Weapons\\Napalm\\fire.wav",// Fire sound
		"Sounds\\Weapons\\Napalm\\fire.wav",// Alt Fire sound
		"",	// Empty weapon sound
		"",	// Alt Empty weapon sound
		DNULL,					// Projectile class
		DNULL,					// Alt Projectile class
		20,						// Muzzle Flash radius
		{0.8f, 0.3f, 0.0f},		// Muzzle Flash color
		"Lightning",			// Name
		0,						// Name ID (resource)
		"",						// flash sprite name
		"",						// alt flash Sprite name
		0.25f,					// Flash duration
		0.40f,					// Flash scale
		{0.0f, 0.0f, 0.0f},		// 3rd person gun offset
		{0.0f, -1.4f, 0.3f},	// 1st person gun Offset
		{0.0f, 0.0f, 0.0f},		// Muzzle Offset
		{0.03f, 0.03f, 0.15f},	// Recoil
		{0.0f, 0.0f, 0.1f},		// Flash position
		0.625f,					// Eject interval
		0.0f,					// View kick
		DFALSE,					// View kick is Cumulative
		DFALSE,					// Animation looping
		DFALSE,					// Alt anim looping
		DNULL,					// Icon for status bar
		DNULL,					// Highlighted icon for status bar
	},
	// Naga Eye Beam
   	{
		"Models\\Weapons\\Laserrifle_pv.abc",	// Model
		"Models\\Weapons\\Laserrifle_pv.abc",	// Model
		"Skins\\Weapons\\C_DeathRay_pv_t.dtx",	// Skin
		DNULL,								// Model
		DNULL,								// Skin
		WEAP_NAGA_EYEBEAM,		// Type
		TYPE_MELEE, 			// Fire type
		AMMO_NONE,				// Ammo type
		1,						// Ammo Use
		1,						// Alt Ammo Use
		30.0f,					// Min Damage
		45.0f,					// Max Damage
		30.0f,					// Min Alt damage
		45.0f,					// Max Alt damage
		0.2f,					// Reload time
		0.05f,					// Alt reload time
		{0.0f, 0.0f},			// Spread
		{0.0f, 0.0f},			// Alt Spread
		0.0f,					// Projectile Velocity
		0.0f,					// Alt Projectile Velocity
		2000.0f,				// Range
		2000.0f,				// Alt Range
		1,						// Shots per fire
		1,						// Alt Shots per fire
		1,						// Strength or Magic required
		0,						// Two-handed Strength or Magic required
		0,						// Damage Radius (projectiles)
		0,						// Alt Damage Radius (projectiles)
		DFALSE,					// Alt fire zoom
		DFALSE,					// Semi-auto
		"Sounds\\Weapons\\laser\\fire.wav",	// Fire sound
		"",						// Alt Fire sound
//		"Sounds\\Weapons\\laser\\fire.wav",	// Alt Fire sound
		"",	// Empty weapon sound
		"",	// Alt Empty weapon sound
		NULL,					// Projectile class
		NULL,					// Alt Projectile class
		0,						// Muzzle Flash radius
		{0.0f, 0.0f, 0.0f},		// Muzzle Flash color
		"Naga_Beam",			// Name
		0,						// Name ID (resource)
		"",						// flash sprite name
		"",						// alt flash Sprite name
		0.25f,					// Flash duration
		0.10f,					// Flash scale
		{0.0f, 0.0f, 0.0f},		// 3rd person gun offset
		{0.7f, -0.9f, 2.1f},	// Gun Offset
		{0.0f, 0.0f, 0.0f},		// Muzzle Offset
		{0.03f, 0.03f, 0.15f},	// Recoil
		{4.25f, -4.25f, 25.0f},	// Flash position
		0.625f,					// Eject interval
		0.0f,					// View kick
		DFALSE,					// View kick is Cumulative
		DFALSE,					// Animation looping
		DTRUE,					// Alt anim looping
		DNULL,					// Icon for status bar
		DNULL,					// Highlighted icon for status bar
	},
	// Naga spike
	{
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Skins\\Weapons\\knifec_pv_t.dtx",	// Skin
		DNULL,								// Model
		DNULL,								// Skin
		WEAP_NAGA_SPIKE,   		// Type
		TYPE_MAGIC,				// Fire type
		AMMO_NONE,	    		// Ammo type
		1,						// Ammo Use
		1,						// Alt Ammo Use
		60.0f,					// Min Damage
		100.0f,					// Max Damage
		60.0f,					// Min Alt damage
		100.0f,					// Max Alt damage
		0.50f,					// Reload time
		0.50f,					// Alt reload time
		{0.0f, 0.0f},			// Spread
		{0.0f, 0.0f},			// Alt Spread
		2000.0f,				// Projectile Velocity
		2000.0f,				// Alt Projectile Velocity
		800.0f,		    		// Range
		800.0f,	    			// Alt Range
		1,						// Shots per fire
		1,						// Alt Shots per fire
		1,						// Strength or Magic required
		0,						// Two-handed Strength or Magic required
		200,					// Damage Radius (projectiles)
		200,					// Alt Damage Radius (projectiles)
		DFALSE,					// Alt fire zoom
		DFALSE,					// Semi-auto
		"Sounds\\Weapons\\Knife\\fire.wav",	// Fire sound
		"Sounds\\Weapons\\Knife\\alt.wav",	// Alt Fire sound
		"",	// Empty weapon sound
		"",	// Alt Empty weapon sound
		"CNagaSpikeProjectile",	// Projectile class
		"CNagaSpikeProjectile",	// Alt Projectile class
		0,						// Muzzle Flash radius
		{0.0f, 0.0f, 0.0f},		// Muzzle Flash color
		"Naga_Spike",			// Name
		0,						// Name ID (resource)
		"",						// flash sprite name
		"",						// alt flash Sprite name
		0.25f,					// Flash duration
		0.10f,					// Flash scale
		{0.0f, 0.0f, 0.0f},		// 3rd person gun offset
		{0.0f, -1.4f, 0.3f},	// 1st person gun Offset
		{0.0f, 0.0f, 0.0f},		// Muzzle Offset
		{0.03f, 0.03f, 0.15f},	// Recoil
		{0.0f, 0.0f, 0.1f},		// Flash position
		0.625f,					// Eject interval
		0.0f,					// View kick
		DFALSE,					// View kick is Cumulative
		DFALSE,					// Animation looping
		DFALSE,					// Alt anim looping
		DNULL,					// Icon for status bar
		DNULL,					// Highlighted icon for status bar
	},
	// Naga debris drop
	{
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Skins\\Weapons\\knifec_pv_t.dtx",	// Skin
		DNULL,								// Model
		DNULL,								// Skin
		WEAP_NAGA_DEBRIS,  		// Type
		TYPE_MELEE,				// Fire type
		AMMO_NONE,	    		// Ammo type
		1,						// Ammo Use
		1,						// Alt Ammo Use
		75.0f,					// Min Damage
		100.0f,					// Max Damage
		75.0f,					// Min Alt damage
		100.0f,					// Max Alt damage
		0.50f,					// Reload time
		0.50f,					// Alt reload time
		{0.0f, 0.0f},			// Spread
		{0.0f, 0.0f},			// Alt Spread
		0.0f,					// Projectile Velocity
		0.0f,					// Alt Projectile Velocity
		800.0f,		    		// Range
		800.0f,	    			// Alt Range
		1,						// Shots per fire
		1,						// Alt Shots per fire
		1,						// Strength or Magic required
		0,						// Two-handed Strength or Magic required
		0,						// Damage Radius (projectiles)
		0,						// Alt Damage Radius (projectiles)
		DFALSE,					// Alt fire zoom
		DFALSE,					// Semi-auto
		"Sounds\\Weapons\\Knife\\fire.wav",	// Fire sound
		"Sounds\\Weapons\\Knife\\alt.wav",	// Alt Fire sound
		"",	// Empty weapon sound
		"",	// Alt Empty weapon sound
		"CNagaDebrisProjectile",// Projectile class
		"CNagaDebrisProjectile",// Alt Projectile class
		0,						// Muzzle Flash radius
		{0.0f, 0.0f, 0.0f},		// Muzzle Flash color
		"Naga_Debris",			// Name
		0,						// Name ID (resource)
		"",						// flash sprite name
		"",						// alt flash Sprite name
		0.25f,					// Flash duration
		0.10f,					// Flash scale
		{0.0f, 0.0f, 0.0f},		// 3rd person gun offset
		{0.0f, -1.4f, 0.3f},	// 1st person gun Offset
		{0.0f, 0.0f, 0.0f},		// Muzzle Offset
		{0.03f, 0.03f, 0.15f},	// Recoil
		{0.0f, 0.0f, 0.1f},		// Flash position
		0.625f,					// Eject interval
		0.0f,					// View kick
		DFALSE,					// View kick is Cumulative
		DFALSE,					// Animation looping
		DFALSE,					// Alt anim looping
		DNULL,					// Icon for status bar
		DNULL,					// Highlighted icon for status bar
	},
	// Gideon shield spell
	{
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Skins\\Weapons\\knifec_pv_t.dtx",	// Skin
		DNULL,								// Model
		DNULL,								// Skin
		WEAP_GIDEON_SHIELD, 	// Type
		TYPE_RIFLE,				// Fire type
		AMMO_NONE,	    		// Ammo type
		1,						// Ammo Use
		1,						// Alt Ammo Use
		10.0f,					// Min Damage
		20.0f,					// Max Damage
		10.0f,					// Min Alt damage
		20.0f,					// Max Alt damage
		1.00f,					// Reload time
		0.0f,					// Alt reload time
		{0.0f, 0.0f},			// Spread
		{0.0f, 0.0f},			// Alt Spread
		0.0f,					// Projectile Velocity
		0.0f,					// Alt Projectile Velocity
		75.0f,		    		// Range
		75.0f,	    			// Alt Range
		1,						// Shots per fire
		1,						// Alt Shots per fire
		1,						// Strength or Magic required
		0,						// Two-handed Strength or Magic required
		0,						// Damage Radius (projectiles)
		0,						// Alt Damage Radius (projectiles)
		DFALSE,					// Alt fire zoom
		DFALSE,					// Semi-auto
		"Sounds\\Enemies\\Gideon\\shield.wav",	// Fire sound
		"Sounds\\Enemies\\Gideon\\shield.wav",	// Alt Fire sound
		"Sounds\\Enemies\\Gideon\\shield.wav",	// Empty weapon sound
		"Sounds\\Enemies\\Gideon\\shield.wav",	// Alt Empty weapon sound
		NULL,					// Projectile class
		NULL,					// Alt Projectile class
		0,						// Muzzle Flash radius
		{0.0f, 0.0f, 0.0f},		// Muzzle Flash color
		"Gideon_Shield",		// Name
		0,						// Name ID (resource)
		"",						// flash sprite name
		"",						// alt flash Sprite name
		0.25f,					// Flash duration
		0.10f,					// Flash scale
		{0.0f, 0.0f, 0.0f},		// 3rd person gun offset
		{0.0f, -1.4f, 0.3f},	// 1st person gun Offset
		{0.0f, 0.0f, 0.0f},		// Muzzle Offset
		{0.03f, 0.03f, 0.15f},	// Recoil
		{0.0f, 0.0f, 0.1f},		// Flash position
		0.625f,					// Eject interval
		0.0f,					// View kick
		DFALSE,					// View kick is Cumulative
		DFALSE,					// Animation looping
		DFALSE,					// Alt anim looping
		DNULL,					// Icon for status bar
		DNULL,					// Highlighted icon for status bar
	},
	// Gideon wind spell
	{
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Skins\\Weapons\\knifec_pv_t.dtx",	// Skin
		DNULL,								// Model
		DNULL,								// Skin
		WEAP_GIDEON_WIND,	 	// Type
		TYPE_RIFLE,				// Fire type
		AMMO_NONE,	    		// Ammo type
		1,						// Ammo Use
		1,						// Alt Ammo Use
		10.0f,					// Min Damage
		20.0f,					// Max Damage
		10.0f,					// Min Alt damage
		20.0f,					// Max Alt damage
		1.00f,					// Reload time
		0.0f,					// Alt reload time
		{0.0f, 0.0f},			// Spread
		{0.0f, 0.0f},			// Alt Spread
		0.0f,					// Projectile Velocity
		0.0f,					// Alt Projectile Velocity
		75.0f,		    		// Range
		75.0f,	    			// Alt Range
		1,						// Shots per fire
		1,						// Alt Shots per fire
		1,						// Strength or Magic required
		0,						// Two-handed Strength or Magic required
		0,						// Damage Radius (projectiles)
		0,						// Alt Damage Radius (projectiles)
		DFALSE,					// Alt fire zoom
		DFALSE,					// Semi-auto
		"Sounds\\Enemies\\Gideon\\windfire.wav",	// Fire sound
		"Sounds\\Enemies\\Gideon\\windfire.wav",	// Alt Fire sound
		"Sounds\\Enemies\\Gideon\\windfire.wav",	// Empty weapon sound
		"Sounds\\Enemies\\Gideon\\windfire.wav",	// Alt Empty weapon sound
		"CBehemothShockwaveProjectile",				// Projectile class
		"CBehemothShockwaveProjectile",				// Alt Projectile class
		0,						// Muzzle Flash radius
		{0.0f, 0.0f, 0.0f},		// Muzzle Flash color
		"Gideon_Shield",		// Name
		0,						// Name ID (resource)
		"",						// flash sprite name
		"",						// alt flash Sprite name
		0.25f,					// Flash duration
		0.10f,					// Flash scale
		{0.0f, 0.0f, 0.0f},		// 3rd person gun offset
		{0.0f, -1.4f, 0.3f},	// 1st person gun Offset
		{0.0f, 0.0f, 0.0f},		// Muzzle Offset
		{0.03f, 0.03f, 0.15f},	// Recoil
		{0.0f, 0.0f, 0.1f},		// Flash position
		0.625f,					// Eject interval
		0.0f,					// View kick
		DFALSE,					// View kick is Cumulative
		DFALSE,					// Animation looping
		DFALSE,					// Alt anim looping
		DNULL,					// Icon for status bar
		DNULL,					// Highlighted icon for status bar
	},
	// Undead Gideon vomit attack
	{
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Skins\\Weapons\\knifec_pv_t.dtx",	// Skin
		DNULL,								// Model
		DNULL,								// Skin
		WEAP_GIDEON_VOMIT, 		// Type
		TYPE_RIFLE,				// Fire type
		AMMO_NONE,	    		// Ammo type
		1,						// Ammo Use
		1,						// Alt Ammo Use
		15.0f,					// Min Damage
		40.0f,					// Max Damage
		10.0f,					// Min Alt damage
		20.0f,					// Max Alt damage
		1.00f,					// Reload time
		0.0f,					// Alt reload time
		{0.0f, 0.0f},			// Spread
		{0.0f, 0.0f},			// Alt Spread
		2000.0f,				// Projectile Velocity
		2000.0f,				// Alt Projectile Velocity
		2000.0f,	    		// Range
		2000.0f,    			// Alt Range
		1,						// Shots per fire
		1,						// Alt Shots per fire
		1,						// Strength or Magic required
		0,						// Two-handed Strength or Magic required
		0,						// Damage Radius (projectiles)
		0,						// Alt Damage Radius (projectiles)
		DFALSE,					// Alt fire zoom
		DFALSE,					// Semi-auto
		"Sounds\\Weapons\\Knife\\fire.wav",	// Fire sound
		"Sounds\\Weapons\\Knife\\alt.wav",	// Alt Fire sound
		"",	// Empty weapon sound
		"",	// Alt Empty weapon sound
		"CVomitProjectile",		// Projectile class
		"CVomitProjectile",		// Alt Projectile class
		0,						// Muzzle Flash radius
		{0.0f, 0.0f, 0.0f},		// Muzzle Flash color
		"Gideon_Vomit",			// Name
		0,						// Name ID (resource)
		"",						// flash sprite name
		"",						// alt flash Sprite name
		0.25f,					// Flash duration
		0.10f,					// Flash scale
		{0.0f, 0.0f, 0.0f},		// 3rd person gun offset
		{0.0f, -1.4f, 0.3f},	// 1st person gun Offset
		{0.0f, 0.0f, 0.0f},		// Muzzle Offset
		{0.03f, 0.03f, 0.15f},	// Recoil
		{0.0f, 0.0f, 0.1f},		// Flash position
		0.625f,					// Eject interval
		0.0f,					// View kick
		DFALSE,					// View kick is Cumulative
		DFALSE,					// Animation looping
		DFALSE,					// Alt anim looping
		DNULL,					// Icon for status bar
		DNULL,					// Highlighted icon for status bar
	},
	// Undead Gideon goo attack
	{
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Skins\\Weapons\\knifec_pv_t.dtx",	// Skin
		DNULL,								// Model
		DNULL,								// Skin
		WEAP_GIDEON_GOO, 		// Type
		TYPE_MAGIC,				// Fire type
		AMMO_NONE,	    		// Ammo type
		1,						// Ammo Use
		1,						// Alt Ammo Use
		10.0f,					// Min Damage
		20.0f,					// Max Damage
		10.0f,					// Min Alt damage
		20.0f,					// Max Alt damage
		1.00f,					// Reload time
		0.0f,					// Alt reload time
		{0.0f, 0.0f},			// Spread
		{0.0f, 0.0f},			// Alt Spread
		400.0f,					// Projectile Velocity
		400.0f,					// Alt Projectile Velocity
		2000.0f,	    		// Range
		2000.0f,    			// Alt Range
		1,						// Shots per fire
		1,						// Alt Shots per fire
		1,						// Strength or Magic required
		0,						// Two-handed Strength or Magic required
		0,						// Damage Radius (projectiles)
		0,						// Alt Damage Radius (projectiles)
		DFALSE,					// Alt fire zoom
		DFALSE,					// Semi-auto
		"Sounds\\Weapons\\Knife\\fire.wav",	// Fire sound
		"Sounds\\Weapons\\Knife\\alt.wav",	// Alt Fire sound
		"",	// Empty weapon sound
		"",	// Alt Empty weapon sound
		"CGooProjectile",		// Projectile class
		"CGooProjectile",		// Alt Projectile class
		0,						// Muzzle Flash radius
		{0.0f, 0.0f, 0.0f},		// Muzzle Flash color
		"Gideon_Goo",			// Name
		0,						// Name ID (resource)
		"",						// flash sprite name
		"",						// alt flash Sprite name
		0.25f,					// Flash duration
		0.10f,					// Flash scale
		{0.0f, 0.0f, 0.0f},		// 3rd person gun offset
		{0.0f, -1.4f, 0.3f},	// 1st person gun Offset
		{0.0f, 0.0f, 0.0f},		// Muzzle Offset
		{0.03f, 0.03f, 0.15f},	// Recoil
		{0.0f, 0.0f, 0.1f},		// Flash position
		0.625f,					// Eject interval
		0.0f,					// View kick
		DFALSE,					// View kick is Cumulative
		DFALSE,					// Animation looping
		DFALSE,					// Alt anim looping
		DNULL,					// Icon for status bar
		DNULL,					// Highlighted icon for status bar
	},
	// Undead Gideon spear attack
	{
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Skins\\Weapons\\knifec_pv_t.dtx",	// Skin
		DNULL,								// Model
		DNULL,								// Skin
		WEAP_GIDEON_SPEAR, 		// Type
		TYPE_MELEE,				// Fire type
		AMMO_NONE,	    		// Ammo type
		1,						// Ammo Use
		1,						// Alt Ammo Use
		10.0f,					// Min Damage
		20.0f,					// Max Damage
		10.0f,					// Min Alt damage
		20.0f,					// Max Alt damage
		1.00f,					// Reload time
		0.0f,					// Alt reload time
		{0.0f, 0.0f},			// Spread
		{0.0f, 0.0f},			// Alt Spread
		0.0f,					// Projectile Velocity
		0.0f,					// Alt Projectile Velocity
		200.0f,		    		// Range
		200.0f,	    			// Alt Range
		1,						// Shots per fire
		1,						// Alt Shots per fire
		1,						// Strength or Magic required
		0,						// Two-handed Strength or Magic required
		0,						// Damage Radius (projectiles)
		0,						// Alt Damage Radius (projectiles)
		DFALSE,					// Alt fire zoom
		DFALSE,					// Semi-auto
		"Sounds\\Weapons\\Knife\\fire.wav",	// Fire sound
		"Sounds\\Weapons\\Knife\\alt.wav",	// Alt Fire sound
		"",	// Empty weapon sound
		"",	// Alt Empty weapon sound
		NULL,					// Projectile class
		NULL,					// Alt Projectile class
		0,						// Muzzle Flash radius
		{0.0f, 0.0f, 0.0f},		// Muzzle Flash color
		"Gideon_Spear",			// Name
		0,						// Name ID (resource)
		"",						// flash sprite name
		"",						// alt flash Sprite name
		0.25f,					// Flash duration
		0.10f,					// Flash scale
		{0.0f, 0.0f, 0.0f},		// 3rd person gun offset
		{0.0f, -1.4f, 0.3f},	// 1st person gun Offset
		{0.0f, 0.0f, 0.0f},		// Muzzle Offset
		{0.03f, 0.03f, 0.15f},	// Recoil
		{0.0f, 0.0f, 0.1f},		// Flash position
		0.625f,					// Eject interval
		0.0f,					// View kick
		DFALSE,					// View kick is Cumulative
		DFALSE,					// Animation looping
		DFALSE,					// Alt anim looping
		DNULL,					// Icon for status bar
		DNULL,					// Highlighted icon for status bar
	},
	// Ancient One eyebeam attack
	{
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Skins\\Weapons\\knifec_pv_t.dtx",	// Skin
		DNULL,								// Model
		DNULL,								// Skin
		WEAP_ANCIENTONE_BEAM,	// Type
		TYPE_MELEE,				// Fire type
		AMMO_NONE,	    		// Ammo type
		1,						// Ammo Use
		1,						// Alt Ammo Use
		35.0f,					// Min Damage
		70.0f,					// Max Damage
		35.0f,					// Min Alt damage
		70.0f,					// Max Alt damage
		1.0f,					// Reload time
		0.0f,					// Alt reload time
		{0.0f, 0.0f},			// Spread
		{0.0f, 0.0f},			// Alt Spread
		0.0f,					// Projectile Velocity
		0.0f,					// Alt Projectile Velocity
		12000.0f,	    		// Range
		12000.0f,    			// Alt Range
		1,						// Shots per fire
		1,						// Alt Shots per fire
		1,						// Strength or Magic required
		0,						// Two-handed Strength or Magic required
		0,						// Damage Radius (projectiles)
		0,						// Alt Damage Radius (projectiles)
		DFALSE,					// Alt fire zoom
		DFALSE,					// Semi-auto
		"",						// Fire sound
		"",						// Alt Fire sound
		"",						// Empty weapon sound
		"",						// Alt Empty weapon sound
		NULL,					// Projectile class
		NULL,					// Alt Projectile class
		0,						// Muzzle Flash radius
		{0.0f, 0.0f, 0.0f},		// Muzzle Flash color
		"Ancientone_Beam",		// Name
		0,						// Name ID (resource)
		"",						// flash sprite name
		"",						// alt flash Sprite name
		0.25f,					// Flash duration
		0.10f,					// Flash scale
		{0.0f, 0.0f, 0.0f},		// 3rd person gun offset
		{0.0f, -1.4f, 0.3f},	// 1st person gun Offset
		{0.0f, 0.0f, 0.0f},		// Muzzle Offset
		{0.03f, 0.03f, 0.15f},	// Recoil
		{0.0f, 0.0f, 0.1f},		// Flash position
		0.625f,					// Eject interval
		0.0f,					// View kick
		DFALSE,					// View kick is Cumulative
		DFALSE,					// Animation looping
		DFALSE,					// Alt anim looping
		DNULL,					// Icon for status bar
		DNULL,					// Highlighted icon for status bar
	},
	// AncientOne Tentacle
	{
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Skins\\Weapons\\knifec_pv_t.dtx",	// Skin
		DNULL,								// Model
		DNULL,								// Skin
		WEAP_ANCIENTONE_TENTACLE, 	// Type
		TYPE_MELEE,				// Fire type
		AMMO_NONE,	    		// Ammo type
		1,						// Ammo Use
		1,						// Alt Ammo Use
		15.0f,					// Min Damage
		20.0f,					// Max Damage
		20.0f,					// Min Alt damage
		40.0f,					// Max Alt damage
		0.01f,					// Reload time
		0.0f,					// Alt reload time
		{0.0f, 0.0f},			// Spread
		{0.0f, 0.0f},			// Alt Spread
		0.0f,					// Projectile Velocity
		0.0f,					// Alt Projectile Velocity
		800.0f,	    		// Range
		800.0f,	   			// Alt Range
		1,						// Shots per fire
		1,						// Alt Shots per fire
		1,						// Strength or Magic required
		0,						// Two-handed Strength or Magic required
		0,						// Damage Radius (projectiles)
		0,						// Alt Damage Radius (projectiles)
		DFALSE,					// Alt fire zoom
		DFALSE,					// Semi-auto
		"Sounds\\Weapons\\Knife\\fire.wav",	// Fire sound
		"Sounds\\Weapons\\Knife\\alt.wav",	// Alt Fire sound
		"",	// Empty weapon sound
		"",	// Alt Empty weapon sound
		NULL,					// Projectile class
		NULL,					// Alt Projectile class
		0,						// Muzzle Flash radius
		{0.0f, 0.0f, 0.0f},		// Muzzle Flash color
		"AncientOne_Tentacle",	// Name
		0,						// Name ID (resource)
		"",						// flash sprite name
		"",						// alt flash Sprite name
		0.25f,					// Flash duration
		0.0f,					// Flash scale
		{0.0f, 0.0f, 0.0f},		// 3rd person gun offset
		{0.0f, -1.4f, 0.3f},	// 1st person gun Offset
		{0.0f, 0.0f, 0.0f},		// Muzzle Offset
		{0.03f, 0.03f, 0.15f},	// Recoil
		{0.0f, 0.0f, 0.1f},		// Flash position
		0.625f,					// Eject interval
		0.0f,					// View kick
		DFALSE,					// View kick is Cumulative
		DFALSE,					// Animation looping
		DFALSE,					// Alt anim looping
		DNULL,					// Icon for status bar
		DNULL,					// Highlighted icon for status bar
	},
	// The Skull
	{
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Skins\\Weapons\\knifec_pv_t.dtx",	// Skin
		DNULL,								// Model
		DNULL,								// Skin
		WEAP_SKULL,				// Type
		TYPE_MELEE,			// Fire type
		AMMO_NONE,				// Ammo type
		1,						// Ammo Use
		1,					// Alt Ammo Use
		20.0f,					// Min Damage
		25.0f,					// Max Damage
		10.0f,					// Min Alt damage
		15.0f,					// Max Alt damage
		1.0f,					// Reload time
		5.0f,					// Alt reload time
		{0.0f, 0.0f},			// Spread
		{0.0f, 0.0f},			// Alt Spread
		500.0f,					// Projectile Velocity
		500.0f,					// Alt Projectile Velocity
		3500.0f,				// Range
		3500.0f,				// Alt Range
		1,						// Shots per fire
		1,						// Alt Shots per fire
		1,						// Strength or Magic required
		0,						// Two-handed Strength or Magic required
		0,						// Damage Radius (projectiles)
		0,						// Alt Damage Radius (projectiles)
		DFALSE,					// Alt fire zoom
		DFALSE,					// Semi-auto
		"",						// Fire sound
		"",						// Alt Fire sound
//		"Sounds\\Weapons\\orb\\fire.wav",	// Fire sound
//		"Sounds\\Weapons\\orb\\alt.wav",	// Alt Fire sound
		"",	// Empty weapon sound
		"",	// Alt Empty weapon sound
		"CSkullProjectile",		// Projectile class
		"CSkullProjectile",	// Alt Projectile class
		0,						// Muzzle Flash radius
		{0.0f, 0.0f, 0.0f},		// Muzzle Flash color
		"The Skull",				// Name
		0,						// Name ID (resource)
		"",						// flash sprite name
		"",						// alt flash Sprite name
		0.0f,					// Flash duration
		0.0f,					// Flash scale
		{0.0f, 0.0f, 0.0f},		// 3rd person gun offset
		{0.0f, -0.5f, 2.4f},	// Gun Offset
		{0.0f, 0.0f, 0.0f},		// Muzzle Offset
		{0.03f, 0.03f, 0.15f},	// Recoil
		{0.0f, -5.0f, 20.0f},	// Flash position
		0.625f,					// Eject interval
		0.0f,					// View kick
		DFALSE,					// View kick is Cumulative
		DFALSE,					// Animation looping
		DFALSE,					// Alt anim looping
		DNULL,					// Icon for status bar
		DNULL,					// Highlighted icon for status bar
	},
#ifdef _ADD_ON
	// The Gremlin's Rocks
	{
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Skins\\Weapons\\knifec_pv_t.dtx",	// Skin
		"Models_ao\\Powerups_ao\\rock1_pu.abc",	// Model
		"Skins_ao\\Powerups_ao\\rock1_pu.dtx",	// Skin
		WEAP_GREMLIN_ROCK,		// Type
		TYPE_MELEE,				// Fire type
		AMMO_NONE,				// Ammo type
		1,						// Ammo Use
		1,						// Alt Ammo Use
		10.0f,					// Min Damage
		20.0f,					// Max Damage
		10.0f,					// Min Alt damage
		20.0f,					// Max Alt damage
		1.0f,					// Reload time
		1.0f,					// Alt reload time
		{50.0f, 50.0f},			// Spread
		{50.0f, 50.0f},			// Alt Spread
		1000.0f,				// Projectile Velocity
		1000.0f,				// Alt Projectile Velocity
		3500.0f,				// Range
		3500.0f,				// Alt Range
		1,						// Shots per fire
		1,						// Alt Shots per fire
		1,						// Strength or Magic required
		0,						// Two-handed Strength or Magic required
		0,						// Damage Radius (projectiles)
		0,						// Alt Damage Radius (projectiles)
		DFALSE,					// Alt fire zoom
		DFALSE,					// Semi-auto
		"",						// Fire sound
		"",						// Alt Fire sound
//		"Sounds\\Weapons\\orb\\fire.wav",	// Fire sound
//		"Sounds\\Weapons\\orb\\alt.wav",	// Alt Fire sound
		"",	// Empty weapon sound
		"",	// Alt Empty weapon sound
		"CRockProjectile",		// Projectile class
		"CRockProjectile",		// Alt Projectile class
		0,						// Muzzle Flash radius
		{0.0f, 0.0f, 0.0f},		// Muzzle Flash color
		"Gremlin Rocks",		// Name
		0,						// Name ID (resource)
		"",						// flash sprite name
		"",						// alt flash Sprite name
		0.0f,					// Flash duration
		0.0f,					// Flash scale
		{0.0f, 0.0f, 0.0f},		// 3rd person gun offset
		{0.0f, -0.5f, 2.4f},	// Gun Offset
		{0.0f, 0.0f, 0.0f},		// Muzzle Offset
		{0.03f, 0.03f, 0.15f},	// Recoil
		{0.0f, -5.0f, 20.0f},	// Flash position
		0.625f,					// Eject interval
		0.0f,					// View kick
		DFALSE,					// View kick is Cumulative
		DFALSE,					// Animation looping
		DFALSE,					// Alt anim looping
		DNULL,					// Icon for status bar
		DNULL,					// Highlighted icon for status bar
	},
	// The Nightmare's Fireballs
	{
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Models\\Weapons\\knifec_pv.abc",	// Model
		"Skins\\Weapons\\knifec_pv_t.dtx",	// Skin
		DNULL,					// Model
		DNULL,					// Skin
		WEAP_NIGHTMARE_FIREBALLS,// Type
		TYPE_PISTOL,			// Fire type
		AMMO_NONE,				// Ammo type
		1,						// Ammo Use
		1,						// Alt Ammo Use
		20.0f,					// Min Damage
		30.0f,					// Max Damage
		20.0f,					// Min Alt damage
		30.0f,					// Max Alt damage
		1.0f,					// Reload time
		1.0f,					// Alt reload time
		{0.0f, 0.0f},			// Spread
		{0.0f, 0.0f},			// Alt Spread
		1000.0f,				// Projectile Velocity
		1000.0f,				// Alt Projectile Velocity
		3500.0f,				// Range
		3500.0f,				// Alt Range
		1,						// Shots per fire
		1,						// Alt Shots per fire
		1,						// Strength or Magic required
		0,						// Two-handed Strength or Magic required
		100,					// Damage Radius (projectiles)
		100,					// Alt Damage Radius (projectiles)
		DFALSE,					// Alt fire zoom
		DFALSE,					// Semi-auto
		"",						// Fire sound
		"",						// Alt Fire sound
//		"Sounds\\Weapons\\orb\\fire.wav",	// Fire sound
//		"Sounds\\Weapons\\orb\\alt.wav",	// Alt Fire sound
		"",	// Empty weapon sound
		"",	// Alt Empty weapon sound
		"CNightmareFireball",	// Projectile class
		"CNightmareFireball",	// Alt Projectile class
		0,						// Muzzle Flash radius
		{0.0f, 0.0f, 0.0f},		// Muzzle Flash color
		"Nightmare Fireballs",	// Name
		0,						// Name ID (resource)
		"",						// flash sprite name
		"",						// alt flash Sprite name
		0.0f,					// Flash duration
		0.0f,					// Flash scale
		{0.0f, 0.0f, 0.0f},		// 3rd person gun offset
		{0.0f, -0.5f, 2.4f},	// Gun Offset
		{0.0f, 0.0f, 0.0f},		// Muzzle Offset
		{0.03f, 0.03f, 0.15f},	// Recoil
		{0.0f, -5.0f, 20.0f},	// Flash position
		0.625f,					// Eject interval
		0.0f,					// View kick
		DFALSE,					// View kick is Cumulative
		DFALSE,					// Animation looping
		DFALSE,					// Alt anim looping
		DNULL,					// Icon for status bar
		DNULL,					// Highlighted icon for status bar
	},
#endif
	// ProximityBomb
	{
		"Models\\Weapons\\C4proximity_pv.abc",	// Model
		"Models\\Weapons\\C4proximity_pv.abc",	// Model
		"Skins\\Weapons\\C_C4prox_pv_t.dtx",	// Skin
		"Models\\Powerups\\proximities_pu.abc",	// 3rd person model
		"Skins\\Powerups\\proximities_pu.dtx",	// 3rd person skin
		WEAP_PROXIMITYBOMB,		// Type
		TYPE_MELEE,				// Fire type
		AMMO_PROXIMITYBOMB,		// Ammo type
		1,						// Ammo Use
		1,						// Alt Ammo Use
		80.0f,					// Min Damage
		120.0f,					// Max Damage
		80.0f,					// Min Alt damage
		120.0f,					// Max Alt damage
		1.0f,					// Reload time
		1.0f,					// Alt reload time
		{0.0f, 0.0f},			// Spread
		{0.0f, 0.0f},			// Alt Spread
		1000.0f,				// Projectile Velocity
		0.0f,					// Alt Projectile Velocity
		2000.0f,				// Range
		0.0f,					// Alt Range
		1,						// Shots per fire
		1,						// Alt Shots per fire
		1,						// Strength or Magic required
		0,						// Two-handed Strength or Magic required
		250,					// Damage Radius (projectiles)
		250,					// Alt Damage Radius (projectiles)
		DFALSE,					// Alt fire zoom
		DFALSE,					// Semi-auto
		"Sounds\\Weapons\\proximities\\fire.wav",	// Fire sound
		"Sounds\\Weapons\\proximities\\alt.wav",	// Alt Fire sound
		"",	// Empty weapon sound
		"",	// Alt Empty weapon sound
		"CProximityBomb",		// Projectile class
		"CProximityBomb",		// Alt Projectile class
		IDS_WEAPON_PROX,		// Muzzle Flash radius
		{0.0f, 0.0f, 0.0f},		// Muzzle Flash color
		"Proximity Bomb",		// Name
		0,						// Name ID (resource)
		"",						// flash sprite name
		"",						// alt flash Sprite name
		0.25f,					// Flash duration
		0.0f,					// Flash scale
		{0.0f, 0.0f, 0.0f},		// 3rd person gun offset
		{-0.7f, 0.0f, 2.4f},	// Gun Offset
		{0.0f, 0.375f, 12.0f},	// Muzzle Offset
		{0.03f, 0.03f, 0.15f},	// Recoil
		{3.0f, -7.5f, 5.0f},	// Flash position
		0.0f,					// Eject interval
		0.0f,					// View kick
		DFALSE,					// View kick is Cumulative
		DTRUE,					// Animation looping
		DFALSE,					// Alt anim looping
		DNULL,					// Icon for status bar
		DNULL,					// Highlighted icon for status bar
	},
	// RemoteBomb
	{
		"Models\\Weapons\\C4remote_pv.abc",	// Model
		"Models\\Weapons\\C4remote_pv.abc",	// Model
		"Skins\\Weapons\\C_C4remote_pv_t.dtx",	// Skin
		"Models\\Powerups\\remotes_pu.abc",	// 3rd person model
		"Skins\\Powerups\\remotes_pu.dtx",	// 3rd person skin
		WEAP_REMOTEBOMB,		// Type
		TYPE_MELEE,				// Fire type
		AMMO_REMOTEBOMB,		// Ammo type
		1,						// Ammo Use
		1,						// Alt Ammo Use
		80.0f,					// Min Damage
		120.0f,					// Max Damage
		80.0f,					// Min Alt damage
		120.0f,					// Max Alt damage
		1.0f,					// Reload time
		1.0f,					// Alt reload time
		{0.0f, 0.0f},			// Spread
		{0.0f, 0.0f},			// Alt Spread
		1000.0f,				// Projectile Velocity
		0.0f,					// Alt Projectile Velocity
		2000.0f,				// Range
		0.0f,					// Alt Range
		1,						// Shots per fire
		1,						// Alt Shots per fire
		1,						// Strength or Magic required
		0,						// Two-handed Strength or Magic required
		250,					// Damage Radius (projectiles)
		250,					// Alt Damage Radius (projectiles)
		DFALSE,					// Alt fire zoom
		DFALSE,					// Semi-auto
		"Sounds\\Weapons\\remotes\\fire.wav",	// Fire sound
		"Sounds\\Weapons\\remotes\\alt.wav",	// Alt Fire sound
		"",	// Empty weapon sound
		"",	// Alt Empty weapon sound
		"CRemoteBomb",			// Projectile class
		"CRemoteBomb",			// Alt Projectile class
		0,						// Muzzle Flash radius
		{0.0f, 0.0f, 0.0f},		// Muzzle Flash color
		"Remote Bomb",			// Name
		IDS_WEAPON_REMOTE,		// Muzzle Flash radius
		"",						// flash sprite name
		"",						// alt flash Sprite name
		0.25f,					// Flash duration
		0.0f,					// Flash scale
		{0.0f, 0.0f, 0.0f},		// 3rd person gun offset
		{-0.7f, 0.0f, 2.4f},	// Gun Offset
		{0.0f, 0.375f, 12.0f},	// Muzzle Offset
		{0.03f, 0.03f, 0.15f},	// Recoil
		{3.0f, -7.5f, 5.0f},	// Flash position
		0.0f,					// Eject interval
		0.0f,					// View kick
		DFALSE,					// View kick is Cumulative
		DTRUE,					// Animation looping
		DFALSE,					// Alt anim looping
		DNULL,					// Icon for status bar
		DNULL,					// Highlighted icon for status bar
	},
	// TimeBomb
	{
		"Models\\Weapons\\C4time_pv.abc",	// Model
		"Models\\Weapons\\C4time_pv.abc",	// Model
		"Skins\\Weapons\\C_C4explode_pv_t.dtx",// Skin
		"Models\\Powerups\\timebomb_pu.abc",	// 3rd person model
		"Skins\\Powerups\\timebomb_pu.dtx",	// 3rd person skin
		WEAP_TIMEBOMB,			// Type
		TYPE_MELEE,				// Fire type
		AMMO_TIMEBOMB,		// Ammo type
		1,						// Ammo Use
		1,						// Alt Ammo Use
		80.0f,					// Min Damage
		120.0f,					// Max Damage
		80.0f,					// Min Alt damage
		120.0f,					// Max Alt damage
		1.0f,					// Reload time
		1.0f,					// Alt reload time
		{0.0f, 0.0f},			// Spread
		{0.0f, 0.0f},			// Alt Spread
		1000.0f,					// Projectile Velocity
		0.0f,					// Alt Projectile Velocity
		2000.0f,				// Range
		0.0f,					// Alt Range
		1,						// Shots per fire
		1,						// Alt Shots per fire
		1,						// Strength or Magic required
		0,						// Two-handed Strength or Magic required
		250,					// Damage Radius (projectiles)
		250,					// Alt Damage Radius (projectiles)
		DFALSE,					// Alt fire zoom
		DFALSE,					// Semi-auto
		"Sounds\\Weapons\\time\\fire.wav",	// Fire sound
		"Sounds\\Weapons\\time\\alt.wav",	// Alt Fire sound
		"",	// Empty weapon sound
		"",	// Alt Empty weapon sound
		"CTimeBomb",			// Projectile class
		"CTimeBomb",			// Alt Projectile class
		0,						// Muzzle Flash radius
		{0.0f, 0.0f, 0.0f},		// Muzzle Flash color
		"Time Bomb",			// Name
		IDS_WEAPON_TIME,		// Muzzle Flash radius
		"",						// flash sprite name
		"",						// alt flash Sprite name
		0.25f,					// Flash duration
		0.0f,					// Flash scale
		{0.0f, 0.0f, 0.0f},		// 3rd person gun offset
		{-0.7f, 0.0f, 2.4f},	// Gun Offset
		{0.0f, 0.375f, 12.0f},	// Muzzle Offset
		{0.03f, 0.03f, 0.15f},	// Recoil
		{3.0f, -7.5f, 5.0f},	// Flash position
		0.0f,					// Eject interval
		0.0f,					// View kick
		DFALSE,					// View kick is Cumulative
		DTRUE,					// Animation looping
		DFALSE,					// Alt anim looping
		DNULL,					// Icon for status bar
		DNULL,					// Highlighted icon for status bar
	}
};

