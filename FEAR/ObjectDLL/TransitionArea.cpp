// ----------------------------------------------------------------------- //
//
// MODULE  : TransitionArea.cpp
//
// PURPOSE : The TransitionArea implementation
//
// CREATED : 11/27/01
//
// (c) 2001-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "Stdafx.h"
	#include "ObjectMsgs.h"
	#include "ParsedMsg.h"
	#include "ContainerCodes.h"
	#include "TransitionArea.h"
	#include "TransitionMgr.h"

LINKFROM_MODULE( TransitionArea );

#if defined ( PROJECT_DARK )

	#define CF_HIDDEN_TRANSITIONAREA CF_HIDDEN

#elif defined ( PROJECT_FEAR )

	// JSC this has to be set to a value or else the '|' operator will be 
	//  missing an operand.
	#define CF_HIDDEN_TRANSITIONAREA CF_HIDDEN

#endif

BEGIN_CLASS( TransitionArea )

	ADD_LONGINTPROP_FLAG( TransitionToLevel, -1, 0, "TODO:PROPDESC" )

END_CLASS_FLAGS( TransitionArea, GameBase, CF_HIDDEN_TRANSITIONAREA|CF_WORLDMODEL, "Defines an area that chains to another level. Objects within the area are carried over to the new level." )


CMDMGR_BEGIN_REGISTER_CLASS( TransitionArea )
	
	ADD_MESSAGE( TRANSITION, 1, NULL, MSG_HANDLER( TransitionArea, HandleTransitionMsg ), "TRANSITION", "activates the transition volume", "msg TransitionArea TRANSITION" )

CMDMGR_END_REGISTER_CLASS( TransitionArea, GameBase )



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	TransitionArea::TransitionArea
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

TransitionArea::TransitionArea( )
:	GameBase		( OT_CONTAINER ),
	m_dwFlags		( FLAG_CONTAINER | FLAG_GOTHRUWORLD | FLAG_FORCECLIENTUPDATE ),
	m_nTransLevel	( -1 )
{
	m_bRequestTransition = false;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	TransitionArea::~TransitionArea
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

TransitionArea::~TransitionArea( )
{

}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	TransitionArea::EngineMessageFn
//
//  PURPOSE:	Handle messages from the engine...
//
// ----------------------------------------------------------------------- //

uint32 TransitionArea::EngineMessageFn( uint32 messageID, void *pData, float fData )
{
	switch( messageID )
	{
		case MID_PRECREATE :
		{
			// Let the GameBase handle the message first

			uint32 dwRet = GameBase::EngineMessageFn( messageID, pData, fData );

			ObjectCreateStruct	*pOCS = (ObjectCreateStruct*)pData;

			if( pOCS )
			{
				if( PRECREATE_WORLDFILE == fData )
				{
					// [RP] HACK - Deal with the double rotation problem
					m_tfWorld.m_rRot = pOCS->m_Rotation;
					m_tfWorld.m_vPos = pOCS->m_Pos;
					pOCS->m_Rotation.Identity();

					ReadProps( &pOCS->m_cProperties );
				}

				PostReadProps( pOCS );
			}

			// Important!! - We already sent the message to the GameBase so DONT do it again.

			return dwRet;

		}
		break;

		case MID_INITIALUPDATE :
		{
			// Don't eat ticks please...
			SetNextUpdate(UPDATE_NEVER);
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

		case MID_UPDATE:
		{
			Update( );
		}
		break;

		default : break;
	}

	return GameBase::EngineMessageFn( messageID, pData, fData );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	TransitionArea::ReadProps
//
//  PURPOSE:	Get the property values...
//
// ----------------------------------------------------------------------- //

void TransitionArea::ReadProps( const GenericPropList *pProps )
{
	ASSERT( pProps != NULL );

	m_nTransLevel = pProps->GetLongInt( "TransitionToLevel", m_nTransLevel );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	TransitionArea::PostReadProps
//
//  PURPOSE:	Set some final values. 
//
// ----------------------------------------------------------------------- //

void TransitionArea::PostReadProps( ObjectCreateStruct *pOCS )
{
	ASSERT( pOCS != NULL );

	pOCS->m_Flags			|= m_dwFlags;
	pOCS->m_ObjectType		= OT_CONTAINER;
	pOCS->m_ContainerCode	= CC_TRANSITION_AREA;

	pOCS->SetFileName(pOCS->m_Name );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	TransitionArea::HandleTransitionMsg
//
//  PURPOSE:	Handle a TRANSITION message...
//
// ----------------------------------------------------------------------- //

void TransitionArea::HandleTransitionMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{

	// [RP] 8/09/03 - Every command is now queued rather than immediately processed.
	//		The player should not be moving when this message is handled so transitioning
	//		now should be ok.  Invistigate further to determine if this is really necessary?  

	// Do the transition in our next update.  We need to do
	// in update rather than trigger processing because the object
	// that triggered us is probably the player.  If the player
	// is moving, he won't be in the transitionarea at the
	// time of the trigger and will be placed back in the transitionarea
	// at the end of his movement.
	m_bRequestTransition = true;
	SetNextUpdate( UPDATE_NEXT_FRAME );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	TransitionArea::Update
//
//  PURPOSE:	Handle our update.
//
// ----------------------------------------------------------------------- //

void TransitionArea::Update( )
{
	// See if we should do a transition.
	if( m_bRequestTransition )
	{
		// Clear the request.
		m_bRequestTransition = false;
		HandleTransition( );
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	TransitionArea::HandleTransition
//
//  PURPOSE:	Do the actually transitioning...
//
// ----------------------------------------------------------------------- //

void TransitionArea::HandleTransition( )
{
	g_pTransMgr->TransitionFromArea( m_hObject );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	TransitionArea::GetDims
//
//  PURPOSE:	Because of the WorldModel double rotation problem we need to manually calc dims
//
// ----------------------------------------------------------------------- //

LTVector TransitionArea::GetDims( )
{
	// "Localize" the dims...
	
	LTVector vDims;
	g_pPhysicsLT->GetObjectDims( m_hObject, &vDims );

	LTMatrix mat;
	m_tfWorld.m_rRot.ConvertToMatrix( mat );

	vDims = mat * vDims;

	vDims.x = (float)fabs( vDims.x );
	vDims.y = (float)fabs( vDims.y );
	vDims.z = (float)fabs( vDims.z );

	return vDims;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	TransitionArea::Save
//
//  PURPOSE:	Save the object...
//
// ----------------------------------------------------------------------- //

void TransitionArea::Save( ILTMessage_Write *pMsg, uint32 dwSaveFlags )
{
	if( !pMsg ) return;

	SAVE_DWORD( m_dwFlags );
	SAVE_INT( m_nTransLevel );
	
	SAVE_ROTATION( m_tfWorld.m_rRot );
	SAVE_VECTOR( m_tfWorld.m_vPos );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	TransitionArea::Load
//
//  PURPOSE:	Load the object...
//
// ----------------------------------------------------------------------- //

void TransitionArea::Load( ILTMessage_Read *pMsg, uint32 dwSaveFlags )
{
	if( !pMsg ) return;

	LOAD_DWORD( m_dwFlags );
	LOAD_INT( m_nTransLevel );

	LOAD_ROTATION( m_tfWorld.m_rRot );
	LOAD_VECTOR( m_tfWorld.m_vPos );
}
