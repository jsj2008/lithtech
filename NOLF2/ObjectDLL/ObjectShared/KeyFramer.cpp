// ----------------------------------------------------------------------- //
//
// MODULE  : Keyframer.cpp
//
// PURPOSE : Keyframer implementation
//
// CREATED : 10/7/97
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "KeyFramer.h"
#include "iltserver.h"
#include "ServerUtilities.h"
#include "stdio.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "SoundMgr.h"
#include "ltbeziercurve.h"
#include "CVarTrack.h"
#include "CommandMgr.h"
#include "ServerSoundMgr.h"

CVarTrack	g_vtDisableKeyframers;

LINKFROM_MODULE( KeyFramer );


// Makes some code cleaner when calling the bezier routines...
#define CURVE_PTS(keyData1, keyData2) \
	(keyData1).m_vPos, (keyData1).m_BezierNextCtrl, (keyData2).m_BezierPrevCtrl, (keyData2).m_vPos

extern const char* c_aWaveTypes[]; 


LTFLOAT GetKFWaveValue(LTFLOAT fSpeed, LTFLOAT fPercent, KFWaveType eWaveType)
{
	if (eWaveType == KFWAVE_LINEAR) return fSpeed;

	LTFLOAT fNewSpeed = 0.0f;
	LTFLOAT fScalePercent = fSpeed * 0.003f;

	switch (eWaveType)
	{
		case KFWAVE_SINE:
			fNewSpeed = MATH_HALFPI * fSpeed * (LTFLOAT)sin( fPercent * MATH_PI );
		break;

		case KFWAVE_SLOWOFF:
			fNewSpeed = MATH_HALFPI * fSpeed * (LTFLOAT)cos( fPercent * MATH_HALFPI );
		break;

		case KFWAVE_SLOWON:
			fNewSpeed = MATH_HALFPI * fSpeed * (LTFLOAT)sin( fPercent * MATH_HALFPI ) + fScalePercent;
		break;
	}

	return fNewSpeed;
}


#pragma force_active on
BEGIN_CLASS(KeyFramer)
	ADD_KEYFRAMER_PROPERTIES(0)
END_CLASS_DEFAULT_FLAGS_PLUGIN(KeyFramer, GameBase, LTNULL, LTNULL, 0, KeyframerPlugin)
#pragma force_active off


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LTBOOL ValidateGotoMsg
//
//  PURPOSE:	Make sure GOTO message works
//
// ----------------------------------------------------------------------- //

static LTBOOL ValidateGotoMsg( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( !pInterface ) return LTFALSE;

	// Check the External File property to see if the keys exist somewhere else...

	char *pObj = CCommandMgrPlugin::GetCurrentObjectName();

	GenericProp gProp;
	if( pInterface->GetPropGeneric( pObj, "ExternalKeyFile", &gProp ) == LT_OK )
	{
		if( gProp.m_String[0] )
		{
			return LTTRUE;
		}
	}

	if( LT_NOTFOUND == pInterface->FindObject( cpMsgParams.m_Args[1] ))
	{
		if( CCommandMgrPlugin::s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow( LTTRUE );
			pInterface->CPrint( "ERROR! - ValidateGotoMsg()" );
			pInterface->CPrint( "    MSG - GOTO - Could not find object '%s'!", cpMsgParams.m_Args[1] );
		}
		
		return LTFALSE;
	}

	if( !_stricmp( pInterface->GetObjectClass( cpMsgParams.m_Args[1] ), "Key" ))
	{
		return LTTRUE;
	}

	if( CCommandMgrPlugin::s_bShowMsgErrors )
	{
		pInterface->ShowDebugWindow( LTTRUE );
		pInterface->CPrint( "ERROR! - ValidateGotoMsg()" );
		pInterface->CPrint( "    MSG - GOTO - Object '%s' is not of type 'Key'!", cpMsgParams.m_Args[1] );
	}
	
	return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LTBOOL ValidateMoveToMsg
//
//  PURPOSE:	Make sure MOVETO message is valid and the command it might send
//
// ----------------------------------------------------------------------- //

static LTBOOL ValidateMoveToMsg( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( !pInterface ) return LTFALSE;

	if( cpMsgParams.m_nArgs < 2 || cpMsgParams.m_nArgs > 3 )
	{
		if( CCommandMgrPlugin::s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow( LTTRUE );
			pInterface->CPrint( "ERROR! - ValidateMoveToMsg()" );
			pInterface->CPrint( "    MSG - MOVETO - Invalid number of arguments!", cpMsgParams.m_Args[1] );
		}
		
		return LTFALSE;
	}

	// Check the External File property to see if the keys exist somewhere else...

	char *pObj = CCommandMgrPlugin::GetCurrentObjectName();

	bool bExternalKeyFile = false;
	GenericProp gProp;

	if( pInterface->GetPropGeneric( pObj, "ExternalKeyFile", &gProp ) == LT_OK )
	{
		if( gProp.m_String[0] )
		{
			bExternalKeyFile = true;
		}
	}

	if( !bExternalKeyFile && (LT_NOTFOUND == pInterface->FindObject( cpMsgParams.m_Args[1] )) )
	{
		if( CCommandMgrPlugin::s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow( LTTRUE );
			pInterface->CPrint( "ERROR! - ValidateMoveToMsg()" );
			pInterface->CPrint( "    MSG - MOVETO - Could not find object '%s'!", cpMsgParams.m_Args[1] );
		}
		
		return LTFALSE;
	}

	if( bExternalKeyFile || (!_stricmp( pInterface->GetObjectClass( cpMsgParams.m_Args[1] ), "Key" )) )
	{
		if( cpMsgParams.m_nArgs == 3 && cpMsgParams.m_Args[2] )
		{
			CCommandMgrPlugin cmdmgr;

			if( cmdmgr.IsValidCmd( pInterface, cpMsgParams.m_Args[2] ))
			{
				return LTTRUE;
			}
			
			if( CCommandMgrPlugin::s_bShowMsgErrors )
			{
				pInterface->ShowDebugWindow( LTTRUE );
				pInterface->CPrint( "ERROR! - ValidateMoveToMsg()" );
				pInterface->CPrint( "    MSG - MOVETO - Command '%s' is not valid!", cpMsgParams.m_Args[2] );
			}
			
			return LTFALSE;

		}

		return LTTRUE;
	}

	if( CCommandMgrPlugin::s_bShowMsgErrors )
	{
		pInterface->ShowDebugWindow( LTTRUE );
		pInterface->CPrint( "ERROR! - ValidateMoveToMsg()" );
		pInterface->CPrint( "    MSG - MOVETO - Object '%s' is not of type 'Key'!", cpMsgParams.m_Args[1] );
	}
	
	return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LTBOOL ValidateTargetMsg
//
//  PURPOSE:	Make sure TARGET message is good
//
// ----------------------------------------------------------------------- //

static LTBOOL ValidateTargetMsg( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( !pInterface ) return LTFALSE;

	if( LT_NOTFOUND == pInterface->FindObject( cpMsgParams.m_Args[1] ))
	{
		if( CCommandMgrPlugin::s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow( LTTRUE );
			pInterface->CPrint( "ERROR! - ValidateTargetMsg()" );
			pInterface->CPrint( "    MSG - TARGET - Could not find object '%s'!", cpMsgParams.m_Args[1] );
		}
		
		return LTFALSE;
	}

	return LTTRUE;
}

//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( KeyFramer )

//					Message		Num Params	Validation FnPtr		Syntax

	CMDMGR_ADD_MSG( ON,			1,			NULL,					"ON" )
	CMDMGR_ADD_MSG( OFF,		1,			NULL,					"OFF" )
	CMDMGR_ADD_MSG( PAUSE,		1,			NULL,					"PAUSE" )
	CMDMGR_ADD_MSG( RESUME,		1,			NULL,					"RESUME" )
	CMDMGR_ADD_MSG( FORWARD,	1,			NULL,					"FORWARD" )
	CMDMGR_ADD_MSG( BACKWARD,	1,			NULL,					"BACKWARD" )
	CMDMGR_ADD_MSG( REVERSE,	1,			NULL,					"REVERSE" )
	CMDMGR_ADD_MSG( TOGGLEDIR,	1,			NULL,					"TOGGLEDIR" )
	CMDMGR_ADD_MSG( GOTO,		2,			ValidateGotoMsg,		"GOTO <key>" )
	CMDMGR_ADD_MSG( MOVETO,		-1,			ValidateMoveToMsg,		"MOVETO <key> [destination cmd]" )
	CMDMGR_ADD_MSG( TARGET,		2,			ValidateTargetMsg,		"TARGET <object>" )
	CMDMGR_ADD_MSG( CLEARTARGET, 1,			NULL,					"CLEARTARGET" )
	CMDMGR_ADD_MSG( TARGETOFFSET, 4,		NULL,					"TARGETOFFSET <x> <y> <z>" )


CMDMGR_END_REGISTER_CLASS( KeyFramer, GameBase )


// ----------------------------------------------------------------------- //
//
// External DEdit behavior hooks
//
// ----------------------------------------------------------------------- //

LTRESULT KeyframerPlugin::PreHook_EditStringList( const char *szRezPath, 
												  const char *szPropName,
												  char **aszStrings,
												  uint32 *pcStrings,
												  const uint32 cMaxStrings,
												  const uint32 cMaxStringLength )
{
	_ASSERT( szPropName && aszStrings && pcStrings );

	// See if we can handle the list...

	if( !_stricmp( szPropName, "Wavetype" ) )
	{
		// Fill in the list with our wanve types...

		for( int i = 0; i <= KFWAVE_MAX; i++ )
		{
			strcpy( aszStrings[(*pcStrings)++], c_aWaveTypes[i] );
		}

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}


// Add/subtract the VALUES of the rotation data.  Note that this has no geometric
// meaning - the keyframer just uses it to store offsets from one rotation to
// the next but it never actually goes BETWEEN rotations with this.
inline void AddRotationValues(
	LTRotation *pOut,
	LTRotation *pRot1,
	LTRotation *pRot2)
{
	for (uint32 nLoop = 0; nLoop < 4; ++nLoop)
		pOut->m_Quat[nLoop] = pRot1->m_Quat[nLoop] + pRot2->m_Quat[nLoop];
}

inline void SubtractRotationValues(
	LTRotation *pOut,
	LTRotation *pRot1,
	LTRotation *pRot2)
{
	for (uint32 nLoop = 0; nLoop < 4; ++nLoop)
		pOut->m_Quat[nLoop] = pRot1->m_Quat[nLoop] - pRot2->m_Quat[nLoop];
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::KeyFramer()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

KeyFramer::KeyFramer() : GameBase(OT_NORMAL)
{
	m_hstrObjectName	= LTNULL;
	m_hstrTargetName	= LTNULL;
	m_hstrBaseKeyName	= LTNULL;
	m_hstrActiveSnd		= LTNULL;
	m_pObjectList		= LTNULL;
	m_hTargetObject		= LTNULL;
	m_hActiveSnd		= LTNULL;
	m_hActiveSndObj		= LTNULL;
	m_hstrDestCmd		= LTNULL;

	m_bIgnoreOffsets	= LTFALSE;
	m_bStartActive		= LTFALSE;
	m_bStartPaused		= LTFALSE;
	m_bLooping			= LTFALSE;
	m_bActive			= LTFALSE;
	m_bPaused			= LTFALSE;
	m_bFinished			= LTFALSE;
	m_bUseVelocity		= LTFALSE;
	m_bAlignToPath		= LTFALSE;
	m_bFirstUpdate		= LTTRUE;
	m_bPushObjects		= LTTRUE;

	m_pKeys				= LTNULL;
	m_pCurKey			= LTNULL;
	m_pPosition1		= LTNULL;
	m_pPosition2		= LTNULL;
	m_pLastKey			= LTNULL;
	m_pDestinationKey	= LTNULL;

	m_nKeyDataIndex		= 0xffffffff;
	m_nNumKeys			= 0;
	m_fCurTime			= 0.0f;
	m_fEndTime			= 0.0f;
	m_fTotalPathTime	= 0.0f;
	m_fTotalDistance	= 0.0f;
	m_fVelocity			= 0.0f;
	m_fSoundRadius		= 1000.0f;

	m_fEarliestGoActiveTime		= 0.0f;

	m_eDirection		= KFD_FORWARD;
	m_eWaveform			= KFWAVE_LINEAR;

	m_pOffsets.SetMemCopyable(1);
	m_pOffsets.SetGrowBy(3);
	m_pRotations.SetMemCopyable(1);
	m_pRotations.SetGrowBy(3);

	m_vTargetOffset.Init();
	m_vCurPos.Init();

	m_fKeyPercent = 0.0f;	 // Percent of distance/time between key1 and key2...
	m_pCommands = LTNULL;

	{ // BL 09/29/00 Added to fix falling off keyframed objects after loading game
		m_bPausedOnLoad = LTFALSE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::~KeyFramer()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

KeyFramer::~KeyFramer()
{
	if (m_hstrObjectName)
	{
        g_pLTServer->FreeString(m_hstrObjectName);
	}

	if (m_hstrTargetName)
	{
        g_pLTServer->FreeString(m_hstrTargetName);
	}

	if (m_hstrBaseKeyName)
	{
        g_pLTServer->FreeString(m_hstrBaseKeyName);
	}

	if (m_hstrActiveSnd)
	{
        g_pLTServer->FreeString(m_hstrActiveSnd);
	}

	if (m_hstrDestCmd)
	{
        g_pLTServer->FreeString(m_hstrDestCmd);
	}

	if (m_hActiveSnd)
	{
        // g_pLTServer->KillSoundLoop(m_hActiveSnd);
		g_pLTServer->SoundMgr()->KillSound(m_hActiveSnd);
	}

	if (m_pObjectList)
	{
        g_pLTServer->RelinquishList(m_pObjectList);
	}

	while (m_pKeys)
	{
		KEYNODE* pNext = m_pKeys->pNext;
		debug_delete(m_pKeys);
		m_pKeys = pNext;
	}

	if (m_pCommands)
	{
		debug_deletea(m_pCommands);
		m_pCommands = LTNULL;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::GoActive
//
//	PURPOSE:	Start the KeyFramer going!
//
// ----------------------------------------------------------------------- //

void KeyFramer::GoActive(LTBOOL bReset)
{
	if (m_bActive || !m_pKeys || !m_pLastKey || !m_pObjectList || (m_pObjectList->m_nInList < 1)) return;

	// Go active

	m_bFinished = LTFALSE;
	m_bActive	= LTTRUE;

	if (bReset)
	{
		m_fCurTime	= (m_eDirection == KFD_FORWARD) ? 0.0f : m_fEndTime;
		m_pCurKey	= (m_eDirection == KFD_FORWARD) ? m_pKeys : m_pLastKey;
		m_vCurPos   = (m_eDirection == KFD_FORWARD) ? m_pKeys->keyData.m_vPos : m_pLastKey->keyData.m_vPos;
	}

    SetNextUpdate(UPDATE_NEXT_FRAME);


    m_fEarliestGoActiveTime = g_pLTServer->GetTime() + UPDATE_NEXT_FRAME;


	// Start active sound...

	if (m_hstrActiveSnd && g_pServerSoundMgr)
	{
        const char* pSound = g_pLTServer->GetStringData(m_hstrActiveSnd);
		if (!pSound) return;

		if (m_hActiveSnd)
		{
            // g_pLTServer->KillSoundLoop(m_hActiveSnd);
			g_pLTServer->SoundMgr()->KillSound(m_hActiveSnd);
			m_hActiveSnd = LTNULL;
		}

		ObjectLink* pLink = m_pObjectList->m_pFirstLink;
		if (pLink && pLink->m_hObject)
		{
			m_hActiveSndObj = pLink->m_hObject;
			m_hActiveSnd = g_pServerSoundMgr->PlaySoundFromObject(m_hActiveSndObj, pSound,
				m_fSoundRadius, SOUNDPRIORITY_MISC_MEDIUM, PLAYSOUND_GETHANDLE | PLAYSOUND_LOOP);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::GoInActive
//
//	PURPOSE:	Stop the KeyFramer...
//
// ----------------------------------------------------------------------- //

void KeyFramer::GoInActive()
{
	m_bActive = LTFALSE;
    SetNextUpdate(UPDATE_NEVER);

	if (m_hActiveSnd)
	{
        //g_pLTServer->KillSoundLoop(m_hActiveSnd);
		g_pLTServer->SoundMgr()->KillSound(m_hActiveSnd);
		m_hActiveSnd = LTNULL;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::CreateKeyList
//
//	PURPOSE:	Create our list of keys.
//
// ----------------------------------------------------------------------- //

LTBOOL KeyFramer::CreateKeyList()
{
	ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;
	int numObjects;
	HOBJECT hObject;

	if (!m_hstrObjectName || !m_hstrBaseKeyName || m_pObjectList)
		return LTFALSE;

	// Make sure our object and key index are valid...

    const char* pNames = g_pLTServer->GetStringData(m_hstrObjectName);
	if (!pNames || !pNames[0])
		return LTFALSE;


	// Find all the objects with the given names (m_hstrObjectName may be
	// of the form: name1;name2;name3...

	ConParse parse;
	parse.Init(pNames);

    m_pObjectList = g_pLTServer->CreateObjectList();
	if (!m_pObjectList)
		return LTFALSE;

	while (g_pCommonLT->Parse(&parse) == LT_OK)
	{
		if (parse.m_nArgs > 0 && parse.m_Args[0])
		{
			// Find the objects...

            g_pLTServer->FindNamedObjects(parse.m_Args[0],objArray);

			numObjects = objArray.NumObjects();

			for (int i = 0; i < numObjects; i++)
			{
				hObject = objArray.GetObject(i);
                g_pLTServer->AddObjectToList(m_pObjectList, hObject);
                g_pLTServer->CreateInterObjectLink(m_hObject, hObject);
			}
		}
	}


	// If there aren't any objects, don't continue...

	if (m_pObjectList->m_nInList < 1)
	{
        g_pLTServer->RelinquishList(m_pObjectList);
		m_pObjectList = LTNULL;
		return LTFALSE;
	}

	// Create the list of keys...
	LTFLOAT fTime = 0.0f;
	KEYNODE* pNode = LTNULL;

	// make sure we have valid blind data
	if( m_nKeyDataIndex == 0xffffffff )
		return LTFALSE;

	// get the blind data for this keyframer
	uint8* blindData = NULL;
	uint32 blindDataSize = 0;
	if( g_pLTServer->GetBlindObjectData( m_nKeyDataIndex, KEYFRAMER_BLINDOBJECTID, blindData, blindDataSize ) != LT_OK )
		return LTFALSE;
	uint8* curBlindData = blindData;

	// grab the number of keys in the blind data
	m_nNumKeys = *((uint32*)curBlindData);
	curBlindData += 4;

	for( uint32 i = 0; i < m_nNumKeys; i++ )
	{
		if( !m_pKeys )
		{
			m_pKeys = debug_new( KEYNODE );
			pNode = m_pKeys;
		}
		else
		{
			pNode->pNext = debug_new( KEYNODE );
			pNode->pNext->pPrev = pNode;
			pNode = pNode->pNext;
		}

		// create the key from the blind data
		curBlindData = pNode->keyData.Copy( curBlindData );

		fTime += pNode->keyData.m_fTimeStamp;
		pNode->keyData.m_fRealTime = fTime;

		// store the last key and key time
		m_fEndTime = fTime;
		m_pLastKey = pNode;
	}

	// we're done with the blind data for this keyframer, free it
	g_pLTServer->FreeBlindObjectData( m_nKeyDataIndex, KEYFRAMER_BLINDOBJECTID );


	// If we didn't find any keys, return...

	if (!m_pKeys)
	{
		return LTFALSE;
	}


	// See if we need to calculate the velocity info...

	if (m_bUseVelocity)
	{
		CalculateVelocityInfo();
	}


	// For each object we're controlling, save it's offset from the first key...

	KEYNODE* pFirstPosKey = m_pKeys;

	if (pFirstPosKey)
	{
		// Get the first position key's location

		LTVector vKeyPos;
		LTRotation rKeyRot;

		vKeyPos = pFirstPosKey->keyData.m_vPos;
		rKeyRot = pFirstPosKey->keyData.m_rRot;

		long i = 0;
		ObjectLink* pLink = m_pObjectList->m_pFirstLink;
		while (pLink)
		{
			LTVector vObjPos;
			g_pLTServer->GetObjectPos(pLink->m_hObject, &vObjPos);

			m_pOffsets[i] = vObjPos - vKeyPos;

			LTRotation rObjRot;
			g_pLTServer->GetObjectRotation(pLink->m_hObject, &rObjRot);

			SubtractRotationValues(
				&m_pRotations[i],
				&rObjRot,
				&rKeyRot);

			i++;
			pLink = pLink->m_pNext;
		}
	}

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::CalculateVelocityInfo
//
//	PURPOSE:	Calculate the velocity info for all the keys if using
//				velocity to calculate position information
//
// ----------------------------------------------------------------------- //

void KeyFramer::CalculateVelocityInfo()
{
	if (m_fTotalPathTime <= 0.0 || !m_pKeys) return;

	m_fTotalDistance = 0.0f;

	// Calculate the total distance between all the keys...

	KEYNODE* pCurKey = m_pKeys;
	while (pCurKey && pCurKey->pNext)
	{
		if( (pCurKey->keyData.m_nKeyFlags & BEZNEXT_KEY) && (pCurKey->pNext->keyData.m_nKeyFlags & BEZPREV_KEY) )
		{
			m_fTotalDistance += Bezier_SegmentLength(
				CURVE_PTS(pCurKey->keyData, pCurKey->pNext->keyData));
			pCurKey->pNext->keyData.m_fDistToLastKey = m_fTotalDistance;
		}
		else
		{
			m_fTotalDistance += VEC_DIST( pCurKey->keyData.m_vPos, pCurKey->pNext->keyData.m_vPos );
			pCurKey->pNext->keyData.m_fDistToLastKey = m_fTotalDistance;
		}

		pCurKey = pCurKey->pNext;
	}


	// Calculate linear velocity...

	m_fVelocity = m_fTotalDistance / m_fTotalPathTime;


	// Set each key's distance to last key...

	pCurKey = m_pKeys;
	while (pCurKey)
	{
		pCurKey->keyData.m_fDistToLastKey = m_fTotalDistance - pCurKey->keyData.m_fDistToLastKey;

		pCurKey = pCurKey->pNext;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::ProcessCurrentKey
//
//	PURPOSE:	Processes the current key
//
// ----------------------------------------------------------------------- //

void KeyFramer::ProcessCurrentKey()
{
	if (!m_pCurKey) return;


	// Keep track of the current key...

	KEYNODE* pOldCurKey = m_pCurKey;


	// Set the pos1 and pos2 keys...

	if (pOldCurKey->keyData.m_nKeyFlags & POSITION_KEY)
	{
		if (m_eDirection == KFD_FORWARD)
		{
			m_pPosition1 = pOldCurKey;
			m_pPosition2 = pOldCurKey->pNext;
		}
		else
		{
			m_pPosition2 = pOldCurKey;
			m_pPosition1 = pOldCurKey->pPrev;
		}
	}


	// Adjust m_pCurKey...

	if (m_eDirection == KFD_FORWARD)
	{
		m_pCurKey = m_pCurKey->pNext;
	}
	else
	{
		m_pCurKey = m_pCurKey->pPrev;
	}


	// Process the key we just passed...

	if (pOldCurKey->keyData.m_nKeyFlags & SOUND_KEY)
	{
        const char* pSound = pOldCurKey->keyData.m_hstrSoundName ? g_pLTServer->GetStringData(pOldCurKey->keyData.m_hstrSoundName) : LTNULL;
		if (pSound)
		{
			g_pServerSoundMgr->PlaySoundFromPos(pOldCurKey->keyData.m_vPos, pSound, pOldCurKey->keyData.m_fSoundRadius, SOUNDPRIORITY_MISC_MEDIUM);
		}
	}

	if (pOldCurKey->keyData.m_nKeyFlags & MESSAGE_KEY)
	{
		const char *pCmd = g_pLTServer->GetStringData( pOldCurKey->keyData.m_hstrCommand );

		if( g_pCmdMgr->IsValidCmd( pCmd ) )
		{
			g_pCmdMgr->Process( pCmd, m_hObject, m_hObject );
		}
	}


	// If we're moving to a destination, see if we have reached it yet...

	if (m_pDestinationKey)
	{
		if (pOldCurKey == m_pDestinationKey)
		{
			ReachedDestination();
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::ReachedDestination
//
//	PURPOSE:	Handle reaching the destination key...
//
// ----------------------------------------------------------------------- //

void KeyFramer::ReachedDestination()
{
	// Default behavoir...Pause the keyframer...

	Pause();


	// If we have a destination command, process it...

	if (m_hstrDestCmd)
	{
        const char* pCmd = g_pLTServer->GetStringData(m_hstrDestCmd);

		if (pCmd && g_pCmdMgr->IsValidCmd(pCmd))
		{
			g_pCmdMgr->Process(pCmd, m_hObject, m_hObject);
		}

        g_pLTServer->FreeString(m_hstrDestCmd);
		m_hstrDestCmd = LTNULL;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 KeyFramer::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			Update();
		}
		break;

		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProps();
			}
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

		case MID_LINKBROKEN :
		{
			HandleLinkBroken((HOBJECT)pData);
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

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::HandleLinkBroken()
//
//	PURPOSE:	Handle MID_LINKBROKEN engine message
//
// --------------------------------------------------------------------------- //

void KeyFramer::HandleLinkBroken(HOBJECT hLink)
{
	if (!m_pObjectList || !hLink) return;

	// Kill active sound if object associated with it goes away...

	if (m_hActiveSndObj == hLink)
	{
		if (m_hActiveSnd)
		{
            //g_pLTServer->KillSoundLoop(m_hActiveSnd);
			g_pLTServer->SoundMgr()->KillSound(m_hActiveSnd);
			m_hActiveSnd = LTNULL;
			m_hActiveSndObj = LTNULL;
		}
	}

	ObjectLink* pLink = m_pObjectList->m_pFirstLink;
	ObjectLink* pPrevious = LTNULL;

	while (pLink)
	{
		if (pLink->m_hObject == hLink)
		{
			m_pObjectList->m_nInList--;

			if (pLink == m_pObjectList->m_pFirstLink)
			{
				//m_pObjectList->m_pFirstLink = LTNULL;
				m_pObjectList->m_pFirstLink = pLink->m_pNext;
			}
			else if (pPrevious)
			{
				pPrevious->m_pNext = pLink->m_pNext;
			}

			// If the list is now empty, remove it...

			if (!m_pObjectList->m_nInList)
			{
                g_pLTServer->RelinquishList(m_pObjectList);
				m_pObjectList = LTNULL;
			}

			return;
		}

		pPrevious = pLink;
		pLink = pLink->m_pNext;
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::OnTrigger()
//
//	PURPOSE:	Process keyframer trigger messages
//
// --------------------------------------------------------------------------- //

bool KeyFramer::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_On("ON");
	static CParsedMsg::CToken s_cTok_Off("OFF");
	static CParsedMsg::CToken s_cTok_Pause("PAUSE");
	static CParsedMsg::CToken s_cTok_Resume("RESUME");
	static CParsedMsg::CToken s_cTok_Forward("FORWARD");
	static CParsedMsg::CToken s_cTok_Backward("BACKWARD");
	static CParsedMsg::CToken s_cTok_Reverse("REVERSE");
	static CParsedMsg::CToken s_cTok_ToggleDir("TOGGLEDIR");
	static CParsedMsg::CToken s_cTok_Goto("GOTO");
	static CParsedMsg::CToken s_cTok_MoveTo("MOVETO");
	static CParsedMsg::CToken s_cTok_Target("TARGET");
	static CParsedMsg::CToken s_cTok_ClearTarget("CLEARTARGET");
	static CParsedMsg::CToken s_cTok_TargetOffset("TARGETOFFSET");

	if (cMsg.GetArg(0) == s_cTok_On)
	{
		On();
	}
	else if (cMsg.GetArg(0) == s_cTok_Off)
	{
		Off();
	}
	else if (cMsg.GetArg(0) == s_cTok_Pause)
	{
		if (m_bFirstUpdate)
		{
			DeferCommand(cMsg);
		}
		else
		{
			Pause();
		}
	}
	else if (cMsg.GetArg(0) == s_cTok_Resume)
	{
		Resume();
	}
	else if (cMsg.GetArg(0) == s_cTok_Forward)
	{
		Forward();
	}
	else if ((cMsg.GetArg(0) == s_cTok_Backward) ||
			 (cMsg.GetArg(0) == s_cTok_Reverse))
	{
		Backward();
	}
	else if (cMsg.GetArg(0) == s_cTok_ToggleDir)
	{
		ToggleDir();
	}
	else if (cMsg.GetArg(0) == s_cTok_Goto)
	{
		if (m_bFirstUpdate)
		{
			DeferCommand(cMsg);
		}
		else
		{
			if (cMsg.GetArgCount() > 1)
			{
				GoToKey(cMsg.GetArg(1));
			}
		}
	}
	else if (cMsg.GetArg(0) == s_cTok_MoveTo)
	{
		if (m_bFirstUpdate)
		{
			DeferCommand(cMsg);
		}
		else
		{
			MoveToKey(cMsg);
		}
	}
	else if (cMsg.GetArg(0) == s_cTok_Target)
	{
		if (m_bFirstUpdate)
		{
			DeferCommand(cMsg);
		}
		else
		{
			if (cMsg.GetArgCount() > 1)
			{
				SetTarget(cMsg.GetArg(1));
			}
		}
	}
	else if (cMsg.GetArg(0) == s_cTok_ClearTarget)
	{
		if (m_bFirstUpdate)
		{
			DeferCommand(cMsg);
		}
		else
		{
			SetTarget(LTNULL);
		}
	}
	else if (cMsg.GetArg(0) == s_cTok_TargetOffset)
	{
		if (m_bFirstUpdate)
		{
			DeferCommand(cMsg);
		}
		else
		{
			if (cMsg.GetArgCount() > 3)
			{
				m_vTargetOffset.x = (LTFLOAT) atof(cMsg.GetArg(1));
				m_vTargetOffset.y = (LTFLOAT) atof(cMsg.GetArg(2));
				m_vTargetOffset.z = (LTFLOAT) atof(cMsg.GetArg(3));
			}
		}
	}
	else
		return GameBase::OnTrigger(hSender, cMsg);

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::DeferCommand
//
//	PURPOSE:	Defer a command to be processed later...
//
// ----------------------------------------------------------------------- //

void KeyFramer::DeferCommand(const CParsedMsg &cMsg)
{
	const uint32 k_nCommandBufferLen = 1024;

	// Alloc if we haven't already
	// m_pCommands will get deleted at the end of the first update.
	if(!m_pCommands)
	{
		m_pCommands = debug_newa(char, k_nCommandBufferLen);
		memset(m_pCommands,0,k_nCommandBufferLen);
	}
	else
	{
		// We already have some commands so throw on a separator
		strcat(m_pCommands,";");
	}

	uint32 nCurLen = strlen(m_pCommands);
	cMsg.ReCreateMsg(&m_pCommands[nCurLen], k_nCommandBufferLen - nCurLen, 0);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::ReadProps
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL KeyFramer::ReadProps()
{
	GenericProp genProp;
    if (g_pLTServer->GetPropGeneric("ObjectName", &genProp) == LT_OK)
	{
		SetObjectName(genProp.m_String);
	}

    if (g_pLTServer->GetPropGeneric("TargetName", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
             m_hstrTargetName = g_pLTServer->CreateString(genProp.m_String);
	}

    if (g_pLTServer->GetPropGeneric("TargetOffset", &genProp) == LT_OK)
	{
		m_vTargetOffset = genProp.m_Vec;
	}

    if (g_pLTServer->GetPropGeneric("BaseKeyName", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
             m_hstrBaseKeyName = g_pLTServer->CreateString(genProp.m_String);
	}

	if (g_pLTServer->GetPropGeneric("KeyDataIndex", &genProp) == LT_OK)
	{
		if( genProp.m_Float >= 0.0f )
			m_nKeyDataIndex = (uint32)genProp.m_Float;
		else
			m_nKeyDataIndex = 0xffffffff;
	}

    if (g_pLTServer->GetPropGeneric("ActiveSound", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
             m_hstrActiveSnd = g_pLTServer->CreateString(genProp.m_String);
	}

    if (g_pLTServer->GetPropGeneric("SoundRadius", &genProp) == LT_OK)
	{
		m_fSoundRadius = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("StartActive", &genProp) == LT_OK)
	{
		m_bStartActive = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("StartPaused", &genProp) == LT_OK)
	{
		m_bStartPaused = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("Looping", &genProp) == LT_OK)
	{
		m_bLooping = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("IgnoreOffsets", &genProp) == LT_OK)
	{
		m_bIgnoreOffsets = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("PushObjects", &genProp) == LT_OK)
	{
		m_bPushObjects = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("AlignToPath", &genProp) == LT_OK)
	{
		m_bAlignToPath = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("TotalPathTime", &genProp) == LT_OK)
	{
		m_fTotalPathTime = genProp.m_Float;
		m_bUseVelocity = (m_fTotalPathTime > 0.0 ? LTTRUE : LTFALSE);
	}

	// Get the waveform...

	if( g_pLTServer->GetPropGeneric( "Wavetype", &genProp ) == LT_OK )
	{
		if( genProp.m_String[0] )
		{
			for( int i = 0; i <= KFWAVE_MAX; i++ )
			{
				if( !_stricmp( genProp.m_String, c_aWaveTypes[i] ))
				{
					m_eWaveform = static_cast<KFWaveType>(i);
					break;
				}
			}
		}
	}

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::SetObjectName
//
//	PURPOSE:	Set the object name...
//
// ----------------------------------------------------------------------- //

void KeyFramer::SetObjectName(char* pName)
{
	if (!pName || !pName[0]) return;

	if (m_hstrObjectName)
	{
        g_pLTServer->FreeString(m_hstrObjectName);
	}

     m_hstrObjectName = g_pLTServer->CreateString(pName);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::SetTarget
//
//	PURPOSE:	Set the target object...
//
// ----------------------------------------------------------------------- //

void KeyFramer::SetTarget(const char* pName)
{
	m_hTargetObject = LTNULL;

	if (!pName) return;

	ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;
    g_pLTServer->FindNamedObjects(const_cast<char*>(pName), objArray);

	if (objArray.NumObjects())
	{
		m_hTargetObject = objArray.GetObject(0);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

void KeyFramer::InitialUpdate()
{
	if (!g_vtDisableKeyframers.IsInitted())
	{
        g_vtDisableKeyframers.Init(g_pLTServer, "DisableKeyframers", LTNULL, 0.0f);
	}

    SetNextUpdate(UPDATE_NEXT_FRAME);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::Update()
//
//	PURPOSE:	Update
//
// ----------------------------------------------------------------------- //

// BL 09/29/00 Added to fix falling off keyframed objects after loading game
extern int32 g_bPlayerUpdated;

void KeyFramer::Update()
{
	{ // BL 09/29/00 Added to fix falling off keyframed objects after loading game
		if ( m_bPausedOnLoad )
		{
			if ( g_bPlayerUpdated < 0 )
			{
				m_bPausedOnLoad = LTFALSE;
			}
			else
			{
				SetNextUpdate(UPDATE_NEXT_FRAME);
				return;
			}
		}
	}

	// Need to check this here (instead of InitialUpdate)...This insures
	// that all the Keys have been added to the world...

	if (m_bFirstUpdate)
	{
		m_bFirstUpdate = LTFALSE;

		CreateKeyList();

		if (m_hstrTargetName)
		{
            const char* pName = g_pLTServer->GetStringData(m_hstrTargetName);
			SetTarget(pName);
		}

		// Must be active to pause...

		if (m_bStartActive || m_bStartPaused)
		{
			GoActive();
		}

		if (m_bStartPaused)
		{
			Pause();
		}

		// Check for deferred commands
		if (m_pCommands)
		{
			CAutoMessage cTempMsg;
			cTempMsg.Writeuint32(MID_TRIGGER);
			cTempMsg.Writeuint32((uint32)m_pCommands);
			ObjectMessageFn(m_hObject, cTempMsg.Read());
			debug_deletea(m_pCommands);
			m_pCommands = LTNULL;
		}

		// Make sure we're not eating ticks if we're not active...
		if (!m_bActive)
		{
			SetNextUpdate(UPDATE_NEVER);
		}

		return; // Don't process objects this frame...
	}


	// If we should be disabled stop updating us...

	if (g_vtDisableKeyframers.GetFloat() != 0.0f)
	{
		// For now this is a one-way trip for testing...go to sleep and
		// never wake up...
        Pause();
		return;
	}


	// See if we are even supposed to be here (today)...

	if (!m_bActive)
	{
		GoInActive();
		return;
	}
	else
	{
        SetNextUpdate(UPDATE_NEXT_FRAME);
	}


	// Process all keys that we might have passed by (NOTE: we check for
	// m_bActive again because keys we process may pause the keyframer or
	// turn it off).

	while (m_pCurKey && m_bActive)
	{
		if (m_bUseVelocity)
		{
            if(!m_vCurPos.NearlyEquals(m_pCurKey->keyData.m_vPos, 0.0f))
            {
				break;
			}
		}
		else
		{
			if (m_eDirection == KFD_FORWARD)
			{
				if (m_pCurKey->keyData.m_fRealTime > m_fCurTime) break;
			}
			else
			{
				if (m_pCurKey->keyData.m_fRealTime < m_fCurTime) break;
			}
		}

		ProcessCurrentKey();
	}

	// Are we at the end of the key list?

	if (!m_pCurKey)
	{
		if (m_eDirection == KFD_FORWARD)
		{
			m_pCurKey  = m_pKeys;
			m_fCurTime = 0.0f;
			m_vCurPos  = m_pKeys->keyData.m_vPos;
		}
		else
		{
			m_pCurKey  = m_pLastKey;
			m_fCurTime = m_fEndTime;
			m_vCurPos  = m_pLastKey->keyData.m_vPos;
		}

		if (!m_bLooping)
		{
			m_bFinished = LTTRUE;
		}
	}

	// Update (move/rotate) all the object(s)...(NOTE: we check for
	// m_bActive again because keys we process may pause the keyframer or
	// turn it off).

	if (m_bActive)
	{
		UpdateObjects();
	}

	if (m_bFinished)
	{
		GoInActive();
	}

	// Increment timer

    float fTime = g_pLTServer->GetFrameTime();
	m_fCurTime += (m_eDirection == KFD_FORWARD) ? fTime : -fTime;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::UpdateObjects()
//
//	PURPOSE:	Update the object(s) position(s) / rotation(s)
//
// ----------------------------------------------------------------------- //

void KeyFramer::UpdateObjects(LTBOOL bInterpolate, LTBOOL bTeleport)
{
	if (!m_pPosition1 || !m_pPosition2) return;

	// bAtKey is set to true if we're at a key...

	LTBOOL bAtKey = LTFALSE;


	// Calculate m_vCurPos...

	if (!CalcCurPos(bAtKey)) return;


	// Get the new angle from position 1's angle

	LTRotation rRotNew, rRot1, rRot2;
	rRot1 = m_pPosition1->keyData.m_rRot;
	rRot2 = m_pPosition2->keyData.m_rRot;

	if (bAtKey)
	{
		rRotNew = (m_eDirection == KFD_FORWARD) ? rRot2 : rRot1;
	}
	else
	{
        rRotNew.Slerp(rRot1, rRot2, m_fKeyPercent);
	}

	// Now add the relative position and rotation to every object in
	// the list...

	long i = 0;
	ObjectLink* pLink = m_pObjectList ? m_pObjectList->m_pFirstLink : LTNULL;
	while (pLink)
	{
		LTVector vOldPos;
		g_pLTServer->GetObjectPos(pLink->m_hObject, &vOldPos);

		// Set object's new position...

		LTVector vPos = m_vCurPos;

		if (!m_bIgnoreOffsets)
		{
			vPos += m_pOffsets[i];
		}

		if (m_bPushObjects && !bTeleport)
		{
            g_pLTServer->MoveObject(pLink->m_hObject, &vPos);
		}
		else
		{
			g_pLTServer->SetObjectPos(pLink->m_hObject, &vPos);
		}


		// See if we can rotate this object...

		//if (CanRotateObject(pLink->m_hObject))
		//{
			// Initialize to normal rotation calculation...

			rRot1 = rRotNew;


			// If we have a target object, align the object to face it...

			if (m_hTargetObject)
			{
				LTVector vTargetPos;
				g_pLTServer->GetObjectPos(m_hTargetObject, &vTargetPos);

				vTargetPos += m_vTargetOffset;

				LTVector vDir = vTargetPos - vPos;
				vDir.Normalize();

				rRot1 = LTRotation(vDir, LTVector(0.0f, 1.0f, 0.0f));
			}
			else if (m_bAlignToPath)
			{
				// Align the object to the path, if necessary...

				LTVector vDir = vPos - vOldPos;
				vDir.Normalize();

				if (m_eDirection == KFD_BACKWARD)
				{
					vDir = -vDir;
				}

				rRot1 = LTRotation(vDir, LTVector(0.0f, 1.0f, 0.0f));

				// Now adjust the rotation to account for the pitch and roll of
				// the key...

				LTVector vNewPYR;
				LTVector vPYR1 = m_pPosition1->keyData.m_vPitchYawRoll;
				LTVector vPYR2 = m_pPosition2->keyData.m_vPitchYawRoll;
				VEC_LERP(vNewPYR, vPYR1, vPYR2, m_fKeyPercent);

				LTVector vU, vR, vF;
				vU = rRot1.Up();
				vR = rRot1.Right();
				vF = rRot1.Forward();

				// Adjust pitch...

				rRot1.Rotate(vR, vNewPYR.x);

				// Adjust roll...

				rRot1.Rotate(vF, vNewPYR.z);
			}


			if (!m_bIgnoreOffsets && !m_hTargetObject)
			{
				AddRotationValues(
					&rRot1,
					&rRot1,
					&m_pRotations[i]);
			}


			// Set object's new rotation...

			/*
            g_pLTServer->GetObjectRotation( pLink->m_hObject, &rRot2 );

			// See if the object rotated significantly.
			if ( fabs(rRot1.m_Spin - rRot2.m_Spin) > 0.0f || VEC_DISTSQR(rRot1.m_Vec, rRot2.m_Vec) > 0.0f)
			{*/

				if (bInterpolate)
				{
                    g_pLTServer->RotateObject(pLink->m_hObject, &rRot1);
				}
				else
				{
					g_pLTServer->SetObjectRotation(pLink->m_hObject, &rRot1);
				}

				/* // See if we should rotate the dims of the object.
                if (GetObjectType(pLink->m_hObject) == OT_MODEL)
				{
                    HMODELANIM hAni = g_pLTServer->GetModelAnimation(pLink->m_hObject);
					if (hAni != INVALID_ANI)
					{
						LTVector vDims;
                        g_pLTServer->GetModelAnimUserDims( pLink->m_hObject, &vDims, hAni );
						Utils::RotateDims(rRot1, vDims);
                        g_pLTServer->SetObjectDims2(pLink->m_hObject, &vDims);
					}
				}
			}*/

		//}

		i++;
		pLink = pLink->m_pNext;
	}
}

uint32 KeyFramer::GetKeyIndex(KEYNODE *pNode)
{
	KEYNODE *pCur;
	uint32 index;

	index = 0;
	for(pCur=m_pKeys; pCur; pCur=pCur->pNext)
	{
		if(pCur == pNode)
			return index;

		++index;
	}

	return 0;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::CalcCurPos()
//
//	PURPOSE:	Calculate m_fCurPos and fKeyPercent
//
// ----------------------------------------------------------------------- //

LTBOOL KeyFramer::CalcCurPos(LTBOOL & bAtKey)
{
	if (!m_pPosition1 || !m_pPosition2) return LTFALSE;

	bAtKey = LTFALSE;

	LTVector vPos1 = m_pPosition1->keyData.m_vPos;
	LTVector vPos2 = m_pPosition2->keyData.m_vPos;

	const float c_fEpsilon = 0.001f;
	LTBOOL bUpdateObjects = LTFALSE;
	float fPos1Time, fPos2Time;

	// See if we should update the objects...

	if (m_bUseVelocity)
	{
		if (m_fVelocity > 0.0 && m_fTotalDistance > 0.0f)
		{
			bUpdateObjects = LTTRUE;
		}
	}
	else // Using time...
	{
		fPos1Time = m_pPosition1->keyData.m_fRealTime;
		fPos2Time = m_pPosition2->keyData.m_fRealTime;

		if (m_fCurTime >= fPos1Time && m_fCurTime <= fPos2Time)
		{
			bUpdateObjects = LTTRUE;
		}
	}

	if (!bUpdateObjects) return LTFALSE;


	// See if we should velocity to calculate m_vCurPos...

	if (m_bUseVelocity)
	{
		// Find how far along the total path we are and calculate the distance we should move based on waveform...

		float fPathPercent	= LTCLAMP( m_fCurTime / m_fTotalPathTime, 0.0f, 1.0f );
		float fMoveDist		= GetKFWaveValue( m_fVelocity, fPathPercent, m_eWaveform ) * g_pLTServer->GetFrameTime();
		float fMovePercent;

		// Calculate distance between pos1 and pos2

		float fDistBetweenKeys;
		if( (m_pPosition1->keyData.m_nKeyFlags & BEZNEXT_KEY) && (m_pPosition2->keyData.m_nKeyFlags & BEZPREV_KEY) )
		{
			fDistBetweenKeys	= ( float )fabs( m_pPosition1->keyData.m_fDistToLastKey - m_pPosition2->keyData.m_fDistToLastKey );
			
			// If the keys are in the same place move to pos2
			fMovePercent		= (fDistBetweenKeys <= MATH_EPSILON) ? 1.0f : (fMoveDist / fDistBetweenKeys);
			
			{
				// TODO: Find a better, more correct, algorithm.  This looks infinitely better than doing
				//		 nothing, but still has it's problems.

				// Calculate how far we would move based on the linear percentage
		
				float fSegLength = Bezier_SubSegmentLength( CURVE_PTS( m_pPosition1->keyData, m_pPosition2->keyData), m_fKeyPercent, m_fKeyPercent + fMovePercent );
				float fActualPercent = (fDistBetweenKeys <= MATH_EPSILON) ? 1.0f : (fSegLength / fDistBetweenKeys);
			
				// Adjust the percent for how far we need to move to keep consistent motion
							
				if( (fSegLength > 0.0f) && (fMoveDist > 0.0f) )
					fMovePercent /= ((fSegLength / fMoveDist));
			}
		}
		else
		{
			fDistBetweenKeys	= vPos2.Dist( vPos1 );
			
			// If the keys are in the same place move to pos2
			fMovePercent		= (fDistBetweenKeys <= MATH_EPSILON) ? 1.0f : (fMoveDist / fDistBetweenKeys);
		}

		// See if we'll move past (or to) the destination pos...

		if (m_eDirection == KFD_FORWARD)
		{
			m_fKeyPercent += fMovePercent;
			if (m_fKeyPercent >= 1.0f || fPathPercent >= 1.0f )
				
			{
				bAtKey = LTTRUE;
			}
		}
		else  // KFD_BACKWARD
		{
			m_fKeyPercent -= fMovePercent;
			if (m_fKeyPercent <= MATH_EPSILON || fPathPercent <= MATH_EPSILON )
			{
				bAtKey = LTTRUE;
			}
		}

		// Calculate the position to move to...

		if (bAtKey)
		{
			m_vCurPos = (m_eDirection == KFD_FORWARD) ? vPos2 : vPos1;

			// Reset key percent based on the current direction...

			m_fKeyPercent = (m_eDirection == KFD_FORWARD) ? LTCLAMP( -(1.0f - m_fKeyPercent), 0.0f, 1.0f ) : LTCLAMP( m_fKeyPercent - 1.0f, 0.0f, 1.0f );
		}
		else
		{
			if ( (m_pPosition1->keyData.m_nKeyFlags & BEZNEXT_KEY) && (m_pPosition2->keyData.m_nKeyFlags & BEZPREV_KEY) )
			{
				Bezier_Evaluate(m_vCurPos, CURVE_PTS(m_pPosition1->keyData, m_pPosition2->keyData), m_fKeyPercent);
			}
			else
			{
				VEC_LERP(m_vCurPos, vPos1, vPos2, m_fKeyPercent);
			}
		}

	}
	else  // Using time to calculate m_vCurPos...
	{
		float fTimeRange = (fPos2Time - fPos1Time);
		if (-c_fEpsilon <= fTimeRange && fTimeRange < c_fEpsilon)
		{
			fTimeRange = 0.0f;
		}

		m_fKeyPercent = fTimeRange != 0.0f ? (m_fCurTime - fPos1Time) / fTimeRange : 0.0f;

		// Get the new position from position 1's position

		if ( (m_pPosition1->keyData.m_nKeyFlags & BEZNEXT_KEY) && (m_pPosition2->keyData.m_nKeyFlags & BEZPREV_KEY) )
		{
			Bezier_Evaluate(m_vCurPos, CURVE_PTS(m_pPosition1->keyData, m_pPosition2->keyData), m_fKeyPercent);
		}
		else
		{
			VEC_LERP(m_vCurPos, vPos1, vPos2, m_fKeyPercent);
		}
	}

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::GoToKey
//
//	PURPOSE:	Set the current key to the specified key
//
// ----------------------------------------------------------------------- //

void KeyFramer::GoToKey(const char* pKeyName)
{
	if (!pKeyName) return;

	// Find the specified key...

	KEYNODE* pCurKey = FindKey(pKeyName);

	// Couldn't find the key...

	if (!pCurKey)
	{
        g_pLTServer->CPrint("ERROR in KeyFramer::GoToKey() - Couldn't find key '%s'", pKeyName);
		return;
	}


	// Make sure the keyframer isn't paused...

	Resume();


	// Set the current key and current time...

	m_pCurKey  = pCurKey;
	m_fCurTime = pCurKey->keyData.m_fRealTime;
	m_vCurPos  = pCurKey->keyData.m_vPos;


	// Set up m_pPosition1 and m_pPosition2...

	ProcessCurrentKey();


	// Make sure that m_pPosition1 and m_pPosition2 are valid...

	if (!m_pPosition1)
	{
		m_pPosition1 = m_pPosition2;
	}
	else if (!m_pPosition2)
	{
		m_pPosition2 = m_pPosition1;
	}

	if (!m_pPosition1)
	{
        g_pLTServer->CPrint("ERROR in KeyFramer::GoToKey() - m_pPosition1 and m_pPosition2 are both LTNULL!!!!");
	}


	// Move/Rotate the objects as necessary...

	UpdateObjects(LTFALSE, LTTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::MoveToKey
//
//	PURPOSE:	Set the destination key to the specified key
//
// ----------------------------------------------------------------------- //

void KeyFramer::MoveToKey(const CParsedMsg &cMsg)
{
	if (cMsg.GetArgCount() < 2) return;

	const char* pKeyName = cMsg.GetArg(1);

	// See if we have a destination command...

	if (cMsg.GetArgCount() > 2)
	{
		if (m_hstrDestCmd)
		{
            g_pLTServer->FreeString(m_hstrDestCmd);
		}

        m_hstrDestCmd = g_pLTServer->CreateString(const_cast<char*>(cMsg.GetArg(2).c_str()));
	}


	// This isn't quite right, doesn't work with 4 floor elevator with
	// the last key being the bottom floor...



	// Find the destination key...

	KEYNODE* pCur = (m_eDirection == KFD_FORWARD) ? m_pPosition1 : m_pPosition2;

	LTBOOL bIsAtOrBefore = LTFALSE;
	m_pDestinationKey = FindKey(pKeyName, pCur, &bIsAtOrBefore);


	// Couldn't find the key...

	if (!m_pDestinationKey)
	{
        g_pLTServer->CPrint("ERROR in KeyFramer::MoveToKey() - Couldn't find key '%s'", pKeyName);
		return;
	}


	// Make sure the keyframer isn't paused...

	Resume();


	// Check to see if the destination key is the current key...

	if (m_pDestinationKey == pCur)
	{
		// See if we are actually at the key position...

        if(m_vCurPos.NearlyEquals(pCur->keyData.m_vPos, 0.0f))
		{
			// Okay, we just need to process the key...

			ProcessCurrentKey();
			return;
		}
		else
		{
			// If we aren't looping, we are someplace between m_pPosition1
			// and m_pPosition2 moving away from m_pCurKey.  So, we just
			// need to switch directions....

			//if (!m_bLooping)
			{
				ToggleDir();
				return;
			}
		}
	}
	else //if (!m_bLooping)
	{
		// Check to see if we need to change directions to get to the
		// destination key...

		if (bIsAtOrBefore)
		{
			if (m_eDirection == KFD_BACKWARD)
			{
				ToggleDir();
			}
		}
		else
		{
			if (m_eDirection == KFD_FORWARD)
			{
				ToggleDir();
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::FindKey
//
//	PURPOSE:	Find the specified key
//
// ----------------------------------------------------------------------- //

KEYNODE* KeyFramer::FindKey(const char* pKeyName, KEYNODE* pTest, LTBOOL* pbAtOrBefore)
{
	if (!pKeyName)
		return LTNULL;

	const char* baseKeyName = g_pLTServer->GetStringData( m_hstrBaseKeyName );
	if( !baseKeyName )
		return LTNULL;

	uint32 baseKeyNameLen = strlen( baseKeyName );

	// don't bother comparing if the bases don't even match
	if( strnicmp( baseKeyName, pKeyName, baseKeyNameLen ) != 0 )
		return LTNULL;

	// make sure there's more to the test name
	uint32 keyNameLen = strlen( pKeyName );
	if( keyNameLen <= baseKeyNameLen )
		return LTNULL;

	// move the pointer to the key number portion
	const char* extension = pKeyName + baseKeyNameLen;

	char genExt[16];
	if( (keyNameLen - baseKeyNameLen) > 10 )
		return LTNULL;

	// loop through keys, generating key names and comparing on the fly
	KEYNODE* curKey = m_pKeys;
	for( uint32 i = 0; (i < m_nNumKeys) && curKey; i++ )
	{
		if( pTest && pbAtOrBefore )
		{
			if( curKey == pTest )
			{
				*pbAtOrBefore = LTTRUE;
			}
		}

		if( i > 0 && i < 10 )
			sprintf( genExt, "0%d", i );
		else
			sprintf( genExt, "%d", i );

		if( strcmp( extension, genExt ) == 0 )
			break;

		curKey = curKey->pNext;
	}

	return curKey;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::On()
//
//	PURPOSE:	Turn keyframer on
//
// --------------------------------------------------------------------------- //

void KeyFramer::On()
{
	m_bPaused = LTFALSE;

	if (m_bFirstUpdate)
	{
		// Can't activate before Update is called...so just start active...

		m_bStartActive = LTTRUE;
	}
	else
	{
		GoActive();
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::Off()
//
//	PURPOSE:	Turn keyframer off
//
// --------------------------------------------------------------------------- //

void KeyFramer::Off()
{
	GoInActive();
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::Pause()
//
//	PURPOSE:	Pause keyframer
//
// --------------------------------------------------------------------------- //

void KeyFramer::Pause()
{
	if (m_bActive && !m_bPaused)
	{
		m_bPaused = LTTRUE;
		GoInActive();
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::Resume()
//
//	PURPOSE:	Resume keyframer
//
// --------------------------------------------------------------------------- //

void KeyFramer::Resume()
{
	if (m_bPaused)
	{
		m_bPaused = LTFALSE;
		GoActive(LTFALSE);
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::Forward()
//
//	PURPOSE:	Change keyframer direction to forward
//
// --------------------------------------------------------------------------- //

void KeyFramer::Forward()
{
	// Make sure the keypercent starts with the correct value...

	if (m_eDirection == KFD_BACKWARD && m_fKeyPercent > 0.99f)
	{
		m_fKeyPercent = 0.0f;
	}

	m_eDirection = KFD_FORWARD;
	m_pCurKey = m_pPosition2;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::Backward()
//
//	PURPOSE:	Change keyframer direction to backward
//
// --------------------------------------------------------------------------- //

void KeyFramer::Backward()
{
	// Make sure the keypercent starts with the correct value...

	if (m_eDirection == KFD_FORWARD && m_fKeyPercent < 0.01f)
	{
		m_fKeyPercent = 1.0f;
	}

	m_eDirection = KFD_BACKWARD;
	m_pCurKey = m_pPosition1;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::ToggleDir()
//
//	PURPOSE:	Toggle keyframer direction
//
// --------------------------------------------------------------------------- //

void KeyFramer::ToggleDir()
{
	if (m_eDirection == KFD_FORWARD)
	{
		Backward();
	}
	else
	{
		Forward();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void KeyFramer::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	// Save m_pCommands
    HSTRING hStr = g_pLTServer->CreateString(m_pCommands);
	SAVE_HSTRING(hStr);

	// Make sure we created the keylist (if Save is called before the first
	// update, this may not have been created)...

	CreateKeyList();

    SAVE_HOBJECT(m_hActiveSndObj);
    SAVE_HOBJECT(m_hTargetObject);
	SAVE_VECTOR(m_vTargetOffset);
	SAVE_VECTOR(m_vCurPos);

    SAVE_TIME(m_fEarliestGoActiveTime);
    SAVE_FLOAT(m_fTotalPathTime);
    SAVE_FLOAT(m_fTotalDistance);
    SAVE_FLOAT(m_fVelocity);
    SAVE_FLOAT(m_fCurTime);
    SAVE_FLOAT(m_fEndTime);
    SAVE_FLOAT(m_fSoundRadius);
    SAVE_FLOAT(m_fKeyPercent);
    SAVE_HSTRING(m_hstrObjectName);
    SAVE_HSTRING(m_hstrTargetName);
    SAVE_HSTRING(m_hstrBaseKeyName);
    SAVE_HSTRING(m_hstrActiveSnd);
    SAVE_HSTRING(m_hstrDestCmd);
	SAVE_DWORD(m_nKeyDataIndex);
    SAVE_BOOL(m_bUseVelocity);
    SAVE_BOOL(m_bStartActive);
    SAVE_BOOL(m_bStartPaused);
    SAVE_BOOL(m_bLooping);
    SAVE_BOOL(m_bIgnoreOffsets);
    SAVE_BOOL(m_bActive);
    SAVE_BOOL(m_bPaused);
    SAVE_BOOL(m_bFinished);
    SAVE_BOOL(m_bFirstUpdate);
    SAVE_BOOL(m_bAlignToPath);
    SAVE_BOOL(m_bPushObjects);
    SAVE_WORD(m_nNumKeys);
    SAVE_BYTE(m_eDirection);
    SAVE_BYTE(m_eWaveform);

	// Determine the position in the list of m_pCurKey, m_pPosition1, and
	// m_pPosition2.  Also, save out m_pKeys...

	int nCurKeyIndex    = -1;
	int nPosition1Index = -1;
	int nPosition2Index = -1;
	int nDestKeyIndex   = -1;

    int i;
	KEYNODE* pCurKey = m_pKeys;
    for (i=0; i < m_nNumKeys && pCurKey; i++)
	{
		if (m_pCurKey == pCurKey)
		{
			nCurKeyIndex = i;
		}
		if (m_pPosition1 == pCurKey)
		{
			nPosition1Index = i;
		}
		if (m_pPosition2 == pCurKey)
		{
			nPosition2Index = i;
		}
		if (m_pDestinationKey == pCurKey)
		{
			nDestKeyIndex = i;
		}

		pCurKey = pCurKey->pNext;
	}

	// Save out the positions of our pointer data members...

	SAVE_INT(nCurKeyIndex);
    SAVE_INT(nPosition1Index);
    SAVE_INT(nPosition2Index);
    SAVE_INT(nDestKeyIndex);


	// Save the number of objects to be key-framed...

    uint8 nNumInList = m_pObjectList ? m_pObjectList->m_nInList : 0;
    SAVE_BYTE(nNumInList);

	// Save the offsets and rotations for each object...

	for (i=0; i < nNumInList; i++)
	{
		SAVE_VECTOR(m_pOffsets[i]);
		SAVE_ROTATION(m_pRotations[i]);
	}

	// Save the objects we're supposed to key-frame...

	if (m_pObjectList && nNumInList)
	{
		ObjectLink* pLink = m_pObjectList->m_pFirstLink;
		while (pLink)
		{
            SAVE_HOBJECT(pLink->m_hObject);
			pLink = pLink->m_pNext;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void KeyFramer::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	// Load m_pCommands
    HSTRING hStr;
	LOAD_HSTRING(hStr);
	if(hStr)
	{
        const char *pstr = g_pLTServer->GetStringData(hStr);
		if(pstr && pstr[0])
		{
			if(m_pCommands)
			{
				debug_deletea(m_pCommands);
			}

			m_pCommands = debug_newa(char, 1024);
			strcpy(m_pCommands,pstr);
		}

        g_pLTServer->FreeString(hStr);
	}

	LOAD_HOBJECT(m_hActiveSndObj);
	LOAD_HOBJECT(m_hTargetObject);
    LOAD_VECTOR(m_vTargetOffset);
    LOAD_VECTOR(m_vCurPos);

    LOAD_TIME(m_fEarliestGoActiveTime);
    LOAD_FLOAT(m_fTotalPathTime);
    LOAD_FLOAT(m_fTotalDistance);
    LOAD_FLOAT(m_fVelocity);
    LOAD_FLOAT(m_fCurTime);
    LOAD_FLOAT(m_fEndTime);
    LOAD_FLOAT(m_fSoundRadius);
    LOAD_FLOAT(m_fKeyPercent);
    LOAD_HSTRING(m_hstrObjectName);
    LOAD_HSTRING(m_hstrTargetName);
    LOAD_HSTRING(m_hstrBaseKeyName);
    LOAD_HSTRING(m_hstrActiveSnd);
    LOAD_HSTRING(m_hstrDestCmd);
	LOAD_DWORD(m_nKeyDataIndex);
    LOAD_BOOL(m_bUseVelocity);
    LOAD_BOOL(m_bStartActive);
    LOAD_BOOL(m_bStartPaused);
    LOAD_BOOL(m_bLooping);
    LOAD_BOOL(m_bIgnoreOffsets);
    LOAD_BOOL(m_bActive);
    LOAD_BOOL(m_bPaused);
    LOAD_BOOL(m_bFinished);
    LOAD_BOOL(m_bFirstUpdate);
    LOAD_BOOL(m_bAlignToPath);
    LOAD_BOOL(m_bPushObjects);
    LOAD_WORD(m_nNumKeys);
    LOAD_BYTE_CAST(m_eDirection, KFDirection);
    LOAD_BYTE_CAST(m_eWaveform, KFWaveType);

	// Build the m_pKeys data member...

	uint8* blindData;
	uint32 blindDataSize;
	if( g_pLTServer->GetBlindObjectData( m_nKeyDataIndex, KEYFRAMER_BLINDOBJECTID, blindData, blindDataSize ) != LT_OK )
	{
		ASSERT(0);			// for some reason the blind data isn't there
		m_nNumKeys = 0;
	}

	m_pKeys = LTNULL;

	if( blindData && m_nNumKeys )
	{
		uint8* curBlindData = blindData;

		ASSERT( *((uint32*)curBlindData) == m_nNumKeys );	// .dat is probably more recent than save file
		m_nNumKeys = *((uint32*)curBlindData);
		curBlindData += 4;

		int i;
		KEYNODE* pNode = LTNULL;
		float fTime = 0.0f;

		for (i=0; i < m_nNumKeys; i++)
		{
			if( i == 0 )
			{
				m_pKeys = debug_new( KEYNODE );
				pNode = m_pKeys;
			}
			else
			{
				pNode->pNext = debug_new( KEYNODE );
				pNode->pNext->pPrev = pNode;
				pNode = pNode->pNext;
			}

			// create the key from the blind data
			curBlindData = pNode->keyData.Copy( curBlindData );

			fTime += pNode->keyData.m_fTimeStamp;
			pNode->keyData.m_fRealTime = fTime;
		}

	}

	// we're done with the blind data for this keyframer, free it
	if( blindData )
		g_pLTServer->FreeBlindObjectData( m_nKeyDataIndex, KEYFRAMER_BLINDOBJECTID );


	// Determine the positions of our pointer data members...

    int nCurKeyIndex;
	LOAD_INT(nCurKeyIndex);
    int nPosition1Index;
	LOAD_INT(nPosition1Index);
    int nPosition2Index;
	LOAD_INT(nPosition2Index);
    int nDestKeyIndex;
	LOAD_INT(nDestKeyIndex);

	KEYNODE* pCurKey = m_pKeys;
	int i;
	for (i=0; i < m_nNumKeys && pCurKey; i++)
	{
		if (nCurKeyIndex == i)
		{
			m_pCurKey = pCurKey;
		}
		if (nPosition1Index == i)
		{
			m_pPosition1 = pCurKey;
		}
		if (nPosition2Index == i)
		{
			m_pPosition2 = pCurKey;
		}
		if (nDestKeyIndex == i)
		{
			m_pDestinationKey = pCurKey;
		}

		// This will end up being set to the last key...

		m_pLastKey = pCurKey;

		pCurKey = pCurKey->pNext;
	}

	// Load the number of objects we're supposed to key frame...

    uint8 nNumInList;
	LOAD_BYTE(nNumInList);

	// Load the offsets and rotations for each object...

	for (i=0; i < nNumInList; i++)
	{
		LOAD_VECTOR(m_pOffsets[i]);
		LOAD_ROTATION(m_pRotations[i]);
	}

	// Load the objects we're supposed to key-frame...

	if (nNumInList > 0)
	{
		if (m_pObjectList)
		{
            g_pLTServer->RelinquishList(m_pObjectList);
		}

        m_pObjectList = g_pLTServer->CreateObjectList();

		if (m_pObjectList)
		{
			HOBJECT* hObjectArray = debug_newa(HOBJECT, nNumInList);

			HOBJECT hObj = LTNULL;
			for (i=0; i < nNumInList; i++)
			{
				LOAD_HOBJECT(hObj);
				if (hObj)
				{
					hObjectArray[i] = hObj;
				}
				else
				{
					hObjectArray[i] = LTNULL;
				}
			}

			// Kind of assy, but need to add the objects in reverse order
			// since AddObjectToList always adds at the front of the list...

			for (i=nNumInList-1; i >= 0; i--)
			{
				if (hObjectArray[i])
				{
                    g_pLTServer->AddObjectToList(m_pObjectList, hObjectArray[i]);
				}
			}

			if (hObjectArray)
			{
				debug_deletea(hObjectArray);
			}
		}
	}

	// See if we need to calculate the velocity info...
	if (m_bUseVelocity)
	{
		CalculateVelocityInfo();
	}


	// If we were active, restart active sound...

	if (m_bActive)
	{
		m_bActive = LTFALSE; // need to clear this first...
		GoActive(LTFALSE);   // don't reset any values...
	}

	{ // BL 09/29/00 Added to fix falling off keyframed objects after loading game
		m_bPausedOnLoad = LTTRUE;
	}
}