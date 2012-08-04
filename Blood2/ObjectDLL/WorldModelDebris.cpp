// ----------------------------------------------------------------------- //
//
// MODULE  : WorldModelDebris.cpp
//
// PURPOSE : A WorldModelDebris object
//
// CREATED : 2/27/98
//
// ----------------------------------------------------------------------- //

// Includes...
#include "WorldModelDebris.h"
#include "ObjectUtilities.h"
#include "Rotating.h"
#include <stdio.h>

BEGIN_CLASS(WorldModelDebris)
	ADD_ROTATING_AGGREGATE()
	ADD_VISIBLE_FLAG(0, PF_HIDDEN)
	ADD_SOLID_FLAG(0, PF_HIDDEN)
	ADD_GRAVITY_FLAG(0, PF_HIDDEN)
	ADD_VECTORPROP_VAL(InitialVelocity, 0, 0, 0)
	ADD_REALPROP(GravityMultiplier, 1.0f)
END_CLASS_DEFAULT(WorldModelDebris, CDestructableBrush, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldModelDebris::WorldModelDebris()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

WorldModelDebris::WorldModelDebris() : CDestructableBrush()
{
	AddAggregate(&m_Rotating);

	m_bOn = DFALSE;
	m_fGravityMultiplier = 1.0f;
	VEC_INIT(m_vInitialVelocity);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldModelDebris::~WorldModelDebris()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

WorldModelDebris::~WorldModelDebris()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldModelDebris::Setup
//
//	PURPOSE:	Set up a WorldModelDebris with the information needed
//
// ----------------------------------------------------------------------- //

void WorldModelDebris::Start()
{
//	if (!pvRotationPeriods || !pvVel) return;

	// Make sure it's healed
	m_damage.Heal(10000);

	// Start rotating
	m_Rotating.SetSpinUp();
	m_bOn = DTRUE;

	DDWORD dwFlags = FLAG_VISIBLE | FLAG_GRAVITY | FLAG_RAYHIT | FLAG_SOLID;
	g_pServerDE->SetObjectFlags(m_hObject, dwFlags);

	g_pServerDE->SetVelocity(m_hObject, &m_vInitialVelocity);
	g_pServerDE->SetBlockingPriority(m_hObject, 100);
	g_pServerDE->SetForceIgnoreLimit(m_hObject, 0.0f);

	g_pServerDE->SetNextUpdate(m_hObject, 0.01f);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	WorldModelDebris::EngineMessageFn()
//
//	PURPOSE:	Handler for engine messages
//
// --------------------------------------------------------------------------- //

DDWORD WorldModelDebris::EngineMessageFn(DDWORD messageID, void *pData, float fData)
{
	switch (messageID)
	{
		case MID_UPDATE:
		{
			Update();
			break;
		}

		case MID_PRECREATE:
		{
			DDWORD dwRet = CDestructableBrush::EngineMessageFn(messageID, pData, fData);

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
				ReadProp((ObjectCreateStruct*)pData);
			
			return dwRet;
		}

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				m_Rotating.Init(m_hObject);
				m_Rotating.SetOff();
			}
			break;
		}

		case MID_SAVEOBJECT:
			Save((HMESSAGEWRITE)pData, (DDWORD)fData);
			break;

		case MID_LOADOBJECT:
			Load((HMESSAGEREAD)pData, (DDWORD)fData);
			break;

		default : break;
	}

	return CDestructableBrush::EngineMessageFn(messageID, pData, fData);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	WorldModelDebris::ReadProp()
//
//	PURPOSE:	Reads WorldModelDebris properties
//
// --------------------------------------------------------------------------- //

void WorldModelDebris::ReadProp(ObjectCreateStruct *pStruct)
{
	GenericProp gen;

	if (g_pServerDE->GetPropGeneric("GravityMultiplier", &gen) == LT_OK)
	{
		m_fGravityMultiplier = gen.m_Float;
	}

	if (g_pServerDE->GetPropGeneric("InitialVelocity", &gen) == LT_OK)
	{
		VEC_COPY(m_vInitialVelocity, gen.m_Vec);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldModelDebris::Update
//
//	PURPOSE:	Update the WorldModelDebris
//
// ----------------------------------------------------------------------- //

void WorldModelDebris::Update()
{
	CollisionInfo collisionInfo;

	if (m_bOn) 
	{
		// Set acceleration based on gravity multiplier..
		DVector vGravity;
		g_pServerDE->GetGlobalForce(&vGravity);
		VEC_MULSCALAR(vGravity, vGravity, (m_fGravityMultiplier - 1.0f)); 
		g_pServerDE->SetAcceleration(m_hObject, &vGravity);

		g_pServerDE->SetNextUpdate(m_hObject, 0.01f);
		g_pServerDE->GetStandingOn(m_hObject, &collisionInfo);
			
		if (collisionInfo.m_hObject) 
		{
			m_bOn = DFALSE;
			m_Rotating.SetSpinDown();
		}
	}
	else
	{
		g_pServerDE->SetNextUpdate(m_hObject, 0.0f);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldModelDebris::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void WorldModelDebris::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveGame)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageVector(hWrite, &m_vInitialVelocity);
	pServerDE->WriteToMessageByte(hWrite, m_bOn);
	pServerDE->WriteToMessageFloat(hWrite, m_fGravityMultiplier);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldModelDebris::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void WorldModelDebris::Load(HMESSAGEREAD hRead, DDWORD dwLoadGame)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	pServerDE->ReadFromMessageVector(hRead, &m_vInitialVelocity);
	m_bOn				 = (DBOOL)pServerDE->ReadFromMessageByte(hRead);
	m_fGravityMultiplier = pServerDE->ReadFromMessageFloat(hRead);
}



