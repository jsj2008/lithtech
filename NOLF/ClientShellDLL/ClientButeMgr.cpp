// ----------------------------------------------------------------------- //
//
// MODULE  : ClientButeMgr.cpp
//
// PURPOSE : ClientButeMgr implementation - Client-side attributes
//
// CREATED : 2/02/99
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ClientButeMgr.h"

#define CBMGR_CHEATS_TAG				"Cheats"
#define CBMGR_CHEATS_ATTRIBUTE_NAME		"Cheat"

#define CBMGR_GAME_TAG					"Game"
#define CBMGR_CAMERA_TAG				"Camera"
#define CBMGR_REVERB_TAG				"Reverb"
#define CBMGR_WEATHER_TAG				"Weather"
#define CBMGR_SPECIALFX_TAG				"SpecialFX"
#define CBMGR_BREATHFX_TAG				"BreathFX"
#define CBMGR_INTERFACE_TAG				"Interface"

#define CBMGR_WORLD_TAG					"World"
#define CBMGR_WORLD_SINGLE_PATH			"SinglePath"
#define CBMGR_WORLD_MULTI_PATH			"MultiPath"


static char s_aTagName[30];
static char s_aAttName[100];

CClientButeMgr* g_pClientButeMgr = LTNULL;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::CClientButeMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CClientButeMgr::CClientButeMgr()
{
	m_nNumCheatAttributes	= 0;
	m_nNumSingleWorldPaths	= 0;
	m_nNumMultiWorldPaths	= 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::~CClientButeMgr()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CClientButeMgr::~CClientButeMgr()
{
	Term();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CClientButeMgr::Init(ILTCSBase *pInterface, const char* szAttributeFile)
{
    if (g_pClientButeMgr || !szAttributeFile) return LTFALSE;
    if (!Parse(pInterface, szAttributeFile)) return LTFALSE;


	// Set up global pointer...

	g_pClientButeMgr = this;


	// Determine how many cheat attributes there are...

	m_nNumCheatAttributes = 0;
	sprintf(s_aAttName, "%s%d", CBMGR_CHEATS_ATTRIBUTE_NAME, m_nNumCheatAttributes);

	while (m_buteMgr.Exist(CBMGR_CHEATS_TAG, s_aAttName))
	{
		m_nNumCheatAttributes++;
		sprintf(s_aAttName, "%s%d", CBMGR_CHEATS_ATTRIBUTE_NAME, m_nNumCheatAttributes);
	}

	m_nNumSingleWorldPaths = 0;
	sprintf(s_aAttName,"%s%d",CBMGR_WORLD_SINGLE_PATH, m_nNumSingleWorldPaths);
	while (m_buteMgr.Exist(CBMGR_WORLD_TAG, s_aAttName))
	{
		m_nNumSingleWorldPaths++;
		sprintf(s_aAttName,"%s%d",CBMGR_WORLD_SINGLE_PATH, m_nNumSingleWorldPaths);
	}

	m_nNumMultiWorldPaths = 0;
	sprintf(s_aAttName,"%s%d",CBMGR_WORLD_MULTI_PATH, m_nNumMultiWorldPaths);
	while (m_buteMgr.Exist(CBMGR_WORLD_TAG, s_aAttName))
	{
		m_nNumMultiWorldPaths++;
		sprintf(s_aAttName,"%s%d",CBMGR_WORLD_MULTI_PATH, m_nNumMultiWorldPaths);
	}


    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::Term()
//
//	PURPOSE:	Clean up.
//
// ----------------------------------------------------------------------- //

void CClientButeMgr::Term()
{
    g_pClientButeMgr = LTNULL;
}


/////////////////////////////////////////////////////////////////////////////
//
//	C H E A T  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::GetCheat()
//
//	PURPOSE:	Get the cheat specified by the number
//
// ----------------------------------------------------------------------- //

CString CClientButeMgr::GetCheat(uint8 nCheatNum)
{
	CString str;
	if (nCheatNum < m_nNumCheatAttributes)
	{
		sprintf(s_aAttName, "%s%d", CBMGR_CHEATS_ATTRIBUTE_NAME, nCheatNum);
		str = m_buteMgr.GetString(CBMGR_CHEATS_TAG, s_aAttName);
	}

	return str;
}


/////////////////////////////////////////////////////////////////////////////
//
//	G A M E  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::GetGameAttributeInt()
//
//	PURPOSE:	Get a game attribute as an int
//
// ----------------------------------------------------------------------- //

int CClientButeMgr::GetGameAttributeInt(char* pAttribute)
{
	if (!pAttribute) return 0;

	return m_buteMgr.GetInt(CBMGR_GAME_TAG, pAttribute);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::GetGameAttributeFloat()
//
//	PURPOSE:	Get a game attribute as a float
//
// ----------------------------------------------------------------------- //

float CClientButeMgr::GetGameAttributeFloat(char* pAttribute)
{
	if (!pAttribute) return 0.0f;

	return (float) m_buteMgr.GetDouble(CBMGR_GAME_TAG, pAttribute);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::GetGameAttributeString()
//
//	PURPOSE:	Get a game attribute as a string
//
// ----------------------------------------------------------------------- //

CString CClientButeMgr::GetGameAttributeString(char* pAttribute)
{
	CString str;
	if (!pAttribute) return str;

	return m_buteMgr.GetString(CBMGR_GAME_TAG, pAttribute);
}


/////////////////////////////////////////////////////////////////////////////
//
//	C A M E R A  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::GetCameraAttributeInt()
//
//	PURPOSE:	Get a camera attribute as an int
//
// ----------------------------------------------------------------------- //

int CClientButeMgr::GetCameraAttributeInt(char* pAttribute)
{
	if (!pAttribute) return 0;

	return m_buteMgr.GetInt(CBMGR_CAMERA_TAG, pAttribute);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::GetCameraAttributeFloat()
//
//	PURPOSE:	Get a camera attribute as a float
//
// ----------------------------------------------------------------------- //

float CClientButeMgr::GetCameraAttributeFloat(char* pAttribute)
{
	if (!pAttribute) return 0.0f;

	return (float) m_buteMgr.GetDouble(CBMGR_CAMERA_TAG, pAttribute);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::GetCameraAttributeString()
//
//	PURPOSE:	Get a camera attribute as a string
//
// ----------------------------------------------------------------------- //

CString CClientButeMgr::GetCameraAttributeString(char* pAttribute)
{
	CString str;
	if (!pAttribute) return str;

	return m_buteMgr.GetString(CBMGR_CAMERA_TAG, pAttribute);
}




/////////////////////////////////////////////////////////////////////////////
//
//	R E V E R B  Related functions...
//
/////////////////////////////////////////////////////////////////////////////


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::GetReverbAttributeFloat()
//
//	PURPOSE:	Get a reverb attribute as a float
//
// ----------------------------------------------------------------------- //

float CClientButeMgr::GetReverbAttributeFloat(char* pAttribute)
{
	if (!pAttribute) return 0.0f;

	return (float) m_buteMgr.GetDouble(CBMGR_REVERB_TAG, pAttribute);
}




/////////////////////////////////////////////////////////////////////////////
//
//	W E A T H E R  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::GetWeatherAttributeFloat()
//
//	PURPOSE:	Get a weather attribute as a float
//
// ----------------------------------------------------------------------- //

float CClientButeMgr::GetWeatherAttributeFloat(char* pAttribute)
{
	if (!pAttribute) return 0.0f;

	return (float) m_buteMgr.GetDouble(CBMGR_WEATHER_TAG, pAttribute);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::GetWeatherAttributeString()
//
//	PURPOSE:	Get a weather attribute as a string
//
// ----------------------------------------------------------------------- //

CString CClientButeMgr::GetWeatherAttributeString(char* pAttribute)
{
	CString str;
	if (!pAttribute) return str;

	return m_buteMgr.GetString(CBMGR_WEATHER_TAG, pAttribute);
}


/////////////////////////////////////////////////////////////////////////////
//
//	S P E C I A L  F X  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::GetSpecialFXAttributeFloat()
//
//	PURPOSE:	Get a special fx attribute as a float
//
// ----------------------------------------------------------------------- //

float CClientButeMgr::GetSpecialFXAttributeFloat(char* pAttribute)
{
	if (!pAttribute) return 0.0f;

	return (float) m_buteMgr.GetDouble(CBMGR_SPECIALFX_TAG, pAttribute);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::GetSpecialFXAttributeString()
//
//	PURPOSE:	Get a special fx attribute as a string
//
// ----------------------------------------------------------------------- //

CString CClientButeMgr::GetSpecialFXAttributeString(char* pAttribute)
{
	CString str;
	if (!pAttribute) return str;

	return m_buteMgr.GetString(CBMGR_SPECIALFX_TAG, pAttribute);
}



/////////////////////////////////////////////////////////////////////////////
//
//	B R E A T H  F X  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::GetBreathFXAttributeFloat()
//
//	PURPOSE:	Get a breath fx attribute as a float
//
// ----------------------------------------------------------------------- //

float CClientButeMgr::GetBreathFXAttributeFloat(char* pAttribute)
{
	if (!pAttribute) return 0.0f;

	return (float) m_buteMgr.GetDouble(CBMGR_BREATHFX_TAG, pAttribute);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::GetBreathFXAttributeInt()
//
//	PURPOSE:	Get a breath fx attribute as a integer
//
// ----------------------------------------------------------------------- //

int CClientButeMgr::GetBreathFXAttributeInt(char* pAttribute)
{
	if (!pAttribute) return 0;

	return m_buteMgr.GetInt(CBMGR_BREATHFX_TAG, pAttribute);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::GetBreathFXAttributeString()
//
//	PURPOSE:	Get a breath fx attribute as a string
//
// ----------------------------------------------------------------------- //

CString CClientButeMgr::GetBreathFXAttributeString(char* pAttribute)
{
	CString str;
	if (!pAttribute) return str;

	return m_buteMgr.GetString(CBMGR_BREATHFX_TAG, pAttribute);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::GetBreathFXAttributeVector()
//
//	PURPOSE:	Get a breath fx attribute as a vector
//
// ----------------------------------------------------------------------- //

LTVector CClientButeMgr::GetBreathFXAttributeVector(char* pAttribute)
{
	return m_buteMgr.GetVector(CBMGR_BREATHFX_TAG, pAttribute);
}



/////////////////////////////////////////////////////////////////////////////
//
//	I N T E R F A C E  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::GetInterfaceAttributeString()
//
//	PURPOSE:	Get a interface attribute as a string
//
// ----------------------------------------------------------------------- //

CString CClientButeMgr::GetInterfaceAttributeString(char* pAttribute)
{
	CString str;
	if (!pAttribute) return str;

	return m_buteMgr.GetString(CBMGR_INTERFACE_TAG, pAttribute);
}


/////////////////////////////////////////////////////////////////////////////
//
//	W O R L D  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

void CClientButeMgr::GetWorldPath(uint8 nPath, char* pBuf, int nBufLen, LTBOOL bSingle)
{
	if (nPath < 0) return;

	if (bSingle)
	{
		if (nPath > m_nNumSingleWorldPaths) return;
		sprintf(s_aAttName,"%s%d",CBMGR_WORLD_SINGLE_PATH, nPath);
	}
	else
	{
		if (nPath > m_nNumMultiWorldPaths) return;
		sprintf(s_aAttName,"%s%d",CBMGR_WORLD_MULTI_PATH, nPath);
	}


	CString str = m_buteMgr.GetString(CBMGR_WORLD_TAG, s_aAttName);

	if (!str.IsEmpty())
	{
		strncpy(pBuf, (char*)(LPCSTR)str, nBufLen);
	}
}


