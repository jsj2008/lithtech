// ----------------------------------------------------------------------- //
//
// MODULE  : ServerSoundZoneVolume.cpp
//
// PURPOSE : A sound zone volume that's referenced by the ServerNonPointSound
//			object. Exists on the server, but info is gathered by ServerNonPointSound
//			and sent to the client, so this really doesn't do anything except
//			get edited by the editor..
//
// CREATED : 08/18/04
//
// (c) 1998-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //
#include "Stdafx.h"
#include "ServerSoundZoneVolume.h"

LINKFROM_MODULE( ServerSoundZoneVolume );

BEGIN_CLASS(SoundZoneVolume)
	ADD_VECTORPROP_VAL_FLAG(Dims, 20.0f, 20.0f, 20.0f, PF_DIMS | PF_LOCALDIMS, "The half dimensions of this volume box in centimeters")
END_CLASS_FLAGS(SoundZoneVolume, BaseClass, 0, "A volume that indicates a non-point sound source.")


SoundZoneVolume::SoundZoneVolume()
:	GameBase					( OT_NORMAL ),
	m_vHalfDims						( 5.0f, 5.0f, 5.0f )
{
}

SoundZoneVolume::~SoundZoneVolume()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 SoundZoneVolume::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	switch(messageID)
	{
	case MID_PRECREATE:
		{
			uint32 dwRet = GameBase::EngineMessageFn(messageID, pData, fData);

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp(&((ObjectCreateStruct*)pData)->m_cProperties);
			}

			PostPropRead((ObjectCreateStruct*)pData);

			return dwRet;
		}
		break;

	case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}
		}
		break;

	case MID_UPDATE:
		{
			SetNextUpdate(UPDATE_NEVER);
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
//	ROUTINE:	SoundZoneVolume:ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

bool SoundZoneVolume::ReadProp( const GenericPropList *pProps )
{
	// read in all our props
	m_vHalfDims = pProps->GetVector( "Dims", m_vHalfDims );

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundZoneVolume:PostPropRead()
//
//	PURPOSE:	Update properties
//
// ----------------------------------------------------------------------- //

void SoundZoneVolume::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	// Set the Update!

	pStruct->m_NextUpdate = 0.0f;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundZoneVolume:InitialUpdate()
//
//	PURPOSE:	Handle initial update
//
// ----------------------------------------------------------------------- //

bool SoundZoneVolume::InitialUpdate()
{
	SetNextUpdate(UPDATE_NEVER);
	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundZoneVolume::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void SoundZoneVolume::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	// Save stuff here..
	SAVE_VECTOR(m_vHalfDims);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundZoneVolume::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void SoundZoneVolume::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	// Load stuff here...
	LOAD_VECTOR(m_vHalfDims);
}

LTVector	SoundZoneVolume::GetHalfDims()
{
	return m_vHalfDims;
};


