// ----------------------------------------------------------------------- //
//
// MODULE  : TransitionAggregate.cpp
//
// PURPOSE : The TransitionAggregate implementation
//
// CREATED : 12/03/01
//
// (c) 2001-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "Stdafx.h"
	#include "MsgIDs.h"
	#include "PlayerObj.h"
	#include "TransitionArea.h"
	#include "TransitionAggregate.h"
	#include "TransitionMgr.h"


CMDMGR_BEGIN_REGISTER_CLASS( CTransitionAggregate )
CMDMGR_END_REGISTER_CLASS( CTransitionAggregate, IAggregate )


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTransitionAggregate::CTransitionAggregate
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CTransitionAggregate::CTransitionAggregate( )
:	IAggregate		( "CTransitionAggregate" ),
	m_hObject		( NULL )
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

uint32 CTransitionAggregate::EngineMessageFn( LPBASECLASS pObject, uint32 messageID, void *pData, float fData )
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

	LTRigidTransform tfLocal;
	LTRigidTransform tfObjectWorld;
	LTRigidTransform const& tfTransAreaWorld = pTransArea->GetWorldTransform( );
	LTRotation rInverseRot = ~tfTransAreaWorld.m_rRot;

	g_pLTServer->GetObjectPos( m_hObject, &tfObjectWorld.m_vPos );
	g_pLTServer->GetObjectRotation( m_hObject, &tfObjectWorld.m_rRot );
	LTVector vVel;
	g_pPhysicsLT->GetVelocity( m_hObject, &vVel );

	tfLocal.m_vPos = rInverseRot * ( tfObjectWorld.m_vPos - tfTransAreaWorld.m_vPos );
	tfLocal.m_rRot = tfObjectWorld.m_rRot * ~tfTransAreaWorld.m_rRot;
	LTVector vRelVel = rInverseRot * vVel;

	SAVE_VECTOR( tfLocal.m_vPos );
	SAVE_ROTATION( tfLocal.m_rRot );
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

	LTRigidTransform tfLocal;
	LTRigidTransform tfObjectWorld;
	LTVector vVelRel;
	LTRigidTransform const& tfTransAreaWorld = pTransArea->GetWorldTransform( );

	LOAD_VECTOR( tfLocal.m_vPos );
	LOAD_ROTATION( tfLocal.m_rRot );
	LOAD_VECTOR( vVelRel );
	
	// Calc pos and rot based on offsets and current TransArea...

	tfObjectWorld.m_vPos = tfTransAreaWorld.m_vPos + ( tfTransAreaWorld.m_rRot * tfLocal.m_vPos );
	tfObjectWorld.m_rRot = tfTransAreaWorld.m_rRot * tfLocal.m_rRot;
	LTVector vVel = tfTransAreaWorld.m_rRot * vVelRel;

	if( IsPlayer( m_hObject ))
	{
		// Since the PlayerObj is controlled by the client we need to notify the
		// client of the rotation.  We are only worried about the Yaw since Roll doesn't 
		// matter and Pitch can be preserved on the client.


		CPlayerObj *pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject( m_hObject );
		if( !pPlayer ) return;

		
		float		fYaw;
		LTVector	vF = tfObjectWorld.m_rRot.Forward();

		// We don't care about Roll...

		vF.y = 0.0f; 
		vF.Normalize();

		// Yaw = arctan( vF.x / vF.z );
		// atan2 is well defined even for vF.z == 0

		fYaw = (float)atan2( vF.x, vF.z );

		pPlayer->TeleportClientToServerPos( true );
	}

	g_pLTServer->SetObjectTransform( m_hObject,tfObjectWorld );
	g_pPhysicsLT->SetVelocity( m_hObject, vVel );
}
