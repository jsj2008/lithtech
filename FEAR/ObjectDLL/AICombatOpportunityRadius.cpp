// ----------------------------------------------------------------------- //
//
// MODULE  : AICombatOpportunityRadius.cpp
//
// PURPOSE : 
//
// CREATED : 6/08/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AICombatOpportunityRadius.h"
#include "DebugLineSystem.h"

#if defined ( PROJECT_DARK )

	#define CF_HIDDEN_AICombatOpportunityRadius 0

#elif defined ( PROJECT_FEAR )

	#define CF_HIDDEN_AICombatOpportunityRadius CF_HIDDEN

#endif

BEGIN_CLASS(AICombatOpportunityRadius)
	ADD_REALPROP_FLAG(InnerRadius,			0.0f,		0|PF_RADIUS, " [WorldEdit units]")
	ADD_REALPROP_FLAG(OuterRadius,			1024.0f,	0|PF_RADIUS, " [WorldEdit units]")
END_CLASS_FLAGS(AICombatOpportunityRadius, GameBase, CF_HIDDEN_AICombatOpportunityRadius, "A simple circular region used in either the EnemyArea or AIArea of an AICombatOpportunity")

CMDMGR_BEGIN_REGISTER_CLASS(AICombatOpportunityRadius)
	ADD_MESSAGE( DEBUG_DRAWRADIUS,	2,	NULL,	MSG_HANDLER( AICombatOpportunityRadius, HandleDebugDrawRadiusMsg ),	"DEBUG_DRAWRADIUS", "AICombatOpportunityRadius.", "msg CombatOpportunityRadius (DEBUG_DRAWRADIUS 1/0)" )
CMDMGR_END_REGISTER_CLASS(AICombatOpportunityRadius, GameBase)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AICombatOpportunityRadius::HandleToObject
//
//	PURPOSE:	This utility function handles casting an HOBJECT to an 
//				AICombatOpportunityRadius.  In debug, the function asserts 
//				if the cast if the object is not of the expected type.  In 
//				release, no checks are performed to avoid performance 
//				overhead.  This mimics the behavior of the AINode version
//				of this function.
//
// ----------------------------------------------------------------------- //

AICombatOpportunityRadius* AICombatOpportunityRadius::HandleToObject(HOBJECT hObject)
{
	LTASSERT(IsKindOf(hObject, "AICombatOpportunityRadius"), "AICombatOpportunityRadius::HandleToObject : Object is being cast to invalid type.");
	return (AICombatOpportunityRadius*)g_pLTServer->HandleToObject(hObject);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AICombatOpportunityRadius::Con/destructor
//
//	PURPOSE:	Construct the object into an inert state.
//
// ----------------------------------------------------------------------- //

AICombatOpportunityRadius::AICombatOpportunityRadius() : 
	m_flOuterRadius(0.f)
	, m_flInnerRadius(0.f)
{
#ifndef _FINAL
	m_bDebugDraw = false;
#endif
}

AICombatOpportunityRadius::~AICombatOpportunityRadius()
{
#ifndef _FINAL
		DebugLineSystem& sys = LineSystem::GetSystem(this, "Range");
		sys.Clear();
#endif
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AICombatOpportunityRadius::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the AICombatOpportunityRadius
//              
//----------------------------------------------------------------------------

void AICombatOpportunityRadius::Load(ILTMessage_Read *pMsg)
{
	LOAD_FLOAT(m_flOuterRadius);
	LOAD_FLOAT(m_flInnerRadius);

	// Don't serialize:
	//m_bDebugDraw
}

void AICombatOpportunityRadius::Save(ILTMessage_Write *pMsg)
{
	SAVE_FLOAT(m_flOuterRadius);
	SAVE_FLOAT(m_flInnerRadius);

	// Don't serialize:
	//m_bDebugDraw
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	AICombatOpportunityRadius::EngineMessageFn
//
//  PURPOSE:	Handle messages from the engine.
//
// ----------------------------------------------------------------------- //

uint32 AICombatOpportunityRadius::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	uint32 nRet = super::EngineMessageFn(messageID, pData, fData);

	switch(messageID)
	{
        case MID_PRECREATE:
		{
			int nInfo = (int)fData;
			if (nInfo == PRECREATE_WORLDFILE || nInfo == PRECREATE_STRINGPROP || nInfo == PRECREATE_NORMAL)
			{
				ObjectCreateStruct* pocs = (ObjectCreateStruct*)pData;
				ReadProp( &pocs->m_cProperties );
			}
		}
		break;

		case MID_SAVEOBJECT:
		{
            Save((ILTMessage_Write*)pData);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((ILTMessage_Read*)pData);
		}
		break;

		case MID_INITIALUPDATE:
		{
			SetNextUpdate( UPDATE_NEVER );
		}
		break;
	}

	return nRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AICombatOpportunityRadius::ReadProp()
//
//	PURPOSE:	Read in the property values.
//
// ----------------------------------------------------------------------- //

void AICombatOpportunityRadius::ReadProp(const GenericPropList *pProps)
{
	m_flOuterRadius = pProps->GetReal( "OuterRadius", m_flOuterRadius );
	m_flInnerRadius = pProps->GetReal( "InnerRadius", m_flInnerRadius );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AICombatOpportunityRadius::Contains()
//
//	PURPOSE:	Returns true if the passed in position is valid, false if 
//				it is not.  Validity is based on the distance between the 
//				point and the origin of the radius.  
//
// ----------------------------------------------------------------------- //

bool AICombatOpportunityRadius::Contains(const LTVector& vPosition) const
{
	float flDistanceFromOriginSqr = (GetPosition() - vPosition).MagSqr();

	LTVector vDir = vPosition - GetPosition();
	vDir.Normalize();

	// Draw the two parts of the line; green for center towards object, 
	// red for outside radius

#ifndef _FINAL
	if (m_bDebugDraw)
	{
		DebugLineSystem& sys = LineSystem::GetSystem(this, "Range");
		sys.Clear();

		sys.AddLine(GetPosition(), 
			GetPosition() + vDir * m_flInnerRadius
			, Color::Red );

		sys.AddLine(
			GetPosition() + vDir * m_flInnerRadius, 
			GetPosition() + vDir * (m_flInnerRadius + m_flOuterRadius), 
			Color::Green );
	}
#endif

	// Position is inside the inner radius

	float flInnerRadiusSqr = m_flInnerRadius*m_flInnerRadius;
	if (flDistanceFromOriginSqr < flInnerRadiusSqr)
	{
		return false;
	}
	
	// Position it outside the outer radius.

	float flOuterRadiusSqr = m_flOuterRadius*m_flOuterRadius;
	if (flDistanceFromOriginSqr > flOuterRadiusSqr)
	{
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AICombatOpportunityRadius::GetPosition()
//
//	PURPOSE:	Returns the position of the AICombatOpportunityRadius.
//				This position is not cached, as this object may be 
//				keyframed or attached to a worldmodel.
//
// ----------------------------------------------------------------------- //

LTVector AICombatOpportunityRadius::GetPosition() const
{
	LTVector vRadiusOrigin;
	g_pLTServer->GetObjectPos(GetHOBJECT(), &vRadiusOrigin);
	return vRadiusOrigin;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AICombatOpportunityRadius::GetRadius()
//
//	PURPOSE:	Returns the radius of the AICombatOpportunityRadius.
//
// ----------------------------------------------------------------------- //

float AICombatOpportunityRadius::GetOuterRadius() const
{
	return m_flOuterRadius;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AICombatOpportunityRadius::HandleDebugDrawRadiusMsg()
//
//	PURPOSE:	Handles the DEBUG_DRAWRADIUS message.
//
// ----------------------------------------------------------------------- //

void AICombatOpportunityRadius::HandleDebugDrawRadiusMsg( HOBJECT hSender, const CParsedMsg& crParsedMsg )
{
#ifndef _FINAL
	static CParsedMsg::CToken s_cTok_0( "0" );
	m_bDebugDraw = !(crParsedMsg.GetArg( 1 ) == s_cTok_0);
	if (!m_bDebugDraw)
	{
		// Line system was turned off.  Clear the system.

		DebugLineSystem& sys = LineSystem::GetSystem(this, "Range");
		sys.Clear();
	}
#endif
}
