// ----------------------------------------------------------------------- //
//
// MODULE  : ServerNonPointSound.cpp
//
// PURPOSE : A non-point sound object. This is the server side representation
//			and mostly exists for the editor. Info is then transmitted to the
//			client for actual work.
//
// CREATED : 08/18/04
//
// (c) 1998-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "SFXMsgIds.h"
#include "ServerNonPointSound.h"
#include "ServerSoundZoneVolume.h"
#include "iltserver.h"
#include "ServerUtilities.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "ServerSoundMgr.h"
#include "SoundDB.h"
#include "SoundFilterDB.h"
#include "ltbasedefs.h"

LINKFROM_MODULE( ServerNonPointSound );

BEGIN_CLASS(SoundNonPoint)
// NOTE: the following 3 props are copies of ones that exist in ltengineobjects
// and gamebase. They were re-added here so I could set them to be hidden, as
// requested by the sound guys. -- Terry
ADD_VECTORPROP_FLAG(Pos, PF_DISTANCE | PF_HIDDEN, "The position in the world of this object.")
ADD_ROTATIONPROP_FLAG(Rotation, PF_HIDDEN, "The orientation of this object represented by euler angles in degrees")
ADD_BOOLPROP_FLAG(Template, false, PF_HIDDEN, "Indicates that this object will not exist at runtime, and may be used as an object template for Spawner objects")


ADD_BOOLPROP(StartOn, true, "This flag toggles the sound to start in the on state.")
ADD_STRINGPROP_FLAG(Sound, "", PF_FILENAME, "The path to any .wav file may be entered here. This sound file will be played when the SoundFX object is triggered on.")
ADD_LONGINTPROP(Priority, -1.0f, "You can assign Priority values to SoundFX objects so that some sounds will take priority over others. The valid values are 0, 1, and 2. Lower numbers have lower priority.")
ADD_REALPROP_FLAG(OuterRadius, 500.0f, PF_RADIUS, "See InnerRadius.")
ADD_REALPROP_FLAG(InnerRadius, 100.0f, PF_RADIUS, "It's useful and simple to give a sound two radii, one Inner where it plays at full Volume, and one Outer where it plays at a reduced volume. It's a cheap way of having a sound fade over distance without requiring possibly expensive audio processing in-game. The values a set in WorldEdit units.")
ADD_LONGINTPROP(Volume, 100, "This value is expressed as a percentage. It is how loud the sound is at its origin point.")
ADD_REALPROP(PitchShift, 1.0f, "This value speeds up or slows down the file.  You can set a different pitch with this property to make it play lower or higher in pitch.  1.00 is normal pitch, 1.10 would be pitched up 10%, 0.90 would be 10% pitched down.  It does this by changing the playback sample rate of the file.")
ADD_BOOLPROP(Loop, 1, "This flag toogles whether the sound will loop continuously or play only once when triggered on.")
ADD_STRINGPROP_FLAG(Filter, "UnFiltered", PF_STATICLIST, "Every sound played in FEAR may have some type of software filtering done to it before it is passed off to the sound card.  The current implementation allows sounds to independently specify what filter they use when they are played, or (by default) use the 'dynamic' filter associated with the current listener (camera) position.  Also sounds can specify that they do not want to be filtered.")
ADD_LONGINTPROP(MixChannel, PLAYSOUND_MIX_DEFAULT, "This is the mixer channel for the sound effect. Putting -1 will use the sound class's default value.")
ADD_REALPROP(DopplerFactor, 1.0f, "This is Doppler multiplier. 1 is default (no change), and 0 turns off Doppler.")
ADD_STRINGPROP_FLAG(UseOcclusion, "Full", PF_STATICLIST, "Setting this flag to control the type of occlusion. 'No Inner Radius' means no occlusion inside the inner radius. (3D sounds only).")

ADD_STRINGPROP_FLAG(SoundZone0, "", PF_OBJECTLINK, "The name of the object that defines part of the nonpoint sound.")
ADD_STRINGPROP_FLAG(SoundZone1, "", PF_OBJECTLINK, "The name of the object that defines part of the nonpoint sound.")
ADD_STRINGPROP_FLAG(SoundZone2, "", PF_OBJECTLINK, "The name of the object that defines part of the nonpoint sound.")
ADD_STRINGPROP_FLAG(SoundZone3, "", PF_OBJECTLINK, "The name of the object that defines part of the nonpoint sound.")
ADD_STRINGPROP_FLAG(SoundZone4, "", PF_OBJECTLINK, "The name of the object that defines part of the nonpoint sound.")

END_CLASS_FLAGS_PLUGIN(SoundNonPoint, GameBase, 0, CServerNonPointSoundPlugin, "SoundNonPoint is used to specify a sound to be played from a non-point source.  Examples of it's use are playing water sounds flowing through the length of a pipe." )

CMDMGR_BEGIN_REGISTER_CLASS( SoundNonPoint )

ADD_MESSAGE( ON,		1,	NULL,	MSG_HANDLER( SoundNonPoint, HandleToggleOnMsg ),		"ON", "Tells the specified sound to be played", "msg SoundFX ON" )
ADD_MESSAGE( OFF,		1,	NULL,	MSG_HANDLER( SoundNonPoint, HandleToggleOffMsg ),		"OFF", "Tells the specified looping sound to stop playing after finishing", "msg SoundFX OFF" )

CMDMGR_END_REGISTER_CLASS( SoundNonPoint, GameBase )


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerNonPointSoundPlugin::Constructor/Destructor()
//
//	PURPOSE:	Handle allocating and deallocating the required plugins
//
// ----------------------------------------------------------------------- //

CServerNonPointSoundPlugin::CServerNonPointSoundPlugin()
{
	m_pSoundFilterDBPlugin = debug_new(SoundFilterDBPlugin);
}

CServerNonPointSoundPlugin::~CServerNonPointSoundPlugin()
{
	if (m_pSoundFilterDBPlugin)
	{
		debug_delete(m_pSoundFilterDBPlugin);
		m_pSoundFilterDBPlugin = NULL;
	}
}

LTRESULT CServerNonPointSoundPlugin::PreHook_EditStringList(
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
//	ROUTINE:	SoundNonPoint::ServerSoundNonPoint()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

SoundNonPoint::SoundNonPoint()
:	GameBase		( OT_NORMAL ),
	m_bLoadFromSave	( false )
{
	int32 i;

	for (i=0; i < MAX_SOUND_VOLUMES; i++)
	{
		m_hSoundZoneObj[i] = NULL;
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundNonPoint::~ServerSoundNonPoint()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

SoundNonPoint::~SoundNonPoint()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 SoundNonPoint::EngineMessageFn(uint32 messageID, void *pData, float fData)
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
				m_bLoadFromSave = false;
			}
			else
			{
				m_bLoadFromSave = true;
			}
		}
		break;

	case MID_ALLOBJECTSCREATED:

		// if not a savegame, call this to resolve object links
		// (otherwise, all this info has already been saved)
		{
			if (!m_bLoadFromSave)
			{
				AllObjectsCreated();
				CreateSpecialFX();
			}
		}
		break;

	case MID_UPDATE:
		{
			SetNextUpdate(UPDATE_NEVER); // UPDATE_NEXT_FRAME
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
//	ROUTINE:	SoundNonPoint:HandleToggleMsg
//
//	PURPOSE:	Handle a TOGGLE message...
//
// ----------------------------------------------------------------------- //

//void SoundNonPoint::HandleToggleMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
//{
//}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundNonPoint:HandleAbortMsg
//
//	PURPOSE:	Handle a ABORT message...
//
// ----------------------------------------------------------------------- //

//void SoundNonPoint::HandleAbortMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
//{
//}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundNonPoint:ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

bool SoundNonPoint::ReadProp( const GenericPropList *pProps )
{
	m_SCS.m_bStartOn		= pProps->GetBool( "StartOn", m_SCS.m_bStartOn );
	m_SCS.m_sSound			= pProps->GetString( "Sound", "" );
	m_SCS.m_nPriority		= (uint8)pProps->GetLongInt( "Priority", m_SCS.m_nPriority );
	m_SCS.m_fOuterRadius	= pProps->GetReal( "OuterRadius", m_SCS.m_fOuterRadius );
	m_SCS.m_fInnerRadius	= pProps->GetReal( "InnerRadius", m_SCS.m_fInnerRadius );
	m_SCS.m_nVolume			= (uint8)pProps->GetLongInt( "Volume", m_SCS.m_nVolume );
	m_SCS.m_fPitchShift		= pProps->GetReal( "PitchShift", m_SCS.m_fPitchShift );
	m_SCS.m_nMixChannel		= (int16)pProps->GetLongInt( "MixChannel", m_SCS.m_nMixChannel );
	m_SCS.m_fDopplerFactor	= pProps->GetReal( "DopplerFactor", m_SCS.m_fDopplerFactor );

	m_sSoundZone[0] = pProps->GetString( "SoundZone0", "" );
	m_sSoundZone[1] = pProps->GetString( "SoundZone1", "" );
	m_sSoundZone[2] = pProps->GetString( "SoundZone2", "" );
	m_sSoundZone[3] = pProps->GetString( "SoundZone3", "" );
	m_sSoundZone[4] = pProps->GetString( "SoundZone4", "" );

	const char *pszOcclusion = pProps->GetString( "UseOcclusion", "" );
	m_SCS.m_bUseOcclusion = true;
	m_SCS.m_bOcclusionNoInnerRadius = false;
	if ( pszOcclusion && pszOcclusion[0] )
	{
		if (LTStrCmp(pszOcclusion, "Full") == 0)
		{
			m_SCS.m_bUseOcclusion = true;
			m_SCS.m_bOcclusionNoInnerRadius = false;
		}
		else if (LTStrCmp(pszOcclusion, "None") == 0)
		{
			m_SCS.m_bUseOcclusion = false;
			m_SCS.m_bOcclusionNoInnerRadius = false;
		}
		else if (LTStrCmp(pszOcclusion, "No Inner Radius") == 0)
		{
			m_SCS.m_bUseOcclusion = true;
			m_SCS.m_bOcclusionNoInnerRadius = true;
		}
	}

	const char *pszFilter = pProps->GetString( "Filter", "" );
	if( pszFilter && pszFilter[0] )
	{
		m_SCS.m_hFilterRecord = SoundFilterDB::Instance().GetFilterRecord( pszFilter );
	}

	if (m_SCS.m_bStartOn)
	{
		m_SCS.m_bSoundOn = true;
	}
	else
	{
		m_SCS.m_bSoundOn = false;
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundNonPoint:PostPropRead()
//
//	PURPOSE:	Update properties
//
// ----------------------------------------------------------------------- //

void SoundNonPoint::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	pStruct->m_Flags |= FLAG_FORCECLIENTUPDATE;

	// Set the Update!

	pStruct->m_NextUpdate = 0.0f;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundNonPoint:InitialUpdate()
//
//	PURPOSE:	Handle initial update
//
// ----------------------------------------------------------------------- //

bool SoundNonPoint::InitialUpdate()
{
	SetNextUpdate(UPDATE_NEVER);

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundNonPoint:InitialUpdate()
//
//	PURPOSE:	Handle initial update
//
// ----------------------------------------------------------------------- //

bool SoundNonPoint::AllObjectsCreated()
{
	// resolve the object links. Basically, get the sound zone volumes
	// and collect their info into the non-point sound object that
	// gets created on the client.

	ObjArray <HOBJECT, 1> objArray;
	int32 i;

	m_SCS.m_nNumZones = 0;

	for (i=0; i < MAX_SOUND_VOLUMES; i++)
	{
		SoundZoneVolume* pSZV;

		if(!LTStrEmpty(m_sSoundZone[i].c_str()))
		{
			g_pLTServer->FindNamedObjects(m_sSoundZone[i].c_str(), objArray);
			if(objArray.NumObjects() > 0)
			{
				if (IsKindOf(objArray.GetObject(0), "SoundZoneVolume"))
				{
					HOBJECT hObj;

					hObj = objArray.GetObject(0);
					pSZV = (SoundZoneVolume*) g_pLTServer->HandleToObject(hObj);

					// do a type check before casting...

					if (pSZV)
					{
						g_pLTServer->GetObjectPos(hObj, &m_SCS.m_SoundZone[m_SCS.m_nNumZones].m_vPos);
						g_pLTServer->GetObjectRotation(hObj, &m_SCS.m_SoundZone[m_SCS.m_nNumZones].m_rRotation);
						m_SCS.m_SoundZone[m_SCS.m_nNumZones].m_vHalfDims = pSZV->GetHalfDims();

						m_SCS.m_nNumZones++;
					}
				}

			}
		}
	}
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundNonPoint::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void SoundNonPoint::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	m_SCS.Write(pMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundNonPoint::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void SoundNonPoint::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	m_SCS.Read(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundNonPoint::HandleToggleOnMsg
//
//	PURPOSE:	Handle a toggle on message...
//
// ----------------------------------------------------------------------- //

void SoundNonPoint::HandleToggleOnMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	m_SCS.m_bSoundOn = true;
	SendToggleMsg();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundNonPoint::HandleToggleOffMsg
//
//	PURPOSE:	Handle a toggle off message...
//
// ----------------------------------------------------------------------- //

void SoundNonPoint::HandleToggleOffMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	m_SCS.m_bSoundOn = false;
	SendToggleMsg();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundNonPoint::SendToggleMsg()
//
//	PURPOSE:	Turn the non-point sound on or off..
//
// ----------------------------------------------------------------------- //

void SoundNonPoint::SendToggleMsg()
{
	//if( !m_bSendTriggerFXMsg )
	//	return;

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SFX_MESSAGE );
	cMsg.Writeuint8( SFX_SOUND_NONPOINT_ID );
	cMsg.WriteObject( m_hObject );
	cMsg.Writeuint8( SNPFX_TOGGLE_MSG );
	cMsg.Writeuint8( m_SCS.m_bSoundOn );
	g_pLTServer->SendToClient( cMsg.Read(), NULL, MESSAGE_GUARANTEED );

	CreateSpecialFX();
}


void SoundNonPoint::CreateSpecialFX( bool bUpdateClients /* = false */ )
{
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8( SFX_SOUND_NONPOINT_ID );
		m_SCS.Write( cMsg );
		g_pLTServer->SetObjectSFXMessage( m_hObject, cMsg.Read() );
	}

	if( bUpdateClients )
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_SFX_MESSAGE );
		cMsg.Writeuint8( SFX_SOUND_NONPOINT_ID );
		m_SCS.Write( cMsg );
		cMsg.Writeuint8( SNPFX_ALLFX_MSG );
		g_pLTServer->SendToClient( cMsg.Read(), NULL, MESSAGE_GUARANTEED );
	}
}


