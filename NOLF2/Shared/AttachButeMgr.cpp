// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include "AttachButeMgr.h"
#include "CommonUtilities.h"

// Globals/statics

CAttachButeMgr* g_pAttachButeMgr = LTNULL;

static char s_aTagName[30];
static char s_aAttName[100];

// Defines

#define	ABM_ATTACHMENT						"Attachment"
#define	ABM_ATTACHMENT_NAME					"Name"
#define	ABM_ATTACHMENT_PROPERTIES			"Properties"
#define	ABM_ATTACHMENT_TYPE					"Type"
#define	ABM_ATTACHMENT_WEAPON				"Weapon"
#define	ABM_ATTACHMENT_CLASS				"Class"
#define	ABM_ATTACHMENT_MODEL				"Model"
#define	ABM_ATTACHMENT_SKIN					"Skin"
#define	ABM_ATTACHMENT_DEATH				"Death"
#define	ABM_ATTACHMENT_SHOT					"Shot"
#define ABM_ATTACHMENT_DELETE_ON_DEATH		"DeleteOnDeath"
#define	ABM_ATTACHMENT_RENDERSTYLE			"RenderStyle"
#define ABM_ATTACHMENT_DETACH_WHEN_SHOT		"DetachWhenShot"
#define	ABM_ATTACHMENT_DEBRIS_TYPE			"DebrisType"
#define	ABM_ATTACHMENT_TRANSLUCENT			"Translucent"

#define	ABM_REQUIREMENT						"Requirement"
#define	ABM_REQUIREMENT_MODEL				"Model"
#define	ABM_REQUIREMENT_STYLE				"Style"
#define	ABM_REQUIREMENT_ATTACHMENT			"Attachment"
#define	ABM_REQUIREMENT_SOCKET				"Socket"

#define ABM_PVATTACHMENT					"PVAttachment"
#define	ABM_PVATTACHMENT_NAME				"Name"
#define	ABM_PVATTACHMENT_MODEL				"Model"
#define ABM_PVATTACHMENT_SKIN				"Skin"
#define ABM_PVATTACHMENT_RENDERSTYLE		"RenderStyle"
#define ABM_PVATTACHMENT_SCALE				"Scale"
#define	ABM_PVATTACHMENT_TRANSLUCENT		"Translucent"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachButeMgr::CAttachButeMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CAttachButeMgr::CAttachButeMgr()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachButeMgr::~CAttachButeMgr()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CAttachButeMgr::~CAttachButeMgr()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachButeMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CAttachButeMgr::Init(const char* szAttributeFile)
{
    if (g_pAttachButeMgr || !szAttributeFile) return LTFALSE;
    if (!Parse(szAttributeFile)) return LTFALSE;

	// Set up global pointer

	g_pAttachButeMgr = this;

	// See how many attribute templates there are

	// Attachment records...

	m_cAttachmentID = 0;
	sprintf(s_aTagName, "%s%d", ABM_ATTACHMENT, m_cAttachmentID);

	while (m_buteMgr.Exist(s_aTagName))
	{
		m_cAttachmentID++;
		sprintf(s_aTagName, "%s%d", ABM_ATTACHMENT, m_cAttachmentID);
	}

	// Requirement records...

	m_cRequirementID = 0;
	sprintf(s_aTagName, "%s%d", ABM_REQUIREMENT, m_cRequirementID);

	while (m_buteMgr.Exist(s_aTagName))
	{
		m_cRequirementID++;
		sprintf(s_aTagName, "%s%d", ABM_REQUIREMENT, m_cRequirementID);
	}

	// PlayerView Attachment records...

	m_cPVAttachmentID = 0;
	sprintf( s_aTagName, "%s%d", ABM_PVATTACHMENT, m_cPVAttachmentID );

	while( m_buteMgr.Exist( s_aTagName ))
	{
		m_cPVAttachmentID++;
		sprintf( s_aTagName, "%s%d", ABM_PVATTACHMENT, m_cPVAttachmentID );
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachButeMgr::Term()
//
//	PURPOSE:	Clean up.
//
// ----------------------------------------------------------------------- //

void CAttachButeMgr::Term()
{
	m_buteMgr.Term();
    g_pAttachButeMgr = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachButeMgr::GetAttachment*()
//
//	PURPOSE:	Gets an attribute template property or information
//
// ----------------------------------------------------------------------- //

int CAttachButeMgr::GetAttachmentIDByName(const char *szName)
{
	for ( int iAttachmentID = 0 ; iAttachmentID < m_cAttachmentID ; iAttachmentID++ )
	{
		char szTestName[128];
		GetAttachmentName(iAttachmentID,szTestName,sizeof(szTestName));
		if ( strcmp(szName,szTestName) == 0 )
		{
			return iAttachmentID;
		}
	}

	return -1;
}

void CAttachButeMgr::GetAttachmentName(int nAttachmentID, char *pBuf, uint32 nBufLen)
{
	if (nAttachmentID < 0 || nAttachmentID > m_cAttachmentID)
	{
		pBuf[0] = LTNULL;
		return;
	}

	sprintf(s_aTagName, "%s%d", ABM_ATTACHMENT, nAttachmentID);
	m_buteMgr.GetString(s_aTagName, ABM_ATTACHMENT_NAME, "", pBuf, nBufLen);
}

int CAttachButeMgr::GetAttachmentType(int nAttachmentID)
{
	if (nAttachmentID < 0 || nAttachmentID > m_cAttachmentID) return 255;
	sprintf(s_aTagName, "%s%d", ABM_ATTACHMENT, nAttachmentID);
	return m_buteMgr.GetInt(s_aTagName, ABM_ATTACHMENT_TYPE);
}

void CAttachButeMgr::GetAttachmentProperties(int nAttachmentID, char *pBuf, uint32 nBufLen)
{
	if (nAttachmentID < 0 || nAttachmentID > m_cAttachmentID)
	{
		pBuf[0] = LTNULL;
		return;
	}

	sprintf(s_aTagName, "%s%d", ABM_ATTACHMENT, nAttachmentID);
	m_buteMgr.GetString(s_aTagName, ABM_ATTACHMENT_PROPERTIES, "", pBuf, nBufLen);
}

int CAttachButeMgr::GetAttachmentWeapon(int nAttachmentID)
{
	if (nAttachmentID < 0 || nAttachmentID > m_cAttachmentID) return 255;
	sprintf(s_aTagName, "%s%d", ABM_ATTACHMENT, nAttachmentID);
	return m_buteMgr.GetInt(s_aTagName, ABM_ATTACHMENT_WEAPON);
}

void CAttachButeMgr::GetAttachmentClass(int nAttachmentID, char *pBuf, uint32 nBufLen)
{
	if (nAttachmentID < 0 || nAttachmentID > m_cAttachmentID)
	{
		pBuf[0] = LTNULL;
		return;
	}

	sprintf(s_aTagName, "%s%d", ABM_ATTACHMENT, nAttachmentID);
	m_buteMgr.GetString(s_aTagName, ABM_ATTACHMENT_CLASS, "", pBuf, nBufLen);
}

void CAttachButeMgr::GetAttachmentModel(int nAttachmentID, char *pBuf, uint32 nBufLen)
{
	if (nAttachmentID < 0 || nAttachmentID > m_cAttachmentID)
	{
		pBuf[0] = LTNULL;
		return;
	}

	sprintf(s_aTagName, "%s%d", ABM_ATTACHMENT, nAttachmentID);
	m_buteMgr.GetString(s_aTagName, ABM_ATTACHMENT_MODEL, "", pBuf, nBufLen);
}

void CAttachButeMgr::GetAttachmentSkin(int nAttachmentID, char *pBuf, uint32 nBufLen)
{
	if (nAttachmentID < 0 || nAttachmentID > m_cAttachmentID)
	{
		pBuf[0] = LTNULL;
		return;
	}
		
	sprintf(s_aTagName, "%s%d", ABM_ATTACHMENT, nAttachmentID);
	m_buteMgr.GetString(s_aTagName, ABM_ATTACHMENT_SKIN, "", pBuf, nBufLen);
}

int CAttachButeMgr::GetAttachmentDeath(int nAttachmentID)
{
	if (nAttachmentID < 0 || nAttachmentID > m_cAttachmentID) return 255;
	sprintf(s_aTagName, "%s%d", ABM_ATTACHMENT, nAttachmentID);
	return m_buteMgr.GetInt(s_aTagName, ABM_ATTACHMENT_DEATH);
}

int CAttachButeMgr::GetAttachmentShot(int nAttachmentID)
{
	if (nAttachmentID < 0 || nAttachmentID > m_cAttachmentID) return 255;
	sprintf(s_aTagName, "%s%d", ABM_ATTACHMENT, nAttachmentID);
	return m_buteMgr.GetInt(s_aTagName, ABM_ATTACHMENT_SHOT);
}

bool CAttachButeMgr::GetAttachmentDeleteOnDeath(int nAttachmentID)
{
	if (nAttachmentID < 0 || nAttachmentID > m_cAttachmentID) return false;
	sprintf(s_aTagName, "%s%d", ABM_ATTACHMENT, nAttachmentID);
	return !!m_buteMgr.GetInt( s_aTagName, ABM_ATTACHMENT_DELETE_ON_DEATH);
}

bool CAttachButeMgr::GetAttachmentTranslcuent(int nAttachmentID)
{
	if (nAttachmentID < 0 || nAttachmentID > m_cAttachmentID) return false;
	sprintf(s_aTagName, "%s%d", ABM_ATTACHMENT, nAttachmentID);
	return !!m_buteMgr.GetInt( s_aTagName, ABM_ATTACHMENT_TRANSLUCENT, 0);
}

void CAttachButeMgr::GetAttachmentSkins(int nAttachmentID, CButeListReader* pSkinReader)
{
	ASSERT((nAttachmentID >= 0) && (nAttachmentID < m_cAttachmentID));
	sprintf(s_aTagName, "%s%d", ABM_ATTACHMENT, nAttachmentID);
	pSkinReader->Read(&m_buteMgr, s_aTagName, ABM_ATTACHMENT_SKIN, MAX_PATH);
}

void CAttachButeMgr::CopyAttachmentSkins(int nAttachmentID, char* paszDest, int strLen)
{
	ASSERT((nAttachmentID >= 0) && (nAttachmentID <= m_cAttachmentID));
	sprintf(s_aTagName, "%s%d", ABM_ATTACHMENT, nAttachmentID);
	m_blrAttachSkinReader.ReadAndCopy(&m_buteMgr, s_aTagName, ABM_ATTACHMENT_SKIN, paszDest, strLen);
}

void CAttachButeMgr::GetAttachmentRenderStyles(int nAttachmentID, CButeListReader* pRenderStyleReader)
{
	ASSERT((nAttachmentID >= 0) && (nAttachmentID < m_cAttachmentID));
	sprintf(s_aTagName, "%s%d", ABM_ATTACHMENT, nAttachmentID);
	pRenderStyleReader->Read(&m_buteMgr, s_aTagName, ABM_ATTACHMENT_RENDERSTYLE, MAX_PATH);
}

void CAttachButeMgr::CopyAttachmentRenderStyles(int nAttachmentID, char* paszDest, int strLen)
{
	ASSERT((nAttachmentID >= 0) && (nAttachmentID <= m_cAttachmentID));
	sprintf(s_aTagName, "%s%d", ABM_ATTACHMENT, nAttachmentID);
	m_blrAttachRenderStyleReader.ReadAndCopy(&m_buteMgr, s_aTagName, ABM_ATTACHMENT_RENDERSTYLE, paszDest, strLen);
}

int CAttachButeMgr::GetNumRequirements( const char *szModel,char *szStyle )
{
	int numRequired = 0;
	int n = 0;
	char szTestModel[256];
	char szTestStyle[256];

	while (n < m_cRequirementID)
	{
		sprintf(s_aTagName, "%s%d", ABM_REQUIREMENT, n);
		m_buteMgr.GetString(s_aTagName, ABM_REQUIREMENT_MODEL, "", szTestModel, sizeof(szTestModel));

		if (stricmp(szTestModel,szModel) == 0)
		{
			m_buteMgr.GetString(s_aTagName, ABM_REQUIREMENT_STYLE, "", szTestStyle, sizeof(szTestStyle));
			if (strlen(szTestStyle) == 0 || stricmp(szTestStyle,szStyle) == 0)
			{
				numRequired++;
			}
		}

		n++;
	}

	return numRequired;
}

int CAttachButeMgr::GetRequirementIDs( const char *szModel,int *pBuf,int nBufLen )
{
	int numRequired = 0;
	int n = 0;
	char szTestModel[256];

	while (n < m_cRequirementID && numRequired < nBufLen)
	{
		sprintf(s_aTagName, "%s%d", ABM_REQUIREMENT, n);
		m_buteMgr.GetString(s_aTagName, ABM_REQUIREMENT_MODEL, "", szTestModel, sizeof(szTestModel));

		if (stricmp(szTestModel,szModel) == 0)
		{
			pBuf[numRequired] = n;
			numRequired++;
		}

		n++;
	}

	return numRequired;
}


int CAttachButeMgr::GetRequirementAttachment(int nRequirementID)
{
	if (nRequirementID < 0 || nRequirementID > m_cRequirementID) return -1;
	sprintf(s_aTagName, "%s%d", ABM_REQUIREMENT, nRequirementID);
	char attachName[256]; 
	m_buteMgr.GetString(s_aTagName, ABM_REQUIREMENT_ATTACHMENT, "", attachName, sizeof(attachName));
	return GetAttachmentIDByName(attachName);
}


void CAttachButeMgr::GetRequirementSocket(int nRequirementID, char *pBuf, uint32 nBufLen)
{
	if (nRequirementID < 0 || nRequirementID > m_cRequirementID)
	{
		pBuf[0] = LTNULL;
		return;
	}
	sprintf(s_aTagName, "%s%d", ABM_REQUIREMENT, nRequirementID);
	m_buteMgr.GetString(s_aTagName, ABM_REQUIREMENT_SOCKET, "", pBuf, nBufLen);

}

bool CAttachButeMgr::GetAttachmentDetachWhenShot( int nAttachmentID )
{
	if (nAttachmentID < 0 || nAttachmentID > m_cAttachmentID) return false;
	sprintf(s_aTagName, "%s%d", ABM_ATTACHMENT, nAttachmentID);
	return !!m_buteMgr.GetInt( s_aTagName, ABM_ATTACHMENT_DETACH_WHEN_SHOT, true);
}

void CAttachButeMgr::GetAttachmentDebrisType( int nAttachmentID, char *pBuf, uint32 nBufLen )
{
	if (nAttachmentID < 0 || nAttachmentID > m_cAttachmentID)
	{
		pBuf[0] = LTNULL;
		return;
	}

	sprintf(s_aTagName, "%s%d", ABM_ATTACHMENT, nAttachmentID);
	m_buteMgr.GetString(s_aTagName, ABM_ATTACHMENT_DEBRIS_TYPE, "", pBuf, nBufLen);	
}


////////////////////////////////////////////////////
// PlayerView Attachments
////////////////////////////////////////////////////

int CAttachButeMgr::GetPVAttachmentIDByName( const char *szName )
{
	for( int i = 0; i < m_cPVAttachmentID; ++i )
	{
		char szTestName[128] = {0};
		GetPVAttachmentName( i, szTestName, ARRAY_LEN( szTestName ));
		
		if( strcmp( szName, szTestName ) == 0 )
		{
			return i;
		}
	}

	return -1;
}

void CAttachButeMgr::GetPVAttachmentName( int nPVAttachmentID, char *pBuf, uint32 nBufLen )
{
	if( nPVAttachmentID < 0 || nPVAttachmentID > m_cPVAttachmentID )
	{
		pBuf[0] = LTNULL;
		return;
	}

	sprintf( s_aTagName, "%s%d", ABM_PVATTACHMENT, nPVAttachmentID );
	m_buteMgr.GetString( s_aTagName, ABM_PVATTACHMENT_NAME, "", pBuf, nBufLen );
}

void CAttachButeMgr::GetPVAttachmentModel( int nPVAttachmentID, char *pBuf, uint32 nBufLen )
{
	if (nPVAttachmentID < 0 || nPVAttachmentID > m_cPVAttachmentID)
	{
		pBuf[0] = LTNULL;
		return;
	}

	sprintf( s_aTagName, "%s%d", ABM_PVATTACHMENT, nPVAttachmentID );
	m_buteMgr.GetString( s_aTagName, ABM_PVATTACHMENT_MODEL, "", pBuf, nBufLen );
}

void CAttachButeMgr::GetPVAttachmentSkins( int nPVAttachmentID, CButeListReader* pSkinReader )
{
	ASSERT( (nPVAttachmentID >= 0) && (nPVAttachmentID < m_cPVAttachmentID) );
	
	sprintf( s_aTagName, "%s%d", ABM_PVATTACHMENT, nPVAttachmentID );
	pSkinReader->Read( &m_buteMgr, s_aTagName, ABM_PVATTACHMENT_SKIN, MAX_PATH );
}

void CAttachButeMgr::CopyPVAttachmentSkins( int nPVAttachmentID, char* paszDest, int strLen )
{
	ASSERT( (nPVAttachmentID >= 0) && (nPVAttachmentID <= m_cPVAttachmentID) );
	
	sprintf( s_aTagName, "%s%d", ABM_PVATTACHMENT, nPVAttachmentID );
	m_blrAttachSkinReader.ReadAndCopy( &m_buteMgr, s_aTagName, ABM_PVATTACHMENT_SKIN, paszDest, strLen );
}

void CAttachButeMgr::GetPVAttachmentRenderStyles( int nPVAttachmentID, CButeListReader* pRenderStyleReader )
{
	ASSERT( (nPVAttachmentID >= 0) && (nPVAttachmentID < m_cPVAttachmentID) );

	sprintf( s_aTagName, "%s%d", ABM_PVATTACHMENT, nPVAttachmentID );
	pRenderStyleReader->Read( &m_buteMgr, s_aTagName, ABM_PVATTACHMENT_RENDERSTYLE, MAX_PATH );
}

void CAttachButeMgr::CopyPVAttachmentRenderStyles( int nPVAttachmentID, char* paszDest, int strLen )
{
	ASSERT( (nPVAttachmentID >= 0) && (nPVAttachmentID <= m_cPVAttachmentID) );
	
	sprintf( s_aTagName, "%s%d", ABM_PVATTACHMENT, nPVAttachmentID );
	m_blrAttachRenderStyleReader.ReadAndCopy( &m_buteMgr, s_aTagName, ABM_PVATTACHMENT_RENDERSTYLE, paszDest, strLen );
}

bool CAttachButeMgr::GetPVAttachmentTranslcuent( int nPVAttachmentID )
{
	if( nPVAttachmentID < 0 || nPVAttachmentID > m_cPVAttachmentID )
		return false;

	sprintf( s_aTagName, "%s%d", ABM_PVATTACHMENT, nPVAttachmentID );
	return !!m_buteMgr.GetInt( s_aTagName, ABM_PVATTACHMENT_TRANSLUCENT, 0 );
}

LTVector CAttachButeMgr::GetPVAttachmentScale( int nPVAttachmentID )
{
	if( nPVAttachmentID < 0 || nPVAttachmentID > m_cPVAttachmentID )
	{
		return LTVector(0, 0, 0);
	}
		
	sprintf( s_aTagName, "%s%d", ABM_PVATTACHMENT, nPVAttachmentID );
	return m_buteMgr.GetVector( s_aTagName, ABM_PVATTACHMENT_SCALE, CAVector( 0, 0, 0));
}