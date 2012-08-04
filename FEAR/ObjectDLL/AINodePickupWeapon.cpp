// ----------------------------------------------------------------------- //
//
// MODULE  : AINodePickupWeapon.cpp
//
// PURPOSE : 
//
// CREATED : 7/01/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AINodePickupWeapon.h"
#include "WeaponItems.h"

LINKFROM_MODULE(AINodePickupWeapon);

#if defined ( PROJECT_DARK )

	#define CF_HIDDEN_AINodePickupWeapon 0

#elif defined ( PROJECT_FEAR )

	#define CF_HIDDEN_AINodePickupWeapon CF_HIDDEN

#endif

BEGIN_CLASS(AINodePickupWeapon)

	ADD_STRINGPROP_FLAG(Object,					"",				PF_OBJECTLINK, "The name of the WeaponItem instance the AI gains by using this node.  Note that this field must be filled out, or the AI will not use the node.  Additionally, this object must be a WeaponItem instance.")
	ADD_STRINGPROP_FLAG(SmartObject,			"", 			PF_STATICLIST|PF_DIMS, "TODO:PROPDESC")

END_CLASS_FLAGS_PLUGIN(AINodePickupWeapon, AINodeSmartObject, CF_HIDDEN_AINodePickupWeapon, AINodePickupWeaponPlugin, "AIs run to this node to pick up a weapon at it")

CMDMGR_BEGIN_REGISTER_CLASS(AINodePickupWeapon)
CMDMGR_END_REGISTER_CLASS(AINodePickupWeapon, AINodeSmartObject)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINodePickupWeapon::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

AINodePickupWeapon::AINodePickupWeapon()
{
}

AINodePickupWeapon::~AINodePickupWeapon()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodePickupWeapon::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the AINodePickupWeapon
//              
//----------------------------------------------------------------------------

void AINodePickupWeapon::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

void AINodePickupWeapon::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodePickupWeapon::ReadProp
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------

void AINodePickupWeapon::ReadProp(const GenericPropList *pProps)
{
	super::ReadProp(pProps);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodePickupWeapon::ReadProp
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------

void AINodePickupWeapon::InitNode()
{
	super::InitNode();

	// Insure the 'Object' field (the AnimObject) points to a WeaponItem 
	// instance.  If it does not, print a warning and clear the pointer.

	if (!m_hAnimObject)
	{
		ObjectCPrint(GetHOBJECT(), "AINodePickupWeapon does not have a valid WeaponItem specified; an AINodePickupWeapon object requires a WeaponItem to be specified in the 'Object' field.");
	}
	else if (!IsKindOf(m_hAnimObject, "WeaponItem"))
	{
		ObjectCPrint(GetHOBJECT(), "AINodePickupWeapon does not have a valid WeaponItem specified: %s is not a WeaponItem", m_strObject.c_str());
		m_hAnimObject = NULL;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodePickupWeapon::IsNodeValid
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------

bool AINodePickupWeapon::IsNodeValid( CAI* pAI, const LTVector& vPosAI, HOBJECT hThreat, EnumAIThreatPosition eThreatPos, uint32 dwStatusFlags )
{
	if (!super::IsNodeValid(pAI, vPosAI, hThreat, eThreatPos, dwStatusFlags))
	{
		return false;
	}

	// WeaponItem does not exist.

	if ( !m_hAnimObject )
	{
		return false;
	}

	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodePickupWeapon::GetWeaponItem
//              
//	PURPOSE:	Returns the handle of the WeaponItem this node points at.
//              
//----------------------------------------------------------------------------

HOBJECT AINodePickupWeapon::GetWeaponItem() const
{
	return m_hAnimObject;
}
