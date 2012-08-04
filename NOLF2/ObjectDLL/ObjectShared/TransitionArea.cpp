// ----------------------------------------------------------------------- //
//
// MODULE  : TransitionArea.cpp
//
// PURPOSE : The TransitionArea implementation
//
// CREATED : 11/27/01
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "stdafx.h"
	#include "ObjectMsgs.h"
	#include "ParsedMsg.h"
	#include "ContainerCodes.h"
	#include "TransitionArea.h"
	#include "TransitionMgr.h"

LINKFROM_MODULE( TransitionArea );

BEGIN_CLASS( TransitionArea )

	ADD_LONGINTPROP_FLAG( TransitionToLevel, -1, 0 )

END_CLASS_DEFAULT_FLAGS( TransitionArea, GameBase, NULL, NULL, CF_WORLDMODEL )


CMDMGR_BEGIN_REGISTER_CLASS( TransitionArea )
	
	CMDMGR_ADD_MSG( TRANSITION, 1, NULL, "TRANSITION" )

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
	m_tfWorld.m_Scale.Init( 1.0f, 1.0f, 1.0f );
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

uint32 TransitionArea::EngineMessageFn( uint32 messageID, void *pData, LTFLOAT fData )
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
					m_tfWorld.m_Rot = pOCS->m_Rotation;
					m_tfWorld.m_Pos = pOCS->m_Pos;
					pOCS->m_Rotation.Identity();

					ReadProps( pOCS );
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

void TransitionArea::ReadProps( ObjectCreateStruct *pOCS )
{
	ASSERT( pOCS != LTNULL );

	GenericProp	gProp;

	if( g_pLTServer->GetPropGeneric( "TransitionToLevel", &gProp ) == LT_OK )
	{
		m_nTransLevel = gProp.m_Long;
	}
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
	ASSERT( pOCS != LTNULL );

	pOCS->m_Flags			|= m_dwFlags;
	pOCS->m_ObjectType		= OT_CONTAINER;
	pOCS->m_ContainerCode	= CC_TRANSITION_AREA;

	SAFE_STRCPY( pOCS->m_Filename, pOCS->m_Name );
	pOCS->m_SkinName[0] = '\0';
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	TransitionArea::OnTrigger
//
//  PURPOSE:	Handle trigger messages 
//
// ----------------------------------------------------------------------- //

bool TransitionArea::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Transition("TRANSITION");

	if( cMsg.GetArg(0) == s_cTok_Transition )
	{
		// Do the transition in our next update.  We need to do
		// in update rather than trigger processing because the object
		// that triggered us is probably the player.  If the player
		// is moving, he won't be in the transitionarea at the
		// time of the trigger and will be placed back in the transitionarea
		// at the end of his movement.
		m_bRequestTransition = true;
		SetNextUpdate( UPDATE_NEXT_FRAME );
	}
	else
		return GameBase::OnTrigger(hSender, cMsg);

	return true;
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
	m_tfWorld.m_Rot.ConvertToMatrix( mat );

	vDims = mat * vDims;

	vDims.x = (LTFLOAT)fabs( vDims.x );
	vDims.y = (LTFLOAT)fabs( vDims.y );
	vDims.z = (LTFLOAT)fabs( vDims.z );

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
	
	SAVE_ROTATION( m_tfWorld.m_Rot );
	SAVE_VECTOR( m_tfWorld.m_Pos );
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

	LOAD_ROTATION( m_tfWorld.m_Rot );
	LOAD_VECTOR( m_tfWorld.m_Pos );
}