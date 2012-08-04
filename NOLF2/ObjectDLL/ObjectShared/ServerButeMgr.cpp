// ----------------------------------------------------------------------- //
//
// MODULE  : ServerButeMgr.cpp
//
// PURPOSE : ServerButeMgr implementation - Server-side attributes
//
// CREATED : 2/02/99
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ServerButeMgr.h"

#define SBMGR_PLAYER_TAG				"Player"
#define SBMGR_SECURITYCAMERA_TAG		"SecurityCamera"
#define	SBMGR_WON_TAG					"WON"
#define SBMGR_WON_ADDR					"ServerAddr"
#define SBMGR_WON_PORT					"ServerPort"
#define	SBMGR_MISC_TAG					"Misc"
#define SBMGR_AMOBOX_TAG				"AmmoBox"
#define SBMGR_DOOMSDAYDEVICE_TAG		"DoomsDayDevice"

static char s_aTagName[30];
static char s_aAttName[100];

CServerButeMgr* g_pServerButeMgr = LTNULL;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerButeMgr::CServerButeMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CServerButeMgr::CServerButeMgr()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerButeMgr::~CServerButeMgr()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CServerButeMgr::~CServerButeMgr()
{
	Term();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerButeMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CServerButeMgr::Init(const char* szAttributeFile)
{
    if (g_pServerButeMgr || !szAttributeFile) return LTFALSE;
    if (!Parse(szAttributeFile)) return LTFALSE;

	// Set up global pointer...

	g_pServerButeMgr = this;

	m_nNumWONServers = 0;
    LTBOOL bDone = LTFALSE;
	while (!bDone)
	{
		sprintf(s_aAttName,"%s%d",SBMGR_WON_ADDR, m_nNumWONServers);
		if (m_buteMgr.Exist(SBMGR_WON_TAG,s_aAttName))
			m_nNumWONServers++;
		else
            bDone = LTTRUE;
	}



    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerButeMgr::Tem()
//
//	PURPOSE:	Clean up.
//
// ----------------------------------------------------------------------- //

void CServerButeMgr::Term()
{
	m_buteMgr.Term();
    g_pServerButeMgr = LTNULL;
}


/////////////////////////////////////////////////////////////////////////////
//
//	P L A Y E R  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerButeMgr::GetPlayerAtrributeInt()
//
//	PURPOSE:	Get a player attribute as an int
//
// ----------------------------------------------------------------------- //

int CServerButeMgr::GetPlayerAttributeInt(char* pAttribute, int nDefVal)
{
	return m_buteMgr.GetInt(SBMGR_PLAYER_TAG, pAttribute, nDefVal);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerButeMgr::GetPlayerAtrributeFloat()
//
//	PURPOSE:	Get a player attribute as a float
//
// ----------------------------------------------------------------------- //

float CServerButeMgr::GetPlayerAttributeFloat(char* pAttribute)
{
	return (float) m_buteMgr.GetDouble(SBMGR_PLAYER_TAG, pAttribute);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerButeMgr::GetPlayerAtrributeString()
//
//	PURPOSE:	Get a player attribute as a string
//
// ----------------------------------------------------------------------- //

void CServerButeMgr::GetPlayerAttributeString(char* pAttribute, char* pBuf, int nBufLen)
{
	m_buteMgr.GetString(SBMGR_PLAYER_TAG, pAttribute, pBuf, nBufLen);
}




/////////////////////////////////////////////////////////////////////////////
//
//	S E C U R I T Y  C A M E R A  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerButeMgr::GetSecurityCameraInt()
//
//	PURPOSE:	Get a secuirty camera attribute as an int
//
// ----------------------------------------------------------------------- //

int CServerButeMgr::GetSecurityCameraInt(char* pAttribute)
{
	return m_buteMgr.GetInt(SBMGR_SECURITYCAMERA_TAG, pAttribute);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerButeMgr::GetSecurityCameraFloat()
//
//	PURPOSE:	Get a security camera attribute as a float
//
// ----------------------------------------------------------------------- //

float CServerButeMgr::GetSecurityCameraFloat(char* pAttribute)
{
	return (float) m_buteMgr.GetDouble(SBMGR_SECURITYCAMERA_TAG, pAttribute);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerButeMgr::GetSecurityCameraString()
//
//	PURPOSE:	Get a security camera attribute as a string
//
// ----------------------------------------------------------------------- //

void CServerButeMgr::GetSecurityCameraString(char* pAttribute, char* pBuf, int nBufLen)
{
	m_buteMgr.GetString(SBMGR_SECURITYCAMERA_TAG, pAttribute, pBuf, nBufLen);
}


void CServerButeMgr::GetWONAddress(int nServer, char* pBuf, int nBufLen)
{
	if (nServer < 0 || nServer > m_nNumWONServers) return;

	sprintf(s_aAttName,"%s%d",SBMGR_WON_ADDR, nServer);

	m_buteMgr.GetString(SBMGR_WON_TAG, s_aAttName, pBuf, nBufLen);

}

uint32 CServerButeMgr::GetWONPort(int nServer)
{
	if (nServer < 0 || nServer > m_nNumWONServers) return 0;

	sprintf(s_aAttName,"%s%d",SBMGR_WON_PORT, nServer);
	return m_buteMgr.GetDword(SBMGR_WON_TAG, s_aAttName, 0);

}

// Body functions

LTFLOAT CServerButeMgr::GetBodyStairsFallSpeed()
{
	return (LTFLOAT)m_buteMgr.GetDouble("Body", "StairsFallSpeed", 300.0f);
}

LTFLOAT CServerButeMgr::GetBodyStairsFallStopSpeed()
{
	return (LTFLOAT)m_buteMgr.GetDouble("Body", "StairsFallStopSpeed", 150.0f);
}

/////////////////////////////////////////////////////////////////////////////
//
//	M I S C  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

LTFLOAT CServerButeMgr::GetSummaryDelay()
{
	return (LTFLOAT)m_buteMgr.GetDouble(SBMGR_MISC_TAG, "SummaryDelay", 15.0f);
}


bool CServerButeMgr::GetDoomsDayDeviceFxName( char* pszFxName, int nLen )
{
	// Check inputs.
	if( !pszFxName )
	{
		ASSERT( !"CServerButeMgr::GetDoomsDayDeviceFxName: Invalid inputs." );
		return false;
	}

	strncpy( pszFxName, m_buteMgr.GetString( SBMGR_DOOMSDAYDEVICE_TAG, "FxName" ), nLen );
	pszFxName[nLen-1] = 0;

	return true;
}

bool CServerButeMgr::GetDoomsDayDeviceDamageType( char* pszDamageType, int nLen )
{
	// Check inputs.
	if( !pszDamageType )
	{
		ASSERT( !"CServerButeMgr::GetDoomsDayDeviceFxName: Invalid inputs." );
		return false;
	}

	strncpy( pszDamageType, m_buteMgr.GetString( SBMGR_DOOMSDAYDEVICE_TAG, "DamageType" ), nLen );
	pszDamageType[nLen-1] = 0;

	return true;
}

float CServerButeMgr::GetDoomsDayDeviceDamageDelay( )
{
	return ( float )m_buteMgr.GetDouble( SBMGR_DOOMSDAYDEVICE_TAG, "DamageDelay", 1.5f );
}

float CServerButeMgr::GetDoomsDayDeviceDamageAmount( )
{
	return ( float )m_buteMgr.GetDouble( SBMGR_DOOMSDAYDEVICE_TAG, "DamageAmount", 5.0f );
}

float CServerButeMgr::GetDoomsDayDeviceDamageInterval( )
{
	return ( float )m_buteMgr.GetDouble( SBMGR_DOOMSDAYDEVICE_TAG, "DamageInterval", 3.0f );
}

float CServerButeMgr::GetDoomsDayDeviceEndTime( )
{
	return ( float )m_buteMgr.GetDouble( SBMGR_DOOMSDAYDEVICE_TAG, "EndTime", 5.0f );
}

float CServerButeMgr::GetDoomsDayDeviceRespawnTime( )
{
	return (float)m_buteMgr.GetDouble( SBMGR_DOOMSDAYDEVICE_TAG, "RespawnTime", 30.0f );
}


/////////////////////////////////////////////////////////////////////////////
//
//	A M M O  B O X  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerButeMgr::GetAmmoBoxAtrributeInt()
//
//	PURPOSE:	Get an AmmoBox attribute as an int
//
// ----------------------------------------------------------------------- //

int CServerButeMgr::GetAmmoBoxAttributeInt( const char* pAttribute, int nDefVal )
{
	return m_buteMgr.GetInt( SBMGR_AMOBOX_TAG, pAttribute, nDefVal );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerButeMgr::GetAmmoBoxAtrributeFloat()
//
//	PURPOSE:	Get an AmmoBox attribute as a float
//
// ----------------------------------------------------------------------- //

float CServerButeMgr::GetAmmoBoxAttributeFloat( const char* pAttribute )
{
	return (float) m_buteMgr.GetDouble( SBMGR_AMOBOX_TAG, pAttribute );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerButeMgr::GetAmmoBoxAtrributeString()
//
//	PURPOSE:	Get an AmmoBox attribute as a string
//
// ----------------------------------------------------------------------- //

void CServerButeMgr::GetAmmoBoxAttributeString(const char* pAttribute, char* pBuf, int nBufLen )
{
	m_buteMgr.GetString( SBMGR_AMOBOX_TAG, pAttribute, pBuf, nBufLen );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerButeMgr::GetAmmoBoxAttributeListReader()
//
//	PURPOSE:	Get an AmmoBox attribute as a CButeListReader
//
// ----------------------------------------------------------------------- //

void CServerButeMgr::GetAmmoBoxAttributeListReader( CButeListReader *pList, const char* pszAttName, const uint32 nStrLen )
{
	if( !pList )
		return;

	pList->Read( &m_buteMgr, SBMGR_AMOBOX_TAG, pszAttName, nStrLen );
}