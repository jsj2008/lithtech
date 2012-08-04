// ----------------------------------------------------------------------- //
//
// MODULE  : NodeController.cpp
//
// PURPOSE : Model node control
//
// CREATED : 1997
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "StdAfx.h"
#include "SFXMsgIds.h"
#include "NodeController.h"
#include "iltmodel.h"
#include "CharacterFx.h"
#include "SoundMgr.h"
#include "VarTrack.h"
#include "HudMgr.h"
#include "GameClientShell.h"
#include "MsgIds.h"
#include "HUDSubtitles.h"
#include "HUDDialogue.h"
#include "iltfilemgr.h"
#include "sys/win/mpstrconv.h"

//Lip sync file version information
#define LIPSYNC_FILE_ID				(('L' << 24) | ('I' << 16) | ('P' << 8) | 'S')
#define LIPSYNC_FILE_VERSION		1

#define INVALID_NODE_CONTROL		-1


VarTrack	g_vtLipFlapMaxRot;
VarTrack	g_vtLipFlapFreq;
VarTrack	g_vtDisableLipFlap;

// ----------------------------------------------------------------------- //

CNodeController::CNodeController()
{
    m_pCharacterFX = NULL;

	m_cNodeControls = 0;
	m_cNodes = 0;

	m_fCurLipFlapRot = 0.0f;
	m_bAddedNodeControlFn = false;
	m_bUseRadioSound = false;

    m_hSound = NULL;
	m_hRadioSound = NULL;
	m_nUniqueDialogueId = 0;
	m_bSubtitlePriority = false;

    m_pSixteenBitBuffer = NULL;
    m_pEightBitBuffer = NULL;
	m_dwSamplesPerSecond = 0;
	
	m_bShowingSubtitles = false;
}

// ----------------------------------------------------------------------- //

CNodeController::~CNodeController()
{
	// Make sure we free up allocated memory...
	CleanUpLipSyncNodeControls();
}

// ----------------------------------------------------------------------- //

bool CNodeController::Init(CCharacterFX* pCharacterFX)
{
	LTASSERT( pCharacterFX, "Invalide CharacterFX" );
    if ( !pCharacterFX ) return false;

	if (!g_vtLipFlapMaxRot.IsInitted())
	{
        g_vtLipFlapMaxRot.Init(g_pLTClient, "LipFlapMaxRot", NULL, 30.0f);
	}

	if (!g_vtLipFlapFreq.IsInitted())
	{
        g_vtLipFlapFreq.Init(g_pLTClient, "LipFlapFreq", NULL, 20.0f);
	}

	if (!g_vtDisableLipFlap.IsInitted())
	{
        g_vtDisableLipFlap.Init(g_pLTClient, "DisableLipFlap", NULL, 0.0f);
	}


	// Store our backpointer

	m_pCharacterFX = pCharacterFX;

	// Map all the nodes in our skeleton in the bute file to the nodes in the actual model file

	m_cNodes = 0;

	HMODELNODE hCurNode = INVALID_MODEL_NODE;
    while ( g_pLTClient->GetModelLT()->GetNextNode(GetCFX()->GetServerObj(), hCurNode, hCurNode) == LT_OK)
	{
		char szName[64] = "";
        g_pLTClient->GetModelLT()->GetNodeName(GetCFX()->GetServerObj(), hCurNode, szName, 64);

		ModelsDB::HNODE hModelNode = g_pModelsDB->GetSkeletonNode(m_pCharacterFX->GetModelSkeleton(), szName);

		if ( hModelNode )
		{
			if (m_cNodes < kMaxNodes)
			{
				m_aNodes[m_cNodes].hNode = hModelNode;
				m_aNodes[m_cNodes].hModelNode = hCurNode;
				m_cNodes++;
			}
			else
			{
				DebugCPrint(1,"ERROR in CNodeController::Init()!");
				DebugCPrint(1,"    More than (%d) model nodes!", kMaxNodes);
				return false;
			}
		}
	}

    return true;
}

// ----------------------------------------------------------------------- //

void CNodeController::Update()
{
	//don't bother if we are paused
	if( g_pGameClientShell->IsServerPaused( ))
		return;

	if ( m_cNodeControls > 0 )
	{
		if(!m_bAddedNodeControlFn)
		{
			g_pLTClient->GetModelLT()->AddNodeControlFn(GetCFX()->GetServerObj(), CNodeController::NodeControlFn, this);
			m_bAddedNodeControlFn = true;
		}
	}
	else
	{
		if(m_bAddedNodeControlFn)
		{
			g_pLTClient->GetModelLT()->RemoveNodeControlFn(GetCFX()->GetServerObj(), CNodeController::NodeControlFn, this);
			m_bAddedNodeControlFn = false;
		}
	}

	// Reset all our nodes matrices

	for ( int iNode = 0 ; iNode < kMaxNodes ; iNode++ )
	{
		// Only reset the matrix if our control refcount is greater than zero (ie we are being controlled)

		if ( m_aNodes[iNode].cControllers > 0 )
		{
			m_aNodes[iNode].matTransform.Identity();
		}
	}
	
	// Handle lip flapping without any valid node controls
	if ( ( m_hSound != NULL ) && ( m_cNodeControls == 0 ) )
	{
		UpdateLipFlapControl(NULL);
	}

	// Update all our active node controls.

	int cValidNodeControls = 0;

	for ( int iNodeControl = 0 ; iNodeControl < kMaxNodeControls && cValidNodeControls < m_cNodeControls ; iNodeControl++ )
	{
		if ( m_aNodeControls[iNodeControl].bValid )
		{
			// Update the node control

			cValidNodeControls++;

			switch ( m_aNodeControls[iNodeControl].eControl )
			{
				case eControlLipFlap:
					UpdateLipFlapControl(&m_aNodeControls[iNodeControl]);
					break;

				case eControlLipSync:
					UpdateLipSyncControl(&m_aNodeControls[iNodeControl]);
					break;

				default:
                    LTERROR( "Invalid NodeControl type." );
                    m_aNodeControls[iNodeControl].bValid = false;
					break;
			}

			// Check to see if the node control is done

			if ( !m_aNodeControls[iNodeControl].bValid )
			{
				RemoveNodeControl(iNodeControl);
			}
		}
	}
}

// ----------------------------------------------------------------------- //

void CNodeController::UpdateLipFlapControl(NCSTRUCT *pNodeControl)
{
	bool bEnd = false;

	// Make sure the sound handle is valid and check to see if the sound is done
	if ( !m_hSound || g_pLTClient->IsDone(m_hSound) )
	{
		bEnd = true;
	}


	LTRotation rRot;
	rRot.Init();

    if( !bEnd )
	{
		// [KLS 9/10/02] - If the sound system is re-inited our buffers will be cleared so
		// we need to make sure we always have valid sound buffers...

		if (!m_pSixteenBitBuffer && !m_pEightBitBuffer)
		{
			LTRESULT ltRes = g_pLTClient->GetSoundData(m_hSound, m_pSixteenBitBuffer,	
				m_pEightBitBuffer, &m_dwSamplesPerSecond, NULL);
		
			if( ltRes != LT_OK )
			{
				DebugCPrint(1,"CNodeController::UpdateLipFlapControl:  Could not read sound data!  Is it streamed?");
			}
		}
	
		// Process the current sound data (average over sound amplitude)
		float fAverage = 0.0f;  // this will hold the average, normalized from 0.0f to 1.0f.

		uint32 dwOffset = 0;
		uint32 dwSize   = 0;

		// Average over the data.  We do an average of the data from the current point
		// being played to 1/g_vtLipFlapFreq.GetFloat() seconds ahead of that point.
		if( LT_OK == g_pLTClient->GetSoundOffset(m_hSound, &dwOffset, &dwSize))
		{
			// Determine the end of the data we wish to average over.

			const uint32 dwDivisor = uint32(g_vtLipFlapFreq.GetFloat());
			uint32 dwOffsetEnd = dwOffset + m_dwSamplesPerSecond/dwDivisor;
			dwOffsetEnd = LTMIN(dwOffsetEnd, dwSize);

			// Accumulate the the amplitudes for the average.

			uint32 dwMaxAmplitude = 0;
			uint32 dwNumSamples = 0;
			uint32 dwAccum = 0;

			if( m_pSixteenBitBuffer )
			{
				for( int16 * pIterator = m_pSixteenBitBuffer + dwOffset;
					 pIterator < m_pSixteenBitBuffer + dwOffsetEnd;
					 ++pIterator)
				{
					dwAccum += abs(*pIterator);
					++dwNumSamples;
				}

				dwMaxAmplitude = 65536/2;
			}
			else if( m_pEightBitBuffer )
			{
				for( int8 * pIterator = m_pEightBitBuffer + dwOffset;
					 pIterator < m_pEightBitBuffer + dwOffsetEnd;
					 ++pIterator)
				{
					dwAccum += abs(*pIterator);
					++dwNumSamples;
				}

				dwMaxAmplitude = 256/2;
			}

			// And find the average!
			if( dwNumSamples > 0 )
			{
				fAverage = float(dwAccum) / float(dwNumSamples) / float(dwMaxAmplitude);
			}
		}

		// Do the rotation.

		LTVector vAxis(0.0f, 0.0f, 1.0f);
		float fMaxRot = MATH_DEGREES_TO_RADIANS(g_vtLipFlapMaxRot.GetFloat());

		// Calculate the rotation

		m_fCurLipFlapRot =  fAverage*fMaxRot;
		rRot.Rotate(vAxis, -m_fCurLipFlapRot);

		// Check if they have a valid modelnode.
		if( pNodeControl && !pNodeControl->pNStruct->hNode )
		{
			LTERROR( "CNodeController::UpdateLipFlapControl: Invalid modelnode." );
			bEnd = true;
		}
	}

	if( !bEnd && pNodeControl )
	{
		// Create a rotation matrix and apply it to the current offset matrix
		LTMatrix m1;
		rRot.ConvertToMatrix(m1);
		pNodeControl->pNStruct->matTransform = pNodeControl->pNStruct->matTransform * m1;
	}

	if( bEnd )
	{
		KillLipSyncSound( true );
		if ( pNodeControl )
			pNodeControl->bValid = false;
	}
}

// ----------------------------------------------------------------------- //

void CNodeController::UpdateLipSyncControl(NCSTRUCT *pNodeControl)
{
	// Make sure the sound handle is valid and check to see if the sound is done
	if ( !m_hSound || g_pLTClient->IsDone(m_hSound) )
	{
		KillLipSyncSound( true );
		pNodeControl->bValid = false;

		return;
	}

	ObjectContextTimer engineTimer = m_pCharacterFX->GetServerObj();

	pNodeControl->fTimer += engineTimer.GetTimerElapsedS();
	if ( pNodeControl->fTimer < 0.0f ) 
		return;

	//the amount of time each keyframe lasts
	static const float kfKeyFrameTime = 1.0f / 30.0f;	

	int iKeyframe = int(pNodeControl->fTimer / kfKeyFrameTime);
	
	if ( iKeyframe >= pNodeControl->cKeyframes ) 
		return;

	int iKeyframeNext = LTMIN<int>(pNodeControl->cKeyframes-1, iKeyframe+1);
	float fInterpolation = pNodeControl->fTimer / kfKeyFrameTime - (float)iKeyframe;

	// Check if they have a valid modelnode.
	if( !pNodeControl->pNStruct->hNode )
	{
		LTERROR( "CNodeController::UpdateLipSyncControl: Invalid modelnode." );
		return;
	}

//	g_pLTClient->CPrint("%d: Updating lip syncing, keyframe = %d, fInterpolation = %f", pNodeControl, iKeyframe, fInterpolation);

	LTVector vTranslation = pNodeControl->aKeyframes[iKeyframe].vTranslation*(1.0f - fInterpolation) + 
							pNodeControl->aKeyframes[iKeyframeNext].vTranslation*(fInterpolation);

	LTRotation rRotation;
	rRotation.Slerp(pNodeControl->aKeyframes[iKeyframe].rRotation, pNodeControl->aKeyframes[iKeyframeNext].rRotation, fInterpolation);

	LTMatrix m;
	m.Identity();

	rRotation.ConvertToMatrix(m);
	m.SetTranslation(vTranslation);

	pNodeControl->pNStruct->matTransform = pNodeControl->pNStruct->matTransform * m;
}

// ----------------------------------------------------------------------- //

bool CNodeController::HandleNodeControlMessage(uint8 byMsgId, ILTMessage_Read *pMsg)
{
	switch ( byMsgId )
	{
		case CFX_NODECONTROL_LIP_SYNC:
		{
			HandleNodeControlLipSyncMessage(pMsg);
			break;
		}

		default :
		{
			DebugCPrint(1,"ERROR in CNodeController::HandleNodeControlMessage invalid message Id!");
		}
		break;
	}

    return true;
}

// ----------------------------------------------------------------------- //

void CNodeController::HandleNodeControlLipSyncMessage(ILTMessage_Read *pMsg)
{
	uint32 nIndex = pMsg->Readuint32();
	if( nIndex == -1 )
	{
		KillLipSyncSound( true );
		return;
	}

	float fOuterRadius = pMsg->Readfloat();
	float fInnerRadius = pMsg->Readfloat();
	m_nUniqueDialogueId = pMsg->Readuint8( );
	CharacterSoundType cst = (CharacterSoundType)pMsg->Readuint8( );
	int16 nMixChannel = pMsg->Readint16();
	m_bUseRadioSound = pMsg->Readbool();
	char szIcon[128] = "";
	pMsg->ReadString(szIcon,LTARRAYSIZE(szIcon));


	m_bSubtitlePriority = (cst == CST_DIALOG);
	bool bCensor = (cst == CST_DEATH) || (cst == CST_DAMAGE);

	if (bCensor && g_pVersionMgr->IsLowViolence())
	{
		//don't play the sound, but do notify the server that we're done with it
		KillLipSyncSound( true );
		return;
	}

	
	const char* szStringID = StringIDFromIndex( nIndex  );
	HandleNodeControlLipSync(szStringID, fOuterRadius, fInnerRadius, nMixChannel,szIcon);
}

// ----------------------------------------------------------------------- //

void CNodeController::HandleNodeControlLipFlap( char const* pszStringID, float fOuterRadius, float fInnerRadius, int16 nMixChannel, const char* szIcon)
{
	if (g_vtDisableLipFlap.GetFloat())
		return;

	if ( m_hSound )
	{
		KillLipSyncSound( false );
	}

	bool bSubtitles = false;

	m_bShowingSubtitles = bSubtitles;
	m_sDialogueIcon = szIcon ? szIcon : "";

	ResetSoundBufferData();

	// Start the sound.
	m_hSound = m_pCharacterFX->PlayLipSyncSound(pszStringID, fOuterRadius, fInnerRadius, bSubtitles, 
		m_bSubtitlePriority, nMixChannel, true, szIcon, m_bUseRadioSound, &m_hRadioSound);
	if( m_hSound == NULL )
	{
		KillLipSyncSound( true );
		return;
	}

	ModelsDB::HNODE hModelNode = g_pModelsDB->GetSkeletonNode(m_pCharacterFX->GetModelSkeleton(), "Face_Jaw");

	// See if we are already controlling the jaw node

	int iCtrl = FindNodeControl(hModelNode, eControlLipFlap);

	// Check to make sure all the info is ok...

	if ( !hModelNode || fOuterRadius <= 0.0f)
	{
		if( iCtrl != INVALID_NODE_CONTROL )
		{
            m_aNodeControls[iCtrl].bValid = false;
		}

		return;
	}

	// Add the node control structure...

	int iNodeControl = (iCtrl != INVALID_NODE_CONTROL) ? iCtrl : AddNodeControl();

	LTASSERT( iNodeControl != INVALID_NODE_CONTROL, "Invalid NodeControl" );
	if ( iNodeControl == INVALID_NODE_CONTROL )
	{
		return;
	}

	NSTRUCT* pStruct = FindNode( hModelNode );
	if( !pStruct )
		return;

	NCSTRUCT* pNodeControl = &m_aNodeControls[iNodeControl];
	pNodeControl->eControl = eControlLipFlap;
	pNodeControl->pNStruct = pStruct;
    pNodeControl->bValid = true;

	// Increment the number of controllers for this node
	pStruct->cControllers++;
}

// ----------------------------------------------------------------------- //

void CNodeController::HandleNodeControlLipSync( char const* pszStringID, float fOuterRadius, float fInnerRadius, int16 nMixChannel, const char* szIcon)
{
	const char* pszVoiceFile = g_pLTIStringEdit->GetVoicePath( g_pLTDBStringEdit, pszStringID );

	char szPhoneme[_MAX_PATH + 1];
	LTStrCpy( szPhoneme, pszVoiceFile, LTARRAYSIZE( szPhoneme ));
    

	// [KLS 3/1/02] - Make sure we clean up any lip syncing node controls that are 
	// currently active...

	CleanUpLipSyncNodeControls();

	char* pch = strchr(szPhoneme, '.');

	if ( !pch )
	{
		// DebugCPrint(1,"CNodeController::HandleNodeControlLipSync() invalid pszSoundFile: '%s'", pszSoundFile ? pszSoundFile : "NULL");
		return;
	}


	LTStrCpy( pch, ".lip", LTARRAYSIZE( szPhoneme ) - ( pch - szPhoneme ));

    ILTInStream* pStream = g_pLTBase->FileMgr()->OpenFile(szPhoneme);
    if (!pStream)
	{
		// g_pLTClient->CPrint("Playing sound '%s' as a lip FLAP sound...Couldn't find .lip file!!!", pszSoundFile);

		// If the lipsync file doesn't exist just use the old-school lipflap
		HandleNodeControlLipFlap( pszStringID, fOuterRadius, fInnerRadius, nMixChannel,szIcon );
		return;
	}

	//make sure it is the right file type and version
	uint32 nFileType = 0;
	uint32 nVersion = 0;

	pStream->Read(&nFileType, sizeof(nFileType));
	pStream->Read(&nVersion, sizeof(nVersion));

	//make sure they match
	if((nFileType != LIPSYNC_FILE_ID) || (nVersion != LIPSYNC_FILE_VERSION))
	{
		pStream->Release();
	
		DebugCPrint(1,"Playing sound '%s' as a lip FLAP sound: (nFileType != LIPSYNC_FILE_ID) || (nVersion != LIPSYNC_FILE_VERSION)!!!", pszStringID);
	
		HandleNodeControlLipFlap( pszStringID, fOuterRadius, fInnerRadius, nMixChannel,szIcon );
		return;
	}

	// If the radius is 0, that means we were supposed to stop the current sound, so we're done...
	if (fOuterRadius <= 0.0f)
	{
		pStream->Release();
		return;
	}
	
	uint32 cBones		= 0;
	pStream->Read(&cBones, sizeof(cBones));

	//read in the number of keyframes
	uint32 cKeyframes	= 0;
	pStream->Read(&cKeyframes, sizeof(cKeyframes));

	bool bSubtitles = false;
	m_sDialogueIcon = szIcon ? szIcon : "";
	m_hSound = m_pCharacterFX->PlayLipSyncSound(pszStringID, fOuterRadius, fInnerRadius, bSubtitles, 
		m_bSubtitlePriority, nMixChannel, true, szIcon, m_bUseRadioSound, &m_hRadioSound);
	if( !m_hSound )
	{
		KillLipSyncSound( true );
	}

	int32 aiNodeControls[kMaxNodeControls];

	for ( uint32 iBone = 0 ; iBone < cBones ; iBone++ )
	{
		char szBone[256];

		//read in the size of the bone
		int nBoneSize = 0;
		pStream->Read(&nBoneSize, sizeof(nBoneSize));

		//now read in the bone name
		pStream->Read(szBone, nBoneSize);

		//end the string appropriately
		szBone[nBoneSize] = '\0';

		ModelsDB::HNODE hModelNode = g_pModelsDB->GetSkeletonNode(m_pCharacterFX->GetModelSkeleton(), szBone);
		if( !hModelNode )
		{
			char szErrorMsg[256] = {0};
            LTSNPrintF( szErrorMsg, LTARRAYSIZE(szErrorMsg), "Invalid modelnode: (%s).", szBone );
			LTERROR( szErrorMsg );
			aiNodeControls[iBone] = INVALID_NODE_CONTROL;
			continue;
		}
		NSTRUCT* pStruct = FindNode( hModelNode );
		if( !pStruct )
		{
			char szErrorMsg[256] = {0};
			LTSNPrintF( szErrorMsg, LTARRAYSIZE(szErrorMsg), "Invalid modelnode: (%s).", szBone );
			LTERROR( szErrorMsg );
			aiNodeControls[iBone] = INVALID_NODE_CONTROL;
			continue;
		}

		int iNodeControl = FindNodeControl(hModelNode, eControlLipSync);
		iNodeControl = (iNodeControl != INVALID_NODE_CONTROL) ? iNodeControl : AddNodeControl();
		
		aiNodeControls[iBone] = iNodeControl;

		LTASSERT( iNodeControl != INVALID_NODE_CONTROL, "Invalid NodeControl" );
		if( iNodeControl == INVALID_NODE_CONTROL )
			continue;

		m_aNodeControls[iNodeControl].bValid = true;
		m_aNodeControls[iNodeControl].eControl = eControlLipSync;
		m_aNodeControls[iNodeControl].pNStruct = pStruct;
		m_aNodeControls[iNodeControl].cKeyframes = cKeyframes;
		m_aNodeControls[iNodeControl].aKeyframes = debug_newa(NCKEYFRAME, cKeyframes);
		m_bShowingSubtitles = bSubtitles;
		m_aNodeControls[iNodeControl].fTimer = -GetConsoleFloat("fudge", 0.0f);
	
		pStruct->cControllers++;
	}

	for ( iBone = 0 ; iBone < cBones ; iBone++ )
	{
		//read in all the keyframes for this node
		if( aiNodeControls[iBone] != INVALID_NODE_CONTROL )
		{
			pStream->Read(m_aNodeControls[aiNodeControls[iBone]].aKeyframes, sizeof(NCKEYFRAME) * cKeyframes);
		}
	}

	pStream->Release();
}

// Clean up all eControlLipSync node controls...

void CNodeController::CleanUpLipSyncNodeControls()
{
	KillLipSyncSound( false );

	// Reset all eControlLipSync controls...

	for (int i = 0; i < kMaxNodeControls; i++)
	{
			// Free up the memory...

			if (m_aNodeControls[i].aKeyframes)
			{
				debug_deletea(m_aNodeControls[i].aKeyframes);
				m_aNodeControls[i].aKeyframes = NULL;
			}

			// Reset the node control...

			m_aNodeControls[i].Reset();
		}
	}

// ----------------------------------------------------------------------- //

NSTRUCT* CNodeController::FindNode(HMODELNODE hModelNode)
{
	for ( int iNode = 0 ; iNode < kMaxNodes ; iNode++ )
	{
		if ( m_aNodes[iNode].hModelNode == hModelNode )
		{
			return &m_aNodes[iNode];
		}
	}

    return NULL;
}

NSTRUCT* CNodeController::FindNode(ModelsDB::HNODE hNode)
{
	for ( int iNode = 0 ; iNode < kMaxNodes ; iNode++ )
	{
		if ( m_aNodes[iNode].hNode == hNode )
		{
			return &m_aNodes[iNode];
		}
	}

	return NULL;
}

// ----------------------------------------------------------------------- //

int CNodeController::FindNodeControl(ModelsDB::HNODE hNode, Control eControl)
{
	for(int i = 0; i < kMaxNodeControls; i++)
	{
		if(m_aNodeControls[i].bValid && m_aNodeControls[i].pNStruct->hNode == hNode && m_aNodeControls[i].eControl == eControl)
			return i;
	}

	return INVALID_NODE_CONTROL;
}

// ----------------------------------------------------------------------- //

int CNodeController::AddNodeControl()
{
	for ( int iNodeControl = 0 ; iNodeControl < kMaxNodeControls ; iNodeControl++ )
	{
		if ( !m_aNodeControls[iNodeControl].bValid )
		{
			m_cNodeControls++;
			return iNodeControl;
		}
	}

	return INVALID_NODE_CONTROL;
}

// ----------------------------------------------------------------------- //

void CNodeController::RemoveNodeControl(int iNodeControl)
{
	LTASSERT( !m_aNodeControls[iNodeControl].bValid, "NodeControl not valid." );

	if( m_aNodeControls[iNodeControl].pNStruct )
	{
		m_aNodeControls[iNodeControl].pNStruct->cControllers--;
	}

	m_cNodeControls--;
}

// ----------------------------------------------------------------------- //

void CNodeController::NodeControlFn(const NodeControlData& Data, void *pUserData)
{
	CNodeController* pNC = (CNodeController*)pUserData;
	LTASSERT( pNC, "No NodeCOntrol" );
	if ( !pNC ) return;

	//build up the current transform
	LTTransform tWS = (*Data.m_pModelTransform) * (*Data.m_pNodeTransform);
    
	//build up a matrix that they can modify
	LTMatrix mResult;
	tWS.ToMatrix(mResult);

	pNC->HandleNodeControl(Data.m_hModel, Data.m_hNode, &mResult);

	//and now convert back to the node transform
	LTTransform tResult;
	tResult.FromMatrixWithScale(mResult);

	//move it into object space
	tResult = Data.m_pModelTransform->GetInverse() * tResult;

	//and store this back into the node transform
	Data.m_pNodeTransform->m_vPos = tResult.m_vPos;
	Data.m_pNodeTransform->m_rRot = tResult.m_rRot;
}

// ----------------------------------------------------------------------- //

void CNodeController::HandleNodeControl(HOBJECT /*hObj*/, HMODELNODE hNode, LTMatrix *pGlobalMat)
{
	NSTRUCT* pNode = FindNode(hNode);

	if ( pNode && pNode->cControllers > 0 )
	{
		*pGlobalMat = *pGlobalMat * pNode->matTransform;
	}
}


// ----------------------------------------------------------------------- //

bool CNodeController::IsLipSynching()
{
	for (int i = 0; i < kMaxNodeControls ; i++)
	{
		if ( m_aNodeControls[i].bValid && (m_aNodeControls[i].eControl == eControlLipFlap || m_aNodeControls[i].eControl == eControlLipSync ))
		{
			return true;
		}
	}

	return false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::KillLipSyncSound
//
//	PURPOSE:	Kill the lipsync sound.
//
// ----------------------------------------------------------------------- //

void CNodeController::KillLipSyncSound( bool bSendNotification )
{
	if( m_hSound )
	{
		g_pLTClient->SoundMgr()->KillSound( m_hSound );
		m_hSound = NULL;
	}

	if( m_hRadioSound )
	{
		g_pLTClient->SoundMgr()->KillSound( m_hRadioSound );
		m_hRadioSound = NULL;
	}

	if( m_bShowingSubtitles )
	{
 		if( g_pSubtitles )
			g_pSubtitles->Clear();
		m_bShowingSubtitles = false;
	}

	if (!m_sDialogueIcon.empty())
	{
		if( g_pHUDDialogue )
			g_pHUDDialogue->Hide(m_sDialogueIcon.c_str());
		m_sDialogueIcon = "";
	}

	ResetSoundBufferData();

	if( bSendNotification && m_pCharacterFX )
	{
		if( !m_pCharacterFX->GetServerObj( ))
		{
			LTERROR( "CNodeController::KillLipSyncSound:  No characterfx object for nodecontroller." );
			return;
		}

		// Tell the server that the sound finished.
		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_OBJECT_MESSAGE );
		cMsg.WriteObject( m_pCharacterFX->GetServerObj( ));
		cMsg.Writeuint32( MID_SFX_MESSAGE );
		cMsg.Writeuint8( SFX_CHARACTER_ID );
		cMsg.WriteBits(CFX_NODECONTROL_LIP_SYNC, FNumBitsExclusive<CFX_COUNT>::k_nValue );
		cMsg.Writeuint32( m_nUniqueDialogueId );
		cMsg.Writeint16( 0 ); // mixchannel
		cMsg.Writebool( false ); // use radio sound?
		g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
	}
}
