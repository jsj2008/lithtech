
#include "NagaCeilingDebris.h"
#include "ObjectUtilities.h"
#include "SFXMsgIds.h"
#include <stdio.h>
#include "SoundTypes.h"

BEGIN_CLASS(NagaCeilingDebris)
END_CLASS_DEFAULT_FLAGS(NagaCeilingDebris, CDestructableBrush, NULL, NULL, CF_HIDDEN)

NagaCeilingDebris::NagaCeilingDebris() : B2BaseClass(OT_MODEL)
{
	m_fStartTime = 0.0f;
	m_bFall = DFALSE;

	m_fXRot = 0.0f;
	m_fYRot = 0.0f;
	m_fZRot = 0.0f;
}

NagaCeilingDebris::~NagaCeilingDebris()
{

}

DDWORD NagaCeilingDebris::EngineMessageFn(DDWORD messageID, void *pData, float fData)
{
	DDWORD dwResult = B2BaseClass::EngineMessageFn(messageID,pData,fData);

	switch (messageID)
	{
		case MID_PRECREATE:
		{
			ObjectCreateStruct *ocStruct = (ObjectCreateStruct *)pData;

			int iTemp = g_pServerDE->IntRandom(6,9);

			sprintf(ocStruct->m_Filename,"models\\gibs\\stone\\gib%d.abc", iTemp);
			sprintf(ocStruct->m_SkinName,"skins\\gibs\\stone\\gib%d.dtx", iTemp);

			ocStruct->m_Flags = FLAG_VISIBLE | FLAG_GRAVITY | FLAG_SOLID | FLAG_RAYHIT;
			ocStruct->m_ObjectType = OT_MODEL;
			break;
		}

		case MID_INITIALUPDATE:
		{
			DVector vScale;

			DFLOAT fX, fY, fZ, rotVal = MATH_PI / 20.0f;

			fX = g_pServerDE->Random(4.0f, 6.0f);
			fY = g_pServerDE->Random(4.0f, 6.0f);
			fZ = g_pServerDE->Random(4.0f, 6.0f);

			VEC_SET(vScale, fX, fY, fZ);
			g_pServerDE->ScaleObject(m_hObject, &vScale);
			g_pServerDE->SetBlockingPriority(m_hObject, 100);
			g_pServerDE->SetForceIgnoreLimit(m_hObject, 0.0f);
			g_pServerDE->SetNextUpdate(m_hObject, 0.01f);
			g_pServerDE->SetObjectMass(m_hObject, 1000.0f);

			m_fXRot = g_pServerDE->Random(-rotVal, rotVal);
			m_fYRot = g_pServerDE->Random(-rotVal, rotVal);
			m_fZRot = g_pServerDE->Random(-rotVal, rotVal);

			m_fStartTime = g_pServerDE->GetTime();

			m_bFall = DTRUE;
			break;
		}

		case MID_UPDATE:
		{
			Update();
			break;
		}

	}

	return dwResult;
}

void NagaCeilingDebris::Update()
{
	if (m_fStartTime - g_pServerDE->GetTime() > 10.0f)
	{
		g_pServerDE->RemoveObject(m_hObject);
		return;
	}

	CollisionInfo collisionInfo;

	g_pServerDE->SetNextUpdate(m_hObject, 0.01f);
	g_pServerDE->GetStandingOn(m_hObject, &collisionInfo);
		
	// Rotate the object...
	DRotation	rRot;

	g_pServerDE->GetObjectRotation(m_hObject, &rRot);
	g_pServerDE->EulerRotateX(&rRot, m_fXRot);
	g_pServerDE->EulerRotateY(&rRot, m_fYRot);
	g_pServerDE->EulerRotateZ(&rRot, m_fZRot);
	g_pServerDE->SetObjectRotation(m_hObject, &rRot);

	if(collisionInfo.m_hObject) 
	{
		DVector vPoint;
		DVector vVector;

		VEC_SET(vVector, 0.0f, 1.0f, 0.0f);
		g_pServerDE->GetObjectPos(m_hObject, &vPoint);

		// Create the smaller rock fragments
		HMESSAGEWRITE hMessage = g_pServerDE->StartInstantSpecialEffectMessage(&vPoint);
		g_pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);
		g_pServerDE->WriteToMessageVector(hMessage, &vPoint);
		g_pServerDE->WriteToMessageVector(hMessage, &vVector);
		g_pServerDE->WriteToMessageDWord(hMessage, EXP_NAGA_STONE_CHUNK);
		g_pServerDE->EndMessage(hMessage);

		DamageObjectsInRadius(m_hObject, this, vPoint, 175.0f, 50.0f, DAMAGE_TYPE_NORMAL);
		PlaySoundFromPos(&vPoint, "Sounds\\Gibs\\Stone\\gib_impact4.wav", 750.0f, SOUNDPRIORITY_MISC_MEDIUM);

		g_pServerDE->RemoveObject(m_hObject);
		return;
	}
}