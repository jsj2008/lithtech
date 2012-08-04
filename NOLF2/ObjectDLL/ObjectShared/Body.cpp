// ----------------------------------------------------------------------- //
//
// MODULE  : Body.cpp
//
// PURPOSE : Body Prop - Implementation
//
// CREATED : 1997 (was BodyProp)
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Attachments.h"
#include "Body.h"
#include "iltserver.h"
#include "WeaponFXTypes.h"
#include "ClientServerShared.h"
#include "GameServerShell.h"
#include "SurfaceFunctions.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "SoundMgr.h"
#include "MsgIDs.h"
#include "SFXMsgIDs.h"
#include "SharedFXStructs.h"
#include "CharacterHitBox.h"
#include "AIVolumeMgr.h"
#include "BodyState.h"
#include "PlayerObj.h"
#include "WeaponItems.h"
#include "AIStimulusMgr.h"
#include "AIVolume.h"
#include "AIInformationVolumeMgr.h"
#include "AIUtils.h"
#include "CharacterMgr.h"
#include "Searchable.h"
#include "ServerMissionMgr.h"
#include "VersionMgr.h"

#include <queue>
#include <vector>

const float kfDMPlayerBodyLifetime = 15.0f;

LINKFROM_MODULE( Body );

// Externs

extern CGameServerShell* g_pGameServerShell;
extern CAIStimulusMgr* g_pAIStimulusMgr;
extern CAIInformationVolumeMgr* g_pAIInformationVolumeMgr;

// LT Class Defs

#pragma force_active on
BEGIN_CLASS(Body)
	ADD_SEARCHABLE_AGGREGATE(PF_GROUP(3), 0)
END_CLASS_DEFAULT_FLAGS(Body, Prop, NULL, NULL, CF_HIDDEN)
#pragma force_active off

CMDMGR_BEGIN_REGISTER_CLASS( Body )
CMDMGR_END_REGISTER_CLASS( Body, Prop )

// Static

static LTFLOAT s_fUpdateDelta		= 0.1f;

static char s_szKeyNoise[]	= "NOISE";
static char s_szKeyLand[]	= "LAND";

#define	KEY_BUTE_SOUND	"BUTE_SOUND_KEY"


Body::BodyList Body::m_lstBodies;

static CVarTrack s_vtBodyCapRadius;
static CVarTrack s_vtBodyCapRadiusCount;
static CVarTrack s_vtBodyCapTotalCount;


char* g_szArrowFrontDeath = "DArrow";
static char* s_szArrowFrontSlump = "arrowfall";

char* g_szArrowBackDeath = "DArrowBack";
static char* s_szArrowBackSlump = "arrowfallBack";

CVarTrack g_vtBodyExplosionVelMultiplier;
float g_kfDefaultBodyExplosionVelocity = 3.0f;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Body::Body()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Body::Body() : Prop()
{
	m_pSearch = debug_new( CSearchable );
	AddAggregate(m_pSearch);

	m_vColor.Init();
	m_vDeathDir.Init(0.0f, -1.0f, 0.0f);

	m_eDeathType	= CD_NORMAL;
	m_bFirstUpdate	= LTTRUE;
	m_fStartTime	= 0.0f;
	m_eDamageType	= DT_UNSPECIFIED;

	m_fBodyResetTime= 0.f;

	m_eBodyStatePrevious= kState_BodyNormal;
 	m_eModelNodeLastHit = eModelNodeInvalid;
	m_pAttachments  = LTNULL;
    m_hHitBox       = LTNULL;

	m_eBodyState	= kState_BodyNormal;
    m_pState        = LTNULL;

	m_bMoveToFloor	= LTFALSE;

	m_fLifetime		= -1.0f;
	m_bFadeAfterLifetime = true;

	m_cWeapons = 0;

	m_hChecker		= LTNULL;

	m_cSpears		= 0;

	m_eDeathStimID	= kStimID_Unset;

	m_bCanBeCarried	= true;
	m_bUpdateHitBox = false;
	m_bDimsUpdated = false;
	m_fEnergy = 0.0f;

	for( int nWeapons = 0; nWeapons < ARRAY_LEN( m_ahWeapons ); nWeapons++ )
	{
		m_ahWeapons[nWeapons].SetReceiver( *this );
	}

	for( int nSpears = 0; nSpears < ARRAY_LEN( m_ahSpears ); nSpears++ )
	{
		m_ahSpears[nSpears].SetReceiver( *this );
	}

	m_hCharacter	= LTNULL;

	m_bPermanentBody = false;

	m_bCanCarry = false;
	m_bBeingCarried = false;
	m_bCanRevive = false;

	// Give ourselves touchnotify so we get put into containers.
	m_dwFlags |= FLAG_TOUCH_NOTIFY;

	// Add this instance to a list of all bodies.
	m_lstBodies.push_back( this );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Body::~Body()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

Body::~Body()
{
	ReleasePowerups();

	if (m_pAttachments)
	{
		RemoveAggregate(m_pAttachments);
		CAttachments::Destroy(m_pAttachments);
        m_pAttachments = LTNULL;
	}

	g_pCharacterMgr->RemoveDeathScene(&m_DeathScene);

	if (m_hHitBox)
	{
        g_pLTServer->RemoveObject(m_hHitBox);
	}

	RemovePowerupObjects();

	// Erase this instance from the list of all bodies.
	BodyList::iterator it = m_lstBodies.begin( );
	while( it != m_lstBodies.end( ))
	{
		if( *it == this )
		{
			m_lstBodies.erase( it );
			break;
		}

		it++;
	}

	RemoveAggregate(m_pSearch);
	debug_delete(m_pSearch);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Body::RemovePowerupObjects()
//
//	PURPOSE:	Remove all our pickup objects...
//
// ----------------------------------------------------------------------- //

void Body::RemovePowerupObjects()
{
	for (uint32 iWeapon = 0; iWeapon < kMaxWeapons; iWeapon++)
	{
		if ( m_ahWeapons[iWeapon] )
		{
			HATTACHMENT hAttachment;
			if ( LT_OK == g_pLTServer->FindAttachment(m_hObject, m_ahWeapons[iWeapon], &hAttachment) )
			{
				g_pLTServer->RemoveAttachment(hAttachment);
			}

			g_pLTServer->RemoveObject(m_ahWeapons[iWeapon]);

			m_ahWeapons[iWeapon] = NULL;
		}
	}

	for (uint32 iSpear = 0; iSpear < kMaxSpears; iSpear++)
	{
		if ( m_ahSpears[iSpear] )
		{
			HATTACHMENT hAttachment;
			if ( LT_OK == g_pLTServer->FindAttachment(m_hObject, m_ahSpears[iSpear], &hAttachment) )
			{
				g_pLTServer->RemoveAttachment(hAttachment);
			}

			g_pLTServer->RemoveObject(m_ahSpears[iSpear]);

			m_ahSpears[iSpear] = NULL;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Body::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 Body::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
            uint32 dwRet = Prop::EngineMessageFn(messageID, pData, fData);
			m_pSearch->Enable(false);
			return dwRet;
		}
		break;

		case MID_UPDATE:
		{
			uint32 dwRet = Prop::EngineMessageFn(messageID, pData, fData);

			Update();

			return dwRet;
		}
		break;

		case MID_MODELSTRINGKEY:
		{
			if( HandleModelString((ArgList*)pData))
				return LT_OK;
		}
		break;

		case MID_INITIALUPDATE:
		{
			uint32 dwRet = Prop::EngineMessageFn(messageID, pData, fData);

			// Set up the animator

			m_Animator.Init(m_hObject);

            SetNextUpdate(s_fUpdateDelta);

            // g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_SPY_VISION, USRFLG_SPY_VISION);
			
			//can be moved...
 			g_pCommonLT->SetObjectFlags( m_hObject, OFT_User, USRFLG_CAN_ACTIVATE, USRFLG_CAN_ACTIVATE );

			return dwRet;
		}
		break;

		case MID_TOUCHNOTIFY:
		{
			HOBJECT hObject = (HOBJECT)pData;

			if ( m_pState )
			{
				m_pState->HandleTouch((HOBJECT)pData);
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

			uint32 dwRet = Prop::EngineMessageFn(messageID, pData, fData);

			// We need to reset our sfx message since values
			// could have changed across save versions.
			
			CreateSpecialFX( );
			
			return dwRet;
		}
		break;

		default : break;
	}

	return Prop::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Body::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 Body::ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg)
{
	pMsg->SeekTo(0);
	uint32 messageID = pMsg->Readuint32();
	switch(messageID)
	{
		case MID_DAMAGE:
		{
			DamageStruct damage;
			damage.InitFromMessage(pMsg);

			HandleDamage(damage);
		}
		break;

		default : break;
	}

	uint32 nRet = Prop::ObjectMessageFn(hSender, pMsg);

	if (MID_DAMAGE == messageID)
	{
		// Need to process this after damage aggregate has had a shot at it.  Ignore
		// if the body is dead, since dead objects don't record lastdamagetype.

		if ( !m_damage.IsDead( ) && m_damage.GetLastDamageType() == DT_EXPLODE )
		{
			// Only shoot off if we're not doing anything else and we aren't
			// a permanent body.
			if ( m_eBodyState == kState_BodyNormal && !m_bPermanentBody )
			{
				// Need to set the body's death direction based on this
				// damage (not based on the original death direction)
				//
				
				// It's like we're starting over...so fresh and so clean...
				m_fStartTime = g_pLTServer->GetTime();
				m_vDeathDir = m_damage.GetLastDamageDir();
				m_vDeathDir.Normalize();
				m_vDeathDir *= (1.0f + g_vtBodyExplosionVelMultiplier.GetFloat());

				SetState(kState_BodyExplode);
				SetNextUpdate(s_fUpdateDelta);
			}
		}
	}

	return nRet;

}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Body::OnDeactivate()
//
//	PURPOSE:	Handler for deactivation messages.
//
// --------------------------------------------------------------------------- //

uint32 Body::OnDeactivate()
{
	return super::OnDeactivate();
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Body::OnTrigger()
//
//	PURPOSE:	Handler for trigger messages.
//
// --------------------------------------------------------------------------- //

bool Body::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Splash("SPLASH");
	static CParsedMsg::CToken s_cTok_Move("CARRY");
	static CParsedMsg::CToken s_cTok_Revive( "REVIVE" );
	static CParsedMsg::CToken s_cTok_Search( "SEARCH" );
	static CParsedMsg::CToken s_cTok_DropAtPos( "DROPATPOS" );
	static CParsedMsg::CToken s_cTok_Remove( "REMOVE" );

	// Handle Remove explicitly to avoid attachments getting left around
	if ( cMsg.GetArg(0) == s_cTok_Remove )
	{
		RemovePowerupObjects();
	}

	if (Prop::OnTrigger(hSender, cMsg))
		return true;

	bool bResult = true;

	if ( cMsg.GetArg(0) == s_cTok_Splash )
	{
		SetState(kState_BodyNormal);
		g_pLTServer->SetModelAnimation(m_hObject, g_pLTServer->GetAnimIndex(m_hObject, "DSharkSplash"));
		g_pLTServer->SetModelLooping(m_hObject, LTTRUE);
	}
	else if ( cMsg.GetArg(0) == s_cTok_Move && IsPlayer(hSender))
	{
		CPlayerObj *pPlayer = (CPlayerObj *)g_pLTServer->HandleToObject(hSender);
		
		if (pPlayer && m_bCanBeCarried)
		{
			if (kState_BodyNormal == m_eBodyState)
			{
				pPlayer->SetCarriedObject(m_hObject);
				SetState(kState_BodyCarried);
				SetNextUpdate(UPDATE_NEXT_FRAME);
				m_bFirstUpdate = LTTRUE;

				if (m_eDeathStimID != kStimID_Unset)
					g_pAIStimulusMgr->RemoveStimulus(m_eDeathStimID);
				m_eDeathStimID	= kStimID_Unset;

//				ReleasePowerups(true);

//				for ( uint32 iSpear = 0 ; iSpear < m_cSpears ; iSpear++ )
//				{
//					g_pCommonLT->SetObjectFlags( m_ahSpears[iSpear], OFT_Flags, FLAG_FORCECLIENTUPDATE , FLAG_FORCECLIENTUPDATE );
//					g_pCommonLT->SetObjectFlags( m_ahSpears[iSpear], OFT_User, USRFLG_ATTACH_HIDE1SHOW3, USRFLG_ATTACH_HIDE1SHOW3);
//				}


				ShowObjectAttachments(m_hObject,false);


			}
			else if (kState_BodyCarried == m_eBodyState)
			{
				pPlayer->SetCarriedObject(LTNULL);
				LTVector vPos, vForward, vUp(0.0f,1.0f,0.0f);
				LTRotation rRot;

				g_pLTServer->GetObjectPos(hSender, &vPos);
				g_pLTServer->GetObjectRotation(hSender, &rRot);

				vForward = rRot.Forward();

				LTRotation rNewRot(vForward,vUp);
				rNewRot.Rotate(vUp,MATH_PI);


				g_pLTServer->SetObjectPos(m_hObject, &vPos);
				g_pLTServer->SetObjectPos(m_hHitBox, &vPos);
				g_pLTServer->SetObjectRotation(m_hObject, &rNewRot);
				g_pLTServer->SetObjectRotation(m_hHitBox, &rNewRot);

				if (m_hHitBox)
				{
					CCharacterHitBox* pHitBox = (CCharacterHitBox *)g_pLTServer->HandleToObject(m_hHitBox);
					if (pHitBox)
					{
						pHitBox->SetDimsToModel();
					}
				}				

				SetState(kState_BodyDropped);
				SetNextUpdate(UPDATE_NEXT_FRAME);
				m_bFirstUpdate = LTTRUE;

				ShowObjectAttachments(m_hObject,true);
//				m_pAttachments->HideAttachments(false);
//				for ( uint32 iSpear = 0 ; iSpear < m_cSpears ; iSpear++ )
//				{
//					g_pCommonLT->SetObjectFlags( m_ahSpears[iSpear], OFT_Flags, 0 , FLAG_FORCECLIENTUPDATE );
//					g_pCommonLT->SetObjectFlags( m_ahSpears[iSpear], OFT_User, 0, USRFLG_ATTACH_HIDE1SHOW3);
//				}

			}
		}
	}
	else if( cMsg.GetArg( 0 ) == s_cTok_DropAtPos )
	{
		if( cMsg.GetArgCount() != 4 )
			return true;

		CPlayerObj *pPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject(hSender));
		if( !pPlayer )
			return true;

		LTVector vPos, vF, vUp(0.0f,1.0f,0.0f);
	
		vPos.x = (float)atof( cMsg.GetArg(1).c_str() );
		vPos.y = (float)atof( cMsg.GetArg(2).c_str() );
		vPos.z = (float)atof( cMsg.GetArg(3).c_str() );

		pPlayer->SetCarriedObject(LTNULL);

		LTRotation rRot;
		g_pLTServer->GetObjectRotation(hSender, &rRot);

		vF = rRot.Forward();

		LTRotation rNewRot( vF, vUp );
		rNewRot.Rotate( vUp, MATH_PI );

		g_pLTServer->SetObjectPos(m_hObject, &vPos);
		g_pLTServer->SetObjectPos(m_hHitBox, &vPos);
		g_pLTServer->SetObjectRotation(m_hObject, &rNewRot);
		g_pLTServer->SetObjectRotation(m_hHitBox, &rNewRot);

		if (m_hHitBox)
		{
			CCharacterHitBox* pHitBox = (CCharacterHitBox *)g_pLTServer->HandleToObject(m_hHitBox);
			if (pHitBox)
			{
				pHitBox->SetDimsToModel();
			}
		}				

		SetState(kState_BodyDropped);
		SetNextUpdate(UPDATE_NEXT_FRAME);
		m_bFirstUpdate = LTTRUE;

		ShowObjectAttachments(m_hObject,true);

	}
	else if( cMsg.GetArg( 0 ) == s_cTok_Revive && IsPlayer( hSender ))
	{
		if( IsRevivePlayerGameType( ) && m_hCharacter && IsPlayer( m_hCharacter ) && GetCanRevive( ))
		{
			CPlayerObj *pDeadPlayer = dynamic_cast<CPlayerObj*>( g_pLTServer->HandleToObject( m_hCharacter ));
			if( pDeadPlayer && pDeadPlayer->CanRevive( ))
			{
				// Bring the player back from the dead and get rid of the body...

				pDeadPlayer->Revive();

				CPlayerObj *pReviver = dynamic_cast<CPlayerObj*>( g_pLTServer->HandleToObject( hSender ));
				if (pReviver)
				{
					pReviver->GetPlayerScore()->AddBonus(g_pServerMissionMgr->GetServerSettings().m_nReviveScore);
				}
								
				// Get rid of any weapons or other poerups we had...

				RemovePowerupObjects();

				// Remove the body...
				
				g_pLTServer->RemoveObject( m_hObject );
			}
		}
	}
	else if( cMsg.GetArg( 0 ) == s_cTok_Search && IsPlayer( hSender ))
	{
		// In the case when we're stuck to a wall, if the player searches us, slump to
		// the ground...

		HMODELANIM hAni = g_pLTServer->GetModelAnimation(m_hObject);
		HMODELANIM hArrowFrontDeathAni = g_pLTServer->GetAnimIndex(m_hObject, g_szArrowFrontDeath);
		HMODELANIM hArrowBackDeathAni = g_pLTServer->GetAnimIndex(m_hObject, g_szArrowBackDeath);

		if (hAni == hArrowFrontDeathAni || hAni == hArrowBackDeathAni)
		{

			HMODELANIM hArrowSlumpAni = ( (hAni == hArrowFrontDeathAni) ? 
				g_pLTServer->GetAnimIndex(m_hObject, s_szArrowFrontSlump) : 
				g_pLTServer->GetAnimIndex(m_hObject, s_szArrowBackSlump) );

			if (hArrowSlumpAni != INVALID_ANI)
			{
				g_pLTServer->SetModelAnimation(m_hObject, hArrowSlumpAni);
				g_pLTServer->SetModelLooping(m_hObject, LTFALSE);
			}
		}
	}
#ifndef _FINAL
	//check for messages handled by search aggregate
	else if ( (cMsg.GetArg(0) != "UPDATE") && (cMsg.GetArg(0) != s_cTok_Search) )
	{
        g_pLTServer->CPrint("Unrecognized command (\"%s\")", cMsg.GetArg(0).c_str());
	}
#endif
	else
	{
		bResult = false;
	}

	if (m_hHitBox)
	{
		CCharacterHitBox* pHitBox = (CCharacterHitBox *)g_pLTServer->HandleToObject(m_hHitBox);
		if (pHitBox)
		{
			pHitBox->SetCanBeSearched(CanBeSearched());
		}
	}

	return bResult;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Body::HandleModelString()
//
//	PURPOSE:	Handles model keyframe strings
//
// ----------------------------------------------------------------------- //

bool Body::HandleModelString(ArgList* pArgList)
{
    if (!g_pLTServer || !pArgList || !pArgList->argv || pArgList->argc == 0) 
		return false;

	for(int i=0;i<pArgList->argc;i++)
	{
		char* pKey = pArgList->argv[i];
		if (!pKey) 
			return false;

		LTBOOL bSlump = !_stricmp(pKey, s_szKeyNoise);
		LTBOOL bLand = !_stricmp(pKey, s_szKeyLand);

		if ( bSlump || bLand )
		{
			// Hitting the ground noise
			LTVector vPos;
			g_pLTServer->GetObjectPos(m_hObject, &vPos);

			IntersectQuery IQuery;
			IntersectInfo IInfo;

			IQuery.m_From = vPos;
			IQuery.m_To = vPos - LTVector(0,96,0);
			IQuery.m_Flags = INTERSECT_OBJECTS | INTERSECT_HPOLY | IGNORE_NONSOLID;
			IQuery.m_FilterFn = GroundFilterFn;

			SurfaceType eSurface;

			g_cIntersectSegmentCalls++;
			if (g_pLTServer->IntersectSegment(&IQuery, &IInfo))
			{
				if (IInfo.m_hPoly != INVALID_HPOLY)
				{
					eSurface = (SurfaceType)GetSurfaceType(IInfo.m_hPoly);
				}
				else if (IInfo.m_hObject) // Get the texture flags from the object...
				{
					eSurface = (SurfaceType)GetSurfaceType(IInfo.m_hObject);
				}
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			SURFACE* pSurf = g_pSurfaceMgr->GetSurface(eSurface);
			_ASSERT(pSurf);
			if (!pSurf) 
				return false;

			// Update the noise info. We use a time in the future so AI's don't *instantly* react to the sound
			// AI's twice as sensitive to landing sound (because it's louder)

			m_DeathScene.SetNoise(pSurf->fDeathNoiseModifier * (bLand ? 2.0f : 1.0f), g_pLTServer->GetTime() + GetRandom(0.5f, 1.0f));

			// Register AllyDeathSound stimulus.
			LTFloat fDeathNoiseVolume = pSurf->fDeathNoiseModifier * (bLand ? 2.0f : 1.0f);
			g_pAIStimulusMgr->RegisterStimulus( kStim_AllyDeathSound, 1, m_hObject, LTNULL, m_DeathScene.GetRelationData(), m_DeathScene.GetPosition(), fDeathNoiseVolume, 1.f );
		}
		else if(!_stricmp(pKey,"LIFETIME"))
		{
			if(pArgList->argc > (i+1))
			{
				m_fLifetime = (float)atoi(pArgList->argv[i+1]);
				m_fStartTime = g_pLTServer->GetTime();
				m_bFadeAfterLifetime = false;
			}
		}
		else if(!_stricmp(pKey,"HITBOX_DIMS") && m_hHitBox)
		{
			if(pArgList->argc > (i+2))
			{
				CCharacterHitBox* pHitBox = dynamic_cast<CCharacterHitBox*>(g_pLTServer->HandleToObject(m_hHitBox));
				if( pHitBox )
				{
					HMODELANIM hCurAnim = INVALID_ANI;
					if( LT_OK == g_pModelLT->GetCurAnim( m_hObject, MAIN_TRACKER, hCurAnim ))
					{
						LTVector vDims;
						vDims.x = vDims.z = (float)atoi(pArgList->argv[i+1]);
						vDims.y = (float)atoi(pArgList->argv[i+2]);

						g_pPhysicsLT->SetObjectDims(m_hHitBox, &vDims, 0);
						pHitBox->SetAnimControllingDims( true, hCurAnim );

						UpdateClientHitBox();
					}
				}				
			}
		}
		else if(!_stricmp(pKey,"HITBOX_OFFSET") && m_hHitBox)
		{
			if(pArgList->argc > (i+3))
			{
				CCharacterHitBox* pHitBox = dynamic_cast<CCharacterHitBox*>(g_pLTServer->HandleToObject(m_hHitBox));
				if( pHitBox )
				{
					HMODELANIM hCurAnim = INVALID_ANI;
					if( LT_OK == g_pModelLT->GetCurAnim( m_hObject, MAIN_TRACKER, hCurAnim ))
					{
						LTVector vOffset;
						vOffset.x = (float)atoi(pArgList->argv[i+1]);
						vOffset.y = (float)atoi(pArgList->argv[i+2]);
						vOffset.z = (float)atoi(pArgList->argv[i+3]);

						pHitBox->SetOffset(vOffset);
						pHitBox->SetAnimControllingOffset( true, hCurAnim );

						UpdateClientHitBox();
					}
				}
			}
		}
		else if( !_stricmp(pKey,"HITBOX_DEFAULT") && m_hHitBox )
		{
			CCharacterHitBox* pHitBox = dynamic_cast<CCharacterHitBox*>(g_pLTServer->HandleToObject(m_hHitBox));
			if( pHitBox )
			{
				pHitBox->SetDimsToModel();
			}
		}
		else if( !_stricmp( pKey, KEY_BUTE_SOUND ))
		{
			// Don't let prop have it.  This sound will be played on client.
			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Body::Init()
//
//	PURPOSE:	Setup the object
//
// ----------------------------------------------------------------------- //

void Body::Init(const BODYINITSTRUCT& bi)
{
	if (!bi.pCharacter || !bi.pCharacter->m_hObject) return;

	// Get the death type etc

	m_eDeathType		= bi.pCharacter->GetDeathType();
	m_eModelId			= bi.pCharacter->GetModelId();
	m_eModelSkeleton    = bi.pCharacter->GetModelSkeleton();
	m_hCharacter		= bi.pCharacter->m_hObject;
	m_bCanRevive		= bi.bCanRevive;

	// Get the body lifetime
	m_fLifetime		= bi.fBodyLifetime;
	m_bPermanentBody = bi.bPermanentBody;

	if (!g_vtBodyExplosionVelMultiplier.IsInitted())
	{
		g_vtBodyExplosionVelMultiplier.Init(g_pLTServer, "BodyExplosionMult", NULL, g_kfDefaultBodyExplosionVelocity);
	}

	// Create the SFX
	
	m_BCS.Clear();

	m_BCS.eModelId = m_eModelId;

	m_BCS.bPermanentBody = m_bPermanentBody;


	switch(bi.eBodyState)
	{
		case kState_BodyNormal:
			m_BCS.eBodyState = eBodyStateNormal;
			break;
		case kState_BodyChair:
			m_BCS.eBodyState = eBodyStateChair;
			break;	
		case kState_BodyLedge:
			m_BCS.eBodyState = eBodyStateLedge;
			break;	
		case kState_BodyLadder:
			m_BCS.eBodyState = eBodyStateLadder;
			break;	
		case kState_BodyLaser:
			m_BCS.eBodyState = eBodyStateLaser;
			break;	
		case kState_BodyExplode:
			m_BCS.eBodyState = eBodyStateExplode;
			break;	
		case kState_BodyUnderwater:
			m_BCS.eBodyState = eBodyStateUnderwater;
			break;	
		case kState_BodyStairs:
			m_BCS.eBodyState = eBodyStateStairs;
			break;	
		case kState_BodyDecay:
			m_BCS.eBodyState = eBodyStateDecay;
			break;	
		case kState_BodyCrush:
			m_BCS.eBodyState = eBodyStateCrush;
			break;	
		case kState_BodyPoison:
			m_BCS.eBodyState = eBodyStatePoison;
			break;	
		case kState_BodyAcid:
			m_BCS.eBodyState = eBodyStateAcid;
			break;	
		case kState_BodyArrow:
			m_BCS.eBodyState = eBodyStateArrow;
			break;	
		case kState_BodyFade:
			m_BCS.eBodyState = eBodyStateFade;
			break;	
	}
	
	if (IsPlayer(bi.pCharacter->m_hObject))
	{
		CPlayerObj* pPlayer = (CPlayerObj*) bi.pCharacter;
		m_BCS.nClientId = (uint8) g_pLTServer->GetClientID(pPlayer->GetClient());
	}

	// Create the death scene

	CreateDeathScene(bi.pCharacter);

	// We'll handle creating the necessary debris...

	m_damage.m_bCreatedDebris = LTTRUE;

	m_damage.SetCanDamage( LTFALSE );
	m_damage.SetApplyDamagePhysics(LTFALSE);
	m_damage.SetMass(g_pModelButeMgr->GetModelMass(m_eModelId));

	// Let us get hit by decay powder

	m_damage.ClearCantDamageFlags(DamageTypeToFlag(DT_GADGET_DECAYPOWDER));

	switch ( g_pModelButeMgr->GetModelType(m_eModelId) )
	{
		case eModelTypeVehicle:
		{
			m_eDeathType = CD_GIB;
		}
		break;
	}

	CDestructible* pDest = bi.pCharacter->GetDestructible();
	if (pDest)
	{
		m_eDamageType = pDest->GetDeathType();
		m_vDeathDir = pDest->GetDeathDir();
		m_vDeathDir.Normalize();

		float fAdjustFactor = ( pDest->GetMaxHitPoints() <= 0.0f ? 0.5f :
							    pDest->GetDeathDamage() / pDest->GetMaxHitPoints());

		// If we're using the body golfing cheat, make us fly far...
		if (g_vtBodyExplosionVelMultiplier.GetFloat() > g_kfDefaultBodyExplosionVelocity)
		{
			fAdjustFactor = g_vtBodyExplosionVelMultiplier.GetFloat();
		}

		m_vDeathDir *= (1.0f + fAdjustFactor);

		// Set the energy
		m_fEnergy = pDest->GetEnergy();
	}

	m_pSearch->CopySearchProperties(bi.pCharacter->GetSearchable());

	if (IsPlayer(bi.pCharacter->m_hObject))
	{
		m_pSearch->SetRandomItemSet(NULL);
	}

	m_pSearch->Enable(m_pSearch->HasItem());

	LTFLOAT fHitPts = pDest->GetMaxHitPoints();
	m_damage.Reset(fHitPts, 0.0f, 0.0f);
	m_damage.SetHitPoints(fHitPts);
	m_damage.SetMaxHitPoints(fHitPts);
	m_damage.SetArmorPoints(0.0f);
	m_damage.SetMaxArmorPoints(0.0f);
	m_damage.SetEnergy(0.0f);
	m_damage.SetMaxEnergy(0.0f);	

	// Copy our user flags over, setting our surface type to flesh

	uint32 dwUsrFlags = /*USRFLG_SPY_VISION |*/ SurfaceToUserFlag(g_pModelButeMgr->GetFleshSurfaceType(bi.pCharacter->GetModelId()));
    g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, dwUsrFlags, 0xFF000000 /*| USRFLG_SPY_VISION*/);

	// Make sure model doesn't slide all over the place...
    g_pPhysicsLT->SetFrictionCoefficient(m_hObject, 500.0f);

	// Set our dims...
	LTVector vDims;
	g_pPhysicsLT->GetObjectDims(bi.pCharacter->m_hObject, &vDims);
	g_pPhysicsLT->SetObjectDims(m_hObject, &vDims, 0);


	// Create the box used for weapon impact detection...

	CreateHitBox(bi);

	CCharacterHitBox* pHitBox = dynamic_cast<CCharacterHitBox*>(g_pLTServer->HandleToObject(m_hHitBox));
	if( pHitBox )
	{
		g_pPhysicsLT->GetObjectDims( m_hHitBox, &m_BCS.vHitBoxDims );
		
		m_BCS.vHitBoxOffset = pHitBox->GetOffset();
		m_BCS.bCanBeSearched = pHitBox->CanBeSearched();
	}

	LTFLOAT r, g, b, a; 
    g_pLTServer->GetObjectColor(bi.pCharacter->m_hObject, &r, &g, &b, &a);
    g_pLTServer->SetObjectColor(m_hObject, r, g, b, a);

	LTVector vScale;
    g_pLTServer->GetObjectScale(bi.pCharacter->m_hObject, &vScale);
    g_pLTServer->ScaleObject(m_hObject, &vScale);

	// Copy our animation over

    HMODELANIM hAni = g_pLTServer->GetModelAnimation(bi.pCharacter->m_hObject);
    g_pLTServer->SetModelAnimation(m_hObject, hAni);
    g_pLTServer->SetModelLooping(m_hObject, LTFALSE);
	g_pLTServer->SetModelPlaying( m_hObject, LTTRUE );

	// Match the timing of the animation from the character.
	// This takes care of cases where a character was killed while transitioning
	// into a stunned animation.

	ANIMTRACKERID nTracker;
	if( LT_OK == g_pModelLT->GetMainTracker( bi.pCharacter->m_hObject, nTracker ) )
	{
		uint32 nTime;
		g_pModelLT->GetCurAnimTime( bi.pCharacter->m_hObject, nTracker, nTime );
		nTime += (uint32)( g_pLTServer->GetFrameTime() * 1000.f );

		if( LT_OK == g_pModelLT->GetMainTracker( m_hObject, nTracker ) )
		{
			g_pModelLT->SetCurAnimTime( m_hObject, nTracker, nTime );
		}
	}

	// Copy the flags from the character to us

	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags(bi.pCharacter->m_hObject, OFT_Flags, dwFlags);
	m_dwFlags |= dwFlags | FLAG_REMOVEIFOUTSIDE;
    g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, m_dwFlags, FLAGMASK_ALL);

	// Move the attachments aggregate from the char to us...

	bool bRemoveWeapons		= IsMultiplayerGame() ? !IsPlayer( m_hCharacter ) : true;
	bool bCanSearch			= IsMultiplayerGame() ? !IsPlayer( m_hCharacter ) : true;

	if (!m_pAttachments && bi.eBodyState != eBodyStateLaser)
	{
		bi.pCharacter->TransferWeapons(this, bRemoveWeapons);
	}

	
	if (m_cWeapons)
	{
		bool bAdded = false;
		for ( uint32 iWeapon = 0 ; iWeapon < m_cWeapons ; iWeapon++ )
		{
			bAdded |= m_pSearch->AddPickupItem(m_ahWeapons[iWeapon],true);
		}
		
		if (bAdded && bCanSearch)
		{
			m_pSearch->Enable(true);
		}
	}


	if (!m_pAttachments)
	{
		m_pAttachments = bi.pCharacter->TransferAttachments( bRemoveWeapons );

		if (m_pAttachments)
		{
			AddAggregate(m_pAttachments);
			m_pAttachments->ReInit(m_hObject);

			m_pAttachments->HideAttachments( false );
		}
	}

	// Set up the spears

	bi.pCharacter->TransferSpears(this);

	// General inventory
	m_lstInventory = bi.pCharacter->GetInventory();

	// Set our state
	SetState(bi.eBodyState);

	if( IsPlayer( m_hCharacter) )
	{
		// Players can't be carried.

		m_BCS.bCanBeCarried = m_bCanBeCarried = m_bCanCarry = false;
	}
	else
	{
		m_BCS.bCanBeCarried = m_bCanBeCarried = m_bCanCarry = !!(g_pModelButeMgr->CanModelBeCarried(m_eModelId));
	}

	m_BCS.eDeathDamageType = bi.pCharacter->GetDeathDamageType();	

	m_BCS.bCanBeRevived = m_bCanRevive;

	CreateSpecialFX( );

	// Keep the number of bodies low in a given area.
	if( g_pGameServerShell->IsCapNumberOfBodies( ))
		CapNumberOfBodies( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::CreateFactoryState
//
//	PURPOSE:	Create a state
//
// ----------------------------------------------------------------------- //

CBodyState* Body::AI_FACTORY_NEW_State(EnumAIStateType eStateType)
{
	// Call AI_FACTORY_NEW for the requested type of state.

	switch( eStateType )
	{
		#define STATE_TYPE_AS_SWITCH_BODY 1
		#include "AIStateTypeEnums.h"
		#undef STATE_TYPE_AS_SWITCH_BODY

		default: AIASSERT( 0, m_hObject, "Body::AI_FACTORY_NEW_State: Unrecognized state type.");
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Body::SetState()
//
//	PURPOSE:	Changes our state
//
// ----------------------------------------------------------------------- //

void Body::SetState(EnumAIStateType eBodyState, LTBOOL bLoad /* = LTFALSE */)
{
	m_eBodyStatePrevious = m_eBodyState;
	m_eBodyState = eBodyState;

	if ( m_pState )
	{
		AI_FACTORY_DELETE(m_pState);
        m_pState = LTNULL;
	}

	m_pState = AI_FACTORY_NEW_State( m_eBodyState );

	if ( m_pState )
	{
		if ( !bLoad )
		{
			m_pState->Init(this);
		}
		else
		{
			m_pState->InitLoad(this);
		}
	}

	// Give us an update so we can let the state figure out what to do.
	SetNextUpdate(s_fUpdateDelta);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Body::FaceDir()
//
//	PURPOSE:	Turn to face a specificied direction
//
// ----------------------------------------------------------------------- //

void Body::FaceDir(const LTVector& vTargetDir)
{
    if (!g_pLTServer || !m_hObject) return;

	LTVector vDir = vTargetDir;

	if ( vDir.MagSqr() == 0.0f )
	{
		// Facing the same position... this would be a divide by zero case
		// when we normalize. So just return.
		return;
	}

	vDir.y = 0.0f; // Don't look up/down
	VEC_NORM(vDir);

	LTRotation rRot(vDir, LTVector(0.0f, 1.0f, 0.0f));
	g_pLTServer->SetObjectRotation(m_hObject, &rRot);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Body::FacePos()
//
//	PURPOSE:	Turn to face a specific pos
//
// ----------------------------------------------------------------------- //

void Body::FacePos(const LTVector& vTargetPos)
{
    if (!g_pLTServer || !m_hObject) return;

	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);

	LTVector vDir;
	VEC_SUB(vDir, vTargetPos, vPos);

	if ( vDir.MagSqr() == 0.0f )
	{
		// Facing the same position... this would be a divide by zero case
		// when we normalize. So just return.
		return;
	}

	vDir.y = 0.0f; // Don't look up/down
	VEC_NORM(vDir);

	LTRotation rRot(vDir, LTVector(0.0f, 1.0f, 0.0f));
	g_pLTServer->SetObjectRotation(m_hObject, &rRot);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Body::CreateHitBox()
//
//	PURPOSE:	Create our hit box
//
// ----------------------------------------------------------------------- //

void Body::CreateHitBox(const BODYINITSTRUCT& bi)
{
	if (m_hHitBox) return;

	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);

    HCLASS hClass = g_pLTServer->GetClass("CCharacterHitBox");
	if (!hClass) return;

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	theStruct.m_Pos = vPos;
	g_pLTServer->GetObjectRotation(m_hObject, &theStruct.m_Rotation);

	// Allocate an object...

    CCharacterHitBox* pHitBox = (CCharacterHitBox *)g_pLTServer->CreateObject(hClass, &theStruct);
	if (!pHitBox) return;

	m_hHitBox = pHitBox->m_hObject;

	pHitBox->Init(m_hObject, this);
	pHitBox->SetCanActivate(LTTRUE);
	pHitBox->SetCanBeSearched(CanBeSearched());

	if (m_hHitBox)
	{
		LTVector vDims;
		g_pPhysicsLT->GetObjectDims(m_hObject, &vDims);
		g_pPhysicsLT->SetObjectDims(m_hHitBox, &vDims,0);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Body::Update()
//
//	PURPOSE:	Do update
//
// ----------------------------------------------------------------------- //

void Body::Update()
{
	if ( !m_hObject ) return;

    LTVector vPos, vMin, vMax;
    g_pLTServer->GetWorldBox(vMin, vMax);
    g_pLTServer->GetObjectPos(m_hObject, &vPos);

	if (vPos.x < vMin.x || vPos.y < vMin.y || vPos.z < vMin.z ||
		vPos.x > vMax.x || vPos.y > vMax.y || vPos.z > vMax.z)
	{
		g_pLTServer->RemoveObject( m_hObject );
	}

	// Update the animator

	m_Animator.Update();

	// Make sure our hit box is in the correct position...

	UpdateHitBox();

	if ( m_bFirstUpdate )
	{
		m_bFirstUpdate = LTFALSE;
		m_bDimsUpdated = false;
		m_fStartTime = g_pLTServer->GetTime();
	}

	SetNextUpdate(s_fUpdateDelta);

	// We keep the body active to update dims until the animation has completed...

	uint32 dwFlags;
	g_pModelLT->GetPlaybackState( m_hObject, MAIN_TRACKER, dwFlags );
	LTBOOL bUpdatingDims = !(dwFlags & MS_PLAYDONE);


	// Finalize the dims of our hit box...

	// The deactivate-check, update, activate-check is ordered so that
	// deactivation will always happen one update after you enter a state,
	// but activation will always happen when you just entered the state.
	// This avoids a lot of issues with changing between active/inactive states.

	if ( !bUpdatingDims && !m_bDimsUpdated && (!m_pState || m_pState->CanUpdateDims( )))
	{
		if (m_hHitBox)
		{
			CCharacterHitBox* pHitBox = (CCharacterHitBox*) g_pLTServer->HandleToObject(m_hHitBox);
			if ( pHitBox )
			{
				LTVector vDims, vNewDims;
				
				pHitBox->GetDefaultModelDims( vDims );
				vNewDims = vDims;

				if( !pHitBox->IsAnimControllingDims() )
				{
					// Only shrink us down if we're not sitting or stuck to a wall

					if ( m_eBodyStatePrevious == kState_BodyChair )
					{
						vNewDims.x *= 1.5f;
						vNewDims.z *= 1.5f;
						vNewDims.y = 45.0f;
					}
					else if ( m_eBodyStatePrevious == kState_BodyArrow )
					{
					}
					else
					{
						vNewDims.x *= 2.0f;
						vNewDims.z *= 2.0f;
						vNewDims.y = 15.0f;
					}

					g_pPhysicsLT->SetObjectDims(m_hHitBox, &vNewDims, 0);
				}

				if( !pHitBox->IsAnimControllingOffset() )
				{
					LTVector vOffset(0, vNewDims.y - vDims.y, 0);
					pHitBox->SetOffset(vOffset);
				}

				pHitBox->Update();

				m_bUpdateHitBox = true;
				m_bDimsUpdated = true;
				m_BCS.bHitBoxUpdated = true;
			}
		}

		// Register AllyDeathVisible stimulus.
		AIVolumePlayerInfo* pPlayerInfoVol = LTNULL;
		AIInformationVolume* pVolume = g_pAIInformationVolumeMgr->FindContainingVolume(LTNULL, vPos, eAxisAll, 0.0f);
		if( pVolume && pVolume->GetVolumeType() == AIInformationVolume::eTypePlayerInfo )
		{
			pPlayerInfoVol = (AIVolumePlayerInfo*)pVolume;
		}

		//if we are in an active hiding volume, do not create the stimulus.
		//if body is going to go away on its own, do not create the stimulus.
		if ( !pPlayerInfoVol || !pPlayerInfoVol->IsHiding() || !pPlayerInfoVol->IsOn() && ( m_fLifetime == -1.f ) && !g_pModelButeMgr->AIIgnoreBody(m_eModelId) )
		{
			m_eDeathStimID = g_pAIStimulusMgr->RegisterStimulus( kStim_AllyDeathVisible, 1, m_hObject, LTNULL, m_DeathScene.GetRelationData(), vPos, 1.f, 1.f );
		}

		//set body lifetime here for deathmatch because CanDeactivate() will return false if the lifetime is
		// set prior to this point, and therefore we will never make it into this section
		if( !IsCoopMultiplayerGameType( ) && !GetCanRevive( ))
		{
			if (m_pSearch->HasItem())
				m_fLifetime = kfDMPlayerBodyLifetime;
			else
				m_fLifetime = 0.1f;
		}


		SetNextUpdate(UPDATE_NEVER);


	}

	if (m_hHitBox && m_bUpdateHitBox)
	{
		UpdateClientHitBox();
		m_bUpdateHitBox = false;
	}


	if ( m_pState )
	{
		m_pState->Update();
	}

	if ( bUpdatingDims || (m_pState && !m_pState->CanDeactivate()) )
	{
	    SetNextUpdate(s_fUpdateDelta);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Body::UpdateHitBox()
//
//	PURPOSE:	Update our hit box position
//
// ----------------------------------------------------------------------- //

void Body::UpdateHitBox()
{
	if ( !m_hHitBox ) return;

    CCharacterHitBox* pHitBox = (CCharacterHitBox*) g_pLTServer->HandleToObject(m_hHitBox);
	if ( pHitBox )
	{
		pHitBox->Update();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Body::UpdateClientHitBox()
//
//	PURPOSE:	Update the client's hit box
//
// ----------------------------------------------------------------------- //

void Body::UpdateClientHitBox()
{
	if ( m_hHitBox && m_hObject )
	{
		LTVector vHitDims, vHitOffset(0.0f, 0.0f, 0.0f);
		g_pPhysicsLT->GetObjectDims(m_hHitBox, &vHitDims);

		bool bCanBeSearched = false;

		CCharacterHitBox* pHitBox = dynamic_cast<CCharacterHitBox*>(g_pLTServer->HandleToObject(m_hHitBox));
		if( pHitBox )
		{
			vHitOffset = pHitBox->GetOffset();
			bCanBeSearched = pHitBox->CanBeSearched();

			g_pPhysicsLT->GetObjectDims( m_hHitBox, &m_BCS.vHitBoxDims );
			m_BCS.vHitBoxOffset = pHitBox->GetOffset();
			m_BCS.bCanBeSearched = pHitBox->CanBeSearched();
		}

		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_SFX_MESSAGE);
		cMsg.Writeuint8(SFX_BODY_ID);
		cMsg.WriteObject(m_hObject);
		cMsg.Writeuint8(BFX_HITBOX_MSG);
		cMsg.WriteCompLTVector( vHitDims );
		cMsg.WriteCompLTVector( vHitOffset );
		cMsg.Writebool( bCanBeSearched );
		g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);

	}

	CreateSpecialFX();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Body::HandleDamage()
//
//	PURPOSE:	Handle getting damaged
//
// ----------------------------------------------------------------------- //

void Body::HandleDamage(const DamageStruct& damage)
{
	if ( damage.eType == DT_GADGET_DECAYPOWDER )
	{
		if ( m_eBodyState == kState_BodyNormal )
		{
			SetState(kState_BodyDecay);
			SetNextUpdate(s_fUpdateDelta);
		}
	}
	else if ( damage.fDamage > 0.0f && !damage.hContainer )
	{
		if ( m_eBodyState != kState_BodyLaser )
		{
			// Only twitch when we've "settled"

	        if ( g_pLTServer->GetModelPlaybackState(m_hObject) & MS_PLAYDONE )
			{
				m_Animator.Twitch();
			}
		}
	}

	// Tell our client about the damage so it can do any necessary fx.

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_SFX_MESSAGE);
	cMsg.Writeuint8(SFX_BODY_ID);
	cMsg.WriteObject(m_hObject);
	cMsg.Writeuint8(BFX_DAMAGEFX_MSG);
	cMsg.Writeuint8(damage.eType);
	g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Body::CreateDeathScene()
//
//	PURPOSE:	Create the appropriate kind of death scene
//
// ----------------------------------------------------------------------- //

void Body::CreateDeathScene(CCharacter *pChar)
{
	_ASSERT(pChar);
	if ( !pChar ) return;

	m_DeathScene.Set(pChar, this);
	m_DeathScene.SetPain(pChar->GetLastPainVolume(), pChar->GetLastPainTime());

	g_pCharacterMgr->AddDeathScene(&m_DeathScene);
}


void Body::AddWeapon(HOBJECT hWeapon, char *pszPosition)
{
	if ( m_cWeapons < kMaxWeapons )
	{

		HATTACHMENT hAttachment;
		if ( LT_OK == g_pLTServer->CreateAttachment(m_hObject, hWeapon, pszPosition, &LTVector(0,0,0), &LTRotation(), &hAttachment) )
		{
			m_ahWeapons[m_cWeapons++] = hWeapon;
			return;
		}
	}

	// Unless we actually stuck the Weapon into us, we'll fall through into here.

	g_pLTServer->RemoveObject(hWeapon);

}

bool Body::AddPickupItem(HOBJECT hItem, bool bForcePickup) 
{
	bool bAdded = m_pSearch->AddPickupItem(hItem,bForcePickup);
	if (bAdded)
		m_pSearch->Enable(true);
	return bAdded;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Body::AddSpear
//
//	PURPOSE:	Stick a spear into us
//
// ----------------------------------------------------------------------- //

void Body::AddSpear(HOBJECT hSpear, const LTRotation& rRot, ModelNode eModelNode)
{
	bool bCanSearch	= IsMultiplayerGame() ? !IsPlayer( m_hCharacter ) : true;
	if ( m_cSpears < kMaxSpears )
	{
		eModelNode = eModelNode != eModelNodeInvalid ? eModelNode : m_eModelNodeLastHit;
		char* szNode = (char *)g_pModelButeMgr->GetSkeletonNodeName(m_eModelSkeleton, eModelNode);

		// Get the node transform because we need to make rotation relative

		HMODELNODE hNode;
		if (szNode && LT_OK == g_pModelLT->GetNode(m_hObject, szNode, hNode) )
		{
			LTransform transform;
			if ( LT_OK == g_pModelLT->GetNodeTransform(m_hObject, hNode, transform, LTTRUE) )
			{
				LTRotation rRotNode = transform.m_Rot;

				LTRotation rAttachment = rRotNode.Conjugate()*rRot;
								
				LTVector vSpearDims;

				// Get the dims of the spear from the model animation since the dims may
				// have been adjusted for the pickup box and we want to use the small
				// dims for this calculation...

				uint32 dwAni = g_pLTServer->GetModelAnimation(hSpear);
			 	if (dwAni != INVALID_ANI)
				{
					g_pCommonLT->GetModelAnimUserDims(hSpear, &vSpearDims, dwAni);
				}
				else // use normal dims...
				{
					g_pPhysicsLT->GetObjectDims( hSpear, &vSpearDims );
				}

				LTVector vAttachment = rAttachment.Forward() * -(vSpearDims.z * 2.0f);


				HATTACHMENT hAttachment;
				if ( LT_OK == g_pLTServer->CreateAttachment(m_hObject, hSpear,
					szNode, &vAttachment, &rAttachment, &hAttachment) )
				{
					m_ahSpears[m_cSpears++] = hSpear;

					//flag the spear to be hidden on low violence clients
					g_pCommonLT->SetObjectFlags(hSpear, OFT_User, USRFLG_ATTACH_HIDEGORE, USRFLG_ATTACH_HIDEGORE);


					bool bAdded = m_pSearch->AddPickupItem(hSpear);
		
					if (bAdded && bCanSearch)
					{
						m_pSearch->Enable(true);
					}

					return;
				}
			}
		}
	}

	// Unless we actually stuck the spear into us, we'll fall through into here.

	g_pLTServer->RemoveObject(hSpear);
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Body::AddToObjectList
//
//  PURPOSE:	Add objects we created to the list...
//
// ----------------------------------------------------------------------- //

void Body::AddToObjectList( ObjectList *pObjList, eObjListControl eControl )
{
	if( !pObjList ) return;

	// Send to base first...

	Prop::AddToObjectList( pObjList, eControl );

	// Add members we created...

	AddObjectToList( pObjList, m_hHitBox, eControl );

	for ( uint32 iWeapon = 0 ; iWeapon < kMaxWeapons ; iWeapon++ )
	{
		AddObjectToList( pObjList, m_ahWeapons[iWeapon], eControl );
	}

	

	// Add our attachments...

	if( m_pAttachments )
		m_pAttachments->AddToObjectList( pObjList, eControl );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Body::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Body::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;
	
	// Save the create struct for use with the specialFX message...
	
	m_BCS.Write( pMsg );

	ANIMTRACKERID nTracker;
	if ( LT_OK == g_pModelLT->GetMainTracker(m_hObject, nTracker) )
	{
		uint32 dwTime;
		g_pModelLT->GetCurAnimTime(m_hObject, nTracker, dwTime);
		SAVE_DWORD(dwTime);
	}
	else
	{
		SAVE_DWORD(0);
	}


	SAVE_DWORD(m_eBodyStatePrevious);
	SAVE_HOBJECT(m_hHitBox);
	SAVE_VECTOR(m_vColor);
	SAVE_VECTOR(m_vDeathDir);

	SAVE_TIME(m_fStartTime);
	SAVE_BYTE( (m_bDimsUpdated ? 1 : 0));

	SAVE_FLOAT(m_fLifetime);
	SAVE_bool(m_bFadeAfterLifetime);
	SAVE_FLOAT(m_fEnergy);
	SAVE_BOOL(m_bFirstUpdate);
	SAVE_BYTE(m_eModelId);
	SAVE_BYTE(m_eModelSkeleton);
	SAVE_BYTE(m_eDeathType);
	SAVE_BYTE(m_eDamageType);
	SAVE_DWORD(m_eBodyState);
	SAVE_HOBJECT(m_hChecker);

	SAVE_INT(m_cWeapons);

	for ( uint32 iWeapon = 0 ; iWeapon < kMaxWeapons ; iWeapon++ )
	{
		SAVE_HOBJECT(m_ahWeapons[iWeapon]);
	}

	if ( m_pState )
	{
		m_pState->Save(pMsg);
	}

	SAVE_BOOL(!!m_pAttachments);
	if ( m_pAttachments )
	{
		SAVE_DWORD(m_pAttachments->GetType());
	}

	SAVE_INT(m_cSpears);

	for ( uint32 iSpear = 0 ; iSpear < kMaxSpears ; iSpear++ )
	{
		SAVE_HOBJECT(m_ahSpears[iSpear]);
	}

	m_DeathScene.Save(pMsg);

	SAVE_HOBJECT( m_hCharacter );
	SAVE_bool( m_bPermanentBody );

	SAVE_bool( m_bCanCarry );
	SAVE_bool( m_bBeingCarried );
	SAVE_HOBJECT( m_hCarrier );

	SAVE_bool( m_bCanRevive );

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Body::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Body::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	m_BCS.Read( pMsg );

	uint32 dwTime;
	LOAD_DWORD(dwTime);

	ANIMTRACKERID nTracker;
	if ( LT_OK == g_pModelLT->GetMainTracker(m_hObject, nTracker) )
	{
		g_pModelLT->SetCurAnimTime(m_hObject, nTracker, dwTime);
	}

	LOAD_DWORD_CAST(m_eBodyStatePrevious, EnumAIStateType);
	LOAD_HOBJECT(m_hHitBox);
	LOAD_VECTOR(m_vColor);
	LOAD_VECTOR(m_vDeathDir);
	 
	LOAD_TIME(m_fStartTime);

	uint8 bDimsUpdated;
	LOAD_BYTE(bDimsUpdated);
	m_bDimsUpdated = !!bDimsUpdated;

	LOAD_FLOAT(m_fLifetime);
	LOAD_bool(m_bFadeAfterLifetime);
	LOAD_FLOAT(m_fEnergy);
	LOAD_BOOL(m_bFirstUpdate);
	LOAD_BYTE_CAST(m_eModelId, ModelId);
	LOAD_BYTE_CAST(m_eModelSkeleton, ModelSkeleton);
	LOAD_BYTE_CAST(m_eDeathType, CharacterDeath);
	LOAD_BYTE_CAST(m_eDamageType, DamageType);
	LOAD_DWORD_CAST(m_eBodyState, EnumAIStateType);
	LOAD_HOBJECT(m_hChecker);

	LOAD_INT(m_cWeapons);

	for ( uint32 iWeapon = 0 ; iWeapon < kMaxWeapons ; iWeapon++ )
	{
		LOAD_HOBJECT(m_ahWeapons[iWeapon]);
	}

	SetState(m_eBodyState, LTTRUE);

	if ( m_pState )
	{
		m_pState->Load(pMsg);
	}

	LTBOOL bAttachments;
	LOAD_BOOL(bAttachments);

	if ( bAttachments )
	{
		uint32 dwAttachmentsType;
		LOAD_DWORD(dwAttachmentsType);

		m_pAttachments = CAttachments::Create(dwAttachmentsType);

		_ASSERT(m_pAttachments);

		if (m_pAttachments)
		{
			AddAggregate(m_pAttachments);
		}
	}

	LOAD_INT(m_cSpears);

	for ( uint32 iSpear = 0 ; iSpear < kMaxSpears ; iSpear++ )
	{
		LOAD_HOBJECT(m_ahSpears[iSpear]);
	}

	m_DeathScene.Load(pMsg);

	LOAD_HOBJECT( m_hCharacter );
	LOAD_bool( m_bPermanentBody );

	LOAD_bool( m_bCanCarry );
	LOAD_bool( m_bBeingCarried );
	LOAD_HOBJECT( m_hCarrier );

	if( g_pVersionMgr->GetCurrentSaveVersion( ) < CVersionMgr::kSaveVersion__1_3 )
	{
		m_bCanRevive = false;
	}
	else
	{
		LOAD_bool( m_bCanRevive );
	}

	g_pCharacterMgr->AddDeathScene(&m_DeathScene);

	m_bUpdateHitBox = true;
	SetNextUpdate(s_fUpdateDelta);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimatorBody::Init
//
//	PURPOSE:	Initialize the animator
//
// ----------------------------------------------------------------------- //

void CAnimatorBody::Init(HOBJECT hObject)
{
	CAnimator::Init(hObject);

	// Set up our twitch ani

	m_eAniTrackerTwitch = AddAniTracker("Twitch");
    m_eAniTwitch = AddAni("Twitch");

	EnableAniTracker(m_eAniTrackerTwitch);
    LoopAniTracker(m_eAniTrackerTwitch, LTFALSE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimatorBody::Update
//
//	PURPOSE:	Update the animator
//
// ----------------------------------------------------------------------- //

void CAnimatorBody::Update()
{
	CAnimator::Update();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimatorBody::Twitch
//
//	PURPOSE:	Blend in a twitch animation (if we're not currently doing so)
//
// ----------------------------------------------------------------------- //

void CAnimatorBody::Twitch()
{
    LoopAniTracker(m_eAniTrackerTwitch, LTFALSE);
	SetAni(m_eAniTwitch, m_eAniTrackerTwitch);
}

// ----------------------------------------------------------------------- //

void Body::SetChecker(HOBJECT hChecker)
{
	m_hChecker = hChecker;
}

// ----------------------------------------------------------------------- //

void Body::ReleasePowerup(HOBJECT hPowerup)
{
	
	HATTACHMENT hAttachment;
	if ( LT_OK == g_pLTServer->FindAttachment(m_hObject, hPowerup, &hAttachment) )
	{
		g_pLTServer->RemoveAttachment(hAttachment);

		g_pCommonLT->SetObjectFlags(hPowerup, OFT_Flags, FLAG_TOUCH_NOTIFY,  FLAG_TOUCH_NOTIFY);
/*
		LTVector vBodyPos;
		g_pLTServer->GetObjectPos(m_hObject, &vBodyPos);
		g_pLTServer->SetObjectPos(hPowerup, &vBodyPos);
*/
		LTRotation vBodyRot;
		g_pLTServer->GetObjectRotation(m_hObject, &vBodyRot);
		g_pLTServer->SetObjectRotation(hPowerup, &vBodyRot);




		uint32 dwAni = g_pLTServer->GetAnimIndex(hPowerup, "World");
		if (dwAni != INVALID_ANI)
		{
			LTVector vDims;
			g_pCommonLT->GetModelAnimUserDims(hPowerup, &vDims, dwAni);
			g_pPhysicsLT->SetObjectDims(hPowerup, &vDims, 0);
			g_pLTServer->SetModelAnimation(hPowerup, dwAni);
		}

		HOBJECT hFilterList[] = {m_hObject, m_hHitBox, LTNULL};
		MoveObjectToFloor(hPowerup,hFilterList);

	}
}

// ----------------------------------------------------------------------- //

void Body::ReleasePowerups(bool bWeaponsOnly)
{

	for ( uint32 iWeapon = 0 ; iWeapon < m_cWeapons ; iWeapon++ )
	{
		if (m_ahWeapons[iWeapon])
		{
			ReleasePowerup(m_ahWeapons[iWeapon]);
			m_pSearch->RemovePickupItem(m_ahWeapons[iWeapon]);
			m_ahWeapons[iWeapon] = LTNULL;
		}
	}

	m_cWeapons = 0;

	m_pSearch->ClearPickupItems();

	for ( uint32 iSpear = 0 ; iSpear < kMaxSpears ; iSpear++ )
	{
		if ( m_ahSpears[iSpear] )
		{
			if (bWeaponsOnly)
			{
				m_pSearch->AddPickupItem(m_ahSpears[iSpear]);
			}
			else
			{
				ReleasePowerup(m_ahSpears[iSpear]);
				m_ahSpears[iSpear] = LTNULL;
			}
		}
	}


	m_pSearch->Enable(m_pSearch->HasItem());
}

LTBOOL Body::CanCheckPulse()
{
	if ( m_eBodyState == kState_BodyNormal )
	{
		switch ( m_eBodyStatePrevious )
		{
			case kState_BodyNormal:
			case kState_BodyLedge:
			case kState_BodyLadder:
			case kState_BodyExplode:
			case kState_BodyStairs:
			case kState_BodyCrush:
				return LTTRUE;

			case kState_BodyPoison:
			case kState_BodyAcid:
				// Don't check the pulse unless the body isn't fresh, we might get hit by whatever killed him.
				// Plus we avoid some squirrely animation issues.
				return (g_pLTServer->GetTime() > m_fStartTime + 4.0f);
				break;
		}
	}

	return LTFALSE;
}

bool  Body::CanBeSearched() {return m_pSearch->IsEnabled();}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Body::DropInventoryObject
//
//	PURPOSE:	ReleasePowerups() handles dropping weapons & spears, etc...
//				Since we don't want that, we use DropInventoryObject() 
//				for general inventory. If you're not using the general 
//				inventory, then just ignore this.
//
// ----------------------------------------------------------------------- //
void Body::DropInventoryObject()
{
	// Go through the gameservershell since this is game specific
	g_pGameServerShell->DropInventoryObject(this);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Body::OnLinkBroken
//
//	PURPOSE:	Handle case where other object referred to gets deleted.
//
// ----------------------------------------------------------------------- //
void Body::OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj )
{
	HATTACHMENT hAttachment = NULL;
	if ( LT_OK == g_pLTServer->FindAttachment(m_hObject, hObj, &hAttachment) )
	{
		g_pLTServer->RemoveAttachment(hAttachment);
	}

	Prop::OnLinkBroken( pRef, hObj );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Body::CapNumberOfBodies
//
//	PURPOSE:	Keep body density low where this body is.
//
// ----------------------------------------------------------------------- //

struct BodyDistance
{
	Body*		m_pBody;
	float		m_fDistanceSqr;

	bool operator()( BodyDistance const& a, BodyDistance const& b) const
	{
		// a should be considered "smaller" than b if its distance
		// is less than b.
		return a.m_fDistanceSqr < b.m_fDistanceSqr;
	}
};

void Body::CapNumberOfBodies( )
{
	std::priority_queue< BodyDistance, std::vector< BodyDistance >, BodyDistance > queRadius;
	std::priority_queue< BodyDistance, std::vector< BodyDistance >, BodyDistance > queTotal;

	LTVector vPos;
	g_pLTServer->GetObjectPos( m_hObject, &vPos );

	LTVector vOtherPos;
	BodyDistance bodyDistance;

	// Initialize console variables.
	if(!s_vtBodyCapRadius.IsInitted())
		s_vtBodyCapRadius.Init(g_pLTServer, "BodyCapRadius", NULL, 500.0f);
	if(!s_vtBodyCapRadiusCount.IsInitted())
		s_vtBodyCapRadiusCount.Init(g_pLTServer, "BodyCapRadiusCount", NULL, 4.0f);
	if(!s_vtBodyCapTotalCount.IsInitted())
		s_vtBodyCapTotalCount.Init(g_pLTServer, "BodyCapTotalCount", NULL, 10.0f);

	float fBodyCapRadiusSqr = s_vtBodyCapRadius.GetFloat( );
	fBodyCapRadiusSqr *= fBodyCapRadiusSqr;
	int nBodyCapRadiusCount = ( int )s_vtBodyCapRadiusCount.GetFloat( );
	nBodyCapRadiusCount = Max( 0, nBodyCapRadiusCount );
	int nBodyCapTotalCount = ( int )s_vtBodyCapTotalCount.GetFloat( );
	nBodyCapTotalCount = Max( 0, nBodyCapTotalCount );

	// Iterate through the list of bodies and put them into 2
	// priority queues.  Sorted by farthest at top of queue.
	for( BodyList::iterator iter = m_lstBodies.begin( ); iter != m_lstBodies.end( ); iter++ )
	{
		bodyDistance.m_pBody = *iter;
		if( !bodyDistance.m_pBody )
			continue;

		// Don't remove bodies that are permanent.
		if( bodyDistance.m_pBody->m_bPermanentBody )
			continue;

		// Skip bodies that aren't in the normal state, such as bodies
		// being carried.
		if( bodyDistance.m_pBody->m_eBodyState != kState_BodyNormal )
			continue;
	
		// Get the distance from the new body.
		g_pLTServer->GetObjectPos( bodyDistance.m_pBody->m_hObject, &vOtherPos );
		bodyDistance.m_fDistanceSqr = vPos.DistSqr( vOtherPos );

		// See if it's in range to check for.
		if( bodyDistance.m_fDistanceSqr < fBodyCapRadiusSqr )
		{
			queRadius.push( bodyDistance );
		}

		// Put in complete list of bodies.
		queTotal.push( bodyDistance );
	}

	// Get the number of bodies that will be removed by the radius check.
	int nNumCapByRadius = queRadius.size( ) - nBodyCapRadiusCount;
	nNumCapByRadius = Max( 0, nNumCapByRadius );

	// Remove all the farthest bodies until the density is good.
	int nRadiusCount = nNumCapByRadius;
	while( nRadiusCount > 0 )
	{
		// Get the farthest body in radius.
		BodyDistance const& body = queRadius.top( );

		// Fade the body away.
		body.m_pBody->SetState( kState_BodyFade );

		// Go to the next farthest body.
		queRadius.pop( );

		// Count off this body.
		nRadiusCount--;
	}

	// Keep the whole level capped by removing the very farthest bodies.  Consider
	// bodies removed by radius as not part of the total count.
	int nTotalCount = queTotal.size( ) - nNumCapByRadius - nBodyCapTotalCount;
	while( nTotalCount > 0 )
	{
		// Get the farthest body.
		BodyDistance const& body = queTotal.top( );

		// Fade the body away.
		body.m_pBody->SetState( kState_BodyFade );

		// Go to the next farthest body.
		queTotal.pop( );

		// Count off this body.
		nTotalCount--;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Body::SetCanCarry
//
//	PURPOSE:	Sets the body to be carried or not
//
// ----------------------------------------------------------------------- //

void Body::SetCanCarry( bool bCarry )
{
	if( m_bCanCarry != bCarry )
	{
		m_bCanCarry = (bCarry && m_bCanBeCarried);
		m_BCS.bCanBeCarried = m_bCanCarry;

		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_SFX_MESSAGE);
		cMsg.Writeuint8(SFX_BODY_ID);
		cMsg.WriteObject(m_hObject);
		cMsg.Writeuint8(BFX_CAN_CARRY);
		cMsg.Writebool(m_bCanCarry);
		g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
	}
	
	CreateSpecialFX();
}
		
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Body::SetCanCarry
//
//	PURPOSE:	Sets the body to be carried or not
//
// ----------------------------------------------------------------------- //

void Body::SetBeingCarried( bool bBeingCarried, HOBJECT hCarrier ) 
{ 
	m_bBeingCarried = bBeingCarried; 
	m_hCarrier = hCarrier;

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_SFX_MESSAGE);
	cMsg.Writeuint8(SFX_BODY_ID);
	cMsg.WriteObject(m_hObject);
	cMsg.Writeuint8(BFX_CARRIED);
	cMsg.Writebool(m_bBeingCarried);
	cMsg.WriteObject(m_hCarrier);
	g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);

	CreateSpecialFX();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Body::SetPermanentBody
//
//	PURPOSE:	Sets the body to be permanent
//
// ----------------------------------------------------------------------- //

void Body::SetPermanentBody( bool bPermanent )
{ 
	if( m_bPermanentBody == bPermanent )
		return;

	m_bPermanentBody = bPermanent;
	m_BCS.bPermanentBody = bPermanent;

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_SFX_MESSAGE);
	cMsg.Writeuint8(SFX_BODY_ID);
	cMsg.WriteObject(m_hObject);
	cMsg.Writeuint8(BFX_PERMANENT);
	cMsg.Writebool(m_bPermanentBody);
	g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);

	CreateSpecialFX();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Body::ResetLifetime
//
//	PURPOSE:	Restarts the body's lifetime
//
// ----------------------------------------------------------------------- //

void Body::ResetLifetime(float fLifetime)
{
	m_fStartTime = g_pLTServer->GetTime();
	m_fLifetime = fLifetime;
	SetNextUpdate(s_fUpdateDelta);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Body::SetCanRevive
//
//	PURPOSE:	Sets the body to be revivable
//
// ----------------------------------------------------------------------- //

void Body::SetCanRevive( bool bCanRevive )
{ 
	if( m_bCanRevive == bCanRevive )
		return;

	m_bCanRevive = bCanRevive;
	m_BCS.bCanBeRevived = bCanRevive;

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_SFX_MESSAGE);
	cMsg.Writeuint8(SFX_BODY_ID);
	cMsg.WriteObject(m_hObject);
	cMsg.Writeuint8(BFX_CANREVIVE);
	cMsg.Writebool(m_bCanRevive);
	g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);

	CreateSpecialFX();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Body::CreateSpecialFX
//
//	PURPOSE:	Sets the specialfx message ofr this body...
//
// ----------------------------------------------------------------------- //

void Body::CreateSpecialFX()
{
	CAutoMessage cMsg;
	cMsg.Writeuint8(SFX_BODY_ID);
	m_BCS.Write(cMsg);
	g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Body::HideBody
//
//	PURPOSE:	Hide the body and it's associated objects.
//
// ----------------------------------------------------------------------- //

void Body::HideBody( bool bHide )
{
	uint32 nFlags = ( bHide ) ? 0 : ( FLAG_VISIBLE | FLAG_RAYHIT | FLAG_TOUCH_NOTIFY );
	g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, nFlags, ( FLAG_VISIBLE | FLAG_RAYHIT | FLAG_TOUCH_NOTIFY ));

	// Setup our hitbox.
	nFlags = ( bHide ) ? 0 : ( FLAG_RAYHIT | FLAG_TOUCHABLE );
	g_pCommonLT->SetObjectFlags( m_hHitBox, OFT_Flags, nFlags, ( FLAG_RAYHIT | FLAG_TOUCHABLE ));

	// Setup our attachments.
	ShowObjectAttachments( m_hObject, !bHide );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Body::HandleDestroy
//
//	PURPOSE:	Destroy the body
//
// ----------------------------------------------------------------------- //

void Body::HandleDestroy(HOBJECT hDamager)
{
// Don't allow killing of revivable body.
/*
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, 0, USRFLG_CAN_ACTIVATE);
	m_damage.SetCanDamage(LTFALSE);

	// If this body is revivable and belongs to the player, then tell the player the body died.
	if( IsRevivePlayerGameType( ) && GetCanRevive( ))
	{
		SetCanRevive( false );
		CPlayerObj *pDeadPlayer = dynamic_cast<CPlayerObj*>( g_pLTServer->HandleToObject( m_hCharacter ));
		if( pDeadPlayer )
		{
			pDeadPlayer->CreateRespawnBody( );
		}
	}
*/
}

