#include "Stdafx.h"
#include "ShatterTypeDB.h"

//
// Defines...
//

//the name of the category that contains all of the shatter types
static const char * const STDB_Category = "Fx/ShatterType";


//the names of different properties stored in the records
static const char * const g_pszSTDB_Material				= "Material";
static const char * const g_pszSTDB_RenderSolid				= "RenderSolid";
static const char * const g_pszSTDB_Lifetime				= "Lifetime";
static const char * const g_pszSTDB_FadeoutStart			= "FadeoutStart";
static const char * const g_pszSTDB_GravityScale			= "GravityScale";
static const char * const g_pszSTDB_ShatterMethod			= "ShatterMethod";
static const char * const g_pszSTDB_BounceStrength			= "BounceStrength";
static const char * const g_pszSTDB_ImmediateMinRotation	= "ImmediateMinRotation";
static const char * const g_pszSTDB_ImmediateMaxRotation	= "ImmediateMaxRotation";
static const char * const g_pszSTDB_IndirectMinRotation		= "IndirectMinRotation";
static const char * const g_pszSTDB_IndirectMaxRotation		= "IndirectMaxRotation";

static const char * const g_pszSTDB_GlassMinRadial				= "GlassMinFractures";
static const char * const g_pszSTDB_GlassMaxRadial				= "GlassMaxFractures";
static const char * const g_pszSTDB_GlassMinLength				= "GlassMinDebrisLength";
static const char * const g_pszSTDB_GlassMaxLength				= "GlassMaxDebrisLength";
static const char * const g_pszSTDB_GlassImmediateRadius		= "GlassImmediateRadius";
static const char * const g_pszSTDB_GlassImmediateMinVelocity	= "GlassImmediateMinVelocity";
static const char * const g_pszSTDB_GlassImmediateMaxVelocity	= "GlassImmediateMaxVelocity";
static const char * const g_pszSTDB_GlassImmediateFalloff		= "GlassImmediateFalloff";
static const char * const g_pszSTDB_GlassFallDelay				= "GlassFallDelay";
static const char * const g_pszSTDB_GlassFallDistanceDelay		= "GlassFallDistanceDelay";
static const char * const g_pszSTDB_GlassIndirectMinVelocity	= "GlassIndirectMinVelocity";
static const char * const g_pszSTDB_GlassIndirectMaxVelocity	= "GlassIndirectMaxVelocity";

static const char * const g_pszSTDB_TileUOffset				= "TileUOffset";
static const char * const g_pszSTDB_TileVOffset				= "TileVOffset";
static const char * const g_pszSTDB_TileWidth				= "TileUWidth";
static const char * const g_pszSTDB_TileHeight				= "TileVHeight";

//----------------------------------------------------------------------------
// CShatterTypeDB
//----------------------------------------------------------------------------

CShatterTypeDB::CShatterTypeDB() :	
	CGameDatabaseMgr(),
	m_hCatTypes(NULL)
{
}

CShatterTypeDB::~CShatterTypeDB()
{
}

//singleton support
CShatterTypeDB&CShatterTypeDB::Instance()
{
	static CShatterTypeDB s_Singleton;
	return s_Singleton;
}

//called to initialize the database from the provided database file
bool CShatterTypeDB::Init( const char *szDatabaseFile /* = DB_Default_File  */ )
{
	if( !OpenDatabase( szDatabaseFile ))
		return false;

	// Get handles to all of the categories in the database...
	m_hCatTypes = g_pLTDatabase->GetCategory( m_hDatabase, STDB_Category );
	if( !m_hCatTypes )
		return false;

	return true;
}

//cleans up the object and releases all database references it contains
void CShatterTypeDB::Term()
{
	m_hCatTypes = NULL;
}

//-------------------------------
// Shatter Type Access

//called to access a shatter type given a textual name. This will return NULL if none is found
HSHATTERTYPE CShatterTypeDB::GetShatterType( const char *pszType ) const
{
	return g_pLTDatabase->GetRecord( m_hCatTypes, pszType );
}

//called to access a record given an index. This will return NULL if the index is out of range
HSHATTERTYPE CShatterTypeDB::GetShatterType( uint32 nIndex ) const
{
	return g_pLTDatabase->GetRecordByIndex( m_hCatTypes, nIndex );
}

//called to get the number of shatter types there are in the database
uint32 CShatterTypeDB::GetNumTypes( )	const
{
	return g_pLTDatabase->GetNumRecords(m_hCatTypes);
}

//-------------------------------
// Common Properties

//called to get the material filename that should be used for the shattering type
const char* CShatterTypeDB::GetMaterialName(HSHATTERTYPE hType) const
{
	return GetString(hType, g_pszSTDB_Material, 0, ""); 
}

//determines if this should be rendered as a solid or translucent object
bool CShatterTypeDB::IsRenderSolid(HSHATTERTYPE hType) const
{
	return GetBool(hType, g_pszSTDB_RenderSolid, 0, false);
}

//determines the length that this glass is kept around
float CShatterTypeDB::GetLifetime(HSHATTERTYPE hType) const
{
	return GetFloat(hType, g_pszSTDB_Lifetime, 0, 10.0f);
}

//determines when the fading out of the object should begin
float CShatterTypeDB::GetFadeoutStart(HSHATTERTYPE hType) const
{
	return GetFloat(hType, g_pszSTDB_FadeoutStart, 0, 10.0f);
}

//determines the scale to apply to the gravity to allow user control over how fast they fall
float CShatterTypeDB::GetGravityScale(HSHATTERTYPE hType) const
{
	return GetFloat(hType, g_pszSTDB_GravityScale, 0, 1.0f);
}

//called to get the name of the shattering type that should be performed for this object
const char* CShatterTypeDB::GetShatterMethod(HSHATTERTYPE hType) const
{
	return GetString(hType, g_pszSTDB_ShatterMethod, 0, "");
}

//the amount of energy left after a bounce
float CShatterTypeDB::GetBounceStrength(HSHATTERTYPE hType) const
{
	return GetFloat(hType, g_pszSTDB_BounceStrength, 0, 0.1f);
}

//the amount of angular velocity to apply to the pieces of debris in the immediate
//impact, measured in degrees per second
float CShatterTypeDB::GetImmediateMinRotation(HSHATTERTYPE hType) const
{
	return GetFloat(hType, g_pszSTDB_ImmediateMinRotation, 0, -90.0f);
}

float CShatterTypeDB::GetImmediateMaxRotation(HSHATTERTYPE hType) const
{
	return GetFloat(hType, g_pszSTDB_ImmediateMaxRotation, 0, 90.0f);
}


//the amount of angular velocity to apply to the pieces of debris outside the immediate
//impact, measured in degrees per second
float CShatterTypeDB::GetIndirectMinRotation(HSHATTERTYPE hType) const
{
	return GetFloat(hType, g_pszSTDB_IndirectMinRotation, 0, -15.0f);
}

float CShatterTypeDB::GetIndirectMaxRotation(HSHATTERTYPE hType) const
{
	return GetFloat(hType, g_pszSTDB_IndirectMaxRotation, 0, 15.0f);
}


//-------------------------------
// Glass Shattering Properties

//called to access the range of radial fractures that can be created for the glass
uint32 CShatterTypeDB::GetGlassMinRadialFractures(HSHATTERTYPE hType) const
{
	return (uint32)GetInt32(hType, g_pszSTDB_GlassMinRadial, 0, 3);
}

uint32 CShatterTypeDB::GetGlassMaxRadialFractures(HSHATTERTYPE hType) const
{
	return (uint32)GetInt32(hType, g_pszSTDB_GlassMaxRadial, 0, 3);
}

//called to access the range for the valid debris lengths
float CShatterTypeDB::GetGlassMinDebrisLength(HSHATTERTYPE hType) const
{
	return GetFloat(hType, g_pszSTDB_GlassMinLength, 0, 10.0f);
}

float CShatterTypeDB::GetGlassMaxDebrisLength(HSHATTERTYPE hType) const
{
	return GetFloat(hType, g_pszSTDB_GlassMaxLength, 0, 100.0f);
}

//the radius where forces are immediately applied around the impact vector
float CShatterTypeDB::GetGlassImmediateRadius(HSHATTERTYPE hType) const
{
	return GetFloat(hType, g_pszSTDB_GlassImmediateRadius, 0, 5.0f);
}

//the maximum velocity to apply to pieces in the immediate radius
float CShatterTypeDB::GetGlassImmediateMinVelocity(HSHATTERTYPE hType) const
{
	return GetFloat(hType, g_pszSTDB_GlassImmediateMinVelocity, 0, 50.0f);
}

float CShatterTypeDB::GetGlassImmediateMaxVelocity(HSHATTERTYPE hType) const
{
	return GetFloat(hType, g_pszSTDB_GlassImmediateMaxVelocity, 0, 100.0f);
}

//the falloff to apply as distance increases from the impact point
float CShatterTypeDB::GetGlassImmediateFalloff(HSHATTERTYPE hType) const
{
	return GetFloat(hType, g_pszSTDB_GlassImmediateFalloff, 0, 0.01f);
}

//a base delay to apply to the falling of any pieces outside the immediate radius
float CShatterTypeDB::GetGlassFallDelay(HSHATTERTYPE hType) const
{
	return GetFloat(hType, g_pszSTDB_GlassFallDelay, 0, 0.1f);
}

//an additional delay for non-immediate pieces as the distance increases from the impact
float CShatterTypeDB::GetGlassFallDistanceDelay(HSHATTERTYPE hType) const
{
	return GetFloat(hType, g_pszSTDB_GlassFallDistanceDelay, 0, 0.005f);
}

//the velocity range to use for the the indirect pieces
float CShatterTypeDB::GetGlassIndirectMinVelocity(HSHATTERTYPE hType) const
{
	return GetFloat(hType, g_pszSTDB_GlassIndirectMinVelocity, 0, -3.0f);
}

float CShatterTypeDB::GetGlassIndirectMaxVelocity(HSHATTERTYPE hType) const
{
	return GetFloat(hType, g_pszSTDB_GlassIndirectMaxVelocity, 0, 3.0f);
}

//-------------------------------
// Tile Shattering Properties

//in texture space, the corner to start the tile separation from
float CShatterTypeDB::GetTileUOffset(HSHATTERTYPE hType) const
{
	return GetFloat(hType, g_pszSTDB_TileUOffset, 0, 0.0f);
}

float CShatterTypeDB::GetTileVOffset(HSHATTERTYPE hType) const
{
	return GetFloat(hType, g_pszSTDB_TileVOffset, 0, 0.0f);
}

//in texture space, the dimensions of a single tile
float CShatterTypeDB::GetTileWidth(HSHATTERTYPE hType) const
{
	return GetFloat(hType, g_pszSTDB_TileWidth, 0, 1.0f);
}

float CShatterTypeDB::GetTileHeight(HSHATTERTYPE hType) const
{
	return GetFloat(hType, g_pszSTDB_TileHeight, 0, 1.0f);
}



//----------------------------------------------------------------------------
// CShatterTypeDBPlugin
//----------------------------------------------------------------------------

#ifdef _SERVERBUILD // Server-side only

CShatterTypeDBPlugin& CShatterTypeDBPlugin::Instance()
{
	static CShatterTypeDBPlugin s_Singleton;
	return s_Singleton;
}

//called by WorldEdit to fill in the list of strings for a specified property
void CShatterTypeDBPlugin::PopulateStringList(char** aszStrings, uint32* pcStrings,
												const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	if( !aszStrings || !pcStrings )
	{
		LTERROR( "CNavMarkerTypeDBPlugin::PopulateStringList: Invalid input parameters" );
		return;
	}

	//add an entry for the 'None' option as well just to make it more user friendly
	if(*pcStrings < cMaxStrings)
	{
		LTStrCpy( aszStrings[(*pcStrings)++], "None", cMaxStringLength );
	}

	// Add an entry for each type combination...
	uint32 nNumTypes = CShatterTypeDB::Instance().GetNumTypes();

	for( uint32 nType = 0; nType < nNumTypes; ++nType )
	{
		//see if we have run out of room
		if(*pcStrings >= cMaxStrings)
		{
			//we are out of room, so throw a message to aid engineers, but exit safely
			LTERROR( "Too many types to fit in the list.  Enlarge list size?" );
			return;
		}

		HSHATTERTYPE hType = CShatterTypeDB::Instance().GetShatterType( nType );
		if( !hType )
			continue;

		const char *pszTypeName = CShatterTypeDB::Instance().GetRecordName( hType );
		if( !pszTypeName )
			continue;

		LTStrCpy( aszStrings[(*pcStrings)++], pszTypeName, cMaxStringLength );
	}
}


#endif // _SERVERBUILD
