// ----------------------------------------------------------------------- //
//
// MODULE  :	PositionalDamage.cpp
//
// PURPOSE :	Provides the implementation for the positional damage class
//				which applies a certain amount of damage at a specific point
//				in space
//
// CREATED :	11/29/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //
#include "Stdafx.h"
#include "PositionalDamage.h"

//------------------------------------------------------------------------------
// PositionalDamage_Plugin
// Handles WorldEdit integration for this object
//------------------------------------------------------------------------------
class PositionalDamage_Plugin : 
	public IObjectPlugin
{
public:
    
	LTRESULT PositionalDamage_Plugin::PreHook_EditStringList(const char* szRezPath,
												const char* szPropName,
												char** aszStrings,
												uint32* pcStrings,
												const uint32 cMaxStrings,
												const uint32 cMaxStringLength)
	{
		if( LTStrIEquals( "DamageType", szPropName ))
		{
			// Add an entry for each supported damage type
			for (uint32 nCurrType = 0; nCurrType < kNumDamageTypes; nCurrType++)
			{
				DamageType eDT = static_cast<DamageType>(nCurrType);
				if (((*pcStrings) + 1) < cMaxStrings)
				{
					LTStrCpy( aszStrings[(*pcStrings)++], DamageTypeToString(eDT), cMaxStringLength );
				}
			}

			return LT_OK;
		}

		//unhandled type
		return LT_ERROR;
	}
};

//------------------------------------------------------------------------------
// PositionalDamage Object
//------------------------------------------------------------------------------
LINKFROM_MODULE( PositionalDamage );

BEGIN_CLASS(PositionalDamage)
	ADD_REALPROP(Damage, -1.0f, "This is the amount of damage that should be applied to the target object. A negative value indicates that an infinite amount of damage should be applied")
	ADD_STRINGPROP_FLAG(DamageType, "UNSPECIFIED", PF_STATICLIST, "Indicates the type of damage that should be applied.")
	ADD_STRINGPROP_FLAG(Object0, "", PF_OBJECTLINK, "The name of the object that should have the damage applied to it when this object is activated")
	ADD_STRINGPROP_FLAG(Object1, "", PF_OBJECTLINK, "The name of the object that should have the damage applied to it when this object is activated")
END_CLASS_FLAGS_PLUGIN(PositionalDamage, GameBase, 0, PositionalDamage_Plugin, "A point light source that emits light equally in all directions")

CMDMGR_BEGIN_REGISTER_CLASS( PositionalDamage )
	//			Message		Num Params	Validation FnPtr		Syntax
	ADD_MESSAGE( ON,	1,	NULL,	MSG_HANDLER( PositionalDamage, HandleOnMsg ),	"ON", "When this object receives an ON command it will apply the damage to the listed objects.", "msg PositionalDamage ON" )
CMDMGR_END_REGISTER_CLASS( PositionalDamage, GameBase )

PositionalDamage::PositionalDamage()
{
	m_fDamage		= -1.0f;
	m_eDamageType	= DT_UNSPECIFIED;
}

PositionalDamage::~PositionalDamage()
{
}

//called to handle loading in of the scale properties
void PositionalDamage::ReadProperties(const GenericPropList *pProps)
{
	m_fDamage = pProps->GetReal("Damage", m_fDamage);
	m_eDamageType = StringToDamageType(pProps->GetString("DamageType", ""));

	for(uint32 nCurrObject = 0; nCurrObject < knNumObjects; nCurrObject++)
	{
		char pszPropName[32];
		LTSNPrintF(pszPropName, LTARRAYSIZE(pszPropName), "Object%d", nCurrObject);
		m_sObjects[nCurrObject] = pProps->GetString(pszPropName, "");
	}
}

//handles events sent from the engine. These are primarily messages
//associated with saving and loading
uint32 PositionalDamage::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	switch(messageID)
	{
	case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE ||
				fData == PRECREATE_STRINGPROP ||
				fData == PRECREATE_NORMAL)
			{
				//let the child object handle loading in the necessary properties
				ReadProperties(&((ObjectCreateStruct*)pData)->m_cProperties);
			}
			break;
		}

	case MID_INITIALUPDATE:
		{
			SetNextUpdate(UPDATE_NEVER);
			break;
		}

	case MID_LOADOBJECT:
		{
			Load((ILTMessage_Read*)pData, (uint32)fData);
			break;
		}

	case MID_SAVEOBJECT:
		{
			Save((ILTMessage_Write*)pData, (uint32)fData);			
			break;
		}

	default : 
		break;
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
}

void PositionalDamage::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	pMsg->Writefloat(m_fDamage);
	pMsg->Writeuint8(m_eDamageType);

	for(uint32 nCurrObject = 0; nCurrObject < knNumObjects; nCurrObject++)
		pMsg->WriteString(m_sObjects[nCurrObject].c_str());
}

void PositionalDamage::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	m_fDamage		= pMsg->Readfloat();
	m_eDamageType	= (DamageType)pMsg->Readuint8();

	for(uint32 nCurrObject = 0; nCurrObject < knNumObjects; nCurrObject++)
	{
		char pszBuffer[256];
		pMsg->ReadString(pszBuffer, LTARRAYSIZE(pszBuffer));
		m_sObjects[nCurrObject] = pszBuffer;
	}
}

//---------------------------------------------------------------------------------------------
// PositionalDamage message handlers
//---------------------------------------------------------------------------------------------

//handles the ON command which applies the damage to the listed objects
void PositionalDamage::HandleOnMsg( HOBJECT /*hSender*/, const CParsedMsg &crParsedMsg )
{
	//grab the position information on this object
	LTRigidTransform tObjTrans;
	g_pLTServer->GetObjectTransform(m_hObject, &tObjTrans);

	//determine the actual damage we should apply
	static const float kfInfiniteDamage = 1000000000.0f;
	float fDamage = (m_fDamage < 0.0f) ? kfInfiniteDamage : m_fDamage;

	//now run through our object list
	for(uint32 nCurrObject = 0; nCurrObject < knNumObjects; nCurrObject++)
	{
		//skip over empty strings
		if(m_sObjects[nCurrObject].empty())
			continue;

		//now grab the object that the name refers to
		ObjArray<HOBJECT, 1> ObjList;
		uint32 nNumFound;
		g_pLTServer->FindNamedObjects(m_sObjects[nCurrObject].c_str( ), ObjList, &nNumFound);
		if(nNumFound < 1)
			continue;

		//now we need to apply the damage to this object
		DamageStruct damage;
		damage.fDamage = fDamage;
		damage.eType = m_eDamageType;
		damage.SetPositionalInfo(tObjTrans.m_vPos, tObjTrans.m_rRot.Forward());

		damage.DoDamage(NULL, ObjList.GetObject(0));
	}	
}




