// ----------------------------------------------------------------------- //
//
// MODULE  : KeyMgr.cpp
//
// PURPOSE : Attribute file manager for key item info
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "KeyMgr.h"
#include "IDList.h"


#define KEY_TAG					"KeyItem"

#define KEY_NAME				"Name"
#define KEY_NAME_ID				"NameId"
#define KEY_DESC				"DescriptionId"
#define KEY_IMAGE				"Image"
#define KEY_FILENAME			"Filename"
#define KEY_SKIN				"Skin"
#define KEY_RENDERSTYLE			"RenderStyle"
#define	KEY_FXNAME				"FXName"
#define KEY_SOUNDNAME			"SoundName"

CKeyMgr* g_pKeyMgr = LTNULL;

#ifndef _CLIENTBUILD
#include "Character.h"
CKeyMgr CKeyMgrPlugin::sm_KeyMgr;
#endif

static char s_aTagName[30];
static char s_aAttName[30];

KEY::KEY()
{
	nId = 0;
	szName[0] = 0;
	nNameId = 0;
	nDescriptionId = 0;
	szImage[0] = 0;
	szFilename[0] = 0;
	szSoundName[0] = 0;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CKeyMgr::CKeyMgr()
{
}

CKeyMgr::~CKeyMgr()
{
	Term();
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CKeyMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CKeyMgr::Init(const char* szAttributeFile)
{
    if (g_pKeyMgr || !szAttributeFile) return LTFALSE;
    if (!Parse(szAttributeFile)) return LTFALSE;


	// Set up global pointer...
	g_pKeyMgr = this;

	uint16 nNumKeys = 0;

	sprintf(s_aTagName, "%s0", KEY_TAG);
	while (m_buteMgr.Exist(s_aTagName))
	{
		KEY* pNew = debug_new(KEY);
		pNew->nId = nNumKeys;
		m_buteMgr.GetString(s_aTagName, KEY_NAME, "", pNew->szName, sizeof(pNew->szName));
		pNew->nNameId = (uint16)m_buteMgr.GetInt(s_aTagName, KEY_NAME_ID, 0);
		pNew->nDescriptionId = (uint16)m_buteMgr.GetInt(s_aTagName, KEY_DESC, 0);
		m_buteMgr.GetString(s_aTagName, KEY_IMAGE, "", pNew->szImage, sizeof(pNew->szImage));
		m_buteMgr.GetString(s_aTagName, KEY_FILENAME, "", pNew->szFilename, sizeof(pNew->szFilename));
		m_buteMgr.GetString(s_aTagName, KEY_FXNAME, "", pNew->szFXName, sizeof(pNew->szFXName));
		m_buteMgr.GetString(s_aTagName, KEY_SOUNDNAME, "", pNew->szSoundName, sizeof(pNew->szSoundName));

		pNew->blrSkins.Read(&m_buteMgr, s_aTagName, KEY_SKIN, KEY_MAX_FILE_PATH);
		pNew->blrRenderStyles.Read(&m_buteMgr, s_aTagName, KEY_RENDERSTYLE, KEY_MAX_FILE_PATH);
		m_KeyArray.push_back(pNew);

		nNumKeys++;
		sprintf(s_aTagName, "%s%d", KEY_TAG, nNumKeys);
	}

	// Clear out our objref's.
	m_mapKeyControl.clear( );

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CKeyMgr::Term()
//
//	PURPOSE:	Clean up.
//
// ----------------------------------------------------------------------- //

void CKeyMgr::Term()
{
    g_pKeyMgr = LTNULL;

	KeyArray::iterator iter = m_KeyArray.begin();

	while (iter != m_KeyArray.end())
	{
		debug_delete(*iter);
		iter++;
	}

	m_KeyArray.clear();

	m_mapKeyControl.clear( );
}


KEY* CKeyMgr::GetKey(uint16 nID)
{
	if (nID >= m_KeyArray.size()) return LTNULL;
	
	return m_KeyArray[nID];
}

KEY* CKeyMgr::GetKey(const char *pszName)
{
	if (!pszName || !pszName[0]) return LTNULL;
	KeyArray::iterator iter = m_KeyArray.begin();
	while (iter != m_KeyArray.end() && stricmp(pszName,(*iter)->szName) != 0)
	{
		iter++;
	}

	if (iter != m_KeyArray.end())
		return (*iter);
	return LTNULL;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CKeyMgr::AddKeyControl
//
//  PURPOSE:	Add a Key to an objects list keys that control it
//
// ----------------------------------------------------------------------- //

void CKeyMgr::AddKeyControl(HOBJECT hObj, uint16 nKeyId )
{
	if( !hObj || !IsValidKey( nKeyId ))
		return;

	// Map the hobject to the keyid.
	KeyControlMapEntry& entry = m_mapKeyControl[hObj];
	entry.m_ObjRef.SetReceiver( *this );
	entry.m_ObjRef = hObj;
	entry.m_IDList.Add( nKeyId );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CKeyMgr::OnLinkBroken
//
//  PURPOSE:	Handle the case of a link going bad.
//
// ----------------------------------------------------------------------- //

void CKeyMgr::OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj )
{
	// Remove it from our keyid map.  We'll clean it out of our store
	// map later.  We can't do it now since the ref object needs to be
	// valid after returning from this function.
	KeyControlMap::iterator it = m_mapKeyControl.find( hObj );
	if( it == m_mapKeyControl.end( ))
		return;

	m_mapKeyControl.erase( it );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CKeyMgr::CanCharacterControlObject
//
//  PURPOSE:	See if a Character has all the kes needed to control an object
//
// ----------------------------------------------------------------------- //

bool CKeyMgr::CanCharacterControlObject( HOBJECT hChar, HOBJECT hObj )
{
#ifndef _CLIENTBUILD

	if( !hChar || !hObj || !IsCharacter( hChar ))
		return false;

	// If there are no keys that control this object the Character shouldn't be able to control it.
	
	KeyControlMap::iterator iter = m_mapKeyControl.find( hObj );
	if( iter == m_mapKeyControl.end() )
		return false;

	CCharacter *pChar = (CCharacter*)g_pLTServer->HandleToObject( hChar );
	if( !pChar )
		return false;

	IDList *pCharKeyList = pChar->GetKeyList();
	uint8 nDummy;

	IDList& idList = (*iter).second.m_IDList;

	// Make sure the character has all the keys that are needed to control the object...

	for( uint8 i = 0; i < idList.m_IDArray.size(); ++i )
	{
		if( !pCharKeyList->Have( idList.m_IDArray[i], nDummy ))
		{
			return false;
		}
	}

#endif

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CKeyMgr::Save
//
//  PURPOSE:	Save the list of objects that need keys and which keys control them...
//
// ----------------------------------------------------------------------- //

void CKeyMgr::Save( ILTMessage_Write *pMsg, uint32 dwSaveFlags )
{
	if( !pMsg )
		return;

	uint32 dwEntries = m_mapKeyControl.size();
	pMsg->Writeuint32( dwEntries );

	KeyControlMap::iterator iter = m_mapKeyControl.begin();
	while( iter != m_mapKeyControl.end() )
	{
		pMsg->WriteObject( (HOBJECT)(*iter).second.m_ObjRef );
		(*iter).second.m_IDList.Save( pMsg );
		
		++iter;
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CKeyMgr::Load
//
//  PURPOSE:	Load the list of objects that need keys and which keys control them...
//
// ----------------------------------------------------------------------- //

void CKeyMgr::Load( ILTMessage_Read *pMsg, uint32 dwLoadFlags )
{
	if( !pMsg )
		return;

	m_mapKeyControl.clear();

	uint32 dwEntries = pMsg->Readuint32();
	
	for( uint32 i = 0; i < dwEntries; ++i )
	{
		HOBJECT hObj = pMsg->ReadObject();

		KeyControlMapEntry &entry = m_mapKeyControl[hObj];
		entry.m_ObjRef.SetReceiver( *this );
		entry.m_ObjRef = hObj;
		entry.m_IDList.Load( pMsg );
	}
}

#ifndef _CLIENTBUILD
////////////////////////////////////////////////////////////////////////////
//
// CKeyMgrPlugin is used to help facilitate populating the DEdit object
// properties that use CKeyMgr
//
////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CKeyMgrPlugin::PreHook_EditStringList
//
//	PURPOSE:	Fill the string list
//
// ----------------------------------------------------------------------- //

LTRESULT CKeyMgrPlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char** aszStrings,
    uint32* pcStrings,
    const uint32 cMaxStrings,
    const uint32 cMaxStringLength)
{
	if (!g_pKeyMgr)
	{
		// This will set the g_pKeyMgr...Since this could also be
		// set elsewhere, just check for the global bute mgr...

		char szFile[256];
		sprintf(szFile, "%s\\%s", szRezPath, KEY_DEFAULT_FILE);
        sm_KeyMgr.SetInRezFile(LTFALSE);
        sm_KeyMgr.Init(szFile);
	}

	if (!PopulateStringList(aszStrings, pcStrings, cMaxStrings, cMaxStringLength))
	{
		return LT_UNSUPPORTED;
	}

	return LT_OK;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CKeyMgrPlugin::PopulateStringList
//
//	PURPOSE:	Populate the list
//
// ----------------------------------------------------------------------- //

LTBOOL CKeyMgrPlugin::PopulateStringList(char** aszStrings, uint32* pcStrings,
    const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
    if (!aszStrings || !pcStrings) return LTFALSE;
	_ASSERT(aszStrings && pcStrings);

	// Add an entry for each Key type

	int nNumKey = g_pKeyMgr->GetNumKeys();
	_ASSERT(nNumKey > 0);

    KEY* pKey = LTNULL;

	for (int i=0; i < nNumKey; i++)
	{
		_ASSERT(cMaxStrings > (*pcStrings) + 1);

		pKey = g_pKeyMgr->GetKey(i);
		if (pKey && pKey->szName[0])
		{
            uint32 dwImpactFXNameLen = strlen(pKey->szName);

			if (dwImpactFXNameLen < cMaxStringLength && ((*pcStrings) + 1) < cMaxStrings)
			{
				strcpy(aszStrings[(*pcStrings)++], pKey->szName);
			}
		}
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CKeyMgrPlugin::PreHook_Dims
//
//	PURPOSE:	Determine the dims for this prop
//
// ----------------------------------------------------------------------- //

LTRESULT CKeyMgrPlugin::PreHook_Dims(
			const char* szRezPath,
			const char* szPropValue,
			char* szModelFilenameBuf,
			int	  nModelFilenameBufLen,
			LTVector & vDims)
{

	if (!szModelFilenameBuf || nModelFilenameBufLen < 1) return LT_UNSUPPORTED;

	szModelFilenameBuf[0] = '\0';

	if (!g_pKeyMgr)
	{
		// This will set the g_pKeyMgr...Since this could also be
		// set elsewhere, just check for the global bute mgr...

		char szFile[256];
		sprintf(szFile, "%s\\%s", szRezPath, KEY_DEFAULT_FILE);
        sm_KeyMgr.SetInRezFile(LTFALSE);
        sm_KeyMgr.Init(szFile);
	}

	KEY* pKey = g_pKeyMgr->GetKey((char*)szPropValue);
	if (!pKey || !pKey->szFilename[0])
	{
		return LT_UNSUPPORTED;
	}

	strcpy(szModelFilenameBuf, pKey->szFilename);
	
	// Need to convert the .ltb filename to one that DEdit understands...

	ConvertLTBFilename(szModelFilenameBuf);


	return LT_OK;
}
#endif
