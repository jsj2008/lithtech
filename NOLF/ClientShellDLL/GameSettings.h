#ifndef __GameSettings_H
#define __GameSettings_H

#include "clientutilities.h"

// Values for 3 position options..

#define RS_LOW						0
#define	RS_MED						1
#define RS_HIGH						2


class CGameClientShell;

// ***********************************************
//
//		Class Definition
//
// ***********************************************

class CGameSettings
{

public:

	CGameSettings();
	~CGameSettings()	{}

    LTBOOL       Init (ILTClient* pClientDE, CGameClientShell* pClientShell);

    LTBOOL       GetBoolVar(char *pVar);
    void        SetBoolVar(char *pVar, LTBOOL bVal);
    uint8       GetByteVar(char *pVar);
    void        SetByteVar(char *pVar, uint8 nVal);
	float		GetFloatVar(char *pVar);
	void		SetFloatVar(char *pVar, float fVal);

	// misc access functions

    LTBOOL       ScreenFlash() {return GetBoolVar("ScreenFlash");}

	// control access functions

    LTBOOL       MouseLook()                 { return GetBoolVar("MouseLook"); }
    LTBOOL       MouseInvertY()              { return GetBoolVar("MouseInvertY");}
    void        SetMouseLook(LTBOOL bVal)    { SetBoolVar("MouseLook",bVal); }
    void        SetMouseInvertY(LTBOOL bVal) { SetBoolVar("MouseInvertY",bVal); }

	float		MouseSensitivity()			{ return GetFloatVar("MouseSensitivity");}
    LTBOOL       UseJoystick()               { return GetBoolVar("UseJoystick"); }
    LTBOOL       Lookspring()                { return GetBoolVar("LookSpring"); }
    LTBOOL       RunLock()                   { return GetBoolVar("RunLock"); }

    void        SetRunLock (LTBOOL bRunLock) { SetBoolVar("RunLock",bRunLock); }

	// sound access functions

    LTBOOL       MusicEnabled()              { return GetBoolVar("MusicEnable"); }
	float		MusicVolume()				{ return GetFloatVar("MusicVolume");}
    LTBOOL       SoundEnabled()              { return GetBoolVar("SoundEnable"); }
	float		SoundVolume()				{ return GetFloatVar("SoundVolume");}
	float		SoundChannels()				{ return GetFloatVar("SoundChannels");}
    LTBOOL       Sound16Bit()                { return GetBoolVar("Sound16Bit"); }

	// top-level detail access functions

    LTBOOL      Gore()                      { return m_bAllowGore && GetBoolVar("Gore"); }
	LTBOOL		IsGoreAllowed()				{ return m_bAllowGore; }

	// low-level detail access functions

	float		ModelLOD()					{ return GetFloatVar("ModelLOD");}
    uint8       Shadows()                   { return GetByteVar("MaxModelShadows"); }
	float		NumBulletHoles()			{ return GetFloatVar("BulletHoles");}
	float		TextureDetailSetting()		{ return GetFloatVar("TextureDetail");}
	float		DynamicLightSetting()		{ return GetFloatVar("DynamicLightSetting");}
    LTBOOL       LightMap()                 { return GetBoolVar("LightMap");}
    uint8       SpecialFXSetting()          { return GetConsoleInt("PerformanceLevel",1); }
    LTBOOL       EnvironmentMapping()       { return GetBoolVar("EnvMapEnable"); }
    LTBOOL       ModelFullBrights()         { return GetBoolVar("ModelFullbrite"); }
    LTBOOL       CloudMapLight()            { return GetBoolVar("CloudMapLight"); }
    uint8       PlayerViewWeaponSetting()   { return GetByteVar("PVWeapons"); }
    LTBOOL       PolyGrids()                { return GetBoolVar("PolyGrids"); }
    LTBOOL       DrawSky()                  { return GetBoolVar("DrawSky"); }
    LTBOOL       FogEnable()                { return GetBoolVar("FogEnable"); }

	// read/write functions
    void        LoadDemoSettings(ILTStream *pStream);
    void        SaveDemoSettings(ILTStream *pStream);

	// utility functions
	void		SetLowDetail();
	void		SetMedDetail();
	void		SetHiDetail();

	// settings implementation functions

    LTBOOL       ImplementRendererSetting();
	void		ImplementMusicSource();
	void		ImplementMusicVolume();
	void		ImplementSoundVolume();
	void		ImplementSoundQuality();
	void		ImplementMouseSensitivity();


private:

    ILTClient*          m_pClientDE;
	CGameClientShell*	m_pClientShell;
	RMode				CurrentRenderer;


	// Allow any gore (even display of the option in game)
    LTBOOL       m_bAllowGore;

	HCONSOLEVAR m_hTmpVar;
	char		m_tmpStr[128];


};

inline LTBOOL    CGameSettings::GetBoolVar(char *pVar)
{
	m_hTmpVar = m_pClientDE->GetConsoleVar(pVar);
	if (m_hTmpVar)
        return (LTBOOL)m_pClientDE->GetVarValueFloat(m_hTmpVar);
	else
        return LTFALSE;
};


inline  void    CGameSettings::SetBoolVar(char *pVar, LTBOOL bVal)
{
	sprintf (m_tmpStr, "+%s %d", pVar, bVal ? 1 : 0);
	m_pClientDE->RunConsoleString (m_tmpStr);
};

inline  uint8   CGameSettings::GetByteVar(char *pVar)
{
	m_hTmpVar = m_pClientDE->GetConsoleVar(pVar);
	if (m_hTmpVar)
        return (uint8)m_pClientDE->GetVarValueFloat(m_hTmpVar);
	else
		return 0;
};

inline  void    CGameSettings::SetByteVar(char *pVar, uint8 nVal)
{
	sprintf (m_tmpStr, "+%s %d", pVar, nVal);
	m_pClientDE->RunConsoleString (m_tmpStr);
}


inline	float	CGameSettings::GetFloatVar(char *pVar)
{
	m_hTmpVar = m_pClientDE->GetConsoleVar(pVar);
	if (m_hTmpVar)
		return m_pClientDE->GetVarValueFloat(m_hTmpVar);
	else
		return 0.0f;
};
inline	void	CGameSettings::SetFloatVar(char *pVar, float fVal)
{
	sprintf (m_tmpStr, "+%s %f", pVar, fVal);
	m_pClientDE->RunConsoleString (m_tmpStr);
}


#endif