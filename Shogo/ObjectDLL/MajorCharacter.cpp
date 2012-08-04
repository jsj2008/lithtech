// ----------------------------------------------------------------------- //
//
// MODULE  : MajorCharacter.cpp
//
// PURPOSE : MajorCharacter - Implementation
//
// CREATED : 3/19/98
//
// ----------------------------------------------------------------------- //

#include "MajorCharacter.h"
#include "cpp_server_de.h"
#include "RiotServerShell.h"
#include "RiotMsgIds.h"
#include "PlayerObj.h"

extern CRiotServerShell* g_pRiotServerShellDE;

BEGIN_CLASS(MajorCharacter)
	PROP_DEFINEGROUP(AvailableSounds, PF_GROUP1)
		ADD_BOOLPROP_FLAG(Spot, 0, PF_GROUP1)
		ADD_BOOLPROP_FLAG(Idle, 0, PF_GROUP1)
	ADD_LONGINTPROP(State, BaseAI::IDLE)
END_CLASS_DEFAULT(MajorCharacter, BaseAI, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MajorCharacter::MajorCharacter()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

MajorCharacter::MajorCharacter() : BaseAI()
{
	m_bUsingHitDetection = DFALSE;
}

MajorCharacter::~MajorCharacter()
{
	KillDlgSnd( );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MajorCharacter::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

DDWORD MajorCharacter::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	CPlayerObj *pPlayerObj;

	switch (messageID)
	{
		case MID_PLAYDIALOG:
		{
			DialogQueueCharacter *pDialogQueueCharacter;

			pDialogQueueCharacter = ( DialogQueueCharacter * )g_pServerDE->ReadFromMessageDWord( hRead );
			pPlayerObj = g_pRiotServerShellDE->GetFirstPlayer( );
			if( pPlayerObj )
			{
				if( pDialogQueueCharacter )
				{
					PlayDialogSound( pDialogQueueCharacter->m_szDialogFile, pDialogQueueCharacter->m_eCharacterSoundType );

					// Check if sound played
					if( m_hCurDlgSnd )
						pPlayerObj->SetDialogActive( DTRUE );
				}

				g_pServerDE->BreakInterObjectLink( pPlayerObj->m_hObject, m_hObject );
			}
		}
		break;
	}
	
	return CBaseCharacter::ObjectMessageFn (hSender, messageID, hRead);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MajorCharacter::KillDlgSnd
//
//	PURPOSE:	Kill the current dialog sound
//
// ----------------------------------------------------------------------- //

void MajorCharacter::KillDlgSnd()
{
	CPlayerObj *pPlayerObj;

	if( m_hCurDlgSnd )
	{
		pPlayerObj = g_pRiotServerShellDE->GetFirstPlayer( );
		if( pPlayerObj )
		{
			pPlayerObj->SetDialogActive( DFALSE );
		}
	}

	BaseAI::KillDlgSnd( );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MajorCharacter::PlayDialogSound
//
//	PURPOSE:	Play a dialog sound
//
// ----------------------------------------------------------------------- //

void MajorCharacter::PlayDialogSound(char* pSound, CharacterSoundType eType,
									 DBOOL bAtObjectPos)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pSound || !pSound[0]) return;
	if ((m_damage.IsDead() && eType != CST_DEATH)) return;
	if (eType == CST_EXCLAMATION && !m_bCanPlayDialogSound) return;


	// Kill current sound...

	KillDlgSnd();

	m_hCurDlgSnd = PlaySoundLocal( pSound, m_nBasePriority + SOUNDPRIORITYMOD_HIGH, DFALSE, DTRUE, DTRUE, 100, DTRUE );

	if (m_hCurDlgSnd && m_hDlgSprite && eType == CST_EXCLAMATION)
	{
		// Reset the filename in case the file has changed somehow...

		DVector vScale;
		char* pFilename = GetDialogSpriteFilename(vScale);
		if (!pFilename) return;
		pServerDE->SetObjectFilenames(m_hDlgSprite, pFilename, "");

		// Show the sprite...

		DDWORD dwFlags = pServerDE->GetObjectFlags(m_hDlgSprite);
		pServerDE->SetObjectFlags(m_hDlgSprite, dwFlags | FLAG_VISIBLE);
	}
	
	m_eCurDlgSndType = eType;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	MajorCharacter::ProcessCommand()
//
//	PURPOSE:	Process a command
//
// --------------------------------------------------------------------------- //

DBOOL MajorCharacter::ProcessCommand(char** pTokens, int nArgs, char* pNextCommand)
{
	CServerDE* pServerDE = GetServerDE();
	CPlayerObj *pPlayerObj;

	if (!pServerDE || !pTokens || nArgs < 1) return DFALSE;

	// Queue up this playsound command.
	if( stricmp( TRIGGER_PLAY_SOUND, pTokens[0] ) == 0 && nArgs > 1 )
	{
		// Get sound name from message...

		char* pSoundName = pTokens[1];
		pPlayerObj = g_pRiotServerShellDE->GetFirstPlayer( );

		if( pSoundName && pPlayerObj )
		{
			DialogQueueCharacter *pDialogQueueCharacter;
			DialogQueueElement *pDialogQueueElement;

			pDialogQueueElement = new DialogQueueElement;
			pDialogQueueElement->m_hObject = m_hObject;
			pDialogQueueCharacter = new DialogQueueCharacter;
			pDialogQueueElement->m_pData = pDialogQueueCharacter;
			SAFE_STRCPY( pDialogQueueCharacter->m_szDialogFile, pSoundName );
			pDialogQueueCharacter->m_eCharacterSoundType = CST_EXCLAMATION;

			dl_AddTail( pPlayerObj->GetDialogQueue( ), &pDialogQueueElement->m_Link, pDialogQueueElement );
			g_pServerDE->CreateInterObjectLink( pPlayerObj->m_hObject, m_hObject );
		}

		return DTRUE;
	}

	// Let parent class have a whack at it...
	return BaseAI::ProcessCommand(pTokens, nArgs, pNextCommand);
}
