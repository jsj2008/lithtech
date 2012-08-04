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

#define	ABM_REQUIREMENT						"Requirement"
#define	ABM_REQUIREMENT_MODEL				"Model"
#define	ABM_REQUIREMENT_STYLE				"Style"
#define	ABM_REQUIREMENT_ATTACHMENT			"Attachment"
#define	ABM_REQUIREMENT_SOCKET				"Socket"


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

LTBOOL CAttachButeMgr::Init(ILTCSBase *pInterface, const char* szAttributeFile)
{
    if (g_pAttachButeMgr || !szAttributeFile) return LTFALSE;
    if (!Parse(pInterface, szAttributeFile)) return LTFALSE;

	// Set up global pointer

	g_pAttachButeMgr = this;

	// See how many attribute templates there are

	m_cAttachmentID = 0;
	sprintf(s_aTagName, "%s%d", ABM_ATTACHMENT, m_cAttachmentID);

	while (m_buteMgr.Exist(s_aTagName))
	{
		m_cAttachmentID++;
		sprintf(s_aTagName, "%s%d", ABM_ATTACHMENT, m_cAttachmentID);
	}

	m_cRequirementID = 0;
	sprintf(s_aTagName, "%s%d", ABM_REQUIREMENT, m_cRequirementID);

	while (m_buteMgr.Exist(s_aTagName))
	{
		m_cRequirementID++;
		sprintf(s_aTagName, "%s%d", ABM_REQUIREMENT, m_cRequirementID);
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
		if ( !strcmp(szName, GetAttachmentName(iAttachmentID)) )
		{
			return iAttachmentID;
		}
	}

	return -1;
}

CString CAttachButeMgr::GetAttachmentName(int nAttachmentID)
{
	if (nAttachmentID < 0 || nAttachmentID > m_cAttachmentID) return "";
	sprintf(s_aTagName, "%s%d", ABM_ATTACHMENT, nAttachmentID);
	return m_buteMgr.GetString(s_aTagName, ABM_ATTACHMENT_NAME);
}

int CAttachButeMgr::GetAttachmentType(int nAttachmentID)
{
	if (nAttachmentID < 0 || nAttachmentID > m_cAttachmentID) return 255;
	sprintf(s_aTagName, "%s%d", ABM_ATTACHMENT, nAttachmentID);
	return m_buteMgr.GetInt(s_aTagName, ABM_ATTACHMENT_TYPE);
}

CString CAttachButeMgr::GetAttachmentProperties(int nAttachmentID)
{
	if (nAttachmentID < 0 || nAttachmentID > m_cAttachmentID) return "";
	sprintf(s_aTagName, "%s%d", ABM_ATTACHMENT, nAttachmentID);
	return m_buteMgr.GetString(s_aTagName, ABM_ATTACHMENT_PROPERTIES);
}

int CAttachButeMgr::GetAttachmentWeapon(int nAttachmentID)
{
	if (nAttachmentID < 0 || nAttachmentID > m_cAttachmentID) return 255;
	sprintf(s_aTagName, "%s%d", ABM_ATTACHMENT, nAttachmentID);
	return m_buteMgr.GetInt(s_aTagName, ABM_ATTACHMENT_WEAPON);
}

CString CAttachButeMgr::GetAttachmentClass(int nAttachmentID)
{
	if (nAttachmentID < 0 || nAttachmentID > m_cAttachmentID) return "";
	sprintf(s_aTagName, "%s%d", ABM_ATTACHMENT, nAttachmentID);
	return m_buteMgr.GetString(s_aTagName, ABM_ATTACHMENT_CLASS);
}

CString CAttachButeMgr::GetAttachmentModel(int nAttachmentID)
{
	if (nAttachmentID < 0 || nAttachmentID > m_cAttachmentID) return "";
	sprintf(s_aTagName, "%s%d", ABM_ATTACHMENT, nAttachmentID);
	return m_buteMgr.GetString(s_aTagName, ABM_ATTACHMENT_MODEL);
}

CString CAttachButeMgr::GetAttachmentSkin(int nAttachmentID)
{
	if (nAttachmentID < 0 || nAttachmentID > m_cAttachmentID) return "";
	sprintf(s_aTagName, "%s%d", ABM_ATTACHMENT, nAttachmentID);
	return m_buteMgr.GetString(s_aTagName, ABM_ATTACHMENT_SKIN);
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



int CAttachButeMgr::GetNumRequirements(char *szModel,char *szStyle)
{
	int numRequired = 0;
	int n = 0;
	CString sTestModel;
	CString sTestStyle;

	while (n < m_cRequirementID)
	{
		sprintf(s_aTagName, "%s%d", ABM_REQUIREMENT, n);
		sTestModel = m_buteMgr.GetString(s_aTagName, ABM_REQUIREMENT_MODEL);

		if (sTestModel.CompareNoCase(szModel) == 0)
		{
			sTestStyle = m_buteMgr.GetString(s_aTagName, ABM_REQUIREMENT_STYLE);
			if (sTestStyle.GetLength() == 0 || sTestStyle.CompareNoCase(szStyle) == 0)
			{
				numRequired++;
			}
		}

		n++;
	}

	return numRequired;
}

int CAttachButeMgr::GetRequirementIDs(char *szModel,char *szStyle,int *pBuf,int nBufLen)
{
	int numRequired = 0;
	int n = 0;
	CString sTestModel;
	CString sTestStyle;

	while (n < m_cRequirementID && numRequired < nBufLen)
	{
		sprintf(s_aTagName, "%s%d", ABM_REQUIREMENT, n);
		sTestModel = m_buteMgr.GetString(s_aTagName, ABM_REQUIREMENT_MODEL);

		if (sTestModel.CompareNoCase(szModel) == 0)
		{
			sTestStyle = m_buteMgr.GetString(s_aTagName, ABM_REQUIREMENT_STYLE);
			if (sTestStyle.GetLength() == 0 || sTestStyle.CompareNoCase(szStyle) == 0)
			{
				pBuf[numRequired] = n;
				numRequired++;
			}
		}

		n++;
	}

	return numRequired;
}


int CAttachButeMgr::GetRequirementAttachment(int nRequirementID)
{
	if (nRequirementID < 0 || nRequirementID > m_cRequirementID) return -1;
	sprintf(s_aTagName, "%s%d", ABM_REQUIREMENT, nRequirementID);
	CString attachName = m_buteMgr.GetString(s_aTagName, ABM_REQUIREMENT_ATTACHMENT);
	return GetAttachmentIDByName(attachName);
}


CString CAttachButeMgr::GetRequirementSocket(int nRequirementID)
{
	if (nRequirementID < 0 || nRequirementID > m_cRequirementID) return "";
	sprintf(s_aTagName, "%s%d", ABM_REQUIREMENT, nRequirementID);
	return m_buteMgr.GetString(s_aTagName, ABM_REQUIREMENT_SOCKET);

}