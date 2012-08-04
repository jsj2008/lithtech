#include "bdefs.h"

#include "iltphysics.h"
#include "de_objects.h"
#include "moveobject.h"
#include "syscounter.h"
#include "clientmgr.h"
#include "cmoveabstract.h"
#include "particlesystem.h"
#include "clientshell.h"
#include "impl_common.h"
#include "console.h"
#include "stringmgr.h"
#include "consolecommands.h"
#include "sysinput.h"
#include "setupobject.h"
#include "polygrid.h"
#include "volumeeffect.h"
#include "sprite.h"
#include "conparse.h"
#include "sysdebugging.h"
#include "linesystem.h"
#include "sysclientde_impl.h"
#include "sysstreamsim.h"
#include "ltsysoptim.h"

#include "strtools.h"

#include "render.h"

#include "ltobjref.h"
#include "ltobjectcreate.h"

#include "iltmessage.h"
#include "ltmessage_client.h"

#include "collision.h"

#include "ltrendererstats.h"
#include "rendererframestats.h"

#include "ltvertexshadermgr.h"
#include "ltpixelshadermgr.h"
#include "lteffectshadermgr.h"
#include "ltinfo_impl.h"

//------------------------------------------------------------------
//------------------------------------------------------------------
// Holders and their headers.
//------------------------------------------------------------------
//------------------------------------------------------------------

//IWorldClientBSP holder
#include "world_client_bsp.h"
static IWorldClientBSP *world_bsp_client;
define_holder(IWorldClientBSP, world_bsp_client);

//IWorldSharedBSP holder
#include "world_shared_bsp.h"
static IWorldSharedBSP *world_bsp_shared;
define_holder(IWorldSharedBSP, world_bsp_shared);

//IClientFileMgr
#include "client_filemgr.h"
static IClientFileMgr *client_file_mgr;
define_holder(IClientFileMgr, client_file_mgr);

//IClientShell game client shell object.
#include "iclientshell.h"
static IClientShell *i_client_shell;
define_holder(IClientShell, i_client_shell);

//the ILTClient game interface
#include "iltclient.h"
static ILTClient *ilt_client;
define_holder(ILTClient, ilt_client);

//IWorldBlindObjectData holder
#include "world_blind_object_data.h"
static IWorldBlindObjectData *world_blind_object_data;
define_holder(IWorldBlindObjectData, world_blind_object_data);

//ILTTransform game interface.
#include "ilttransform.h"
static ILTTransform *ilt_transform;
define_holder(ILTTransform, ilt_transform);

//ILTCommon client instance.
#include "iltcommon.h"
static ILTCommon *ilt_common_client;
define_holder_to_instance(ILTCommon, ilt_common_client, Client);

//ILTModel game interface client instance.
#include "iltmodel.h"
static ILTModel *ilt_model_client;
define_holder_to_instance(ILTModel, ilt_model_client, Client);

//ILTSoundMgr game interface client version.
#include "iltsoundmgr.h"
static ILTSoundMgr *ilt_soundmgr_client;
define_holder_to_instance(ILTSoundMgr, ilt_soundmgr_client, Client);

//ILTVideoMgr game interface
#include "iltvideomgr.h"
static ILTVideoMgr *ilt_videomgr;
define_holder(ILTVideoMgr, ilt_videomgr);

//ILTDrawPrim interface.
#include "iltdrawprim.h"
static ILTDrawPrim *ilt_drawprim;
define_holder(ILTDrawPrim, ilt_drawprim);

//the ILTTexInterface interface.
#include "ilttexinterface.h"
static ILTTexInterface *ilt_texinterface;
define_holder(ILTTexInterface, ilt_texinterface);

//the ILTFontManager interface.
#include "iltfontmanager.h"
static ILTFontManager *ilt_fontmanager;
define_holder(ILTFontManager, ilt_fontmanager);

//the ILTCursor game interface
#include "iltcursor.h"
static ILTCursor *ilt_cursor;
define_holder(ILTCursor, ilt_cursor);

//the ILTDirectMusicMgr game interface.
#include "iltdirectmusic.h"
static ILTDirectMusicMgr *ilt_directmusicmgr;
define_holder(ILTDirectMusicMgr, ilt_directmusicmgr);

//the ILTBenchmarkMgr game interface.
#include "iltbenchmark.h"
static ILTBenchmarkMgr *ilt_benchmarkmgr;
define_holder(ILTBenchmarkMgr, ilt_benchmarkmgr);

//ILTWidgetManager game interface.
#include "iltwidgetmanager.h"
static ILTWidgetManager *ilt_WidgetManager;
define_holder(ILTWidgetManager, ilt_WidgetManager);

//ILTPhysics game interface
#include "iltphysics.h"
static ILTPhysics *ilt_physics_client;
define_holder_to_instance(ILTPhysics, ilt_physics_client, Client);

//IWorldParticleBlockerData holder
#include "world_particle_blocker_data.h"
static IWorldParticleBlockerData *world_particle_blocker_data;
define_holder(IWorldParticleBlockerData, world_particle_blocker_data);

#include "iltmath.h"
static ILTMath *ilt_math;
define_holder(ILTMath, ilt_math);

// ---------------------------------
// Begin undocumented random puff.
// ---------------------------------


class TempObjArray
{
public:
	LTVector	m_Point; // Used by GetPointContainers.

	LTObject	**m_pObjects;
	uint32	  m_nMaxObjects;

	uint32	  *m_pNumObjects;
	uint32	  *m_pTotalNumFound;
};



extern int32 g_CV_ForceClear;
extern int32 g_CV_ForceSoundDisable;

extern uint32 g_Ticks_MoveObject;
extern uint32 g_nMoveObjectCalls;

// ----------------------------------------------------------------- //
// Forward Declarations
// ----------------------------------------------------------------- //
void ci_GetPointContainersCB(WorldTreeObj *pObj, void *pUser);




LTRESULT ci_GetObjectScale(HLOCALOBJ hObj, LTVector *pScale)
{
	if (!hObj)
		RETURN_ERROR(1, CLTClient::GetObjectScale, LT_INVALIDPARAMS);

	VEC_COPY(*pScale, ((LTObject*)hObj)->m_Scale);
	return LT_OK;
}


LTRESULT ci_SetObjectScale(HLOCALOBJ hObj, const LTVector *pScale)
{
	if (!hObj)
		RETURN_ERROR(1, CLTClient::SetObjectScale, LT_INVALIDPARAMS);

	LTObject *pObj = (LTObject*)hObj;

	if (pObj->m_Scale == *pScale)
		return LT_OK;

	g_pClientMgr->ScaleObject(pObj, pScale);

	return LT_OK;
}

void ci_SetObjectRotation(HLOCALOBJ hObj, const LTRotation *pRotation)
{
	if (!hObj || !pRotation)
		return;

	if (hObj->m_Rotation == *pRotation)
		return;

	g_pClientMgr->RotateObject((LTObject*)hObj, pRotation);
}

void ci_SetObjectPosAndRotation(HLOCALOBJ hObj, const LTVector *pPos, const LTRotation *pRotation)
{
	if (!hObj || !pPos || !pRotation)
		return;

	if (hObj->m_Pos == *pPos && hObj->m_Rotation == *pRotation)
		return;

	g_pClientMgr->MoveAndRotateObject((LTObject*)hObj, pPos, pRotation);
}

void ci_FindObjectsInSphereCB(WorldTreeObj *pObj, void *pUser)
{
	if (pObj->GetObjType() != WTObj_DObject)
		return;

	LTObject *pObject = (LTObject*)pObj;
	TempObjArray *pStruct = (TempObjArray*)pUser;

	if (*pStruct->m_pNumObjects < pStruct->m_nMaxObjects)
	{
		pStruct->m_pObjects[*pStruct->m_pNumObjects] = pObject;
		++(*pStruct->m_pNumObjects);
	}

	++(*pStruct->m_pTotalNumFound);
}


LTRESULT ci_FindObjectsInSphere(const LTVector *pCenter, float radius,
	HLOCALOBJ *inObjects, uint32 nInObjects, uint32 *nOutObjects, uint32 *nFound)
{
	float radius_sqr = radius * radius;

	*nOutObjects = 0;
	*nFound = 0;

	// Just search them all.
	for (uint32 i = 0; i < NUM_OBJECTTYPES; i++)
	{
		LTLink *pListHead = &g_pClientMgr->m_ObjectMgr.m_ObjectLists[i].m_Head;
		for (LTLink *pCur = pListHead->m_pNext; pCur != pListHead; pCur = pCur->m_pNext)
		{
			LTObject *pObject = (LTObject*)pCur->m_pData;

			float test = (pObject->GetPos() - *pCenter).MagSqr();
			if (test < radius_sqr)
			{
				if (*nOutObjects < nInObjects)
				{
					inObjects[*nOutObjects] = (HLOCALOBJ)pObject;
					++(*nOutObjects);
				}

				++(*nFound);
			}
		}
	}

	return LT_OK;
}



LTRESULT ci_FindObjectsInBox(const LTVector *pCenter, float radius,
	HLOCALOBJ *inObjects, uint32 nInObjects, uint32 *nOutObjects, uint32 *nFound)
{
	*nOutObjects = 0;
	*nFound = 0;

	WorldTree *pWorldTree = world_bsp_client->ClientTree();
	if (!pWorldTree)
		return LT_NOTFOUND;

	LTVector boxMin = *pCenter - LTVector(radius, radius, radius);
	LTVector boxMax = *pCenter + LTVector(radius, radius, radius);

	TempObjArray theArray;
	theArray.m_pObjects = inObjects;
	theArray.m_nMaxObjects = nInObjects;
	theArray.m_pNumObjects = nOutObjects;
	theArray.m_pTotalNumFound = nFound;

	pWorldTree->FindObjectsInBox(&boxMin, &boxMax, ci_FindObjectsInSphereCB,
		&theArray);

	return LT_OK;
}

//
//ILTClient implementation.
//

class CLTClient : public ILTClient {
  public:
	declare_interface(CLTClient);

	CLTClient();
	~CLTClient();

	void InitFunctionPointers();
	void Term();

	//////////////////////////////////////////////////////////////////////////////
	// ILTCSBase implementation

	// Sub-interface accessors
	virtual ILTCommon *Common();
	virtual ILTPhysics *Physics();
	virtual ILTModel *GetModelLT();
	virtual ILTTransform *GetTransformLT();
	virtual ILTSoundMgr *SoundMgr();
	virtual ILTMath *Math();

	virtual LTRESULT OpenFile(const char *pFilename, ILTStream **pStream);
	virtual LTRESULT CopyFile(const char *pszSourceFile, const char *pszDestFile);

	virtual LTRESULT GetObjectPos(HLOCALOBJ hObj, LTVector *pPos);
	virtual LTRESULT GetObjectRotation(HLOCALOBJ hObj, LTRotation *pRot);

	virtual LTRESULT SendTo(ILTMessage_Read *pMsg, const char *sAddr, uint16 port);
	virtual LTRESULT StartPing(const char *pAddr, uint16 nPort, uint32 *pPingID);
	virtual LTRESULT GetPingStatus(uint32 nPingID, uint32 *pStatus, uint32 *pLatency);
	virtual LTRESULT RemovePing(uint32 nPingID);

	virtual uint32 GetModelAnimation(HLOCALOBJ hObj);
	virtual void SetModelAnimation(HLOCALOBJ hObj, uint32 iAnim);

	virtual LTRESULT ResetModelAnimation(HLOCALOBJ hObj);

	virtual uint32 GetModelPlaybackState(HLOCALOBJ hObj);
	virtual bool GetModelLooping(HLOCALOBJ hObj);
	virtual void SetModelLooping(HLOCALOBJ hObj, bool bLoop);
	virtual HMODELANIM GetAnimIndex(HOBJECT hObj, const char *pAnimName);

	// vertex shaders
	virtual bool				AddVertexShader(const char *pFileName, int VertexShaderID,
												const uint32 *pVertexElements, uint32 VertexElementsSize,
												bool bCompileShader);
	virtual void				RemoveVertexShader(int VertexShaderID);
	virtual void				RemoveAllVertexShaders();
	virtual LTVertexShader*		GetVertexShader(int VertexShaderID);

	// pixel shaders
	virtual bool				AddPixelShader(const char *pFileName, int PixelShaderID, bool bCompileShader);
	virtual void				RemovePixelShader(int PixelShaderID);
	virtual void				RemoveAllPixelShaders();
	virtual LTPixelShader*		GetPixelShader(int PixelShaderID);

	virtual void CPrint(const char *pMsg, ...);

	virtual uint32 GetPointContainers(const LTVector *pPoint, HLOCALOBJ *pList, uint32 maxListSize);
	virtual bool GetContainerCode(HLOCALOBJ hObj, uint16 *pCode);
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
	virtual float GetVarValueFloat(HCONSOLEVAR hVar);
	virtual const char* GetVarValueString(HCONSOLEVAR hVar);

	virtual LTFLOAT GetTime();
	virtual LTFLOAT GetFrameTime();
	virtual LTRESULT GetSourceWorldOffset(LTVector& vOffset);
	virtual LTRESULT RemoveObject(HLOCALOBJ hObj);

	virtual LTRESULT GetBlindObjectData(uint32 nNum, uint32 nId, uint8*& pData, uint32& nSize);
	virtual LTRESULT FreeBlindObjectData(uint32 nNum, uint32 nId);

	virtual LTRESULT OpenMemoryStream(ILTStream **pStream, uint32 nCacheSize);

	virtual LTRESULT GetLightGroupID(const char *pName, uint32 *pResult) const;
	virtual LTRESULT GetOccluderID(const char *pName, uint32 *pResult) const;
	virtual LTRESULT GetTextureEffectVarID(const char *pName, uint32 nStage, uint32 *pResult) const;

	virtual LTRESULT LinkObjRef(HOBJECT hObj, LTObjRef *pRef);

	//////////////////////////////////////////////////////////////////////////////
	// ILTClient implementation

	//sub-interface accessors from ILTClient
	virtual ILTVideoMgr *VideoMgr();
	virtual ILTDrawPrim *GetDrawPrim();
	virtual ILTTexInterface *GetTexInterface();
	virtual ILTFontManager *GetFontManager();
	virtual ILTCursor *Cursor();
	virtual ILTDirectMusicMgr *GetDirectMusicMgr();
	virtual ILTBenchmarkMgr *GetBenchmarkMgr();
	virtual ILTWidgetManager *GetWidgetManager();


	virtual LTRESULT SetObjectPos(HLOCALOBJ hObj, const LTVector *pPos, bool bForce);

	virtual LTParticle* AddParticle(HLOCALOBJ hObj, const LTVector *pPos, const LTVector *pVelocity, const LTVector *pColor, float lifeTime);
	virtual LTRESULT StartQuery(const char *pInfo);
	virtual LTRESULT UpdateQuery();
	virtual LTRESULT GetQueryResults(NetSession* &pListHead);
	virtual LTRESULT EndQuery();
	virtual LTRESULT GetSConValueFloat(const char *pName, float &val);
	virtual LTRESULT GetSConValueString(const char *pName, char *valBuf, uint32 bufLen);

	virtual LTRESULT SendToServer(ILTMessage_Read *pMsg, uint32 flags);

	virtual LTRESULT GetServerIPAddress(uint8 pAddr[4], uint16 *pPort);

	virtual LTRESULT ProcessAttachments(HOBJECT pObj);

	virtual LTRESULT GetSpriteControl(HLOCALOBJ hObj, ILTSpriteControl* &pControl);
	virtual LTRESULT SetSpriteEffectShaderID(HLOCALOBJ hObj, uint32 EffectShaderID);

	virtual LTRESULT GetCanvasFn(HOBJECT hCanvas, CanvasDrawFn &fn, void* &pUserData);
	virtual LTRESULT SetCanvasFn(HOBJECT hCanvas, CanvasDrawFn fn, void* pUserData);
	virtual LTRESULT GetCanvasRadius(HOBJECT hCanvas, float &radius);
	virtual LTRESULT SetCanvasRadius(HOBJECT hCanvas, float radius);

	virtual LTRESULT GetMaxRadiusInPoly(const HPOLY hPoly, const LTVector& vPos, float& fMaxRadius);

    //Renderer Stats
    virtual LTRESULT GetRendererStats(LTRendererStats &refStats);

	//Object Render Groups
	virtual LTRESULT SetObjectRenderGroup(HOBJECT hObj, uint32 nGroup);
	virtual LTRESULT SetObjectRenderGroupEnabled(uint32 nGroup, bool bEnabled);
	virtual LTRESULT SetAllObjectRenderGroupEnabled();

	//Glow mappings
	virtual LTRESULT AddGlowRenderStyleMapping(const char* pszSource, const char* pszMapTo);
	virtual LTRESULT SetGlowDefaultRenderStyle(const char* pszFilename);
	virtual LTRESULT SetNoGlowRenderStyle(const char* pszFilename);

	// Global light stuff
	virtual LTRESULT GetGlobalLightDir(LTVector &dir);
	virtual LTRESULT SetGlobalLightDir(const LTVector &dir);

	virtual LTRESULT GetGlobalLightColor(LTVector &color);
	virtual LTRESULT SetGlobalLightColor(const LTVector& color);
	virtual LTRESULT GetGlobalLightConvertToAmbient(float& fConvertToAmbient);
	virtual LTRESULT SetGlobalLightConvertToAmbient(float fConvertToAmbient);

	virtual LTRESULT GetPointShade(const LTVector *pPoint, LTVector *pColor);

	virtual LTRESULT GetLightGroupColor(uint32 nID, LTVector *pColor) const;
	virtual LTRESULT SetLightGroupColor(uint32 nID, const LTVector &vColor);

	virtual LTRESULT GetOccluderEnabled(uint32 nID, bool *pResult) const;
	virtual LTRESULT SetOccluderEnabled(uint32 nID, bool bEnabled);
	virtual LTRESULT SetTextureEffectVar(uint32 nID, uint32 nVar, float fVal);

	// Other
	virtual LTRESULT IsLocalToServer(bool *bResult);

	// Input
	virtual LTRESULT GetDeviceObjectName( char const* pszDeviceName, uint32 nObjectId,
		char* pszDeviceObjectName, uint32 nDeviceObjectNameLen );
};

//instantiate our ILTClient implementation class
define_interface(CLTClient, ILTClient);

//we have delayed initialization, so we keep track of a pointer to the
//implementation class.
static CLTClient *ilt_client_imp = NULL;

CLTClient::CLTClient()
{
	//save the pointer to the instance of this class.
	ilt_client_imp = this;
}

CLTClient::~CLTClient()
{
	Term();
}

void CLTClient::Term()
{
}

ILTCommon *CLTClient::Common()
{
	return ilt_common_client;
}

ILTPhysics *CLTClient::Physics()
{
	return ilt_physics_client;
}

ILTModel *CLTClient::GetModelLT()
{
	return ilt_model_client;
}

ILTTransform *CLTClient::GetTransformLT()
{
	return ilt_transform;
}

ILTSoundMgr *CLTClient::SoundMgr()
{
	return ilt_soundmgr_client;
}

ILTVideoMgr *CLTClient::VideoMgr()
{
	return ilt_videomgr;
}

ILTDrawPrim *CLTClient::GetDrawPrim()
{
	return ilt_drawprim;
}

ILTTexInterface *CLTClient::GetTexInterface()
{
	return ilt_texinterface;
}

ILTFontManager *CLTClient::GetFontManager()
{
	return ilt_fontmanager;
}

ILTWidgetManager *CLTClient::GetWidgetManager()
{
	return ilt_WidgetManager;
}

ILTCursor *CLTClient::Cursor()
{
	return ilt_cursor;
}

ILTDirectMusicMgr *CLTClient::GetDirectMusicMgr()
{
	return ilt_directmusicmgr;
}

ILTBenchmarkMgr *CLTClient::GetBenchmarkMgr()
{
	return ilt_benchmarkmgr;
}

ILTMath *CLTClient::Math()
{
	return ilt_math;
}

LTRESULT CLTClient::OpenFile(const char *pFilename, ILTStream **pStream)
{
	if (!pFilename || !pStream)
	{
		RETURN_ERROR(1, CLTClient::OpenFile, LT_INVALIDPARAMS);
	}

	FileRef ref;
	ref.m_FileType = FILE_ANYFILE;
	ref.m_pFilename = pFilename;
	*pStream = client_file_mgr->OpenFile(&ref);
	if (*pStream)
	{
		return LT_OK;
	}
	else
	{
		RETURN_ERROR(3, CLTClient::OpenFile, LT_NOTFOUND);
	}
}

LTRESULT CLTClient::CopyFile(const char *pszSourceFile, const char *pszDestFile)
{
	return client_file_mgr->CopyFile(pszSourceFile, pszDestFile );
}

LTRESULT CLTClient::SetObjectPos(HLOCALOBJ hObj, const LTVector *pPos, bool bForce)
{
	FN_NAME(CLTClient::SetObjectPos);

	CHECK_PARAMS2(hObj && pPos);

	if (hObj->m_Pos == *pPos)
		return LT_OK;

	g_pClientMgr->MoveObject(hObj, pPos, bForce);

	return LT_OK;
}

LTRESULT CLTClient::GetObjectPos(HLOCALOBJ hObj, LTVector *pPos)
{
	FN_NAME(CLTClient::GetObjectPos);

	CHECK_PARAMS2(hObj && pPos)

	*pPos = hObj->GetPos();

	return LT_OK;
}

LTRESULT CLTClient::GetObjectRotation(HLOCALOBJ hObj, LTRotation *pRotation)
{
	FN_NAME(CLTClient::GetObjectRotation);
	CHECK_PARAMS2(hObj && pRotation)

	LTObject *pObject = (LTObject*)hObj;

	*pRotation = pObject->m_Rotation;
	return LT_OK;
}


LTParticle *CLTClient::AddParticle(HLOCALOBJ hObj, const LTVector *pPos, const LTVector *pVelocity, const LTVector *pColor, float lifeTime)
{
	LTParticleSystem *pSystem = (LTParticleSystem*)hObj;

	if (pSystem && pSystem->m_ObjectType == OT_PARTICLESYSTEM)
	{
		return (LTParticle*)ps_AddParticle(pSystem, pPos, pColor, pVelocity, lifeTime);
	}
	else
	{
		return LTNULL;
	}
}

LTRESULT CLTClient::StartQuery(const char *pInfo)
{
	CBaseDriver *pDriver = g_pClientMgr->m_NetMgr.GetMainDriver();

	if (!pDriver)
		RETURN_ERROR(1, StartQuery, LT_NOTINITIALIZED);

	return pDriver->StartQuery(pInfo);
}

LTRESULT CLTClient::UpdateQuery()
{
	CBaseDriver *pDriver = g_pClientMgr->m_NetMgr.GetMainDriver();

	if (!pDriver)
		RETURN_ERROR(1, UpdateQuery, LT_NOTINITIALIZED);

	return pDriver->UpdateQuery();
}

LTRESULT CLTClient::GetQueryResults(NetSession* &pListHead)
{
	CBaseDriver *pDriver = g_pClientMgr->m_NetMgr.GetMainDriver();

	pListHead = LTNULL;
	if (!pDriver)
		RETURN_ERROR(1, GetQueryResults, LT_NOTINITIALIZED);

	return pDriver->GetQueryResults(pListHead);
}

LTRESULT CLTClient::EndQuery()
{
	CBaseDriver *pDriver = g_pClientMgr->m_NetMgr.GetMainDriver();

	if (!pDriver)
		RETURN_ERROR(1, EndQuery, LT_NOTINITIALIZED);

	return pDriver->EndQuery();
}

LTRESULT CLTClient::GetSConValueFloat(const char *pName, float &val)
{
	// Check and init..
	CHECK_PARAMS(pName, CLTClient::GetSConValueFloat);

	val = 0.0f;

	LTCommandVar *pVar = cc_FindConsoleVar(&g_pClientMgr->m_ServerConsoleMirror, pName);
	if (!pVar)
		RETURN_ERROR_PARAM(2, CLTClient::GetSConValueFloat, LT_NOTFOUND, pName);

	val = pVar->floatVal;
	return LT_OK;
}

LTRESULT CLTClient::GetSConValueString(const char *pName, char *valBuf, uint32 bufLen)
{
	// Check and init..
	CHECK_PARAMS(pName && (bufLen>0), CLTClient::GetSConValueString);
	valBuf[0] = 0;

	LTCommandVar *pVar = cc_FindConsoleVar(&g_pClientMgr->m_ServerConsoleMirror, pName);
	if (!pVar)
		RETURN_ERROR_PARAM(2, CLTClient::GetSConValueString, LT_NOTFOUND, pName);

	LTStrCpy(valBuf, pVar->pStringVal, bufLen);

	return LT_OK;
}

LTRESULT CLTClient::SendToServer(ILTMessage_Read *pMsg, uint32 flags)
{
	CLTMsgRef_Read cRefMsg(pMsg);

	if (!g_pClientMgr->m_pCurShell)
	{
		RETURN_ERROR(2, CLTClient::SendToServer, LT_NOTINITIALIZED);
	}

	CHECK_PARAMS(pMsg, CLTClient::SendToServer);

	CLTMessage_Read *pPacketMsg = (CLTMessage_Read *)pMsg;

	// Build a new packet with the "game message" ID
	CPacket_Write cNewPacket;
	cNewPacket.Writeuint8(CMSG_MESSAGE);
	cNewPacket.WritePacket(pPacketMsg->GetPacket());

	// Send it
	bool bResult = g_pClientMgr->m_NetMgr.SendPacket(CPacket_Read(cNewPacket), g_pClientMgr->m_pCurShell->m_HostID, flags);

	if (!bResult)
		return LT_ERROR;

	return LT_OK;
}

LTRESULT CLTClient::SendTo(ILTMessage_Read *pMsg, const char *sAddr, uint16 port)
{
	CLTMsgRef_Read cRefMsg(pMsg);

	CLTMessage_Read *pPacketMsg = (CLTMessage_Read *)pMsg;

	// Handle it through the proper channels if it's being sent to the server
	// (Note : This is only something we can check for if it's a numeric address...)
	if (isdigit(sAddr[0]) && g_pClientMgr->m_pCurShell && g_pClientMgr->m_pCurShell->m_HostID)
	{
		uint8 nDestAddr[4];
		uint32 nTempDestAddr[4];
		sscanf(sAddr, "%d.%d.%d.%d", &nTempDestAddr[0], &nTempDestAddr[1], &nTempDestAddr[2], &nTempDestAddr[3]);
		nDestAddr[0] = (uint8)nTempDestAddr[0];
		nDestAddr[1] = (uint8)nTempDestAddr[1];
		nDestAddr[2] = (uint8)nTempDestAddr[2];
		nDestAddr[3] = (uint8)nTempDestAddr[3];
		uint8 nHostAddr[4];
		uint16 nHostPort;
		g_pClientMgr->m_pCurShell->m_HostID->GetIPAddress(nHostAddr, &nHostPort);
		if ((nHostAddr[0] == nDestAddr[0]) &&
			(nHostAddr[1] == nDestAddr[1]) &&
			(nHostAddr[2] == nDestAddr[2]) &&
			(nHostAddr[3] == nDestAddr[3]) &&
			(nHostPort == port))
		{
			if (g_pClientMgr->m_NetMgr.SendPacket(pPacketMsg->GetPacket(), g_pClientMgr->m_pCurShell->m_HostID))
				return LT_OK;
		}
	}

	for (uint32 i=0; i < g_pClientMgr->m_NetMgr.m_Drivers; i++)
	{
		CBaseDriver *pDriver = g_pClientMgr->m_NetMgr.m_Drivers[i];

		if (pDriver->m_DriverFlags & NETDRIVER_TCPIP)
		{
			return pDriver->SendTcpIp(pPacketMsg->GetPacket(), sAddr, port);
		}
	}

	return LT_NOTINITIALIZED;
}

LTRESULT CLTClient::StartPing(const char *pAddr, uint16 nPort, uint32 *pPingID)
{
	// If we don't have any drivers, initialize the networking for them
	if (g_pClientMgr->m_NetMgr.m_Drivers.GetSize() == 0)
		InitNetworking(NULL, 0);

	for (uint32 i=0; i < g_pClientMgr->m_NetMgr.m_Drivers; i++)
	{
		CBaseDriver *pDriver = g_pClientMgr->m_NetMgr.m_Drivers[i];

		if (pDriver->m_DriverFlags & NETDRIVER_TCPIP)
		{
			return pDriver->StartPing(pAddr, nPort, pPingID);
		}
	}

	return LT_NOTINITIALIZED;
}

LTRESULT CLTClient::GetPingStatus(uint32 nPingID, uint32 *pStatus, uint32 *pLatency)
{
	for (uint32 i=0; i < g_pClientMgr->m_NetMgr.m_Drivers; i++)
	{
		CBaseDriver *pDriver = g_pClientMgr->m_NetMgr.m_Drivers[i];

		if (pDriver->m_DriverFlags & NETDRIVER_TCPIP)
		{
			return pDriver->GetPingStatus(nPingID, pStatus, pLatency);
		}
	}

	return LT_NOTINITIALIZED;
}

LTRESULT CLTClient::RemovePing(uint32 nPingID)
{
	for (uint32 i=0; i < g_pClientMgr->m_NetMgr.m_Drivers; i++)
	{
		CBaseDriver *pDriver = g_pClientMgr->m_NetMgr.m_Drivers[i];

		if (pDriver->m_DriverFlags & NETDRIVER_TCPIP)
		{
			return pDriver->RemovePing(nPingID);
		}
	}

	return LT_NOTINITIALIZED;
}

LTRESULT CLTClient::GetServerIPAddress(uint8 pAddr[4], uint16 *pPort)
{
	// check the params
	if (!pAddr || !pPort)
		return LT_INVALIDPARAMS;
	// Check the connection state
	if (!g_pClientMgr || !g_pClientMgr->m_pCurShell || !g_pClientMgr->m_pCurShell->m_HostID)
		return LT_NOTCONNECTED;
	// Get the IP address
	if (!g_pClientMgr->m_pCurShell->m_HostID->GetIPAddress(pAddr, pPort))
		return LT_NOTCONNECTED;
	return LT_OK;
}

LTRESULT CLTClient::ProcessAttachments(HOBJECT pObj)
{
	CHECK_PARAMS(pObj, CLTClient::ProcessAttachments);

	for (Attachment *pAttachment=pObj->m_Attachments;
		pAttachment;
		pAttachment=pAttachment->m_pNext)
	{
		r_ProcessAttachment(pObj, pAttachment);
	}

	return LT_OK;
}

LTRESULT CLTClient::GetSpriteControl(HLOCALOBJ hObj, ILTSpriteControl* &pControl)
{
	CHECK_PARAMS(hObj && hObj->m_ObjectType == OT_SPRITE, CLTClient::GetSpriteControl);
	pControl = &((SpriteInstance*)hObj)->m_SCImpl;
	return LT_OK;
}

LTRESULT CLTClient::SetSpriteEffectShaderID(HLOCALOBJ hObj, uint32 EffectShaderID)
{
	CHECK_PARAMS(hObj && hObj->m_ObjectType == OT_SPRITE, CLTClient::SetSpriteEffectShaderID);
	((SpriteInstance*)hObj)->m_nEffectShaderID = EffectShaderID;
	return LT_OK;
}

LTRESULT CLTClient::GetCanvasFn(HOBJECT hCanvas, CanvasDrawFn &fn, void* &pUserData)
{
	FN_NAME(CLTClient::GetCanvasFn);

	CHECK_PARAMS2(hCanvas && hCanvas->m_ObjectType == OT_CANVAS);

	Canvas *pCanvas = ToCanvas(hCanvas);
	fn = pCanvas->m_Fn;
	pUserData = pCanvas->m_pFnUserData;
	return LT_OK;
}

LTRESULT CLTClient::SetCanvasFn(HOBJECT hCanvas, CanvasDrawFn fn, void* pUserData)
{
	FN_NAME(CLTClient::SetCanvasFn);

	CHECK_PARAMS2(hCanvas && hCanvas->m_ObjectType == OT_CANVAS);

	Canvas *pCanvas = ToCanvas(hCanvas);
	pCanvas->m_Fn = fn;
	pCanvas->m_pFnUserData = pUserData;
	return LT_OK;
}

LTRESULT CLTClient::GetCanvasRadius(HOBJECT hCanvas, float &radius)
{
	FN_NAME(CLTClient::GetCanvasRadius);

	CHECK_PARAMS2(hCanvas && hCanvas->m_ObjectType == OT_CANVAS);

	Canvas *pCanvas = ToCanvas(hCanvas);
	radius = pCanvas->m_CanvasRadius;
	return LT_OK;
}

LTRESULT CLTClient::SetCanvasRadius(HOBJECT hCanvas, float radius)
{
	FN_NAME(CLTClient::GetCanvasRadius);

	CHECK_PARAMS2(hCanvas && hCanvas->m_ObjectType == OT_CANVAS);

	Canvas *pCanvas = ToCanvas(hCanvas);
	pCanvas->m_CanvasRadius = radius;

	g_pClientMgr->RelocateObject(pCanvas);

	return LT_OK;
}

LTRESULT CLTClient::GetMaxRadiusInPoly(const HPOLY hPoly, const LTVector& vPos, float& fMaxRadius)
{
	WorldPoly* pPoly = world_bsp_client->GetPolyFromHPoly(hPoly);

	if (pPoly == NULL)
	{
		// Give them a decent default
		fMaxRadius = 0.0f;
		RETURN_ERROR(5, CLTServer::GetMaxRadiusInPoly, LT_NOTFOUND);
	}

	// Default to some huge value (this will be overridden)
	fMaxRadius = 100000.0f;

	// Alright, we have our polygon, so now let us find the maximum extents
	uint32 nNumPts = pPoly->GetNumVertices();

	// Sanity check for degenerate polygons
	ASSERT(nNumPts > 2);

	// We first off need a normal
	LTVector vNormal = (pPoly->GetVertex(1) - pPoly->GetVertex(0)).Cross(pPoly->GetVertex(2) - pPoly->GetVertex(0));

	LTVector vPrev = pPoly->GetVertex(nNumPts - 1);
	for (uint32 nCurrPt = 0; nCurrPt < nNumPts; nCurrPt++)
	{
		// Grab our vertex
		LTVector vCurr = pPoly->GetVertex(nCurrPt);

		// Build up a plane with this information
		LTVector vEdgeNormal = vNormal.Cross(vCurr - vPrev);
		vEdgeNormal.Normalize();

		// Now find the distance
		float fDist = (vPos - vPrev).Dot(vEdgeNormal);

		// Make our radius the minimum
		if (fDist < fMaxRadius)
			fMaxRadius = fDist;

		if (fMaxRadius <= 0.0f)
		{
			// The point is outside the polygon
			fMaxRadius = 0.0f;
			break;
		}

		vPrev = vCurr;
	}

	// Success
	return LT_OK;
}


LTRESULT CLTClient::GetGlobalLightDir(LTVector &dir)
{
	dir = r_GetRenderStruct()->m_GlobalLightDir;
	return LT_OK;
}

LTRESULT CLTClient::SetGlobalLightDir(const LTVector &dir)
{
	float fDirMag = dir.Mag();
	LTVector vUnitDir = dir / fDirMag;
	if (fDirMag < 0.001f)
		vUnitDir.Init(1.0f, 0.0f, 0.0f);

	r_GetRenderStruct()->m_GlobalLightDir = dir;
	return LT_OK;
}

LTRESULT CLTClient::GetGlobalLightColor(LTVector &color)
{
	color = r_GetRenderStruct()->m_GlobalLightColor;
	return LT_OK;
}

LTRESULT CLTClient::SetGlobalLightColor(const LTVector& color)
{
	r_GetRenderStruct()->m_GlobalLightColor = color;
	return LT_OK;
}

LTRESULT CLTClient::GetGlobalLightConvertToAmbient(float& fConvertToAmbient)
{
	fConvertToAmbient = r_GetRenderStruct()->m_GlobalLightConvertToAmbient;
	return LT_OK;
}

LTRESULT CLTClient::SetGlobalLightConvertToAmbient(float fConvertToAmbient)
{
	r_GetRenderStruct()->m_GlobalLightConvertToAmbient = fConvertToAmbient;
	return LT_OK;
}


LTRESULT CLTClient::GetPointShade(const LTVector *pPoint, LTVector *pColor)
{
	return ilt_common_client->GetPointShade(pPoint, pColor);
}

uint32 CLTClient::GetModelAnimation(HLOCALOBJ hObj)
{
	HMODELANIM iAnim = 0;
	LTRESULT ret = ilt_model_client->GetCurAnim(hObj, MAIN_TRACKER, iAnim);
	return ( ret == LT_OK ) ? iAnim : (uint32)-1;
}

void CLTClient::SetModelAnimation(HLOCALOBJ hObj, uint32 iAnim)
{
	ilt_model_client->SetCurAnim(hObj, MAIN_TRACKER, iAnim);
}

LTRESULT CLTClient::ResetModelAnimation(HLOCALOBJ hObj)
{
	LTRESULT dResult = ilt_model_client->ResetAnim(hObj, MAIN_TRACKER);
	return dResult;
}

uint32 CLTClient::GetModelPlaybackState(HLOCALOBJ hObj)
{
	uint32 state = 0;
	LTRESULT dResult = ilt_model_client->GetPlaybackState(hObj, MAIN_TRACKER, state);
	return state;
}

bool CLTClient::GetModelLooping(HLOCALOBJ hObj)
{
	return ilt_model_client->GetLooping(hObj, MAIN_TRACKER) == LT_YES;
}

void CLTClient::SetModelLooping(HLOCALOBJ hObj, bool bLoop)
{
	ilt_model_client->SetLooping(hObj, MAIN_TRACKER, bLoop);
}

HMODELANIM CLTClient::GetAnimIndex(HOBJECT hObj, const char *pAnimName)
{
	return ic_GetAnimIndex(hObj, pAnimName);
}

bool CLTClient::AddVertexShader(const char *pFileName, int VertexShaderID,
								const uint32 *pVertexElements, uint32 VertexElementsSize, bool bCompileShader)
{
	bool bSuccess = false;

	// Open the vertex shader.
	ILTStream *pStream = NULL;
	if (OpenFile(pFileName, &pStream) == LT_OK)
	{
		bSuccess = LTVertexShaderMgr::GetSingleton().AddVertexShader(pStream, pFileName, VertexShaderID,
																	 (D3DVERTEXELEMENT9*)pVertexElements,
																	 VertexElementsSize, bCompileShader);
	}

	// Close the file.
	if (pStream != NULL)
	{
		pStream->Release();
	}

	return bSuccess;
}

void CLTClient::RemoveVertexShader(int VertexShaderID)
{
	LTVertexShaderMgr::GetSingleton().RemoveVertexShader(VertexShaderID);
}

void CLTClient::RemoveAllVertexShaders()
{
	LTVertexShaderMgr::GetSingleton().RemoveAllVertexShaders();
}

LTVertexShader* CLTClient::GetVertexShader(int VertexShaderID)
{
	return LTVertexShaderMgr::GetSingleton().GetVertexShader(VertexShaderID);
}

bool CLTClient::AddPixelShader(const char *pFileName, int PixelShaderID, bool bCompileShader)
{
	bool bSuccess = false;

	// Open the pixel shader.
	ILTStream *pStream = NULL;
	if (OpenFile(pFileName, &pStream) == LT_OK)
	{
		bSuccess = LTPixelShaderMgr::GetSingleton().AddPixelShader(pStream, pFileName, PixelShaderID, bCompileShader);
	}

	// Close the file.
	if (pStream != NULL)
	{
		pStream->Release();
	}

	return bSuccess;
}

void CLTClient::RemovePixelShader(int PixelShaderID)
{
	LTPixelShaderMgr::GetSingleton().RemovePixelShader(PixelShaderID);
}

void CLTClient::RemoveAllPixelShaders()
{
	LTPixelShaderMgr::GetSingleton().RemoveAllPixelShaders();
}

LTPixelShader* CLTClient::GetPixelShader(int PixelShaderID)
{
	return LTPixelShaderMgr::GetSingleton().GetPixelShader(PixelShaderID);
}

void CLTClient::CPrint(const char *pMsg, ...)
{
	va_list marker;
	va_start(marker, pMsg);

	static const uint32 knBufferSize = 512;
	char str[knBufferSize];
	LTVSNPrintF(str, knBufferSize, pMsg, marker);
	va_end(marker);

	uint32 len = strlen(str);
	if (str[len-1] != '\n')
	{
		str[len] = '\n';
		str[len+1] = '\0';
	}

	con_PrintString(CONRGB(255,255,255), 0, str);
}

uint32 CLTClient::GetPointContainers(const LTVector *pPoint, HLOCALOBJ *pList, uint32 maxListSize)
{
	WorldTree *pWorldTree = world_bsp_client->ClientTree();
	if (!pWorldTree || !pPoint || !pList || maxListSize == 0)
		return 0;

	TempObjArray theArray;
	uint32 nObjects = 0;
	theArray.m_Point = *pPoint;
	theArray.m_pObjects = pList;
	theArray.m_nMaxObjects = maxListSize;
	theArray.m_pNumObjects = &nObjects;

	pWorldTree->FindObjectsOnPoint(pPoint, ci_GetPointContainersCB, &theArray);
	return nObjects;
}


bool CLTClient::GetContainerCode(HLOCALOBJ hObj, uint16 *pCode)
{
	ContainerInstance *pContainer = (ContainerInstance*)hObj;
	if (!pContainer || pContainer->m_ObjectType != OT_CONTAINER || !pCode)
		return false;

	*pCode = pContainer->m_ContainerCode;
	return true;
}


uint32 CLTClient::GetObjectContainers(HOBJECT hObj,
	HOBJECT *pContainerList, uint32 maxListSize)
{
	LTObject *pObj = (LTObject*)hObj;

	if (!pObj)
		return 0;

	LTVector vObjMin = pObj->GetBBoxMin();
	LTVector vObjMax = pObj->GetBBoxMax();

	// Run through the containers looking for intersections
	uint32 nResult = 0;
	LTLink *pListHead = &g_pClientMgr->m_ObjectMgr.m_ObjectLists[OT_CONTAINER].m_Head;
	for (LTLink *pCur = pListHead->m_pNext; pCur != pListHead; pCur = pCur->m_pNext)
	{
		ContainerInstance *pContainer = (ContainerInstance*)pCur->m_pData;
		// Skip it if their extents don't overlap
		LTVector vContainerMin = pContainer->GetBBoxMin();
		LTVector vContainerMax = pContainer->GetBBoxMax();
		if ((vContainerMin.x > vObjMax.x) ||
			(vContainerMin.y > vObjMax.y) ||
			(vContainerMin.z > vObjMax.z) ||
			(vContainerMax.x < vObjMin.x) ||
			(vContainerMax.y < vObjMin.y) ||
			(vContainerMax.z < vObjMin.z))
			continue;

		// Check the BSP
		if (!DoesBoxIntersectBSP(pContainer->GetValidBsp()->GetRootNode(), vObjMin, vObjMax, true))
			continue;

		// Write this object in the array
		if (pContainerList && (nResult < maxListSize))
			pContainerList[nResult] = (HOBJECT)pContainer;
		++nResult;
	}
	return nResult;
}

uint32 CLTClient::GetContainedObjects(HOBJECT hContainer,
	HOBJECT *pObjectList, uint32 maxListSize)
{
	LTObject *pObj = (LTObject*)hContainer;

	if (!pObj)
		return 0;

	// Must be a container
	if (pObj->m_ObjType != OT_CONTAINER)
		return 0;

	ContainerInstance *pContainer = (ContainerInstance*)pObj;

	LTVector vContainerMin = pContainer->GetBBoxMin();
	LTVector vContainerMax = pContainer->GetBBoxMax();

	// Run through the objects looking for intersections
	uint32 nResult = 0;
	for (uint32 nCurObjType = 0; nCurObjType < NUM_OBJECTTYPES; ++nCurObjType)
	{
		// Don't contain containers
		if (nCurObjType == OT_CONTAINER)
			continue;

		LTLink *pListHead = &g_pClientMgr->m_ObjectMgr.m_ObjectLists[nCurObjType].m_Head;
		for (LTLink *pCur = pListHead->m_pNext; pCur != pListHead; pCur = pCur->m_pNext)
		{
			LTObject *pCurObj = (LTObject*)pCur->m_pData;

			// Skip it if their extents don't overlap
			LTVector vObjMin = pObj->GetBBoxMin();
			LTVector vObjMax = pObj->GetBBoxMax();
			if ((vContainerMin.x > vObjMax.x) ||
				(vContainerMin.y > vObjMax.y) ||
				(vContainerMin.z > vObjMax.z) ||
				(vContainerMax.x < vObjMin.x) ||
				(vContainerMax.y < vObjMin.y) ||
				(vContainerMax.z < vObjMin.z))
				continue;

			// Check the BSP
			if (!DoesBoxIntersectBSP(pContainer->GetValidBsp()->GetRootNode(), vObjMin, vObjMax, true))
				continue;

			// Write this object in the array
			if (pObjectList && (nResult < maxListSize))
				pObjectList[nResult] = (HOBJECT)pCurObj;
			++nResult;
		}
	}
	return nResult;
}

HSTRING CLTClient::FormatString(int messageCode, ...)
{
	uint8 *pBuffer;
	int bufferLen;
	va_list marker;
	HSTRING ret;

	/* Check our localized module first */
	if (g_pClientMgr->m_hLocalizedClientResourceModule)
	{
		va_start(marker, messageCode);

		pBuffer = str_FormatString(g_pClientMgr->m_hLocalizedClientResourceModule,
			messageCode, &marker, &bufferLen);

		va_end(marker);

		if (pBuffer)
		{
			ret = str_CreateString(pBuffer);
			str_FreeStringBuffer(pBuffer);
			return ret;
		}
	}

	if (g_pClientMgr->m_hClientResourceModule)
	{
		va_start(marker, messageCode);

		pBuffer = str_FormatString(g_pClientMgr->m_hClientResourceModule,
			messageCode, &marker, &bufferLen);

		va_end(marker);

		if (pBuffer)
		{
			ret = str_CreateString(pBuffer);
			str_FreeStringBuffer(pBuffer);
			return ret;
		}
	}

	return LTNULL;
}

HSTRING CLTClient::CopyString(HSTRING hString)
{
	if (hString)
		return str_CopyString(hString);
	else
		return LTNULL;
}

HSTRING CLTClient::CreateString(const char *pString)
{
	if (pString)
		return str_CreateStringAnsi(pString);
	else
		return LTNULL;
}

void CLTClient::FreeString(HSTRING hString)
{
	ic_FreeString(hString);
}

bool CLTClient::CompareStrings(HSTRING hString1, HSTRING hString2)
{
	if (hString1 && hString2)
		return str_CompareStrings(hString1, hString2);
	else if (hString1)
		return false;
	else
		return true;
}

bool CLTClient::CompareStringsUpper(HSTRING hString1, HSTRING hString2)
{
	if (hString1 && hString2)
		return str_CompareStringsUpper(hString1, hString2);
	else if (hString1)
		return false;
	else
		return true;
}

const char *CLTClient::GetStringData(HSTRING hString)
{
	if (hString)
		return str_GetStringData(hString);
	else
		return "";
}


float CLTClient::GetVarValueFloat(HCONSOLEVAR hVar)
{
	if (hVar)
		return ((LTCommandVar*)hVar)->floatVal;
	else
		return 0.0f;
}


const char *CLTClient::GetVarValueString(HCONSOLEVAR hVar)
{
	if (hVar)
	{
		return ((LTCommandVar*)hVar)->pStringVal;
	}
	else
	{
		return LTNULL;
	}
}

LTFLOAT CLTClient::GetTime()
{
	return g_pClientMgr->m_CurTime;
}

LTFLOAT CLTClient::GetFrameTime()
{
	return g_pClientMgr->m_FrameTime;
}

LTRESULT CLTClient::GetSourceWorldOffset(LTVector& vOffset)
{
	vOffset = world_bsp_shared->GetSourceWorldOffset();
	return LT_OK;
}


LTRESULT CLTClient::RemoveObject(HLOCALOBJ hObj)
{
	if (!hObj)
	{
		RETURN_ERROR(1, CLTClient::RemoveObject, LT_INVALIDPARAMS);
	}

	LTObject *pObject = (LTObject*)hObj;
	if (pObject->m_ObjectID != (uint16)-1)
	{
		RETURN_ERROR(1, CLTClient::RemoveObject, LT_CANTREMOVESERVEROBJECT);
	}

	return g_pClientMgr->RemoveObjectFromClientWorld(pObject);
}

LTRESULT CLTClient::GetBlindObjectData(uint32 nNum, uint32 nId, uint8*& pData, uint32& nSize)
{
	if (!world_blind_object_data->GetBlindObjectData(nNum, nId, pData, nSize))
		return LT_ERROR;

	return LT_OK;
}

LTRESULT CLTClient::FreeBlindObjectData(uint32 nNum, uint32 nId)
{
	if (!world_blind_object_data->FreeBlindObjectData(nNum, nId))
		return LT_ERROR;

	return LT_OK;
}

LTRESULT CLTClient::OpenMemoryStream(ILTStream **pStream, uint32 nCacheSize)
{
	if (!pStream)
	{
		RETURN_ERROR(1, CLTClient::OpenMemoryStream, LT_INVALIDPARAMS);
	}

	// Open the memory stream

	*pStream = streamsim_OpenMemStream(nCacheSize);

	return LT_OK;
}

LTRESULT CLTClient::GetTextureEffectVarID(const char *pName, uint32 nStage, uint32 *pResult) const
{
	char pszFullName[256];
	LTSNPrintF(pszFullName, sizeof(pszFullName), "%s%n", pName, nStage);

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

LTRESULT CLTClient::LinkObjRef(HOBJECT hObj, LTObjRef *pRef)
{
	if (!hObj)
		return LT_ERROR;

	LTObject *pObject = (LTObject*)hObj;

	dl_Insert(&pObject->m_RefList, pRef);

	return LT_OK;
}

LTRESULT CLTClient::GetLightGroupID(const char *pName, uint32 *pResult) const
{
	// Return the hash code
	*pResult = st_GetHashCode(pName);
	return LT_OK;
}

LTRESULT CLTClient::GetLightGroupColor(uint32 nID, LTVector *pColor) const
{
	// Ask the world bsp about the lightgroup color
	if (!world_bsp_client)
		return LT_NOTINWORLD;

	return world_bsp_client->GetLightGroupColor(nID, pColor);
}

LTRESULT CLTClient::SetLightGroupColor(uint32 nID, const LTVector &vColor)
{
	// Make sure our status is acceptable
	if (!world_bsp_client)
		return LT_NOTINWORLD;

	if (!r_GetRenderStruct() || !r_GetRenderStruct()->SetLightGroupColor)
		return LT_NOTINWORLD;

	// Get the old color
	LTVector vOldColor;
	if (world_bsp_client->GetLightGroupColor(nID, &vOldColor) != LT_OK)
		return LT_NOTFOUND;

	// Don't actually do anything if it's not going to change
	if (vColor.NearlyEquals(vOldColor, 0.0001f))
		return LT_OK;

	// Update the color in the current world
	LTRESULT nResult = world_bsp_client->SetLightGroupColor(nID, vColor);
	if (nResult != LT_OK)
		return nResult;

	// Tell the renderer about the new color
	if (!r_GetRenderStruct()->SetLightGroupColor(nID, vColor))
		return LT_ERROR;

	return LT_OK;
}

LTRESULT CLTClient::GetOccluderID(const char *pName, uint32 *pResult) const
{
	// Return the hash code
	*pResult = st_GetHashCode_ic(pName);
	return LT_OK;
}

LTRESULT CLTClient::GetOccluderEnabled(uint32 nID, bool *pResult) const
{
	// Ask the renderer about the occluder state
	if (!r_GetRenderStruct())
		return LT_NOTINWORLD;

	return r_GetRenderStruct()->GetOccluderEnabled(nID, pResult);
}

LTRESULT CLTClient::SetOccluderEnabled(uint32 nID, bool bEnabled)
{
	// Tell the renderer the occluder state
	if (!r_GetRenderStruct())
		return LT_NOTINWORLD;

	return r_GetRenderStruct()->SetOccluderEnabled(nID, bEnabled);
}

LTRESULT CLTClient::SetTextureEffectVar(uint32 nID, uint32 nVar, float fVal)
{
	// Make sure our status is acceptable
	if (!r_GetRenderStruct())
		return LT_NOTINWORLD;

	// Tell the renderer about the new variable
	if (!r_GetRenderStruct()->SetTextureEffectVar(nID, nVar, fVal))
		return LT_ERROR;

	return LT_OK;
}

LTRESULT CLTClient::GetRendererStats(LTRendererStats &refStats)
{

	if (!r_GetRenderStruct())
		return LT_ERROR;

    // Fill our stats struct
    if(GetFrameStats(refStats))
    {
        return LT_ERROR;
    }

    return LT_OK;
}

//Object Render Groups
LTRESULT CLTClient::SetObjectRenderGroup(HOBJECT hObj, uint32 nGroup)
{
	if(!hObj || (nGroup >= MAX_OBJECT_RENDER_GROUPS))
		return LT_INVALIDPARAMS;

	((LTObject*)hObj)->m_nRenderGroup = nGroup;

	return LT_OK;
}

LTRESULT CLTClient::SetObjectRenderGroupEnabled(uint32 nGroup, bool bEnabled)
{
	if (!r_GetRenderStruct())
		return LT_NOTINWORLD;

	if(nGroup >= MAX_OBJECT_RENDER_GROUPS)
		return LT_INVALIDPARAMS;

	r_GetRenderStruct()->SetObjectGroupEnabled(nGroup, bEnabled);

	return LT_OK;
}

LTRESULT CLTClient::SetAllObjectRenderGroupEnabled()
{
	if (!r_GetRenderStruct())
		return LT_NOTINWORLD;

	r_GetRenderStruct()->SetAllObjectGroupEnabled();

	return LT_OK;
}

LTRESULT CLTClient::AddGlowRenderStyleMapping(const char* pszSource, const char* pszMapTo)
{
	if (!r_GetRenderStruct())
		return LT_NOTINWORLD;

	return r_GetRenderStruct()->AddGlowRenderStyleMapping(pszSource, pszMapTo) ? LT_OK : LT_ERROR;
}


LTRESULT CLTClient::SetGlowDefaultRenderStyle(const char* pszFilename)
{
	if (!r_GetRenderStruct())
		return LT_NOTINWORLD;

	return r_GetRenderStruct()->SetGlowDefaultRenderStyle(pszFilename) ? LT_OK : LT_ERROR;
}

LTRESULT CLTClient::SetNoGlowRenderStyle(const char* pszFilename)
{
	if (!r_GetRenderStruct())
		return LT_NOTINWORLD;

	return r_GetRenderStruct()->SetNoGlowRenderStyle(pszFilename) ? LT_OK : LT_ERROR;
}


LTRESULT CLTClient::IsLocalToServer(bool *bResult)
{
	if (!bResult)
		return LT_ERROR;

	if (!g_pClientMgr->m_pCurShell)
		return LT_ERROR;

	*bResult = g_pClientMgr->m_pCurShell->m_bLocal;

	return LT_OK;
}


LTRESULT CLTClient::GetDeviceObjectName( char const* pszDeviceName, uint32 nObjectId,
										char* pszDeviceObjectName, uint32 nDeviceObjectNameLen )
{
	if (!g_pClientMgr->m_InputMgr)
	{
		return LT_ERROR;
	}

	if (!g_pClientMgr->m_InputMgr->GetDeviceObjectName( pszDeviceName, nObjectId,
		pszDeviceObjectName, nDeviceObjectNameLen ))
	{
		return LT_NOTFOUND;
	}

	return LT_OK;
}

// ----------------------------------------------------------------- //
// Creation functions.
// ----------------------------------------------------------------- //

void CreateLTClient()
{
	ilt_client_imp->InitFunctionPointers();
}



// ----------------------------------------------------------------- //
// Internal helpers.
// ----------------------------------------------------------------- //

bool _IsPointInContainer(const LTVector *pPoint, ContainerInstance *pContainer)
{
	float dist = (*pPoint - pContainer->GetPos()).MagSqr();

	WorldBsp *pWorldBsp = (WorldBsp*)pContainer->m_pOriginalBsp;
	if (dist >= pWorldBsp->GetBoundRadiusSqr())
		return false;

	// Transform the point..
	LTVector transformedPoint;
	MatVMul_H(&transformedPoint, &pContainer->m_BackTransform, pPoint);

	if (!ci_IsPointInsideBSP(pWorldBsp->m_RootNode, transformedPoint))
	{
		return true;
	}

	return false;
}


// ----------------------------------------------------------------- //
// Static functions.
// ----------------------------------------------------------------- //


LTRESULT ci_StartGame(StartGameRequest *pRequest)
{
	return g_pClientMgr->StartShell(pRequest);
}


LTRESULT ci_GetGameMode(int *mode)
{
	if (g_pClientMgr->m_pCurShell)
		*mode = g_pClientMgr->m_pCurShell->m_ShellMode;
	else
		*mode = GAMEMODE_NONE;

	return LT_OK;
}


LTRESULT ci_GetLocalClientID(uint32 *pID)
{
	*pID = 0;

	if (!g_pClientMgr->m_pCurShell || g_pClientMgr->m_pCurShell->m_ClientID == (uint16)-1)
	{
		return LT_NOTCONNECTED;
	}

	*pID = g_pClientMgr->m_pCurShell->m_ClientID;
	return LT_OK;
}


bool ci_IsConnected()
{
	return !!g_pClientMgr->m_pCurShell;
}


void ci_Disconnect()
{
	g_pClientMgr->EndShell();
}



void ci_ShutdownWithMessage(const char *pMsg, ...)
{
	va_list marker;
	va_start(marker, pMsg);

	static const uint32 knBufferSize = 512;

	char str[knBufferSize];
	LTVSNPrintF(str, knBufferSize, pMsg, marker);

	va_end(marker);

	dsi_OnClientShutdown(str);
}

void ci_Shutdown()
{
	ci_ShutdownWithMessage("");
}

FileEntry* ci_GetFileList(const char *pDirName)
{
	return client_file_mgr->GetFileList(pDirName);
}


LTRESULT ci_ReadConfigFile(const char *pFilename)
{
	if (cc_RunConfigFile(&g_ClientConsoleState, pFilename, 0, VARFLAG_SAVE))
		return LT_OK;
	else
		return LT_ERROR;
}


LTRESULT ci_WriteConfigFile(const char *pFilename)
{
	if (cc_SaveConfigFile(&g_ClientConsoleState, pFilename))
		return LT_OK;
	else
		return LT_ERROR;
}

LTRESULT ci_WriteConfigFileFields(const char *pFilename, uint32 nNumValues, const char** pValues)
{
	if (cc_SaveConfigFileFields(&g_ClientConsoleState, pFilename, nNumValues, pValues))
		return LT_OK;
	else
		return LT_ERROR;
}


void ci_GetAxisOffsets(LTFLOAT *offsets)
{
	offsets[0] = g_pClientMgr->m_AxisOffsets[0];
	offsets[1] = g_pClientMgr->m_AxisOffsets[1];
	offsets[2] = g_pClientMgr->m_AxisOffsets[2];
}


void ci_PlayJoystickEffect(const char *pEffectName, float x, float y)
{
	g_pClientMgr->m_InputMgr->PlayJoystickEffect(g_pClientMgr->m_InputMgr, pEffectName, x, y);
}

bool ci_InitMusic(const char *szMusicDLL)
{
	return g_pClientMgr->AppInitMusic(szMusicDLL) == LT_OK;
}

bool ci_LoadSong(const char *szSong)
{
	void *pSong;

	if (!GetMusicMgr()->m_bValid)
		return false;

	pSong = GetMusicMgr()->GetSong(szSong);
	if (!pSong)
	{
		pSong = GetMusicMgr()->CreateSong(szSong);
		if (!pSong)
			return false;
	}

	return true;
}

void ci_DestroyAllSongs()
{
	if (!GetMusicMgr()->m_bValid)
		return;

	GetMusicMgr()->DestroyAllSongs();
}

void ci_StopMusic(uint32 dwPlayBoundaryFlags)
{
	if (!GetMusicMgr()->m_bValid)
		return;

	GetMusicMgr()->Stop(dwPlayBoundaryFlags);
}

bool ci_PauseMusic()
{
	if (!GetMusicMgr()->m_bValid)
		return false;

	return GetMusicMgr()->Pause(MUSIC_IMMEDIATE);
}


bool ci_ResumeMusic()
{
	if (!GetMusicMgr()->m_bValid)
		return false;

	return GetMusicMgr()->Resume();
}

bool ci_AddSongToPlayList(const char *szPlayList, const char *szSong)
{
	void *pPlayList;
	void *pSong;

	ASSERT(szPlayList);
	ASSERT(szSong);

	if (!GetMusicMgr()->m_bValid)
		return false;

	if (!szPlayList || !szSong)
		return false;

	pPlayList = GetMusicMgr()->GetPlayList(szPlayList);
	if (!pPlayList)
	{
		pPlayList = GetMusicMgr()->CreatePlayList(szPlayList);
		if (!pPlayList)
			return false;
	}

	pSong = GetMusicMgr()->GetSong(szSong);
	if (!pSong)
	{
		pSong = GetMusicMgr()->CreateSong(szSong);
		if (!pSong)
			return false;
	}

	return GetMusicMgr()->AddSongToList(pPlayList, pSong);
}

void ci_DeletePlayList(const char *szPlayList)
{
	void *pPlayList;

	ASSERT(szPlayList);

	if (!GetMusicMgr()->m_bValid)
		return;

	if (!szPlayList)
		return;

	pPlayList = GetMusicMgr()->GetPlayList(szPlayList);
	if (!pPlayList)
		return;

	GetMusicMgr()->RemoveList(pPlayList);
}

bool ci_PlayList(const char *szPlayList, const char *szTransition, bool bLoop, uint32 dwPlayBoundaryFlags)
{
	void *pPlayList;
	void *pTransition;

	if (!GetMusicMgr()->m_bValid)
		return false;

	if (!szPlayList)
		return false;

	pPlayList = GetMusicMgr()->GetPlayList(szPlayList);
	if (!pPlayList)
		return false;

	pTransition = LTNULL;
	if (szTransition)
	{
		pTransition = GetMusicMgr()->GetSong(szTransition);
		if (!pTransition)
		{
			pTransition = GetMusicMgr()->CreateSong(szTransition);
			if (!pTransition)
				return false;
		}
	}

	return GetMusicMgr()->PlayList(pPlayList, pTransition, bLoop, dwPlayBoundaryFlags);
}

short ci_GetMusicVolume()
{
	if (!GetMusicMgr()->m_bValid)
		return 0;

	if (GetMusicMgr()->m_ulCaps & MUSIC_VOLUMECONTROL)
		return (short)GetMusicMgr()->GetVolume();

	return 0;
}


void ci_SetMusicVolume(short wVolume)
{
	if (!GetMusicMgr()->m_bValid)
		return;

	if (GetMusicMgr()->m_ulCaps & MUSIC_VOLUMECONTROL)
		GetMusicMgr()->SetVolume(wVolume);
}

// Get the sound duration for a client only sound...
LTRESULT ci_GetSoundTimer(HLTSOUND hSound, LTFLOAT *fTimer)
{
	if (!g_pClientMgr)
		return LT_OK;

	if (!GetClientILTSoundMgrImpl()->IsValid())
		return LT_OK;

	if (!hSound || !fTimer)
		RETURN_ERROR(1, GetSoundDuration, LT_INVALIDPARAMS);

	const CSoundInstance * const pSoundInstance = GetClientILTSoundMgrImpl()->FindSoundInstance(hSound, true);
	if (!pSoundInstance)
		RETURN_ERROR(1, GetSoundTimer, LT_INVALIDPARAMS);

	if (pSoundInstance->GetSoundInstanceFlags() & SOUNDINSTANCEFLAG_PLAYING)
	{
		*fTimer = (pSoundInstance->GetDuration() - pSoundInstance->GetTimer()) / 1000.0f;
		return LT_OK;
	}

	return LT_FINISHED;
}

#ifdef USE_ABSTRACT_SOUND_INTERFACES
static LTRESULT GetSoundStructures( HLTSOUND hSound, CSoundInstance*& pSoundInstance,
							   CSoundBuffer*& pSoundBuffer, LTSOUNDINFO*& pSoundInfo )
#else // USE_ABSTRACT_SOUND_INTERFACES
static LTRESULT GetSoundStructures( HLTSOUND hSound, CSoundInstance*& pSoundInstance,
							   CSoundBuffer*& pSoundBuffer, AILSOUNDINFO*& pSoundInfo )
#endif // USE_ABSTRACT_SOUND_INTERFACES
{
	if (!g_pClientMgr)
		return LT_OK;

	if (!GetClientILTSoundMgrImpl()->IsValid())
		return LT_OK;

	if (!hSound)
		RETURN_ERROR(1, GetSoundStructures, LT_INVALIDPARAMS);

	pSoundInstance = GetClientILTSoundMgrImpl()->FindSoundInstance(hSound, true);
	if( !pSoundInstance )
		RETURN_ERROR(1, GetSoundStructures, LT_INVALIDPARAMS);

	pSoundBuffer = pSoundInstance->GetSoundBuffer();
	ASSERT(pSoundBuffer);
	if ( !pSoundBuffer )
		RETURN_ERROR(1, GetSoundStructures, LT_ERROR);

	// Decompress data.
	if (pSoundBuffer->IsCompressed() && !pSoundBuffer->GetDecompressedSoundBuffer())
	{
		if( pSoundBuffer->DecompressData() != LT_OK )
		{
			RETURN_ERROR(1, GetSoundStructures, LT_ERROR);
		}
	}

#ifdef USE_ABSTRACT_SOUND_INTERFACES
	pSoundInfo = const_cast< LTSOUNDINFO* >( pSoundBuffer->GetSampleInfo());
#else
	pSoundInfo = const_cast< AILSOUNDINFO* >( pSoundBuffer->GetSampleInfo());
#endif

	if( !pSoundInfo )
		RETURN_ERROR(1, GetSoundStructures, LT_ERROR);

	return LT_OK;
}

// Get the sound data and current offset into data.
LTRESULT ci_GetSoundData(HLTSOUND hSound,
						int16 * & pSixteenBitData, int8 * & pEightBitData,
						uint32 * dwSamplesPerSecond, uint32 * dwChannels)
{
	CSoundInstance* pSoundInstance = NULL;
	CSoundBuffer* pSoundBuffer = NULL;
#ifdef USE_ABSTRACT_SOUND_INTERFACES
	LTSOUNDINFO* pSoundInfo = NULL;
#else // USE_ABSTRACT_SOUND_INTERFACES
	AILSOUNDINFO* pSoundInfo = NULL;
#endif // USE_ABSTRACT_SOUND_INTERFACES
	LTRESULT ltRes = GetSoundStructures( hSound, pSoundInstance, pSoundBuffer, pSoundInfo );
	if( ltRes != LT_OK )
		return ltRes;

	if (dwSamplesPerSecond)
		*dwSamplesPerSecond = pSoundInfo->rate;
	if (dwChannels)
		*dwChannels = pSoundInfo->channels;

	if (pSoundInfo->bits == 8)
	{
		pEightBitData = reinterpret_cast<int8*>(pSoundBuffer->GetSoundData());
		pSixteenBitData = LTNULL;
	}
	else if (pSoundInfo->bits == 16)
	{
		pEightBitData = LTNULL;
		pSixteenBitData = reinterpret_cast<int16*>(pSoundBuffer->GetSoundData());
	}
	else
	{
		ASSERT(!"GetSoundData: Invalid bit depth.");
		pEightBitData = LTNULL;
		pSixteenBitData = LTNULL;

		RETURN_ERROR(1, GetSoundData, LT_ERROR);
	}

	return LT_OK;
}

LTRESULT ci_GetSoundOffset(HLTSOUND hSound, uint32 *dwOffset, uint32 *dwSize)
{
	CSoundInstance* pSoundInstance = NULL;
	CSoundBuffer* pSoundBuffer = NULL;
#ifdef USE_ABSTRACT_SOUND_INTERFACES
	LTSOUNDINFO* pSoundInfo = NULL;
#else // USE_ABSTRACT_SOUND_INTERFACES
	AILSOUNDINFO* pSoundInfo = NULL;
#endif // USE_ABSTRACT_SOUND_INTERFACES
	LTRESULT ltRes = GetSoundStructures( hSound, pSoundInstance, pSoundBuffer, pSoundInfo );
	if( ltRes != LT_OK )
		return ltRes;

	if ( !( pSoundInstance->GetSoundInstanceFlags() & SOUNDINSTANCEFLAG_PLAYING))
		return LT_FINISHED;

	if (dwOffset)
	{
		*dwOffset  = (pSoundInstance->GetDuration() - pSoundInstance->GetTimer())
					 * pSoundInfo->rate
					 * pSoundInfo->channels;
		*dwOffset /= 1000;  // timer is in milliseconds, rate is in seconds.
	}

	if (dwSize)
	{
		if (pSoundInfo->bits)
			*dwSize = pSoundBuffer->GetSoundDataLen() / pSoundInfo->bits * 8;
		else
		{
			return LT_ERROR;
		}
	}

	return LT_OK;
}

// Check if a client only sound is done playing...
bool ci_IsDone(HLTSOUND hSound)
{
	bool bResult;
	GetClientILTSoundMgrImpl()->IsSoundDone(hSound, bResult);
	return bResult;
}

void ci_GetListener(bool *bListenerInClient, LTVector *pPos, LTRotation *pRot)
{
	// Get the "in client" state
	if (bListenerInClient)
		*bListenerInClient = GetClientILTSoundMgrImpl()->IsListenerInClient();
	// Get the position
	if (pPos)
		*pPos = GetClientILTSoundMgrImpl()->GetListenerPosition();
	// Calculate an LTRotation..
	if (pRot)
	{
		LTMatrix mListener;
		LTVector vForward, vRight, vUp;
		vForward = GetClientILTSoundMgrImpl()->GetListenerFront();
		vRight = GetClientILTSoundMgrImpl()->GetListenerRight();
		vUp = vRight.Cross(vForward);
		mListener.SetBasisVectors(&vRight, &vUp, &vForward);
		pRot->ConvertFromMatrix(mListener);
	}
}


bool ci_CastRay(ClientIntersectQuery *pQuery, ClientIntersectInfo *pInfo)
{
	float mag = pQuery->m_Direction.Mag();
	if (mag < 0.00001f)
		return false;

	// Scale it to 10000.
	float scale = 10000.0f / mag;
	pQuery->m_To = pQuery->m_Direction * scale;
	pQuery->m_To = pQuery->m_To + pQuery->m_From;

	return world_bsp_client->IntersectSegment(pQuery, pInfo);
}


LTRESULT ci_RegisterConsoleProgram(const char *pName, ConsoleProgramFn fn)
{
	if (cc_FindCommand(&g_ClientConsoleState, pName))
		RETURN_ERROR(1, CLTClient::RegisterConsoleProgram, LT_ALREADYEXISTS);

	cc_AddCommand(&g_ClientConsoleState, pName, fn, CMD_USERCOMMAND);
	return LT_OK;
}


LTRESULT ci_UnregisterConsoleProgram(const char *pName)
{
	uint32 nRemoved = 0;
	LTExtraCommandStruct *pCommand;
	while ((pCommand = cc_FindCommand(&g_ClientConsoleState, pName)) != LTNULL)
	{
		++nRemoved;
		cc_RemoveCommand(&g_ClientConsoleState, pCommand);
	}

	if (nRemoved == 0)
	{
		RETURN_ERROR(1, CLTClient::UnregisterConsoleProgram, LT_NOTFOUND);
	}

	return LT_OK;
}


HCONSOLEVAR ci_GetConsoleVar(const char *pName)
{
	return (HCONSOLEVAR)cc_FindConsoleVar(&g_ClientConsoleState, pName);
}

float ci_GetVarValueFloat(HCONSOLEVAR hVar)
{
	if (hVar)
		return ((LTCommandVar*)hVar)->floatVal;
	else
		return 0.0f;
}


const char * ci_GetVarValueString(HCONSOLEVAR hVar)
{
	if (hVar)
	{
		return ((LTCommandVar*)hVar)->pStringVal;
	}
	else
	{
		return LTNULL;
	}
}


void ci_SetInputState(bool bOn)
{
	g_pClientMgr->m_bInputState = bOn != 0;
}


LTRESULT ci_ClearInput()
{
	return g_pClientMgr->ClearInput();
}


DeviceBinding* ci_GetDeviceBindings(uint32 nDevice)
{
	if (!g_pClientMgr->m_InputMgr)
		return LTNULL;

	return g_pClientMgr->m_InputMgr->GetDeviceBindings(nDevice);
}

void ci_FreeDeviceBindings(DeviceBinding* pBindings)
{
	if (!g_pClientMgr->m_InputMgr)
		return;

	g_pClientMgr->m_InputMgr->FreeDeviceBindings(pBindings);
}

// Clear bindings for a device.
bool ci_ClearBinding(const char *pDeviceName, const char *pTriggerName)
{
	if (!g_pClientMgr->m_InputMgr)
		return false;

	return g_pClientMgr->m_InputMgr->ClearBindings(g_pClientMgr->m_InputMgr,pDeviceName,pTriggerName);
}

	// Add a binding for a device (set ranges to 0 to not use ranges).
bool ci_AddBinding(const char *pDeviceName, const char *pTriggerName, const char *pActionName,
					float rangeLow, float rangeHigh)
{
	if (!g_pClientMgr->m_InputMgr)
		return false;
	return g_pClientMgr->m_InputMgr->AddBinding(g_pClientMgr->m_InputMgr,pDeviceName,pTriggerName,
												pActionName,rangeLow,rangeHigh);

}



LTRESULT ci_StartDeviceTrack(uint32 nDevices, uint32 nBufferSize)
{
	if (!g_pClientMgr->m_InputMgr)
		return LT_ERROR;

	g_pClientMgr->m_bTrackingInputDevices = true;

	if (!g_pClientMgr->m_InputMgr->StartDeviceTrack(g_pClientMgr->m_InputMgr, nDevices, nBufferSize))
	{
		return LT_ERROR;
	}

	return LT_OK;
}

LTRESULT ci_TrackDevice(DeviceInput *pInputArray, uint32 *pnInOut)
{
	if (!g_pClientMgr->m_InputMgr)
	{
		return LT_ERROR;
	}

	if (!g_pClientMgr->m_bTrackingInputDevices)
	{
		return LT_ERROR;
	}

	if (!g_pClientMgr->m_InputMgr->TrackDevice(pInputArray, pnInOut))
	{
		return LT_ERROR;
	}

	return LT_OK;
}

LTRESULT ci_EndDeviceTrack()
{
	if (!g_pClientMgr->m_InputMgr)
	{
		return LT_ERROR;
	}

	g_pClientMgr->m_bTrackingInputDevices = false;

	if (!g_pClientMgr->m_InputMgr->EndDeviceTrack())
	{
		return LT_ERROR;
	}

	return LT_OK;
}

DeviceObject* ci_GetDeviceObjects(uint32 nDeviceFlags)
{
	if (!g_pClientMgr->m_InputMgr)
		return LTNULL;
	return g_pClientMgr->m_InputMgr->GetDeviceObjects(nDeviceFlags);
}

void ci_FreeDeviceObjects(DeviceObject *pList)
{
	if (!g_pClientMgr->m_InputMgr)
		return;
	g_pClientMgr->m_InputMgr->FreeDeviceObjects(pList);
}

LTRESULT ci_GetDeviceName(uint32 nDeviceType, char *strBuffer, uint32 nBufferSize)
{
	if (!g_pClientMgr->m_InputMgr)
	{
		return LT_ERROR;
	}

	if (!g_pClientMgr->m_InputMgr->GetDeviceName (nDeviceType, strBuffer, nBufferSize))
	{
		return LT_NOTFOUND;
	}

	return LT_OK;
}

LTRESULT ci_IsDeviceEnabled(const char *strDeviceName, bool *pIsEnabled)
{
	if (!g_pClientMgr->m_InputMgr)
		return LT_ERROR;

	*pIsEnabled = g_pClientMgr->m_InputMgr->IsDeviceEnabled (strDeviceName);

	return LT_OK;
}

LTRESULT ci_EnableDevice(const char *strDeviceName)
{
	if (!g_pClientMgr->m_InputMgr)
	{
		return LT_ERROR;
	}

	if (!g_pClientMgr->m_InputMgr->EnableDevice (g_pClientMgr->m_InputMgr, strDeviceName))
	{
		return LT_ERROR;
	}

	return LT_OK;
}

uint32 ci_GetNumRawDevices()
{
	return 0;
}

LTRESULT ci_GetRawDeviceData(uint32 deviceNum, void* pData)
{
	return LT_ERROR;
}

uint32 ci_GetRawDeviceDataSize(uint32 deviceNum)
{
	return 0;
}

uint32 ci_GetRawDeviceActuatorDataSize(uint32 deviceNum)
{
	return 0;
}

LTRESULT ci_SetRawDeviceActuatorData(uint32 deviceNum, void *pData)
{
	return LT_ERROR;
}

float ci_GetGameTime()
{
	if (g_pClientMgr->m_pCurShell)
	{
		return g_pClientMgr->m_pCurShell->m_GameTime;
	}
	else
	{
		return 0.0f;
	}
}

float ci_GetGameFrameTime()
{
	if (g_pClientMgr->m_pCurShell)
	{
		return g_pClientMgr->m_pCurShell->m_GameFrameTime;
	}
	else
	{
		return 0.0f;
	}
}

LTRESULT ci_GetSkyDef(SkyDef *pDef)
{
	memcpy(pDef, &g_pClientMgr->m_SkyDef, sizeof(SkyDef));
	return LT_OK;
}

bool ci_IsCommandOn(int command)
{
	return g_pClientMgr->IsCommandOn((uint32)command);
}


void ci_RunConsoleString(const char *pString)
{
	c_CommandHandler(pString);
}


HLOCALOBJ ci_GetClientObject()
{
	if (g_pClientMgr->m_pCurShell)
	{
		return (HLOCALOBJ)g_pClientMgr->m_pCurShell->GetClientObject();
	}
	else
	{
		return LTNULL;
	}
}

void ci_GetGlobalLightScale(LTVector *pScale)
{
	*pScale = g_pClientMgr->m_GlobalLightScale;
}

void ci_SetGlobalLightScale(const LTVector *pScale)
{
	g_pClientMgr->m_GlobalLightScale = *pScale;
	g_pClientMgr->m_GlobalLightScale.x = LTCLAMP(g_pClientMgr->m_GlobalLightScale.x, 0.0f, 2.0f);
	g_pClientMgr->m_GlobalLightScale.y = LTCLAMP(g_pClientMgr->m_GlobalLightScale.y, 0.0f, 2.0f);
	g_pClientMgr->m_GlobalLightScale.z = LTCLAMP(g_pClientMgr->m_GlobalLightScale.z, 0.0f, 2.0f);
}

void ci_OffsetGlobalLightScale(const LTVector *pOffset)
{
	g_pClientMgr->m_GlobalLightScale = g_pClientMgr->m_GlobalLightScale + *pOffset;
	g_pClientMgr->m_GlobalLightScale.x = LTCLAMP(g_pClientMgr->m_GlobalLightScale.x, 0.0f, 2.0f);
	g_pClientMgr->m_GlobalLightScale.y = LTCLAMP(g_pClientMgr->m_GlobalLightScale.y, 0.0f, 2.0f);
	g_pClientMgr->m_GlobalLightScale.z = LTCLAMP(g_pClientMgr->m_GlobalLightScale.z, 0.0f, 2.0f);
}

HLOCALOBJ ci_CreateObject(ObjectCreateStruct *pStruct)
{
	InternalObjectSetup setup;
	setup.m_pSetup = pStruct;

	setup.m_Filename[0].m_FileType = FILE_CLIENTFILE;
	setup.m_Filename[0].m_pFilename = pStruct->m_Filename;

	for (uint32 nCurTexture = 0; nCurTexture < MAX_MODEL_TEXTURES; nCurTexture++)
	{
		setup.m_SkinNames[nCurTexture].m_pFilename = pStruct->m_SkinNames[nCurTexture];
		setup.m_SkinNames[nCurTexture].m_FileType = FILE_CLIENTFILE;
	}
	for (uint32 nCurRS = 0; nCurRS < MAX_MODEL_RENDERSTYLES; nCurRS++)
	{
		setup.m_RenderStyleNames[nCurRS].m_pFilename = pStruct->m_RenderStyleNames[nCurRS];
		setup.m_RenderStyleNames[nCurRS].m_FileType = FILE_CLIENTFILE;
	}

	LTObject *pObject;
	if (g_pClientMgr->AddObjectToClientWorld(OBJID_CLIENTCREATED, &setup, &pObject, true, true) != LT_OK)
		return LTNULL;

	return (HLOCALOBJ)pObject;
}


void ci_GetObjectColor(HLOCALOBJ hObject, float *r, float *g, float *b, float *a)
{
	if (!hObject)
		return;

	LTObject *pObject = (LTObject*)hObject;

	if (r)
		*r = (float)pObject->m_ColorR * MATH_ONE_OVER_255;
	if (g)
		*g = (float)pObject->m_ColorG * MATH_ONE_OVER_255;
	if (b)
		*b = (float)pObject->m_ColorB * MATH_ONE_OVER_255;
	if (a)
		*a = (float)pObject->m_ColorA * MATH_ONE_OVER_255;
}


void ci_SetObjectColor(HLOCALOBJ hObject, float r, float g, float b, float a)
{
	if (!hObject)
		return;

	LTObject *pObject = (LTObject*)hObject;
	pObject->m_ColorR = (unsigned char)(r * 255.0f);
	pObject->m_ColorG = (unsigned char)(g * 255.0f);
	pObject->m_ColorB = (unsigned char)(b * 255.0f);
	pObject->m_ColorA = (unsigned char)(a * 255.0f);
}


void* ci_GetObjectUserData(HLOCALOBJ hObj)
{
	if (hObj)
		return ((LTObject*)hObj)->m_pUserData;
	else
		return LTNULL;
}


void ci_SetObjectUserData(HLOCALOBJ hObj, void *pData)
{
	if (hObj)
		((LTObject*)hObj)->m_pUserData = pData;
}


LTRESULT ci_Get3DCameraPt(HLOCALOBJ hCamera, int sx, int sy, LTVector *pOut)
{
	CameraInstance *pObject = (CameraInstance*)hCamera;
	if (!pObject || pObject->m_ObjectType != OT_CAMERA || !pOut)
		RETURN_ERROR(1, Get3DCameraPt, LT_INVALIDPARAMS);

	int left, top, right, bottom;
	if (pObject->m_bFullScreen)
	{
		left = top = 0;
		right = r_GetRenderStruct()->m_Width;
		bottom = r_GetRenderStruct()->m_Height;
	}
	else
	{
		left = pObject->m_Left;
		top = pObject->m_Top;
		right = pObject->m_Right;
		bottom = pObject->m_Bottom;
	}

	if (sx < left || sx >= right || sy < top || sy >= bottom)
		RETURN_ERROR(1, Get3DCameraPt, LT_OUTSIDE);

	// Figure out the scaling value to scale the FOV to 90 degrees.
	float xAngle = pObject->m_xFov * 0.5f;
	float yAngle = pObject->m_yFov * 0.5f;

	float sinX = ltsinf(xAngle);
	float cosX = ltcosf(xAngle);
	float sinY = ltsinf(yAngle);
	float cosY = ltcosf(yAngle);

	// Find what x coordinate each angle intercepts the y=1 plane at.
	// A 45 degree angle intercepts at x=1.
	float xCoord = 1.0f / lttanf(MATH_HALFPI - xAngle);
	float yCoord = 1.0f / lttanf(MATH_HALFPI - yAngle);
	float xScale = 1.0f / xCoord; // Scale to 45 degree angle to make it x=z.
	float yScale = 1.0f / yCoord; // Scale to 45 degree angle to make it y=z.

	float halfWidth = (float)((right - left) >> 1);
	float halfHeight = (float)((bottom - top) >> 1);
	float centerX = (float)left + halfWidth;
	float centerY = (float)top + halfHeight;

	float wx = (((float)sx - centerX) / halfWidth) / xScale;
	float wy = -((((float)sy - centerY) / halfHeight) / yScale);

	LTVector vRight, vUp, vForward;
	quat_GetVectors((float*)&pObject->m_Rotation, (float*)&vRight, (float*)&vUp, (float*)&vForward);
	*pOut = vRight * wx;
	*pOut += vUp * wy;
	*pOut += vForward;
	*pOut += pObject->GetPos();

	return LT_OK;
}

void ci_GetCameraFOV(HLOCALOBJ hObj, float *pX, float *pY)
{
	CameraInstance *pCamera = (CameraInstance*)hObj;

	ASSERT(pCamera->m_ObjectType == OT_CAMERA);
	if (!pCamera || pCamera->m_ObjectType != OT_CAMERA || !pX || !pY)
		return;

	*pX = pCamera->m_xFov;
	*pY = pCamera->m_yFov;
}


void ci_SetCameraFOV(HLOCALOBJ hObj, float fovX, float fovY)
{
	CameraInstance *pCamera = (CameraInstance*)hObj;

	ASSERT(pCamera->m_ObjectType == OT_CAMERA);
	if (!pCamera || pCamera->m_ObjectType != OT_CAMERA)
		return;

	const float k_fPIOver100 = MATH_PI / 100.0f;

	fovX = LTCLAMP(fovX, k_fPIOver100, (199.0f*k_fPIOver100));
	fovY = LTCLAMP(fovY, k_fPIOver100, (199.0f*k_fPIOver100));

	pCamera->m_xFov = fovX;
	pCamera->m_yFov = fovY;
}


void ci_GetCameraRect(HLOCALOBJ hObj, bool *bFullscreen,
	int *left, int *top, int *right, int *bottom)
{
	CameraInstance *pCamera = (CameraInstance*)hObj;
	if (!pCamera || pCamera->m_ObjectType != OT_CAMERA || !bFullscreen || !left || !top || !right || !bottom)
		return;

	*bFullscreen = (pCamera->m_bFullScreen != 0);
	*left = pCamera->m_Left;
	*top = pCamera->m_Top;
	*right = pCamera->m_Right;
	*bottom = pCamera->m_Bottom;
}

void ci_SetCameraRect(HLOCALOBJ hObj, bool bFullScreen,
	int left, int top, int right, int bottom)
{
	CameraInstance *pCamera = (CameraInstance*)hObj;
	if (!pCamera || pCamera->m_ObjectType != OT_CAMERA)
		return;

	pCamera->m_bFullScreen = bFullScreen;
	pCamera->m_Left = left;
	pCamera->m_Top = top;
	pCamera->m_Right = right;
	pCamera->m_Bottom = bottom;
}

bool ci_GetCameraLightAdd(HLOCALOBJ hCamera, LTVector *pAdd)
{
	CameraInstance *pCamera = (CameraInstance*)hCamera;
	if (!pCamera || pCamera->m_ObjectType != OT_CAMERA)
		return false;

	*pAdd = pCamera->m_LightAdd;
	return true;
}

bool ci_SetCameraLightAdd(HLOCALOBJ hCamera, const LTVector *pAdd)
{
	CameraInstance *pCamera = (CameraInstance*)hCamera;
	if (!pCamera || pCamera->m_ObjectType != OT_CAMERA)
		return false;

	pCamera->m_LightAdd = *pAdd;
	return true;
}

LTRESULT ci_SetupParticleSystem(HLOCALOBJ hObj, const char *pTextureName, float gravityAccel, uint32 flags, float particleRadius)
{
	LTObject *pObject = (LTObject*)hObj;

	if (!pObject || pObject->m_ObjectType != OT_PARTICLESYSTEM || !pTextureName)
		RETURN_ERROR(1, SetupParticleSystem, LT_INVALIDPARAMS);

	LTParticleSystem *pSystem = (LTParticleSystem*)pObject;
	pSystem->m_GravityAccel = gravityAccel;
	pSystem->m_psFlags = flags;
	pSystem->m_ParticleRadius = particleRadius;

	if (ps_SetTexture(pSystem, pTextureName) != LT_OK)
	{
		RETURN_ERROR(1, SetupParticleSystem, LT_NOTFOUND);
	}

	return LT_OK;
}

void ci_AddParticles(HLOCALOBJ hObj, uint32 nParticles,
	const LTVector *pMinOffset, const LTVector *pMaxOffset,
	const LTVector *pMinVelocity, const LTVector *pMaxVelocity,
	const LTVector *pMinColor, const LTVector *pMaxColor,
	float minLifetime, float maxLifetime)
{
	LTParticleSystem *pSystem = (LTParticleSystem*)hObj;

	if (pSystem && pSystem->m_ObjectType == OT_PARTICLESYSTEM)
	{
		ps_AddParticles(pSystem, nParticles, pMinOffset, pMaxOffset, pMinVelocity, pMaxVelocity,
			pMinColor, pMaxColor, minLifetime, maxLifetime);
	}
}



bool ci_GetParticles(HLOCALOBJ hObj, LTParticle **pHead, LTParticle **pTail)
{
	LTParticleSystem *pSystem = (LTParticleSystem*)hObj;

	if (pSystem && pSystem->m_ObjectType != OT_PARTICLESYSTEM)
	{
		return false;
	}

	*pHead = (LTParticle*)pSystem->m_ParticleHead.m_pNext;
	*pTail = (LTParticle*)&pSystem->m_ParticleHead;
	return true;
}

void ci_RemoveParticle(HLOCALOBJ hSystem, LTParticle *pParticle)
{
	if (!hSystem || !pParticle)
		return;

	LTParticleSystem *pSystem = (LTParticleSystem*)hSystem;
	if (pSystem->m_ObjectType != OT_PARTICLESYSTEM)
		return;

	ps_RemoveParticle(pSystem, (PSParticle*)pParticle);
}


LTRESULT ci_OptimizeParticles(HLOCALOBJ hSystem)
{
	LTParticleSystem *pSystem = (LTParticleSystem*)hSystem;
	if (!pSystem || pSystem->m_ObjectType != OT_PARTICLESYSTEM)
	{
		RETURN_ERROR(1, CLTClient::OptimizeParticles, LT_INVALIDPARAMS);
	}

	ps_OptimizeParticles(pSystem);

	g_pClientMgr->MoveObject(pSystem, &pSystem->GetPos(), true);

	return LT_OK;
}

LTRESULT ci_SortParticles(HLOCALOBJ hSystem, const LTVector& vDir, uint32 nNumIters)
{
	LTParticleSystem *pSystem = (LTParticleSystem*)hSystem;
	if (!pSystem || pSystem->m_ObjectType != OT_PARTICLESYSTEM)
	{
		RETURN_ERROR(1, CLTClient::OptimizeParticles, LT_INVALIDPARAMS);
	}

	ps_SortParticles(pSystem, vDir, nNumIters);

	return LT_OK;
}

LTRESULT ci_SetParticleSystemEffectShaderID(HLOCALOBJ hObj, uint32 EffectShaderID)
{
	LTParticleSystem *pSystem = (LTParticleSystem*)hObj;
	if (!pSystem || pSystem->m_ObjectType != OT_PARTICLESYSTEM)
	{
		RETURN_ERROR(1, CLTClient::SetParticleSystemEffectShaderID, LT_INVALIDPARAMS);
	}

	pSystem->m_nEffectShaderID = EffectShaderID;

	return LT_OK;
}


bool ci_SetupVolumeEffect(HLOCALOBJ hObj, VolumeEffectInfo& info)
{
	LTVolumeEffect* pVE = (LTVolumeEffect*)hObj;
	if( !pVE || pVE->m_ObjectType != OT_VOLUMEEFFECT )
		return false;

	return ve_Init( pVE, &info );
}


LTRESULT ci_SetVolumeEffectEffectShaderID(HLOCALOBJ hObj, uint32 EffectShaderID)
{
	LTVolumeEffect* pVE = (LTVolumeEffect*)hObj;
	if( !pVE || pVE->m_ObjectType != OT_VOLUMEEFFECT )
	{
			RETURN_ERROR(1, CLTClient::SetVolumeEffectEffectShaderID, LT_INVALIDPARAMS);
	}

	pVE->m_nEffectShaderID = EffectShaderID;

	return LT_OK;
}

bool ci_GetParticleBlockersInAABB(const LTVector& pos, const LTVector& dims, std::vector<uint32>& indices)
{
	return world_particle_blocker_data->GetBlockersInAABB( pos, dims, indices );
}


bool ci_GetParticleBlockerEdgesXZ(uint32 index, LTPlane& blockerPlane, uint32& numEdges, LTPlane*& edgePlanes)
{
	return world_particle_blocker_data->GetBlockerEdgesXZ( index, blockerPlane, numEdges, edgePlanes );
}


bool ci_SetupPolyGrid(HLOCALOBJ hObj, uint32 width, uint32 height, uint32 nPGFlags, bool* pValidVerts)
{
	LTPolyGrid *pGrid = (LTPolyGrid*)hObj;
	if (!pGrid || pGrid->m_ObjectType != OT_POLYGRID)
		return false;

	return pg_Init(pGrid, width, height, nPGFlags, pValidVerts);
}


LTRESULT ci_SetPolyGridTexture(HLOCALOBJ hObj, const char *pName)
{
	LTPolyGrid *pGrid = (LTPolyGrid*)hObj;
	if (!pGrid || pGrid->m_ObjectType != OT_POLYGRID)
	{
		RETURN_ERROR_PARAM(1, CLTClient::SetPolyGridTexture, LT_ERROR, "invalid PolyGrid");
	}

	FileRef ref;
	ref.m_FileType = FILE_CLIENTFILE;
	ref.m_pFilename = pName;

	LTRESULT dResult = LoadSprite(&ref, &pGrid->m_pSprite);
	if (dResult != LT_OK)
		return dResult;

	spr_InitTracker(&pGrid->m_SpriteTracker, pGrid->m_pSprite);
	return LT_OK;
}


LTRESULT ci_SetPolyGridEnvMap(HLOCALOBJ hObj, const char *pFilename)
{
	LTPolyGrid *pGrid = (LTPolyGrid*)hObj;
	if (!pGrid || pGrid->m_ObjectType != OT_POLYGRID)
		RETURN_ERROR(0, SetPolyGridEnvMap, LT_INVALIDPARAMS);

	if (pFilename)
	{
		FileRef ref;
		ref.m_FileType = FILE_CLIENTFILE;
		ref.m_pFilename = pFilename;
		pGrid->m_pEnvMap = g_pClientMgr->AddSharedTexture(&ref);
		if (pGrid->m_pEnvMap)
		{
			return LT_OK;
		}
		else
		{
			RETURN_ERROR(1, SetPolyGridEnvMap, LT_NOTFOUND);
		}
	}
	else
	{
		pGrid->m_pEnvMap = LTNULL;
		return LT_OK;
	}
}


LTRESULT ci_GetPolyGridTextureInfo(HLOCALOBJ hObj, float *xPan, float *yPan, float *xScale, float *yScale, float *m_fBaseReflection, float *fVolumeIOR)
{
	LTPolyGrid *pGrid = (LTPolyGrid*)hObj;
	if (!pGrid || pGrid->m_ObjectType != OT_POLYGRID)
	{
		RETURN_ERROR_PARAM(1, CLTClient::GetPolyGridTextureInfo, LT_ERROR, "invalid PolyGrid");
	}

	if (!xPan || !yPan || !xScale || !yScale)
		RETURN_ERROR(0, GetPolyGridTextureInfo, LT_INVALIDPARAMS);

	*xPan = pGrid->m_xPan;
	*yPan = pGrid->m_yPan;
	*xScale = pGrid->m_xScale;
	*yScale = pGrid->m_yScale;
	if (m_fBaseReflection)
		*m_fBaseReflection = pGrid->m_fBaseReflection;
	if (fVolumeIOR)
		*fVolumeIOR = pGrid->m_fFresnelVolumeIOR;

	return LT_OK;
}


LTRESULT ci_SetPolyGridTextureInfo(HLOCALOBJ hObj, float xPan, float yPan, float xScale, float yScale, float fBaseReflection, float fVolumeIOR)
{
	LTPolyGrid *pGrid = (LTPolyGrid*)hObj;
	if (!pGrid || pGrid->m_ObjectType != OT_POLYGRID)
	{
		RETURN_ERROR_PARAM(1, CLTClient::SetPolyGridTextureInfo, LT_ERROR, "invalid PolyGrid");
	}

	pGrid->m_xPan = xPan;
	pGrid->m_yPan = yPan;
	pGrid->m_xScale = xScale;
	pGrid->m_yScale = yScale;
	pGrid->m_fBaseReflection = fBaseReflection;
	pGrid->m_fFresnelVolumeIOR = fVolumeIOR;

	return LT_OK;
}


LTRESULT ci_GetPolyGridInfo(HLOCALOBJ hObj, char **pBytes, uint32 *pWidth, uint32 *pHeight, PGColor **pColorTable)
{
	LTPolyGrid *pGrid = (LTPolyGrid*)hObj;
	if (!pGrid || pGrid->m_ObjectType != OT_POLYGRID || !pBytes || !pWidth || !pHeight || !pColorTable)
		RETURN_ERROR(0, GetPolyGridInfo, LT_INVALIDPARAMS);

	*pBytes = pGrid->m_Data;
	*pWidth = pGrid->m_Width;
	*pHeight = pGrid->m_Height;
	*pColorTable = pGrid->m_ColorTable;
	return LT_OK;
}

LTRESULT ci_SetPolyGridEffectShaderID(HLOCALOBJ hObj, uint32 EffectShaderID)
{
	LTPolyGrid *pGrid = (LTPolyGrid*)hObj;
	if (!pGrid || pGrid->m_ObjectType != OT_POLYGRID)
	{
		RETURN_ERROR_PARAM(1, CLTClient::SetPolygridEffectShaderID, LT_INVALIDPARAMS, "invalid PolyGrid");
	}

	pGrid->m_nEffectShaderID = EffectShaderID;

	return LT_OK;
}


void ci_GetLightColor(HLOCALOBJ hObj, float *r, float *g, float *b)
{
	ci_GetObjectColor(hObj, r, g, b, LTNULL);
}

void ci_SetLightColor(HLOCALOBJ hObject, float r, float g, float b)
{
	if (!hObject)
		return;

	LTObject *pObject = (LTObject*)hObject;
	ci_SetObjectColor(hObject, r, g, b, (float)pObject->m_ColorA * MATH_ONE_OVER_255);
}

float ci_GetLightRadius(HLOCALOBJ hObj)
{
	DynamicLight *pObject = (DynamicLight*)hObj;
	if (pObject->m_ObjectType == OT_LIGHT)
	{
		return pObject->m_LightRadius;
	}
	else
	{
		return 1.0f;
	}
}

void ci_SetLightRadius(HLOCALOBJ hObj, float radius)
{
	DynamicLight *pObject = (DynamicLight*)hObj;
	if (pObject->m_ObjectType != OT_LIGHT)
		return;

	if (radius > pObject->m_LightRadius)
	{
		pObject->m_LightRadius = radius;
		g_pClientMgr->RelocateObject(pObject);
	}
	else
	{
		pObject->m_LightRadius = radius;
	}
}

void ci_GetPointContainersCB(WorldTreeObj *pObj, void *pUser)
{
	if (pObj->GetObjType() != WTObj_DObject)
		return;

	LTObject *pObject = (LTObject*)pObj;
	if (pObject->m_ObjectType != OT_CONTAINER)
		return;

	TempObjArray *pArray = (TempObjArray*)pUser;
	if (*pArray->m_pNumObjects >= pArray->m_nMaxObjects)
		return;

	if (_IsPointInContainer(&pArray->m_Point, pObject->ToContainer()))
	{
		pArray->m_pObjects[*pArray->m_pNumObjects] = (LTObject*)pObj;
		++(*pArray->m_pNumObjects);
	}
}

LTRESULT ci_ClipSprite(HLOCALOBJ hObj, HPOLY hPoly)
{
	if (!hObj)
		return LT_ERROR;

	SpriteInstance *pSprite = (SpriteInstance*)hObj;
	if (pSprite->m_ObjectType != OT_SPRITE)
		return LT_ERROR;

	pSprite->m_ClipperPoly = hPoly;
	return LT_OK;
}

LTRESULT ci_GetPolyTextureFlags(HPOLY hPoly, uint32 *pFlags)
{
	if ((hPoly == INVALID_HPOLY) || !pFlags)
		RETURN_ERROR(1, CLTClient::GetPolyTextureFlags, LT_INVALIDPARAMS);

	*pFlags = 0;

	if (!g_pClientMgr->m_pCurShell)
		RETURN_ERROR(2, CLTClient::GetPolyTextureFlags, LT_ERROR);

	WorldPoly *pPoly = world_bsp_client->GetPolyFromHPoly(hPoly);
	if (!pPoly)
		RETURN_ERROR(2, CLTClient::GetPolyTextureFlags, LT_ERROR);

	*pFlags = pPoly->GetSurface()->m_TextureFlags;
	return LT_OK;
}

LTRESULT ci_SetModelHook(ModelHookFn fn, void *pUser)
{
	g_pClientMgr->m_ModelHookFn = fn;
	g_pClientMgr->m_ModelHookUser = pUser;
	return LT_OK;
}

LTRESULT ci_InitNetworking(const char *sDriver, uint32 dwFlags)
{
	// Get rid of any running shells..
	if (g_pClientMgr->m_pCurShell)
	{
		delete g_pClientMgr->m_pCurShell;
		g_pClientMgr->m_pCurShell = LTNULL;
	}

	// Make sure no drivers are loaded.
	g_pClientMgr->m_NetMgr.InitDrivers();
	return(LT_OK);
}


LTRESULT ci_GetServiceList(NetService* &pListHead)
{
	return g_pClientMgr->m_NetMgr.GetServiceList(pListHead);
}


LTRESULT ci_FreeServiceList(NetService *pListHead)
{
	CHECK_PARAMS(pListHead, CLTClient::GetServiceList);
	return g_pClientMgr->m_NetMgr.FreeServiceList(pListHead);
}


LTRESULT ci_SelectService(HNETSERVICE hNetService)
{
	CHECK_PARAMS(hNetService, CLTClient::SelectService);
	return g_pClientMgr->m_NetMgr.SelectService(hNetService);
}


LTRESULT ci_GetSessionList(NetSession *&pListHead, const char *pInfo)
{
	return g_pClientMgr->m_NetMgr.GetSessionList(pListHead, pInfo);
}


LTRESULT ci_FreeSessionList(NetSession *pListHead)
{
	return g_pClientMgr->m_NetMgr.FreeSessionList(pListHead);
}

LTRESULT ci_AddInternetDriver()
{
	// Add the internet driver
	if (g_pClientMgr->m_NetMgr.GetDriver("internet"))
		return LT_ALREADYEXISTS;

	if (g_pClientMgr->m_NetMgr.AddDriver("internet"))
		return LT_OK;

	return LT_ERROR;
}

LTRESULT ci_RemoveInternetDriver()
{
	// Remove the internet driver
	CBaseDriver* pDriver = g_pClientMgr->m_NetMgr.GetDriver("internet");
	if (pDriver)
	{
		g_pClientMgr->m_NetMgr.RemoveDriver(pDriver);
		return LT_OK;
	}

	return LT_ERROR;
}

LTRESULT ci_GetTcpIpAddress(char* sAddress, uint32 dwBufferSize, uint16 &hostPort)
{
	if (!sAddress)
		RETURN_ERROR(1, CLTClient::GetTcpIpAddress, LT_INVALIDPARAMS);

	return g_pClientMgr->m_NetMgr.GetLocalIpAddress(sAddress, dwBufferSize, hostPort);
}

LTRESULT ci_IsLobbyLaunched(const char* sDriver)
{
	if (!sDriver)
		sDriver = "internet";

	CBaseDriver* pDriver = g_pClientMgr->m_NetMgr.GetDriver(sDriver);
	if (!pDriver)
	{
		pDriver = g_pClientMgr->m_NetMgr.AddDriver(sDriver);
		if (!pDriver)
			RETURN_ERROR(1, CLTClient::IsLobbyLaunched, LT_ERROR);
	}

	if (!pDriver->IsLobbyLaunched())
		return(LT_ERROR);

	return(LT_OK);
}

LTRESULT ci_GetLobbyLaunchInfo(const char* sDriver, void** ppLobbyLaunchData)
{
	if (!ppLobbyLaunchData)
		RETURN_ERROR(1, CLTClient::GetLobbyLaunchInfo, LT_INVALIDPARAMS);

	if (!sDriver)
		sDriver = "internet";

	CBaseDriver* pDriver = g_pClientMgr->m_NetMgr.GetDriver(sDriver);
	if (!pDriver)
	{
		pDriver = g_pClientMgr->m_NetMgr.AddDriver(sDriver);
		if (!pDriver)
			RETURN_ERROR(1, CLTClient::GetLobbyLaunchInfo, LT_ERROR);
	}

	if (!pDriver->GetLobbyLaunchInfo(ppLobbyLaunchData))
		RETURN_ERROR(1, CLTClient::GetLobbyLaunchInfo, LT_ERROR);

	return LT_OK;
}


bool ci_IntersectSegment(IntersectQuery *pQuery, ClientIntersectInfo *pInfo)
{
	return world_bsp_client->IntersectSegment(pQuery, pInfo);
}

bool ci_IntersectSweptSphere(const LTVector& vStart, const LTVector& vEnd, float fRadius, LTVector& vPos, LTVector& vNormal)
{
	return world_bsp_client->IntersectSweptSphere(vStart, vEnd, fRadius, vPos, vNormal);
}


LTRESULT ci_SetConsoleEnable(bool bEnable)
{
	g_ClientGlob.m_bConsoleEnabled = bEnable;
	if (bEnable == FALSE)
	{
		g_ClientGlob.m_bIsConsoleUp = FALSE;
	}

	return	LT_OK;
}




// ----------------------------------------------------------------- //
// Interface functions.
// ----------------------------------------------------------------- //

void CLTClient::InitFunctionPointers()
{
	// Set all the function pointers.
	StartGame = ci_StartGame;
	GetGameMode = ci_GetGameMode;
	GetLocalClientID = ci_GetLocalClientID;
	IsConnected = ci_IsConnected;
	Disconnect = ci_Disconnect;
	Shutdown = ci_Shutdown;
	ShutdownWithMessage = ci_ShutdownWithMessage;

	GetRenderModes = dsi_GetRenderModes;
	RelinquishRenderModes = dsi_RelinquishRenderModes;
	SetRenderMode = dsi_SetRenderMode;
	GetRenderMode = dsi_GetRenderMode;
	ShutdownRender = dsi_ShutdownRender;

	GetFileList = ci_GetFileList;
	FreeFileList = ic_FreeFileList;
	ReadConfigFile = ci_ReadConfigFile;
	WriteConfigFile = ci_WriteConfigFile;
	WriteConfigFileFields = ci_WriteConfigFileFields;

	GetAxisOffsets = ci_GetAxisOffsets;
	PlayJoystickEffect = ci_PlayJoystickEffect;

	InitMusic = ci_InitMusic;
	LoadSong = ci_LoadSong;
	DestroyAllSongs = ci_DestroyAllSongs;
	StopMusic = ci_StopMusic;
	PauseMusic = ci_PauseMusic;
	ResumeMusic = ci_ResumeMusic;
	AddSongToPlayList = ci_AddSongToPlayList;
	DeletePlayList = ci_DeletePlayList;
	PlayList = ci_PlayList;
	GetMusicVolume = ci_GetMusicVolume;
	SetMusicVolume = ci_SetMusicVolume;

	GetSoundTimer = ci_GetSoundTimer;
	GetSoundData = ci_GetSoundData;
	GetSoundOffset = ci_GetSoundOffset;
	IsDone = ci_IsDone;
	GetListener = ci_GetListener;

	RegisterConsoleProgram = ci_RegisterConsoleProgram;
	UnregisterConsoleProgram = ci_UnregisterConsoleProgram;
	GetConsoleVar = ci_GetConsoleVar;

	StartCounter = ic_StartCounter;
	EndCounter = ic_EndCounter;

	SetInputState = ci_SetInputState;
	ClearInput = ci_ClearInput;

	GetDeviceBindings = ci_GetDeviceBindings;
	FreeDeviceBindings = ci_FreeDeviceBindings;

	ClearBinding = ci_ClearBinding;
	AddBinding = ci_AddBinding;

	StartDeviceTrack = ci_StartDeviceTrack;
	TrackDevice = ci_TrackDevice;
	EndDeviceTrack = ci_EndDeviceTrack;

	GetDeviceObjects = ci_GetDeviceObjects;
	FreeDeviceObjects = ci_FreeDeviceObjects;

	GetDeviceName = ci_GetDeviceName;
	IsDeviceEnabled = ci_IsDeviceEnabled;
	EnableDevice = ci_EnableDevice;
	GetNumRawDevices = ci_GetNumRawDevices;
	GetRawDeviceData = ci_GetRawDeviceData;
	GetRawDeviceDataSize = ci_GetRawDeviceDataSize;
	GetRawDeviceActuatorDataSize = ci_GetRawDeviceActuatorDataSize;
	SetRawDeviceActuatorData = ci_SetRawDeviceActuatorData;

	GetGameTime = ci_GetGameTime;
	GetGameFrameTime = ci_GetGameFrameTime;

	DebugOut = ::DebugOut;
	GetSkyDef = ci_GetSkyDef;

	IsCommandOn = ci_IsCommandOn;
	RunConsoleString = ci_RunConsoleString;
	GetClientObject = ci_GetClientObject;

	GetGlobalLightScale = ci_GetGlobalLightScale;
	SetGlobalLightScale = ci_SetGlobalLightScale;
	OffsetGlobalLightScale = ci_OffsetGlobalLightScale;

	CreateObject = ci_CreateObject;

	IntersectSegment = ci_IntersectSegment;
	IntersectSweptSphere = ci_IntersectSweptSphere;
	CastRay = ci_CastRay;

	GetObjectScale = ci_GetObjectScale;
	SetObjectScale = ci_SetObjectScale;

	FindObjectsInSphere = ci_FindObjectsInSphere;
	FindObjectsInBox = ci_FindObjectsInBox;

	SetObjectRotation = ci_SetObjectRotation;
	SetObjectPosAndRotation = ci_SetObjectPosAndRotation;

	GetObjectColor = ci_GetObjectColor;
	SetObjectColor = ci_SetObjectColor;

	GetObjectUserData = ci_GetObjectUserData;
	SetObjectUserData = ci_SetObjectUserData;

	Get3DCameraPt = ci_Get3DCameraPt;
	GetCameraFOV = ci_GetCameraFOV;
	SetCameraFOV = ci_SetCameraFOV;
	GetCameraRect = ci_GetCameraRect;
	SetCameraRect = ci_SetCameraRect;
	GetCameraLightAdd = ci_GetCameraLightAdd;
	SetCameraLightAdd = ci_SetCameraLightAdd;

	SetupParticleSystem = ci_SetupParticleSystem;
	AddParticles = ci_AddParticles;
	SortParticles = ci_SortParticles;
	GetParticles = ci_GetParticles;
	RemoveParticle = ci_RemoveParticle;
	OptimizeParticles = ci_OptimizeParticles;
	SetParticleSystemEffectShaderID = ci_SetParticleSystemEffectShaderID;

	GetNextLine = linesystem_GetNextLine;
	GetLineInfo = linesystem_GetLineInfo;
	SetLineInfo = linesystem_SetLineInfo;
	AddLine = linesystem_AddLine;
	RemoveLine = linesystem_RemoveLine;

	SetupVolumeEffect = ci_SetupVolumeEffect;
	SetVolumeEffectEffectShaderID = ci_SetVolumeEffectEffectShaderID;

	GetParticleBlockersInAABB = ci_GetParticleBlockersInAABB;
	GetParticleBlockerEdgesXZ = ci_GetParticleBlockerEdgesXZ;

	SetupPolyGrid = ci_SetupPolyGrid;
	SetPolyGridEnvMap = ci_SetPolyGridEnvMap;
	SetPolyGridTexture = ci_SetPolyGridTexture;
	GetPolyGridTextureInfo = ci_GetPolyGridTextureInfo;
	SetPolyGridTextureInfo = ci_SetPolyGridTextureInfo;
	GetPolyGridInfo = ci_GetPolyGridInfo;
	SetPolyGridEffectShaderID = ci_SetPolyGridEffectShaderID;

	GetLightColor = ci_GetLightColor;
	SetLightColor = ci_SetLightColor;
	GetLightRadius = ci_GetLightRadius;
	SetLightRadius = ci_SetLightRadius;

	ClipSprite = ci_ClipSprite;

	GetPolyTextureFlags = ci_GetPolyTextureFlags;
	SetModelHook = ci_SetModelHook;

	InitNetworking = ci_InitNetworking;
	GetServiceList = ci_GetServiceList;
	FreeServiceList = ci_FreeServiceList;
	SelectService = ci_SelectService;
	GetSessionList = ci_GetSessionList;
	FreeSessionList = ci_FreeSessionList;
	AddInternetDriver = ci_AddInternetDriver;
	RemoveInternetDriver = ci_RemoveInternetDriver;
	GetLobbyLaunchInfo = ci_GetLobbyLaunchInfo;
	IsLobbyLaunched = ci_IsLobbyLaunched;
	GetTcpIpAddress = ci_GetTcpIpAddress;

	GetVersionInfoExt = in_GetVersionInfoExt;
	GetPerformanceInfo = in_GetPerformanceInfo;

	SetConsoleEnable = ci_SetConsoleEnable;

	// Init the system-dependent stuff.
	cis_Init();
}


void ci_Term()
{
	cis_Term();
}
