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
	
	#include "stdafx.h"
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
	
	ADD_BOOLPROP_FLAG( BoxPhysics, LTTRUE, 0 )
	ADD_BOOLPROP_FLAG(RemainOn, LTTRUE, PF_GROUP(3) | PF_HIDDEN)
	ADD_BOOLPROP_FLAG(RotateAway, LTFALSE, PF_GROUP(3) | PF_HIDDEN)

	ADD_VECTORPROP_VAL_FLAG(RotationAngles, 0.0f, 5.0f, 0.0f, 0)

	// Override the common props...

	ADD_REALPROP_FLAG(PowerOnTime, 100.0f, 0)
	ADD_REALPROP_FLAG(PowerOffTime, 0.0f, 0)
	ADD_REALPROP_FLAG(MoveDelay, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(OnWaitTime, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(OffWaitTime, 0.0f, PF_HIDDEN)

END_CLASS_DEFAULT_FLAGS( SpinningWorldModel, ActiveWorldModel, NULL, NULL, CF_WORLDMODEL )

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
	m_bUpdateSpin( LTTRUE )
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

void SpinningWorldModel::ReadProps( ObjectCreateStruct *pOCS )
{
	_ASSERT( pOCS != NULL );

	// Read base class props first

	ActiveWorldModel::ReadProps( pOCS );

	// Get how long it takes to make one full rotation around the axi...
	// Set the RotationAngles for an axi to be almost a full cirlce in degrees.
	// This way when the OnAngles get calculated they will be accurate for 
	// computing the percentage used in animated light maps.

	if( m_vRotationAngles.x )
	{
		m_vFinalVelocity.x = MATH_CIRCLE / m_vRotationAngles.x;
		m_vRotationAngles.x = 359.9f;
	}

	if( m_vRotationAngles.y )
	{
		m_vFinalVelocity.y = MATH_CIRCLE / m_vRotationAngles.y;
		m_vRotationAngles.y = 359.9f;
	}

	if( m_vRotationAngles.z )
	{
		m_vFinalVelocity.z = MATH_CIRCLE / m_vRotationAngles.z;
		m_vRotationAngles.z = 359.9f;
	}

	m_vOffAngles.Init();

}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SpinningWorldModel::SetOff
//
//  PURPOSE:	Sets the AWM to the On state
//
// ----------------------------------------------------------------------- //

void SpinningWorldModel::SetOn( LTBOOL bInitialize )
{
	ActiveWorldModel::SetOn( bInitialize );

	m_fLastTime = g_pLTServer->GetTime();

	// Keep spinning...

	SetNextUpdate( UPDATE_NEXT_FRAME );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SpinningWorldModel::UpdateOn
//
//  PURPOSE:	Handel updating the On state
//
// ----------------------------------------------------------------------- //

void SpinningWorldModel::UpdateOn( const LTFLOAT &fCurTime )
{
	LTVector	vNewPos(0.0f, 0.0f, 0.0f);
	LTRotation	rNewRot;
	LTFLOAT		fDeltaTm = fCurTime - m_fLastTime;
	LTFLOAT		fPercent = 0.0f;
	LTVector	vOldAngles( m_fPitch, m_fYaw, m_fRoll );

	m_bUpdateSpin = LTTRUE;

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

	LTFLOAT	fDifLeft = (m_vOnAngles - LTVector( m_fPitch, m_fYaw, m_fRoll )).Mag();
	LTFLOAT	fFinalDif = (m_vOnAngles - m_vOffAngles).Mag();
		
	// Get the percent of our rotation to use for the light ani...

	fPercent = ( fFinalDif > MATH_EPSILON ? 1 - fDifLeft / fFinalDif : 1.0f );

	if( !CalculateNewPosRot( vNewPos, rNewRot, m_vOnPos, m_fPowerOnTime, fPercent, LTTRUE ) && !(m_dwPropFlags & AWM_PROP_FORCEMOVE) )
	{
		// Restore our angles...

		m_fPitch	= vOldAngles.x;
		m_fYaw		= vOldAngles.y;
		m_fRoll		= vOldAngles.z;

		m_fMoveStartTm	+= g_pLTServer->GetFrameTime();
		m_fLastTime		= fCurTime;
		m_bUpdateSpin	= LTFALSE;

		return;
	}

	g_pLTServer->MoveObject( m_hObject, &vNewPos );

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
		m_bUpdateSpin	= LTFALSE;

		return;
	}

	g_pLTServer->RotateObject( m_hObject, &rNewRot );

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

void SpinningWorldModel::UpdatePowerOn( const LTFLOAT &fCurTime )
{
	LTBOOL	bDoneInX = LTFALSE;
	LTBOOL	bDoneInY = LTFALSE;
	LTBOOL	bDoneInZ = LTFALSE;
	LTFLOAT	fPercent;
	LTFLOAT	fRate;
	LTFLOAT	fFrameTm = g_pLTServer->GetFrameTime();
	
	fPercent = (fCurTime - m_fMoveStartTm) / m_fPowerOnTime;

	if( m_bUpdateSpin )
	{
		// Update Pitch PowerUP...

		if( m_vFinalVelocity.x )
		{
			fRate = m_vFinalVelocity.x / m_fPowerOnTime;
					
			m_vVelocity.x += GetWaveformValue( fRate, fPercent ) * fFrameTm * m_vRotateDir.x;

			if( m_vVelocity.x >= m_vFinalVelocity.x )
			{
				m_vVelocity.x = m_vFinalVelocity.x;
				bDoneInX = LTTRUE;
			}
		}
		else
		{
			bDoneInX = LTTRUE;
		}

		// Update Yaw PowerUP...

		if( m_vFinalVelocity.y )
		{
			fRate = m_vFinalVelocity.y / m_fPowerOnTime;
			
			m_vVelocity.y += GetWaveformValue( fRate, fPercent ) * fFrameTm * m_vRotateDir.y;

			if( m_vVelocity.y >= m_vFinalVelocity.y )
			{
				m_vVelocity.y = m_vFinalVelocity.y;
				bDoneInY = LTTRUE;
			}
		}
		else
		{
			bDoneInY = LTTRUE;
		}

		// Update Roll PowerUP...

		if( m_vFinalVelocity.z )
		{
			fRate = m_vFinalVelocity.z / m_fPowerOnTime;

			m_vVelocity.z += GetWaveformValue( fRate, fPercent ) * fFrameTm * m_vRotateDir.z;

			if( m_vVelocity.z >= m_vFinalVelocity.z )
			{
				m_vVelocity.z = m_vFinalVelocity.z;
				bDoneInZ = LTTRUE;
			}
		}
		else
		{
			bDoneInZ = LTTRUE;
		}
	}

	if( fPercent > 1.0f )
	{
		bDoneInX = LTTRUE;
		bDoneInY = LTTRUE;
		bDoneInZ = LTTRUE;

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

void SpinningWorldModel::UpdateOff( const LTFLOAT &fCurTime )
{

}



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SpinningWorldModel::UpdatePowerOff
//
//  PURPOSE:	Handel updating the PowerOff state
//
// ----------------------------------------------------------------------- //

void SpinningWorldModel::UpdatePowerOff( const LTFLOAT &fCurTime )
{
	LTBOOL	bDoneInX = LTFALSE;
	LTBOOL	bDoneInY = LTFALSE;
	LTBOOL	bDoneInZ = LTFALSE;
	LTFLOAT	fPercent;
	LTFLOAT	fRate;
	LTFLOAT	fFrameTm = g_pLTServer->GetFrameTime();
	
	fPercent = (fCurTime - m_fMoveStartTm) / m_fPowerOffTime;
	if( fPercent > 1.0f )
	{
		bDoneInX = LTTRUE;
		bDoneInY = LTTRUE;
		bDoneInZ = LTTRUE;

		m_vVelocity = LTVector( 0.0f, 0.0f, 0.0f );
	}
	
	if( m_bUpdateSpin )
	{
		// Update Pitch PowerDOWN...

		if( m_vFinalVelocity.x  )
		{
			fRate = m_vFinalVelocity.x / m_fPowerOffTime;
			
			m_vVelocity.x -= GetWaveformValue( fRate, fPercent ) * fFrameTm * m_vRotateDir.x ;

			if( m_vVelocity.x < MATH_EPSILON )
			{
				m_vVelocity.x = 0.0f;
				bDoneInX = LTTRUE;
			}
		}
		else
		{
			bDoneInX = LTTRUE;
		}

		// Update Yaw PowerDOWN...

		if( m_vFinalVelocity.y )
		{
			fRate = m_vFinalVelocity.y / m_fPowerOffTime;
			
			m_vVelocity.y -= GetWaveformValue( fRate, fPercent ) * fFrameTm * m_vRotateDir.y;

			if( m_vVelocity.y < MATH_EPSILON )
			{
				m_vVelocity.y = 0.0f;
				bDoneInY = LTTRUE;
			}
		}
		else
		{
			bDoneInY = LTTRUE;
		}

		// Update Roll PowerDOWN...

		if( m_vFinalVelocity.z )
		{
			fRate = m_vFinalVelocity.z / m_fPowerOffTime;
			
			m_vVelocity.z -= GetWaveformValue( fRate, fPercent ) * fFrameTm * m_vRotateDir.z;

			if( m_vVelocity.z < MATH_EPSILON )
			{
				m_vVelocity.z = 0.0f;
				bDoneInZ = LTTRUE;
			}
		}
		else
		{
			bDoneInZ = LTTRUE;
		}
	}

	// Let the "on" update do the actual rotating

	UpdateOn( fCurTime );

	if( bDoneInX && bDoneInY && bDoneInZ )
	{
		SetOff();
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
	SAVE_BYTE( m_bUpdateSpin );
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
	LOAD_BYTE( m_bUpdateSpin );
	LOAD_TIME( m_fLastTime );
}