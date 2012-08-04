/****************************************************************************
;
;	 MODULE:		FLAGOBJECTS (.CPP)
;
;	PURPOSE:		Flag Objects for CTF
;
;	HISTORY:		12/30/98 [blg] This file was created
;
;	COMMENT:		Copyright (c) 1998, Monolith Productions Inc.
;
****************************************************************************/

// Includes...

#include "FlagObjects.h"
#include "ObjectUtilities.h"
#include "Spawner.h"
#include "StdIo.h"
#include <mbstring.h>
#include "SoundTypes.h"


// Statics...

static FlagObject* s_pTeam1Flag = NULL;
static FlagObject* s_pTeam2Flag = NULL;


// Functions...

// *********************************************************************** //
// FlagObject

BEGIN_CLASS(FlagObject)

	ADD_STRINGPROP(ObjectName, "CTF_Flag")					// Item name
	ADD_LONGINTPROP(TeamID, 0)								// Team ID (1 or 2)

END_CLASS_DEFAULT(FlagObject, B2BaseClass, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagObject::~FlagObject
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

FlagObject::~FlagObject()
{
	if (g_pServerDE && m_hFlagGrabString)
	{
		g_pServerDE->FreeString(m_hFlagGrabString);
		m_hFlagGrabString = DNULL;
	}

	if (this == s_pTeam1Flag)
	{
		s_pTeam1Flag = NULL;
	}

	if (this == s_pTeam2Flag)
	{
		s_pTeam2Flag = NULL;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagObject::EngineMessageFn
//
//	PURPOSE:	Handler for engine messages
//
// ----------------------------------------------------------------------- //

DDWORD FlagObject::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	// Handle the engine messages we're interested in...

	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			PostPropRead((ObjectCreateStruct*)pData);

			break;
		}

		case MID_INITIALUPDATE:
		{
			OnInitialUpdate(pData, fData);
			break;
		}

		case MID_TOUCHNOTIFY:
		{
			if (!IsWithPlayer())
			{
				OnTouchNotify((HOBJECT)pData);
			}
			break;
		}

		case MID_UPDATE:
		{
			if (IsOnGround())
			{
				ReturnToFlagStand();
			}
			break;
		}
	}

	return B2BaseClass::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagObject::ReadProp
//
//	PURPOSE:	Reads FlagObject properties
//
// ----------------------------------------------------------------------- //

DBOOL FlagObject::ReadProp(ObjectCreateStruct *pStruct)
{
	GenericProp genProp;

	if (g_pServerDE->GetPropGeneric("TeamID", &genProp) == DE_OK)
	{
		m_nTeamID = (int)genProp.m_Long;
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagStand::PostPropRead()
//
//	PURPOSE:	Updates the properties now that they have been read
//
// ----------------------------------------------------------------------- //

void FlagObject::PostPropRead(ObjectCreateStruct* pStruct)
{
	// Sanity checks...

	if (!pStruct) return;


	// Set the flags we want...

	pStruct->m_Flags |= FLAG_TOUCH_NOTIFY | FLAG_VISIBLE;
	pStruct->m_Flags &= ~FLAG_GRAVITY;

	g_pServerDE->SetNextUpdate(m_hObject, 0.001f);


	// Set the model filename

	if (m_nTeamID == 1)
	{
		char* pFilename = "Models\\WorldObjects\\flagblue.abc";
		_mbscpy((unsigned char*)pStruct->m_Filename, (const unsigned char*)pFilename);
	}
	else if (m_nTeamID == 2)
	{
		char* pFilename = "Models\\WorldObjects\\flagred.abc";
		_mbscpy((unsigned char*)pStruct->m_Filename, (const unsigned char*)pFilename);
	}
	

	// Set the skin filename

	if (m_nTeamID == 1)
	{
		char* pSkin = "Skins\\WorldObjects\\flagblue.dtx";	
		_mbscpy((unsigned char*)pStruct->m_SkinName, (const unsigned char*)pSkin);
	}
	else if (m_nTeamID = 2)
	{
		char* pSkin = "Skins\\WorldObjects\\flagred.dtx";	
		_mbscpy((unsigned char*)pStruct->m_SkinName, (const unsigned char*)pSkin);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagObject::OnInitialUpdate()
//
//	PURPOSE:	Handles the MID_INITIALUPDATE engine message
//
// ----------------------------------------------------------------------- //

void FlagObject::OnInitialUpdate(void* pData, DFLOAT fData)
{
	// Set the dims...

	DVector vDims;

	g_pServerDE->GetModelAnimUserDims(m_hObject, &vDims, g_pServerDE->GetModelAnimation(m_hObject));

	if (g_pServerDE->SetObjectDims2(m_hObject, &vDims) == DE_ERROR)
	{
		g_pServerDE->SetObjectDims(m_hObject, &vDims);
	}


	// Save our initial flag stand position...

	g_pServerDE->GetObjectPos(m_hObject, &m_vStand);
	g_pServerDE->GetObjectRotation(m_hObject, &m_rStand);

	m_bInStand    = DTRUE;
	m_bOnGround   = DFALSE;
	m_bWithPlayer = DFALSE;


	// Set our static pointer...

	if (m_nTeamID == TEAM_1)
	{
		s_pTeam1Flag = this;
	}

	if (m_nTeamID == TEAM_2)
	{
		s_pTeam2Flag = this;
	}


	// Create our trigger string if necessary...

	if (!m_hFlagGrabString)
	{
		m_hFlagGrabString = g_pServerDE->CreateString("flaggrab");
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagObject::ObjectMessageFn
//
//	PURPOSE:	Handle messages from objects
//
// ----------------------------------------------------------------------- //

DDWORD FlagObject::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	switch (messageID)
	{
		case MID_TRIGGER:
		{
			// Get the trigger name...

			HSTRING hMsg = g_pServerDE->ReadFromMessageHString(hRead);
			if (hMsg)
			{
				char* sMsg = g_pServerDE->GetStringData(hMsg);
				if (sMsg)
				{
					// Check if we should return to the flag stand...

					if (_mbsicmp((const unsigned char*)sMsg, (const unsigned char*)"return") == 0)
					{
						ReturnToFlagStand();
					}
				}

				g_pServerDE->FreeString(hMsg);
			}

			break;
		}

		default: break;
	}

	return B2BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagObject::SetPos
//
//	PURPOSE:	Sets the flags position to the given value
//
// ----------------------------------------------------------------------- //

void FlagObject::SetPos(DVector* pPos)
{
	// Sanity checks...

	if (!pPos) return;
	if (!g_pServerDE) return;


	// Set the objects position...

	g_pServerDE->SetObjectPos(m_hObject, pPos);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagObject::ReturnToFlagStand
//
//	PURPOSE:	Moves the flag back to it's original flag stand position
//
// ----------------------------------------------------------------------- //

void FlagObject::ReturnToFlagStand()
{
	// If we're already in our flag stand, just return...

	if (m_bInStand) return;


	// Set our "stand" position...

	DRotation rTemp;
	g_pServerDE->SetupEuler(&rTemp, 1, 1, 1);
	g_pServerDE->SetObjectRotation(m_hObject, &rTemp);

	g_pServerDE->SetObjectPos(m_hObject, &m_vStand);
	g_pServerDE->SetObjectRotation(m_hObject, &m_rStand);


	// Play a cool respawn sound...

	char* sSound = "sounds_multipatch\\CtfFlagRespawn.WAV";
	PlaySoundFromPos(&m_vStand, sSound, 2000.0f, SOUNDPRIORITY_MISC_MEDIUM, DFALSE, DFALSE, DFALSE);


	// Flag that we are back in our stand...

	m_bInStand    = DTRUE;
	m_bOnGround   = DFALSE;
	m_bWithPlayer = DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagObject::GiveToPlayer
//
//	PURPOSE:	Gives the flag to a player
//
// ----------------------------------------------------------------------- //

void FlagObject::GiveToPlayer()
{
	// Set all the flags such that we are being carried by a player...

	m_bInStand    = DFALSE;
	m_bOnGround   = DFALSE;
	m_bWithPlayer = DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagObject::DropToGround()
//
//	PURPOSE:	Drops the flag to the ground
//
// ----------------------------------------------------------------------- //

void FlagObject::DropToGround()
{
	// Move the flag to the ground...

	MoveObjectToGround(m_hObject);


	// Flag that we are not in our stand...

	m_bInStand    = DFALSE;
	m_bOnGround   = DTRUE;
	m_bWithPlayer = DFALSE;


	// Set our next update so that we'll respawn if we're still on the ground...

	g_pServerDE->SetNextUpdate(m_hObject, 30.0f);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagObject::OnTouchNotify()
//
//	PURPOSE:	Handles the MID_TOUCHNOTIFY engine messsage
//
// ----------------------------------------------------------------------- //

void FlagObject::OnTouchNotify(HOBJECT hToucher)
{
	// Determine if a player object touched us...

	if (!g_pServerDE->IsKindOf(g_pServerDE->GetObjectClass(hToucher), g_pServerDE->GetClass("CPlayerObj")))
	{
		return;
	}


	// Trigger this player object so they know they picked up this flag...

	TriggerPlayer(hToucher);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagObject::TriggerPlayer
//
//	PURPOSE:	Tell the player that they got this flag
//
// ----------------------------------------------------------------------- //

void FlagObject::TriggerPlayer(HOBJECT hPlayer)
{
	HMESSAGEWRITE hMessage = g_pServerDE->StartMessageToObject(this, hPlayer, MID_TRIGGER);
	g_pServerDE->WriteToMessageHString(hMessage, m_hFlagGrabString);
	g_pServerDE->EndMessage(hMessage);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagObject::IsOtherFlagInStand
//
//	PURPOSE:	Determines if the other flag is in it's flag stand
//
// ----------------------------------------------------------------------- //

DBOOL FlagObject::IsOtherFlagInStand()
{
	// Figure out which is the other flag...

	FlagObject* pOtherFlag = NULL;

	if (this == s_pTeam1Flag)
	{
		pOtherFlag = s_pTeam2Flag;
	}
	else if (this == s_pTeam2Flag)
	{
		pOtherFlag = s_pTeam1Flag;
	}

	if (!pOtherFlag) return(DFALSE);
	if (pOtherFlag == this) return(DFALSE);


	// Return whether or not the other flag is in it's flag stand...

	return(pOtherFlag->IsInFlagStand());
}


// *********************************************************************** //
// FlagStand

BEGIN_CLASS(FlagStand)

	ADD_STRINGPROP(ObjectName, "CTF_FlagStand")						// Item name
	ADD_LONGINTPROP(TeamID, 0)										// Team ID (1 or 2)

END_CLASS_DEFAULT(FlagStand, B2BaseClass, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagStand::EngineMessageFn
//
//	PURPOSE:	Handler for engine messages
//
// ----------------------------------------------------------------------- //

DDWORD FlagStand::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	// Handle the engine messages we're interested in...

	switch(messageID)
	{
		case MID_PRECREATE:
		{
			DDWORD dwRet = B2BaseClass::EngineMessageFn(messageID, pData, fData);

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			PostPropRead((ObjectCreateStruct*)pData);

			return(dwRet);
		}

		case MID_INITIALUPDATE:
		{
			OnInitialUpdate(pData, fData);
			break;
		}

		case MID_TOUCHNOTIFY:
		{
			OnTouchNotify((HOBJECT)pData);
			break;
		}

		default : break;
	}

	return B2BaseClass::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagStand::ReadProp
//
//	PURPOSE:	Reads FlagStand properties
//
// ----------------------------------------------------------------------- //

DBOOL FlagStand::ReadProp(ObjectCreateStruct* pStruct)
{
	// Sanity checks...

	if (!pStruct) return(DFALSE);


	// Read each prop...

	GenericProp genProp;

	if (g_pServerDE->GetPropGeneric("TeamID", &genProp) == DE_OK)
	{
		m_nTeamID = (int)genProp.m_Long;
	}


	// All done...

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagStand::PostPropRead()
//
//	PURPOSE:	Updates the properties now that they have been read
//
// ----------------------------------------------------------------------- //

void FlagStand::PostPropRead(ObjectCreateStruct* pStruct)
{
	// Sanity checks...

	if (!pStruct) return;


	// Set the flags we want...

	pStruct->m_Flags |= /*FLAG_SOLID |*/ FLAG_TOUCH_NOTIFY | FLAG_VISIBLE | FLAG_GRAVITY;
	pStruct->m_Flags &= ~FLAG_GRAVITY;	// (temp) ???

	g_pServerDE->SetNextUpdate(m_hObject, 0.001f);


	// Set the model filename

	if (m_nTeamID == 1)
	{
		char* pFilename = "Models_Multipatch\\flagbase.abc";
		_mbscpy((unsigned char*)pStruct->m_Filename, (const unsigned char*)pFilename);
	}
	else if (m_nTeamID == 2)
	{
		char* pFilename = "Models_Multipatch\\flagbase.abc";
		_mbscpy((unsigned char*)pStruct->m_Filename, (const unsigned char*)pFilename);
	}
	

	// Set the skin filename

	if (m_nTeamID == 1)
	{
		char* pSkin = "Skins_Multipatch\\flagbaseblue.dtx";	
		_mbscpy((unsigned char*)pStruct->m_SkinName, (const unsigned char*)pSkin);
	}
	else if (m_nTeamID = 2)
	{
		char* pSkin = "Skins_Multipatch\\flagbasered.dtx";	
		_mbscpy((unsigned char*)pStruct->m_SkinName, (const unsigned char*)pSkin);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagStand::OnInitialUpdate()
//
//	PURPOSE:	Handles the MID_INITIALUPDATE engine message
//
// ----------------------------------------------------------------------- //

void FlagStand::OnInitialUpdate(void* pData, DFLOAT fData)
{
	// Set the dims...

	DVector vDims;

	g_pServerDE->GetModelAnimUserDims(m_hObject, &vDims, g_pServerDE->GetModelAnimation(m_hObject));

	if (g_pServerDE->SetObjectDims2(m_hObject, &vDims) == DE_ERROR)
	{
		g_pServerDE->SetObjectDims(m_hObject, &vDims);
	}


	// Move the flag stand to the ground...

	MoveObjectToGround(m_hObject);


	// Create our trigger string if necessary...

	if (!m_hFlagGiveString)
	{
		m_hFlagGiveString = g_pServerDE->CreateString("flaggive");
	}


	// Spawn our flag object...

	SpawnFlag();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagStand::OnTouchNotify()
//
//	PURPOSE:	Handles the MID_TOUCHNOTIFY engine messsage
//
// ----------------------------------------------------------------------- //

void FlagStand::OnTouchNotify(HOBJECT hToucher)
{
	// Determine if a player object touched us...

	if (!g_pServerDE->IsKindOf(g_pServerDE->GetObjectClass(hToucher), g_pServerDE->GetClass("CPlayerObj")))
	{
		return;
	}


	// Trigger this player object so they know they touched a flag stand...

	TriggerPlayer(hToucher);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagStand::ObjectMessageFn
//
//	PURPOSE:	Handle messages from objects
//
// ----------------------------------------------------------------------- //

DDWORD FlagStand::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	switch (messageID)
	{
		case MID_TRIGGER:
		{
			break;
		}

		default: break;
	}

	return B2BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagStand::SpawnFlag
//
//	PURPOSE:	Spawns a flag object
//
// ----------------------------------------------------------------------- //

DBOOL FlagStand::SpawnFlag()
{
	// Sanity checks...

	if (m_nTeamID < 1) return(DFALSE);
	if (m_nTeamID > 2) return(DFALSE);


	// Get the position and rotation for the flag...

	DVector vPos;
	g_pServerDE->GetObjectPos(m_hObject, &vPos);
	vPos.x -= 16;
	vPos.y += 32;

	DRotation rRot;
	g_pServerDE->GetObjectRotation(m_hObject, &rRot);


	// Spawn the flag object for the team..

	if (!m_hFlagObject)
	{
		char sSpawnString[128];
		sprintf(sSpawnString, "FlagObject TeamID %i", GetTeamID());

		m_hFlagObject = SpawnObject(sSpawnString, &vPos, &rRot, NULL);
		if (!m_hFlagObject) return(DFALSE);
	}


	// All done...

	return(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagStand::TriggerPlayer
//
//	PURPOSE:	Tell the player that they got this flag
//
// ----------------------------------------------------------------------- //

void FlagStand::TriggerPlayer(HOBJECT hPlayer)
{
	HMESSAGEWRITE hMessage = g_pServerDE->StartMessageToObject(this, hPlayer, MID_TRIGGER);
	g_pServerDE->WriteToMessageHString(hMessage, m_hFlagGiveString);
	g_pServerDE->EndMessage(hMessage);
}



