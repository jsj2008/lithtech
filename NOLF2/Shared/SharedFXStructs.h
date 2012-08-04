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
#include "ModelButeMgr.h"
#include "SFXMsgIDs.h"
#include "SharedMovement.h"
#include "CharacterAlignment.h"
#include "DamageTypes.h"
#include "TeamMgr.h"

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
		eZzz			= 0x04,
		eCigarette		= 0x08,
		eCigaretteSmoke	= 0x10,
	};

	CHARCREATESTRUCT() { Clear(); }
	void Clear();

	void SetSleeping(LTBOOL bSleeping) { if ( bSleeping ) byFXFlags |= eZzz; else byFXFlags &= ~eZzz; }
	inline LTBOOL IsSleeping() const { return byFXFlags & eZzz ? LTTRUE : LTFALSE; }

	void SetChatting(LTBOOL bChatting) { if ( bChatting ) byFXFlags |= eChat; else byFXFlags &= ~eChat; }
	inline LTBOOL IsChatting() const { return byFXFlags & eChat ? LTTRUE : LTFALSE; }

    virtual void Write(ILTMessage_Write *pMsg);
    virtual void Read(ILTMessage_Read *pMsg);

    bool	        bIsCinematicAI;
    LTBOOL          bIsPlayer;
    uint8           byFXFlags;
	ModelId			eModelId;
	ModelSkeleton	eModelSkeleton;
	ModelType		eModelType;
    uint8           nTrackers;
    uint8           nDimsTracker;
    LTFLOAT         fStealthPercent;
	uint8			nClientID;
	CharacterClass	eCrosshairCharacterClass;
	DamageFlags		nDamageFlags;				// What types of damage are cureently affecting us
	float			fPitch;
	bool			bCanCarry;
	bool			bCanWake;
	bool			bTracking;
	bool			bRadarVisible;
	LTVector		vHitBoxDims;
	LTVector		vHitBoxOffset;
	uint8			nCarrying;
};

inline void CHARCREATESTRUCT::Clear()
{
    bIsPlayer					= LTFALSE;
	byFXFlags					= 0;
	eModelId					= eModelIdInvalid;
	eModelSkeleton				= eModelSkeletonInvalid;
	eModelType					= eModelTypeInvalid;
	nTrackers					= 0;
	nDimsTracker				= MAIN_TRACKER;
	fStealthPercent				= 0.0f;
	nClientID					= (uint8)-1;
	eCrosshairCharacterClass	= UNKNOWN;
	nDamageFlags				= 0;
	fPitch						= 0.0f;
	bCanCarry					= false;
	bCanWake					= true;
	bTracking					= false;
	bRadarVisible				= false;
	bIsCinematicAI				= false;
	nCarrying					= CFX_CARRY_NONE;
	
	vHitBoxDims.Init();
	vHitBoxOffset.Init();
}

/////////////////////////////////////////////////////////////////////////////
//
// BODYCREATESTRUCT class Definition
//
// (used by client-side CBodyFX and server-side Body classes)
//
/////////////////////////////////////////////////////////////////////////////

struct BODYCREATESTRUCT : public SFXCREATESTRUCT
{
	BODYCREATESTRUCT( ) { Clear(); }

	void Clear();

    virtual void Write(ILTMessage_Write *pMsg);
    virtual void Read(ILTMessage_Read *pMsg);

	ModelId			eModelId;
//  uint8           nTrackers;
	BodyState		eBodyState;
	uint8			nClientId;
	bool			bCanBeCarried;
	bool			bCanBeRevived;
	DamageType		eDeathDamageType;
	bool			bPermanentBody;
	bool			bHitBoxUpdated;
	LTVector		vHitBoxDims;
	LTVector		vHitBoxOffset;
	bool			bCanBeSearched;
};

inline void BODYCREATESTRUCT::Clear()
{
	eModelId			= eModelIdInvalid;
//	nTrackers			= 0;
	eBodyState			= eBodyStateNormal;
	nClientId			= (uint8)-1;
	bCanBeCarried		= true;
	bCanBeRevived		= false;
	eDeathDamageType	= DT_INVALID;
	bPermanentBody		= false;
	bHitBoxUpdated		= false;
	bCanBeSearched		= false;

	vHitBoxDims.Init();
	vHitBoxOffset.Init();
}

/////////////////////////////////////////////////////////////////////////////
//
// STEAMCREATESTRUCT class Definition
//
// (used by client-side CSteamFX and server-side Steam classes)
//
/////////////////////////////////////////////////////////////////////////////

#ifndef _CLIENTBUILD

#define ADD_STEAM_PROPS() \
	ADD_REALPROP_FLAG(Range, 200.0f, 0) \
	ADD_REALPROP_FLAG(VolumeRadius, 10.0f, 0) \
	ADD_REALPROP_FLAG(ParticleVelocity, 75.0f, 0) \
	ADD_REALPROP_FLAG(ParticleRadius, 10000.0f, 0) \
	ADD_STRINGPROP_FLAG(Particle, "SFX\\Impact\\Spr\\Smoke.spr", PF_FILENAME) \
	ADD_REALPROP_FLAG(CreateDelta, 0.2f, 0) \
	ADD_LONGINTPROP_FLAG(NumParticles, 2, 0) \
	ADD_REALPROP_FLAG(StartAlpha, 0.5f, 0) \
	ADD_REALPROP_FLAG(EndAlpha, 0.0f, 0) \
	ADD_REALPROP_FLAG(StartScale, 1.0f, 0) \
	ADD_REALPROP_FLAG(EndScale, 2.0f, 0) \
	ADD_VECTORPROP_VAL(MinDriftVel, 0.0f, 0.0f ,0.0f) \
	ADD_VECTORPROP_VAL(MaxDriftVel, 0.0f, 20.0f ,0.0f) \
	ADD_COLORPROP(Color1, 255, 255 ,255) \
	ADD_COLORPROP(Color2, 255, 255 ,255) \
	ADD_REALPROP_FLAG(SoundRadius, 300.0f, PF_RADIUS) \
	ADD_STRINGPROP_FLAG(SoundName, "Snd\\Event\\steam.wav", PF_FILENAME)

#endif

struct STEAMCREATESTRUCT : public SFXCREATESTRUCT
{
	STEAMCREATESTRUCT();
	~STEAMCREATESTRUCT();

    virtual void Write(ILTMessage_Write *pMsg);
    virtual void Read(ILTMessage_Read *pMsg);

	// Server-side only...

	virtual	void ReadProps();

    LTFLOAT  fRange;
    LTFLOAT  fVel;
    LTFLOAT  fSoundRadius;
    LTFLOAT  fParticleRadius;
    LTFLOAT  fStartAlpha;
    LTFLOAT  fEndAlpha;
    LTFLOAT  fStartScale;
    LTFLOAT  fEndScale;
    LTFLOAT  fCreateDelta;
    LTFLOAT  fVolumeRadius;
    uint8   nNumParticles;
	HSTRING	hstrSoundName;
	HSTRING	hstrParticle;
    LTVector vColor1;
    LTVector vColor2;
    LTVector vMinDriftVel;
    LTVector vMaxDriftVel;
};

inline STEAMCREATESTRUCT::STEAMCREATESTRUCT()
{
	fRange			= 0.0f;
	fVel			= 0.0f;
	fSoundRadius	= 0.0f;
	fParticleRadius	= 0.0f;
	fStartAlpha		= 0.0f;
	fEndAlpha		= 0.0f;
	fStartScale		= 0.0f;
	fEndScale		= 0.0f;
	fCreateDelta	= 0.0f;
	fVolumeRadius	= 0.0f;
	nNumParticles	= 0;
    hstrSoundName   = LTNULL;
    hstrParticle    = LTNULL;
	vColor1.Init();
	vColor2.Init();
	vMinDriftVel.Init();
	vMaxDriftVel.Init();
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

    uint8       nImpactFX;
    LTFLOAT      fDamageRadius;
    LTVector     vPos;
    LTRotation   rRot;
};

inline EXPLOSIONCREATESTRUCT::EXPLOSIONCREATESTRUCT()
{
	nImpactFX		= 0;
	fDamageRadius	= 0.0f;
	vPos.Init();
	rRot.Init();
}

/////////////////////////////////////////////////////////////////////////////
//
// SPRINKLETYPECREATESTRUCT class Definition
//
// (used by client-side SprinklesFX and server-side Sprinkles classes)
//
/////////////////////////////////////////////////////////////////////////////

struct SPRINKLETYPECREATESTRUCT
{
    SPRINKLETYPECREATESTRUCT();
	~SPRINKLETYPECREATESTRUCT();

    virtual void Write(ILTMessage_Write *pMsg);
    virtual void Read(ILTMessage_Read *pMsg);

    HSTRING      m_hFilename;
    HSTRING      m_hSkinName;
    uint32       m_Count;
    float        m_Speed;
    float        m_Size;
    float        m_SpawnRadius;
    LTVector     m_AnglesVel;
    LTVector     m_ColorMin;
    LTVector     m_ColorMax;
};

inline SPRINKLETYPECREATESTRUCT::SPRINKLETYPECREATESTRUCT()
{
    m_hFilename     = LTNULL;
    m_hSkinName     = LTNULL;
	m_Count			= 0;
	m_Speed			= 0.0f;
	m_Size			= 0.0f;
	m_SpawnRadius	= 0.0f;
	m_AnglesVel.Init();
	m_ColorMin.Init();
	m_ColorMax.Init();
}

#define MAX_SPRINKLE_TYPES	8

/////////////////////////////////////////////////////////////////////////////
//
// SPRINKLESCREATESTRUCT class Definition
//
// (used by client-side SprinklesFX and server-side Sprinkles classes)
//
/////////////////////////////////////////////////////////////////////////////

struct SPRINKLESCREATESTRUCT : public SFXCREATESTRUCT
{
    SPRINKLESCREATESTRUCT();

    virtual void Write(ILTMessage_Write *pMsg);
    virtual void Read(ILTMessage_Read *pMsg);

    uint32                      m_nTypes;
	SPRINKLETYPECREATESTRUCT	m_Types[MAX_SPRINKLE_TYPES];

};

inline SPRINKLESCREATESTRUCT::SPRINKLESCREATESTRUCT()
{
	m_nTypes = 0;
}

/////////////////////////////////////////////////////////////////////////////
//
// LTCREATESTRUCT class Definition
//
// (used by client-side CLaserTriggerFX and server-side LaserTrigger classes)
//
/////////////////////////////////////////////////////////////////////////////

struct LTCREATESTRUCT : public SFXCREATESTRUCT
{
	LTCREATESTRUCT();
	~LTCREATESTRUCT();

    virtual void Write(ILTMessage_Write *pMsg);
    virtual void Read(ILTMessage_Read *pMsg);

    LTVector     vColor;
    LTVector     vDims;
    LTFLOAT      fAlpha;
    LTFLOAT      fSpriteScale;
    LTBOOL       bCreateSprite;
	HSTRING		hstrSpriteFilename;
};

inline LTCREATESTRUCT::LTCREATESTRUCT()
{
	vColor.Init();
	vDims.Init();
	fAlpha			= 0.0f;
	fSpriteScale	= 1.0f;
    bCreateSprite   = LTTRUE;
    hstrSpriteFilename = LTNULL;
}

/////////////////////////////////////////////////////////////////////////////
//
// MINECREATESTRUCT class Definition
//
// (used by client-side CMineFX and server-side Mine classes)
//
/////////////////////////////////////////////////////////////////////////////

struct MINECREATESTRUCT : public SFXCREATESTRUCT
{
	MINECREATESTRUCT();

    virtual void Write(ILTMessage_Write *pMsg);
    virtual void Read(ILTMessage_Read *pMsg);

    LTFLOAT  fMinRadius;
    LTFLOAT  fMaxRadius;
};

inline MINECREATESTRUCT::MINECREATESTRUCT()
{
	fMinRadius	= 0.0f;
	fMaxRadius	= 0.0f;
}


/////////////////////////////////////////////////////////////////////////////
//
// PVCREATESTRUCT class Definition
//
// (used by client-side CPlayerVehicleFX and server-side PlayerVehicle classes)
//
/////////////////////////////////////////////////////////////////////////////

struct PVCREATESTRUCT : public SFXCREATESTRUCT
{
	PVCREATESTRUCT();

    virtual void Write(ILTMessage_Write *pMsg);
    virtual void Read(ILTMessage_Read *pMsg);

	PlayerPhysicsModel	ePhysicsModel;
};

inline PVCREATESTRUCT::PVCREATESTRUCT()
{
	ePhysicsModel = PPM_NORMAL;
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
	uint8		nTriggerTypeId;
	
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
	nTriggerTypeId		( -1 ),
	nPlayerInsideID		( -1 ),
	nPlayerOutsideID	( -1 )
{

}

/////////////////////////////////////////////////////////////////////////////
//
// RADAROBJCREATESTRUCT class Definition
//
// (used by client-side CRadarObjectFX and server-side RadarObject classes)
//
/////////////////////////////////////////////////////////////////////////////

struct RADAROBJCREATESTRUCT : public SFXCREATESTRUCT
{
	RADAROBJCREATESTRUCT();

	virtual void Write( ILTMessage_Write *pMsg );
	virtual void Read( ILTMessage_Read *pMsg );

	uint8	nRadarTypeId;
	bool	bOn;
	uint8	nTeamId;
};

inline RADAROBJCREATESTRUCT::RADAROBJCREATESTRUCT()
:	SFXCREATESTRUCT		( ),
	nRadarTypeId		( -1 ),
	bOn					( true )
{
	nTeamId = INVALID_TEAM;
}

#endif  // __SHARED_FX_STRUCTS_H__