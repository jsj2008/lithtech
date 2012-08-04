// ----------------------------------------------------------------------- //
//
// MODULE  : DestructableBrush.cpp
//
// PURPOSE : A worldmodel object type that assigns "Destructable" type to
//			 the object
//
// CREATED : 10/25/97
//
// ----------------------------------------------------------------------- //

#include <stdio.h>
#include "DestructableBrush.h"
#include "generic_msg_de.h"
#include "gib.h"
#include "ClientServerShared.h"
#include "WorldModelDebris.h"
#include "PhysicalAttributes.h"
#include "SharedDefs.h"
#include <mbstring.h>


BEGIN_CLASS(CDestructableBrush)
	ADD_DESTRUCTABLE_AGGREGATE()
	ADD_VISIBLE_FLAG(1,0)
	ADD_SOLID_FLAG(1,0)
	ADD_GRAVITY_FLAG(0,0)
	ADD_REALPROP(HitPoints, 0.0f)
	ADD_REALPROP(ObjectMass, 100.0f)
	ADD_BOOLPROP(Destructable, DTRUE)
	ADD_BOOLPROP(Pushable, DFALSE)
	ADD_BOOLPROP(BoxPhysics, DTRUE)
	ADD_BOOLPROP(AllowMarks, DTRUE)
	ADD_BOOLPROP(FireThrough, DFALSE)
	ADD_LONGINTPROP(SurfaceType, SURFTYPE_UNKNOWN)
	ADD_DEBRIS_AGGREGATE()
END_CLASS_DEFAULT(CDestructableBrush, B2BaseClass, NULL, NULL)

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructableBrush::CDestructableBrush
//
//	PURPOSE:	constructor
//
// --------------------------------------------------------------------------- //

CDestructableBrush::CDestructableBrush() : B2BaseClass(OT_WORLDMODEL) 
{ 
	AddAggregate(&m_damage);
	AddAggregate(&m_Debris);
	m_fInitHitPoints		= 0; 
	m_fMass					= 100.0f;
	m_bDestructable			= DTRUE;
	m_hstrDebrisSkin		= DNULL;
	m_bPushable				= DFALSE;
	m_bBoxPhysics			= DFALSE;
	m_bAllowMarks			= DTRUE;
	m_eSurfType				= SURFTYPE_UNKNOWN;
	m_bFireThrough			= DFALSE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructableBrush::~CDestructableBrush
//
//	PURPOSE:	destructor
//
// --------------------------------------------------------------------------- //

CDestructableBrush::~CDestructableBrush()
{ 
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (m_hstrDebrisSkin)
	{
		pServerDE->FreeString(m_hstrDebrisSkin);
		m_hstrDebrisSkin = NULL;
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructableBrush::ReadProp()
//
//	PURPOSE:	Reads properties
//
// --------------------------------------------------------------------------- //

DBOOL CDestructableBrush::ReadProp(ObjectCreateStruct *pStruct)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pStruct) return DFALSE;

	GenericProp genProp;

	if (g_pServerDE->GetPropGeneric("HitPoints", &genProp) == DE_OK)
	{
		m_fInitHitPoints = genProp.m_Float;
	}

	if (g_pServerDE->GetPropGeneric("ObjectMass", &genProp) == DE_OK)
	{
		m_fMass = genProp.m_Float;
	}

	if (g_pServerDE->GetPropGeneric("Destructable", &genProp) == DE_OK)
	{
		m_bDestructable = genProp.m_Bool;
	}

	if (g_pServerDE->GetPropGeneric("Pushable", &genProp) == DE_OK)
	{
		m_bPushable = genProp.m_Bool;
	}

	if (g_pServerDE->GetPropGeneric("BoxPhysics", &genProp) == DE_OK)
	{
		m_bBoxPhysics = genProp.m_Bool;
	}

	if (g_pServerDE->GetPropGeneric("AllowMarks", &genProp) == DE_OK)
	{
		m_bAllowMarks = genProp.m_Bool;
	}

	if (g_pServerDE->GetPropGeneric("SurfaceType", &genProp) == DE_OK)
	{
		m_eSurfType = (SurfaceType)genProp.m_Long;
	}

	if (g_pServerDE->GetPropGeneric("FireThrough", &genProp) == DE_OK)
	{
		m_bFireThrough = genProp.m_Bool;
	}

	return DTRUE;
}

	
// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructableBrush::PostPropRead()
//
//	PURPOSE:	Initializes data
//
// --------------------------------------------------------------------------- //

void CDestructableBrush::PostPropRead(ObjectCreateStruct *pStruct)
{
	_mbscpy((unsigned char*)pStruct->m_Filename, (const unsigned char*)pStruct->m_Name);
	pStruct->m_SkinName[0] = '\0';
	pStruct->m_Flags |= FLAG_FULLPOSITIONRES | FLAG_MODELGOURAUDSHADE;
	if (m_bBoxPhysics)
		pStruct->m_Flags |= FLAG_BOXPHYSICS;

	if (m_bPushable)	// Don't want to have to move marks too.
	{
		m_bAllowMarks = DFALSE;
		pStruct->m_Flags |= FLAG_GRAVITY;
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructableBrush::EngineMessageFn()
//
//	PURPOSE:	Handles engine messages.
//
// --------------------------------------------------------------------------- //

DDWORD CDestructableBrush::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	CServerDE* pServerDE = GetServerDE();
	switch(messageID)
	{
		case MID_UPDATE:
			if (m_bDestructable && m_damage.IsDead())
			{
				DVector vDir;
				m_damage.GetLastDamageDirection(&vDir);
				m_Debris.Create(vDir, m_damage.GetLastDamageAmount());
				CreateWorldModelDebris();
				pServerDE->RemoveObject(m_hObject);
			}
			break;


		case MID_PRECREATE:
		{
			// Need to call base class to have the object name read in before
			// we call PostPropRead()

			DDWORD dwRet = B2BaseClass::EngineMessageFn(messageID, pData, fData);

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			PostPropRead((ObjectCreateStruct*)pData);

			return dwRet;
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				m_damage.Init(m_hObject);
				m_damage.SetHitPoints(m_fInitHitPoints);
				m_damage.SetMass(m_fMass);
				m_Debris.Init(m_hObject);
				pServerDE->SetNextUpdate(m_hObject, 0.000f);

				m_damage.SetApplyDamagePhysics(m_bPushable);

				if (m_bPushable)
				{
					pServerDE->SetBlockingPriority(m_hObject, BLOCKPRIORITY_PUSHABLE);
					// Mark this object as moveable
					DDWORD dwUsrFlags = pServerDE->GetObjectUserFlags(m_hObject);
					dwUsrFlags |= USRFLG_MOVEABLE;
					pServerDE->SetObjectUserFlags(m_hObject, dwUsrFlags);
				}
				else
				{
					pServerDE->SetBlockingPriority(m_hObject, BLOCKPRIORITY_NONPUSHABLE);
				}


				// Mark this object as savable
				DDWORD dwFlags = g_pServerDE->GetObjectUserFlags(m_hObject);
				dwFlags |= USRFLG_SAVEABLE;
				dwFlags |= (m_eSurfType << 24);
				g_pServerDE->SetObjectUserFlags(m_hObject, dwFlags);
			}
		}
		break;

		case MID_SAVEOBJECT:
			Save((HMESSAGEWRITE)pData, (DDWORD)fData);
			break;

		case MID_LOADOBJECT:
			Load((HMESSAGEREAD)pData, (DDWORD)fData);
			break;

		default : break;
	}

	return B2BaseClass::EngineMessageFn(messageID, pData, fData);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructableBrush::ObjectMessageFn()
//
//	PURPOSE:	Processes a message from a server object.
//
// --------------------------------------------------------------------------- //
DDWORD CDestructableBrush::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = GetServerDE();
	switch(messageID)
	{
			
		case MID_DAMAGE:
		{
			// Call baseclass so that the damage can be processed.
			DDWORD dwRet = B2BaseClass::ObjectMessageFn(hSender, messageID, hRead);

			if (m_bDestructable && m_damage.IsDead())
			{
				pServerDE->SetNextUpdate(m_hObject, 0.001f);
			}

			return dwRet;
		}
	}

	return B2BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructableBrush::CreateWorldModelDebris()
//
//	PURPOSE:	Create world model debris...
//
// ----------------------------------------------------------------------- //

void CDestructableBrush::CreateWorldModelDebris()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE);

	char* pName = pServerDE->GetObjectName(m_hObject);
	if (!pName || !pName[0]) return;

	// Find all the debris objects...
	int nNum = 0;

	char strKey[128]; memset(strKey, 0, 128);
	char strNum[18];  memset(strNum, 0, 18);

	HCLASS hWMDebris = pServerDE->GetClass("WorldModelDebris");

	while (1)
	{
		// Create the keyname string...

		sprintf(strKey, "%sdebris%d", pName, nNum);
		
		// Find any debris with that name...

		ObjectList* pTempKeyList = pServerDE->FindNamedObjects(strKey);
		ObjectLink* pLink = pTempKeyList->m_pFirstLink;
		if (!pLink) return;
		
		while (pLink)
		{
			if (pServerDE->IsKindOf(pServerDE->GetObjectClass(pLink->m_hObject), hWMDebris))
			{
				WorldModelDebris* pDebris = (WorldModelDebris*)pServerDE->HandleToObject(pLink->m_hObject);
				if (!pDebris) break;

//				DVector vVel, vRotPeriods;
//				VEC_SET(vVel, pServerDE->Random(-200.0f, 200.0f), 
//							  pServerDE->Random(100.0f, 300.0f),
//							  pServerDE->Random(-200.0f, 200.0f) );

//				VEC_SET(vRotPeriods, pServerDE->Random(-1.0f, 1.0f),
//						pServerDE->Random(-1.0f, 1.0f), pServerDE->Random(-1.0f, 1.0f));

//				pDebris->Start(&vRotPeriods, &vVel);
				pDebris->Start();
			}

			pLink = pLink->m_pNext;
		}

		pServerDE->RelinquishList (pTempKeyList);
		
		// Increment the counter...

		nNum++;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructableBrush::GetSurfaceType
//
//	PURPOSE:	Returns the surface type of the object, or it's debris.
//
// ----------------------------------------------------------------------- //

SurfaceType CDestructableBrush::GetSurfaceType()
{
	SurfaceType eType;

	if (m_eSurfType != SURFTYPE_UNKNOWN)
		eType = m_eSurfType;
	else
		eType = m_Debris.GetType();

	return eType;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructableBrush::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CDestructableBrush::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageFloat(hWrite, m_fInitHitPoints);
	pServerDE->WriteToMessageFloat(hWrite, m_fMass);
	pServerDE->WriteToMessageByte(hWrite, m_bDestructable);
	pServerDE->WriteToMessageByte(hWrite, m_bPushable);
	pServerDE->WriteToMessageByte(hWrite, m_bBoxPhysics);
	pServerDE->WriteToMessageByte(hWrite, m_bAllowMarks);
	pServerDE->WriteToMessageHString(hWrite, m_hstrDebrisSkin);
	pServerDE->WriteToMessageDWord(hWrite, m_eSurfType);
	pServerDE->WriteToMessageByte(hWrite, m_bFireThrough);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructableBrush::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CDestructableBrush::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	m_fInitHitPoints	= pServerDE->ReadFromMessageFloat(hRead);
	m_fMass				= pServerDE->ReadFromMessageFloat(hRead);
	m_bDestructable		= pServerDE->ReadFromMessageByte(hRead);
	m_bPushable			= pServerDE->ReadFromMessageByte(hRead);
	m_bBoxPhysics		= pServerDE->ReadFromMessageByte(hRead);
	m_bAllowMarks		= pServerDE->ReadFromMessageByte(hRead);
	m_hstrDebrisSkin	= pServerDE->ReadFromMessageHString(hRead);
	m_eSurfType			= (SurfaceType)pServerDE->ReadFromMessageDWord(hRead);
	m_bFireThrough		= pServerDE->ReadFromMessageByte(hRead);
}



