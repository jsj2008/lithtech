#include "stdafx.h"
#include "ClientFXDB.h"
#include <algorithm>
#include "resourceextensions.h"
#include "iltfilemgr.h"
#include "ltfileoperations.h"
#include "CLTFileToILTInStream.h"

#if defined(PLATFORM_XENON)
// XENON: Necessary code for implementing runtime swapping
#include "endianswitch.h"
#endif // PLATFORM_XENON

//-----------------------------------------------------------------
// DLL loading data
//-----------------------------------------------------------------

//the version of the file format we support
#define		EFFECT_FILE_VERSION		2


//-----------------------------------------------------------------
// File Format utilities
//-----------------------------------------------------------------

//header of an effect file
struct SEffectHeader
{
	//LTFX four CC
	uint32	m_nFourCC;

	//the version format of this file
	uint32	m_nFileVersion;

	//the version number of the effect DLL that this was compiled with
	uint32	m_nEffectVersion;

	//the size of the string table within this file
	uint32	m_nStringTableSize;

	//the size of the function curve data block
	uint32	m_nFunctionCurveDataSize;

	//the total number of effect keys
	uint32	m_nTotalKeys;

	//the total number of effects
	uint32	m_nTotalEffects;

	//the number of effect types that are contained within this file
	uint32	m_nNumEffectTypes;

	//different offsets into the file, all absolute, and intended mainly for use when loading just
	//the effect list for tools
	uint32	m_nStringTableOffset;	
	uint32	m_nEffectListOffset;
};

//this structure represents an FX group stored on disk
struct SDiskFXGroup
{
	//the index into the string table for the name of this group
	uint32		m_nGroupNameIndex;

	//the length of this group in milliseconds
	uint32		m_nGroupLenMS;

	//the number of keys in this group
	uint32		m_nNumKeys;
};

//this structure represents an FX key stored on disk
struct SDiskFXKey
{
	//the type of the effect
	uint32		m_nEffectType;

	//the time that this key starts at
	float		m_fStartTime;

	//the time that this key ends
	float		m_fEndTime;

	//TODO:JO The following two fields are deprecated and can be removed in next FX version change

	//the integer ID of this key used for linking
	uint32		m_nID;

	//the ID of the effect that we are optionally linked to
	uint32		m_nLinkedID;

	//the number of properties for this key
	uint32		m_nNumProps;
};

//this structure represents the header for an FX property stored on disk
struct SDiskFXProp
{
	//the index into the string table for the property name
	uint32		m_nPropNameIndex;

	//the file position after the property data (note this needs to change...)
	uint32		m_nPostPropPosition;
};

#if defined(PLATFORM_XENON)
// XENON: Necessary code for implementing runtime swapping
// Type descriptions for the endian swapper
DATATYPE_TO_ENDIANFORMAT(SEffectHeader, "10i");
DATATYPE_TO_ENDIANFORMAT(SDiskFXGroup, "3i");
DATATYPE_TO_ENDIANFORMAT(SDiskFXKey, "i2f3i");
DATATYPE_TO_ENDIANFORMAT(SDiskFXProp, "2i");
#endif // PLATFORM_XENON

inline void LTStream_Read(ILTInStream* pInStream, SEffectHeader& Header)
{
	pInStream->Read(&Header, sizeof(Header));
}

//callback function to sort the effects by name
static bool SortEffectNameCallback(const FX_GROUP* lhs, const FX_GROUP* rhs)
{
	//we sort by just doing a comparison on the name of the groups
	if(LTStrICmp(lhs->m_sName, rhs->m_sName) < 0)
		return true;

	return false;
}

//-----------------------------------------------------------------
// IFXResourceCollector implementation for resource collection
//-----------------------------------------------------------------

class CFXResourceCollector :
	public IFXResourceCollector
{
public:

	CFXResourceCollector(const CClientFXDB& Database, CClientFXDB::TFxResourceList& ResList) :
		m_Database(Database),
		m_ResourceList(ResList)
	{
	}

	//this function will be called with each resource that is encountered. The filename is
	//relative to the resource root
	virtual void	CollectResource(const char* pszResource)
	{
		if(!LTStrEmpty(pszResource))
			m_ResourceList.push_back(pszResource);
	}

	//this function will be called with each child effect that is encountered. Note that this
	//may be a semicolon delimited listing of effects
	virtual void	CollectEffect(const char* pszEffectList)
	{
		if(LTStrEmpty(pszEffectList))
			return;

		//the current character offset
		uint32 nDestOffset = 0;
		const char* pszCurrChar = pszEffectList;
		char pszEffectName[MAX_CLIENTFX_NAME_LEN + 1];

		//we need to divide the name of the effect into a listing of client fx that are semicolon
		//delimited. For each effect name, we put this into the name buffer and get resources
		while(*pszCurrChar != '\0')
		{
			//if this is a semicolon, we need to create the client fx as long as we've copied over
			//something
			if(*pszCurrChar == ';')
			{
				if(nDestOffset > 0)
				{
					m_Database.GetFxResources(pszEffectName, m_ResourceList);
					nDestOffset = 0;
				}
			}
			else if (isspace(*pszCurrChar))
			{
				//we just want to ignore any whitespace at this point
			}
			else
			{
				//we have a character, add it to the end of the buffer if there is room
				if(nDestOffset < LTARRAYSIZE(pszEffectName) - 1)
				{
					pszEffectName[nDestOffset] = *pszCurrChar;
					pszEffectName[nDestOffset + 1] = '\0';
					nDestOffset++;
				}
			}

			//and move onto the next character
			++pszCurrChar;
		}

		//now see if there was an ending effect
		if(nDestOffset > 0)
		{
			m_Database.GetFxResources(pszEffectName, m_ResourceList);
			nDestOffset = 0;
		}
	}

private:

	//the database we use for handling effect requests
	const CClientFXDB&				m_Database;

	//the list where we add all of our resources
	CClientFXDB::TFxResourceList&	m_ResourceList;
};

//-----------------------------------------------------------------
// CClientFXDB construction
//-----------------------------------------------------------------
CClientFXDB::CClientFXDB()
{
	LTStrCpy(m_pszClientFxModuleFile, "", LTARRAYSIZE(m_pszClientFxModuleFile));
	m_bExtractedClientFx	= false;

	m_nNumEffectTypes		= 0;
	m_pEffectTypes			= NULL;
	m_hClientFxModule		= NULL;

	m_pfnDeleteFX			= NULL;
	m_pfnGetVersion			= NULL;

}

CClientFXDB::~CClientFXDB()
{
	UnloadFxFiles();
	UnloadFxDll();
}

bool CClientFXDB::LoadFxFilesInDir(ILTCSBase* pLTCSBase, const char* pszRelDirectory)
{
	//we need to make sure that we have our clientfx types
	if(!m_pEffectTypes)
		return false;

	// Obtain a snapshot of our ClientFX directory
	FileEntry *pFiles = pLTCSBase->FileMgr()->GetFileList(pszRelDirectory);

	//if there are no files in the directory, no big deal, we just have no effects
	if( !pFiles ) 
		return true;

	FileEntry *pEntry = pFiles;
	while( pEntry )
	{
		// Ignore directorys... only look at files
		if( pEntry->m_Type == eLTFileEntryType_File )
		{
			// Is this a ClientFx file?
			if( CResExtUtil::IsFileOfType(pEntry->m_pBaseFilename, RESEXT_EFFECT_PACKED) )
			{
				//attempt to open up the file stream
				ILTInStream *pFxFile = pLTCSBase->FileMgr()->OpenFile(pEntry->m_pFullFilename);
				if(!pFxFile)
				{
					pLTCSBase->CPrint("Error opening effect file %s", pEntry->m_pFullFilename);
					return false;
				}

				// Try and load it
				if( !LoadFxGroups(*pFxFile) )
				{
					//we failed to load this group, but don't return false, just inform the user, and
					//keep loading other files
					pLTCSBase->CPrint("An error occurred loading the effect file %s. Try repacking the effect file to ensure it is the latest version.", pEntry->m_pFullFilename);
				}

				//make sure to release our stream
				LTSafeRelease(pFxFile);
			}
		}

		pEntry = pEntry->m_pNext;
	}

	// Free the List we obtained
	pLTCSBase->FileMgr()->FreeFileList( pFiles );

	//now that we have loaded in all of the effects we can now sort them so that we can find
	//the effects quickly later
	std::sort(m_Effects.begin(), m_Effects.end(), SortEffectNameCallback);

#ifndef _FINAL
	//we also need to run through and bring up a console error for every naming conflict we find
	if(m_Effects.size() >= 2)
	{
		for(uint32 nCurrEffect = 0; nCurrEffect < m_Effects.size() - 1; nCurrEffect++)
		{
			//see if this conflicts with the following effect (safe check to make since we
			//have sorted the effects)
			if(LTStrIEquals(m_Effects[nCurrEffect]->m_sName, m_Effects[nCurrEffect + 1]->m_sName))
			{
				pLTCSBase->CPrint("Error: Found multiple ClientFX using the name %s", m_Effects[nCurrEffect]->m_sName);
			}
		}
	}
#endif

	//success
	return true;
}

bool CClientFXDB::LoadToolFxFilesInDir(const char* pszAbsoluteDirectory)
{
	//we need to make sure that we have our clientfx types
	if(!m_pEffectTypes)
		return false;

	//get a listing of the files in the directory
	char pszFilter[MAX_PATH];
	LTStrCpy(pszFilter, pszAbsoluteDirectory, LTARRAYSIZE(pszFilter));
	uint32 nStrLen = LTStrLen(pszFilter);

	//append on our filter
	LTStrCat(pszFilter, "\\*." RESEXT_EFFECT_PACKED, LTARRAYSIZE(pszFilter));

	//now iterate through all of the effects
	LTFINDFILEHANDLE hCurrFile;
	LTFINDFILEINFO FileInfo;

	if(LTFileOperations::FindFirst(pszFilter, hCurrFile, &FileInfo))
	{
		do
		{
			// Build the full filename
			char pszFile[MAX_PATH];
			LTSNPrintF(pszFile, LTARRAYSIZE(pszFile), "%s\\%s", pszAbsoluteDirectory, FileInfo.name );

			//open up a stream for this file
			CLTFileToILTInStream FxFile;
			if(FxFile.Open(pszFile))
			{
				//now allow it to load
				LoadFxGroups(FxFile);
			}
		}
		while(LTFileOperations::FindNext(hCurrFile, &FileInfo));

		LTFileOperations::FindClose(hCurrFile);
	}

	//now that we have loaded in all of the effects we can now sort them so that we can find
	//the effects quickly later
	std::sort(m_Effects.begin(), m_Effects.end(), SortEffectNameCallback);

	//success
	return true;
}

//called to free an effect group object that has been constructed (note that it does not
//delete the memory since it assumes that it was created in place)
void CClientFXDB::FreeFxGroup(FX_GROUP& FxGroup)
{
	//delete all the key property lists
	for(uint32 nCurrKey = 0; nCurrKey < FxGroup.m_KeyList.GetSize(); nCurrKey++)
	{
		if(m_pfnFreePropList)
		{
			m_pfnFreePropList(FxGroup.m_KeyList[nCurrKey].m_pProps);
			FxGroup.m_KeyList[nCurrKey].m_pProps = NULL;
		}
	}

	//free the memory associated with our key list
	FxGroup.m_KeyList.Free();
	FxGroup.~FX_GROUP();
}

void CClientFXDB::UnloadFxFiles()
{
	//remove all of the effects from the database
	for(TEffectList::iterator it = m_Effects.begin(); it != m_Effects.end(); it++)
	{
		FreeFxGroup(**it);
	}
	m_Effects.clear();

	//and now all of our data blocks
	for(TStringTableList::iterator it = m_DataBlocks.begin(); it != m_DataBlocks.end(); it++)
	{
		debug_deletea( *it );
	}
	m_DataBlocks.clear();
}

//-----------------------------------------------------------------
// CClientFXDB DLL management
//-----------------------------------------------------------------

#if defined(PLATFORM_SEM)
extern "C"
{
	uint32 fxGetNum();
	uint32 fxGetVersion();
	FX_REF fxGetRef(int nFx);
	void fxDelete(CBaseFX *pDeleteFX);
	void fxFreePropList(CBaseFXProps* pPropList);
	void fxSetPlayer(HOBJECT hPlayer);
	void fxInitDLLRuntime();
	void fxTermDLLRuntime();
}
#endif // PLATFORM_SEM

//loads the effect DLL
bool CClientFXDB::LoadFxDll(const char* pszClientFxFile, bool bUseEngineFileSystem)
{
	//make sure that we don't have any other dll's already bound
	UnloadFxDll();

#if defined(PLATFORM_SEM)

	FX_GETNUM pfnNum = fxGetNum;
	FX_GETREF pfnRef = fxGetRef;

	FX_INITDLLRUNTIME pfnInitDLLRuntime = fxInitDLLRuntime;

	m_pfnDeleteFX			= fxDelete;
	m_pfnFreePropList		= fxFreePropList;
	m_pfnGetVersion			= fxGetVersion;
	m_pfnTermDLLRuntime		= fxTermDLLRuntime;

#else // PLATFORM_SEM

	//if we are using the engine system, extract the file from the archive if needed, otherwise
	//just use the file that the provided us
	if(bUseEngineFileSystem)
	{
		//make sure to extract the fxd file if necessary
		if(g_pLTBase->FileMgr()->ExtractFile(pszClientFxFile, m_pszClientFxModuleFile, LTARRAYSIZE(m_pszClientFxModuleFile), m_bExtractedClientFx) != LT_OK)
		{
			UnloadFxDll();
			return false;
		}
	}
	else
	{
		LTStrCpy(m_pszClientFxModuleFile, pszClientFxFile, LTARRAYSIZE(m_pszClientFxModuleFile));
	}


	m_hClientFxModule = LTLibraryLoader::OpenLibrary(m_pszClientFxModuleFile);
	if (!m_hClientFxModule)
	{
		UnloadFxDll();
		return false;
	}

	//merge our interface database with the database in the DLL we just loaded.
	TSetMasterFn pSetMasterFn = (TSetMasterFn)LTLibraryLoader::GetProcAddress(m_hClientFxModule, "SetMasterDatabase");
	//check if the function existed.
	if (pSetMasterFn != NULL)
	{
		//merge our database with theirs
		pSetMasterFn(GetMasterDatabase());
	}

	// Attempt to retrieve the FX reference structure function
	FX_GETNUM pfnNum = (FX_GETNUM)LTLibraryLoader::GetProcAddress(m_hClientFxModule, "fxGetNum");
	FX_GETREF pfnRef = (FX_GETREF)LTLibraryLoader::GetProcAddress(m_hClientFxModule, "fxGetRef");

	FX_INITDLLRUNTIME pfnInitDLLRuntime	 = (FX_INITDLLRUNTIME)LTLibraryLoader::GetProcAddress(m_hClientFxModule, "fxInitDLLRuntime");

	m_pfnTermDLLRuntime		= (FX_TERMDLLRUNTIME)LTLibraryLoader::GetProcAddress(m_hClientFxModule, "fxTermDLLRuntime");
	m_pfnDeleteFX			= (FX_DELETEFUNC)LTLibraryLoader::GetProcAddress(m_hClientFxModule, "fxDelete");
	m_pfnFreePropList		= (FX_FREEPROPLIST)LTLibraryLoader::GetProcAddress(m_hClientFxModule, "fxFreePropList");
	m_pfnGetVersion			= (FX_GETVERSION)LTLibraryLoader::GetProcAddress(m_hClientFxModule, "fxGetVersion");

	//validate all the functions were properly loaded
	if (!pfnNum || !pfnRef || !pfnInitDLLRuntime || !m_pfnTermDLLRuntime || 
		!m_pfnDeleteFX || !m_pfnFreePropList || !m_pfnGetVersion) 
	{
		//failed to get all of the DLL entry points, so fail
		UnloadFxDll();
		return false;
	}

#endif // !PLATFORM_SEM

	//we should now allow the module to initialize itself
	pfnInitDLLRuntime();

	// Okay, if we got here then this is a valid dll with some special
	// fx in it....
	m_nNumEffectTypes = pfnNum();
	
	//allocate our list of effect types
	m_pEffectTypes = debug_newa(FX_REF, m_nNumEffectTypes);

	if(!m_pEffectTypes)
	{
		UnloadFxDll();
		return false;
	}

	for (uint32 nCurrEffect = 0; nCurrEffect < m_nNumEffectTypes; nCurrEffect ++)
	{
		// Retrieve the FX reference structure
		m_pEffectTypes[nCurrEffect] = pfnRef(nCurrEffect);
	}

	// Success !!
	return true;
}

//unloads the effect DLL
void CClientFXDB::UnloadFxDll()
{
	//we need to make sure that we don't have any existing loaded effect files
	UnloadFxFiles();

	//allow the module to terminate itself if we have the appropriate function
	if(m_pfnTermDLLRuntime)
	{
		m_pfnTermDLLRuntime();
	}

#if !defined(PLATFORM_SEM)

	//make sure to unload our reference to the library
	if (m_hClientFxModule) 
	{
		LTLibraryLoader::CloseLibrary(m_hClientFxModule);
	}

	//handle deleting the extracted file
	if(m_bExtractedClientFx)
	{
		g_pLTBase->FileMgr()->DeleteExtractedFile(m_pszClientFxModuleFile);
		m_bExtractedClientFx = false;
	}

	//and clear out the DLL name out
	LTStrCpy(m_pszClientFxModuleFile, "", LTARRAYSIZE(m_pszClientFxModuleFile));

#endif // !PLATFORM_SEM
	
	//make sure to invalidate our hooks into the DLL
	m_hClientFxModule		= NULL;
	m_pfnDeleteFX			= NULL;
	m_pfnGetVersion			= NULL;
	m_pfnTermDLLRuntime		= NULL;
	m_pfnFreePropList		= NULL;
	
	//cleanup our list of effect definitions
	debug_deletea(m_pEffectTypes);
	m_pEffectTypes		= NULL;
	m_nNumEffectTypes	= 0;
}

//-----------------------------------------------------------------
// CClientFXDB file loading code
//-----------------------------------------------------------------

bool CClientFXDB::ReadFXKey(ILTInStream& InFile, float fTotalTime, FX_KEY* pKey, CMemBlockAllocator& Allocator,
							const char* pszStringTable,	const uint8* pCurveData)
{
	//read in the disk key
	SDiskFXKey DiskKey;
	InFile.Read(&DiskKey, sizeof(DiskKey));

#if defined(PLATFORM_XENON)
	// XENON: Swap data at runtime
	LittleEndianToNative(&DiskKey);
#endif // PLATFORM_XENON

	//make sure this is a valid effect type
	if(DiskKey.m_nEffectType >= m_nNumEffectTypes)
		return false;

	pKey->m_pFxRef = &m_pEffectTypes[DiskKey.m_nEffectType];

	//ok, we can now convert our properties over to the appropriate form
	if(!pKey->m_pFxRef->m_pfnCreateProps)
		return false;

	//alright, get our property object
	pKey->m_pProps = pKey->m_pFxRef->m_pfnCreateProps(Allocator);
	if(!pKey->m_pProps)
		return false;

	//make sure to clamp the times to always be between 0 and the length of the group,
	//otherwise numerical accuracy problems will arise
	float fStartTime = LTCLAMP(DiskKey.m_fStartTime, 0.0f, fTotalTime);
	float fEndTime   = LTCLAMP(DiskKey.m_fEndTime, 0.0f, fTotalTime);	

	//determine if this is a continuous effect
	float fLifetime = fEndTime - fStartTime;
	bool bContinuous = (fLifetime >= fTotalTime - 0.01f);

	//now setup the lifespan of the key
	pKey->m_pProps->SetLifetime(fStartTime, fEndTime, bContinuous);

	for (uint32 nCurrProp = 0; nCurrProp < DiskKey.m_nNumProps; nCurrProp++)
	{
		//read in the property header
		SDiskFXProp DiskProp;
		InFile.Read(&DiskProp, sizeof(DiskProp));

		#if defined(PLATFORM_XENON)
			// XENON: Swap data at runtime
			LittleEndianToNative(&DiskProp);
		#endif // PLATFORM_XENON

		//now let the effect handle the parsing
		const char* pszPropName = &pszStringTable[DiskProp.m_nPropNameIndex];
		if(!pKey->m_pProps->LoadProperty(&InFile, pszPropName, pszStringTable, pCurveData))
		{
			//failed to load it, move to a valid location
			InFile.SeekTo(DiskProp.m_nPostPropPosition);
		}

		LTASSERT(InFile.GetPos() == DiskProp.m_nPostPropPosition, "Error: Mismatch in the size of the loaded parameter of an effect");
	}
	
	//notify the properties that we are done loading so it can perform any post load tasks necessary
	if(!pKey->m_pProps->PostLoadProperties())
	{
		//they failed
		m_pfnFreePropList(pKey->m_pProps);
		pKey->m_pProps = NULL;
		return false;
	}
	
	return true;
}

bool CClientFXDB::ReadFXGroup(ILTInStream& InFile, FX_GROUP* pFxGroup, CMemBlockAllocator& Allocator,
							  const char* pszStringTable, const uint8* pCurveData)
{
	//read in the disk representation
	SDiskFXGroup DiskGroup;
	InFile.Read(&DiskGroup, sizeof(DiskGroup));

	#if defined(PLATFORM_XENON)
		// XENON: Swap data at runtime
		LittleEndianToNative(&DiskGroup);
	#endif // PLATFORM_XENON

	//get our name from the string table
	pFxGroup->m_sName		= &pszStringTable[DiskGroup.m_nGroupNameIndex];
	pFxGroup->m_tmTotalTime = DiskGroup.m_nGroupLenMS / 1000.0f;
	
	//allocate room for the FX
	pFxGroup->m_KeyList.Allocate(DiskGroup.m_nNumKeys, Allocator);

	// Read in each FXKey
	for(uint32 nCurrKey = 0; nCurrKey < pFxGroup->m_KeyList.GetSize(); nCurrKey++)
	{
		ReadFXKey(InFile, pFxGroup->m_tmTotalTime, &pFxGroup->m_KeyList[nCurrKey], Allocator, pszStringTable, pCurveData);
	}

	//alright, by this time either we are in order, or there is a cyclic dependency
	return true;
}


bool CClientFXDB::ReadFXGroups(ILTInStream& InFile, TEffectList &collGroupFx, uint32 nNumGroups,
							   CMemBlockAllocator& Allocator, const char* pszStringTable, const uint8* pCurveData)
{
	//allocate all of our groups
	FX_GROUP* pGroups = Allocator.AllocateObjects<FX_GROUP>(nNumGroups);

	//now handle loading in each of our groups
	for(uint32 nCurrGroup = 0; nCurrGroup < nNumGroups; nCurrGroup++ )
	{
		if(!ReadFXGroup(InFile, &pGroups[nCurrGroup], Allocator, pszStringTable, pCurveData))
		{
			//make sure to clean up any effects we had already loaded
			for(uint32 nDestroyGroup = 0; nDestroyGroup < nNumGroups; nDestroyGroup++)
				FreeFxGroup(pGroups[nDestroyGroup]);

			return false;
		}
	}

	//we have successfully loaded in all groups, so now we can add them to our list (don't add them
	//before because if we failed, we would have invalid effects in the list)
	collGroupFx.reserve(collGroupFx.size() + nNumGroups);
	for(uint32 nCurrGroup = 0; nCurrGroup < nNumGroups; nCurrGroup++ )
	{
		collGroupFx.push_back(&pGroups[nCurrGroup]);
	}
	return true;
}

bool CClientFXDB::LoadFxGroups(ILTInStream& InFile)
{
	//read in the header of this file
	SEffectHeader Header;
	InFile >> Header;

	#if defined(PLATFORM_XENON)
		// XENON: Swap data at runtime
		LittleEndianToNative(&Header);
	#endif // PLATFORM_XENON

	//get the version number of the effect system
	uint32 nEffectVersion = m_pfnGetVersion();

	//perform a sanity check on the file type
	if(	(Header.m_nFourCC != LTMakeFourCC('L', 'T', 'F', 'X')) ||
		(Header.m_nFileVersion != EFFECT_FILE_VERSION) ||
		(Header.m_nEffectVersion != nEffectVersion) ||
		(Header.m_nNumEffectTypes != m_nNumEffectTypes))
	{
		return false;
	}

	//determine how large our data block is going to need to be
	uint32 nDataBlockSize = 0;

	//room for our data tables
	nDataBlockSize += AlignAllocSize(Header.m_nStringTableSize);
	nDataBlockSize += AlignAllocSize(Header.m_nFunctionCurveDataSize);

	//and now room for our effect objects
	nDataBlockSize += AlignAllocSize(sizeof(FX_KEY) * Header.m_nTotalKeys);
	nDataBlockSize += AlignAllocSize(sizeof(FX_GROUP) * Header.m_nTotalEffects);

	//and read in the count of each effect type and allocate room for its properties
	for(uint32 nCurrType = 0; nCurrType < Header.m_nNumEffectTypes; nCurrType++)
	{
		//read in the number of effects of this type
		uint32 nNumEffects;
		InFile >> nNumEffects;

		#if defined(PLATFORM_XENON)
			// XENON: Swap data at runtime
			LittleEndianToNative(&nNumEffects);
		#endif // PLATFORM_XENON

		//and now add that memory to our data block
		nDataBlockSize += AlignAllocSize(m_pEffectTypes[nCurrType].m_pfnGetPropSize() * nNumEffects);
	}

	//allocate our data block
	uint8* pDataBlock = debug_newa(uint8, nDataBlockSize);
	if(!pDataBlock)
	{
		return false;
	}

	//setup a memory block allocator for this data block
	CMemBlockAllocator Allocator(pDataBlock, nDataBlockSize);

	//read in our string table
	char* pszStringTable = (char*)Allocator.Allocate(Header.m_nStringTableSize);
	InFile.Read(pszStringTable, Header.m_nStringTableSize);

	//read in our function curve data
	uint8* pFunctionCurveData = (uint8*)Allocator.Allocate(Header.m_nFunctionCurveDataSize);
	InFile.Read(pFunctionCurveData, Header.m_nFunctionCurveDataSize);

	//read in all of our effects 
	if(!ReadFXGroups(InFile, m_Effects, Header.m_nTotalEffects, Allocator, pszStringTable, pFunctionCurveData))
	{
		debug_deletea(pDataBlock);
		return false;
	}

	//add this string table to our list of string tables
	m_DataBlocks.push_back(pDataBlock);

	//we now need to make sure that we used the memory properly
	LTASSERT(Allocator.GetAllocationOffset() == Allocator.GetBlockSize(), "Error: Incorrect usage of memory when loading clientfx");

	// Success !!
	return true;
}


//------------------------------------------------------------------
// CClientFXDB Group finding utilities
//------------------------------------------------------------------

const FX_GROUP* CClientFXDB::FindGroupFX(const char *sName) const
{
	//bail if we don't have a valid name
	if(!sName || !sName[0] ) 
		return NULL;

	// Locate the group using a binary search since all of our effects have been sorted
	//based upon name already
	int32 nMin = 0; 
	int32 nMax = (int32)m_Effects.size() - 1;

	while(nMin <= nMax)
	{
		int32 nMid = (nMin + nMax) / 2;
		int32 nKey = stricmp(sName, m_Effects[nMid]->m_sName);

		//check for a match
		if(nKey == 0)
		{
			return m_Effects[nMid];
		}

		//no match, continue the binary search
		if(nKey < 0)
		{
			//bottom half
			nMax = nMid - 1;
		}
		else
		{
			//top half
			nMin = nMid + 1;
		}
	}

	//No match was found
	return NULL;
}

//Finds an effect of the appropraite type
const FX_REF* CClientFXDB::FindFX(const char *sName) const
{
	for(uint32 nCurrEffect = 0; nCurrEffect < m_nNumEffectTypes; nCurrEffect++)
	{
		if (LTStrIEquals(sName, m_pEffectTypes[nCurrEffect].m_sName)) 
			return &m_pEffectTypes[nCurrEffect];
	}

	// Failure !!
	return NULL;
}

//given a resource name, this will collect all of the resources that are reference by that effect.
//This will return false if the named effect cannot be found. Note that this will not clear
//the provided list before adding resources so that resources can be accumulated from multiple effects.
bool CClientFXDB::GetFxResources(const char* pszEffect, TFxResourceList& ResourceList) const
{
	//find our effect reference
	const FX_GROUP* pFxGroup = FindGroupFX(pszEffect);
	if(!pFxGroup)
		return false;

	//setup our interface that the effect will call into
	CFXResourceCollector Collector(*this, ResourceList);

	//run through each key and collect resources
	for(uint32 nCurrKey = 0; nCurrKey < pFxGroup->m_KeyList.GetSize(); nCurrKey++)
	{
		const FX_KEY& CurrKey = pFxGroup->m_KeyList[nCurrKey];

		//now let the effect collect all of the resources that it uses
		CurrKey.m_pProps->CollectResources(Collector);
	}

	//success
	return true;
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

//------------------------------------------------------------------
// CClientFXDB Singleton support
//------------------------------------------------------------------
CClientFXDB& CClientFXDB::GetSingleton()
{
	static CClientFXDB s_ClientFXDB;
	return s_ClientFXDB;
}

