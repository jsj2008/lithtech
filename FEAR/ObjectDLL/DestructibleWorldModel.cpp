#include "Stdafx.h"
#include "DestructibleWorldModel.h"
#include "ShatterTypeDB.h"

//value indicating an invalid blind object data index
#define INVALID_BLIND_OBJECT_DATA		-1

//Command manager support
CMDMGR_BEGIN_REGISTER_CLASS( CDestructibleWorldModel )
CMDMGR_END_REGISTER_CLASS( CDestructibleWorldModel, CDestructibleModel )


//------------------------------------------------------------------------------------------------
//CDestructibleWorldModelPlugin

CDestructibleWorldModelPlugin::CDestructibleWorldModelPlugin()
{
}

CDestructibleWorldModelPlugin::~CDestructibleWorldModelPlugin()
{
}

//called by WorldEdit whenever the a string list associated with this object needs to be filled in
LTRESULT CDestructibleWorldModelPlugin::PreHook_EditStringList(	const char* szRezPath, const char* szPropName, 
																char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, 
																const uint32 cMaxStringLength)
{
	//determine if this is a property that we care about
	if(LTStrIEquals(szPropName, "ShatterType"))
	{
		CShatterTypeDBPlugin::Instance().PopulateStringList(aszStrings, pcStrings, cMaxStrings, cMaxStringLength);
		return LT_OK;
	}

	//not one of our strings, let the base class handle it
	return CDestructibleModelPlugin::PreHook_EditStringList(szRezPath, szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength);
}


//------------------------------------------------------------------------------------------------
//CDestructibleWorldModel

CDestructibleWorldModel::CDestructibleWorldModel() :
	CDestructibleModel(),
	m_hShatterInfo(NULL),
	m_nBlindObjectData(INVALID_BLIND_OBJECT_DATA)
{
}

CDestructibleWorldModel::~CDestructibleWorldModel()
{
}

//handle serialization to and from a message buffer
void CDestructibleWorldModel::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	//we don't need to let the base class handle the save since that is handled by falling back to the
	//parent in the message handler

	//now we need to save our custom data
	SAVE_HRECORD(m_hShatterInfo);
	pMsg->Writeint32(m_nBlindObjectData);
}

void CDestructibleWorldModel::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	//we don't need to let the base class handle the load since that is handled by falling back to the
	//parent in the message handler

	//now we need to load our custom data
	LOAD_HRECORD( m_hShatterInfo, CShatterTypeDB::Instance().GetShatterCategory());
	m_nBlindObjectData	= pMsg->Readint32();
}

//handle messages that we receive from the engine
uint32 CDestructibleWorldModel::EngineMessageFn(LPBASECLASS pObject, uint32 messageID, void *pData, float fData)
{
	switch (messageID)
	{
	case MID_PRECREATE:
		{
			if ((fData == PRECREATE_WORLDFILE) || (fData == PRECREATE_STRINGPROP))
			{
				ReadProp(&((ObjectCreateStruct*)pData)->m_cProperties);
			}
		}
		break;

	case MID_SAVEOBJECT:
		{
			//now allow for our base class to handle it's message handling
			uint32 nRet = CDestructibleModel::EngineMessageFn(pObject, messageID, pData, fData);

			Save((ILTMessage_Write*)pData, (uint32)fData);

			return nRet;
		}
		break;

	case MID_LOADOBJECT:
		{
			//now allow for our base class to handle it's message handling
			uint32 nRet = CDestructibleModel::EngineMessageFn(pObject, messageID, pData, fData);

			Load((ILTMessage_Read*)pData, (uint32)fData);

			return nRet;
		}
		break;

	default : break;
	}

	//now allow for our base class to handle it's message handling
	return CDestructibleModel::EngineMessageFn(pObject, messageID, pData, fData);
}

//called to read in properties associated with the object
bool CDestructibleWorldModel::ReadProp(const GenericPropList *pProps)
{
	//the base class will load it's properties by us passing up the engine message to it, so no need
	//to call up to the base class here.

	//and now load in our properties
	const char* pszShatterType = pProps->GetString("ShatterType", "None");
	m_hShatterInfo = CShatterTypeDB::Instance().GetShatterType(pszShatterType);

	m_nBlindObjectData = (int32)(pProps->GetReal("BlindObjectIndex", INVALID_BLIND_OBJECT_DATA) + 0.5f);

	return true;
}

//called when the object is destroyed and should be shattered
bool CDestructibleWorldModel::ShatterWorldModel(const LTVector& vPos, const LTVector& vDir)
{
	//make sure that we have a valid object before sending it to the client
	if((m_nBlindObjectData == INVALID_BLIND_OBJECT_DATA) || !m_hShatterInfo)
		return false;

	//write a message that can be sent to the client that contains the impact information
	//and the information necessary to create the shattering effect
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_SHATTER_WORLD_MODEL);

	//write out the information needed to get to the blind object data that holds the geometry for
	//shattering
	cMsg.Writeuint32((uint32)m_nBlindObjectData);

	//and also write out the database record that controls our properties dictating how to shatter
	cMsg.WriteDatabaseRecord(g_pLTDatabase, m_hShatterInfo);


	//we need to send down the transform of this world model because we can't really rely on the
	//world model being in existance on the client by the time we handle this message since it has
	//been after all destroyed.
	LTRigidTransform tObjTrans;
	g_pLTServer->GetObjectTransform(m_hObject, &tObjTrans);
	cMsg.WriteLTVector(tObjTrans.m_vPos);
	cMsg.WriteLTRotation(tObjTrans.m_rRot);

	//and write out the position and direction of the impact so we can do location specific breaking
	cMsg.WriteLTVector(vPos);
	cMsg.WriteLTVector(vDir);	

	g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);

	//success
	return true;
}

//override the base class's when the object is destroyed so we can create our shatter effect
void CDestructibleWorldModel::HandleObjectDestroyed(const DamageStruct& DamageInfo)
{
	if(DamageInfo.HasPositionalInfo())
	{
		//the damage we took was positional, so use that data to shatter our object
		ShatterWorldModel(DamageInfo.GetDamagePos(), DamageInfo.GetDamageDir());
	}
	else
	{
		//our damage wasn't positional, so just shatter from the center of the model in the direction
		//opposite the forward facing of the object
		LTRigidTransform tObjTrans;
		g_pLTServer->GetObjectTransform(m_hObject, &tObjTrans);

		ShatterWorldModel(tObjTrans.m_vPos, -tObjTrans.m_rRot.Forward());
	}

	//now let our base class handle the destruction as well
	CDestructibleModel::HandleObjectDestroyed(DamageInfo);
}
