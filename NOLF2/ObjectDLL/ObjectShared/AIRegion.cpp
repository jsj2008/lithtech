// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved

#include "StdAfx.h"
#include "AIRegion.h"
#include "AIVolume.h"
#include "AIVolumeMgr.h"
#include "AINodeMgr.h"
#include "AIUtils.h"
#include "PSets.h"
#include "AIAssert.h"

LINKFROM_MODULE( AIRegion );

#pragma force_active on
BEGIN_CLASS(AIRegion)

	ADD_VECTORPROP_VAL_FLAG(Dims, 16.0f, 16.0f, 16.0f, PF_HIDDEN | PF_DIMS)
	ADD_PSETS_PROP(PF_GROUP(1))
/*
	PROP_DEFINEGROUP(Search, PF_GROUP(1))

		ADD_STRINGPROP_FLAG(PostSearchMsg1,  "", PF_GROUP(2))
		ADD_STRINGPROP_FLAG(PostSearchMsg2,  "", PF_GROUP(2))
		ADD_STRINGPROP_FLAG(PostSearchMsg3,  "", PF_GROUP(2))
		ADD_STRINGPROP_FLAG(PostSearchMsg4,  "", PF_GROUP(2))
		ADD_STRINGPROP_FLAG(PostSearchMsg5,  "", PF_GROUP(2))
		ADD_STRINGPROP_FLAG(PostSearchMsg6,  "", PF_GROUP(2))
		ADD_STRINGPROP_FLAG(PostSearchMsg7,  "", PF_GROUP(2))
		ADD_STRINGPROP_FLAG(PostSearchMsg8,  "", PF_GROUP(2))
		ADD_STRINGPROP_FLAG(PostSearchMsg9,  "", PF_GROUP(2))
		ADD_STRINGPROP_FLAG(PostSearchMsg10, "", PF_GROUP(2))
		ADD_STRINGPROP_FLAG(PostSearchMsg11, "", PF_GROUP(2))
		ADD_STRINGPROP_FLAG(PostSearchMsg12, "", PF_GROUP(2))
		ADD_STRINGPROP_FLAG(PostSearchMsg13, "", PF_GROUP(2))
		ADD_STRINGPROP_FLAG(PostSearchMsg14, "", PF_GROUP(2))
		ADD_STRINGPROP_FLAG(PostSearchMsg15, "", PF_GROUP(2))
		ADD_STRINGPROP_FLAG(PostSearchMsg16, "", PF_GROUP(2))
		ADD_STRINGPROP_FLAG(PostSearchMsg17, "", PF_GROUP(2))
		ADD_STRINGPROP_FLAG(PostSearchMsg18, "", PF_GROUP(2))
		ADD_STRINGPROP_FLAG(PostSearchMsg19, "", PF_GROUP(2))
		ADD_STRINGPROP_FLAG(PostSearchMsg20, "", PF_GROUP(2))
		ADD_STRINGPROP_FLAG(PostSearchMsg21, "", PF_GROUP(2))
		ADD_STRINGPROP_FLAG(PostSearchMsg22, "", PF_GROUP(2))
		ADD_STRINGPROP_FLAG(PostSearchMsg23, "", PF_GROUP(2))
		ADD_STRINGPROP_FLAG(PostSearchMsg24, "", PF_GROUP(2))
		ADD_STRINGPROP_FLAG(PostSearchMsg25, "", PF_GROUP(2))
		ADD_STRINGPROP_FLAG(PostSearchMsg26, "", PF_GROUP(2))
		ADD_STRINGPROP_FLAG(PostSearchMsg27, "", PF_GROUP(2))
		ADD_STRINGPROP_FLAG(PostSearchMsg28, "", PF_GROUP(2))
		ADD_STRINGPROP_FLAG(PostSearchMsg29, "", PF_GROUP(2))
		ADD_STRINGPROP_FLAG(PostSearchMsg30, "", PF_GROUP(2))
		ADD_STRINGPROP_FLAG(PostSearchMsg31, "", PF_GROUP(2))
		ADD_STRINGPROP_FLAG(PostSearchMsg32, "", PF_GROUP(2))
*/
END_CLASS_DEFAULT(AIRegion, BaseClass, NULL, NULL)
#pragma force_active off

// ----------------------------------------------------------------------- //

AIRegion::AIRegion() : BaseClass(OT_NORMAL)
{
	m_hstrName = LTNULL;

	m_cSearchNodes = 0;
	m_cSearchers = 0;

	m_cPostSearchMsgs = 0;

	for ( uint32 iNode = 0 ; iNode < kMaxSearchNodes ; iNode++ )
	{
		m_apSearchNodes[iNode] = LTNULL;
	}

	for ( uint32 iPostSearchMsg = 0 ; iPostSearchMsg < kMaxPostSearchMsgs ; iPostSearchMsg++ )
	{
		m_ahstrPostSearchMsgs[iPostSearchMsg] = LTNULL;
	}
	memset(m_bPSets,0,8*sizeof(LTBOOL));
}

// ----------------------------------------------------------------------- //

AIRegion::~AIRegion()
{
	FREE_HSTRING(m_hstrName);

	for ( uint32 iPostSearchMsg = 0 ; iPostSearchMsg < kMaxPostSearchMsgs ; iPostSearchMsg++ )
	{
		FREE_HSTRING(m_ahstrPostSearchMsgs[iPostSearchMsg]);
	}
}

// ----------------------------------------------------------------------- //

uint32 AIRegion::EngineMessageFn(uint32 messageID, void *pv, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
            uint32 dwRet = BaseClass::EngineMessageFn(messageID, pv, fData);

			if ( (int)fData == PRECREATE_WORLDFILE || (int)fData == PRECREATE_STRINGPROP )
			{
				ReadProp((ObjectCreateStruct*)pv);
			}

			return dwRet;
		}
		break;

		case MID_INITIALUPDATE:
			{
				// Don't eat ticks please...
				SetNextUpdate(m_hObject, UPDATE_NEVER);
			}
			break;

		case MID_ALLOBJECTSCREATED:
			if( m_cSearchNodes == 0 )
			{
				AIError( "Region \"%s\" has zero seach nodes!", GetName() );
			}
			break;

		case MID_SAVEOBJECT:
		{
            Save((ILTMessage_Write*)pv);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((ILTMessage_Read*)pv);
		}
		break;
	}

	return BaseClass::EngineMessageFn(messageID, pv, fData);
}

// ----------------------------------------------------------------------- //

LTBOOL AIRegion::ReadProp(ObjectCreateStruct *pData)
{
    if (!g_pLTServer || !pData) return LTFALSE;

	// Clear out our permission sets
	memset(m_bPSets,0,8*sizeof(LTBOOL));

	// Read them in
	bool bSets[8];
	g_pLTServer->GetPropBool("Key1", &bSets[0]);
	g_pLTServer->GetPropBool("Key2", &bSets[1]);
	g_pLTServer->GetPropBool("Key3", &bSets[2]);
	g_pLTServer->GetPropBool("Key4", &bSets[3]);
	g_pLTServer->GetPropBool("Key5", &bSets[4]);
	g_pLTServer->GetPropBool("Key6", &bSets[5]);
	g_pLTServer->GetPropBool("Key7", &bSets[6]);
	g_pLTServer->GetPropBool("Key8", &bSets[7]);

	for ( uint32 nSetLoop = 0; nSetLoop < 8; ++nSetLoop )
		m_bPSets[nSetLoop] = (bSets[nSetLoop] ? LTTRUE : LTFALSE);

    if ( g_pLTServer->GetPropGeneric( "Name", &g_gp ) == LT_OK )
		if ( g_gp.m_String[0] )
            m_hstrName = g_pLTServer->CreateString( g_gp.m_String );
	
	for ( uint32 iPostSearchMsg = 0 ; iPostSearchMsg < kMaxPostSearchMsgs ; iPostSearchMsg++ )
	{
		char szBuffer[128];
		sprintf(szBuffer, "PostSearchMsg%d", iPostSearchMsg+1);

        if ( g_pLTServer->GetPropGeneric( szBuffer, &g_gp ) == LT_OK )
			if ( g_gp.m_String[0] )
                m_ahstrPostSearchMsgs[iPostSearchMsg] = g_pLTServer->CreateString( g_gp.m_String );

	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void AIRegion::Save(ILTMessage_Write *pMsg)
{
	SAVE_HSTRING(m_hstrName);
	SAVE_DWORD(m_cSearchNodes);
	SAVE_DWORD(m_cSearchers);
	SAVE_DWORD(m_cPostSearchMsgs);
	
	SAVE_DWORD(m_lstVolumes.size());

	AIVolume* pVolume;
	AIREGION_VOLUME_LIST::iterator it;
	for ( it = m_lstVolumes.begin() ; it != m_lstVolumes.end() ; ++it )
	{
		pVolume = *it;
		SAVE_COBJECT(pVolume);
	}

	for ( uint32 iNode = 0 ; iNode < kMaxSearchNodes ; iNode++ )
	{
		SAVE_COBJECT(m_apSearchNodes[iNode]);
	}

	for ( uint32 iPostSearchMsg = 0 ; iPostSearchMsg < kMaxPostSearchMsgs ; iPostSearchMsg++ )
	{
		SAVE_HSTRING(m_ahstrPostSearchMsgs[iPostSearchMsg]);
	}

	SAVE_VECTOR(m_vExtentsMin);
	SAVE_VECTOR(m_vExtentsMax);

	// Permission sets
	pMsg->Writeuint8(m_bPSets[0]);
	pMsg->Writeuint8(m_bPSets[1]);
	pMsg->Writeuint8(m_bPSets[2]);
	pMsg->Writeuint8(m_bPSets[3]);
	pMsg->Writeuint8(m_bPSets[4]);
	pMsg->Writeuint8(m_bPSets[5]);
	pMsg->Writeuint8(m_bPSets[6]);
	pMsg->Writeuint8(m_bPSets[7]);
}

// ----------------------------------------------------------------------- //

void AIRegion::Load(ILTMessage_Read *pMsg)
{
	LOAD_HSTRING(m_hstrName);
	LOAD_DWORD(m_cSearchNodes);
	LOAD_DWORD(m_cSearchers);
	LOAD_DWORD(m_cPostSearchMsgs);

	uint32 cVolumes;
	LOAD_DWORD(cVolumes);
	m_lstVolumes.resize(cVolumes);

	for ( uint32 iVolume=0; iVolume < cVolumes ; ++iVolume )
	{
		LOAD_COBJECT( m_lstVolumes[iVolume], AIVolume );
	}

	for ( uint32 iNode = 0 ; iNode < kMaxSearchNodes ; iNode++ )
	{
		LOAD_COBJECT(m_apSearchNodes[iNode], AINodeSearch);
	}

	for ( uint32 iPostSearchMsg = 0 ; iPostSearchMsg < kMaxPostSearchMsgs ; iPostSearchMsg++ )
	{
		LOAD_HSTRING(m_ahstrPostSearchMsgs[iPostSearchMsg]);
	}

	LOAD_VECTOR(m_vExtentsMin);
	LOAD_VECTOR(m_vExtentsMax);

	// Permission sets
    m_bPSets[0] = pMsg->Readuint8();
	m_bPSets[1] = pMsg->Readuint8();
	m_bPSets[2] = pMsg->Readuint8();
	m_bPSets[3] = pMsg->Readuint8();
	m_bPSets[4] = pMsg->Readuint8();
	m_bPSets[5] = pMsg->Readuint8();
	m_bPSets[6] = pMsg->Readuint8();
	m_bPSets[7] = pMsg->Readuint8();
}

// ----------------------------------------------------------------------- //

uint8 AIRegion::GetPsetByte()
{
	uint8 rVal = 0;
	for (int i = 0; i < 8; i++)
	{
		if (m_bPSets[i])
			rVal |= 1<<i;
	}
	return rVal;
}

// ----------------------------------------------------------------------- //

void AIRegion::AddVolume(AIVolume* pVolume)
{
	AIASSERT(pVolume, m_hObject, "AIRegion::AddVolume: Volume is NULL");
	m_lstVolumes.push_back(pVolume);

	// Recalculate extents of box containing all volumes in region.

	if( m_lstVolumes.size() == 1 )
	{
		m_vExtentsMin = pVolume->GetBackBottomLeft();
		m_vExtentsMax = pVolume->GetFrontTopRight();
	}
	else {
		LTVector vExtent = pVolume->GetBackBottomLeft();
		m_vExtentsMin.x = Min( m_vExtentsMin.x, vExtent.x );
		m_vExtentsMin.y = Min( m_vExtentsMin.y, vExtent.y );
		m_vExtentsMin.z = Min( m_vExtentsMin.z, vExtent.z );

		vExtent = pVolume->GetFrontTopRight();
		m_vExtentsMax.x = Max( m_vExtentsMax.x, vExtent.x );
		m_vExtentsMax.y = Max( m_vExtentsMax.y, vExtent.y );
		m_vExtentsMax.z = Max( m_vExtentsMax.z, vExtent.z );
	}

	g_pAINodeMgr->EnumerateNodesInVolume(kNode_Search, pVolume, 53.f, (AINode**)m_apSearchNodes, &m_cSearchNodes, kMaxSearchNodes);
}

const LTVector& AIRegion::GetExtentsMin() const
{
	return m_vExtentsMin; 
}

// ----------------------------------------------------------------------- //

AINodeSearch* AIRegion::FindNearestSearchNode(const LTVector& vPos, LTFLOAT fCurTime) const
{
	_ASSERT(IsSearchable());

    LTFLOAT  fMinDistanceSqr = (float)INT_MAX;
    AINodeSearch* pClosestNode = LTNULL;

    for ( uint32 iSearchNode = 0 ; iSearchNode < m_cSearchNodes ; iSearchNode++ )
	{
		if ( m_apSearchNodes[iSearchNode]->GetType() != kNode_Search ) 
			continue;

		AINodeSearch* pNode = m_apSearchNodes[iSearchNode];
		if ( pNode->IsLockedDisabledOrTimedOut() )
			continue;

		LTFLOAT fDistanceSqr = VEC_DISTSQR(vPos, pNode->GetPos());

		if ( fDistanceSqr < fMinDistanceSqr )
		{
			if ( kStatus_Ok == pNode->GetStatus(vPos, LTNULL) )
			{
				fMinDistanceSqr = fDistanceSqr;
				pClosestNode = pNode;
			}
		}
	}

	return pClosestNode;
}

// ----------------------------------------------------------------------- //

void AIRegion::AddSearcher(CAI* pAI)
{
	m_cSearchers++;
}

// ----------------------------------------------------------------------- //

void AIRegion::RemoveSearcher(CAI* pAI)
{
	m_cSearchers--;

	if ( 0 == m_cSearchers )
	{
		for ( uint32 iSearchNode = 0 ; iSearchNode < m_cSearchNodes ; iSearchNode++ )
		{
			m_apSearchNodes[iSearchNode]->SearchReset();
		}
	}
}

