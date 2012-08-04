// ----------------------------------------------------------------------- //
//
// MODULE  : BloodSplatFX.cpp
//
// PURPOSE : Blood splat special FX - Implementation
//
// CREATED : 10/13/97
//
// ----------------------------------------------------------------------- //

#include "BloodSplatFX.h"
#include "cpp_client_de.h"
#include "dlink.h"
#include "BloodClientShell.h"
#include "ClientUtilities.h"

#define REGION_DIAMETER			100.0f  // Squared distance actually
#define MAX_SPLATS_IN_REGION	10

DList CBloodSplatFX::m_BloodSplatList =
{
	0,
	{ DNULL, DNULL, DNULL }
};

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodSplatFX::Init
//
//	PURPOSE:	Create the mark
//
// ----------------------------------------------------------------------- //

DBOOL CBloodSplatFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!psfxCreateStruct) return DFALSE;

	CSpecialFX::Init(psfxCreateStruct);

	BSCREATESTRUCT* pMark = (BSCREATESTRUCT*)psfxCreateStruct;

	VEC_COPY(m_vPos, pMark->m_Pos);
	ROT_COPY( m_Rotation, pMark->m_Rotation );
	VEC_SET( m_vScale, pMark->m_fScale, pMark->m_fScale, pMark->m_fScale );
	m_fGrowScale = pMark->m_fGrowScale;

	if( m_hstrSprite )
		g_pClientDE->FreeString( m_hstrSprite );
	m_hstrSprite = g_pClientDE->CopyString( pMark->m_hstrSprite );

	if(m_fGrowScale)
		m_fGrowTime = 4.0f;
	else
		m_fGrowTime = 0.0f;

	m_bShrink = DFALSE;

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodSplatFX::Term
//
//	PURPOSE:	Term
//
// ----------------------------------------------------------------------- //

DBOOL CBloodSplatFX::Term()
{
	m_bShrink = DTRUE;
	m_fGrowTime = 0.0f;

	return DFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodSplatFX::CreateObject
//
//	PURPOSE:	Create object associated with the mark
//
// ----------------------------------------------------------------------- //

DBOOL CBloodSplatFX::CreateObject(CClientDE *pClientDE)
{
	DLink *pCur;
	CBloodSplatFX *pSplat;

	if (!CSpecialFX::CreateObject(pClientDE)) return DFALSE;

	// Before we create a new bloodsplat see if there is already another
	// one close by that we could use instead...

	HOBJECT hMoveObj		 = DNULL;
	HOBJECT hObj			 = DNULL;
	DFLOAT	fClosestDist	 = REGION_DIAMETER;
	DBYTE	nNumInRegion	 = 0;
	DVector vPos;

	pCur = m_BloodSplatList.m_Head.m_pNext;
	while( pCur != &m_BloodSplatList.m_Head )
	{
		pSplat = ( CBloodSplatFX * )pCur->m_pData;
		pCur = pCur->m_pNext;

		if( pSplat && pSplat != this )
		{
			hObj = pSplat->GetObject();
			if (hObj)
			{
				pClientDE->GetObjectPos(hObj, &vPos);
				
				DFLOAT fDist = VEC_DISTSQR(vPos, m_vPos);
				if (fDist < REGION_DIAMETER)
				{
					if (fDist < fClosestDist)
					{
						fClosestDist = fDist;
						hMoveObj = hObj;
					}

					if (++nNumInRegion > MAX_SPLATS_IN_REGION)
					{
						// Just move this bullet-hole to the correct pos, and
						// remove thyself...

						pClientDE->SetObjectPos(hMoveObj, &m_vPos);
						return DFALSE;
					}
				}
			}
		}
	}

	if(m_hServerObject)
		pClientDE->GetObjectPos(m_hServerObject, &m_vPos);

	if (!m_hstrSprite)
	{
		switch(GetRandom(1,3))
		{
			case 1:		m_hstrSprite = pClientDE->CreateString("sprites\\blood1.spr");	break;
			case 2:		m_hstrSprite = pClientDE->CreateString("sprites\\blood2.spr");	break;
			case 3:		m_hstrSprite = pClientDE->CreateString("sprites\\blood3.spr");	break;
			default:	m_hstrSprite = pClientDE->CreateString("sprites\\blood1.spr");	break;
		}
	}

	// Setup the mark...
	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	createStruct.m_ObjectType = OT_SPRITE;
	_mbscpy((unsigned char*)createStruct.m_Filename, (const unsigned char*)m_pClientDE->GetStringData( m_hstrSprite ));
	createStruct.m_Flags	  = FLAG_VISIBLE | FLAG_ROTATEABLESPRITE;
	VEC_COPY(createStruct.m_Pos, m_vPos);
	ROT_COPY( createStruct.m_Rotation, m_Rotation );

	m_hObject = pClientDE->CreateObject(&createStruct);

	m_pClientDE->SetObjectScale(m_hObject, &m_vScale);
	m_pClientDE->SetObjectColor(m_hObject, 0.1f, 0.1f, 0.1f, 1.0f);

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodSplatFX::Update
//
//	PURPOSE:	Update the mark
//
// ----------------------------------------------------------------------- //

DBOOL CBloodSplatFX::Update()
{
	if(!m_hObject || !m_pClientDE) return DFALSE;

	DFLOAT fDelta = m_pClientDE->GetFrameTime();
	if (fDelta > 0.25f) fDelta = 0.25f;

	if(m_fGrowScale && m_fGrowTime > 0 && !m_bShrink)
	{
		DFLOAT fScale = m_fGrowScale * fDelta * 30.0f;
		m_vScale.x *= GetRandom(1.0f,1.0f + fScale * fDelta);
		m_vScale.y *= GetRandom(1.0f,1.0f + fScale * fDelta);
		m_vScale.z *= GetRandom(1.0f,1.0f + fScale * fDelta);
		
		m_pClientDE->SetObjectScale(m_hObject, &m_vScale);
		m_fGrowTime -= fDelta;
	}
	else if(m_bShrink)
	{
		if(m_fGrowTime >= 5.0f)
			return DFALSE;

		DFLOAT fAlpha = 0.0f;
		DVector vColor;

		if(m_fGrowScale)
		{
			DFLOAT fScale = m_fGrowScale * fDelta * 30.0f;
			m_vScale.x *= GetRandom(1.0f - fScale,1.0f);
			m_vScale.y *= GetRandom(1.0f - fScale,1.0f);
			m_vScale.z *= GetRandom(1.0f - fScale,1.0f);

			m_pClientDE->SetObjectScale(m_hObject, &m_vScale);
		}
		
		m_pClientDE->GetObjectColor(m_hObject,&vColor.x, &vColor.y, &vColor.z, &fAlpha);
		m_pClientDE->SetObjectColor(m_hObject,vColor.x,vColor.y,vColor.z,1.0f - (m_fGrowTime/5.0f));
		m_fGrowTime += m_pClientDE->GetFrameTime();
	}

	return DTRUE;
}