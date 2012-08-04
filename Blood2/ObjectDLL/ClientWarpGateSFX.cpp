// ----------------------------------------------------------------------- //
//
// MODULE  : ClientWarpGateSFX.cpp
//
// PURPOSE : CClientWarpGateSFX - Definition
//
// CREATED : 8-15-98
//
// ----------------------------------------------------------------------- //

#include "ClientWarpGateSFX.h"
#include "ClientServerShared.h"
#include "ObjectUtilities.h"
#include "cpp_server_de.h"
#include "SFXMsgIds.h"
#include <mbstring.h>


BEGIN_CLASS(CClientWarpGateSFX)
	ADD_REALPROP(RampUpTime, 5.0f)
	ADD_REALPROP(RampDownTime, 5.0f)
	ADD_BOOLPROP(InitiallyOn, DFALSE)

	PROP_DEFINEGROUP(SpriteData, PF_GROUP1)
		ADD_REALPROP_FLAG(SpriteMinScale, 0.1f, PF_GROUP1)
		ADD_REALPROP_FLAG(SpriteMaxScale, 1.0f, PF_GROUP1)
		ADD_REALPROP_FLAG(SpriteAlpha, 0.75f, PF_GROUP1)
		ADD_LONGINTPROP_FLAG(SpriteRampUpType, 1, PF_GROUP1)
		ADD_LONGINTPROP_FLAG(SpriteRampDownType, 2, PF_GROUP1)
		ADD_BOOLPROP_FLAG(SpriteAlign, DFALSE, PF_GROUP1)
		ADD_STRINGPROP_FLAG(SpriteFile, "Sprites\\rift.spr", PF_GROUP1)

	PROP_DEFINEGROUP(ParticleSystem1Data, PF_GROUP2)
		ADD_REALPROP_FLAG(PS1SystemRadius, 5000.0f, PF_GROUP2)
		ADD_REALPROP_FLAG(PS1PositionRadius, 100.0f, PF_RADIUS | PF_GROUP2)
		ADD_VECTORPROP_FLAG(PS1Offset, PF_GROUP2)
		ADD_VECTORPROP_FLAG(PS1Rotations, PF_GROUP2)
		ADD_REALPROP_FLAG(PS1MinVelocity, 10.0f, PF_GROUP2)
		ADD_REALPROP_FLAG(PS1MaxVelocity, 25.0f, PF_GROUP2)
		ADD_LONGINTPROP_FLAG(PS1NumParticles, 10, PF_GROUP2)
		ADD_LONGINTPROP_FLAG(PS1EmitType, 1, PF_GROUP2)
		ADD_COLORPROP_FLAG(PS1MinColor, 50.0f, 50.0f, 50.0f, PF_GROUP2)
		ADD_COLORPROP_FLAG(PS1MaxColor, 255.0f, 255.0f, 255.0f, PF_GROUP2)
		ADD_REALPROP_FLAG(PS1SystemAlpha, 0.5f, PF_GROUP2)
		ADD_REALPROP_FLAG(PS1MinLifetime, 4.0f, PF_GROUP2)
		ADD_REALPROP_FLAG(PS1MaxLifetime, 5.0f, PF_GROUP2)
		ADD_REALPROP_FLAG(PS1AddDelay, 0.25f, PF_GROUP2)
		ADD_REALPROP_FLAG(PS1Gravity, 15.0f, PF_GROUP2)
		ADD_LONGINTPROP_FLAG(PS1RampUpType, 1, PF_GROUP2)
		ADD_LONGINTPROP_FLAG(PS1RampDownType, 1, PF_GROUP2)
		ADD_BOOLPROP_FLAG(PS1Align, DFALSE, PF_GROUP2)
		ADD_STRINGPROP_FLAG(PS1ParticleFile, "SpriteTextures\\smoke64_3.dtx", PF_GROUP2)

	PROP_DEFINEGROUP(ParticleSystem2Data, PF_GROUP3)
		ADD_REALPROP_FLAG(PS2SystemRadius, 5000.0f, PF_GROUP3)
		ADD_REALPROP_FLAG(PS2PositionRadius, 100.0f, PF_RADIUS | PF_GROUP3)
		ADD_VECTORPROP_FLAG(PS2Offset, PF_GROUP3)
		ADD_VECTORPROP_FLAG(PS2Rotations, PF_GROUP3)
		ADD_REALPROP_FLAG(PS2MinVelocity, 10.0f, PF_GROUP3)
		ADD_REALPROP_FLAG(PS2MaxVelocity, 25.0f, PF_GROUP3)
		ADD_LONGINTPROP_FLAG(PS2NumParticles, 10, PF_GROUP3)
		ADD_LONGINTPROP_FLAG(PS2EmitType, 1, PF_GROUP3)
		ADD_COLORPROP_FLAG(PS2MinColor, 50.0f, 50.0f, 50.0f, PF_GROUP3)
		ADD_COLORPROP_FLAG(PS2MaxColor, 255.0f, 255.0f, 255.0f, PF_GROUP3)
		ADD_REALPROP_FLAG(PS2SystemAlpha, 0.5f, PF_GROUP3)
		ADD_REALPROP_FLAG(PS2MinLifetime, 4.0f, PF_GROUP3)
		ADD_REALPROP_FLAG(PS2MaxLifetime, 5.0f, PF_GROUP3)
		ADD_REALPROP_FLAG(PS2AddDelay, 0.25f, PF_GROUP3)
		ADD_REALPROP_FLAG(PS2Gravity, 15.0f, PF_GROUP3)
		ADD_LONGINTPROP_FLAG(PS2RampUpType, 1, PF_GROUP3)
		ADD_LONGINTPROP_FLAG(PS2RampDownType, 1, PF_GROUP3)
		ADD_BOOLPROP_FLAG(PS2Align, DFALSE, PF_GROUP3)
		ADD_STRINGPROP_FLAG(PS2ParticleFile, "SpriteTextures\\lensflare_1.dtx", PF_GROUP3)

	ADD_REALPROP(SoundRadius, 200.0f)
	ADD_STRINGPROP(RampUpSound, "")
	ADD_STRINGPROP(LoopSound, "")
	ADD_STRINGPROP(RampDownSound, "")
END_CLASS_DEFAULT(CClientWarpGateSFX, CClientSFX, NULL, NULL)

// ----------------------------------------------------------------------- //

CClientWarpGateSFX::CClientWarpGateSFX()
{
	// [blg]

	bOn           = DTRUE;
	bFirstUpdate  = DFALSE;
	bInitiallyOn  = DFALSE;

	fRampUpTime   = 5.0f;
	fRampDownTime = 5.0f;
	fSoundRadius  = 200.0f;

	hstrSound1 = DNULL;
	hstrSound2 = DNULL;
	hstrSound3 = DNULL;

	wSpr.fMinScale     = 0.1f;
	wSpr.fMaxScale     = 1.0f;
	wSpr.fAlpha        = 0.75f;
	wSpr.nRampUpType   = 1;
	wSpr.nRampDownType = 2;
	wSpr.bAlign        = DFALSE;
	wSpr.hstrSprite    = g_pServerDE->CreateString("Sprites\\rift.spr");

	wPS1.fSystemRadius = 5000.0f;
	wPS1.fPosRadius = 100.0f;
	wPS1.vOffset;
	wPS1.vRotations;
	wPS1.fMinVelocity = 10.0f;
	wPS1.fMaxVelocity = 25.0f;
	wPS1.nNumParticles = 10;
	wPS1.nEmitType = 1;
	VEC_SET(wPS1.vMinColor, 50.0f, 50.0f, 50.0f);
	VEC_SET(wPS1.vMaxColor, 255.0f, 255.0f, 255.0f);
	wPS1.fAlpha = 0.5f;
	wPS1.fMinLifetime = 4.0f;
	wPS1.fMaxLifetime = 5.0f;
	wPS1.fAddDelay = 0.25f;
	wPS1.fGravity = 15.0f;
	wPS1.nRampUpType = 1;
	wPS1.nRampDownType = 1;
	wPS1.bAlign = DFALSE;
	wPS1.hstrParticle = g_pServerDE->CreateString("SpriteTextures\\smoke64_3.dtx");

	wPS2.fSystemRadius = 5000.0f;
	wPS2.fPosRadius = 100.0f;
	wPS2.vOffset;
	wPS2.vRotations;
	wPS2.fMinVelocity = 10.0f;
	wPS2.fMaxVelocity = 25.0f;
	wPS2.nNumParticles = 10;
	wPS2.nEmitType = 1;
	VEC_SET(wPS2.vMinColor, 50.0f, 50.0f, 50.0f);
	VEC_SET(wPS2.vMaxColor, 255.0f, 255.0f, 255.0f);
	wPS2.fAlpha = 0.5f;
	wPS2.fMinLifetime = 4.0f;
	wPS2.fMaxLifetime = 5.0f;
	wPS2.fAddDelay = 0.25f;
	wPS2.fGravity = 15.0f;
	wPS2.nRampUpType = 1;
	wPS2.nRampDownType = 1;
	wPS2.bAlign = DFALSE;
	wPS2.hstrParticle = g_pServerDE->CreateString("SpriteTextures\\lensflare_1.dtx");
}

CClientWarpGateSFX::~CClientWarpGateSFX()
{
	if (wSpr.hstrSprite)
	{
		g_pServerDE->FreeString(wSpr.hstrSprite);
	}

	if (wPS1.hstrParticle)
	{
		g_pServerDE->FreeString(wPS1.hstrParticle);
	}

	if (wPS2.hstrParticle)
	{
		g_pServerDE->FreeString(wPS2.hstrParticle);
	}
}


// ----------------------------------------------------------------------- //

void CClientWarpGateSFX::SetupFX()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	DVector		tempPos;
	pServerDE->GetObjectPos(m_hObject, &tempPos);

	if(wSpr.hstrSprite)
	{
		HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&tempPos);
		pServerDE->WriteToMessageByte(hMessage, SFX_WARPGATESPRITE_ID);

		pServerDE->WriteToMessageObject(hMessage, m_hObject);
		pServerDE->WriteToMessageFloat(hMessage, fRampUpTime);
		pServerDE->WriteToMessageFloat(hMessage, fRampDownTime);
		pServerDE->WriteToMessageFloat(hMessage, wSpr.fMinScale);
		pServerDE->WriteToMessageFloat(hMessage, wSpr.fMaxScale);
		pServerDE->WriteToMessageFloat(hMessage, wSpr.fAlpha);
		pServerDE->WriteToMessageDWord(hMessage, wSpr.nRampUpType);
		pServerDE->WriteToMessageDWord(hMessage, wSpr.nRampDownType);
		pServerDE->WriteToMessageByte(hMessage, wSpr.bAlign);
		pServerDE->WriteToMessageHString(hMessage, wSpr.hstrSprite);

		pServerDE->EndMessage(hMessage);
	}

	if(wPS1.hstrParticle)
	{
		HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&tempPos);
		pServerDE->WriteToMessageByte(hMessage, SFX_WARPGATEPARTICLE_ID);

		pServerDE->WriteToMessageObject(hMessage, m_hObject);
		pServerDE->WriteToMessageFloat(hMessage, fRampUpTime);
		pServerDE->WriteToMessageFloat(hMessage, fRampDownTime);
		pServerDE->WriteToMessageFloat(hMessage, wPS1.fSystemRadius);
		pServerDE->WriteToMessageFloat(hMessage, wPS1.fPosRadius);
		pServerDE->WriteToMessageVector(hMessage, &wPS1.vOffset);
		pServerDE->WriteToMessageVector(hMessage, &wPS1.vRotations);
		pServerDE->WriteToMessageFloat(hMessage, wPS1.fMinVelocity);
		pServerDE->WriteToMessageFloat(hMessage, wPS1.fMaxVelocity);
		pServerDE->WriteToMessageDWord(hMessage, wPS1.nNumParticles);
		pServerDE->WriteToMessageDWord(hMessage, wPS1.nEmitType);
		pServerDE->WriteToMessageVector(hMessage, &wPS1.vMinColor);
		pServerDE->WriteToMessageVector(hMessage, &wPS1.vMaxColor);
		pServerDE->WriteToMessageFloat(hMessage, wPS1.fAlpha);
		pServerDE->WriteToMessageFloat(hMessage, wPS1.fMinLifetime);
		pServerDE->WriteToMessageFloat(hMessage, wPS1.fMaxLifetime);
		pServerDE->WriteToMessageFloat(hMessage, wPS1.fAddDelay);
		pServerDE->WriteToMessageFloat(hMessage, wPS1.fGravity);
		pServerDE->WriteToMessageDWord(hMessage, wPS1.nRampUpType);
		pServerDE->WriteToMessageDWord(hMessage, wPS1.nRampDownType);
		pServerDE->WriteToMessageByte(hMessage, wPS1.bAlign);
		pServerDE->WriteToMessageHString(hMessage, wPS1.hstrParticle);

		pServerDE->EndMessage(hMessage);
	}

	if(wPS2.hstrParticle)
	{
		HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&tempPos);
		pServerDE->WriteToMessageByte(hMessage, SFX_WARPGATEPARTICLE_ID);

		pServerDE->WriteToMessageObject(hMessage, m_hObject);
		pServerDE->WriteToMessageFloat(hMessage, fRampUpTime);
		pServerDE->WriteToMessageFloat(hMessage, fRampDownTime);
		pServerDE->WriteToMessageFloat(hMessage, wPS2.fSystemRadius);
		pServerDE->WriteToMessageFloat(hMessage, wPS2.fPosRadius);
		pServerDE->WriteToMessageVector(hMessage, &wPS2.vOffset);
		pServerDE->WriteToMessageVector(hMessage, &wPS2.vRotations);
		pServerDE->WriteToMessageFloat(hMessage, wPS2.fMinVelocity);
		pServerDE->WriteToMessageFloat(hMessage, wPS2.fMaxVelocity);
		pServerDE->WriteToMessageDWord(hMessage, wPS2.nNumParticles);
		pServerDE->WriteToMessageDWord(hMessage, wPS2.nEmitType);
		pServerDE->WriteToMessageVector(hMessage, &wPS2.vMinColor);
		pServerDE->WriteToMessageVector(hMessage, &wPS2.vMaxColor);
		pServerDE->WriteToMessageFloat(hMessage, wPS2.fAlpha);
		pServerDE->WriteToMessageFloat(hMessage, wPS2.fMinLifetime);
		pServerDE->WriteToMessageFloat(hMessage, wPS2.fMaxLifetime);
		pServerDE->WriteToMessageFloat(hMessage, wPS2.fAddDelay);
		pServerDE->WriteToMessageFloat(hMessage, wPS2.fGravity);
		pServerDE->WriteToMessageDWord(hMessage, wPS2.nRampUpType);
		pServerDE->WriteToMessageDWord(hMessage, wPS2.nRampDownType);
		pServerDE->WriteToMessageByte(hMessage, wPS2.bAlign);
		pServerDE->WriteToMessageHString(hMessage, wPS2.hstrParticle);

		pServerDE->EndMessage(hMessage);
	}
}

// ----------------------------------------------------------------------- //

DDWORD CClientWarpGateSFX::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	switch(messageID)
	{
		case MID_TRIGGER:
		{
			HandleTrigger(hSender, hRead);
			break;
		}
		default : break;
	}

	return CClientSFX::ObjectMessageFn(hSender, messageID, hRead);
}

// ----------------------------------------------------------------------- //

DDWORD CClientWarpGateSFX::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return 0;

	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE)	// [blg]
			{
				ReadProp((ObjectCreateStruct*)pData);
			}
			break;
		}

		case MID_INITIALUPDATE:
		{
			pServerDE->SetNextUpdate(m_hObject, 1.0f);

			DDWORD dwFlags = g_pServerDE->GetObjectFlags(m_hObject);
			dwFlags &= ~FLAG_GRAVITY;
			dwFlags |= FLAG_FORCECLIENTUPDATE;
			g_pServerDE->SetObjectFlags(m_hObject, dwFlags);

			DDWORD dwUsrFlags = g_pServerDE->GetObjectUserFlags(m_hObject);
			if(bOn)		dwUsrFlags |= USRFLG_VISIBLE;
				else	dwUsrFlags &= ~USRFLG_VISIBLE;
			dwUsrFlags |= USRFLG_SAVEABLE;
			g_pServerDE->SetObjectUserFlags(m_hObject, dwUsrFlags);
			bFirstUpdate = 1;
 			break;
		}

		case MID_UPDATE:
		{
			if(bFirstUpdate)	{ SetupFX(); bFirstUpdate = 0; }

			DDWORD dwUsrFlags = g_pServerDE->GetObjectUserFlags(m_hObject);
			if(bOn)		dwUsrFlags |= USRFLG_VISIBLE;
				else	dwUsrFlags &= ~USRFLG_VISIBLE;
			g_pServerDE->SetObjectUserFlags(m_hObject, dwUsrFlags);

			pServerDE->SetNextUpdate(m_hObject, 0.1f);
		    if(!pServerDE) pServerDE->RemoveObject(m_hObject);
			break;
		}

		case MID_SAVEOBJECT:
			Save((HMESSAGEWRITE)pData, (DDWORD)fData);
			break;

		case MID_LOADOBJECT:
			Load((HMESSAGEREAD)pData, (DDWORD)fData);
			break;

		default : break;
	}

	return CClientSFX::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //

void CClientWarpGateSFX::ReadProp(ObjectCreateStruct *pData)
{
	CServerDE* pServerDE = GetServerDE();
	if(!pServerDE || !pData) return;

	char		szString[MAX_CS_FILENAME_LEN];
	long		tempLong;

	if (wSpr.hstrSprite)	// [blg]
	{
		g_pServerDE->FreeString(wSpr.hstrSprite);
	}

	if (wPS1.hstrParticle)	// [blg]
	{
		g_pServerDE->FreeString(wPS1.hstrParticle);
	}

	if (wPS2.hstrParticle)	// [blg]
	{
		g_pServerDE->FreeString(wPS2.hstrParticle);
	}

	pServerDE->GetPropReal("RampUpTime", &fRampUpTime);
	pServerDE->GetPropReal("RampDownTime", &fRampDownTime);
	pServerDE->GetPropBool("InitiallyOn", &bOn);

	pServerDE->GetPropReal("SpriteMinScale", &wSpr.fMinScale);
	pServerDE->GetPropReal("SpriteMaxScale", &wSpr.fMaxScale);
	pServerDE->GetPropReal("SpriteAlpha", &wSpr.fAlpha);
	pServerDE->GetPropLongInt("SpriteRampUpType", &tempLong);
	wSpr.nRampUpType = (DDWORD)tempLong;
	pServerDE->GetPropLongInt("SpriteRampDownType", &tempLong);
	wSpr.nRampDownType = (DDWORD)tempLong;
	pServerDE->GetPropBool("SpriteAlign", &wSpr.bAlign);
	pServerDE->GetPropString("SpriteFile", szString, MAX_CS_FILENAME_LEN);
	wSpr.hstrSprite = pServerDE->CreateString(szString);

	pServerDE->GetPropReal("PS1SystemRadius", &wPS1.fSystemRadius);
	pServerDE->GetPropReal("PS1PositionRadius", &wPS1.fPosRadius);
	pServerDE->GetPropVector("PS1Offset", &wPS1.vOffset);
	pServerDE->GetPropVector("PS1Rotations", &wPS1.vRotations);
	pServerDE->GetPropReal("PS1MinVelocity", &wPS1.fMinVelocity);
	pServerDE->GetPropReal("PS1MaxVelocity", &wPS1.fMaxVelocity);
	pServerDE->GetPropLongInt("PS1NumParticles", &tempLong);
	wPS1.nNumParticles = (DDWORD)tempLong;
	pServerDE->GetPropLongInt("PS1EmitType", &tempLong);
	wPS1.nEmitType = (DDWORD)tempLong;
	pServerDE->GetPropColor("PS1MinColor", &wPS1.vMinColor);
	pServerDE->GetPropColor("PS1MaxColor", &wPS1.vMaxColor);
	pServerDE->GetPropReal("PS1SystemAlpha", &wPS1.fAlpha);
	pServerDE->GetPropReal("PS1MinLifetime", &wPS1.fMinLifetime);
	pServerDE->GetPropReal("PS1MaxLifetime", &wPS1.fMaxLifetime);
	pServerDE->GetPropReal("PS1AddDelay", &wPS1.fAddDelay);
	pServerDE->GetPropReal("PS1Gravity", &wPS1.fGravity);
	pServerDE->GetPropLongInt("PS1RampUpType", &tempLong);
	wPS1.nRampUpType = (DDWORD)tempLong;
	pServerDE->GetPropLongInt("PS1RampDownType", &tempLong);
	wPS1.nRampDownType = (DDWORD)tempLong;
	pServerDE->GetPropBool("PS1Align", &wPS1.bAlign);
	pServerDE->GetPropString("PS1ParticleFile", szString, MAX_CS_FILENAME_LEN);
	wPS1.hstrParticle = pServerDE->CreateString(szString);

	pServerDE->GetPropReal("PS2SystemRadius", &wPS2.fSystemRadius);
	pServerDE->GetPropReal("PS2PositionRadius", &wPS2.fPosRadius);
	pServerDE->GetPropVector("PS2Offset", &wPS2.vOffset);
	pServerDE->GetPropVector("PS2Rotations", &wPS2.vRotations);
	pServerDE->GetPropReal("PS2MinVelocity", &wPS2.fMinVelocity);
	pServerDE->GetPropReal("PS2MaxVelocity", &wPS2.fMaxVelocity);
	pServerDE->GetPropLongInt("PS2NumParticles", &tempLong);
	wPS2.nNumParticles = (DDWORD)tempLong;
	pServerDE->GetPropLongInt("PS2EmitType", &tempLong);
	wPS2.nEmitType = (DDWORD)tempLong;
	pServerDE->GetPropColor("PS2MinColor", &wPS2.vMinColor);
	pServerDE->GetPropColor("PS2MaxColor", &wPS2.vMaxColor);
	pServerDE->GetPropReal("PS2SystemAlpha", &wPS2.fAlpha);
	pServerDE->GetPropReal("PS2MinLifetime", &wPS2.fMinLifetime);
	pServerDE->GetPropReal("PS2MaxLifetime", &wPS2.fMaxLifetime);
	pServerDE->GetPropReal("PS2AddDelay", &wPS2.fAddDelay);
	pServerDE->GetPropReal("PS2Gravity", &wPS2.fGravity);
	pServerDE->GetPropLongInt("PS2RampUpType", &tempLong);
	wPS2.nRampUpType = (DDWORD)tempLong;
	pServerDE->GetPropLongInt("PS2RampDownType", &tempLong);
	wPS2.nRampDownType = (DDWORD)tempLong;
	pServerDE->GetPropBool("PS2Align", &wPS2.bAlign);
	pServerDE->GetPropString("PS2ParticleFile", szString, MAX_CS_FILENAME_LEN);
	wPS2.hstrParticle = pServerDE->CreateString(szString);

	pServerDE->GetPropReal("SoundRadius", &fSoundRadius);
	pServerDE->GetPropString("RampUpSound", szString, MAX_CS_FILENAME_LEN);
	hstrSound1 = pServerDE->CreateString(szString);
	pServerDE->GetPropString("LoopSound", szString, MAX_CS_FILENAME_LEN);
	hstrSound2 = pServerDE->CreateString(szString);
	pServerDE->GetPropString("RampDownSound", szString, MAX_CS_FILENAME_LEN);
	hstrSound3 = pServerDE->CreateString(szString);
	return;
}

// ----------------------------------------------------------------------- //

void CClientWarpGateSFX::HandleTrigger(HOBJECT hSender, HMESSAGEREAD hRead)
{
	HSTRING hMsg = g_pServerDE->ReadFromMessageHString(hRead);
	char *pszMessage = g_pServerDE->GetStringData(hMsg);
	DDWORD dwUsrFlags = g_pServerDE->GetObjectUserFlags(m_hObject);

    if( _mbsncmp((const unsigned char*)pszMessage, (const unsigned char*)"TOGGLE", 6) == 0)
		bOn = !bOn;
    else if( _mbsncmp((const unsigned char*)pszMessage, (const unsigned char*)"ON", 2) == 0)
		bOn = DTRUE;
    else if( _mbsncmp((const unsigned char*)pszMessage, (const unsigned char*)"OFF", 3) == 0)
		bOn = DFALSE;
    
	g_pServerDE->SetObjectUserFlags(m_hObject, dwUsrFlags);
	g_pServerDE->FreeString(hMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWarpGateSFX::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CClientWarpGateSFX::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageFloat(hWrite, fRampUpTime);
	pServerDE->WriteToMessageFloat(hWrite, fRampDownTime);
	pServerDE->WriteToMessageByte(hWrite, bInitiallyOn);

	pServerDE->WriteToMessageFloat(hWrite, wSpr.fMinScale);
	pServerDE->WriteToMessageFloat(hWrite, wSpr.fMaxScale);
	pServerDE->WriteToMessageFloat(hWrite, wSpr.fAlpha);
	pServerDE->WriteToMessageDWord(hWrite, wSpr.nRampUpType);
	pServerDE->WriteToMessageDWord(hWrite, wSpr.nRampDownType);
	pServerDE->WriteToMessageByte(hWrite, wSpr.bAlign);
	pServerDE->WriteToMessageHString(hWrite, wSpr.hstrSprite);

	pServerDE->WriteToMessageFloat(hWrite, wPS1.fSystemRadius);
	pServerDE->WriteToMessageFloat(hWrite, wPS1.fPosRadius);
	pServerDE->WriteToMessageVector(hWrite, &wPS1.vOffset);
	pServerDE->WriteToMessageVector(hWrite, &wPS1.vRotations);

	pServerDE->WriteToMessageFloat(hWrite, wPS1.fMinVelocity);
	pServerDE->WriteToMessageFloat(hWrite, wPS1.fMaxVelocity);
	pServerDE->WriteToMessageDWord(hWrite, wPS1.nNumParticles);
	pServerDE->WriteToMessageDWord(hWrite, wPS1.nEmitType);
	pServerDE->WriteToMessageVector(hWrite, &wPS1.vMinColor);
	pServerDE->WriteToMessageVector(hWrite, &wPS1.vMaxColor);
	pServerDE->WriteToMessageFloat(hWrite, wPS1.fAlpha);
	pServerDE->WriteToMessageFloat(hWrite, wPS1.fMinLifetime);
	pServerDE->WriteToMessageFloat(hWrite, wPS1.fMaxLifetime);
	pServerDE->WriteToMessageFloat(hWrite, wPS1.fAddDelay);
	pServerDE->WriteToMessageFloat(hWrite, wPS1.fGravity);
	pServerDE->WriteToMessageDWord(hWrite, wPS1.nRampUpType);
	pServerDE->WriteToMessageDWord(hWrite, wPS1.nRampDownType);
	pServerDE->WriteToMessageByte(hWrite, wPS1.bAlign);
	pServerDE->WriteToMessageHString(hWrite, wPS1.hstrParticle);

	pServerDE->WriteToMessageFloat(hWrite, wPS2.fSystemRadius);
	pServerDE->WriteToMessageFloat(hWrite, wPS2.fPosRadius);
	pServerDE->WriteToMessageVector(hWrite, &wPS2.vOffset);
	pServerDE->WriteToMessageVector(hWrite, &wPS2.vRotations);

	pServerDE->WriteToMessageFloat(hWrite, wPS2.fMinVelocity);
	pServerDE->WriteToMessageFloat(hWrite, wPS2.fMaxVelocity);
	pServerDE->WriteToMessageDWord(hWrite, wPS2.nNumParticles);
	pServerDE->WriteToMessageDWord(hWrite, wPS2.nEmitType);
	pServerDE->WriteToMessageVector(hWrite, &wPS2.vMinColor);
	pServerDE->WriteToMessageVector(hWrite, &wPS2.vMaxColor);
	pServerDE->WriteToMessageFloat(hWrite, wPS2.fAlpha);
	pServerDE->WriteToMessageFloat(hWrite, wPS2.fMinLifetime);
	pServerDE->WriteToMessageFloat(hWrite, wPS2.fMaxLifetime);
	pServerDE->WriteToMessageFloat(hWrite, wPS2.fAddDelay);
	pServerDE->WriteToMessageFloat(hWrite, wPS2.fGravity);
	pServerDE->WriteToMessageDWord(hWrite, wPS2.nRampUpType);
	pServerDE->WriteToMessageDWord(hWrite, wPS2.nRampDownType);
	pServerDE->WriteToMessageByte(hWrite, wPS2.bAlign);
	pServerDE->WriteToMessageHString(hWrite, wPS2.hstrParticle);

	pServerDE->WriteToMessageFloat(hWrite, fSoundRadius);
	pServerDE->WriteToMessageHString(hWrite, hstrSound1);
	pServerDE->WriteToMessageHString(hWrite, hstrSound2);
	pServerDE->WriteToMessageHString(hWrite, hstrSound3);

	pServerDE->WriteToMessageByte(hWrite, bOn);
	pServerDE->WriteToMessageByte(hWrite, bFirstUpdate);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWarpGateSFX::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CClientWarpGateSFX::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hRead) return;

	fRampUpTime			= pServerDE->ReadFromMessageFloat(hRead);
	fRampDownTime		= pServerDE->ReadFromMessageFloat(hRead);
	bInitiallyOn		= pServerDE->ReadFromMessageByte(hRead);

	wSpr.fMinScale		= pServerDE->ReadFromMessageFloat(hRead);
	wSpr.fMaxScale		= pServerDE->ReadFromMessageFloat(hRead);
	wSpr.fAlpha			= pServerDE->ReadFromMessageFloat(hRead);
	wSpr.nRampUpType	= pServerDE->ReadFromMessageDWord(hRead);
	wSpr.nRampDownType	= pServerDE->ReadFromMessageDWord(hRead);
	wSpr.bAlign			= pServerDE->ReadFromMessageByte(hRead);
	wSpr.hstrSprite		= pServerDE->ReadFromMessageHString(hRead);

	wPS1.fSystemRadius	= pServerDE->ReadFromMessageFloat(hRead);
	wPS1.fPosRadius		= pServerDE->ReadFromMessageFloat(hRead);
	pServerDE->ReadFromMessageVector(hRead, &wPS1.vOffset);
	pServerDE->ReadFromMessageVector(hRead, &wPS1.vRotations);

	wPS1.fMinVelocity	= pServerDE->ReadFromMessageFloat(hRead);
	wPS1.fMaxVelocity	= pServerDE->ReadFromMessageFloat(hRead);
	wPS1.nNumParticles	= pServerDE->ReadFromMessageDWord(hRead);
	wPS1.nEmitType		= pServerDE->ReadFromMessageDWord(hRead);
	pServerDE->ReadFromMessageVector(hRead, &wPS1.vMinColor);
	pServerDE->ReadFromMessageVector(hRead, &wPS1.vMaxColor);
	wPS1.fAlpha			= pServerDE->ReadFromMessageFloat(hRead);
	wPS1.fMinLifetime	= pServerDE->ReadFromMessageFloat(hRead);
	wPS1.fMaxLifetime	= pServerDE->ReadFromMessageFloat(hRead);
	wPS1.fAddDelay		= pServerDE->ReadFromMessageFloat(hRead);
	wPS1.fGravity		= pServerDE->ReadFromMessageFloat(hRead);
	wPS1.nRampUpType	= pServerDE->ReadFromMessageDWord(hRead);
	wPS1.nRampDownType	= pServerDE->ReadFromMessageDWord(hRead);
	wPS1.bAlign			= pServerDE->ReadFromMessageByte(hRead);
	wPS1.hstrParticle	= pServerDE->ReadFromMessageHString(hRead);

	wPS2.fSystemRadius	= pServerDE->ReadFromMessageFloat(hRead);
	wPS2.fPosRadius		= pServerDE->ReadFromMessageFloat(hRead);
	pServerDE->ReadFromMessageVector(hRead, &wPS2.vOffset);
	pServerDE->ReadFromMessageVector(hRead, &wPS2.vRotations);

	wPS2.fMinVelocity	= pServerDE->ReadFromMessageFloat(hRead);
	wPS2.fMaxVelocity	= pServerDE->ReadFromMessageFloat(hRead);
	wPS2.nNumParticles	= pServerDE->ReadFromMessageDWord(hRead);
	wPS2.nEmitType		= pServerDE->ReadFromMessageDWord(hRead);
	pServerDE->ReadFromMessageVector(hRead, &wPS2.vMinColor);
	pServerDE->ReadFromMessageVector(hRead, &wPS2.vMaxColor);
	wPS2.fAlpha			= pServerDE->ReadFromMessageFloat(hRead);
	wPS2.fMinLifetime	= pServerDE->ReadFromMessageFloat(hRead);
	wPS2.fMaxLifetime	= pServerDE->ReadFromMessageFloat(hRead);
	wPS2.fAddDelay		= pServerDE->ReadFromMessageFloat(hRead);
	wPS2.fGravity		= pServerDE->ReadFromMessageFloat(hRead);
	wPS2.nRampUpType	= pServerDE->ReadFromMessageDWord(hRead);
	wPS2.nRampDownType	= pServerDE->ReadFromMessageDWord(hRead);
	wPS2.bAlign			= pServerDE->ReadFromMessageByte(hRead);
	wPS2.hstrParticle	= pServerDE->ReadFromMessageHString(hRead);

	fSoundRadius		= pServerDE->ReadFromMessageFloat(hRead);
	hstrSound1			= pServerDE->ReadFromMessageHString(hRead);
	hstrSound2			= pServerDE->ReadFromMessageHString(hRead);
	hstrSound3			= pServerDE->ReadFromMessageHString(hRead);

	bOn					= pServerDE->ReadFromMessageByte(hRead);
	bFirstUpdate		= pServerDE->ReadFromMessageByte(hRead);
}
