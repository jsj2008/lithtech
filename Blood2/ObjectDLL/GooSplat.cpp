
#include "goosplat.h"
#include "shareddefs.h"
#include <stdio.h>


GooSplat::GooSplat()
{

}

GooSplat::~GooSplat()
{

}

DDWORD GooSplat::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch (messageID)
	{
		case MID_PRECREATE:
		{
			ObjectCreateStruct *ocStruct = (ObjectCreateStruct *)pData;

			sprintf(ocStruct->m_Filename,"sprites\\bloodpool.spr");
			ocStruct->m_ObjectType = OT_SPRITE;
			break;
		}

		case MID_INITIALUPDATE:
			InitialUpdate();
			break;

		case MID_TOUCHNOTIFY:
			HandleTouch((HOBJECT)pData);
			break;
	}

	return B2BaseClass::EngineMessageFn(messageID,pData,fData);
}

DBOOL GooSplat::InitialUpdate()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject) return DFALSE;

	// Align the sprite to the surface directly along the forward vector

	DRotation rRot;
	DVector vPos, vU, vR, vF;
	pServerDE->GetObjectPos(m_hObject, &vPos);
	pServerDE->GetObjectRotation(m_hObject, &rRot);
	pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

	// Determine where on the surface to place the sprite...

	IntersectInfo ii;
	IntersectQuery iq;

	VEC_COPY(iq.m_From, vPos);
	VEC_COPY(iq.m_Direction, vF);
	iq.m_Flags	 = IGNORE_NONSOLID;
	iq.m_FilterFn = DNULL;

	if (pServerDE->CastRay(&iq, &ii))
	{
		DVector vTemp;
		VEC_COPY(vPos, ii.m_Point);
		VEC_COPY(vTemp, ii.m_Plane.m_Normal);
		VEC_COPY(vF, vTemp);

		// Place just in front of the wall

		VEC_MULSCALAR(vTemp, vTemp, 0.25f);
		VEC_ADD(vPos, vPos, vTemp);

		pServerDE->SetObjectPos(m_hObject, &vPos);
	}

	return DTRUE;
}


void GooSplat::HandleTouch(HOBJECT hObj)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject) return;
	
	if (hObj)
	{
		HCLASS hPlayer = pServerDE->GetClass("CPlayerObj");
		HCLASS hClass = pServerDE->GetObjectClass(hObj);

		if (pServerDE->IsKindOf(hClass,hPlayer))
		{
			HMESSAGEWRITE hMessage = pServerDE->StartMessageToObject(this, hObj, MID_IMMOBILIZE);
			pServerDE->WriteToMessageFloat(hMessage,GOO_STICK_TIME);
		}
	}
}