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

#define CBMGR_DEBUGKEYS_TAG					"DebugKey"
#define CBMGR_DEBUGKEYS_ATTRIBUTE_NAME		"Name"
#define CBMGR_DEBUGKEYS_ATTRIBUTE_KEY		"Key"
#define CBMGR_DEBUGKEYS_ATTRIBUTE_MODIFIER	"Modifier"
#define CBMGR_DEBUGKEYS_ATTRIBUTE_STRING	"String"
#define CBMGR_DEBUGKEYS_ATTRIBUTE_TITLE		"Title"

#define CBMGR_GLOW_RENDERSTYLE_MAP			"GlowRSMap"
#define CBMGR_GLOW_DEFAULT_RENDERSTYLE		"GlowDefaultRS"
#define CBMGR_NO_GLOW_RENDERSTYLE			"GlowNoGlowRS"
#define CBMGR_GLOW_MAP_RENDERSTYLE			"GlowMapRS"
#define CBMGR_GLOW_MAP_RENDERSTYLETO		"GlowMapRSTo"


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

	m_nNumDebugKeys = 0;
	m_aNumDebugLevels = LTNULL;
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

	delete[] m_aNumDebugLevels;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CClientButeMgr::Init(const char* szAttributeFile)
{
    if (g_pClientButeMgr || !szAttributeFile) return LTFALSE;
    if (!Parse(szAttributeFile)) return LTFALSE;


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


	// Determine how many debug key attributes there are...

	m_nNumDebugKeys = 0;
	sprintf(s_aTagName, "%s%d", CBMGR_DEBUGKEYS_TAG, m_nNumDebugKeys);

	while (m_buteMgr.Exist(s_aTagName))
	{
		m_nNumDebugKeys++;
		sprintf(s_aTagName, "%s%d", CBMGR_DEBUGKEYS_TAG, m_nNumDebugKeys);
	}

	m_aNumDebugLevels = new int[m_nNumDebugKeys];
	memset(m_aNumDebugLevels, 0, sizeof(int)*m_nNumDebugKeys);
	for( int i = 0; i < m_nNumDebugKeys; ++i )
	{
		sprintf(s_aTagName, "%s%d", CBMGR_DEBUGKEYS_TAG, i );
		sprintf(s_aAttName, "%s%d", CBMGR_DEBUGKEYS_ATTRIBUTE_STRING, m_aNumDebugLevels[i] );

		while (m_buteMgr.Exist(s_aTagName, s_aAttName))
		{
			++m_aNumDebugLevels[i];
			sprintf(s_aAttName, "%s%d", CBMGR_DEBUGKEYS_ATTRIBUTE_STRING, m_aNumDebugLevels[i] );
		}
	}

	//we need to setup the glow mapping table here
	m_nNumGlowMappings = 0;
	while(1)
	{
		sprintf(s_aAttName, "%s%d", CBMGR_GLOW_MAP_RENDERSTYLE, m_nNumGlowMappings);
		if( m_buteMgr.Exist(CBMGR_GLOW_RENDERSTYLE_MAP, s_aAttName))
		{
			sprintf(s_aAttName, "%s%d", CBMGR_GLOW_MAP_RENDERSTYLETO, m_nNumGlowMappings);
			if(m_buteMgr.Exist(CBMGR_GLOW_RENDERSTYLE_MAP, s_aAttName))
			{
				m_nNumGlowMappings++;
			}
			else 
				break;
		}
		else
			break;
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

void CClientButeMgr::GetCheat(uint8 nCheatNum, char *pBuf, uint16 nBufLen)
{
	pBuf[0] = LTNULL;
	if (nCheatNum < m_nNumCheatAttributes)
	{
		sprintf(s_aAttName, "%s%d", CBMGR_CHEATS_ATTRIBUTE_NAME, nCheatNum);
		m_buteMgr.GetString(CBMGR_CHEATS_TAG, s_aAttName, "", pBuf, nBufLen);
	}

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

void CClientButeMgr::GetGameAttributeString(char* pAttribute, char *pBuf, uint16 nBufLen)
{
	if (!pAttribute)
	{
		pBuf[0] = LTNULL;
		return;
	}

	m_buteMgr.GetString(CBMGR_GAME_TAG, pAttribute, "", pBuf, nBufLen);
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

void CClientButeMgr::GetCameraAttributeString(char* pAttribute, char *pBuf, uint16 nBufLen)
{
	if (!pAttribute)
	{
		pBuf[0] = LTNULL;
		return;
	}

	m_buteMgr.GetString(CBMGR_CAMERA_TAG, pAttribute, "", pBuf, nBufLen);
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

void CClientButeMgr::GetWeatherAttributeString(char* pAttribute, char *pBuf, uint16 nBufLen)
{
	pBuf[0] = LTNULL;
	if (!pAttribute) return;

	m_buteMgr.GetString(CBMGR_WEATHER_TAG, pAttribute, "", pBuf, nBufLen);
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

void CClientButeMgr::GetSpecialFXAttributeString(char* pAttribute, char *pBuf, uint16 nBufLen)
{
	pBuf[0] = LTNULL;
	if (!pAttribute) return;

	m_buteMgr.GetString(CBMGR_SPECIALFX_TAG, pAttribute, "", pBuf, nBufLen);
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

void CClientButeMgr::GetBreathFXAttributeString(char* pAttribute, char *pBuf, uint16 nBufLen)
{
	pBuf[0] = LTNULL;
	if (!pAttribute) return;

	m_buteMgr.GetString(CBMGR_BREATHFX_TAG, pAttribute, "", pBuf, nBufLen);
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

void CClientButeMgr::GetInterfaceAttributeString(char* pAttribute, char *pBuf, uint16 nBufLen)
{
	if (!pAttribute) return;

	m_buteMgr.GetString(CBMGR_INTERFACE_TAG, pAttribute, pBuf, nBufLen);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::GetInterfaceAttributeFloat()
//
//	PURPOSE:	Get a interface attribute as a float
//
// ----------------------------------------------------------------------- //

float CClientButeMgr::GetInterfaceAttributeFloat(char* pAttribute, float fDef)
{
	if (!pAttribute) return fDef;

	return m_buteMgr.GetFloat(CBMGR_INTERFACE_TAG, pAttribute, fDef);
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


	m_buteMgr.GetString(CBMGR_WORLD_TAG, s_aAttName, "", pBuf, nBufLen);

}


/////////////////////////////////////////////////////////////////////////////
//
//	D E B U G Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::GetNumDebugLevels()
//
//	PURPOSE:	Get the number of debug levels for this key.
//
// ----------------------------------------------------------------------- //

int CClientButeMgr::GetNumDebugLevels(uint8 nDebugKey) const
{
	if( nDebugKey < m_nNumDebugKeys )
	{
		return m_aNumDebugLevels[nDebugKey];
	}

	return -1;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::GetDebugName()
//
//	PURPOSE:	Get the key associated with this debug level
//
// ----------------------------------------------------------------------- //

void CClientButeMgr::GetDebugName(uint8 nDebugKey, char * pBuf, uint16 nBufLen)
{
	if (nDebugKey < m_nNumDebugKeys)
	{
		sprintf(s_aTagName, "%s%d", CBMGR_DEBUGKEYS_TAG, nDebugKey);
		m_buteMgr.GetString(s_aTagName, CBMGR_DEBUGKEYS_ATTRIBUTE_NAME, "", pBuf, nBufLen);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::GetDebugKeyId()
//
//	PURPOSE:	Get the key id
//
// ----------------------------------------------------------------------- //

int CClientButeMgr::GetDebugKeyId(uint8 nDebugKey)
{
	int nResult = -1;

	if (nDebugKey < m_nNumDebugKeys)
	{
		sprintf(s_aTagName, "%s%d", CBMGR_DEBUGKEYS_TAG, nDebugKey);
		nResult = m_buteMgr.GetInt(s_aTagName, CBMGR_DEBUGKEYS_ATTRIBUTE_KEY, -1 );
	}

	return nResult;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::GetDebugModifierId()
//
//	PURPOSE:	Get the modifier id
//
// ----------------------------------------------------------------------- //

int CClientButeMgr::GetDebugModifierId(uint8 nDebugKey)
{
	int nResult = -1;

	if (nDebugKey < m_nNumDebugKeys)
	{
		sprintf(s_aTagName, "%s%d", CBMGR_DEBUGKEYS_TAG, nDebugKey);
		nResult = m_buteMgr.GetInt(s_aTagName, CBMGR_DEBUGKEYS_ATTRIBUTE_MODIFIER, -1 );
	}

	return nResult;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::GetDebugString()
//
//	PURPOSE:	Get the console string associated with this debug level
//
// ----------------------------------------------------------------------- //

void CClientButeMgr::GetDebugString(uint8 nDebugKey, uint8 nDebugLevel, char * pBuf, uint16 nBufLen)
{
	if (nDebugKey < m_nNumDebugKeys)
	{
		sprintf(s_aTagName, "%s%d", CBMGR_DEBUGKEYS_TAG, nDebugKey);

		if( nDebugLevel < GetNumDebugLevels(nDebugKey) )
		{
			sprintf(s_aAttName, "%s%d", CBMGR_DEBUGKEYS_ATTRIBUTE_STRING, nDebugLevel);
			m_buteMgr.GetString(s_aTagName, s_aAttName, "", pBuf, nBufLen);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::GetDebugTitle()
//
//	PURPOSE:	Get the message string associated with this debug level
//
// ----------------------------------------------------------------------- //

void CClientButeMgr::GetDebugTitle(uint8 nDebugKey, uint8 nDebugLevel, char * pBuf, uint16 nBufLen)
{
	if (nDebugKey < m_nNumDebugKeys)
	{
		sprintf(s_aTagName, "%s%d", CBMGR_DEBUGKEYS_TAG, nDebugKey);

		if( nDebugLevel < GetNumDebugLevels(nDebugKey) )
		{
			sprintf(s_aAttName, "%s%d", CBMGR_DEBUGKEYS_ATTRIBUTE_TITLE, nDebugLevel);
			m_buteMgr.GetString(s_aTagName, s_aAttName, "", pBuf, nBufLen);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
//
//	Screen Glow Related functions...
//
/////////////////////////////////////////////////////////////////////////////

void CClientButeMgr::GetDefaultGlowRS(char* pBuf, uint16 nBufLen)
{
	m_buteMgr.GetString(CBMGR_GLOW_RENDERSTYLE_MAP, CBMGR_GLOW_DEFAULT_RENDERSTYLE, "", pBuf, nBufLen);
}

void CClientButeMgr::GetNoGlowRS(char* pBuf, uint16 nBufLen)
{
	m_buteMgr.GetString(CBMGR_GLOW_RENDERSTYLE_MAP, CBMGR_NO_GLOW_RENDERSTYLE, "", pBuf, nBufLen);
}

void CClientButeMgr::GetGlowMappingRS(uint32 nMapping, char* pMapBuf, uint16 nMapLen, char* pMapToBuf, uint16 nMapToLen)
{
	sprintf(s_aAttName, "%s%d", CBMGR_GLOW_MAP_RENDERSTYLE, nMapping);
	m_buteMgr.GetString(CBMGR_GLOW_RENDERSTYLE_MAP, s_aAttName, "", pMapBuf, nMapLen);

	sprintf(s_aAttName, "%s%d", CBMGR_GLOW_MAP_RENDERSTYLETO, nMapping);
	m_buteMgr.GetString(CBMGR_GLOW_RENDERSTYLE_MAP, s_aAttName, "", pMapToBuf, nMapToLen);
}