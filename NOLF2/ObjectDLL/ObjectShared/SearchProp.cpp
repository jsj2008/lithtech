// ----------------------------------------------------------------------- //
//
// MODULE  : SearchProp.cpp
//
// PURPOSE : Searchable Prop - Implementation
//
// CREATED : 12/21/2001
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "SearchProp.h"
#include "ObjectMsgs.h"

LINKFROM_MODULE( SearchProp );

BEGIN_CLASS(SearchProp)
	ADD_DESTRUCTIBLE_MODEL_AGGREGATE(PF_GROUP(1), PF_HIDDEN)
	
	// Hide all of our parent properties (these are all set via the
	// PropTypes.txt bute file)
	ADD_STRINGPROP_FLAG(Filename, "", PF_HIDDEN | PF_MODEL)

	// The list of available prop types...
	ADD_STRINGPROP_FLAG(Type, "", PF_STATICLIST | PF_DIMS | PF_LOCALDIMS)

	ADD_SEARCHABLE_AGGREGATE(PF_GROUP(3), 0)
	ADD_STRINGPROP_FLAG(SearchSoundName, "Interface\\Snd\\SearchPropLoop.wav",PF_GROUP(3) | PF_FILENAME)

END_CLASS_DEFAULT_FLAGS_PLUGIN(SearchProp, PropType, NULL, NULL, 0, CSearchPropPlugin)


// Register with the CommandMgr...

CMDMGR_BEGIN_REGISTER_CLASS( SearchProp )
CMDMGR_END_REGISTER_CLASS( SearchProp, PropType )

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSearchProp::CSearchProp()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
SearchProp::SearchProp() : PropType()
{
	AddAggregate(&m_search);
}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SearchProp::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 SearchProp::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
            uint32 dwRet = PropType::EngineMessageFn(messageID, pData, fData);

			m_search.Enable(true);
			m_bTouchable = LTFALSE;
			m_damage.SetMaxHitPoints(1.0f);
			m_damage.SetHitPoints(1.0f);
			m_damage.SetCanDamage(LTFALSE);

			return dwRet;
		}
		break;

		case MID_INITIALUPDATE:
		{
			uint32 dwRet = PropType::EngineMessageFn(messageID, pData, fData);

			g_pCommonLT->SetObjectFlags( m_hObject, OFT_User, m_search.IsEnabled() ? USRFLG_CAN_SEARCH : 0, USRFLG_CAN_SEARCH );

			return dwRet;
		};
		break;


		default : break;
	}

	return PropType::EngineMessageFn(messageID, pData, fData);
}



#ifndef __PSX2
LTRESULT CSearchPropPlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char** aszStrings,
    uint32* pcStrings,
    const uint32 cMaxStrings,
    const uint32 cMaxStringLength)
{

	if (_strcmpi("Type", szPropName) == 0)
	{
		if (m_PropTypeMgrPlugin.PreHook_EditStringList(szRezPath,
			szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength) == LT_OK)
		{
			//strip out strings associated with non-searchable props
			uint32 n = 0;
			for (uint32 i = 0; i < (*pcStrings); i++)
			{
				//assume each proptype is not searchable
				bool bGood = false;
				PROPTYPE* pPropType = g_pPropTypeMgr->GetPropType(aszStrings[i]);

				//if we do have a searchable prop, keep it
				if (pPropType && pPropType->bSearchable)
				{
					bGood = true;
				}

				
				if (bGood)
				{
					//if we have a searchable prop, copy it to the next slot
					if (n < i)
					{
						SAFE_STRCPY(aszStrings[n],aszStrings[i]);
					}
					n++;
				}

			}

			//adjust our count
			(*pcStrings) = n;

			return LT_OK;
		}
	}
	else if ( LT_OK == CPropTypePlugin::PreHook_EditStringList(szRezPath, szPropName,
		aszStrings, pcStrings, cMaxStrings, cMaxStringLength) )
	{
		return LT_OK;
	}
	else if (m_SearchItemPlugin.PreHook_EditStringList(szRezPath,
			szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength) == LT_OK)
	{
		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

LTRESULT CSearchPropPlugin::PreHook_Dims(
			const char* szRezPath,
			const char* szPropValue,
			char* szModelFilenameBuf,
			int	  nModelFilenameBufLen,
			LTVector & vDims)
{
	if (LT_OK == CPropTypePlugin::PreHook_Dims(szRezPath, szPropValue,
		szModelFilenameBuf, nModelFilenameBufLen, vDims))
	{
		return LT_OK;
	}

	return LT_UNSUPPORTED;

}

#endif

