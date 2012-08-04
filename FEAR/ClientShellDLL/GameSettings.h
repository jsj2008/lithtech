// ----------------------------------------------------------------------- //
//
// MODULE  : GameSettings.h
//
// PURPOSE : Handles implementation of various game settings
//
// (c) 1997-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __GAMESETTINGS_H
#define __GAMESETTINGS_H

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

	bool		Init (ILTClient* pClientDE, CGameClientShell* pClientShell);

	bool		GetBoolVar(const char *pVar, bool bDefault = false);
	void		SetBoolVar(const char *pVar, bool bVal);
	uint8		GetByteVar(const char *pVar, uint8 nDefault = 0);
	void		SetByteVar(const char *pVar, uint8 nVal);
	float		GetFloatVar(const char *pVar, float fDefault = 0.0f);
	void		SetFloatVar(const char *pVar, float fVal);


	// control access functions

	bool		MouseInvertY()				{ return GetBoolVar("MouseInvertY");}
	void		SetMouseInvertY(bool bVal)	{ SetBoolVar("MouseInvertY",bVal); }

	// low-level detail access functions

	float		NumBulletHoles()			{ return GetFloatVar("BulletHoles", 300.0f);}
	uint8		SpecialFXSetting()			{ return (uint8)LTMIN(GetConsoleInt("PerformanceLevel",3),255); }
	bool		PolyGrids()					{ return GetBoolVar("PolyGrids", true); }


private:

	ILTClient			*m_pClientDE;
	CGameClientShell	*m_pClientShell;
	RMode				CurrentRenderer;


	HCONSOLEVAR m_hTmpVar;
	char		m_tmpStr[128];


};

inline bool    CGameSettings::GetBoolVar(const char *pVar, bool bDefault)
{
	m_hTmpVar = m_pClientDE->GetConsoleVariable(pVar);
	if (m_hTmpVar)
		return (m_pClientDE->GetConsoleVariableFloat(m_hTmpVar) != 0.0f);
	else
		return bDefault;
};


inline  void    CGameSettings::SetBoolVar(const char *pVar, bool bVal)
{
	m_pClientDE->SetConsoleVariableFloat(pVar, (bVal) ? 1.0f : 0.0f);
};

inline  uint8   CGameSettings::GetByteVar(const char *pVar, uint8 nDefault)
{
	m_hTmpVar = m_pClientDE->GetConsoleVariable(pVar);
	if (m_hTmpVar)
		return (uint8)LTCLAMP(m_pClientDE->GetConsoleVariableFloat(m_hTmpVar), 0.0f, 255.0f);
	else
		return nDefault;
};

inline  void    CGameSettings::SetByteVar(const char *pVar, uint8 nVal)
{
	m_pClientDE->SetConsoleVariableFloat(m_tmpStr, (float)nVal);
}


inline	float	CGameSettings::GetFloatVar(const char *pVar, float fDefault)
{
	m_hTmpVar = m_pClientDE->GetConsoleVariable(pVar);
	if (m_hTmpVar)
		return m_pClientDE->GetConsoleVariableFloat(m_hTmpVar);
	else
		return fDefault;
}

inline	void	CGameSettings::SetFloatVar(const char *pVar, float fVal)
{
	m_pClientDE->SetConsoleVariableFloat(m_tmpStr, fVal);
}


#endif //__GAMESETTINGS_H