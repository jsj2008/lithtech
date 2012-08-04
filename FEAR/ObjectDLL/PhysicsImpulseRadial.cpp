// ----------------------------------------------------------------------- //
//
// MODULE  : PhysicsImpulseRadial.cpp
//
// PURPOSE : PhysicsImpulseRadial - Implementation
//
// CREATED : 04/15/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "PhysicsImpulseRadial.h"
#include "PhysicsUtilities.h"

LINKFROM_MODULE( PhysicsImpulseRadial );

BEGIN_CLASS(PhysicsImpulseRadial)

ADD_REALPROP_FLAG(ImpulseForce, PhysicsUtilities::DEFAULT_IMPULSE_FORCE, 0, "The amount of force the PhysicsImpulseRadial object will apply to objects inside the RadiusMin.  This force value is linearly scaled between ImpulseForce and 0 between RadiusMin and RadiusMax.  The value is measured in game unit newton seconds.")
	ADD_REALPROP_FLAG(RadiusMin, 50.0f, PF_RADIUS, "This the minimum radius measured in WorldEdit units from the center of the PhysicsImpulseRadial object in which 100% of the impulse force will be applied to objects.  The force amount is linearly scaled from ImpulseForce to 0 between the RadiusMin and RadiusMax.")
	ADD_REALPROP_FLAG(RadiusMax, 200.0f, PF_RADIUS, "This the maximum radius measured in WorldEdit units from the center of the PhysicsImpulseRadial object in which the impulse force will be applied to objects.  The force amount is linearly scaled from ImpulseForce to 0 between the RadiusMin and RadiusMax.")
	ADD_BOOLPROP_FLAG(RemoveWhenDone, true, 0, "This flag toggles whether or not the PhysicsImpulseRadial object is removed after being triggered. Sometimes you will want to trigger an PhysicsImpulseRadial object repeatedly. In which case you would set this flag to false.")

END_CLASS_FLAGS(PhysicsImpulseRadial, GameBase, 0, "The PhysicsImpulseRadial object should be used to apply a physical force in a sphere around the position of the object.  All physically simulated objects within RadiusMin distance from the PhysicsImpulseRadial object will have ImpulseForce amount of force applied to them in the direction determined by the vector from the center of the PhysicsImpulseRadial object to the center of the physically simulated object.  The force applied to physically simulated objects outside of RadiusMin but inside RadiusMax is linearly scaled between ImpulseForce and 0 between RadiusMin and RadiusMax.  The force value is measured in game unit newton seconds.")


CMDMGR_BEGIN_REGISTER_CLASS( PhysicsImpulseRadial )

	ADD_MESSAGE( ON,	1,	NULL,	MSG_HANDLER( PhysicsImpulseRadial, HandleOnMsg ),	"ON", "Trigger the object to apply the specified impulse force.", "msg PhysicsImpulseRadial ON" )

CMDMGR_END_REGISTER_CLASS( PhysicsImpulseRadial, GameBase )

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PhysicsImpulseRadial::PhysicsImpulseRadial()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

PhysicsImpulseRadial::PhysicsImpulseRadial() : GameBase()
{
	m_fImpulse				= PhysicsUtilities::DEFAULT_IMPULSE_FORCE;
	m_fRadiusMin			= 50.0f;
	m_fRadiusMax			= 200.0f;
    m_bRemoveWhenDone       = true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PhysicsImpulseRadial::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 PhysicsImpulseRadial::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	switch(messageID)
	{
		case MID_INITIALUPDATE:
		{
			SetNextUpdate(UPDATE_NEVER);
		}
		break;

		case MID_PRECREATE :
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp(&((ObjectCreateStruct*)pData)->m_cProperties);
			}
		}
		break;

		case MID_SAVEOBJECT:
		{
            Save((ILTMessage_Write*)pData, (uint32)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((ILTMessage_Read*)pData, (uint32)fData);
		}
		break;

		default : break;
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PhysicsImpulseRadial::HandleOnMsg
//
//	PURPOSE:	Handle ON message...
//
// ----------------------------------------------------------------------- //

void PhysicsImpulseRadial::HandleOnMsg( HOBJECT /*hSender*/, const CParsedMsg& /*crParsedMsg*/ )
{
	ApplyForceToObjectsInSphere();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PhysicsImpulseRadial::ReadProp
//
//	PURPOSE:	Read object properties
//
// ----------------------------------------------------------------------- //

void PhysicsImpulseRadial::ReadProp(const GenericPropList *pProps)
{
	m_fImpulse			= pProps->GetReal( "ImpulseForce", m_fImpulse );
	m_fRadiusMin		= pProps->GetReal( "RadiusMin", m_fRadiusMin );
	m_fRadiusMax		= pProps->GetReal( "RadiusMax", m_fRadiusMax );
	m_bRemoveWhenDone	= pProps->GetBool( "RemoveWhenDone", m_bRemoveWhenDone );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PhysicsImpulseRadial::ApplyForceToObject()
//
//	PURPOSE:	Apply impulse force to the specified object
//
// ----------------------------------------------------------------------- //

void PhysicsImpulseRadial::ApplyForceToObject(const LTVector & vMyPos, HOBJECT hObj)
{
	if (!hObj) return;

	LTVector vObjPos;
	g_pLTServer->GetObjectPos(hObj, &vObjPos);

    LTVector vDir = vObjPos - vMyPos;
    float fDist = vDir.Mag();

	if (fDist > m_fRadiusMax)
		return;

	// Scale force if necessary...
    float fMultiplier = 1.0f;
	if (fDist > m_fRadiusMin && m_fRadiusMin < m_fRadiusMax)
	{
        float fPercent = (fDist - m_fRadiusMin) / (m_fRadiusMax - m_fRadiusMin);
		fPercent = fPercent > 1.0f ? 1.0f : (fPercent < 0.0f ? 0.0f : fPercent);

		fMultiplier = (1.0f - fPercent);
	}

	//Apply a physical impulse force to the object if direction is valid...
	if (vDir != LTVector::GetIdentity())
	{
		vDir.Normalize();
		vDir.SetMagnitude(fMultiplier);
		PhysicsUtilities::ApplyPhysicsImpulseForce(hObj, m_fImpulse, vDir, LTVector(0, 0, 0), true);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PhysicsImpulseRadial::ApplyForceToObjectsInSphere()
//
//	PURPOSE:	Apply force to all the objects in our radius
//
// ----------------------------------------------------------------------- //

void PhysicsImpulseRadial::ApplyForceToObjectsInSphere()
{
	if (!m_hObject || m_fRadiusMax <= 0.0f) return;

	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);

	ObjectList* pList = g_pLTServer->FindObjectsTouchingSphere(&vPos, m_fRadiusMax);
	if (!pList) return;

	ObjectLink* pLink = pList->m_pFirstLink;
	while (pLink)
	{
		ApplyForceToObject(vPos, pLink->m_hObject);
		pLink = pLink->m_pNext;
	}

    g_pLTServer->RelinquishList(pList);

	if (m_bRemoveWhenDone)
	{
		g_pLTServer->RemoveObject(m_hObject);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PhysicsImpulseRadial::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void PhysicsImpulseRadial::Save(ILTMessage_Write *pMsg, uint32 /*dwSaveFlags*/)
{
	if (!pMsg) return;

	SAVE_FLOAT(m_fImpulse);
	SAVE_FLOAT(m_fRadiusMin);
	SAVE_FLOAT(m_fRadiusMax);
    SAVE_BOOL(m_bRemoveWhenDone);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PhysicsImpulseRadial::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void PhysicsImpulseRadial::Load(ILTMessage_Read *pMsg, uint32 /*dwLoadFlags*/)
{
	if (!pMsg) return;

	LOAD_FLOAT(m_fImpulse);
	LOAD_FLOAT(m_fRadiusMin);
	LOAD_FLOAT(m_fRadiusMax);
	LOAD_BOOL(m_bRemoveWhenDone);
}
