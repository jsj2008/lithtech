//------------------------------------------------------------------
//
//   ClientFXDB.h
//
//   This class is responsible for binding any client effects DLL's,
//	loading effects from files, and keeping a global list of all
//	effects that the game can use. All ClientFXMgrs tap into this
//	database to access the non-instanced effect data
//
//	Created: 8/10/02
//
//------------------------------------------------------------------

#ifndef __CLIENTFXDB_H__
#define __CLIENTFXDB_H__

#ifndef __BASEFX_H__
#include "BaseFx.h"
#endif

#ifndef __MEMBLOCKALLOCATOR_H__
#include "memblockallocator.h"
#endif

#ifndef __LTLIBRARYLOADER_H__
#include "ltlibraryloader.h"
#endif

//-------------------------------------------------------------------
// FX_KEY
//
//	Defines data for a single effect key item, which is a single type
// like a particle system or sprite and is the atomic element of effects
//-------------------------------------------------------------------
struct FX_KEY
{
	FX_KEY() :
		m_pFxRef(NULL),
		m_pProps(NULL)
	{
	}
		
	//the structure needed to determine the name and creation entry points for this effect
	FX_REF			*m_pFxRef;

	//the properties that represent the parameters of this effect
	CBaseFXProps	*m_pProps;
};

//-------------------------------------------------------------------
// FX_GROUP
//
//	This is a composite of one or more FX_KEYs and controls a full
//	effect.
//-------------------------------------------------------------------
struct FX_GROUP
{
	FX_GROUP() :
		m_tmTotalTime(0.0f),
		m_sName(NULL)
	{
	}

	~FX_GROUP()
	{
	}

	//the name of this group. Note that this points into the effect string
	//table and therefore must never be changed or deleted.
	const char*				m_sName;

	float					m_tmTotalTime;
	CMemBlockArray<FX_KEY>	m_KeyList;
};


//-------------------------------------------------------------------
// CClientFXDB
//
// A collection of FX_GROUPs that compose all possible effects that
// the game can play
//-------------------------------------------------------------------

class CClientFXDB
{
public:

	//accesses the one database object
	static CClientFXDB&		GetSingleton();

	//not intended for use as a base class
	~CClientFXDB();

	//for managing the DLL. Note that this must be loaded before effect files can be loaded, and unloading
	//the DLL will force unloading of effect files
	bool				LoadFxDll(const char* pszClientFxFile, bool bUseEngineFileSystem);
	void				UnloadFxDll();

	//handle loading and freeing the effect file data
	bool				LoadFxFilesInDir(ILTCSBase* pLTCSBase, const char* pszRelDirectory);

	//called to load the effect files from a tools environment where the engine is not present
    bool				LoadToolFxFilesInDir(const char* pszAbsoluteDirectory);

	//called to unload all currently loaded effect files
	void				UnloadFxFiles();

	//used for finding specific effects for creation
	const FX_GROUP*		FindGroupFX(const char *sName) const;

	//called to delete an effect
	void				DeleteEffect(CBaseFX* pFx);

	//sets up the parameters for the effect
	void				SetAppFocus(bool bAppFocus);

	//finds the data for creating a specific effect key
	const FX_REF*		FindFX(const char *sName) const;

	//given a resource name, this will collect all of the resources that are reference by that effect.
	//This will return false if the named effect cannot be found. Note that this will not clear
	//the provided list before adding resources so that resources can be accumulated from multiple effects.
	typedef std::vector<std::string>	TFxResourceList;
	bool				GetFxResources(const char* pszEffect, TFxResourceList& ResourceList) const;

private:

	//The list of various effects that can be created
	typedef std::vector<FX_GROUP*, LTAllocator<FX_GROUP*, LT_MEM_TYPE_CLIENTSHELL> >	TEffectList;
	TEffectList			m_Effects;

	//the list of the string tables that the effects reference
	typedef std::vector<uint8*, LTAllocator<uint8*, LT_MEM_TYPE_CLIENTSHELL> >		TStringTableList;
	TStringTableList	m_DataBlocks;

	//called to free an effect group object that has been constructed (note that it does not
	//delete the memory since it assumes that it was created in place)
	void				FreeFxGroup(FX_GROUP& FxGroup);

	//for loading in the FX files
	bool				LoadFxGroups(ILTInStream& InFile);

	bool				ReadFXKey(	ILTInStream& InFile, float fTotalTime, FX_KEY* pKey, CMemBlockAllocator& Allocator, 
									const char* pszStringTable, const uint8* pCurveData);

	bool				ReadFXGroup(ILTInStream& InFile, FX_GROUP* pFxGroup, CMemBlockAllocator& Allocator, 
									const char* pszStringTable, const uint8* pCurveData);

	bool				ReadFXGroups(	ILTInStream& InFile, TEffectList &collGroupFx, uint32 nNumGroups, CMemBlockAllocator& Allocator,
										const char* pszStringTable, const uint8* pCurveData);

	//the handle to the DLL that is currently loaded
	HLTMODULE			m_hClientFxModule;

	//the filename of the module that we loaded
	char				m_pszClientFxModuleFile[MAX_PATH];

	//whether or not this file was extracted from an archive and needs to be freed
	bool				m_bExtractedClientFx;

	//DLL entry points
	FX_GETVERSION		m_pfnGetVersion;
	FX_FREEPROPLIST		m_pfnFreePropList;
	FX_DELETEFUNC		m_pfnDeleteFX;
	FX_TERMDLLRUNTIME	m_pfnTermDLLRuntime;

	//this is only intended for use as a singleton so prevent instantiation
	CClientFXDB();

	//The list of the different effect classes that we can instantiate
	FX_REF*				m_pEffectTypes;
	uint32				m_nNumEffectTypes;

};

#endif
