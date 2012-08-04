// ----------------------------------------------------------------------- //
//
// MODULE  : ModelsDB.h
//
// PURPOSE : Definition of the models database.
//
// CREATED : 03/09/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __MODELSDB_H__
#define __MODELSDB_H__


//
// Includes...
//

#include "GameDatabaseMgr.h"
#include "resourceextensions.h"
#include "SurfaceDB.h"
#include "AnimationProp.h"
#include "ClientServerSharedEnums.h"
#include "CommonUtilities.h"
#include "DamageTypes.h"

//
// Defines...
//

class ModelsDB;
extern ModelsDB* g_pModelsDB;

enum NodeFlag
{
	NODEFLAG_NONE			= (0),		// This node has no special flags
	NODEFLAG_CRITICALHIT	= (1),		// This node is a "critical hit" node
	NODEFLAG_TALK			= (2),		// This node is a "talk" (mouth) node
	NODEFLAG_TIRE			= (4),		// This node is a "tire" node
	NODEFLAG_TURNING_TIRE	= (8),		// This node is a "turning tire" node
	NODEFLAG_ROTOR			= (16),	// This node is a "rotor" node (for helicopters)
	NODEFLAG_EYE			= (32),	// This node is a "eye" node (for humans)
	NODEFLAG_WALLSTICK		= (64),	// This node is a "wall stick" node (for humans)
};



class ModelsDB : public CGameDatabaseMgr
{
	DECLARE_SINGLETON( ModelsDB );

public:

	typedef HRECORD HMODEL;
	typedef HRECORD	HNODE;
	typedef HRECORD	HSKELETON;
	typedef HRECORD	HTRACKERNODEGROUP;
	typedef HRECORD	HBLINKNODEGROUP;
	typedef HRECORD	HSOCKETSET;
	typedef	HRECORD	HSEVERDAMAGE;
	typedef HRECORD HSEVERPIECE;
	typedef HRECORD HSEVERBODY;
	typedef HRECORD HLEAN;

	typedef std::vector<HSEVERPIECE, LTAllocator<HSEVERPIECE, LT_MEM_TYPE_GAMECODE> > HPieceArray;


public :	// Methods...

	bool	Init( const char *szDatabaseFile = DB_Default_File );
	void	Term() {};

	// Gets the filecrc to use for models.
	uint32	GetFileCRC( ) const;

	// HMODEL Accessors.
	HCATEGORY		GetModelsCategory( ) const { return m_hModelsCat; }
	uint32			GetNumModels() const;
	HMODEL			GetModel( uint32 nIndex ) const;
	HMODEL			GetModelByRecordName( char const* pszRecordName ) const;
	char const*		GetRecordNameOfHModel( HMODEL hModel ) const;
	HRECORD			GetModelSoundTemplate( HMODEL hModel ) const;
	HSKELETON		GetModelSkeleton( HMODEL hModel ) const;
	uint32			GetNumModelAnimationTrees( HMODEL hModel ) const;
	const char*		GetModelAnimationTree( HMODEL hModel, uint32 iTree ) const;
	const char*		GetModelAIName( HMODEL hModel ) const;
	float			GetModelMass(HMODEL hModel) const;
	float			GetModelHitPoints(HMODEL hModel) const;
	float			GetModelMaxHitPoints(HMODEL hModel) const;
	float			GetModelArmor(HMODEL hModel) const;
	float			GetModelMaxArmor(HMODEL hModel) const;
	HRECORD			GetModelLoudMovementSnd(HMODEL hModel) const;
	HRECORD			GetModelQuietMovementSnd(HMODEL hModel) const;
	HRECORD			GetModelDeathGibSnd(HMODEL hModel) const;
	HRECORD			GetModelDeathDecapitateSnd(HMODEL hModel) const;
	const char*		GetModelNameId(HMODEL hModel) const;
	HSOCKETSET		GetModelSocketSet( HMODEL hModel ) const;
	const char*		GetDefaultPhysicsWeightSet(HMODEL hModel) const;
	HLEAN			GetModelLeanRecord( HMODEL hModel ) const;
	bool			GetModelShortRecoils( HMODEL hModel ) const;
	HRECORD			GetModelNameDisplay(HMODEL hModel) const;
	HRECORD			GetModelInsigniaDisplay(HMODEL hModel) const;
	bool			GetModelPreSale( HMODEL hModel ) const;
	bool			GetModelCanWallStick( HMODEL hModel ) const;


	const char*		GetModelFilename(HMODEL hModel) const;
	uint32			GetNumMaterials(HMODEL hModel) const;
	const char*		GetMaterialFilename(HMODEL hModel, uint8 iMaterial) const;
	void			CopyMaterialFilenames(HMODEL hModel, char* paszDest, uint32 nNumValues, uint32 nStrLen) const;
	const char*		GetHandsMaterialFilename(HMODEL hModel) const;
	
	uint32			GetNumPersistentClientFX(HMODEL hModel) const;
	const char*		GetPersistentClientFXName( HMODEL hModel, uint32 nClientFX ) const;
	bool			GetPersistentClientFXVisible( HMODEL hModel, uint32 nClientFX ) const;
	bool			GetPersistentClientFXLoop( HMODEL hModel, uint32 nClientFX ) const;
	bool			GetPersistentClientFXSmoothShutdown( HMODEL hModel, uint32 nClientFX ) const;
	
	uint32			GetNumAltHeadMaterials( HMODEL hModel ) const;
	const char*		GetAltHeadMaterial( HMODEL hModel, uint8 iMaterial ) const;
	uint32			GetNumAltBodyMaterials( HMODEL hModel ) const;
	const char*		GetAltBodyMaterial( HMODEL hModel, uint8 iMaterial ) const;
	HSEVERBODY		GetSeverBodyRecord( HMODEL hModel ) const;
	bool			CanGib( HMODEL hModel ) const;
	bool			AlwaysGib( HMODEL hModel ) const;
	const char*		GetGibFX( HMODEL hModel ) const;
	const char*		GetInterfaceFX( HMODEL hModel ) const;
	uint32			GetNumDeathFX( HMODEL hModel) const;
	HRECORD			GetDeathFXRecord( HMODEL hModel, uint8 iFX ) const;
    
	bool			ShowGameStartPoint( HMODEL hModel ) const;
	bool			IsModelTranslucent( HMODEL hModel ) const;

	//access the different surface types for the object
	SurfaceType		GetArmorSurfaceType( HMODEL hModel ) const;
	SurfaceType		GetFleshSurfaceType( HMODEL hModel ) const;

	HRECORD			GetCollisionProperty( HMODEL hModel ) const;

	// Default Attachments
	uint32			GetNumDefaultAttachments( HMODEL hModel ) const;
	const char*		GetDefaultAttachment( HMODEL hModel, uint32 nIndex ) const;

	// Default Weapons
	uint32			GetNumDefaultWeapons( HMODEL hModel ) const;
	void			GetDefaultWeapon(HMODEL hModel, uint32 nIndex, const char*& pszWeaponPosition, HRECORD& hWeapon ) const;

	// Required Weapons
	uint32			GetNumRequiredWeapons( HMODEL hModel ) const;
	void			GetRequiredWeapon(HMODEL hModel, uint32 nIndex, const char*& pszWeaponPosition, HRECORD& hWeapon ) const;

	// PlayerView Attachment
	uint32			GetNumPlayerViewAttachments( HMODEL hModel ) const;
	void			GetPlayerViewAttachment(HMODEL hModel, uint32 nIndex, const char*& pszPVAttachmentPosition, HRECORD& hPVAttachment ) const;

	float			GetUnalertDamageFactor( HMODEL hModel ) const;

	// Splashes accessors
	float			GetSplashesFootstepImpulse( HMODEL hModel ) const;
	float			GetSplashesJumpingShallowImpulse( HMODEL hModel ) const;
	float			GetSplashesJumpingDeepImpulse( HMODEL hModel ) const;

	// Multiplayer models.
	HATTRIBUTE		GetDMModelsAttribute( ) const;
	uint32			GetNumDMModels() const;
	HMODEL			GetDMModel( uint32 nIndex ) const;

	// Team models...
	uint32			GetNumTeamModels( ) const;
	HMODEL			GetFriendlyTeamModel( uint32 nIndex ) const;
	HMODEL			GetEnemyTeamModel( uint32 nIndex ) const;

	// Insignias
	HATTRIBUTE		GetInsigniaAttribute( ) const;
	bool			GetInsigniaFolder( char* pszFolder, uint32 nSize ) const;
	// Attribute lists all the presale insignias.
	HATTRIBUTE		GetPreSaleInsigniaAttribute( ) const;

	// Global Information (used by the AnimationContext)

	const char* const	GetNullWeightSetName() const;
	const char* const	GetMissingAnimationName() const;
	bool			GetBodyRagdoll() const;
	float			GetBodyDeathDirMinY() const;
	bool			GetBodyDeathForce() const;
	float			GetBodyDeathForceScale() const;
	bool			GetBodyDirDeathForce() const;
	float			GetBodyDirDeathForceScale() const;
	float			GetBodyStickDist() const;
	float			GetBodyStickTime() const;
	float			GetBodyDeathScaleVelocity() const;
	float			GetBodyDeathClampVelocityMax() const;
	const char* const GetRagdollDeathAnimationName( ) const;


	// HSOCKETSET Accessors.
	uint32			GetNumSocketSets() const;
	HSOCKETSET		GetSocketSet( uint32 nIndex ) const;
	HSOCKETSET		GetHSocketSetByRecordName( char const* pszRecordName ) const;
	uint32			GetSocketSetNumSockets( HSOCKETSET hModelSocketSet ) const;		
	const char*		GetSocketSetSocket( HSOCKETSET hModelSocketSet, uint32 nIndex ) const;

	// HSKELETON Accessors.
	HCATEGORY		GetSkeletonCategory( ) const { return m_hSkeletonCat; }
	HSKELETON		GetHSkeletonByRecordName( char const* pszRecordName ) const;
	uint32			GetSkeletonNumNodes(HSKELETON hModelSkeleton) const;
	HNODE			GetSkeletonNode(HSKELETON hModelSkeleton, uint32 nIndex) const;
	HNODE			GetSkeletonNode(HSKELETON hModelSkeleton, const char* szName) const;
	HBLINKNODEGROUP GetSkeletonBlinkNodeGroup(HSKELETON hModelSkeleton) const;
	const char*		GetSkeletonDefaultFrontDeathAni(HSKELETON hSkeleton) const;
	const char*		GetSkeletonDefaultBackDeathAni(HSKELETON hSkeleton) const;
	const char*		GetSkeletonDefaultFrontShortRecoilAni(HSKELETON hSkeleton) const;
	const char*		GetSkeletonDefaultBackShortRecoilAni(HSKELETON hSkeleton) const;
	HNODE			GetSkeletonDefaultHitNode(HSKELETON hSkeleton) const;
	HTRACKERNODEGROUP	GetSkeletonTrackerNodesLookAt(HSKELETON hSkeleton) const;
	HTRACKERNODEGROUP	GetSkeletonTrackerNodesAimAt(HSKELETON hSkeleton) const;
	HTRACKERNODEGROUP	GetSkeletonTrackerNodesArm(HSKELETON hSkeleton) const;
	HNODE			GetSkeletonNodeAlongPath( HOBJECT hObject, HSKELETON hSkeleton, const LTVector &vFrom, const LTVector &vDir, LTRigidTransform *pObjectTrans = NULL );

	// HTRACKERNODEGROUP Accessors.
	const char*		GetTrackerAimerNodeName(HTRACKERNODEGROUP hModelTrackerNodeGroup) const;
	LTRect2f		GetTrackerAimerNodeLimits(HTRACKERNODEGROUP hModelTrackerNodeGroup) const;
	float			GetTrackerAimerNodeMaxSpeed(HTRACKERNODEGROUP hModelTrackerNodeGroup) const;
	uint32			GetNumTrackerControlledNodes(HTRACKERNODEGROUP hModelTrackerNodeGroup) const;
	const char*		GetTrackerControlledNodeName(HTRACKERNODEGROUP hModelTrackerNodeGroup, uint32 nIndex) const;
	float			GetTrackerControlledNodeWeight(HTRACKERNODEGROUP hModelTrackerNodeGroup, uint32 nIndex) const;

	// HBLINKGROUP Accessors.
	float			GetBlinkDuration(HBLINKNODEGROUP hModelBlinkNodeGroup) const;
	LTVector2		GetBlinkFrequency(HBLINKNODEGROUP hModelBlinkNodeGroup) const;
	uint32			GetNumBlinkNodes(HBLINKNODEGROUP hModelBlinkNodeGroup) const;
	const char*		GetBlinkNodeName(HBLINKNODEGROUP hModelBlinkNodeGroup, uint32 nIndex) const;
	LTVector		GetBlinkNodeAxis(HBLINKNODEGROUP hModelBlinkNodeGroup, uint32 nIndex) const;
	float			GetBlinkNodeAngle(HBLINKNODEGROUP hModelBlinkNodeGroup, uint32 nIndex) const;

	// HNODE Accessors.
	HCATEGORY GetNodesCategory( ) const { return m_hNodesCat; }
    char const* GetNodeName( HNODE hNode ) const;
	uint32 GetNodeFlags( HNODE hNode ) const;
	char const* GetNodeFrontDeathAni( HNODE hNode ) const;
	char const* GetNodeBackDeathAni( HNODE hNode ) const;
	char const* GetNodeFrontShortRecoilAni( HNODE hNode ) const;
	char const* GetNodeBackShortRecoilAni( HNODE hNode ) const;
	EnumAnimProp GetNodeBodyAnimProp( HNODE hNode ) const;
	float GetNodeDamageFactor( HNODE hNode ) const;
	float GetNodeInstDamageImpulseForceScale( HNODE hNode ) const;
	HNODE GetNodeParentNode( HNODE hNode ) const;
	HNODE GetNodeRecoilParentNode( HNODE hNode ) const;
	HitLocation GetNodeLocation( HNODE hNode ) const;
	float GetNodeRadius( HNODE hNode ) const;
	float GetNodePriority( HNODE hNode ) const;
	bool GetNodeAttachSpears( HNODE hNode ) const;
	bool GetNodeAutoTarget( HNODE hNode ) const;
	HRECORD GetNodeOverrideWeaponFX( HNODE hNode ) const;
	bool GetNodeCanWallStick( HNODE hNode ) const;


	// HSEVERBODY Accessors
	HCATEGORY	GetSeverBodyCategory() const { return m_hSeverBodyCat; }
	const char*	GetBodyModelFile( HSEVERBODY hBody ) const;
	uint32		GetBodyNumMaterials( HSEVERBODY hBody ) const;
	const char*	GetBodyMaterial( HSEVERBODY hBody, uint32 nIndex ) const;
	void		CopyBodyMaterialFilenames( HSEVERBODY hBody, char* paszDest, uint32 nNumValues, uint32 nStrLen ) const;
	uint32		GetBodyNumPieces( HSEVERBODY hBody ) const;
	HSEVERPIECE	GetBodyPiece(HSEVERBODY hBody, uint32 nIndex) const;
	HSEVERPIECE GetRandomPiece(HSEVERBODY hBody) const;

    // HSEVERPIECE Accessors
	HCATEGORY	GetSeverPieceCategory() const { return m_hSeverPieceCat; }
	HitLocation GetSPLocation( HSEVERPIECE hPiece ) const;
	uint32		GetSPNumPieces( HSEVERPIECE hPiece ) const;
	const char*	GetSPPiece(HSEVERPIECE hPiece, uint32 nIndex) const;
	uint32		GetSPNumNodes( HSEVERPIECE hPiece ) const;
	HNODE		GetSPNode( HSEVERPIECE hPiece, uint32 nIndex) const;
	uint32		GetSPNumSockets( HSEVERPIECE hPiece ) const;
	const char*	GetSPSocket( HSEVERPIECE hPiece, uint32 nIndex ) const;
	const char*	GetSPModelFile( HSEVERPIECE hPiece ) const;
	uint32		GetSPNumMaterials( HSEVERPIECE hPiece ) const;
	const char*	GetSPMaterial( HSEVERPIECE hPiece, uint32 nIndex ) const;
	void		CopySPMaterialFilenames( HSEVERPIECE hPiece, char* paszDest, uint32 nNumValues, uint32 nStrLen ) const;
	HNODE		GetSPSourceNode( HSEVERPIECE hPiece) const;
	const char*	GetSPBodyFX( HSEVERPIECE hPiece ) const;
	const char*	GetSPBodyFXSocket( HSEVERPIECE hPiece ) const;
	const char*	GetSPPartFX( HSEVERPIECE hPiece ) const;
	const char*	GetSPPartFXSocket( HSEVERPIECE hPiece ) const;
	uint32		GetSPChance( HSEVERPIECE hPiece ) const;
	HSEVERDAMAGE GetSPDamageRecord( HSEVERPIECE hPiece ) const;
	float		GetSPResistance( HSEVERPIECE hPiece ) const;

	//is hPiece excluded by the already severed hTestPiece
	bool		IsExcludedPiece( HSEVERPIECE hPiece, HSEVERPIECE hTestPiece ) const;

	// HSEVERDAMAGE Accessors.
	HCATEGORY GetSeverDamageCategory() const { return m_hSeverDamageCat; }
	float GetSeverDamageMax(HSEVERDAMAGE hSeverDamage, DamageType eDT) const;
	float GetSeverChance(HSEVERDAMAGE hSeverDamage, DamageType eDT, float fDamage) const;

	HCATEGORY	GetLeanCategory( ) const { return m_hLeanCat;	}
	float		GetLeanAngle( HLEAN hLean ) const;
	uint32		GetLeanOutTime( HLEAN hLean ) const;
	uint32		GetLeanCenterTime( HLEAN hLean ) const;
	float		GetLeanCameraWeight( HLEAN hLean ) const;
	uint32		GetLeanNumNodes( HLEAN hLean ) const;
	const char*	GetLeanNodeName( HLEAN hLean, uint32 nNode ) const;
	float		GetLeanNodeWeight( HLEAN hLean, uint32 nNode ) const;

	HCATEGORY	GetSyncActionCategory() const { return m_hSyncActionCat; }
	HRECORD		GetSyncActionRecord( const char* pszSyncAction ) const;

	// ModelDecalSet Accessors.
	HCATEGORY GetModelDecalSetCategory() const { return m_hModelDecalSetCat; }
	HRECORD GetDamageSpecificDecalRecord(HMODEL hModel, const char* pszNodeName, HRECORD hDamageType, bool bLowGore=false) const;

	HRECORD GetModelInitialMovementSoundRecord( HMODEL hModel ) const;

	//DeathFX Accessors
	DamageType	GetDeathFXDamageType( HRECORD hDeathFX ) const;
	const char*	GetDeathFXModelFilename(HMODEL hDeathFX) const;
	uint32		GetDeathFXNumMaterials(HMODEL hDeathFX) const;
	const char*	GetDeathFXMaterialFilename(HMODEL hDeathFX, uint8 iMaterial) const;
	void		CopyDeathFXMaterialFilenames(HMODEL hDeathFX, char* paszDest, uint32 nNumValues, uint32 nStrLen) const;

private	:	// Members...

	HCATEGORY	m_hModelsCat;
	HCATEGORY	m_hNodesCat;
	HCATEGORY	m_hSkeletonCat;
	HCATEGORY	m_hSocketSetsCat;
	HCATEGORY	m_hSeverDamageCat;
	HCATEGORY	m_hSeverPieceCat;
	HCATEGORY	m_hSeverBodyCat;
	HCATEGORY	m_hModelDecalSetCat;
	HCATEGORY	m_hLeanCat;
	HCATEGORY	m_hSyncActionCat;
	HRECORD		m_hGlobalRecord;

};


#endif  // __MODELSDB_H__
