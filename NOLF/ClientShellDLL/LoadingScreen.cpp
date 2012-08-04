// ----------------------------------------------------------------------- //
//
// MODULE  : LoadingScreen.cpp
//
// PURPOSE : Background-thread loading screen encapsulation class
//
// CREATED : 2000
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "LoadingScreen.h"
#include "InterfaceMgr.h"
#include "GameClientShell.h"
#include "WinUtil.h"
#include "BaseFolder.h"

extern CGameClientShell* g_pGameClientShell;

CLoadingScreen::CLoadingScreen() :
	m_eCurState(STATE_NONE),
	m_bDrawMultiplayer(LTFALSE),
	m_nNumAttachments(0),
	m_hWorldName(LTNULL),
	m_hWorldPhoto(LTNULL),
	m_TextPos(0,0),
	m_PhotoPos(0,0)

{
	// Create the event handles
	m_hEventEnd = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hEventThreadRunning = CreateEvent(NULL, TRUE, FALSE, NULL);

	m_nOldFarZ = 10000;
	m_bOldFogEnable = LTFALSE;
}

CLoadingScreen::~CLoadingScreen()
{
	// Terminate the object, just in case...
	Term();

	DeleteObject(m_hEventEnd);
	DeleteObject(m_hEventThreadRunning);
}

void CLoadingScreen::CreateScaleFX(char *szFXName)
{
	CScaleFX* pScaleFX = g_pFXButeMgr->GetScaleFX(szFXName);
	if (pScaleFX)
	{
		CBaseScaleFX *pSFX = debug_new(CBaseScaleFX);
		g_pFXButeMgr->CreateScaleFX(pScaleFX,m_vPos, m_vF, LTNULL, &m_rRot, pSFX);
		m_SFXArray.Add(pSFX);
		g_pInterfaceMgr->AddInterfaceSFX(pSFX, IFX_NORMAL);				

		//adjust the object's position based on screen res
		HOBJECT hSFX = pSFX->GetObject();
		if (hSFX)
		{
			LTVector vNewPos;
			g_pLTClient->GetObjectPos(hSFX, &vNewPos);
			vNewPos.z *= g_pInterfaceResMgr->GetXRatio();
			g_pLTClient->SetObjectPos(hSFX, &vNewPos);
		}
	}
}

void CLoadingScreen::CreateCharFX(INT_CHAR *pChar)
{
	if (pChar)
	{
		BSCREATESTRUCT bcs;
	    LTVector vPos, vTemp, vScale(1.0f,1.0f,1.0f);
	    LTRotation rRot = m_rRot;

		char modName[128];
		char skinName[128];
		char skin2Name[128];

		ModelId eModelId = g_pModelButeMgr->GetModelId(pChar->szModel);
		ModelStyle	eModelStyle = g_pModelButeMgr->GetStyle(pChar->szStyle);
		const char* pFilename = g_pModelButeMgr->GetModelFilename(eModelId, eModelStyle);
		SAFE_STRCPY(modName, pFilename);
		const char* pSkin = g_pModelButeMgr->GetBodySkinFilename(eModelId, eModelStyle);
		SAFE_STRCPY(skinName, pSkin);
		const char* pSkin2 = g_pModelButeMgr->GetHeadSkinFilename(eModelId, eModelStyle);
		SAFE_STRCPY(skin2Name, pSkin2);

		VEC_COPY(vPos,m_vPos);
		VEC_SET(vScale,1.0f,1.0f,1.0f);
		VEC_MULSCALAR(vScale, vScale, pChar->fScale);

		LTVector vModPos = pChar->vPos;
	    LTFLOAT fRot = pChar->fRot;
		fRot  = MATH_PI + DEG2RAD(fRot);
	    g_pLTClient->RotateAroundAxis(&rRot, &m_vU, fRot);

		VEC_MULSCALAR(vTemp, m_vF, vModPos.z);
		VEC_MULSCALAR(vTemp, vTemp, g_pInterfaceResMgr->GetXRatio());
		VEC_ADD(vPos, vPos, vTemp);

		VEC_MULSCALAR(vTemp, m_vR, vModPos.x);
		VEC_ADD(vPos, vPos, vTemp);

		VEC_MULSCALAR(vTemp, m_vU, vModPos.y);
		VEC_ADD(vPos, vPos, vTemp);

		VEC_COPY(bcs.vPos, vPos);
		bcs.rRot = rRot;
		VEC_COPY(bcs.vInitialScale, vScale);
		VEC_COPY(bcs.vFinalScale, vScale);
		VEC_SET(bcs.vInitialColor, 1.0f, 1.0f, 1.0f);
		VEC_SET(bcs.vFinalColor, 1.0f, 1.0f, 1.0f);
		bcs.bUseUserColors = LTTRUE;

		bcs.pFilename = modName;
		bcs.pSkin = skinName;
		bcs.pSkin2 = skin2Name;
		bcs.dwFlags = FLAG_VISIBLE | FLAG_FOGDISABLE | FLAG_NOLIGHT;

		bcs.nType = OT_MODEL;
		bcs.fInitialAlpha = 1.0f;
		bcs.fFinalAlpha = 1.0f;
		bcs.fLifeTime = 1000000.0f;
		bcs.bLoop = LTTRUE;


		if (m_CharSFX.Init(&bcs))
		{
			m_CharSFX.CreateObject(g_pLTClient);
			if (m_CharSFX.GetObject())
			{
				g_pInterfaceMgr->AddInterfaceSFX(&m_CharSFX, IFX_WORLD);
			}
		}
	}
}

void CLoadingScreen::CreateAttachFX(INT_ATTACH *pAttach)
{
	if (m_nNumAttachments < MAX_INT_ATTACHMENTS)
	{
		BSCREATESTRUCT bcs;
	    LTVector vPos, vTemp, vScale(1.0f,1.0f,1.0f);
	    LTRotation rRot = m_rRot;

		CString str = "";
		char szModel[128];
		char szSkin[128];

		str = g_pAttachButeMgr->GetAttachmentModel(pAttach->nAttachmentID);
		strncpy(szModel, (char*)(LPCSTR)str, 128);

		str = g_pAttachButeMgr->GetAttachmentSkin(pAttach->nAttachmentID);
		strncpy(szSkin, (char*)(LPCSTR)str, 128);

		VEC_SET(vScale,1.0f,1.0f,1.0f);
		VEC_MULSCALAR(vScale, vScale, pAttach->fScale);

		VEC_COPY(bcs.vInitialScale, vScale);
		VEC_COPY(bcs.vFinalScale, vScale);
		VEC_SET(bcs.vInitialColor, 1.0f, 1.0f, 1.0f);
		VEC_SET(bcs.vFinalColor, 1.0f, 1.0f, 1.0f);
		bcs.bUseUserColors = LTTRUE;

		bcs.pFilename = szModel;
		bcs.pSkin = szSkin;
		bcs.dwFlags = FLAG_VISIBLE | FLAG_FOGDISABLE | FLAG_NOLIGHT;

		bcs.nType = OT_MODEL;
		bcs.fInitialAlpha = 1.0f;
		bcs.fFinalAlpha = 1.0f;
		bcs.fLifeTime = 1000000.0f;
		bcs.bLoop = LTTRUE;

		CBaseScaleFX *pSFX = &m_aAttachment[m_nNumAttachments].sfx;

		if (!pSFX->Init(&bcs)) return;
		
		pSFX->CreateObject(g_pLTClient);
		if (!pSFX->GetObject()) return;

		HOBJECT hChar = m_CharSFX.GetObject();
		if (!hChar) return;
		if (g_pModelLT->GetSocket(hChar, pAttach->pszSocket, m_aAttachment[m_nNumAttachments].socket) != LT_OK)
			return;

		g_pInterfaceMgr->AddInterfaceSFX(pSFX, IFX_ATTACH);
		m_nNumAttachments++;
	}
}

void CLoadingScreen::CreateInterfaceSFX(eFolderID eFolder)
{
	HOBJECT hCamera = g_pGameClientShell->GetInterfaceCamera();
	if (!hCamera) return;

    g_pLTClient->GetObjectPos(hCamera, &m_vPos);
    g_pLTClient->GetObjectRotation(hCamera, &m_rRot);
    g_pLTClient->GetRotationVectors(&m_rRot, &m_vU, &m_vR, &m_vF);

	int n = 0;
	char szAttName[30];

	sprintf(szAttName,"ScaleName%d",n);
	while (g_pLayoutMgr->HasCustomValue(eFolder,szAttName))
	{
		char szFXName[128];
		g_pLayoutMgr->GetFolderCustomString(eFolder,szAttName,szFXName,128);
		if (strlen(szFXName))
		{
			CreateScaleFX(szFXName);
		}

		n++;
		sprintf(szAttName,"ScaleName%d",n);

	}

	INT_CHAR *pChar = g_pLayoutMgr->GetFolderCharacter(eFolder);
	if (pChar)
	{
		CreateCharFX(pChar);
		if (m_CharSFX.GetObject())
		{
			int i;
			int reqID[MAX_INT_ATTACHMENTS];
			int numReq = g_pAttachButeMgr->GetRequirementIDs(pChar->szModel,pChar->szStyle,reqID,MAX_INT_ATTACHMENTS);
			for (i = 0; i < numReq; i++)
			{
				INT_ATTACH acs;
				acs.fScale = pChar->fScale;
				acs.nAttachmentID = g_pAttachButeMgr->GetRequirementAttachment(reqID[i]);
				CString socket = g_pAttachButeMgr->GetRequirementSocket(reqID[i]);
				acs.pszSocket = (char *)(LPCTSTR)socket;

				CreateAttachFX(&acs);
			}

			int numAtt = g_pLayoutMgr->GetFolderNumAttachments(eFolder);
			for (i = 0; i < numAtt; i++)
			{
				char szTemp[128];
				char *pName = LTNULL;
				char *pSocket = LTNULL;
				g_pLayoutMgr->GetFolderAttachment(eFolder, i, szTemp, 128);

				pName = strtok(szTemp,";");
				pSocket = strtok(NULL,";");

				INT_ATTACH acs;

				acs.fScale = pChar->fScale;
				acs.nAttachmentID = g_pAttachButeMgr->GetAttachmentIDByName(pName);
				acs.pszSocket = pSocket;

				CreateAttachFX(&acs);
			}
		}
	}
}

void CLoadingScreen::ClearAttachFX()
{
	for (int i = 0; i < m_nNumAttachments; i++)
	{
		g_pInterfaceMgr->RemoveInterfaceSFX(&m_aAttachment[i].sfx);
		m_aAttachment[i].sfx.Reset();
		m_aAttachment[i].sfx.Term();
		m_aAttachment[i].socket = INVALID_MODEL_SOCKET;
	}
	m_nNumAttachments = 0;
}

void CLoadingScreen::RemoveInterfaceSFX()
{
	while (m_SFXArray.GetSize() > 0)
	{
		CSpecialFX *pSFX = m_SFXArray[0];
		g_pInterfaceMgr->RemoveInterfaceSFX(pSFX);
		debug_delete(pSFX);
		m_SFXArray.Remove(0);
	}

	g_pInterfaceMgr->RemoveInterfaceSFX(&m_CharSFX);
	m_CharSFX.Reset();
	m_CharSFX.Term();

	ClearAttachFX();
}

void CLoadingScreen::UpdateInterfaceSFX()
{
	// Update the model's animtracker by hand...
	g_pInterfaceMgr->UpdateModelAnimations(m_fCurFrameDelta);

	for (int i = 0; i < m_nNumAttachments; i++)
	{
		CBaseScaleFX *pSFX = &m_aAttachment[i].sfx;
		
		HMODELSOCKET hSocket = m_aAttachment[i].socket;
		LTVector vPos;
		LTRotation rRot;
		LTransform transform;
		if (g_pModelLT->GetSocketTransform(m_CharSFX.GetObject(), hSocket, transform, LTTRUE) == LT_OK)
		{
			g_pTransLT->Get(transform, vPos, rRot);
            g_pLTClient->SetObjectPos(pSFX->GetObject(), &vPos, LTTRUE);
            g_pLTClient->SetObjectRotation(pSFX->GetObject(), &rRot);

		}
	}
}

LTBOOL CLoadingScreen::Init()
{
	if (m_eCurState != STATE_NONE)
		return LTFALSE;

	// Load the "Loading..." backdrop
	if (g_pGameClientShell->GetGameType() == SINGLE)
	{
		CreateInterfaceSFX(FOLDER_ID_LOADSCREEN_SINGLE);
	}
	else
	{
		CreateInterfaceSFX(FOLDER_ID_LOADSCREEN_MULTI);
	}

	m_TextPos = g_pLayoutMgr->GetLoadStringPos();
	m_PhotoPos = g_pLayoutMgr->GetLoadPhotoPos();

	m_TextPos.x += g_pInterfaceResMgr->GetXOffset();
	m_TextPos.y +=  g_pInterfaceResMgr->GetYOffset();
	m_PhotoPos.x += g_pInterfaceResMgr->GetXOffset();
	m_PhotoPos.y +=  g_pInterfaceResMgr->GetYOffset();

	// Remember whether or not we're in multiplayer
	m_bDrawMultiplayer = g_pGameClientShell->IsMultiplayerGame();// && g_pInterfaceMgr->IsFragCountDrawn();
	// Reset the frame counter
	m_nFrameCounter = 0;
	m_fLastFrameTime = CWinUtil::GetTime();
	m_fCurFrameDelta = 0.0f;
	
	m_eCurState = STATE_INIT;

	return LTTRUE;
}

LTBOOL CLoadingScreen::Term()
{
	if (m_eCurState != STATE_INIT)
		return LTFALSE;

	// Clean up
	RemoveInterfaceSFX();

	m_eCurState = STATE_NONE;

	if (m_hWorldName)
	{
		g_pLTClient->FreeString(m_hWorldName);
		m_hWorldName = LTNULL;
	}

	if (m_hWorldPhoto)
	{
		g_pLTClient->DeleteSurface(m_hWorldPhoto);
		m_hWorldPhoto = LTNULL;
	}

	return LTTRUE;
}

DWORD WINAPI CLoadingScreen::ThreadBootstrap(void *pData)
{
	return ((CLoadingScreen*)pData)->RunThread();
}

int CLoadingScreen::RunThread()
{
	// Change state
	m_eCurState = STATE_ACTIVE;

	// Tell the main thread we're now in our main loop
	SetEvent(m_hEventThreadRunning);

	// The main rendering loop...  (i.e. keep drawing until someone tells us to stop)
	while (WaitForSingleObject(m_hEventEnd, 0) == WAIT_TIMEOUT)
	{
		// Draw the frame..
		Update();
		
		// Make sure we're not running faster than 10fps so stuff can still happen in the background
		Sleep(100);
	}
	return 0;
}

LTBOOL CLoadingScreen::Update()
{
	// Make sure we're in a valid state...
	if ((m_eCurState != STATE_ACTIVE) && (m_eCurState != STATE_SHOW))
		return LTFALSE;

	g_pLTClient->ClearScreen(LTNULL, CLEARSCREEN_SCREEN | CLEARSCREEN_RENDER);
	// Mmm..  Triple dimensional...
	g_pLTClient->Start3D();

	// Update our interface SFX since the interface mgr doesn't know about us
	UpdateInterfaceSFX();

	// Update the interface mgr
	g_pInterfaceMgr->DrawSFX();


    HSURFACE hDestSurf = g_pLTClient->GetScreenSurface();
	HLTCOLOR hShadeColor = g_pLayoutMgr->GetShadeColor();

	// Go into optimized2d so the multiplayer info can draw
	g_pLTClient->StartOptimized2D();

//		g_pInterfaceMgr->GetClientInfoMgr()->Draw(LTTRUE, LTTRUE);

	if (m_hWorldPhoto)
		g_pLTClient->DrawSurfaceToSurface(hDestSurf,m_hWorldPhoto,LTNULL,m_PhotoPos.x,m_PhotoPos.y);

	CLTGUIFont *pFont = g_pInterfaceResMgr->GetTitleFont();
	
	pFont->Draw(m_hWorldName, hDestSurf, m_TextPos.x+1, m_TextPos.y+1, LTF_JUSTIFY_CENTER,kBlack);
	pFont->Draw(m_hWorldName, hDestSurf, m_TextPos.x, m_TextPos.y, LTF_JUSTIFY_CENTER,kWhite);

    g_pLTClient->EndOptimized2D();

	g_pLTClient->End3D();



	// Fill in the out-of-resolution areas
	int xo = g_pInterfaceResMgr->GetXOffset();
	int yo = g_pInterfaceResMgr->GetYOffset();

//	HSURFACE hBack = g_pInterfaceResMgr->GetSharedSurface(m_sBackground);
//  g_pLTClient->DrawSurfaceToSurface(hDestSurf, hBack, LTNULL, xo, yo);


    LTRect rect(0,0,g_pInterfaceResMgr->GetScreenWidth(),yo);
    g_pLTClient->FillRect(hDestSurf,&rect,hShadeColor);

	rect.top = yo;
	rect.bottom = rect.top;
    //g_pLTClient->FillRect(hDestSurf,&rect,m_hBarColor);

	rect.bottom = g_pInterfaceResMgr->GetScreenWidth();
	rect.top = rect.bottom;
    g_pLTClient->FillRect(hDestSurf,&rect,hShadeColor);

	rect.bottom = g_pInterfaceResMgr->GetScreenHeight();
	rect.top = (rect.bottom - yo);
    g_pLTClient->FillRect(hDestSurf,&rect,hShadeColor);

	rect.bottom = rect.top;
	rect.top = rect.bottom;
    //g_pLTClient->FillRect(hDestSurf,&rect,m_hBarColor);

	if (xo > 0)
	{
		rect.left = 0;
		rect.right = xo;
		rect.top = yo;
		rect.bottom = (g_pInterfaceResMgr->GetScreenHeight() - yo);
	    g_pLTClient->FillRect(hDestSurf,&rect,hShadeColor);

		rect.right = g_pInterfaceResMgr->GetScreenWidth();
		rect.left = g_pInterfaceResMgr->GetScreenWidth() - xo;
		rect.top = yo;
		rect.bottom = (g_pInterfaceResMgr->GetScreenHeight() - yo);
	    g_pLTClient->FillRect(hDestSurf,&rect,hShadeColor);
	}

    g_pLTClient->FlipScreen(FLIPSCREEN_CANDRAWCONSOLE);

	// Count it..
	++m_nFrameCounter;

	LTFLOAT fCurTime = CWinUtil::GetTime();
	m_fCurFrameDelta = fCurTime - m_fLastFrameTime;
	m_fLastFrameTime = fCurTime;

	return LTTRUE;
}

LTBOOL CLoadingScreen::Show(LTBOOL bRun)
{
	if (bRun && !GetConsoleInt("DynamicLoadScreen",1))
		bRun = LTFALSE;
	// Make sure we're in the correct state
	if (m_eCurState == STATE_NONE)
	{
		if (!Init())
			return LTFALSE;
	}

	if (m_eCurState != STATE_INIT)
		return LTFALSE;

	// Turn off the cursor

	g_pInterfaceMgr->UseCursor(LTFALSE);

	// Set up the FarZ & turn off fog (farz of 0 is bogus)

	m_nOldFarZ = GetConsoleInt("FarZ", 10000);
	m_nOldFarZ = m_nOldFarZ == 0 ? 10000 : m_nOldFarZ;

	m_bOldFogEnable = (LTBOOL) GetConsoleInt("FogEnable", 0);

	g_pGameClientShell->SetFarZ(10000);
	WriteConsoleInt("FogEnable", 0);

	// Make sure we're not in optimized 2D mode (happens sometimes...)
	g_pLTClient->EndOptimized2D();

	
	// Go into the right state..
	m_eCurState = STATE_SHOW;

	// Update once so the screen's showing
	Update();

	// Start updating if they wanted it to..
	if (bRun)
		return Resume();

	// Ok, it's visible or active
	return LTTRUE;
}

LTBOOL CLoadingScreen::Pause()
{
	// Make sure we're in the right state
	if (m_eCurState != STATE_ACTIVE)
		return LTFALSE;

	// Shut down the loading screen thread
	SetEvent(m_hEventEnd);
	WaitForSingleObject(m_hThreadHandle, INFINITE);

	// Ok, it's just visible now..
	m_eCurState = STATE_SHOW;

	return LTTRUE;
}

LTBOOL CLoadingScreen::Resume()
{
	// Ensure our state
	if (m_eCurState != STATE_SHOW)
		return LTFALSE;

	// Reset the events
	ResetEvent(m_hEventEnd);
	ResetEvent(m_hEventThreadRunning);

	// Start up the loading screen thread
	uint32 uThreadID;
	m_hThreadHandle = CreateThread(NULL, 0, ThreadBootstrap, (void *)this, 0, &uThreadID);

	// Handle what shouldn't be possible..
	if (!m_hThreadHandle)
		return LTFALSE;

	// Wait for the loading thread to stop touching stuff..
	WaitForSingleObject(m_hEventThreadRunning, INFINITE);

	// Now we're actually active.  (Thank you Mr. Thread..)
	return LTTRUE;
}

LTBOOL CLoadingScreen::Hide()
{
	// Ensure our state
	if (m_eCurState == STATE_ACTIVE)
	{
		// Stop!!!
		if (!Pause())
			return LTFALSE;
	}

	if (m_eCurState != STATE_SHOW)
		return LTFALSE;

	// Clear the screen
	g_pInterfaceMgr->ClearAllScreenBuffers();

	// Change state
	m_eCurState = STATE_INIT;

	// Clean up
	Term();

	// Re-set the console...
	g_pGameClientShell->SetFarZ(m_nOldFarZ);
	WriteConsoleInt("FogEnable", (int)m_bOldFogEnable);

	// Done!
	return LTTRUE;
}


void CLoadingScreen::SetWorldName(HSTRING hWorld)
{
	if (m_hWorldName)
	{
		g_pLTClient->FreeString(m_hWorldName);
		m_hWorldName = LTNULL;
	}

	m_hWorldName = g_pLTClient->CopyString(hWorld);
}

void CLoadingScreen::SetWorldPhoto(char *pszPhoto)
{
	if (!pszPhoto) return;
	if (m_hWorldPhoto)
	{
		g_pLTClient->DeleteSurface(m_hWorldPhoto);
		m_hWorldPhoto = LTNULL;
	}

	m_hWorldPhoto = g_pLTClient->CreateSurfaceFromBitmap(pszPhoto);
	if (!m_hWorldPhoto)
		m_hWorldPhoto = g_pLTClient->CreateSurfaceFromBitmap("interface\\photos\\missions\\default.pcx");

}