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
#include "ilttransform.h"
#include "CharacterFx.h"
#include "SoundMgr.h"
#include "VarTrack.h"
#include "HudMgr.h"
#include "GameClientShell.h"
#include "MsgIds.h"

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
    m_pCharacterFX = LTNULL;

	m_cNodeControls = 0;

	m_fCurLipFlapRot = 0.0f;
	m_bAddedNodeControlFn = false;

    m_hSound = LTNULL;
	m_nUniqueDialogueId = 0;
	m_bSubtitlePriority = false;

    m_pSixteenBitBuffer = LTNULL;
    m_pEightBitBuffer = LTNULL;
	m_dwSamplesPerSecond = 0;
	
	m_bShowingSubtitles = LTFALSE;
}

// ----------------------------------------------------------------------- //

CNodeController::~CNodeController()
{
	// Make sure we free up allocated memory...
	CleanUpLipSyncNodeControls();
}

// ----------------------------------------------------------------------- //

LTBOOL CNodeController::Init(CCharacterFX* pCharacterFX)
{
	_ASSERT(pCharacterFX);
    if ( !pCharacterFX ) return LTFALSE;

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

	int iNode = 0;
	HMODELNODE hCurNode = INVALID_MODEL_NODE;
    while ( g_pLTClient->GetModelLT()->GetNextNode(GetCFX()->GetServerObj(), hCurNode, hCurNode) == LT_OK)
	{
		char szName[64] = "";
        g_pLTClient->GetModelLT()->GetNodeName(GetCFX()->GetServerObj(), hCurNode, szName, 64);

		ModelNode eModelNode = g_pModelButeMgr->GetSkeletonNode(m_pCharacterFX->GetModelSkeleton(), szName);

		if ( eModelNode != eModelNodeInvalid )
		{
			_ASSERT(eModelNode < kMaxNodes);

			if (eModelNode < kMaxNodes)
			{
				m_aNodes[eModelNode].eModelNode = eModelNode;
				m_aNodes[eModelNode].hModelNode = hCurNode;
			}
			else
			{
				g_pLTClient->CPrint("ERROR in CNodeController::Init()!");
				g_pLTClient->CPrint("    More than (%d) model nodes!", kMaxNodes);
				return LTFALSE;
			}
		}
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CNodeController::Update()
{
	//don't bother if we are paused
	uint32 nFlags;
	g_pLTClient->Common()->GetObjectFlags(GetCFX()->GetServerObj(), OFT_Flags, nFlags);

	if(nFlags & FLAG_PAUSED)
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

				case eControlHeadFollowObj:
					UpdateHeadFollowObjControl(&m_aNodeControls[iNodeControl]);
					break;

				default:
                    _ASSERT(LTFALSE);
                    m_aNodeControls[iNodeControl].bValid = LTFALSE;
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
				g_pLTClient->CPrint("CNodeController::UpdateLipFlapControl:  Could not read sound data!  Is it streamed?");
			}
		}
	
		// Process the current sound data (average over sound amplitude)
		LTFLOAT fAverage = 0.0f;  // this will hold the average, normalized from 0.0f to 1.0f.

		uint32 dwOffset = 0;
		uint32 dwSize   = 0;

		// Average over the data.  We do an average of the data from the current point
		// being played to 1/g_vtLipFlapFreq.GetFloat() seconds ahead of that point.
		if( LT_OK == g_pLTClient->GetSoundOffset(m_hSound, &dwOffset, &dwSize))
		{
			// Determine the end of the data we wish to average over.

			const uint32 dwDivisor = uint32(g_vtLipFlapFreq.GetFloat());
			uint32 dwOffsetEnd = dwOffset + m_dwSamplesPerSecond/dwDivisor;
			dwOffsetEnd = Min<uint32>(dwOffsetEnd, dwSize);

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
				fAverage = LTFLOAT(dwAccum) / LTFLOAT(dwNumSamples) / LTFLOAT(dwMaxAmplitude);
			}
		}

		// Do the rotation.

		LTVector vAxis(0.0f, 0.0f, 1.0f);
		LTFLOAT fMaxRot = MATH_DEGREES_TO_RADIANS(g_vtLipFlapMaxRot.GetFloat());

		// Calculate the rotation

		m_fCurLipFlapRot =  fAverage*fMaxRot;
		rRot.Rotate(vAxis, -m_fCurLipFlapRot);

		// Check if they have a valid modelnode.
		if( pNodeControl->eModelNode == eModelNodeInvalid )
		{
			_ASSERT( !"CNodeController::UpdateLipFlapControl: Invalid modelnode." );
			bEnd = true;
		}
	}

	if( !bEnd )
	{
		// Create a rotation matrix and apply it to the current offset matrix
		LTMatrix m1;
		rRot.ConvertToMatrix(m1);
		m_aNodes[pNodeControl->eModelNode].matTransform = m_aNodes[pNodeControl->eModelNode].matTransform * m1;
	}

	if( bEnd )
	{
		KillLipSyncSound( true );
	    pNodeControl->bValid = LTFALSE;
	}
}

// ----------------------------------------------------------------------- //

void CNodeController::UpdateLipSyncControl(NCSTRUCT *pNodeControl)
{
	// Make sure the sound handle is valid and check to see if the sound is done

    if ( !m_hSound || g_pLTClient->IsDone(m_hSound) )
	{
		KillLipSyncSound( true );
		pNodeControl->bValid = LTFALSE;

		return;
	}

	pNodeControl->fTimer += g_pLTClient->GetFrameTime();
	if ( pNodeControl->fTimer < 0.0f ) 
		return;

	//the amount of time each keyframe lasts
	static const float kfKeyFrameTime = 1.0f / 30.0f;	

	int iKeyframe = int(pNodeControl->fTimer / kfKeyFrameTime);
	
	if ( iKeyframe >= pNodeControl->cKeyframes ) 
		return;

	int iKeyframeNext = Min<int>(pNodeControl->cKeyframes-1, iKeyframe+1);
	float fInterpolation = pNodeControl->fTimer / kfKeyFrameTime - (float)iKeyframe;

	// Check if they have a valid modelnode.
	if( pNodeControl->eModelNode == eModelNodeInvalid )
	{
		_ASSERT( !"CNodeController::UpdateLipSyncControl: Invalid modelnode." );
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

	m_aNodes[pNodeControl->eModelNode].matTransform = m_aNodes[pNodeControl->eModelNode].matTransform * m;
}

// ----------------------------------------------------------------------- //

LTBOOL CNodeController::HandleNodeControlMessage(uint8 byMsgId, ILTMessage_Read *pMsg)
{
	switch ( byMsgId )
	{
		case CFX_NODECONTROL_LIP_SYNC:
		{
			HandleNodeControlLipSyncMessage(pMsg);
			break;
		}

		case CFX_NODECONTROL_HEAD_FOLLOW_OBJ:
		{
			HandleNodeControlHeadFollowObjMessage(pMsg);
			break;
		}

		default :
		{
			g_pLTClient->CPrint("ERROR in CNodeController::HandleNodeControlMessage invalid message Id!");
		}
		break;
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CNodeController::HandleNodeControlLipSyncMessage(ILTMessage_Read *pMsg)
{
    HSTRING hSoundFile = pMsg->ReadHString();
    LTFLOAT fRadius = pMsg->Readfloat();
	m_nUniqueDialogueId = pMsg->Readuint8( );
	CharacterSoundType cst = (CharacterSoundType)pMsg->Readuint8( );

	m_bSubtitlePriority = (cst == CST_DIALOG);
	bool bCensor = (cst == CST_DEATH) || (cst == CST_DAMAGE);

	if (bCensor && g_pVersionMgr->IsLowViolence())
	{
		//don't play the sound, but do notify the server that we're done with it
		KillLipSyncSound( true );
		return;
	}

	char const* pszSoundFile = g_pLTClient->GetStringData(hSoundFile);

	HandleNodeControlLipSync(pszSoundFile, fRadius);

    g_pLTClient->FreeString(hSoundFile);
	hSoundFile = NULL;
}

// ----------------------------------------------------------------------- //

void CNodeController::HandleNodeControlLipFlap( char const* pszSoundFile, LTFLOAT fRadius)
{
	if (g_vtDisableLipFlap.GetFloat())
		return;

 	ModelNode eModelNode = g_pModelButeMgr->GetSkeletonNode(m_pCharacterFX->GetModelSkeleton(), "Face_Jaw");

	// See if we are already controlling the jaw node

	int iCtrl = FindNodeControl(eModelNode, eControlLipFlap);

	LTBOOL bSubtitles = LTFALSE;
	char szSound[128];
    strcpy(szSound, pszSoundFile);

	// Check to make sure all the info is ok...

	if ((eModelNode == eModelNodeInvalid) || fRadius <= 0.0f)
	{
		if( iCtrl != INVALID_NODE_CONTROL )
		{
			if ( m_hSound )
			{
				KillLipSyncSound( false );
			}

            m_aNodeControls[iCtrl].bValid = LTFALSE;
		}

		return;
	}

	// Add the node control structure...

	int iNodeControl = (iCtrl != INVALID_NODE_CONTROL) ? iCtrl : AddNodeControl();

	ASSERT(iNodeControl != INVALID_NODE_CONTROL);
	if ( iNodeControl == INVALID_NODE_CONTROL )
	{
		return;
	}

	if ( m_hSound )
	{
		KillLipSyncSound( false );
	}

	NCSTRUCT* pNodeControl = &m_aNodeControls[iNodeControl];
	pNodeControl->eControl = eControlLipFlap;
	pNodeControl->eModelNode = eModelNode;

	m_bShowingSubtitles = bSubtitles;

	ResetSoundBufferData();

	// Start the sound.
	m_hSound = m_pCharacterFX->PlayLipSyncSound(szSound, fRadius, bSubtitles, m_bSubtitlePriority);
	if( m_hSound == NULL )
	{
		KillLipSyncSound( true );
		return;
	}

    pNodeControl->bValid = LTTRUE;

	// Increment the number of controllers for this node

	m_aNodes[eModelNode].cControllers++;
}

// ----------------------------------------------------------------------- //

void CNodeController::HandleNodeControlLipSync( char const* pszSoundFile, LTFLOAT fRadius)
{
	char szSound[_MAX_PATH + 1];
	char szPhoneme[_MAX_PATH + 1];

    strcpy(szSound, pszSoundFile);
	strcpy(szPhoneme, pszSoundFile);
    

	// [KLS 3/1/02] - Make sure we clean up any lip syncing node controls that are 
	// currently active...

	CleanUpLipSyncNodeControls();

	char* pch = strchr(szPhoneme, '.');

	if ( !pch )
	{
		// g_pLTClient->CPrint("CNodeController::HandleNodeControlLipSync() invalid pszSoundFile: '%s'", pszSoundFile ? pszSoundFile : "NULL");
		return;
	}


	strcpy(pch, ".lip");

    ILTStream* pStream = LTNULL;
    LTRESULT dr = g_pLTBase->OpenFile(szPhoneme, &pStream);

    if (dr != LT_OK || !pStream)
	{
		// g_pLTClient->CPrint("Playing sound '%s' as a lip FLAP sound...Couldn't find .lip file!!!", pszSoundFile);

		// If the lipsync file doesn't exist just use the old-school lipflap
		HandleNodeControlLipFlap( szSound, fRadius );
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
	
		g_pLTClient->CPrint("Playing sound '%s' as a lip FLAP sound: (nFileType != LIPSYNC_FILE_ID) || (nVersion != LIPSYNC_FILE_VERSION)!!!", pszSoundFile);
	
		HandleNodeControlLipFlap( szSound, fRadius );
		return;
	}

	// If the radius is 0, that means we were supposed to stop the current sound, so we're done...
	if (fRadius <= 0.0f)
	{
		pStream->Release();
		return;
	}
	
	uint32 cBones		= 0;
	pStream->Read(&cBones, sizeof(cBones));

	//read in the number of keyframes
	uint32 cKeyframes	= 0;
	pStream->Read(&cKeyframes, sizeof(cKeyframes));

	LTBOOL bSubtitles = LTFALSE;
	m_hSound = m_pCharacterFX->PlayLipSyncSound(szSound, fRadius, bSubtitles, m_bSubtitlePriority);
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

	 	ModelNode eModelNode = g_pModelButeMgr->GetSkeletonNode(m_pCharacterFX->GetModelSkeleton(), szBone);
		if( eModelNode == eModelNodeInvalid )
		{
			_ASSERT( !"CNodeController::HandleNodeControlLipSync:  Invalid modelnode." );
			aiNodeControls[iBone] = INVALID_NODE_CONTROL;
			continue;
		}

		int iNodeControl = FindNodeControl(eModelNode, eControlLipSync);
		iNodeControl = (iNodeControl != INVALID_NODE_CONTROL) ? iNodeControl : AddNodeControl();
		
		aiNodeControls[iBone] = iNodeControl;

		ASSERT( iNodeControl != INVALID_NODE_CONTROL );
		if( iNodeControl == INVALID_NODE_CONTROL )
			continue;

		m_aNodeControls[iNodeControl].bValid = LTTRUE;
		m_aNodeControls[iNodeControl].eControl = eControlLipSync;
		m_aNodeControls[iNodeControl].eModelNode = eModelNode;
		m_aNodeControls[iNodeControl].cKeyframes = cKeyframes;
		m_aNodeControls[iNodeControl].aKeyframes = debug_newa(NCKEYFRAME, cKeyframes);
		m_bShowingSubtitles = bSubtitles;
		m_aNodeControls[iNodeControl].fTimer = -GetConsoleFloat("fudge", 0.0f);
	
		m_aNodes[eModelNode].cControllers++;
	}

	for ( uint32 iBone = 0 ; iBone < cBones ; iBone++ )
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
			m_aNodeControls[i].aKeyframes = LTNULL;
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

    return LTNULL;
}

// ----------------------------------------------------------------------- //

int CNodeController::FindNodeControl(ModelNode eNode, Control eControl)
{
	for(int i = 0; i < kMaxNodeControls; i++)
	{
		if(m_aNodeControls[i].bValid && m_aNodeControls[i].eModelNode == eNode && m_aNodeControls[i].eControl == eControl)
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
	_ASSERT(!m_aNodeControls[iNodeControl].bValid);

	if( m_aNodeControls[iNodeControl].eModelNode != eModelNodeInvalid )
		m_aNodes[m_aNodeControls[iNodeControl].eModelNode].cControllers--;

	m_cNodeControls--;
}

// ----------------------------------------------------------------------- //

void CNodeController::NodeControlFn(const NodeControlData& Data, void *pUserData)
{
	CNodeController* pNC = (CNodeController*)pUserData;
	_ASSERT(pNC);
	if ( !pNC ) return;

	pNC->HandleNodeControl(Data.m_hModel, Data.m_hNode, Data.m_pNodeTransform);
}

// ----------------------------------------------------------------------- //

void CNodeController::HandleNodeControl(HOBJECT hObj, HMODELNODE hNode, LTMatrix *pGlobalMat)
{
	NSTRUCT* pNode = FindNode(hNode);

	if ( pNode && pNode->cControllers > 0 )
	{
		*pGlobalMat = *pGlobalMat * pNode->matTransform;
	}
}


// ----------------------------------------------------------------------- //

LTBOOL CNodeController::IsLipSynching()
{
	for (int i = 0; i < kMaxNodeControls ; i++)
	{
		if ( m_aNodeControls[i].bValid && (m_aNodeControls[i].eControl == eControlLipFlap || m_aNodeControls[i].eControl == eControlLipSync ))
		{
			return LTTRUE;
		}
	}

	return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::UpdateHeadFollowObjControl
//
//	PURPOSE:	Updates a follow object node control
//
// ----------------------------------------------------------------------- //

void CNodeController::UpdateHeadFollowObjControl(NCSTRUCT *pNodeControl)
{
    LTVector vPos;
    LTRotation rRot;
	LTransform transform;
    LTVector vU, vR, vF;

	if( pNodeControl->eModelNode == eModelNodeInvalid )
	{
		_ASSERT( !"CNodeController::UpdateHeadFollowObjControl: Invalid modelnode." );
		return;
	}
	
	//----------------------------------------------------------------------
	// Get information about the control node...
	// *** NOTE: On the head node... vU faces forward, vR faces down, vF faces right ***

	// Get access to the controls...
    ILTModel *pModelLT = g_pLTClient->GetModelLT();
    ILTTransform *pTransformLT = g_pLTClient->GetTransformLT();

	// Get the transform of the node we're controlling
    pModelLT->GetNodeTransform(GetCFX()->GetServerObj(), m_aNodes[pNodeControl->eModelNode].hModelNode, transform, LTTRUE);

	// Get forward vector of character.
	g_pLTClient->GetObjectRotation(GetCFX()->GetServerObj(), &rRot);
	vF = rRot.Forward();
	vU = rRot.Up();
	vR = rRot.Right();

	// Decompose the transform into the position and rotation
	pTransformLT->Get(transform, vPos, rRot);

	//----------------------------------------------------------------------
	// Get information about the follow object position...
    LTVector vObjPos;

	if(pNodeControl->hFollowObjNode == INVALID_MODEL_NODE)
	{
		// Just get the position of the follow object if the follow node is invalid
        g_pLTClient->GetObjectPos(pNodeControl->hFollowObj, &vObjPos);
	}
	else
	{
		// Get the transform of the node we're following
        pModelLT->GetNodeTransform(pNodeControl->hFollowObj, pNodeControl->hFollowObjNode, transform, LTTRUE);

		// Decompose the transform into the position and rotation
		pTransformLT->GetPos(transform, vObjPos);
	}

	// Turn the follow control off if the expire time has past
	if(pNodeControl->fFollowExpireTime <= 0.0f)
	{
		pNodeControl->fFollowExpireTime = 0.0f;
        pNodeControl->bFollowOn = LTFALSE;
	}
	else
        pNodeControl->fFollowExpireTime -= g_pGameClientShell->GetFrameTime();

	//----------------------------------------------------------------------
	// Setup the rotation matrix to directly follow the destination position

	LTRotation rNew;
	rNew.Identity();

	LTVector vDir;
	LTFLOAT fDot;
	LTFLOAT fAngle;

	//
	// Track on the XZ plane.
	//

	// Get the direction that we're going to face...
    vDir = vObjPos - vPos;
	vDir.y = 0.f;
	vDir.Normalize();	

	// Find angle between char's facing and target dir.

	fDot = vDir.Dot( vF );

	fAngle = (LTFLOAT)acos( fDot );
	fAngle = Min<LTFLOAT>(fAngle, 1.f);	

	// Look left or right.
	if ( vDir.Dot( vR ) > 0.0f )
	{
		rNew.Rotate( LTVector(0.f, 1.f, 0.f), fAngle);
	}
	else {
		rNew.Rotate( LTVector(0.f, 1.f, 0.f), -fAngle);
	}


	//
	// Track on the YZ plane.
	//

	LTVector vAxis;
	vAxis = rNew.Right();

	// Get the direction that we're going to face...
    vDir = vObjPos - vPos;
	vDir.x = 0.f;
	vDir.Normalize();	

	// Find angle between char's facing and target dir.

	fDot = vDir.Dot( vF );

	fAngle = (LTFLOAT)acos( fDot );
	fAngle = Min<LTFLOAT>(fAngle, 0.5f);	

	// look up or down.

	if ( vDir.Dot( vU ) < 0.0f )
	{
		rNew.Rotate( vAxis, fAngle);
	}
	else {
		rNew.Rotate( vAxis, -fAngle);
	}


	// Create a rotation matrix and apply it to the current offset matrix
    LTMatrix m1;
	rNew.ConvertToMatrix(m1);
	
	LTMatrix m2, invm2;
	rRot.ConvertToMatrix(m2);
	invm2 = m2;
	invm2.Inverse();

	m1 = invm2 * m1 * m2;

	m_aNodes[pNodeControl->eModelNode].matTransform = m1;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::HandleNodeControlHeadFollowObjMessage
//
//	PURPOSE:	Handle CFX_NODECONTROL_HEAD_FOLLOW_OBJ from the server
//
// ----------------------------------------------------------------------- //

void CNodeController::HandleNodeControlHeadFollowObjMessage(ILTMessage_Read *pMsg)
{
	HOBJECT hObj			= pMsg->ReadObject();
    HSTRING hFollowObjNode	= pMsg->ReadHString();
    LTFLOAT fTurnRate		= pMsg->Readfloat();
    LTFLOAT fExpireTime		= pMsg->Readfloat();
    LTBOOL bOn				= pMsg->Readuint8();

	ModelNode eModelNode = g_pModelButeMgr->GetSkeletonNode(m_pCharacterFX->GetModelSkeleton(), "Head");
	
	// See if we are already controlling the head node
	int iCtrl = FindNodeControl(eModelNode, eControlHeadFollowObj);

	char szFollowObjNode[64];
    strcpy(szFollowObjNode, g_pLTClient->GetStringData(hFollowObjNode));
    g_pLTClient->FreeString(hFollowObjNode);

	// Check to make sure all the info is ok...
	if((eModelNode == eModelNodeInvalid) || !hObj || (fTurnRate <= 0.0f))
	{
		if(iCtrl != INVALID_NODE_CONTROL)
            m_aNodeControls[iCtrl].bValid = LTFALSE;

		return;
	}

	// Add the node control structure...
	int iNodeControl = (iCtrl != INVALID_NODE_CONTROL) ? iCtrl : AddNodeControl();
	_ASSERT(iNodeControl != INVALID_NODE_CONTROL);
	if ( iNodeControl == INVALID_NODE_CONTROL )
		return;

    m_aNodeControls[iNodeControl].bValid = LTTRUE;
	m_aNodeControls[iNodeControl].eControl = eControlHeadFollowObj;
	m_aNodeControls[iNodeControl].eModelNode = eModelNode;

	m_aNodeControls[iNodeControl].hFollowObj = hObj;
	m_aNodeControls[iNodeControl].hFollowObjNode = INVALID_MODEL_NODE;
	m_aNodeControls[iNodeControl].fFollowRate = fTurnRate;
	m_aNodeControls[iNodeControl].fFollowExpireTime = fExpireTime;
	m_aNodeControls[iNodeControl].bFollowOn = bOn;

	// Find the node in the follow object...
	if(szFollowObjNode)
	{
		HMODELNODE hCurNode = INVALID_MODEL_NODE;

        while(g_pLTClient->GetModelLT()->GetNextNode(hObj, hCurNode, hCurNode) == LT_OK)
		{
			char szName[64] = "";
            g_pLTClient->GetModelLT()->GetNodeName(hObj, hCurNode, szName, 64);

			if(!strcmp(szName, szFollowObjNode))
			{
				m_aNodeControls[iNodeControl].hFollowObjNode = hCurNode;
				break;
			}
		}
	}

	// Increment the number of controllers for this node...
	m_aNodes[eModelNode].cControllers++;
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
		m_hSound = LTNULL;
	}

	if( m_bShowingSubtitles )
	{
 		g_pSubtitles->Clear();
		m_bShowingSubtitles = false;
	}

	ResetSoundBufferData();

	if( bSendNotification )
	{
		if( !m_pCharacterFX->GetServerObj( ))
		{
			ASSERT( !"CNodeController::KillLipSyncSound:  No characterfx object for nodecontroller." );
			return;
		}

		// Tell the server that the sound finished.
		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_OBJECT_MESSAGE );
		cMsg.WriteObject( m_pCharacterFX->GetServerObj( ));
		cMsg.Writeuint32( MID_SFX_MESSAGE );
		cMsg.Writeuint8( SFX_CHARACTER_ID );
		cMsg.Writeuint8( CFX_NODECONTROL_LIP_SYNC );
		cMsg.Writeuint32( m_nUniqueDialogueId );
		g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
	}
}