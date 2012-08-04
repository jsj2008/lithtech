// ----------------------------------------------------------------------- //
//
// MODULE  : LeanNodeController.cpp
//
// PURPOSE : Implementation of class used to manage the player lean
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "LeanNodeController.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLeanNodeController::CLeanNodeController
//
//	PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CLeanNodeController::CLeanNodeController( )
:	m_fLeanAngle	( 0.0f )
{
	m_hObject.SetReceiver( *this );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLeanNodeController::~CLeanNodeController
//
//	PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CLeanNodeController::~CLeanNodeController( )
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLeanNodeController::~CLeanNodeController
//
//	PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

void CLeanNodeController::SetLeanAngle( float fLean )
{
	fLean = LTCLAMP( fLean, -MATH_PI, MATH_PI );
	m_fLeanAngle = fLean;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLeanNodeController::Init
//
//	PURPOSE:	Initialize the node controller...
//
// ----------------------------------------------------------------------- //

bool CLeanNodeController::Init( HOBJECT hObject, HRECORD hLeanRecord )
{
	if( !hObject || !hLeanRecord )
		return false;

	// Make sure all nodes are cleared before initializing...
	UnRegisterNodes( m_hObject );

	m_hObject = hObject;

	// Allocate the number of nodes specified to be controlled...
	uint32 nNumLeanNodes = ModelsDB::Instance( ).GetLeanNumNodes( hLeanRecord );
	m_aLeanNodes.resize( nNumLeanNodes );

	// Run through each node and add the node control function that will modify it...
	for( uint32 nNode = 0; nNode < nNumLeanNodes; ++nNode )
	{
		const char *pszNodeName = ModelsDB::Instance( ).GetLeanNodeName( hLeanRecord, nNode );
		if( !pszNodeName || !pszNodeName[0] )
			continue;

		if( g_pLTBase->GetModelLT()->GetNode( m_hObject, pszNodeName, m_aLeanNodes[nNode].m_hNode ) == LT_OK )
		{
			// The node is valid on the model so set it up to be controlled...

			m_aLeanNodes[nNode].m_pLeanNodeController = this;
			m_aLeanNodes[nNode].m_fWeight = ModelsDB::Instance( ).GetLeanNodeWeight( hLeanRecord, nNode );

			g_pLTBase->GetModelLT()->AddNodeControlFn( m_hObject, m_aLeanNodes[nNode].m_hNode, LeanNodeControler, &m_aLeanNodes[nNode] );
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CLeanNodeController::UnRegisterNodes
//
//  PURPOSE:	Remove all node control functions and clear the list of lean nodes...
//
// ----------------------------------------------------------------------- //

void CLeanNodeController::UnRegisterNodes( HOBJECT hObject )
{
	if( !hObject )
		return;

	uint32 nNumNodes = m_aLeanNodes.size();
	for( uint32 nNode = 0; nNode < nNumNodes; ++nNode )
	{
		CLeanNode *pLeanNode = &m_aLeanNodes[nNode];
		
		if( pLeanNode->m_hNode != INVALID_MODEL_NODE )
		{
			g_pLTBase->GetModelLT()->RemoveNodeControlFn( hObject, pLeanNode->m_hNode, LeanNodeControler, &m_aLeanNodes[nNode] );
		}
	}

	m_aLeanNodes.resize( 0 );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CLeanNodeController::OnLinkBroken
//
//  PURPOSE:	Implementing classes will have this function called
//				when HOBJECT ref points to gets deleted.
//
// ----------------------------------------------------------------------- //

void CLeanNodeController::OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj )
{
	if ( &m_hObject == pRef )
	{
		UnRegisterNodes( hObj );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CLeanNodeController::LeanNodeControler
//
//  PURPOSE:	Node controller for handling updating the rotation of the node...
//
// ----------------------------------------------------------------------- //

void CLeanNodeController::LeanNodeControler( const NodeControlData& iData, void *pUserData )
{
	LTASSERT( pUserData, "Invalid User Data!" );

    CLeanNode *pLeanNode = (CLeanNode*)pUserData;
	CLeanNodeController *pLeanNodeController = pLeanNode->m_pLeanNodeController;

	// Don't modify anything if not actually leaning...
	if( pLeanNodeController->IsLeaning( ))
	{
		// Factor in the amount of roll to apply to the node based on the current lean angle and node weight...
		float fRoll = pLeanNodeController->m_fLeanAngle * pLeanNode->m_fWeight;
		iData.m_pNodeTransform->m_rRot = (LTRotation( 0.0f, 0.0f, fRoll) * iData.m_pNodeTransform->m_rRot);
	}
}

// EOF
