// ----------------------------------------------------------------------- //
//
// MODULE  : GadgetDisabler.cpp
//
// PURPOSE : GadgetDisabler implementation
//
// CREATED : 8/30/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "stdafx.h"
	#include "MsgIDs.h"
	#include "GadgetTargetTypes.h"
	#include "GadgetDisabler.h"
	#include "ClientWeaponBase.h"
	#include "PlayerMgr.h"
	#include "PlayerStats.h"
	#include "InterfaceMgr.h"
	#include "PopupMgr.h"
	#include "TargetMgr.h"
	
extern VarTrack g_vtProgressBarScaleToSkills;

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CGadgetDisabler::Update
//
//  PURPOSE:	Update the disabler...
//
// ----------------------------------------------------------------------- //

void CGadgetDisabler::Update( )
{
	// Are we actually disabling a gadget?

	if( !m_bDisabling || !m_hTargetObj ) return;

	// If stoped disabling send message to the target

	IClientWeaponBase *pClientWeapon = g_pPlayerMgr->GetCurrentClientWeapon();
	if( !pClientWeapon ) return;

	const WEAPON *pWeaponData = pClientWeapon->GetWeapon();
	if( !pWeaponData ) return;

	const AMMO* pAmmoData = g_pWeaponMgr->GetAmmo(pWeaponData->nDefaultAmmoId);
	if (!pAmmoData) return;


	LTRotation	rCamRot;
	LTVector	vTargetPos, vCamPos, vDir, vCamF;

	HLOCALOBJ	hCamera = g_pPlayerMgr->GetCamera();

	g_pLTClient->GetObjectRotation( hCamera, &rCamRot );
	vCamF = rCamRot.Forward();

	g_pLTClient->GetObjectPos( hCamera, &vCamPos );
	g_pLTClient->GetObjectPos( m_hTargetObj, &vTargetPos );
	
	// See if we are within range of the target...

	vDir = vTargetPos - vCamPos;
	bool bInRange = (g_pPlayerMgr->GetTargetMgr()->IsTargetInRange());

	// See if we are looking at the target...

	vDir.Normalize();
	LTFLOAT	fCamAngle = vDir.Dot( vCamF );
	fCamAngle = fCamAngle < 0.0f ? 0.0f : fCamAngle;
	fCamAngle *= 90.0f;
	
	LTFLOAT	fMinAngle = 5.0f;
	LTBOOL bLookingAtTarget = !(fCamAngle < (90.0f - fMinAngle ));
	
	float fGadgetSkillEffect = g_pPlayerStats->GetSkillModifier(SKL_GADGET,GadgetModifiers::eEffect);

	// Base the speed of searching on the skill level...
	
	if (!g_pGameClientShell->IsGamePaused())
	{
		m_fTimer -= g_pLTClient->GetFrameTime() * fGadgetSkillEffect;
		if (m_fTimer < 0.0f)
		{
			m_fTimer = 0.0f;
		}
	}


//jrg - changing so activate can disable as well as fire
//	LTBOOL bButtonDown = (m_pGTInfo->m_eTargetType == eINVALID ? 
//							g_pLTClient->IsCommandOn( COMMAND_ID_ACTIVATE ) :
//							pClientWeapon->IsFireButtonDown());
	LTBOOL bButtonDown =  g_pLTClient->IsCommandOn( COMMAND_ID_ACTIVATE ) || pClientWeapon->IsFireButtonDown();

	//if we somehow switched weapons... STOP!
	if (m_pGTInfo->m_eTargetType != eINVALID && (g_pPlayerMgr->GetTargetMgr()->RequiredGadgetDamageType() != pAmmoData->eInstDamageType || pClientWeapon->GetState() != W_FIRING))
	{
		bButtonDown = LTFALSE;
	}
		

	// Do we still meet the requirements for disabling?

	if( m_fTimer <= 0.0f || !bButtonDown || !bLookingAtTarget || !bInRange )
	{

		// Send message to target with the amount of time left...

		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_GADGETTARGET );

		cMsg.WriteObject( m_hTargetObj );
		cMsg.Writefloat( m_fTimer);
		g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );

		// Stop disabling...

		m_bDisabling	= LTFALSE;
		m_hTargetObj	= LTNULL;
		m_bShowTimeBar	= LTFALSE;
		g_pPlayerMgr->GetTargetMgr()->LockTarget(NULL);

		g_pPopup->Hide();

		// Let the progress bar hide it's self...

		g_pPlayerStats->UpdateProgress( 0 );
		g_pHUDMgr->QueueUpdate( kHUDProgressBar );
	}

	// Get the percentage of disabling we have done

	uint8 nMaxProgress = GetMaxProgress();	
	uint8 nVal = uint8( (m_fTimer / m_fTotalTime) * nMaxProgress );

	// Update the meter...

	if( m_bShowTimeBar )
	{
		// Show the progress bar...

		g_pPlayerStats->UpdateMaxProgress( g_vtProgressBarScaleToSkills.GetFloat() > 1.0f ? 100 : nMaxProgress );
		g_pPlayerStats->UpdateProgress( nVal );
		g_pHUDMgr->QueueUpdate( kHUDProgressBar );
	}

	// Decipher the code...
	
	if( m_pGTInfo->m_eTargetType == eCodedText )
	{
		if( m_fTimer <= 0 )
		{
			// Once decoded, leave a popup for the player to read
			g_pInterfaceMgr->ShowPopup( m_dwCodeID, 2 );
		}
		else if( m_bDisabling )
		{
			char szString[512] = {0};
			LoadString( m_dwCodeID, szString, sizeof(szString) );
			char *pBody = strchr(szString,'@');
			if (pBody)
			{
				++pBody;
			}
			else
			{
				pBody = szString;
			}

			uint32 i;
			for( i = 0; i < strlen( pBody ); ++i )
			{
				if( pBody[i] != ' ' && pBody[i] != '\n' && pBody[i] != '@' )
				{
					// As the percent of disabling left decreases, decrypt more of the text...

					if( (m_szCodedText[i] != pBody[i]) &&  (GetRandom( GetRandom(0,1), nVal ) == 0) )
						m_szCodedText[i] = pBody[i];
				}
				else
				{
					m_szCodedText[i] = pBody[i];
				}
			}
			m_szCodedText[i] = NULL;

			g_pPopup->Show( 2, m_szCodedText );
		}
	}
	else if( m_pGTInfo->m_eTargetType == eInvisibleInk )
	{
		if( m_fTimer <= 0 )
		{
			// Once visible, leave a popup for the player to read
			g_pInterfaceMgr->ShowPopup( m_dwCodeID, 4 );
		}
		else if( m_bDisabling )
		{
			char szString[512] = {0};
			LoadString( m_dwCodeID, szString, sizeof(szString) );

			POPUP *pPopup = g_pPopupMgr->GetPopup( 4 );
			if( !pPopup )
			{
				m_fTimer = 0;
				return;
			}

			uint8 nA = 0;
			if( nVal >= 25 )
				nA = 100 - nVal;
			else
				// Not to accurate but it looks good
				nA = int(((100.0f - nVal) / 75.0f) * 255) + (100 - nVal);
			
			uint8 a, r, g, b;

			GET_ARGB(pPopup->argbTextColor, a, r, g, b);

			g_pPopup->SetTextColor( r, g, b, nA ) ;
			g_pPopup->Show( 4, szString );

		}
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CGadgetDisabler::OnGadgetTargetMessage
//
//  PURPOSE:	Read in the GadgetTarget's message and start disabling
//
// ----------------------------------------------------------------------- //

void CGadgetDisabler::OnGadgetTargetMessage( ILTMessage_Read *pMsg )
{
	if( !pMsg ) return;

	// Read the data for disabling this Gadget Target...

	m_hTargetObj = pMsg->ReadObject();
	uint8 eType = pMsg->Readuint8(); 
	m_fTotalTime = pMsg->Readfloat();
	LTFLOAT	fTime = pMsg->Readfloat();
	m_dwCodeID = pMsg->Readuint32();

	// Get the Gadget Target Info?

	m_pGTInfo = &GTInfoArray[eType];
	if( !m_pGTInfo ) return;
	
	m_bShowTimeBar = m_pGTInfo->m_bShowTimeBar;

	// Start disabling

	m_fTimer = fTime;
	m_bDisabling = LTTRUE;
	g_pPlayerMgr->GetTargetMgr()->LockTarget(m_hTargetObj);

	// Do any special handeling for the different types...

	if( m_pGTInfo->m_eTargetType == eCodedText )
	{
		// Randomize the coded text but base it off of the structure of the real text...

		memset( m_szCodedText, 0, ARRAY_LEN(m_szCodedText) );

		char szString[512] = {0};
		LoadString( m_dwCodeID, szString, ARRAY_LEN(szString) );

		char szCharSet[512] = {0};
		LoadString( IDS_DECODEMSG_CHARSET, szCharSet, ARRAY_LEN(szCharSet) );
		uint32 nCharSetLen = strlen( szCharSet );
			
		for( uint32 i = 0; i < strlen( szString ); ++i )
		{
			if( szString[i] != ' ' && szString[i] != '\n' && szString[i] != '@' )
			{
				char randChar = 'X';

				// Get a random character from the character set...

				if( szCharSet[0] && nCharSetLen > 0 )
				{
					randChar = szCharSet[GetRandom( 0, nCharSetLen - 1 )];
				}
				 
				m_szCodedText[i] = randChar;
			}
			else
			{
				m_szCodedText[i] = szString[i];
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CGadgetDisabler::GetMaxProgress
//
//  PURPOSE:	Figure out the max allowable progress for the progress bar...
//
// ----------------------------------------------------------------------- //

uint8 CGadgetDisabler::GetMaxProgress()
{
	float fGadgetSkillEffect = g_pPlayerStats->GetSkillModifier(SKL_GADGET,GadgetModifiers::eEffect);

	uint8 nMaxProgress = 100;
	if( g_vtProgressBarScaleToSkills.GetFloat() > 0.0f )
	{
		nMaxProgress = (fGadgetSkillEffect > 0.0f ? uint8(100 / fGadgetSkillEffect) : 0);
	}

	return nMaxProgress;
}


//LTTRUE if the target can be disabled without using a gadget.
LTBOOL	CGadgetDisabler::DisableOnActivate()
{
	if (!m_hTargetObj) return LTFALSE;
	if( !m_pGTInfo ) return LTFALSE;

	return (m_pGTInfo->m_eTargetType == eINVALID);
}