// ----------------------------------------------------------------------- //
//
// MODULE  : CNodeController.cpp
//
// PURPOSE : CNodeController implementation
//
// CREATED : 05.20.1999
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

VarTrack	g_vtLipSyncMaxRot;
VarTrack	g_vtLipSyncFreq;

//#define GRAPH_LIPSYNC_SOUND
#ifdef GRAPH_LIPSYNC_SOUND
#include "SFXMgr.h"
#include "GameClientShell.h"
#include "GraphFX.h"

class GraphPoints
{
public:

	GraphPoints()
		: data(0),
	      numPoints(0),
		  maxPoints(0)	{}

    void RecordData(int16 * pSixteenData,  uint32 dwNumElements, uint32 dwCurElement)
	{
		RecordData( pSixteenData, NULL, dwNumElements, dwCurElement);
	}

    void RecordData(int8 * pEightData, uint32 dwNumElements, uint32 dwCurElement)
	{
		RecordData( pEightData, dwNumElements, dwCurElement);
	}


    void RecordData(int16 * pSixteenData, int8 * pEightData, uint32 dwNumElements, uint32 dwCurElement)
	{
		const int points_to_display = 500;
		const int interval = 10;

		if( !data )
		{
			debug_deletea(data);
			maxPoints = points_to_display;
			data = new GraphFXPoint[points_to_display];
			ASSERT(data);
			if( !data ) return;
		}

		numPoints = 0;

		if( pEightData )
		{
			for( int8 * iter = pEightData + dwCurElement - points_to_display*interval/2, j = 0;
			     iter < pEightData + dwCurElement + points_to_display*interval/2 && iter < pEightData + dwNumElements;
				 iter += interval, ++j)
			{

                data[j] = GraphFXPoint( LTFLOAT(abs(*iter))/LTFLOAT(256/2) );

				++numPoints;
			}
		}
		else if( pSixteenData )
		{
			for( int16 * iter = pSixteenData + dwCurElement - points_to_display*interval/2, j = 0;
			     iter < pSixteenData + dwCurElement + points_to_display*interval/2 && iter < pSixteenData + dwNumElements;
				 iter += interval, ++j)
			{

                data[j] = GraphFXPoint( LTFLOAT(abs(*iter))/LTFLOAT(65536/2) );

				++numPoints;
			}
		}
	}

    LTBOOL GetData(GraphFXPoint * * PtrToData, uint32 * PtrToNumPoints)
	{
		*PtrToData = data;
		*PtrToNumPoints = numPoints;

		return data != NULL;
	}

private:


	GraphFXPoint * data;
    uint32   numPoints;
    uint32   maxPoints;

};


static GraphPoints g_GraphPoints;
#endif

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	HackRotationFn
//
//	PURPOSE:	Hack method of converting time to radians for rotations
//
// ----------------------------------------------------------------------- //

LTFLOAT HackRotationFn(LTFLOAT fDistance, LTFLOAT fTimer, LTFLOAT fDuration)
{
    LTFLOAT fAmount;

	if ( fTimer/fDuration > .25f )
	{
		// Linear decrease after 1/4 of duration

		fAmount = (-4.0f/3.0f)*fTimer/fDuration + 4.0f/3.0f;
	}
	else
	{
		// Linear increase until 1/4 of duration

		fAmount = (float)sqrt(4.0f*fTimer/fDuration);
	}

	return fDistance*fAmount;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::CNodeController
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CNodeController::CNodeController()
{
    m_pCharacterFX = LTNULL;

	m_cNodeControls = 0;
	m_cNodes = 0;

	m_cRecoils = 0;

	for ( int iRecoilTimer = 0 ; iRecoilTimer < MAX_RECOILS ; iRecoilTimer++ )
	{
		m_fRecoilTimers[iRecoilTimer] = 0.0f;
	}

	m_fCurLipSyncRot = 0.0f;
    m_bOpeningMouth  = LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::~CNodeController
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CNodeController::~CNodeController()
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::Init
//
//	PURPOSE:	Initializes us
//
// ----------------------------------------------------------------------- //

LTBOOL CNodeController::Init(CCharacterFX* pCharacterFX)
{
	_ASSERT(pCharacterFX);
    if ( !pCharacterFX ) return LTFALSE;

	if (!g_vtLipSyncMaxRot.IsInitted())
	{
        g_vtLipSyncMaxRot.Init(g_pLTClient, "LipSyncMaxRot", NULL, 25.0f);
	}

	if (!g_vtLipSyncFreq.IsInitted())
	{
        g_vtLipSyncFreq.Init(g_pLTClient, "LipSyncFreq", NULL, 20.0f);
	}

	// Store our backpointer

	m_pCharacterFX = pCharacterFX;

	// Map all the nodes in our skeleton in the bute file to the nodes in the actual .abc model file

	int iNode = 0;
	HMODELNODE hCurNode = INVALID_MODEL_NODE;
    while ( g_pLTClient->GetNextModelNode(GetCFX()->GetServerObj(), hCurNode, &hCurNode) == LT_OK)
	{
		_ASSERT(m_cNodes < MAX_NODES);

		char szName[64] = "";
        g_pLTClient->GetModelNodeName(GetCFX()->GetServerObj(), hCurNode, szName, 64);

		ModelNode eModelNode = g_pModelButeMgr->GetSkeletonNode(m_pCharacterFX->GetModelSkeleton(), szName);

		if ( eModelNode != eModelNodeInvalid )
		{
			m_aNodes[eModelNode].eModelNode = eModelNode;
			m_aNodes[eModelNode].hModelNode = hCurNode;
		}

		m_cNodes++;
	}

	// Find our "rotor" nodes

	int cNodes = g_pModelButeMgr->GetSkeletonNumNodes(m_pCharacterFX->GetModelSkeleton());

	for ( iNode = 0 ; iNode < cNodes ; iNode++ )
	{
		if ( NODEFLAG_ROTOR & g_pModelButeMgr->GetSkeletonNodeFlags(m_pCharacterFX->GetModelSkeleton(), (ModelNode)iNode) )
		{
            AddNodeControlRotationTimed((ModelNode)iNode, LTVector(0,1,0), 40000.0f, 20000.0f);
		}
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::Update
//
//	PURPOSE:	Updates all our Node Controls
//
// ----------------------------------------------------------------------- //

void CNodeController::Update()
{
	// Check to see if we are controlling any nodes, and en/dis-able the callback accordingly

	_ASSERT(m_cNodeControls >= 0);

	if ( m_cNodeControls > 0 )
	{
        g_pLTClient->ModelNodeControl(GetCFX()->GetServerObj(), CNodeController::NodeControlFn, this);
	}
	else
	{
        g_pLTClient->ModelNodeControl(GetCFX()->GetServerObj(), LTNULL, LTNULL);
	}

	// Reset all our nodes matrices

	for ( int iNode = 0 ; iNode < MAX_NODES ; iNode++ )
	{
		// Only reset the matrix if our control refcount is greater than zero (ie we are being controlled)

		if ( m_aNodes[iNode].cControllers > 0 )
		{
			m_aNodes[iNode].matTransform.Identity();
		}
	}

	// Update all our active node controls.

	int cValidNodeControls = 0;

	for ( int iNodeControl = 0 ; iNodeControl < MAX_NODECONTROLS && cValidNodeControls < m_cNodeControls ; iNodeControl++ )
	{
		if ( m_aNodeControls[iNodeControl].bValid )
		{
			// Update the node control

			cValidNodeControls++;

			switch ( m_aNodeControls[iNodeControl].eControl )
			{
				case eControlRotationTimed:
					UpdateRotationTimed(&m_aNodeControls[iNodeControl]);
					break;

				case eControlLipSync:
					UpdateLipSyncControl(&m_aNodeControls[iNodeControl]);
					break;

				case eControlScript:
					UpdateScriptControl(&m_aNodeControls[iNodeControl]);
					break;

				case eControlHeadFollowObj:
					UpdateHeadFollowObjControl(&m_aNodeControls[iNodeControl]);
					break;

				case eControlHeadFollowPos:
					UpdateHeadFollowPosControl(&m_aNodeControls[iNodeControl]);
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

	// Update all our recoil timers

	for ( int iRecoilTimer = 0 ; iRecoilTimer < m_cRecoils ; iRecoilTimer++ )
	{
        m_fRecoilTimers[iRecoilTimer] -= g_pGameClientShell->GetFrameTime();

		if ( m_fRecoilTimers[iRecoilTimer] <= 0.0f )
		{
			m_cRecoils--;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::UpdateRotationTimed
//
//	PURPOSE:	Updates a RotationTimed node control
//
// ----------------------------------------------------------------------- //

void CNodeController::UpdateRotationTimed(NCSTRUCT* pNodeControl)
{
    LTRotation rRot(0.0f, 0.0f, 0.0f, 1.0f);
	LTransform transform;
    LTMatrix matRotate;

//  g_pLTClient->CPrint("Rotating %s around <%f,%f,%f> by %f radians", g_pModelButeMgr->GetSkeletonNodeName(GetCFX()->GetModelSkeleton(), pNodeControl->eModelNode),
//		pNodeControl->vRotationAxis.x, pNodeControl->vRotationAxis.y, pNodeControl->vRotationAxis.z,
//		pNodeControl->fnRotationFunction(pNodeControl->fRotationDistance, pNodeControl->fRotationTimer, pNodeControl->fRotationDuration));

	// Rotate the node around that axis
	Mat_SetupRot(&matRotate, &pNodeControl->vRotationAxis,
		pNodeControl->fnRotationFunction(pNodeControl->fRotationDistance, pNodeControl->fRotationTimer, pNodeControl->fRotationDuration));

	// Apply the rotation to the node
	m_aNodes[pNodeControl->eModelNode].matTransform = matRotate * m_aNodes[pNodeControl->eModelNode].matTransform;

	// Update our timing info
    LTFLOAT fRotationTime = g_pGameClientShell->GetFrameTime();
	pNodeControl->fRotationTimer += fRotationTime;

	if ( pNodeControl->fRotationTimer >= pNodeControl->fRotationDuration )
	{
		// We're done

        pNodeControl->bValid = LTFALSE;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::UpdateSoundControl
//
//	PURPOSE:	Updates a sound node control
//
// ----------------------------------------------------------------------- //

void CNodeController::UpdateLipSyncControl(NCSTRUCT *pNodeControl)
{
	// Make sure the sound handle is valid and check to see if the sound is done...
    if(!pNodeControl->hLipSyncSound || g_pLTClient->IsDone(pNodeControl->hLipSyncSound))
	{
		g_pLTClient->KillSound(pNodeControl->hLipSyncSound);
        pNodeControl->hLipSyncSound = LTNULL;

		if (pNodeControl->bShowingSubtitles)
		{
 			g_pInterfaceMgr->ClearSubtitle();
		}

        pNodeControl->pSixteenBitBuffer = LTNULL;
        pNodeControl->pEightBitBuffer = LTNULL;
        pNodeControl->bValid = LTFALSE;
		return;
	}


	//
	// Process the current sound data (average over sound amplitude).
	//

    LTFLOAT fAverage = 0.0f;  // this will hold the average, normalized from 0.0f to 1.0f.

	// Get the sound buffer.
	if( !pNodeControl->pSixteenBitBuffer && !pNodeControl->pEightBitBuffer )
	{
        uint32 dwChannels = 0;

        g_pLTClient->GetSoundData(pNodeControl->hLipSyncSound,
							pNodeControl->pSixteenBitBuffer,
							pNodeControl->pEightBitBuffer,
							&pNodeControl->dwSamplesPerSecond,
							&dwChannels);
		ASSERT( dwChannels == 1);
		// If you want to use multi-channel sounds (why would you?), you'll need to
		//  to account for the interleaving of channels in the following code.
	}

	ASSERT( pNodeControl->pSixteenBitBuffer || pNodeControl->pEightBitBuffer );


	// Average over the data.  We do an average of the data from the current point
	// being played to 1/g_vtLipSyncFreq.GetFloat() seconds ahead of that point.
    uint32 dwOffset = 0;
    uint32 dwSize   = 0;

    if( LT_OK == g_pLTClient->GetSoundOffset(pNodeControl->hLipSyncSound, &dwOffset, &dwSize) )
	{
		// Determine the end of the data we wish to average over.
        const uint32 dwDivisor = uint32(g_vtLipSyncFreq.GetFloat());
        uint32 dwOffsetEnd = dwOffset + pNodeControl->dwSamplesPerSecond/dwDivisor;
		if( dwOffsetEnd > dwSize )
			dwOffsetEnd = dwSize;


		// Accumulate the the amplitudes for the average.
        uint32 dwMaxAmplitude = 0;

        uint32 dwNumSamples = 0;
        uint32 dwAccum = 0;

		if( pNodeControl->pSixteenBitBuffer )
		{
			for( int16 * pIterator = pNodeControl->pSixteenBitBuffer + dwOffset;
			     pIterator < pNodeControl->pSixteenBitBuffer + dwOffsetEnd;
				 ++pIterator)
			{
				dwAccum += abs(*pIterator);
				++dwNumSamples;
			}

			dwMaxAmplitude = 65536/2;

			#ifdef GRAPH_LIPSYNC_SOUND
				g_GraphPoints.RecordData(pNodeControl->pSixteenBitBuffer,dwSize,dwOffset);
			#endif

		}
		else if( pNodeControl->pEightBitBuffer )
		{
			for( int8 * pIterator = pNodeControl->pEightBitBuffer + dwOffset;
			     pIterator < pNodeControl->pEightBitBuffer + dwOffsetEnd;
				 ++pIterator)
			{
				dwAccum += abs(*pIterator);
				++dwNumSamples;
			}

			dwMaxAmplitude = 256/2;

			#ifdef GRAPH_LIPSYNC_SOUND
				g_GraphPoints.RecordData(pNodeControl->pEightBitBuffer,dwSize,dwOffset);
			#endif
		}

		// And find the average!
		if( dwNumSamples > 0 )
            fAverage = LTFLOAT(dwAccum) / LTFLOAT(dwNumSamples) / LTFLOAT(dwMaxAmplitude);

    } //if( LT_OK == g_pLTClient->GetSoundOffset(pNodeControl->hLipSyncSound, &dwOffset, &dwSize) )

	//
	// Do the rotation.
	//
    ILTMath *pMathLT = g_pLTClient->GetMathLT();

    LTRotation rRot;
    rRot.Init();
    LTVector vAxis(0.0f, 0.0f, 1.0f);
    LTFLOAT fMaxRot = MATH_DEGREES_TO_RADIANS(g_vtLipSyncMaxRot.GetFloat());

	// Calculate the rotation.
	m_fCurLipSyncRot =  fAverage*fMaxRot;
	pMathLT->RotateAroundAxis(rRot, vAxis, -m_fCurLipSyncRot);

	// Create a rotation matrix and apply it to the current offset matrix
    LTMatrix m1;
	pMathLT->SetupRotationMatrix(m1, rRot);
	m_aNodes[pNodeControl->eModelNode].matTransform = m_aNodes[pNodeControl->eModelNode].matTransform * m1;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::UpdateScriptControl
//
//	PURPOSE:	Updates a script node control
//
// ----------------------------------------------------------------------- //

void CNodeController::UpdateScriptControl(NCSTRUCT *pNodeControl)
{
    LTFLOAT fTime = g_pLTClient->GetTime() - pNodeControl->fScriptTime;
	ModelNScript eScript = (ModelNScript)pNodeControl->nScript;
	int nCurPt = pNodeControl->nCurrentScriptPt;
	int nLastPt = pNodeControl->nLastScriptPt;

//  g_pLTClient->CPrint("Running node script!");

	// Check to see if the script is over... and make it invalid if so
	if(fTime > g_pModelButeMgr->GetNScriptPtTime(eScript, nLastPt))
	{
//      g_pLTClient->CPrint("Ending node script!");

        pNodeControl->bValid = LTFALSE;
		return;
	}

	// Get access to the controls...
    ILTMath *pMathLT = g_pLTClient->GetMathLT();

	// Get the current script point
	while(fTime > g_pModelButeMgr->GetNScriptPtTime(eScript, nCurPt))
		nCurPt++;

	// Calculate the scale value from the last position to the current
    LTFLOAT fPtTime1 = g_pModelButeMgr->GetNScriptPtTime(eScript, nCurPt - 1);
    LTFLOAT fPtTime2 = g_pModelButeMgr->GetNScriptPtTime(eScript, nCurPt);
    LTFLOAT fScale = (fTime - fPtTime1) / (fPtTime2 - fPtTime1);

	//----------------------------------------------------------------------------
	// Setup the initial position vector
    LTVector vPos;

	// Get a direction vector from the last position to the current
    LTVector vPosDir = g_pModelButeMgr->GetNScriptPtPosOffset(eScript, nCurPt) - g_pModelButeMgr->GetNScriptPtPosOffset(eScript, nCurPt - 1);
	VEC_MULSCALAR(vPosDir, vPosDir, fScale);
	vPos = g_pModelButeMgr->GetNScriptPtPosOffset(eScript, nCurPt - 1) + vPosDir;

	//----------------------------------------------------------------------------
	// Setup the initial rotation vector
    LTVector vRot;

	// Get a direction rotation from the last position to the current
    LTVector vRotDir = g_pModelButeMgr->GetNScriptPtRotOffset(eScript, nCurPt) - g_pModelButeMgr->GetNScriptPtRotOffset(eScript, nCurPt - 1);
	VEC_MULSCALAR(vRotDir, vRotDir, fScale);
	vRot = g_pModelButeMgr->GetNScriptPtRotOffset(eScript, nCurPt - 1) + vRotDir;

	// Calculate the rotation...
    LTRotation rRot;
    LTVector vR, vU, vF;

    rRot.Init();
	pMathLT->GetRotationVectors(rRot, vR, vU, vF);

	pMathLT->RotateAroundAxis(rRot, vR, vRot.x);
	pMathLT->RotateAroundAxis(rRot, vU, vRot.y);
	pMathLT->RotateAroundAxis(rRot, vF, vRot.z);

	//----------------------------------------------------------------------------
	// Setup the matrix using the offset position and rotation
    LTMatrix m1;
	pMathLT->SetupTransformationMatrix(m1, vPos, rRot);
	m_aNodes[pNodeControl->eModelNode].matTransform = m_aNodes[pNodeControl->eModelNode].matTransform * m1;
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

	//----------------------------------------------------------------------
	// Get information about the control node...
	// *** NOTE: On the head node... vU faces forward, vR faces down, vF faces right ***

	// Get access to the controls...
    ILTMath *pMathLT = g_pLTClient->GetMathLT();
    ILTModel *pModelLT = g_pLTClient->GetModelLT();
    ILTTransform *pTransformLT = g_pLTClient->GetTransformLT();

	// Get the transform of the node we're controlling
    pModelLT->GetNodeTransform(GetCFX()->GetServerObj(), m_aNodes[pNodeControl->eModelNode].hModelNode, transform, LTTRUE);

	// Decompose the transform into the position and rotation
	pTransformLT->Get(transform, vPos, rRot);
	pMathLT->GetRotationVectors(rRot, vR, vU, vF);

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

	// Get the direction that we're going to face...
    LTVector vDir = vObjPos - vPos;

	// Setup some temp vectors that are on the x/z plane...
    LTVector vTempU, vTempF, vTempDir;
	vTempU = vU; vTempU.y = 0.0f;
	vTempF = vF; vTempF.y = 0.0f;
	vTempDir = vDir; vTempDir.y = 0.0f;

	VEC_NORM(vTempU);
	VEC_NORM(vTempF);
	VEC_NORM(vTempDir);

	// Get the dot products between the dir vector and the up and forward to determine the rotation angles
    LTFLOAT fDotUDir = VEC_DOT(vTempU, vTempDir);
    LTFLOAT fDotFDir = VEC_DOT(vTempF, vTempDir);
    LTFLOAT fDotRDir = 0.0f;

	// Init the vectors to get a rotation matrix from...
    LTVector vRotAxisR(1.0f, 0.0f, 0.0f);

	// Get the first rotation angle
    LTFLOAT fAngle1 = pNodeControl->bFollowOn ? fDotUDir : 1.0f;
	if(fAngle1 < -0.1f) fAngle1 = -0.1f;		// HACK! Limit the head rotation
	fAngle1 = (1.0f - fAngle1) * MATH_HALFPI;
	if(fDotFDir < 0.0f) fAngle1 *= -1.0f;

	// Do a full rotation around the first axis so we can get an angle for the second axis
    LTFLOAT fTempAngle = pNodeControl->bFollowOn ? ((1.0f - fDotUDir) * MATH_HALFPI) : 0.0f;
	pMathLT->RotateAroundAxis(rRot, vR, (fDotFDir < 0.0f) ? -fTempAngle : fTempAngle);
	pMathLT->GetRotationVectors(rRot, vR, vU, vF);

	VEC_NORM(vDir);
	fDotUDir = VEC_DOT(vU, vDir);
	fDotRDir = VEC_DOT(vR, vDir);

	// Get the second rotation angle
    LTFLOAT fAngle2 = pNodeControl->bFollowOn ? fDotUDir : 1.0f;
	if(fAngle2 < 0.25f) fAngle2 = 0.25f;		// HACK! Limit the head rotation
	fAngle2 = (1.0f - fAngle2) * MATH_HALFPI;
	if(fDotRDir > 0.0f) fAngle2 *= -1.0f;

	// Calculate a max rotation value
    LTFLOAT fRotMax = (pNodeControl->fFollowRate * g_pGameClientShell->GetFrameTime() / 180.0f) * MATH_PI;

	// Interpolate the angles based off the previous angle
	if(fAngle1 > pNodeControl->vFollowAngles.y + fRotMax) fAngle1 = pNodeControl->vFollowAngles.y + fRotMax;
	else if(fAngle1 < pNodeControl->vFollowAngles.y - fRotMax) fAngle1 = pNodeControl->vFollowAngles.y - fRotMax;

	if(fAngle2 > pNodeControl->vFollowAngles.x + fRotMax) fAngle2 = pNodeControl->vFollowAngles.x + fRotMax;
	else if(fAngle2 < pNodeControl->vFollowAngles.x - fRotMax) fAngle2 = pNodeControl->vFollowAngles.x - fRotMax;

	// Create a new rotation and rotate around each controlled axis
    LTRotation rNewRot;
    rNewRot.Init();

	pMathLT->RotateAroundAxis(rNewRot, vRotAxisR, fAngle1);
	pNodeControl->vFollowAngles.y = fAngle1;

	pMathLT->GetRotationVectors(rNewRot, vR, vU, vF);

	pMathLT->RotateAroundAxis(rNewRot, vF, fAngle2);
	pNodeControl->vFollowAngles.x = fAngle2;

	// If we're turned off and back at the start rotation... make the control invalid
	if(!pNodeControl->bFollowOn && pNodeControl->vFollowAngles.x == 0.0f && pNodeControl->vFollowAngles.y == 0.0f)
	{
        pNodeControl->bValid = LTFALSE;
		return;
	}

	// Create a rotation matrix and apply it to the current offset matrix
    LTMatrix m1;
	pMathLT->SetupRotationMatrix(m1, rNewRot);
	m_aNodes[pNodeControl->eModelNode].matTransform = m_aNodes[pNodeControl->eModelNode].matTransform * m1;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::UpdateHeadFollowPosControl
//
//	PURPOSE:	Updates a follow position node control
//
// ----------------------------------------------------------------------- //

void CNodeController::UpdateHeadFollowPosControl(NCSTRUCT *pNodeControl)
{
    LTVector vPos;
    LTRotation rRot;
	LTransform transform;
    LTVector vU, vR, vF;

	//----------------------------------------------------------------------
	// Get information about the control node...
	// *** NOTE: On the head node... vU faces forward, vR faces down, vF faces right ***

	// Get access to the controls...
    ILTMath *pMathLT = g_pLTClient->GetMathLT();
    ILTModel *pModelLT = g_pLTClient->GetModelLT();
    ILTTransform *pTransformLT = g_pLTClient->GetTransformLT();

	// Get the transform of the node we're controlling
    pModelLT->GetNodeTransform(GetCFX()->GetServerObj(), m_aNodes[pNodeControl->eModelNode].hModelNode, transform, LTTRUE);

	// Decompose the transform into the position and rotation
	pTransformLT->Get(transform, vPos, rRot);
	pMathLT->GetRotationVectors(rRot, vR, vU, vF);

	// Get information about the follow position...
    LTVector vObjPos = pNodeControl->vFollowPos;

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

	// Get the direction that we're going to face...
    LTVector vDir = vObjPos - vPos;

	// Setup some temp vectors that are on the x/z plane...
    LTVector vTempU, vTempF, vTempDir;
	vTempU = vU; vTempU.y = 0.0f;
	vTempF = vF; vTempF.y = 0.0f;
	vTempDir = vDir; vTempDir.y = 0.0f;

	VEC_NORM(vTempU);
	VEC_NORM(vTempF);
	VEC_NORM(vTempDir);

	// Get the dot products between the dir vector and the up and forward to determine the rotation angles
    LTFLOAT fDotUDir = VEC_DOT(vTempU, vTempDir);
    LTFLOAT fDotFDir = VEC_DOT(vTempF, vTempDir);
    LTFLOAT fDotRDir = 0.0f;

	// Init the vectors to get a rotation matrix from...
    LTVector vRotAxisR(1.0f, 0.0f, 0.0f);

	// Get the first rotation angle
    LTFLOAT fAngle1 = pNodeControl->bFollowOn ? fDotUDir : 1.0f;
	if(fAngle1 < -0.1f) fAngle1 = -0.1f;		// HACK! Limit the head rotation
	fAngle1 = (1.0f - fAngle1) * MATH_HALFPI;
	if(fDotFDir < 0.0f) fAngle1 *= -1.0f;

	// Do a full rotation around the first axis so we can get an angle for the second axis
    LTFLOAT fTempAngle = pNodeControl->bFollowOn ? ((1.0f - fDotUDir) * MATH_HALFPI) : 0.0f;
	pMathLT->RotateAroundAxis(rRot, vR, (fDotFDir < 0.0f) ? -fTempAngle : fTempAngle);
	pMathLT->GetRotationVectors(rRot, vR, vU, vF);

	VEC_NORM(vDir);
	fDotUDir = VEC_DOT(vU, vDir);
	fDotRDir = VEC_DOT(vR, vDir);

	// Get the second rotation angle
    LTFLOAT fAngle2 = pNodeControl->bFollowOn ? fDotUDir : 1.0f;
	if(fAngle2 < 0.25f) fAngle2 = 0.25f;		// HACK! Limit the head rotation
	fAngle2 = (1.0f - fAngle2) * MATH_HALFPI;
	if(fDotRDir > 0.0f) fAngle2 *= -1.0f;

	// Calculate a max rotation value
    LTFLOAT fRotMax = (pNodeControl->fFollowRate * g_pGameClientShell->GetFrameTime() / 180.0f) * MATH_PI;

	// Interpolate the angles based off the previous angle
	if(fAngle1 > pNodeControl->vFollowAngles.y + fRotMax) fAngle1 = pNodeControl->vFollowAngles.y + fRotMax;
	else if(fAngle1 < pNodeControl->vFollowAngles.y - fRotMax) fAngle1 = pNodeControl->vFollowAngles.y - fRotMax;

	if(fAngle2 > pNodeControl->vFollowAngles.x + fRotMax) fAngle2 = pNodeControl->vFollowAngles.x + fRotMax;
	else if(fAngle2 < pNodeControl->vFollowAngles.x - fRotMax) fAngle2 = pNodeControl->vFollowAngles.x - fRotMax;

	// Create a new rotation and rotate around each controlled axis
    LTRotation rNewRot;
    rNewRot.Init();

	pMathLT->RotateAroundAxis(rNewRot, vRotAxisR, fAngle1);
	pNodeControl->vFollowAngles.y = fAngle1;

	pMathLT->GetRotationVectors(rNewRot, vR, vU, vF);

	pMathLT->RotateAroundAxis(rNewRot, vF, fAngle2);
	pNodeControl->vFollowAngles.x = fAngle2;

	// If we're turned off and back at the start rotation... make the control invalid
	if(!pNodeControl->bFollowOn && pNodeControl->vFollowAngles.x == 0.0f && pNodeControl->vFollowAngles.y == 0.0f)
	{
        pNodeControl->bValid = LTFALSE;
		return;
	}

	// Create a rotation matrix and apply it to the current offset matrix
    LTMatrix m1;
	pMathLT->SetupRotationMatrix(m1, rNewRot);
	m_aNodes[pNodeControl->eModelNode].matTransform = m_aNodes[pNodeControl->eModelNode].matTransform * m1;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::HandleNodeControlMessage
//
//	PURPOSE:	Handle CFX_NODECONTROL_*_MSG from the server
//
// ----------------------------------------------------------------------- //

LTBOOL CNodeController::HandleNodeControlMessage(uint8 byMsgId, HMESSAGEREAD hMessage)
{
	// Handle it...

	switch ( byMsgId )
	{
		case CFX_NODECONTROL_LIP_SYNC:
		{
			HandleNodeControlLipSyncMessage(hMessage);
			break;
		}

		case CFX_NODECONTROL_HEAD_FOLLOW_OBJ:
		{
			HandleNodeControlHeadFollowObjMessage(hMessage);
			break;
		}

		case CFX_NODECONTROL_HEAD_FOLLOW_POS:
		{
			HandleNodeControlHeadFollowPosMessage(hMessage);
			break;
		}

		case CFX_NODECONTROL_SCRIPT:
		{
			HandleNodeControlScriptMessage(hMessage);
			break;
		}
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::HandleNodeControlRecoilMessage
//
//	PURPOSE:	Handle CFX_NODECONTROL_RECOIL_MSG from the server
//
// ----------------------------------------------------------------------- //

void CNodeController::HandleNodeControlRecoilMessage(HMESSAGEREAD hMessage)
{
	if ( m_cRecoils >= MAX_RECOILS )
		return;

	m_fRecoilTimers[m_cRecoils++] = 0.50f;


	ModelNode eModelNode;
    eModelNode = (ModelNode)g_pLTClient->ReadFromMessageByte(hMessage);

    LTVector vRecoilDir;
    g_pLTClient->ReadFromMessageCompVector(hMessage, &vRecoilDir);

	// Get the magnitude of the recoil vector

    LTFLOAT fRecoilMag = VEC_MAGSQR(vRecoilDir);

	// Get the unit impact/recoil vector

	vRecoilDir /= (float)sqrt(fRecoilMag);

	// Cap it if necessary

	if ( fRecoilMag > 100.0f )
	{
		fRecoilMag = 100.0f;
	}

	// Get the position of the impact

	NSTRUCT* pNode = &m_aNodes[eModelNode];
    ILTModel* pModelLT = g_pLTClient->GetModelLT();
	LTransform transform;
    pModelLT->GetNodeTransform(GetCFX()->GetServerObj(), pNode->hModelNode, transform, LTTRUE);

	// Decompose the transform into the position and rotation

    LTVector vPos;
    ILTTransform* pTransformLT = g_pLTClient->GetTransformLT();
	pTransformLT->GetPos(transform, vPos);

    LTVector vRecoilPos = vPos;

	// Add angular rotations up the recoil parent chain

	ModelNode eModelNodeCurrent = g_pModelButeMgr->GetSkeletonNodeRecoilParent(GetCFX()->GetModelSkeleton(), eModelNode);

	while ( eModelNodeCurrent != eModelNodeInvalid )
	{
		// Get the rotation of the node

		NSTRUCT* pNode = &m_aNodes[eModelNodeCurrent];

		LTransform transform;
        ILTModel* pModelLT = g_pLTClient->GetModelLT();

		// Get the transform of the node we're controlling

        pModelLT->GetNodeTransform(GetCFX()->GetServerObj(), pNode->hModelNode, transform, LTTRUE);

        ILTTransform* pTransformLT = g_pLTClient->GetTransformLT();

		// Decompose the transform into the position and rotation

        LTVector vPos;
        LTRotation rRot;
		pTransformLT->Get(transform, vPos, rRot);

		// Get the rotation vectors of the transform

        LTVector vRight, vUp, vForward;
        g_pLTClient->GetRotationVectors(&rRot, &vUp, &vRight, &vForward);

		// Cross the right vector with the impact vector to get swing

        LTVector vRotationAxis = vRight.Cross(vRecoilDir);
		vRotationAxis.Norm();

		// Add the timed rotation control for the swing

		// !!! HACK
		// !!! Do not add swing if this is a leg node

		if ( !strstr(g_pModelButeMgr->GetSkeletonNodeName(GetCFX()->GetModelSkeleton(), eModelNodeCurrent), "leg") )
		AddNodeControlRotationTimed(eModelNodeCurrent, vRotationAxis, MATH_PI/1000.0f*fRecoilMag, 0.50f);

		// Use the right vector to get twist, but make sure the sign is correct based on location
		// of impact and whether we're getting shot at from behind/front etc

		vRotationAxis = vRight;
		vRotationAxis.Norm();

		// Get the twist

        LTVector vSideDir = vRecoilPos-vPos;
		vSideDir.Norm();

        LTFLOAT fSign = vUp.Dot(vRecoilDir);
		fSign *= vForward.Dot(vSideDir);

		if ( fSign > 0.0f )
		{
			vRotationAxis = -vRotationAxis;
		}

		// Add the timed rotation control for the twist

	//	AddNodeControlRotationTimed(eModelNodeCurrent, vRotationAxis, MATH_PI/1000.0f*fRecoilMag, 0.50f);

		// Decrease the magnitude

		fRecoilMag /= 2.0f;

		eModelNodeCurrent = g_pModelButeMgr->GetSkeletonNodeRecoilParent(GetCFX()->GetModelSkeleton(), eModelNodeCurrent);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::HandleNodeControlLipSyncMessage
//
//	PURPOSE:	Handle CFX_NODECONTROL_LIP_SYNC from the server
//
// ----------------------------------------------------------------------- //

void CNodeController::HandleNodeControlLipSyncMessage(HMESSAGEREAD hMessage)
{
    HSTRING hSound = g_pLTClient->ReadFromMessageHString(hMessage);
    LTFLOAT fRadius = g_pLTClient->ReadFromMessageFloat(hMessage);

	HandleNodeConrolLipSync(hSound, fRadius);
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::HandleNodeControlLipSyncMessage
//
//	PURPOSE:	Handle CFX_NODECONTROL_LIP_SYNC from the server
//
// ----------------------------------------------------------------------- //

void CNodeController::HandleNodeConrolLipSync(HSTRING hSound, LTFLOAT fRadius)
{
 	ModelNode eModelNode = g_pModelButeMgr->GetSkeletonNode(m_pCharacterFX->GetModelSkeleton(), "LowerJaw");

	// See if we are already controlling the jaw node
	int iCtrl = FindNodeControl(eModelNode, eControlLipSync);

	char szSound[128];
    strcpy(szSound, g_pLTClient->GetStringData(hSound));
    g_pLTClient->FreeString(hSound);

	// Check to make sure all the info is ok...
	if((eModelNode == eModelNodeInvalid) || fRadius <= 0.0f)
	{
		if(iCtrl > -1)
		{
			if(m_aNodeControls[iCtrl].hLipSyncSound)
			{
                g_pLTClient->KillSound(m_aNodeControls[iCtrl].hLipSyncSound);
                m_aNodeControls[iCtrl].hLipSyncSound = LTNULL;

				if (m_aNodeControls[iCtrl].bShowingSubtitles)
				{
					g_pInterfaceMgr->ClearSubtitle();
				}
			}

            m_aNodeControls[iCtrl].bValid = LTFALSE;
		}

		return;
	}

	// Add the node control structure...
	int iNodeControl = (iCtrl > -1) ? iCtrl : AddNodeControl();

	_ASSERT(iNodeControl >= 0);
	if ( iNodeControl < 0 )
		return;

	if(m_aNodeControls[iNodeControl].hLipSyncSound)
	{
        g_pLTClient->KillSound(m_aNodeControls[iNodeControl].hLipSyncSound);
        m_aNodeControls[iNodeControl].hLipSyncSound = LTNULL;

		if (m_aNodeControls[iNodeControl].bShowingSubtitles)
		{
			g_pInterfaceMgr->ClearSubtitle();
		}
	}

    m_aNodeControls[iNodeControl].bValid = LTTRUE;
	m_aNodeControls[iNodeControl].eControl = eControlLipSync;
	m_aNodeControls[iNodeControl].eModelNode = eModelNode;
    m_aNodeControls[iNodeControl].pSixteenBitBuffer = LTNULL;
    m_aNodeControls[iNodeControl].pEightBitBuffer = LTNULL;

	LTBOOL bSubtitles = LTFALSE;
	m_aNodeControls[iNodeControl].hLipSyncSound = m_pCharacterFX->PlayLipSyncSound(szSound, fRadius, bSubtitles);
	m_aNodeControls[iNodeControl].bShowingSubtitles = bSubtitles;

	// Increment the number of controllers for this node...
	m_aNodes[eModelNode].cControllers++;

	// PLH DEBUG
	// Show graph over character.
#ifdef GRAPH_LIPSYNC_SOUND
	GCREATESTRUCT graph_init;
	graph_init.hServerObj = m_pCharacterFX->GetServerObj();
    graph_init.m_vOffsetPos = LTVector(0.0f,70.0f,0.0f);
	graph_init.m_fWidth = 60.0f;
	graph_init.m_fHeight = 60.0f;
    graph_init.m_bIgnoreX = LTTRUE;
	graph_init.m_UpdateGraphCallback
        = make_callback2<LTBOOL,GraphFXPoint * *, uint32 *>(g_GraphPoints,GraphPoints::GetData);

	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	psfxMgr->CreateSFX(SFX_GRAPH_ID, &graph_init);
#endif

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::HandleNodeControlScriptMessage
//
//	PURPOSE:	Handle CFX_NODECONTROL_SCRIPT from the server
//
// ----------------------------------------------------------------------- //

void CNodeController::HandleNodeControlScriptMessage(HMESSAGEREAD hMessage)
{
    uint8 nNumScripts = g_pLTClient->ReadFromMessageByte(hMessage);

    LTFLOAT fTime = g_pLTClient->GetTime();

    for(uint8 i = 0; i < nNumScripts; i++)
	{
        uint8 nScript = g_pLTClient->ReadFromMessageByte(hMessage);
		ModelNode eModelNode = g_pModelButeMgr->GetSkeletonNode(m_pCharacterFX->GetModelSkeleton(), g_pModelButeMgr->GetNScriptNodeName((ModelNScript)nScript));

		// Add the node control structure...
		int iNodeControl = AddNodeControl();

		_ASSERT(iNodeControl >= 0);
		if ( iNodeControl < 0 || g_pModelButeMgr->GetNScriptNumPts((ModelNScript)nScript) < 2)
			continue;

        m_aNodeControls[iNodeControl].bValid = LTTRUE;
		m_aNodeControls[iNodeControl].eControl = eControlScript;
		m_aNodeControls[iNodeControl].eModelNode = eModelNode;

		m_aNodeControls[iNodeControl].nScript = nScript;
		m_aNodeControls[iNodeControl].fScriptTime = fTime;
		m_aNodeControls[iNodeControl].nCurrentScriptPt = 0;
		m_aNodeControls[iNodeControl].nLastScriptPt = g_pModelButeMgr->GetNScriptNumPts((ModelNScript)nScript) - 1;

		// Increment the number of controllers for this node...
		m_aNodes[eModelNode].cControllers++;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::HandleNodeControlHeadFollowObjMessage
//
//	PURPOSE:	Handle CFX_NODECONTROL_HEAD_FOLLOW_OBJ from the server
//
// ----------------------------------------------------------------------- //

void CNodeController::HandleNodeControlHeadFollowObjMessage(HMESSAGEREAD hMessage)
{
//  ModelNode eModelNode = (ModelNode)g_pLTClient->ReadFromMessageByte(hMessage);
    HOBJECT hObj = g_pLTClient->ReadFromMessageObject(hMessage);
    HSTRING hFollowObjNode = g_pLTClient->ReadFromMessageHString(hMessage);
    LTFLOAT fTurnRate = g_pLTClient->ReadFromMessageFloat(hMessage);
    LTFLOAT fExpireTime = g_pLTClient->ReadFromMessageFloat(hMessage);
    LTBOOL bOn = g_pLTClient->ReadFromMessageByte(hMessage);

	ModelNode eModelNode = g_pModelButeMgr->GetSkeletonNode(m_pCharacterFX->GetModelSkeleton(), "Head_node");

	// See if we are already controlling the head node
	int iCtrl = FindNodeControl(eModelNode, eControlHeadFollowObj);
	if(iCtrl == -1) iCtrl = FindNodeControl(eModelNode, eControlHeadFollowPos);

	char szFollowObjNode[64];
    strcpy(szFollowObjNode, g_pLTClient->GetStringData(hFollowObjNode));
    g_pLTClient->FreeString(hFollowObjNode);

	// Check to make sure all the info is ok...
	if((eModelNode == eModelNodeInvalid) || !hObj || (fTurnRate <= 0.0f))
	{
		if(iCtrl > -1)
            m_aNodeControls[iCtrl].bValid = LTFALSE;

		return;
	}

	// Add the node control structure...
	int iNodeControl = (iCtrl > -1) ? iCtrl : AddNodeControl();
	_ASSERT(iNodeControl >= 0);
	if ( iNodeControl < 0 )
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

        while(g_pLTClient->GetNextModelNode(hObj, hCurNode, &hCurNode) == LT_OK)
		{
			char szName[64] = "";
            g_pLTClient->GetModelNodeName(hObj, hCurNode, szName, 64);

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
//	ROUTINE:	CNodeController::HandleNodeControlHeadFollowPosMessage
//
//	PURPOSE:	Handle CFX_NODECONTROL_HEAD_FOLLOW_POS from the server
//
// ----------------------------------------------------------------------- //

void CNodeController::HandleNodeControlHeadFollowPosMessage(HMESSAGEREAD hMessage)
{
    LTVector vPos;

//  ModelNode eModelNode = (ModelNode)g_pLTClient->ReadFromMessageByte(hMessage);
    g_pLTClient->ReadFromMessageVector(hMessage, &vPos);
    LTFLOAT fTurnRate = g_pLTClient->ReadFromMessageFloat(hMessage);
    LTFLOAT fExpireTime = g_pLTClient->ReadFromMessageFloat(hMessage);
    LTBOOL bOn = g_pLTClient->ReadFromMessageByte(hMessage);

	ModelNode eModelNode = g_pModelButeMgr->GetSkeletonNode(m_pCharacterFX->GetModelSkeleton(), "Head_node");

	// See if we are already controlling the head node
	int iCtrl = FindNodeControl(eModelNode, eControlHeadFollowObj);
	if(iCtrl == -1) iCtrl = FindNodeControl(eModelNode, eControlHeadFollowPos);

	// Check to make sure all the info is ok...
	if((eModelNode == eModelNodeInvalid) || (fTurnRate <= 0.0f))
	{
		if(iCtrl > -1)
            m_aNodeControls[iCtrl].bValid = LTFALSE;

		return;
	}

	// Add the node control structure...
	int iNodeControl = (iCtrl > -1) ? iCtrl : AddNodeControl();
	_ASSERT(iNodeControl >= 0);
	if ( iNodeControl < 0 )
		return;

    m_aNodeControls[iNodeControl].bValid = LTTRUE;
	m_aNodeControls[iNodeControl].eControl = eControlHeadFollowPos;
	m_aNodeControls[iNodeControl].eModelNode = eModelNode;

	m_aNodeControls[iNodeControl].vFollowPos = vPos;
	m_aNodeControls[iNodeControl].fFollowRate = fTurnRate;
	m_aNodeControls[iNodeControl].fFollowExpireTime = fExpireTime;
	m_aNodeControls[iNodeControl].bFollowOn = bOn;

	// Increment the number of controllers for this node...
	m_aNodes[eModelNode].cControllers++;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::FindNode
//
//	PURPOSE:	Finds the node struct given a node handle
//
// ----------------------------------------------------------------------- //

NSTRUCT* CNodeController::FindNode(HMODELNODE hModelNode)
{
	for ( int iNode = 0 ; iNode < MAX_NODES ; iNode++ )
	{
		if ( m_aNodes[iNode].hModelNode == hModelNode )
		{
			return &m_aNodes[iNode];
		}
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::FindNodeControl
//
//	PURPOSE:	Finds the node control struct given a node id
//
// ----------------------------------------------------------------------- //

int CNodeController::FindNodeControl(ModelNode eNode, Control eControl)
{
	for(int i = 0; i < MAX_NODECONTROLS; i++)
	{
		if(m_aNodeControls[i].bValid && m_aNodeControls[i].eModelNode == eNode && m_aNodeControls[i].eControl == eControl)
			return i;
	}

	return -1;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::AddNodeControl
//
//	PURPOSE:	Adds a node control (returns the index of a free one)
//
// ----------------------------------------------------------------------- //

int CNodeController::AddNodeControl()
{
	for ( int iNodeControl = 0 ; iNodeControl < MAX_NODECONTROLS ; iNodeControl++ )
	{
		if ( !m_aNodeControls[iNodeControl].bValid )
		{
			m_cNodeControls++;
			return iNodeControl;
		}
	}

	return -1;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::AddNodeControlRotationTimed
//
//	PURPOSE:	Adds a timed rotation acceleration node control
//
// ----------------------------------------------------------------------- //

void CNodeController::AddNodeControlRotationTimed(ModelNode eModelNode, const LTVector& vAxis, LTFLOAT fDistance, LTFLOAT fDuration)
{
	int iNodeControl = AddNodeControl();
	_ASSERT(iNodeControl >= 0);
	if ( iNodeControl < 0 ) return;

    m_aNodeControls[iNodeControl].bValid = LTTRUE;
	m_aNodeControls[iNodeControl].eControl = eControlRotationTimed;
	m_aNodeControls[iNodeControl].eModelNode = eModelNode;

	m_aNodeControls[iNodeControl].vRotationAxis = vAxis;
	m_aNodeControls[iNodeControl].fRotationDistance = fDistance;
	m_aNodeControls[iNodeControl].fRotationDuration = fDuration;
	m_aNodeControls[iNodeControl].fRotationTimer = 0.0f;
	m_aNodeControls[iNodeControl].fnRotationFunction = HackRotationFn;

	m_aNodes[eModelNode].cControllers++;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::RemoveNodeControl
//
//	PURPOSE:	Removes a node control
//
// ----------------------------------------------------------------------- //

void CNodeController::RemoveNodeControl(int iNodeControl)
{
	_ASSERT(!m_aNodeControls[iNodeControl].bValid);

	m_aNodes[m_aNodeControls[iNodeControl].eModelNode].cControllers--;

	m_cNodeControls--;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::NodeControlFn
//
//	PURPOSE:	Node control callback
//
// ----------------------------------------------------------------------- //

void CNodeController::NodeControlFn(HOBJECT hObj, HMODELNODE hNode, LTMatrix *pGlobalMat, void *pUserData)
{
	CNodeController* pNC = (CNodeController*)pUserData;
	_ASSERT(pNC);
	if ( !pNC ) return;

	pNC->HandleNodeControl(hObj, hNode, pGlobalMat);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeController::HandleNodeControl
//
//	PURPOSE:	Control the specified node (if applicable)
//
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
//
//	ROUTINE:	CNodeController::IsLipSynching
//
//	PURPOSE:	Are we playing a lip synch sound.
//
// ----------------------------------------------------------------------- //

LTBOOL CNodeController::IsLipSynching()
{
	for (int i = 0; i < MAX_NODECONTROLS; i++)
	{
		if (m_aNodeControls[i].hLipSyncSound)
			return LTTRUE;
	}

	return LTFALSE;
}

