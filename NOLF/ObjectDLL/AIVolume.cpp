// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include <limits.h>
#include "AIVolume.h"
#include "ContainerCodes.h"
#include "FastHeap.h"
#include "AIPath.h"
#include "Door.h"
#include "AIVolumeMgr.h"
#include "AINodeMgr.h"
#include "AIRegion.h"
#include "AIRegionMgr.h"

BEGIN_CLASS(AIVolume)
		ADD_STRINGPROP(StairsDir,	"")
		ADD_STRINGPROP(LedgeDir,	"")
        ADD_BOOLPROP(Vertical, LTFALSE)
		ADD_STRINGPROP_FLAG(ViewNode1, "", PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(ViewNode2, "", PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(ViewNode3, "", PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(ViewNode4, "", PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(ViewNode5, "", PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(ViewNode6, "", PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(ViewNode7, "", PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(ViewNode8, "", PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(ViewNode9, "", PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(ViewNode10, "", PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(ViewNode11, "", PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(ViewNode12, "", PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(ViewNode13, "", PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(ViewNode14, "", PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(ViewNode15, "", PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(ViewNode16, "", PF_OBJECTLINK)
END_CLASS_DEFAULT(AIVolume, BaseClass, NULL, NULL)

// Statics

const static LTFLOAT c_fNeighborThreshhold = 0.5f;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIVolume::AIVolume()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

AIVolume::AIVolume() : BaseClass(OT_WORLDMODEL)
{
    m_bStairs = LTFALSE;
    m_bLedge = LTFALSE;
    m_bVertical = LTTRUE;

	for ( uint32 iViewNode = 0 ; iViewNode < kMaxViewNodes ; iViewNode++ )
	{
		m_ahstrViewNodes[iViewNode] = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIVolume::~AIVolume()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

AIVolume::~AIVolume()
{
	for ( uint32 iViewNode = 0 ; iViewNode < kMaxViewNodes ; iViewNode++ )
	{
		FREE_HSTRING(m_ahstrViewNodes[iViewNode]);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIVolume::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 AIVolume::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
            g_pLTServer->SetNextUpdate(m_hObject, 0.01f);

            uint32 dwRet = BaseClass::EngineMessageFn(messageID, pData, fData);
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;

			pStruct->m_Flags |= FLAG_VISIBLE;
			pStruct->m_ObjectType = OT_WORLDMODEL;//CONTAINER;
			SAFE_STRCPY(pStruct->m_Filename, pStruct->m_Name);
			pStruct->m_SkinName[0] = '\0';
			//pStruct->m_ContainerCode = CC_VOLUME;

			return dwRet;
			break;
		}

		default : break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIVolume::ReadProp
//
//	PURPOSE:	Reads properties
//
// ----------------------------------------------------------------------- //

LTBOOL AIVolume::ReadProp(ObjectCreateStruct *pData)
{
	GenericProp genProp;
    if (!g_pLTServer || !pData) return LTFALSE;

    if ( (g_pLTServer->GetPropGeneric("StairsDir", &genProp ) == LT_OK) && genProp.m_String[0] )
	{
        m_bStairs = LTTRUE;
		sscanf(genProp.m_String, "%f %f %f", &m_vStairsDir.x, &m_vStairsDir.y, &m_vStairsDir.z);
		m_vStairsDir.Norm();
	}

    if ( (g_pLTServer->GetPropGeneric("LedgeDir", &genProp ) == LT_OK) && genProp.m_String[0] )
	{
        m_bLedge = LTTRUE;
		sscanf(genProp.m_String, "%f %f %f", &m_vLedgeDir.x, &m_vLedgeDir.y, &m_vLedgeDir.z);
		m_vLedgeDir.Norm();
	}

    if ( (g_pLTServer->GetPropGeneric("Vertical", &genProp ) == LT_OK) )
	{
		m_bVertical = genProp.m_Bool;
	}

	for ( uint32 iViewNode = 0 ; iViewNode < kMaxViewNodes ; iViewNode++ )
	{
		char szBuffer[128];
		sprintf(szBuffer, "ViewNode%d", iViewNode+1);

        if ( g_pLTServer->GetPropGeneric( szBuffer, &genProp ) == LT_OK )
			if ( genProp.m_String[0] )
                m_ahstrViewNodes[iViewNode] = g_pLTServer->CreateString( genProp.m_String );

	}

    return LTTRUE;
}

CAIVolumeNeighbor::CAIVolumeNeighbor()
{
}

CAIVolumeNeighbor::~CAIVolumeNeighbor()
{
}

void CAIVolumeNeighbor::Load(HMESSAGEREAD hRead)
{
	LOAD_INT(m_iVolume);
	LOAD_VECTOR(m_vConnectionPos);
	LOAD_VECTOR(m_avConnectionEndpoints[0]);
	LOAD_VECTOR(m_avConnectionEndpoints[1]);
	LOAD_VECTOR(m_vConnectionPerpDir);
	LOAD_VECTOR(m_vConnectionDir);
	LOAD_FLOAT(m_fConnectionLength);
}

void CAIVolumeNeighbor::Save(HMESSAGEWRITE hWrite)
{
	SAVE_INT(m_iVolume);
	SAVE_VECTOR(m_vConnectionPos);
	SAVE_VECTOR(m_avConnectionEndpoints[0]);
	SAVE_VECTOR(m_avConnectionEndpoints[1]);
	SAVE_VECTOR(m_vConnectionPerpDir);
	SAVE_VECTOR(m_vConnectionDir);
	SAVE_FLOAT(m_fConnectionLength);
}

void CAIVolumeNeighbor::Init(CAIVolume* pThis, CAIVolume* pNeighbor)
{
	m_iVolume = pNeighbor->GetIndex();

	// Compute the 2d intersection of the two volumes, and compute important
	// things about the geometry of the connection

    LTVector vFrontLeft(0,0,0);
    LTVector vFrontRight(0,0,0);
    LTVector vBackLeft(0,0,0);
    LTVector vBackRight(0,0,0);

	vFrontLeft.x = Max<LTFLOAT>(pThis->GetFrontTopLeft().x, pNeighbor->GetFrontTopLeft().x);
	vFrontLeft.z = Min<LTFLOAT>(pThis->GetFrontTopLeft().z, pNeighbor->GetFrontTopLeft().z);

	vFrontRight.x = Min<LTFLOAT>(pThis->GetFrontTopRight().x, pNeighbor->GetFrontTopRight().x);
	vFrontRight.z = Min<LTFLOAT>(pThis->GetFrontTopRight().z, pNeighbor->GetFrontTopRight().z);

	vBackLeft.x = Max<LTFLOAT>(pThis->GetBackTopLeft().x, pNeighbor->GetBackTopLeft().x);
	vBackLeft.z = Max<LTFLOAT>(pThis->GetBackTopLeft().z, pNeighbor->GetBackTopLeft().z);

	vBackRight.x = Min<LTFLOAT>(pThis->GetBackTopRight().x, pNeighbor->GetBackTopRight().x);
	vBackRight.z = Max<LTFLOAT>(pThis->GetBackTopRight().z, pNeighbor->GetBackTopRight().z);

	// We know connection position (the center of the intersection) easily.

	m_vConnectionPos = (vFrontLeft+vFrontRight+vBackLeft+vBackRight)/4.0f;

	// We need y for vertical movement

#define _A_b pThis->GetFrontBottomRight().y
#define _A_t pThis->GetFrontTopRight().y
#define _B_b pNeighbor->GetFrontBottomRight().y
#define _B_t pNeighbor->GetFrontTopRight().y

	if ( (_A_t >= _B_t) && (_A_t >= _B_b) && (_A_b >= _B_t) && (_A_b >= _B_b) )
	{
		m_vConnectionPos.y = _A_b; // or _B_t
	}
	else if ( (_A_t <= _B_t) && (_A_t <= _B_b) && (_A_b <= _B_t) && (_A_b <= _B_b) )
	{
		m_vConnectionPos.y = _A_t; // or _B_b
	}
	else if ( (_A_t >= _B_t) && (_A_t >= _B_b) && (_A_b <= _B_t) && (_A_b >= _B_b) )
	{
		m_vConnectionPos.y = (_A_b + _B_t)/2.0f;
	}
	else if ( (_A_t <= _B_t) && (_A_t >= _B_b) && (_A_b <= _B_t) && (_A_b <= _B_b) )
	{
		m_vConnectionPos.y = (_A_t + _B_b)/2.0f;
	}
	else if ( (_A_t >= _B_t) && (_A_t >= _B_b) && (_A_b <= _B_t) && (_A_b <= _B_b) )
	{
		m_vConnectionPos.y = (_B_b + _B_t)/2.0f;
	}
	else if ( (_A_t <= _B_t) && (_A_t >= _B_b) && (_A_b <= _B_t) && (_A_b >= _B_b) )
	{
		m_vConnectionPos.y = (_A_b + _A_t)/2.0f;
	}
	else
	{
		m_vConnectionPos.y = -float(INT_MAX);
        DANGER(g_pLTServer, blong);
	}

	// Find the endpoints of the line across the connection, and the vector perpendicular to this

	if ( pThis->Inside(pNeighbor->GetFrontTopLeft()) || pThis->Inside(pNeighbor->GetBackTopRight()) ||
		 pThis->Inside(pNeighbor->GetFrontBottomLeft()) || pThis->Inside(pNeighbor->GetBackBottomRight()) )
	{
        m_avConnectionEndpoints[0] = vFrontRight + LTVector(0, m_vConnectionPos.y, 0);
        m_avConnectionEndpoints[1] = vBackLeft + LTVector(0, m_vConnectionPos.y, 0);
		m_vConnectionPerpDir = vFrontRight - vBackLeft;
		m_vConnectionDir = m_avConnectionEndpoints[1] - m_avConnectionEndpoints[0];
		m_vConnectionDir.y = 0.0f;
		m_fConnectionLength = VEC_MAG(m_vConnectionDir);
		m_vConnectionDir.Norm();
	}
	else
	{
        m_avConnectionEndpoints[0] = vFrontLeft + LTVector(0, m_vConnectionPos.y, 0);
        m_avConnectionEndpoints[1] = vBackRight + LTVector(0, m_vConnectionPos.y, 0);
		m_vConnectionPerpDir = vFrontLeft - vBackRight;
		m_vConnectionDir = m_avConnectionEndpoints[1] - m_avConnectionEndpoints[0];
		m_vConnectionDir.y = 0.0f;
		m_fConnectionLength = VEC_MAG(m_vConnectionDir);
		m_vConnectionDir.Norm();
	}

	LTFLOAT fTemp = m_vConnectionPerpDir[0];
	m_vConnectionPerpDir[0] = m_vConnectionPerpDir[2];
	m_vConnectionPerpDir[2] = fTemp;
	m_vConnectionPerpDir.Norm();

	// Make sure it points into this volume

    LTVector vThisCenter = (pThis->GetFrontTopLeft()+pThis->GetBackTopRight())/2.0f;
    LTVector vThisCenterDir = vThisCenter - m_vConnectionPos;
	vThisCenterDir.y = 0;
	vThisCenterDir.Norm();

	if ( vThisCenterDir.Dot(m_vConnectionPerpDir) < 0.0f )
	{
		m_vConnectionPerpDir = -m_vConnectionPerpDir;
	}

//  g_pLTServer->CPrint("cxn @ %f,%f,%f in %f,%f,%f : %f,%f,%f",
//		EXPANDVEC(m_vConnectionPos), EXPANDVEC(vThisCenter), EXPANDVEC(m_vConnectionPerpDir));
}

CAIVolume::CAIVolume()
{
    m_hstrName = LTNULL;

	m_iRegion = CAIRegion::kInvalidRegion;

	m_cNeighbors = 0;
    m_aNeighbors = LTNULL;

    m_bStairs = LTFALSE;
    m_bLedge = LTFALSE;
    m_bVertical = LTFALSE;

    m_bHadDoors = LTFALSE;
	m_cDoors = 0;
    memset(m_ahDoors, LTNULL, cm_nMaxDoors*sizeof(HOBJECT));

	for ( uint32 iViewNode = 0 ; iViewNode < kMaxViewNodes ; iViewNode++ )
	{
		m_adwViewNodes[iViewNode] = CAINode::kInvalidNodeID;
	}

    m_hLift = LTNULL;
}

CAIVolume::~CAIVolume()
{
	FREE_HSTRING(m_hstrName);

	if ( m_aNeighbors )
	{
		debug_deletea(m_aNeighbors);
	}

	g_pAIVolumeMgr->Unlink(m_hLift);

	for ( int iDoor = 0 ; iDoor < cm_nMaxDoors ; iDoor++ )
	{
		g_pAIVolumeMgr->Unlink(m_ahDoors[iDoor]);
	}
}

void CAIVolume::Load(HMESSAGEREAD hRead)
{
	LOAD_HSTRING(m_hstrName);
	LOAD_INT(m_iVolume);
	LOAD_DWORD(m_iRegion);
	LOAD_VECTOR(m_vFrontTopLeft);
	LOAD_VECTOR(m_vFrontTopRight);
	LOAD_VECTOR(m_vBackTopLeft);
	LOAD_VECTOR(m_vBackTopRight);
	LOAD_VECTOR(m_vFrontBottomLeft);
	LOAD_VECTOR(m_vFrontBottomRight);
	LOAD_VECTOR(m_vBackBottomLeft);
	LOAD_VECTOR(m_vBackBottomRight);
	LOAD_INT(m_cNeighbors);
	LOAD_BOOL(m_bHadDoors);
	LOAD_INT(m_cDoors);
	LOAD_HOBJECT(m_hLift);
	LOAD_BOOL(m_bStairs);
	LOAD_BOOL(m_bLedge);
	LOAD_BOOL(m_bVertical);
	LOAD_VECTOR(m_vStairsDir);
	LOAD_VECTOR(m_vLedgeDir);

	m_aNeighbors = debug_newa(CAIVolumeNeighbor, m_cNeighbors);
	for ( uint32 iNeighbor = 0 ; iNeighbor < m_cNeighbors ; iNeighbor++ )
	{
		m_aNeighbors[iNeighbor].Load(hRead);
	}

	for ( uint32 iDoor = 0 ; iDoor < cm_nMaxDoors ; iDoor++ )
	{
		LOAD_HOBJECT(m_ahDoors[iDoor]);
	}

	for ( uint32 iViewNode = 0 ; iViewNode < kMaxViewNodes ; iViewNode++ )
	{
		LOAD_DWORD(m_adwViewNodes[iViewNode]);
	}
}

void CAIVolume::Save(HMESSAGEWRITE hWrite)
{
	SAVE_HSTRING(m_hstrName);
	SAVE_INT(m_iVolume);
	SAVE_DWORD(m_iRegion);
	SAVE_VECTOR(m_vFrontTopLeft);
	SAVE_VECTOR(m_vFrontTopRight);
	SAVE_VECTOR(m_vBackTopLeft);
	SAVE_VECTOR(m_vBackTopRight);
	SAVE_VECTOR(m_vFrontBottomLeft);
	SAVE_VECTOR(m_vFrontBottomRight);
	SAVE_VECTOR(m_vBackBottomLeft);
	SAVE_VECTOR(m_vBackBottomRight);
	SAVE_INT(m_cNeighbors);
	SAVE_BOOL(m_bHadDoors);
	SAVE_INT(m_cDoors);
	SAVE_HOBJECT(m_hLift);
	SAVE_BOOL(m_bStairs);
	SAVE_BOOL(m_bLedge);
	SAVE_BOOL(m_bVertical);
	SAVE_VECTOR(m_vStairsDir);
	SAVE_VECTOR(m_vLedgeDir);

	for ( uint32 iNeighbor = 0 ; iNeighbor < m_cNeighbors ; iNeighbor++ )
	{
		m_aNeighbors[iNeighbor].Save(hWrite);
	}

	for ( uint32 iDoor = 0 ; iDoor < cm_nMaxDoors ; iDoor++ )
	{
		SAVE_HOBJECT(m_ahDoors[iDoor]);
	}

	for ( uint32 iViewNode = 0 ; iViewNode < kMaxViewNodes ; iViewNode++ )
	{
		SAVE_DWORD(m_adwViewNodes[iViewNode]);
	}
}

void CAIVolume::Init(int32 iVolume, const AIVolume& vol)
{
	char szName[128];
    g_pLTServer->GetObjectName(vol.m_hObject, szName, 127);
    m_hstrName = g_pLTServer->CreateString(szName);

	m_iVolume = iVolume;

    LTVector vVolumePos;
    LTVector vVolumeDims;

    g_pLTServer->GetObjectPos(vol.m_hObject, &vVolumePos);
    g_pLTServer->GetObjectDims(vol.m_hObject, &vVolumeDims);

    m_vFrontTopRight    = vVolumePos + LTVector(vVolumeDims.x,   vVolumeDims.y,  vVolumeDims.z);
    m_vFrontTopLeft     = vVolumePos + LTVector(-vVolumeDims.x,  vVolumeDims.y,  vVolumeDims.z);
    m_vBackTopRight     = vVolumePos + LTVector(vVolumeDims.x,   vVolumeDims.y,  -vVolumeDims.z);
    m_vBackTopLeft      = vVolumePos + LTVector(-vVolumeDims.x,  vVolumeDims.y,  -vVolumeDims.z);
    m_vFrontBottomRight = vVolumePos + LTVector(vVolumeDims.x,   -vVolumeDims.y, vVolumeDims.z);
    m_vFrontBottomLeft  = vVolumePos + LTVector(-vVolumeDims.x,  -vVolumeDims.y, vVolumeDims.z);
    m_vBackBottomRight  = vVolumePos + LTVector(vVolumeDims.x,   -vVolumeDims.y, -vVolumeDims.z);
    m_vBackBottomLeft   = vVolumePos + LTVector(-vVolumeDims.x,  -vVolumeDims.y, -vVolumeDims.z);

	// Get the view volumes

	for ( uint32 iViewNode = 0 ; iViewNode < kMaxViewNodes ; iViewNode++ )
	{
		if ( vol.GetViewNode(iViewNode) )
		{
			CAINode* pNode = g_pAINodeMgr->GetNode(vol.GetViewNode(iViewNode));

			if ( pNode )
			{
				m_adwViewNodes[iViewNode] = pNode->GetID();
			}
			else
			{
				g_pLTServer->CPrint("********** Volume ''%s'' has a view node that does not exist!", GetName());
			}
		}
	}

	// Find any doors located in our volume

    HCLASS  hDoor = g_pLTServer->GetClass("Door");
    HCLASS  hSwitch = g_pLTServer->GetClass("Switch");
    HOBJECT hCurObject = LTNULL;
    while (hCurObject = g_pLTServer->GetNextObject(hCurObject))
	{
        if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hCurObject), hDoor) && !g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hCurObject), hSwitch))
		{
            Door* pDoor = (Door*)g_pLTServer->HandleToObject(hCurObject);
			if ( !pDoor->IsAITriggerable() ) continue;

            LTVector vPos;
            LTVector vDims;

            g_pLTServer->GetObjectPos(hCurObject, &vPos);
            g_pLTServer->GetObjectDims(hCurObject, &vDims);

			if ( Inside(vPos, vDims.y*2.0f) )
			{
				if ( m_cDoors == cm_nMaxDoors )
				{
					_ASSERT(!"Max number of doors in a volume exceeded!!!!");
                    g_pLTServer->CPrint("Max number of doors in a volume exceeded!!!!");
				}
				else
				{
                    m_bHadDoors = LTTRUE;
					g_pAIVolumeMgr->Link(hCurObject);
					m_ahDoors[m_cDoors++] = hCurObject;
				}
			}
		}
	}

    hCurObject = LTNULL;
    while (hCurObject = g_pLTServer->GetNextInactiveObject(hCurObject))
	{
        if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hCurObject), hDoor) && !g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hCurObject), hSwitch))
		{
            Door* pDoor = (Door*)g_pLTServer->HandleToObject(hCurObject);
			if ( !pDoor->IsAITriggerable() ) continue;

            LTVector vPos;
            LTVector vDims;

            g_pLTServer->GetObjectPos(hCurObject, &vPos);
            g_pLTServer->GetObjectDims(hCurObject, &vDims);

			if ( Inside(vPos, vDims.y*2.0f) )
			{
				if ( m_cDoors == cm_nMaxDoors )
				{
					_ASSERT(!"Max number of doors in a volume exceeded!!!!");
                    g_pLTServer->CPrint("Max number of doors in a volume exceeded!!!!");
				}
				else
				{
                    m_bHadDoors = LTTRUE;
					g_pAIVolumeMgr->Link(hCurObject);
					m_ahDoors[m_cDoors++] = hCurObject;
				}
			}
		}
	}

	// Validate volume dims. Must be 64x64 if there's no doors. One side must be <= 128 if there are doors
#ifndef _FINAL // Don't spam multiplayer server with warnings...
	if ( m_cDoors == 0 )
	{
		if ( vVolumeDims.x < 32 )
            g_pLTServer->CPrint("WARNING: Volume \"%s\" is only %d units wide/x (should be at least 64)", GetName(), (int)vVolumeDims.x*2);

		if ( vVolumeDims.z < 32 )
            g_pLTServer->CPrint("WARNING: Volume \"%s\" is only %d units deep/z (should be at least 64)", GetName(), (int)vVolumeDims.z*2);
	}
	else //	if ( m_cDoors != 0 )
	{
		if ( vVolumeDims.x >= 128 && vVolumeDims.z >= 128 )
            g_pLTServer->CPrint("WARNING: Volume \"%s\" is a suspiciously large door volume!", GetName());
	}
#endif

	// Misc flags that have been set

	m_bStairs = vol.HasStairs();
	m_vStairsDir = vol.GetStairsDir();

	m_bLedge = vol.HasLedge();
	m_vLedgeDir = vol.GetLedgeDir();

	m_bVertical = vol.IsVertical();
}

void CAIVolume::EnumerateSearchNodes(uint32* aiSearchNodes, uint32* pcSearchNodes, const uint32 nMaxSearchNodes) const
{
	for ( uint32 iNode = 0 ; iNode < g_pAINodeMgr->GetNumNodes() && *pcSearchNodes < nMaxSearchNodes ; iNode++ )
	{
		CAINode* pNode = g_pAINodeMgr->GetNode(iNode);

		if ( pNode && pNode->IsSearchable() && Inside(pNode->GetPos(), 53.0f) )
		{
			aiSearchNodes[(*pcSearchNodes)++] = pNode->GetID();
		}
	}
}

void CAIVolume::HandleBrokenLink(HOBJECT hObject)
{
	if ( m_hLift == hObject )
	{
        m_hLift = LTNULL;
	}

	for ( int iDoor = 0 ; iDoor < cm_nMaxDoors ; iDoor++ )
	{
		if ( m_ahDoors[iDoor] == hObject )
		{
            m_ahDoors[iDoor] = LTNULL;
			m_cDoors--;
		}
	}
}

LTBOOL CAIVolume::Intersects(const CAIVolume& vol) const
{
	// This is crazy slow, only use at level load time!!!!!!!

	if ( DoesSegmentIntersectAABB(m_vFrontTopRight,			m_vFrontTopLeft,			vol.m_vBackBottomLeft,	vol.m_vFrontTopRight) ) return LTTRUE;
	if ( DoesSegmentIntersectAABB(m_vFrontTopRight,			m_vFrontBottomRight,		vol.m_vBackBottomLeft,	vol.m_vFrontTopRight) ) return LTTRUE;
	if ( DoesSegmentIntersectAABB(m_vFrontTopRight,			m_vBackTopRight,			vol.m_vBackBottomLeft,	vol.m_vFrontTopRight) ) return LTTRUE;
	if ( DoesSegmentIntersectAABB(m_vFrontTopLeft,			m_vFrontTopRight,			vol.m_vBackBottomLeft,	vol.m_vFrontTopRight) ) return LTTRUE;
	if ( DoesSegmentIntersectAABB(m_vFrontTopLeft,			m_vFrontBottomLeft,			vol.m_vBackBottomLeft,	vol.m_vFrontTopRight) ) return LTTRUE;
	if ( DoesSegmentIntersectAABB(m_vFrontTopLeft,			m_vBackTopLeft,				vol.m_vBackBottomLeft,	vol.m_vFrontTopRight) ) return LTTRUE;
	if ( DoesSegmentIntersectAABB(m_vFrontBottomRight,		m_vFrontBottomLeft,			vol.m_vBackBottomLeft,	vol.m_vFrontTopRight) ) return LTTRUE;
	if ( DoesSegmentIntersectAABB(m_vFrontBottomRight,		m_vFrontTopRight,			vol.m_vBackBottomLeft,	vol.m_vFrontTopRight) ) return LTTRUE;
	if ( DoesSegmentIntersectAABB(m_vFrontBottomRight,		m_vBackBottomRight,			vol.m_vBackBottomLeft,	vol.m_vFrontTopRight) ) return LTTRUE;
	if ( DoesSegmentIntersectAABB(m_vFrontBottomLeft,		m_vFrontBottomRight,		vol.m_vBackBottomLeft,	vol.m_vFrontTopRight) ) return LTTRUE;
	if ( DoesSegmentIntersectAABB(m_vFrontBottomLeft,		m_vFrontTopLeft,			vol.m_vBackBottomLeft,	vol.m_vFrontTopRight) ) return LTTRUE;
	if ( DoesSegmentIntersectAABB(m_vFrontBottomLeft,		m_vBackBottomLeft,			vol.m_vBackBottomLeft,	vol.m_vFrontTopRight) ) return LTTRUE;
	if ( DoesSegmentIntersectAABB(m_vBackTopRight,			m_vBackTopLeft,				vol.m_vBackBottomLeft,	vol.m_vFrontTopRight) ) return LTTRUE;
	if ( DoesSegmentIntersectAABB(m_vBackTopRight,			m_vBackBottomRight,			vol.m_vBackBottomLeft,	vol.m_vFrontTopRight) ) return LTTRUE;
	if ( DoesSegmentIntersectAABB(m_vBackTopRight,			m_vFrontTopRight,			vol.m_vBackBottomLeft,	vol.m_vFrontTopRight) ) return LTTRUE;
	if ( DoesSegmentIntersectAABB(m_vBackTopLeft,			m_vBackTopRight,			vol.m_vBackBottomLeft,	vol.m_vFrontTopRight) ) return LTTRUE;
	if ( DoesSegmentIntersectAABB(m_vBackTopLeft,			m_vBackBottomLeft,			vol.m_vBackBottomLeft,	vol.m_vFrontTopRight) ) return LTTRUE;
	if ( DoesSegmentIntersectAABB(m_vBackTopLeft,			m_vFrontTopLeft,			vol.m_vBackBottomLeft,	vol.m_vFrontTopRight) ) return LTTRUE;
	if ( DoesSegmentIntersectAABB(m_vBackBottomRight,		m_vBackBottomLeft,			vol.m_vBackBottomLeft,	vol.m_vFrontTopRight) ) return LTTRUE;
	if ( DoesSegmentIntersectAABB(m_vBackBottomRight,		m_vBackTopRight,			vol.m_vBackBottomLeft,	vol.m_vFrontTopRight) ) return LTTRUE;
	if ( DoesSegmentIntersectAABB(m_vBackBottomRight,		m_vFrontBottomRight,		vol.m_vBackBottomLeft,	vol.m_vFrontTopRight) ) return LTTRUE;
	if ( DoesSegmentIntersectAABB(m_vBackBottomLeft,		m_vBackBottomRight,			vol.m_vBackBottomLeft,	vol.m_vFrontTopRight) ) return LTTRUE;
	if ( DoesSegmentIntersectAABB(m_vBackBottomLeft,		m_vBackTopLeft,				vol.m_vBackBottomLeft,	vol.m_vFrontTopRight) ) return LTTRUE;
	if ( DoesSegmentIntersectAABB(m_vBackBottomLeft,		m_vFrontBottomLeft,			vol.m_vBackBottomLeft,	vol.m_vFrontTopRight) ) return LTTRUE;

	if ( DoesSegmentIntersectAABB(vol.m_vFrontTopRight,		vol.m_vFrontTopLeft,		m_vBackBottomLeft,		m_vFrontTopRight) ) return LTTRUE;
	if ( DoesSegmentIntersectAABB(vol.m_vFrontTopRight,		vol.m_vFrontBottomRight,	m_vBackBottomLeft,		m_vFrontTopRight) ) return LTTRUE;
	if ( DoesSegmentIntersectAABB(vol.m_vFrontTopRight,		vol.m_vBackTopRight,		m_vBackBottomLeft,		m_vFrontTopRight) ) return LTTRUE;
	if ( DoesSegmentIntersectAABB(vol.m_vFrontTopLeft,		vol.m_vFrontTopRight,		m_vBackBottomLeft,		m_vFrontTopRight) ) return LTTRUE;
	if ( DoesSegmentIntersectAABB(vol.m_vFrontTopLeft,		vol.m_vFrontBottomLeft,		m_vBackBottomLeft,		m_vFrontTopRight) ) return LTTRUE;
	if ( DoesSegmentIntersectAABB(vol.m_vFrontTopLeft,		vol.m_vBackTopLeft,			m_vBackBottomLeft,		m_vFrontTopRight) ) return LTTRUE;
	if ( DoesSegmentIntersectAABB(vol.m_vFrontBottomRight,	vol.m_vFrontBottomLeft,		m_vBackBottomLeft,		m_vFrontTopRight) ) return LTTRUE;
	if ( DoesSegmentIntersectAABB(vol.m_vFrontBottomRight,	vol.m_vFrontTopRight,		m_vBackBottomLeft,		m_vFrontTopRight) ) return LTTRUE;
	if ( DoesSegmentIntersectAABB(vol.m_vFrontBottomRight,	vol.m_vBackBottomRight,		m_vBackBottomLeft,		m_vFrontTopRight) ) return LTTRUE;
	if ( DoesSegmentIntersectAABB(vol.m_vFrontBottomLeft,	vol.m_vFrontBottomRight,	m_vBackBottomLeft,		m_vFrontTopRight) ) return LTTRUE;
	if ( DoesSegmentIntersectAABB(vol.m_vFrontBottomLeft,	vol.m_vFrontTopLeft,		m_vBackBottomLeft,		m_vFrontTopRight) ) return LTTRUE;
	if ( DoesSegmentIntersectAABB(vol.m_vFrontBottomLeft,	vol.m_vBackBottomLeft,		m_vBackBottomLeft,		m_vFrontTopRight) ) return LTTRUE;
	if ( DoesSegmentIntersectAABB(vol.m_vBackTopRight,		vol.m_vBackTopLeft,			m_vBackBottomLeft,		m_vFrontTopRight) ) return LTTRUE;
	if ( DoesSegmentIntersectAABB(vol.m_vBackTopRight,		vol.m_vBackBottomRight,		m_vBackBottomLeft,		m_vFrontTopRight) ) return LTTRUE;
	if ( DoesSegmentIntersectAABB(vol.m_vBackTopRight,		vol.m_vFrontTopRight,		m_vBackBottomLeft,		m_vFrontTopRight) ) return LTTRUE;
	if ( DoesSegmentIntersectAABB(vol.m_vBackTopLeft,		vol.m_vBackTopRight,		m_vBackBottomLeft,		m_vFrontTopRight) ) return LTTRUE;
	if ( DoesSegmentIntersectAABB(vol.m_vBackTopLeft,		vol.m_vBackBottomLeft,		m_vBackBottomLeft,		m_vFrontTopRight) ) return LTTRUE;
	if ( DoesSegmentIntersectAABB(vol.m_vBackTopLeft,		vol.m_vFrontTopLeft,		m_vBackBottomLeft,		m_vFrontTopRight) ) return LTTRUE;
	if ( DoesSegmentIntersectAABB(vol.m_vBackBottomRight,	vol.m_vBackBottomLeft,		m_vBackBottomLeft,		m_vFrontTopRight) ) return LTTRUE;
	if ( DoesSegmentIntersectAABB(vol.m_vBackBottomRight,	vol.m_vBackTopRight,		m_vBackBottomLeft,		m_vFrontTopRight) ) return LTTRUE;
	if ( DoesSegmentIntersectAABB(vol.m_vBackBottomRight,	vol.m_vFrontBottomRight,	m_vBackBottomLeft,		m_vFrontTopRight) ) return LTTRUE;
	if ( DoesSegmentIntersectAABB(vol.m_vBackBottomLeft,	vol.m_vBackBottomRight,		m_vBackBottomLeft,		m_vFrontTopRight) ) return LTTRUE;
	if ( DoesSegmentIntersectAABB(vol.m_vBackBottomLeft,	vol.m_vBackTopLeft,			m_vBackBottomLeft,		m_vFrontTopRight) ) return LTTRUE;
	if ( DoesSegmentIntersectAABB(vol.m_vBackBottomLeft,	vol.m_vFrontBottomLeft,		m_vBackBottomLeft,		m_vFrontTopRight) ) return LTTRUE;

	return LTFALSE;
}

LTBOOL CAIVolume::Inside2d(const LTVector& vPos, LTFLOAT fThreshhold) const
{
	// More likely to early out on X or Z than Y, so do in that order.

	return ( (vPos.x <= m_vFrontTopRight.x-fThreshhold)	&&							// right vs. left
			 (vPos.x >= m_vFrontTopLeft.x+fThreshhold)	&&
			 (vPos.z <= m_vFrontTopLeft.z-fThreshhold)	&&							// front vs. back
			 (vPos.z >= m_vBackTopLeft.z+fThreshhold) );
}

LTBOOL CAIVolume::Inside2dLoose(const LTVector& vPos, LTFLOAT fThreshhold) const
{
	if ( Inside2d(vPos, fThreshhold) ) return LTTRUE;

	LTFLOAT fThreshholdSqr = fThreshhold*fThreshhold;

	for ( uint32 iNeighbor = 0 ; iNeighbor < m_cNeighbors ; iNeighbor++ )
	{
		// For each neighbor, check to see our sphere (pos w/ radius fThreshhold) intersects the connection segment, and is at least
		// fThreshhold units away from the endpoints of the connection segment

		const CAIVolumeNeighbor& Neighbor = m_aNeighbors[iNeighbor];

		LTVector R = vPos; R.y = 0.0f;

		LTVector P0 = Neighbor.GetConnectionEndpoint1(); P0.y = 0.0f;
		LTVector P1 = Neighbor.GetConnectionEndpoint2(); P1.y = 0.0f;
		LTVector v = Neighbor.GetConnectionDir();
		LTFLOAT fLength = Neighbor.GetConnectionLength();

		LTFLOAT t = (R - P0).Dot(v);

		if ( (t > fThreshhold) && (t < (fLength - fThreshhold)) )
		{
			return LTTRUE;
		}
	}

	return LTFALSE;
}

LTBOOL CAIVolume::Inside(const LTVector& vPos, LTFLOAT fVerticalThreshhold /* = 0.0f*/) const
{
	// More likely to early out on X or Z than Y, so do in that order.

	return ( (vPos.x <= m_vFrontTopRight.x)	&&							// right vs. left
			 (vPos.x >= m_vFrontTopLeft.x)	&&
			 (vPos.z <= m_vFrontTopLeft.z)	&&							// front vs. back
			 (vPos.z >= m_vBackTopLeft.z)	&&
			 (vPos.y <= (m_vFrontTopLeft.y+fVerticalThreshhold))	&&	// top vs. bottom
			 (vPos.y >= (m_vFrontBottomLeft.y-fVerticalThreshhold)) );
}

void CAIVolume::InitNeighbors(CAIVolume** apVolumeNeighbors, uint32 cNeighbors)
{
    LTBOOL abValidNeighbors[32];

	{for ( uint32 iNeighbor = 0 ; iNeighbor < cNeighbors ; iNeighbor++ )
	{
        LTVector vFrontLeft(0,0,0);
        LTVector vBackRight(0,0,0);

		CAIVolume* pNeighbor = apVolumeNeighbors[iNeighbor];

		vFrontLeft.x = Max<LTFLOAT>(GetFrontTopLeft().x, pNeighbor->GetFrontTopLeft().x);
		vFrontLeft.z = Min<LTFLOAT>(GetFrontTopLeft().z, pNeighbor->GetFrontTopLeft().z);

		vBackRight.x = Min<LTFLOAT>(GetBackTopRight().x, pNeighbor->GetBackTopRight().x);
		vBackRight.z = Max<LTFLOAT>(GetBackTopRight().z, pNeighbor->GetBackTopRight().z);

		// Make sure this isn't an unintended intersection

		if ( vFrontLeft.DistSqr(vBackRight) < c_fNeighborThreshhold )
		{
            abValidNeighbors[iNeighbor] = LTFALSE;
		}
		else
		{
            abValidNeighbors[iNeighbor] = LTTRUE;
			m_cNeighbors++;
		}
	}}

	m_aNeighbors = debug_newa(CAIVolumeNeighbor, m_cNeighbors);

	m_cNeighbors = 0;

	{for ( uint32 iNeighbor = 0 ; iNeighbor < cNeighbors ; iNeighbor++ )
	{
		if ( abValidNeighbors[iNeighbor] )
		{
			m_aNeighbors[m_cNeighbors].Init(this, apVolumeNeighbors[iNeighbor]);
			m_cNeighbors++;
		}
	}}
}

CAINode* CAIVolume::FindViewNode() const
{
	for ( uint32 iViewNode = 0 ; iViewNode < kMaxViewNodes ; iViewNode++ )
	{
		uint32 dwViewNode = m_adwViewNodes[iViewNode];
		CAINode* pNode = (dwViewNode == CAINode::kInvalidNodeID ? LTNULL : g_pAINodeMgr->GetNode(dwViewNode));
		if ( pNode )
		{
			if ( !pNode->IsLocked() )
			{
				return pNode;
			}
		}
	}

	return LTNULL;
}

LTBOOL CAIVolume::HasRegion() const
{
	return m_iRegion != CAIRegion::kInvalidRegion;
}

CAIRegion* CAIVolume::GetRegion() const
{
	_ASSERT(HasRegion());
	return g_pAIRegionMgr->GetRegionByIndex(m_iRegion);
}

void CAIVolume::SetRegion(uint32 iRegion)
{
	if (CAIRegion::kInvalidRegion != m_iRegion)
	{
		CAIRegion *pRegion = g_pAIRegionMgr->GetRegionByIndex(iRegion);
		const char *pRegionName;
		if (!pRegion)
			pRegionName = "**Something Unknown**";
		else
			pRegionName = pRegion->GetName();
		g_pLTServer->CPrint("Warning, AIVolume ''%s'' added to multiple regions (''%s'', then ''%s'')", GetName(), pRegionName, pRegionName);
	}
	else
	{
		m_iRegion = iRegion;
	}
}