// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerCamera.cpp
//
// PURPOSE : PlayerCamera - Implementation
//
// (c) 1997-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PLAYERCAMERA_H__
#define __PLAYERCAMERA_H__

#include "AnimationDescriptors.h"
#include "ClientFXSequence.h"

class CPlayerCamera
{
	public:		// Methods...

		// Constants for switching camera modes...
		enum CameraMode	
		{
			kCM_None = -1,			// Null camera mode...
			kCM_FirstPerson,		// Normal first-person view...
			kCM_Chase,				// Chase camera (mpasscam)...
			kCM_Cinematic,			// Cinematic camera, used for in-game movies...
			kCM_Track,				// Fixed 3rd person camera, tracking some other object...
			kCM_Fly,				// Fly around mode.
			kCM_Follow,				// Follow mode.
			kCM_Fixed,				// Fixed first person view.
		};

		// Constructor
		CPlayerCamera();
		~CPlayerCamera();

		bool Init( );

		// The local player object has been initialized...
		void InitLocalPlayer( );

		// Handle first comming into a world...
		void OnEnterWorld( );

		void Save( ILTMessage_Write *pMsg );
		void Load( ILTMessage_Read *pMsg );

		// Render the camera... This should only be called from CGameClientShell::RenderCamera().
		void Render( );
		
		// Update the position & orientation of the camera based on the target...
		void Update( );

		// Set the target object of the camera (what it's attached to or looking at)...
		void SetTargetObject( HOBJECT hTarget );

		// Get the target object of the camera...
		HOBJECT GetTargetObject( ) const { return m_hTarget; }

		// Handle setting of the camera mode...
		void SetCameraMode( CameraMode eCamMode );

		// Test if the specified camera mode is set...
		CameraMode GetCameraMode( ) const { return m_eCameraMode; }

		// Get the actual camera object...
		HOBJECT	GetCamera( ) const { return m_hCamera; }

		// Test if the camera is allowed to render...
		bool CanRender( ) const { return m_bCanRender; }

		// Rotate the camera in local space by the euler offsets.
		void ApplyLocalRotationOffset( const LTVector& vPYROffset );

		// Calculates a local camera rotation given an euler offset.
		void CalcLocalRotationOffset( LTRotation const& rSrcLocalRotation, const LTVector& vPYROffset,
											LTRotation& rDestLocalRotation );

		LTVector const& GetCameraPos( ) const
		{
			return m_vPos;
		}

		void SetCameraPos( LTVector const& vPos )
		{
			m_vPos = vPos;
			m_tfWorld.m_vPos = vPos;
		}

		// Rotation accessors...
		LTRotation GetCameraRotation( ) const
		{
			LTRigidTransform const& tfWorld = GetWorldTransform( );
			LTRotation rWorldRot = tfWorld.m_rRot * m_rLocalRotation;
			return rWorldRot;
		}

		LTVector const& GetCameraActivePos( );

		LTRotation GetCameraActiveRotation ( );

		void SetCameraRotation( LTRotation const& rRot )
		{
			LTRigidTransform const& tfWorld = GetWorldTransform( );
			SetLocalCameraRotation( ~tfWorld.m_rRot * rRot );
		}

		// Interpolate into the specified rotation...
		void RotateCamera( LTRotation const &rRot, bool bPitch, bool bYaw, bool bRoll );

		LTRotation const& GetLocalCameraRotation( ) const
		{
			return m_rLocalRotation;
		}

		void SetLocalCameraRotation( LTRotation const& rLocalCameraRot )
		{
			m_rLocalRotation = rLocalCameraRot;
		}

		void SaveBackupRotation( )
		{
			m_rBackupCameraRot = GetCameraRotation();
		}

		void RestoreBackupRotation( )
		{
			SetCameraRotation( m_rBackupCameraRot );
		}


		// Gets the transform to go from local camera space to world space.
		LTRigidTransform const& GetWorldTransform( ) const { return m_tfWorld; }

		// Reset the camera objects state...
		void ResetCamera( );

		// Toggles zoom.
		void HandleZoomRequest( bool bEnable);

		// Cancel all zoom.
		void ExitZoom( );

		// Shows the range finder HUD element...
		void ShowRangeFinder( );

		// Check if we are in any state of zoom.
		bool IsZoomed( )		const { return m_eZoomState != eEffectState_Out; }
		// Check if we are changing our zoom state.
		bool IsZooming( )		const { return !m_ZoomTimer.IsTimedOut( ); }
		// Check to see if we are exitting our zoom.
		bool IsExittingZoom( )	const { return m_eZoomState == eEffectState_Exiting;}

		float GetZoomMag( )		const;


		// Is the FOV allowed to be changed...
		bool CanChangeFOV();

		// Attach the camera to a socket on the target...
		void AttachToTarget( bool bAttach = true, bool bInterpolate = false );

		// Is the camera using an animation to update it's position...
		bool IsAttachedToTarget( ) const { return m_bAttachedToTarget; }

		void SetTargetAttachRot( LTRotation const& rRot ) { m_rTargetAttachRot = rRot; }
		LTRotation const& GetTargetAttachRot( ) const { return m_rTargetAttachRot; }

		void SetFirstPersonOffset( LTVector vOffset )	{ m_vFirstPersonOffset = vOffset; }
		const LTVector& GetFirstPersonOffset( ) const	{ return m_vFirstPersonOffset; }

		void CalcNonClipPos( LTVector & vPos, LTRotation & rRot );
		void CalcNoClipPos_WithoutObject( LTVector & vPos, LTRotation & rRot );

		// Write the camera position and rotation to the messages...
		bool WriteCameraInfo( ILTMessage_Write *pMsg, uint32 dwFlags );

		void SetCameraClamping(const LTRotation& rClamp, const LTVector2& vPitchRange, const LTVector2& vYawRange);
		void ClearCameraClamping() {m_bClamp = false;}

		// Retrieve the smoothing offset so other systems can adjust accordingly...
		float GetCameraSmoothingOffset( ) const { return m_fCameraSmoothingOffset; }

		// Handle camera starting a specific motion, such as crouch or run...
		void HandleStartMotion( );

		// Continuely update the pitch until it's with the allowed extents....
		void UpdateSmoothPitchClamping( LTRotation &rRot );

		// Resets the height interp frame value.
		void ResetHeightInterp( ) { m_fLastHeightInterp = GetCameraPos( ).y; }

		// Reset the camera rotations when we have entered a change to our world transform, such as going in 
		// and out of playerlure.
		void ResetCameraRotations( );

		// Will the PlayerCamera allow the crosshair to render...
		bool CanShowCrosshair( ) const { return m_bCanShowCrosshair; }

		// Temporairly disable camera smoothing for one update...
		void DisableCameraSmoothing( ) { m_bAllowCameraSmoothing = false; m_CameraSmoothingTimer.Stop( ); }


	private:	// Methods...

		// Point the camera at a position from a position
		void PointAtPosition(const LTVector &pos, const LTRotation &rot, const LTVector &pointFrom);

		// Move the camera to the position...
		void MoveCameraToPosition( LTVector const& vPos );

		// Find the optimal camera position
		LTVector FindChaseCameraPosition( );

		void SaveCameraMode( )
		{
			m_eSaveCameraMode = m_eCameraMode;
		}

		void RestoreCameraMode( )
		{
			m_eCameraMode = m_eSaveCameraMode;
		}

		// Updates the world transform.
		void UpdateWorldTransform( );

		// Update position and rotation while in chase mode...
		void UpdateChase( );

		// Update position and rotation while in track mode...
		void UpdateTrack( );

		// Update position and rotation while in first person mode...
		void UpdateFirstPerson( );

		// Update position and rotation while in fly around mode...
		void UpdateFly( );

		// Update position and rotation while in follow mode...
		void UpdateFollow( );

		// Update position and rotation while in fixed around mode...
		void UpdateFixed( );

		// Gets the first person offset in local space.
		LTVector GetFirstPersonLocalSpaceOffset( );

		// Create the actual camera object to use...
		bool CreateCameraObject( );
		
		// Create an object to use for collision detection when calculating a no-clip pos...
		bool CreateCameraCollisionObject( );

		// Check the list of alternative cameras for the first live one and switch modes if found...
		bool UpdateCinematicCameras( );

		// Update the camera's sway...
		void UpdateCameraSway( );

		// Update any extra focus rotation
		void UpdateCameraFocusDirection( );

		// Get the targets camera socket position...
		void GetTargetSocketPosRot( LTVector& vPos, LTRotation& rRot, EnumAnimDesc eCameraDesc );

		// Calculates the local camera rotation.
		void CalculateLocalCameraRotation( );

		// Clamp camera rotation to some range of angles
		void ClampLocalAngles( float& fPitch, float& fYaw );

		// Actually handle the RenderCamera call..
		void RenderCamera( );

		// Handle special actions required for pre/post rendering while in chase mode...
		void RenderChase( );

		// Handle special actions required for pre/post rendering while in first person mode...
		void RenderFirstPerson( );

		// Sets the aspect ratio to zoom transition.
		void SetZoomAspectRatio( );

		// Start zoomming...
		void EnterZoom( );
		// Update zoom.
		void UpdateZoom( );
		// Finish zoomming...
		void ExitingZoom( );

		// Update the render target based on current settings
		void UpdateRenderTarget( );

		// Smooth the camera movement to avoid large jumps in position between frames...
		void UpdateCameraSmoothing( LTVector &rvPos );

		
	private :	// Members...

		// The actual camera object...
		LTObjRef		m_hCamera;

		// Worldspace position.
		LTVector		m_vPos;

		// Worldspace rotation.
		LTRotation		m_rLocalRotation;

		// Cinema position.

		LTVector		m_vActivePos;

		// Cinema rotation

		LTRotation		m_vActiveRotation;

		// Used to backup the camera rotation while game is paused.
		LTRotation		m_rBackupCameraRot;

		// Used to interpolate from this snap-shot rotation into another rotation...
		LTRotation		m_rSnapShotRotation;

		// If not at identity this is the rotation to interpolate into...
		LTRotation		m_rInterpolationRotation;

		// Are we interpolating into a rotation...
		bool			m_bInterpolateRotation;

		// Interpolate over the duration of this timer...
		StopWatchTimer	m_InterpolationTimer;

		// Flags for enabling pitch along specific axi... 
		bool			m_bInterpolatePitch;
		bool			m_bInterpolateYaw;
		bool			m_bInterpolateRoll;

		// World transform.
		LTRigidTransform m_tfWorld;

		// The euler angles for the orientation that the camera was in when it was attached
		// to the target. This allows for the orientation of the camera to be taken from
		// the model when it ends
		LTRotation		m_rTargetAttachRot;

		bool			m_bAttachedToTarget;
		bool			m_bLerpAttached;

		// Target object for the camera to follow
		LTObjRef		m_hTarget;

		// The last position of our target. Used in chase and track modes...
		LTVector		m_vLastTargetPos;
		
		// The last rotation of our target. Used in chase and track modes...
		LTRotation		m_rLastTargetRot;

		// Last optimal camera pos. Used in chase mode...
		LTVector		m_vLastOptPos;
		
		CameraMode		m_eCameraMode;
		CameraMode		m_eSaveCameraMode;

		// Offset from the target's position.
		// If a wall is between the optimal camera position and the target, the camera
		// will move closer to the target's position added to this offset. (Chase view)
		LTVector		m_vChaseOffset;

		// Offset from the target's position...
		// The camera will point at this potision (Chase view)
		LTVector		m_vChasePointAtOffset;

		// Offset from the target's position...
		// The camera will point at this potision (Track view)
		LTVector		m_vTrackPointAtOffset;

		// Offset from the target's position.
		// The camera will be located at this offset from the target's position...
		LTVector		m_vFirstPersonOffset;

		// The object used for collision detection when calculating a no-clip pos...
		LTObjRef		m_hCollisionObject;

		// Camera zoom related variables...

		EffectState		m_eZoomState;
		StopWatchTimer	m_ZoomTimer;		// Used for timing of zoom in/out.
		float			m_fSaveLODScale;	// LOD Scale value before zooming
		bool			m_bIronSight;		// Indicates looking down barrel of gun rather than thru scope.

		// We need an update before the camera can actually be rendered...
		bool			m_bCanRender;

		// Last camera descriptor of the PlayerBody...
		EnumAnimDesc	m_eLastCameraDesc;

		bool			m_bClamp;
		LTVector		m_vPitchClamp;
		LTVector		m_vYawClamp;

		// The height interpolation offset for smoothing out steps and junk
		bool			m_bHasLastHeightInterp;
		float			m_fLastHeightInterp;

		float			m_fSnapShotSmoothPitch;
		float			m_fInterpSmoothPitch;
		StopWatchTimer	m_SmoothPitchTimer;

		
		HRENDERTARGET	m_hRenderTarget;
		LTVector2n		m_nRenderTargetSize;

		// Interpolation timers for movement along the axis...
		StopWatchTimer	m_CameraSmoothingTimer;
		
		// Individual snapshot values to interpolate from, along the axis...
		float			m_fCameraSmoothingSnapShot;

		// Individual offsets from actual position to interpolated position...
		float			m_fCameraSmoothingOffset;

		// Individual previous interpolated values...
		float			m_fLastInterpolatedValue;

		CClientFXSequence m_fxZoomSequence;

		// Specifies if the PlayerCamera will allow the crosshair to render or not...
		bool			m_bCanShowCrosshair;

		// Disable camera smoothing.  Reset after each update...
		bool			m_bAllowCameraSmoothing;

		// Specifies if the PlayerBody aim tracking limits should be clamped to a specific value...
		bool			m_bSetAimTrackCollisionLimits;

		LTVector		m_vLastPositionOfCollision;
};


#endif//__PLAYERCAMERA_H__

