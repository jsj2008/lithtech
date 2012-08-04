// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerCamera.cpp
//
// PURPOSE : PlayerCamera - Implementation
//
// (c) 1997-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "PlayerCamera.h"
#include "CMoveMgr.h"
#include "resourceextensions.h"
#include "CharacterFX.h"
#include "ClientWeaponMgr.h"
#include "SFXMgr.h"
#include "CameraFX.h"
#include "LTEulerAngles.h"
#include "HeadBobMgr.h"
#include "PlayerBodyMgr.h"
#include "VehicleMgr.h"
#include "LeanMgr.h"
#include "PlayerLureFX.h"
#include "SkillDefs.h"
#include <float.h>
#include "HUDOverlay.h"
#include "HUDCrosshair.h"
#include "objectdetector.h"
#include "BindMgr.h"
#include "commandids.h"
#include "LadderMgr.h"
#include "ClientDB.h"
#include "GameRenderLayers.h"

#include "ILTRenderer.h"
extern ILTRenderer *g_pLTRenderer;

#include "iltinput.h"
static ILTInput *g_pLTInput = NULL;
define_holder(ILTInput, g_pLTInput);


VarTrack		g_vtCameraClipDistance;
VarTrack		g_vtCameraCollisionObjScale;
VarTrack		g_vtCameraCollisionUseObject;

VarTrack		g_vtChaseCamOffset;
VarTrack		g_vtChaseCamPointAtOffset;
VarTrack		g_vtChaseCamDistBack;
VarTrack		g_vtChaseCamPitchAdjust;
VarTrack		g_vtChaseCamLODBias;
VarTrack		g_vtCameraFirstPersonLODBias;

VarTrack		g_vtTrackCamPointAtOffset;

VarTrack		g_vtCameraMovementMult;
VarTrack		g_vtFOVYNormal;
VarTrack		g_vtFOVYWide;
VarTrack		g_vtFOVAspectRatioScale;
VarTrack		g_vtCameraSwayXFreq;
VarTrack		g_vtCameraSwayYFreq;
VarTrack		g_vtCameraSwayXSpeed;
VarTrack		g_vtCameraSwayYSpeed;
VarTrack		g_vtCameraSwayDuckMult;
VarTrack		g_vtAttachedCamInterpolationRate;
VarTrack		g_vtCameraAttachedOffsetX;
VarTrack		g_vtCameraAttachedOffsetY;
VarTrack		g_vtCameraAttachedOffsetZ;
VarTrack		g_vtCameraLeashLen;
VarTrack		g_vtCameraAimTrackingYMax;
VarTrack		g_vtCameraAimTrackingYMaxZoomed;
VarTrack		g_vtCameraAimTrackingCollisionMaxSpeed;
VarTrack		g_vtCameraAimTrackingCollisionThreshold;

VarTrack		g_vtCameraFocusRangeSqr;
VarTrack		g_vtCameraFocusScaleMin;
VarTrack		g_vtCameraFocusScaleMax;

VarTrack		g_vtCameraClampIdleLookUp;
VarTrack		g_vtCameraClampIdleLookDown;
VarTrack		g_vtCameraClampMovingLookDown;
VarTrack		g_vtCameraClampMovingLookUp;
VarTrack		g_vtCameraClampCrouchIdleLookUp;
VarTrack		g_vtCameraClampCrouchIdleLookDown;
VarTrack		g_vtCameraClampCrouchMovingLookUp;
VarTrack		g_vtCameraClampCrouchMovingLookDown;
VarTrack		g_vtCameraClampChaseLookUp;
VarTrack		g_vtCameraClampChaseLookDown;

VarTrack		g_vtCameraPixelDouble;

VarTrack		g_vtCameraSmoothingEnabled;
VarTrack		g_vtCameraSmoothingInterpolateTime;
VarTrack		g_vtCameraSmoothingAllowedDistance;
VarTrack		g_vtCameraSmoothingLeashLength;
VarTrack		g_vtCameraHeightInterpSpeedUp;
VarTrack		g_vtCameraHeightInterpSpeedDown;


extern ObjectDetector g_iFocusObjectDetector;

#define DEFAULT_COLLISION_MODEL_SCALE	12.0f

// Camera's X offset from the player when it is in the MLOOK state
#define DEFAULT_CAMERA_DIST_MLOOK_X	0
// Camera's Y offset from the player when it is in the MLOOK state
#define DEFAULT_CAMERA_DIST_MLOOK_Y 10
// Camera's Z offset from the player when it is in the MLOOK state
#define DEFAULT_CAMERA_DIST_MLOOK_Z 29

#define FOVY_MIN				1.5f

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClampLocalAnglesToPlayerLure( )
//
//	PURPOSE:	Calculates a rotation offset applied to a rotation in local space.
//
// ----------------------------------------------------------------------- //

static void ClampLocalAnglesToPlayerLure( PlayerLureFX const& playerLure, float& fPitch, float& fYaw )
{
	// Check if the camera has restricted freedom.
	if( playerLure.GetCameraFreedom( ) == kPlayerLureCameraFreedomUnlimited )
		return;

	// Get the max pitch and yaw allowed for this lure.
	float fPitchMax = 0.0f;
	float fPitchMin = 0.0f;
	float fYawMax = 0.0f;
	float fYawMin = 0.0f;
	switch( playerLure.GetCameraFreedom( ))
	{
		case kPlayerLureCameraFreedomLimited:
			// Convert the left/right/up/down angles to min/max's in radians.
			playerLure.GetLimitedRanges( fYawMin, fYawMax, fPitchMax, fPitchMin );

			// Convert over to radians.  Have to negate the values for pitch since it 
			// works opposite intuitive values.
			fYawMin = DEG2RAD( fYawMin );
			fYawMax = DEG2RAD( fYawMax );
			fPitchMax = -DEG2RAD( fPitchMax );
			fPitchMin = -DEG2RAD( fPitchMin );
			break;
		case kPlayerLureCameraFreedomNone:
		default:
			break;
	}

	// Clamp the pitch and yaw.
	fPitch = Clamp( fPitch, fPitchMin, fPitchMax );
	fYaw = Clamp( fYaw, fYawMin, fYawMax );
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::CPlayerCamera()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CPlayerCamera::CPlayerCamera()
:	m_hCamera					( NULL ),
	m_vPos						( 0.0f, 0.0f, 0.0f ),
	m_rLocalRotation			( ),
	m_vActivePos				( 0.0f, 0.0f, 0.0f ),
	m_vActiveRotation			( ),
	m_rBackupCameraRot			( ),
	m_rSnapShotRotation			( ),
	m_rInterpolationRotation	( ),
	m_bInterpolateRotation		( false ),
	m_bInterpolatePitch			( true ),
	m_bInterpolateYaw			( true ),
	m_bInterpolateRoll			( true ),
	m_vLastTargetPos			( 0.0f, 0.0f, 0.0f ),
	m_vLastOptPos				( 0.0f, 0.0f, 0.0f ),
	m_eCameraMode				( kCM_None ),
	m_eSaveCameraMode			( kCM_None ),
	m_vFirstPersonOffset		( 0.0f, 0.0f, 0.0f ),
	m_vChaseOffset				( 0.0f, 0.0f, 0.0f ),
	m_vChasePointAtOffset		( 0.0f, 0.0f, 0.0f ),
	m_vTrackPointAtOffset		( 0.0f, 0.0f, 0.0f ),
	m_hCollisionObject			( NULL ),
	m_eZoomState				( eEffectState_Out ),
	m_bIronSight				( true ),
	m_fSaveLODScale				( 1.0f ),
	m_bAttachedToTarget			( false ),
	m_bLerpAttached				( false ),
	m_bCanRender				( false ),
	m_eLastCameraDesc			( kAD_Invalid ),
	m_bClamp					( false ),
	m_fLastHeightInterp			( 0.0f ),
	m_fSnapShotSmoothPitch		( 0.0f ),
	m_fInterpSmoothPitch		( 0.0f ),
	m_hRenderTarget				( NULL ),
	m_fCameraSmoothingSnapShot	( 0.0f ),
	m_fCameraSmoothingOffset	( 0.0f ),
	m_fLastInterpolatedValue	( 0.0f ),
	m_bCanShowCrosshair			( true ),
	m_bAllowCameraSmoothing		( true ),
	m_bSetAimTrackCollisionLimits( true ),
	m_vLastPositionOfCollision	( LTVector::GetIdentity( ))
{
	m_tfWorld.m_vPos.Init( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::~CPlayerCamera()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CPlayerCamera::~CPlayerCamera()
{
	if( m_hCamera )
	{
		g_pLTClient->RemoveObject( m_hCamera );
		m_hCamera = NULL;
	}

	if( m_hCollisionObject )
	{
		g_pLTClient->RemoveObject( m_hCollisionObject );
		m_hCollisionObject = NULL;
	}
	
	if( m_hRenderTarget )
	{
		g_pLTRenderer->ReleaseRenderTarget( m_hRenderTarget );
		m_hRenderTarget = NULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::Init()
//
//	PURPOSE:	Init the camera
//
// ----------------------------------------------------------------------- //

bool CPlayerCamera::Init( )
{
	ClientDB& ClientDatabase = ClientDB::Instance( );
	HRECORD hSharedRecord = ClientDatabase.GetClientSharedRecord( );

	g_vtCameraClipDistance.Init( g_pLTClient, "CameraClipDist", NULL, 30.0f);
	g_vtCameraCollisionObjScale.Init( g_pLTClient, "CameraCollisionObjScale", NULL, DEFAULT_COLLISION_MODEL_SCALE );
	g_vtCameraCollisionUseObject.Init( g_pLTClient, "CameraCollisionUseObject", NULL, 1.0f );

	g_vtChaseCamOffset.Init( g_pLTClient, "ChaseCamOffset", NULL, 80.0f );
	g_vtChaseCamPointAtOffset.Init( g_pLTClient, "ChaseCamPointAtOffset", NULL, 50.0f );
	g_vtChaseCamDistBack.Init( g_pLTClient, "ChaseCamDistBack", NULL, 180.0f );
	g_vtChaseCamPitchAdjust.Init( g_pLTClient, "ChaseCamPitchAdjust", NULL, 0.0f );
	g_vtChaseCamLODBias.Init( g_pLTClient, "ChaseCamLODBias", NULL, 0.0f );
	g_vtCameraFirstPersonLODBias.Init( g_pLTClient, "CameraFirstPersonLODBias", NULL, -1000000.0f );

	g_vtTrackCamPointAtOffset.Init( g_pLTClient, "TrackCamPointAtOffset", NULL, 50.0f );

	g_vtCameraMovementMult.Init( g_pLTClient, "CameraMovementMult", NULL, 0.3f );
	g_vtFOVYNormal.Init( g_pLTClient, "FovY", NULL, 70.0f );
	g_vtFOVYWide.Init( g_pLTClient, "FovYWidescreen", NULL, 70.0f );
	g_vtFOVAspectRatioScale.Init( g_pLTClient, "FovAspectRatioScale", NULL, g_pInterfaceMgr->GetDefaultFOVAspectRatioScale() );
	g_vtCameraSwayXFreq.Init( g_pLTClient, "CameraSwayXFreq", NULL, 13.0f );
	g_vtCameraSwayYFreq.Init( g_pLTClient, "CameraSwayYFreq", NULL, 5.0f );
	g_vtCameraSwayXSpeed.Init( g_pLTClient, "CameraSwayXSpeed", NULL, 12.0f );
	g_vtCameraSwayYSpeed.Init( g_pLTClient, "CameraSwayYSpeed", NULL, 1.5f );
	g_vtCameraSwayDuckMult.Init( g_pLTClient, "CameraSwayCrouchMultiplier", NULL, 0.5f );
	g_vtAttachedCamInterpolationRate.Init( g_pLTClient, "AttachedCamIntrepolationRate", NULL, 0.1f );
	g_vtCameraAttachedOffsetX.Init( g_pLTClient, "CameraAttachedOffsetX", NULL, 0.0f );
	g_vtCameraAttachedOffsetY.Init( g_pLTClient, "CameraAttachedOffsetY", NULL, 0.0f );
	g_vtCameraAttachedOffsetZ.Init( g_pLTClient, "CameraAttachedOffsetZ", NULL, 0.0f );
	g_vtCameraLeashLen.Init( g_pLTClient, "CameraLeashLen", NULL, 30.0f );
	g_vtCameraAimTrackingYMax.Init( g_pLTClient, "CameraAimTrackingYMax", NULL, 70.0f );
	g_vtCameraAimTrackingYMaxZoomed.Init( g_pLTClient, "CameraAimTrackingYMaxZoomed", NULL, 65.0f );
	g_vtCameraAimTrackingCollisionMaxSpeed.Init( g_pLTClient, "CameraAimTrackingCollisionMaxSpeed", NULL, 75.0f );
	g_vtCameraAimTrackingCollisionThreshold.Init( g_pLTClient, "CameraAimTrackingCollisionThreshold", NULL, 60.0f );

	g_vtCameraFocusRangeSqr.Init( g_pLTClient, "CameraFocusRangeSqr", NULL, 90000.0f );
	g_vtCameraFocusScaleMin.Init( g_pLTClient, "CameraFocusScaleMin", NULL, 3.0f );
	g_vtCameraFocusScaleMax.Init( g_pLTClient, "CameraFocusScaleMax", NULL, 20.0f );

	g_vtCameraClampIdleLookUp.Init( g_pLTClient, "CameraClampIdleLookUp", NULL, 85.0f );
	g_vtCameraClampIdleLookDown.Init( g_pLTClient, "CameraClampIdleLookDown", NULL, 85.0f );
	g_vtCameraClampMovingLookUp.Init( g_pLTClient, "CameraClampMovingLookUp", NULL, 85.0f );
	g_vtCameraClampMovingLookDown.Init( g_pLTClient, "CameraClampMovingLookDown", NULL, 70.0f );
	g_vtCameraClampCrouchIdleLookUp.Init( g_pLTClient, "CameraClampCrouchIdleLookUp", NULL, 85.0f );
	g_vtCameraClampCrouchIdleLookDown.Init( g_pLTClient, "CameraClampCrouchIdleLookDown", NULL, 65.0f );
	g_vtCameraClampCrouchMovingLookUp.Init( g_pLTClient, "CameraClampCrouchMovingLookUp", NULL, 85.0f );
	g_vtCameraClampCrouchMovingLookDown.Init( g_pLTClient, "CameraClampCrouchMovingLookDown", NULL, 45.0f );
	g_vtCameraClampChaseLookUp.Init( g_pLTClient, "CameraClampChaseLookUp", NULL, 45.0f );
	g_vtCameraClampChaseLookDown.Init( g_pLTClient, "CameraClampChaseLookDown", NULL, 45.0f );
	
	g_vtCameraPixelDouble.Init( g_pLTClient, "CameraPixelDouble", NULL, 0.0f );

	g_vtCameraSmoothingEnabled.Init( g_pLTClient, "CameraSmoothingEnabled", NULL, ClientDatabase.GetBool( hSharedRecord, CDB_bCameraSmoothingEnabled ));
	g_vtCameraSmoothingInterpolateTime.Init( g_pLTClient, "CameraSmoothingInterpolateTime", NULL, ClientDatabase.GetInt32( hSharedRecord, CDB_dwCameraSmoothingInterpolateTime ) * 0.001f );
	g_vtCameraSmoothingAllowedDistance.Init( g_pLTClient, "CameraSmoothingAllowedDistance", NULL, ClientDatabase.GetFloat( hSharedRecord, CDB_fCameraSmoothingAllowedDistance ));
	g_vtCameraSmoothingLeashLength.Init( g_pLTClient, "CameraSmoothingLeashLength", NULL, ClientDatabase.GetFloat( hSharedRecord, CDB_fCameraSmoothingLeashLength ));
	g_vtCameraHeightInterpSpeedUp.Init( g_pLTClient, "CameraHeightInterpSpeedUp", NULL, ClientDatabase.GetFloat( hSharedRecord, CDB_CameraHeightInterpSpeedUp ));
	g_vtCameraHeightInterpSpeedDown.Init( g_pLTClient, "CameraHeightInterpSpeedDown", NULL, ClientDatabase.GetFloat( hSharedRecord, CDB_CameraHeightInterpSpeedDown ));

	

	if( !CreateCameraObject() )
		return false;

	if( !CreateCameraCollisionObject() )
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::InitLocalPlayer()
//
//	PURPOSE:	The local player object has been initialized...
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::InitLocalPlayer( )
{
	EngineTimer playerTimer( g_pMoveMgr->GetServerObject( ));
	LTASSERT( playerTimer.IsValid( ), "Invalid player timer. " );
	m_ZoomTimer.SetEngineTimer( playerTimer );
	m_InterpolationTimer.SetEngineTimer( playerTimer );
	m_SmoothPitchTimer.SetEngineTimer( playerTimer );
	m_CameraSmoothingTimer.SetEngineTimer( playerTimer );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::OnEnterWorld()
//
//	PURPOSE:	Handle first comming into a world...
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::OnEnterWorld( )
{
	ResetCamera();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::Render()
//
//	PURPOSE:	Render the camera...
//
//	NOTE:		This should only be called from CGameClientShell::RenderCamera().
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::Render( )
{
	switch( GetCameraMode() )
	{
		case kCM_Chase:
		{
			RenderChase( );
		}
		break;

		case kCM_FirstPerson:
		{
			RenderFirstPerson( );
		}
		break;

		case kCM_Follow:
		case kCM_Cinematic:
		case kCM_Track:
		case kCM_Fixed:
		case kCM_Fly:
		{
			// No special handling required...
			RenderCamera( );	
		}
		break;
		
		default:
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::RenderCamera()
//
//	PURPOSE:	Actually handle the RenderCamera call..
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::RenderCamera( )
{
	bool bRenderedToTarget = false;
	
	// If we're currently pointed to the screen, and we're going to be rendering to another target,
	// render to the other target instead.
	if( g_pLTRenderer->GetCurrentRenderTarget() == NULL )
	{
		UpdateRenderTarget();
		if( m_hRenderTarget != NULL )
		{
			g_pLTRenderer->SetRenderTarget( m_hRenderTarget );
			bRenderedToTarget = true;	
		}
	}
	
	// Clear the screen and render the camera...
	g_pLTClient->GetRenderer()->ClearRenderTarget( CLEARRTARGET_ALL, 0 );
	g_pLTClient->GetRenderer()->RenderCamera( m_hCamera );
	
	// If we rendered to our internal render target, copy it to the screen
	// and restore the screen render target
	if( bRenderedToTarget )
	{
		LTRect2f rFullRect( 0.0f, 0.0f, 1.0f, 1.0f );
		g_pLTRenderer->StretchRect( m_hRenderTarget, rFullRect, NULL, rFullRect );
		g_pLTRenderer->SetRenderTargetScreen( );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::RenderChase()
//
//	PURPOSE:	Handle special actions required for pre/post rendering while in chase mode...
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::RenderChase( )
{
	g_pModelLT->SetLODDistanceBias( m_hTarget, g_vtChaseCamLODBias.GetFloat( ));

	RenderCamera( );

	// Set LOD back to default...
	g_pModelLT->SetLODDistanceBias( m_hTarget, 0.0f );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::RenderFirstPerson()
//
//	PURPOSE:	Handle special actions required for pre/post rendering while in first person mode...
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::RenderFirstPerson( )
{
	bool bHidPlayerBody = false;
	bool bUsedLocalSettings = false;

	// Do special case stuff if we're in playerbody mode.
	if( CPlayerBodyMgr::Instance().IsEnabled( ))
	{
		// Handle special zoom state if this isn't an ironsights zoom.
		if( m_eZoomState == eEffectState_In && !m_bIronSight )
		{
			// Hide the body if zoomed, so we can't see the hands/gun.
			if( !CPlayerBodyMgr::Instance().IsPlayerBodyHidden( ))
			{
				CPlayerBodyMgr::Instance().HidePlayerBody( true, false );

				//hide the weapon, but not it's shadow
				g_pClientWeaponMgr->HideWeapons(false);

				bHidPlayerBody = true;
			}
		}
		// Not zoomed.
		else
		{
			// Don't switch render layersif climbing ladders, this way our hands don't show through the ladder...
			if( !LadderMgr::Instance().IsClimbing( ) && !CPlayerBodyMgr::Instance().InCloseEncounter() )
			{
				// Set the player body to use the player render layer to hide clipping issues...
				g_pLTRenderer->SetObjectDepthBiasTableIndex( CPlayerBodyMgr::Instance( ).GetObject( ), eRenderLayer_Player );

				// Set the client weapon to use the local materials...
				g_pClientWeaponMgr->SetWeaponDepthBiasTableIndex( eRenderLayer_Player );
				g_pClientWeaponMgr->SetWeaponLODDistanceBias( g_vtCameraFirstPersonLODBias.GetFloat( ));
			}

			// Set the LOD offset so the highest LOD is always used when rendering the first person camera...
			g_pModelLT->SetLODDistanceBias( m_hTarget, g_vtCameraFirstPersonLODBias.GetFloat( ));

			bUsedLocalSettings = true;
		}
	}

	RenderCamera( );

	if( bUsedLocalSettings )
	{
		// Set the PlayerBody to use the normal render layer...
		g_pLTRenderer->SetObjectDepthBiasTableIndex( CPlayerBodyMgr::Instance( ).GetObject( ), eRenderLayer_None );

		// Set the client weapon to use the normal materials...
		g_pClientWeaponMgr->SetWeaponDepthBiasTableIndex( eRenderLayer_None );
		g_pClientWeaponMgr->SetWeaponLODDistanceBias( 0.0f );

		// Set LOD back to default...
		g_pModelLT->SetLODDistanceBias( m_hTarget, 0.0f );
	}

	if( bHidPlayerBody )
	{
		CPlayerBodyMgr::Instance().HidePlayerBody( false );
		g_pClientWeaponMgr->ShowWeapons( );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::CameraUpdate()
//
//	PURPOSE:	Update the position & orientation of the camera based
//				on the target
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::Update( )
{
	if( !m_hTarget )
		return;

	UpdateWorldTransform( );

	// Poll the CameraFX to see if there are any live ones...
	UpdateCinematicCameras( );

	// Update based on specific camera mode...
	switch( GetCameraMode() )
	{
		case kCM_Chase:
		{
			UpdateChase( );
		}
		break;
	
		case kCM_Track:
			{
				UpdateTrack( );
			}
			break;

		case kCM_Fixed:
			{
				UpdateFixed( );
			}
			break;

		case kCM_FirstPerson:
			{
				UpdateFirstPerson( );
			}
			break;

		case kCM_Follow:
			{
				UpdateFollow( );
			}
			break;

		case kCM_Fly:
			{
				UpdateFly( );
			}
			break;

		case kCM_Cinematic:
		{
		}
		break;

		case kCM_None:
		default:
		break;
	}


	// Rendering is now allowed...
	m_bCanRender = true;

	// So is camera smoothing...
	m_bAllowCameraSmoothing = true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::UpdateWorldTransform()
//
//	PURPOSE:	Update the world transform.
//
// ----------------------------------------------------------------------- //
void CPlayerCamera::UpdateWorldTransform()
{
	if( g_pMoveMgr->GetVehicleMgr()->IsVehiclePhysics())
	{
		PlayerLureFX* pPlayerLureFX = PlayerLureFX::GetPlayerLureFX( g_pMoveMgr->GetVehicleMgr()->GetPlayerLureId( ));
		if( !pPlayerLureFX )
			return;

		// Apply the new world transform.
		LTRotation rNewWorld;
		g_pLTClient->GetObjectRotation( pPlayerLureFX->GetServerObj( ), &rNewWorld );
		if( pPlayerLureFX->GetRetainOffsets( ))
			rNewWorld = pPlayerLureFX->GetOffsetTransform( ).m_rRot * rNewWorld;

		// See if it actually changed.
		if( rNewWorld != m_tfWorld.m_rRot )
		{
			// If we're tracking both pitch and yaw, then we can't possibly hit the boundary of
			// the the playerlure camera freedom.  We can just accept the new rotation.
			if( pPlayerLureFX->GetTrackPitch() && pPlayerLureFX->GetTrackYaw())
			{
				m_tfWorld.m_rRot = rNewWorld;
			}
			// We're not completely tracking the rotations of the playerlure, so
			// we need to test if our camera has hit a boundary due to playerlure rotations.
			else
			{
				LTRotation rOldLocalRot = GetLocalCameraRotation();
				LTRotation rOldWorldCameraRot = GetCameraRotation( );
				LTRotation rOldWorldTrans = m_tfWorld.m_rRot;

				// Set the new world transform rotation.
				m_tfWorld.m_rRot = rNewWorld;

				// Find the change to the world transform.  Zero out
				// the delta components of the euler angles we're not tracking.
				// We always track roll.
				LTRotation rDeltaWorld = ~rOldWorldTrans * m_tfWorld.m_rRot;
				EulerAngles euDeltaWorld = Eul_FromQuat( rDeltaWorld, EulOrdYXZr );
				// Not tracking pitch.
				if( !pPlayerLureFX->GetTrackPitch( ))
					euDeltaWorld.y = 0.0f;
				// Not tracking yaw.
				if( !pPlayerLureFX->GetTrackYaw( ))
					euDeltaWorld.x = 0.0f;
				LTRotation rNewDeltaWorld( euDeltaWorld.y, euDeltaWorld.x, euDeltaWorld.z );
				LTRotation rNewWorldCameraRot = rNewDeltaWorld * rOldWorldCameraRot;
				SetCameraRotation( rNewWorldCameraRot );

				// Check if the camera has restricted freedom.
				if( pPlayerLureFX->GetCameraFreedom( ) != kPlayerLureCameraFreedomUnlimited )
				{
					// Compare the old angles with the new angles and see if our camera has passed
					// beyond a limit boundary.
					EulerAngles euOldAngles = Eul_FromQuat( rOldLocalRot, EulOrdYXZr );
					LTVector vOldPYR( euOldAngles.y, euOldAngles.x, euOldAngles.z );
					EulerAngles euNewAngles = Eul_FromQuat( GetLocalCameraRotation( ), EulOrdYXZr );
					LTVector vNewPYR( euNewAngles.y, euNewAngles.x, euNewAngles.z );

					// Take the delta and make sure it's between +/- PI.  This prevents
					// a delta which wraps around from looking like it actually went
					// the opposite direction.
					LTVector vDiffPYR = vNewPYR - vOldPYR;
					vDiffPYR.x = LimitToPosNegPi( vDiffPYR.x );
					vDiffPYR.y = LimitToPosNegPi( vDiffPYR.y );

					// Recalculate the new PYR using the differences.  This makes sure
					// the new PYR will cross the boundary rather than wrap to the other
					// side of the circle.
					vNewPYR.x = vOldPYR.x + vDiffPYR.x;
					vNewPYR.y = vOldPYR.y + vDiffPYR.y;

					LTVector vNewPYRBeforeClamp = vNewPYR;
					ClampLocalAnglesToPlayerLure( *pPlayerLureFX, vNewPYR.x, vNewPYR.y );
					if( vNewPYRBeforeClamp.DistSqr( vNewPYR ) != 0.0f )
						SetLocalCameraRotation( LTRotation( vNewPYR.x, vNewPYR.y, vNewPYR.z ));
				}
			}
		}
	}
	else if( m_bClamp )
	{
/*		if( m_hTarget )
		{
			g_pLTBase->GetObjectRotation( m_hTarget, &m_tfWorld.m_rRot );
		}
*/
		EulerAngles euNewAngles = Eul_FromQuat( GetLocalCameraRotation( ), EulOrdYXZr );
		LTVector vNewPYR( euNewAngles.y, euNewAngles.x, euNewAngles.z );
		LTVector vNewPYRBeforeClamp = vNewPYR;
		ClampLocalAngles( vNewPYR.x, vNewPYR.y );
		if( vNewPYRBeforeClamp.DistSqr( vNewPYR ) != 0.0f )
		{
			SetLocalCameraRotation( LTRotation( vNewPYR.x, vNewPYR.y, vNewPYR.z ));
		}

	}
	else
	{
		m_tfWorld.m_rRot.Init( );
	}

	// The position is currently not using a transform.
	m_tfWorld.m_vPos = GetCameraPos( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::UpdateChase()
//
//	PURPOSE:	Update position and rotation while in chase mode...
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::UpdateChase( )
{
	LTVector vTargetPos;
	g_pLTClient->GetObjectPos( m_hTarget, &vTargetPos );

	LTRotation rTargetRot;
	g_pLTClient->GetObjectRotation( m_hTarget, &rTargetRot );

	// Update offsets based on console variables...
	m_vChaseOffset.Init( 0.0f, g_vtChaseCamOffset.GetFloat(), 0.0f );
	m_vChasePointAtOffset.Init( 0.0f, g_vtChaseCamPointAtOffset.GetFloat(), 0.0f );

	LTVector vCamPos = FindChaseCameraPosition();

	// Move the camera to the optimal position...
	MoveCameraToPosition( vCamPos );

	// Point the camera at the target...
	LTVector vPointAt = vTargetPos + m_vChasePointAtOffset;
	PointAtPosition( vPointAt, rTargetRot, vCamPos );

	// Set the camera to use the rotation calculated by the player camera,
	// however we still need to calculate the correct rotation to be sent
	// to the player...

	float fAdjust = DEG2RAD(g_vtChaseCamPitchAdjust.GetFloat());
	LTRotation rAdjust( fAdjust, 0.0f, 0.0f );

	SetCameraRotation( rAdjust * GetCameraRotation( ));

	g_pLTClient->SetObjectTransform(GetCamera( ), LTRigidTransform(GetCameraPos(), GetCameraRotation()));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::UpdateTrack()
//
//	PURPOSE:	Update rotation while in track mode...
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::UpdateTrack( )
{
	LTVector vTargetPos;
	g_pLTClient->GetObjectPos( m_hTarget, &vTargetPos );

	// Update offsets based on console variables...
	m_vTrackPointAtOffset.Init( 0.0f, g_vtTrackCamPointAtOffset.GetFloat(), 0.0f );

	// Point the camera at the target...
	LTVector vPointAt = (vTargetPos + m_vTrackPointAtOffset) - m_vPos;
	vPointAt.Normalize();

	static LTVector vUp(0.0f, 1.0f, 0.0f);

	//determine the right vector
	LTVector vRight = vPointAt.Cross(vUp).GetUnit();
	LTVector vTrueUp = vRight.Cross( vPointAt );
	LTRotation rNew(vPointAt,vTrueUp);

	SetCameraRotation(rNew);

	g_pLTClient->SetObjectTransform( GetCamera( ), LTRigidTransform(GetCameraPos(), GetCameraRotation()));
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::UpdateFixed()
//
//	PURPOSE:	Update rotation while in fixed mode...
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::UpdateFixed( )
{
	LTRigidTransform rtfTarget;
	g_pLTClient->GetObjectTransform( m_hTarget, &rtfTarget );
	SetCameraPos( rtfTarget.m_vPos );
	SetCameraRotation( rtfTarget.m_rRot );
	g_pLTClient->SetObjectTransform( GetCamera(), rtfTarget );
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::CalculateLocalCameraRotation()
//
//	PURPOSE:	Calculates the local camera rotation.
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::CalculateLocalCameraRotation( )
{
	// Get our starting local rotation.
	LTRotation rLocalCameraRot = GetLocalCameraRotation( );
	rLocalCameraRot.Normalize( );

	// Set the camera's euler angles.  Clear out the roll since that's not controllable by the
	// player.  Roll can be applied by modifying factors like leanmgr, headbob, etc.  A worldspace
	// transform is applied when the rotation is set on the camera object.
	EulerAngles euLocalCamera = Eul_FromQuat( rLocalCameraRot, EulOrdYXZr );

	// Set our new local rotation.
	LTRotation rNewLocalRot( euLocalCamera.y, euLocalCamera.x, 0.0f );
	rNewLocalRot.Normalize( );
	SetLocalCameraRotation( rNewLocalRot );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::UpdateFirstPerson()
//
//	PURPOSE:	Update position and rotation while in first person mode...
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::UpdateFirstPerson( )
{
	// Get the initial rotation of the camera
	CalculateLocalCameraRotation( );

	// Update the rotation based on any focus target
	UpdateCameraFocusDirection( );

	// Update zoom if applicable...
	if( IsZoomed())
	{
		UpdateZoom();
	}

	// Update the sway when zoomed (with a scope)...
	if( IsZoomed() && !m_bIronSight )
	{
		UpdateCameraSway();
	}

	//this is the transform that should be used for the camera, and filled out for the appropriate
	//mode below
	LTRigidTransform tCameraTransform;

	if( !m_bAttachedToTarget )
	{
		LTVector vLocalOffset = GetFirstPersonLocalSpaceOffset( );

		// Normal updating of the head position...
		LTVector vAdjustment = g_pPlayerMgr->GetHeadBobMgr()->GetCameraOffsets();
		vLocalOffset += vAdjustment;

		float fDuckAmount = g_pPlayerMgr->GetCamDuckAmount();
		vLocalOffset.y += fDuckAmount;
		vLocalOffset += g_pPlayerMgr->GetCameraOffsetMgr()->GetPosDelta();

		// Convert to world space and apply.
		LTRotation rTargetRot;
		g_pLTClient->GetObjectRotation( m_hTarget, &rTargetRot );
		LTVector vTargetPos;
		g_pLTClient->GetObjectPos( m_hTarget, &vTargetPos );
		LTVector vWorldOffset = rTargetRot.RotateVector( vLocalOffset );
		SetCameraPos( vTargetPos + vWorldOffset );

		// Handle adjustments to rotation.
		LTRotation rNewRot = GetCameraRotation( );
		LTVector vPitchYawRollDelta = g_pPlayerMgr->GetCameraOffsetMgr()->GetPitchYawRollDelta();
		vPitchYawRollDelta += g_pPlayerMgr->GetHeadBobMgr()->GetCameraRotations();

		if( vPitchYawRollDelta.MagSqr( ) > 0.0f )
		{
			// Apply the offset to local space and set it in our result.  Don't store it in
			// the permanent value with SetCameraRotation, since this is just a frame modification.
			LTRotation rNewLocalCameraRotation;
			CalcLocalRotationOffset( GetLocalCameraRotation( ), vPitchYawRollDelta, rNewLocalCameraRotation );
			rNewRot = GetWorldTransform( ).m_rRot * rNewLocalCameraRotation;
		}

		// Set the final result.
		rNewRot.Normalize( );
		tCameraTransform.Init(GetCameraPos(), rNewRot);
	}
	else
	{
		LTVector	vPos;
		LTRotation	rRot;

		EnumAnimDesc eCameraDesc = CPlayerBodyMgr::Instance( ).GetCameraDescriptor( );

		// If no longer using the rotation of the camera socket leave the camera at the last rotation...
		if( ((m_eLastCameraDesc == kAD_CAM_Rotation) || (m_eLastCameraDesc == kAD_CAM_RotationAim)) &&
			((eCameraDesc != kAD_CAM_Rotation) && (eCameraDesc != kAD_CAM_RotationAim)) )
		{
			SetCameraRotation( m_rTargetAttachRot );
		}
		else if( ((m_eLastCameraDesc != kAD_CAM_Rotation) && (m_eLastCameraDesc != kAD_CAM_RotationAim)) &&
			( (eCameraDesc == kAD_CAM_Rotation) || (eCameraDesc == kAD_CAM_RotationAim)) )
		{
			// Take snap-shot of current rotation and interpolate into the camera sockets rotation...
			m_rSnapShotRotation = GetCameraRotation( );
			m_InterpolationTimer.Start( g_vtAttachedCamInterpolationRate.GetFloat( ));
		}

		GetTargetSocketPosRot( vPos, rRot, eCameraDesc );
		SetCameraPos( vPos );

		UpdateSmoothPitchClamping( rRot );

		if( m_bInterpolateRotation )
		{
			EulerAngles euCamAngles = Eul_FromQuat( rRot, EulOrdYXZr );
			EulerAngles euInterpAngles = Eul_FromQuat( m_rInterpolationRotation, EulOrdYXZr );

			float fPitch	= (m_bInterpolatePitch ? euInterpAngles.y : euCamAngles.y);
			float fYaw		= (m_bInterpolateYaw ? euInterpAngles.x : euCamAngles.x);
			float fRoll		= (m_bInterpolateRoll ? euInterpAngles.z : euCamAngles.z);

            rRot = LTRotation( fPitch, fYaw, fRoll );
		}
		
		// Normal updating of the head position...
		LTVector vLocalOffset = g_pPlayerMgr->GetCameraOffsetMgr()->GetPosDelta();
		LTVector vWorldOffset = rRot.RotateVector( vLocalOffset );
		SetCameraPos( GetCameraPos( ) + vWorldOffset );

		// If the CameraDescriptor is none, the rotation is not gotten from the animation...
		if( CPlayerBodyMgr::Instance( ).IsEnabled() && eCameraDesc == kAD_None &&
			g_pPlayerMgr->IsPlayerAlive() && !m_bInterpolateRotation )
		{
			rRot = GetCameraRotation( );

			LTVector vPitchYawRollDelta = g_pPlayerMgr->GetCameraOffsetMgr()->GetPitchYawRollDelta();
			vPitchYawRollDelta += g_pPlayerMgr->GetHeadBobMgr()->GetCameraRotations();

			vPitchYawRollDelta.z += g_pPlayerMgr->GetLeanMgr()->GetCameraLeanAngle( );

			if( vPitchYawRollDelta.MagSqr( ) > 0.0f )
			{
				// Apply the offset to local space and set it in our result.  Don't store it in
				// the permanent value with SetCameraRotation, since this is just a frame modification.
				LTRotation rNewLocalCameraRotation;
				CalcLocalRotationOffset( GetLocalCameraRotation( ), vPitchYawRollDelta, rNewLocalCameraRotation );
				rRot = GetWorldTransform( ).m_rRot * rNewLocalCameraRotation;
			}
		}
		else if( ((eCameraDesc == kAD_CAM_Rotation) || (eCameraDesc == kAD_CAM_RotationAim)) &&
			     !m_InterpolationTimer.IsTimedOut( ) && !m_bInterpolateRotation )
		{
			double fDuration = m_InterpolationTimer.GetDuration();
			double fTimeLeft = m_InterpolationTimer.GetTimeLeft();
			float fT = (float)((fDuration - fTimeLeft) / fDuration);

			rRot.Slerp( m_rSnapShotRotation, rRot, WaveFn_Sine(fT) );

			SetCameraRotation( rRot );
		}
		else if( m_bInterpolateRotation )
		{
			if( m_InterpolationTimer.IsTimedOut( ))
			{
				if( g_pPlayerMgr->GetPlayerNodeGoto( ) && g_pPlayerMgr->IsAtNodePosition( ))
					g_pPlayerMgr->ReachedNodeRotation( );

				m_bInterpolateRotation = false;
			}
			else
			{
				double fDuration = m_InterpolationTimer.GetDuration();
				double fTimeLeft = m_InterpolationTimer.GetTimeLeft();
				float fT = (float)((fDuration - fTimeLeft) / fDuration);

				rRot.Slerp( m_rSnapShotRotation, rRot, WaveFn_Sine(fT) );
			}

			SetCameraRotation( rRot );
		}
		
		// Cache this updates camera descriptor...
		m_eLastCameraDesc = eCameraDesc;

		//save this rotation into the yaw pitch and roll as well so that when we
		//come out of the attached mode, our view won't pop
		m_rTargetAttachRot = rRot;

		tCameraTransform.Init(GetCameraPos(), rRot);
	}

	//allow the effect system to tie into the view
	LTRigidTransform tEffectTrans;
	LTVector2 v2EffectFov;
	g_pGameClientShell->GetSimulationTimeClientFXMgr().GetCameraModifier(tCameraTransform, tEffectTrans, v2EffectFov);

	if( CanChangeFOV( ))
	{
		if (g_pInterfaceResMgr->IsWidescreen())
		{
			//determine the FOV to use for this screen setting
			LTVector2 v2FOV = g_pInterfaceResMgr->GetScreenFOV(DEG2RAD(g_vtFOVYWide.GetFloat()), g_vtFOVAspectRatioScale.GetFloat());
			v2FOV *= v2EffectFov;
			g_pLTClient->SetCameraFOV( GetCamera( ), v2FOV.x, v2FOV.y );
		}
		else
		{
			//determine the FOV to use for this screen setting
			LTVector2 v2FOV = g_pInterfaceResMgr->GetScreenFOV(DEG2RAD(g_vtFOVYNormal.GetFloat()), g_vtFOVAspectRatioScale.GetFloat());
			v2FOV *= v2EffectFov;
			g_pLTClient->SetCameraFOV( GetCamera( ), v2FOV.x, v2FOV.y );
		}
	}

	//update the controller state (note, this is in here temporarily as a full manager will eventually
	//be needed in order to control whether or not vibration is enabled, and this should end up not
	//being called on the PC side as we do not use vibration and it avoids iterating through the effects)
	float fControllerMotors[NUM_CLIENTFX_CONTROLLER_MOTORS];
	g_pGameClientShell->GetSimulationTimeClientFXMgr().GetControllerModifier(tCameraTransform, fControllerMotors);
	for(uint32 nCurrMotor = 0; nCurrMotor < NUM_CLIENTFX_CONTROLLER_MOTORS; nCurrMotor++)
	{
		g_pLTInput->SetDeviceObjectValue(ILTInput::eDC_Gamepad, ILTInput::eDOC_Rumble + nCurrMotor, fControllerMotors[nCurrMotor]);
	}

	if( g_vtCameraSmoothingEnabled.GetFloat( ) <= 0.0f )
	{
		// Camera height smoothing...
		m_fCameraSmoothingOffset = 0.0f;
		if( m_bHasLastHeightInterp )
		{
			// Check if we've recorded a previous height interp to interpolate from.  If not, then don't interpolate.
			if( tCameraTransform.m_vPos.y != m_fLastHeightInterp )
			{
				float fInterpSpeedUp = g_vtCameraHeightInterpSpeedUp.GetFloat( );
				float fInterpSpeedDown = g_vtCameraHeightInterpSpeedDown.GetFloat( );

				float fFrameTime = g_pLTClient->GetFrameTime();
				float fInterpSpeed = ( ( tCameraTransform.m_vPos.y > m_fLastHeightInterp ) ? fInterpSpeedUp : fInterpSpeedDown );
				float fOffsetScalar = LTMIN( 1.0f, fFrameTime * fInterpSpeed );
				float fDestHeight = tCameraTransform.m_vPos.y;

				tCameraTransform.m_vPos.y = ( m_fLastHeightInterp + ( ( tCameraTransform.m_vPos.y - m_fLastHeightInterp ) * fOffsetScalar ) );
				m_fCameraSmoothingOffset = ( tCameraTransform.m_vPos.y - fDestHeight );
			}
		}
		m_fLastHeightInterp = tCameraTransform.m_vPos.y;
		m_bHasLastHeightInterp = true;
	}

	UpdateCameraSmoothing( tCameraTransform.m_vPos );

	LTVector vHeadBobOffset = tCameraTransform.m_rRot.RotateVector( g_pPlayerMgr->GetHeadBobMgr( )->GetCameraOffsets( ));
	tCameraTransform.m_vPos += vHeadBobOffset;

	// Allow our effect to blend into the camera transform
	tCameraTransform = ( tCameraTransform * tEffectTrans );

	// Ensure the camera doesn't clip into anything...
	CalcNonClipPos( tCameraTransform.m_vPos, tCameraTransform.m_rRot );

	// Set the final result.
	g_pLTClient->SetObjectTransform( GetCamera( ), tCameraTransform);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::UpdateFollow()
//
//	PURPOSE:	Update position and rotation while in follow mode...
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::UpdateFollow( )
{
	//this is the transform that should be used for the camera, and filled out for the appropriate
	//mode below
	LTRigidTransform tCameraTransform;

	EnumAnimDesc eCameraDesc = CPlayerBodyMgr::Instance( ).GetCameraDescriptor( );
	GetTargetSocketPosRot( tCameraTransform.m_vPos, tCameraTransform.m_rRot, eCameraDesc );

	// Cache this updates camera descriptor...
	m_eLastCameraDesc = eCameraDesc;

	//save this rotation into the yaw pitch and roll as well so that when we
	//come out of the attached mode, our view won't pop
	m_rTargetAttachRot = tCameraTransform.m_rRot;

	// Set the final result.
	SetCameraPos( tCameraTransform.m_vPos );
	SetCameraRotation( tCameraTransform.m_rRot );

	// Make sure we clear out the roll, since sockets aren't guaranteed to be without roll.
	CalculateLocalCameraRotation( );
	tCameraTransform.m_vPos = GetCameraPos( );
	tCameraTransform.m_rRot = GetCameraRotation( );

	// Don't clip into anything...
	CalcNonClipPos( tCameraTransform.m_vPos, tCameraTransform.m_rRot );

	// Set our final transform.
	g_pLTClient->SetObjectTransform( GetCamera( ), tCameraTransform);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::UpdateFly()
//
//	PURPOSE:	Update position and rotation while in fly around mode...
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::UpdateFly( )
{
	// Get the initial rotation of the camera
	CalculateLocalCameraRotation( );

	LTVector vTargetPos;
	g_pLTClient->GetObjectPos( m_hTarget, &vTargetPos );
	SetCameraPos( vTargetPos );

	g_pLTClient->SetObjectTransform( GetCamera(), LTRigidTransform(GetCameraPos(), GetCameraRotation()));
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::UpdateCinematicCameras()
//
//	PURPOSE:	Check the list of alternative cameras for the first live one and switch modes if found...
//
// ----------------------------------------------------------------------- //

bool CPlayerCamera::UpdateCinematicCameras()
{
	CSpecialFXList* pCameraList = g_pGameClientShell->GetSFXMgr()->GetCameraList();
	if( pCameraList )
	{
		int nNumCameras = pCameraList->GetSize();

		for( int nCamera = 0; nCamera < nNumCameras; ++nCamera )
		{
			CCameraFX* pCamFX = (CCameraFX*)(*pCameraList)[nCamera];
			if( !pCamFX )
				continue;

			HOBJECT hCamObj = pCamFX->GetServerObj();

			if( hCamObj )
			{
				uint32 dwUsrFlags;
				g_pCommonLT->GetObjectFlags( hCamObj, OFT_User, dwUsrFlags );

				if( dwUsrFlags & USRFLG_CAMERA_LIVE )
				{
					SetCameraMode( kCM_Cinematic );

					g_pPlayerMgr->EndPlayerSlowMo();

					// Only want to render subtitles for cinematics...
					g_pHUDMgr->UpdateRenderLevel();
					

					g_pPlayerMgr->AllowPlayerMovement( pCamFX->AllowPlayerMovement() );

					// Set the position and rotation directly, since this overrides all user input.
					LTRigidTransform tTrans;
					g_pLTClient->GetObjectTransform(hCamObj, &tTrans);
					g_pLTClient->SetObjectTransform(GetCamera(), tTrans);

					// Update the cinematic camera...
					pCamFX->UpdateFOV();

					g_pInterfaceMgr->SetLetterBox( (pCamFX->GetType() == CT_LETTERBOX) );

					//calculate our field of view
					LTVector2 vFOV = g_pInterfaceResMgr->GetScreenFOV(DEG2RAD(pCamFX->GetFovY()), pCamFX->GetFovAspectScale());

					// Set the FOV to that of the cinematic camera...
					g_pLTClient->SetCameraFOV( m_hCamera, vFOV.x, vFOV.y ); 

					// We have a live cinematic...
					return true;
				}
			}
		}
	}

	// See if a cinematic camera just ended...
	if( GetCameraMode() == kCM_Cinematic )
	{
		ResetCamera();

		// Cache this updates camera descriptor...
		EnumAnimDesc eCameraDesc = CPlayerBodyMgr::Instance( ).GetCameraDescriptor( );
		m_eLastCameraDesc = eCameraDesc;
	}

	// No cinematics...
	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::GetFirstPersonLocalSpaceOffset()
//
//	PURPOSE:	Gets the first person offset in local space.
//
// ----------------------------------------------------------------------- //

LTVector CPlayerCamera::GetFirstPersonLocalSpaceOffset( )
{
	// This is the "eye position".
	LTVector vOffset = m_vFirstPersonOffset;

	// Don't inlude the crouching difference, since we're on a playerlure.  The
	// playerlure can't move the player object up and down, since it's always
	// stuck right to the playerlure object.
	if( !g_pMoveMgr->GetVehicleMgr()->IsVehiclePhysics())
	{
		// Modify the position so it's always based on the standing height...
		vOffset.y += g_pMoveMgr->GetCurrentHeightDifference();
	}

	return vOffset;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::PointAtPosition()
//
//	PURPOSE:	Point the camera at a position from a position
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::PointAtPosition(const LTVector &pos, const LTRotation &rRot, const LTVector &pointFrom)
{
	LTVector vAngles( 0.0f, 0.0f, 0.0f);

	SetCameraRotation( rRot );

	float diffX = pos.x - GetCameraPos( ).x;
	float diffY = pos.z - GetCameraPos( ).z;
	vAngles.y = (float)atan2(diffX, diffY);

	LTVector     target, copy;

	copy = pos - pointFrom;

	target = GetCameraRotation( ).RotateVector( copy );

	diffX = target.z;
	diffY = target.y;


	vAngles.x = (float)-atan2(diffY, diffX);

	LTRotation rOldRot;
	rOldRot = GetCameraRotation( );

	SetCameraRotation( LTRotation(VEC_EXPAND(vAngles)));


	// Make sure rotation is valid...
	if (GetCameraRotation( ).Up().y < 0)
	{
		SetCameraRotation( rOldRot );
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::MoveCameraToPosition()
//
//	PURPOSE:	Move the camera to the position...
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::MoveCameraToPosition( LTVector const& vPos )
{
	if( !m_hTarget )
		return;

	LTVector vDir = (vPos - GetCameraPos( ));

	// The multiplier allows for smooth movement, set to 1.0 for instantaneous movement... 
	float fMovementMultiplier = LTMIN( g_vtCameraMovementMult.GetFloat(), 1.0f );
	LTVector vToMove = (vDir * fMovementMultiplier);

	SetCameraPos( GetCameraPos( ) + vToMove );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::FindChaseCameraPosition()
//
//	PURPOSE:	Find the optimal camera position while in chase mode...
//
// ----------------------------------------------------------------------- //

LTVector CPlayerCamera::FindChaseCameraPosition()
{
	if( !m_hTarget || (GetCameraMode() != kCM_Chase) )
		return LTVector::GetIdentity();

	// Get the target information...

	LTVector vTargetPos;
	g_pLTClient->GetObjectPos( m_hTarget, &vTargetPos );

	LTRotation rTargetRot;
	g_pLTClient->GetObjectRotation( m_hTarget, &rTargetRot );

	// No need to recalculate if nothing has changed...
	if( vTargetPos.NearlyEquals( m_vLastTargetPos, 1.0f ) && (rTargetRot == m_rLastTargetRot) )
	{
		return m_vLastOptPos;
	}
	else
	{
		// Cache the changed target object values...
		m_vLastTargetPos = vTargetPos;
		m_rLastTargetRot = rTargetRot;
	}

	// Determine how far behind the target the camera can go...

	LTVector vPos = (rTargetRot.Forward() * -g_vtChaseCamDistBack.GetFloat()) + (vTargetPos  + m_vChaseOffset);
	
	IntersectInfo iInfo;
	IntersectQuery iQuery;

	// Filter out specific objects we don't want to hit...
	HOBJECT hFilterList[] = {g_pLTClient->GetClientObject(), m_hTarget, NULL};

	iQuery.m_From = vTargetPos + m_vChasePointAtOffset;
	iQuery.m_To = vPos;
	iQuery.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	iQuery.m_FilterFn = ObjListFilterFn;
	iQuery.m_pUserData = hFilterList;

	if( g_pLTClient->IntersectSegment( iQuery, &iInfo ))
	{
		// Place the camera in front of anything in the way...
		vPos = iInfo.m_Point + iInfo.m_Plane.m_Normal;
		
		// Place the camera at the head so it doesn't get stuck when somethings in the way...
		g_pLTClient->SetObjectPos( m_hCamera, iQuery.m_From );
	}

	// Calculate the final position so the camera doesn't clip through anything...
	CalcNonClipPos( vPos, rTargetRot );

	// Cache this position...
	m_vLastOptPos = vPos;

	return vPos;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::SetTargetObject()
//
//	PURPOSE:	Set the target object of the camera (what it's attached to or looking at)...
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::SetTargetObject( HOBJECT hTarget )
{
	// No need to reset to same object...
	if( m_hTarget == hTarget )
		return;

	m_hTarget = hTarget;
/*
	// [RP] 01/13/04 - Do we need to do this?
	// Initialize our position to that of the object...
	if( m_hTarget )
	{
		LTVector vPos;
		g_pLTClient->GetObjectPos( m_hTarget, &vPos );

		// Assume for now first person...
		
		LTRotation rRot;
		g_pLTClient->GetObjectRotation( m_hTarget, &rRot );

		LTVector vLocalSpace = GetFirstPersonLocalSpaceOffset( );
		LTVector vWorldSpace = rRot.RotateVector( vLocalSpace );
		vPos += vWorldSpace;
		SetCameraPos( vPos );
	}
*/
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::SetCameraMode()
//
//	PURPOSE:	Handle setting of the camera mode...
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::SetCameraMode( CameraMode eCamMode )
{
	// Don't set mode more than once...
	if( m_eCameraMode == eCamMode )
		return;

	// Handle exiting a mode...
	switch( m_eCameraMode )
	{
		case kCM_Cinematic:
		{
			// Coming out of a cinematic, make sure the player is allowed to move again...
			if( g_pPlayerMgr )
				g_pPlayerMgr->AllowPlayerMovement( true );
		}
		break;

		default:
		break;
	}

	// Set the new mode...
	m_eCameraMode = eCamMode;

	// Handle entering a mode...
	switch( m_eCameraMode )
	{
		case kCM_Follow:
		case kCM_Fly:
		case kCM_Chase:
		{
			// Clear DamageFX since they may be camera specific...
			// (Would be nice to continue seeing DamageFX on the player while in chase mode)
			g_pDamageFXMgr->Clear();

			// Kill the weapons...
			// (Would be nice to have use of weapons while in chase mode)
			g_pClientWeaponMgr->DisableWeapons();

			// No need to have a crosshair if the weapons don't work...
			g_pInterfaceMgr->EnableCrosshair( false );
		}
		break;

		case kCM_Fixed:
		case kCM_Track:
			{
				// Clear DamageFX since they may be camera specific...
				g_pDamageFXMgr->Clear();

				// Kill the weapons...
				g_pClientWeaponMgr->DisableWeapons();
			}
			break;

		case kCM_FirstPerson:
		{
			// Allow weapons while in first person mode...
			g_pClientWeaponMgr->EnableWeapons();

			// We don't want to accidently fire off a round or two comming out of a cinematic...
			CClientWeapon *pWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
			if( pWeapon )
			{
				pWeapon->ClearFiring();
			}

			// Hide the player...
			// (Kind of defeats the purpose of having a target?)
			g_pPlayerMgr->ShowPlayer(false);

			// Re-enable the crosshair...
			g_pInterfaceMgr->EnableCrosshair( true );

			if( CPlayerBodyMgr::Instance( ).IsEnabled( ))
				AttachToTarget( );
		}
		break;

		case kCM_Cinematic:
		{
			// Stop any ringing from recent explosion damage.
			g_pPlayerMgr->ClearEarringEffect();

			// Make sure we clear whatever was on the screen before we switch to this camera mode...
			ExitZoom();

			g_pClientWeaponMgr->DisableWeapons();

			// Turn off the flashlight...
			g_pPlayerMgr->GetFlashLight()->TurnOff();

			// Make sure we're using the highest model lods, this is done by setting the distance
			// scale to zero so that way the resulting distance is alwyas zero and therefore the
			// highest LOD is used.
			g_pLTClient->SetConsoleVariableFloat("ModelLODDistanceScale", 0.0f);
		}
		break;

		case kCM_None:
		default:
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::CreateCameraObject()
//
//	PURPOSE:	Create the actual camera object to use...
//
// ----------------------------------------------------------------------- //

bool CPlayerCamera::CreateCameraObject()
{
	// Remove any previous camera object...
	if( m_hCamera )
	{
		g_pLTClient->RemoveObject( m_hCamera );
		m_hCamera = NULL;
	}

	// Create the camera...
	ObjectCreateStruct ocs;

	ocs.m_ObjectType = OT_CAMERA;
	m_hCamera = g_pLTClient->CreateObject( &ocs );
	
	if( !m_hCamera )
	{
		LTERROR( "CPlayerCamera::CreateCameraObject() - Camera object could not be created!" );
		return false;
	}

	// Initialize the camera state...
	ResetCamera();

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::ResetCamera()
//
//	PURPOSE:	Reset the camera objects state...
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::ResetCamera()
{
	if( !m_hCamera )
		return;

	g_pLTClient->SetCameraRect(m_hCamera, g_pInterfaceMgr->GetViewportRect());

	// No more zoom...
	ExitZoom( );

	m_bAttachedToTarget	= false;
	m_bLerpAttached		= false;

	// Kill any interpolation currently happening...
	m_InterpolationTimer.Stop( );

	m_bHasLastHeightInterp = false;

	// Need an update before the camera is allowed to render...
	m_bCanRender	= false;

	// Force 1st person...
	SetCameraMode( CPlayerCamera::kCM_FirstPerson );

	// Full HUD again...
	g_pHUDMgr->UpdateRenderLevel();

	g_pClientWeaponMgr->EnableWeapons();
	g_pInterfaceMgr->SetLetterBox( false );

	//determine the FOV to use for this screen setting
	LTVector2 vFOV = g_pInterfaceResMgr->GetScreenFOV(DEG2RAD(g_vtFOVYNormal.GetFloat()), g_vtFOVAspectRatioScale.GetFloat());
	if (g_pInterfaceResMgr->IsWidescreen())
	{
		vFOV = g_pInterfaceResMgr->GetScreenFOV(DEG2RAD(g_vtFOVYWide.GetFloat()), g_vtFOVAspectRatioScale.GetFloat());
	}

	//Update the FOV to reflect what is currently setup in the console variables
	g_pLTClient->SetCameraFOV( g_pPlayerMgr->GetPlayerCamera()->GetCamera(), vFOV.x, vFOV.y );

	// Reset our rotations.
	ResetCameraRotations();

	// Make sure we're using the normal model lod...
	g_pLTClient->SetConsoleVariableFloat("ModelLODDistanceScale", 1.0f );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::ResetCameraRotations()
//
//	PURPOSE:	Reset the camera rotations when we have entered a change to our world transform, such as going in 
//				and out of playerlure.
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::ResetCameraRotations()
{
	if( !m_hCamera )
		return;

	// Remember our old world rotation.
	LTRotation rWorldRot = GetCameraRotation();

	// Get our rotation correct.
	if( g_pMoveMgr->GetVehicleMgr()->IsVehiclePhysics())
	{
		// Update our world transform to reflect the relative offset to the playerlure.
		UpdateWorldTransform( );

		// Get our playerlure object.
		PlayerLureFX* pPlayerLureFX = PlayerLureFX::GetPlayerLureFX( g_pMoveMgr->GetVehicleMgr()->GetPlayerLureId( ));
		if( pPlayerLureFX )
		{
			// If not retaining offsets, then we want to point straight down the playerlure's forward vector, which
			// will be the worldtransform.
			if( !pPlayerLureFX->GetRetainOffsets())
			{
				SetLocalCameraRotation( LTRotation( 0.0f, 0.0f, 0.0f ));
			}
			// Reset our world rotation.  This will give us a new local rotation.
			else
			{
				SetCameraRotation( rWorldRot );
			}
		}
	}
	else
	{
		// Update our world transform to reflect absence of playerlure.
		UpdateWorldTransform( );

		// Reset our world rotation.  This will give us a new local rotation.
		SetCameraRotation( rWorldRot );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::HandleZoomInRequest()
//
//	PURPOSE:	The player requested tp zoom in (hit the zoom key)...
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::HandleZoomRequest(bool bEnable)
{
	if( bEnable )
	{
		// Be sure we are not already in the state requested.
		if(    m_eZoomState == eEffectState_In 
			|| m_eZoomState == eEffectState_Entering  )
		{
			return;
		}

		EnterZoom();
	}
	else
	{
		// Be sure we are not already in the state requested.
		if(    m_eZoomState == eEffectState_Exiting
			|| m_eZoomState == eEffectState_Out  )
		{
			return;
		}

		ExitingZoom();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::EnterZoom()
//
//	PURPOSE:	Start zoomming...
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::EnterZoom()
{
	HMOD hScope = g_pPlayerStats->GetScope();
	if( !hScope )
	{
		LTERROR( "Invalid scope database record." );
		return;
	}

	// Get the magnification power of the zoom.
	float fNewZoomMag = g_pWeaponDB->GetFloat( hScope, WDB_MOD_fMagPower );
	if( fNewZoomMag < 0.0f )
	{
		LTERROR( "Invalid zoom mag power." );
		return;
	}

	// Cache whether we should be showing the weapon while zoomed.
	m_bIronSight = g_pWeaponDB->GetBool( hScope, WDB_MOD_bIronSight );

	// Set the record used for the ClientFX sequence...
	HRECORD hScopeFXSequence = g_pWeaponDB->GetRecordLink( hScope, WDB_MOD_rScopeFXSequence );
	m_fxZoomSequence.SetClientFXSequenceRecord( hScopeFXSequence );

	// See if the crosshair should be disabled...
	m_bCanShowCrosshair = g_pWeaponDB->GetBool( hScope, WDB_MOD_bShowCrosshair );

	// Check if we should transition into zooming.
	float fZoomTime = g_pWeaponDB->GetFloat( hScope, WDB_MOD_fZoomTimeIn );
	if( fZoomTime > 0.0f )
	{
		m_ZoomTimer.Start( fZoomTime );

		m_eZoomState = eEffectState_Entering;

		// Begin the ClientFX sequence...
		m_fxZoomSequence.SetState( CClientFXSequence::eState_Begin );
	}
	// No transition, just go full zoom.
	else
	{
		m_ZoomTimer.Stop( );

		m_eZoomState = eEffectState_In;

		SetZoomAspectRatio();

		ShowRangeFinder( );

		// Begin the ClientFX sequence...
		m_fxZoomSequence.SetState( CClientFXSequence::eState_Loop );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::ExitingZoom()
//
//	PURPOSE:	Transition out of zooming.
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::ExitingZoom()
{
	HMOD hScope = g_pPlayerStats->GetScope();
	if( !hScope )
	{
		LTERROR( "Invalid scope database record." );
		return;
	}

	// Don't draw the range display any more.
	g_pCrosshair->SetRangeDisplay( false );

	
    // Check if we should transition into zooming.
	float fZoomTime = g_pWeaponDB->GetFloat( hScope, WDB_MOD_fZoomTimeOut );
	if( fZoomTime > 0.0f )
	{
		m_ZoomTimer.Start( fZoomTime );

		m_fxZoomSequence.SetState( CClientFXSequence::eState_End );

		m_eZoomState = eEffectState_Exiting;
	}
	// No transition, just kill zoom.
	else
	{
		ExitZoom( );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::ExitZoom()
//
//	PURPOSE:	Cancel all zooming.
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::ExitZoom( )
{
	// Already at normal zoom level?
	if( !IsZoomed( ) && !IsZooming() )
		return;

	// Don't draw the range display any more.
	g_pCrosshair->SetRangeDisplay( false );

	// Reset zoom vars...
	m_eZoomState = eEffectState_Out;
	m_ZoomTimer.Stop( );
	m_fxZoomSequence.SetState( CClientFXSequence::eState_End );

	//determine the FOV to use for this screen setting
	LTVector2 vFOV = g_pInterfaceResMgr->GetScreenFOV(DEG2RAD(g_vtFOVYNormal.GetFloat()), g_vtFOVAspectRatioScale.GetFloat());
	if (g_pInterfaceResMgr->IsWidescreen())
	{
		vFOV = g_pInterfaceResMgr->GetScreenFOV(DEG2RAD(g_vtFOVYWide.GetFloat()), g_vtFOVAspectRatioScale.GetFloat());
	}

	//Update the FOV to reflect what is currently setup in the console variables
	g_pLTClient->SetCameraFOV( g_pPlayerMgr->GetPlayerCamera()->GetCamera(), vFOV.x, vFOV.y );

	g_pLTClient->SetConsoleVariableFloat("ModelLODDistanceScale", m_fSaveLODScale);

	// The camera can show the crosshair again...
	m_bCanShowCrosshair = true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::UpdateZoom
//
//	PURPOSE:	Update the zoom state.
//
// --------------------------------------------------------------------------- //

void CPlayerCamera::UpdateZoom()
{
	// Update the state of the zoom FX sequence...
	m_fxZoomSequence.Update( );

	switch( m_eZoomState )
	{
	case eEffectState_Entering:
		{
			// Setup our aspect ratio.
			SetZoomAspectRatio();

			// Check if we reached full zoom in.
			if( m_ZoomTimer.IsTimedOut())
			{
				m_ZoomTimer.Stop( );

				m_eZoomState = eEffectState_In;

				ShowRangeFinder( );
			}
		}
		break;
	case eEffectState_In:
		break;
	case eEffectState_Exiting:
		{
			SetZoomAspectRatio();

			// Check if we reached full zoom out.
			if( m_ZoomTimer.IsTimedOut())
			{
				ExitZoom( );
			}
		}
		break;
	case eEffectState_Out:
		break;
	default:
		LTERROR( "Invalid zoom state." );
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::SetZoomAspectRatio()
//
//	PURPOSE:	Sets the camera aspect ratio to the zoom mag.
//
// ----------------------------------------------------------------------- //
void CPlayerCamera::SetZoomAspectRatio( )
{
	// Get the current FOV.
	float fFovX = 0;
	float fFovY = 0;
	g_pLTClient->GetCameraFOV( m_hCamera, &fFovX, &fFovY );

	// Cache old X fov value.
	float fOldFovY = fFovY;

	// Get the unscoped fov y.
	float fUnscopedFOVY = DEG2RAD(g_vtFOVYNormal.GetFloat());
	if (g_pInterfaceResMgr->IsWidescreen())
	{
		fUnscopedFOVY = DEG2RAD(g_vtFOVYWide.GetFloat());
	}


	// Get the target zoom magnification.
	// Make sure the new zoom mag is valid.
	HMOD hScope = g_pPlayerStats->GetScope();
	float fZoomMag = g_pWeaponDB->GetFloat( hScope, WDB_MOD_fMagPower );
	float fFovYZoomed = fUnscopedFOVY / fZoomMag;

	// Get amount of time we're into the zooming.
	double fParameterized;
	double fTimerDuration = m_ZoomTimer.GetDuration();
	if( fTimerDuration > 0.0f )
		fParameterized = LTCLAMP( m_ZoomTimer.GetElapseTime() / fTimerDuration, 0.0f, 1.0f );
	else
		fParameterized = 1.0f;

	// Calculate the new fov for zooming in.
	if( m_eZoomState == eEffectState_Entering || m_eZoomState == eEffectState_In )
	{
		fFovY = (float)(fParameterized * ( fFovYZoomed - fUnscopedFOVY ) + fUnscopedFOVY);
	}
	// Calculate the new fov for zooming out.
	else
	{
		fFovY = (float)(fParameterized * ( fUnscopedFOVY - fFovYZoomed ) + fFovYZoomed);
	}

	if( fOldFovY != fFovY )
	{
		//determine the FOV to use for this screen setting
		LTVector2 vFOV = g_pInterfaceResMgr->GetScreenFOV(fFovY, g_vtFOVAspectRatioScale.GetFloat());

		//Update the FOV to reflect what is currently setup in the console variables
		g_pLTClient->SetCameraFOV( g_pPlayerMgr->GetPlayerCamera()->GetCamera(), vFOV.x, vFOV.y );

		// Update the lod adjustment for models...

		//determine the inverse relative projected size
		float fRelProjectedSize = LTDiv(LTTan(fFovY), LTTan(fUnscopedFOVY));

		//and now scale the distances used for model LOD determination by that same relative scale
		g_pLTClient->SetConsoleVariableFloat("ModelLODDistanceScale", fRelProjectedSize);
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::GetZoomMag()
//
//	PURPOSE:	Gets the current zoom mag.
//
// ----------------------------------------------------------------------- //

float CPlayerCamera::GetZoomMag() const
{
	// If we're not zooming or zoomed, then we can assume a magnitued of 1.0f.
	if( !IsZoomed( ))
		return 1.0f;

	// Get the target zoom magnification.
	// Make sure the new zoom mag is valid.
	HMOD hScope = g_pPlayerStats->GetScope();
	if( !hScope )
	{
		LTERROR( "Invalid scope." );
		return 1.0f;
	}

	float fZoomMag = g_pWeaponDB->GetFloat( hScope, WDB_MOD_fMagPower );
	if( fZoomMag < 1.0f )
	{
		LTERROR( "Invalid zoom magnitude.  Must be greater than 1.0" );
		return 1.0f;
	}

	return fZoomMag;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::ShowRangeFinder()
//
//	PURPOSE:	Shows the range finder HUD element...
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::ShowRangeFinder( )
{
	HMOD hScope = g_pPlayerStats->GetScope();
	bool bShowRangeFinder = g_pWeaponDB->GetBool( hScope, WDB_MOD_bShowRangeFinder );
	if( bShowRangeFinder )
	{
		LTVector2 v2RangeFinderPosition = g_pWeaponDB->GetVector2( hScope, WDB_MOD_v2RangeFinderPosition );
		LTVector2n v2nRangeFinderPosition(( int32 )v2RangeFinderPosition.x, ( int32 )v2RangeFinderPosition.y );
		uint32 nRangeFinderColor = g_pWeaponDB->GetInt32( hScope, WDB_MOD_nRangeFinderColor );
		g_pCrosshair->SetRangeDisplay( true, &v2nRangeFinderPosition, nRangeFinderColor );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::CreateCameraCollisionObject()
//
//	PURPOSE:	Create an object to use for collision detection when calculating a no-clip pos...
//
// ----------------------------------------------------------------------- //

bool CPlayerCamera::CreateCameraCollisionObject()
{
	// Remove any previous collision object...
	if( m_hCollisionObject )
	{
		g_pLTClient->RemoveObject( m_hCollisionObject );
		m_hCollisionObject = NULL;
	}

	ClientDB& ClientDatabase = ClientDB::Instance();

	ObjectCreateStruct ocs;
	float fScale = g_vtCameraCollisionObjScale.GetFloat();

	ocs.SetFileName( ClientDatabase.GetString( ClientDatabase.GetClientSharedRecord(), CDB_CameraCollisionModel ) );
	ocs.m_ObjectType = OT_MODEL;
	ocs.m_Scale = fScale;
	ocs.m_Pos.Init();

	// [RP] 01/05/04 - Both DONTFOLLOWSTANDING and NOSLIDING were brought over from a CJ integration (cl. 18319)
	//		Originaly m_Flags was set to 0.  NOSLIDING was causing the camera to get stuck on walls and become
	//		detached from the body.
	ocs.m_Flags		= FLAG_DONTFOLLOWSTANDING /*| FLAG_NOSLIDING*/;
	ocs.m_Flags2	= FLAG2_PLAYERCOLLIDE | FLAG2_SPECIALNONSOLID;

	m_hCollisionObject = g_pLTClient->CreateObject( &ocs );

	if( !m_hCollisionObject )
	{
		LTERROR( "CPlayerCamera::CreateCameraCollisionObject() - Failed to create the collision object!  Check the 'client/shared' database for a valid model." );
		return false;
	}

	// Don't show the shadow for this object...
	g_pLTClient->SetObjectShadowLOD( m_hCollisionObject, eEngineLOD_Never );

	// Scale the dims...
	g_pPhysicsLT->SetObjectDims( m_hCollisionObject, &LTVector( fScale, fScale, fScale ), 0 );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::CalcNonClipPos()
//
//	PURPOSE:	Get a position that won't clip into the world or other objects...
//				Doesn't actullay move the camera to that position!
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::CalcNonClipPos( LTVector & vPos, LTRotation & rRot )
{
	LTVector vTemp, vU, vR, vF;
	vU = rRot.Up();
	vR = rRot.Right();
	vF = rRot.Forward();

	if( !m_hCollisionObject || (g_vtCameraCollisionUseObject.GetFloat() < 1.0f) )
	{
		CalcNoClipPos_WithoutObject( vPos, rRot );
		return;
	}

	LTVector vCamPos;
	g_pLTClient->GetObjectPos( m_hCamera, &vCamPos );

	g_pLTClient->SetObjectTransform( m_hCollisionObject, LTRigidTransform(vCamPos, rRot) );

	if( g_vtCameraCollisionObjScale.GetFloat() != DEFAULT_COLLISION_MODEL_SCALE )
	{
		float fScale = g_vtCameraCollisionObjScale.GetFloat();
		LTVector vScale( fScale, fScale, fScale );

		g_pPhysicsLT->SetObjectDims( m_hCollisionObject, &vScale, 0 );
	}

	// Do the collision checking and get the final pos...
	LTVector vWantedPos = vPos;
	g_pPhysicsLT->MoveObject( m_hCollisionObject, vWantedPos, 0 );
	g_pLTClient->GetObjectPos( m_hCollisionObject, &vPos );

	if( m_bSetAimTrackCollisionLimits )
	{
		if( !m_vLastPositionOfCollision.NearlyEquals( vPos, g_vtCameraAimTrackingCollisionThreshold.GetFloat( )) )
		{
			m_bSetAimTrackCollisionLimits = false;
		}
	}
	else if( !vWantedPos.NearlyEquals( vPos, 1.0f ))
	{
		m_bSetAimTrackCollisionLimits = true;
		m_vLastPositionOfCollision = vPos;
	}

	// Limit the distance away from the wanted pos the camera will be...
	float fLeashLen = g_vtCameraLeashLen.GetFloat( );
	if( vWantedPos.DistSqr( vPos ) > (fLeashLen*fLeashLen) )
	{
		LTVector vDir = (vPos - vWantedPos);
		vDir.Normalize( );

		vPos = (vWantedPos + (vDir * fLeashLen));
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::CalcNoClipPos_WithoutObject()
//
//	PURPOSE:	Just use ray casts to test for collision...
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::CalcNoClipPos_WithoutObject( LTVector & vPos, LTRotation & rRot )
{
	LTVector vTemp, vU, vR, vF;
	vU = rRot.Up();
	vR = rRot.Right();
	vF = rRot.Forward();

	
	uint32 dwFlags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;

	// Make sure we aren't clipping into walls...

    float fClipDistance = g_vtCameraClipDistance.GetFloat();


	// Check for walls to the right...

	vTemp = (vR * fClipDistance);
	vTemp += vPos;

	IntersectQuery iQuery;
	IntersectInfo  iInfo;

	iQuery.m_Flags = dwFlags;
	iQuery.m_From = vPos;
	iQuery.m_To   = vTemp;

	if (g_pLTClient->IntersectSegment(iQuery, &iInfo))
	{
		vTemp = iInfo.m_Point - vPos;
        float fDist = (fClipDistance - vTemp.Mag());

		vTemp = (vR * -fDist);
		vPos += vTemp;
	}
	else
	{
	 	// If we didn't adjust for a wall to the right, check walls to the left...

		vTemp = (vR * -fClipDistance);
		vTemp += vPos;

		iQuery.m_Flags = dwFlags;
		iQuery.m_From = vPos;
		iQuery.m_To = vTemp;

		if (g_pLTClient->IntersectSegment(iQuery, &iInfo))
		{
			vTemp = iInfo.m_Point - vPos;
            float fDist = (fClipDistance - vTemp.Mag());

			vTemp = (vR * fDist);
			vPos += vTemp;
		}
	}

	// Check for ceilings...

	vTemp = vU * fClipDistance;
	vTemp += vPos;

	iQuery.m_Flags = dwFlags;
	iQuery.m_From = vPos;
	iQuery.m_To = vTemp;

	if (g_pLTClient->IntersectSegment(iQuery, &iInfo))
	{
		vTemp = iInfo.m_Point - vPos;
        float fDist = (fClipDistance - vTemp.Mag());

		vTemp = vU * -fDist;
		vPos += vTemp;
	}
	else
	{
 		// If we didn't hit any ceilings, check for floors...

		vTemp = vU * -fClipDistance;
		vTemp += vPos;

		iQuery.m_Flags = dwFlags;
		iQuery.m_From = vPos;
		iQuery.m_To = vTemp;

		if (g_pLTClient->IntersectSegment(iQuery, &iInfo))
		{
			vTemp = iInfo.m_Point - vPos;
            float fDist = (fClipDistance - vTemp.Mag());

			vTemp = vU * fDist;
			vPos += vTemp;
		}
	}

	// Check infront of us...

	vTemp = vF * fClipDistance;
	vTemp += vPos;

	iQuery.m_Flags = dwFlags;
	iQuery.m_From = vPos;
	iQuery.m_To = vTemp;

	if( g_pLTClient->IntersectSegment( iQuery, &iInfo ))
	{
		vTemp = iInfo.m_Point - vPos;
		float fDist = (fClipDistance - vTemp.Mag());

		vTemp = vF * -fDist;
		vPos += vTemp;
	}
	else
	{
		vTemp = vF * -fClipDistance;
		vTemp += vPos;

		iQuery.m_Flags = dwFlags;
		iQuery.m_From = vPos;
		iQuery.m_To = vTemp;

		if( g_pLTClient->IntersectSegment( iQuery, &iInfo ))
		{
			vTemp = iInfo.m_Point - vPos;
			float fDist = (fClipDistance - vTemp.Mag());

			vTemp = vU * -fDist;
			vPos += vTemp;
		}
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::UpdateCameraSway
//
//	PURPOSE:	Update the camera's sway...
//
// --------------------------------------------------------------------------- //

void CPlayerCamera::UpdateCameraSway()
{
	// Apply...
	ObjectContextTimer objectContextTimer( g_pMoveMgr->GetServerObject( ));
	float swayAmount = objectContextTimer.GetTimerElapsedS() / 1000.0f;

	float tm  = (float)objectContextTimer.GetTimerAccumulatedS() / 10.0f;

	// Adjust if ducking...
	float fMult = (g_pPlayerMgr->GetPlayerFlags() & BC_CFLG_DUCK) ? g_vtCameraSwayDuckMult.GetFloat() : 1.0f;

	//Adjust for skills
	fMult *= GetSkillValue(eAimZoomSway);

	float faddP = fMult * g_vtCameraSwayYSpeed.GetFloat() * (float)sin(tm*g_vtCameraSwayYFreq.GetFloat()) * swayAmount;
	float faddY = fMult * g_vtCameraSwayXSpeed.GetFloat() * (float)sin(tm*g_vtCameraSwayXFreq.GetFloat()) * swayAmount;

	ApplyLocalRotationOffset( LTVector( faddP, faddY, 0.0f ));
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::CanChangeFOV
//
//	PURPOSE:	Is the FOV allowed to be changed...
//
// --------------------------------------------------------------------------- //

bool CPlayerCamera::CanChangeFOV()
{
	return ( (GetCameraMode() != kCM_Cinematic) && !IsZoomed( ) && !g_pPlayerMgr->IsUnderwater() );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::UpdateCameraFocusDirection
//
//	PURPOSE:	Update any extra focus rotation
//
// --------------------------------------------------------------------------- //

void CPlayerCamera::UpdateCameraFocusDirection()
{
	// Only do this when using a melee weapon!
	CClientWeapon* pWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();

	if( !( pWeapon && pWeapon->IsMeleeWeapon() ) )
	{
		return;
	}

	// First off, make sure we have a focus object
	if( !CBindMgr::GetSingleton().IsCommandOn( COMMAND_ID_FOCUS ) || !g_iFocusObjectDetector.GetObject() )
	{
		return;
	}

	// Check for axis input here, and return if there is enough to break away...
	if( fabs( CBindMgr::GetSingleton().GetExtremalCommandValue( COMMAND_ID_YAW_ACCEL ) ) > 0.02f )
	{
		return;
	}

	// Get the delta from the camera to the object
	LTVector vFocusObjectPos = g_iFocusObjectDetector.GetObjectSpatialPosition();
	LTVector vCamera = GetCameraPos();

	LTVector vDelta = ( vFocusObjectPos - vCamera );
	float fDeltaMagSqr = vDelta.MagSqr();

	// Calculate the delta rotation...
	LTRotation rCamera = GetLocalCameraRotation();
	LTRotation rFocus( vDelta.GetUnit(), rCamera.Up() );
	LTRotation rDelta = ( rFocus * ~rCamera );

	// Now flatten out this rotation so it only affects yaw
	LTVector vFlattened = rDelta.Forward();
	vFlattened.y = 0.0f;
	vFlattened.Normalize();

	LTRotation rFlattened( vFlattened, LTVector( 0.0f, 1.0f, 0.0f ) );

	// Figure out an appropriate multiplier for distance scaling...
	float fScale = g_vtCameraFocusScaleMax.GetFloat() * ( 1.0f - ( fDeltaMagSqr / g_vtCameraFocusRangeSqr.GetFloat() ) );
	fScale = LTCLAMP( fScale, g_vtCameraFocusScaleMin.GetFloat(), g_vtCameraFocusScaleMax.GetFloat() );

	// Apply a portion of this rotation to the local rotation
	LTRotation rApplied;
	rApplied.Slerp( LTRotation(), rFlattened, LTMIN( g_pLTClient->GetFrameTime() * fScale, 1.0f ) );

	SetLocalCameraRotation( rCamera * rApplied );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::CalcLocalRotationOffset( )
//
//	PURPOSE:	Calculates a rotation offset applied to a rotation in local space.
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::CalcLocalRotationOffset( LTRotation const& rSrcLocalRotation, const LTVector& vPYROffset,
											LTRotation& rDestLocalRotation )
{
	if( !g_pInterfaceMgr->AllowCameraRotation() || !g_pDamageFXMgr->AllowInput() || vPYROffset == LTVector::GetIdentity( ) )
	{
		rDestLocalRotation = rSrcLocalRotation;
		return;
	}

	// Create euler angles so we can apply limits.
	EulerAngles EA = Eul_FromQuat( rSrcLocalRotation, EulOrdYXZr );
	float fYaw		= EA.x + vPYROffset.y;
	float fPitch	= EA.y + vPYROffset.x;
	float fRoll		= EA.z + vPYROffset.z;
	float fLookUp   = fPitch;
	float fLookDown = fPitch;

	// Don't allow much movement up/down if 3rd person...
	if( GetCameraMode() == CPlayerCamera::kCM_Chase )
	{
		fLookDown = MATH_DEGREES_TO_RADIANS( g_vtCameraClampChaseLookDown.GetFloat( ));
		fLookUp = -MATH_DEGREES_TO_RADIANS( g_vtCameraClampChaseLookUp.GetFloat( ));
	}
	else if( g_pMoveMgr->IsDucking( ))
	{
		if( g_pPlayerMgr->GetPlayerFlags() & BC_CFLG_MOVING )
		{
			fLookDown = MATH_DEGREES_TO_RADIANS( g_vtCameraClampCrouchMovingLookDown.GetFloat( ));
			fLookUp = -MATH_DEGREES_TO_RADIANS( g_vtCameraClampCrouchMovingLookUp.GetFloat( ));
		}
		else
		{
			fLookDown = MATH_DEGREES_TO_RADIANS( g_vtCameraClampCrouchIdleLookDown.GetFloat( ));
			fLookUp = -MATH_DEGREES_TO_RADIANS( g_vtCameraClampCrouchIdleLookUp.GetFloat( ));
		}
	}
	else 
	{	
		if( g_pPlayerMgr->GetPlayerFlags() & BC_CFLG_MOVING )
		{
			fLookDown = MATH_DEGREES_TO_RADIANS( g_vtCameraClampMovingLookDown.GetFloat( ));
			fLookUp = -MATH_DEGREES_TO_RADIANS( g_vtCameraClampMovingLookUp.GetFloat( ));
		}
		else
		{
			fLookDown = MATH_DEGREES_TO_RADIANS( g_vtCameraClampIdleLookDown.GetFloat( ));
			fLookUp = -MATH_DEGREES_TO_RADIANS( g_vtCameraClampIdleLookUp.GetFloat( ));
		}
	}

	float fAimTrackingYMax = (IsZoomed( ) ? g_vtCameraAimTrackingYMaxZoomed.GetFloat( ) : g_vtCameraAimTrackingYMax.GetFloat( ));
	if( fPitch > -MATH_DEGREES_TO_RADIANS( fAimTrackingYMax ))
	{
		m_bSetAimTrackCollisionLimits = false;
		CPlayerBodyMgr::Instance( ).SetAimTrackingDefaultMaxSpeed( );
	}

	if( m_bSetAimTrackCollisionLimits )
	{
		LTRect2f rLimits;
		if( CPlayerBodyMgr::Instance( ).GetAimTrackingLimits( rLimits ) )
		{
			rLimits.m_vMax.y = MATH_DEGREES_TO_RADIANS( fAimTrackingYMax );
			CPlayerBodyMgr::Instance( ).SetAimTrackingLimits( rLimits );
			CPlayerBodyMgr::Instance( ).SetAimTrackingMaxSpeed( g_vtCameraAimTrackingCollisionMaxSpeed.GetFloat( ));
		}
	}
	else
	{
		CPlayerBodyMgr::Instance( ).SetAimTrackingDefaultLimits( );
	}

	fPitch = Clamp( fPitch, fLookUp, fLookDown );

	// Do special constraints if playerlure.
	if( g_pMoveMgr->GetVehicleMgr()->IsVehiclePhysics())
	{
		// Get the playerlurefx.  It has all of the data about the lure.
		PlayerLureFX* pPlayerLureFX = PlayerLureFX::GetPlayerLureFX( g_pMoveMgr->GetVehicleMgr()->GetPlayerLureId( ));
		if( !pPlayerLureFX )
		{
			ASSERT( !"CPlayerCamera::CalcLocalRotationOffset: Missing PlayerLureFX." );
			return;
		}

		// Make sure the angles are within the playerlure restrictions.
		ClampLocalAnglesToPlayerLure( *pPlayerLureFX, fPitch, fYaw );
	}

	rDestLocalRotation = LTRotation( fPitch, fYaw, fRoll );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::ApplyLocalRotationOffset( )
//
//	PURPOSE:	Rotate the camera by the offsets
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::ApplyLocalRotationOffset( const LTVector& vPYROffset )
{
	if( vPYROffset == LTVector::GetIdentity( ))
		return;

	LTRotation rNewLocalCameraRot;
	LTRotation rLocalCameraRot = GetLocalCameraRotation( );
	CalcLocalRotationOffset( rLocalCameraRot, vPYROffset, rNewLocalCameraRot );
	SetLocalCameraRotation( rNewLocalCameraRot );
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::AttachToTarget
//
//	PURPOSE:	Attach the camera to a socket on the target...
//
// --------------------------------------------------------------------------- //

void CPlayerCamera::AttachToTarget( bool bAttach, bool bInterpolate )
{
	// We don't want to reset our orientations if we are currently doing what was requested...
	if( m_bAttachedToTarget == bAttach )
		return;

	// Set the new value...
	m_bAttachedToTarget	= bAttach;
	m_bLerpAttached		= bInterpolate;

	// If we are enabling, then cache our values...
	if( bAttach )
	{
		m_rTargetAttachRot	= GetCameraRotation( );
	}
	else
	{
		// We are disabling, make our animation orientation our actual orientation...
		SetCameraRotation( m_rTargetAttachRot );
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::GetTargetSocketPosRot
//
//	PURPOSE:	Get the targets camera socket position...
//
// --------------------------------------------------------------------------- //

void CPlayerCamera::GetTargetSocketPosRot( LTVector& vPos, LTRotation& rRot, EnumAnimDesc eCameraDesc )
{
	HMODELSOCKET hSocket = INVALID_MODEL_SOCKET;
	HOBJECT hBody = NULL;
	
	// If we're alive and have a target object, use that, otherwise use the clientobject, since that's
	// the object that does the ragdoll.
	if (g_pPlayerMgr->IsPlayerAlive() && m_hTarget)
	{
		hBody = m_hTarget;
	}
	// The follow mode also needs to track the target.
	else if( GetCameraMode( ) == kCM_Follow && m_hTarget )
	{
		hBody = m_hTarget;
	}
	// No target or not supposed to use the target, then just use the clientobject.
	else
	{
		hBody = g_pLTClient->GetClientObject();
	}
		

	if( hBody )
	{
		g_pModelLT->DirtyAllNodeTransforms( hBody );

		if (g_pPlayerMgr->IsPlayerDead())
		{
			g_pModelLT->GetSocket( hBody, "CameraDEAD", hSocket );
		}
		else
		{
			g_pModelLT->GetSocket( hBody, "Camera", hSocket );
		}

		if(  hSocket != INVALID_MODEL_SOCKET )
		{
			LTTransform transform;
			if( g_pModelLT->GetSocketTransform( hBody, hSocket, transform, true ) == LT_OK )
			{
				vPos = transform.m_vPos;

				// Only get the transformation if we're not using the player body, or were told to do so by the animation...
				if( ( ( eCameraDesc == kAD_CAM_Rotation ) || ( eCameraDesc == kAD_CAM_RotationAim ) ) || !CPlayerBodyMgr::Instance( ).IsEnabled() ||
					(CPlayerBodyMgr::Instance( ).IsEnabled() && g_pMoveMgr->GetVehicleMgr()->IsVehiclePhysics( )) ||
					GetCameraMode() == kCM_Follow )
				{
					rRot = transform.m_rRot;
				}
				else
				{
					rRot = GetCameraRotation( );
				}
				

				// Factor in an affset...
				LTVector vOffset = rRot.RotateVector( LTVector( g_vtCameraAttachedOffsetX.GetFloat(),
																g_vtCameraAttachedOffsetY.GetFloat(),
																g_vtCameraAttachedOffsetZ.GetFloat() ) );

				vPos += vOffset;
			}
		}
		// Just use the object center when there's no socket.
		else
		{
			g_pLTBase->GetObjectPos( hBody, &vPos );
			g_pLTBase->GetObjectRotation( hBody, &rRot );
		}
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::Save
//
//	PURPOSE:	Save the camera data...
//
// --------------------------------------------------------------------------- //

void CPlayerCamera::Save( ILTMessage_Write *pMsg )
{
	pMsg->Writefloat( m_fSaveLODScale );

	pMsg->WriteLTRotation( m_rLocalRotation );
	pMsg->WriteLTRotation( m_rTargetAttachRot );

	pMsg->Writebool( m_bAttachedToTarget );
	pMsg->Writebool( m_bLerpAttached );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::Load
//
//	PURPOSE:	Load the camera data...
//
// --------------------------------------------------------------------------- //

void CPlayerCamera::Load( ILTMessage_Read *pMsg )
{
	m_fSaveLODScale			= pMsg->Readfloat();

	m_rLocalRotation		= pMsg->ReadLTRotation();
	m_rTargetAttachRot		= pMsg->ReadLTRotation();

	// Set he back up to the loaded one...
	m_rBackupCameraRot		= GetCameraRotation( );
	
	m_bAttachedToTarget		= pMsg->Readbool();
	m_bLerpAttached			= pMsg->Readbool();
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::WriteCameraInfo
//
//	PURPOSE:	Write the camera position and rotation to the messages...
//
// --------------------------------------------------------------------------- //

bool CPlayerCamera::WriteCameraInfo( ILTMessage_Write *pMsg, uint32 dwFlags )
{
	if( !pMsg )
		return false;

	if( !( dwFlags & CLIENTUPDATE_CAMERAINFO ))
		return false;

	LTRotation rCameraRot = GetCameraRotation();
	EulerAngles eaCamera = Eul_FromQuat( rCameraRot, EulOrdYXZr );
	eaCamera.z = g_pPlayerMgr->GetLeanMgr()->GetLeanAngle();

	bool bSeparateCameraFromBody = LadderMgr::Instance().IsClimbing() || g_pPlayerMgr->IsOperatingTurret( ); // || !g_pMoveMgr->IsMoving();
	
	LTRotation rCombined = LTRotation( eaCamera.y, eaCamera.x, eaCamera.z );
	pMsg->WriteCompLTRotation( rCombined );
	pMsg->Writebool(bSeparateCameraFromBody);
	if (bSeparateCameraFromBody)
	{
		//if the camera and body have separate rotations, send the body's yaw up
		EulerAngles EABody;
		LTRotation rRot;
		g_pLTClient->GetObjectRotation(g_pMoveMgr->GetObject(),&rRot);
		EABody = Eul_FromQuat( rRot, EulOrdYXZr );
		pMsg->Writefloat( EABody.x );
	}

	pMsg->WriteCompLTVector( GetCameraPos( ));
	
	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::GetCameraActivePos
//
//	PURPOSE:	Gets the actual active camera position (detects if cinematic)
//
// --------------------------------------------------------------------------- //

LTVector const& CPlayerCamera::GetCameraActivePos( )
{
	if (GetCameraMode() == kCM_Cinematic)
	{
		CSpecialFXList* pCameraList = g_pGameClientShell->GetSFXMgr()->GetCameraList();
		if( pCameraList )
		{
			int nNumCameras = pCameraList->GetSize();

			for( int nCamera = 0; nCamera < nNumCameras; ++nCamera )
			{
				CCameraFX* pCamFX = (CCameraFX*)(*pCameraList)[nCamera];
				if( !pCamFX )
					continue;

				HOBJECT hCamObj = pCamFX->GetServerObj();

				if( hCamObj )
				{
					uint32 dwUsrFlags;
					g_pCommonLT->GetObjectFlags( hCamObj, OFT_User, dwUsrFlags );

					if( dwUsrFlags & USRFLG_CAMERA_LIVE )
					{
						g_pLTClient->GetObjectPos(hCamObj, &m_vActivePos);
					}
				}
			}
		}
	}
	else
	{
		g_pLTClient->GetObjectPos( m_hCamera, &m_vActivePos );
	}

	return m_vActivePos;

}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::GetCameraActiveRotation
//
//	PURPOSE:	Gets the actual active camera rotation (detects if cinematic)
//
// --------------------------------------------------------------------------- //

LTRotation CPlayerCamera::GetCameraActiveRotation ( )
{
	if (GetCameraMode() == kCM_Cinematic)
	{
		CSpecialFXList* pCameraList = g_pGameClientShell->GetSFXMgr()->GetCameraList();
		if( pCameraList )
		{
			int nNumCameras = pCameraList->GetSize();

			for( int nCamera = 0; nCamera < nNumCameras; ++nCamera )
			{
				CCameraFX* pCamFX = (CCameraFX*)(*pCameraList)[nCamera];
				if( !pCamFX )
					continue;

				HOBJECT hCamObj = pCamFX->GetServerObj();

				if( hCamObj )
				{
					uint32 dwUsrFlags;
					g_pCommonLT->GetObjectFlags( hCamObj, OFT_User, dwUsrFlags );

					if( dwUsrFlags & USRFLG_CAMERA_LIVE )
					{
						g_pLTClient->GetObjectRotation(hCamObj, &m_vActiveRotation);
					}
				}
			}
		}
	}
	else
	{
		g_pLTClient->GetObjectRotation( m_hCamera, &m_vActiveRotation );
	}

	return m_vActiveRotation;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::ClampLocalAngles( )
//
//	PURPOSE:	Force camera rotation to be within a limited range
//
// ----------------------------------------------------------------------- //
void CPlayerCamera::SetCameraClamping( const LTRotation& rClamp, const LTVector2& vPitchRange, const LTVector2& vYawRange)
{
	m_bClamp = true;

	EulerAngles euAngles = Eul_FromQuat( rClamp, EulOrdYXZr );

	//save target angles in z component of clamp vector
	m_vPitchClamp.z = euAngles.y;
	m_vYawClamp.z = euAngles.x;

	m_vPitchClamp.x = vPitchRange.x;
	m_vPitchClamp.y = vPitchRange.y;

	m_vYawClamp.x = vYawRange.x;
	m_vYawClamp.y = vYawRange.y;

}

void CPlayerCamera::ClampLocalAngles(float& fPitch, float& fYaw)
{
	if (!m_bClamp)
		return;

	// target angles are saved in z component of clamp vector
	float fPDiff = LimitToPosNegPi(fPitch - m_vPitchClamp.z);
	float fYDiff = LimitToPosNegPi(fYaw - m_vYawClamp.z);

	if (fPDiff > m_vPitchClamp.x)
		fPitch = m_vPitchClamp.z + m_vPitchClamp.x;
	else if (fPDiff < m_vPitchClamp.y)
		fPitch = m_vPitchClamp.z + m_vPitchClamp.y;

	if (fYDiff > m_vYawClamp.x)
		fYaw = m_vYawClamp.z + m_vYawClamp.x;
	else if (fYDiff < m_vYawClamp.y)
		fYaw = m_vYawClamp.z + m_vYawClamp.y;


}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::RotateCamera( )
//
//	PURPOSE:	Interpolate into the specified rotation...
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::RotateCamera( LTRotation const &rRot, bool bPitch, bool bYaw, bool bRoll )
{
	// Cache current rotation...
	m_rSnapShotRotation = GetCameraRotation( );

	// Cache rotation to interpolate to...
	m_rInterpolationRotation = rRot;

	m_bInterpolatePitch = bPitch;
	m_bInterpolateYaw	= bYaw;
	m_bInterpolateRoll	= bRoll;

	// Begin interpolation...
	m_InterpolationTimer.Start( g_vtAttachedCamInterpolationRate.GetFloat( ) );
	m_bInterpolateRotation = true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::HandleStartMotion( )
//
//	PURPOSE:	Handle camera starting a specific motion, such as crouch or run...
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::HandleStartMotion( )
{
	// Create euler angles so we can apply limits.
	EulerAngles EA = Eul_FromQuat( GetLocalCameraRotation( ), EulOrdYXZr );
	float fPitch = EA.y;

	float fLookDown = fPitch;
	float fLookUp = fPitch;

	if( g_pMoveMgr->IsDucking( ))
	{
		if( g_pMoveMgr->GetControlFlags() & BC_CFLG_MOVING )
		{
			fLookDown = MATH_DEGREES_TO_RADIANS( g_vtCameraClampCrouchMovingLookDown.GetFloat( ));
			fLookUp = -MATH_DEGREES_TO_RADIANS( g_vtCameraClampCrouchMovingLookUp.GetFloat( ));
		}
		else
		{
			fLookDown = MATH_DEGREES_TO_RADIANS( g_vtCameraClampCrouchIdleLookDown.GetFloat( ));
			fLookUp = -MATH_DEGREES_TO_RADIANS( g_vtCameraClampCrouchIdleLookUp.GetFloat( ));
		}
	}
	else 
	{
		if( g_pMoveMgr->GetControlFlags() & BC_CFLG_MOVING )
		{
			fLookDown = MATH_DEGREES_TO_RADIANS( g_vtCameraClampMovingLookDown.GetFloat( ));
			fLookUp = -MATH_DEGREES_TO_RADIANS( g_vtCameraClampMovingLookUp.GetFloat( ));
		}
		else
		{
			fLookDown = MATH_DEGREES_TO_RADIANS( g_vtCameraClampIdleLookDown.GetFloat( ));
			fLookUp = -MATH_DEGREES_TO_RADIANS( g_vtCameraClampIdleLookUp.GetFloat( ));
		}
	}

	// If outside extents begin pitch interpolation to move within extents...
	if( fPitch < fLookUp || fPitch > fLookDown )
	{
		m_fInterpSmoothPitch = (fPitch < fLookUp ? fLookUp : fLookDown);
		m_fSnapShotSmoothPitch = fPitch;
		m_SmoothPitchTimer.Start( 0.3f );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::UpdateSmoothPitchClamping( )
//
//	PURPOSE:	Continuely update the pitch until it's with the allowed extents....
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::UpdateSmoothPitchClamping( LTRotation &rRot )
{
	if( m_SmoothPitchTimer.IsTimedOut( ))
		return;

	 // Create euler angles so we can apply limits.
	EulerAngles EA = Eul_FromQuat( rRot, EulOrdYXZr );
	float fPitch = EA.y;

	// If no longer outside of extents just stop the timer...

	float fLookDown = fPitch;
	float fLookUp = fPitch;
	
	if( g_pMoveMgr->IsDucking( ))
	{
		if( g_pMoveMgr->GetControlFlags() & BC_CFLG_MOVING )
		{
			fLookDown = MATH_DEGREES_TO_RADIANS( g_vtCameraClampCrouchMovingLookDown.GetFloat( ));
			fLookUp = -MATH_DEGREES_TO_RADIANS( g_vtCameraClampCrouchMovingLookUp.GetFloat( ));
		}
		else
		{
			fLookDown = MATH_DEGREES_TO_RADIANS( g_vtCameraClampCrouchIdleLookDown.GetFloat( ));
			fLookUp = -MATH_DEGREES_TO_RADIANS( g_vtCameraClampCrouchIdleLookUp.GetFloat( ));
		}
	}
	else 
	{
		if( g_pMoveMgr->GetControlFlags() & BC_CFLG_MOVING )
		{
			fLookDown = MATH_DEGREES_TO_RADIANS( g_vtCameraClampMovingLookDown.GetFloat( ));
			fLookUp = -MATH_DEGREES_TO_RADIANS( g_vtCameraClampMovingLookUp.GetFloat( ));
		}
		else
		{
			fLookDown = MATH_DEGREES_TO_RADIANS( g_vtCameraClampIdleLookDown.GetFloat( ));
			fLookUp = -MATH_DEGREES_TO_RADIANS( g_vtCameraClampIdleLookUp.GetFloat( ));
		}
	}

	if( fPitch < fLookDown && fPitch > fLookUp )
	{
		m_SmoothPitchTimer.Stop( );
		return;
	}

	double fDuration = m_SmoothPitchTimer.GetDuration();
	double fTimeLeft = m_SmoothPitchTimer.GetTimeLeft();
	float fT = (float)((fDuration - fTimeLeft) / fDuration);

	rRot.Slerp( LTRotation( m_fSnapShotSmoothPitch, EA.x, EA.z),
				LTRotation( m_fInterpSmoothPitch, EA.x, EA.z ), fT );
	
	SetCameraRotation( rRot );

	return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::UpdateRenderTarget()
//
//	PURPOSE:	Update the render target based on current settings.
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::UpdateRenderTarget()
{
	// If pixel doubling is turned off, we don't want to have a render target allocated
	// Note: pixel doubling doesn't work when antialiasing is turned on.
	if(( g_vtCameraPixelDouble.GetFloat() == 0.0f ) || ( GetConsoleFloat("AntiAliasFSOverSample", 0.0f) != 0.0f ))
	{
		if( m_hRenderTarget != NULL )
		{
			g_pLTRenderer->ReleaseRenderTarget( m_hRenderTarget );
			m_hRenderTarget = NULL;
		}
		return;
	}

	uint32 nCurTargetDimsX, nCurTargetDimsY;
	g_pLTRenderer->GetCurrentRenderTargetDims(nCurTargetDimsX, nCurTargetDimsY);
	
	// Check the resolution of the render target	
	if( m_hRenderTarget != NULL )
	{
		// If it doesn't match the current resolution, let go of the target
		if( ( m_nRenderTargetSize.x != (int)nCurTargetDimsX / 2 ) || 
			( m_nRenderTargetSize.y != (int)nCurTargetDimsY / 2 ) )
		{
			g_pLTRenderer->ReleaseRenderTarget( m_hRenderTarget );
			m_hRenderTarget = NULL;
		}
	}

	// Allocate a render target if we don't have one
	if( m_hRenderTarget == NULL )
	{
		m_nRenderTargetSize.Init(nCurTargetDimsX / 2, nCurTargetDimsY / 2);
		g_pLTRenderer->CreateRenderTarget(m_nRenderTargetSize.x, m_nRenderTargetSize.y, 
				eRTO_DepthBuffer | eRTO_CurrentFrameEffect | eRTO_PreviousFrameEffect | eRTO_FogVolume,
				m_hRenderTarget);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::UpdateCameraSmoothing()
//
//	PURPOSE:	Smooth the camera movement to avoid large jumps in position between frames...
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::UpdateCameraSmoothing( LTVector &rvPos )
{
	if( g_vtCameraSmoothingEnabled.GetFloat( ) <= 0.0f )
		return;

	if( !m_bAllowCameraSmoothing )
	{
		m_fLastInterpolatedValue = rvPos.y;
		return;
	}
	
	float fFrameTime = g_pLTClient->GetFrameTime( );
	float fYDelta = LTAbs( m_fLastInterpolatedValue - rvPos.y ) * fFrameTime;
	
	// Don't cache values to interpolate from under certain conditions...
	if( !g_pMoveMgr->IsFalling( ) && !g_pMoveMgr->Jumped( ) && !g_pMoveMgr->IsOnLift( )
		&& !g_pPlayerMgr->IsUnderwater( ) && (g_pMoveMgr->IsMoving( ) || g_pMoveMgr->IsDucking( )) )
	{
		if( fYDelta >= (g_vtCameraSmoothingAllowedDistance.GetFloat( ) * fFrameTime) && !m_CameraSmoothingTimer.IsStarted( ))
		{
			float fLeashLen = g_vtCameraSmoothingLeashLength.GetFloat( );
			if( fYDelta > fLeashLen )
			{
				m_fCameraSmoothingSnapShot = (m_fLastInterpolatedValue < rvPos.y ? rvPos.y - fLeashLen : rvPos.y + fLeashLen);
			}
			else
			{
				m_fCameraSmoothingSnapShot = m_fLastInterpolatedValue;
			}
			
			m_CameraSmoothingTimer.Start( g_vtCameraSmoothingInterpolateTime.GetFloat( ));
		}
	}

	// Reset the smoothing offset...
	m_fCameraSmoothingOffset = 0.0f;

	if( m_CameraSmoothingTimer.IsStarted( ))
	{
		if( m_CameraSmoothingTimer.IsTimedOut( ))
		{
			m_CameraSmoothingTimer.Stop( );
		}
		else
		{
			double fDuration = m_CameraSmoothingTimer.GetDuration( );
			double fTimeLeft = m_CameraSmoothingTimer.GetTimeLeft( );
			float fInterp = (float)((fDuration - fTimeLeft) / fDuration);

			float fDest = rvPos.y;
			rvPos.y = LTLERP( m_fCameraSmoothingSnapShot, rvPos.y, WaveFn_Sine(fInterp) );

			// Cache the offset so the player body may be moved the same distance...
			m_fCameraSmoothingOffset = rvPos.y - fDest;
		}
	}

	m_fLastInterpolatedValue = rvPos.y;
}

// EOF
