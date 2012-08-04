// ----------------------------------------------------------------------- //
//
// MODULE  : BulletImpactSFX.cpp
//
// PURPOSE : BulletImpact special FX - Implementation
//
// CREATED : 10/13/97
//
// ----------------------------------------------------------------------- //

#include "BulletImpactSFX.h"
#include "MarkSFX.h"
#include "cpp_client_de.h"
#include "dlink.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBulletImpactSFX::Init
//
//	PURPOSE:	Create the mark
//
// ----------------------------------------------------------------------- //

DBOOL CBulletImpactSFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!psfxCreateStruct) return DFALSE;

	CSpecialFX::Init(psfxCreateStruct);

	MARKCREATESTRUCT* pMark = (MARKCREATESTRUCT*)psfxCreateStruct;

	ROT_COPY( m_Rotation, pMark->m_Rotation );
	m_fScale = pMark->m_fScale;
	m_hstrMarkSprite = pMark->m_hstrSprite;
/	VEC_SET(pMark->m_vColor,0.0f,0.0f,0.0f);

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBulletImpactSFX::CreateObject
//
//	PURPOSE:	Create object associated with the mark
//
// ----------------------------------------------------------------------- //

DBOOL CBulletImpactSFX::CreateObject(CClientDE *pClientDE)
{
	DVector vScale, vPos;

	if (!CSpecialFX::CreateObject(pClientDE)) return DFALSE;

	pClientDE->GetObjectPos(m_hServerObject, &vPos);

	// Setup the mark...
	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	createStruct.m_ObjectType = OT_SPRITE;
	_mbscpy((unsigned char*)createStruct.m_Filename, (const unsigned char*)m_pClientDE->GetStringData( m_hstrMarkSprite ));
	createStruct.m_Flags	  = FLAG_VISIBLE | FLAG_ROTATEABLESPRITE | FLAG_SPRITECHROMAKEY;
	VEC_COPY(createStruct.m_Pos, vPos);
	ROT_COPY( createStruct.m_Rotation, m_Rotation );

	m_hObject = pClientDE->CreateObject(&createStruct);

	VEC_SET( vScale, m_fScale, m_fScale, m_fScale );
	m_pClientDE->SetObjectScale(m_hObject, &vScale);

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBulletImpactSFX::Update
//
//	PURPOSE:	Update the mark
//
// ----------------------------------------------------------------------- //

DBOOL CBulletImpactSFX::Update()
{
	if(!m_hObject || !m_pClientDE) return DFALSE;

	return DTRUE;
}