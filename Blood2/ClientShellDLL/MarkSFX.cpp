// ----------------------------------------------------------------------- //
//
// MODULE  : MarkSFX.cpp
//
// PURPOSE : Mark special FX - Implementation
//
// CREATED : 10/13/97
//
// ----------------------------------------------------------------------- //

#include "MarkSFX.h"
#include "cpp_client_de.h"
#include "dlink.h"
#include "BloodClientShell.h"
#include "ClientUtilities.h"


#define REGION_DIAMETER			100.0f  // Squared distance actually
#define MAX_MARKS_IN_REGION		10


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMarkSFX::Init
//
//	PURPOSE:	Create the mark
//
// ----------------------------------------------------------------------- //

DBOOL CMarkSFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!psfxCreateStruct) return DFALSE;

	CSpecialFX::Init(psfxCreateStruct);

	MARKCREATESTRUCT* pMark = (MARKCREATESTRUCT*)psfxCreateStruct;

	if (pMark->m_bServerObj && pMark->hServerObj)
		g_pClientDE->GetObjectPos(pMark->hServerObj, &m_Pos);
	else
		VEC_COPY( m_Pos, pMark->m_Pos );

	ROT_COPY( m_Rotation, pMark->m_Rotation );
	VEC_SET( m_vScale, pMark->m_fScale, pMark->m_fScale, pMark->m_fScale );
	if( m_hstrSprite )
		g_pClientDE->FreeString( m_hstrSprite );
	m_hstrSprite = g_pClientDE->CopyString( pMark->m_hstrSprite );
	m_bServerObj = pMark->m_bServerObj;

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMarkSFX::CreateObject
//
//	PURPOSE:	Create object associated with the mark
//
// ----------------------------------------------------------------------- //

DBOOL CMarkSFX::CreateObject(CClientDE *pClientDE)
{
	if (!CSpecialFX::CreateObject(pClientDE)) return DFALSE;

	CSFXMgr* psfxMgr = g_pBloodClientShell->GetSFXMgr();
	if (!psfxMgr) return DFALSE;


	// Before we create a new buillet hole see if there is already another
	// bullet hole close by that we could use instead...

	CSpecialFXList* pList = psfxMgr->GetBulletHoleFXList();
	if (!pList) return DFALSE;

	int nNumBulletHoles = pList->GetSize();

	HOBJECT hMoveObj		 = DNULL;
	HOBJECT hObj			 = DNULL;
	DFLOAT	fClosestMarkDist = REGION_DIAMETER;
	DBYTE	nNumInRegion	 = 0;
	DVector vPos;

	for (int i=0; i < nNumBulletHoles; i++)
	{
		if ((*pList)[i])
		{
			hObj = (*pList)[i]->GetObject();
			if (hObj)
			{
				pClientDE->GetObjectPos(hObj, &vPos);
				
				DFLOAT fDist = VEC_DISTSQR(vPos, m_Pos);
				if (fDist < REGION_DIAMETER)
				{
					if (fDist < fClosestMarkDist)
					{
						fClosestMarkDist = fDist;
						hMoveObj = hObj;
					}

					if (++nNumInRegion > MAX_MARKS_IN_REGION)
					{
						// Just move this bullet-hole to the correct pos, and
						// remove thyself...

						pClientDE->SetObjectPos(hMoveObj, &m_Pos);
						return DFALSE;
					}
				}
			}
		}
	}


	// Setup the mark...
	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	createStruct.m_ObjectType = OT_SPRITE;
	_mbscpy((unsigned char*)createStruct.m_Filename, (const unsigned char*)m_pClientDE->GetStringData( m_hstrSprite ));
	createStruct.m_Flags	  = FLAG_VISIBLE | FLAG_ROTATEABLESPRITE;
	VEC_COPY(createStruct.m_Pos, m_Pos);
	ROT_COPY( createStruct.m_Rotation, m_Rotation );

	m_hObject = pClientDE->CreateObject(&createStruct);

	m_pClientDE->SetObjectScale(m_hObject, &m_vScale);


	// See what it hit
	DVector vU, vR;
	pClientDE->GetRotationVectors(&m_Rotation, &vU, &vR, &m_vForward);

	ClientIntersectQuery iq;
	ClientIntersectInfo  ii;

	iq.m_Flags = INTERSECT_OBJECTS | INTERSECT_HPOLY;

	VEC_COPY(iq.m_From, vPos);			// Get start point at the last known position.
	VEC_MULSCALAR(iq.m_To, m_vForward, -1.0f);
	VEC_ADD(iq.m_To, iq.m_To, iq.m_From);	// Get destination point slightly past where we should be

	// Hit something!  try to clip against it. (since this is only being used for bullet marks,
	if (pClientDE->IntersectSegment(&iq, &ii))
	{
		HPOLY hPoly = ii.m_hPoly;
		pClientDE->ClipSprite(m_hObject, hPoly);
	}
	m_pClientDE->SetObjectColor(m_hObject, 0.1f, 0.1f, 0.1f, 1.0f);
	
	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMarkSFX::Update
//
//	PURPOSE:	Update the mark
//
// ----------------------------------------------------------------------- //

DBOOL CMarkSFX::Update()
{
	if (m_bServerObj && m_bWantRemove)
		return DFALSE;

	return DTRUE;
}