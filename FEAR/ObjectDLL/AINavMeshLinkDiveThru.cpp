// ----------------------------------------------------------------------- //
//
// MODULE  : AINavMeshLinkDiveThru.cpp
//
// PURPOSE : AI NavMesh Link DiveThru class implementation.
//
// CREATED : 08/28/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AINavMeshLinkDiveThru.h"
#include "AI.h"
#include "AIPathMgrNavMesh.h"


// WorldEdit

LINKFROM_MODULE( AINavMeshLinkDiveThru );

BEGIN_CLASS( AINavMeshLinkDiveThru )
	ADD_STRINGPROP_FLAG(SmartObject,	"DiveThru",			0|PF_STATICLIST, "SmartObject used to specify animations for traversing the link")
	ADD_STRINGPROP_FLAG(Object,					"",			0|PF_OBJECTLINK, "The name of the object that the AI is to activate.")
END_CLASS_FLAGS_PLUGIN( AINavMeshLinkDiveThru, AINavMeshLinkAbstractOneAnim, 0, AINavMeshLinkDiveThruPlugin, "This link is used to specify that the AI must play a dive animation to traverse the associated brush" )

CMDMGR_BEGIN_REGISTER_CLASS( AINavMeshLinkDiveThru )
CMDMGR_END_REGISTER_CLASS( AINavMeshLinkDiveThru, AINavMeshLinkAbstractOneAnim )

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkDiveThru::Constructor
//              
//	PURPOSE:	Constructor
//              
//----------------------------------------------------------------------------

AINavMeshLinkDiveThru::AINavMeshLinkDiveThru()
{
	m_hAnimObject = NULL;
}

void AINavMeshLinkDiveThru::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_STDSTRING(m_strObject);
	SAVE_HOBJECT(m_hAnimObject);
}

void AINavMeshLinkDiveThru::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_STDSTRING(m_strObject);
	LOAD_HOBJECT(m_hAnimObject);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkDiveThru::ReadProp
//              
//	PURPOSE:	Read properties from WorldEdit.
//              
//----------------------------------------------------------------------------

void AINavMeshLinkDiveThru::ReadProp(const GenericPropList *pProps)
{
	super::ReadProp( pProps );

	// Read the object to activate.

	const char* pszPropString = pProps->GetString( "Object", "" );
	if ( pszPropString[0] )
	{
		m_strObject = pszPropString;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkDiveThru::InitialUpdate
//              
//	PURPOSE:	Add the NavMeshLink to the NavMesh.
//              
//----------------------------------------------------------------------------

void AINavMeshLinkDiveThru::InitialUpdate()
{
	super::InitialUpdate();

	// Cache a pointer to the animation object.

	if( !m_strObject.empty() )
	{
		HOBJECT hObj = NULL;
		FindNamedObject( m_strObject.c_str(), hObj, false );
		m_hAnimObject = hObj;
		m_strObject.clear();
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkDiveThru::ActivateTraversal
//              
//	PURPOSE:	Setup AI to traverse the link.
//              
//----------------------------------------------------------------------------

void AINavMeshLinkDiveThru::ActivateTraversal( CAI* pAI, CAIStateUseSmartObject* pStateUseSmartObject )
{
	super::ActivateTraversal( pAI, pStateUseSmartObject );

	// Sanity check.

	if( !pAI )
	{
		return;
	}
	
	// Set the object to animate.

	pAI->SetAnimObject( m_hAnimObject );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkDiveThru::ApplyTraversalEffect
//              
//	PURPOSE:	Apply changes when finishing traversing the link.
//              
//----------------------------------------------------------------------------

void AINavMeshLinkDiveThru::ApplyTraversalEffect( CAI* pAI )
{
	super::ApplyTraversalEffect( pAI );

	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Clear the object to animate.

	pAI->SetAnimObject( NULL );
}

