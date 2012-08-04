// ----------------------------------------------------------------------- //
//
// MODULE  : CachedFiles.cpp
//
// PURPOSE : Pre load the following files for every level
//
// CREATED : 9/8/98
//
// ----------------------------------------------------------------------- //

#include "CachedFiles.h"

char* g_pCachedModelsOnFoot[] = 
{
	// Weapons...

	"Models\\PV_Weapons\\Assaultrifle_PV.abc",
	"Models\\PV_Weapons\\Colt45_PV.abc",
	"Models\\PV_Weapons\\EnergyGrenade_PV.abc",
	"Models\\PV_Weapons\\KatoGrenade_PV.abc",
	"Models\\PV_Weapons\\machinegun_PV.abc",
	"Models\\PV_Weapons\\shotgun_pv.abc",
	"Models\\PV_Weapons\\Squeakytoy_PV.abc",
	"Models\\PV_Weapons\\Tanto_PV.abc",
	"Models\\PV_Weapons\\TOW_PV.abc",
	"Models\\PV_Weapons\\EnergyGrenadeExplosion.abc",
	"Models\\PV_Weapons\\EnergyGrenadeExplosionCore.abc",
	"Models\\PV_Weapons\\KatoGrenadeExplosion.abc",
	
	"Models\\PV_Weapons\\Shotgunshell.abc",
	"Models\\PV_Weapons\\colt_shell.abc",
	"Models\\PV_Weapons\\assault_shell.abc",
	"Models\\PV_Weapons\\machinegun_shell.abc",
	"Models\\PV_Weapons\\bullgut_projectile.abc",

	"models\\props\\debris\\humanpart1.abc",
	"models\\props\\debris\\humanpart2.abc",
	"models\\props\\debris\\humanpart3.abc",


	// Powerups...

	"Models\\Powerups\\assaultrifle.abc",
	"Models\\Powerups\\BodyArmor_100.abc",
	"Models\\Powerups\\BodyArmor_200.abc",
	"Models\\Powerups\\BodyArmor_50.abc",
	"Models\\Powerups\\colt45.abc",
	"Models\\Powerups\\EnergyGrenade.abc",
	"Models\\Powerups\\FirstAid_10.abc",
	"Models\\Powerups\\FirstAid_15.abc",
	"Models\\Powerups\\FirstAid_25.abc",
	"Models\\Powerups\\FirstAid_50.abc",
	"Models\\Powerups\\KatoGrenade.abc",
	"Models\\Powerups\\Machinegun.abc",
	"Models\\Powerups\\shotgun.abc",
	"Models\\Powerups\\squeakytoy.abc",
	"Models\\Powerups\\tanto.abc",
	"Models\\Powerups\\tow.abc"

};
	
const int NUM_CACHED_MODELSONFOOT = (sizeof(g_pCachedModelsOnFoot) / sizeof(g_pCachedModelsOnFoot[0]));


char* g_pCachedModelsMech[] = 
{
	// Weapons...

	"Models\\PV_Weapons\\Bullgut_Projectile.abc",
	"Models\\PV_Weapons\\bullgut_pv.abc",
	"Models\\PV_Weapons\\Energy_baton_pv.abc",
	"Models\\PV_Weapons\\Energy_blade_pv.abc",
	"Models\\PV_Weapons\\juggernaut_pv.abc",
	"Models\\PV_Weapons\\Katana_pv.abc",
	"Models\\PV_Weapons\\Lasercannon_PV.abc",
	"Models\\PV_Weapons\\MonoKnife_pv.abc",
	"Models\\PV_Weapons\\pulserifle_pv.abc",
	"Models\\PV_Weapons\\redriot_pv.abc",
	"Models\\PV_Weapons\\shredder_pv.abc",
	"Models\\PV_Weapons\\Sniperrifle_pv.abc",
	"Models\\PV_Weapons\\Spider_PV.abc",
	"Models\\PV_Weapons\\SpiderProjectile.abc",
	"Models\\PV_Weapons\\JuggernautExplosion.abc",

	"Models\\PV_Weapons\\sniper_shell.abc",
	"Models\\PV_Weapons\\shredder_shell.abc",
	"Models\\PV_Weapons\\juggernaut_shell.abc",

	// Powerups...

	"Models\\Powerups\\ArmorEnhancement.abc",
	"Models\\Powerups\\ArmorRepair_100.abc",
	"Models\\Powerups\\ArmorRepair_250.abc",
	"Models\\Powerups\\ArmorRepair_500.abc",
	"Models\\Powerups\\ArmorUpgrade.abc",
	"Models\\Powerups\\bullgut.abc",
	"Models\\Powerups\\DamageEnhancement.abc",
	"Models\\Powerups\\DamageUpgrade.abc",
	"Models\\Powerups\\EnergyBaton.abc",
	"Models\\Powerups\\EnergyBlade.abc",
	"Models\\Powerups\\EnergyProtectionEnhancement.abc",
	"Models\\Powerups\\Enhancement_Plug.abc",
	"Models\\Powerups\\ExplosiveProtectionEnhancement.abc",
	"Models\\Powerups\\HealthEnhancement.abc",
	"Models\\Powerups\\HealthUpgrade.abc",
	"Models\\Powerups\\juggernaut.abc",
	"Models\\Powerups\\Katana.abc",
	"Models\\Powerups\\LaserCannon.abc",
	"Models\\Powerups\\MeleeDamageEnhancement.abc",
	"Models\\Powerups\\PowerSurge_100.abc",
	"Models\\Powerups\\PowerSurge_150.abc",
	"Models\\Powerups\\PowerSurge_250.abc",
	"Models\\Powerups\\PowerSurge_50.abc",
	"Models\\Powerups\\ProjectileProtectionEnhancement.abc",
	"Models\\Powerups\\ProtectionEnhancement.abc",
	"Models\\Powerups\\ProtectionUpgrade.abc",
	"Models\\Powerups\\pulserifle.abc",
	"Models\\Powerups\\redriot.abc",
	"Models\\Powerups\\RegenEnhancement.abc",
	"Models\\Powerups\\RegenUpgrade.abc",
	"Models\\Powerups\\shredder.abc",
	"Models\\Powerups\\sniperrifle.abc",
	"Models\\Powerups\\Spider.abc",
	"Models\\Powerups\\TargetingUpgrade.abc",
	"Models\\Powerups\\UltraDamage.abc",
	"Models\\Powerups\\UltraHealth.abc",
	"Models\\Powerups\\UltraInfrared.abc",
	"Models\\Powerups\\UltraNightVision.abc",
	"Models\\Powerups\\UltraPowerSurge.abc",
	"Models\\Powerups\\UltraReflect.abc",
	"Models\\Powerups\\UltraRestore.abc",
	"Models\\Powerups\\UltraShield.abc",
	"Models\\Powerups\\UltraSilencer.abc",
	"Models\\Powerups\\UltraStealth.abc",
	"Models\\Powerups\\Upgrade_Chip.abc",

	// Client-side weapon fx...

	"Models\\PV_Weapons\\SpiderExplosionCore.abc",
	"Models\\PV_Weapons\\JuggernautExplosion.abc"

};
	
const int NUM_CACHED_MODELSMECH  = (sizeof(g_pCachedModelsMech) / sizeof(g_pCachedModelsMech[0]));


char* g_pCachedModels[] = 
{
	// Weapons...

	"Models\\PV_Weapons\\bshell.abc",
	"Models\\PV_Weapons\\explosion.abc",
	"Models\\PV_Weapons\\RedRiotExplosion.abc",
	"Models\\PV_Weapons\\shell.abc",

	// Powerups...

	"Models\\Powerups\\beam.abc",

	// Client-side weapon fx...

	"Models\\PV_Weapons\\RedRiotExplosion.abc"
};
	
const int NUM_CACHED_MODELS  = (sizeof(g_pCachedModels) / sizeof(g_pCachedModels[0]));


	 
char* g_pCachedTexturesOnFoot[] =
{
	// Weapons...

	"Skins\\weapons\\assaultrifle_pv_a.dtx",
	"Skins\\weapons\\colt45_pv_a.dtx",
	"Skins\\weapons\\EnergyGrenade_pv_a.dtx",
	"Skins\\weapons\\KatoGrenade_pv_a.dtx",
	"Skins\\weapons\\Machinegun_pv_a.dtx",
	"Skins\\weapons\\shotgun_pv_a.dtx",
	"Skins\\weapons\\shotgunshell.dtx",
	"Skins\\weapons\\squeakytoy_pv_a.dtx",
	"Skins\\weapons\\tanto_pv_a.dtx",
	"Skins\\weapons\\tow_pv_a.dtx",

	"Skins\\Weapons\\colt_shell.dtx",
	"Skins\\Weapons\\assault_shell.dtx",
	"Skins\\Weapons\\machinegun_shell.dtx",
	"Skins\\Weapons\\shotgunshell.dtx",
	"skins\\weapons\\bullgut_projectile_a.dtx",
	"skins\\props\\debris\\human.dtx",


	// Powerups...

	"Skins\\Powerups\\assaultrifle_a.dtx",
	"Skins\\Powerups\\BodyArmor_100_a.dtx",
	"Skins\\Powerups\\BodyArmor_200_a.dtx",
	"Skins\\Powerups\\BodyArmor_50_a.dtx",
	"Skins\\Powerups\\Colt45_a.dtx",
	"Skins\\Powerups\\EnergyGrenade_a.dtx",
	"Skins\\Powerups\\FirstAid_10_a.dtx",
	"Skins\\Powerups\\FirstAid_15_a.dtx",
	"Skins\\Powerups\\FirstAid_25_a.dtx",
	"Skins\\Powerups\\FirstAid_50_a.dtx",
	"Skins\\Powerups\\KatoGrenade_a.dtx",
	"Skins\\Powerups\\machinegun_a.dtx",
	"Skins\\Powerups\\shotgun_a.dtx",
	"Skins\\Powerups\\squeakytoy_a.dtx",
	"Skins\\Powerups\\tanto_a.dtx",
	"Skins\\Powerups\\tow_a.dtx",

	// Client-side weapon fx...

	"SpecialFX\\Explosions\\EnergyGrenade.dtx",
	"SpecialFX\\Explosions\\EnergyGrenadeCore.dtx",
	"SpecialFX\\Explosions\\KatoGrenade.dtx"

};

const int NUM_CACHED_TEXTURESONFOOT  = (sizeof(g_pCachedTexturesOnFoot) / sizeof(g_pCachedTexturesOnFoot[0]));



char* g_pCachedTexturesMech[] =
{
	// Weapons...

	"Skins\\weapons\\Bullgut_projectile_a.dtx",
	"Skins\\weapons\\Bullgut_pv_a.dtx",
	"Skins\\weapons\\Energy_Baton_pv_a.dtx",
	"Skins\\weapons\\energy_blade_pv_a.dtx",
	"Skins\\weapons\\juggernaut_pv_a.dtx",
	"Skins\\weapons\\katana_pv_a.dtx",
	"Skins\\weapons\\lasercannon_pv_a.dtx",
	"Skins\\weapons\\monoknife_PV_a.dtx",
	"Skins\\weapons\\Pulserifle_pv_a.dtx",
	"Skins\\weapons\\redriot_pv_a.dtx",
	"Skins\\weapons\\Shredder_pv_a.dtx",
	"Skins\\weapons\\sniperrifle_pv_a.dtx",
	"Skins\\weapons\\Spider_pv_a.dtx",
	"Skins\\weapons\\SpiderProjectile_a.dtx",

	"Skins\\Weapons\\sniper_shell.dtx",
	"Skins\\Weapons\\shredder_shell.dtx",
	"Skins\\Weapons\\juggernaut_shell.dtx",

	// Powerups...

	"Skins\\Powerups\\ArmorEnhancement_a.dtx",
	"Skins\\Powerups\\ArmorRepair_100_a.dtx",
	"Skins\\Powerups\\ArmorRepair_250_a.dtx",
	"Skins\\Powerups\\ArmorRepair_500_a.dtx",
	"Skins\\Powerups\\ArmorUpgrade_a.dtx",
	"Skins\\Powerups\\bullgut_a.dtx",
	"Skins\\Powerups\\DamageEnhancement_a.dtx",
	"Skins\\Powerups\\DamageUpgrade_a.dtx",
	"Skins\\Powerups\\EnergyBaton.dtx",
	"Skins\\Powerups\\EnergyBlade.dtx",
	"Skins\\Powerups\\EnergyProtectionEnhancement_a.dtx",
	"Skins\\Powerups\\ExplosiveProtectionEnhancement_a.dtx",
	"Skins\\Powerups\\HealthEnhancement_a.dtx",
	"Skins\\Powerups\\HealthUpgrade_a.dtx",
	"Skins\\Powerups\\juggernaut_a.dtx",
	"Skins\\Powerups\\Katana.dtx",
	"Skins\\Powerups\\lasercannon_a.dtx",
	"Skins\\Powerups\\MeleeDamageEnhancement_a.dtx",
	"Skins\\Powerups\\MonoKnife.dtx",
	"Skins\\Powerups\\PowerSurge_100_a.dtx",
	"Skins\\Powerups\\PowerSurge_150_a.dtx",
	"Skins\\Powerups\\PowerSurge_250_a.dtx",
	"Skins\\Powerups\\PowerSurge_50_a.dtx",
	"Skins\\Powerups\\ProjectileProtectionEnhancement_a.dtx",
	"Skins\\Powerups\\ProtectionEnhancement_a.dtx",
	"Skins\\Powerups\\ProtectionUpgrade_a.dtx",
	"Skins\\Powerups\\pulserifle_a.dtx",
	"Skins\\Powerups\\redriot_a.dtx",
	"Skins\\Powerups\\RegenEnhancement_a.dtx",
	"Skins\\Powerups\\RegenUpgrade_a.dtx",
	"Skins\\Powerups\\shredder_a.dtx",
	"Skins\\Powerups\\sniperrifle_a.dtx",
	"Skins\\Powerups\\Spider_a.dtx",
	"Skins\\Powerups\\TargetingUpgrade.dtx",
	"Skins\\Powerups\\UltraDamage_a.dtx",
	"Skins\\Powerups\\UltraHealth_a.dtx",
	"Skins\\Powerups\\UltraInfrared_a.dtx",
	"Skins\\Powerups\\UltraNightVision_a.dtx",
	"Skins\\Powerups\\UltraPowerSurge_a.dtx",
	"Skins\\Powerups\\UltraReflect_a.dtx",
	"Skins\\Powerups\\UltraRestore_a.dtx",
	"Skins\\Powerups\\UltraShield_a.dtx",
	"Skins\\Powerups\\UltraSilencer_a.dtx",
	"Skins\\Powerups\\UltraStealth_a.dtx",
	"Skins\\Powerups\\Upgrade_Chip.dtx",

	// Client-side weapon fx...

	"SpecialFX\\Explosions\\RedRiot.dtx",
	"SpecialFX\\Explosions\\RedRiotBeam.dtx",
	"SpecialFX\\Explosions\\juggernaut.dtx",
	"SpecialFX\\Explosions\\JuggernautBeam.dtx",
	"SpecialFX\\Explosions\\Beam.dtx",
	"SpecialFX\\Explosions\\SpiderCore.dtx",


};

const int NUM_CACHED_TEXTURESMECH  = (sizeof(g_pCachedTexturesMech) / sizeof(g_pCachedTexturesMech[0]));


char* g_pCachedTextures[] =
{
	// Weapons...

	"Skins\\weapons\\bshell.dtx",
	"Skins\\weapons\\Shell.dtx",
	"skins\\powerups\\beam_a.dtx",

	// Client-side weapon fx...

	"SpriteTextures\\sprite(2)\\ex51127b.dtx",
	"SpecialFX\\ParticleTextures\\Particle.dtx",
	"SpecialFX\\ParticleTextures\\Spark_yellow_1.dtx",
	"SpecialFX\\ParticleTextures\\GreySphere_1.dtx",
	"SpecialFX\\ParticleTextures\\Blood_1.dtx",
	"SpecialFX\\ParticleTextures\\Blood_2.dtx",
	"SpecialFX\\ParticleTextures\\Smoke.dtx",
	"SpecialFX\\ParticleTextures\\Smoke_large.dtx",
	"SpriteTextures\\weapons\\explosions\\BllgCore.dtx"
};

const int NUM_CACHED_TEXTURES  = (sizeof(g_pCachedTextures) / sizeof(g_pCachedTextures[0]));



char* g_pCachedSpriteOnFoot[] =
{
	// Client-side weapon fx...

	"Sprites\\grenade1.spr",
	"Sprites\\colt45muzzleflash.spr",
	"Sprites\\weapons\\mac10flash.spr",
	"Sprites\\ExclHuman.spr",
	"Sprites\\shotgunmuzzleflash.spr"
};

const int NUM_CACHED_SPRITESONFOOT = (sizeof(g_pCachedSpriteOnFoot) / sizeof(g_pCachedSpriteOnFoot[0]));



char* g_pCachedSpriteMech[] =
{
	// Client-side weapon fx...

	"Sprites\\weapons\\ShrddrExp.spr",
	"Sprites\\PulseImpact.spr",
	"Sprites\\weapons\\shrddrexp.spr",
	"Sprites\\Bulletholes\\Laser1.spr",
	"Sprites\\PulseRifle.spr",
	"Sprites\\ExclMech.spr"
};

const int NUM_CACHED_SPRITESMECH = (sizeof(g_pCachedSpriteMech) / sizeof(g_pCachedSpriteMech[0]));



char* g_pCachedSprite[] =
{
	// Client-side weapon fx...

	"Sprites\\BloodSplat1.spr",
	"Sprites\\BloodSplat2.spr",
	"Sprites\\BloodSplat3.spr",
	"Sprites\\BloodSplat4.spr",
	"Sprites\\BullgutFlare.spr",
	"Sprites\\Crosshair.spr",
	"Sprites\\Fire.spr",
	"Sprites\\weapons\\BllgtExp.spr",
	"Sprites\\SmokeTest.spr",
	"Sprites\\Bulletholes\\ImpactFlame1.spr",
	"Sprites\\Bulletholes\\ImpactFlame2.spr",
	"Sprites\\Bulletholes\\ImpactFlame3.spr",
	"Sprites\\Bulletholes\\ImpactFlame4.spr",
	"Sprites\\Bulletholes\\ImpactFlame5.spr",
	"Sprites\\Bulletholes\\ImpactFlame6.spr",
	"Sprites\\glow.spr",
	"Sprites\\weapons\\redriot.spr",
};

const int NUM_CACHED_SPRITES = (sizeof(g_pCachedSprite) / sizeof(g_pCachedSprite[0]));



char* g_pCachedSoundLocal[] =
{
	"bogus"
};

const int NUM_CACHED_SOUNDS_LOCAL = (sizeof(g_pCachedSoundLocal) / sizeof(g_pCachedSoundLocal[0]));



char* g_pCachedSoundAmbient[] =
{
	"bogus"
};

const int NUM_CACHED_SOUNDS_AMBIENT = (sizeof(g_pCachedSoundAmbient) / sizeof(g_pCachedSoundAmbient[0]));



char* g_pCachedSound3DOnFoot[] =
{
	// Weapons...

	"Sounds\\Weapons\\AssaultRifle\\empty.wav",
	"Sounds\\Weapons\\AssaultRifle\\fire.wav",
	"Sounds\\Weapons\\AssaultRifle\\reload.wav",
	"Sounds\\Weapons\\AssaultRifle\\select.wav",
	"sounds\\weapons\\assaultrifle\\zoomin.wav",
	"sounds\\weapons\\assaultrifle\\zoomout.wav",

	"Sounds\\Weapons\\Colt45\\empty.wav",
	"Sounds\\Weapons\\Colt45\\fire.wav",
	"Sounds\\Weapons\\Colt45\\fire2.wav",
	"Sounds\\Weapons\\Colt45\\reload.wav",
	"Sounds\\Weapons\\Colt45\\select1.wav",
	"Sounds\\Weapons\\Colt45\\select2.wav",

	"Sounds\\Weapons\\EnergyGrenade\\empty.wav",
	"Sounds\\Weapons\\EnergyGrenade\\fire.wav",
	"Sounds\\Weapons\\EnergyGrenade\\impact.wav",
	"Sounds\\Weapons\\EnergyGrenade\\projectile.wav",
	"Sounds\\Weapons\\EnergyGrenade\\select.wav",

	"Sounds\\Weapons\\KatoGrenade\\bounce1.wav",
	"Sounds\\Weapons\\KatoGrenade\\bounce2.wav",
	"Sounds\\Weapons\\KatoGrenade\\empty.wav",
	"Sounds\\Weapons\\KatoGrenade\\fire.wav",
	"Sounds\\Weapons\\KatoGrenade\\impact.wav",
	"Sounds\\Weapons\\KatoGrenade\\projectile.wav",
	"Sounds\\Weapons\\KatoGrenade\\select.wav",

	"Sounds\\Weapons\\Machinegun\\empty.wav",
	"Sounds\\Weapons\\Machinegun\\fire.wav",
	"Sounds\\Weapons\\Machinegun\\select.wav",
	"Sounds\\Weapons\\Machinegun\\zoomin.wav",
	"Sounds\\Weapons\\Machinegun\\zoomout.wav",

	"Sounds\\Weapons\\Shotgun\\fire.wav",
	"Sounds\\Weapons\\Shotgun\\select.wav",
	"sounds\\specialfx\\gibs\\human\\explode_1.wav",

	"Sounds\\Weapons\\Squeakytoy\\fire.wav",
	"Sounds\\Weapons\\Squeakytoy\\idle.wav",
	"Sounds\\Weapons\\Squeakytoy\\idle1.wav",
	"Sounds\\Weapons\\Squeakytoy\\idle2.wav",
	"Sounds\\Weapons\\Squeakytoy\\idle3.wav",
	"Sounds\\Weapons\\Squeakytoy\\idle4.wav",
	"Sounds\\Weapons\\Squeakytoy\\select.wav",

	"Sounds\\Weapons\\Tanto\\fire.wav",
	"Sounds\\Weapons\\Tanto\\impact.wav",
	"Sounds\\Weapons\\Tanto\\select.wav",

	"Sounds\\Weapons\\TOW\\empty.wav",
	"Sounds\\Weapons\\TOW\\fire.wav",
	"Sounds\\Weapons\\TOW\\impact.wav",
	"Sounds\\Weapons\\TOW\\projectile.wav",
	"Sounds\\Weapons\\TOW\\select.wav",
	"Sounds\\Weapons\\TOW\\select2.wav",

	"Sounds\\Powerups\\BodyArmor.wav",
	"Sounds\\Powerups\\FirstAid.wav",
	"Sounds\\Powerups\\Weapon_onfoot.wav",

	"Sounds\\Player\\OnFoot\\Choke.wav",
	"Sounds\\Player\\OnFoot\\Choke2.wav",
	"Sounds\\Player\\OnFoot\\Death1.wav",
	"Sounds\\Player\\OnFoot\\Death2.wav",
	"Sounds\\Player\\OnFoot\\Death3.wav",
	"Sounds\\Player\\OnFoot\\Death4.wav",
	"Sounds\\Player\\OnFoot\\Death5.wav",
	"Sounds\\Player\\OnFoot\\Jump1.wav",
	"Sounds\\Player\\OnFoot\\Jump2.wav",
	"Sounds\\Player\\OnFoot\\Pain1.wav",
	"Sounds\\Player\\OnFoot\\Pain2.wav",
	"Sounds\\Player\\OnFoot\\Pain3.wav",
	"Sounds\\Player\\OnFoot\\Pain4.wav",
	"Sounds\\Player\\OnFoot\\Pain5.wav",

};

const int NUM_CACHED_SOUNDS_3DONFOOT = (sizeof(g_pCachedSound3DOnFoot) / sizeof(g_pCachedSound3DOnFoot[0]));


char* g_pCachedSound3DMech[] =
{
	// Weapons...

	"Sounds\\Weapons\\Bullgut\\empty.wav",
	"Sounds\\Weapons\\Bullgut\\fire.wav",
	"Sounds\\Weapons\\Bullgut\\impact.wav",
	"Sounds\\Weapons\\Bullgut\\projectile.wav",
	"Sounds\\Weapons\\Bullgut\\select.wav",

	"Sounds\\Weapons\\EnergyBaton\\fire.wav",
	"Sounds\\Weapons\\EnergyBaton\\impact.wav",
	"Sounds\\Weapons\\EnergyBaton\\select.wav",

	"Sounds\\Weapons\\EnergyBlade\\fire.wav",
	"Sounds\\Weapons\\EnergyBlade\\impact.wav",
	"Sounds\\Weapons\\EnergyBlade\\select.wav",

	"Sounds\\Weapons\\Juggernaut\\empty.wav",
	"Sounds\\Weapons\\Juggernaut\\fire.wav",
	"Sounds\\Weapons\\Juggernaut\\impact.wav",
	"Sounds\\Weapons\\Juggernaut\\reload1.wav",
	"Sounds\\Weapons\\Juggernaut\\reload2.wav",
	"Sounds\\Weapons\\Juggernaut\\select.wav",

	"Sounds\\Weapons\\Katana\\fire.wav",
	"Sounds\\Weapons\\Katana\\impact.wav",
	"Sounds\\Weapons\\Katana\\select.wav",

	"Sounds\\Weapons\\LaserCannon\\empty.wav",
	"Sounds\\Weapons\\LaserCannon\\fire.wav",
	"Sounds\\Weapons\\LaserCannon\\impact.wav",
	"Sounds\\Weapons\\LaserCannon\\projectile.wav",
	"Sounds\\Weapons\\LaserCannon\\select.wav",

	"Sounds\\Weapons\\MonoKnife\\fire.wav",
	"Sounds\\Weapons\\MonoKnife\\impact.wav",
	"Sounds\\Weapons\\MonoKnife\\select.wav",

	"Sounds\\Weapons\\PulseRifle\\empty.wav",
	"Sounds\\Weapons\\PulseRifle\\fire.wav",
	"Sounds\\Weapons\\PulseRifle\\impact.wav",
	"Sounds\\Weapons\\PulseRifle\\projectile.wav",
	"Sounds\\Weapons\\PulseRifle\\reload.wav",
	"Sounds\\Weapons\\PulseRifle\\reload2.wav",
	"Sounds\\Weapons\\PulseRifle\\select.wav",

	"Sounds\\Weapons\\RedRiot\\charge.wav",
	"Sounds\\Weapons\\RedRiot\\deselect.wav",
	"Sounds\\Weapons\\RedRiot\\empty.wav",
	"Sounds\\Weapons\\RedRiot\\fire.wav",
	"Sounds\\Weapons\\RedRiot\\idle.wav",
	"Sounds\\Weapons\\RedRiot\\impact.wav",
	"Sounds\\Weapons\\RedRiot\\projectile.wav",
	"Sounds\\Weapons\\RedRiot\\select.wav",
	"Sounds\\Weapons\\RedRiot\\spin.wav",

	"Sounds\\Weapons\\Shredder\\empty.wav",
	"Sounds\\Weapons\\Shredder\\fire.wav",
	"Sounds\\Weapons\\Shredder\\impact.wav",
	"Sounds\\Weapons\\Shredder\\loop.wav",
	"Sounds\\Weapons\\Shredder\\reload.wav",
	"Sounds\\Weapons\\Shredder\\select.wav",

	"Sounds\\Weapons\\SniperRifle\\empty.wav",
	"Sounds\\Weapons\\SniperRifle\\fire.wav",
	"Sounds\\Weapons\\SniperRifle\\select.wav",
	"Sounds\\Weapons\\SniperRifle\\zoomin.wav",
	"Sounds\\Weapons\\SniperRifle\\zoomout.wav",

	"Sounds\\Weapons\\Spider\\empty.wav",
	"Sounds\\Weapons\\Spider\\fire.wav",
	"Sounds\\Weapons\\Spider\\impact.wav",
	"Sounds\\Weapons\\Spider\\select.wav",
	"Sounds\\Weapons\\Spider\\thud.wav",
	"Sounds\\Weapons\\Spider\\timer.wav",

	"Sounds\\Powerups\\ArmorRepair.wav",
	"Sounds\\Powerups\\PowerSurge.wav",
	"Sounds\\Powerups\\Weapon_mca.wav",

	"SOUNDS\\PLAYER\\VEHICLE_IDLE.WAV",
	"Sounds\\Player\\Mech\\Death1.wav",
	"Sounds\\Player\\Mech\\Death2.wav",
	"Sounds\\Player\\Mech\\Pain1.wav",
	"Sounds\\Player\\Mech\\Pain2.wav",
	"Sounds\\Player\\Mech\\Transform.wav",


};

const int NUM_CACHED_SOUNDS_3DMECH = (sizeof(g_pCachedSound3DMech) / sizeof(g_pCachedSound3DMech[0]));


char* g_pCachedSound3D[] =
{
	// Weapons...

	"Sounds\\Weapons\\Bounce.wav",
	"Sounds\\Weapons\\Bounce2.wav",
	"Sounds\\Weapons\\CriticalHit.wav",
	"Sounds\\Weapons\\shell1.wav",
	"Sounds\\Weapons\\shell2.wav",
	"Sounds\\Weapons\\shell3.wav",
	"Sounds\\Weapons\\shell4.wav",
	"Sounds\\Weapons\\shell5.wav",
	"Sounds\\Weapons\\shell6.wav",

};

const int NUM_CACHED_SOUNDS_3D = (sizeof(g_pCachedSound3D) / sizeof(g_pCachedSound3D[0]));
