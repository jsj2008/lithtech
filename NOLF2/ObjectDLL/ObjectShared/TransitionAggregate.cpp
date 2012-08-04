// ----------------------------------------------------------------------- //
//
// MODULE  : TransitionAggregate.cpp
//
// PURPOSE : The TransitionAggregate implementation
//
// CREATED : 12/03/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "stdafx.h"
	#include "MsgIds.h"
	#include "PlayerObj.h"
	#include "TransitionArea.h"
	#include "TransitionAggregate.h"
	#include "TransitionMgr.h"



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTransitionAggregate::CTransitionAggregate
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CTransitionAggregate::CTransitionAggregate( )
:	IAggregate		( ),
	m_hObject		( LTNULL )
{

}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTransitionAggregate::~CTransitionAggregate
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CTransitionAggregate::~CTransitionAggregate( )
{

}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTransitionAggregate::EngineMessageFn
//
//  PURPOSE:	Handle messages from the engine...
//
// ----------------------------------------------------------------------- //

uint32 CTransitionAggregate::EngineMessageFn( LPBASECLASS pObject, uint32 messageID, void *pData, LTFLOAT fData )
{
	switch( messageID )
	{
		case MID_OBJECTCREATED :
		{
			if( fData != OBJECTCREATED_SAVEGAME )
			{
				ObjectCreated( pObject );
			}
		}
		break;

		case MID_SAVEOBJECT :
		{
			Save( (ILTMessage_Write*)pData, (uint32)fData );
		}
		break;

		case MID_LOADOBJECT :
		{
			Load( (ILTMessage_Read*)pData, (uint32)fData );
		}
		break;
	}

	return IAggregate::EngineMessageFn( pObject, messageID, pData, fData );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTransitionAggregate::ObjectCreated
//
//  PURPOSE:	The object we are associated with has been created...
//
// ----------------------------------------------------------------------- //

void CTransitionAggregate::ObjectCreated( LPBASECLASS pObject )
{
	if( !pObject || !pObject->m_hObject ) return;
	
	// Assign the object associated with this aggregate...

	if( !m_hObject ) m_hObject = pObject->m_hObject;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTransitionAggregate::Save
//
//  PURPOSE:	Save the object...
//
// ----------------------------------------------------------------------- //

void CTransitionAggregate::Save( ILTMessage_Write *pMsg, uint32 dwSaveFlags )
{
	if( !pMsg ) return;

	SAVE_HOBJECT( m_hObject );

	// The rest is dependent on the save type...
	
	if( dwSaveFlags != LOAD_TRANSITION ) return;

	HOBJECT hTransArea = g_pTransMgr->GetTransitionArea();
	if( !hTransArea ) return;

	TransitionArea *pTransArea = (TransitionArea*)g_pLTServer->HandleToObject( hTransArea );
	if( !pTransArea ) return;

	LTransform tfLocal;
	LTransform tfObjectWorld;
	LTransform const& tfTransAreaWorld = pTransArea->GetWorldTransform( );
	LTMatrix mInverseRot;
	tfTransAreaWorld.m_Rot.ConvertToMatrix( mInverseRot );
	mInverseRot.Inverse( );

	g_pLTServer->GetObjectPos( m_hObject, &tfObjectWorld.m_Pos );
	g_pLTServer->GetObjectRotation( m_hObject, &tfObjectWorld.m_Rot );
	LTVector vVel;
	g_pPhysicsLT->GetVelocity( m_hObject, &vVel );

	tfLocal.m_Pos = mInverseRot * ( tfObjectWorld.m_Pos - tfTransAreaWorld.m_Pos );
	tfLocal.m_Rot = tfObjectWorld.m_Rot * ~tfTransAreaWorld.m_Rot;
	LTVector vRelVel = mInverseRot * vVel;

	SAVE_VECTOR( tfLocal.m_Pos );
	SAVE_ROTATION( tfLocal.m_Rot );
	SAVE_VECTOR( vRelVel );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTransitionAggregate::Load
//
//  PURPOSE:	Load the object...
//
// ----------------------------------------------------------------------- //

void CTransitionAggregate::Load( ILTMessage_Read *pMsg, uint32 dwLoadFlags )
{
	if( !pMsg ) return;
	
	LOAD_HOBJECT( m_hObject );

	// The rest is dependent on the load type...

	if( dwLoadFlags != LOAD_TRANSITION ) return;
	
	HOBJECT hTransArea = g_pTransMgr->GetTransitionArea();
	if( !hTransArea ) return;
	
	TransitionArea *pTransArea = (TransitionArea*)g_pLTServer->HandleToObject( hTransArea );
	if( !pTransArea ) return;

	LTransform tfLocal;
	LTransform tfObjectWorld;
	LTVector vVelRel;
	LTransform const& tfTransAreaWorld = pTransArea->GetWorldTransform( );
	LTMatrix mRotation;
	tfTransAreaWorld.m_Rot.ConvertToMatrix( mRotation );

	LOAD_VECTOR( tfLocal.m_Pos );
	LOAD_ROTATION( tfLocal.m_Rot );
	LOAD_VECTOR( vVelRel );
	
	// Calc pos and rot based on offsets and current TransArea...

	tfObjectWorld.m_Pos = tfTransAreaWorld.m_Pos + ( mRotation * tfLocal.m_Pos );
	tfObjectWorld.m_Rot = tfTransAreaWorld.m_Rot * tfLocal.m_Rot;
	LTVector vVel = mRotation * vVelRel;

	if( IsPlayer( m_hObject ))
	{
		// Since the PlayerObj is controlled by the client we need to notify the
		// client of the rotation.  We are only worried about the Yaw since Roll doesn't 
		// matter and Pitch can be preserved on the client.


		CPlayerObj *pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject( m_hObject );
		if( !pPlayer ) return;

		
		LTFLOAT		fYaw;
		LTVector	vF = tfObjectWorld.m_Rot.Forward();

		// We don't care about Roll...

		vF.y = 0.0f; 
		vF.Normalize();

		// Yaw = arctan( vF.x / vF.z );
		// atan2 is well defined even for vF.z == 0

		fYaw = (LTFLOAT)atan2( vF.x, vF.z );
		
		// Inform the client of the correct camera/player orientation...

		LTVector vVec( 0.0f, fYaw, 0.0f );
		
		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_PLAYER_ORIENTATION );
		cMsg.Writeuint8( MID_ORIENTATION_YAW );
		cMsg.WriteLTVector( vVec );
		g_pLTServer->SendToClient(cMsg.Read(), pPlayer->GetClient(), MESSAGE_GUARANTEED);
		
	}

	g_pLTServer->SetObjectPos( m_hObject, &tfObjectWorld.m_Pos );
	g_pLTServer->SetObjectRotation( m_hObject, &tfObjectWorld.m_Rot );
	g_pPhysicsLT->SetVelocity( m_hObject, &vVel );
}
