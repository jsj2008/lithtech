// ----------------------------------------------------------------------- //
//
// MODULE  : ClientExplosionSFX.cpp
//
// PURPOSE : CClientExplosionSFX - Definition
//
// CREATED : 7-1-98
//
// ----------------------------------------------------------------------- //

#include "ClientExplosionSFX.h"
#include "ObjectUtilities.h"
#include "cpp_server_de.h"
#include "SFXMsgIds.h"
#include "ClientServerShared.h"
#include "SoundTypes.h"


BEGIN_CLASS(CClientExplosionSFX)
	ADD_LONGINTPROP(ExplosionType, 0)
	ADD_REALPROP(Damage, 50.0f)
	ADD_REALPROP_FLAG(DamageRadius, 200.0f, PF_RADIUS)
	ADD_VECTORPROP_VAL(Direction, 0.0f, 1.0f, 0.0f)
	ADD_STRINGPROP(WaveFile, "Sounds\\exp_tnt.wav")
END_CLASS_DEFAULT(CClientExplosionSFX, CClientSFX, NULL, NULL)

// ----------------------------------------------------------------------- //

void CClientExplosionSFX::Setup(DVector *vPos, DVector *vNormal, DFLOAT fOffset)
{
	VEC_COPY(m_vPos, *vPos);
	VEC_COPY(m_vNormal, *vNormal);
	m_fOffset = fOffset;

	if(m_fOffset)
	{
		DVector	temp;
		VEC_NORM(m_vNormal);
		VEC_MULSCALAR(temp, m_vNormal, m_fOffset);
		VEC_ADD(m_vPos, m_vPos, temp);
	}
}

// ----------------------------------------------------------------------- //

void CClientExplosionSFX::SetupModel(DVector *vScale1, DVector *vScale2, DFLOAT fDuration, DFLOAT fInitAlpha,
					DBYTE bWaveForm, DBYTE bFade, DBOOL bRandRot, char *szModel, char *szSkin)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	HSTRING		hszModel, hszModelSkin;

	if(szModel)		hszModel = pServerDE->CreateString(szModel);
		else		hszModel = pServerDE->CreateString("Models\\Explosions\\exp_sphere.abc");

	if(szSkin)		hszModelSkin = pServerDE->CreateString(szSkin);
		else		hszModelSkin = pServerDE->CreateString("Skins\\Explosions\\Explosion_1.dtx");

	// Tell the clients about the Explosion...
	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&m_vPos);
	pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSION_ID);
//	pServerDE->WriteToMessageByte(hMessage, DFALSE);

	pServerDE->WriteToMessageVector(hMessage, &m_vPos);
	pServerDE->WriteToMessageVector(hMessage, &m_vNormal);

	pServerDE->WriteToMessageVector(hMessage, vScale1);
	pServerDE->WriteToMessageVector(hMessage, vScale2);
	pServerDE->WriteToMessageFloat(hMessage, fDuration);
	pServerDE->WriteToMessageFloat(hMessage, fInitAlpha);
	pServerDE->WriteToMessageByte(hMessage, bWaveForm);
	pServerDE->WriteToMessageByte(hMessage, bFade);
	pServerDE->WriteToMessageByte(hMessage, bRandRot);
	pServerDE->WriteToMessageHString(hMessage, hszModel);
	pServerDE->WriteToMessageHString(hMessage, hszModelSkin);

	pServerDE->EndMessage(hMessage);

	pServerDE->FreeString( hszModel );
	pServerDE->FreeString( hszModelSkin );

}

// ----------------------------------------------------------------------- //

void CClientExplosionSFX::SetupSprite(DVector *vScale1, DVector *vScale2, DFLOAT fDuration,
					DFLOAT fInitAlpha, DBYTE bWaveForm, DBYTE bFade, DBOOL bAlign, char *szSprite)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	HSTRING		hszSprite;

	if(szSprite)	hszSprite = pServerDE->CreateString(szSprite);
		else		hszSprite = pServerDE->CreateString("Sprites\\Explode128.spr");

	// Tell the clients about the Explosion...
	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&m_vPos);
	pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONSPRITE_ID);
//	pServerDE->WriteToMessageByte(hMessage, DFALSE);

	pServerDE->WriteToMessageVector(hMessage, &m_vPos);
	pServerDE->WriteToMessageVector(hMessage, &m_vNormal);

	pServerDE->WriteToMessageVector(hMessage, vScale1);
	pServerDE->WriteToMessageVector(hMessage, vScale2);
	pServerDE->WriteToMessageFloat(hMessage, fDuration);
	pServerDE->WriteToMessageFloat(hMessage, fInitAlpha);
	pServerDE->WriteToMessageByte(hMessage, bWaveForm);
	pServerDE->WriteToMessageByte(hMessage, bFade);
	pServerDE->WriteToMessageByte(hMessage, bAlign);
	pServerDE->WriteToMessageHString(hMessage, hszSprite);

	pServerDE->EndMessage(hMessage);

	pServerDE->FreeString( hszSprite );

}

// ----------------------------------------------------------------------- //

void CClientExplosionSFX::SetupRing(DVector *vColor, DFLOAT fRadius, DFLOAT fPosRadius,
					DFLOAT fVelocity, DFLOAT fGravity, DDWORD nParticles, DFLOAT fDuration,
					DFLOAT fInitAlpha, DFLOAT fDelay, DBYTE bFade, DBOOL bAlign, char *szParticle)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	HSTRING		hszRingParticle;

	if(szParticle)	hszRingParticle = pServerDE->CreateString(szParticle);
		else		hszRingParticle = pServerDE->CreateString("SpriteTextures\\smoke64_2.dtx");

	// Tell the clients about the Explosion...
	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&m_vPos);
	pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONRING_ID);
//	pServerDE->WriteToMessageByte(hMessage, DFALSE);

	pServerDE->WriteToMessageVector(hMessage, &m_vPos);
	pServerDE->WriteToMessageVector(hMessage, &m_vNormal);

	pServerDE->WriteToMessageVector(hMessage, vColor);
	pServerDE->WriteToMessageFloat(hMessage, fRadius);
	pServerDE->WriteToMessageFloat(hMessage, fPosRadius);
	pServerDE->WriteToMessageFloat(hMessage, fVelocity);
	pServerDE->WriteToMessageFloat(hMessage, fGravity);
	pServerDE->WriteToMessageDWord(hMessage, nParticles);
	pServerDE->WriteToMessageFloat(hMessage, fDuration);
	pServerDE->WriteToMessageFloat(hMessage, fInitAlpha);
	pServerDE->WriteToMessageFloat(hMessage, fDelay);
	pServerDE->WriteToMessageByte(hMessage, bFade);
	pServerDE->WriteToMessageByte(hMessage, bAlign);
	pServerDE->WriteToMessageHString(hMessage, hszRingParticle);

	pServerDE->EndMessage(hMessage);

	pServerDE->FreeString( hszRingParticle );
}

// ----------------------------------------------------------------------- //

void CClientExplosionSFX::SetupWave(DVector *vScale1, DVector *vScale2, DFLOAT fDuration, DFLOAT fInitAlpha,
					DFLOAT fDelay, DBYTE bWaveForm, DBYTE bFade, DBOOL bAlign, char *szWave)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	HSTRING		hszWave;

	if(szWave)		hszWave = pServerDE->CreateString(szWave);
		else		hszWave = pServerDE->CreateString("Sprites\\Shockring.spr");

	// Tell the clients about the Explosion...
	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&m_vPos);
	pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONWAVE_ID);
//	pServerDE->WriteToMessageByte(hMessage, DFALSE);

	pServerDE->WriteToMessageVector(hMessage, &m_vPos);
	pServerDE->WriteToMessageVector(hMessage, &m_vNormal);

	pServerDE->WriteToMessageVector(hMessage, vScale1);
	pServerDE->WriteToMessageVector(hMessage, vScale2);
	pServerDE->WriteToMessageFloat(hMessage, fDuration);
	pServerDE->WriteToMessageFloat(hMessage, fInitAlpha);
	pServerDE->WriteToMessageFloat(hMessage, fDelay);
	pServerDE->WriteToMessageByte(hMessage, bWaveForm);
	pServerDE->WriteToMessageByte(hMessage, bFade);
	pServerDE->WriteToMessageByte(hMessage, bAlign);
	pServerDE->WriteToMessageHString(hMessage, hszWave);

	pServerDE->EndMessage(hMessage);

	pServerDE->FreeString( hszWave );
}

// ----------------------------------------------------------------------- //

void CClientExplosionSFX::SetupLight(DVector *vColor1, DVector *vColor2, DFLOAT fDuration,
				DFLOAT fDelay, DFLOAT fRadius1, DFLOAT fRadius2)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	// Tell the clients about the light...
	HMESSAGEWRITE hMessage = pServerDE->StartSpecialEffectMessage(this);
	pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONLIGHT_ID);
//pServerDE->WriteToMessageByte(hMessage, DFALSE);

	pServerDE->WriteToMessageVector(hMessage, &m_vPos);
	pServerDE->WriteToMessageVector(hMessage, vColor1);
	pServerDE->WriteToMessageVector(hMessage, vColor2);
	pServerDE->WriteToMessageFloat(hMessage, fDuration);
	pServerDE->WriteToMessageFloat(hMessage, fDelay);
	pServerDE->WriteToMessageFloat(hMessage, fRadius1);
	pServerDE->WriteToMessageFloat(hMessage, fRadius2);

	pServerDE->EndMessage(hMessage);
}

// ----------------------------------------------------------------------- //

void CClientExplosionSFX::SetupFrag(DVector *vScale, DVector *vRotateMax, DFLOAT fSpread,
				DFLOAT fDuration, DFLOAT fVelocity, DFLOAT fBounceMod, DFLOAT fGravity,
				DFLOAT fFadeTime, DBOOL bRandDir, char *szModel, char *szSkin)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	HSTRING		hszModel, hszModelSkin;

	if(szModel)		hszModel = pServerDE->CreateString(szModel);
		else		hszModel = pServerDE->CreateString("Models\\Explosions\\exp_sphere.abc");

	if(szSkin)		hszModelSkin = pServerDE->CreateString(szSkin);
		else		hszModelSkin = pServerDE->CreateString("Skins\\Explosions\\Explosion_1.dtx");

	// Tell the clients about the light...
	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&m_vPos);
	pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFRAG_ID);
//	pServerDE->WriteToMessageByte(hMessage, DFALSE);

	pServerDE->WriteToMessageVector(hMessage, &m_vPos);
	pServerDE->WriteToMessageVector(hMessage, &m_vNormal);
	pServerDE->WriteToMessageVector(hMessage, vScale);
	pServerDE->WriteToMessageVector(hMessage, vRotateMax);
	pServerDE->WriteToMessageFloat(hMessage, fSpread);
	pServerDE->WriteToMessageFloat(hMessage, fDuration);
	pServerDE->WriteToMessageFloat(hMessage, fVelocity);
	pServerDE->WriteToMessageFloat(hMessage, fBounceMod);
	pServerDE->WriteToMessageFloat(hMessage, fGravity);
	pServerDE->WriteToMessageFloat(hMessage, fFadeTime);
	pServerDE->WriteToMessageByte(hMessage, bRandDir);
	pServerDE->WriteToMessageHString(hMessage, hszModel);
	pServerDE->WriteToMessageHString(hMessage, hszModelSkin);

	pServerDE->EndMessage(hMessage);

	pServerDE->FreeString( hszModel );
	pServerDE->FreeString( hszModelSkin );
}

// ----------------------------------------------------------------------- //

void CClientExplosionSFX::SetupFX(DVector *vPos, DVector *vNormal, DDWORD nType)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&m_vPos);
	pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

	pServerDE->WriteToMessageVector(hMessage, vPos);
	pServerDE->WriteToMessageVector(hMessage, vNormal);
	pServerDE->WriteToMessageDWord(hMessage, nType);

	pServerDE->EndMessage(hMessage);
}

// ----------------------------------------------------------------------- //

void CClientExplosionSFX::SetupFX()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&m_vPos);
	pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

	pServerDE->WriteToMessageVector(hMessage, &m_vPos);
	pServerDE->WriteToMessageVector(hMessage, &m_vNormal);
	pServerDE->WriteToMessageDWord(hMessage, m_nType);

	pServerDE->EndMessage(hMessage);

	DamageObjectsInRadius(m_hObject, NULL, m_vPos, m_fDamageRadius, m_fDamage);
	if (m_hstrSound)
		PlaySoundFromPos(&m_vPos, pServerDE->GetStringData(m_hstrSound), 1000.0f, SOUNDPRIORITY_MISC_HIGH );
}

// ----------------------------------------------------------------------- //

DDWORD CClientExplosionSFX::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
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

DDWORD CClientExplosionSFX::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return 0;

	switch(messageID)
	{
		case MID_PRECREATE:
		{
			ReadProp((ObjectCreateStruct*)pData);
			break;
		}

		case MID_INITIALUPDATE:
		{
			pServerDE->SetNextUpdate(m_hObject, 0.1f);
			pServerDE->GetObjectPos(m_hObject, &m_vPos);
			DDWORD dwUserFlags = pServerDE->GetObjectUserFlags(m_hObject);
			pServerDE->SetObjectUserFlags(m_hObject, dwUserFlags | USRFLG_SAVEABLE);
			break;
		}

		case MID_UPDATE:
		{
			pServerDE->SetNextUpdate(m_hObject, 0.1f);
			if(m_bTriggered)	pServerDE->RemoveObject(m_hObject);
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

void CClientExplosionSFX::ReadProp(ObjectCreateStruct *pData)
{
	CServerDE* pServerDE = GetServerDE();
	if(!pServerDE || !pData) return;

	long		temp;

	pServerDE->GetPropLongInt("ExplosionType", &temp);
	m_nType = (DDWORD)temp;
	pServerDE->GetPropReal("Damage", &m_fDamage);
	pServerDE->GetPropReal("DamageRadius", &m_fDamageRadius);
	pServerDE->GetPropVector("Direction", &m_vNormal);

	char buf[MAX_CS_FILENAME_LEN];

	buf[0] = '\0';
	pServerDE->GetPropString("WaveFile", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hstrSound = pServerDE->CreateString(buf);
}

// ----------------------------------------------------------------------- //

void CClientExplosionSFX::HandleTrigger(HOBJECT hSender, HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = GetServerDE();
	if(!pServerDE) return;

	HSTRING hMsg = pServerDE->ReadFromMessageHString(hRead);
	if(!hMsg) return;

	HSTRING hstr = pServerDE->CreateString("TRIGGER");
	if(pServerDE->CompareStringsUpper(hMsg, hstr))
	{
		SetupFX();
		m_bTriggered = 1;
	}
	pServerDE->FreeString(hMsg);
	pServerDE->FreeString(hstr);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientExplosionSFX::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CClientExplosionSFX::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageVector(hWrite, &m_vPos);
	pServerDE->WriteToMessageVector(hWrite, &m_vNormal);
	pServerDE->WriteToMessageFloat(hWrite, m_fOffset);
	pServerDE->WriteToMessageFloat(hWrite, m_fDamage);
	pServerDE->WriteToMessageFloat(hWrite, m_fDamageRadius);
	pServerDE->WriteToMessageDWord(hWrite, m_nType);
	pServerDE->WriteToMessageHString(hWrite, m_hstrSound);
	pServerDE->WriteToMessageByte(hWrite, m_bTriggered);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientExplosionSFX::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CClientExplosionSFX::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hRead) return;

	pServerDE->ReadFromMessageVector(hRead, &m_vPos);
	pServerDE->ReadFromMessageVector(hRead, &m_vNormal);
	m_fOffset		= pServerDE->ReadFromMessageFloat(hRead);
	m_fDamage		= pServerDE->ReadFromMessageFloat(hRead);
	m_fDamageRadius	= pServerDE->ReadFromMessageFloat(hRead);
	m_nType			= pServerDE->ReadFromMessageDWord(hRead);
	m_hstrSound		= pServerDE->ReadFromMessageHString(hRead);
	m_bTriggered	= pServerDE->ReadFromMessageByte(hRead);
}

