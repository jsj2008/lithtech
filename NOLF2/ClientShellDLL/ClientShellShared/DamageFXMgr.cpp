// ----------------------------------------------------------------------- //
//
// MODULE  : DamageFXMgr.cpp
//
// PURPOSE : Damage FX Manager class - Implementation
//
// CREATED : 1/20/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "stdafx.h"
	#include "DamageFXMgr.h"
	#include "VarTrack.h"
	#include "SoundMgr.h"
	#include "HUDMgr.h"
	#include "GameClientShell.h"
	#include "MsgIDs.h"
	#include "CMoveMgr.h"
	#include "FXButeMgr.h"
	#include "VehicleMgr.h"
	#include "ClientWeaponMgr.h"

//
// Defines...
//

	#define DMGFXMGR_TAG					"DamageFX"

	#define	DMGFXMGR_NAME					"Name"
	#define DMGFXMGR_DAMAGETYPE				"DamageType"
	#define DMGFXMGR_HUDICON				"HudIcon"
	#define DMGFXMGR_STARTSND				"StartSound"
	#define DMGFXMGR_LOOPSND				"LoopSound"
	#define DMGFXMGR_TINTCOLOR				"TintColor"
	#define	DMGFXMGR_TINTRAMPUPTIME			"TintRampUpTime"
	#define DMGFXMGR_TINTRAMPDOWNTIME		"TintRampDownTime"
	#define	DMGFXMGR_TINTOFFTIME			"TintOffTime"
	#define DMGFXMGR_TINTONTIME				"TintOnTime"
	#define DMGFXMGR_FADETIME				"FadeTime"
	#define DMGFXMGR_ROTMAX					"RotationMax"
	#define DMGFXMGR_ROTSPEED				"RotationSpeed"
	#define DMGFXMGR_MINFXPERCENT			"MinFXPercent"
	#define DMGFXMGR_FOVMAX					"FOVMax"
	#define DMGFXMGR_FOVXSPEED				"FOVXSpeed"
	#define DMGFXMGR_FOVYSPEED				"FOVYSpeed"
	#define DMGFXMGR_LIGHTSCALESPEED		"LightScaleSpeed"
	#define DMGFXMGR_LIGHTSCALECOLOR		"LightScaleColor"
	#define DMGFXMGR_ALLOWMOVEMENT			"AllowMovement"
	#define DMGFXMGR_ALLOWINPUT				"AllowInput"
	#define DMGFXMGR_NUMJUMPSTOESCAPE		"NumJumpsToEscape"
	#define DMGFXMGR_FXNAME					"FXName"
	#define DMGFXMGR_HEALTHFX				"TakingHealthFXName"
	#define DMGFXMGR_ARMORFX				"TakingArmorFXName"
	#define DMGFXMGR_INSTANTEFFECT			"InstantEffect"
	#define DMGFXMGR_3RDPERSONFXNAME		"3rdPersonFXName"
	#define	DMGFXMGR_3RDPERSONINSTFXNAME	"3rdPersonInstantFXName"
	#define	DMGFXMGR_3RDPERSONDEATHFXNAME	"3rdPersonDeathFXName"
	#define	DMGFXMGR_BODYFXNAME				"BodyFXName"
	#define DMGFXMGR_ATTACHCAMERATOANI		"AttachCameraToAnimation"
	#define DMGFXMGR_SHOWCLIENTMODEL		"ShowLocalClientModel"
	#define DMGFXMGR_ANIMATIONCONTROLSFX	"AnimationControlsFX"


	#define	DM_SPRINKLES_TAG				"Sprinkles"

	#define KEY_DAMAGEFX					"DAMAGE_FX_KEY"


//
// Globals...
//

	CDamageFXMgr	*g_pDamageFXMgr = LTNULL;

	static char		s_aTagName[30];
	static char		s_aAttributeName[30];
	VarTrack		g_vtEnableDamageFX;


//
// Externs...
//

	extern VarTrack g_vtFOVXNormal;
	extern VarTrack g_vtFOVYNormal;
	extern CGameClientShell* g_pGameClientShell;


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDamageFXMgr::CDamageFXMgr
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CDamageFXMgr::CDamageFXMgr()
:	CGameButeMgr				( ),
	m_vDamageFXTintColor		( 0.0f, 0.0f, 0.0f ),
	m_bTintColorChanged			( LTFALSE ),
	m_vDamageFXLightScaleColor	( 0.0f, 0.0f, 0.0f ),
	m_bLightScaleColorChanged	( false ),
	m_bAllowMovement			( true ),
	m_nDisableWeaponCounts		( 0 )
{
    m_lstDamageFx.Init( LTTRUE );
	m_lstActiveDmgFx.Init( LTFALSE );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDamageFXMgr::~CDamageFXMgr
//
//  PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CDamageFXMgr::~CDamageFXMgr()
{
	Term();
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDamageFXMgr::Term
//
//  PURPOSE:	Clean up after ourselfs
//
// ----------------------------------------------------------------------- //

void CDamageFXMgr::Term()
{
	g_pDamageFXMgr = LTNULL;

	m_lstDamageFx.Clear();
	m_lstActiveDmgFx.Clear();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDamageFXMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CDamageFXMgr::Init(const char* szAttributeFile)
{
	if( g_pDamageFXMgr || !szAttributeFile ) return LTFALSE;
    if( !Parse( szAttributeFile )) return LTFALSE;

	// Set the global ptr

	g_pDamageFXMgr = this;

	// Read in the properties for each Damage FX type...

	int nNum = 0;
	sprintf( s_aTagName, "%s%d", DMGFXMGR_TAG, nNum );

	while( m_buteMgr.Exist( s_aTagName ))
	{
		DAMAGEFX	*pDamageFX = debug_new( DAMAGEFX );

		if( pDamageFX && pDamageFX->Init( m_buteMgr, s_aTagName ))
		{
			// Set the ID and add it on the list...

			pDamageFX->m_nID = nNum;
			m_lstDamageFx.AddTail( pDamageFX );
		}
		else
		{
			debug_delete( pDamageFX );
			return LTFALSE;
		}

		++nNum;
		sprintf( s_aTagName, "%s%d", DMGFXMGR_TAG, nNum );
	}

	g_vtEnableDamageFX.Init( g_pLTClient, "EnableDamageFX", LTNULL, 1.0f );

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	*CDamageFXMgr::GetDamageFX
//
//  PURPOSE:	Get the specified DamageFX record
//
// ----------------------------------------------------------------------- //

DAMAGEFX *CDamageFXMgr::GetDamageFX( uint32 nID )
{
	DAMAGEFX **pCur = LTNULL;

	pCur = m_lstDamageFx.GetItem( TLIT_FIRST );

	while( pCur )
	{
		if( *pCur && (*pCur)->m_nID == nID )
		{
			return *pCur;
		}

		pCur = m_lstDamageFx.GetItem( TLIT_NEXT );
	}

	return LTNULL;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	*CDamageFXMgr::GetDamageFX
//
//  PURPOSE:	Get the specified DamageFX record
//
// ----------------------------------------------------------------------- //

DAMAGEFX *CDamageFXMgr::GetDamageFX( char *pName )
{
	DAMAGEFX **pCur = LTNULL;

	pCur = m_lstDamageFx.GetItem( TLIT_FIRST );

	while( pCur )
	{
		if( *pCur && (*pCur)->m_szName[0] && (!_stricmp( (*pCur)->m_szName, pName )) )
		{
			return *pCur;
		}

		pCur = m_lstDamageFx.GetItem( TLIT_NEXT );
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDamageFXMgr::Update
//
//  PURPOSE:	Update all active damage fx
//
// ----------------------------------------------------------------------- //

void CDamageFXMgr::Update()
{
	if( g_vtEnableDamageFX.GetFloat() < 1.0f )
	{
		DAMAGEFX *pDamageFX = GetFirstDamageFX();
		while( pDamageFX )
		{
			pDamageFX->m_vtTestFX.SetFloat( 0.0f );

			pDamageFX = GetNextDamageFX();
		}

		Clear();
		return;
	}

	//if we are using an external camera, make sure to clear anything that might be causing rendering
	//issues
	if(g_pPlayerMgr->IsUsingExternalCamera())
	{
		LTVector vNoTint(0.0f, 0.0f, 0.0f);
		g_pGameClientShell->GetScreenTintMgr()->Set( TINT_DAMAGEFX, &vNoTint );

		g_pGameClientShell->GetLightScaleMgr()->ClearLightScale(CLightScaleMgr::eLightScaleDamage);

		//and we don't need to update
		return;
	}

	//don't bother updating if we are paused, or using an alternate camera
	if(!g_pInterfaceMgr->IsInGame( ) ||g_pGameClientShell->IsGamePaused())
		return;

	m_bTintColorChanged = LTFALSE;
	m_vDamageFXTintColor.Init();

	m_bLightScaleColorChanged = LTFALSE;
	m_vDamageFXLightScaleColor.Init( 1.0f, 1.0f, 1.0f );

	m_bAllowMovement	= true;
	m_bAllowInput		= true;
	bool bHadDisableWeaponCounts = ( m_nDisableWeaponCounts > 0 );
	m_nDisableWeaponCounts = 0;

	// Update all active Damage FX...
	
	DAMAGEFX *pDamageFX = GetFirstActiveFX();
	while( pDamageFX )
	{
		pDamageFX->Update(g_pGameClientShell->GetFrameTime());
		
		pDamageFX = GetNextActiveFX();
	}

	if(m_bTintColorChanged)
	{
		g_pGameClientShell->GetScreenTintMgr()->Set( TINT_DAMAGEFX, &m_vDamageFXTintColor );
	}
	else
	{
		LTVector vNoTint(0.0f, 0.0f, 0.0f);
		g_pGameClientShell->GetScreenTintMgr()->Set( TINT_DAMAGEFX, &vNoTint );
	}

	if( m_bLightScaleColorChanged )
		g_pGameClientShell->GetLightScaleMgr()->SetLightScale( m_vDamageFXLightScaleColor, CLightScaleMgr::eLightScaleDamage );
	else
		g_pGameClientShell->GetLightScaleMgr()->ClearLightScale(CLightScaleMgr::eLightScaleDamage);

	// Check if we should try to enable our weapons.
	if( m_nDisableWeaponCounts == 0 && bHadDisableWeaponCounts )
	{
		g_pPlayerMgr->GetClientWeaponMgr()->EnableWeapons();
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDamageFXMgr::Clear
//
//  PURPOSE:	Stop every active damage fx
//
// ----------------------------------------------------------------------- //

void CDamageFXMgr::Clear()
{
	// Stop all Damage FX...
	
	DAMAGEFX *pDamageFX = GetFirstDamageFX();
	while( pDamageFX )
	{
		pDamageFX->Stop( LTFALSE );
	
		pDamageFX = GetNextDamageFX();
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDamageFXMgr::IsDamageActive
//
//  PURPOSE:	Given damage flags, find if we are taking damage of that type
//
// ----------------------------------------------------------------------- //

LTBOOL CDamageFXMgr::IsDamageActive( DamageFlags nDmgFlag )
{
	// Detrimine if any damage fx related to the passed in flags are active...
	
	DAMAGEFX *pDamageFX = GetFirstActiveFX();
	while( pDamageFX )
	{
		if( nDmgFlag & pDamageFX->m_nDamageFlag )
		{
			return LTTRUE;
		}

		pDamageFX = GetNextActiveFX();
	}

	return LTFALSE;
}




// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDamageFXMgr::IsFOVAffected
//
//  PURPOSE:	Are any of our active damage FX affecting to camera FOV
//
// ----------------------------------------------------------------------- //

bool CDamageFXMgr::IsFOVAffected()
{
	// Detrimine if any damage fx related to the passed in flags are active...
	
	DAMAGEFX *pDamageFX = GetFirstActiveFX();
	while( pDamageFX )
	{
		if( pDamageFX->m_fFOVMax > 0.0f  )
		{
			return true;
		}

		pDamageFX = GetNextActiveFX();
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDamageFXMgr::SetDamageFXTintColor
//
//  PURPOSE:	Set the new tint color for the damage fx
//
// ----------------------------------------------------------------------- //

void CDamageFXMgr::SetDamageFXTintColor( LTVector &vCol, bool bForce )
{
	if( m_bTintColorChanged )
	{
		m_vDamageFXTintColor = vCol;
		g_pGameClientShell->GetScreenTintMgr()->Set( TINT_DAMAGEFX, &m_vDamageFXTintColor );
	}
	else if( m_vDamageFXTintColor != vCol )
	{
		m_vDamageFXTintColor.x = Max( m_vDamageFXTintColor.x, vCol.x );
		m_vDamageFXTintColor.y = Max( m_vDamageFXTintColor.y, vCol.y );
		m_vDamageFXTintColor.z = Max( m_vDamageFXTintColor.z, vCol.z );	

		m_bTintColorChanged = LTTRUE;
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDamageFXMgr::SetDamageFXLightScale
//
//  PURPOSE:	Set the new light scale color for the damage fx
//
// ----------------------------------------------------------------------- //

void CDamageFXMgr::SetDamageFXLightScale( LTVector &vCol, bool bForce )
{
	//if the color is just bright white, don't set it
	if(vCol.NearlyEquals(LTVector(1.0f, 1.0f, 1.0f), 0.01f))
		return;

	if( bForce )
	{
		m_vDamageFXLightScaleColor = vCol;
		m_bLightScaleColorChanged = LTTRUE;
	}
	else 
	{
		m_vDamageFXLightScaleColor.x = Min( m_vDamageFXLightScaleColor.x, vCol.x );
		m_vDamageFXLightScaleColor.y = Min( m_vDamageFXLightScaleColor.y, vCol.y );
		m_vDamageFXLightScaleColor.z = Min( m_vDamageFXLightScaleColor.z, vCol.z );
		m_bLightScaleColorChanged = LTTRUE;
	}

}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDamageFXMgr::SetDamageFXAllowMovement
//
//  PURPOSE:	Sets weather or not any FX doesn't allow movement.
//
// ----------------------------------------------------------------------- //

void CDamageFXMgr::SetDamageFXMovementAndInput( bool bMove, bool bInput, bool bAllowWeapons )
{
	m_bAllowMovement = bMove;
	m_bAllowInput = bInput;
	if( !bAllowWeapons )
		m_nDisableWeaponCounts++;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDamageFXMgr::OnModelKey
//
//  PURPOSE:	Handle model key
//
// ----------------------------------------------------------------------- //

bool CDamageFXMgr::OnModelKey( HLOCALOBJ hObj, ArgList *pArgs )
{
	if (!hObj || !pArgs || !pArgs->argv || pArgs->argc == 0) return false;

	char* pKey = pArgs->argv[0];
	if (!pKey) return false;

	if( stricmp( pKey, KEY_DAMAGEFX ) == 0 )
	{
		// Start or Stop FX if they are controlled by the animation...

		if( (pArgs->argc > 1) && pArgs->argv[1] )
		{
			if( stricmp( pArgs->argv[1], "START") == 0 )
			{
				DAMAGEFX *pDamageFX = GetFirstActiveFX();
				while( pDamageFX )
				{
					if( pDamageFX->m_bAnimationControlsFX )
					{
						pDamageFX->StartSoundAndVisuals();
					}

					pDamageFX = GetNextActiveFX();
				}
			}
			else if( stricmp( pArgs->argv[1], "STOP" ) == 0 )
			{
				DAMAGEFX *pDamageFX = GetFirstActiveFX();
				while( pDamageFX )
				{
					if( pDamageFX->m_bAnimationControlsFX )
					{
						pDamageFX->StopSoundAndVisuals();
					}

					pDamageFX = GetNextActiveFX();
				}
			}
		}

		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	DAMAGEFX::DAMAGEFX
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

DAMAGEFX::DAMAGEFX()
:	m_nID					( DMGFXMGR_INVALID_ID ),
	m_nDamageFlag			( 0 ),
	m_szName				( LTNULL ),
	m_szIcon				( LTNULL ),
	m_szStartSound			( LTNULL ),
	m_szLoopSound			( LTNULL ),
	m_szFXName				( LTNULL ),
	m_szTakingHealthFXName	( LTNULL ),
	m_szTakingArmorFXName	( LTNULL ),
	m_sz3rdPersonFXName		( LTNULL ),
	m_sz3rdPersonInstFXName	( LTNULL ),
	m_sz3rdPersonDeathFXName( LTNULL ),
	m_szBodyFXName			( LTNULL ),
	m_vMaxTintColor			( 0.0f, 0.0f, 0.0f ),
	m_vCurTintColor			( 0.0f, 0.0f, 0.0f ),
	m_TintDir				( DAMAGEFX_TINT_RAMPUP ),
	m_fTintRampUpTm			( 0.0f ),
	m_fTintRampDownTm		( 0.0f ),
	m_fTintOffTm			( 0.0f ),
	m_fTintOnTm				( 0.0f ),
	m_fRotMax				( 0.0f ),
	m_fRotSpeed				( 0.0f ),
	m_fRotDir				( 0.0f ),
	m_fOffsetRot			( 0.0f ),
	m_fMaxRot				( 0.0f ),
	m_fMinRot				( 0.0f ),
	m_fMoveMult				( 0.0f ),
	m_fMinFXPercent			( 0.0f ),
	m_fFOVXSpeed			( 0.0f ),
	m_fFOVYSpeed			( 0.0f ),
	m_fFOVMax				( 0.0f ),
	m_fFOVXOffset			( 0.0f ),
    m_fFOVYOffset			( 0.0f ),
    m_fFOVXDir				( 1.0f ),
    m_fFOVYDir				( 1.0f ),
	m_fLightScale			( 0.0f ),
	m_fLightScaleSpeed		( 0.0f ),
	m_vLightScaleColor		( 0.0f, 0.0f, 0.0f ),
	m_bActive				( LTFALSE ),
	m_bFade					( LTFALSE ),
	m_fFadeTm				( 0.0f ),
	m_bAllowMovement		( true ),
	m_bAllowInput			( true ),
	m_bAllowWeapons			( true ),
	m_hLoopSound			( LTNULL ),
	m_pSprinkles			( LTNULL ),
	m_nNumSprinkles			( 0 ),
	m_bJumpRequested		( LTFALSE ),
	m_nNumJumps				( 0 ),
	m_nJumpsToEscape		( -1 ),
	m_bAttachCameraToAni	( LTFALSE ),
	m_bShowClientModel		( LTFALSE ),
	m_bAnimationControlsFX	( LTFALSE ),
	m_bUpdateSoundAndVisuals( LTFALSE ),
	m_fElapsedTime			( 0.0f ),
	m_fEndTime				( 0.0f )
{
	for( int i = 0; i < MAX_SPRINKLE_TYPES; ++i )
	{
		m_aszSprinkleName[i] = LTNULL;
	}

	m_szVarTrackName[0] = '\0';
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	DAMAGEFX::~DAMAGEFX
//
//  PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

DAMAGEFX::~DAMAGEFX()
{
	debug_deletea( m_szName );
	debug_deletea( m_szIcon );
	debug_deletea( m_szStartSound );
	debug_deletea( m_szLoopSound );
	debug_deletea( m_szFXName );
	debug_deletea( m_szTakingHealthFXName );
	debug_deletea( m_szTakingArmorFXName );
	debug_deletea( m_sz3rdPersonFXName );
	debug_deletea( m_sz3rdPersonInstFXName );
	debug_deletea( m_sz3rdPersonDeathFXName );
	debug_deletea( m_szBodyFXName );

	if( m_hLoopSound )
	{
		g_pLTClient->SoundMgr()->KillSound( m_hLoopSound );
		m_hLoopSound = LTNULL;
	}

	for( int i = 0; i < MAX_SPRINKLE_TYPES; ++i )
	{
		debug_deletea( m_aszSprinkleName[i] );
	}

	DestroySprinkles();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	DAMAGEFX::Init
//
//  PURPOSE:	Build the DamageFX struct
//
// ----------------------------------------------------------------------- //

LTBOOL DAMAGEFX::Init( CButeMgr &ButeMgr, char *aTagName )
{
	if( !aTagName ) return LTFALSE;

	m_szName					= GetString( ButeMgr, aTagName, DMGFXMGR_NAME, DMGFXMGR_MAX_NAME_LEN );
	m_szIcon					= GetString( ButeMgr, aTagName, DMGFXMGR_HUDICON, DMGFXMGR_MAX_PATH );
	m_szStartSound				= GetString( ButeMgr, aTagName, DMGFXMGR_STARTSND, DMGFXMGR_MAX_PATH );
	m_szLoopSound				= GetString( ButeMgr, aTagName, DMGFXMGR_LOOPSND, DMGFXMGR_MAX_PATH );
	m_szFXName					= GetString( ButeMgr, aTagName, DMGFXMGR_FXNAME, DMGFXMGR_MAX_NAME_LEN );
	m_szTakingHealthFXName		= GetString( ButeMgr, aTagName, DMGFXMGR_HEALTHFX, DMGFXMGR_MAX_NAME_LEN );
	m_szTakingArmorFXName		= GetString( ButeMgr, aTagName, DMGFXMGR_ARMORFX, DMGFXMGR_MAX_NAME_LEN );
	m_sz3rdPersonFXName			= GetString( ButeMgr, aTagName, DMGFXMGR_3RDPERSONFXNAME, DMGFXMGR_MAX_NAME_LEN );
	m_sz3rdPersonInstFXName		= GetString( ButeMgr, aTagName, DMGFXMGR_3RDPERSONINSTFXNAME, DMGFXMGR_MAX_NAME_LEN );
	m_sz3rdPersonDeathFXName	= GetString( ButeMgr, aTagName, DMGFXMGR_3RDPERSONDEATHFXNAME, DMGFXMGR_MAX_NAME_LEN );
	m_szBodyFXName				= GetString( ButeMgr, aTagName, DMGFXMGR_BODYFXNAME, DMGFXMGR_MAX_NAME_LEN );

	m_nNumSprinkles = 0;
	sprintf( s_aAttributeName, "%s%d", DM_SPRINKLES_TAG, m_nNumSprinkles );
	while( ButeMgr.Exist( aTagName, s_aAttributeName ) && m_nNumSprinkles < MAX_SPRINKLE_TYPES )
	{
		m_aszSprinkleName[m_nNumSprinkles] = GetString( ButeMgr, aTagName, s_aAttributeName, DMGFXMGR_MAX_NAME_LEN );

		++m_nNumSprinkles;
		sprintf( s_aAttributeName, "%s%d", DM_SPRINKLES_TAG, m_nNumSprinkles );
	}
	
	m_fTintRampUpTm		= (LTFLOAT)ButeMgr.GetDouble( aTagName, DMGFXMGR_TINTRAMPUPTIME );
	m_fTintRampDownTm	= (LTFLOAT)ButeMgr.GetDouble( aTagName, DMGFXMGR_TINTRAMPDOWNTIME );
	m_fTintOffTm		= (LTFLOAT)ButeMgr.GetDouble( aTagName, DMGFXMGR_TINTOFFTIME );
	m_fTintOnTm			= (LTFLOAT)ButeMgr.GetDouble( aTagName, DMGFXMGR_TINTONTIME );
	m_fFadeTm			= (LTFLOAT)ButeMgr.GetDouble( aTagName, DMGFXMGR_FADETIME );
	
	m_vMaxTintColor		= ButeMgr.GetVector( aTagName, DMGFXMGR_TINTCOLOR );
	m_vMaxTintColor		*= MATH_ONE_OVER_255;

	m_fRotMax			= (LTFLOAT)ButeMgr.GetDouble( aTagName, DMGFXMGR_ROTMAX );
	m_fRotSpeed			= (LTFLOAT)ButeMgr.GetDouble( aTagName, DMGFXMGR_ROTSPEED );

	m_fMinFXPercent		= (LTFLOAT)ButeMgr.GetDouble( aTagName, DMGFXMGR_MINFXPERCENT );
	m_fMinFXPercent		= LTCLAMP( m_fMinFXPercent, 0.0f, 1.0f );

	m_fFOVMax			= (LTFLOAT)ButeMgr.GetDouble( aTagName, DMGFXMGR_FOVMAX );
	m_fFOVXSpeed		= (LTFLOAT)ButeMgr.GetDouble( aTagName, DMGFXMGR_FOVXSPEED );
	m_fFOVYSpeed		= (LTFLOAT)ButeMgr.GetDouble( aTagName, DMGFXMGR_FOVYSPEED );

	m_fLightScaleSpeed	= (LTFLOAT)ButeMgr.GetDouble( aTagName, DMGFXMGR_LIGHTSCALESPEED );
	m_vLightScaleColor	= ButeMgr.GetVector( aTagName, DMGFXMGR_LIGHTSCALECOLOR );
	m_vLightScaleColor	*= MATH_ONE_OVER_255;

	m_bAllowMovement	= !!ButeMgr.GetInt( aTagName, DMGFXMGR_ALLOWMOVEMENT );
	m_bAllowInput		= !!ButeMgr.GetInt( aTagName, DMGFXMGR_ALLOWINPUT );

	m_nJumpsToEscape	= ButeMgr.GetInt( aTagName, DMGFXMGR_NUMJUMPSTOESCAPE );

	m_bInstantEffect	= (LTBOOL)ButeMgr.GetInt( aTagName, DMGFXMGR_INSTANTEFFECT, 0 );

	m_bAttachCameraToAni= (LTBOOL)ButeMgr.GetInt( aTagName, DMGFXMGR_ATTACHCAMERATOANI, 0 );
	m_bShowClientModel	= (LTBOOL)ButeMgr.GetInt( aTagName, DMGFXMGR_SHOWCLIENTMODEL, 0 );

	m_bAnimationControlsFX = (LTBOOL)ButeMgr.GetInt( aTagName, DMGFXMGR_ANIMATIONCONTROLSFX, 0 );

	// Set up the flag for testing against this DamageFX...

	int nDamageType = StringToDamageType( ButeMgr.GetString( aTagName, DMGFXMGR_DAMAGETYPE ));
	m_nDamageFlag = DamageFlags( (DamageFlags)1 << (DamageFlags)nDamageType );

	// Init the VarTrack for easy testing...

	sprintf( m_szVarTrackName, "Test%sFX", m_szName );

	m_vtTestFX.Init( g_pLTClient, m_szVarTrackName, LTNULL, 0.0f );

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	DAMAGEFX::Start
//
//  PURPOSE:	Start the appropriate fx for either the local or non local characters...
//
// ----------------------------------------------------------------------- //

void DAMAGEFX::Start( )
{
	if( m_bActive || !g_pMoveMgr || !g_pHUDMgr || !g_pDamageFXMgr || !g_pClientFXMgr || g_pPlayerMgr->IsPlayerDead() )
		return;

	m_bActive = LTTRUE;

	// Add ourselves to the global active list

	g_pDamageFXMgr->m_lstActiveDmgFx.Add( this );

	// Update the hud icon

	g_pHUDMgr->QueueUpdate( kHUDDamage );

	// Should we start sound and visual fx now or wait for the animation...

	m_bUpdateSoundAndVisuals = !m_bAnimationControlsFX || g_pMoveMgr->IsBodyOnLadder();
	if( m_bUpdateSoundAndVisuals )
	{
		StartSoundAndVisuals();
	}

	m_fFOVXOffset = 0.0f;
	m_fFOVYOffset = 0.0f;
	m_fLightScale = 0.0f;

	// Clear our jump count

	m_nNumJumps = 0;

	m_bAllowWeapons = true;

	if( !g_pMoveMgr->GetVehicleMgr()->IsVehiclePhysics() )
	{
		if( m_bShowClientModel )
			g_pCommonLT->SetObjectFlags( g_pLTClient->GetClientObject(), OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE );

		if( m_bAttachCameraToAni )
		{
			g_pPlayerMgr->AttachCameraToHead( true, IsMultiplayerGame() );
			m_bAllowWeapons = false;
			g_pPlayerMgr->GetClientWeaponMgr()->DisableWeapons();
			
			// Send message to server so all clients can stop the sound...
			// An id of invalid means stop

			IClientWeaponBase *pWeapon = g_pPlayerMgr->GetCurrentClientWeapon();
			if( pWeapon )
			{
				CAutoMessage cMsg;
				cMsg.Writeuint8( MID_WEAPON_SOUND_LOOP );
				cMsg.Writeuint8( PSI_INVALID );
				cMsg.Writeuint8( pWeapon->GetWeaponId() );
				g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
			}
		}

		if (!m_bAllowInput)
			g_pPlayerMgr->AllowPlayerMovement(LTFALSE);

	}

}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	DAMAGEFX::Stop
//
//  PURPOSE:	Stop the appropriate fx for either the local or non local characters...
//
// ----------------------------------------------------------------------- //

void DAMAGEFX::Stop( LTBOOL bFade /* = LTTRUE  */ )
{
	if( !m_bActive || !g_pPlayerMgr || !g_pClientFXMgr )
		return;

	m_bActive = LTFALSE;
	m_bUpdateSoundAndVisuals = LTFALSE;
	
	if( !m_bAnimationControlsFX || g_pPlayerMgr->IsPlayerDead() || g_pMoveMgr->IsBodyOnLadder() )
	{
		StopSoundAndVisuals( bFade );
	}

	// We no longer want to see the model

	if( m_bShowClientModel )
		g_pCommonLT->SetObjectFlags( g_pLTClient->GetClientObject(), OFT_Flags, 0, FLAG_VISIBLE );

	if( m_bAttachCameraToAni )
	{
		// Reset the player movement before detaching the camera from the head...

		if (!m_bAllowInput)
			g_pPlayerMgr->AllowPlayerMovement(LTTRUE);

		g_pPlayerMgr->AttachCameraToHead( false );

		if( !m_bAllowWeapons )
			g_pPlayerMgr->GetClientWeaponMgr()->EnableWeapons();
		
		IClientWeaponBase *pWeapon = g_pPlayerMgr->GetClientWeaponMgr()->GetCurrentClientWeapon();
		if( pWeapon )
		{
			pWeapon->ClearFiring();
		}
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	DAMAGEFX::Update
//
//  PURPOSE:	Update an active DamageFX
//
// ----------------------------------------------------------------------- //

void DAMAGEFX::Update( float fElapsedTime )
{
	if( !g_pPlayerMgr || !g_pMoveMgr || !g_pDamageFXMgr )
		return;
	
	HLOCALOBJ hCamera = g_pPlayerMgr->GetCamera();
	if( !hCamera ) 
		return;

	//update our elapsed time
	m_fElapsedTime += fElapsedTime;

	// ABM 5/6/02 Quick check on non-looping FX to see if they need to terminate
	if (m_bInstantEffect && m_DamageFXInstance.IsValid())
	{
		if (m_DamageFXInstance.GetInstance()->IsDone())
		{
			Stop();
			return;
		}
	}

	// If we want to see the model make sure to continualy set it visible (it gets set invisible elsewhere)...

	if( m_bActive && m_bShowClientModel && !g_pMoveMgr->GetVehicleMgr()->IsVehiclePhysics() )
		g_pCommonLT->SetObjectFlags( g_pLTClient->GetClientObject(), OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE );

	// See if we can free ourselfs by jumping (Glue Bomb, Bear Trap)...

	if( (m_nJumpsToEscape > 0) && m_bActive )
	{
		if( g_pLTClient->IsCommandOn( COMMAND_ID_JUMP ) && !m_bJumpRequested )
		{
			// Tap

			m_bJumpRequested = LTTRUE;
			++m_nNumJumps;
		}
		else if( !g_pLTClient->IsCommandOn( COMMAND_ID_JUMP ))
		{
			// Ok the player let go of the jump key... let them tap again

			m_bJumpRequested = LTFALSE;
		}

		if( m_nNumJumps >= m_nJumpsToEscape )
		{
			// End the effect on the client

			Stop();

			// Send message to server to clear progressive damage.

			CAutoMessage cMsg;
			cMsg.Writeuint8( MID_CLEAR_PROGRESSIVE_DAMAGE );
			cMsg.Writeuint64( m_nDamageFlag );
			g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
			
		}
	}
	
	LTFLOAT fMove		= g_pPlayerMgr->GetMoveMgr()->GetMovementPercent();
	LTFLOAT fFrameTime	= g_pGameClientShell->GetFrameTime();

	LTFLOAT	fFovX, fFovY;
	g_pLTClient->GetCameraFOV(hCamera, &fFovX, &fFovY);
	
	m_fMoveMult = m_fMinFXPercent + (fMove * (1.0f - m_fMinFXPercent));

	LTRotation rot;
	g_pLTClient->GetObjectRotation( hCamera, &rot );
    LTVector vF = rot.Forward();

	if( m_bActive && m_bUpdateSoundAndVisuals )
	{
		// FOV

		LTFLOAT fXSpeed = m_fFOVXSpeed * fFrameTime * m_fFOVXDir;
        LTFLOAT fYSpeed = m_fFOVYSpeed * fFrameTime * m_fFOVYDir;

		m_fFOVXOffset += fXSpeed;
		if( m_fFOVXOffset > m_fFOVMax )
		{
			m_fFOVXOffset = m_fFOVMax;
			m_fFOVXDir = -m_fFOVXDir;
		}
		else if( m_fFOVXOffset < -m_fFOVMax )
		{
			m_fFOVXOffset = -m_fFOVMax;
			m_fFOVXDir = -m_fFOVXDir;
		}


		m_fFOVYOffset += fYSpeed;
		if( m_fFOVYOffset > m_fFOVMax )
		{
			m_fFOVYOffset = m_fFOVMax;
			m_fFOVYDir = -m_fFOVYDir;
		}
		else if( m_fFOVYOffset < -m_fFOVMax )
		{
			m_fFOVYOffset = -m_fFOVMax;
			m_fFOVYDir = -m_fFOVYDir;
		}
		

		// ROTATION

		LTFLOAT fRotSpeed = m_fRotSpeed * fFrameTime * m_fRotDir;

		m_fOffsetRot += fRotSpeed;
		if( m_fOffsetRot >= m_fMaxRot )
		{
			m_fOffsetRot = m_fMaxRot;
			m_fRotDir = -1.0f;
			m_fMinRot = -m_fRotMax * GetRandom( 0.5f, 1.0f );
		}
		else if( m_fOffsetRot <= m_fMinRot )
		{
			m_fOffsetRot = m_fMinRot;
			m_fRotDir = 1.0f;
			m_fMaxRot = m_fRotMax * GetRandom( 0.5f, 1.0f );
		}

	}
	else if( m_bFade )
	{
		LTBOOL	bDone = LTFALSE;
		
		if( m_fElapsedTime >= m_fEndTime )
		{
			bDone = LTTRUE;
		}
		else
		{
			// FOV
					
			LTFLOAT fXSpeed = m_fFOVXSpeed * fFrameTime;
			LTFLOAT fYSpeed = m_fFOVYSpeed * fFrameTime;

			if( m_fFOVXOffset < -fXSpeed )
			{
				m_fFOVXOffset += fXSpeed;
				bDone = LTFALSE;
			}
			else if( m_fFOVXOffset > fXSpeed )
			{
				m_fFOVXOffset -= fXSpeed;
				bDone = LTFALSE;
			}
			else
			{
				m_fFOVXOffset = 0.0f;
			}

			if( m_fFOVYOffset < -fYSpeed )
			{
				m_fFOVYOffset += fYSpeed;
				bDone = LTFALSE;
			}
			else if( m_fFOVYOffset > fYSpeed )
			{
				m_fFOVYOffset -= fYSpeed;
				bDone = LTFALSE;
			}
			else
			{
				m_fFOVYOffset = 0.0f;
			}

			// ROTATION

			LTFLOAT fRotSpeed = m_fRotSpeed * fFrameTime;

			if( m_fOffsetRot < -fRotSpeed )
			{
				m_fOffsetRot += fRotSpeed;
				bDone = LTFALSE;
			}
			else if( m_fOffsetRot > fRotSpeed )
			{
				m_fOffsetRot -= fRotSpeed;
				bDone = LTFALSE;
			}
			else
			{
				m_fOffsetRot = 0.0f;
			}

		}
	
		if( bDone )
			m_bFade = LTFALSE;
	}
	else
	{
		fFovX			= DEG2RAD( g_vtFOVXNormal.GetFloat() );
		fFovY			= DEG2RAD( g_vtFOVYNormal.GetFloat() );
		m_fFOVXOffset	= 0.0f;
		m_fFOVYOffset	= 0.0f;
		m_fOffsetRot	= 0.0f;
	}

	// Update the overall damage fx screen tint based on this damagefx's screen tint

	{
		bool bForce = ( !m_bActive && !m_bUpdateSoundAndVisuals && !m_bFade);
		g_pDamageFXMgr->SetDamageFXTintColor( UpdateTintColor(), bForce );
		g_pDamageFXMgr->SetDamageFXLightScale( UpdateLightScale(), bForce );

		fFovX += m_fFOVXOffset * m_fMoveMult;
		fFovY += m_fFOVYOffset * m_fMoveMult;

		if (!g_pPlayerMgr->IsZoomed() && !g_pPlayerMgr->IsZooming())
			g_pLTClient->SetCameraFOV( hCamera, fFovX, fFovY );

		rot.Rotate( vF, m_fOffsetRot * m_fMoveMult );
		
		if( !g_pMoveMgr->GetVehicleMgr()->IsVehiclePhysics() )
			g_pLTClient->SetObjectRotation( hCamera, &rot );
	}

	// Set our movement and input...
	g_pDamageFXMgr->SetDamageFXMovementAndInput( m_bAllowMovement, m_bAllowInput, m_bAllowWeapons );

	if( !IsActive() )
	{
		g_pHUDMgr->QueueUpdate( kHUDDamage );
		
		// Remove from the Activelist

		g_pDamageFXMgr->m_lstActiveDmgFx.Remove( this );
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	DAMAGEFX::UpdateTintColor
//
//  PURPOSE:	Update the current tint color
//
// ----------------------------------------------------------------------- //

LTVector DAMAGEFX::UpdateTintColor( )
{
	LTVector vRes( 0.0f, 0.0f, 0.0f );

	if( m_bActive && m_bUpdateSoundAndVisuals )
	{
		switch( m_TintDir )
		{
			case DAMAGEFX_TINT_RAMPUP :
			{
				if( m_fElapsedTime >= m_fEndTime )
				{
					// We're at maximum tint...
					m_vCurTintColor = m_vMaxTintColor;

					//change our state
					m_TintDir = DAMAGEFX_TINT_ON;

					//and update our timer
					m_fElapsedTime -= m_fEndTime;
					m_fEndTime		= m_fTintOnTm;
				}
				else
				{
					// Gradually increase tint towards the max color

					m_vCurTintColor = m_vMaxTintColor * (m_fElapsedTime / m_fEndTime);
				}
			}
			break;

			case DAMAGEFX_TINT_RAMPDOWN :
			{
				if( m_fElapsedTime >= m_fEndTime )
				{
					// We're at no tint...

					m_vCurTintColor.Init();
					
					//change our state
					m_TintDir = DAMAGEFX_TINT_OFF;

					//and update our timer
					m_fElapsedTime -= m_fEndTime;
					m_fEndTime		= m_fTintOffTm;
				}
				else
				{
					// Gradually decrease tint towards no color

					m_vCurTintColor = m_vMaxTintColor * (1.0f - m_fElapsedTime / m_fEndTime);
				}
			}
			break;

			case DAMAGEFX_TINT_ON :
			{
				// Stay at max color for the specified time...
				
				m_vCurTintColor = m_vMaxTintColor;
				
				if( m_fElapsedTime >= m_fEndTime )
				{
					//change our state
					m_TintDir = DAMAGEFX_TINT_RAMPDOWN;

					//and update our timer
					m_fElapsedTime -= m_fEndTime;
					m_fEndTime		= m_fTintRampDownTm;
				}
			}
			break;

			case DAMAGEFX_TINT_OFF :
			{
				// Stay at no color for the specified time...

				m_vCurTintColor.Init();

				if( m_fElapsedTime >= m_fEndTime )
				{
					//change our state
					m_TintDir = DAMAGEFX_TINT_RAMPUP;

					//and update our timer
					m_fElapsedTime -= m_fEndTime;
					m_fEndTime		= m_fTintRampUpTm;
				}
			}
			break;

			default : break;
		}

		vRes = m_vCurTintColor;
	}
	else if( m_bFade )
	{
		vRes = m_vCurTintColor * (LTMAX(0.0f, 1.0f - m_fElapsedTime / m_fEndTime));
	}

	return vRes;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	DAMAGEFX::UpdateLightScale
//
//  PURPOSE:	Update the current light scale color
//
// ----------------------------------------------------------------------- //

LTVector DAMAGEFX::UpdateLightScale( )
{
	LTFLOAT		fFrameTime	= g_pGameClientShell->GetFrameTime();
		
	if( m_bActive && m_bUpdateSoundAndVisuals )
	{
		m_fLightScale += m_fLightScaleSpeed * fFrameTime;
		if( m_fLightScale > 1.0f )
			m_fLightScale = 1.0f;

		LTVector vLightScale( 1.0f - m_fLightScale, 1.0f - m_fLightScale, 1.0f - m_fLightScale );
		vLightScale.x = Max( m_vLightScaleColor.x, vLightScale.x );
		vLightScale.y = Max( m_vLightScaleColor.y, vLightScale.y );
		vLightScale.z = Max( m_vLightScaleColor.z, vLightScale.z );

		return vLightScale;
	}
	else if( m_bFade )
	{
		// Fade out the light scale color

		LTFLOAT fLightScale = m_fLightScale * LTMAX(0.0f, 1.0f - (m_fElapsedTime / m_fEndTime));

		LTVector vLightScale( 1.0f - fLightScale, 1.0f - fLightScale, 1.0f - fLightScale );
		vLightScale.x = Max( m_vLightScaleColor.x, vLightScale.x );
		vLightScale.y = Max( m_vLightScaleColor.y, vLightScale.y );
		vLightScale.z = Max( m_vLightScaleColor.z, vLightScale.z );

		return vLightScale;
	}

	return LTVector( 1.0f, 1.0f, 1.0f );
}



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	DAMAGEFX::CreateSprinkles
//
//  PURPOSE:	Create the sprinkles associated with this damage fx
//
// ----------------------------------------------------------------------- //

void DAMAGEFX::CreateSprinkles()
{
	if( !m_pSprinkles && m_nNumSprinkles > 0 )
	{
		SPRINKLESCREATESTRUCT scs;

		scs.m_nTypes = m_nNumSprinkles;

		// Find the requested Sprinkles and add them to the SprinkleFX create struct...

		for( int i = 0; i < m_nNumSprinkles; ++i )
		{
			SPRINKLEFX *pFX = g_pFXButeMgr->GetSprinkleFX( m_aszSprinkleName[i] );
			if( !pFX ) return;

			scs.m_Types[i].m_hFilename      = g_pLTClient->CreateString( pFX->szFileName );
            scs.m_Types[i].m_hSkinName      = g_pLTClient->CreateString( pFX->szSkinName );
			scs.m_Types[i].m_Count			= pFX->dwCount;			
			scs.m_Types[i].m_Speed			= pFX->fSpeed;			
			scs.m_Types[i].m_Size			= pFX->fSize;			
			scs.m_Types[i].m_SpawnRadius	= pFX->fSpawnRadius;	
			scs.m_Types[i].m_ColorMax		= pFX->vColorMax;		
			scs.m_Types[i].m_ColorMin		= pFX->vColorMin;		
			scs.m_Types[i].m_AnglesVel		= pFX->vAnglesVel;		
		}

		m_pSprinkles = (SprinklesFX*) g_pGameClientShell->GetSFXMgr()->CreateSFX( SFX_SPRINKLES_ID, &scs );
		if( m_pSprinkles )
			m_pSprinkles->Update();
		
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	DAMAGEFX::DestroySprinkles
//
//  PURPOSE:	Delete all the sprinkles
//
// ----------------------------------------------------------------------- //

void DAMAGEFX::DestroySprinkles()
{
	if( m_pSprinkles )
	{
		m_pSprinkles->WantRemove();
		m_pSprinkles = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	DAMAGEFX::StartSoundAndVisuals
//
//  PURPOSE:	Start fx for sound and visuals
//
// ----------------------------------------------------------------------- //

void DAMAGEFX::StartSoundAndVisuals()
{
	if( !m_bActive ) 
		return;

	// We can now update the FX..

	m_bUpdateSoundAndVisuals = LTTRUE;

	if( m_szStartSound[0] )
	{
		// Play the start sound

		g_pClientSoundMgr->PlaySoundLocal( m_szStartSound, SOUNDPRIORITY_PLAYER_MEDIUM );
	}

	if( m_szLoopSound[0] && !m_hLoopSound )
	{
		// Play the looping sound..

		uint32 dwFlags = PLAYSOUND_GETHANDLE | PLAYSOUND_LOOP | PLAYSOUND_CLIENT;
		m_hLoopSound = g_pClientSoundMgr->PlaySoundLocal( m_szLoopSound, SOUNDPRIORITY_PLAYER_LOW, dwFlags );
	}

	// Start the tint to ramp up towards max color...

	m_fElapsedTime = 0.0f;
	m_fEndTime = m_fTintRampUpTm;

	m_TintDir = DAMAGEFX_TINT_RAMPUP;
	
	CreateSprinkles();

	// Create the FxED created FX...

	if( m_szFXName[0] )
	{
		LTVector vCamPos;
		g_pLTClient->GetObjectPos( g_pPlayerMgr->GetCamera(), &vCamPos );

		CLIENTFX_CREATESTRUCT	fxInit( m_szFXName, FXFLAG_LOOP | FXFLAG_REALLYCLOSE, LTVector(0,0,0) ); 
		g_pClientFXMgr->CreateClientFX(&m_DamageFXInstance, fxInit, LTTRUE );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	DAMAGEFX::StartSoundAndVisuals
//
//  PURPOSE:	Stop fx for sound and visuals
//
// ----------------------------------------------------------------------- //

void DAMAGEFX::StopSoundAndVisuals( LTBOOL bFade /* = LTTRUE  */ )
{
	m_bFade = bFade;
	m_bUpdateSoundAndVisuals = LTFALSE;

	m_fElapsedTime = 0.0f;
	m_fEndTime = m_fFadeTm;

	m_vtTestFX.SetFloat( 0.0f );

	if( m_hLoopSound )
	{
		g_pLTClient->SoundMgr()->KillSound( m_hLoopSound );
		m_hLoopSound = LTNULL;
	}

	DestroySprinkles();

	if( m_DamageFXInstance.IsValid() )
	{
		g_pClientFXMgr->ShutdownClientFX( &m_DamageFXInstance );
	}	
}


// ------------------------------------------------------------------------//
//
//	Private Helper functions
//
// ------------------------------------------------------------------------//

LTBOOL CDamageFXMgr::GetBool(char *pTag,char *pAttribute)
{
    return (LTBOOL) m_buteMgr.GetInt(pTag,pAttribute, 0);
}

LTFLOAT CDamageFXMgr::GetFloat(char *pTag,char *pAttribute)
{
    return (LTFLOAT)m_buteMgr.GetDouble(pTag, pAttribute, 0.0f);
}

int	CDamageFXMgr::GetInt(char *pTag,char *pAttribute)
{
	return m_buteMgr.GetInt(pTag, pAttribute, 0);
}

LTIntPt CDamageFXMgr::GetPoint(char *pTag,char *pAttribute)
{
    CPoint zero(0,0);
    CPoint tmp = m_buteMgr.GetPoint(pTag, pAttribute, zero);
    LTIntPt pt(tmp.x,tmp.y);
	return pt;
}

uint32 CDamageFXMgr::GetDWord(char *pTag,char *pAttribute)
{
    return (uint32)m_buteMgr.GetInt(pTag, pAttribute, 0);
}

void CDamageFXMgr::GetString(char *pTag,char *pAttribute,char *pBuf, int nBufLen)
{
	m_buteMgr.GetString(pTag, pAttribute, "" ,pBuf, nBufLen);
}

LTVector CDamageFXMgr::GetVector(char *pTag,char *pAttribute)
{
	CAVector vRet(0.0,0.0,0.0);
	return m_buteMgr.GetVector(pTag, pAttribute, vRet);
}
