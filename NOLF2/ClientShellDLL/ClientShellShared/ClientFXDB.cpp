#include "stdafx.h"
#include "FxFlags.h"
#include "FxDefs.h"
#include "iltmessage.h"
#include "iltdrawprim.h"
#include "ClientFXDB.h"
#include "CMoveMgr.h"
#include "WinUtil.h"

#define MAX_TAG_SIZE		(64)
#define MAX_LINE_SIZE		(2048)

typedef int (*FX_GETNUM)();
typedef FX_REF (*FX_GETREF)(int);

//-----------------------------------------------------------------
// DLL loading data
//-----------------------------------------------------------------
#ifdef _WIN32
	#include "windows.h"

namespace
{
	char		sDLLTmpFile[MAX_PATH + 1] = "";
}

#endif

//-----------------------------------------------------------------
// Link status...
//-----------------------------------------------------------------
struct LINK_STATUS
{
	bool			m_bLinked;
	uint32			m_dwLinkedID;
	char			m_sLinkedNodeName[32];
};

//-----------------------------------------------------------------
// Helpers to read from a text .fxf file...
//-----------------------------------------------------------------

template< typename T>
inline void	ReadTextFile( ILTStream *pStream, const char *szFormat, T *t1, T *t2 = LTNULL, T *t3 = LTNULL, T *t4 = LTNULL,  T *t5 = LTNULL  )
{
	char	szTag[MAX_TAG_SIZE] = {0};
	char	szLine[MAX_LINE_SIZE] = {0};
	
	// Save the current pos before it moves when we read

	uint32	dwPos = pStream->GetPos();
	uint32	dwLen = pStream->GetLen();

	if( LT_OK != pStream->Read( szLine, ((ARRAY_LEN( szLine ) + dwPos) > dwLen ? dwLen - dwPos : ARRAY_LEN( szLine ) )))
	{
		g_pLTClient->CPrint( "A line in the *.fxf file is too long!!" );
	}
	
	// Only get the info we want, one line at a time...
	
	strtok( szLine, "\n" );
	sscanf( szLine, szFormat, szTag, t1, t2, t3, t4, t5 );
	
	if( LT_OK != pStream->SeekTo( dwPos + strlen( szLine ) ))
	{
		g_pLTClient->CPrint( "Couldn't set the file ptr position for *.fxf file" );
	}
}


//-----------------------------------------------------------------
// Helper function to setup keys
//
//  Given a key data structure as well as a list of
//	properties, it will pull all of the data for the
//	key out of the property list and setup the key
//	with it.
//
//------------------------------------------------------------------
static void SetupKey(FX_KEY* pKey, FX_PROP* pPropList, uint32 nNumProps)
{
	//go through the property list and parse in all the known variables and
	//count up how many of each key type we have
	for(uint32 nCurrProp = 0; nCurrProp < nNumProps; nCurrProp++)
	{
		FX_PROP& fxProp = pPropList[nCurrProp];

		if( !_stricmp( fxProp.m_sName, FXPROP_DISABLEATDIST ))
		{
			pKey->m_bDisableAtDistance = fxProp.GetComboVal() ? true : false;
		}
		else if( !_stricmp( fxProp.m_sName, FXPROP_MAXSTARTOFFSET ))
		{
			pKey->m_fMaxStartOffset = fxProp.GetFloatVal();
		}
		else if( !_stricmp( fxProp.m_sName, FXPROP_RANDOMSTARTOFFSET ))
		{
			pKey->m_bRandomStartOffset = fxProp.GetComboVal() ? true : false;
		}
		else if( !_stricmp( fxProp.m_sName, FXPROP_STARTOFFSETINTERVAL ))
		{
			pKey->m_fStartOffsetInterval = fxProp.GetFloatVal();
		}
		else if( !_stricmp( fxProp.m_sName, FXPROP_SMOOTHSHUTDOWN ))
		{
			pKey->m_bSmoothShutdown = fxProp.GetComboVal() ? true : false;
		}
		else if( !_stricmp( fxProp.m_sName, FXPROP_DETAILLEVEL ))
		{
			pKey->m_nDetailLevel = fxProp.GetComboVal();
			assert((pKey->m_nDetailLevel < FX_NUM_DETAIL_SETTINGS) && "Found an invalid detail setting");
		}
		else if( !_stricmp( fxProp.m_sName, FXPROP_ISGORE ))
		{
			pKey->m_bGore = fxProp.GetComboVal() ? true : false;
		}
	}
}

//-----------------------------------------------------------------
// CClientFXDB construction
//-----------------------------------------------------------------
CClientFXDB::CClientFXDB()
{
	m_nNumEffectTypes		= 0;
	m_pEffectTypes			= NULL;
	m_hDLLInst				= NULL;

	m_pfnSetPlayer			= NULL;
	m_pfnSetAppFocus		= NULL;
	m_pfnDeleteFX			= NULL;
	m_pfnSetCreateFunction	= NULL;

}

CClientFXDB::~CClientFXDB()
{
	Term();
}

bool CClientFXDB::Init(ILTClient* pLTClient)
{
	// Try and load our ClientFX dll (.fxd)
	if( !LoadFxDll() )
		return false;
	
	// Search for and load ALL valid GroupFX (.fxf)
	//
	
	// Obtain a snapshot of our ClientFX directory
	FileEntry *pFiles = pLTClient->GetFileList("ClientFX");
	if( !pFiles ) 
		return false;

	FileEntry *pEntry = pFiles;
	while( pEntry )
	{
		// Ignore directorys... only look at files
		if( pEntry->m_Type == TYPE_FILE )
		{
			char drive[_MAX_DRIVE];
			char dir[_MAX_DIR];
			char fname[_MAX_FNAME];
			char ext[_MAX_EXT];

			_splitpath( pEntry->m_pFullFilename, drive, dir, fname, ext );

			// Is this a ClientFx file?
			if( !stricmp( ".fxf", ext ) )
			{
				// Try and load it
				if( !LoadFxGroups( pLTClient, pEntry->m_pFullFilename ) )
				{
					return false;
				}
			}
		}

		pEntry = pEntry->m_pNext;
	}

	// Free the List we obtained
	g_pLTClient->FreeFileList( pFiles );

	//success
	return true;
}

void CClientFXDB::Term()
{
	//empty out our types
	debug_deletea(m_pEffectTypes);
	m_pEffectTypes		= NULL;
	m_nNumEffectTypes	= 0;

	// Delete all the FX groups
	CLinkListNode<FX_GROUP *> *pGroupNode = m_collGroupFX.GetHead();

	while (pGroupNode)
	{
		//delete all the key property lists
		for(uint32 nCurrKey = 0; nCurrKey < pGroupNode->m_Data->m_nNumKeys; nCurrKey++)
		{
			if(m_pfnFreePropList)
			{
				m_pfnFreePropList(pGroupNode->m_Data->m_pKeys[nCurrKey].m_pProps);
				pGroupNode->m_Data->m_pKeys[nCurrKey].m_pProps = NULL;
			}
		}

		debug_delete( pGroupNode->m_Data );		
		pGroupNode = pGroupNode->m_pNext;
	}
	m_collGroupFX.RemoveAll();

	UnloadFxDll();
}

//-----------------------------------------------------------------
// CClientFXDB DLL management
//-----------------------------------------------------------------

//loads the effect DLL
bool CClientFXDB::LoadFxDll()
{

#ifdef WIN32

	//make sure that we don't have any other dll's already bound
	UnloadFxDll();

	// Load the library
	const char *sName = "ClientFX.fxd";
	char sTmp[MAX_PATH + 1];
	LTSNPrintF(sTmp, sizeof(sTmp), "Game\\%s", sName);

	//if we have a local copy use it
	if( CWinUtil::FileExist( sTmp ))
	{

		m_hDLLInst = ::LoadLibrary(sTmp);
		if (!m_hDLLInst) 
			return false;
	}
	else
	{
		//otherwise copy it out of the rez file
		if (!strlen(sDLLTmpFile))
		{
			DWORD sz = ::GetTempPath(sizeof(sTmp),sTmp);
			if (sz > sizeof(sTmp)) 
				return false;

			::GetTempFileName(sTmp,"cfx",0, sDLLTmpFile);

			if (LT_OK != g_pLTClient->CopyFile(sName,sDLLTmpFile))
				return false;
		}



		m_hDLLInst = ::LoadLibrary(sDLLTmpFile);
		if (!m_hDLLInst) 
			return false;
	}

	//merge our interface database with the database in the DLL we just loaded.
	TSetMasterFn pSetMasterFn = (TSetMasterFn)GetProcAddress(m_hDLLInst, "SetMasterDatabase");
	//check if the function existed.
	if (pSetMasterFn != NULL)
	{
		//merge our database with theirs
		pSetMasterFn(GetMasterDatabase());
	}

	// Attempt to retrieve the FX reference structure function

	FX_GETNUM pfnNum = (FX_GETNUM)::GetProcAddress(m_hDLLInst, "fxGetNum");
	if (!pfnNum) 
		return false;

	FX_GETREF pfnRef = (FX_GETREF)::GetProcAddress(m_hDLLInst, "fxGetRef");
	if (!pfnRef) 
		return false;

	FX_SETPLAYERFUNC pfnSetPlayer = (FX_SETPLAYERFUNC)::GetProcAddress(m_hDLLInst, "fxSetPlayer");
	if (!pfnSetPlayer) 
		return false;

	FX_SETAPPFOCUS pfnSetAppFocus = (FX_SETAPPFOCUS)::GetProcAddress(m_hDLLInst, "fxSetAppFocus");
	if (!pfnSetAppFocus) 
		return false;

	FX_DELETEFUNC	pfnDelete = (FX_DELETEFUNC)::GetProcAddress(m_hDLLInst, "fxDelete");
	if( !pfnDelete) 
		return false;

	FX_SETCREATEFUNCTION pfnSetCreateFunction = (FX_SETCREATEFUNCTION)::GetProcAddress(m_hDLLInst, "fxSetCreateFunction");
	if( !pfnSetCreateFunction) 
		return false;

	FX_CREATEPROPLIST pfnCreatePropList = (FX_CREATEPROPLIST)::GetProcAddress(m_hDLLInst, "fxCreatePropList");
	if( !pfnCreatePropList) 
		return false;

	FX_FREEPROPLIST pfnFreePropList = (FX_FREEPROPLIST)::GetProcAddress(m_hDLLInst, "fxFreePropList");
	if( !pfnFreePropList) 
		return false;

	m_pfnSetPlayer			= pfnSetPlayer;
	m_pfnSetAppFocus		= pfnSetAppFocus;
	m_pfnDeleteFX			= pfnDelete;
	m_pfnSetCreateFunction	= pfnSetCreateFunction;
	m_pfnCreatePropList		= pfnCreatePropList;
	m_pfnFreePropList		= pfnFreePropList;

	// Okay, if we got here then this is a valid dll with some special
	// fx in it....

	m_nNumEffectTypes = pfnNum();
	
	//allocate our list of effect types
	m_pEffectTypes = debug_newa(FX_REF, m_nNumEffectTypes);

	if(!m_pEffectTypes)
	{
		m_nNumEffectTypes = 0;
	}
	else
	{
		for (uint32 nCurrEffect = 0; nCurrEffect < m_nNumEffectTypes; nCurrEffect ++)
		{
			// Retrieve the FX reference structure
			m_pEffectTypes[nCurrEffect] = pfnRef(nCurrEffect);
		}
	}

#endif

	// Success !!

	return true;
}

//unloads the effect DLL
void CClientFXDB::UnloadFxDll()
{

#ifdef WIN32

	if (!m_hDLLInst) 
		return;

	// Free the library
	::FreeLibrary(m_hDLLInst);

	if (strlen(sDLLTmpFile))
	{
		HMODULE hMod = ::GetModuleHandle(sDLLTmpFile);

		// if it is not still being used, delete the temp file
		if (!hMod)
		{
			::DeleteFile(sDLLTmpFile);
			sDLLTmpFile[0] = 0;
		}
	}
	
	//make sure to invalidate our hooks into the DLL
	m_hDLLInst				= NULL;
	m_pfnSetPlayer			= NULL;
	m_pfnSetAppFocus		= NULL;
	m_pfnDeleteFX			= NULL;
	m_pfnSetCreateFunction	= NULL;

#endif

}

//-----------------------------------------------------------------
// CClientFXDB file loading code
//-----------------------------------------------------------------

bool CClientFXDB::ReadFXProp( bool bText, ILTStream* pFxFile, FX_PROP& fxProp )
{
	if( bText )
	{
		// Read in the name
		ReadTextFile( pFxFile, "%s %s", fxProp.m_sName );
		
		// Read the type
		ReadTextFile( pFxFile, "%s %i", &fxProp.m_nType );

		// Read the data
		switch (fxProp.m_nType)
		{
			case FX_PROP::STRING  : ReadTextFile( pFxFile, "%s %s", fxProp.m_data.m_sVal ); break;
			case FX_PROP::INTEGER : ReadTextFile( pFxFile, "%s %i", &fxProp.m_data.m_nVal ); break;
			case FX_PROP::FLOAT   : ReadTextFile( pFxFile, "%s %f", &fxProp.m_data.m_fVal ); break;
			case FX_PROP::COMBO   : ReadTextFile( pFxFile, "%s %s", fxProp.m_data.m_sVal ); break;
			case FX_PROP::VECTOR  : ReadTextFile( pFxFile, "%s %f %f %f", &fxProp.m_data.m_fVec[0], &fxProp.m_data.m_fVec[1], &fxProp.m_data.m_fVec[2] ); break;
			case FX_PROP::VECTOR4 : ReadTextFile( pFxFile, "%s %f %f %f %f", &fxProp.m_data.m_fVec4[0], &fxProp.m_data.m_fVec4[1], &fxProp.m_data.m_fVec4[2], &fxProp.m_data.m_fVec4[3] ); break;
			case FX_PROP::CLRKEY  : 
				{
					LTFLOAT r, g, b, a;
					ReadTextFile( pFxFile, "%s %f %f %f %f %f", &fxProp.m_data.m_clrKey.m_tmKey, &r, &g, &b, &a );

					DWORD dwRed   = (int)(r * 255.0f);
					DWORD dwGreen = (int)(g * 255.0f);
					DWORD dwBlue  = (int)(b * 255.0f);
					DWORD dwAlpha = (int)(a * 255.0f);
					
					fxProp.m_data.m_clrKey.m_dwCol = dwRed | (dwGreen << 8) | (dwBlue << 16) | (dwAlpha << 24);
				}
				break;

			case FX_PROP::PATH	  : ReadTextFile( pFxFile, "%s %s", fxProp.m_data.m_sVal ); break;
		}							
	}
	else
	{
		BYTE nameLen;
		pFxFile->Read(&nameLen, 1);

		// Read in the name

		pFxFile->Read(&fxProp.m_sName, nameLen);

		// Read the type

		pFxFile->Read(&fxProp.m_nType, sizeof(FX_PROP::eDataType));

		// Read the data

		switch (fxProp.m_nType)
		{
			case FX_PROP::STRING  : pFxFile->Read(&fxProp.m_data.m_sVal, 128); break;
			case FX_PROP::INTEGER : pFxFile->Read(&fxProp.m_data.m_nVal, sizeof(int)); break;
			case FX_PROP::FLOAT   : pFxFile->Read(&fxProp.m_data.m_fVal, sizeof(float)); break;
			case FX_PROP::COMBO   : pFxFile->Read(&fxProp.m_data.m_sVal, 128); break;
			case FX_PROP::VECTOR  : pFxFile->Read(&fxProp.m_data.m_fVec, sizeof(float) * 3); break;
			case FX_PROP::VECTOR4 : pFxFile->Read(&fxProp.m_data.m_fVec4, sizeof(float) * 4); break;
			case FX_PROP::CLRKEY  : pFxFile->Read(&fxProp.m_data.m_clrKey, sizeof(FX_PROP::FX_CLRKEY) ); break;
			case FX_PROP::PATH	  : pFxFile->Read(&fxProp.m_data.m_sVal, 128); break;
		}							
	}

	return true;
}

bool CClientFXDB::ReadFXKey( bool bText, ILTStream* pFxFile, float fTotalTime, FX_KEY* pKey, FX_PROP* pPropBuffer, uint32 nBuffLen )
{
	// Read in the reference name
	char sTmp[128];
	if( bText )
	{
		ReadTextFile( pFxFile, "%s %s", sTmp );
	}
	else
	{
		pFxFile->Read(sTmp, 128);
	}

	pKey->m_pFxRef = FindFX( strtok(sTmp, ";" ));

	// Read in the key ID
	if( bText )
	{
		ReadTextFile( pFxFile, "%s %lu", &pKey->m_dwID );
	}
	else
	{
		pFxFile->Read(&pKey->m_dwID, sizeof(uint32));
	}

	// Read in the link status
	LINK_STATUS ls;
	if( bText )
	{
		ReadTextFile( pFxFile, "%s %i", &ls.m_bLinked );
		ReadTextFile( pFxFile, "%s %lu", &ls.m_dwLinkedID );

		//read in the linked node name but make sure that it is cleared out first
		ls.m_sLinkedNodeName[0] = '\0';
		ReadTextFile( pFxFile, "%s %s", ls.m_sLinkedNodeName );
	}
	else
	{
		pFxFile->Read(&ls, sizeof(LINK_STATUS));
	}
		
	pKey->m_bLinked = ls.m_bLinked;
	pKey->m_dwLinkedID = ls.m_dwLinkedID;
	strcpy(pKey->m_sLinkedNodeName, ls.m_sLinkedNodeName);

	//check to make sure that the key is not motion linked to itself though
	if(pKey->m_bLinked && (pKey->m_dwLinkedID == pKey->m_dwID))
	{
		pKey->m_bLinked = false;
	}

	// Read in the start time
	if( bText )
	{
		ReadTextFile( pFxFile, "%s %f", &pKey->m_tmStart );
	}
	else
	{
		pFxFile->Read(&pKey->m_tmStart, sizeof(float));
	}

	// Read in the end time
	if( bText )
	{
		ReadTextFile( pFxFile, "%s %f", &pKey->m_tmEnd );
	}
	else
	{
		pFxFile->Read(&pKey->m_tmEnd, sizeof(float));
	}

	// Read in the key repeat
	uint32 nKeyRepeats = 0;
	if( bText )
	{
		ReadTextFile( pFxFile, "%s %lu", &nKeyRepeats );
	}
	else
	{
		pFxFile->Read(&nKeyRepeats, sizeof(uint32));
	}

	
	// Read in dummy values
	uint32	dwDummy;
	LTFLOAT	fDummy;
	if( bText )
	{
		ReadTextFile( pFxFile, "%s %lu", &dwDummy );
		ReadTextFile( pFxFile, "%s %f", &fDummy );
		ReadTextFile( pFxFile, "%s %f", &fDummy );
	}
	else
	{
		pFxFile->Read(&dwDummy, sizeof(uint32));
		pFxFile->Read(&dwDummy, sizeof(uint32));
		pFxFile->Read(&dwDummy, sizeof(uint32));
	}
	
	// Read in the number of properties
	uint32 dwNumProps;
	if( bText )
	{
		ReadTextFile( pFxFile, "%s %lu", &dwNumProps );
	}
	else
	{
		pFxFile->Read(&dwNumProps, sizeof(uint32));
	}
	
	uint32 k;
	for (k = 0; k < dwNumProps; k ++)
	{
		if(k >= nBuffLen)
		{
			assert(!"Error: Found a key with too many properties, truncating additional properties");
			break;
		}
		else
		{
			ReadFXProp( bText, pFxFile, pPropBuffer[k] );
		}	
	}

	//ok, we can now convert our properties over to the appropriate form
	int32 nFXID = FindFXID(pKey->m_pFxRef->m_sName);

	if(nFXID < 0)
		return false;

	//alright, get our property object
	pKey->m_pProps = m_pfnCreatePropList(nFXID);

	if(!pKey->m_pProps)
		return false;

	//make sure to clamp the times to always be between 0 and the length of the group,
	//otherwise numerical accuracy problems will arise
	pKey->m_tmStart = LTCLAMP(pKey->m_tmStart, 0.0f, fTotalTime);
	pKey->m_tmEnd   = LTCLAMP(pKey->m_tmEnd, 0.0f, fTotalTime);	

	//now setup the lifespan of the key
	pKey->m_pProps->SetLifetime(pKey->m_tmEnd - pKey->m_tmStart, nKeyRepeats);

	
	//and let it convert its properties
	if(!pKey->m_pProps->ParseProperties(pPropBuffer, k))
	{
		m_pfnFreePropList(pKey->m_pProps);
		pKey->m_pProps = NULL;
		return false;
	}
	
	//setup this key (read out our keys and base properties)
	SetupKey(pKey, pPropBuffer, k);

	return true;
}

bool CClientFXDB::ReadFXGroup( bool bText, ILTStream* pFxFile, FX_GROUP* pFxGroup, FX_PROP* pPropBuffer, uint32 nBuffLen )
{
	assert(pFxGroup);

	//make sure to clear out any data already in the effect group
	pFxGroup->Term();

	// Read in the number of FX in this group
	uint32 dwNumFx		= 0;
	uint32 dwPhaseLen	= 0;
	
	if( bText )
	{
		// Read in the name of this FX group
		ReadTextFile( pFxFile, "%s %s", pFxGroup->m_sName );
		
		ReadTextFile( pFxFile, "%s %lu", &dwNumFx );
		
		// Read in the phase length
		ReadTextFile( pFxFile, "%s %lu", &dwPhaseLen );
	}
	else
	{
		pFxFile->Read(&dwNumFx, sizeof(uint32));
		
		// Read in the name of this FX group
		pFxFile->Read(pFxGroup->m_sName, 128);

		// Read in the phase length
		pFxFile->Read(&dwPhaseLen, sizeof(dwPhaseLen));
	}

	// Initialize total time to zero, then find the total time
	// as we read in the keys.
	pFxGroup->m_tmTotalTime = dwPhaseLen / 1000.0f;

	//allocate room for the FX
	pFxGroup->m_pKeys = debug_newa(FX_KEY, dwNumFx);

	if(!pFxGroup->m_pKeys)
		return false;

	//save the number of keys
	pFxGroup->m_nNumKeys = dwNumFx;
	
	// Read in the FXKey
	for( uint32 nCurrEffect = 0; nCurrEffect < dwNumFx; nCurrEffect ++ )
	{
		ReadFXKey( bText, pFxFile, pFxGroup->m_tmTotalTime, &pFxGroup->m_pKeys[nCurrEffect], pPropBuffer, nBuffLen );
	}

	//we need to sort the effects based upon the order that they need to be created in. The creation
	//order needs to have any effects that are motion linked come last so that they can have the
	//effects that they are linked to created before them

	//This sort is a sort of bubble sort where it will go through and if it finds any effects
	//that violate this property, it will correct them. It needs to take care to properly handle
	//cyclic graphs though, which are invalid
	for(uint32 nCurrPass = 0; nCurrPass < dwNumFx; nCurrPass++)
	{
		bool bDoneSorting = true;

		//take a pass through to sort...
		for(uint32 nCurrKey = 0; nCurrKey < dwNumFx; nCurrKey++)
		{
			//see if this effect is motion linked, if not we don't need to worry
			if(!pFxGroup->m_pKeys[nCurrKey].m_bLinked)
				continue;

			//get the ID of the node we are linked to
			uint32 nLinkID = pFxGroup->m_pKeys[nCurrKey].m_dwLinkedID;

			//it is linked, we need to see if the effect that it is linked to comes after
			for(uint32 nCurrLink = nCurrKey + 1; nCurrLink < dwNumFx; nCurrLink++)
			{
				if(pFxGroup->m_pKeys[nCurrLink].m_dwID == nLinkID)
				{
					//this is the effect and it does come after, so we need to swap it
					FX_KEY TempKey = pFxGroup->m_pKeys[nCurrKey];
					pFxGroup->m_pKeys[nCurrKey] = pFxGroup->m_pKeys[nCurrLink];
					pFxGroup->m_pKeys[nCurrLink] = TempKey;

					//we have to take another pass again just to make sure
					bDoneSorting = false;

					break;
				}
			}
		}

		//see if we are done sorting
		if(bDoneSorting)
			break;
	}

	//alright, by this time either we are in order, or there is a cyclic dependency



	return true;
}


bool CClientFXDB::ReadFXGroups( bool bText, ILTStream* pFxFile, CLinkList<FX_GROUP *> &collGroupFx )
{
	// Read in the number of FX groups in this file
	uint32 dwNumGroups;

	if( bText )
	{
		ReadTextFile( pFxFile, "%s %lu", &dwNumGroups );
	}
	else
	{
		pFxFile->Read(&dwNumGroups, sizeof(uint32));
	}

	//allocate a working buffer that keys can read properties into
	static const uint32		knMaxKeyProps = 512;
	FX_PROP*				pPropBuffer = debug_newa(FX_PROP, knMaxKeyProps);

	if(!pPropBuffer)
		return false;

	for( uint32 i = 0; i < dwNumGroups; i ++ )
	{
		// Create a new group.
		FX_GROUP *pFxGroup = debug_new( FX_GROUP );

		if( !ReadFXGroup( bText, pFxFile, pFxGroup, pPropBuffer, knMaxKeyProps ))
		{
			debug_deletea(pPropBuffer);
			return false;
		}

		collGroupFx.AddTail(pFxGroup);
	}

	//free our working buffer
	debug_deletea(pPropBuffer);

	return true;
}

bool CClientFXDB::LoadFxGroups(ILTClient* pClient, const char *sFileName )
{
	ILTStream *pFxFile;

	// Attempt to open the client fx file
	pClient->OpenFile(sFileName, &pFxFile);

	if(!pFxFile)
		return false;

	char szTag[MAX_TAG_SIZE] = {0};

	//remember where we are in our list of effects, so that we don't reinitalize keys that are already
	//in the list
	CLinkListNode<FX_GROUP *> *pTailNode = m_collGroupFX.GetTail();

	// Figure out if we are reading a binary file or text...
	pFxFile->Read( szTag, 7 );
	pFxFile->SeekTo( 0 );

	// This is a text file if we can read an asci "Groups:".
	bool bText = !_stricmp( szTag, "Groups:" );
	ReadFXGroups( bText, pFxFile, m_collGroupFX );

	//clean up the file
	pFxFile->Release();
	pFxFile = NULL;


	// Run through the FX groups we added to the end of the list and setup any non-instance specific
	// information
	CLinkListNode<FX_GROUP *> *pFxGroupNode = (pTailNode) ? pTailNode : m_collGroupFX.GetHead();

	while (pFxGroupNode)
	{
		uint32 nNumKeys = pFxGroupNode->m_Data->m_nNumKeys;

		for(uint32 nCurrKey = 0; nCurrKey < nNumKeys; nCurrKey++)
		{
			FX_KEY *pKey = &pFxGroupNode->m_Data->m_pKeys[nCurrKey];

			float tmLength = pKey->m_tmEnd - pKey->m_tmStart;

			if (tmLength >= pFxGroupNode->m_Data->m_tmTotalTime - 0.01f)
			{
				pKey->m_bContinualLoop = true;
			}
			else
			{
				pKey->m_bContinualLoop = false;
			}
		}

		pFxGroupNode = pFxGroupNode->m_pNext;
	}

	// Success !!
	return true;
}


//------------------------------------------------------------------
// CClientFXDB Group finding utilities
//------------------------------------------------------------------

FX_GROUP* CClientFXDB::FindGroupFX(const char *sName)
{
	if( !sName[0] ) return LTNULL;

	// Locate the group

	CLinkListNode<FX_GROUP *> *pGroupNode = m_collGroupFX.GetHead();

	while (pGroupNode)
	{
		if (!stricmp(pGroupNode->m_Data->m_sName, sName))
		{
			// This is the one we want

			return pGroupNode->m_Data;
		}

		pGroupNode = pGroupNode->m_pNext;
	}

	// Failure....

	return LTNULL;
}

//Finds an effect of the appropraite type
FX_REF* CClientFXDB::FindFX(const char *sName)
{
	for(uint32 nCurrEffect = 0; nCurrEffect < m_nNumEffectTypes; nCurrEffect++)
	{
		if (!stricmp(sName, m_pEffectTypes[nCurrEffect].m_sName)) 
			return &m_pEffectTypes[nCurrEffect];
	}

	// Failure !!
	return NULL;
}

int32 CClientFXDB::FindFXID(const char *sName)
{
	for(uint32 nCurrEffect = 0; nCurrEffect < m_nNumEffectTypes; nCurrEffect++)
	{
		if (!stricmp(sName, m_pEffectTypes[nCurrEffect].m_sName)) 
			return nCurrEffect;
	}

	// Failure !!
	return -1;
}


//------------------------------------------------------------------
// CClientFXDB External interface for DLL hooks
//------------------------------------------------------------------

//called to delete an effect
void CClientFXDB::DeleteEffect(CBaseFX* pFx)
{
	if(m_pfnDeleteFX)
	{
		m_pfnDeleteFX(pFx);
	}
}

//sets up the parameters for the effect
void CClientFXDB::SetAppFocus(bool bAppFocus)
{
	if(m_pfnSetAppFocus)
	{
		m_pfnSetAppFocus(bAppFocus);
	}
}

//sets the player object
void CClientFXDB::SetPlayer(HOBJECT hObj)
{
	if(m_pfnSetPlayer)
	{
		m_pfnSetPlayer(hObj);
	}
}

//sets a callback function and user data for 
void CClientFXDB::SetCreateCallback(TCreateClientFXFn pFn, void* pUserData)
{
	if(m_pfnSetCreateFunction)
	{
		m_pfnSetCreateFunction(pFn, pUserData);
	}
}

//------------------------------------------------------------------
// CClientFXDB Singleton support
//------------------------------------------------------------------
CClientFXDB& CClientFXDB::GetSingleton()
{
	static CClientFXDB s_ClientFXDB;
	return s_ClientFXDB;
}

