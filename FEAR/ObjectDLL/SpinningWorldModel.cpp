// ----------------------------------------------------------------------- //
//
// MODULE  : SpinningWorldModel.cpp	
//
// PURPOSE : SpinningWorldModel implementation
//
// CREATED : 5/22/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//
	
	#include "Stdafx.h"
	#include "SpinningWorldModel.h"

//
// Defines...
//

LINKFROM_MODULE( SpinningWorldModel );

// 
// Add props...
//


BEGIN_CLASS( SpinningWorldModel )

	// Set the type

	AWM_SET_TYPE_ROTATING

	// Overrides
	
	ADD_BOOLPROP_FLAG( BoxPhysics, true, 0, "In the engine, WorldModels have two possible physics models. In the first, the player can walk on and touch every curve and corner of their surface. In the second, the player can only interact with a bounding box that surrounds all the brushes in the WorldModel, just like the box you would get if you selected them in WorldEdit. For geometry with a simple rectangular shape, this is preferred because it's cheaper to calculate. However, for a lot of objects, it's limiting. If you need a player to be able to shoot through the bars in a prison door, you will need BoxPhysics set to FALSE." )
	ADD_BOOLPROP_FLAG(RemainOn, true, PF_GROUP(3) | PF_HIDDEN, "If this is FALSE the Object will start turnning itself off or close as soon as it turns on or opens.  If TRUE the object will stay on or open untill told to turn off, either by the player or a message.")
	ADD_BOOLPROP_FLAG(RotateAway, false, PF_GROUP(3) | PF_HIDDEN, "If set to TRUE RotatingWorldModels will rotate away from the player or AI that activated it.")

	ADD_BOOLPROP_FLAG(RotateAroundCenter, true, 0, "When set to TRUE, object will use the center of worldmodel as rotation point.  When sent to TRUE, object will use WorldModel object as rotation point.  If RotationPoint is specified, this property is ignored.")
	ADD_VECTORPROP_VAL_FLAG(RotationAngles, 0.0f, 5.0f, 0.0f, 0, "These represent how far the WorldModel will rotate around the RotationPoint in the specified axi when turned on.  (0.0 90.0 0.0) will rotate the WorldModel about the RotationPoint 90 degrees around the WorldModels local Y axis.")

	// Override the common props...

	ADD_REALPROP_FLAG(PowerOnTime, 100.0f, 0, "Sets the time in seconds for how long it takes the WorldModel to go from the Off state to the on state.")
	ADD_REALPROP_FLAG(PowerOffTime, 0.0f, 0, "If other than 0.0, sets the time in seconds for how long it takes the WorldModel to go from the On state to the off state.  If this is 0.0 then the PowerOnTime value is used." )
	ADD_REALPROP_FLAG(MoveDelay, 0.0f, PF_HIDDEN, "Amount of delay in seconds between the time the WorldModel is triggered and when it begins its movement.")
	ADD_REALPROP_FLAG(OnWaitTime, 0.0f, PF_HIDDEN, "Amount of time in seconds that the WorldModel will remain on before turnning off automatically, and the amount of time before the WorldModel can be triggered on again." )
	ADD_REALPROP_FLAG(OffWaitTime, 0.0f, PF_HIDDEN, "Amount of time in secomds before the WorldModel can be turned on after being turned off.")

	ADD_PREFETCH_RESOURCE_PROPS()

END_CLASS_FLAGS_PREFETCH(SpinningWorldModel, ActiveWorldModel, CF_WORLDMODEL, DefaultPrefetch<ActiveWorldModel>, "SpinningWorldModels are WorldModels that will continualy rotate around a specified point.  You can use these to easily set up fans or other continualy rotating objects." )

//
// Register the calss with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( SpinningWorldModel )
CMDMGR_END_REGISTER_CLASS( SpinningWorldModel, ActiveWorldModel )

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SpinningWorldModel::SpinningWorldModel
//
//  PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

SpinningWorldModel::SpinningWorldModel()
:	ActiveWorldModel(),
	m_vVelocity( 0.0f, 0.0f, 0.0f ),
	m_vFinalVelocity( 0.0f, 0.0f, 0.0f ),
	m_fLastTime( 0.0f ),
	m_bUpdateSpin( true )
{

}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SpinningWorldModel::~SpinningWorldModel
//
//  PURPOSE:	Destroy object
//
// ----------------------------------------------------------------------- //

SpinningWorldModel::~SpinningWorldModel()
{

}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SpinningWorldModel::ReadProps
//
//  PURPOSE:	Read in property values
//
// ----------------------------------------------------------------------- //

void SpinningWorldModel::ReadProps( const GenericPropList *pProps )
{
	LTASSERT(pProps != NULL, "TODO: Add description here");

	// Read base class props first

	ActiveWorldModel::ReadProps( pProps );

	// Default to using rotatearoundcenter for spinningworldmodels.
	m_dwPropFlags |= (pProps->GetBool( "RotateAroundCenter", true ) ? AWM_PROP_ROTATEAROUNDCENTER : 0);

	// [RP] 03/08/05 - The calculations below don't make since.  Why are we
	//		calculating the velocity by dividing 2*pi by the degrees specified
	//		in the editor?  Changing the calculations at this time would mess
	//		up existing content so reevaluate after F.E.A.R.  The below comment
	//		is also obsolete since the current tech doesn't have animated lightmaps.

	// Get how long it takes to make one full rotation around the axi...
	// Set the RotationAngles for an axi to be almost a full cirlce in degrees.
	// This way when the OnAngles get calculated they will be accurate for 
	// computing the percentage used in animated light maps.

	if( m_vRotationAngles.x )
	{
		m_vFinalVelocity.x = MATH_CIRCLE / MATH_RADIANS_TO_DEGREES(m_vRotationAngles.x);
		m_vRotationAngles.x = MATH_CIRCLE;
	}

	if( m_vRotationAngles.y )
	{
		m_vFinalVelocity.y = MATH_CIRCLE / MATH_RADIANS_TO_DEGREES(m_vRotationAngles.y);
		m_vRotationAngles.y = MATH_CIRCLE;
	}

	if( m_vRotationAngles.z )
	{
		m_vFinalVelocity.z = MATH_CIRCLE / MATH_RADIANS_TO_DEGREES(m_vRotationAngles.z);
		m_vRotationAngles.z = MATH_CIRCLE;
	}

	m_vInitOffAngles.Init();
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SpinningWorldModel::SetOn
//
//  PURPOSE:	Sets the AWM to the On state
//
// ----------------------------------------------------------------------- //

void SpinningWorldModel::SetOn( bool bInitialize )
{
	ActiveWorldModel::SetOn( bInitialize );

	m_fLastTime = g_pLTServer->GetTime();

	// Keep spinning...

	SetNextUpdate( UPDATE_NEXT_FRAME );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SpinningWorldModel::SetPowerOn
//
//  PURPOSE:	Sets the AWM to the PowerOn state
//
// ----------------------------------------------------------------------- //

void SpinningWorldModel::SetPowerOn( float fTime, uint8 nWaveform )
{
	ActiveWorldModel::SetPowerOn( fTime, nWaveform );

	// Reset values to begin the spin from where it ended up...
	m_fLastTime = g_pLTServer->GetTime( );
	m_fPitch = m_fYaw = m_fRoll = 0.0f;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SpinningWorldModel::UpdateOn
//
//  PURPOSE:	Handel updating the On state
//
// ----------------------------------------------------------------------- //

void SpinningWorldModel::UpdateOn( const double &fCurTime )
{
	LTVector	vNewPos(0.0f, 0.0f, 0.0f);
	LTRotation	rNewRot;
	float		fDeltaTm = (float)(fCurTime - m_fLastTime);
	float		fPercent = 0.0f;
	LTVector	vOldAngles( m_fPitch, m_fYaw, m_fRoll );

	m_bUpdateSpin = true;

	if( m_vVelocity.x )
	{
		m_fPitch += m_vVelocity.x * fDeltaTm;
	}

	if( m_vVelocity.y )
	{
		m_fYaw += m_vVelocity.y * fDeltaTm;
	}

	if( m_vVelocity.z )
	{
		m_fRoll += m_vVelocity.z * fDeltaTm;
	}

	float	fDifLeft = (m_vInitOnAngles - LTVector( m_fPitch, m_fYaw, m_fRoll )).Mag();
	float	fFinalDif = (m_vInitOnAngles - m_vInitOffAngles).Mag();
		
	// Get the percent of our rotation to use for the light ani...

	fPercent = ( fFinalDif > MATH_EPSILON ? 1 - fDifLeft / fFinalDif : 1.0f );

	uint32 nFlags;
	g_pCommonLT->GetObjectFlags( m_hObject, OFT_Flags, nFlags );
	bool bTestCollisions = !!( nFlags & FLAG_SOLID );

	if( !CalculateNewPosRot( vNewPos, rNewRot, m_vOnPos, m_fPowerOnTime, fPercent, bTestCollisions ) && !(m_dwPropFlags & AWM_PROP_FORCEMOVE) )
	{
		// Restore our angles...

		m_fPitch	= vOldAngles.x;
		m_fYaw		= vOldAngles.y;
		m_fRoll		= vOldAngles.z;

		m_fMoveStartTm	+= g_pLTServer->GetFrameTime();
		m_fLastTime		= fCurTime;
		m_bUpdateSpin	= false;

		return;
	}

	g_pLTServer->Physics()->MoveObject( m_hObject, vNewPos, 0 );

	// Check to see if we actually moved anywhere...

	LTVector	vPos;
	g_pLTServer->GetObjectPos( m_hObject, &vPos );
	if( !vPos.NearlyEquals( vNewPos, MATH_EPSILON ) )
	{
		// Restore our angles...

		m_fPitch	= vOldAngles.x;
		m_fYaw		= vOldAngles.y;
		m_fRoll		= vOldAngles.z;

		m_fMoveStartTm	+= g_pLTServer->GetFrameTime();
		m_fLastTime		= fCurTime;
		m_bUpdateSpin	= false;

		return;
	}

	g_pLTServer->RotateObject( m_hObject, rNewRot );

	// Keep the Pitch, Yaw and Roll within 2PI so they will never over flow...

	if( m_fPitch > MATH_CIRCLE )
	{
		m_fPitch = -(MATH_CIRCLE - m_fPitch);
	}
	else if( m_fPitch < -MATH_CIRCLE )
	{
		m_fPitch += MATH_CIRCLE;
	}

	if( m_fYaw > MATH_CIRCLE )
	{
		m_fYaw = -(MATH_CIRCLE - m_fYaw);
	}
	else if( m_fYaw < -MATH_CIRCLE )
	{
		m_fYaw += MATH_CIRCLE;
	}

	if( m_fRoll > MATH_CIRCLE )
	{
		m_fRoll = -(MATH_CIRCLE - m_fRoll);
	}
	else if( m_fRoll < -MATH_CIRCLE )
	{
		m_fRoll += MATH_CIRCLE;
	}

	m_fLastTime = fCurTime;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SpinningWorldModel::UpdatePowerOn
//
//  PURPOSE:	Handel updating the PowerOn state
//
// ----------------------------------------------------------------------- //

void SpinningWorldModel::UpdatePowerOn( const double &fCurTime )
{
	bool	bDoneInX = false;
	bool	bDoneInY = false;
	bool	bDoneInZ = false;
	float	fPercent;
	float	fRate;
	float	fFrameTm = g_pLTServer->GetFrameTime();
	
	fPercent = ((float)(fCurTime - m_fMoveStartTm)) / m_fPowerOnTime;

	if( m_bUpdateSpin )
	{
		// Update Pitch PowerUP...

		if( m_vFinalVelocity.x )
		{
			fRate = m_vFinalVelocity.x / m_fPowerOnTime;
					
			m_vVelocity.x += GetWaveformValue( fRate, fPercent ) * fFrameTm * m_vInitRotDir.x;

			if( m_vVelocity.x >= m_vFinalVelocity.x )
			{
				m_vVelocity.x = m_vFinalVelocity.x;
				bDoneInX = true;
			}
		}
		else
		{
			bDoneInX = true;
		}

		// Update Yaw PowerUP...

		if( m_vFinalVelocity.y )
		{
			fRate = m_vFinalVelocity.y / m_fPowerOnTime;
			
			m_vVelocity.y += GetWaveformValue( fRate, fPercent ) * fFrameTm * m_vInitRotDir.y;

			if( m_vVelocity.y >= m_vFinalVelocity.y )
			{
				m_vVelocity.y = m_vFinalVelocity.y;
				bDoneInY = true;
			}
		}
		else
		{
			bDoneInY = true;
		}

		// Update Roll PowerUP...

		if( m_vFinalVelocity.z )
		{
			fRate = m_vFinalVelocity.z / m_fPowerOnTime;

			m_vVelocity.z += GetWaveformValue( fRate, fPercent ) * fFrameTm * m_vInitRotDir.z;

			if( m_vVelocity.z >= m_vFinalVelocity.z )
			{
				m_vVelocity.z = m_vFinalVelocity.z;
				bDoneInZ = true;
			}
		}
		else
		{
			bDoneInZ = true;
		}
	}

	if( fPercent > 1.0f )
	{
		bDoneInX = true;
		bDoneInY = true;
		bDoneInZ = true;

		m_vVelocity = m_vFinalVelocity;
	}

	// Let the "on" update do the actual rotating

	UpdateOn( fCurTime );

	if( bDoneInX && bDoneInY && bDoneInZ )
	{
		SetOn();
	}
}



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SpinningWorldModel::UpdateOff
//
//  PURPOSE:	Handel updating the Off state
//
// ----------------------------------------------------------------------- //

void SpinningWorldModel::UpdateOff( const double &fCurTime )
{

}



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SpinningWorldModel::UpdatePowerOff
//
//  PURPOSE:	Handel updating the PowerOff state
//
// ----------------------------------------------------------------------- //

void SpinningWorldModel::UpdatePowerOff( const double &fCurTime )
{
	bool	bDoneInX = false;
	bool	bDoneInY = false;
	bool	bDoneInZ = false;
	float	fPercent;
	float	fRate;
	float	fFrameTm = g_pLTServer->GetFrameTime();
	
	fPercent = ((float)(fCurTime - m_fMoveStartTm)) / m_fPowerOffTime;
	if( fPercent > 1.0f )
	{
		bDoneInX = true;
		bDoneInY = true;
		bDoneInZ = true;

		m_vVelocity = LTVector( 0.0f, 0.0f, 0.0f );
	}
	
	if( m_bUpdateSpin )
	{
		// Update Pitch PowerDOWN...

		if( m_vFinalVelocity.x  )
		{
			fRate = m_vFinalVelocity.x / m_fPowerOffTime;
			
			m_vVelocity.x -= GetWaveformValue( fRate, fPercent ) * fFrameTm * m_vInitRotDir.x ;

			if( m_vVelocity.x < MATH_EPSILON )
			{
				m_vVelocity.x = 0.0f;
				bDoneInX = true;
			}
		}
		else
		{
			bDoneInX = true;
		}

		// Update Yaw PowerDOWN...

		if( m_vFinalVelocity.y )
		{
			fRate = m_vFinalVelocity.y / m_fPowerOffTime;
			
			m_vVelocity.y -= GetWaveformValue( fRate, fPercent ) * fFrameTm * m_vInitRotDir.y;

			if( m_vVelocity.y < MATH_EPSILON )
			{
				m_vVelocity.y = 0.0f;
				bDoneInY = true;
			}
		}
		else
		{
			bDoneInY = true;
		}

		// Update Roll PowerDOWN...

		if( m_vFinalVelocity.z )
		{
			fRate = m_vFinalVelocity.z / m_fPowerOffTime;
			
			m_vVelocity.z -= GetWaveformValue( fRate, fPercent ) * fFrameTm * m_vInitRotDir.z;

			if( m_vVelocity.z < MATH_EPSILON )
			{
				m_vVelocity.z = 0.0f;
				bDoneInZ = true;
			}
		}
		else
		{
			bDoneInZ = true;
		}
	}

	// Let the "on" update do the actual rotating

	UpdateOn( fCurTime );

	if( bDoneInX && bDoneInY && bDoneInZ )
	{
		SetOff( !AWM_INITIAL_STATE );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SpinningWorldModel::OnSave
//
//  PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void SpinningWorldModel::OnSave( ILTMessage_Write *pMsg, uint32 dwSaveFlags )
{
	if( !pMsg ) return;

	// Send to base class

	ActiveWorldModel::OnSave( pMsg, dwSaveFlags );

	SAVE_VECTOR( m_vVelocity );
	SAVE_VECTOR( m_vFinalVelocity );
	SAVE_bool( m_bUpdateSpin );
	SAVE_TIME( m_fLastTime );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SpinningWorldModel::OnLoad
//
//  PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void SpinningWorldModel::OnLoad( ILTMessage_Read *pMsg, uint32 dwSaveFlags )
{
	if( !pMsg ) return;

	// Send to base class

	ActiveWorldModel::OnLoad( pMsg, dwSaveFlags );

	LOAD_VECTOR( m_vVelocity );
	LOAD_VECTOR( m_vFinalVelocity );
	LOAD_bool( m_bUpdateSpin );
	LOAD_TIME( m_fLastTime );
}
