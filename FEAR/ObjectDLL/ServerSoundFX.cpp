// ----------------------------------------------------------------------- //
//
// MODULE  : ServerSoundFX.cpp
//
// PURPOSE : A start/stoppable ambient sound object.
//
// CREATED : 09/11/98
//
// (c) 1998-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "ServerSoundFX.h"
#include "iltserver.h"
#include "ServerUtilities.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "ServerSoundMgr.h"
#include "SoundDB.h"
#include "SoundFilterDB.h"
#include "ltbasedefs.h"

LINKFROM_MODULE( ServerSoundFx );

BEGIN_CLASS(SoundFX)
	// NOTE: the following 3 props are copies of ones that exist in ltengineobjects
	// and gamebase. They were re-added here so I could set them to be hidden, as
	// requested by the sound guys. -- Terry
	ADD_VECTORPROP_FLAG(Pos, PF_DISTANCE | PF_HIDDEN, "The position in the world of this object.")
	ADD_ROTATIONPROP_FLAG(Rotation, PF_HIDDEN, "The orientation of this object represented by euler angles in degrees")
	ADD_BOOLPROP_FLAG(Template, false, PF_HIDDEN, "Indicates that this object will not exist at runtime, and may be used as an object template for Spawner objects")


	ADD_BOOLPROP(StartOn, true, "This flag toggles the sound to start in the on state.")
	ADD_STRINGPROP_FLAG(Sound, "", PF_FILENAME, "The path to any .wav file, or a sound database entry, may be entered here. This sound file will be played when the SoundFX object is triggered on.")
	ADD_LONGINTPROP(Priority, 1.0f, "You can assign Priority values to SoundFX objects so that some sounds will take priority over others. The valid values are 0, 1, and 2. Lower numbers have lower priority.")
	ADD_REALPROP_FLAG(OuterRadius, 500.0f, PF_RADIUS, "See InnerRadius.")
	ADD_REALPROP_FLAG(InnerRadius, 100.0f, PF_RADIUS, "It's useful and simple to give a sound two radii, one Inner where it plays at full Volume, and one Outer where it plays at a reduced volume. It's a cheap way of having a sound fade over distance without requiring possibly expensive audio processing in-game. The values a set in WorldEdit units.")
	ADD_LONGINTPROP(Volume, 100, "This value is expressed as a percentage. It is how loud the sound is at its origin point.")
	ADD_REALPROP(PitchShift, 1.0f, "This value speeds up or slows down the file.  You can set a different pitch with this property to make it play lower or higher in pitch.  1.00 is normal pitch, 1.10 would be pitched up 10%, 0.90 would be 10% pitched down.  It does this by changing the playback sample rate of the file.")
	ADD_BOOLPROP(Ambient, 1, "Setting Ambient to TRUE means that the sound will play as if it were coming from everywhere around you, but will get quieter as you move farther away from it.  Setting Ambient to FALSE means that the sound will play from a specific point.")
	ADD_BOOLPROP(Loop, 1, "This flag toogles whether the sound will loop continuously or play only once when triggered on.")
	ADD_BOOLPROP(PlayAttached, 0, "Setting this flag to true will allow its origin point to move. This flag is useful if the sound is to be attached to a moving object.")
	ADD_STRINGPROP_FLAG(Filter, "UnFiltered", PF_STATICLIST, "Every sound played in FEAR may have some type of software filtering done to it before it is passed off to the sound card.  The current implementation allows sounds to independently specify what filter they use when they are played, or (by default) use the 'dynamic' filter associated with the current listener (camera) position.  Also sounds can specify that they do not want to be filtered.")
	ADD_LONGINTPROP(MixChannel, PLAYSOUND_MIX_DEFAULT, "This is the mixer channel for the sound effect. Putting -1 will use the sound class's default value.")
	ADD_REALPROP(DopplerFactor, 1.0f, "This is Doppler multiplier. 1 is default (no change), and 0 turns off Doppler.")
	ADD_STRINGPROP_FLAG(UseOcclusion, "Full", PF_STATICLIST, "Setting this flag to control the type of occlusion. 'No Inner Radius' means no occlusion inside the inner radius. (3D sounds only).")
	ADD_BOOLPROP(VolumeFade, 0, "Setting VolumeFade to TRUE means that the sound can be controlled by VOLUME messages. This is for optimization so only set for sounds you need to do this to.")
	
	ADD_PREFETCH_RESOURCE_PROPS()

END_CLASS_FLAGS_PLUGIN_PREFETCH(SoundFX, GameBase, 0, CSoundFXPlugin, DefaultPrefetch<SoundFX>, "SoundFX objects are used to play a specific sound file at a specific position in the database." )

CMDMGR_BEGIN_REGISTER_CLASS( SoundFX )

	ADD_MESSAGE( TOGGLE,	1,	NULL,	MSG_HANDLER( SoundFX, HandleToggleMsg ),	"TOGGLE", "Toggles the on/off state of the sound.", "msg SoundFX TOGGLE" )
	ADD_MESSAGE( ON,		1,	NULL,	MSG_HANDLER( SoundFX, HandleOnMsg ),		"ON", "Tells the specified sound to be played", "msg SoundFX ON" )
	ADD_MESSAGE( OFF,		1,	NULL,	MSG_HANDLER( SoundFX, HandleOffMsg ),		"OFF", "Tells the specified looping sound to stop playing after finishing", "msg SoundFX OFF" )
	ADD_MESSAGE( ABORT,		1,	NULL,	MSG_HANDLER( SoundFX, HandleAbortMsg ),		"ABORT", "Tells the specified looping sound to stop playing immediately", "msg SoundFX ABORT" )
	ADD_MESSAGE( VOLUME,	3,	NULL,	MSG_HANDLER( SoundFX, HandleVolumeMsg ),	"VOLUME", "Tells the specified sound to change volume. Set a time to fade, if desired.", "msg SoundFX VOLUME [volume] [fadetime]" )

CMDMGR_END_REGISTER_CLASS( SoundFX, GameBase )



const float SOUND_DONE_POLL_TIME=1.0f;
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundFXPlugin::Constructor/Destructor()
//
//	PURPOSE:	Handle allocating and deallocating the required plugins
//
// ----------------------------------------------------------------------- //

CSoundFXPlugin::CSoundFXPlugin()
{
	m_pSoundFilterDBPlugin = debug_new(SoundFilterDBPlugin);
}

CSoundFXPlugin::~CSoundFXPlugin()
{
	if (m_pSoundFilterDBPlugin)
	{
		debug_delete(m_pSoundFilterDBPlugin);
		m_pSoundFilterDBPlugin = NULL;
	}
}

LTRESULT CSoundFXPlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char** aszStrings,
	uint32* pcStrings,
	const uint32 cMaxStrings,
	const uint32 cMaxStringLength)
{
	if( LTStrIEquals( "Filter", szPropName ))
	{
		if (!m_pSoundFilterDBPlugin->PopulateStringList(aszStrings, pcStrings, cMaxStrings, cMaxStringLength))
		{
			return LT_UNSUPPORTED;
		}

		return LT_OK;
	}
	else if( LTStrIEquals( "UseOcclusion", szPropName ))
	{
		if (cMaxStrings < 3)
		{
			return LT_UNSUPPORTED;
		}

		LTStrCpy(aszStrings[0], "None", cMaxStringLength);
		LTStrCpy(aszStrings[1], "Full", cMaxStringLength);
		LTStrCpy(aszStrings[2], "No Inner Radius", cMaxStringLength);

		*pcStrings=3;


		return LT_OK;
	}


	return LT_UNSUPPORTED;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFX::SoundFX()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

SoundFX::SoundFX()
:	GameBase		( OT_NORMAL ),
	m_bStartOn		( true ),
	m_sSound		( ),
	m_hsndSound		( NULL ),
	m_hFilterRecord	( NULL ),
	m_fOuterRadius	( 0.0f ),
	m_fInnerRadius	( 0.0f ),
	m_nVolume		( 0 ),
	m_fPitchShift	( 1.0f ),
	m_bAmbient		( false ),
	m_bControlVolume		( false ),
	m_bLooping		( true ),
	m_bAttached		( false ),
	m_nPriority		( SOUNDPRIORITY_MISC_LOW ),
	m_nMixChannel	( PLAYSOUND_MIX_DEFAULT ),
	m_fDopplerFactor (1.0f),
	m_bUseOcclusion (true),
	m_bOcclusionNoInnerRadius (false)
{
	
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFX::~SoundFX()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

SoundFX::~SoundFX()
{
	if (m_hsndSound)
	{
		g_pLTServer->SoundMgr()->KillSound(m_hsndSound);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 SoundFX::EngineMessageFn(uint32 messageID, void *pData, float fData)
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
			if (m_hsndSound)
			{
				bool bDone;

				g_pLTServer->SoundMgr()->IsSoundDone(m_hsndSound, bDone);

				if (bDone)
				{
					g_pLTServer->SoundMgr()->KillSound(m_hsndSound);
					m_hsndSound = NULL;
				}
				else
				{
					SetNextUpdate(SOUND_DONE_POLL_TIME);
					break;
				}
			}

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
//	ROUTINE:	SoundFX:HandleToggleMsg
//
//	PURPOSE:	Handle a TOGGLE message...
//
// ----------------------------------------------------------------------- //

void SoundFX::HandleToggleMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if (m_hsndSound)
	{
		if (m_bLooping)
		{
			g_pLTServer->SoundMgr()->KillSoundLoop(m_hsndSound);
		}
		else
		{
			g_pLTServer->SoundMgr()->KillSound(m_hsndSound);
		}

		m_hsndSound = NULL;
	}
	else
	{
		PlaySound( );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFX:HandleOnMsg
//
//	PURPOSE:	Handle a ON message...
//
// ----------------------------------------------------------------------- //

void SoundFX::HandleOnMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg  )
{
	PlaySound();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFX:HandleOffMsg
//
//	PURPOSE:	Handle a OFF message...
//
// ----------------------------------------------------------------------- //

void SoundFX::HandleOffMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if (m_hsndSound)
	{
		if (m_bLooping)
		{
			g_pLTServer->SoundMgr()->KillSoundLoop(m_hsndSound);
		}
		else
		{
			g_pLTServer->SoundMgr()->KillSound(m_hsndSound);
		}
		m_hsndSound = NULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFX:HandleVolumeMsg
//
//	PURPOSE:	Handle a VOLUME message...
//
// ----------------------------------------------------------------------- //

void SoundFX::HandleVolumeMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	uint8 nVol=0;
	float fFadeTime=0.0f;

	if (crParsedMsg.GetArgCount() > 1) 
	{
		nVol = (uint8)atoi(crParsedMsg.GetArg(1));
	}

	if (crParsedMsg.GetArgCount() > 2) 
	{
		fFadeTime = (float)atof(crParsedMsg.GetArg(2));
	}

	if (m_hsndSound)
	{
		g_pLTServer->SoundMgr()->SetSoundVolume(m_hsndSound, nVol, fFadeTime);
		m_nVolume = nVol;
	}
	else
	{
		m_nVolume = nVol;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFX:HandleAbortMsg
//
//	PURPOSE:	Handle a ABORT message...
//
// ----------------------------------------------------------------------- //

void SoundFX::HandleAbortMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if (m_hsndSound)
	{
		g_pLTServer->SoundMgr()->KillSound(m_hsndSound);
		m_hsndSound = NULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFX:ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

bool SoundFX::ReadProp( const GenericPropList *pProps )
{
	m_bStartOn		= pProps->GetBool( "StartOn", m_bStartOn );
	m_sSound		= pProps->GetString( "Sound", "" );
	m_nPriority		= (uint8)pProps->GetLongInt( "Priority", m_nPriority );
	m_fOuterRadius	= pProps->GetReal( "OuterRadius", m_fOuterRadius );
	m_fInnerRadius	= pProps->GetReal( "InnerRadius", m_fInnerRadius );
	m_nVolume		= (uint8)pProps->GetLongInt( "Volume", m_nVolume );
	m_fPitchShift	= pProps->GetReal( "PitchShift", m_fPitchShift );
	m_bAmbient		= pProps->GetBool( "Ambient", m_bAmbient );
	m_bControlVolume	= pProps->GetBool( "VolumeFade", m_bControlVolume );
	m_bLooping		= pProps->GetBool( "Loop", m_bLooping );
	m_bAttached		= pProps->GetBool( "PlayAttached", m_bAttached );
	m_nMixChannel	= (int16)pProps->GetLongInt( "MixChannel", m_nMixChannel );
	m_fDopplerFactor	= pProps->GetReal( "DopplerFactor", m_fDopplerFactor );

	//m_bUseOcclusion	= pProps->GetBool( "UseOcclusion", m_bUseOcclusion );
	//m_bOcclusionNoInnerRadius = ;

	const char *pszOcclusion = pProps->GetString( "UseOcclusion", "" );
	m_bUseOcclusion = true;
	m_bOcclusionNoInnerRadius = false;
	if ( pszOcclusion && pszOcclusion[0] )
	{
		if (LTStrCmp(pszOcclusion, "Full") == 0)
		{
			m_bUseOcclusion = true;
			m_bOcclusionNoInnerRadius = false;
		}
		else if (LTStrCmp(pszOcclusion, "None") == 0)
		{
			m_bUseOcclusion = false;
			m_bOcclusionNoInnerRadius = false;
		}
		else if (LTStrCmp(pszOcclusion, "No Inner Radius") == 0)
		{
			m_bUseOcclusion = true;
			m_bOcclusionNoInnerRadius = true;
		}
	}

	const char *pszFilter = pProps->GetString( "Filter", "" );
	if( pszFilter && pszFilter[0] )
	{
		m_hFilterRecord = SoundFilterDB::Instance().GetFilterRecord( pszFilter );
	}
	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFX:PostPropRead()
//
//	PURPOSE:	Update properties
//
// ----------------------------------------------------------------------- //

void SoundFX::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	// Set the Update!

	pStruct->m_NextUpdate = 0.0f;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFX:InitialUpdate()
//
//	PURPOSE:	Handle initial update
//
// ----------------------------------------------------------------------- //

bool SoundFX::InitialUpdate()
{
	if (m_bStartOn)
	{
		PlaySound();
	}
	else
	{
		SetNextUpdate(UPDATE_NEVER);
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFX::PlaySound
//
//	PURPOSE:	Plays the requested sound file.
//
// ----------------------------------------------------------------------- //

void SoundFX::PlaySound()
{
	// Kill the current sound

	if (m_hsndSound)
	{
		g_pLTServer->SoundMgr()->KillSound(m_hsndSound);
		m_hsndSound = NULL;
	}

	// Play the sound...

	if( !m_sSound.empty() )
	{
		HRECORD hSR = g_pSoundDB->GetSoundDBRecord(m_sSound.c_str() );
		if( hSR )
		{
			uint32 dwFlags = PLAYSOUND_GETHANDLE | PLAYSOUND_TIME | PLAYSOUND_REVERB;
			SoundPriority priority;

			dwFlags |= (m_nVolume < 100 ? PLAYSOUND_CTRL_VOL : 0);
			dwFlags |= (m_fPitchShift != 1.0F ? PLAYSOUND_CTRL_PITCH : 0);

			if (m_bControlVolume)
			{
				dwFlags |= PLAYSOUND_ALLOW_FADE_CONTROL;
			}
			dwFlags |= (m_bAmbient ? PLAYSOUND_AMBIENT : PLAYSOUND_3D);
			dwFlags |= (m_bUseOcclusion ? PLAYSOUND_USEOCCLUSION : 0);
			dwFlags |= (m_bOcclusionNoInnerRadius ? PLAYSOUND_USEOCCLUSION_NO_INNER_RADIUS : 0);
			if (m_bLooping)
			{
				dwFlags |= PLAYSOUND_LOOP;
			}

			if (m_nPriority == 0xff)
			{
				priority = SOUNDPRIORITY_INVALID;
			}
			else
			{
				priority = SOUNDPRIORITY_MISC_MEDIUM;
			}

			m_hsndSound = g_pServerSoundMgr->PlayDBSoundFromObject( m_hObject, hSR, m_fOuterRadius, priority, 
																  dwFlags, m_nVolume, m_fPitchShift, m_fInnerRadius,
																  DEFAULT_SOUND_CLASS, PLAYSOUND_MIX_OBJECTS);
		}
		else
		{
			PlaySoundInfo playSoundInfo;

			PLAYSOUNDINFO_INIT(playSoundInfo);

			playSoundInfo.m_dwFlags = PLAYSOUND_GETHANDLE | PLAYSOUND_TIME | PLAYSOUND_REVERB;
			if (m_bLooping)
			{
				playSoundInfo.m_dwFlags |= PLAYSOUND_LOOP;
			}

			if (m_bAttached)
			{
				playSoundInfo.m_dwFlags |= PLAYSOUND_ATTACHED;
				playSoundInfo.m_hObject = m_hObject;
			}

			LTStrCpy(playSoundInfo.m_szSoundName, m_sSound.c_str(), LTARRAYSIZE(playSoundInfo.m_szSoundName));
			playSoundInfo.m_nPriority = m_nPriority;
			playSoundInfo.m_fOuterRadius = m_fOuterRadius;
			playSoundInfo.m_fInnerRadius = m_fInnerRadius;
			playSoundInfo.m_nMixChannel = m_nMixChannel;
			playSoundInfo.m_fDopplerFactor = m_fDopplerFactor;

			if (m_nVolume < 100)
			{
				playSoundInfo.m_nVolume = m_nVolume;
				playSoundInfo.m_dwFlags |= PLAYSOUND_CTRL_VOL;
			}

			if (m_fPitchShift != 1.0f)
			{
				playSoundInfo.m_dwFlags |= PLAYSOUND_CTRL_PITCH;
				playSoundInfo.m_fPitchShift = m_fPitchShift;
			}

			g_pLTServer->GetObjectPos(m_hObject, &playSoundInfo.m_vPosition);
			if (m_bControlVolume)
			{
				playSoundInfo.m_dwFlags |= PLAYSOUND_ALLOW_FADE_CONTROL;
			}
			
			if (m_bAmbient)
			{
				playSoundInfo.m_dwFlags |= PLAYSOUND_AMBIENT;
			}
			else
			{
				playSoundInfo.m_dwFlags |= PLAYSOUND_3D;
				if (m_bUseOcclusion)
				{
					playSoundInfo.m_dwFlags |= PLAYSOUND_USEOCCLUSION;
				}
				if (m_bOcclusionNoInnerRadius)
				{
					playSoundInfo.m_dwFlags |= PLAYSOUND_USEOCCLUSION_NO_INNER_RADIUS;
				}
			}

			m_hsndSound = g_pServerSoundMgr->PlaySoundDirect(playSoundInfo);
		}

		if (!m_bLooping && m_hsndSound)
		{
			// start the sound polling
			SetNextUpdate(SOUND_DONE_POLL_TIME);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFX::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void SoundFX::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	SAVE_STDSTRING(m_sSound);

	SAVE_FLOAT(m_fOuterRadius);
	SAVE_FLOAT(m_fInnerRadius);
	SAVE_BYTE(m_nVolume);
	SAVE_HRECORD(m_hFilterRecord);
	SAVE_FLOAT(m_fPitchShift);
	SAVE_BOOL(m_bAmbient);
	SAVE_BOOL(m_bControlVolume);
	SAVE_BOOL(m_bLooping);
	SAVE_BOOL(m_bAttached);
	SAVE_BYTE(m_nPriority);
	SAVE_bool(m_hsndSound != 0);
	SAVE_WORD(m_nMixChannel);
	SAVE_FLOAT(m_fDopplerFactor);
	SAVE_BOOL(m_bUseOcclusion);
	SAVE_BOOL(m_bOcclusionNoInnerRadius);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFX::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void SoundFX::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	m_hsndSound		= NULL;
	
	LOAD_STDSTRING(m_sSound);
	LOAD_FLOAT(m_fOuterRadius);
	LOAD_FLOAT(m_fInnerRadius);
	LOAD_BYTE(m_nVolume);
	LOAD_HRECORD(m_hFilterRecord, SoundFilterDB::Instance().GetSoundFilterCategory());
	LOAD_FLOAT(m_fPitchShift);
	LOAD_BOOL(m_bAmbient);
	LOAD_BOOL(m_bControlVolume);
	LOAD_BOOL(m_bLooping);
	LOAD_BOOL(m_bAttached);
	LOAD_BYTE(m_nPriority);

	bool bSoundPlaying;
	LOAD_bool(bSoundPlaying);
	LOAD_WORD(m_nMixChannel);
	LOAD_FLOAT(m_fDopplerFactor);
	LOAD_BOOL(m_bUseOcclusion);
	LOAD_BOOL(m_bOcclusionNoInnerRadius);
	if (bSoundPlaying && m_bLooping)
	{
		PlaySound();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFX::GetPrefetchResourceList
//
//	PURPOSE:	determines the list of all needed resources
//
// ----------------------------------------------------------------------- //

void SoundFX::GetPrefetchResourceList(const char* pszObjectName, IObjectResourceGatherer* pInterface, ResourceList& Resources )
{
	AddSoundResourceToObjectGatherer(pInterface, Resources, pszObjectName, "Sound");
}
