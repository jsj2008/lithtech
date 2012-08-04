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

//-------------------------------------------------------------------
// FX_KEY
//
//	Defines data for a single effect key item, which is a single type
// like a particle system or sprite and is the atomic element of effects
//-------------------------------------------------------------------
struct FX_KEY
{
	FX_KEY() :
		m_bDisableAtDistance( false ),
		m_fMaxStartOffset(0.0f),
		m_bRandomStartOffset(false),
		m_fStartOffsetInterval(0.0f),
		m_bSmoothShutdown(true),
		m_nDetailLevel(0),
		m_bGore(false)
	{
	}

	~FX_KEY()
	{
	}
		

	FX_REF							   *m_pFxRef;
	CBaseFXProps					   *m_pProps;
	uint32								m_dwID;
	float								m_tmStart;
	float								m_tmEnd;

	bool								m_bLinked;
	uint32								m_dwLinkedID;
	char								m_sLinkedNodeName[32];
	bool								m_bContinualLoop;

	//information about us needing to run several simulation updates before fully begin
	//running it

	//the maximum time that the starting time can be offset by
	float								m_fMaxStartOffset;

	//whether or not we should use the max start offset or pick a val in [0...start]
	bool								m_bRandomStartOffset;

	//whether or not this effect is going to shut down smooth
	bool								m_bSmoothShutdown;

	//the detail level of this effect
	uint32								m_nDetailLevel;

	//the time slices we should use between 0 and start to simulate
	float								m_fStartOffsetInterval;

	//determines whether or not this effect is related to gore so that it can be disabled
	//on low violence settings
	bool								m_bGore;

	//boolean indicating whether or not this effect should be frozen when
	//it falls outside of some predetermined radius (used to optimize
	//away updating effects too far away)
	bool								m_bDisableAtDistance;
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
		m_pKeys(NULL),
		m_nNumKeys(0),
		m_tmTotalTime(0.0f)
	{
		m_sName[0] = '\0';
	}

	~FX_GROUP()
	{
		Term();
	}

	void Term()
	{
		debug_deletea(m_pKeys);
		m_pKeys			= NULL;
		m_nNumKeys		= 0;
		m_tmTotalTime	= 0.0f;
		m_sName[0]		= '\0';
	}

	char								m_sName[128];
	float								m_tmTotalTime;
	FX_KEY*								m_pKeys;
	uint32								m_nNumKeys;
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

	//handle initialization/freeing of resources
	bool							Init(ILTClient* pLTClient);
	void							Term();

	//used for finding specific effects for creation
	FX_GROUP*						FindGroupFX(const char *sName);

	//called to delete an effect
	void							DeleteEffect(CBaseFX* pFx);

	//sets up the parameters for the effect
	void							SetAppFocus(bool bAppFocus);

	//sets the player object
	void							SetPlayer(HOBJECT hObj);

	//sets a callback function and user data for 
	void							SetCreateCallback(TCreateClientFXFn pFn, void* pUserData);

	//finds the data for creating a specific effect key
	FX_REF*							FindFX(const char *sName);

private:

	int32							FindFXID(const char *sName);

	//for managing the DLL
	bool							LoadFxDll();
	void							UnloadFxDll();

	//for loading in the FX files
	bool							LoadFxGroups(ILTClient* pClient, const char *sName);
	bool							ReadFXProp( bool bText, ILTStream* pFxFile, FX_PROP& fxProp );
	bool							ReadFXKey( bool bText, ILTStream* pFxFile, float fTotalTime, FX_KEY* pKey, FX_PROP* pPropBuffer, uint32 nBuffLen );
	bool							ReadFXGroup( bool bText, ILTStream* pFxFile, FX_GROUP* pFxGroup, FX_PROP* pPropBuffer, uint32 nBuffLen );
	bool							ReadFXGroups( bool bText, ILTStream* pFxFile, CLinkList<FX_GROUP *> &collGroupFx );

	//the actual DLL handle
	HINSTANCE						m_hDLLInst;

	//DLL entry points
	FX_CREATEPROPLIST				m_pfnCreatePropList;
	FX_FREEPROPLIST					m_pfnFreePropList;
	FX_SETPLAYERFUNC				m_pfnSetPlayer;
	FX_SETAPPFOCUS					m_pfnSetAppFocus;
	FX_DELETEFUNC					m_pfnDeleteFX;
	FX_SETCREATEFUNCTION			m_pfnSetCreateFunction;

	//this is only intended for use as a singleton so prevent instantiation
	CClientFXDB();

	//The list of the different effect classes that we can instantiate
	FX_REF*							m_pEffectTypes;
	uint32							m_nNumEffectTypes;

	//The list of various effects that can be created
	CLinkList<FX_GROUP *>			m_collGroupFX;
};

#endif
