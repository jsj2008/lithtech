// ----------------------------------------------------------------------- //
//
// MODULE  : ClientParticleStreamSFX.cpp
//
// PURPOSE : CClientParticleStreamSFX - Definition
//
// CREATED : 7-1-98
//
// ----------------------------------------------------------------------- //

#include "ClientParticleStreamSFX.h"
#include "ClientServerShared.h"
#include "ObjectUtilities.h"
#include "cpp_server_de.h"
#include "SFXMsgIds.h"
#include <mbstring.h>


BEGIN_CLASS(CClientParticleStreamSFX)
	ADD_REALPROP(SystemRadius, 1000.0f)
	ADD_REALPROP_FLAG(PositionRadius, 1.0f, PF_RADIUS)
	ADD_REALPROP(MinVelocity, 0.0f)
	ADD_REALPROP(MaxVelocity, 25.0f)
	ADD_LONGINTPROP(NumParticles, 1)
	ADD_REALPROP(VelSpread, 0.0f)
    ADD_COLORPROP(MinColor, 255.0f, 255.0f, 255.0f)   
    ADD_COLORPROP(MaxColor, 255.0f, 255.0f, 255.0f)   
	ADD_REALPROP(SystemAlpha, 1.0f)
	ADD_REALPROP(MinLifetime, 1.0f)
	ADD_REALPROP(MaxLifetime, 2.0f)
	ADD_REALPROP(RampTime, 0.0f)
	ADD_REALPROP(AddDelay, 0.1f)
	ADD_REALPROP(Gravity, 15.0f)
	ADD_BOOLPROP(RampAmount, DFALSE)
	ADD_BOOLPROP(RampOffset, DFALSE)
	ADD_BOOLPROP(RampVelocity, DFALSE)
	ADD_BOOLPROP(RampLifetime, DFALSE)
	ADD_BOOLPROP(InitiallyOn, DFALSE)
	ADD_STRINGPROP(ParticleFile, "SpriteTextures\\drop32_1.dtx")
	ADD_REALPROP(SoundRadius, 200.0f)
	ADD_STRINGPROP(RampUpSound, "")
	ADD_STRINGPROP(LoopSound, "")
	ADD_STRINGPROP(RampDownSound, "")
END_CLASS_DEFAULT(CClientParticleStreamSFX, CClientSFX, NULL, NULL)

// ----------------------------------------------------------------------- //

void CClientParticleStreamSFX::SetupFX()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	HMESSAGEWRITE hMessage = pServerDE->StartSpecialEffectMessage(this);
	pServerDE->WriteToMessageByte(hMessage, SFX_PARTICLESTREAM_ID);

	pServerDE->WriteToMessageFloat(hMessage, fRadius);
	pServerDE->WriteToMessageFloat(hMessage, fPosRadius);
	pServerDE->WriteToMessageFloat(hMessage, fMinVel);
	pServerDE->WriteToMessageFloat(hMessage, fMaxVel);
	pServerDE->WriteToMessageDWord(hMessage, nNumParticles);
	pServerDE->WriteToMessageFloat(hMessage, fSpread);
	pServerDE->WriteToMessageVector(hMessage, &vColor1);
	pServerDE->WriteToMessageVector(hMessage, &vColor2);
	pServerDE->WriteToMessageFloat(hMessage, fAlpha);
	pServerDE->WriteToMessageFloat(hMessage, fMinLife);
	pServerDE->WriteToMessageFloat(hMessage, fMaxLife);
	pServerDE->WriteToMessageFloat(hMessage, fRampTime);
	pServerDE->WriteToMessageFloat(hMessage, fDelay);
	pServerDE->WriteToMessageFloat(hMessage, fGravity);
	pServerDE->WriteToMessageByte(hMessage, bRampFlags);
	pServerDE->WriteToMessageByte(hMessage, bOn);
	pServerDE->WriteToMessageHString(hMessage, hstrTexture);
	pServerDE->WriteToMessageFloat(hMessage, fSoundRadius);
	pServerDE->WriteToMessageHString(hMessage, hstrSound1);
	pServerDE->WriteToMessageHString(hMessage, hstrSound2);
	pServerDE->WriteToMessageHString(hMessage, hstrSound3);

	pServerDE->EndMessage(hMessage);
}

// ----------------------------------------------------------------------- //

DDWORD CClientParticleStreamSFX::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
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

DDWORD CClientParticleStreamSFX::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return 0;

	switch(messageID)
	{
		case MID_PRECREATE:
		{
			// Only read properties if object was created from a world file.
			if (fData == PRECREATE_WORLDFILE)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}
			break;
		}

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				pServerDE->SetNextUpdate(m_hObject, 0.0f);

				DDWORD dwFlags = g_pServerDE->GetObjectFlags(m_hObject);
				dwFlags &= ~FLAG_GRAVITY;
				dwFlags |= FLAG_FORCECLIENTUPDATE;
				g_pServerDE->SetObjectFlags(m_hObject, dwFlags);

				DDWORD dwUsrFlags = g_pServerDE->GetObjectUserFlags(m_hObject);

				dwUsrFlags |= USRFLG_SAVEABLE;

				if(bOn)		dwUsrFlags |= USRFLG_VISIBLE;
					else	dwUsrFlags &= ~USRFLG_VISIBLE;
				pServerDE->SetObjectUserFlags(m_hObject, dwUsrFlags);
 
				SetupFX();
			}

			CacheFiles();
			break;
		}
/* No updates necessary for this object, since it the flags can be set by the trigger fn.
   GK 8/20/98
		case MID_UPDATE:
		{
			DDWORD dwUsrFlags = g_pServerDE->GetObjectUserFlags(m_hObject);
			if(bOn)		dwUsrFlags |= USRFLG_VISIBLE;
				else	dwUsrFlags &= ~USRFLG_VISIBLE;
			pServerDE->SetObjectUserFlags(m_hObject, dwUsrFlags);

			pServerDE->SetNextUpdate(m_hObject, 0.1f);
		    if(!pServerDE) pServerDE->RemoveObject(m_hObject);
			break;
		}
*/
		case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData, (DDWORD)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData, (DDWORD)fData);
		}
		break;

		default : break;
	}

	return CClientSFX::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //

void CClientParticleStreamSFX::ReadProp(ObjectCreateStruct *pData)
{
	CServerDE* pServerDE = GetServerDE();
	if(!pServerDE || !pData) return;

	char		szString[MAX_CS_FILENAME_LEN];
	long		tempLong;
	DBOOL		temp;

	pServerDE->GetPropReal("SystemRadius", &fRadius);
	pServerDE->GetPropReal("PositionRadius", &fPosRadius);
	pServerDE->GetPropReal("MinVelocity", &fMinVel);
	pServerDE->GetPropReal("MaxVelocity", &fMaxVel);
	pServerDE->GetPropLongInt("NumParticles", &tempLong);
	nNumParticles = (DDWORD)tempLong;
	pServerDE->GetPropReal("VelSpread", &fSpread);
	pServerDE->GetPropColor("MinColor", &vColor1);
	pServerDE->GetPropColor("MaxColor", &vColor2);
	pServerDE->GetPropReal("SystemAlpha", &fAlpha);
	pServerDE->GetPropReal("MinLifetime", &fMinLife);
	pServerDE->GetPropReal("MaxLifetime", &fMaxLife);
	pServerDE->GetPropReal("RampTime", &fRampTime);
	pServerDE->GetPropReal("AddDelay", &fDelay);
	pServerDE->GetPropReal("Gravity", &fGravity);

	bRampFlags = 0;
	pServerDE->GetPropBool("RampAmount", &temp);
	if(temp)	bRampFlags |= 0x01;
	pServerDE->GetPropBool("RampOffset", &temp);
	if(temp)	bRampFlags |= 0x02;
	pServerDE->GetPropBool("RampVelocity", &temp);
	if(temp)	bRampFlags |= 0x04;
	pServerDE->GetPropBool("RampLifetime", &temp);
	if(temp)	bRampFlags |= 0x08;

	pServerDE->GetPropBool("InitiallyOn", &bOn);

	pServerDE->GetPropString("ParticleFile", szString, MAX_CS_FILENAME_LEN);
	hstrTexture = pServerDE->CreateString(szString);

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

void CClientParticleStreamSFX::HandleTrigger(HOBJECT hSender, HMESSAGEREAD hRead)
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
    
	if(bOn)		
		dwUsrFlags |= USRFLG_VISIBLE;
	else	
		dwUsrFlags &= ~USRFLG_VISIBLE;

	g_pServerDE->SetObjectUserFlags(m_hObject, dwUsrFlags);

	g_pServerDE->FreeString(hMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientParticleStreamSFX::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CClientParticleStreamSFX::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	// Only need to save the data that changes (all the data in the
	// special fx message is saved/loaded for us)...

	pServerDE->WriteToMessageByte(hWrite, bOn);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientParticleStreamSFX::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CClientParticleStreamSFX::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	bOn = (DBOOL) pServerDE->ReadFromMessageByte(hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientParticleStreamSFX::CacheFiles
//
//	PURPOSE:	Cache resources used by the object
//
// ----------------------------------------------------------------------- //

void CClientParticleStreamSFX::CacheFiles()
{
	if (!g_pServerDE) return;

	// {MD 9/23/98}
	if(!(g_pServerDE->GetServerFlags() & SS_CACHING))
		return;

	char* pFile = DNULL;

	if (hstrSound1)
	{
		if (pFile = g_pServerDE->GetStringData(hstrSound1))
			 g_pServerDE->CacheFile(FT_SOUND ,pFile);
	}

	if (hstrSound2)
	{
		if (pFile = g_pServerDE->GetStringData(hstrSound2))
			 g_pServerDE->CacheFile(FT_SOUND ,pFile);
	}

	if (hstrSound3)
	{
		if (pFile = g_pServerDE->GetStringData(hstrSound3))
			 g_pServerDE->CacheFile(FT_SOUND ,pFile);
	}

	if (hstrTexture)
	{
		if (pFile = g_pServerDE->GetStringData(hstrTexture))
			 g_pServerDE->CacheFile(FT_TEXTURE ,pFile);
	}

}
