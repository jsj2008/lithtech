//----------------------------------------------------------------------------
//
// ShatterTypeDB.h
//
// Provides support for accessing all of the associated properties found within
// the game database with information about the different types of shattering
// effects.
//
//----------------------------------------------------------------------------

#ifndef __SHATTERTYPEDB_H__
#define __SHATTERTYPEDB_H__

// Includes...
#include "GameDatabaseMgr.h"



typedef HRECORD	HSHATTERTYPE;

//----------------------------------------------------------------------------
// CShatterTypeDB
//  The actual database of the different shatter types and their associated
// parameters
//----------------------------------------------------------------------------
class CShatterTypeDB : 
	public CGameDatabaseMgr
{
public :	

	//singleton support
	static CShatterTypeDB& Instance();

	//called to initialize the database from the provided database file
	bool	Init(const char *szDatabaseFile = DB_Default_File);

	//cleans up the object and releases all database references it contains
	void	Term();

	//-------------------------------
	// Shatter Type Access

	//called to access a shatter type given a textual name. This will return NULL if none is found
	HSHATTERTYPE	GetShatterType( const char *pszType ) const;

	//called to access a record given an index. This will return NULL if the index is out of range
	HSHATTERTYPE	GetShatterType( uint32 nIndex ) const;

	//called to access the category that all the shatter types are contained within
	HCATEGORY		GetShatterCategory() const			{ return m_hCatTypes; }

	//called to get the number of shatter types there are in the database
	uint32			GetNumTypes( )	const;

	//-------------------------------
	// Common Properties

	//called to get the material filename that should be used for the shattering type
	const char*		GetMaterialName(HSHATTERTYPE hType) const;

	//determines if this should be rendered as a solid or translucent object
	bool			IsRenderSolid(HSHATTERTYPE hType) const;

	//determines the length that this glass is kept around
	float			GetLifetime(HSHATTERTYPE hType) const;

	//determines when the fading out of the object should begin
	float			GetFadeoutStart(HSHATTERTYPE hType) const;

	//determines the scale to apply to the gravity to allow user control over how fast they fall
	float			GetGravityScale(HSHATTERTYPE hType) const;

	//called to get the name of the shattering type that should be performed for this object
	const char*		GetShatterMethod(HSHATTERTYPE hType) const;

	//the amount of energy left after a bounce
	float			GetBounceStrength(HSHATTERTYPE hType) const;

	//the amount of angular velocity to apply to the pieces of debris in the immediate
	//impact, measured in degrees per second
	float			GetImmediateMinRotation(HSHATTERTYPE hType) const;
	float			GetImmediateMaxRotation(HSHATTERTYPE hType) const;

	//the amount of angular velocity to apply to the pieces of debris outside the immediate
	//impact, measured in degrees per second
	float			GetIndirectMinRotation(HSHATTERTYPE hType) const;
	float			GetIndirectMaxRotation(HSHATTERTYPE hType) const;

	//-------------------------------
	// Glass Shattering Properties

	//called to access the range of radial fractures that can be created for the glass
	uint32			GetGlassMinRadialFractures(HSHATTERTYPE hType) const;
	uint32			GetGlassMaxRadialFractures(HSHATTERTYPE hType) const;

	//called to access the range for the valid debris lengths
	float			GetGlassMinDebrisLength(HSHATTERTYPE hType) const;
	float			GetGlassMaxDebrisLength(HSHATTERTYPE hType) const;

	//the radius where forces are immediately applied around the impact vector
	float			GetGlassImmediateRadius(HSHATTERTYPE hType) const;

	//the maximum velocity to apply to pieces in the immediate radius
	float			GetGlassImmediateMinVelocity(HSHATTERTYPE hType) const;
	float			GetGlassImmediateMaxVelocity(HSHATTERTYPE hType) const;

	//the falloff to apply as distance increases from the impact point
	float			GetGlassImmediateFalloff(HSHATTERTYPE hType) const;

	//a base delay to apply to the falling of any pieces outside the immediate radius
	float			GetGlassFallDelay(HSHATTERTYPE hType) const;

	//an additional delay for non-immediate pieces as the distance increases from the impact
	float			GetGlassFallDistanceDelay(HSHATTERTYPE hType) const;

	//the velocity range to use for the the indirect pieces
	float			GetGlassIndirectMinVelocity(HSHATTERTYPE hType) const;
	float			GetGlassIndirectMaxVelocity(HSHATTERTYPE hType) const;

	//-------------------------------
	// Tile Shattering Properties

	//in texture space, the corner to start the tile separation from
	float			GetTileUOffset(HSHATTERTYPE hType) const;
	float			GetTileVOffset(HSHATTERTYPE hType) const;

	//in texture space, the dimensions of a single tile
	float			GetTileWidth(HSHATTERTYPE hType) const;
	float			GetTileHeight(HSHATTERTYPE hType) const;
	
private	:	

	//lifetime operators are private to prevent issues with the singleton
	CShatterTypeDB();
	virtual ~CShatterTypeDB();
	PREVENT_OBJECT_COPYING(CShatterTypeDB);

	//keep track of the category that contains all of our records
	HCATEGORY	m_hCatTypes;
};


//----------------------------------------------------------------------------
// CShatterTypeDBPlugin
//  Used to help facilitate populating the WorldEdit object properties with
// a list of the different shatter types
//----------------------------------------------------------------------------
#ifdef _SERVERBUILD

#include "iobjectplugin.h"

class CShatterTypeDBPlugin : 
	public IObjectPlugin
{
public:

	//singleton support
	static CShatterTypeDBPlugin& Instance();

	//called by WorldEdit to fill in the list of strings for a specified property
	static void PopulateStringList(char** aszStrings, uint32* pcStrings,
		const uint32 cMaxStrings, const uint32 cMaxStringLength);

private:

	//lifetime operators are private since we only support a singleton
	CShatterTypeDBPlugin()					{}
	virtual ~CShatterTypeDBPlugin()			{}

	PREVENT_OBJECT_COPYING(CShatterTypeDBPlugin);
};

#endif // _SERVERBUILD


#endif // __SHATTERTYPEDB_H__
