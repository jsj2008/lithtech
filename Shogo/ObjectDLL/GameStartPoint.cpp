// ----------------------------------------------------------------------- //
//
// MODULE  : GameStartPoint.cpp
//
// PURPOSE : GameStartPoint - Definition
//
// CREATED : 9/30/97
//
// ----------------------------------------------------------------------- //

#include "GameStartPoint.h"
#include "cpp_server_de.h"
#include "PlayerModeTypes.h"
#include "RiotServerShell.h"
#include "RiotObjectUtilities.h"
#include "CachedFiles.h"
#include "DebrisTypes.h"
#include "SurfaceFunctions.h"

extern CRiotServerShell* g_pRiotServerShellDE;

DDWORD GameStartPoint::m_dwCounter = 0;

BEGIN_CLASS(GameStartPoint)
	ADD_LONGINTPROP(PlayerMode, 0)
	ADD_LONGINTPROP(GameType, 0)
	ADD_STRINGPROP(TriggerTarget, "")
	ADD_STRINGPROP(TriggerMessage, "")
END_CLASS_DEFAULT_FLAGS(GameStartPoint, StartPoint, NULL, NULL, CF_ALWAYSLOAD)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameStartPoint::GameStartPoint
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

GameStartPoint::GameStartPoint() : StartPoint()
{ 
	m_nPlayerMode			= PM_MODE_FOOT; 
	m_eGameType				= SINGLE; 
	m_hstrName				= DNULL;
	m_hstrTriggerTarget		= DNULL;
	m_hstrTriggerMessage	= DNULL;
	VEC_INIT(m_vPitchYawRoll);
	m_dwCounter++;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameStartPoint::~GameStartPoint
//
//	PURPOSE:	Deallocate
//
// ----------------------------------------------------------------------- //

GameStartPoint::~GameStartPoint()
{ 
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (m_hstrName)
	{
		pServerDE->FreeString(m_hstrName);
	}

	if (m_hstrTriggerTarget)
	{
		pServerDE->FreeString(m_hstrTriggerTarget);
	}

	if (m_hstrTriggerMessage)
	{
		pServerDE->FreeString(m_hstrTriggerMessage);
	}
	m_dwCounter--;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameStartPoint::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD GameStartPoint::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return 0;

	DDWORD dwRet = StartPoint::EngineMessageFn(messageID, pData, fData);

	switch(messageID)
	{
		case MID_PRECREATE:
		{
			ObjectCreateStruct *pStruct = (ObjectCreateStruct*)pData;

			if (pStruct)
			{
				if (fData == PRECREATE_WORLDFILE)
				{
					ReadProp(pStruct);
				}

				SAFE_STRCPY(pStruct->m_Name, "ShogoStartPoint");
			}			

			break;
		}

		case MID_INITIALUPDATE:
		{
			CacheFiles( );
			break;
		}

		default : break;
	}

	return dwRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameStartPoint::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

DBOOL GameStartPoint::ReadProp(ObjectCreateStruct *pData)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pData) return DFALSE;

	long nLongVal;
	pServerDE->GetPropLongInt("PlayerMode", &nLongVal);

	if (PM_MODE_FOOT <= nLongVal && nLongVal <= PM_MULTIPLAYER_MCA)
	{
		m_nPlayerMode = nLongVal;
	}


	pServerDE->GetPropLongInt("GameType", &nLongVal);

	if (SINGLE <= nLongVal && nLongVal <= CAPTURE_FLAG)
	{
		m_eGameType = (GameType) nLongVal;
	}


	char buf[MAX_CS_FILENAME_LEN];
	buf[0] = '\0';
	pServerDE->GetPropString("Name", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hstrName = pServerDE->CreateString(buf);


	pServerDE->GetPropRotationEuler("Rotation", &m_vPitchYawRoll);

	GenericProp genProp;
	if (pServerDE->GetPropGeneric("TriggerTarget", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
		{
			m_hstrTriggerTarget = pServerDE->CreateString(genProp.m_String);
		}
	}

	if (pServerDE->GetPropGeneric("TriggerMessage", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
		{
			m_hstrTriggerMessage = pServerDE->CreateString(genProp.m_String);
		}
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameStartPoint::CacheFiles
//
//	PURPOSE:	Cache files associated with this object
//
// ----------------------------------------------------------------------- //

void GameStartPoint::CacheFiles()
{
	if( !m_hObject || m_dwCounter > 1 )
		return;

	// Don't cache if the world already loaded...
	if( !( g_pServerDE->GetServerFlags( ) & SS_CACHING ))
		return;

	// Okay, determine if this start point is used in the current game
	// (i.e., Single player are only used in single player, etc.)

	if (m_eGameType != g_pRiotServerShellDE->GetGameType()) return;

	// Cache files that are onfoot or mech level specific...
	if (m_nPlayerMode == PM_MODE_FOOT)
	{
		CacheOnFootFiles();
	}
	else if (m_nPlayerMode != PM_MODE_KID)
	{
		CacheMechaFiles();
	}

	CachePlayerModeFiles( );

	CacheSurfaceFiles( );
}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameStartPoint::CacheOnFootFiles
//
//	PURPOSE:	Cache files associated with on foot levels
//
// ----------------------------------------------------------------------- //

void GameStartPoint::CacheOnFootFiles()
{
	int i;

	if (!g_pServerDE) return;

	for (i=0; i < NUM_CACHED_MODELSONFOOT; i++)
	{
		g_pServerDE->CacheFile(FT_MODEL, g_pCachedModelsOnFoot[i]);
	}

	for ( i=0; i < NUM_CACHED_TEXTURESONFOOT; i++)
	{
		g_pServerDE->CacheFile(FT_TEXTURE, g_pCachedTexturesOnFoot[i]);
	}

	for (i=0; i < NUM_CACHED_SOUNDS_3DONFOOT; i++)
	{
		g_pServerDE->CacheFile(FT_SOUND, g_pCachedSound3DOnFoot[i]);
	}

	for (i=0; i < NUM_CACHED_SPRITESONFOOT; i++)
	{
		g_pServerDE->CacheFile(FT_SPRITE, g_pCachedSpriteOnFoot[i]);
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameStartPoint::CacheMechaFiles
//
//	PURPOSE:	Cache files associated with on mecha levels
//
// ----------------------------------------------------------------------- //

void GameStartPoint::CacheMechaFiles()
{
	int i;

	if (!g_pServerDE) return;

	for (i=0; i < NUM_CACHED_MODELSMECH; i++)
	{
		g_pServerDE->CacheFile(FT_MODEL, g_pCachedModelsMech[i]);
	}

	for (i=0; i < NUM_CACHED_TEXTURESMECH; i++)
	{
		g_pServerDE->CacheFile(FT_TEXTURE, g_pCachedTexturesMech[i]);
	}

	for (i=0; i < NUM_CACHED_SPRITESMECH; i++)
	{
		g_pServerDE->CacheFile(FT_SPRITE, g_pCachedSpriteMech[i]);
	}

	for (i=0; i < NUM_CACHED_SOUNDS_3DMECH; i++)
	{
		g_pServerDE->CacheFile(FT_SOUND, g_pCachedSound3DMech[i]);
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameStartPoint::CachePlayerModeFiles
//
//	PURPOSE:	Cache files associated with the current player mode.
//
// ----------------------------------------------------------------------- //

void GameStartPoint::CachePlayerModeFiles()
{
	DBYTE nModelId;
	char* pFile;
	int i;
	DebrisType eType;
	int nMaxDebrisSounds;

	// Can't cache files, since we don't know what player is yet...
	if( m_nPlayerMode == PM_CURRENT_MCA )
		return;

	// Get the modelid...
	nModelId = PlayerModeToModelID( m_nPlayerMode );

	// Cache the player models...
	pFile = DNULL;
	for( i = GT_FIRST; i < GT_LAST; i++ )
	{
		pFile = GetGibModel( nModelId, GibType( i ), MS_NORMAL );
		if( pFile )
		{
			g_pServerDE->CacheFile( FT_MODEL, pFile );
		}
	}

	// Cache player skin...
	pFile = GetSkin( nModelId, UCA, MS_NORMAL );
	if( pFile )
		g_pServerDE->CacheFile( FT_TEXTURE, pFile );

	// Cache gib sounds...
	switch( GetModelType( nModelId ))
	{
		case MT_HUMAN:
			eType = DBT_HUMAN_PARTS;
			break;

		case MT_MECHA:
			eType = DBT_MECHA_PARTS;
			break;

		default:
			eType = DBT_HUMAN_PARTS;
			break;
	}

	nMaxDebrisSounds = GetNumDebrisBounceSounds(eType);
	for( i = 0; i < nMaxDebrisSounds; i++ )
	{
		pFile = GetDebrisBounceSound( eType, i );
		if( pFile )
		{
			g_pServerDE->CacheFile( FT_SOUND, pFile );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameStartPoint::CacheSurfaceFiles()
//
//	PURPOSE:	Cache files associated with surface flags
//
// ----------------------------------------------------------------------- //

static SurfaceType s_aSurfaceTypes[] =
{
	ST_AIR,
	ST_CHAINFENCE,
	ST_STONE,
	ST_STONE_HEAVY,
	ST_STONE_LIGHT,
	ST_METAL,
	ST_METAL_HEAVY,
	ST_METAL_LIGHT,
	ST_METAL_HOLLOW,
	ST_METAL_HOLLOW_HEAVY,
	ST_METAL_HOLLOW_LIGHT,
	ST_WOOD,
	ST_DENSE_WOOD,
	ST_LIGHT_WOOD,
	ST_GLASS,
	ST_ENERGY,
	ST_BUILDING,
	ST_TERRAIN,
	ST_CLOTH,
	ST_PLASTIC,
	ST_PLASTIC_HEAVY,
	ST_PLASTIC_LIGHT,
	ST_FLESH,
	ST_SKY,
	ST_MECHA,
	ST_LIQUID
};

void GameStartPoint::CacheSurfaceFiles( )
{
	SurfaceType eSurfaceType;
	DebrisType eDebrisType;
	int i, nSurfaceIndex;
	DVector v;
	char *pDir;

	if (!g_pServerDE) return;

	for( nSurfaceIndex = 0; nSurfaceIndex < sizeof( s_aSurfaceTypes ) / sizeof( SurfaceType ); nSurfaceIndex++ )
	{
		eSurfaceType = s_aSurfaceTypes[nSurfaceIndex];

		eDebrisType = GetVectorDebrisType( eSurfaceType );
		for( i = 0; i < GetNumDebrisModels( eDebrisType ); i++ )
		{
			g_pServerDE->CacheFile( FT_MODEL, GetDebrisModel( eDebrisType, v, i ));
		}
		g_pServerDE->CacheFile( FT_TEXTURE, GetDebrisSkin( eDebrisType ));
		for( i = 0; i < GetNumDebrisBounceSounds( eDebrisType ); i++ )
		{
			g_pServerDE->CacheFile( FT_SOUND, GetDebrisBounceSound( eDebrisType, i ));
		}
		for( i = 0; i < GetNumDebrisExplodeSounds( eDebrisType ); i++ )
		{
			g_pServerDE->CacheFile( FT_SOUND, GetDebrisExplodeSound( eDebrisType, i ));
		}

		g_pServerDE->CacheFile( FT_SPRITE, GetImpactSprite( eSurfaceType, GUN_COLT45_ID	));
		g_pServerDE->CacheFile( FT_SOUND, GetFootStepSound( eSurfaceType, m_nPlayerMode, DFALSE ));
		g_pServerDE->CacheFile( FT_SOUND, GetFootStepSound( eSurfaceType, m_nPlayerMode, DTRUE ));

		pDir = GetImpactSoundDir( eSurfaceType );
		if (pDir)
		{
			for( i = 1; i <= 5; i++ )
			{
				sprintf(s_FileBuffer,"Sounds\\Weapons\\impacts\\%s\\impact%d.wav", pDir, i);
				g_pServerDE->CacheFile( FT_SOUND, s_FileBuffer );
			}
		}
	}
}
