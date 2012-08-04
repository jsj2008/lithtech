// ----------------------------------------------------------------------- //
//
// MODULE  : SharedFXStructs.h
//
// PURPOSE : Shared Special FX structs
//
// CREATED : 10/21/99
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SHARED_FX_STRUCTS_H__
#define __SHARED_FX_STRUCTS_H__

#include "Globals.h"
#include "SharedBaseFXStructs.h"
#include "SFXMsgIds.h"
#include "SharedMovement.h"
#include "DamageTypes.h"
#include "TeamMgr.h"
#include "CharacterAlignment.h"
#include "ModelsDB.h"
#include "CTFDB.h"
#include "TeamClientFXDB.h"
#include "ControlPointDB.h"

/////////////////////////////////////////////////////////////////////////////
//
// CHARCREATESTRUCT class Definition
//
// (used by client-side CCharacterFX and server-side Character classes)
//
/////////////////////////////////////////////////////////////////////////////

struct CHARCREATESTRUCT : public SFXCREATESTRUCT
{
	enum Flags
	{
		eFlashLight		= 0x01,
		eChat			= 0x02,
		eSlide			= 0x04,
		eSMCharge		= 0x08,

		eFlagLast		= eSMCharge,
	};

	CHARCREATESTRUCT() { Clear(); }
	void Clear();

	void SetChatting(bool bChatting) { if ( bChatting ) byFXFlags |= eChat; else byFXFlags &= ~eChat; }
	inline bool IsChatting() const { return byFXFlags & eChat ? true : false; }

	void SetSliding(bool bSliding) { if ( bSliding ) byFXFlags |= eSlide; else byFXFlags &= ~eSlide; }
	inline bool IsSliding() const { return byFXFlags & eSlide ? true : false; }

	void SetHasSlowMoRecharge(bool bHasSlowMoRecharge) { if ( bHasSlowMoRecharge ) byFXFlags |= eSMCharge; else byFXFlags &= ~eSMCharge; }
	inline bool HasSlowMoRecharge() const { return byFXFlags & eSMCharge ? true : false; }

	virtual void Write(ILTMessage_Write *pMsg);
	virtual void Read(ILTMessage_Read *pMsg);

	bool				bIsCinematicAI;
	bool				bIsPlayer;
	bool				bIsDead;
	LTVector			vDeathDir;
	float				fDeathImpulseForce;
	ModelsDB::HNODE		hModelNodeLastHit;
	float				fDeathNodeImpulseForceScale;
	HAMMO				hDeathAmmo;
	DamageType			eDeathDamageType;
	uint8				byFXFlags;
	ModelsDB::HMODEL	hModel; // Used for sp.
	uint8				nMPModelIndex; // Used for mp
	float				fStealthPercent;
	uint8				nClientID;
	EnumCharacterStance	eCrosshairPlayerStance;
	DamageFlags			nDamageFlags;				// What types of damage are cureently affecting us
	LTVector			vHitBoxDims;
	LTVector			vHitBoxOffset;
	PlayerPhysicsModel	ePlayerPhysicsModel;
	uint32				nTimeScaleNumerator;
	uint32				nTimeScaleDenominator;
	HWEAPON				hCurWeaponRecord;
	bool				bIsSpectating;
	bool				bUseDefaultHitboxDims;
	bool				bPermanentBody;
};

inline void CHARCREATESTRUCT::Clear()
{
	bIsPlayer					= false;
	bIsDead						= false;
	vDeathDir					.Init();					
	fDeathImpulseForce			= 0.0f;
	hModelNodeLastHit			= NULL;
	fDeathNodeImpulseForceScale	= 0.0f;
	hDeathAmmo					= NULL;
	eDeathDamageType			= DT_INVALID;
	byFXFlags					= 0;
	hModel						= NULL;
	nMPModelIndex				= (uint8)-1;
	fStealthPercent				= 1.0f;
	nClientID					= INVALID_TEAM;
	eCrosshairPlayerStance		= kCharStance_Undetermined;
	nDamageFlags				= 0;
	bIsCinematicAI				= false;
	ePlayerPhysicsModel			= PPM_NORMAL;
	hCurWeaponRecord			= NULL;
	bIsSpectating				= false;
	bUseDefaultHitboxDims		= false;
	bPermanentBody				= false;

	vHitBoxDims.Init();
	vHitBoxOffset.Init();
}

/////////////////////////////////////////////////////////////////////////////
//
// EXPLOSIONCREATESTRUCT class Definition
//
// (used by client-side CExplosionFX and server-side Explosion classes)
//
/////////////////////////////////////////////////////////////////////////////

struct EXPLOSIONCREATESTRUCT : public SFXCREATESTRUCT
{
	EXPLOSIONCREATESTRUCT();

    virtual void Write(ILTMessage_Write *pMsg);
    virtual void Read(ILTMessage_Read *pMsg);

    uint8		nImpactFX;
    LTVector	vPos;
    LTRotation	rRot;
	HAMMO		hAmmo;
	float		fRadius;
	float		fMinDamageRadius;
	float		fImpulse;

};

inline EXPLOSIONCREATESTRUCT::EXPLOSIONCREATESTRUCT()
{
	nImpactFX		= 0;
	vPos.Init();
	rRot.Init();
	hAmmo = NULL;
	fRadius = 0.0f;
	fMinDamageRadius = 0.0f;
	fImpulse = 0.0f;
}

/////////////////////////////////////////////////////////////////////////////
//
// TRIGGERCREATESTRUCT class Definition
//
// (used by client-side CTriggerFX and server-side Trigger classes)
//
/////////////////////////////////////////////////////////////////////////////

struct TRIGGERCREATESTRUCT : public SFXCREATESTRUCT
{
	TRIGGERCREATESTRUCT();

	virtual void Write( ILTMessage_Write *pMsg );
	virtual void Read( ILTMessage_Read *pMsg );

	float		fHUDLookAtDist;
	float		fHUDAlwaysOnDist;
	bool		bLocked;
	LTVector	vDims;
	HRECORD		hTriggerTypeRecord;
	
	// PlayerTrigger specific
	uint32		nPlayerInsideID;
	uint32		nPlayerOutsideID;
};

inline TRIGGERCREATESTRUCT::TRIGGERCREATESTRUCT()
:	SFXCREATESTRUCT		(),
	fHUDLookAtDist		( -1.0f ),
	fHUDAlwaysOnDist	( -1.0f ),
	bLocked				( false ),
	vDims				( 0.0f, 0.0f, 0.0f ),
	hTriggerTypeRecord	( NULL ),
	nPlayerInsideID		(( uint32 )-1 ),
	nPlayerOutsideID	(( uint32 )-1 )
{

}

struct TURRETCREATESTRUCT : public SFXCREATESTRUCT
{
	TURRETCREATESTRUCT( )
	:	SFXCREATESTRUCT		( ),
		m_hTurret			( NULL ),
		m_hOperatingObject	( NULL ),
		m_hTurretWeapon		( NULL ),
		m_bRemoteActivation	( false ),
		m_nDamageState		( 0 )
	{ }

	void Write( ILTMessage_Write *pMsg )
	{
		if( !pMsg )
			return;

		pMsg->WriteDatabaseRecord( g_pLTDatabase, m_hTurret );
		pMsg->Writebool( m_hOperatingObject != NULL );
		pMsg->WriteObject( m_hOperatingObject );
		pMsg->Writebool( m_hTurretWeapon != NULL );
		pMsg->WriteObject( m_hTurretWeapon );
		pMsg->Writebool( m_bRemoteActivation );
		pMsg->Writeuint32( m_nDamageState );
	}
	
	void Read( ILTMessage_Read *pMsg )
	{
		if( !pMsg )
			return;

		m_hTurret = pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetTurretsCategory( ));
		m_bOperatingObjectValid = pMsg->Readbool();
		m_hOperatingObject = pMsg->ReadObject( );
		m_bTurretWeaponValid = pMsg->Readbool();
		m_hTurretWeapon = pMsg->ReadObject( );
		m_bRemoteActivation = pMsg->Readbool( );
		m_nDamageState = pMsg->Readuint32( );
	}

	// Record of the turret in the database...
	HTURRET		m_hTurret;

	// Object that is currently operating the turret...
	bool		m_bOperatingObjectValid;
	LTObjRef	m_hOperatingObject;

	// The server object turret weapon...
	bool		m_bTurretWeaponValid;
	LTObjRef	m_hTurretWeapon;

	// Determines if a player is allowed to directly or remotely activate the turret...
	bool		m_bRemoteActivation;

	uint32		m_nDamageState;
};

struct CTFFLAGBASECREATESTRUCT : public SFXCREATESTRUCT
{
	CTFFLAGBASECREATESTRUCT( )
	{ 
		m_hFlagBaseRec = NULL;
		m_nTeamId = INVALID_TEAM;
		m_eCTFFlagBaseState = kCTFFlagBaseState_HasFlag;
		m_bAllowSteals = true;
	}

	void Write( ILTMessage_Write *pMsg )
	{
		if( !pMsg )
			return;

		pMsg->WriteDatabaseRecord( g_pLTDatabase, m_hFlagBaseRec );

		bool bSendTeam = ( m_nTeamId != INVALID_TEAM );
		pMsg->Writebool( bSendTeam );
		if( bSendTeam )
		{
			pMsg->WriteBits(m_nTeamId, FNumBitsExclusive<MAX_TEAMS>::k_nValue);
		}

		pMsg->WriteBits( m_eCTFFlagBaseState, FNumBitsExclusive<kCTFFlagBaseState_NumStates>::k_nValue );
		pMsg->WriteObject( m_hFlag );
		pMsg->Writebool( m_bAllowSteals );
	}

	void Read( ILTMessage_Read *pMsg )
	{
		if( !pMsg )
			return;

		m_hFlagBaseRec = pMsg->ReadDatabaseRecord( g_pLTDatabase, DATABASE_CATEGORY( CTFFlagBase ).GetCategory( ));

		if( pMsg->Readbool())
		{
			m_nTeamId = pMsg->ReadBits( FNumBitsExclusive<MAX_TEAMS>::k_nValue );
		}
		else
		{
			m_nTeamId = INVALID_TEAM;
		}

		m_eCTFFlagBaseState = ( CTFFlagBaseState )pMsg->ReadBits( FNumBitsExclusive<kCTFFlagBaseState_NumStates>::k_nValue );
		m_hFlag = pMsg->ReadObject( );
		m_bAllowSteals = pMsg->Readbool();
	}

	// Record of ctfrules used to define flagbase.
	HRECORD	m_hFlagBaseRec;

	// Team this flag base belongs to.
	uint8 m_nTeamId;

	// Current state of the flagbase.
	CTFFlagBaseState m_eCTFFlagBaseState;

	// Flag we are associated with.
	LTObjRef m_hFlag;

	// Allow steals.
	bool m_bAllowSteals;
};

struct CTFFLAGCREATESTRUCT : public SFXCREATESTRUCT
{
	CTFFLAGCREATESTRUCT( )
	{ 
		m_eCTFFlagState = kCTFFlagState_InBase;
	}

	void Write( ILTMessage_Write *pMsg )
	{
		if( !pMsg )
			return;

		pMsg->WriteObject( m_hFlagBase );
		pMsg->WriteBits( m_eCTFFlagState, FNumBitsExclusive<kCTFFlagState_NumStates>::k_nValue );
	}

	void Read( ILTMessage_Read *pMsg )
	{
		if( !pMsg )
			return;

		m_hFlagBase = pMsg->ReadObject();
		m_eCTFFlagState = ( CTFFlagState )pMsg->ReadBits( FNumBitsExclusive<kCTFFlagState_NumStates>::k_nValue );
	}

	// FlagBase we're associated with.
	LTObjRef m_hFlagBase;

	// Current state of the flagbase.
	CTFFlagState m_eCTFFlagState;
};

struct TEAMCLIENTFXCREATESTRUCT : public SFXCREATESTRUCT
{
	TEAMCLIENTFXCREATESTRUCT( )
	{ 
		m_nTeamId = INVALID_TEAM;
		m_hTeamClientFXRec = NULL;
	}

	void Write( ILTMessage_Write *pMsg )
	{
		if( !pMsg )
			return;

		pMsg->WriteDatabaseRecord( g_pLTDatabase, m_hTeamClientFXRec );

		bool bSendTeam = ( m_nTeamId != INVALID_TEAM );
		pMsg->Writebool( bSendTeam );
		if( bSendTeam )
		{
			pMsg->WriteBits(m_nTeamId, FNumBitsExclusive<MAX_TEAMS>::k_nValue);
		}
	}

	void Read( ILTMessage_Read *pMsg )
	{
		if( !pMsg )
			return;

		m_hTeamClientFXRec = pMsg->ReadDatabaseRecord( g_pLTDatabase, DATABASE_CATEGORY( TeamClientFX ).GetCategory( ));

		if( pMsg->Readbool())
		{
			m_nTeamId = pMsg->ReadBits( FNumBitsExclusive<MAX_TEAMS>::k_nValue );
		}
		else
		{
			m_nTeamId = INVALID_TEAM;
		}
	}

	// Object to use as parent of clientfx.
	LTObjRef m_hParentObject;

	// Record to use.
	HRECORD m_hTeamClientFXRec;

	// Team this flag base belongs to.
	uint8 m_nTeamId;
};

/////////////////////////////////////////////////////////////////////////////
//
// SOUNDNONPOINTCREATESTRUCT class Definition
//
// (used by client-side CSoundNonPointFX and server-side SoundNonPoint classes)
//
/////////////////////////////////////////////////////////////////////////////

const int32 MAX_SOUND_VOLUMES=5;
const LTFLOAT SOUND_VOLUME_INFINITY=100000000.0f;

struct SOUNDNONPOINTZONECREATESTRUCT : public SFXCREATESTRUCT
{
	SOUNDNONPOINTZONECREATESTRUCT();

	virtual void Write(ILTMessage_Write *pMsg);
	virtual void Read(ILTMessage_Read *pMsg);

	LTVector m_vPos;
	LTVector m_vHalfDims;
	LTRotation m_rRotation;
};

struct SOUNDNONPOINTCREATESTRUCT : public SFXCREATESTRUCT
{
	SOUNDNONPOINTCREATESTRUCT();

	virtual void Write(ILTMessage_Write *pMsg);
	virtual void Read(ILTMessage_Read *pMsg);

	std::string		m_sSound;
	bool			m_bStartOn;
	bool			m_bSoundOn;
	float			m_fOuterRadius;
	float			m_fInnerRadius;
	uint8			m_nVolume;
	HRECORD			m_hFilterRecord;
	float			m_fPitchShift;
	unsigned char	m_nPriority;
	int16			m_nMixChannel;
	float			m_fDopplerFactor;
	bool			m_bUseOcclusion;
	bool			m_bOcclusionNoInnerRadius;
	int16			m_nNumZones;

	SOUNDNONPOINTZONECREATESTRUCT m_SoundZone[MAX_SOUND_VOLUMES];
};


/////////////////////////////////////////////////////////////////////////////
//
// VOLUMETRICLIGHTCREATESTRUCT class Definition
//
// (used by client-side CVolumetricLightFX and server-side LightSpot classes)
//
/////////////////////////////////////////////////////////////////////////////

struct VOLUMETRICLIGHTCREATESTRUCT : public SFXCREATESTRUCT
{
	VOLUMETRICLIGHTCREATESTRUCT();

	virtual void Read(ILTMessage_Read *pMsg);
	virtual void Write(ILTMessage_Write *pMsg);

	float						m_fNoiseIntensity;
	float						m_fNoiseScale;
	LTVector					m_vColor;
	std::string					m_sTexture;
	float						m_fAttenuation;
	float						m_fDepth;
	float						m_fFarZ;
	bool						m_bAdditive;
	bool						m_bShadow;
	LTEnum<uint8, EEngineLOD>	m_eLOD;
};

struct NAVMARKERCREATESTRUCT : public SFXCREATESTRUCT
{
	NAVMARKERCREATESTRUCT();

	virtual void Read(ILTMessage_Read *pMsg);
	virtual void Write(ILTMessage_Write *pMsg);

	bool		m_bIsActive;
	LTObjRef	m_hTarget;
	uint8		m_nClientID;
	HRECORD		m_hType;
	uint8		m_nTeamId;
	LTVector	m_vPos;
	// String id to use.
	int32		m_nStringId;
	// Secondary string to use if string id not specified.
	std::wstring m_wsString;

	bool		m_bBroadcast;
	bool		m_bInstant;
};

inline NAVMARKERCREATESTRUCT::NAVMARKERCREATESTRUCT()
{
	m_bIsActive = false;
	m_hTarget = NULL;
	m_hType = NULL;
	m_nTeamId = INVALID_TEAM;
	m_nStringId = -1;
	m_nClientID = (uint8)-1;
	m_bBroadcast = false;
	m_bInstant = false;
}

/////////////////////////////////////////////////////////////////////////////
//
// PHYSICSCONSTRAINTCREATESTRUCT class Definition
//
// (used by client-side CPhysicsConstraintFX and server-side ConstraintXXX classes)
// Not all members are sent for each constraint.  Only the required subset for the
// constraint type is sent.
//
/////////////////////////////////////////////////////////////////////////////

struct PHYSICSCONSTRAINTCREATESTRUCT : public SFXCREATESTRUCT
{
	PHYSICSCONSTRAINTCREATESTRUCT( )
	:	SFXCREATESTRUCT( )
	,	m_eConstraintType( kConstraintType_NumTypes )
	,	m_bHasObject1( false )
	,	m_hObject1( INVALID_HOBJECT )
	,	m_bHasObject2( false )
	,	m_hObject2( INVALID_HOBJECT )
	,	m_vPivotPt1( LTVector::GetIdentity( ))
	,	m_vPivotPt2( LTVector::GetIdentity( ))
	,	m_vPivotDir1( LTVector::GetIdentity( ))
	,	m_vPivotDir2( LTVector::GetIdentity( ))
	,	m_vPivotPerp1( LTVector::GetIdentity( ))
	,	m_vPivotPerp2( LTVector::GetIdentity( ))
	,	m_vAxis1( LTVector::GetIdentity( ))
	,	m_vAxis2( LTVector::GetIdentity( ))
	,	m_vAxisPerp1( LTVector::GetIdentity( ))
	,	m_vAxisPerp2( LTVector::GetIdentity( ))
	,	m_vTwist1( LTVector::GetIdentity( ))
	,	m_vTwist2( LTVector::GetIdentity( ))
	,	m_vPlane1( LTVector::GetIdentity( ))
	,	m_vPlane2( LTVector::GetIdentity( ))
	,	m_vRotation1( LTVector::GetIdentity( ))
	,	m_vRotation2( LTVector::GetIdentity( ))
	,	m_vSuspension2( LTVector::GetIdentity( ))
	,	m_fMovementMin( 0.0f )
	,	m_fMovementMax( 0.0f )
	,	m_fAngleMin( 0.0f )
	,	m_fAngleMax( 0.0f )
	,	m_fFriction( 0.0f )
	,	m_fConeAngle( 0.0f )
	,	m_fPosCone( 0.0f )
	,	m_fNegCone( 0.0f )
	,	m_fTwistMin( 0.0f )
	,	m_fTwistMax( 0.0f )
	,	m_fDistance( 0.0f )
	,	m_fSuspensionMin( 0.0f )
	,	m_fSuspensionMax( 0.0f )
	,	m_fSuspensionStrength( 0.0f )
	,	m_fSuspensionDamping( 0.0f )
	{ }

	virtual void Read( ILTMessage_Read *pMsg );
	virtual void Write( ILTMessage_Write *pMsg );

	EConstraintType	m_eConstraintType;

	bool			m_bHasObject1;
	LTObjRef		m_hObject1;

	bool			m_bHasObject2;
	LTObjRef		m_hObject2;

	LTVector		m_vPivotPt1;
	LTVector		m_vPivotPt2;

	LTVector		m_vPivotDir1;
	LTVector		m_vPivotDir2;

	LTVector		m_vPivotPerp1;
	LTVector		m_vPivotPerp2;

	LTVector		m_vAxis1;
	LTVector		m_vAxis2;

	LTVector		m_vAxisPerp1;
	LTVector		m_vAxisPerp2;

	LTVector		m_vTwist1;
	LTVector		m_vTwist2;

	LTVector		m_vPlane1;
	LTVector		m_vPlane2;

	LTVector		m_vRotation1;
	LTVector		m_vRotation2;
	
	LTVector		m_vSuspension2;

	float			m_fMovementMin;
	float			m_fMovementMax;

	float			m_fAngleMin;
	float			m_fAngleMax;

	float			m_fFriction;

	float			m_fConeAngle;

	float			m_fPosCone;
	float			m_fNegCone;

	float			m_fTwistMin;
	float			m_fTwistMax;

	float			m_fDistance;

	float			m_fSuspensionMin;
	float			m_fSuspensionMax;

	float			m_fSuspensionStrength;

	float			m_fSuspensionDamping;
};

/////////////////////////////////////////////////////////////////////////////
//
// CLIENTFXGROUPCREATESTRUCT class Definition
//
/////////////////////////////////////////////////////////////////////////////

struct CLIENTFXGROUPCREATESTRUCT : public SFXCREATESTRUCT
{
	CLIENTFXGROUPCREATESTRUCT( )
	:	m_nMsgID( 0 )
	,	m_sFXName( )
	,	m_dwFxFlags( 0 )
	,	m_hTargetObj( INVALID_HOBJECT )
	{ }

	virtual void Read( ILTMessage_Read *pMsg )
	{
		m_nMsgID = pMsg->Readuint8( );
		Read_StdString( pMsg, m_sFXName );
		m_dwFxFlags = pMsg->Readuint32( );

		if( pMsg->Readbool( ) )
		{
			m_hTargetObj = pMsg->ReadObject( );
		}
	}

	virtual void Write( ILTMessage_Write *pMsg )
	{
		pMsg->Writeuint8( m_nMsgID );
		pMsg->WriteString( m_sFXName.c_str( ) );
		pMsg->Writeuint32( m_dwFxFlags );

		bool bWriteTarget = ( m_hTargetObj != INVALID_HOBJECT );
		pMsg->Writebool( bWriteTarget );

		if( bWriteTarget )
		{
			pMsg->WriteObject( m_hTargetObj );
		}
	}
	
	uint8			m_nMsgID;
	std::string		m_sFXName;
	uint32			m_dwFxFlags;
	LTObjRef		m_hTargetObj;
};

/////////////////////////////////////////////////////////////////////////////
//
// PICKUPITEMCREATESTRUCT class Definition
//
/////////////////////////////////////////////////////////////////////////////

struct PICKUPITEMCREATESTRUCT : public SFXCREATESTRUCT
{
	PICKUPITEMCREATESTRUCT( )
	:	m_sClientFX( )
	,	m_sDroppedClientFX( )
	,	m_nTeamId( INVALID_TEAM )
	,	m_ePickupItemType( kPickupItemType_Unknown )
	,	m_hRecord( NULL )
	,	m_bLocked( false )
	,	m_hNavMarker( INVALID_HOBJECT )
	{ }

	virtual void Read( ILTMessage_Read *pMsg );
	virtual void Write( ILTMessage_Write *pMsg );

	virtual void Load( ILTMessage_Read *pMsg );
	
	std::string		m_sClientFX;
	std::string		m_sDroppedClientFX;
	uint8			m_nTeamId;
	PickupItemType	m_ePickupItemType;
	HRECORD			m_hRecord;
	bool			m_bLocked;
	LTObjRef		m_hNavMarker;
};

/////////////////////////////////////////////////////////////////////////////
//
// PROJECTILECREATESTRUCT class Definition
//
/////////////////////////////////////////////////////////////////////////////

struct PROJECTILECREATESTRUCT : public SFXCREATESTRUCT
{
	PROJECTILECREATESTRUCT( )
	:	m_hWeapon( NULL )
	,	m_hAmmo( NULL )
	,	m_bFiredFromValid( false )
	,	m_hFiredFrom( INVALID_HOBJECT )
	,	m_nTeamId( INVALID_TEAM )
	,	m_sOverrideFX( )
	,	m_sSameTeamFX( )
	,	m_sOtherTeamFX( )
	{ }

	virtual void Read( ILTMessage_Read *pMsg );
	virtual void Write( ILTMessage_Write *pMsg );

	HWEAPON		m_hWeapon;
	HAMMO		m_hAmmo;
	bool		m_bFiredFromValid;
	LTObjRef	m_hFiredFrom;
	uint8		m_nTeamId;
	std::string	m_sOverrideFX;
	std::string	m_sSameTeamFX;
	std::string m_sOtherTeamFX;
};


/////////////////////////////////////////////////////////////////////////////
//
// CONTROLPOINTCREATESTRUCT class Definition
//
/////////////////////////////////////////////////////////////////////////////
struct CONTROLPOINTCREATESTRUCT : public SFXCREATESTRUCT
{
	CONTROLPOINTCREATESTRUCT( )
	{ 
		m_hControlPointRec = NULL;
		m_eControlPointState = kControlPointState_Neutral;
		m_nControlPointId = CONTROLPOINT_INVALID_ID;
		m_vZoneDims.Init( );
	}

	void Write( ILTMessage_Write *pMsg )
	{
		if( !pMsg )
			return;

		pMsg->WriteDatabaseRecord( g_pLTDatabase, m_hControlPointRec );

		pMsg->WriteBits( m_eControlPointState, FNumBitsExclusive<kControlPointState_NumStates>::k_nValue );
		pMsg->WriteBits( m_nControlPointId, FNumBitsInclusive<MAX_CONTROLPOINT_OBJECTS>::k_nValue );
		pMsg->WriteLTVector( m_vZoneDims );
	}

	void Read( ILTMessage_Read *pMsg )
	{
		if( !pMsg )
			return;

		m_hControlPointRec = pMsg->ReadDatabaseRecord( g_pLTDatabase, DATABASE_CATEGORY( CPTypes ).GetCategory( ));

		m_eControlPointState = ( EControlPointState )pMsg->ReadBits( FNumBitsExclusive<kControlPointState_NumStates>::k_nValue );
		m_nControlPointId = ( uint8 )pMsg->ReadBits( FNumBitsInclusive<MAX_CONTROLPOINT_OBJECTS>::k_nValue );
		m_vZoneDims = pMsg->ReadLTVector();
	}

	// Record of controlpoint
	HRECORD	m_hControlPointRec;

	// Current state of the controlpoint.
	EControlPointState m_eControlPointState;

	// Unique id for this CP.
	uint16 m_nControlPointId;

	// Zone dims
	LTVector m_vZoneDims;
};


#endif  // __SHARED_FX_STRUCTS_H__

// EOF 
