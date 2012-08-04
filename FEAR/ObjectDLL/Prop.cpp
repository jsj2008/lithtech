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

#include "Stdafx.h"
#include "Prop.h"
#include "ServerUtilities.h"
#include "ClientServerShared.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "PlayerObj.h"
#include "SoundMgr.h"
#include "AIState.h"
#include "CharacterHitBox.h"
#include "ServerSoundMgr.h"
#include "AIStimulusMgr.h"
#include "Explosion.h"
#include "Attachments.h"
#include "resourceextensions.h"
#include "PropsDB.h"

extern CAIStimulusMgr* g_pAIStimulusMgr;

#define	KEY_BUTE_SOUND	"BUTE_SOUND_KEY"

#define ModelNone	"<none>"


LINKFROM_MODULE( Prop );

BEGIN_CLASS(Prop)
	ADD_DESTRUCTIBLE_MODEL_AGGREGATE(PF_GROUP(1), 0)
	ADD_STRINGPROP_FLAG(Filename, "", PF_FILENAME | PF_HIDDEN | PF_MODEL, "Specifies the ." RESEXT_MODEL_COMPRESSED " file the model's geometry is in.")
	ADD_STRINGPROP_FLAG(Material, "", PF_FILENAME | PF_HIDDEN, "Specifies the material for the model")
	ADD_STRINGPROP_FLAG(Model, ModelNone, PF_STATICLIST | PF_DIMS | PF_LOCALDIMS, "Contains a list of Prop objects that set the model and materials.")
	ADD_REALPROP(Scale, 1.0f, "This value changes the size of the object. It is a multiplicative value based on the original size of the object. The default scale is 1.0.")
	ADD_VISIBLE_FLAG(1, 0)
	ADD_SOLID_FLAG(0, 0)
	ADD_GRAVITY_FLAG(0, 0)
	ADD_SHADOW_FLAG(0, 0)
	ADD_BOOLPROP(MoveToFloor, true, "Tells a model to appear on the floor when the level opens, even if you placed it in the air inside WorldEdit. Useful for objects like trees and shrubs on terrain, where it's sometimes difficult to line up the exact floor level under an object.")
	ADD_REALPROP(Alpha, 1.0f, "Sets overall translucency of the object, regardless of its alpha mask.")
	ADD_COLORPROP(ObjectColor, 255.0f, 255.0f, 255.0f, "This color picker is used to add a color tint to the Prop.")
	ADD_BOOLPROP(Additive, false, "This flag enables the Additive blending mode.")
	ADD_BOOLPROP(Multiply, false, "This flag enables the Multiply blending mode.")
	ADD_BOOLPROP(RayHit, true, "This flag toggles whether or not the object will be rayhit.")
	ADD_COMMANDPROP_FLAG(CommandOn, "", PF_NOTIFYCHANGE, "Command sent the when prop is activated on.  Prop will toggle to off if activated again." )
	ADD_COMMANDPROP_FLAG(CommandOff, "", PF_NOTIFYCHANGE, "Command sent the when prop is activated off.  Prop will toggle to on if activated again." )
	ADD_BOOLPROP_FLAG(CanTransition, true, 0, "When this object is placed within a TransitionArea, it will be transitioned to the next level when a transition occurs.  Set this flag to flase if you do not want the object to transition.")
	ADD_BOOLPROP_FLAG(CanDeactivate, true, 0, "Set this flag to false if you want the prop to always receive updates.")
	ADD_STRINGPROP_FLAG(ActivationType, "Default", PF_STATICLIST, "A list of different activatable types used for player interaction.")
	ADD_PREFETCH_RESOURCE_PROPS()

END_CLASS_FLAGS_PLUGIN_PREFETCH(Prop, GameBase, 0, CPropPlugin, DefaultPrefetch<Prop>, "Prop objects are used to place models in the level.  The models are model records specified in the game database." )

//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( Prop )

	ADD_MESSAGE( ANIM,		2,	NULL,	MSG_HANDLER( Prop, HandleAnimMsg ),		"ANIM <ani name>", "Tells a Prop to play a specific animation once", "To make a Prop named \"Prop\" play an animation named \"Fall\" the command would look like:<BR><BR>msg Prop (ANIM Fall)" )			// We dont know if the animation is valid or not 
	ADD_MESSAGE( ANIMLOOP,	2,	NULL,	MSG_HANDLER( Prop, HandleAnimLoopMsg ),	"ANIMLOOP <ani name>", "Tells a Prop to play a specific animation continuously", "To make a Prop named \"Prop\" play an animation named \"Fall\" the command would look like:<BR><BR>msg Prop (ANIMLOOP Fall)" )
	ADD_MESSAGE( ACTIVATE,	1,	NULL,	MSG_HANDLER( Prop, HandleActivateMsg ),	"ACTIVATE", "TODO:CMDDESC", "TODO:CMDEXP" )
	ADD_MESSAGE( DESTROY,	1,	NULL,	MSG_HANDLER( Prop, HandleDestroyMsg ),	"DESTROY", "This command destroys an object in the game as if it had been destroyed trough the loss of all of its hit points. It will play any effects and sounds set up to play at the time of its destruction", "msg Prop DESTROY" )

CMDMGR_END_REGISTER_CLASS( Prop, GameBase )

LTRESULT CPropPlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char** aszStrings,
    uint32* pcStrings,
    const uint32 cMaxStrings,
    const uint32 cMaxStringLength)
{
	if( LTStrIEquals( "Model", szPropName ))
	{
		LTStrCpy(aszStrings[(*pcStrings)++], ModelNone, cMaxStringLength );

		uint32 cModels = g_pPropsDB->GetNumProps();
		LTASSERT(cMaxStrings >= cModels, "TODO: Add description here");
		for ( uint32 iModel = 0 ; iModel < cModels ; iModel++ )
		{
			// exit out early if we can't hold any more strings
			if( *pcStrings >= cMaxStrings )
				break;

			// Only list model templates in WorldEdit that have been
			// set in modelbutes.txt to type = generic prop.
			PropsDB::HPROP hProp = g_pPropsDB->GetProp( iModel );
			LTStrCpy( aszStrings[(*pcStrings)++], g_pPropsDB->GetRecordName( hProp ), cMaxStringLength );
		}

		qsort( aszStrings + 1, *pcStrings - 1, sizeof( char * ), CaseInsensitiveCompare );

		return LT_OK;
	}
	else if( !LTStrICmp( szPropName, "ActivationType" ))
	{
		if( CategoryPlugin::Instance().PopulateStringList( DATABASE_CATEGORY( Activate ).GetCategory(), 
			aszStrings, pcStrings, cMaxStrings, cMaxStringLength ))
		{
			return LT_OK;
		}
	}

	if (m_DestructibleModelPlugin.PreHook_EditStringList(szRezPath,
		szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength) == LT_OK)
	{
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

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPropPlugin::PreHook_Dims
//
//	PURPOSE:	Determine the dims for this item
//
// ----------------------------------------------------------------------- //

LTRESULT CPropPlugin::PreHook_Dims(const char* szRezPath,
								   const char* szPropName, 
										 const char* szPropValue,
										 char* szModelFilenameBuf,
										 int nModelFilenameBufLen,
										 LTVector & vDims,
										 const char* pszObjName, 
										 ILTPreInterface *pInterface)
{

	if (!szModelFilenameBuf || nModelFilenameBufLen < 1 || !g_pPropsDB )
		return LT_UNSUPPORTED;

	szModelFilenameBuf[0] = '\0';

	// Get the prop and make sure it's not "<none>"
	PropsDB::HPROP hProp = NULL;
	static CParsedMsg::CToken s_cTok_None( ModelNone );
	CParsedMsg::CToken cTokModel( szPropValue );
	if( cTokModel != s_cTok_None )
		hProp = g_pPropsDB->GetPropByRecordName( cTokModel.c_str());
	if( !hProp )
		return LT_UNSUPPORTED;

	// Get the model and set it as our dims model
	const char *pszModel = g_pPropsDB->GetPropFilename( hProp );
	if( !pszModel[0] )
		return LT_UNSUPPORTED;
	LTStrCpy( szModelFilenameBuf, pszModel, nModelFilenameBufLen );

	return LT_OK;
}


#define ANIM_WORLD	"Idle"

#define DEFAULT_ATTACH_MIN_VEL	64.0f
#define DEFAULT_ATTACH_MAX_VEL	256.0f
#define DEFAULT_ATTACH_MIN_YADD	128.0f
#define DEFAULT_ATTACH_MAX_YADD	256.0f
#define DEFAULT_ATTACH_FADE_DELAY	5.0f
#define DEFAULT_ATTACH_FADE_DURATION 1.0f	

static VarTrack	s_vtAttachmentMinVel;
static VarTrack	s_vtAttachmentMaxVel;
static VarTrack	s_vtAttachmentMinYAdd;
static VarTrack	s_vtAttachmentMaxYAdd;
static VarTrack	s_vtAttachmentFadeDelay;
static VarTrack	s_vtAttachmentFadeDuration;


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::Prop()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Prop::Prop() : GameBase(OT_MODEL)
{
    m_bMoveToFloor		= true;
	m_bFirstUpdate		= true;
	m_bCanDeactivate	= true;

	m_fScale = 1.0f;
	m_vObjectColor.Init(255.0f, 255.0f, 255.0f);
	m_fAlpha = 1.0f;

	m_dwUsrFlgs = USRFLG_MOVEABLE;

	m_dwFlags	= FLAG_DONTFOLLOWSTANDING;
	m_dwFlags2	= 0;

	m_eState = kState_PropDefault;

	// Used only for sending event trigger messages.
	m_bActivatedOn = false;

	AddAggregate(&m_damage);
	MakeTransitionable();

	m_bAttachmentShotOff	= false;
	m_hAttachmentOwner		= NULL;

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
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 Prop::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	switch(messageID)
	{
		case MID_TOUCHNOTIFY:
		{
			if( m_bAttachmentShotOff )
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
				ReadProp(&pStruct->m_cProperties);

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
//	ROUTINE:	Prop::PropRead()
//
//	PURPOSE:	Update properties
//
// ----------------------------------------------------------------------- //

void Prop::ReadProp(const GenericPropList *pProps)
{
	if( !pProps )
		return;

	m_fAlpha		= pProps->GetReal( "Alpha", m_fAlpha );
	m_vObjectColor	= pProps->GetColor( "ObjectColor", m_vObjectColor );
	m_bMoveToFloor	= pProps->GetBool( "MoveToFloor", m_bMoveToFloor );
	m_fScale		= pProps->GetReal( "Scale", m_fScale );
	
	m_sActivateOnCommand	= pProps->GetCommand( "CommandOn", "" );
	m_sActivateOffCommand	= pProps->GetCommand( "CommandOff", "" );
	m_bCanTransition		= pProps->GetBool( "CanTransition", m_bCanTransition );
	m_bCanDeactivate		= pProps->GetBool( "CanDeactivate", m_bCanDeactivate );

	if( pProps->GetBool( "Additive", false ))
	{
		m_dwFlags  |= FLAG_NOLIGHT;
	}
	
	if( pProps->GetBool( "Multiply", false ))
	{
		m_dwFlags  |= FLAG_NOLIGHT;
	}
	
	if( pProps->GetBool( "RayHit", true ))
	{
		m_dwFlags |= FLAG_RAYHIT;

		// Set touch notify so projectiles can impact with us...
		m_dwFlags |= FLAG_TOUCH_NOTIFY;
	}
	
	// Remove if outside the world

	m_dwFlags |= FLAG_REMOVEIFOUTSIDE | FLAG_MODELKEYS;

	// If this prop is one that shouldn't stop the player, set the appropriate
	// flag...

	if (m_damage.GetMass() < g_pModelsDB->GetModelMass(g_pModelsDB->GetModelByRecordName(DEFAULT_PLAYERNAME)))
	{
		m_dwFlags |= FLAG_CLIENTNONSOLID;
	}
	
	// Get the activate type...
	m_ActivateTypeHandler.SetActivateType( pProps->GetString( "ActivationType", "Default" ) );
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

	PropsDB::HPROP hProp = NULL;
	static CParsedMsg::CToken s_cTok_None( ModelNone );
	CParsedMsg::CToken cTokModel( pStruct->m_cProperties.GetString( "Model", "" ));
	if( cTokModel != s_cTok_None && cTokModel.c_str()[0] )
		hProp = g_pPropsDB->GetPropByRecordName( cTokModel.c_str());

	if( hProp )
	{
		char const* pszPropFilename = g_pPropsDB->GetPropFilename( hProp );
		if( pszPropFilename && pszPropFilename[0] )
			LTStrCpy( pStruct->m_Filename, pszPropFilename, LTARRAYSIZE( pStruct->m_Filename ));

		g_pPropsDB->CopyMaterialFilenames( hProp, pStruct->m_Materials[0], LTARRAYSIZE( pStruct->m_Materials ),
			LTARRAYSIZE( pStruct->m_Materials[0] ));
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

	m_vObjectColor /= 255.0f;
    g_pLTServer->SetObjectColor(m_hObject, m_vObjectColor.x, m_vObjectColor.y,
								m_vObjectColor.z, m_fAlpha);


	// Default us to our "world" animation...
	// Pick a random world animation from the list.

	uint32 iWorldAnim = 1;
	char szWorldAnim[32];
	LTSNPrintF( szWorldAnim, LTARRAYSIZE( szWorldAnim ), "%s%d", ANIM_WORLD, iWorldAnim++ );
	HMODELANIM hWorldAnim = g_pLTServer->GetAnimIndex( m_hObject, szWorldAnim );
	while( hWorldAnim != INVALID_ANI )
	{
		m_lstWorldAnims.push_back( hWorldAnim );
		LTSNPrintF( szWorldAnim, LTARRAYSIZE( szWorldAnim ), "%s%d", ANIM_WORLD, iWorldAnim++ );
		hWorldAnim = g_pLTServer->GetAnimIndex( m_hObject, szWorldAnim );
	}
	PlayRandomWorldAnim();

	// Set the dims based on the current animation...

    LTVector vDims;
    g_pModelLT->GetModelAnimUserDims(m_hObject, g_pLTServer->GetModelAnimation(m_hObject), &vDims);

	// Set object dims based on scale value...

    LTVector vNewDims;
	vNewDims.x = m_fScale * vDims.x;
	vNewDims.y = m_fScale * vDims.y;
	vNewDims.z = m_fScale * vDims.z;

    g_pLTServer->SetObjectScale(m_hObject, m_fScale);
	g_pPhysicsLT->SetObjectDims(m_hObject, &vNewDims, 0);


	// Only need to update if we're moving the object to the floor...Else, disable
	// updates so the object isn't eating ticks...

	SetNextUpdate((m_bMoveToFloor || !m_bCanDeactivate) ? UPDATE_NEXT_FRAME : UPDATE_NEVER);

	// Send the Activate SpecialFX message.  If we never set the Activate Type no message will get sent...

	m_ActivateTypeHandler.Init( m_hObject );
	m_ActivateTypeHandler.SetState( ACTIVATETYPE::eOn, false );
	m_ActivateTypeHandler.SetDisabled( false, false );
	m_ActivateTypeHandler.CreateActivateTypeMsg();

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::HandleAnimMsg
//
//	PURPOSE:	Handle a ANIM message...
//
// ----------------------------------------------------------------------- //

void Prop::HandleAnimMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	g_pLTServer->SetModelLooping( m_hObject, false) ;
	g_pLTServer->GetModelLT()->SetCurAnim( m_hObject, MAIN_TRACKER, g_pLTServer->GetAnimIndex( m_hObject, crParsedMsg.GetArg( 1 ).c_str() ), true);
	SetNextUpdate( UPDATE_NEXT_FRAME ); // Needed to get string keys
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::HandleAnimLoopMsg
//
//	PURPOSE:	Handle a ANIMLOOP message...
//
// ----------------------------------------------------------------------- //

void Prop::HandleAnimLoopMsg(	HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	g_pLTServer->SetModelLooping( m_hObject, true );
	g_pLTServer->GetModelLT()->SetCurAnim( m_hObject, MAIN_TRACKER, g_pLTServer->GetAnimIndex(m_hObject, crParsedMsg.GetArg( 1 ).c_str() ), true);
	SetNextUpdate( UPDATE_NEXT_FRAME ); // Needed to get string keys
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::HandleActivateMsg
//
//	PURPOSE:	Handle a ACTIVATE message...
//
// ----------------------------------------------------------------------- //

void Prop::HandleActivateMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags( m_hObject, OFT_User, dwFlags );

	if( dwFlags & USRFLG_CAN_ACTIVATE )
	{
		SendActivateMessage( hSender );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::HandleDestroyMsg
//
//	PURPOSE:	Handle a DESTROY message...
//
// ----------------------------------------------------------------------- //

void Prop::HandleDestroyMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	m_damage.HandleDestruction( hSender );
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
	static CParsedMsg::CToken s_cTok_KEY_BUTE_SOUND ( KEY_BUTE_SOUND );
		
	if (!pArgList || !pArgList->argv || pArgList->argc == 0) return;

	char* pKey = pArgList->argv[0];
	if (!pKey) return;

	CParsedMsg::CToken tok( pKey );

	if ( tok == s_cTok_KEY_BUTE_SOUND )
	{
		if( pArgList->argc > 1 && pArgList->argv[1] )
		{
			g_pServerSoundMgr->PlaySoundFromObject( m_hObject, pArgList->argv[1], NULL,
				-1.0f, SOUNDPRIORITY_MISC_LOW, 0, SMGR_DEFAULT_VOLUME, 1.0f, -1.0f,
				DEFAULT_SOUND_CLASS, PLAYSOUND_MIX_OBJECTS);
		}
	}
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

		g_pLTServer->GetModelLT()->SetCurAnim( m_hObject, MAIN_TRACKER, hWorldAnim, true);
		g_pLTServer->GetModelLT()->SetLooping( m_hObject, MAIN_TRACKER, true );
		
		// Start the looping animation at a random frame.

		uint32 nLength;
		g_pModelLT->GetAnimLength( m_hObject, hWorldAnim, nLength );

		uint32 nTime = GetRandom( 0, nLength );
		g_pModelLT->SetCurAnimTime( m_hObject, MAIN_TRACKER, nTime );
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

	SAVE_FLOAT(m_fScale);
	SAVE_FLOAT(m_fAlpha);
    SAVE_BOOL(m_bMoveToFloor);
    SAVE_BOOL(m_bFirstUpdate);
	SAVE_bool( m_bCanDeactivate );
    SAVE_VECTOR(m_vObjectColor);

//[jrg - 10/19/04] - this seems redundant since the aggregate is saved automatically
//	m_damage.Save(pMsg, dwSaveFlags);
	SAVE_DWORD(m_eState);


	SAVE_DWORD(m_lstWorldAnims.size());
	for( HMODELANIM_LIST::iterator it = m_lstWorldAnims.begin(); it != m_lstWorldAnims.end(); ++it )
	{
		SAVE_DWORD(*it);
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
	SAVE_DOUBLE( m_fFadeStartTime );
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

	LOAD_FLOAT(m_fScale);
	LOAD_FLOAT(m_fAlpha);
    LOAD_BOOL(m_bMoveToFloor);
    LOAD_BOOL(m_bFirstUpdate);
	LOAD_bool( m_bCanDeactivate );
    LOAD_VECTOR(m_vObjectColor);

//[jrg - 10/19/04] - this seems redundant since the aggregate is loaded automatically
//					and since this explicit load throws an assert, I'm removing it
//	m_damage.Load(pMsg, dwLoadFlags);
	LOAD_DWORD_CAST(m_eState, EnumAIStateType);

	HMODELANIM hWorldAnim;
	uint32 cWorldAnims;
	LOAD_DWORD(cWorldAnims);
	for( uint32 iWorldAnim = 0; iWorldAnim < cWorldAnims; ++iWorldAnim )
	{
		LOAD_DWORD(hWorldAnim);
		m_lstWorldAnims.push_back(hWorldAnim);
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
	LOAD_DOUBLE( m_fFadeStartTime );
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
//	ROUTINE:	Prop::SendActivateMessage
//
//	PURPOSE:	Send the activate message.  Toggles between 2 different
//				messages.
//
// ----------------------------------------------------------------------- //

void Prop::SendActivateMessage( HOBJECT hSender )
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

	// Queue the command.
	g_pCmdMgr->QueueCommand( pszActivateCommand, hSender, NULL );
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
		s_vtAttachmentMinVel.Init( g_pLTServer, "AttachmentMinVel", NULL, DEFAULT_ATTACH_MIN_VEL );
	}
	if( !s_vtAttachmentMaxVel.IsInitted() )
	{
		s_vtAttachmentMaxVel.Init( g_pLTServer, "AttachmentMaxVel", NULL, DEFAULT_ATTACH_MAX_VEL );
	}
	if( !s_vtAttachmentMinYAdd.IsInitted() )
	{
		s_vtAttachmentMinYAdd.Init( g_pLTServer, "AttachmentMinYAdd", NULL, DEFAULT_ATTACH_MIN_YADD );
	}
	if( !s_vtAttachmentMaxYAdd.IsInitted() )
	{
		s_vtAttachmentMaxYAdd.Init( g_pLTServer, "AttachmentMaxYAdd", NULL, DEFAULT_ATTACH_MAX_YADD );
	}
	if( !s_vtAttachmentFadeDelay.IsInitted() )
	{
		s_vtAttachmentFadeDelay.Init( g_pLTServer, "AttachmentFadeDelay", NULL, DEFAULT_ATTACH_FADE_DELAY );
	}
	if( !s_vtAttachmentFadeDuration.IsInitted() )
	{
		s_vtAttachmentFadeDuration.Init( g_pLTServer, "AttachmentFadeDuration", NULL, DEFAULT_ATTACH_FADE_DURATION );
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
		g_pModelLT->SetCurAnim( m_hObject, MAIN_TRACKER, hWorldAnim, true );
	}
	

	LTVector vVel = vDir;
	vVel.Normalize();
	vVel *= GetRandom( s_vtAttachmentMinVel.GetFloat(), s_vtAttachmentMaxVel.GetFloat() );
	vVel.y += GetRandom( s_vtAttachmentMinYAdd.GetFloat(), s_vtAttachmentMaxYAdd.GetFloat() );
	
	g_pPhysicsLT->SetVelocity( m_hObject, vVel );

	// Don't play the touch animation and sounds when recieving a touch notify...

	m_bAttachmentShotOff = true;

	g_pLTServer->SetBlockingPriority( m_hObject, 0 );
	g_pPhysicsLT->SetForceIgnoreLimit( m_hObject, 0.0f );

	// Give it some rotation...

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

        float fDot1 = info.m_Plane.DistTo( vP0 ) - info.m_Plane.m_Dist;
        float fDot2 = info.m_Plane.DistTo( vP1 ) - info.m_Plane.m_Dist;

		if( fDot1 < 0.0f && fDot2 < 0.0f || fDot1 > 0.0f && fDot2 > 0.0f )
		{
			vPos = vP1;
		}
		else
		{
			vPos = vP0.Lerp(vP1, -fDot1 / (fDot2 - fDot1));
		}

		// Set our new "real" pos...

        g_pLTServer->SetObjectPos( m_hObject, vPos );
	}

	// If we're on the ground (or an object), stop movement...

	CollisionInfo standingInfo;
    g_pLTServer->Physics()->GetStandingOn( m_hObject, &standingInfo );

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
				g_pLTServer->SetObjectRotation( m_hObject, rRot );

				m_bRotating = false;
				float fFadeDuration = s_vtAttachmentFadeDuration.GetFloat();
				if (IsMultiplayerGameServer())
				{
					fFadeDuration = 0.0f;
				}
				StartFade( fFadeDuration, s_vtAttachmentFadeDelay.GetFloat() );
			}
		}
	}
	
	// Remove the stoping velocity...

	vVel = -info.m_vStopVel;
	g_pPhysicsLT->SetVelocity( m_hObject, vVel );
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
	m_fFadeDuration		= Clamp( fDuration, 0.0f, 100000.0f );
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

	double fCurTime = g_pLTServer->GetTime();
	bool bDone = ( m_fFadeDuration <= MATH_EPSILON ) && ( m_fFadeStartTime <= fCurTime );

	// Update the fade if the delay has past...

	if( !bDone && ( m_fFadeStartTime <= fCurTime ) )
	{
		float t = (float)((fCurTime - m_fFadeStartTime) / m_fFadeDuration);
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
	if (m_bFirstUpdate)
	{
		// Make sure object starts on floor...
		m_bFirstUpdate = false;

		if (m_bMoveToFloor)
			MoveObjectToFloor(m_hObject);

		SetNextUpdate(UPDATE_NEXT_FRAME);
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
			g_pLTServer->SetObjectRotation( m_hObject, rRot );

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

	LTVector vPos;
	LTRotation rRot;
	g_pLTServer->GetObjectPos(m_hObject,&vPos);
	g_pLTServer->GetObjectRotation(m_hObject,&rRot);

}

//-----------------------------------------------------------------------------
// Prefetching
//-----------------------------------------------------------------------------

void Prop::GetPrefetchResourceList(const char* pszObjectName, IObjectResourceGatherer* pInterface, ResourceList& Resources )
{
	char pszPropType[256] = {'\0'};

	// Get prop's database records...
	pInterface->GetPropString(pszObjectName, "Model", pszPropType, LTARRAYSIZE(pszPropType), "");
	if (LTStrEmpty(pszPropType))
		return;

	// Get prop resources through its database records...
	PropsDB::HPROP hProp = g_pPropsDB->GetPropByRecordName(pszPropType);
	GetModelResources(Resources, hProp);
	GetMaterialListResources(Resources, hProp);
}
