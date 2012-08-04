// ----------------------------------------------------------------------- //
//
// MODULE  : MissionDB.cpp
//
// PURPOSE : Implementation of Mission database
//
// CREATED : 02/03/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "Stdafx.h"
#include "MissionDB.h"
#include "WinUtil.h"
#include "ltfileoperations.h"
#include "ltprofileutils.h"
#include "ltstrutils.h"
#include "iltfilemgr.h"
#include "ltoutnullconverter.h"
#include "sys/win/mpstrconv.h"
#include "StringUtilities.h"
#include "GameModeMgr.h"
#include "ltfileread.h"
#include "ltfilewrite.h"

#if defined(PLATFORM_LINUX)
#include <unistd.h>
#else
#include <direct.h>
#endif

//
// Defines...
//
const char* const MDB_World =			    "World";

const char* const MDB_MissionCat =		    "Missions/Missions";
const char* const MDB_LevelCat =		    "Missions/Levels";
const char* const MDB_MissionOrderCat =	    "Missions/MissionOrder";
const char* const MDB_MissionSharedCat =    "Missions/Shared";

const char* const MDB_MissionSharedRecord = "Shared";
const char* const MDB_SinglePlayerWorld =   "SinglePlayerWorlds";
const char* const MDB_MultiPlayerWorld =    "MultiPlayerWorlds";


	
//
// Globals...
//

CMissionDB* g_pMissionDB = NULL;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionDB::CMissionDB()
//
//	PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CMissionDB::CMissionDB()
{
	Reset();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionDB::~CMissionDB()
//
//	PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CMissionDB::~CMissionDB()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionDB::Reset()
//
//	PURPOSE:	Resets the state of CMissionDB.
//
// ----------------------------------------------------------------------- //

void CMissionDB::Reset()
{
	// close the database if it was previously opened
	if (m_hDatabase)
	{
		g_pLTDatabase->ReleaseDatabase(m_hDatabase);
		m_hDatabase = NULL;
	}

	// reset the internal handles
	m_hMissionCat = NULL;
	m_hLevelCat = NULL;
	m_hMissionOrder = NULL;
	m_hMissionSharedRecord = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionDB::Init()
//
//	PURPOSE:	Initialize the database...
//
// ----------------------------------------------------------------------- //
bool CMissionDB::Init( const char *szDatabaseFile /* = DB_Default_File  */ )
{
	bool bInRez = ( LTStrICmp(szDatabaseFile,DB_Default_File) == 0  );
	if( !OpenDatabase( szDatabaseFile, bInRez ))
		return false;

	// Set the global database pointer...
	g_pMissionDB = this;

	// Get handles to all of the categories in the database...
	m_hMissionCat = g_pLTDatabase->GetCategory(m_hDatabase,MDB_MissionCat);
	m_hLevelCat = g_pLTDatabase->GetCategory(m_hDatabase,MDB_LevelCat);

	// Get the missionorder record to use.
	char const* pszMissionOrder = GetConsoleString( "MissionOrder", MDB_MissionOrderDefault );
	m_hMissionOrder = g_pLTDatabase->GetRecord(m_hDatabase,MDB_MissionOrderCat,pszMissionOrder);

	m_hMissionSharedRecord = g_pLTDatabase->GetRecord(m_hDatabase, MDB_MissionSharedCat, MDB_MissionSharedRecord);

	return true;
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CMissionDB::IsUsingDB()
//	PURPOSE:	Determine if a particular DB was opened
// ----------------------------------------------------------------------- //
bool CMissionDB::IsUsingDB(const char *szDatabaseFile)
{
	return (LTStrICmp(m_sDatabaseFile.c_str(),szDatabaseFile) == 0);
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CMissionDB::GetNumMissions()
//	PURPOSE:	Get the number of missions in the DB
// ----------------------------------------------------------------------- //
uint32 CMissionDB::GetNumMissions() const
{
	return g_pLTDatabase->GetNumRecords( m_hMissionCat );
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CMissionDB::GetMission()
//	PURPOSE:	Get a mission record from an index
// ----------------------------------------------------------------------- //
HRECORD CMissionDB::GetMission(uint32 nMissionId) const
{
	return g_pLTDatabase->GetRecordByIndex( m_hMissionCat, nMissionId );
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CMissionDB::GetLevel()
//	PURPOSE:	Get a level record from mission and level indices
// ----------------------------------------------------------------------- //
HRECORD CMissionDB::GetLevel( uint32 nMissionId, uint32 nLevelId ) const
{
	HRECORD hMissionRec = GetMission( nMissionId );
	return GetRecordLink(hMissionRec,MDB_Levels,nLevelId);
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CMissionDB::GetLevel()
//	PURPOSE:	Get a level record from a mission record and a level index
// ----------------------------------------------------------------------- //
HRECORD CMissionDB::GetLevel( HRECORD hMission, uint32 nLevelId ) const
{
	return GetRecordLink(hMission,MDB_Levels,nLevelId);
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CMissionDB::GetWorldName()
//	PURPOSE:	Get the name of the world for a given level, with or without
//					the path info
// ----------------------------------------------------------------------- //
const char* CMissionDB::GetWorldName(HRECORD hLevel, bool bIncludePath ) const
{
	static char szPath[MAX_PATH] = "";
	static char szFile[MAX_PATH] = "";
	LTFileOperations::SplitPath( g_pMissionDB->GetString(hLevel,MDB_World), szPath, szFile, NULL );

#if defined(PLATFORM_LINUX)
	// SplitPath will return a Linux formatted file path, but since this may not actually be
	// a filesystem path we need to convert it use the archive-compatible separators
	for (uint32 nIndex = 0; nIndex < LTStrLen(szPath); ++nIndex)
	{
		if (szPath[nIndex] == '/')
		{
			szPath[nIndex] = '\\';
		}
	}
#endif // PLATFORM_LINUX
	
	if (bIncludePath)
	{
		LTStrCat(szPath,szFile,LTARRAYSIZE(szPath));
		return szPath;
	}
	else
	{
		return szFile;
	}

}
// ----------------------------------------------------------------------- //
//	ROUTINE:	CMissionDB::IsMissionLevel()
//	PURPOSE:	Is the specified world part of a missionorder?
// ----------------------------------------------------------------------- //
bool CMissionDB::IsMissionLevel( char const* pWorldFile, uint32 & nMissionId, uint32 & nLevel) const
{
	std::string sWorldName = pWorldFile;
	sWorldName += ".";
	sWorldName += RESEXT_WORLD_PACKED;

	// Get the missions attribute for our missionorder record.
	HATTRIBUTE hMissions = g_pMissionDB->GetAttribute( GetMissionOrderRec(), MDB_Missions );
	uint32 nNumMissionsInMissionOrder = g_pLTDatabase->GetNumValues( hMissions );
	for( uint32 nMissionOrderIndex = 0; nMissionOrderIndex < nNumMissionsInMissionOrder; nMissionOrderIndex++ )
	{
		// Get the mission in our missionorder.
		HRECORD hMission = g_pLTDatabase->GetRecordLink( hMissions, nMissionOrderIndex, NULL );
		if( !hMissions )
			continue;

		uint32 numLevels = GetNumValues(hMission,MDB_Levels);
		for (uint32 nL = 0; nL < numLevels; nL++)
		{
			HRECORD hLevel = GetRecordLink(hMission,MDB_Levels,nL);
			char szMissionWorldName[MAX_PATH];
			LTStrCpy(szMissionWorldName, GetString(hLevel, MDB_World), MAX_PATH);

			if (LTStrICmp(sWorldName.c_str(), szMissionWorldName) == 0)
			{
				nMissionId = g_pLTDatabase->GetRecordIndex( hMission );
				nLevel = nL;
				return true;
			}
		}
	}

	return false;
}

void CMissionDB::GetMissionDisplayName( HRECORD hMission, HRECORD hLevel, wchar_t* pwszMissionDisplayName, uint32 nLen ) const
{
	// First try to use the stringid version.
	const char* szNameId = GetString(hMission,MDB_Name);
	LTStrCpy( pwszMissionDisplayName, LoadString(szNameId), nLen );
	if( !LTStrEmpty( pwszMissionDisplayName ))
	{
		return;
	}
	// If stringid isn't available, see if they have any raw string defined.
	else
	{
		LTStrCpy( pwszMissionDisplayName, g_pMissionDB->GetWString(hMission,MDB_NameStr), nLen );
		if( !LTStrEmpty( pwszMissionDisplayName ))
		{
			return;
		}
	}

	// If no raw string, just use the world name.
	LTStrCpy( pwszMissionDisplayName, MPA2W(g_pMissionDB->GetWorldName(hLevel,false)).c_str(), nLen );
	
}

void CMissionDB::GetMissionDisplayName( const char* pWorldFile, wchar_t* pwszMissionDisplayName, uint32 nLen ) const
{
	uint32 nMission = (uint32)-1;
	uint32 nLevel = (uint32)-1;

	if (g_pMissionDB->IsMissionLevel(pWorldFile,nMission,nLevel))
	{
		HRECORD	hMission = g_pMissionDB->GetMission(nMission);
		HRECORD	hLevel = g_pMissionDB->GetLevel(hMission,nLevel);
		GetMissionDisplayName(hMission,hLevel,pwszMissionDisplayName,nLen);

	}
	else
	{
		char szName[MAX_PATH*2] = "";
		LTFileOperations::SplitPath(pWorldFile,NULL,szName,NULL);

		LTStrCpy(pwszMissionDisplayName, MPA2W(szName).c_str(),nLen);
	}
}



// ----------------------------------------------------------------------- //
//	ROUTINE:	CMissionDB::CreateMPDB()
//	PURPOSE:	Create a Mission DB for MP games at runtime
// ----------------------------------------------------------------------- //
bool CMissionDB::CreateMPDB()
{
#if defined(PLATFORM_XENON)
	// XENON: This code will not work on Xenon, since it's writing to the hard drive
	return false;
#else // !PLATFORM_XENON

	char pszAbsolute[MAX_PATH];
	g_pLTBase->FileMgr()->GetAbsoluteUserFileName(MDB_MP_File, pszAbsolute, LTARRAYSIZE(pszAbsolute));

	if (CWinUtil::FileExist(pszAbsolute))
	{
		remove(pszAbsolute);
	}

	StringSet filenames;

	// Get a list of world names and sort them alphabetically

	uint8 nNumPaths = GetNumValues(m_hMissionSharedRecord, MDB_MultiPlayerWorld);

	char pathBuf[128];
	FileEntry** pFilesArray = debug_newa(FileEntry*, nNumPaths);

	if (pFilesArray)
	{
		for (int i=0; i < nNumPaths; ++i)
		{
			pathBuf[0] = '\0';
			
			LTStrCpy(pathBuf, GetString(m_hMissionSharedRecord, MDB_MultiPlayerWorld, i), LTARRAYSIZE(pathBuf));
	
			if (pathBuf[0])
			{
				pFilesArray[i] = g_pLTBase->FileMgr()->GetFileList(pathBuf);
			}
			else
			{
				pFilesArray[i] = NULL;
			}
		}
	}


	char strBaseName[256];
	char* pBaseName = NULL;
	char* pBaseExt = NULL;

	for (int i=0; i < nNumPaths; ++i)
	{
		pathBuf[0] = '\0';
		LTStrCpy(pathBuf, GetString(m_hMissionSharedRecord, MDB_MultiPlayerWorld, i), LTARRAYSIZE(pathBuf));

		if (pathBuf[0] && pFilesArray[i])
		{
			char path[MAX_PATH];
			LTSNPrintF( path, LTARRAYSIZE( path ), "%s\\", pathBuf);
			FileEntry* ptr = pFilesArray[i];

			while (ptr)
			{
				if (ptr->m_Type == eLTFileEntryType_File)
				{
					LTStrCpy(strBaseName, ptr->m_pBaseFilename, LTARRAYSIZE(strBaseName));
					pBaseName = strtok (strBaseName, ".");
					pBaseExt = strtok (NULL, "\0");
					if (pBaseExt && LTStrICmp (pBaseExt, RESEXT_WORLD_PACKED) == 0)
					{
						char szString[512];
						LTSNPrintF( szString, LTARRAYSIZE( szString ), "%s%s", path, pBaseName);

						// add this to the array
						filenames.insert(szString);
					}
				}

				ptr = ptr->m_pNext;
			}

			g_pLTBase->FileMgr()->FreeFileList(pFilesArray[i]);
		}
	}

	debug_deletea(pFilesArray);

	int index = 0;
	char szLabel[256];

	HDATABASECREATOR hDBC = g_pLTDatabaseCreator->CreateNewDatabase();
	if (!hDBC) return false; //assert?

	//create our categories
	HCATEGORYCREATOR hMissionCat = g_pLTDatabaseCreator->CreateCategory(hDBC,MDB_MissionCat,"");
	HCATEGORYCREATOR hLevelCat = g_pLTDatabaseCreator->CreateCategory(hDBC,MDB_LevelCat,"");
	HCATEGORYCREATOR hMissionOrderCat = g_pLTDatabaseCreator->CreateCategory(hDBC,MDB_MissionOrderCat,"");


	typedef std::vector<HRECORDCREATOR, LTAllocator<HRECORDCREATOR, LT_MEM_TYPE_GAMECODE> > HRecordCreatorArray;
	HRecordCreatorArray	vecMissions;
	HRecordCreatorArray	vecLevels;

	//Create the records first. All of the records must be created before any RecordLink attributes can be filled.
	HRECORDCREATOR hMO = g_pLTDatabaseCreator->CreateRecord(hMissionOrderCat,MDB_MissionOrderDefault);
	StringSet::iterator iter = filenames.begin();
	while (iter != filenames.end())
	{
		//create the level and mission label.
		LTStrCpy( szLabel, ( *iter ).c_str( ), LTARRAYSIZE( szLabel ));

		// Create the level record.
		HRECORDCREATOR hLevel = g_pLTDatabaseCreator->CreateRecord(hLevelCat,szLabel);
		vecLevels.push_back(hLevel);

		//create the mission record
		HRECORDCREATOR hMission = g_pLTDatabaseCreator->CreateRecord(hMissionCat,szLabel);
		vecMissions.push_back(hMission);

		++index;
		iter++;
	}

	//create and fill the attributes
	HATTRIBUTECREATOR hMOMissions = g_pLTDatabaseCreator->CreateAttribute(hMO,MDB_Missions,eAttributeType_RecordLink,eAttributeUsage_Default,filenames.size());
	iter = filenames.begin();
	index = 0;
	while (iter != filenames.end())
	{

		std::string sWorldName = (*iter);
		
		sWorldName += ".";
		sWorldName += RESEXT_WORLD_PACKED;

		//find the ini file for the map
		MultiplayerMissionIni sIniFile(sWorldName.c_str());

		//get the mission and level record for this file
		HRECORDCREATOR hMission = vecMissions[index];
		HRECORDCREATOR hLevel = vecLevels[index];

		//add the mission to the mission order record
		bool bSuccess = g_pLTDatabaseCreator->SetRecordLink(hMOMissions,index,hMission);
		LTASSERT(bSuccess,"CMissionDB::CreateMPDB() - Failed to set Mission record link");

		//create attributes for mission record
		HATTRIBUTECREATOR hMissionNameId = g_pLTDatabaseCreator->CreateAttribute(hMission,MDB_Name,eAttributeType_String,eAttributeUsage_Default,1);
		g_pLTDatabaseCreator->SetString(hMissionNameId,0,"");

		wchar_t wszTmp[256] = L"";
		sIniFile.GetName(wszTmp,LTARRAYSIZE(wszTmp));
		HATTRIBUTECREATOR hMissionNameStr = g_pLTDatabaseCreator->CreateAttribute(hMission,MDB_NameStr,eAttributeType_WString,eAttributeUsage_Default,1);
		g_pLTDatabaseCreator->SetWString(hMissionNameStr,0,wszTmp);

		HATTRIBUTECREATOR hMissionLevels = g_pLTDatabaseCreator->CreateAttribute(hMission,MDB_Levels,eAttributeType_RecordLink,eAttributeUsage_Default,1);
		//set the link to the correct Level record
		bSuccess = g_pLTDatabaseCreator->SetRecordLink(hMissionLevels,0,hLevel);
		LTASSERT(bSuccess,"CMissionDB::CreateMPDB() - Failed to set Level record link");

		//create dummy attributes for entries found in the schema, but not needed for MP
		HATTRIBUTECREATOR hTmpAtt = g_pLTDatabaseCreator->CreateAttribute(hMission,MDB_DefaultWeapons,eAttributeType_RecordLink,eAttributeUsage_Default,1);
		g_pLTDatabaseCreator->SetRecordLink(hTmpAtt,0,NULL);

		hTmpAtt = g_pLTDatabaseCreator->CreateAttribute(hMission,MDB_SelectedWeapon,eAttributeType_RecordLink,eAttributeUsage_Default,1);
		g_pLTDatabaseCreator->SetRecordLink(hTmpAtt,0,NULL);

		hTmpAtt = g_pLTDatabaseCreator->CreateAttribute(hMission,MDB_DefaultMods,eAttributeType_RecordLink,eAttributeUsage_Default,1);
		g_pLTDatabaseCreator->SetRecordLink(hTmpAtt,0,NULL);

		hTmpAtt = g_pLTDatabaseCreator->CreateAttribute(hMission,MDB_Layout,eAttributeType_RecordLink,eAttributeUsage_Default,1);
		g_pLTDatabaseCreator->SetRecordLink(hTmpAtt,0,NULL);

		hTmpAtt = g_pLTDatabaseCreator->CreateAttribute(hMission,MDB_BriefingLayout,eAttributeType_RecordLink,eAttributeUsage_Default,1);
		g_pLTDatabaseCreator->SetRecordLink(hTmpAtt,0,NULL);

		hTmpAtt = g_pLTDatabaseCreator->CreateAttribute(hMission,MDB_ResetPlayer,eAttributeType_Bool,eAttributeUsage_Default,1);
		g_pLTDatabaseCreator->SetBool(hTmpAtt,0,true);


		//create attributes for the level record
		HATTRIBUTECREATOR hLevelWorld = g_pLTDatabaseCreator->CreateAttribute(hLevel,MDB_World,eAttributeType_String,eAttributeUsage_Default,1);
		bSuccess = g_pLTDatabaseCreator->SetString(hLevelWorld,0,sWorldName.c_str());
		LTASSERT(bSuccess,"CMissionDB::CreateMPDB() - Failed to set WorldName");


		char szPhoto[MAX_PATH] = "";
		sIniFile.GetPhoto(szPhoto,LTARRAYSIZE(szPhoto));
		HATTRIBUTECREATOR hLevelPhoto = g_pLTDatabaseCreator->CreateAttribute(hLevel,MDB_Photo,eAttributeType_String,eAttributeUsage_Default,1);
		g_pLTDatabaseCreator->SetString(hLevelPhoto,0,szPhoto);


		hTmpAtt = g_pLTDatabaseCreator->CreateAttribute(hLevel,MDB_MinPlayers,eAttributeType_Int32,eAttributeUsage_Default,1);
		g_pLTDatabaseCreator->SetInt32(hTmpAtt,0,sIniFile.GetMinPlayers());

		hTmpAtt = g_pLTDatabaseCreator->CreateAttribute(hLevel,MDB_MaxPlayers,eAttributeType_Int32,eAttributeUsage_Default,1);
		g_pLTDatabaseCreator->SetInt32(hTmpAtt,0,sIniFile.GetMaxPlayers());

		char szTmp[128] = "";
		StringArray featureSets;
		
		uint8 nCount = 0;
		while (1)
		{
			sIniFile.GetRequiredFeatures(szTmp,LTARRAYSIZE(szTmp),nCount);
			if (LTStrEmpty(szTmp))
			{
				break;
			}
			featureSets.push_back(szTmp);
			++nCount;
		}
		
		hTmpAtt = g_pLTDatabaseCreator->CreateAttribute(hLevel,MDB_RequiredFeatures,eAttributeType_String,eAttributeUsage_Default,nCount);
		for (uint8 nIndex = 0;nIndex < nCount; ++nIndex )
		{
			g_pLTDatabaseCreator->SetString(hTmpAtt,nIndex,featureSets[nIndex].c_str());
		}

		sIniFile.GetSupportedFeatures(szTmp,LTARRAYSIZE(szTmp));
		hTmpAtt = g_pLTDatabaseCreator->CreateAttribute(hLevel,MDB_SupportedFeatures,eAttributeType_String,eAttributeUsage_Default,1);
		g_pLTDatabaseCreator->SetString(hTmpAtt,0,szTmp);

		//create dummy attributes for entries found in the schema, but not needed for MP
		hTmpAtt = g_pLTDatabaseCreator->CreateAttribute(hLevel,MDB_Name,eAttributeType_String,eAttributeUsage_Default,1);
		g_pLTDatabaseCreator->SetString(hTmpAtt,0,"");

		hTmpAtt = g_pLTDatabaseCreator->CreateAttribute(hLevel,MDB_Briefing,eAttributeType_String,eAttributeUsage_Default,1);
		g_pLTDatabaseCreator->SetString(hTmpAtt,0,"");

		hTmpAtt = g_pLTDatabaseCreator->CreateAttribute(hLevel,MDB_Help,eAttributeType_String,eAttributeUsage_Default,1);
		g_pLTDatabaseCreator->SetString(hTmpAtt,0,"");

		hTmpAtt = g_pLTDatabaseCreator->CreateAttribute(hLevel,MDB_DefaultWeapons,eAttributeType_RecordLink,eAttributeUsage_Default,1);
		g_pLTDatabaseCreator->SetRecordLink(hTmpAtt,0,NULL);

		hTmpAtt = g_pLTDatabaseCreator->CreateAttribute(hLevel,MDB_DefaultMods,eAttributeType_RecordLink,eAttributeUsage_Default,1);
		g_pLTDatabaseCreator->SetRecordLink(hTmpAtt,0,NULL);

		++index;
		iter++;
	}

	filenames.clear();

	ILTOutStream* pOutFile = g_pLTBase->FileMgr()->OpenUserFileForWriting(MDB_MP_File);
	if(pOutFile)
	{
		LTOutNullConverter OutConverter(*pOutFile);
		g_pLTDatabaseCreator->SaveDatabase(hDBC,OutConverter);
		g_pLTDatabaseCreator->ReleaseDatabase(hDBC);
	}

	return true;

#endif // !PLATFORM_XENON
}

bool CMissionDB::CheckMPLevelRequirements(HRECORD hMission,StringSet& setRequiredMapFeatures) const
{
#ifdef _CLIENTBUILD
	if (!IsMultiplayerGameClient())
#elif _SERVERBUILD
	if (!IsMultiplayerGameServer())
#endif
	{
		return true;
	}


	HRECORD	hLevel = GetLevel(hMission,0);

	//get the list of features that this map requires
	StringSet setRequiredFeatures;
	StringSet setSupportedFeatures;

	//step through each of the required feature sets supported by the map
	uint32 nNumFeatureSets = g_pMissionDB->GetNumValues(hLevel,MDB_RequiredFeatures);
	bool bFoundSupportedSet = true;
	for (uint32 n = 0; n < nNumFeatureSets;++n)
	{
		const char* szRequiredFeatures = g_pMissionDB->GetString(hLevel,MDB_RequiredFeatures,n);
		DelimitedStringToStringContainer(szRequiredFeatures,setRequiredFeatures,",");

		//assume it's going to work
		bFoundSupportedSet = true;
		//for each feature, check to see if the game mode uses it
		StringSet::iterator iterRF = setRequiredFeatures.begin();
		while (iterRF != setRequiredFeatures.end() && bFoundSupportedSet)
		{
			StringSet::iterator iterModeFeature = setRequiredMapFeatures.find( iterRF->c_str() );

			//if the mode doesn't support this feature skip the map
			if (iterModeFeature == setRequiredMapFeatures.end())
			{
				bFoundSupportedSet = false;
			}
			iterRF++;
		}

		//if we've gotten this far, we've found a set that works
		if (bFoundSupportedSet)
		{
			break;
		}
	}

	//we didn't find any supported sets, so we can't use this map
	if (!bFoundSupportedSet)
	{
		return false;
	}

	//get the list of features that this map supports
	setSupportedFeatures.clear();
	const char* szSupportedFeatures = g_pMissionDB->GetString(hLevel,MDB_SupportedFeatures);
	DelimitedStringToStringContainer(szSupportedFeatures,setSupportedFeatures,",");

	StringSet::iterator iterRMF = setRequiredMapFeatures.begin();
	while (iterRMF != setRequiredMapFeatures.end())
	{
		StringSet::iterator iterRF = setRequiredFeatures.find( iterRMF->c_str() );
		StringSet::iterator iterSF = setSupportedFeatures.find( iterRMF->c_str() );

		if (iterRF == setRequiredFeatures.end() && iterSF == setSupportedFeatures.end())
		{
			return false;
		}
		iterRMF++;
	}
	
	return true;
}

MultiplayerMissionIni::MultiplayerMissionIni(const char* szWorldName)
{
	char szPath[MAX_PATH*2] = "";
	char szName[MAX_PATH*2] = "";
	LTFileOperations::SplitPath(szWorldName,szPath,szName,NULL);

	m_sPath = szPath;
	std::string sIniName = szPath;

	sIniName += szName;
	sIniName += ".ini";

#if defined(PLATFORM_LINUX)
	// make the separators filesystem compatible
	for (uint32 nIndex = 0; nIndex < sIniName.size(); ++nIndex)
	{
		if (sIniName[nIndex] == '/')
		{
			sIniName[nIndex] = '\\';
		}
	}
#endif // PLATFORM_LINUX
	
	//find the file in the file system (overwrites szPath)
	g_pLTBase->FileMgr()->ExtractFile(sIniName.c_str(),szPath,LTARRAYSIZE(szPath),m_bExtracted);

	//verify this file exists
	bool bExists = LTFileOperations::FileExists( szPath );
	if (!bExists)
	{
		m_sFile = "";
		return;
	}
	
#if defined(PLATFORM_LINUX)
	// on Linux platforms, we need to convert to a single-byte character file (if necessary)
	CLTFileRead cFileRead;
	if (!cFileRead.Open(szPath))
	{
		return;
	}
	
	// get the size
	uint64 nSize = 0;
	if (!cFileRead.GetFileSize(nSize))
	{
		return;
	}
	
	// read the complete file
	char* pszFileData = NULL;
	LT_MEM_TRACK_ALLOC(pszFileData = new char[nSize], LT_MEM_TYPE_GAMECODE);
	
	if (!cFileRead.Read(pszFileData, nSize))
	{
		return;
	}
	
	// we're done with the input file
	cFileRead.Close();
	
	// look for the Unicode header
	if (((uint8)pszFileData[0] == 0xFF) && ((uint8)pszFileData[1] == 0xFE))
	{
		// open the output file
		CLTFileWrite cFileWrite;
		if (!cFileWrite.Open(szPath, false))
		{
			delete pszFileData;
			return;
		}
		
		// write every other byte to the output file
		for (uint32 nIndex = 2; nIndex < nSize; nIndex += 2)
		{
			if (!cFileWrite.Write((void*)&pszFileData[nIndex], 1))
			{
				delete pszFileData;
				return;
			}
		}
	}
	
	// free the buffer
	delete pszFileData;

#endif // PLATFORM_LINUX
	
	m_sFile = szPath;

};

MultiplayerMissionIni::~MultiplayerMissionIni()
{
	if (IsValid() && m_bExtracted)
	{
		g_pLTBase->FileMgr()->DeleteExtractedFile(m_sFile.c_str());
	}
}


bool MultiplayerMissionIni::GetName(wchar_t* pBuffer, uint32 nBufferSize) const
{
	if (!IsValid())
	{
		LTStrCpy(pBuffer,L"",nBufferSize);
		return false;
	}

	//read the namestr
	LTProfileUtils::ReadString(MDB_Mission_L,MDB_NameStr_L,L"",pBuffer,nBufferSize,MPA2W(m_sFile.c_str()).c_str());

#ifdef _CLIENTBUILD

	//overwrite the namestr if we have a name id (and we're on the client)
	if (GetCurExecutionShellContext() == eExecutionShellContext_Client)
	{
		char szNameId[256] = "";
		LTProfileUtils::ReadString(MDB_Mission,MDB_Name,"",szNameId,LTARRAYSIZE(szNameId),m_sFile.c_str());
		if (!LTStrEmpty(szNameId))
		{
			LTStrCpy(pBuffer,LoadString(szNameId),nBufferSize);
		}
	}
#endif

	return true;

}
bool MultiplayerMissionIni::GetPhoto(char* pBuffer, uint32 nBufferSize) const
{
	if (!IsValid())
	{
		LTStrCpy(pBuffer,"",nBufferSize);
		return false;
	}

	char szPath[MAX_PATH*2] = "";
	char szFN[256] = "";


	//create the file system path to verify it exists
	LTProfileUtils::ReadString(MDB_Mission,MDB_Photo,"",szFN,LTARRAYSIZE(szFN),m_sFile.c_str());

	if	(!LTStrEmpty(szFN))
	{
		LTStrCpy(szPath,m_sPath.c_str(),LTARRAYSIZE(szPath));
		LTStrCat(szPath,szFN,LTARRAYSIZE(szFN));

		ILTInStream* pStream = g_pLTBase->FileMgr()->OpenFile(szPath);
		if (pStream)
		{
			//we've verified it exists, now create the resource path for it
			LTStrCpy(pBuffer,szPath,nBufferSize);

			pStream->Release();
			pStream = NULL;

			return true;
		}
	}

	LTStrCpy(pBuffer,"",nBufferSize);
	return false;
}

uint32	MultiplayerMissionIni::GetMinPlayers() const
{
	if (!IsValid())
	{
		return 0;
	}

	return LTProfileUtils::ReadUint32(MDB_Mission,MDB_MinPlayers,0,m_sFile.c_str());

}
uint32	MultiplayerMissionIni::GetMaxPlayers() const
{
	if (!IsValid())
	{
		return 0;
	}
	return LTProfileUtils::ReadUint32(MDB_Mission,MDB_MaxPlayers,0,m_sFile.c_str());
}


bool MultiplayerMissionIni::GetRequiredFeatures(char* pBuffer, uint32 nBufferSize, uint8 nIndex) const
{
	if (!IsValid())
	{
		LTStrCpy(pBuffer,"",nBufferSize);
		return false;
	}

	char szName[128] = "";
	if (nIndex)
	{
		LTSNPrintF(szName,LTARRAYSIZE(szName),"%s%d",MDB_RequiredFeatures,nIndex);
	}
	else
	{
		LTSNPrintF(szName,LTARRAYSIZE(szName),"%s",MDB_RequiredFeatures);
	}


	//read the namestr
	LTProfileUtils::ReadString(MDB_Mission,szName,"",pBuffer,nBufferSize,m_sFile.c_str());

	return true;
}

bool MultiplayerMissionIni::GetSupportedFeatures(char* pBuffer, uint32 nBufferSize) const
{
	if (!IsValid())
	{
		LTStrCpy(pBuffer,"",nBufferSize);
		return false;
	}

	//read the namestr
	LTProfileUtils::ReadString(MDB_Mission,MDB_SupportedFeatures,"",pBuffer,nBufferSize,m_sFile.c_str());

	return true;
}
