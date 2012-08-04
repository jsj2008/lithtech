// ----------------------------------------------------------------------- //
//
// MODULE  : ModelsDB.cpp
//
// PURPOSE : Implementation of the models database.
//
// CREATED : 03/09/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "Stdafx.h"
#include "ModelsDB.h"
#include "AnimationContext.h"
#include "AnimationPropStrings.h"
#include "crc32utils.h"
#include "ltfileoperations.h"
#include "iltfilemgr.h"

#if defined( _CLIENTBUILD )
#include "..\ClientShellDll\GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;
#else
#endif


//
// Defines...
//

// ModelsDB categories
const char* const ModelsDB_ModelsCat =			"Character/Models";
const char* const ModelsDB_NodesCat =			"Character/Models/Skeletons/Nodes";
const char* const ModelsDB_SkeletonCat =		"Character/Models/Skeletons";
const char* const ModelsDB_SocketSetCat =		"Character/Models/SocketSets";
const char* const ModelsDB_GlobalCat =			"Character/Models/Global";
const char* const ModelsDB_SeverDamageCat =		"Character/Models/Sever/SeverDamage";
const char* const ModelsDB_SeverPieceCat =		"Character/Models/Sever/SeverPiece";
const char* const ModelsDB_SeverBodyCat =		"Character/Models/Sever/SeverBody";
const char* const ModelsDB_ModelDecalSetCat =	"Character/Models/ModelDecalSet";
const char* const ModelsDB_LeanCat =			"Character/Models/Lean";
const char* const ModelsDB_SyncActionCat =		"Character/Models/SyncAction";
	
//
// Globals...
//

ModelsDB* g_pModelsDB = NULL;

#if defined( _SERVERBUILD ) || defined( _CLIENTBUILD )
VarTrack g_vtBodySeverTest;
VarTrack g_vtSkeletonNodeDebug;
#endif


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::ModelsDB()
//
//	PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

ModelsDB::ModelsDB()
{
	m_hModelsCat = NULL;
	m_hNodesCat = NULL;
	m_hSkeletonCat = NULL;
	m_hSocketSetsCat = NULL;
	m_hSeverDamageCat = NULL;
	m_hSeverPieceCat = NULL;
	m_hSeverBodyCat = NULL;
	m_hModelDecalSetCat = NULL;
	m_hLeanCat = NULL;
	m_hSyncActionCat = NULL;
	m_hGlobalRecord = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::~ModelsDB()
//
//	PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

ModelsDB::~ModelsDB( )
{
	g_pModelsDB = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::Init()
//
//	PURPOSE:	Initialize the database...
//
// ----------------------------------------------------------------------- //
bool ModelsDB::Init( const char *szDatabaseFile /* = FDB_Default_File  */ )
{
	if( !OpenDatabase( szDatabaseFile ))
	{
		LTERROR( "ModelsDB::Init - Couldn't open database." );
		return false;
	}

	// Set the global database pointer...
	g_pModelsDB = this;

	// Get handles to all of the categories in the database...
	m_hModelsCat = g_pLTDatabase->GetCategory(m_hDatabase, ModelsDB_ModelsCat);
	if( !m_hModelsCat )
	{
		LTERROR( "ModelsDB::Init - Couldn't initialize Models category." );
		return false;
	}

	m_hNodesCat = g_pLTDatabase->GetCategory(m_hDatabase, ModelsDB_NodesCat);
	if( !m_hNodesCat )
	{
		LTERROR( "ModelsDB::Init - Couldn't initialize Nodes category." );
		return false;
	}

	m_hSkeletonCat = g_pLTDatabase->GetCategory(m_hDatabase, ModelsDB_SkeletonCat);
	if( !m_hSkeletonCat )
	{
		LTERROR( "ModelsDB::Init - Couldn't initialize Skeletons category." );
		return false;
	}

	m_hSocketSetsCat = g_pLTDatabase->GetCategory(m_hDatabase, ModelsDB_SocketSetCat);
	if( !m_hSocketSetsCat )
	{
		LTERROR( "ModelsDB::Init - Couldn't initialize SocketSets category." );
		return false;
	}

	m_hSeverDamageCat = g_pLTDatabase->GetCategory(m_hDatabase, ModelsDB_SeverDamageCat);
	if( !m_hSeverDamageCat )
	{
		LTERROR( "ModelsDB::Init - Couldn't initialize SeverDamage category." );
		return false;
	}

	m_hSeverPieceCat = g_pLTDatabase->GetCategory(m_hDatabase, ModelsDB_SeverPieceCat);
	if( !m_hSeverPieceCat )
	{
		LTERROR( "ModelsDB::Init - Couldn't initialize SeverPiece category." );
		return false;
	}

	m_hSeverBodyCat = g_pLTDatabase->GetCategory(m_hDatabase, ModelsDB_SeverBodyCat);
	if( !m_hSeverBodyCat )
	{
		LTERROR( "ModelsDB::Init - Couldn't initialize SeverBody category." );
		return false;
	}

	m_hModelDecalSetCat = g_pLTDatabase->GetCategory(m_hDatabase, ModelsDB_ModelDecalSetCat);
	if( !m_hModelDecalSetCat )
	{
		LTERROR( "ModelsDB::Init - Couldn't initialize ModelDecalSet category." );
		return false;
	}

	m_hLeanCat = g_pLTDatabase->GetCategory( m_hDatabase, ModelsDB_LeanCat );
	if( !m_hLeanCat )
	{
		LTERROR( "ModelsDB::Init - Couldn't initialize Lean category." );
		return false;
	}

	m_hSyncActionCat = g_pLTDatabase->GetCategory( m_hDatabase, ModelsDB_SyncActionCat );
	if( !m_hSyncActionCat )
	{
		LTERROR( "ModelsDB::Init - Couldn't initialize SyncAction category." );
		return false;
	}

	HCATEGORY hGlobalCat = g_pLTDatabase->GetCategory(m_hDatabase, ModelsDB_GlobalCat);
	m_hGlobalRecord = g_pLTDatabase->GetRecordByIndex( hGlobalCat, 0 );
	if( !m_hGlobalRecord )
	{
		LTERROR( "ModelsDB::Init - Couldn't initialize Global category." );
		return false;
	}

	SERVER_AND_CLIENT_CODE
	(
		SERVER_CODE
		(
			if (g_pLTBase && !g_vtBodySeverTest.IsInitted()) 
			{
				g_vtBodySeverTest.Init(g_pLTBase, "BodySeverTest", NULL, 0.0f);
			}
		)



		if( g_pLTBase && !g_vtSkeletonNodeDebug.IsInitted( ))
		{
			g_vtSkeletonNodeDebug.Init( g_pLTBase, "SkeletonNodeDebug", NULL, 0.0f );
		}
	)

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetHModelsByRecordName()
//
//	PURPOSE:	Gets the HMODEL object by gdb record name.
//
// ----------------------------------------------------------------------- //
ModelsDB::HMODEL ModelsDB::GetModelByRecordName( char const* pszRecordName ) const
{
	return ( HMODEL )GetRecord( m_hModelsCat, pszRecordName );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetNumModels()
//
//	PURPOSE:	Gets the number of models are available.
//
// ----------------------------------------------------------------------- //
uint32 ModelsDB::GetNumModels( ) const
{
	return g_pLTDatabase->GetNumRecords( m_hModelsCat );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetModel()
//
//	PURPOSE:	Gets the model by index.
//
// ----------------------------------------------------------------------- //
ModelsDB::HMODEL ModelsDB::GetModel( uint32 nIndex ) const
{
	return ( HMODEL )g_pLTDatabase->GetRecordByIndex( m_hModelsCat, nIndex );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetRecordNameOfHModel()
//
//	PURPOSE:	Gets the record name of an HMODEL
//
// ----------------------------------------------------------------------- //
char const* ModelsDB::GetRecordNameOfHModel( HMODEL hModel ) const
{
	return GetRecordName( hModel );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetModelSoundTemplate()
//
//	PURPOSE:	Gets the soundtemplate.
//
// ----------------------------------------------------------------------- //
HRECORD ModelsDB::GetModelSoundTemplate( HMODEL hModel ) const
{
	return GetRecordLink( hModel, "SoundTemplate" );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetModelSkeleton()
//
//	PURPOSE:	Gets the skeleton.
//
// ----------------------------------------------------------------------- //
ModelsDB::HSKELETON ModelsDB::GetModelSkeleton( HMODEL hModel ) const
{
	return ( HSKELETON )GetRecordLink( hModel, "Skeleton" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetNumModelAnimationTrees()
//
//	PURPOSE:	Gets the number of animation trees.
//
// ----------------------------------------------------------------------- //
uint32 ModelsDB::GetNumModelAnimationTrees( HMODEL hModel ) const
{
	return GetNumValues( hModel, "AnimationTree" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetModelAnimationTree()
//
//	PURPOSE:	Gets the animation tree.
//
// ----------------------------------------------------------------------- //
char const* ModelsDB::GetModelAnimationTree( HMODEL hModel, uint32 iTree ) const
{
	return GetString( hModel, "AnimationTree", iTree );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetModelAIName()
//
//	PURPOSE:	Gets the ainame.
//
// ----------------------------------------------------------------------- //
char const* ModelsDB::GetModelAIName( HMODEL hModel ) const
{
	HRECORD hRecord = GetRecordLink(hModel, "AIName");
	return GetRecordName( hRecord );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetModelType()
//
//	PURPOSE:	Gets the mass.
//
// ----------------------------------------------------------------------- //
float ModelsDB::GetModelMass(HMODEL hModel) const
{
	return GetFloat( hModel, "Mass" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetModelHitPoints()
//
//	PURPOSE:	Gets the hitpoints.
//
// ----------------------------------------------------------------------- //
float ModelsDB::GetModelHitPoints(HMODEL hModel) const
{
	return GetFloat( hModel, "HitPoints" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetModelMaxHitPoints()
//
//	PURPOSE:	Gets the maxhitpoints.
//
// ----------------------------------------------------------------------- //
float ModelsDB::GetModelMaxHitPoints(HMODEL hModel) const
{
	return GetFloat( hModel, "MaxHitPoints" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetModelArmor()
//
//	PURPOSE:	Gets the armor.
//
// ----------------------------------------------------------------------- //
float ModelsDB::GetModelArmor(HMODEL hModel) const
{
	return GetFloat( hModel, "Armor" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetModelMaxArmor()
//
//	PURPOSE:	Gets the maxarmor.
//
// ----------------------------------------------------------------------- //
float ModelsDB::GetModelMaxArmor(HMODEL hModel) const
{
	return GetFloat( hModel, "MaxArmor" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetModelLoudMovementSnd()
//
//	PURPOSE:	Gets the loud movement sound.
//
// ----------------------------------------------------------------------- //
HRECORD ModelsDB::GetModelLoudMovementSnd(HMODEL hModel) const
{
	return GetRecordLink( hModel, "LoudMovementSound" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetModelQuietMovementSnd()
//
//	PURPOSE:	Gets the quiet movement sound.
//
// ----------------------------------------------------------------------- //
HRECORD ModelsDB::GetModelQuietMovementSnd(HMODEL hModel) const
{
	return GetRecordLink( hModel, "QuietMovementSound" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetModelDeathGibSnd()
//
//	PURPOSE:	Gets the death gib sound.
//
// ----------------------------------------------------------------------- //
HRECORD ModelsDB::GetModelDeathGibSnd(HMODEL hModel) const
{
	return GetRecordLink( hModel, "DeathGibSound" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetModelDeathDecapitateSnd()
//
//	PURPOSE:	Gets the death gib sound.
//
// ----------------------------------------------------------------------- //
HRECORD ModelsDB::GetModelDeathDecapitateSnd(HMODEL hModel) const
{
	return GetRecordLink( hModel, "DeathDecapitateSound" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetModelNameId()
//
//	PURPOSE:	Gets the nameid.
//
// ----------------------------------------------------------------------- //
const char* ModelsDB::GetModelNameId(HMODEL hModel) const
{
	const char* szStringID = GetString( hModel, "NameId" );
	if( !szStringID )
		return "";

	return szStringID;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetModelSocketSet()
//
//	PURPOSE:	Gets the socketset.
//
// ----------------------------------------------------------------------- //
ModelsDB::HSOCKETSET ModelsDB::GetModelSocketSet(HMODEL hModel) const
{
	return GetRecordLink( hModel, "SocketSet" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetDefaultPhysicsWeightSet()
//
//	PURPOSE:	Gets the default physics weight set.
//
// ----------------------------------------------------------------------- //
char const* ModelsDB::GetDefaultPhysicsWeightSet(HMODEL hModel) const
{
	return GetString( hModel, "DefaultPhysicsWeightSet" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetModelFilename() const
//
//	PURPOSE:	Gets the model filename.
//
// ----------------------------------------------------------------------- //
char const* ModelsDB::GetModelFilename(HMODEL hModel) const
{
	return GetString( hModel, "ModelFile" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetNumMaterials()
//
//	PURPOSE:	Gets the number of material filenames.
//
// ----------------------------------------------------------------------- //
uint32 ModelsDB::GetNumMaterials( HMODEL hModel ) const
{
	return GetNumValues( hModel, "Material" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetMaterialFilename()
//
//	PURPOSE:	Gets the material filename.
//
// ----------------------------------------------------------------------- //
char const* ModelsDB::GetMaterialFilename(HMODEL hModel, uint8 iMaterial) const
{
	return GetString( hModel, "Material", iMaterial );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::CopyMaterialFilenames()
//
//	PURPOSE:	Gets all the material filenames.
//
// ----------------------------------------------------------------------- //
void ModelsDB::CopyMaterialFilenames(HMODEL hModel, char* paszDest, uint32 nNumValues, uint32 nStrLen) const
{
	CopyStringValues( hModel, "Material", paszDest, nNumValues, nStrLen );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetHandsMaterialFilename()
//
//	PURPOSE:	Gets the hands material filename.
//
// ----------------------------------------------------------------------- //
char const* ModelsDB::GetHandsMaterialFilename( HMODEL hModel ) const
{
	return GetString( hModel, "HandsMaterial" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetNumPersistentClientFX()
//
//	PURPOSE:	Gets the number of ClientFX specified...
//
// ----------------------------------------------------------------------- //
uint32 ModelsDB::GetNumPersistentClientFX( HMODEL hModel ) const
{
	return GetNumValues( hModel, "PersistentClientFX" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetPersistentClientFXName()
//
//	PURPOSE:	Gets the ClientFX...
//
// ----------------------------------------------------------------------- //
char const* ModelsDB::GetPersistentClientFXName( HMODEL hModel, uint32 nClientFX ) const
{
	HATTRIBUTE hPersistentFX = GetAttribute( hModel, "PersistentClientFX" );
	HATTRIBUTE hClientFX = GetStructAttribute( hPersistentFX, nClientFX, "ClientFX" );
	return GetString( hClientFX );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetPersistentClientFXVisible()
//
//	PURPOSE:	Gets the ClientFX Visible flag...
//
// ----------------------------------------------------------------------- //
bool ModelsDB::GetPersistentClientFXVisible( HMODEL hModel, uint32 nClientFX ) const
{
	HATTRIBUTE hPersistentFX = GetAttribute( hModel, "PersistentClientFX" );
	HATTRIBUTE hClientFX = GetStructAttribute( hPersistentFX, nClientFX, "Visible" );
	return GetBool( hClientFX );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetPersistentClientFXLoop()
//
//	PURPOSE:	Gets the ClientFX Loop flag...
//
// ----------------------------------------------------------------------- //
bool ModelsDB::GetPersistentClientFXLoop( HMODEL hModel, uint32 nClientFX ) const
{
	HATTRIBUTE hPersistentFX = GetAttribute( hModel, "PersistentClientFX" );
	HATTRIBUTE hClientFX = GetStructAttribute( hPersistentFX, nClientFX, "Loop" );
	return GetBool( hClientFX );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetPersistentClientFXSmoothShutdown()
//
//	PURPOSE:	Gets the ClientFX SmoothShutdown flag...
//
// ----------------------------------------------------------------------- //
bool ModelsDB::GetPersistentClientFXSmoothShutdown( HMODEL hModel, uint32 nClientFX ) const
{
	HATTRIBUTE hPersistentFX = GetAttribute( hModel, "PersistentClientFX" );
	HATTRIBUTE hClientFX = GetStructAttribute( hPersistentFX, nClientFX, "SmoothShutdown" );
	return GetBool( hClientFX );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetNumAltHeadMaterials()
//
//	PURPOSE:	Gets the number of alternate head materials.
//
// ----------------------------------------------------------------------- //
uint32 ModelsDB::GetNumAltHeadMaterials( HMODEL hModel ) const
{
	return GetNumValues( hModel, "AltHeadMaterial" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetAltHeadMaterial()
//
//	PURPOSE:	Gets the alternate head material.
//
// ----------------------------------------------------------------------- //
char const* ModelsDB::GetAltHeadMaterial(HMODEL hModel, uint8 iMaterial) const
{
	return GetString( hModel, "AltHeadMaterial", iMaterial );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetNumAltBodyMaterials()
//
//	PURPOSE:	Gets the number of alternate body materials.
//
// ----------------------------------------------------------------------- //
uint32 ModelsDB::GetNumAltBodyMaterials( HMODEL hModel ) const
{
	return GetNumValues( hModel, "AltBodyMaterial" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetAltBodyMaterial()
//
//	PURPOSE:	Gets the alternate body material.
//
// ----------------------------------------------------------------------- //
char const* ModelsDB::GetAltBodyMaterial(HMODEL hModel, uint8 nIndex) const
{
	return GetString( hModel, "AltBodyMaterial", nIndex );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::ShowGameStartPoint()
//
//	PURPOSE:	Checks if model should be shown on GameStartPoint
//
// ----------------------------------------------------------------------- //
bool ModelsDB::ShowGameStartPoint( HMODEL hModel ) const
{
	return GetBool( hModel, "ShowGameStartPoint" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::IsModelTranslucent()
//
//	PURPOSE:	Checks if model is set to translucent.
//
// ----------------------------------------------------------------------- //
bool ModelsDB::IsModelTranslucent( HMODEL hModel ) const
{
	return GetBool( hModel, "Translucent" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetArmorSurfaceType()
//
//	PURPOSE:	Gets the surface type for armored character.
//
// ----------------------------------------------------------------------- //
SurfaceType ModelsDB::GetArmorSurfaceType( HMODEL hModel ) const
{
	return g_pSurfaceDB->GetSurfaceType( GetRecordLink( hModel, "ArmorSurfaceType" ));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetFleshSurfaceType()
//
//	PURPOSE:	Gets the surface type for unarmored character.
//
// ----------------------------------------------------------------------- //
SurfaceType ModelsDB::GetFleshSurfaceType( HMODEL hModel ) const
{
	return g_pSurfaceDB->GetSurfaceType( GetRecordLink( hModel, "FleshSurfaceType" ));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetCollisionProperty()
//
//	PURPOSE:	Gets the CollisionProperty from the model.
//
// ----------------------------------------------------------------------- //
HRECORD ModelsDB::GetCollisionProperty( HMODEL hModel ) const
{
	return GetRecordLink( hModel, "CollisionProperty" );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetNumDefaultAttachments()
//
//	PURPOSE:	Gets the number of default attachments.
//
// ----------------------------------------------------------------------- //
uint32 ModelsDB::GetNumDefaultAttachments( HMODEL hModel ) const
{
	return GetNumValues( hModel, "DefaultAttachment" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetDefaultAttachment()
//
//	PURPOSE:	Gets the default attachment.
//
// ----------------------------------------------------------------------- //
char const* ModelsDB::GetDefaultAttachment(HMODEL hModel, uint32 nIndex) const
{
	HRECORD hRecord = GetRecordLink( hModel, "DefaultAttachment", nIndex );
	return g_pLTDatabase->GetRecordName( hRecord );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetNumDefaultWeapons()
//
//	PURPOSE:	Gets the number of default weapons.
//
// ----------------------------------------------------------------------- //
uint32 ModelsDB::GetNumDefaultWeapons( HMODEL hModel ) const
{
	return GetNumValues( hModel, "DefaultWeapon" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetDefaultWeapon()
//
//	PURPOSE:	Gets the default weapon.
//
// ----------------------------------------------------------------------- //
void ModelsDB::GetDefaultWeapon(HMODEL hModel, uint32 nIndex, const char*& pszWeaponPosition, HRECORD& hWeapon ) const
{
	hWeapon = GetRecordLink( hModel, "DefaultWeapon", nIndex );
	pszWeaponPosition = GetString( hModel, "DefaultWeaponSocket", nIndex );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetNumRequiredWeapons()
//
//	PURPOSE:	Gets the number of required weapons.
//
// ----------------------------------------------------------------------- //
uint32 ModelsDB::GetNumRequiredWeapons( HMODEL hModel ) const
{
	return GetNumValues( hModel, "RequiredWeapon" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetRequiredWeapon()
//
//	PURPOSE:	Gets the required weapon.
//
// ----------------------------------------------------------------------- //
void ModelsDB::GetRequiredWeapon(HMODEL hModel, uint32 nIndex, const char*& pszWeaponPosition, HRECORD& hWeapon ) const
{
	hWeapon = GetRecordLink( hModel, "RequiredWeapon", nIndex );
	pszWeaponPosition = GetString( hModel, "RequiredWeaponSocket", nIndex );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetNumPlayerViewAttachments()
//
//	PURPOSE:	Gets the number of pv attachments.
//
// ----------------------------------------------------------------------- //
uint32 ModelsDB::GetNumPlayerViewAttachments( HMODEL hModel ) const
{
	return GetNumValues( hModel, "PlayerViewAttachment" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetPlayerViewAttachment()
//
//	PURPOSE:	Gets the pv attachment.
//
// ----------------------------------------------------------------------- //
void ModelsDB::GetPlayerViewAttachment(HMODEL hModel, uint32 nIndex, const char*& pszPVAttachmentPosition, HRECORD& hPVAttachment ) const
{
	hPVAttachment = GetRecordLink( hModel, "PlayerViewAttachment", nIndex );
	pszPVAttachmentPosition = GetString( hModel, "PlayerViewAttachmentSocket", nIndex );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetUnalertDamageFactor()
//
//	PURPOSE:	Gets the unalert damage factor.
//
// ----------------------------------------------------------------------- //
float ModelsDB::GetUnalertDamageFactor( HMODEL hModel ) const
{
	return GetFloat( hModel, "UnalertDamageFactor" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetSeverBodyRecord
//
//	PURPOSE:	Gets the data required to sever limbs
//
// ----------------------------------------------------------------------- //
ModelsDB::HSEVERBODY ModelsDB::GetSeverBodyRecord( HMODEL hModel ) const
{
	return (HSEVERBODY) GetRecordLink( hModel, "SeverBody" );
};


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::CanGib
//
//	PURPOSE:	Can this model be gibbed
//
// ----------------------------------------------------------------------- //
bool ModelsDB::CanGib( HMODEL hModel ) const
{
	return GetBool(hModel,"CanGib");
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::AlwaysGib
//
//	PURPOSE:	Should this model gib on every death
//
// ----------------------------------------------------------------------- //
bool ModelsDB::AlwaysGib( HMODEL hModel ) const
{
	return GetBool(hModel,"AlwaysGib");
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetGibFX
//
//	PURPOSE:	Get the FX for gibbing this model
//
// ----------------------------------------------------------------------- //
const char* ModelsDB::GetGibFX( HMODEL hModel ) const
{
	return GetString(hModel,"GibFX");
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetInterfaceFX
//
//	PURPOSE:	Get the FX to show in the interface for this model
//
// ----------------------------------------------------------------------- //
const char* ModelsDB::GetInterfaceFX( HMODEL hModel ) const
{
	return GetString(hModel,"InterfaceFX");
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetNumDeathFX
//
//	PURPOSE:	Get the number of special death effects for this model
//
// ----------------------------------------------------------------------- //
uint32 ModelsDB::GetNumDeathFX( HMODEL hModel) const
{
	return GetNumValues( hModel, "DeathFX" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetDeathFXRecord
//
//	PURPOSE:	Get the special death effects for this model
//
// ----------------------------------------------------------------------- //
HRECORD ModelsDB::GetDeathFXRecord( HMODEL hModel, uint8 iFX ) const
{
	return GetRecordLink( hModel, "DeathFX", iFX );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetSplashesFootstepImpulse() const
//
//	PURPOSE:	Gets the splashes information for footstep impulse.
//
// ----------------------------------------------------------------------- //
float ModelsDB::GetSplashesFootstepImpulse( HMODEL hModel ) const
{
	HATTRIBUTE hSplashes = GetAttribute( hModel, "Splashes" );
	HATTRIBUTE hImpulse = GetStructAttribute( hSplashes, 0, "FootstepImpulse" );
	return GetFloat( hImpulse );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetSplashesJumpingShallowImpulse() const
//
//	PURPOSE:	Gets the splashes information for footstep impulse.
//
// ----------------------------------------------------------------------- //
float ModelsDB::GetSplashesJumpingShallowImpulse( HMODEL hModel ) const
{
	HATTRIBUTE hSplashes = GetAttribute( hModel, "Splashes" );
	HATTRIBUTE hImpulse = GetStructAttribute( hSplashes, 0, "JumpingShallowImpulse" );
	return GetFloat( hImpulse );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetSplashesJumpingDeepImpulse() const
//
//	PURPOSE:	Gets the splashes information for footstep impulse.
//
// ----------------------------------------------------------------------- //
float ModelsDB::GetSplashesJumpingDeepImpulse( HMODEL hModel ) const
{
	HATTRIBUTE hSplashes = GetAttribute( hModel, "Splashes" );
	HATTRIBUTE hImpulse = GetStructAttribute( hSplashes, 0, "JumpingDeepImpulse" );
	return GetFloat( hImpulse );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetDMModelsAttribute
//
//	PURPOSE:	Gets the dm models attributel
//
// ----------------------------------------------------------------------- //
HATTRIBUTE ModelsDB::GetDMModelsAttribute( ) const
{
	return GetAttribute( m_hGlobalRecord, "DeathmatchModels" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetNumDMModels() const
//
//	PURPOSE:	Gets the number of dm models.
//
// ----------------------------------------------------------------------- //
uint32 ModelsDB::GetNumDMModels( ) const
{
	return g_pLTDatabase->GetNumValues( GetDMModelsAttribute());
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetDMModel() const
//
//	PURPOSE:	Gets the deathmatch model by index.
//
// ----------------------------------------------------------------------- //
ModelsDB::HMODEL ModelsDB::GetDMModel( uint32 nIndex ) const
{
	return ( HMODEL )g_pLTDatabase->GetRecordLink( GetDMModelsAttribute(), nIndex, NULL );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetNumTeamModels() const
//
//	PURPOSE:	Gets the number of team models available.
//
// ----------------------------------------------------------------------- //
uint32 ModelsDB::GetNumTeamModels( ) const
{
	HATTRIBUTE hTeamModelsStruct = g_pLTDatabase->GetAttribute( m_hGlobalRecord, "TeamModel" );
	return g_pLTDatabase->GetNumValues( hTeamModelsStruct );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetFriendlyTeamModel() const
//
//	PURPOSE:	Gets the model record used for friendly teammembers...
//
// ----------------------------------------------------------------------- //

ModelsDB::HMODEL ModelsDB::GetFriendlyTeamModel( uint32 nIndex ) const
{
	HATTRIBUTE hTeamModelsStruct = g_pLTDatabase->GetAttribute( m_hGlobalRecord, "TeamModel" );
	HATTRIBUTE hTeamModel = GetStructAttribute( hTeamModelsStruct, nIndex, "Friendly" );
	return (HMODEL)GetRecordLink( hTeamModel, 0 );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetEnemyTeamModel() const
//
//	PURPOSE:	Gets the model record used for enemy teammembers...
//
// ----------------------------------------------------------------------- //

ModelsDB::HMODEL ModelsDB::GetEnemyTeamModel( uint32 nIndex ) const
{
	HATTRIBUTE hTeamModelsStruct = g_pLTDatabase->GetAttribute( m_hGlobalRecord, "TeamModel" );
	HATTRIBUTE hTeamModel = GetStructAttribute( hTeamModelsStruct, nIndex, "Enemy" );
	return (HMODEL)GetRecordLink( hTeamModel, 0 );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetInsigniaAttribute() const
//
//	PURPOSE:	Gets the record used for insignias.
//
// ----------------------------------------------------------------------- //
HATTRIBUTE ModelsDB::GetInsigniaAttribute( ) const
{
	return GetAttribute( m_hGlobalRecord, "DefaultInsigniaTexture" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetPreSaleInsigniaAttribute() const
//
//	PURPOSE:	Gets the record used for insignias.
//
// ----------------------------------------------------------------------- //
HATTRIBUTE ModelsDB::GetPreSaleInsigniaAttribute( ) const
{
	return GetAttribute( m_hGlobalRecord, "PreSaleInsigniaTextures" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetInsigniaFolder() const
//
//	PURPOSE:	Gets the folder for insignias.
//
// ----------------------------------------------------------------------- //
bool ModelsDB::GetInsigniaFolder( char* pszFolder, uint32 nSize ) const
{
	// Get the first insignia so we can pull the folder path out of it.
	char const* pszFirstInsignia = GetString( GetInsigniaAttribute(), 0, "" );
	if( LTStrEmpty( pszFirstInsignia ))
		return false;

	// Get the folder to it.
	char szPatchDir[MAX_PATH*2] = "";
	LTFileOperations::SplitPath( pszFirstInsignia, szPatchDir, NULL, NULL );
	LTStrCpy( pszFolder, szPatchDir, nSize );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetNullWeightSetName() const
//
//	PURPOSE:	Gets name of the 'null' weightset used for disabled 
//				animation trackers.
//
// ----------------------------------------------------------------------- //
const char* const ModelsDB::GetNullWeightSetName() const
{
	return GetString( m_hGlobalRecord, "NullWeightSetName" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::MissingAnimationName() const
//
//	PURPOSE:	Gets the name of the animation to play when an animation 
//				is missing.
//
// ----------------------------------------------------------------------- //
const char* const ModelsDB::GetMissingAnimationName() const
{
	return GetString( m_hGlobalRecord, "MissingAnimationName" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetBodyRagdoll() const
//
//	PURPOSE:	Returns false if ragdoll is disabled, any other value if 
//				enabled.
//
// ----------------------------------------------------------------------- //

bool ModelsDB::GetBodyRagdoll() const
{
	return GetBool( m_hGlobalRecord, "BodyRagdoll" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetBodyRagdoll() const
//
//	PURPOSE:	Minimum Y death velocity for the body.  If the velocity is 
//				less than this value, the velocity will be clamped to it.
//
// ----------------------------------------------------------------------- //

float ModelsDB::GetBodyDeathDirMinY() const
{
	return GetFloat( m_hGlobalRecord, "BodyDeathDirMinY" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetBodyDeathForce() const
//
//	PURPOSE:	Enables or disables the application of a scaled death 
//				impulse to all bodies in an object on death.
//
// ----------------------------------------------------------------------- //

bool ModelsDB::GetBodyDeathForce() const
{
	return GetBool( m_hGlobalRecord, "BodyDeathForce" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetBodyDeathForceScale() const
//
//	PURPOSE:	If BodyDeathForce is true, the death impulse is scaled 
//				by this value and is applied to all rigid bodies.
//
// ----------------------------------------------------------------------- //

float ModelsDB::GetBodyDeathForceScale() const
{
	return GetFloat( m_hGlobalRecord, "BodyDeathForceScale" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::BodyDirDeathForce() const
//
//	PURPOSE:	If true, an additional death impulse is applied to the 
//				node which took the damage that killed the AI.
//
// ----------------------------------------------------------------------- //

bool ModelsDB::GetBodyDirDeathForce() const
{
	return GetBool( m_hGlobalRecord, "BodyDirDeathForce" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetBodyDirDeathForceScale() const
//
//	PURPOSE:	Additional scalar to the death impulse, applied to the 
//				node the character was hit in which resulted in the death.
//
// ----------------------------------------------------------------------- //

float ModelsDB::GetBodyDirDeathForceScale() const
{
	return GetFloat( m_hGlobalRecord, "BodyDirDeathForceScale" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetBodyStickDist() const
//
//	PURPOSE:	Distance to check for an obstacle to 'stick' an AI to 
//				when a particular weapon sticks an AI to walls.
//
// ----------------------------------------------------------------------- //

float ModelsDB::GetBodyStickDist() const
{
	return GetFloat( m_hGlobalRecord, "BodyStickDist" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetBodyStickTime() const
//
//	PURPOSE:	Amount of time a ragdoll 'stuck' to geometry spends 
//				interpolating to the stuck position before entering ragdoll.
//
// ----------------------------------------------------------------------- //

float ModelsDB::GetBodyStickTime() const
{
	return GetFloat( m_hGlobalRecord, "BodyStickTime" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetBodyDeathScaleVelocity() const
//
//	PURPOSE:	Scalar applied to a characters existing rigidbody 
//				velocities prior to death forces being applied.  This can 
//				be used to reduce contribution of animations or other 
//				forces to the death velocity.
//
// ----------------------------------------------------------------------- //

float ModelsDB::GetBodyDeathScaleVelocity() const
{
	return GetFloat( m_hGlobalRecord, "BodyDeathScaleVelocity" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetBodyDeathClampVelocityMax() const
//
//	PURPOSE:	Max rigidbody velocity prior to death impulse application.
//				This can be used to reduce contribution of animations or 
//				other forces to the death velocity.
//
// ----------------------------------------------------------------------- //

float ModelsDB::GetBodyDeathClampVelocityMax() const
{
	return GetFloat( m_hGlobalRecord, "BodyDeathClampVelocityMax" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetRagdollDeathAnimationName() const
//
//	PURPOSE:	Animation name to play on character when the die and ragdoll...
//
// ----------------------------------------------------------------------- //

const char* const ModelsDB::GetRagdollDeathAnimationName( ) const
{
	return GetString( m_hGlobalRecord, "RagdollDeathAnimation" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetNumSocketSets() const
//
//	PURPOSE:	Gets the number of socketsets.
//
// ----------------------------------------------------------------------- //
uint32 ModelsDB::GetNumSocketSets( ) const
{
	return g_pLTDatabase->GetNumRecords( m_hSocketSetsCat );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetSocketSet()
//
//	PURPOSE:	Gets the socketset by index.
//
// ----------------------------------------------------------------------- //
ModelsDB::HSOCKETSET ModelsDB::GetSocketSet( uint32 nIndex ) const
{
	return ( HSOCKETSET )g_pLTDatabase->GetRecordByIndex( m_hSocketSetsCat, nIndex );
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetSocketSetNumSockets() const
//
//	PURPOSE:	Gets the number of sockets in a socketset.
//
// ----------------------------------------------------------------------- //
uint32 ModelsDB::GetSocketSetNumSockets(HSOCKETSET hModelSocketSet ) const
{
	return GetNumValues(( HRECORD )hModelSocketSet, "Sockets" );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetSocketSetSocket() const
//
//	PURPOSE:	Gets the Socket name.
//
// ----------------------------------------------------------------------- //
char const* ModelsDB::GetSocketSetSocket( HSOCKETSET hModelSocketSet, uint32 nIndex ) const
{
	return GetString(( HRECORD )hModelSocketSet, "Sockets", nIndex );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetHSkeletonByRecordName() const
//
//	PURPOSE:	Gets the HSKELETON object by gdb record name.
//
// ----------------------------------------------------------------------- //
ModelsDB::HSKELETON ModelsDB::GetHSkeletonByRecordName( char const* pszRecordName ) const
{
	return ( HSKELETON )GetRecord( m_hSkeletonCat, pszRecordName );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetSkeletonNumNodes() const
//
//	PURPOSE:	Gets the number of nodes on skeleton.
//
// ----------------------------------------------------------------------- //
uint32 ModelsDB::GetSkeletonNumNodes(HSKELETON hModelSkeleton) const
{
	return GetNumValues(( HRECORD )hModelSkeleton, "Nodes" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetSkeletonNode() const
//
//	PURPOSE:	Gets the HNODE from a skeleton by index.
//
// ----------------------------------------------------------------------- //
ModelsDB::HNODE ModelsDB::GetSkeletonNode(HSKELETON hModelSkeleton, uint32 nIndex) const
{
	return ( HNODE )GetRecordLink(( HRECORD )hModelSkeleton, "Nodes", nIndex );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetSkeletonNode() const
//
//	PURPOSE:	Gets the HNODE from a skeleton by index.
//
// ----------------------------------------------------------------------- //
ModelsDB::HNODE ModelsDB::GetSkeletonNode(HSKELETON hModelSkeleton, const char* szName) const
{
	HATTRIBUTE hNodes = GetAttribute(( HRECORD )hModelSkeleton, "Nodes" );
	uint32 nNumValues = g_pLTDatabase->GetNumValues( hNodes );

	for( uint32 iNode = 0 ; iNode < nNumValues; iNode++ )
	{
		HNODE hNode = ( HNODE )g_pLTDatabase->GetRecordLink( hNodes, iNode, NULL );
		if( LTStrIEquals( GetNodeName( hNode ), szName ))
		{
			return hNode;
		}
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetSkeletonBlinkNodeGroup() const
//
//	PURPOSE:	Gets the BlinkNodeGroup
//
// ----------------------------------------------------------------------- //
ModelsDB::HBLINKNODEGROUP ModelsDB::GetSkeletonBlinkNodeGroup(HSKELETON hSkeleton) const
{
	return ( HBLINKNODEGROUP )GetRecordLink(( HRECORD )hSkeleton, "BlinkNodes" );
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetSkeletonDefaultFrontDeathAni() const
//
//	PURPOSE:	Gets the DefaultFrontDeathAni
//
// ----------------------------------------------------------------------- //
char const* ModelsDB::GetSkeletonDefaultFrontDeathAni(HSKELETON hSkeleton) const
{
	return GetString(( HRECORD )hSkeleton, "DefaultFrontDeathAni" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetSkeletonDefaultFrontDeathAni() const
//
//	PURPOSE:	Gets the DefaultBackDeathAni
//
// ----------------------------------------------------------------------- //
char const* ModelsDB::GetSkeletonDefaultBackDeathAni(HSKELETON hSkeleton) const
{
	return GetString(( HRECORD )hSkeleton, "DefaultBackDeathAni" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetSkeletonDefaultFrontShortRecoilAni() const
//
//	PURPOSE:	Gets the DefaultFrontShortRecoilAni
//
// ----------------------------------------------------------------------- //
char const* ModelsDB::GetSkeletonDefaultFrontShortRecoilAni(HSKELETON hSkeleton) const
{
	return GetString(( HRECORD )hSkeleton, "DefaultFrontShortRecoilAni" );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetSkeletonDefaultFrontShortRecoilAni() const
//
//	PURPOSE:	Gets the DefaultBackShortRecoilAni
//
// ----------------------------------------------------------------------- //
char const* ModelsDB::GetSkeletonDefaultBackShortRecoilAni(HSKELETON hSkeleton) const
{
	return GetString(( HRECORD )hSkeleton, "DefaultBackShortRecoilAni" );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetSkeletonDefaultHitNode() const
//
//	PURPOSE:	Gets the DefaultHitNode
//
// ----------------------------------------------------------------------- //
ModelsDB::HNODE ModelsDB::GetSkeletonDefaultHitNode(HSKELETON hSkeleton) const
{
	return ( HNODE )GetRecordLink(( HRECORD )hSkeleton, "DefaultHitNode" );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetSkeletonTrackerNodesLookAt() const
//
//	PURPOSE:	Gets the LookAtTrackerNodes
//
// ----------------------------------------------------------------------- //
ModelsDB::HTRACKERNODEGROUP	ModelsDB::GetSkeletonTrackerNodesLookAt(HSKELETON hSkeleton) const
{
	return ( HTRACKERNODEGROUP )GetRecordLink(( HRECORD )hSkeleton, "LookAtTrackerNodes" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetSkeletonTrackerNodesAimAt() const
//
//	PURPOSE:	Gets the AimAtTrackerNodes
//
// ----------------------------------------------------------------------- //
ModelsDB::HTRACKERNODEGROUP	ModelsDB::GetSkeletonTrackerNodesAimAt(HSKELETON hSkeleton) const
{
	return ( HTRACKERNODEGROUP )GetRecordLink(( HRECORD )hSkeleton, "AimAtTrackerNodes" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetSkeletonTrackerNodesArm() const
//
//	PURPOSE:	Gets the ArmTrackerNodes
//
// ----------------------------------------------------------------------- //
ModelsDB::HTRACKERNODEGROUP	ModelsDB::GetSkeletonTrackerNodesArm(HSKELETON hSkeleton) const
{
	return ( HTRACKERNODEGROUP )GetRecordLink(( HRECORD )hSkeleton, "ArmTrackerNodes" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetSkeletonNodeAlongPath() const
//
//	PURPOSE:	Returns the highest priority node along the specified path...
//				Returns NULL if no node along the path.
//
// ----------------------------------------------------------------------- //
ModelsDB::HNODE ModelsDB::GetSkeletonNodeAlongPath( HOBJECT hObject, HSKELETON hSkeleton, const LTVector &vFrom,
												    const LTVector &vDir,  LTRigidTransform *pObjectTrans /*= NULL*/  )
{
	ModelsDB::HNODE hModelNode = NULL;
	float fMaxPriority = (float)(-INT_MAX);

	SERVER_AND_CLIENT_CODE
	(
		if( g_vtSkeletonNodeDebug.GetFloat( ) == 1.0f )
		{
			g_pLTBase->CPrint("Checking skeleton nodes..................");
		}
	)

	uint32 nNumNodes = GetSkeletonNumNodes( hSkeleton );
	for( uint32 nNode = 0; nNode < nNumNodes; ++nNode )
	{
		ModelsDB::HNODE hCurNode = GetSkeletonNode( hSkeleton, nNode );

		// Don't do transforms if we don't need to...
		float fNodeRadius = GetNodeRadius( hCurNode );
		if( fNodeRadius <= 0.0f )
		{
			continue;
		}

		const char* szNodeName = GetNodeName( hCurNode );
		if( !szNodeName )
		{
			// Don't even bother looking for the transform of a nameless node...
			continue;
		}

		HMODELNODE hNode;
		if( g_pModelLT->GetNode( hObject, szNodeName, hNode ) != LT_OK )
		{
			continue;
		}

		LTTransform transNode;
		if( g_pModelLT->GetNodeTransform( hObject, hNode, transNode, (pObjectTrans ? false : true) ) != LT_OK )
		{
			continue;
		}

		if( pObjectTrans )
		{
			transNode.m_vPos = (*pObjectTrans) * transNode.m_vPos;
		}

		// Distance along ray to point of closest approach to node point

		const LTVector vRelativeNodePos = transNode.m_vPos - vFrom;
		const float fRayDist = vDir.Dot(vRelativeNodePos);
		const float fDistSqr = (vDir*fRayDist - vRelativeNodePos).MagSqr();

		// Ignore the node if it wasn't within the radius of the hit spot.
		if( fDistSqr > fNodeRadius*fNodeRadius )
		{
			continue;
		}

		if( g_vtSkeletonNodeDebug.GetFloat( ) == 1.0f )
		{
			g_pLTBase->CPrint("Found '%s'", szNodeName );
		}

		// Get the hit priority of this node.
		float fPriority = GetNodePriority( hCurNode );

		// Ignore if not a higher priority node.
		if ( fPriority < fMaxPriority )
		{
			continue;
		}

		// Highest priority hit node so far.
		hModelNode = hCurNode;
		fMaxPriority = fPriority;
	}

	return hModelNode;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetTrackerAimerNodeName() const
//
//	PURPOSE:	Gets the Aimer node name found in the model file.
//
// ----------------------------------------------------------------------- //
char const* ModelsDB::GetTrackerAimerNodeName(HTRACKERNODEGROUP hModelTrackerNodeGroup) const
{
	return GetString(( HRECORD )hModelTrackerNodeGroup, "AimerNodeName" );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetTrackerAimerNodeName() const
//
//	PURPOSE:	Gets the Aimer node limits.
//
// ----------------------------------------------------------------------- //
LTRect2f ModelsDB::GetTrackerAimerNodeLimits(HTRACKERNODEGROUP hModelTrackerNodeGroup) const
{
	LTRect2f limits;
	LTVector2 vRangeX = GetVector2(( HRECORD )hModelTrackerNodeGroup, "AimerRangeX" );
	LTVector2 vRangeY = GetVector2(( HRECORD )hModelTrackerNodeGroup, "AimerRangeY" );

	limits.m_vMin.x = vRangeX.x;
	limits.m_vMin.y = vRangeY.x;
	limits.m_vMax.x = vRangeX.y;
	limits.m_vMax.y = vRangeY.y;

	return limits;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetTrackerAimerNodeMaxSpeed() const
//
//	PURPOSE:	Gets the Aimer node max speed.
//
// ----------------------------------------------------------------------- //
float ModelsDB::GetTrackerAimerNodeMaxSpeed(HTRACKERNODEGROUP hModelTrackerNodeGroup) const
{
	return GetFloat(( HRECORD )hModelTrackerNodeGroup, "AimerMaxSpeed" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetNumTrackerControlledNodes() const
//
//	PURPOSE:	Gets the number of controlled nodes.
//
// ----------------------------------------------------------------------- //
uint32 ModelsDB::GetNumTrackerControlledNodes(HTRACKERNODEGROUP hModelTrackerNodeGroup) const
{
	return GetNumValues(( HRECORD )hModelTrackerNodeGroup, "TrackNodeName" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetTrackerControlledNodeName() const
//
//	PURPOSE:	Gets the node name of controlled node.
//
// ----------------------------------------------------------------------- //
char const* ModelsDB::GetTrackerControlledNodeName(HTRACKERNODEGROUP hModelTrackerNodeGroup, uint32 nIndex) const
{
	return GetString(( HRECORD )hModelTrackerNodeGroup, "TrackNodeName", nIndex );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetTrackerControlledNodeWeight() const
//
//	PURPOSE:	Gets the node weight of controlled node.
//
// ----------------------------------------------------------------------- //
float ModelsDB::GetTrackerControlledNodeWeight(HTRACKERNODEGROUP hModelTrackerNodeGroup, uint32 nIndex) const
{
	return GetFloat(( HRECORD )hModelTrackerNodeGroup, "TrackNodeWeight", nIndex );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetBlinkFrequency() const
//
//	PURPOSE:	Gets the duration of the blink in seconds.
//
// ----------------------------------------------------------------------- //
float ModelsDB::GetBlinkDuration(HBLINKNODEGROUP hModelBlinkNodeGroup) const
{
	return GetFloat(( HRECORD )hModelBlinkNodeGroup, "Duration" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetBlinkFrequency() const
//
//	PURPOSE:	Gets the frequency range of the duration in seconds.
//
// ----------------------------------------------------------------------- //
LTVector2 ModelsDB::GetBlinkFrequency(HBLINKNODEGROUP hModelBlinkNodeGroup) const
{
	return GetVector2(( HRECORD )hModelBlinkNodeGroup, "Frequency" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetNumBlinkNodes() const
//
//	PURPOSE:	Gets the number of blinknodes.
//
// ----------------------------------------------------------------------- //
uint32 ModelsDB::GetNumBlinkNodes(HBLINKNODEGROUP hModelBlinkNodeGroup) const
{
	return GetNumValues(( HRECORD )hModelBlinkNodeGroup, "NodeName" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetBlinkNodeName() const
//
//	PURPOSE:	Gets the node name of blinknode.
//
// ----------------------------------------------------------------------- //
char const* ModelsDB::GetBlinkNodeName(HBLINKNODEGROUP hModelBlinkNodeGroup, uint32 nIndex) const
{
	return GetString(( HRECORD )hModelBlinkNodeGroup, "NodeName", nIndex );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetBlinkNodeAxis() const
//
//	PURPOSE:	Gets the rotation axis of the blinknode.
//
// ----------------------------------------------------------------------- //
LTVector ModelsDB::GetBlinkNodeAxis(HBLINKNODEGROUP hModelBlinkNodeGroup, uint32 nIndex) const
{
	return GetVector3(( HRECORD )hModelBlinkNodeGroup, "NodeRotationAxis", nIndex );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetBlinkNodeAngle() const
//
//	PURPOSE:	Gets the rotation angle of the blinknode.
//
// ----------------------------------------------------------------------- //
float ModelsDB::GetBlinkNodeAngle(HBLINKNODEGROUP hModelBlinkNodeGroup, uint32 nIndex) const
{
	return DEG2RAD( GetFloat(( HRECORD )hModelBlinkNodeGroup, "NodeRotationAngle", nIndex ));
}





// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetNodeName() const
//
//	PURPOSE:	Gets the node name found in the model file.
//
// ----------------------------------------------------------------------- //
char const* ModelsDB::GetNodeName( ModelsDB::HNODE hNode ) const
{
	return GetString(( HRECORD )hNode, "Name" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetNodeFlags() const
//
//	PURPOSE:	Gets the node Flags.
//
// ----------------------------------------------------------------------- //
uint32 ModelsDB::GetNodeFlags( ModelsDB::HNODE hNode ) const
{
	return ( uint32 )GetInt32(( HRECORD )hNode, "Flags" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetNodeFrontDeathAni() const
//
//	PURPOSE:	Gets the node frontdeathani.
//
// ----------------------------------------------------------------------- //
char const* ModelsDB::GetNodeFrontDeathAni( ModelsDB::HNODE hNode ) const
{
	return GetString(( HRECORD )hNode, "FrontDeathAni" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetNodeBackDeathAni() const
//
//	PURPOSE:	Gets the node BackDeathAni.
//
// ----------------------------------------------------------------------- //
char const* ModelsDB::GetNodeBackDeathAni( ModelsDB::HNODE hNode ) const
{
	return GetString(( HRECORD )hNode, "BackDeathAni" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetNodeFrontShortRecoilAni() const
//
//	PURPOSE:	Gets the node FrontLongRecoilAni.
//
// ----------------------------------------------------------------------- //
char const* ModelsDB::GetNodeFrontShortRecoilAni( ModelsDB::HNODE hNode ) const
{
	return GetString(( HRECORD )hNode, "FrontShortRecoilAni" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetNodeFrontShortRecoilAni() const
//
//	PURPOSE:	Gets the node FrontLongRecoilAni.
//
// ----------------------------------------------------------------------- //
char const* ModelsDB::GetNodeBackShortRecoilAni( ModelsDB::HNODE hNode ) const
{
	return GetString(( HRECORD )hNode, "BackShortRecoilAni" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetNodeDamageFactor() const
//
//	PURPOSE:	Gets the node DamageFactor
//
// ----------------------------------------------------------------------- //
EnumAnimProp ModelsDB::GetNodeBodyAnimProp( ModelsDB::HNODE hNode ) const
{
	return AnimPropUtils::Enum( GetString(( HRECORD )hNode, "BodyAnimProp" ));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetNodeDamageFactor() const
//
//	PURPOSE:	Gets the node DamageFactor
//
// ----------------------------------------------------------------------- //
float ModelsDB::GetNodeDamageFactor( ModelsDB::HNODE hNode ) const
{
	return GetFloat(( HRECORD )hNode, "DamageFactor" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetNodeInstDamageImpulseForceScale() const
//
//	PURPOSE:	Gets the node InstDamageImpulseForceScale
//
// ----------------------------------------------------------------------- //
float ModelsDB::GetNodeInstDamageImpulseForceScale( ModelsDB::HNODE hNode ) const
{
	return GetFloat(( HRECORD )hNode, "InstDamageImpulseForceScale" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetNodeParentNode() const
//
//	PURPOSE:	Gets the node ParentNode
//
// ----------------------------------------------------------------------- //
ModelsDB::HNODE ModelsDB::GetNodeParentNode( ModelsDB::HNODE hNode ) const
{
	return ( HNODE )GetRecordLink(( HRECORD )hNode, "ParentNode" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetNodeRecoilParentNode() const
//
//	PURPOSE:	Gets the node RecoilParentNode
//
// ----------------------------------------------------------------------- //
ModelsDB::HNODE ModelsDB::GetNodeRecoilParentNode( ModelsDB::HNODE hNode ) const
{
	return ( HNODE )GetRecordLink(( HRECORD )hNode, "RecoilParentNode" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetNodeLocation() const
//
//	PURPOSE:	Gets the node Location
//
// ----------------------------------------------------------------------- //
HitLocation ModelsDB::GetNodeLocation( ModelsDB::HNODE hNode ) const
{
	return HitLocationFromString(GetString(( HRECORD )hNode, "Location" ));

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetNodeRadius() const
//
//	PURPOSE:	Gets the node Radius
//
// ----------------------------------------------------------------------- //
float ModelsDB::GetNodeRadius( ModelsDB::HNODE hNode ) const
{
	return GetFloat(( HRECORD )hNode, "Radius" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetNodePriority() const
//
//	PURPOSE:	Gets the node Priority
//
// ----------------------------------------------------------------------- //
float ModelsDB::GetNodePriority( ModelsDB::HNODE hNode ) const
{
	return GetFloat(( HRECORD )hNode, "Priority" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetNodeAttachSpears() const
//
//	PURPOSE:	Gets the node AttachSpears
//
// ----------------------------------------------------------------------- //
bool ModelsDB::GetNodeAttachSpears( ModelsDB::HNODE hNode ) const
{
	return GetBool(( HRECORD )hNode, "AttachSpears" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetNodeAutoTarget() const
//
//	PURPOSE:	Gets the node AttachSpears
//
// ----------------------------------------------------------------------- //
bool ModelsDB::GetNodeAutoTarget( ModelsDB::HNODE hNode ) const
{
	return GetBool(( HRECORD )hNode, "AutoTarget" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetNodeOverrideWeaponFX() const
//
//	PURPOSE:	Gets the node override weapon fx
//
// ----------------------------------------------------------------------- //
HRECORD ModelsDB::GetNodeOverrideWeaponFX( ModelsDB::HNODE hNode ) const
{
	return GetRecordLink(( HRECORD )hNode, "OverrideWeaponFX" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetNodeCanWallStick() const
//
//	PURPOSE:	Gets the node flag can wall stick
//
// ----------------------------------------------------------------------- //
bool ModelsDB::GetNodeCanWallStick( HNODE hNode ) const
{
	return GetBool( hNode, "CanWallStick" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetFileCRC
//
//	PURPOSE:	Calculate the filecrc.
//
// ----------------------------------------------------------------------- //

uint32 ModelsDB::GetFileCRC( ) const
{

	uint32 nFileCRC = 0;
	ILTInStream* pFileStream = NULL;	

	// Create an array to store the model crc's.  We make enough room for all models, but
	// only the mp models will be filled in.  It is indexed by modelid, so that no model is
	// crc'd more than once.
	std::vector< uint8 > aModelCRC;
	aModelCRC.resize( GetNumModels( ));

	HATTRIBUTE hAttribute = GetDMModelsAttribute();
	uint32 nNumValues = g_pLTDatabase->GetNumValues( hAttribute );

	// Read in the models
	for ( uint32 iDMModel = 0 ; iDMModel < nNumValues; iDMModel++ )
	{
		char const* pszModelFilename = GetModelFilename(( HMODEL )g_pLTDatabase->GetRecordLink( hAttribute, iDMModel, NULL ));
		ILTInStream* pFileStream = g_pLTBase->FileMgr()->OpenFile(pszModelFilename);
		nFileCRC += CRC32Utils::CalcArchiveFileCRC(pFileStream, pszModelFilename);
		if (pFileStream)
		{
			pFileStream->Release();
		}
		aModelCRC[iDMModel] = 1;
	}
	
	//const char *pszFriendlyTeamModel = GetModelFilename( GetFriendlyTeamModel( ));
	//ILTInStream* pFileStream = g_pLTBase->FileMgr()->OpenFile(pszFriendlyTeamModel);
	//nFileCRC += CRC32Utils::CalcArchiveFileCRC(pFileStream, pszFriendlyTeamModel);
	//if (pFileStream)
	//{
	//	pFileStream->Release();
	//}

	//const char *pszEnemyTeamModel = GetModelFilename( GetEnemyTeamModel( ));
	//ILTInStream* pFileStream = g_pLTBase->FileMgr()->OpenFile(pszEnemyTeamModel);
	//nFileCRC += CRC32Utils::CalcArchiveFileCRC(pFileStream, pszEnemyTeamModel);
	//if (pFileStream)
	//{
	//	pFileStream->Release();
	//}

	return nFileCRC;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetBody??() const
//
//	PURPOSE:	Various access function for SeverBody records
//
// ----------------------------------------------------------------------- //
const char* ModelsDB::GetBodyModelFile( HSEVERBODY hBody ) const
{
	return GetString(hBody,"ModelFile");
}
uint32 ModelsDB::GetBodyNumMaterials( HSEVERBODY hBody ) const
{
	return GetNumValues(hBody,"Material");
}
const char*	ModelsDB::GetBodyMaterial( HSEVERBODY hBody, uint32 nIndex ) const
{
	return GetString(hBody,"Material",nIndex);
}

void ModelsDB::CopyBodyMaterialFilenames( HSEVERBODY hBody, char* paszDest, uint32 nNumValues, uint32 nStrLen ) const
{
	CopyStringValues( hBody, "Material", paszDest, nNumValues, nStrLen );
}

uint32 ModelsDB::GetBodyNumPieces( HSEVERBODY hBody ) const
{
	return GetNumValues(hBody,"Piece");
}

ModelsDB::HSEVERPIECE	ModelsDB::GetBodyPiece(HSEVERBODY hBody, uint32 nIndex) const
{
	return GetRecordLink(hBody,"Piece",nIndex);
}
ModelsDB::HSEVERPIECE ModelsDB::GetRandomPiece(HSEVERBODY hBody) const
{
	uint32 nTotal = 0;
	uint32 nNumPieces = GetBodyNumPieces(hBody);

	//early outs
	if (nNumPieces == 0)
		return NULL;
	if (nNumPieces == 1)
		return GetBodyPiece(hBody,0);

	for (uint32 n = 0; n < nNumPieces; ++n)
	{
		HSEVERPIECE hPiece = GetBodyPiece(hBody,n);
		nTotal += GetSPChance(hPiece);
	}

	uint32 nChoice = GetRandom(0,nTotal-1);


	for (uint32 n = 0; n < nNumPieces; ++n)
	{
		HSEVERPIECE hPiece = GetBodyPiece(hBody,n);

		uint32 nChance = GetSPChance(hPiece);
		if (nChoice < nChance)
			return hPiece;
		else
			nChoice -= nChance;
	}

	return NULL;

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetSP??() const
//
//	PURPOSE:	Various access function for SeverPiece records
//
// ----------------------------------------------------------------------- //
HitLocation ModelsDB::GetSPLocation( HSEVERPIECE hPiece ) const
{
	return HitLocationFromString(GetString( hPiece, "Location" ));
}

uint32		ModelsDB::GetSPNumPieces( HSEVERPIECE hPiece ) const
{
	return GetNumValues(hPiece,"Piece");
}

const char*	ModelsDB::GetSPPiece(HSEVERPIECE hPiece, uint32 nIndex) const
{
	return GetString(hPiece,"Piece",nIndex);
}

uint32		ModelsDB::GetSPNumNodes( HSEVERPIECE hPiece ) const
{
	return GetNumValues(hPiece,"Node");
}

ModelsDB::HNODE		ModelsDB::GetSPNode( HSEVERPIECE hPiece, uint32 nIndex) const
{
	return (HNODE)GetRecordLink(hPiece,"Node",nIndex);
}

uint32		ModelsDB::GetSPNumSockets( HSEVERPIECE hPiece ) const
{
	return GetNumValues(hPiece,"Socket");
}

const char*	ModelsDB::GetSPSocket( HSEVERPIECE hPiece, uint32 nIndex ) const
{
	return GetString(hPiece,"Socket",nIndex);
}

const char*	ModelsDB::GetSPModelFile( HSEVERPIECE hPiece ) const
{
	return GetString(hPiece,"ModelFile");
}

uint32		ModelsDB::GetSPNumMaterials( HSEVERPIECE hPiece ) const
{
	return GetNumValues(hPiece,"Material");
}

const char*	ModelsDB::GetSPMaterial( HSEVERPIECE hPiece, uint32 nIndex ) const
{
	return GetString(hPiece,"Material",nIndex);
}

void		ModelsDB::CopySPMaterialFilenames( HSEVERPIECE hPiece, char* paszDest, uint32 nNumValues, uint32 nStrLen ) const
{
	CopyStringValues( hPiece, "Material", paszDest, nNumValues, nStrLen );
}

ModelsDB::HNODE		ModelsDB::GetSPSourceNode( HSEVERPIECE hPiece) const
{
	return (HNODE)GetRecordLink(hPiece,"SourceNode");
}

const char*	ModelsDB::GetSPBodyFX( HSEVERPIECE hPiece ) const
{
	return GetString(hPiece,"BodyFX");
}

const char*	ModelsDB::GetSPBodyFXSocket( HSEVERPIECE hPiece ) const
{
	return GetString(hPiece,"BodyFXSocket");
}

const char*	ModelsDB::GetSPPartFX( HSEVERPIECE hPiece ) const
{
	return GetString(hPiece,"PartFX");
}

const char*	ModelsDB::GetSPPartFXSocket( HSEVERPIECE hPiece ) const
{
	return GetString(hPiece,"PartFXSocket");
}

uint32		ModelsDB::GetSPChance( HSEVERPIECE hPiece ) const
{
	return GetInt32(hPiece,"Chance");
}

ModelsDB::HSEVERDAMAGE ModelsDB::GetSPDamageRecord( HSEVERPIECE hPiece ) const
{
	return (HSEVERDAMAGE)GetRecordLink(hPiece,"Damage");
}

float		ModelsDB::GetSPResistance( HSEVERPIECE hPiece ) const
{
	return GetFloat(hPiece,"Resistance");
}

//is hPiece excluded by the already severed hTestPiece
bool ModelsDB::IsExcludedPiece( HSEVERPIECE hPiece, HSEVERPIECE hTestPiece ) const
{
	uint32 nNumPieces = GetNumValues(hPiece,"Exclusion");
	for (uint32 p = 0; p < nNumPieces; ++p )
	{
		if ( (HSEVERPIECE)GetRecordLink(hPiece,"Exclusion",p,NULL) == hTestPiece)
		{
			return true;
		}
	}

	return false;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetSeverDamageMax() const
//
//	PURPOSE:	Gets the max damage required to sever a limb
//
// ----------------------------------------------------------------------- //
float ModelsDB::GetSeverDamageMax(HSEVERDAMAGE hSeverDamage, DamageType eDT) const
{
	uint32 n = g_pLTDatabase->GetNumAttributes(hSeverDamage);
	HATTRIBUTE hA = g_pLTDatabase->GetAttributeByIndex(hSeverDamage,0);
	const char *psz = g_pLTDatabase->GetAttributeName(hA);

	HATTRIBUTE hStructAtt =  GetAttribute(hSeverDamage,"DamageTypes");
	uint32 nNumDamageTypes = g_pLTDatabase->GetNumValues(hStructAtt);

	//step through each damage type
	for (uint32 n = 0; n < nNumDamageTypes; ++n)
	{
		HATTRIBUTE hTmpAtt = GetStructAttribute(hStructAtt,n,"DamageType");
		HRECORD hDT = g_pLTDatabase->GetRecordLink(hTmpAtt,0,NULL);

		//if this damage type matches the passed in damage type
		if (g_pDTDB->GetDamageType(hDT) == eDT)
		{
			//see if it has a chance of severing a limb
			hTmpAtt = GetStructAttribute(hStructAtt,n,"SeverMaxChance");
			float fMax = g_pLTDatabase->GetFloat(hTmpAtt,0,0.0f);
			if (fMax > 0.0f)
			{
				//if it has a chance figure out the amount of dmage required to max out that chance
				hTmpAtt = GetStructAttribute(hStructAtt,n,"SeverMaxDamage");
				return g_pLTDatabase->GetFloat(hTmpAtt,0,0.0f);
			}
		}
	}

	return 0.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetSeverChance() const
//
//	PURPOSE:	Gets the chance of severing a limb based on damage type
//				and amount
//
// ----------------------------------------------------------------------- //
float ModelsDB::GetSeverChance(HSEVERDAMAGE hSeverDamage, DamageType eDT, float fDamage) const
{
	HATTRIBUTE hStructAtt =  GetAttribute(hSeverDamage,"DamageTypes");
	uint32 nNumDamageTypes = g_pLTDatabase->GetNumValues(hStructAtt);

	//step through each damage type
	for (uint32 n = 0; n < nNumDamageTypes; ++n)
	{
		HATTRIBUTE hTmpAtt = GetStructAttribute(hStructAtt,n,"DamageType");
		HRECORD hDT = g_pLTDatabase->GetRecordLink(hTmpAtt,0,NULL);

		//if this damage type matches the passed in damage type
		if (g_pDTDB->GetDamageType(hDT) == eDT)
		{
			hTmpAtt = GetStructAttribute(hStructAtt,n,"SeverMinChance");
			float fMinChance = g_pLTDatabase->GetFloat(hTmpAtt,0,0.0f);

			hTmpAtt = GetStructAttribute(hStructAtt,n,"SeverMaxChance");
			float fMaxChance = g_pLTDatabase->GetFloat(hTmpAtt,0,0.0f);

			if (fMaxChance <= 0.0f)
				return 0.0f;

			SERVER_AND_CLIENT_CODE
			(
				if (g_vtBodySeverTest.GetFloat() > 0.0f)
				{
					return 1.0f;
				}
			)

			hTmpAtt = GetStructAttribute(hStructAtt,n,"SeverMinDamage");
			float fMinDamage = g_pLTDatabase->GetFloat(hTmpAtt,0,0.0f);

			hTmpAtt = GetStructAttribute(hStructAtt,n,"SeverMaxDamage");
			float fMaxDamage = g_pLTDatabase->GetFloat(hTmpAtt,0,0.0f);

			if (fMinDamage > fDamage)
				return 0.0f;

			fDamage = LTMIN(fDamage,fMaxDamage);

			float fRange = (fMaxDamage - fMinDamage);
			float fScale = ((fRange <= 0.0f) ? 1.0f :(fDamage-fMinDamage)/fRange );

			return LTCLAMP(	(fMinChance + fScale * (fMaxChance - fMinChance)) , 0.0f, 1.0f);
		}
	}
	return 0.0f;

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetDamageSpecificDecal() const
//
//	PURPOSE:	Gets the model decal record for the given model node
//				to be used with the specified damage type.
//
// ----------------------------------------------------------------------- //
HRECORD ModelsDB::GetDamageSpecificDecalRecord(HMODEL hModel, const char* pszNodeName, HRECORD hDamageType, bool bLowGore) const
{
	// Validate input.
	if (!hModel || !hDamageType)
		return NULL;

	// See if our character has a ModelDecalSet.
	HRECORD hModelDecalSet = GetRecordLink(hModel, "ModelDecalSet");
	if (!hModelDecalSet)
		return NULL;

	// Get the model node record.
	HSKELETON hModelSkeleton = GetModelSkeleton(hModel);
	if (!hModelSkeleton)
		return NULL;

	HNODE hSkeletonNode = GetSkeletonNode(hModelSkeleton, pszNodeName);
	if (!hSkeletonNode)
		return NULL;

	// Find the node we're looking for...
	HATTRIBUTE hNodes = GetAttribute(hModelDecalSet, "Nodes");
	uint32 iNumNodes = g_pLTDatabase->GetNumValues(hNodes);
	for (uint32 iNode = 0; iNode < iNumNodes; iNode++)
	{
		HRECORD hModelDecalNode = GetRecordLink(hNodes, iNode);
		if (!hModelDecalNode)
			continue;

		HRECORD hNode = GetRecordLink(hModelDecalNode, "Node");
		if (hNode == hSkeletonNode)
		{
			// Find the DamageType we're looking for...
			HATTRIBUTE hDecals = GetAttribute(hModelDecalNode, "Decals");
			uint32 iNumDecals = g_pLTDatabase->GetNumValues(hDecals);
			for (uint32 iDecal = 0; iDecal < iNumDecals; iDecal++)
			{
				HATTRIBUTE hDamageTypes = GetStructAttribute(hDecals, iDecal, "DamageTypes");
				uint32 iNumDamageTypes = g_pLTDatabase->GetNumValues(hDamageTypes);
				for (uint32 iDamageType = 0; iDamageType < iNumDamageTypes; iDamageType++)
				{
					HRECORD hDT = GetRecordLink(hDamageTypes, iDamageType);
					if (!hDT)
						continue;

					if (hDT == hDamageType)
					{
						// Don't use gore decals in low gore settings.
						if (bLowGore)
						{
							HATTRIBUTE hIsGore = GetStructAttribute(hDecals, iDecal, "IsGore");
							bool bGore = GetBool(hIsGore);
							if (bGore)
								continue;
						}

						HATTRIBUTE hDecalType = GetStructAttribute(hDecals, iDecal, "Decal");
						return GetRecordLink(hDecalType);
					}
				}
			}
		}
	}

	// Couldn't find any matches.
	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetModelLeanRecord() const
//
//	PURPOSE:	Get the lean record associated with the model...
//
// ----------------------------------------------------------------------- //

ModelsDB::HLEAN ModelsDB::GetModelLeanRecord( HMODEL hModel ) const
{
	return GetRecordLink( hModel, "Lean" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetModelShortRecoils() const
//
//	PURPOSE:	Returns true if the passed in model supports shortrecoils, 
//				false if it does not.  This is a property of the model, not
//				the skeleton, so that different models can share the same 
//				skeleton (but still have differences in use of recoils).
//
//				All of this is required because we assert if we fail to 
//				play a shortrecoil for a given hitnode.  This is desired,
//				as it helps track down badly configured assets, but 
//				suppessing this behavior may be desirable if a character 
//				intentionally is missing recoils.
//
// ----------------------------------------------------------------------- //

bool ModelsDB::GetModelShortRecoils( HMODEL hModel ) const
{
	return GetBool( hModel, "ShortRecoils" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetModelNameDisplay()
//
//	PURPOSE:	Returns the custom name display for this model
//
// ----------------------------------------------------------------------- //
HRECORD ModelsDB::GetModelNameDisplay(HMODEL hModel) const
{
	return GetRecordLink( hModel, "NameDisplay" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetModelInsigniaDisplay()
//
//	PURPOSE:	Returns the custom insignia display for this model
//
// ----------------------------------------------------------------------- //
HRECORD ModelsDB::GetModelInsigniaDisplay(HMODEL hModel) const
{
	return GetRecordLink( hModel, "InsigniaDisplay" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetModelPreSale()
//
//	PURPOSE:	Gets whether the model is for limited presale.
//
// ----------------------------------------------------------------------- //
bool ModelsDB::GetModelPreSale(HMODEL hModel) const
{
	return GetBool( hModel, "PreSale" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetModelCanWallStick()
//
//	PURPOSE:	Gets whether the model is allowed to be stuck to walls
//
// ----------------------------------------------------------------------- //
bool ModelsDB::GetModelCanWallStick(HMODEL hModel) const
{
	return GetBool( hModel, "CanWallStick" );
}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetLeanMaxAngle() const
//
//	PURPOSE:	Get the max angle for a particular lean...
//
// ----------------------------------------------------------------------- //

float ModelsDB::GetLeanAngle( HLEAN hLean ) const
{
	return GetFloat( hLean, "LeanAngle" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetLeanOutTime() const
//
//	PURPOSE:	Get the lean out time for a particular lean...
//
// ----------------------------------------------------------------------- //

uint32 ModelsDB::GetLeanOutTime( HLEAN hLean ) const
{
	return (uint32)GetInt32( hLean, "LeanOutTime" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetLeanCenterTime() const
//
//	PURPOSE:	Get the lean center time for a particular lean...
//
// ----------------------------------------------------------------------- //

uint32 ModelsDB::GetLeanCenterTime( HLEAN hLean ) const
{
	return (uint32)GetInt32( hLean, "LeanCenterTime" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetLeanCameraWeight() const
//
//	PURPOSE:	Get the camera weight for a particular lean...
//
// ----------------------------------------------------------------------- //

float ModelsDB::GetLeanCameraWeight( HLEAN hLean ) const
{
	return GetFloat( hLean, "LeanCameraWeight" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetLeanNumNodes() const
//
//	PURPOSE:	Get the number of nodes associated with the lean...
//
// ----------------------------------------------------------------------- //

uint32 ModelsDB::GetLeanNumNodes( HLEAN hLean ) const
{
	return GetNumValues(( HRECORD )hLean, "Nodes" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetLeanNodeName() const
//
//	PURPOSE:	Get the name of the specified node...
//
// ----------------------------------------------------------------------- //

const char* ModelsDB::GetLeanNodeName( HLEAN hLean, uint32 nNode ) const
{
    HATTRIBUTE hNode = GetAttribute( hLean, "Nodes" );
	HATTRIBUTE hNodeName = GetStructAttribute( hNode, nNode, "Name" );
	return GetString( hNodeName );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetLeanNodeWeight() const
//
//	PURPOSE:	Get the weight to be applied to the specified node...
//
// ----------------------------------------------------------------------- //

float ModelsDB::GetLeanNodeWeight( HLEAN hLean, uint32 nNode ) const
{
	HATTRIBUTE hNode = GetAttribute( hLean, "Nodes" );
	HATTRIBUTE hNodeWeight = GetStructAttribute( hNode, nNode, "Weight" );
	return GetFloat( hNodeWeight );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetModelInitialMovementSoundRecord() const
//
//	PURPOSE:	Retrieve the initial movement sound for the model...
//
// ----------------------------------------------------------------------- //

HRECORD ModelsDB::GetModelInitialMovementSoundRecord( HMODEL hModel ) const
{
	return GetRecordLink( hModel, "InitialMovementSound" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelsDB::GetSyncActionRecord()
//
//	PURPOSE:	Gets the HRECORD object by gdb record name.
//
// ----------------------------------------------------------------------- //
HRECORD ModelsDB::GetSyncActionRecord( char const* pszSyncAction ) const
{
	return GetRecord( m_hSyncActionCat, pszSyncAction );
}


//DeathFX Accessors
DamageType ModelsDB::GetDeathFXDamageType( HRECORD hDeathFX ) const
{
	HRECORD hDT = GetRecordLink(hDeathFX,"DamageType",0,NULL);
	return g_pDTDB->GetDamageType(hDT);
	
}
const char* ModelsDB::GetDeathFXModelFilename(HMODEL hDeathFX) const
{
	return GetString( hDeathFX, "ModelFile" );
}

uint32 ModelsDB::GetDeathFXNumMaterials(HMODEL hDeathFX) const
{
	return GetNumValues( hDeathFX, "Material" );
}

const char* ModelsDB::GetDeathFXMaterialFilename(HMODEL hDeathFX, uint8 iMaterial) const
{
	return GetString( hDeathFX, "Material", iMaterial );

}
void ModelsDB::CopyDeathFXMaterialFilenames(HMODEL hDeathFX, char* paszDest, uint32 nNumValues, uint32 nStrLen) const
{
	CopyStringValues( hDeathFX, "Material", paszDest, nNumValues, nStrLen );
}
