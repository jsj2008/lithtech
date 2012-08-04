// ----------------------------------------------------------------------- //
//
// MODULE  : Kura.cpp
//
// PURPOSE : Kura - Implementation
//
// CREATED : 3/19/98
//
// ----------------------------------------------------------------------- //

#include "Kura.h"

BEGIN_CLASS(Kura)
	ADD_LONGINTPROP(WeaponId, GUN_SHOTGUN_ID)
	ADD_STRINGPROP_FLAG(Filename, GetModel(MI_AI_KURA_ID), PF_DIMS | PF_HIDDEN)
	ADD_BOOLPROP(Ghost, DFALSE)
END_CLASS_DEFAULT(Kura, MajorCharacter, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Kura::Kura()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Kura::Kura() : MajorCharacter()
{
	m_nModelId	 = MI_AI_KURA_ID;
	m_bIsMecha	 = DFALSE;
	m_nWeaponId	 = GUN_SHOTGUN_ID;
	m_cc		 = UCA;
	m_bGhost	 = DFALSE;

	m_fDimsScale[MS_NORMAL] = 1.0f;
	m_fDimsScale[MS_SMALL]  = 1.0f;
	m_fDimsScale[MS_LARGE]  = 1.0f;

	m_fWalkVel		= 200.0f;
	m_fRunVel		= 400.0f;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Kura::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD Kura::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			DDWORD dwRet = BaseAI::EngineMessageFn(messageID, pData, fData);
			if (fData == 1.0f || fData == 2.0f )
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			if (m_eModelSize == MS_SMALL)
			{
				m_nWeaponId	= GUN_NONE;
			}
			return dwRet;
		}

		case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData, (DDWORD)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData, (DDWORD)fData);
		}
		break;

		default : break;
	}


	return BaseAI::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Kura::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

DBOOL Kura::ReadProp(ObjectCreateStruct *pData)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pData) return DFALSE;

	GenericProp genProp;
	if ( pServerDE->GetPropGeneric( "Ghost", &genProp ) == DE_OK )
		m_bGhost = genProp.m_Bool;

	if (m_bGhost)
	{
		DFLOAT r, g, b, a;
		pServerDE->GetObjectColor(m_hObject, &r, &g, &b, &a);
		pServerDE->SetObjectColor(m_hObject, r, g, b, 0.35f);

		m_dwFlags &= ~FLAG_SHADOW;
		m_nWeaponId	= GUN_NONE;
	}


	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Kura::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Kura::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageByte(hWrite, m_bGhost);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Kura::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Kura::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	m_bGhost = (DBOOL) pServerDE->ReadFromMessageByte(hRead);
}