//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
#include "bdefs.h"
#include "ltbasedefs.h"

#include "servermgr.h"
#include "iltphysics.h"
#include "smoveabstract.h"
#include "s_client.h"
#include "dhashtable.h"
#include "collision.h"
#include "serverde_impl.h"
#include "animtracker.h"
#include "s_object.h"
#include "impl_common.h"
#include "stringmgr.h"
#include "s_net.h"
#include "server_extradata.h"
#include "geomroutines.h"
#include "genericprop_setup.h"
#include "interlink.h"
#include "conparse.h"
#include "sysstreamsim.h"
#include "game_serialize.h"
#include "server_interface.h"
#include "sysdebugging.h"

#include "strtools.h"

#include "ltobjref.h"
#include "ltobjectcreate.h"

#include "ltmessage_server.h"

#include "fullintersectline.h"

//------------------------------------------------------------------
//------------------------------------------------------------------
// Holders and their headers.
//------------------------------------------------------------------
//------------------------------------------------------------------

//server file mgr.
#include "server_filemgr.h"
static IServerFileMgr *server_filemgr;
define_holder(IServerFileMgr, server_filemgr);

//server console state
#include "server_consolestate.h"
#include "concommand.h"
static IServerConsoleState *console_state;
define_holder(IServerConsoleState, console_state);

//IWorldServerBSP holder
#include "world_server_bsp.h"
static IWorldServerBSP *world_bsp_server;
define_holder(IWorldServerBSP, world_bsp_server);

//IWorldSharedBSP holder
#include "world_shared_bsp.h"
static IWorldSharedBSP *world_bsp_shared;
define_holder(IWorldSharedBSP, world_bsp_shared);

//IWorldBlindObjectData holder
#include "world_blind_object_data.h"
static IWorldBlindObjectData *world_blind_object_data;
define_holder(IWorldBlindObjectData, world_blind_object_data);

//IServerShell game server shell object.
#include "iservershell.h"
static IServerShell *i_server_shell;
define_holder(IServerShell, i_server_shell);

//ILTModel interface server implementation
#include "iltmodel.h"
static ILTModel *ilt_model_server;
define_holder_to_instance(ILTModel, ilt_model_server, Server);

//ILTCommon game interface server version
#include "iltcommon.h"
static ILTCommon *ilt_common_server;
define_holder_to_instance(ILTCommon, ilt_common_server, Server);

//ILTTransform game interface.
#include "ilttransform.h"
static ILTTransform *ilt_transform;
define_holder(ILTTransform, ilt_transform);

//ILTSoundMgr game interface server version.
#include "iltsoundmgr.h"
static ILTSoundMgr *ilt_soundmgr_server;
define_holder_to_instance(ILTSoundMgr, ilt_soundmgr_server, Server);

//ILTPhysics game interface
#include "iltphysics.h"
static ILTPhysics *ilt_physics_server;
define_holder_to_instance(ILTPhysics, ilt_physics_server, Server);


extern uint32 g_dwSaveFileVersion;


#define MAX_CLASS_HEIRARCHY_LEN 256

class SphereFindStruct {
public:

	ObjectList  *m_pSphereTouchList;
	const LTVector	*m_pSphereTouchPos;
	float	   m_SphereTouchRadius;
};

class BoxFindStruct {
public:

	ObjectList	*m_pBoxTouchList;
};

class GPCStruct {
public:

	HOBJECT	 *m_pList;
	uint32	  m_MaxListSize;
	uint32	  m_CurListSize;
	LTVector	m_Point;
};


extern bool ServerIntersectSegment(IntersectQuery *pQuery, IntersectInfo *pInfo);


// --------------------------------------------------------------- //
// Helper functions.
// --------------------------------------------------------------- //

extern float g_DebugMaxDims;

extern uint32 g_SphereFindTicks, g_SphereFindCount;

// Forward definition...
static ObjectList* si_FindObjectsTouchingSphere(const LTVector *pPosition, float radius);

bool si_AnyPointsOnFrontside(LTVector *pPts, int nPts, LTPlane *pPlane)
{
	while (nPts--)
	{
		if (pPlane->DistTo(*pPts) > 0.0f)
		{
			return true;
		}

		++pPts;
	}

	return false;
}


bool si_AnyPointsOnBackside(LTVector *pPts, int nPts, LTPlane *pPlane)
{
	while (nPts--)
	{
		if (pPlane->DistTo(*pPts) < 0.0f)
		{
			return true;
		}

		++pPts;
	}

	return false;
}


const GenericProp* _FindProp(const char *pPropName)
{
	if (!g_pServerMgr->m_pCurOCS)
		return LTNULL;

	return g_pServerMgr->m_pCurOCS->m_cProperties.GetProp(pPropName);
}

bool si_GetPointShade(const LTVector *pPoint, LTVector *pColor)
{
	LTRGBColor rgb;

	w_DoLightLookup(world_bsp_shared->LightTable(), pPoint, &rgb);

	pColor->x = rgb.rgb.r;
	pColor->y = rgb.rgb.g;
	pColor->z = rgb.rgb.b;
	return true;
}

// Callback for GetPointContainers.
void GPCCallback(WorldTreeObj *pObj, void *pUser);



// --------------------------------------------------------------- //
// CLTServer implementation.
// --------------------------------------------------------------- //

class CLTServer : public ILTServer
{
public:
	declare_interface(CLTServer);

	CLTServer();
	~CLTServer();

	void SetupFunctionPointers();

	//////////////////////////////////////////////////////////////////////////////
	// ILTCSBase implementation

	//sub-interface acessors from ILTCSBase
	virtual ILTCommon *Common();
	virtual ILTPhysics *Physics();
	virtual ILTModel *GetModelLT();
	virtual ILTTransform *GetTransformLT();
	virtual ILTSoundMgr *SoundMgr();

	virtual LTRESULT OpenFile(const char *pFilename, ILTStream **pStream);
	virtual LTRESULT CopyFile(const char *pszSourceFile, const char *pszDestFile);
	virtual void SetModelAnimation(HOBJECT hObj, HMODELANIM hAnim);
	virtual HMODELANIM GetModelAnimation(HOBJECT hObj);
	virtual void SetModelLooping(HOBJECT hObj, bool bLoop);
	virtual bool GetModelLooping(HOBJECT hObj);
	virtual LTRESULT ResetModelAnimation(HOBJECT hObj);
	virtual uint32 GetModelPlaybackState(HOBJECT hObj);
	virtual HMODELANIM GetAnimIndex(HOBJECT hObj, const char *pAnimName);
	virtual void CPrint(const char *pMsg, ...);
	virtual uint32 GetPointContainers(const LTVector *pPoint, HOBJECT *pList, uint32 maxListSize);
	virtual bool GetContainerCode(HOBJECT hObj, uint16 *pCode);
	virtual uint32 GetObjectContainers(HOBJECT hObj,
		HOBJECT *pContainerList, uint32 maxListSize);
	virtual uint32 GetContainedObjects(HOBJECT hContainer,
		HOBJECT *pObjectList, uint32 maxListSize);
	virtual HSTRING FormatString(int messageCode, ...);
	virtual HSTRING CopyString(HSTRING hString);
	virtual HSTRING CreateString(const char *pString);
	virtual void FreeString(HSTRING hString);
	virtual bool CompareStrings(HSTRING hString1, HSTRING hString2);
	virtual bool CompareStringsUpper(HSTRING hString1, HSTRING hString2);
	virtual const char* GetStringData(HSTRING hString);
	virtual float GetVarValueFloat(HCONVAR hVar);
	virtual const char* GetVarValueString(HCONVAR hVar);
	virtual LTFLOAT GetTime();
	virtual LTFLOAT GetFrameTime();
	virtual LTRESULT GetSourceWorldOffset(LTVector& vOffset);

	virtual LTRESULT RemoveObject(HOBJECT hObject);

	virtual LTRESULT SendTo(ILTMessage_Read *pMsg, const char *sAddr, uint16 port);
	virtual LTRESULT StartPing(const char *pAddr, uint16 nPort, uint32 *pPingID);
	virtual LTRESULT GetPingStatus(uint32 nPingID, uint32 *pStatus, uint32 *pLatency);
	virtual LTRESULT RemovePing(uint32 nPingID);

	virtual LTRESULT GetTextureEffectVarID(const char *pName, uint32 nStage, uint32 *pResult) const;

	virtual LTRESULT GetLightGroupID(const char *pName, uint32 *pResult) const;
	virtual LTRESULT GetOccluderID(const char *pName, uint32 *pResult) const;

	virtual LTRESULT GetBlindObjectData(uint32 nNum, uint32 nId, uint8*& pData, uint32& nSize);
	virtual LTRESULT FreeBlindObjectData(uint32 nNum, uint32 nId);

	virtual LTRESULT LinkObjRef(HOBJECT hObj, LTObjRef *pRef);

	virtual void BreakInterObjectLink( HOBJECT hOwner, HOBJECT hLinked );
	virtual void BreakInterObjectLink( HOBJECT hObj, HOBJECT hLinked, bool bNotify );

	virtual LTRESULT OpenMemoryStream(ILTStream **pStream, uint32 nCacheSize);

	//////////////////////////////////////////////////////////////////////////////
	// ILTServer implementation

	virtual LTRESULT GetNumClassProps(const HCLASS hClass, uint32 &count);
	virtual LTRESULT GetClassProp(const HCLASS hClass,
		const uint32 iProp, ClassPropInfo &info);
	virtual LTRESULT GetClassName(const HCLASS hClass,
		char *pName, uint32 maxNameBytes);
	virtual LTRESULT RemoveObject(const HCLASS hClass, LPBASECLASS pObject);
	virtual LTRESULT GetClientPing(HCLIENT hClient, float &ping);
	virtual LTRESULT GetClientAddr(HCLIENT hClient, uint8 pAddr[4], uint16 *pPort);
	virtual LTRESULT GetNetFlags(HOBJECT hObj, uint32 &flags);
	virtual LTRESULT SetNetFlags(HOBJECT pObj, uint32 flags);
	virtual LTRESULT GetHPolyObject(const HPOLY hPoly, HOBJECT &hObject);
	virtual bool GetClientData(HCLIENT hClient, uint8* pData, int& maxLen);
	virtual bool SetClientData(HCLIENT hClient, uint8 const* pData, int len);

	virtual LTRESULT FindNamedObjects(const char *pName, BaseObjArray<HOBJECT> &objArray, uint32 *nTotalFound);

	virtual LTRESULT FindWorldModelObjectIntersections(HOBJECT hObj,
		const LTVector &vNewPos, const LTRotation &rNewRot, BaseObjArray<HOBJECT> &objArray);
	virtual LTRESULT GetWorldBox(LTVector &min, LTVector &max);
	virtual LTRESULT ThreadLoadFile(const char *pFilename, uint32 type);
	virtual LTRESULT UnloadFile(const char *pFilename, uint32 type);

	virtual LTRESULT MoveObject(HOBJECT hObj, const LTVector *pNewPos);
	virtual LTRESULT GetStandingOn(HOBJECT hObj, CollisionInfo *pInfo);

	virtual LTRESULT GetObjectPos(HOBJECT hObj, LTVector* pPos);
	virtual LTRESULT GetObjectRotation(HOBJECT hObj, LTRotation* pRot);
	virtual LTRESULT GetObjectName(HOBJECT pObj, char *pBuf, uint32 bufSize);

	virtual LTRESULT SendToClient(ILTMessage_Read *pMsg, HCLIENT hSendTo, uint32 flags);
	virtual LTRESULT SendToObject(ILTMessage_Read *pMsg, HOBJECT hSender, HOBJECT hSendTo, uint32 flags);
	virtual LTRESULT SetObjectSFXMessage(HOBJECT hObject, ILTMessage_Read *pMsg);
	virtual LTRESULT SendSFXMessage(ILTMessage_Read *pMsg, const LTVector &pos, uint32 flags);
	virtual LTRESULT SendToServer(ILTMessage_Read *pMsg, HOBJECT hSender, uint32 flags);
    virtual LTRESULT GetSaveFileVersion( uint32& nSaveFileVersion ) { nSaveFileVersion = g_dwSaveFileVersion; return LT_OK; }


// Internal.
protected:

	void SendFileIOMessage(uint8 fileType, uint8 msgID, uint16 fileID, bool bTellLocal);

	LTRESULT ThreadLoadTexture(const char *pFilename);
	LTRESULT UnloadTexture(const char *pFilename);

};

//allocate our implementation class, and put it in the database.
define_interface(CLTServer, ILTServer);

//we have delayed initialization, so save a pointer to the implementation class object.
static CLTServer *ilt_server_imp = NULL;

void CreateLTServerDE()
{
	ilt_server_imp->SetupFunctionPointers();
}

CLTServer::CLTServer()
{
	//save the pointer to the implementation class.
	ilt_server_imp = this;
}

CLTServer::~CLTServer()
{

}

ILTCommon *CLTServer::Common()
{
	return ilt_common_server;
}

ILTPhysics *CLTServer::Physics()
{
	return ilt_physics_server;
}

ILTModel *CLTServer::GetModelLT()
{
	return ilt_model_server;
}

ILTTransform *CLTServer::GetTransformLT()
{
	return ilt_transform;
}

ILTSoundMgr *CLTServer::SoundMgr()
{
	return ilt_soundmgr_server;
}

LTRESULT CLTServer::GetNumClassProps(const HCLASS hClass, uint32 &count)
{
	FN_NAME(CLTServer::GetClassDef);

	count = 0;
	CHECK_PARAMS2(hClass);

	CClassData *pClassData = (CClassData*)hClass;

	// Include base classes.
	ClassDef *pCurClass = pClassData->m_pClass;
	while (pCurClass)
	{
		count += (uint32)pCurClass->m_nProps;
		pCurClass = pCurClass->m_ParentClass;
	}

	return LT_OK;
}

LTRESULT CLTServer::GetClassProp(const HCLASS hClass,
	const uint32 iProp, ClassPropInfo &info)
{
	FN_NAME(CLTServer::GetClassDef);

	info.m_PropName[0] = 0;
	CHECK_PARAMS2(hClass);

	CClassData *pClassData = (CClassData*)hClass;

	// Get the heirarchy into an array.
	uint32 nClasses = 0;
	ClassDef *classes[MAX_CLASS_HEIRARCHY_LEN];
	for (ClassDef *pCurClass = pClassData->m_pClass; pCurClass; pCurClass = pCurClass->m_ParentClass)
	{
		classes[nClasses] = pCurClass;
		nClasses++;
	}

	// Go through our array and find the right property.
	uint32 iCurProp = 0;
	for (uint32 i = 0; i < nClasses; i++)
	{
		ClassDef *pClass = classes[nClasses - i - 1];

		// Is it in this class?
		if (iProp >= iCurProp && iProp < (iCurProp + pClass->m_nProps))
		{
			PropDef *pPropDef = &pClass->m_Props[iProp - iCurProp];
			SAFE_STRCPY(info.m_PropName, pPropDef->m_PropName);
			info.m_PropType = pPropDef->m_PropType;
		}

		iCurProp += (uint32)pClass->m_nProps;
	}

	return LT_OK;
}

LTRESULT CLTServer::GetClassName(const HCLASS hClass,
	char *pName, uint32 maxNameBytes)
{
	FN_NAME(CLTServer::GetClassName);

	CHECK_PARAMS2(hClass);
	CClassData *pClassData = (CClassData*)hClass;
	LTStrCpy(pName, pClassData->m_pClass->m_ClassName, maxNameBytes);
	return LT_OK;
}

LTRESULT CLTServer::RemoveObject(const HCLASS hClass, LPBASECLASS pObject)
{
	if (!hClass || !pObject)
		return LT_ERROR;

	CClassData *pClassData = (CClassData*)hClass;

	// Only class-only objects, please
	if ((pClassData->m_pClass->m_ClassFlags & CF_CLASSONLY) == 0)
		return LT_ERROR;

	// Free it
	sm_FreeObjectOfClass(pClassData->m_pClass, pObject);

	return LT_OK;
}

LTRESULT CLTServer::GetClientPing(HCLIENT hClient, float &ping)
{
	CHECK_PARAMS(hClient, ILTServer::GetClientPing);

	Client *pClient = (Client*)hClient;
	if (pClient->m_ConnectionID)
	{
		ping = pClient->m_ConnectionID->GetPing();
		return LT_OK;
	}
	else
	{
		RETURN_ERROR(2, ILTPhysics::GetPing, LT_NOTINITIALIZED);
	}
}

LTRESULT CLTServer::GetClientAddr(HCLIENT hClient, uint8 pAddr[4], uint16 *pPort)
{
	CHECK_PARAMS(hClient, ILTServer::GetClientAddr);
	CHECK_PARAMS(pAddr, ILTServer::GetClientAddr);
	CHECK_PARAMS(pPort, ILTServer::GetClientAddr);

	Client *pClient = (Client*)hClient;

	if (!pClient->m_ConnectionID->GetIPAddress(pAddr, pPort))
	{
		pAddr[0] = 0;
		pAddr[1] = 0;
		pAddr[2] = 0;
		pAddr[3] = 0;
		*pPort = 0;
	}

	return LT_OK;
}

bool CLTServer::GetClientData(HCLIENT hClient, uint8* pData, int& maxLen)
{
	if( !hClient )
		return false;

	Client* pClient = ( Client* )hClient;

	uint32 nCopyLength = Min( pClient->m_ClientDataLen, ( uint32 )maxLen );

	if( pData )
	{
		memcpy( pData, pClient->m_pClientData, nCopyLength );
	}

	maxLen = nCopyLength;

	return true;
}

bool CLTServer::SetClientData( HCLIENT hClient, uint8 const* pData, int len)
{
	if( !hClient )
		return false;

	Client* pClient = ( Client* )hClient;

	// Clear out the old clientdata.
	pClient->m_ClientDataLen = 0;
	if (pClient->m_pClientData)
	{
		delete[] pClient->m_pClientData;
		pClient->m_pClientData = NULL;
	}

	// If they didn't have anything new, leave it cleared out.
    if( !len || !pData )
		return true;

    LT_MEM_TRACK_ALLOC(pClient->m_pClientData = new char[len],LT_MEM_TYPE_NETWORKING);
	memcpy( pClient->m_pClientData, pData, len );
	pClient->m_ClientDataLen = len;

	return true;
}

LTRESULT CLTServer::GetNetFlags(HOBJECT hObj, uint32 &flags)
{
	CHECK_PARAMS(hObj, GetNetFlags);
	flags = hObj->sd->m_NetFlags;
	return LT_OK;
}

LTRESULT CLTServer::SetNetFlags(HOBJECT pObj, uint32 flags)
{
	CHECK_PARAMS(pObj, SetNetFlags);

	// Filter out unwanted ones.
	if (pObj->m_ObjectType != OT_MODEL)
		flags &= ~NETFLAG_ANIMUNGUARANTEED;

	pObj->sd->m_NetFlags = (uint16)flags;
	return LT_OK;
}

LTRESULT CLTServer::GetHPolyObject(const HPOLY hPoly, HOBJECT &hObject)
{
	FN_NAME(CLTServer::GetHPolyObject);

	WorldData *pWorldData = world_bsp_server->GetWorldDataFromHPoly(hPoly);
	if (!pWorldData)
	{
		RETURN_ERROR(5, CLTServer::GetHPolyObject, LT_NOTFOUND);
	}

	// LAME but unless it needs to get faster..  Look for a WorldModel
	// that uses this world data.
	LTLink *pListHead = &g_pServerMgr->m_ObjectMgr.m_ObjectLists[OT_WORLDMODEL].m_Head;
	for (LTLink *pCur = pListHead->m_pNext; pCur != pListHead; pCur = pCur->m_pNext)
	{
		WorldModelInstance *pInst = (WorldModelInstance*)pCur->m_pData;

		if (pInst->GetOriginalBsp() == pWorldData->OriginalBSP())
		{
			hObject = pInst;
			return LT_OK;
		}
	}

	ERR(2, LT_INVALIDPARAMS);
}

LTRESULT CLTServer::FindNamedObjects(const char *pName,
								 BaseObjArray<HOBJECT> &objArray,
								 uint32 *nTotalFound)
{
	FN_NAME(CLTServer::FindNamedObjects);

	CHECK_PARAMS2(pName);

	// Make sure array is clean...
	objArray.Reset();

	// Go thru the hash table and find all matches.
	uint32 keyLen = strlen(pName) + 1;
	HHashElement *hElement = hs_FindElement(g_pServerMgr->m_hNameTable, pName, keyLen);
	uint32 tmpTotalFound = 0;
	while (hElement)
	{
		LTObject *pObj = (LTObject*)hs_GetElementUserData(hElement);

		if (!(pObj->m_InternalFlags & IFLAG_OBJECTGOINGAWAY))
		{
			objArray.AddObject(ServerObjToHandle(pObj));
			tmpTotalFound++;
		}

		hElement = hs_FindNextElement(g_pServerMgr->m_hNameTable, hElement, pName, keyLen);
	}

	if (nTotalFound)
		*nTotalFound = tmpTotalFound;

	return LT_OK;
}

LTRESULT CLTServer::FindWorldModelObjectIntersections(HOBJECT hObj,
	const LTVector &vNewPos, const LTRotation &rNewRot, BaseObjArray<HOBJECT> &objArray)
{
	FN_NAME(CLTServer::FindObjectIntersections);
	CHECK_PARAMS2(hObj);

	LTObject* pObj = HandleToServerObj(hObj);

	// Make sure this is a world model...

	if (!pObj->HasWorldModel())
	{
		RETURN_ERROR(1, ILTPhysics::FindWorldModelObjectIntersections, LT_ERROR);
	}


	// Update the world model's position/rotation, saving the old
	// position/rotation...

	WorldModelInstance* pWorldModel = pObj->ToWorldModel();

	LTVector vOldPos = pWorldModel->GetPos();
	LTRotation rOldRot = pWorldModel->m_Rotation;

	pWorldModel->SetPos(vNewPos);
	pWorldModel->m_Rotation = rNewRot;

	RetransformWorldModel(pWorldModel);


	// Find all the objects that intersect the world model in its new
	// position...

	ObjectList* pList = si_FindObjectsTouchingSphere(&vNewPos,
		pWorldModel->GetRadius());

	if (pList)
	{
		ObjectLink* pLink = pList->m_pFirstLink;

		while (pLink)
		{
			LTObject* pTestObj = HandleToServerObj(pLink->m_hObject);

            if (pTestObj && pTestObj != pObj)
            {
                if (DoesBoxIntersectBSP(pWorldModel->m_pValidBsp->GetRootNode(),
                    pTestObj->m_MinBox, pTestObj->m_MaxBox,
					pWorldModel->m_ObjectType == OT_CONTAINER))
                {
                    // If the object isn't going away, add it to the list...

					if (!(pTestObj->m_InternalFlags & IFLAG_OBJECTGOINGAWAY))
					{
						objArray.AddObject(pLink->m_hObject);
					}
				}
			}

			pLink = pLink->m_pNext;
		}

		si_RelinquishList(pList);
	}


	// Reset the world model's position/rotation...

	pWorldModel->SetPos(vOldPos);
	pWorldModel->m_Rotation = rOldRot;

	RetransformWorldModel(pWorldModel);

	return LT_OK;
}

LTRESULT CLTServer::GetWorldBox(LTVector &min, LTVector &max)
{
	if (!world_bsp_server->IsLoaded())
	{
		RETURN_ERROR(2, ILTPhysics::GetWorldBox, LT_NOTINITIALIZED);
	}

	//get the world box from the server bsp world.
	world_bsp_server->GetWorldBox(min, max);

	return LT_OK;
}

LTRESULT CLTServer::ThreadLoadFile(const char *pFilename, uint32 type)
{
	CHECK_PARAMS(pFilename, ILTPhysics::ThreadLoadFile);

	if (type == FT_MODEL)
	{
		RETURN_ERROR(1,CLTServer::TreadLoadFile, LT_UNSUPPORTED );
	}
	else if (type == FT_TEXTURE)
	{
		return ThreadLoadTexture(pFilename);
	}
	else
	{
		RETURN_ERROR(1, ILTPhysics::ThreadLoadFile, LT_UNSUPPORTED);
	}
}

LTRESULT CLTServer::UnloadFile(const char *pFilename, uint32 type)
{
	CHECK_PARAMS(pFilename, ILTPhysics::UnloadFile);

	if (type == FT_MODEL)
	{
		RETURN_ERROR(1,CLTServer::UnloadFile, LT_UNSUPPORTED );
	}
	else if (type == FT_TEXTURE)
	{
		return UnloadTexture(pFilename);
	}
	else
	{
		RETURN_ERROR(1, ILTPhysics::UnloadFile, LT_UNSUPPORTED);
	}
}

LTRESULT CLTServer::OpenFile(const char *pFilename, ILTStream **pStream)
{
	FN_NAME(ILTPhysics::OpenFile);

	CHECK_PARAMS2(pFilename && pStream);

	*pStream = server_filemgr->OpenFile(pFilename);
	if (*pStream)
	{
		return LT_OK;
	}
	else
	{
		ERR(2, LT_MISSINGFILE);
	}
}

LTRESULT CLTServer::CopyFile(const char *pszSourceFile, const char *pszDestFile)
{
	return server_filemgr->CopyFile(pszSourceFile, pszDestFile);
}

void CLTServer::SetModelAnimation(HOBJECT hObj, HMODELANIM hAnim)
{
	ilt_model_server->SetCurAnim(hObj, MAIN_TRACKER, hAnim);
}

HMODELANIM CLTServer::GetModelAnimation(HOBJECT hObj)
{
	HMODELANIM iAnim = 0;
	LTRESULT ret = ilt_model_server->GetCurAnim(hObj, MAIN_TRACKER, iAnim);
	return ( ret == LT_OK ) ? iAnim : (uint32)-1;

}

void CLTServer::SetModelLooping(HOBJECT hObj, bool bLoop)
{
	ilt_model_server->SetLooping(hObj, MAIN_TRACKER, bLoop);
}

bool CLTServer::GetModelLooping(HOBJECT hObj)
{
	return ilt_model_server->GetLooping(hObj, MAIN_TRACKER) == LT_YES;
}

LTRESULT CLTServer::ResetModelAnimation(HOBJECT hObj)
{
	return ilt_model_server->ResetAnim(hObj, MAIN_TRACKER);
}

uint32 CLTServer::GetModelPlaybackState(HOBJECT hObj)
{
	uint32 state = 0;
	LTRESULT dResult = ilt_model_server->GetPlaybackState(hObj, MAIN_TRACKER, state);
	return state;
}

HMODELANIM CLTServer::GetAnimIndex(HOBJECT hObj, const char *pAnimName)
{
	return ic_GetAnimIndex(hObj, pAnimName);
}

void CLTServer::CPrint(const char *pMsg, ...)
{
	va_list marker;

	static const uint32 knBufferSize = 512;
	char msg[knBufferSize];

	va_start(marker, pMsg);
	LTVSNPrintF( msg, knBufferSize, pMsg, marker );
	va_end(marker);

	dsi_ConsolePrint(msg);
}

uint32 CLTServer::GetPointContainers(const LTVector *pPoint, HOBJECT *pList, uint32 maxListSize)
{
	GPCStruct theStruct;

	theStruct.m_pList = pList;
	theStruct.m_MaxListSize = maxListSize;
	theStruct.m_CurListSize = 0;
	theStruct.m_Point = *pPoint;

	world_bsp_server->ServerTree()->FindObjectsOnPoint(pPoint,
		GPCCallback, &theStruct);

	return theStruct.m_CurListSize;
}


bool CLTServer::GetContainerCode(HOBJECT hObj, uint16 *pCode)
{
	LTObject *pObj;

	if (!hObj)
		return false;

	pObj = HandleToServerObj(hObj);
	if (pObj->m_ObjectType != OT_CONTAINER)
		return false;

	*pCode = ToContainer(pObj)->m_ContainerCode;
	return true;
}

uint32 CLTServer::GetObjectContainers(HOBJECT hObj,
	HOBJECT *pContainerList, uint32 maxListSize)
{
	if (!hObj)
		return 0;

	uint32 count = 0;
	LTObject *pObj = HandleToServerObj(hObj);

	LTLink *pListHead = &pObj->sd->m_Links;
	LTLink *pCur = pListHead->m_pNext;
	while (pCur != pListHead)
	{
		InterLink *pInterLink = (InterLink*)pCur->m_pData;

		// If we're not the owner, than this is a link for pObj being contained.
		if (pInterLink->m_Type == LINKTYPE_CONTAINER)
		{
			if (pObj != pInterLink->m_pOwner)
			{
				if (pContainerList && (count < maxListSize))
					pContainerList[count] = (HOBJECT)pInterLink->m_pOwner;
				++count;
			}
		}

		pCur = pCur->m_pNext;
	}

	return count;
}

uint32 CLTServer::GetContainedObjects(HOBJECT hContainer,
	HOBJECT *pObjectList, uint32 maxListSize)
{
	if (!hContainer)
	{
		return 0;
	}

	uint32 count = 0;
	LTObject *pObj = HandleToServerObj(hContainer);

	LTLink *pListHead = &pObj->sd->m_Links;
	LTLink *pCur = pListHead->m_pNext;
	while (pCur != pListHead)
	{
		InterLink *pInterLink = (InterLink*)pCur->m_pData;

		// If we're the owner, than this is a link for pObj containing something.
		if (pInterLink->m_Type == LINKTYPE_CONTAINER)
		{
			if (pObj == pInterLink->m_pOwner)
			{
				if (pObjectList && (count < maxListSize))
					pObjectList[count] = (HOBJECT)pInterLink->m_pOther;
				++count;
			}
		}

		pCur = pCur->m_pNext;
	}

	return count;
}


HSTRING CLTServer::FormatString(int messageCode, ...)
{
	if (!g_pServerMgr->m_ClassMgr.m_hServerResourceModule)
		return LTNULL;

	va_list marker;
	va_start(marker, messageCode);

	uint8 *pBuf;
	int bufferLen;
	pBuf = str_FormatString(g_pServerMgr->m_ClassMgr.m_hServerResourceModule,
		messageCode, &marker, &bufferLen);

	va_end(marker);

	if (!pBuf)
		return LTNULL;

	HSTRING ret = str_CreateString(pBuf);
	str_FreeStringBuffer(pBuf);
	return ret;
}

HSTRING CLTServer::CopyString(HSTRING hString)
{
	return str_CopyString(hString);
}

HSTRING CLTServer::CreateString(const char *pString)
{
	return str_CreateStringAnsi(pString);
}

void CLTServer::FreeString(HSTRING hString)
{
	ic_FreeString(hString);
}

bool CLTServer::CompareStrings(HSTRING hString1, HSTRING hString2)
{
	return str_CompareStrings(hString1, hString2);
}

bool CLTServer::CompareStringsUpper(HSTRING hString1, HSTRING hString2)
{
	return str_CompareStringsUpper(hString1, hString2);
}

const char *CLTServer::GetStringData(HSTRING hString)
{
	return str_GetStringData(hString);
}

float CLTServer::GetVarValueFloat(HCONVAR hVar)
{
	if (!hVar)
		return 0.0f;

	return ((LTCommandVar*)hVar)->floatVal;
}

const char *CLTServer::GetVarValueString(HCONVAR hVar)
{
	if (!hVar)
		return LTNULL;

	return ((LTCommandVar*)hVar)->pStringVal;
}

LTFLOAT CLTServer::GetTime()
{
	return g_pServerMgr->m_GameTime;
}

LTFLOAT CLTServer::GetFrameTime()
{
	return g_pServerMgr->m_FrameTime;
}

LTRESULT CLTServer::GetSourceWorldOffset(LTVector& vOffset)
{
	vOffset = world_bsp_shared->GetSourceWorldOffset();
	return LT_OK;
}

LTRESULT CLTServer::RemoveObject(HOBJECT hObject)
{
	LTObject *pObject;

	if (!hObject)
		return LT_ERROR;

	pObject = HandleToServerObj(hObject);
	AddObjectToRemoveList(pObject);

	return LT_OK;
}



LTRESULT CLTServer::SendTo(ILTMessage_Read *pMsg, const char *sAddr, uint16 port)
{
	// Reference the message just in case
	CLTMsgRef_Read cMsgRef(pMsg);
	CLTMessage_Read *pServerMsg = reinterpret_cast<CLTMessage_Read *>(pMsg);

	// See if we already know who this person is...
	// (Note : This is only something we can check for if it's a numeric address...)
	if (isdigit(sAddr[0]))
	{
		uint8 nDestAddr[4];
		uint32 nTempDestAddr[4];
		sscanf(sAddr, "%d.%d.%d.%d", &nTempDestAddr[0], &nTempDestAddr[1], &nTempDestAddr[2], &nTempDestAddr[3]);
		nDestAddr[0] = (uint8)nTempDestAddr[0];
		nDestAddr[1] = (uint8)nTempDestAddr[1];
		nDestAddr[2] = (uint8)nTempDestAddr[2];
		nDestAddr[3] = (uint8)nTempDestAddr[3];
		LTLink *pClientHead = &g_pServerMgr->m_Clients.m_Head;
		for (LTLink *pCurClient = pClientHead->m_pNext; pCurClient != pClientHead; pCurClient = pCurClient->m_pNext)
		{
			Client *pClient = (Client*)pCurClient->m_pData;
			uint8 nClientAddr[4];
			uint16 nClientPort;
			pClient->m_ConnectionID->GetIPAddress(nClientAddr, &nClientPort);
			if ((nClientAddr[0] == nDestAddr[0]) &&
				(nClientAddr[1] == nDestAddr[1]) &&
				(nClientAddr[2] == nDestAddr[2]) &&
				(nClientAddr[3] == nDestAddr[3]) &&
				(nClientPort == port))
			{
				if (g_pServerMgr->m_NetMgr.SendPacket(pServerMsg->GetPacket(), pClient->m_ConnectionID))
					return LT_OK;
			}
		}
	}

	for (uint32 i = 0; i < g_pServerMgr->m_NetMgr.m_Drivers; i++)
	{
		CBaseDriver *pDriver = g_pServerMgr->m_NetMgr.m_Drivers[i];

		if (pDriver->m_DriverFlags & NETDRIVER_TCPIP)
		{
			return pDriver->SendTcpIp(pServerMsg->GetPacket(), sAddr, port);
		}
	}

	return LT_NOTINITIALIZED;
}

LTRESULT CLTServer::StartPing(const char *pAddr, uint16 nPort, uint32 *pPingID)
{
	for (uint32 i=0; i < g_pServerMgr->m_NetMgr.m_Drivers; i++)
	{
		CBaseDriver *pDriver = g_pServerMgr->m_NetMgr.m_Drivers[i];

		if (pDriver->m_DriverFlags & NETDRIVER_TCPIP)
		{
			return pDriver->StartPing(pAddr, nPort, pPingID);
		}
	}

	return LT_NOTINITIALIZED;
}

LTRESULT CLTServer::GetPingStatus(uint32 nPingID, uint32 *pStatus, uint32 *pLatency)
{
	for (uint32 i=0; i < g_pServerMgr->m_NetMgr.m_Drivers; i++)
	{
		CBaseDriver *pDriver = g_pServerMgr->m_NetMgr.m_Drivers[i];

		if (pDriver->m_DriverFlags & NETDRIVER_TCPIP)
		{
			return pDriver->GetPingStatus(nPingID, pStatus, pLatency);
		}
	}

	return LT_NOTINITIALIZED;
}

LTRESULT CLTServer::RemovePing(uint32 nPingID)
{
	for (uint32 i=0; i < g_pServerMgr->m_NetMgr.m_Drivers; i++)
	{
		CBaseDriver *pDriver = g_pServerMgr->m_NetMgr.m_Drivers[i];

		if (pDriver->m_DriverFlags & NETDRIVER_TCPIP)
		{
			return pDriver->RemovePing(nPingID);
		}
	}

	return LT_NOTINITIALIZED;
}

void CLTServer::SendFileIOMessage(uint8 fileType, uint8 msgID, uint16 fileID, bool bTellLocal)
{
	// Tell the clients to do the same.
	LTLink *pListHead = &g_pServerMgr->m_Clients.m_Head;
	for (LTLink *pCur = pListHead->m_pNext; pCur != pListHead; pCur = pCur->m_pNext)
	{
		Client *pClient = (Client*)pCur->m_pData;

		// (local clients will just inherit the model)
		if (bTellLocal || !(pClient->m_ClientFlags & CFLAG_LOCAL))
		{
			CPacket_Write cPacket;
			cPacket.Writeuint8(msgID);
			ASSERT(fileType == FT_TEXTURE);
			//cPacket.Writeuint8(fileType);
			cPacket.Writeuint16(fileID);
			::SendToClient(pClient, CPacket_Read(cPacket), false, MESSAGE_GUARANTEED);
		}
	}
}


LTRESULT CLTServer::ThreadLoadTexture(const char *pFilename)
{
	UsedFile *pUsedFile;

	if (server_filemgr->AddUsedFile(pFilename, 0, &pUsedFile) == 0)
	{
		RETURN_ERROR_PARAM(1, ThreadLoadTexture, LT_MISSINGFILE, pFilename);
	}

	SendFileIOMessage(FT_TEXTURE, SMSG_THREADLOAD, (uint16)pUsedFile->m_FileID, true);
	return LT_OK;
}


LTRESULT CLTServer::UnloadTexture(const char *pFilename)
{
	UsedFile *pUsedFile;

	if (server_filemgr->AddUsedFile(pFilename, 0, &pUsedFile) == 0)
	{
		RETURN_ERROR_PARAM(1, ThreadLoadTexture, LT_MISSINGFILE, pFilename);
	}

	SendFileIOMessage(FT_TEXTURE, SMSG_UNLOAD, (uint16)pUsedFile->m_FileID, true);
	return LT_OK;
}

LTRESULT CLTServer::MoveObject(HOBJECT hObj, const LTVector *pNewPos)
{
	return Physics()->MoveObject(hObj, pNewPos, 0);
}

LTRESULT CLTServer::GetStandingOn(HOBJECT hObj, CollisionInfo *pInfo)
{
	return Physics()->GetStandingOn(hObj, pInfo);
}

LTRESULT CLTServer::GetObjectPos(HOBJECT hObj, LTVector *pPos)
{
	FN_NAME(CLTServer::GetObjectPos);
	CHECK_PARAMS2(hObj && pPos);

	*pPos = HandleToServerObj(hObj)->GetPos();

	return LT_OK;
}

LTRESULT CLTServer::GetObjectRotation(HOBJECT hObj, LTRotation *pRotation)
{
	FN_NAME(CLTServer::GetObjectPos);
	CHECK_PARAMS2(hObj && pRotation);

	*pRotation = HandleToServerObj(hObj)->m_Rotation;
	return LT_OK;
}

LTRESULT CLTServer::GetObjectName(HOBJECT pObj, char *pBuf, uint32 bufSize)
{
	FN_NAME(CLTServer::GetObjectName);
	char *pName;

	CHECK_PARAMS2(pObj && pBuf && bufSize > 0);

	if (pObj->sd->m_hName)
	{
		pName = (char*)hs_GetElementKey(pObj->sd->m_hName, LTNULL);
		LTStrCpy(pBuf, pName, bufSize);
	}
	else
	{
		pBuf[0] = 0;
	}

	return LT_OK;
}

LTRESULT CLTServer::SendToClient(ILTMessage_Read *pMsg, HCLIENT hSendTo, uint32 flags)
{
	FN_NAME(CLTServer::SendToClient);
	CHECK_PARAMS2(pMsg);

	// Reference the message just in case
	CLTMsgRef_Read cMsgRef(pMsg);
	CLTMessage_Read *pServerMsg = reinterpret_cast<CLTMessage_Read *>(pMsg);

	// Add the "game message" message ID
	CPacket_Write cWrapMsg;
	cWrapMsg.Writeuint8(SMSG_MESSAGE);
	cWrapMsg.WritePacket(pServerMsg->GetPacket());
	CPacket_Read cFinalMsg(cWrapMsg);

	if (hSendTo)
	{
		::SendToClient((Client*)hSendTo, cFinalMsg, true, flags);
	}
	else
	{
		for (LTLink *pCur = g_pServerMgr->m_Clients.m_Head.m_pNext; pCur != &g_pServerMgr->m_Clients.m_Head; pCur = pCur->m_pNext)
		{
			::SendToClient((Client*)pCur->m_pData, cFinalMsg, true, flags);
		}
	}

	return LT_OK;
}

LTRESULT CLTServer::SendToObject(ILTMessage_Read *pMsg, HOBJECT hSender, HOBJECT hSendTo, uint32 flags)
{
	CHECK_PARAMS(hSendTo && pMsg, ILTServer::SendToObject);

	// Reference the message just in case
	CLTMsgRef_Read cMsgRef(pMsg);

	// Reset the read pointer
	pMsg->SeekTo(0);

	// "Send" it to the object..
	hSendTo->sd->m_pObject->ObjectMessageFn(hSender, pMsg);

	return LT_OK;
}

LTRESULT CLTServer::SetObjectSFXMessage(HOBJECT hObject, ILTMessage_Read *pMsg) 
{
	FN_NAME(CLTServer::SetObjectSFXMessage);
	CHECK_PARAMS2(pMsg);

	// Reference the message just in case
	CLTMsgRef_Read cMsgRef(pMsg);
	CLTMessage_Read *pServerMsg = reinterpret_cast<CLTMessage_Read *>(pMsg);

	// Remember the packet...
	sm_SetObjectSpecialEffectMessage(hObject, pServerMsg->GetPacket());

	return LT_OK;
}

LTRESULT CLTServer::SendSFXMessage(ILTMessage_Read *pMsg, const LTVector &pos, uint32 flags) 
{
	CHECK_PARAMS(pMsg, CLTServer::SendSFXMessage);

	// Reference the message just in case
	CLTMsgRef_Read cMsgRef(pMsg);
	CLTMessage_Read *pServerMsg = reinterpret_cast<CLTMessage_Read *>(pMsg);

	// Add the "Special effect" message ID
	CPacket_Write cWrapMsg;
	cWrapMsg.Writeuint8(SMSG_INSTANTSPECIALEFFECT);
	cWrapMsg.WritePacket(pServerMsg->GetPacket());
	CPacket_Read cFinalMsg(cWrapMsg);

	// Send the packet...
	sm_SendToVisibleClients(cFinalMsg, false, pos, flags);

	return LT_OK;
}

LTRESULT CLTServer::SendToServer(ILTMessage_Read *pMsg, HOBJECT hSender, uint32 flags) 
{
	FN_NAME(CLTServer::SendToServer);

	CHECK_PARAMS2(hSender && pMsg);

	if (i_server_shell == NULL)
	{
		RETURN_ERROR(2, ILTServer::SendToServer, LT_NOTINITIALIZED);
	}

	// Reference the message just in case
	CLTMsgRef_Read cMsgRef(pMsg);

	// Reset the read pointer
	pMsg->SeekTo(0);

	i_server_shell->OnObjectMessage(HandleToBaseClass(hSender), pMsg);

	return LT_OK;
}

LTRESULT CLTServer::GetBlindObjectData(uint32 nNum, uint32 nId, uint8*& pData, uint32& nSize)
{
	if( !world_blind_object_data->GetBlindObjectData( nNum, nId, pData, nSize ) )
		return LT_ERROR;

	return LT_OK;
}

LTRESULT CLTServer::FreeBlindObjectData(uint32 nNum, uint32 nId)
{
	if( !world_blind_object_data->FreeBlindObjectData( nNum, nId ) )
		return LT_ERROR;

	return LT_OK;
}

LTRESULT CLTServer::OpenMemoryStream(ILTStream **pStream, uint32 nCacheSize) 
{
	if (!pStream) 
	{
		RETURN_ERROR(1, CLTClient::OpenMemoryStream, LT_INVALIDPARAMS);
	}

	// Open the memory stream
	*pStream = streamsim_OpenMemStream(nCacheSize);

	return LT_OK;
}

LTRESULT CLTServer::GetTextureEffectVarID(const char *pName, uint32 nStage, uint32 *pResult) const
{
	char pszFullName[256];
	LTSNPrintF(pszFullName, sizeof(pszFullName), "%s%d", pName, nStage);
	
	//make this string lowercase to prevent case issues
	char* pszCurr = pszFullName;
	while(*pszCurr)
	{
		*pszCurr = tolower(*pszCurr);
		pszCurr++;
	}

	// Return the hash code
	*pResult = st_GetHashCode(pszFullName);
	return LT_OK;
}

LTRESULT CLTServer::LinkObjRef(HOBJECT hObj, LTObjRef *pRef)
{
	LTObject *pObject;

	if (!hObj)
		return LT_ERROR;

	pObject = (LTObject*)hObj;

	// Linking is not allowed for objects that are going away.
	if( pObject->m_InternalFlags & IFLAG_OBJECTGOINGAWAY )
		return LT_ERROR;

	dl_Insert(&pObject->m_RefList, pRef);

	return LT_OK;
}

LTRESULT CLTServer::GetLightGroupID(const char *pName, uint32 *pResult) const
{
	// Return the hash code
	*pResult = st_GetHashCode(pName);
	return LT_OK;
}

LTRESULT CLTServer::GetOccluderID(const char *pName, uint32 *pResult) const
{
	// Return the hash code
	*pResult = st_GetHashCode_ic(pName);
	return LT_OK;
}

void CLTServer::BreakInterObjectLink(HOBJECT hOwner, HOBJECT hLinked)
{
	if (hOwner && hLinked)
	{
		// kls disconnect only one link...
		DisconnectLinks(HandleToServerObj(hOwner),
			HandleToServerObj(hLinked), false);
	}
}

void CLTServer::BreakInterObjectLink(HOBJECT hObj, HOBJECT hLinked, bool bNotify)
{
	if (!hObj) 
		return;

	if (hLinked)
	{
		// Break all links between the two objects...
		DisconnectLinks( HandleToServerObj( hObj ), HandleToServerObj( hLinked ), true, bNotify );
	}	
	else
	{
		// Break all the objects links...
		BreakInterLinks( HandleToServerObj( hObj ), LINKTYPE_INTERLINK, bNotify );

		// Get rid of the LTObjRef's too.
		hObj->NotifyObjRefList_Delete();	
	}
}


// --------------------------------------------------------------- //
// ILTPhysics implementation.
// --------------------------------------------------------------- //

LTRESULT si_GetGameInfo(void **ppData, uint32 *pLen)
{
	*ppData = g_pServerMgr->m_pGameInfo;
	*pLen = g_pServerMgr->m_GameInfoLen;
	return LT_OK;
}


HCLASS si_GetClass(const char *pName)
{
	return (HCLASS)g_pServerMgr->m_ClassMgr.FindClassData(pName);
}


LTRESULT si_GetStaticObject(HCLASS hClass, HOBJECT *out)
{
	*out = LTNULL;

	if (!hClass)
	{
		RETURN_ERROR(1, ILTPhysics::GetStaticObject, LT_INVALIDPARAMS);
	}

	CClassData *pClassData = (CClassData*)hClass;
	if (pClassData->m_pStaticObject)
	{
		*out = ServerObjToHandle(pClassData->m_pStaticObject);
		return LT_OK;
	}
	else
	{
		RETURN_ERROR(1, ILTPhysics::GetStaticObject, LT_ERROR);
	}
}


HCLASS si_GetObjectClass(HOBJECT hObject)
{
	if (!hObject)
	{
		RETURN_ERROR(1, ILTPhysics::GetObjectClass, LTNULL);
	}

	LTObject *pObject = HandleToServerObj(hObject);
	CClassData *pClassData = (CClassData*)pObject->sd->m_pClass->m_pInternal[g_pServerMgr->m_ClassMgr.m_ClassIndex];
	return (HCLASS)pClassData;
}


bool si_IsKindOf(HCLASS hClass1, HCLASS hClass2)
{
	if (!hClass1 || !hClass2)
		return false;

	CClassData *pClass1 = (CClassData*)hClass1;
	CClassData *pClass2 = (CClassData*)hClass2;

	ClassDef *pCur = pClass1->m_pClass;
	while (pCur)
	{
		if (pCur == pClass2->m_pClass)
			return true;

		pCur = pCur->m_ParentClass;
	}

	return false;
}

bool si_CastRay(IntersectQuery *pQuery, IntersectInfo *pInfo)
{
	float mag = pQuery->m_Direction.Mag();
	if (mag < 0.00001f)
		return false;

	// Scale it to 10000.
	float scale = 10000.0f / mag;
	pQuery->m_To = pQuery->m_Direction * scale;
	pQuery->m_To += pQuery->m_From;

	return ServerIntersectSegment(pQuery, pInfo);
}


void SphereFindCallback(WorldTreeObj *pObj, void *pCBUser)
{
	if (pObj->GetObjType() != WTObj_DObject)
		return;

    SphereFindStruct *pStruct	= (SphereFindStruct*)pCBUser;
    LTObject *pServerObj		= (LTObject*)pObj;
	float fFullRadius			= pServerObj->GetRadius() + pStruct->m_SphereTouchRadius;

    LTVector vecTo				= pServerObj->GetPos() - *pStruct->m_pSphereTouchPos;
    float distSqr				= vecTo.MagSqr();
   
    if (distSqr >= fFullRadius * fFullRadius)
		return;

	ObjectLink *pLink = (ObjectLink*)sb_Allocate(&g_pServerMgr->m_ObjectLinkBank);
	if (!pLink)
		return;

	pLink->m_hObject = ServerObjToHandle(pServerObj);
	pLink->m_pNext = pStruct->m_pSphereTouchList->m_pFirstLink;
	pStruct->m_pSphereTouchList->m_pFirstLink = pLink;
	++pStruct->m_pSphereTouchList->m_nInList;
}


ObjectList* si_FindObjectsTouchingSphere(const LTVector *pPosition, float radius)
{
	SphereFindStruct theStruct;

	// Setup the global stuff.
	theStruct.m_pSphereTouchList = (ObjectList*)sb_Allocate(&g_pServerMgr->m_ObjectListBank);
	if (!theStruct.m_pSphereTouchList)
		return LTNULL;

	theStruct.m_pSphereTouchList->m_nInList = 0;
	theStruct.m_pSphereTouchList->m_pFirstLink = LTNULL;

	theStruct.m_pSphereTouchPos = pPosition;
	theStruct.m_SphereTouchRadius = radius;

	Counter cntTicks;
	cnt_StartCounter(cntTicks);

	LTVector boxMin = *pPosition - LTVector(radius, radius, radius);
	LTVector boxMax = *pPosition + LTVector(radius, radius, radius);

	world_bsp_server->ServerTree()->FindObjectsInBox(&boxMin, &boxMax, SphereFindCallback, &theStruct);

	g_SphereFindTicks += cnt_EndCounter(cntTicks);

	g_SphereFindCount += theStruct.m_pSphereTouchList->m_nInList;

	return theStruct.m_pSphereTouchList;
}

void BoxFindCallback(WorldTreeObj *pObj, void *pCBUser)
{
	if (pObj->GetObjType() != WTObj_DObject)
		return;

	BoxFindStruct *pBFStruct = (BoxFindStruct*)pCBUser;
	LTObject *pServerObj = (LTObject*)pObj;

	// Just add each object to the object list...

	ObjectLink *pLink = (ObjectLink*)sb_Allocate(&g_pServerMgr->m_ObjectLinkBank);
	if (!pLink)
		return;

	pLink->m_hObject	= ServerObjToHandle( pServerObj );
	pLink->m_pNext		= pBFStruct->m_pBoxTouchList->m_pFirstLink;
	
	pBFStruct->m_pBoxTouchList->m_pFirstLink = pLink;
	++pBFStruct->m_pBoxTouchList->m_nInList;
}

ObjectList* si_GetBoxIntersectors(const LTVector *pMin, const LTVector *pMax)
{
	BoxFindStruct	BFStruct;

	BFStruct.m_pBoxTouchList = (ObjectList*)sb_Allocate( &g_pServerMgr->m_ObjectListBank);
	if( !BFStruct.m_pBoxTouchList )
		return LTNULL;

	BFStruct.m_pBoxTouchList->m_nInList		= 0;
	BFStruct.m_pBoxTouchList->m_pFirstLink	= LTNULL;

	world_bsp_server->ServerTree()->FindObjectsInBox( pMin, pMax, BoxFindCallback, &BFStruct );

	return BFStruct.m_pBoxTouchList;
}

void si_RelinquishList(ObjectList *pList)
{
	if (!pList) 
		return;

	ObjectLink *pLink = pList->m_pFirstLink;
	while (pLink)
	{
		ObjectLink *pNext = pLink->m_pNext;
		sb_Free(&g_pServerMgr->m_ObjectLinkBank, pLink);
		pLink = pNext;
	}

	sb_Free(&g_pServerMgr->m_ObjectListBank, pList);
}

ObjectList* si_CreateObjectList()
{
	ObjectList *pRet;
	LT_MEM_TRACK_ALLOC(pRet = (ObjectList*)sb_Allocate(&g_pServerMgr->m_ObjectListBank), LT_MEM_TYPE_MISC);
	if (!pRet)
		return LTNULL;

	pRet->m_pFirstLink = LTNULL;
	pRet->m_nInList = 0;

	return pRet;
}

ObjectLink* si_AddObjectToList(ObjectList *pList, HOBJECT hObj)
{
	ObjectLink *pLink;
	LT_MEM_TRACK_ALLOC(pLink = (ObjectLink*)sb_Allocate(&g_pServerMgr->m_ObjectLinkBank), LT_MEM_TYPE_MISC);
	if (!pLink)
		return LTNULL;

	pLink->m_pNext = pList->m_pFirstLink;
	pList->m_pFirstLink = pLink;
	pLink->m_hObject = hObj;
	pList->m_nInList++;

	return pLink;
}

void si_RemoveObjectFromList(ObjectList *pList, HOBJECT hObj)
{
	ObjectLink **ppPrev = &pList->m_pFirstLink;
	ObjectLink *pCur = pList->m_pFirstLink;
	while (pCur)
	{
		if (pCur->m_hObject == hObj)
		{
			*ppPrev = pCur->m_pNext;
			--pList->m_nInList;

			sb_Free(&g_pServerMgr->m_ObjectLinkBank, pCur);
			return;
		}

		ppPrev = &pCur->m_pNext;
		pCur = pCur->m_pNext;
	}
}

LTRESULT si_GetPropString(const char *pPropName, char *pRet, int maxLen)
{
	const GenericProp *pProp = _FindProp(pPropName);

	if (pProp && pProp->m_Type == PT_STRING)
	{
		LTStrCpy(pRet, pProp->m_String, maxLen);
		return LT_OK;
	}
	else
	{
		return LT_NOTFOUND;
	}

}

LTRESULT si_GetPropVector(const char *pPropName, LTVector *pRet)
{
	const GenericProp *pProp = _FindProp(pPropName);

	if (!pProp)
		return LT_NOTFOUND;

	if (pProp->m_Type == PT_VECTOR)
		*pRet = pProp->m_Vec;
	else if (pProp->m_Type == PT_COLOR)
		*pRet = pProp->m_Color;
	else
		return LT_NOTFOUND;

	return LT_OK;

}

LTRESULT si_GetPropReal(const char *pPropName, float *pRet)
{
	const GenericProp *pProp = _FindProp(pPropName);

	if (pProp && pProp->m_Type == PT_REAL)
	{
		*pRet = pProp->m_Float;
		return LT_OK;
	}
	else
	{
		return LT_NOTFOUND;
	}

}

LTRESULT si_GetPropFlags(const char *pPropName, uint32 *pRet)
{
	const GenericProp *pProp = _FindProp(pPropName);

	if (pProp && pProp->m_Type == PT_FLAGS)
	{
		*pRet = pProp->m_Long;
		return LT_OK;
	}
	else
	{
		return LT_NOTFOUND;
	}

}

LTRESULT si_GetPropBool(const char *pPropName, bool *pRet)
{
	const GenericProp *pProp = _FindProp(pPropName);

	if (pProp && pProp->m_Type == PT_BOOL)
	{
		*pRet = pProp->m_Bool;
		return LT_OK;
	}
	else
	{
		return LT_NOTFOUND;
	}

}

LTRESULT si_GetPropLongInt(const char *pPropName, int32 *pRet)
{
	const GenericProp *pProp = _FindProp(pPropName);

	if (pProp && pProp->m_Type == PT_LONGINT)
	{
		*pRet = pProp->m_Long;
		return LT_OK;
	}
	else
	{
		return LT_NOTFOUND;
	}

}

LTRESULT si_GetPropRotation(const char *pPropName, LTRotation *pRet)
{
	const GenericProp *pProp = _FindProp(pPropName);

	if (pProp && pProp->m_Type == PT_ROTATION)
	{
		*pRet = pProp->m_Rotation;
		return LT_OK;
	}
	else
	{
		return LT_NOTFOUND;
	}

}

LTRESULT si_GetPropRotationEuler(const char *pPropName, LTVector *pAngles)
{
	const GenericProp *pProp = _FindProp(pPropName);

	if (pProp && pProp->m_Type == PT_ROTATION)
	{
		*pAngles = pProp->m_Vec;
		return LT_OK;
	}
	else
	{
		return LT_NOTFOUND;
	}
}

LTRESULT si_GetPropGeneric(const char *pPropName, GenericProp *pGeneric)
{
	const GenericProp *pProp = _FindProp(pPropName);

	if (pProp)
	{
		*pGeneric = *pProp;
		return LT_OK;
	}
	else
	{
		return LT_NOTFOUND;
	}
}


LTRESULT si_DoesPropExist(const char *pPropName, int32 *pPropType)
{
	const GenericProp *pProp = _FindProp(pPropName);

	if (pProp)
	{
		*pPropType = pProp->m_Type;
		return LT_OK;
	}
	else
	{
		return LT_NOTFOUND;
	}
}

HOBJECT si_ObjectToHandle(LPBASECLASS pObject)
{
	if (!pObject)
		return LTNULL;

	return BaseClassToHandle(pObject);
}

LPBASECLASS si_HandleToObject(HOBJECT hObject)
{
	if (!hObject)
		return LTNULL;

	return ServerObjToBaseClass(HandleToServerObj(hObject));
}

LPBASECLASS si_CreateObject(HCLASS hClass, ObjectCreateStruct *pStruct)
{
	if (!hClass)
	{
		return LTNULL;
	}

	CClassData *pClassData = (CClassData*)hClass;

	// Only class-only objects are allowed to create objects without an OCS
	if (!pStruct && ((pClassData->m_pClass->m_ClassFlags & CF_CLASSONLY) == 0))
		return LTNULL;

	return g_pServerMgr->EZCreateObject(pClassData, pStruct);
}

LPBASECLASS si_CreateObjectProps(HCLASS hClass, ObjectCreateStruct *pStruct, const char *pszProps)
{
	CClassData *pClass = (CClassData*)hClass;

	// Create and construct the object.
	LPBASECLASS pBaseClass = sm_AllocateObjectOfClass(pClass->m_pClass);

	ObjectCreateStruct cNewOCS = *pStruct;
	if (pszProps)
	{
		// Read the properties...
		ConParse parse;
		parse.Init(pszProps);
		while (parse.Parse())
		{
			if (parse.m_nArgs > 0)
			{
				//  Create the new prop...
				// New prop will always be a string type with the first token used as prop name...
				char aPropBuffer[MAX_GP_STRING_LEN];

				// Use the rest of the tokens as the property's value...
				char *pFinger = aPropBuffer;
				uint32 nLength = 0;
				for (int i = 1; i < parse.m_nArgs; i++)
				{
					uint32 nCurLength = strlen(parse.m_Args[i]);
					nLength += nCurLength;
					if (nLength >= MAX_GP_STRING_LEN)
					{
						*pFinger = 0;
						break;
					}
					if (i != 1)
					{
						*pFinger = ' ';
						++pFinger;
						++nLength;
					}
					memcpy(pFinger, parse.m_Args[i], nCurLength);
					pFinger += nCurLength;
				}
				if (nLength <= MAX_GP_STRING_LEN)
					*pFinger = 0;

				// Add it to the property list
				cNewOCS.m_cProperties.AddProp(parse.m_Args[0], GenericProp(aPropBuffer, LT_PT_STRING));
			}
		}
	}

	// Point at our object create struct
	ObjectCreateStruct *pOldOCS = g_pServerMgr->m_pCurOCS;
	g_pServerMgr->m_pCurOCS = &cNewOCS;

	// PostPropRead.
	pStruct->m_hClass = (HCLASS)pClass;
	uint32 nPreCreateResult = pBaseClass->OnPrecreate(pStruct, PRECREATE_STRINGPROP);

	// Clear out the current object create struct pointer
	g_pServerMgr->m_pCurOCS = pOldOCS;

	LTRESULT nAddObjectResult;
	if (nPreCreateResult)
	{
		// Class-only objects are done, at this point.
		if ((pClass->m_pClass->m_ClassFlags & CF_CLASSONLY) != 0)
			return pBaseClass;

		// Moved the addobjecttoworld to before the destruction of the
		// properties.  This is consistent with the way objects are added
		// normally...
		LTObject *pObject;
		nAddObjectResult = sm_AddObjectToWorld(pBaseClass, pClass->m_pClass, pStruct,
			INVALID_OBJECTID, OBJECTCREATED_STRINGPROP, &pObject);
	}

	if ((nPreCreateResult) && (nAddObjectResult == LT_OK))
	{
		pBaseClass->OnAllObjectsCreated();
		return pBaseClass;
	}
	else
	{
		sm_FreeObjectOfClass(pClass->m_pClass, pBaseClass);
		return LTNULL;
	}
}

uint32 si_GetServerFlags()
{
	// Build the flags from internal stuff.
	uint32 flags = g_pServerMgr->m_ServerFlags;
	if (g_pServerMgr->m_InternalFlags & SFLAG_BUILDINGCACHELIST)
		flags |= SS_CACHING;

	return flags;
}

uint32 si_SetServerFlags(uint32 flags)
{
	// Only let them set certain ones..
	g_pServerMgr->m_ServerFlags = (flags & (SS_PAUSED));
	return g_pServerMgr->m_ServerFlags;
}

LTRESULT si_CacheFile(uint32 fileType, const char *pFilename)
{
	return sm_CacheFile(fileType, pFilename);
}

int si_IntRandom(int min, int max)
{
	return min + (rand() % (max - (min - 1)));
}

float si_RandomScale(LTFLOAT scale)
{
	return ((LTFLOAT)rand() / RAND_MAX) * scale;
}

void si_BPrint(const char *pMsg, ...)
{
	va_list marker;
	va_start(marker, pMsg);

	static const uint32 knBufferSize = 512;
	char msg[knBufferSize];
	LTVSNPrintF(msg, knBufferSize, pMsg, marker);

	va_end(marker);

	BPrint(msg);
}


LTRESULT si_GetSkyDef(SkyDef *pDef)
{
	memcpy(pDef, &g_pServerMgr->m_SkyDef, sizeof(SkyDef));
	return LT_OK;
}

LTRESULT si_SetSkyDef(SkyDef *pDef)
{
	memcpy(&g_pServerMgr->m_SkyDef, pDef, sizeof(SkyDef));
	sm_SetSendSkyDef();
	return LT_OK;
}

LTRESULT si_AddObjectToSky(HOBJECT hObject, uint32 index)
{
	// Is it valid?
	LTObject *pObject = HandleToServerObj(hObject);
	if (!pObject)
	{
		RETURN_ERROR_PARAM(1, ILTPhysics::AddObjectToSky, LT_ERROR, "object is LTNULL");
	}

	if (index >= MAX_SKYOBJECTS)
	{
		RETURN_ERROR_PARAM(1, ILTPhysics::AddObjectToSky, LT_ERROR, "invalid index");
	}

	// Is it already in the sky?
	if (pObject->m_InternalFlags & IFLAG_INSKY)
	{
		return LT_OK;
	}

	pObject->m_InternalFlags |= IFLAG_INSKY;
	g_pServerMgr->m_SkyObjects[index] = pObject->m_ObjectID;
	sm_SetSendSkyDef();

	return LT_OK;
}


LTRESULT si_RemoveObjectFromSky(HOBJECT hObject)
{
	// Is it valid?
	LTObject *pObject = HandleToServerObj(hObject);
	if (!pObject)
	{
		RETURN_ERROR_PARAM(1, ILTPhysics::RemoveObjectFromSky, LT_ERROR, "object is LTNULL");
	}

	return sm_RemoveObjectFromSky(pObject);
}


LTRESULT si_GetGlobalLightObject(HOBJECT *hObj)
{
	if (!hObj)
		return LT_ERROR;

	*hObj = g_pServerMgr->GetGlobalLightObject();

	return LT_OK;
}

LTRESULT si_SetGlobalLightObject(HOBJECT hObj)
{
	g_pServerMgr->SetGlobalLightObject(hObj);
	return LT_OK;
}


LTRESULT si_AttachClient(HCLIENT hParent, HCLIENT hChild)
{
	if (!hParent || !hChild)
		RETURN_ERROR(1, ILTPhysics::AttachClient, LT_INVALIDPARAMS);

	return sm_AttachClient((Client*)hParent, (Client*)hChild);
}

LTRESULT si_DetachClient(HCLIENT hChild)
{
	if (!hChild)
		RETURN_ERROR(1, ILTPhysics::DetachClient, LT_INVALIDPARAMS);

	return sm_DetachClient((Client*)hChild);
}

HCLIENT si_GetNextClient(HCLIENT hPrev)
{
	if (hPrev)
	{
		Client *pPrev = (Client*)hPrev;
		if (pPrev->m_Link.m_pNext == &g_pServerMgr->m_Clients.m_Head)
			return LTNULL;

		return (HCLIENT)pPrev->m_Link.m_pNext->m_pData;
	}
	else
	{
		if (g_pServerMgr->m_Clients.m_nElements == 0)
			return LTNULL;

		return (HCLIENT)g_pServerMgr->m_Clients.m_Head.m_pNext->m_pData;
	}
}


HCLIENTREF si_GetNextClientRef(HCLIENTREF hPrev)
{
	LTList *pList = &g_pServerMgr->m_ClientReferences;

	LTLink *pCurLink;
	ClientRef *pPrev = (ClientRef*)hPrev;
	if (pPrev)
	{
		pCurLink = &pPrev->m_Link;
	}
	else
	{
		pCurLink = &pList->m_Head;
	}

	if (pCurLink->m_pNext == &pList->m_Head)
		return LTNULL;
	else
		return (HCLIENTREF)pCurLink->m_pNext->m_pData;
}

uint32 si_GetClientRefInfoFlags(HCLIENTREF hRef)
{
	if (!hRef)
		return 0;

	ClientRef *pRef = (ClientRef*)hRef;

	if (pRef->m_ClientFlags & CFLAG_LOCAL)
		return CIF_LOCAL;
	else
		return 0;
}

bool si_GetClientRefName(HCLIENTREF hRef, char *pName, int maxLen)
{
	if (!hRef)
	{
		pName[0] = 0;
		return false;
	}

	ClientRef *pRef = (ClientRef*)hRef;
	LTStrCpy(pName, pRef->m_ClientName, maxLen);
	return true;
}

HOBJECT si_GetClientRefObject(HCLIENTREF hRef)
{
	if (!hRef)
		return LTNULL;

	ClientRef *pRef = (ClientRef*)hRef;
	LTObject *pObj = sm_FindObject(pRef->m_ObjectID);

	if (pObj)
	{
		return ServerObjToHandle(pObj);
	}
	else
	{
		return LTNULL;
	}
}


uint32 si_GetClientID(HCLIENT hClient)
{
	if (!hClient)
		return (uint32)-1;

	return ((Client*)hClient)->m_ClientID;
}


HCLIENT si_GetClientHandle(uint32 clientID)
{
	HCLIENT hReturnClient = ( HCLIENT )g_pServerMgr->GetClientFromId( clientID );

	return hReturnClient;
}


bool si_GetClientName(HCLIENT hClient, char *pName, int maxLen)
{
	if (!hClient)
		return false;

	Client *pClient = (Client*)hClient;
	LTStrCpy(pName, pClient->m_Name, maxLen);
	return true;
}


bool si_SetClientName(HCLIENT hClient, const char *pName, int maxLen)
{
	if (!hClient)
		return false;

	Client *pClient = (Client*)hClient;

	if (pClient->m_Name)
	{
		dfree(pClient->m_Name);
		pClient->m_Name	= 0;
	}

	LT_MEM_TRACK_ALLOC(pClient->m_Name = (char*)dalloc(maxLen + 1),LT_MEM_TYPE_MISC);
	if (!pClient->m_Name)
		return false;

	LTStrCpy(pClient->m_Name, pName, maxLen);

	return true;
}


void si_SetClientInfoFlags(HCLIENT hClient, uint32 dwClientFlags)
{
	if (!hClient)
		return;

	Client *pClient = (Client*)hClient;

	pClient->m_ClientFlags &= ~CFLAG_FULLRES;
	pClient->m_ClientFlags &= ~CFLAG_SENDCOBJROTATION;

	if (dwClientFlags & CIF_FULLRES)
		pClient->m_ClientFlags |= CFLAG_FULLRES;

	if (dwClientFlags & CIF_SENDCOBJROTATION)
		pClient->m_ClientFlags |= CFLAG_SENDCOBJROTATION;
}


uint32 si_GetClientInfoFlags(HCLIENT hClient)
{
	if (!hClient)
		return 0;

	Client *pClient = (Client*)hClient;

	uint32 flags = 0;
	if (pClient->m_ClientFlags & CFLAG_LOCAL)
		flags |=  CIF_LOCAL;
	if (pClient->m_ClientFlags & CFLAG_VIRTUAL)
		flags |=  CIF_PLAYBACK;
	if (pClient->m_ClientFlags & CFLAG_FULLRES)
		flags |= CIF_FULLRES;
	if (pClient->m_ClientFlags & CFLAG_SENDCOBJROTATION)
		flags |= CIF_SENDCOBJROTATION;
	return flags;
}


void si_SetClientUserData(HCLIENT hClient, void *pData)
{
	if (!hClient)
		return;

	((Client*)hClient)->m_pPluginUserData = pData;
}


void* si_GetClientUserData(HCLIENT hClient)
{
	if (!hClient)
		return LTNULL;

	return ((Client*)hClient)->m_pPluginUserData;
}


LTRESULT si_KickClient(HCLIENT hClient)
{
	if (!hClient)
		RETURN_ERROR(1, ILTServer::KickClient, LT_INVALIDPARAMS);

	Client *pClient = (Client*)hClient;

	// Don't allow kick of host.
	if( pClient->m_ClientFlags & CFLAG_LOCAL )
		RETURN_ERROR_PARAM( 1, ILTServer::KickClient, LT_ERROR, "Cannot kick host." );

	pClient->m_ClientFlags |= CFLAG_KICKED;

	return LT_OK;
}

LTRESULT si_SetClientViewPos(HCLIENT hClient, const LTVector *pPos)
{
	if (!hClient)
		RETURN_ERROR(0, SetClientViewPos, LT_INVALIDPARAMS);

	((Client*)hClient)->m_ViewPos = *pPos;
	return LT_OK;
}

void si_RunGameConString(const char *pString)
{
	cc_HandleCommand(console_state->State(), pString);
}


void si_SetGameConVar(const char *pName, const char *pVal)
{
	if (!pName || !pVal)
		return;

	cc_SetConsoleVariable(console_state->State(), pName, pVal);
}


HCONVAR si_GetGameConVar(const char *pName)
{
	if (!pName)
		return LTNULL;

	LTCommandVar *pVar = cc_FindConsoleVar(console_state->State(), pName);
	return (HCONVAR)pVar;
}


LTRESULT si_CreateInterObjectLink(HOBJECT hOwner, HOBJECT hLinked)
{
	if (!hOwner || !hLinked)
		RETURN_ERROR(1, ILTPhysics::CreateInterObjectLink, LT_INVALIDPARAMS);

	return CreateInterLink(HandleToServerObj(hOwner), HandleToServerObj(hLinked), LINKTYPE_INTERLINK);
}


LTRESULT si_CreateAttachment(HOBJECT hParent, 
									HOBJECT hChild, 
									const char *pSocketName,
									LTVector   *pOffset, 
									LTRotation *pRotationOffset, 
									HATTACHMENT *hAttachment)
{
	LTObject *pParent = HandleToServerObj(hParent);
	LTObject *pChild = HandleToServerObj(hChild);
	if (!pParent || !pChild)
		RETURN_ERROR(1, ILTPhysics::CreateAttachment, LT_INVALIDPARAMS);

	// Get the node index.
	uint32 nodeIndex = (uint32)-1;
	if (pParent->m_ObjectType == OT_MODEL && pSocketName)
	{
		Model *pModel = ToModel(pParent)->GetModelDB();
		if (!pModel)
		{
			RETURN_ERROR(1, ILTPhysics::CreateAttachment, LT_INVALIDDATA);
		}

		if (!pModel->FindSocket(pSocketName, &nodeIndex))
		{
			// Look for a node if we can't find it in a socket
			if (!pModel->FindNode(pSocketName, &nodeIndex))
			{
				RETURN_ERROR(1, ILTPhysics::CreateAttachment, LT_NODENOTFOUND);
			}
			else
			{
				// Move it past the end of the socket list since it's a node attachment
				nodeIndex += pModel->NumSockets();
			}
		}
	}


	// Set it up.
	Attachment *pAttachment;
	LTRESULT dResult = om_CreateAttachment(
							&g_pServerMgr->m_ObjectMgr, 
							pParent, 
							pChild->m_ObjectID,
							nodeIndex, 
							pOffset, 
							pRotationOffset, 
							&pAttachment);

	if (dResult != LT_OK)
		return dResult;

	SetObjectChangeFlags(pParent, CF_ATTACHMENTS);

	if (hAttachment)
		*hAttachment = (HATTACHMENT)pAttachment;

	return LT_OK;
}

LTRESULT si_RemoveAttachment(HATTACHMENT hAttachment)
{
	Attachment *pAttachment = (Attachment*)hAttachment;
	if (!pAttachment)
		RETURN_ERROR(1, ILTPhysics::RemoveAttachment, LT_INVALIDPARAMS);

	LTObject *pParent = sm_FindObject(pAttachment->m_nParentID);
	if (!pParent)
		RETURN_ERROR(1, ILTPhysics::RemoveAttachment, LT_ERROR);

	uint32 nChildID = pAttachment->m_nChildID;

	LTRESULT dResult = om_RemoveAttachment(&g_pServerMgr->m_ObjectMgr, pParent, pAttachment->m_nChildID);
	if (dResult == LT_OK)
	{
		SetObjectChangeFlags(pParent, CF_ATTACHMENTS);
		// Tell the child to send an update
		LTObject *pChild = sm_FindObject(nChildID);
		if (pChild)
		{
			SetObjectChangeFlags(pChild, CF_POSITION | CF_ROTATION);
		}
	}
	else
	{
		RETURN_ERROR(1, ILTPhysics::RemoveAttachment, LT_NOTFOUND);
	}

	return dResult;
}


LTRESULT si_FindAttachment(HOBJECT hParent, HOBJECT hChild, HATTACHMENT *hAttachment)
{
	if (!hParent || !hChild || !hAttachment)
	{
		RETURN_ERROR(1, ILTPhysics::FindAttachment, LT_INVALIDPARAMS);
	}

	*hAttachment = LTNULL;
	LTObject *pParent = (LTObject*)hParent;
	LTObject *pChild = (LTObject*)hChild;
	for (Attachment *pCur = pParent->m_Attachments; pCur; pCur = pCur->m_pNext)
	{
        if (pCur->m_nChildID == pChild->m_ObjectID)
		{
			*hAttachment = (HATTACHMENT)pCur;
			return LT_OK;
		}
	}

	return LT_NOTFOUND;
}


bool si_GetObjectColor(HOBJECT hObject, float *r, float *g, float *b, float *a)
{
	if (!hObject)
		return false;

	LTObject *pObject = HandleToServerObj(hObject);

	if (r)
		*r = pObject->m_ColorR * MATH_ONE_OVER_255;
	if (g)   
		*g = pObject->m_ColorG * MATH_ONE_OVER_255;
	if (b)   
		*b = pObject->m_ColorB * MATH_ONE_OVER_255;
	if (a)   
		*a = pObject->m_ColorA * MATH_ONE_OVER_255;

	return true;
}


bool si_SetObjectColor(HOBJECT hObject, float r, float g, float b, float a)
{
	if (!hObject)
		return false;

	LTObject *pObject = HandleToServerObj(hObject);

	uint8 newColor[4];

	newColor[0] = (uint8)(r * 255.0f);
	newColor[1] = (uint8)(g * 255.0f);
	newColor[2] = (uint8)(b * 255.0f);
	newColor[3] = (uint8)(a * 255.0f);

	if (pObject->m_ColorR != newColor[0] || pObject->m_ColorG != newColor[1] ||
		pObject->m_ColorB != newColor[2] || pObject->m_ColorA != newColor[3])
	{
		pObject->m_ColorR = newColor[0];
		pObject->m_ColorG = newColor[1];
		pObject->m_ColorB = newColor[2];
		pObject->m_ColorA = newColor[3];

		SetObjectChangeFlags(pObject, CF_RENDERINFO);
	}

	return true;
}


uint32 si_GetObjectUserFlags(HOBJECT hObj)
{
	if (!hObj)
		return 0;

	LTObject *pObj = HandleToServerObj(hObj);
	return pObj->m_UserFlags;
}


LTRESULT si_SetObjectUserFlags(HOBJECT hObj, uint32 flags)
{
	LTObject *pObj = HandleToServerObj(hObj);
	if (!pObj)
	{
		RETURN_ERROR(1, ILTPhysics::SetObjectUserFlags, LT_ERROR);
	}

	if (pObj->m_UserFlags != flags)
	{
		pObj->m_UserFlags = flags;
		SetObjectChangeFlags(pObj, CF_FLAGS);
	}

	return LT_OK;
}


HOBJECT si_GetNextObject(HOBJECT hObj)
{
	LTLink *pLink;

	if (hObj)
	{
		pLink = &HandleToServerObj(hObj)->sd->m_ListNode;

		pLink = pLink->m_pNext;
		if (pLink == &g_pServerMgr->m_Objects.m_Head)
			return LTNULL;
	}
	else
	{
		pLink = g_pServerMgr->m_Objects.m_Head.m_pNext;
		if (pLink == &g_pServerMgr->m_Objects.m_Head)
			return LTNULL;
	}

	LTObject *pObj = (LTObject*)pLink->m_pData;
	if (pObj->m_InternalFlags & IFLAG_INACTIVE_MASK)
	{
		return LTNULL;
	}
	else
	{
		return ServerObjToHandle(pObj);
	}
}


HOBJECT si_GetNextInactiveObject(HOBJECT hObj)
{
	LTLink *pLink;

	// Note: it cycles thru the object list backwards here because
	// the inactive objects are at the end of the list.
	if (hObj)
	{
		pLink = &HandleToServerObj(hObj)->sd->m_ListNode;

		pLink = pLink->m_pPrev;
		if (pLink == &g_pServerMgr->m_Objects.m_Head)
			return LTNULL;
	}
	else
	{
		pLink = g_pServerMgr->m_Objects.m_Head.m_pPrev;
		if (pLink == &g_pServerMgr->m_Objects.m_Head)
			return LTNULL;
	}

	LTObject *pObj = (LTObject*)pLink->m_pData;
	if (pObj->m_InternalFlags & IFLAG_INACTIVE_MASK)
	{
		return ServerObjToHandle(pObj);
	}
	else
	{
		return LTNULL;
	}
}


void si_SetNextUpdate(HOBJECT hObj, LTFLOAT nextUpdate)
{
	if (!hObj)
		return;

	HandleToServerObj(hObj)->sd->m_NextUpdate = nextUpdate;
}

void si_SetObjectState(HOBJECT hObj, int state)
{
	if (!hObj)
		return;

	LTObject *pObj = (LTObject*)HandleToServerObj(hObj);

	// Translate the state to internal flags.
	uint32 stateFlags;
	switch (state)
	{
		case OBJSTATE_INACTIVE:
			stateFlags = IFLAG_INACTIVE;
			break;
		case OBJSTATE_INACTIVE_TOUCH:
			stateFlags = IFLAG_INACTIVE_TOUCH;
			break;
		default:
			stateFlags = 0;
			break;
	}

	sm_SetObjectStateFlags(pObj, stateFlags);
}


int si_GetObjectState(HOBJECT hObj)
{
	if (!hObj)
		return OBJSTATE_INACTIVE;

	LTObject *pObj = (LTObject*)HandleToServerObj(hObj);

	if (pObj->m_InternalFlags & IFLAG_INACTIVE)
		return OBJSTATE_INACTIVE;
	else if (pObj->m_InternalFlags & IFLAG_INACTIVE_TOUCH)
		return OBJSTATE_INACTIVE_TOUCH;
	else
		return OBJSTATE_ACTIVE;
}

LTRESULT si_GetObjectScale(HOBJECT hObj, LTVector *pScale)
{
	LTObject *pObj = HandleToServerObj(hObj);
	if (!pObj || !pScale)
		RETURN_ERROR(1, ILTPhysics::GetObjectScale, LT_ERROR);

	*pScale = pObj->m_Scale;
	return LT_OK;
}

void si_ScaleObject(HOBJECT hObj, const LTVector *pNewScale)
{
	LTObject *pObj = HandleToServerObj(hObj);

	pObj->m_Scale = *pNewScale;
	SetObjectChangeFlags(pObj, CF_SCALE);
}



LTRESULT si_GetLastCollision(CollisionInfo *pInfo)
{
	if (!pInfo)
	{
		RETURN_ERROR(1, ILTPhysics::GetLastCollision, LT_INVALIDPARAMS);
	}

	if (!g_pServerMgr->m_pCollisionInfo)
		return LT_ERROR;

	pInfo->m_Plane = g_pServerMgr->m_pCollisionInfo->m_Plane;
	pInfo->m_hObject = g_pServerMgr->m_pCollisionInfo->m_hObject;
	pInfo->m_hPoly = g_pServerMgr->m_pCollisionInfo->m_hPoly;
	pInfo->m_vStopVel = g_pServerMgr->m_pCollisionInfo->m_vStopVel;
	return LT_OK;
}

LTRESULT si_DoObjectRotation(HOBJECT hObj, const LTRotation *pRotation, bool bSnap)
{
	if (!hObj)
	{
		RETURN_ERROR(1, ILTPhysics::SetObjectRotation, LT_ERROR);
	}

	LTObject *pObj = HandleToServerObj(hObj);

	// Avoid setting the change flags if they're the same.
	if (pObj->m_Rotation == *pRotation)
		return LT_OK;

	if (HasWorldModel(pObj))
	{
		MoveState moveState;
		moveState.Setup(world_bsp_server->ServerTree(), g_pServerMgr->m_MoveAbstract, pObj, pObj->m_BPriority);
		RotateWorldModel(&moveState, *pRotation);
	}
	else
	{
		pObj->m_Rotation = *pRotation;
	}

	SetObjectChangeFlags(pObj, CF_ROTATION | ((bSnap) ? CF_SNAPROTATION : 0));

	return LT_OK;
}

LTRESULT si_RotateObject(HOBJECT hObj, const LTRotation *pRotation)
{
	return si_DoObjectRotation(hObj, pRotation, false);
}

void si_SetObjectPos(HOBJECT hObj, const LTVector *pos)
{
	LTObject *pObj = HandleToServerObj(hObj);

	if (!pObj)
		return;

	FullMoveObject(pObj, pos,
		MO_DETACHSTANDING|MO_SETCHANGEFLAG|MO_MOVESTANDINGONS|MO_TELEPORT);
}

LTRESULT si_TeleportObject(HOBJECT hObj, const LTVector *pNewPos)
{
	if (!hObj)
	{
		RETURN_ERROR(1, ILTPhysics::TeleportObject, LT_INVALIDPARAMS);
	}

	si_SetObjectPos(hObj, pNewPos);
	return LT_OK;
}

void si_TiltToPlane(HOBJECT hObj, const LTVector *pNormal)
{
	LTObject *pObj = HandleToServerObj(hObj);

	// Get slope along acceleration...
	LTVector q = pNormal->Cross(pObj->m_Acceleration);
	if (q.MagSqr() <= 0.001f)
		return;

	q.Norm();
	LTVector slopeAccel = q.Cross(*pNormal);

	// Fix acceleration along slope...
	pObj->m_Acceleration = slopeAccel * pObj->m_Acceleration.Mag();
	//acceleration = slopeAccel * acceleration.Mag();
}

LTRESULT si_SetObjectRotation(HOBJECT hObj, const LTRotation *pRotation)
{
	return si_DoObjectRotation(hObj, pRotation, true);
}

uint8 si_GetBlockingPriority(HOBJECT hObj)
{
	if (hObj)
	{
		return HandleToServerObj(hObj)->m_BPriority;
	}
	else
	{
		return 0;
	}
}


LTRESULT si_ClipSprite(HOBJECT hObj, HPOLY hPoly)
{
	if (!hObj)
		return LT_ERROR;

	LTObject *pObj = HandleToServerObj(hObj);
	if (pObj->m_ObjectType != OT_SPRITE)
		return LT_ERROR;

	// Only change if different.
	if (ToSprite(pObj)->m_ClipperPoly != hPoly)
	{
		ToSprite(pObj)->m_ClipperPoly = hPoly;
		SetObjectChangeFlags(pObj, CF_MODELINFO);
	}

	return LT_OK;
}


void si_SetBlockingPriority(HOBJECT hObj, uint8 pri)
{
	if (hObj)
	{
		HandleToServerObj(hObj)->m_BPriority = pri;
	}
}



void si_GetLightColor(HOBJECT hObject, float *r, float *g, float *b)
{
	if (!hObject)
		return;

	si_GetObjectColor(hObject, r, g, b, LTNULL);
}


void si_SetLightColor(HOBJECT hObject, float r, float g, float b)
{
	if (!hObject)
		return;

	LTObject *pObject = HandleToServerObj(hObject);
	si_SetObjectColor(hObject, r, g, b, (float)pObject->m_ColorA / 255.0f);
}


float si_GetLightRadius(HOBJECT hObj)
{
	LTObject *pObj = HandleToServerObj(hObj);
	if (pObj->m_ObjectType == OT_LIGHT)
	{
		return ToDynamicLight(pObj)->m_LightRadius;
	}
	else
	{
		return 1.0f;
	}
}


void si_SetLightRadius(HOBJECT hObj, float radius)
{
	LTObject *pObj = HandleToServerObj(hObj);
	if (pObj->m_ObjectType == OT_LIGHT)
	{
		ToDynamicLight(pObj)->m_LightRadius = radius;
		SetObjectChangeFlags(pObj, CF_RENDERINFO);
	}
}

void si_SetModelPlaying(HOBJECT hObj, bool bPlaying)
{
	LTObject *pObj = HandleToServerObj(hObj);
	if (pObj->m_ObjectType != OT_MODEL)
		return;

	LTAnimTracker *pTracker = &ToModel(pObj)->m_AnimTracker;
	if (((pTracker->m_Flags & AT_PLAYING) != 0) != bPlaying)
	{
		trk_Play(pTracker, bPlaying != 0);
		SetObjectChangeFlags(hObj, CF_MODELINFO);
	}
}


bool si_GetModelPlaying(HOBJECT hObj) 
{
	LTObject *pObj = HandleToServerObj(hObj);
	if (pObj->m_ObjectType != OT_MODEL) 
		return false;

	LTAnimTracker *pTracker = &ToModel(pObj)->m_AnimTracker;
	return (pTracker->m_Flags & AT_PLAYING) != 0;
}


bool si_GetModelFilenames(HOBJECT hObj, char *pFilename, int fileBufLen, char *pSkinName, int skinBufLen) 
{
	if (!hObj) return 
		false;

	LTObject *pObj = HandleToServerObj(hObj);

	if (pObj->m_ObjectType != OT_MODEL) 
		return false;

	if (ilt_model_server->GetModelDBFilename( hObj, pFilename, fileBufLen ) != LT_OK)
	{
		pFilename[0] = 0;
	}

	if (pObj->sd->m_pSkin) 
	{
		LTStrCpy(pSkinName, server_filemgr->GetUsedFilename(pObj->sd->m_pSkin), skinBufLen);
	}
	else 
	{
		pSkinName[0] = 0;
	}

	return true;
}


// Parses string similar to command line
static int si_Parse(const char *pCommand, const char **pNewCommandPos, char *argBuffer, char **argPointers, int *nArgs) 
{
	return cp_Parse(pCommand, pNewCommandPos, argBuffer, argPointers, nArgs);
}

// Callback for CLTServer::GetPointContainers

void GPCCallback(WorldTreeObj *pObj, void *pUser)
{
	if (pObj->GetObjType() != WTObj_DObject) 
		return;

	ContainerInstance *pContainer = (ContainerInstance*)pObj;
	if (pContainer->m_ObjectType != OT_CONTAINER) 
		return;

	GPCStruct *pStruct = (GPCStruct*)pUser;
	if (pStruct->m_CurListSize >= pStruct->m_MaxListSize)
	{
		return;
	}

	WorldBsp *pWorldBsp = (WorldBsp*)pContainer->m_pOriginalBsp;
	float dist = pStruct->m_Point.DistSqr(pContainer->GetPos());
	if (dist > pWorldBsp->GetBoundRadiusSqr()) 
		return;

	// Transform the point..
	LTVector transformedPoint;
	MatVMul_H(&transformedPoint, &pContainer->m_BackTransform, &pStruct->m_Point);

	if (!ci_IsPointInsideBSP(pWorldBsp->m_RootNode, transformedPoint)) 
	{
		pStruct->m_pList[pStruct->m_CurListSize] = pContainer;
		pStruct->m_CurListSize++;
	}
}



LTRESULT si_SaveObjects(const char *pszSaveFileName, ObjectList *pList, uint32 dwParam, uint32 flags) 
{
	ILTStream *pStream = streamsim_Open(pszSaveFileName, "wb");
	if (!pStream) 
		RETURN_ERROR(2, ILTPhysics::SaveObjects, LT_ERROR);

	sm_SaveObjects(pStream, pList, dwParam, flags);
	pStream->Release();
	return LT_OK;
}

LTRESULT si_RestoreObjects(const char *pszRestoreFileName, uint32 dwParam, uint32 flags) 
{
	ILTStream *pStream = streamsim_Open(pszRestoreFileName, "rb");
	if (!pStream) 
		RETURN_ERROR(2, ILTPhysics::RestoreObjects, LT_ERROR);

	LTRESULT ret = sm_RestoreObjects(pStream, dwParam, flags);
	pStream->Release();
	return ret;
}

LTRESULT si_LoadWorld(const char *pszWorldFileName, uint32 flags) 
{
	return g_pServerMgr->DoStartWorld(pszWorldFileName, flags, g_pServerMgr->m_nTrueLastTimeMS);
}

LTRESULT si_RunWorld() 
{
	return g_pServerMgr->DoRunWorld();
}

LTRESULT si_UpdateSessionName(const char* sName) 
{
	return g_pServerMgr->m_NetMgr.SetSessionName(sName);
}

LTRESULT si_GetSessionName(char* sName, uint32 dwBufferSize) 
{
	if (!sName) 
	{
		RETURN_ERROR(1, ILTPhysics::GetSessionName, LT_INVALIDPARAMS);
	}

	return g_pServerMgr->m_NetMgr.GetSessionName(sName, dwBufferSize);
}

LTRESULT si_GetTcpIpAddress(char* sAddress, uint32 dwBufferSize, uint16 &hostPort) 
{
	if (!sAddress) 
	{
		RETURN_ERROR(1, ILTPhysics::GetTcpIpAddress, LT_INVALIDPARAMS);
	}

	return g_pServerMgr->m_NetMgr.GetLocalIpAddress(sAddress, dwBufferSize, hostPort);
}

LTRESULT si_GetMaxConnections(uint32 &nMaxConnections)
{

	return g_pServerMgr->m_NetMgr.GetMaxConnections(nMaxConnections);
}

LTRESULT si_SendToServerApp( ILTMessage_Read& msg ) 
{
	if (!g_pServerMgr->m_pServerAppHandler) 
		return LT_NOTFOUND;

	return g_pServerMgr->m_pServerAppHandler->ShellMessageFn( msg );
}

FileEntry* si_GetFileList(const char *pDirName) 
{
	if (!pDirName) 
		return LTNULL;

	return server_filemgr->GetFileList(pDirName);
}

void CLTServer::SetupFunctionPointers() 
{
	GetGameInfo = si_GetGameInfo;
	GetClass = si_GetClass;
	GetStaticObject = si_GetStaticObject;
	GetObjectClass = si_GetObjectClass;
	IsKindOf = si_IsKindOf;

	CreateObject = si_CreateObject;
	CreateObjectProps = si_CreateObjectProps;

	GetServerFlags = si_GetServerFlags;
	SetServerFlags = si_SetServerFlags;

	CacheFile = si_CacheFile;

	StartCounter = ic_StartCounter;
	EndCounter = ic_EndCounter;
	Random = ic_Random;
	IntRandom = si_IntRandom;
	RandomScale = si_RandomScale;
	BPrint = si_BPrint;
	DebugOut = ::DebugOut;

	GetSkyDef = si_GetSkyDef;
	SetSkyDef = si_SetSkyDef;
	AddObjectToSky = si_AddObjectToSky;
	RemoveObjectFromSky = si_RemoveObjectFromSky;

	GetGlobalLightObject = si_GetGlobalLightObject;
	SetGlobalLightObject = si_SetGlobalLightObject;

	IntersectSegment = ServerIntersectSegment;
	CastRay = si_CastRay;

	GetObjectScale = si_GetObjectScale;
	ScaleObject = si_ScaleObject;

	FindObjectsTouchingSphere = si_FindObjectsTouchingSphere;
	GetBoxIntersecters = si_GetBoxIntersectors;
	GetLastCollision = si_GetLastCollision;
	TeleportObject = si_TeleportObject;
	RotateObject = si_RotateObject;
	TiltToPlane = si_TiltToPlane;
	SetObjectPos = si_SetObjectPos;
	SetObjectRotation = si_SetObjectRotation;
	GetPointShade = si_GetPointShade;
	RelinquishList = si_RelinquishList;
	ObjectToHandle = si_ObjectToHandle;
	HandleToObject = si_HandleToObject;

	CreateObjectList = si_CreateObjectList;
	AddObjectToList = si_AddObjectToList;
	RemoveObjectFromList = si_RemoveObjectFromList;

	GetPropString = si_GetPropString;
	GetPropVector = si_GetPropVector;
	GetPropColor = si_GetPropVector;
	GetPropReal = si_GetPropReal;
	GetPropFlags = si_GetPropFlags;
	GetPropBool = si_GetPropBool;
	GetPropLongInt = si_GetPropLongInt;
	GetPropRotation = si_GetPropRotation;
	GetPropRotationEuler = si_GetPropRotationEuler;
	GetPropGeneric = si_GetPropGeneric;
	DoesPropExist = si_DoesPropExist;

	AttachClient = si_AttachClient;
	DetachClient = si_DetachClient;
	GetNextClient = si_GetNextClient;

	GetNextClientRef = si_GetNextClientRef;
	GetClientRefInfoFlags = si_GetClientRefInfoFlags;
	GetClientRefName = si_GetClientRefName;
	GetClientRefObject = si_GetClientRefObject;

	GetClientID = si_GetClientID;
	GetClientHandle = si_GetClientHandle;
	GetClientName = si_GetClientName;
	SetClientName = si_SetClientName;
	SetClientInfoFlags = si_SetClientInfoFlags;
	GetClientInfoFlags = si_GetClientInfoFlags;
	SetClientUserData = si_SetClientUserData;
	GetClientUserData = si_GetClientUserData;
	KickClient = si_KickClient;
	SetClientViewPos = si_SetClientViewPos;

	RunGameConString = si_RunGameConString;
	SetGameConVar = si_SetGameConVar;
	GetGameConVar = si_GetGameConVar;

	UpperStrcmp = ic_UpperStrcmp;

	CreateInterObjectLink = si_CreateInterObjectLink;

	CreateAttachment = si_CreateAttachment;
	RemoveAttachment = si_RemoveAttachment;
	FindAttachment = si_FindAttachment;

	GetObjectColor = si_GetObjectColor;
	SetObjectColor = si_SetObjectColor;

	GetNextObject = si_GetNextObject;
	GetNextInactiveObject = si_GetNextInactiveObject;
	SetNextUpdate = si_SetNextUpdate;

	GetBlockingPriority = si_GetBlockingPriority;
	SetBlockingPriority = si_SetBlockingPriority;

	ClipSprite = si_ClipSprite;

	GetObjectState = si_GetObjectState;
	SetObjectState = si_SetObjectState;

	GetLightColor = si_GetLightColor;
	SetLightColor = si_SetLightColor;
	GetLightRadius = si_GetLightRadius;
	SetLightRadius = si_SetLightRadius;

	GetAnimName = ic_GetAnimName;
	SetModelPlaying = si_SetModelPlaying;
	GetModelPlaying = si_GetModelPlaying;

	GetModelFilenames = si_GetModelFilenames;
	Parse = si_Parse;

	SaveObjects = si_SaveObjects;
	RestoreObjects = si_RestoreObjects;

	LoadWorld = si_LoadWorld;
	RunWorld = si_RunWorld;

	UpdateSessionName = si_UpdateSessionName;
	GetSessionName = si_GetSessionName;
	GetMaxConnections = si_GetMaxConnections;
	SendToServerApp = si_SendToServerApp;
	GetTcpIpAddress = si_GetTcpIpAddress;

	GetFileList = si_GetFileList;
	FreeFileList = ic_FreeFileList;
}

