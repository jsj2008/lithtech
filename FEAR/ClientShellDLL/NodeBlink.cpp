// ----------------------------------------------------------------------- //
//
// MODULE  : NodeBlink.cpp
//
// PURPOSE : Blink Node Controller implementation
//
// CREATED : 1/30/04
//
// (c) 1998-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include <stdafx.h>
#include "NodeBlink.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BlinkController::BlinkController
//
//	PURPOSE:	Constructor initializes the BlinkController to an intert 
//				state.
//
// ----------------------------------------------------------------------- //

BlinkController::BlinkController() : 
	m_bInBlink(false),
	m_fMinTimeBetweenBlinks(2.f),
	m_fMaxTimeBetweenBlinks(4.f),
	m_flBlinkDuration(0.15f),
	m_hObject(NULL)
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BlinkController::~BlinkController
//
//	PURPOSE:	Destructor handles unhooking any remaining nodes, and 
//				freeing allocated assets.
//
// ----------------------------------------------------------------------- //

BlinkController::~BlinkController()
{
	UnbindNodes();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BlinkController::UpdateNodeBlink
//
//	PURPOSE:	UpdateNodeBlink is the general update function which must 
//				be called to update the state of the blink.  This propagates
//				into the individual BlinkContexts the current ammount of 
//				interpolation to apply.
//
// ----------------------------------------------------------------------- //

void BlinkController::UpdateNodeBlink()
{
	if( m_BlinkNodeContextList.size( ) == 0 )
		return;

	if (m_bInBlink)
	{
		// Handle updating the blink

		if ( m_EndBlinkTimer.IsTimedOut())
		{
			// Stop the blink if it is complete.

			m_bInBlink = false;
			m_NextBlinkTimer.Start( GetRandom( m_fMinTimeBetweenBlinks, m_fMaxTimeBetweenBlinks ));
		}
		else
		{
			// Update the blink

			float flPercent = ( float )(m_EndBlinkTimer.GetElapseTime() / m_EndBlinkTimer.GetDuration());
			flPercent = Clamp(flPercent, 0.f, 1.f);

			// Update the nodes percentages (this could be done by allowing 
			// the NodeContext to store a backpointer to the context, and to 
			// get the percentage from it, but this avoids the circular 
			// dependency.

			for (BlinkNodeContextList::iterator it = m_BlinkNodeContextList.begin(); it != m_BlinkNodeContextList.end(); ++it)
			{
				(*it)->m_flPercent = flPercent;
			}
		}
	}
	else
	{
		// Determine if it is time to start a blink

		if (m_NextBlinkTimer.IsTimedOut())
		{
			m_bInBlink = true;
			m_EndBlinkTimer.Start( m_flBlinkDuration );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BlinkController::Init
//
//	PURPOSE:	Handles initializing the BlinkController given an object 
//				and a blink group.  Clears an existing node functions, and
//				sets up the new node functions.
//
// ----------------------------------------------------------------------- //

void BlinkController::Init(HOBJECT hObj, ModelsDB::HBLINKNODEGROUP hBlinkGroup)
{
	// Clean up any existing bindings.
	
	UnbindNodes();

	// Set up the new bindings.

	m_hObject = hObj;
 
	if( !hBlinkGroup )
		return;

	ObjectContextTimer engineTimer = m_hObject;
	m_NextBlinkTimer.SetEngineTimer( engineTimer );
	m_EndBlinkTimer.SetEngineTimer( engineTimer );

	// Read the shared information

	m_flBlinkDuration = g_pModelsDB->GetBlinkDuration(hBlinkGroup);
	LTVector2 vFrequency = g_pModelsDB->GetBlinkFrequency(hBlinkGroup);
	m_fMinTimeBetweenBlinks = vFrequency.x;
	m_fMaxTimeBetweenBlinks = vFrequency.y;

	// Read the per node information

	int nBlinkNodes = g_pModelsDB->GetNumBlinkNodes(hBlinkGroup);
	for (int iNode = 0; iNode < nBlinkNodes; ++iNode)
	{
		const char* pszNodeName = g_pModelsDB->GetBlinkNodeName(hBlinkGroup, iNode);
		if (pszNodeName)
		{
			HMODELNODE hModelNode = NULL;
			if (LT_OK == g_pLTBase->GetModelLT()->GetNode(hObj, pszNodeName, hModelNode))
			{
				LTVector vAxis = g_pModelsDB->GetBlinkNodeAxis(hBlinkGroup, iNode);
				float flAngle = g_pModelsDB->GetBlinkNodeAngle(hBlinkGroup, iNode);

				// Create a context to hold per node information, and then call 
				
				BlinkNodeContext* pNodeContext = debug_new(BlinkNodeContext);
				pNodeContext->m_hNode = hModelNode;
				pNodeContext->m_rFinal.Rotate(vAxis, flAngle);
				m_BlinkNodeContextList.push_back(pNodeContext);
				
				// Add the control function

				g_pLTBase->GetModelLT()->AddNodeControlFn(hObj, hModelNode, BlinkControllerCB, (void*)pNodeContext);
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BlinkController::UnbindNodes
//
//	PURPOSE:	Unbinds all of the nodes, and frees the node contexts.
//
// ----------------------------------------------------------------------- //

void BlinkController::UnbindNodes()
{
	for (BlinkNodeContextList::iterator it = m_BlinkNodeContextList.begin(); it != m_BlinkNodeContextList.end(); ++it)
	{
		// Unhook the node control function, then delete the context.
		BlinkNodeContext* pBlinkNodeContext = *it;

		if (m_hObject)
		{
			g_pLTBase->GetModelLT()->RemoveNodeControlFn(m_hObject, pBlinkNodeContext->m_hNode, BlinkControllerCB, NULL);
		}

		debug_delete(pBlinkNodeContext);
	}
	m_BlinkNodeContextList.clear();	
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BlinkController::BlinkControllerCB
//
//	PURPOSE:	Callback function called per blink node to adjust the 
//				nodes transformation.
//
// ----------------------------------------------------------------------- //

void BlinkController::BlinkControllerCB(const NodeControlData& NodeData, void* pUserData)
{
	LTASSERT(pUserData, "Invalid user data encountered");

	if (!pUserData)
	{
		return;
	}

	// User data is just a BlinkNodeContext
	BlinkNodeContext* pContext = (BlinkNodeContext*)pUserData;
	
	// Determine the interpolation weight, and interpolate from the current 
	// position towards the final position.
	float flInterpolationWeight = 1.0f - (4.0f*(pContext->m_flPercent - 0.5f)*(pContext->m_flPercent - 0.5f));
	LTRotation rFinalPosInLocalSpace = NodeData.m_pNodeTransform->m_rRot * pContext->m_rFinal;
	NodeData.m_pNodeTransform->m_rRot.Slerp( NodeData.m_pNodeTransform->m_rRot, rFinalPosInLocalSpace, flInterpolationWeight);
}
