// ----------------------------------------------------------------------- //
//
// MODULE  : Prop.cpp
//
// PURPOSE : Model Prop - Definition
//
// CREATED : 10/9/97
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Prop.h"
#include "ServerUtilities.h"
#include "ClientServerShared.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "ModelButeMgr.h"
#include "PlayerObj.h"
#include "SoundMgr.h"
#include "libltphysics.h"
#include "AIState.h"
#include "CharacterHitBox.h"
#include "PropTypeMgr.h"
#include "ServerSoundMgr.h"
#include "AIStimulusMgr.h"
#include "Explosion.h"
#include "Attachments.h"

extern CAIStimulusMgr* g_pAIStimulusMgr;

#define	KEY_BUTE_SOUND	"BUTE_SOUND_KEY"
#define KEY_COMMAND		"CMD"

LINKFROM_MODULE( Prop );


#pragma force_active on
BEGIN_CLASS(Prop)
	ADD_DESTRUCTIBLE_MODEL_AGGREGATE(PF_GROUP(1), 0)
	ADD_STRINGPROP_FLAG(Filename, "", PF_DIMS | PF_LOCALDIMS | PF_FILENAME | PF_MODEL)
	ADD_STRINGPROP_FLAG(Skin, "", PF_FILENAME)
	ADD_VECTORPROP_VAL(Scale, 1.0f, 1.0f, 1.0f)
	ADD_VISIBLE_FLAG(1, 0)
	ADD_SOLID_FLAG(0, 0)
	ADD_GRAVITY_FLAG(0, 0)
	ADD_SHADOW_FLAG(0, 0)
    ADD_BOOLPROP(MoveToFloor, LTTRUE)
	ADD_REALPROP(Alpha, 1.0f)
	ADD_COLORPROP(ObjectColor, 255.0f, 255.0f, 255.0f)
    ADD_BOOLPROP(Additive, LTFALSE)
    ADD_BOOLPROP(Multiply, LTFALSE)
    ADD_BOOLPROP(RayHit, LTTRUE)
	ADD_STRINGPROP_FLAG(DetailLevel, "Low", PF_STATICLIST)
	ADD_STRINGPROP_FLAG(CommandOn, "", PF_NOTIFYCHANGE )
	ADD_STRINGPROP_FLAG(CommandOff, "", PF_NOTIFYCHANGE )
	ADD_BOOLPROP_FLAG(CanTransition, LTTRUE, 0)
END_CLASS_DEFAULT_FLAGS_PLUGIN(Prop, GameBase, NULL, NULL, 0, CPropPlugin)
#pragma force_active off

//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( Prop )

	CMDMGR_ADD_MSG( ANIM, 2, NULL, "ANIM <ani name>" )			// We dont know if the animation is valid or not 
	CMDMGR_ADD_MSG( ANIMLOOP, 2, NULL, "ANIMLOOP <ani name>" )
	CMDMGR_ADD_MSG( ACTIVATE, 1, NULL, "ACTIVATE" )
	CMDMGR_ADD_MSG( REMOVE, 1, NULL, "REMOVE" )
	CMDMGR_ADD_MSG( DESTROY, 1, NULL, "DESTROY" )

CMDMGR_END_REGISTER_CLASS( Prop, GameBase )

#ifndef __PSX2
LTRESULT CPropPlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char** aszStrings,
    uint32* pcStrings,
    const uint32 cMaxStrings,
    const uint32 cMaxStringLength)
{
	if (m_DestructibleModelPlugin.PreHook_EditStringList(szRezPath,
		szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength) == LT_OK)
	{
		return LT_OK;
	}
	else if (_strcmpi("DetailLevel", szPropName) == 0)
	{
		strcpy(aszStrings[(*pcStrings)++], "Low");
		strcpy(aszStrings[(*pcStrings)++], "Medium");
		strcpy(aszStrings[(*pcStrings)++], "High");
		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

LTRESULT CPropPlugin::PreHook_PropChanged( const char *szObjName, 
										   const char *szPropName,
										   const int nPropType,
										   const GenericProp &gpPropValue,
										   ILTPreInterface *pInterface,
										   const char *szModifiers )
{
	// Since we don't have any props that need notification, just pass it to the Destructible model...

	if( LT_OK == m_DestructibleModelPlugin.PreHook_PropChanged( szObjName, szPropName, nPropType, gpPropValue, pInterface, szModifiers ))
	{
		return LT_OK;
	}

	// Only our commands are marked for change notification so just send it to the CommandMgr..

	if( m_CommandMgrPlugin.PreHook_PropChanged( szObjName, 
												szPropName, 
												nPropType, 
												gpPropValue,
												pInterface,
												szModifiers ) == LT_OK )
	{
		return LT_OK;
	}

	return LT_UNSUPPORTED;
}
#endif

#define ANIM_WORLD	"Idle"
#define ANIM_TOUCH	"Touch"
#define ANIM_KNOCK	"Knock"
#define ANIM_HIT	"Hit"

#define DEFAULT_ATTACH_MIN_VEL	64.0f
#define DEFAULT_ATTACH_MAX_VEL	256.0f
#define DEFAULT_ATTACH_MIN_YADD	128.0f
#define DEFAULT_ATTACH_MAX_YADD	256.0f
#define DEFAULT_ATTACH_FADE_DELAY	5.0f
#define DEFAULT_ATTACH_FADE_DURATION 1.0f	

static CVarTrack	s_vtAttachmentMinVel;
static CVarTrack	s_vtAttachmentMaxVel;
static CVarTrack	s_vtAttachmentMinYAdd;
static CVarTrack	s_vtAttachmentMaxYAdd;
static CVarTrack	s_vtAttachmentFadeDelay;
static CVarTrack	s_vtAttachmentFadeDuration;


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PropDisturbStruct::PropDisturbStruct()
//
//	PURPOSE:	Con/Destructor
//
// ----------------------------------------------------------------------- //

PropDisturbStruct::PropDisturbStruct()
{
    hTouchSound			= LTNULL;
	hHitSound			= LTNULL;
	hTouchAnim			= INVALID_ANI;
	hHitAnim			= INVALID_ANI;
	pPD					= LTNULL;
}

PropDisturbStruct::~PropDisturbStruct()
{
	if (hTouchSound)
	{
        g_pLTServer->SoundMgr()->KillSound(hTouchSound);
        hTouchSound = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PropDisturbStruct::Save/Load()
//
//	PURPOSE:	Save/Load
//
// ----------------------------------------------------------------------- //

void PropDisturbStruct::Save(ILTMessage_Write *pMsg)
{
	SAVE_CHARSTRING( sTouchSoundName.c_str( ) );
	SAVE_DWORD( eTouchAnimType );
	SAVE_DWORD( hTouchAnim );
	SAVE_DWORD( hHitAnim );

	if( pPD )
	{
		SAVE_DWORD( pPD->nPropTypeId );
	}
	else 
	{
		SAVE_DWORD( -1 );
	}
}

void PropDisturbStruct::Load(ILTMessage_Read *pMsg)
{
	char szBuf[256];
	LOAD_CHARSTRING( szBuf, ARRAY_LEN( szBuf ) );
	sTouchSoundName = szBuf;
	LOAD_DWORD_CAST( eTouchAnimType, EnumPropAnimationType );
	LOAD_DWORD( hTouchAnim );
	LOAD_DWORD( hHitAnim );

	uint32 nPropTypeId;
	LOAD_DWORD(nPropTypeId);
	if( nPropTypeId != -1 )
	{
		PROPTYPE* pPropType = g_pPropTypeMgr->GetPropType( nPropTypeId );
		UBER_ASSERT( pPropType, "PropDisturbStruct::Load: Could not find proptype" );
		pPD = pPropType->pDisturb;
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::Prop()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Prop::Prop() : GameBase(OT_MODEL)
{
    m_bMoveToFloor		= LTTRUE;
	m_bFirstUpdate		= LTTRUE;
	m_bCanDeactivate	= true;

	m_vScale.Init(1.0f, 1.0f, 1.0f);
	m_vObjectColor.Init(255.0f, 255.0f, 255.0f);
	m_fAlpha = 1.0f;

	m_dwUsrFlgs = USRFLG_MOVEABLE;

	m_dwFlags	= FLAG_DONTFOLLOWSTANDING;
	m_dwFlags2	= 0;

	m_bTouchable = LTTRUE;

	m_pDisturb = LTNULL;

	m_eState = kState_PropDefault;

	m_pDebrisOverride	= LTNULL;

	// Used only for sending event trigger messages.
	m_bActivatedOn = false;

	AddAggregate(&m_damage);
	MakeTransitionable();

	m_bAttachmentShotOff	= false;
	m_hAttachmentOwner		= LTNULL;

	m_fPitch	= 0.0f;
	m_fYaw		= 0.0f;
	m_fRoll		= 0.0f;
	m_fPitchVel	= 0.0f;
	m_fYawVel	= 0.0f;
	m_fRollVel	= 0.0f;

	m_bRotatedToRest	= false;
	m_bRotating			= false;

	m_fFadeStartTime		= 0.0f;
	m_fFadeDuration			= 0.0f;
	m_fStartAlpha			= 0.0f;
	m_fEndAlpha				= 0.0f;
	m_bFading				= false;	
	m_bFadeRemoveWhenDone	= false;
	
	m_bCanTransition		= true;

	m_ActivateTypeHandler.Init( this );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::~Prop()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

Prop::~Prop()
{
	ClearTouchSoundIfDone( LTTRUE );
	ClearHitSoundIfDone( LTTRUE );

	debug_delete(m_pDisturb);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 Prop::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_TOUCHNOTIFY:
		{
			if(m_bTouchable)
			{
				HOBJECT hToucher = (HOBJECT)pData;
				if( IsCharacterHitBox( hToucher ) )
				{
					CCharacterHitBox* pHB = (CCharacterHitBox*)g_pLTServer->HandleToObject(hToucher);
					hToucher = pHB->GetModelObject();
				}

				// Could be a body.
				if( IsCharacter(hToucher) )
				{
					HandleTouch( hToucher );
				}
			}
			else if( m_bAttachmentShotOff )
			{
				HandleAttachmentTouch( (HOBJECT)pData );
			}
		}
		break;

		case MID_UPDATE:
		{
			Update();
		}
		break;

		case MID_MODELSTRINGKEY :
		{
			HandleModelString( (ArgList*)pData );
		}
		break;

		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
				ReadProp(pStruct);

				// If this prop is spawned, assume it should be visible (if they
				// specify Visible 0, our parent class will handle it ;)

				if (fData == PRECREATE_STRINGPROP)
				{
					m_dwFlags |= FLAG_VISIBLE;
				}
			}

			// We must remove aggregates before sending the message to the base classs...

			if( !m_bCanTransition )
			{
				// Disallow transitioning of this object through TransAMs...

				DestroyTransitionAggregate();
			}

            uint32 dwRet = GameBase::EngineMessageFn(messageID, pData, fData);

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
//	ROUTINE:	Prop::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 Prop::ObjectMessageFn( HOBJECT hSender, ILTMessage_Read *pMsg )
{
	pMsg->SeekTo(0);
	uint32 messageID = pMsg->Readuint32();

	switch(messageID)
	{
		case MID_DAMAGE:
		{
			// Let our damage aggregate process the message first...

            uint32 dwRet = GameBase::ObjectMessageFn(hSender, pMsg);

			// Check to see if we have been destroyed

			if ( m_damage.IsDead() )
			{
				HandleDestroy(m_damage.GetLastDamager());
			}
			else 
			{
				HandleHit( m_damage.GetLastDamager() );
			}

			return dwRet;
		}

		default : break;
	}

	return GameBase::ObjectMessageFn(hSender, pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::PropRead()
//
//	PURPOSE:	Update properties
//
// ----------------------------------------------------------------------- //

void Prop::ReadProp(ObjectCreateStruct *pData)
{
	if (!pData) return;

	GenericProp genProp;

    if (g_pLTServer->GetPropGeneric("Alpha", &genProp) == LT_OK)
	{
		m_fAlpha = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("ObjectColor", &genProp) == LT_OK)
	{
		m_vObjectColor = genProp.m_Vec;
	}

    if (g_pLTServer->GetPropGeneric("MoveToFloor", &genProp) == LT_OK)
	{
		 m_bMoveToFloor = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("Scale", &genProp) == LT_OK)
	{
		 m_vScale = genProp.m_Vec;
	}

    if (g_pLTServer->GetPropGeneric("Additive", &genProp) == LT_OK)
	{
		if (genProp.m_Bool)
		{
			m_dwFlags  |= FLAG_NOLIGHT;
		}
	}

    if (g_pLTServer->GetPropGeneric("Multiply", &genProp) == LT_OK)
	{
		if (genProp.m_Bool)
		{
			m_dwFlags  |= FLAG_NOLIGHT;
		}
	}

    if (g_pLTServer->GetPropGeneric("RayHit", &genProp) == LT_OK)
	{
		if (genProp.m_Bool)
		{
			m_dwFlags |= FLAG_RAYHIT;

			// Set touch notify so projectiles can impact with us...
			m_dwFlags |= FLAG_TOUCH_NOTIFY;
		}
	}

    if (g_pLTServer->GetPropGeneric("DetailLevel", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			// TBD!!!
		}
	}

	if( g_pLTServer->GetPropGeneric( "CommandOn", &genProp ) == LT_OK )
	{
		if( genProp.m_String[0] )
		{
			m_sActivateOnCommand = genProp.m_String;
		}
	}

	if( g_pLTServer->GetPropGeneric( "CommandOff", &genProp ) == LT_OK )
	{
		if( genProp.m_String[0] )
		{
			m_sActivateOffCommand = genProp.m_String;
		}
	}

	if( g_pLTServer->GetPropGeneric( "CanTransition", &genProp ) == LT_OK )
	{
		m_bCanTransition = genProp.m_Bool;
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::PostPropRead()
//
//	PURPOSE:	Update properties
//
// ----------------------------------------------------------------------- //

void Prop::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	// Remove if outside the world

	m_dwFlags |= FLAG_REMOVEIFOUTSIDE | FLAG_MODELKEYS;

	// If this prop is one that shouldn't stop the player, set the appropriate
	// flag...

	if (m_damage.GetMass() < g_pModelButeMgr->GetModelMass(g_pModelButeMgr->GetModelId(DEFAULT_PLAYERNAME)))
	{
		m_dwFlags |= FLAG_CLIENTNONSOLID;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::InitialUpdate()
//
//	PURPOSE:	Handle initial update
//
// ----------------------------------------------------------------------- //

void Prop::InitialUpdate()
{
	Setup( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::Setup()
//
//	PURPOSE:	Sets up prop based on member variables.
//
// ----------------------------------------------------------------------- //

bool Prop::Setup( )
{
	// Set Initial flags...(NOTE: The m_dwXXX variables are used so subclasses
	// can adjust the intial state of the prop)

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, m_dwUsrFlgs, m_dwUsrFlgs);
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, m_dwFlags, m_dwFlags);
    g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags2, m_dwFlags2, m_dwFlags2);


	// Set object translucency...

	VEC_DIVSCALAR(m_vObjectColor, m_vObjectColor, 255.0f);
    g_pLTServer->SetObjectColor(m_hObject, m_vObjectColor.x, m_vObjectColor.y,
								m_vObjectColor.z, m_fAlpha);


	// Default us to our "world" animation...
	// Pick a random world animation from the list.

	uint32 iWorldAnim = 1;
	char szWorldAnim[32];
	sprintf( szWorldAnim, "%s%d", ANIM_WORLD, iWorldAnim++ );
	HMODELANIM hWorldAnim = g_pLTServer->GetAnimIndex( m_hObject, szWorldAnim );
	while( hWorldAnim != INVALID_ANI )
	{
		m_lstWorldAnims.push_back( hWorldAnim );
		sprintf( szWorldAnim, "%s%d", ANIM_WORLD, iWorldAnim++ );
		hWorldAnim = g_pLTServer->GetAnimIndex( m_hObject, szWorldAnim );
	}
	PlayRandomWorldAnim();

	// Look for touch animation.

	HMODELANIM hAnim = g_pLTServer->GetAnimIndex(m_hObject, ANIM_TOUCH);
	if( hAnim != INVALID_ANI )
	{
		if(m_pDisturb == LTNULL)
		{
			m_pDisturb = debug_new(PropDisturbStruct);
		}
		m_pDisturb->hTouchAnim = hAnim;
		m_pDisturb->eTouchAnimType = kPA_Touch;

		AIASSERT( g_pLTServer->GetAnimIndex(m_hObject, ANIM_KNOCK) == INVALID_ANI, m_hObject, "Prop::InitialUpdate: Prop cannot have both TOUCH and KNOCK animations." );
	}
	else  // Look for knock animation.
	{
		hAnim = g_pLTServer->GetAnimIndex(m_hObject, ANIM_KNOCK);
		if( hAnim != INVALID_ANI )
		{
			if(m_pDisturb == LTNULL)
			{
				m_pDisturb = debug_new(PropDisturbStruct);
			}
			m_pDisturb->hTouchAnim = hAnim;
			m_pDisturb->eTouchAnimType = kPA_Knock;
		}
	}

	// Look for hit animation...

	hAnim = g_pLTServer->GetAnimIndex( m_hObject, ANIM_HIT );
	if( hAnim != INVALID_ANI )
	{
		if( m_pDisturb == LTNULL )
		{
			m_pDisturb = debug_new( PropDisturbStruct );
		}

		m_pDisturb->hHitAnim = hAnim;
	}

	// Set the dims based on the current animation...

    LTVector vDims;
    g_pCommonLT->GetModelAnimUserDims(m_hObject, &vDims, g_pLTServer->GetModelAnimation(m_hObject));

	// Set object dims based on scale value...

    LTVector vNewDims;
	vNewDims.x = m_vScale.x * vDims.x;
	vNewDims.y = m_vScale.y * vDims.y;
	vNewDims.z = m_vScale.z * vDims.z;

    g_pLTServer->ScaleObject(m_hObject, &m_vScale);
	g_pPhysicsLT->SetObjectDims(m_hObject, &vNewDims, 0);


	// Only need to update if we're moving the object to the floor...Else, disable
	// updates so the object isn't eating ticks...

	SetNextUpdate((m_bMoveToFloor || !m_bCanDeactivate) ? UPDATE_NEXT_FRAME : UPDATE_NEVER);


	// See if we should force our debris aggregate to use a specific
	// debris type...

	if (m_pDebrisOverride && *m_pDebrisOverride)
	{
		DEBRIS* pDebris = g_pDebrisMgr->GetDebris(m_pDebrisOverride);
		if (pDebris)
		{
			m_damage.m_nDebrisId = pDebris->nId;
		}
	}

	// Send the Activate SpecialFX message.  If we never set the Activate Type no message will get sent...
	
	m_ActivateTypeHandler.SetState( ACTIVATETYPE::eOn, false );
	m_ActivateTypeHandler.SetDisabled( false, false );
	m_ActivateTypeHandler.CreateActivateTypeMsg();

	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::OnTrigger()
//
//	PURPOSE:	Handler for prop trigger messages.
//
// --------------------------------------------------------------------------- //

bool Prop::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Anim("ANIM");
	static CParsedMsg::CToken s_cTok_AnimLoop("ANIMLOOP");
	static CParsedMsg::CToken s_cTok_Activate("ACTIVATE");
	static CParsedMsg::CToken s_cTok_Remove("REMOVE");
	static CParsedMsg::CToken s_cTok_Destroy("DESTROY");

	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags(m_hObject, OFT_User, dwFlags);

	if ( cMsg.GetArg(0) == s_cTok_Anim )
	{
        g_pLTServer->SetModelLooping(m_hObject, LTFALSE);
        g_pLTServer->SetModelAnimation(m_hObject, g_pLTServer->GetAnimIndex(m_hObject, ( char* )( char const* )cMsg.GetArg( 1 )));
		g_pLTServer->ResetModelAnimation(m_hObject);
		SetNextUpdate(UPDATE_NEXT_FRAME); // Needed to get string keys
	}
	else if ( cMsg.GetArg(0) == s_cTok_AnimLoop )
	{
        g_pLTServer->SetModelLooping(m_hObject, LTTRUE);
        g_pLTServer->SetModelAnimation(m_hObject, g_pLTServer->GetAnimIndex(m_hObject, ( char* )( char const* )cMsg.GetArg( 1 )));
		g_pLTServer->ResetModelAnimation(m_hObject);
		SetNextUpdate(UPDATE_NEXT_FRAME); // Needed to get string keys
	}
	else if ( cMsg.GetArg(0) == s_cTok_Activate )
	{
		if(dwFlags & USRFLG_CAN_ACTIVATE)
		{
			SendActivateMessage( );

 			HandleTouch(hSender);
		}
	}
	else if( cMsg.GetArg(0) == s_cTok_Remove )
	{
		// Remove us...

		g_pLTServer->RemoveObject( m_hObject );
	}
	else if( cMsg.GetArg(0) == s_cTok_Destroy )
	{
		m_damage.HandleDestruction( LTNULL );
		HandleDestroy( LTNULL );
	}
	else
	{
		return GameBase::OnTrigger(hSender, cMsg);
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::HandleTouch
//
//	PURPOSE:	Handle touch notifies
//
// ----------------------------------------------------------------------- //

void Prop::HandleTouch(HOBJECT hToucher)
{
	// Only characters can touch props.

	if( ( hToucher != LTNULL ) &&
		( !IsCharacter(hToucher) ) &&
		( !IsExplosion(hToucher) ) )
	{
		return;
	}

	if ( (m_eState == kState_PropDestroyed)
		|| (m_eState == kState_PropKnocked) 
		|| (!(m_pDisturb && m_pDisturb->pPD)) )
	{
		return;
	}

	// Resolve the toucher to a character handle.

	HOBJECT hCharacter = LTNULL;
	if( IsCharacterHitBox( hToucher ) )
	{
		CCharacterHitBox* pHB = (CCharacterHitBox*)g_pLTServer->HandleToObject(hToucher);
		hCharacter = pHB->GetModelObject();
	}
	else if( IsExplosion( hToucher ) )
	{
		Explosion* pExplosion = (Explosion*)g_pLTServer->HandleToObject(hToucher);
		hCharacter = pExplosion->GetFiredFrom();
	}
	else if( IsCharacter( hToucher ) ) 
	{
		hCharacter = hToucher;
	}


	// Only players can touch (for now).
	if((hCharacter != LTNULL) && !IsPlayer(hCharacter)) return;

	if(m_eState == kState_PropTouching)
	{
		// Clear sound if done playing.

		ClearTouchSoundIfDone(LTFALSE);

		// Check if both sound and animation are done, or just the sound for a "knock."

		if( m_pDisturb->hTouchSound == LTNULL )
		{
			switch( m_pDisturb->eTouchAnimType )
			{
				// Touch animations reset.

				case kPA_Touch:
					ClearTouchAnimIfDone(m_pDisturb->hTouchAnim, LTFALSE);
					if( g_pLTServer->GetModelAnimation(m_hObject) != m_pDisturb->hTouchAnim )
					{
						m_eState = kState_PropDefault;
						return;
					}
					break;

				// Knock animations remain on the last frame forever.

				case kPA_Knock:
					if( MS_PLAYDONE & g_pLTServer->GetModelPlaybackState(m_hObject) )
					{
						m_bTouchable = LTFALSE;

						g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, 0, USRFLG_CAN_ACTIVATE);

						m_eState = kState_PropKnocked;
						return;
					}
					break;
			}
		}
	}


	// Play the touch sound and animation.

	if( (m_eState == kState_PropDefault) && (hCharacter != LTNULL))
	{
        LTVector vPos;
		g_pLTServer->GetObjectPos(m_hObject, &vPos);

		// Play sound.
		PlayTouchSound(vPos);

		// Play animation.
		if(m_pDisturb->hTouchAnim != INVALID_ANI)
		{
			g_pLTServer->SetModelAnimation(m_hObject, m_pDisturb->hTouchAnim);
			g_pLTServer->SetModelLooping(m_hObject, LTFALSE);
			g_pLTServer->ResetModelAnimation(m_hObject);
		}

		// Register touch disturbance stimulus.

		if( (m_pDisturb->pPD->nTouchAlarmLevel > 0) && (m_pDisturb->pPD->fStimRadius > 0.f) )
		{
			g_pAIStimulusMgr->RegisterStimulus( kStim_EnemyDisturbanceSound, m_pDisturb->pPD->nTouchAlarmLevel, hCharacter, m_hObject, vPos, m_pDisturb->pPD->fStimRadius );

			// Props that are knocked over have a visual stimulus too.

			if( m_pDisturb->eTouchAnimType == kPA_Knock )
			{
				g_pAIStimulusMgr->RegisterStimulus( kStim_EnemyDisturbanceVisible, m_pDisturb->pPD->nTouchAlarmLevel + 1, hCharacter, m_hObject, vPos, m_pDisturb->pPD->fStimRadius );
			}
		}

		m_eState = kState_PropTouching;
	}

	// Update while playing touch sound and/or animation.
	SetNextUpdate(UPDATE_NEXT_FRAME);
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Prop::HandleModelString
//
//  PURPOSE:	Handle reaching a frame string...
//
// ----------------------------------------------------------------------- //

void Prop::HandleModelString( ArgList *pArgList )
{

	static CParsedMsg::CToken s_cTok_KEY_COMMAND( KEY_COMMAND );
	static CParsedMsg::CToken s_cTok_KEY_BUTE_SOUND ( KEY_BUTE_SOUND );
		
	if (!pArgList || !pArgList->argv || pArgList->argc == 0) return;

	char* pKey = pArgList->argv[0];
	if (!pKey) return;

	CParsedMsg::CToken tok( pKey );

	if ( tok == s_cTok_KEY_BUTE_SOUND )
	{
		if( pArgList->argc > 1 && pArgList->argv[1] )
		{
			g_pServerSoundMgr->PlaySoundFromObject( m_hObject, pArgList->argv[1] );
		}
	}
	else if ( tok == s_cTok_KEY_COMMAND )
	{
		LTBOOL bAddParen = LTFALSE;

		char buf[255] = "";
		sprintf(buf, "%s", pArgList->argv[1]);
		for (int i=2; i < pArgList->argc; i++)
		{
			bAddParen = LTFALSE;
			strcat(buf, " ");
			if (strstr(pArgList->argv[i], " "))
			{
				strcat(buf, "(");
				bAddParen = LTTRUE;
			}

			strcat(buf, pArgList->argv[i]);

			if (bAddParen)
			{
				strcat(buf, ")");
			}
		}

		g_pLTServer->CPrint("PROP KEY COMMAND: %s", buf);
		if (buf[0] && g_pCmdMgr->IsValidCmd(buf))
		{
			g_pCmdMgr->Process(buf, m_hObject, m_hObject);
		}
    }

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::HandleDestroy
//
//	PURPOSE:	Destroy the prop
//
// ----------------------------------------------------------------------- //

void Prop::HandleDestroy(HOBJECT hDamager)
{
	if( (!m_pDisturb) || (!m_pDisturb->pPD) ) 
	{
		// Remove us since we don't have a destroyed model to switch to.
		g_pLTServer->RemoveObject( m_hObject );
		return;
	}

	// Kill sound if touching.

	if( m_eState == kState_PropTouching )
	{
		ClearTouchSoundIfDone( LTTRUE );
	}
	else if( m_eState == kState_PropHit )
	{
		ClearHitSoundIfDone( LTTRUE );
	}

	// Remove us if we don't have a destroyed model to switch to.
	if( m_pDisturb->pPD->sDestroyFilename.empty( ) )
	{
		g_pLTServer->RemoveObject( m_hObject );
		return;
	}

	ObjectCreateStruct createstruct;
	createstruct.Clear();

	SAFE_STRCPY(createstruct.m_Filename, m_pDisturb->pPD->sDestroyFilename.c_str( ));

	m_pDisturb->pPD->blrDestroySkinReader.CopyList(0, createstruct.m_SkinNames[0], MAX_CS_FILENAME_LEN+1);
	m_pDisturb->pPD->blrDestroyRenderStyleReader.CopyList(0, createstruct.m_RenderStyleNames[0], MAX_CS_FILENAME_LEN+1);

	g_pCommonLT->SetObjectFilenames(m_hObject, &createstruct);

	m_eState = kState_PropDestroyed;

	// Can't activate a destroyed prop.
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, 0, USRFLG_CAN_ACTIVATE);

	m_damage.SetCanDamage(LTFALSE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::ClearTouchSoundIfDone
//
//	PURPOSE:	Clear the touch sound if done playing, or forced done.
//
// ----------------------------------------------------------------------- //

LTBOOL Prop::ClearTouchSoundIfDone(LTBOOL bForceDone)
{
	if( !m_pDisturb || !m_pDisturb->hHitAnim )
		return LTTRUE;
	
	// Check if sound is still playing.

	if (m_pDisturb->hTouchSound)
	{
	    bool bIsDone;
	    if( bForceDone || (g_pLTServer->SoundMgr()->IsSoundDone(m_pDisturb->hTouchSound, bIsDone) != LT_OK) || bIsDone)
		{
			g_pLTServer->SoundMgr()->KillSound(m_pDisturb->hTouchSound);
	        m_pDisturb->hTouchSound = LTNULL;

			return LTTRUE;
		}
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::ClearTouchAnimIfDone
//
//	PURPOSE:	Clear the touch animation if done playing, or forced done.
//
// ----------------------------------------------------------------------- //

LTBOOL Prop::ClearTouchAnimIfDone(HMODELANIM hAnim, LTBOOL bForceDone)
{
	if( g_pLTServer->GetModelAnimation(m_hObject) == hAnim)
	{
		if( bForceDone || (MS_PLAYDONE & g_pLTServer->GetModelPlaybackState(m_hObject)) )
		{
			// Play the world animation, if one exists.

			if( !m_lstWorldAnims.empty() )
			{
				PlayRandomWorldAnim();
				return LTTRUE;
			}
		}
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::ClearHitSoundIfDone
//
//	PURPOSE:	Clear the hit sound if done playing, or forced done.
//
// ----------------------------------------------------------------------- //

LTBOOL Prop::ClearHitSoundIfDone( LTBOOL bForceDone )
{
	if( !m_pDisturb || !m_pDisturb->hHitAnim )
		return LTTRUE;

	// Check if sound is still playing.

	if( m_pDisturb->hHitSound )
	{
	    bool bIsDone;
	    if( bForceDone || (g_pLTServer->SoundMgr()->IsSoundDone( m_pDisturb->hHitSound, bIsDone ) != LT_OK) || bIsDone)
		{
			g_pLTServer->SoundMgr()->KillSound( m_pDisturb->hHitSound );
	        m_pDisturb->hHitSound = LTNULL;

			return LTTRUE;
		}
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::ClearHitAnimIfDone
//
//	PURPOSE:	Clear the hit animation if done playing, or forced done.
//
// ----------------------------------------------------------------------- //

LTBOOL Prop::ClearHitAnimIfDone( LTBOOL bForceDone )
{
	if( !m_pDisturb || !m_pDisturb->hHitAnim )
		return LTTRUE;

	if( g_pLTServer->GetModelAnimation(m_hObject) == m_pDisturb->hHitAnim )
	{
		if( bForceDone || (MS_PLAYDONE & g_pLTServer->GetModelPlaybackState(m_hObject)) )
		{
			// Play the world animation, if one exists.

			if( !m_lstWorldAnims.empty() )
			{
				PlayRandomWorldAnim();
				return LTTRUE;
			}
		}
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::PlayRandomWorldAnim
//
//	PURPOSE:	Play world anim.
//
// ----------------------------------------------------------------------- //

void Prop::PlayRandomWorldAnim()
{
	if( !m_lstWorldAnims.empty() )
	{
		uint32 iWorldAnim = GetRandom( 0, m_lstWorldAnims.size()-1 );
		HMODELANIM hWorldAnim = m_lstWorldAnims[iWorldAnim];

		g_pLTServer->SetModelAnimation( m_hObject, hWorldAnim );
		g_pLTServer->SetModelLooping( m_hObject, LTTRUE );
		g_pLTServer->ResetModelAnimation(m_hObject);

		// Start the looping animation at a random frame.

		uint32 nLength;
		g_pModelLT->GetAnimLength( m_hObject, hWorldAnim, nLength );

		ANIMTRACKERID nTracker;
		if ( LT_OK == g_pModelLT->GetMainTracker( m_hObject, nTracker ) )
		{
			uint32 nTime = GetRandom( 0, nLength );
			g_pModelLT->SetCurAnimTime( m_hObject, nTracker, nTime );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::PlayTouchSound
//
//	PURPOSE:	Play touch sound.
//
// ----------------------------------------------------------------------- //

void Prop::PlayTouchSound(LTVector& vPos)
{
	if( !m_pDisturb->sTouchSoundName.empty( ))
	{
		uint32 dwFlags = PLAYSOUND_GETHANDLE | PLAYSOUND_TIME;

		m_pDisturb->hTouchSound = g_pServerSoundMgr->PlaySoundFromPos(vPos, m_pDisturb->sTouchSoundName.c_str( ),
			m_pDisturb->pPD->fTouchSoundRadius, SOUNDPRIORITY_MISC_LOW, dwFlags);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::PlayHitSound
//
//	PURPOSE:	Play hit sound.
//
// ----------------------------------------------------------------------- //

void Prop::PlayHitSound( LTVector &vPos )
{
	if( !m_pDisturb || !m_pDisturb->pPD )
		return;

	if( !m_pDisturb->pPD->sHitSound.empty() )
	{
		uint32 dwFlags = PLAYSOUND_GETHANDLE | PLAYSOUND_TIME;

		m_pDisturb->hHitSound = g_pServerSoundMgr->PlaySoundFromPos( vPos, m_pDisturb->pPD->sHitSound.c_str(),
								m_pDisturb->pPD->fHitSoundRadius, SOUNDPRIORITY_MISC_LOW, dwFlags );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Prop::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	m_ActivateTypeHandler.Save( pMsg );

	SAVE_FLOAT(m_fAlpha);
    SAVE_BOOL(m_bMoveToFloor);
    SAVE_BOOL(m_bFirstUpdate);
	SAVE_bool( m_bCanDeactivate );
    SAVE_VECTOR(m_vScale);
    SAVE_VECTOR(m_vObjectColor);

	m_damage.Save(pMsg, dwSaveFlags);
	SAVE_BOOL(m_bTouchable);
	SAVE_DWORD(m_eState);
	
/* THESE SHOULD NOT NEED TO BE SAVED - THEY ARE USED FOR INITIALIZING THE OBJECT ONLY!!!
// 8/4/02 - Remove after testing
	SAVE_DWORD(m_dwUsrFlgs);
	SAVE_DWORD(m_dwFlags);
	SAVE_DWORD(m_dwFlags2);
*/

	SAVE_DWORD(m_lstWorldAnims.size());
	for( HMODELANIM_LIST::iterator it = m_lstWorldAnims.begin(); it != m_lstWorldAnims.end(); ++it )
	{
		SAVE_DWORD(*it);
	}

	if(m_pDisturb != LTNULL)
	{
		SAVE_bool(true);
		m_pDisturb->Save(pMsg);
	}
	else 
	{
		SAVE_bool(false);
	}

	SAVE_bool( m_bActivatedOn );
	SAVE_CHARSTRING( m_sActivateOnCommand.c_str( ));
	SAVE_CHARSTRING( m_sActivateOffCommand.c_str( ));

	SAVE_bool( m_bAttachmentShotOff );
	SAVE_HOBJECT( m_hAttachmentOwner );
	
	SAVE_FLOAT( m_fPitch );
	SAVE_FLOAT( m_fYaw );
	SAVE_FLOAT( m_fRoll );
	SAVE_FLOAT( m_fPitchVel );
	SAVE_FLOAT( m_fYawVel );
	SAVE_FLOAT( m_fRollVel );
	SAVE_bool( m_bRotatedToRest );
	SAVE_bool( m_bRotating );

	SAVE_bool( m_bFading );
	SAVE_FLOAT( m_fFadeStartTime );
	SAVE_FLOAT( m_fFadeDuration );
	SAVE_FLOAT( m_fStartAlpha );
	SAVE_FLOAT( m_fEndAlpha );
	SAVE_bool( m_bFadeRemoveWhenDone );

	SAVE_bool( m_bCanTransition );
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Prop::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	m_ActivateTypeHandler.Load( pMsg );

	LOAD_FLOAT(m_fAlpha);
    LOAD_BOOL(m_bMoveToFloor);
    LOAD_BOOL(m_bFirstUpdate);
	LOAD_bool( m_bCanDeactivate );
    LOAD_VECTOR(m_vScale);
    LOAD_VECTOR(m_vObjectColor);

	m_damage.Load(pMsg, dwLoadFlags);
	LOAD_BOOL(m_bTouchable);
	LOAD_DWORD_CAST(m_eState, EnumAIStateType);

/* THESE SHOULD NOT NEED TO BE SAVED - THEY ARE USED FOR INITIALIZING THE OBJECT ONLY!!!
// 8/4/02 - Remove after testing
	LOAD_DWORD(m_dwUsrFlgs);
	LOAD_DWORD(m_dwFlags);
	LOAD_DWORD(m_dwFlags2);
*/

	HMODELANIM hWorldAnim;
	uint32 cWorldAnims;
	LOAD_DWORD(cWorldAnims);
	for( uint32 iWorldAnim = 0; iWorldAnim < cWorldAnims; ++iWorldAnim )
	{
		LOAD_DWORD(hWorldAnim);
		m_lstWorldAnims.push_back(hWorldAnim);
	}

	if (pMsg->Readbool())
	{
		if(m_pDisturb == LTNULL)
		{
			m_pDisturb = debug_new(PropDisturbStruct);
		}
		m_pDisturb->Load(pMsg);
	}

	LOAD_bool( m_bActivatedOn );

	char szString[1024];

	LOAD_CHARSTRING( szString, ARRAY_LEN( szString ));
	m_sActivateOnCommand = szString;

	LOAD_CHARSTRING( szString, ARRAY_LEN( szString ));
	m_sActivateOffCommand = szString;

	LOAD_bool( m_bAttachmentShotOff );
	LOAD_HOBJECT( m_hAttachmentOwner );
	
	LOAD_FLOAT( m_fPitch );
	LOAD_FLOAT( m_fYaw );
	LOAD_FLOAT( m_fRoll );
	LOAD_FLOAT( m_fPitchVel );
	LOAD_FLOAT( m_fYawVel );
	LOAD_FLOAT( m_fRollVel );
	LOAD_bool( m_bRotatedToRest );
	LOAD_bool( m_bRotating );

	LOAD_bool( m_bFading );
	LOAD_FLOAT( m_fFadeStartTime );
	LOAD_FLOAT( m_fFadeDuration );
	LOAD_FLOAT( m_fStartAlpha );
	LOAD_FLOAT( m_fEndAlpha );
	LOAD_bool( m_bFadeRemoveWhenDone );

	LOAD_bool( m_bCanTransition );

	// We must remove aggregates before sending the message to the base classs...

	if( !m_bCanTransition )
	{
		// Disallow transitioning of this object through TransAMs...

		DestroyTransitionAggregate();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::HandleHit
//
//	PURPOSE:	Handle taking damage...
//
// ----------------------------------------------------------------------- //

void Prop::HandleHit( HOBJECT hDamager )
{
	if( ( hDamager != LTNULL ) &&
		( !IsCharacter(hDamager) ) &&
		( !IsExplosion(hDamager) ) )
	{
		return;
	}

	if( (m_eState == kState_PropDestroyed)
		|| (m_eState == kState_PropKnocked) 
		|| (!(m_pDisturb && m_pDisturb->pPD)) )
	{
		return;
	}

	// If we don't have a valid hit animation default to the touch animation...

	if( m_pDisturb->hHitAnim == INVALID_ANI )
		HandleTouch( hDamager );

	// Resolve the toucher to a character handle.

	HOBJECT hCharacter = LTNULL;
	if( IsCharacterHitBox( hDamager ) )
	{
		CCharacterHitBox* pHB = (CCharacterHitBox*)g_pLTServer->HandleToObject(hDamager);
		hCharacter = pHB->GetModelObject();
	}
	else if( IsExplosion( hDamager ) )
	{
		Explosion* pExplosion = (Explosion*)g_pLTServer->HandleToObject(hDamager);
		hCharacter = pExplosion->GetFiredFrom();
	}
	else if( IsCharacter( hDamager ) ) 
	{
		hCharacter = hDamager;
	}

	// Only characters can hit (for now).
	if((hCharacter != LTNULL) && !IsCharacter(hCharacter)) return;

	if(m_eState == kState_PropHit)
	{
		// Check if both sound and animation are done

		ClearHitSoundIfDone( LTFALSE );
		if( m_pDisturb->hHitSound == LTNULL )
		{
			ClearHitAnimIfDone( LTFALSE );
			if( g_pLTServer->GetModelAnimation(m_hObject) != m_pDisturb->hHitAnim )
			{
				m_eState = kState_PropDefault;
				return;
			}
		}
	}
	
	// Play the hit sound and animation.

	if( (m_eState == kState_PropDefault) && (hCharacter != LTNULL))
	{
        LTVector vPos;
		g_pLTServer->GetObjectPos(m_hObject, &vPos);

		// Play sound.
		PlayHitSound(vPos);

		// Play animation.
		if(m_pDisturb->hHitAnim != INVALID_ANI)
		{
			g_pLTServer->SetModelAnimation( m_hObject, m_pDisturb->hHitAnim );
			g_pLTServer->SetModelLooping( m_hObject, LTFALSE );
			g_pLTServer->ResetModelAnimation(m_hObject);
		}

		// Register touch disturbance stimulus.

		if( (m_pDisturb->pPD->nHitAlarmLevel > 0) && (m_pDisturb->pPD->fStimRadius > 0.f) )
		{
			g_pAIStimulusMgr->RegisterStimulus( kStim_EnemyDisturbanceSound, m_pDisturb->pPD->nHitAlarmLevel, hCharacter, m_hObject, vPos, m_pDisturb->pPD->fStimRadius );
		}

		m_eState = kState_PropHit;
	}

	// Update while playing touch sound and/or animation.
	SetNextUpdate(UPDATE_NEXT_FRAME);
}	

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::SendActivateMessage
//
//	PURPOSE:	Send the activate message.  Toggles between 2 different
//				messages.
//
// ----------------------------------------------------------------------- //

void Prop::SendActivateMessage( )
{
	char const* pszActivateCommand = NULL;

	// Switch activated state.
	m_bActivatedOn = !m_bActivatedOn;

	// Send activate on message if we were just activated.
	if( m_bActivatedOn )
	{
		pszActivateCommand = m_sActivateOnCommand.c_str( );
	}
	// Send activate off message if we were just deactivated.
	else
	{
		pszActivateCommand = m_sActivateOffCommand.c_str( );
	}

	// See if we got a message.
	if( !pszActivateCommand || !pszActivateCommand[0] )
		return;

	// Send the command.
	if( g_pCmdMgr->IsValidCmd( pszActivateCommand ))
	{
		g_pCmdMgr->Process( pszActivateCommand, m_hObject, NULL );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::HandleAttachmentImpact
//
//	PURPOSE:	Handle getting shot off a character as an attachment
//
// ----------------------------------------------------------------------- //

void Prop::HandleAttachmentImpact( CAttachmentPosition *pAttachPos, const LTVector& vDir )
{
	if( !pAttachPos )
		return;

	// TODO: Init these somewhere else
	if( !s_vtAttachmentMinVel.IsInitted() )
	{
		s_vtAttachmentMinVel.Init( g_pLTServer, "AttachmentMinVel", LTNULL, DEFAULT_ATTACH_MIN_VEL );
	}
	if( !s_vtAttachmentMaxVel.IsInitted() )
	{
		s_vtAttachmentMaxVel.Init( g_pLTServer, "AttachmentMaxVel", LTNULL, DEFAULT_ATTACH_MAX_VEL );
	}
	if( !s_vtAttachmentMinYAdd.IsInitted() )
	{
		s_vtAttachmentMinYAdd.Init( g_pLTServer, "AttachmentMinYAdd", LTNULL, DEFAULT_ATTACH_MIN_YADD );
	}
	if( !s_vtAttachmentMaxYAdd.IsInitted() )
	{
		s_vtAttachmentMaxYAdd.Init( g_pLTServer, "AttachmentMaxYAdd", LTNULL, DEFAULT_ATTACH_MAX_YADD );
	}
	if( !s_vtAttachmentFadeDelay.IsInitted() )
	{
		s_vtAttachmentFadeDelay.Init( g_pLTServer, "AttachmentFadeDelay", LTNULL, DEFAULT_ATTACH_FADE_DELAY );
	}
	if( !s_vtAttachmentFadeDuration.IsInitted() )
	{
		s_vtAttachmentFadeDuration.Init( g_pLTServer, "AttachmentFadeDuration", LTNULL, DEFAULT_ATTACH_FADE_DURATION );
	}
	
	// Set the owner of the attachment...
	
	m_hAttachmentOwner = pAttachPos->GetAttachment()->GetObject();

	uint32 dwFlags = FLAG_POINTCOLLIDE | FLAG_NOSLIDING | FLAG_TOUCH_NOTIFY | FLAG_GRAVITY;
	g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, dwFlags, dwFlags | FLAG_GOTHRUWORLD );	

	// Play the world animation so the prop is flat when it comes to rest...

	HMODELANIM	hWorldAnim = INVALID_MODEL_ANIM;
	if( (g_pModelLT->GetAnimIndex( m_hObject, "WORLD", hWorldAnim ) == LT_OK) && (hWorldAnim != INVALID_MODEL_ANIM) )
	{
		g_pModelLT->SetLooping( m_hObject, MAIN_TRACKER, true );
		g_pModelLT->SetCurAnim( m_hObject, MAIN_TRACKER, hWorldAnim );
	}
	

	LTVector vVel = vDir;
	vVel.Normalize();
	vVel *= GetRandom( s_vtAttachmentMinVel.GetFloat(), s_vtAttachmentMaxVel.GetFloat() );
	vVel.y += GetRandom( s_vtAttachmentMinYAdd.GetFloat(), s_vtAttachmentMaxYAdd.GetFloat() );
	
	g_pPhysicsLT->SetVelocity( m_hObject, &vVel );

	// Don't play the touch animation and sounds when recieving a touch notify...

	m_bTouchable = LTFALSE;
	m_bAttachmentShotOff = LTTRUE;

	g_pLTServer->SetBlockingPriority( m_hObject, 0 );
	g_pPhysicsLT->SetForceIgnoreLimit( m_hObject, 0.0f );

	// Give it some rotation...

	float fVal	= MATH_PI;
    float fVal2	= MATH_CIRCLE;
	m_fPitchVel	= GetRandom( -fVal2, fVal2 );
	m_fYawVel	= GetRandom( -fVal2, fVal2 );
	m_fRollVel	= GetRandom( -fVal2, fVal2 );

	m_bRotating	= true;

	SetNextUpdate(UPDATE_NEXT_FRAME);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::HandleAttachmentTouch
//
//	PURPOSE:	Handle getting a touch notify as a shot off attachment...
//
// ----------------------------------------------------------------------- //

void Prop::HandleAttachmentTouch( HOBJECT hToucher )
{
	if( !hToucher || !m_bAttachmentShotOff )
		return;

	// Don't touch the owner...

	if( hToucher == m_hAttachmentOwner )
		return;

	// Or any non-solid objects...
	
	uint32 dwToucherFlags;
	g_pCommonLT->GetObjectFlags( hToucher, OFT_Flags, dwToucherFlags );
	if( !(dwToucherFlags & FLAG_SOLID) )
		return;

	CollisionInfo info;
    g_pLTServer->GetLastCollision( &info );

	LTVector vVel;
	g_pPhysicsLT->GetVelocity( m_hObject, &vVel );

	// Calculate where we really hit the world...
    
	if( IsMainWorld( hToucher ))
	{
        LTVector vPos, vCurVel, vP0, vP1;
        g_pLTServer->GetObjectPos( m_hObject, &vPos );

		vP1 = vPos;
        vCurVel = vVel * g_pLTServer->GetFrameTime();
		vP0 = vP1 - vCurVel;
		vP1 += vCurVel;

        LTFLOAT fDot1 = VEC_DOT( info.m_Plane.m_Normal, vP0 ) - info.m_Plane.m_Dist;
        LTFLOAT fDot2 = VEC_DOT( info.m_Plane.m_Normal, vP1 ) - info.m_Plane.m_Dist;

		if( fDot1 < 0.0f && fDot2 < 0.0f || fDot1 > 0.0f && fDot2 > 0.0f )
		{
			vPos = vP1;
		}
		else
		{
            LTFLOAT fPercent = -fDot1 / (fDot2 - fDot1);
			VEC_LERP( vPos, vP0, vP1, fPercent);
		}

		// Set our new "real" pos...

        g_pLTServer->SetObjectPos( m_hObject, &vPos );
	}

	// If we're on the ground (or an object), stop movement...

	CollisionInfo standingInfo;
    g_pLTServer->GetStandingOn( m_hObject, &standingInfo );

	CollisionInfo* pInfo = standingInfo.m_hObject ? &standingInfo : &info;

	if( pInfo->m_hObject )
	{
		// Don't stop on walls...

		if( pInfo->m_Plane.m_Normal.y > 0.75f )
		{
			vVel.Init();

			// Turn off gravity, solid, and touch notify....

			uint32 dwFlags = FLAG_POINTCOLLIDE | FLAG_NOSLIDING | FLAG_TOUCH_NOTIFY | FLAG_GRAVITY;
			g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, 0, dwFlags );

			// Rotate to rest...

			if( m_bRotating )
			{
				LTRotation rRot( 0.0f, m_fYaw, 0.0f );
				g_pLTServer->SetObjectRotation( m_hObject, &rRot );

				m_bRotating = false;
				StartFade( s_vtAttachmentFadeDuration.GetFloat(), s_vtAttachmentFadeDelay.GetFloat() );
			}
		}
	}
	
	// Remove the stoping velocity...

	vVel = -info.m_vStopVel;
	g_pPhysicsLT->SetVelocity( m_hObject, &vVel );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::StartFade
//
//	PURPOSE:	Begin fading out the prop...
//
// ----------------------------------------------------------------------- //

void Prop::StartFade( float fDuration, float fDelayStartTime /* = 0.0f */, float fEndAlpha /* = 0.0f */, bool bRemove /* = true  */ )
{
	if( m_bFading )
		return;

	m_bFading = true;

	m_fFadeStartTime	= g_pLTServer->GetTime() + fDelayStartTime + g_pLTServer->GetFrameTime();
	m_fFadeDuration		= Clamp( fDuration, 0.1f, 100000.0f );
	m_fEndAlpha			= Clamp( fEndAlpha, 0.0f, 1.0f );
	m_bFadeRemoveWhenDone = bRemove;

	float r, g, b;
	g_pLTServer->GetObjectColor( m_hObject, &r, &g, &b, &m_fStartAlpha );

	SetNextUpdate(UPDATE_NEXT_FRAME);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::UpdateFade
//
//	PURPOSE:	Update fading out the prop...
//
// ----------------------------------------------------------------------- //

void Prop::UpdateFade()
{
	if( !m_bFading )
	{
		return;
	}

	bool bDone = false;
	float fCurTime = g_pLTServer->GetTime();

	// Update the fade if the delay has past...

	if( m_fFadeStartTime <= fCurTime )
	{
		float t = (fCurTime - m_fFadeStartTime) / m_fFadeDuration;
		float fAlpha = Clamp( LTLERP( m_fStartAlpha, m_fEndAlpha, t ), 0.0f, 1.0f );

		float r, g, b, a;
		g_pLTServer->GetObjectColor( m_hObject, &r, &g, &b, &a );
		g_pLTServer->SetObjectColor( m_hObject, r, g, b, fAlpha );

		float fDiff = (float)fabs( m_fEndAlpha - fAlpha );

		if( fDiff <= MATH_EPSILON )
			bDone = true;
	}

	if( bDone )
	{
		if( m_bFadeRemoveWhenDone )
		{
			g_pLTServer->RemoveObject( m_hObject ); 
		}

		m_bFading = false;
	}
	else
	{
		// The fade is not done yet so keep updating...

		SetNextUpdate(UPDATE_NEXT_FRAME);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::Update
//
//	PURPOSE:	Update the prop
//
// ----------------------------------------------------------------------- //

void Prop::Update()
{
	if (m_bFirstUpdate && m_bMoveToFloor)
	{
		// Make sure object starts on floor...
		m_bFirstUpdate = LTFALSE;
		MoveObjectToFloor(m_hObject);

		SetNextUpdate(UPDATE_NEXT_FRAME);
	}
	else if( m_pDisturb && (m_eState == kState_PropTouching) )
	{
		HandleTouch(LTNULL);
	}
	else if( m_pDisturb && (m_eState == kState_PropHit) )
	{
		HandleHit(LTNULL);
	}
	else if( m_bRotating )
	{
		if( m_fPitchVel != 0 || m_fYawVel != 0 || m_fRollVel != 0 )
		{
			float fDeltaTime = g_pLTServer->GetFrameTime();

			m_fPitch += m_fPitchVel * fDeltaTime;
			m_fYaw   += m_fYawVel * fDeltaTime;
			m_fRoll  += m_fRollVel * fDeltaTime;

			LTRotation rRot( m_fPitch, m_fYaw, m_fRoll );
			g_pLTServer->SetObjectRotation( m_hObject, &rRot );

			SetNextUpdate(UPDATE_NEXT_FRAME);
		}
	}
	else if ( m_bFading )
	{
		UpdateFade();
	}
	else if ( m_bCanDeactivate )
	{
		// At this point the model only needs to get updates if we're playing
		// a non-looping animation so we can deactivate the object when the animation
		// is done playing.  However, if the model is playing a looping animation
		// we'll stop updating but leave the object active (so it will continue to
		// animate and get key strings)...

		// See if we're animating...

		HMODELANIM hAnim = g_pLTServer->GetModelAnimation(m_hObject);
		if (hAnim != INVALID_ANI)
		{
			if (g_pLTServer->GetModelLooping(m_hObject))
			{
				// We're playing a looping animation that is long enough, don't deactivate, 
				// just stop updating the object (since the state can only be changed 
				// externally)...NOTE: Short animations are default anis that you can't
				// see so we'll deactivate in those cases...

				uint32 nLength = 0;
				g_pModelLT->GetAnimLength(m_hObject, hAnim, nLength);
				
				if (nLength > 200)
				{
					SetNextUpdate(UPDATE_NEVER, eControlUpdateOnly);
				}
				else
				{
					SetNextUpdate(UPDATE_NEVER);
				}
			}
			else  // Playing a non-looping animation...
			{
				bool bAniDone = !!(MS_PLAYDONE & g_pLTServer->GetModelPlaybackState(m_hObject));

				if (bAniDone)
				{
					// Cool we can stop updating
					SetNextUpdate(UPDATE_NEVER);
				}
				else
				{
					// Keep updating until the animation is done...
					SetNextUpdate(UPDATE_NEXT_FRAME);
				}
			}
		}
		else  // Not animating so no reason to update...
		{
			SetNextUpdate(UPDATE_NEVER);
		}
	}
	else
	{
		// Just keep updating
		SetNextUpdate(UPDATE_NEXT_FRAME);
	}
}