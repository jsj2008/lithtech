// ----------------------------------------------------------------------- //
//
// MODULE  : ClientLaserBeamSFX.cpp
//
// PURPOSE : CClientLaserBeamSFX - Definition
//
// CREATED : 7-1-98
//
// ----------------------------------------------------------------------- //

#include "ClientLaserBeamSFX.h"
#include "ClientServerShared.h"
#include "ObjectUtilities.h"
#include "cpp_server_de.h"
#include "SFXMsgIds.h"
#include "VolumeBrushTypes.h"
#include <mbstring.h>
#include "SoundTypes.h"

BEGIN_CLASS(CClientLaserBeamSFX)
	ADD_LONGINTPROP(BeamType, 0)
	ADD_REALPROP(Damage, 100.0f)
	ADD_REALPROP(DamageRadius, 100.0f)
	ADD_STRINGPROP(Sound, "Sounds\\Thunder.wav")
	ADD_REALPROP_FLAG(SoundRadius, 500.0f, PF_RADIUS)
END_CLASS_DEFAULT(CClientLaserBeamSFX, CClientSFX, NULL, NULL)

// ----------------------------------------------------------------------- //

void CClientLaserBeamSFX::CreateBeam()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&(clb.vSource));
	pServerDE->WriteToMessageByte(hMessage, SFX_LASERBEAM_ID);

	pServerDE->WriteToMessageVector(hMessage, &(clb.vSource));
	pServerDE->WriteToMessageVector(hMessage, &(clb.vDest));
	pServerDE->WriteToMessageByte(hMessage, clb.nType);

	pServerDE->EndMessage(hMessage);
}

// ----------------------------------------------------------------------- //

void CClientLaserBeamSFX::Fire(HOBJECT hSender)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	IntersectQuery	iq;
	IntersectInfo	ii;
	DRotation		rRot;
	DVector			vDist, vFire;

	g_pServerDE->GetObjectPos(m_hObject, &(iq.m_From));
	g_pServerDE->GetObjectRotation(m_hObject, &rRot);
	g_pServerDE->GetRotationVectors(&rRot, &(iq.m_To), &(iq.m_To), &vFire);

	VEC_MULSCALAR(vDist, vFire, 3000.0f)

	VEC_COPY(iq.m_To, iq.m_From);
	VEC_ADD(iq.m_To, iq.m_To, vDist);

	iq.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	iq.m_FilterFn = LiquidFilterFn;
	iq.m_pUserData = NULL;	

	if (g_pServerDE->IntersectSegment(&iq, &ii))
		{ VEC_COPY(vFire, ii.m_Point); }
	else
		{ VEC_COPY(vFire, iq.m_To); }

	DamageObjectsInRadius(hSender, this, vFire, fDamageRadius, fDamage, DAMAGE_TYPE_NORMAL);

	VEC_COPY(clb.vSource, iq.m_From);
	VEC_COPY(clb.vDest, vFire);
	CreateBeam();
}

// ----------------------------------------------------------------------- //

DDWORD CClientLaserBeamSFX::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
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

DDWORD CClientLaserBeamSFX::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
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
			DDWORD dwFlags = g_pServerDE->GetObjectFlags(m_hObject);
			dwFlags &= ~FLAG_GRAVITY;
			dwFlags |= FLAG_FORCECLIENTUPDATE;
			g_pServerDE->SetObjectFlags(m_hObject, dwFlags);
			DDWORD dwUserFlags = pServerDE->GetObjectUserFlags(m_hObject);
			pServerDE->SetObjectUserFlags(m_hObject, dwUserFlags | USRFLG_SAVEABLE);

			pServerDE->SetNextUpdate(m_hObject, 0.1f);
			break;
		}

		case MID_UPDATE:
		{
			if(!pServerDE)	pServerDE->RemoveObject(m_hObject);
			pServerDE->SetNextUpdate(m_hObject, 0.1f);
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

void CClientLaserBeamSFX::ReadProp(ObjectCreateStruct *pData)
{
	CServerDE* pServerDE = GetServerDE();
	if(!pServerDE || !pData) return;

	char		szString[MAX_CS_FILENAME_LEN];
	long		tempLong;

	pServerDE->GetPropLongInt("BeamType", &tempLong);
	clb.nType = (DBYTE)tempLong;

	pServerDE->GetPropReal("Damage", &fDamage);
	pServerDE->GetPropReal("DamageRadius", &fDamageRadius);
	pServerDE->GetPropString("Sound", szString, MAX_CS_FILENAME_LEN);
	hstrSound = pServerDE->CreateString(szString);
	pServerDE->GetPropReal("SoundRadius", &fSoundRadius);
	return;
}

// ----------------------------------------------------------------------- //

void CClientLaserBeamSFX::HandleTrigger(HOBJECT hSender, HMESSAGEREAD hRead)
{
	HSTRING hMsg = g_pServerDE->ReadFromMessageHString(hRead);
	char *pszMessage = g_pServerDE->GetStringData(hMsg);

	if((_mbsncmp((const unsigned char*)pszMessage, (const unsigned char*)"FIRE", 4) == 0))
	{
		Fire(hSender);

		if(hstrSound)
			PlaySoundFromObject(m_hObject, g_pServerDE->GetStringData(hstrSound), fSoundRadius, SOUNDPRIORITY_MISC_HIGH);
	}

	g_pServerDE->FreeString(hMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientLaserBeamSFX::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CClientLaserBeamSFX::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageVector(hWrite, &clb.vSource);
	pServerDE->WriteToMessageVector(hWrite, &clb.vDest);
	pServerDE->WriteToMessageByte(hWrite, clb.nType);

	pServerDE->WriteToMessageFloat(hWrite, fDamage);
	pServerDE->WriteToMessageFloat(hWrite, fDamageRadius);
	pServerDE->WriteToMessageHString(hWrite, hstrSound);
	pServerDE->WriteToMessageFloat(hWrite, fSoundRadius);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientLaserBeamSFX::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CClientLaserBeamSFX::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hRead) return;

	pServerDE->ReadFromMessageVector(hRead, &clb.vSource);
	pServerDE->ReadFromMessageVector(hRead, &clb.vDest);
	clb.nType		= pServerDE->ReadFromMessageByte(hRead);

	fDamage			= pServerDE->ReadFromMessageFloat(hRead);
	fDamageRadius	= pServerDE->ReadFromMessageFloat(hRead);
	hstrSound		= pServerDE->ReadFromMessageHString(hRead);
	fSoundRadius	= pServerDE->ReadFromMessageFloat(hRead);
}

