// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "StdAfx.h"
#include "AIRegion.h"
#include "AIVolumeMgr.h"
#include "AINodeMgr.h"

BEGIN_CLASS(AIRegion)

	ADD_VECTORPROP_VAL_FLAG(Dims, 16.0f, 16.0f, 16.0f, PF_HIDDEN | PF_DIMS)

	PROP_DEFINEGROUP(Volumes, PF_GROUP1)

		ADD_STRINGPROP_FLAG(Volume1, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume2, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume3, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume4, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume5, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume6, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume7, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume8, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume9, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume10, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume11, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume12, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume13, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume14, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume15, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume16, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume17, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume18, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume19, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume20, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume21, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume22, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume23, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume24, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume25, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume26, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume27, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume28, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume29, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume30, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume31, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume32, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume33, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume34, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume35, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume36, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume37, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume38, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume39, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume40, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume41, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume42, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume43, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume44, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume45, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume46, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume47, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume48, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume49, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume50, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume51, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume52, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume53, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume54, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume55, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume56, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume57, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume58, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume59, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume60, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume61, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume62, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume63, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume64, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume65, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume66, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume67, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume68, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume69, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume70, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume71, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume72, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume73, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume74, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume75, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume76, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume77, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume78, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume79, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume80, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume81, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume82, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume83, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume84, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume85, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume86, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume87, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume88, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume89, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume90, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume91, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume92, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume93, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume94, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume95, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume96, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume97, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume98, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume99, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume100, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume101, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume102, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume103, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume104, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume105, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume106, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume107, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume108, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume109, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume110, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume111, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume112, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume113, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume114, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume115, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume116, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume117, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume118, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume119, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume120, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume121, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume122, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume123, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume124, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume125, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume126, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume127, "", PF_OBJECTLINK|PF_GROUP1)
		ADD_STRINGPROP_FLAG(Volume128, "", PF_OBJECTLINK|PF_GROUP1)

	PROP_DEFINEGROUP(Search, PF_GROUP2)

		ADD_STRINGPROP_FLAG(PostSearchMsg1,  "", PF_GROUP2)
		ADD_STRINGPROP_FLAG(PostSearchMsg2,  "", PF_GROUP2)
		ADD_STRINGPROP_FLAG(PostSearchMsg3,  "", PF_GROUP2)
		ADD_STRINGPROP_FLAG(PostSearchMsg4,  "", PF_GROUP2)
		ADD_STRINGPROP_FLAG(PostSearchMsg5,  "", PF_GROUP2)
		ADD_STRINGPROP_FLAG(PostSearchMsg6,  "", PF_GROUP2)
		ADD_STRINGPROP_FLAG(PostSearchMsg7,  "", PF_GROUP2)
		ADD_STRINGPROP_FLAG(PostSearchMsg8,  "", PF_GROUP2)
		ADD_STRINGPROP_FLAG(PostSearchMsg9,  "", PF_GROUP2)
		ADD_STRINGPROP_FLAG(PostSearchMsg10, "", PF_GROUP2)
		ADD_STRINGPROP_FLAG(PostSearchMsg11, "", PF_GROUP2)
		ADD_STRINGPROP_FLAG(PostSearchMsg12, "", PF_GROUP2)
		ADD_STRINGPROP_FLAG(PostSearchMsg13, "", PF_GROUP2)
		ADD_STRINGPROP_FLAG(PostSearchMsg14, "", PF_GROUP2)
		ADD_STRINGPROP_FLAG(PostSearchMsg15, "", PF_GROUP2)
		ADD_STRINGPROP_FLAG(PostSearchMsg16, "", PF_GROUP2)
		ADD_STRINGPROP_FLAG(PostSearchMsg17, "", PF_GROUP2)
		ADD_STRINGPROP_FLAG(PostSearchMsg18, "", PF_GROUP2)
		ADD_STRINGPROP_FLAG(PostSearchMsg19, "", PF_GROUP2)
		ADD_STRINGPROP_FLAG(PostSearchMsg20, "", PF_GROUP2)
		ADD_STRINGPROP_FLAG(PostSearchMsg21, "", PF_GROUP2)
		ADD_STRINGPROP_FLAG(PostSearchMsg22, "", PF_GROUP2)
		ADD_STRINGPROP_FLAG(PostSearchMsg23, "", PF_GROUP2)
		ADD_STRINGPROP_FLAG(PostSearchMsg24, "", PF_GROUP2)
		ADD_STRINGPROP_FLAG(PostSearchMsg25, "", PF_GROUP2)
		ADD_STRINGPROP_FLAG(PostSearchMsg26, "", PF_GROUP2)
		ADD_STRINGPROP_FLAG(PostSearchMsg27, "", PF_GROUP2)
		ADD_STRINGPROP_FLAG(PostSearchMsg28, "", PF_GROUP2)
		ADD_STRINGPROP_FLAG(PostSearchMsg29, "", PF_GROUP2)
		ADD_STRINGPROP_FLAG(PostSearchMsg30, "", PF_GROUP2)
		ADD_STRINGPROP_FLAG(PostSearchMsg31, "", PF_GROUP2)
		ADD_STRINGPROP_FLAG(PostSearchMsg32, "", PF_GROUP2)

END_CLASS_DEFAULT(AIRegion, BaseClass, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIRegion::AIRegion()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

AIRegion::AIRegion() : BaseClass(OT_NORMAL)
{
	for ( uint32 iVolume = 0 ; iVolume < kMaxVolumes ; iVolume++ )
	{
		m_ahstrVolumes[iVolume] = LTNULL;
	}

	for ( uint32 iPostSearchMsg = 0 ; iPostSearchMsg < kMaxPostSearchMsgs ; iPostSearchMsg++ )
	{
		m_ahstrPostSearchMsgs[iPostSearchMsg] = LTNULL;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIRegion::~AIRegion()
//
//	PURPOSE:	Destroy the object
//
// ----------------------------------------------------------------------- //

AIRegion::~AIRegion()
{
	for ( uint32 iVolume = 0 ; iVolume < kMaxVolumes ; iVolume++ )
	{
		FREE_HSTRING(m_ahstrVolumes[iVolume]);
	}

	for ( uint32 iPostSearchMsg = 0 ; iPostSearchMsg < kMaxPostSearchMsgs ; iPostSearchMsg++ )
	{
		FREE_HSTRING(m_ahstrPostSearchMsgs[iPostSearchMsg]);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIRegion::ReadProp
//
//	PURPOSE:	Reads properties
//
// ----------------------------------------------------------------------- //

LTBOOL AIRegion::ReadProp(ObjectCreateStruct *pData)
{
    if (!g_pLTServer || !pData) return LTFALSE;

	GenericProp genProp;
	
	for ( uint32 iVolume = 0 ; iVolume < kMaxVolumes ; iVolume++ )
	{
		char szBuffer[128];
		sprintf(szBuffer, "Volume%d", iVolume+1);

        if ( g_pLTServer->GetPropGeneric( szBuffer, &genProp ) == LT_OK )
			if ( genProp.m_String[0] )
                m_ahstrVolumes[iVolume] = g_pLTServer->CreateString( genProp.m_String );

	}

	for ( uint32 iPostSearchMsg = 0 ; iPostSearchMsg < kMaxPostSearchMsgs ; iPostSearchMsg++ )
	{
		char szBuffer[128];
		sprintf(szBuffer, "PostSearchMsg%d", iPostSearchMsg+1);

        if ( g_pLTServer->GetPropGeneric( szBuffer, &genProp ) == LT_OK )
			if ( genProp.m_String[0] )
                m_ahstrPostSearchMsgs[iPostSearchMsg] = g_pLTServer->CreateString( genProp.m_String );

	}

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIRegion::EngineMessageFn
//
//	PURPOSE:	Handles engine message functions
//
// ----------------------------------------------------------------------- //

uint32 AIRegion::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
            uint32 dwRet = BaseClass::EngineMessageFn(messageID, pData, fData);

			int nInfo = (int)fData;
			if (nInfo == PRECREATE_WORLDFILE || nInfo == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}
			return dwRet;
			break;
		}

		default : break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIRegion::CAIRegion()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

CAIRegion::CAIRegion()
{
	m_hstrName = LTNULL;
	m_iRegion = kInvalidRegion;
	m_cVolumes = 0;
	m_cSearchNodes = 0;
	m_cSearchers = 0;
	m_cPostSearchMsgs = 0;

	for ( uint32 iPostSearchMsg = 0 ; iPostSearchMsg < kMaxPostSearchMsgs ; iPostSearchMsg++ )
	{
		m_ahstrPostSearchMsgs[iPostSearchMsg] = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIRegion::~CAIRegion()
//
//	PURPOSE:	Destroy the object
//
// ----------------------------------------------------------------------- //

CAIRegion::~CAIRegion()
{
	FREE_HSTRING(m_hstrName);

	for ( uint32 iPostSearchMsg = 0 ; iPostSearchMsg < kMaxPostSearchMsgs ; iPostSearchMsg++ )
	{
		FREE_HSTRING(m_ahstrPostSearchMsgs[iPostSearchMsg]);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIRegion::Init()
//
//	PURPOSE:	Initialize us from an engine object AIRegion
//
// ----------------------------------------------------------------------- //

void CAIRegion::Init(uint32 iRegion, const AIRegion& Region)
{
	m_iRegion = iRegion;
    m_hstrName = g_pLTServer->CreateString(g_pLTServer->GetObjectName(Region.m_hObject));

	{for ( uint32 iVolume = 0 ; iVolume < kMaxVolumes ; iVolume++ )
	{
		CAIVolume* pVolume = g_pAIVolumeMgr->GetVolumeByName(g_pLTServer->GetStringData(Region.m_ahstrVolumes[iVolume]));
		if ( pVolume )
		{
			pVolume->SetRegion(m_iRegion);
			m_aiVolumes[m_cVolumes] = pVolume->GetIndex();
			m_cVolumes++;
		}
	}}

	{for ( uint32 iVolume = 0 ; iVolume < m_cVolumes ; iVolume++ )
	{
		CAIVolume* pVolume = g_pAIVolumeMgr->GetVolumeByIndex(m_aiVolumes[iVolume]);
		_ASSERT(pVolume);

		pVolume->EnumerateSearchNodes(m_aiSearchNodes, &m_cSearchNodes, kMaxSearchNodes);
	}}

	{for ( int32 iPostSearchMsg = kMaxPostSearchMsgs-1 ; iPostSearchMsg >= 0 ; iPostSearchMsg-- )
	{
		if ( Region.m_ahstrPostSearchMsgs[iPostSearchMsg] )
		{
			m_ahstrPostSearchMsgs[m_cPostSearchMsgs] = g_pLTServer->CopyString(Region.m_ahstrPostSearchMsgs[iPostSearchMsg]);
			m_cPostSearchMsgs++;
		}
	}}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIRegion::Save()
//
//	PURPOSE:	Save the Region
//
// ----------------------------------------------------------------------- //

void CAIRegion::Save(HMESSAGEWRITE hWrite)
{
	SAVE_DWORD(m_iRegion);
	SAVE_DWORD(m_cVolumes);
	SAVE_HSTRING(m_hstrName);
	SAVE_DWORD(m_cSearchNodes);
	SAVE_DWORD(m_cSearchers);
	SAVE_DWORD(m_cPostSearchMsgs);
	
	for ( uint32 iVolume = 0 ; iVolume < kMaxVolumes ; iVolume++ )
	{
		SAVE_DWORD(m_aiVolumes[iVolume]);
	}

	for ( uint32 iNode = 0 ; iNode < kMaxSearchNodes ; iNode++ )
	{
		SAVE_DWORD(m_aiSearchNodes[iNode]);
	}

	for ( uint32 iPostSearchMsg = 0 ; iPostSearchMsg < kMaxPostSearchMsgs ; iPostSearchMsg++ )
	{
		SAVE_HSTRING(m_ahstrPostSearchMsgs[iPostSearchMsg]);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIRegion::Load()
//
//	PURPOSE:	Load the Region
//
// ----------------------------------------------------------------------- //

void CAIRegion::Load(HMESSAGEREAD hRead)
{
	LOAD_DWORD(m_iRegion);
	LOAD_DWORD(m_cVolumes);
	LOAD_HSTRING(m_hstrName);
	LOAD_DWORD(m_cSearchNodes);
	LOAD_DWORD(m_cSearchers);
	LOAD_DWORD(m_cPostSearchMsgs);
	
	for ( uint32 iVolume = 0 ; iVolume < kMaxVolumes ; iVolume++ )
	{
		LOAD_DWORD(m_aiVolumes[iVolume]);
	}

	for ( uint32 iNode = 0 ; iNode < kMaxSearchNodes ; iNode++ )
	{
		LOAD_DWORD(m_aiSearchNodes[iNode]);
	}

	for ( uint32 iPostSearchMsg = 0 ; iPostSearchMsg < kMaxPostSearchMsgs ; iPostSearchMsg++ )
	{
		LOAD_HSTRING(m_ahstrPostSearchMsgs[iPostSearchMsg]);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeMgr::FindNearestSearchNode
//
//	PURPOSE:	Finds the nearest Search node to vPos
//
// ----------------------------------------------------------------------- //

CAINode* CAIRegion::FindNearestSearchNode(const LTVector& vPos, HOBJECT hThreat) const
{
	_ASSERT(IsSearchable());

    LTFLOAT  fMinDistanceSqr = (float)INT_MAX;
    CAINode *pClosestNode = LTNULL;

    for ( uint32 iSearchNode = 0 ; iSearchNode < m_cSearchNodes ; iSearchNode++ )
	{
		CAINode* pNode = g_pAINodeMgr->GetNode(m_aiSearchNodes[iSearchNode]);
		if ( !pNode->IsLocked() && pNode->IsSearchable() )
		{
            LTFLOAT fDistanceSqr = VEC_DISTSQR(vPos, pNode->GetPos());

			if ( fDistanceSqr < fMinDistanceSqr )
			{
				if ( eSearchStatusOk == pNode->GetSearchStatus(vPos, hThreat) )
				{
					fMinDistanceSqr = fDistanceSqr;
					pClosestNode = pNode;
				}
			}
		}
	}

	return pClosestNode;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIRegion::AddSearcher
//
//	PURPOSE:	Adds a searcher to this region
//
// ----------------------------------------------------------------------- //

void CAIRegion::AddSearcher(CAI* pAI)
{
	m_cSearchers++;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIRegion::RemoveSearcher
//
//	PURPOSE:	Removes a searcher from this region
//
// ----------------------------------------------------------------------- //

void CAIRegion::RemoveSearcher(CAI* pAI)
{
	m_cSearchers--;

	if ( 0 == m_cSearchers )
	{
		for ( uint32 iSearchNode = 0 ; iSearchNode < m_cSearchNodes ; iSearchNode++ )
		{
			CAINode* pNode = g_pAINodeMgr->GetNode(m_aiSearchNodes[iSearchNode]);
			pNode->SearchReset();
		}
	}
}

