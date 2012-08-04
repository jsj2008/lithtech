#ifndef __RIOTSETTINGS_H
#define __RIOTSETTINGS_H

//***********************************************
//
//		Structure and Class Definitions
//
//***********************************************

class CRiotClientShell;
class CRiotMenu;

// Settings struct...

struct Setting
{
	Setting()		{ bStringVar = LTFALSE; memset (strVarName, 0, 32); hVar = LTNULL; nValue = 0.0f; memset (strValue, 0, 32); }

	LTBOOL			bStringVar;

	char			strVarName[32];
	HCONSOLEVAR		hVar;
	
	LTFLOAT			nValue;
	char			strValue[32];
};

// Values for 3 position options..

#define RS_LOW						0
#define	RS_MED						1
#define RS_HIGH						2

// MISC SETTINGS...

#define RS_MISC_FIRST				0
#define RS_MISC_VEHICLEMODE			0
#define RS_MISC_SCREENFLASH			1
#define RS_MISC_LAST				1

// CONTROL SETTINGS...

#define RS_CTRL_FIRST				0
#define RS_CTRL_MOUSELOOK			0
#define RS_CTRL_MOUSEINVERTY		1
#define RS_CTRL_MOUSESENSITIVITY	2
#define RS_CTRL_MOUSEINPUTRATE		3
#define RS_CTRL_JOYLOOK				4
#define RS_CTRL_JOYINVERTY			5
#define RS_CTRL_LOOKSPRING			6
#define RS_CTRL_RUNLOCK				7
#define RS_CTRL_LAST				7

// SOUND SETTINGS...

#define RS_SND_FIRST				0
#define RS_SND_MUSICENABLED			0
#define RS_SND_MUSICVOL				1
#define RS_SND_FX					2
#define RS_SND_SOUNDVOL				3
#define RS_SND_CHANNELS				4
#define RS_SND_16BIT				5
#define RS_SND_LAST					5

// TOP-LEVEL DETAIL SETTINGS...

#define RS_DET_FIRST				0
#define RS_DET_OVERALL				0
#define RS_DET_GORE					1
#define RS_DET_LAST					1

// LOW-LEVEL DETAIL SETTINGS...

#define RS_SUBDET_FIRST				0
#define RS_SUBDET_MODELLOD			0
#define RS_SUBDET_SHADOWS			1
#define RS_SUBDET_BULLETHOLES		2
#define RS_SUBDET_TEXTUREDETAIL		3
#define RS_SUBDET_DYNAMICLIGHTING	4
#define RS_SUBDET_LIGHTMAPPING		5
#define RS_SUBDET_SPECIALFX			6
#define RS_SUBDET_ENVMAPPING		7
#define RS_SUBDET_MODELFB			8
#define RS_SUBDET_CLOUDMAPLIGHT		9
#define RS_SUBDET_PVWEAPONS			10
#define RS_SUBDET_POLYGRIDS			11
#define RS_SUBDET_LAST				11


//***********************************************
//
//		Class Definition
//
//***********************************************

class CRiotSettings
{

public:

	CRiotSettings();
	~CRiotSettings()	{}

	LTBOOL		Init (ILTClient* pClientDE, CRiotClientShell* pClientShell);
	
	// misc access functions

	LTBOOL		VehicleMode()						{ return (LTBOOL)Misc[RS_MISC_VEHICLEMODE].nValue; }
	LTBOOL		ScreenFlash()						{ return (LTBOOL)Misc[RS_MISC_SCREENFLASH].nValue; }
	
	// control access functions

	LTBOOL		MouseLook()							{ return (LTBOOL)Control[RS_CTRL_MOUSELOOK].nValue; }
	LTBOOL		MouseInvertY()						{ return (LTBOOL)Control[RS_CTRL_MOUSEINVERTY].nValue; }
	void		SetMouseLook(LTBOOL bVal)			{ Control[RS_CTRL_MOUSELOOK].nValue = bVal; }
	void		SetMouseInvertY(LTBOOL bVal)			{ Control[RS_CTRL_MOUSEINVERTY].nValue = bVal; }

	LTFLOAT&		ControlSetting(uint32 val)			{ return Control[val].nValue; }
	LTFLOAT&		DetailSetting(uint32 val)			{ return Detail[val].nValue; }
	LTFLOAT&		SubDetailSetting(uint32 val)		{ return SubDetail[val].nValue; }

	LTFLOAT		MouseSensitivity()					{ return		Control[RS_CTRL_MOUSESENSITIVITY].nValue; }
	LTBOOL		JoyLook()							{ return (LTBOOL)Control[RS_CTRL_JOYLOOK].nValue; }
	LTBOOL		JoyInvertY()						{ return (LTBOOL)Control[RS_CTRL_JOYINVERTY].nValue; }
	LTBOOL		Lookspring()						{ return (LTBOOL)Control[RS_CTRL_LOOKSPRING].nValue; }
	LTBOOL		RunLock()							{ return (LTBOOL)Control[RS_CTRL_RUNLOCK].nValue; }
	
	void		SetRunLock (LTBOOL bRunLock)			{ Control[RS_CTRL_RUNLOCK].nValue = bRunLock ? 1.0f : 0.0f; }

	// sound access functions

	LTBOOL		MusicEnabled()						{ return (LTBOOL)Sound[RS_SND_MUSICENABLED].nValue; }
	LTFLOAT		MusicVolume()						{ return		Sound[RS_SND_MUSICVOL].nValue; }
	LTBOOL		SoundEnabled()						{ return (LTBOOL)Sound[RS_SND_FX].nValue; }
	LTFLOAT		SoundVolume()						{ return		Sound[RS_SND_SOUNDVOL].nValue; }
	LTFLOAT		SoundChannels()						{ return		Sound[RS_SND_CHANNELS].nValue; }
	LTBOOL		Sound16Bit()						{ return (LTBOOL)Sound[RS_SND_16BIT].nValue; }

	// top-level detail access functions

	LTFLOAT		GlobalDetail()						{ return		Detail[RS_DET_OVERALL].nValue; }
	LTBOOL		Gore()								
	{ 
		if (m_bAllowGore)
		{
			return (LTBOOL)Detail[RS_DET_GORE].nValue;
		}
		else
		{
			return LTFALSE;  // Never allow gore
		}
	}

	// low-level detail access functions

	LTFLOAT		ModelLOD()							{ return		SubDetail[RS_SUBDET_MODELLOD].nValue; }
	LTBOOL		Shadows()							{ return (LTBOOL)SubDetail[RS_SUBDET_SHADOWS].nValue; }
	LTFLOAT		NumBulletHoles()					{ return		SubDetail[RS_SUBDET_BULLETHOLES].nValue; }
	LTFLOAT		TextureDetailSetting()				{ return		SubDetail[RS_SUBDET_TEXTUREDETAIL].nValue; }
	LTFLOAT		DynamicLightSetting()				{ return		SubDetail[RS_SUBDET_DYNAMICLIGHTING].nValue; }
	uint8		SpecialFXSetting()					{ return (uint8)SubDetail[RS_SUBDET_SPECIALFX].nValue; }
	LTBOOL		EnvironmentMapping()				{ return (LTBOOL)SubDetail[RS_SUBDET_ENVMAPPING].nValue; }
	LTBOOL		ModelFullBrights()					{ return (LTBOOL)SubDetail[RS_SUBDET_MODELFB].nValue; }
	LTBOOL		CloudMapLight()						{ return (LTBOOL)SubDetail[RS_SUBDET_CLOUDMAPLIGHT].nValue; }
	uint8		PlayerViewWeaponSetting()			{ return (uint8)SubDetail[RS_SUBDET_PVWEAPONS].nValue; }
	LTBOOL		PolyGrids()							{ return (LTBOOL)SubDetail[RS_SUBDET_POLYGRIDS].nValue; }

	// display mode access functions

	RMode*		GetRenderMode()						{ return &CurrentRenderer; }
	LTBOOL		Textures8Bit()						{ return (MasterPaletteMode.nValue == 1.0f) ? LTTRUE : LTFALSE; }

	// read/write functions
	
	LTBOOL		ReadSettings();
	void		WriteSettings();

	void		ReadMiscSettings();
	void		WriteMiscSettings();

	void		ReadControlSettings();
	void		WriteControlSettings();

	void		ReadSoundSettings();
	void		WriteSoundSettings();

	void		ReadDetailSettings();
	void		WriteDetailSettings();

	void		ReadSubDetailSettings();
	void		WriteSubDetailSettings();

	void		ReadDisplayModeSettings();
	void		WriteDisplayModeSettings();
	
	// utility functions

	void		SetLowDetail();
	void		SetMedDetail();
	void		SetHiDetail();

	// settings implementation functions

	LTBOOL		ImplementRendererSetting();
	void		ImplementMusicSource();
	void		ImplementMusicVolume();
	void		ImplementSoundEnabled();
	void		ImplementSoundVolume();
	void		ImplementSoundQuality();
	void		ImplementMouseSensitivity();
	void		ImplementInputRate();
	void		ImplementDetailSetting (int nSetting);
	void		ImplementBitDepth();

public:

	ILTClient*			m_pClientDE;
	CRiotClientShell*	m_pClientShell;

	// Default Low, Medium, and High Settings...

	Setting		DefLow[RS_SUBDET_LAST + 1];
	Setting		DefMed[RS_SUBDET_LAST + 1];
	Setting		DefHi[RS_SUBDET_LAST + 1];

	// Misc Settings...

	Setting		Misc[RS_MISC_LAST + 1];
	
	// Control Settings...

	Setting		Control[RS_CTRL_LAST + 1];

	// Sound Settings...

	Setting		Sound[RS_SND_LAST + 1];

	// Renderer Settings...

	RMode		CurrentRenderer;
	Setting		MasterPaletteMode;

	// Top-Level Detail Settings...

	Setting		Detail[RS_DET_LAST + 1];

	// Low-Level Detail Settings...

	Setting		SubDetail[RS_SUBDET_LAST + 1];

	// Allow any gore (even display of the option in game)
	LTBOOL		m_bAllowGore;

};

#endif