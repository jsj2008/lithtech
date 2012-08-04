// ----------------------------------------------------------------------- //
//
// MODULE  : ClientLightningSFX.cpp
//
// PURPOSE : CClientLightningSFX - Definition
//
// CREATED : 7-1-98
//
// ----------------------------------------------------------------------- //

#include "ClientLightningSFX.h"
#include "ClientServerShared.h"
#include "ObjectUtilities.h"
#include "cpp_server_de.h"
#include "SFXMsgIds.h"
#include "VolumeBrushTypes.h"
#include <mbstring.h>
#include "SoundTypes.h"

BEGIN_CLASS(CClientLightningSFX)
	ADD_LONGINTPROP(LightningShape, 2)
	ADD_LONGINTPROP(LightningForm, 1)
	ADD_LONGINTPROP(LightningType, 2)
	ADD_REALPROP(Damage, 500.0f)
	ADD_REALPROP(DamageRadius, 300.0f)
	ADD_STRINGPROP(Sound, "Sounds\\Thunder.wav")
	ADD_REALPROP_FLAG(SoundRadius, 1000.0f, PF_RADIUS)
END_CLASS_DEFAULT(CClientLightningSFX, CClientSFX, NULL, NULL)

// ----------------------------------------------------------------------- //

void CClientLightningSFX::CreateBolt()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	pServerDE->GetObjectPos(m_hObject, &(cl.vSource));

	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&(cl.vSource));
	pServerDE->WriteToMessageByte(hMessage, SFX_LIGHTNING_ID);

	pServerDE->WriteToMessageVector(hMessage, &(cl.vSource));
	pServerDE->WriteToMessageVector(hMessage, &(cl.vDest));
	pServerDE->WriteToMessageByte(hMessage, cl.nShape);
	pServerDE->WriteToMessageByte(hMessage, cl.nForm);
	pServerDE->WriteToMessageByte(hMessage, cl.nType);

	pServerDE->EndMessage(hMessage);
}

// ----------------------------------------------------------------------- //

void CClientLightningSFX::Fire(HOBJECT hSender)
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

	VEC_MULSCALAR(vDist, vFire, 5000.0f)

	VEC_COPY(iq.m_To, iq.m_From);
	VEC_ADD(iq.m_To, iq.m_To, vDist);

	iq.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	iq.m_FilterFn = LiquidFilterFn;
	iq.m_pUserData = NULL;	

	if (g_pServerDE->IntersectSegment(&iq, &ii))
		{ VEC_COPY(cl.vDest, ii.m_Point); }
	else
		{ VEC_COPY(cl.vDest, iq.m_To); }

	CreateBolt();

	if(fDamageRadius)
		DamageObjectsInRadius(hSender, this, cl.vDest, fDamageRadius, fDamage, DAMAGE_TYPE_NORMAL);

	if(hstrSound)
		PlaySoundFromPos(&(iq.m_From), g_pServerDE->GetStringData(hstrSound), fSoundRadius, SOUNDPRIORITY_MISC_HIGH);
}

// ----------------------------------------------------------------------- //

DDWORD CClientLightningSFX::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
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

DDWORD CClientLightningSFX::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
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
		    if(!pServerDE) pServerDE->RemoveObject(m_hObject);
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

void CClientLightningSFX::ReadProp(ObjectCreateStruct *pData)
{
	CServerDE* pServerDE = GetServerDE();
	if(!pServerDE || !pData) return;

	char		szString[MAX_CS_FILENAME_LEN];
	long		tempLong;

	pServerDE->GetPropLongInt("LightningShape", &tempLong);
	cl.nShape = (DBYTE)tempLong;
	pServerDE->GetPropLongInt("LightningForm", &tempLong);
	cl.nForm = (DBYTE)tempLong;
	pServerDE->GetPropLongInt("LightningType", &tempLong);
	cl.nType = (DBYTE)tempLong;

	pServerDE->GetPropReal("Damage", &fDamage);
	pServerDE->GetPropReal("DamageRadius", &fDamageRadius);
	pServerDE->GetPropString("Sound", szString, MAX_CS_FILENAME_LEN);
	hstrSound = pServerDE->CreateString(szString);
	pServerDE->GetPropReal("SoundRadius", &fSoundRadius);
	return;
}

// ----------------------------------------------------------------------- //

void CClientLightningSFX::HandleTrigger(HOBJECT hSender, HMESSAGEREAD hRead)
{
	HSTRING hMsg = g_pServerDE->ReadFromMessageHString(hRead);
	char *pszMessage = g_pServerDE->GetStringData(hMsg);

	if(_mbsncmp((const unsigned char*)pszMessage, (const unsigned char*)"FIRE", 4) == 0)
		Fire(hSender);

	g_pServerDE->FreeString(hMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientLightningSFX::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CClientLightningSFX::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageVector(hWrite, &cl.vSource);
	pServerDE->WriteToMessageVector(hWrite, &cl.vDest);
	pServerDE->WriteToMessageByte(hWrite, cl.nShape);
	pServerDE->WriteToMessageByte(hWrite, cl.nForm);
	pServerDE->WriteToMessageByte(hWrite, cl.nType);

	pServerDE->WriteToMessageFloat(hWrite, fDamage);
	pServerDE->WriteToMessageFloat(hWrite, fDamageRadius);

	pServerDE->WriteToMessageHString(hWrite, hstrSound);
	pServerDE->WriteToMessageFloat(hWrite, fSoundRadius);

	pServerDE->WriteToMessageByte(hWrite, bTriggered);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientLightningSFX::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CClientLightningSFX::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hRead) return;

	pServerDE->ReadFromMessageVector(hRead, &cl.vSource);
	pServerDE->ReadFromMessageVector(hRead, &cl.vDest);
	cl.nShape		= pServerDE->ReadFromMessageByte(hRead );
	cl.nForm		= pServerDE->ReadFromMessageByte(hRead );
	cl.nType		= pServerDE->ReadFromMessageByte(hRead );

	fDamage			= pServerDE->ReadFromMessageFloat(hRead );
	fDamageRadius	= pServerDE->ReadFromMessageFloat(hRead );

	hstrSound		= pServerDE->ReadFromMessageHString(hRead );
	fSoundRadius	= pServerDE->ReadFromMessageFloat(hRead );

	bTriggered		= pServerDE->ReadFromMessageByte(hRead );
}

