// ----------------------------------------------------------------------- //
//
// MODULE  : LittleKid.cpp
//
// PURPOSE : LittleKid - Implementation
//
// CREATED : 5/18/98
//
// ----------------------------------------------------------------------- //

#include "LittleKid.h"

BEGIN_CLASS(LittleKid)
	ADD_LONGINTPROP( WeaponId, GUN_NONE )
	ADD_STRINGPROP_FLAG( Filename, GetModel(MI_AI_LITTLEBOY_ID), PF_DIMS | PF_HIDDEN )
	ADD_BOOLPROP( Boy, DTRUE )
END_CLASS_DEFAULT( LittleKid, BaseAI, NULL, NULL )


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LittleKid::LittleKid()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

LittleKid::LittleKid() : BaseAI()
{
	m_nModelId	= MI_AI_LITTLEBOY_ID;
	m_nWeaponId	= GUN_NONE;
	m_cc		= BYSTANDER;
	m_bCreateHandHeldWeapon	= DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LittleKid::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD LittleKid::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == 1.0f || fData == 2.0f )
			{
				ReadProp((ObjectCreateStruct*)pData);
			}
		}
		break;

		default : break;
	}

	return BaseAI::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LittleKid::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

DBOOL LittleKid::ReadProp(ObjectCreateStruct *pData)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pData) return DFALSE;

	GenericProp genProp;
	if ( pServerDE->GetPropGeneric( "Boy", &genProp ) == DE_OK )
		m_nModelId = genProp.m_Bool ? MI_AI_LITTLEBOY_ID : MI_AI_LITTLEGIRL_ID;

	return DTRUE;
}
