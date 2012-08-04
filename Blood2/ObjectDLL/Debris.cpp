// ----------------------------------------------------------------------- //
//
// MODULE  : Debris.cpp
//
// PURPOSE : Debris implementation
//
// CREATED : 5/12/98
//
// ----------------------------------------------------------------------- //

#include "Debris.h"
#include "cpp_server_de.h"
#include "ObjectUtilities.h"
#include "generic_msg_de.h"
#include "SFXMsgIds.h"
#include "Gib.h"
#include "ClientGibFX.h"
#include "SoundTypes.h"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebris::CDebris()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

CDebris::CDebris() : Aggregate()
{
	m_hObject			= DNULL;

	m_hstrSound	= DNULL;
	m_hstrTexture1	= DNULL;
	m_hstrTexture2	= DNULL;
	m_hstrModel1	= DNULL;
	m_hstrModel2	= DNULL;

	m_nAmount	= 10;
	m_bStone	= DFALSE;
	m_bMetal	= DFALSE;
	m_bWood		= DFALSE;
	m_bEnergy	= DFALSE;
	m_bGlass	= DFALSE;
	m_bTerrain	= DFALSE;
	m_bPlastic	= DFALSE;
	m_bFlesh	= DFALSE;
	m_bLiquid	= DFALSE;
	m_bExploding = DFALSE;
	m_bCustom	= DFALSE;
	m_fScale	= 1.0f;

	m_fDamageRadius		= 200;
	m_fExplodeDamage	= 100;

	m_eType		= SURFTYPE_UNKNOWN;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebris::~CDebris()
//
//	PURPOSE:	Destroy object
//
// ----------------------------------------------------------------------- //

CDebris::~CDebris()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	if (m_hstrSound)
	{
		pServerDE->FreeString(m_hstrSound);
	}
	if (m_hstrModel1)
	{
		pServerDE->FreeString(m_hstrModel1);
	}
	if (m_hstrModel2)
	{
		pServerDE->FreeString(m_hstrModel2);
	}

	if (m_hstrTexture1)
	{
		pServerDE->FreeString(m_hstrTexture1);
	}
	if (m_hstrTexture2)
	{
		pServerDE->FreeString(m_hstrTexture2);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebris::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD CDebris::EngineMessageFn(LPBASECLASS pObject, DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
			
			if (pStruct)
			{
				if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
				{
					ReadProp(pStruct);
				}
			}
			break;
		}

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
				InitialUpdate();
			break;
		}

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


	return Aggregate::EngineMessageFn(pObject, messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebris::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

DBOOL CDebris::ReadProp(ObjectCreateStruct *)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	long nLongVal;
	if (pServerDE->GetPropLongInt("Amount", &nLongVal) == DE_OK)
		m_nAmount = (DBYTE)nLongVal;

	DBOOL bBoolVal;
	if (pServerDE->GetPropBool("Stone", &bBoolVal) == DE_OK)
		m_bStone = bBoolVal;

	if (pServerDE->GetPropBool("Metal", &bBoolVal) == DE_OK)
		m_bMetal = bBoolVal;

	if (pServerDE->GetPropBool("Wood", &bBoolVal) == DE_OK)
		m_bWood = bBoolVal;
	
	if (pServerDE->GetPropBool("Energy", &bBoolVal) == DE_OK)
		m_bEnergy = bBoolVal;
	
	if (pServerDE->GetPropBool("Glass", &bBoolVal) == DE_OK)
		m_bGlass = bBoolVal;
	
	if (pServerDE->GetPropBool("Terrain", &bBoolVal) == DE_OK)
		m_bTerrain = bBoolVal;

	if (pServerDE->GetPropBool("Plastic", &bBoolVal) == DE_OK)
		m_bPlastic = bBoolVal;

	if (pServerDE->GetPropBool("Metal", &bBoolVal) == DE_OK)
		m_bMetal = bBoolVal;

	if (pServerDE->GetPropBool("Liquid", &bBoolVal) == DE_OK)
		m_bLiquid = bBoolVal;

	if (pServerDE->GetPropBool("Custom", &bBoolVal) == DE_OK)
		m_bCustom = bBoolVal;

	if (pServerDE->GetPropBool("Flesh", &bBoolVal) == DE_OK)
		m_bFlesh = bBoolVal;

	if (pServerDE->GetPropBool("Exploding", &bBoolVal) == DE_OK)
		m_bExploding = bBoolVal;

	DFLOAT fRealVal;
	if (pServerDE->GetPropReal("Scale", &fRealVal) == DE_OK)
		m_fScale = fRealVal;

	char buf[MAX_CS_FILENAME_LEN];
	if (pServerDE->GetPropString("CustomSound", buf, MAX_CS_FILENAME_LEN) == DE_OK)
	{
		if (_mbstrlen(buf) > 0)
			m_hstrSound = pServerDE->CreateString(buf);
	}

	if (pServerDE->GetPropString("CustomModel", buf, MAX_CS_FILENAME_LEN) == DE_OK)
	{
		if (_mbstrlen(buf) > 0)
		{
			m_hstrModel1 = pServerDE->CreateString(buf);
//			m_hstrModel2 = pServerDE->CopyString(m_hstrModel1);
		}
	}

	if (pServerDE->GetPropString("CustomTexture", buf, MAX_CS_FILENAME_LEN) == DE_OK)
	{
		if (_mbstrlen(buf) > 0)
		{
			m_hstrTexture1 = pServerDE->CreateString(buf);
//			m_hstrTexture2 = pServerDE->CopyString(m_hstrTexture1);
		}
	}

	return DTRUE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebris::Init
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

DBOOL CDebris::Init(HOBJECT hObject)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!hObject || !pServerDE) return DFALSE;

	m_hObject = hObject;

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebris::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

void CDebris::InitialUpdate()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	if (m_bStone)
		m_eType = SURFTYPE_STONE;
	else if (m_bMetal)
		m_eType = SURFTYPE_METAL;
	else if (m_bWood)
		m_eType = SURFTYPE_WOOD;
	else if (m_bTerrain)
		m_eType = SURFTYPE_TERRAIN;
	else if (m_bPlastic)
		m_eType = SURFTYPE_PLASTIC;
	else if (m_bGlass)
		m_eType = SURFTYPE_GLASS;
	else if (m_bFlesh)
		m_eType = SURFTYPE_FLESH;
	else if (m_bLiquid)
		m_eType = SURFTYPE_LIQUID;
	else if (m_bEnergy)
		m_eType = SURFTYPE_ENERGY;

//	m_eType = m_eType / 10;

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebris::Create
//
//	PURPOSE:	Create the debris
//
// ----------------------------------------------------------------------- //

void CDebris::Create(DVector vDir, DFLOAT fDamage)
{
	if (!g_pServerDE) return;

//	HCLASS hClass = g_pServerDE->GetClass( "CClientGibFX" );
//	if( !hClass )
//		return;

	VEC_NEGATE(vDir, vDir);

	DFLOAT fVelFactor = 1.0f;

//	DDWORD nType = m_eType*10;
//	if (nType == SURFTYPE_STONE || nType == SURFTYPE_METAL)
//		fVelFactor = 0.75f;

	DFLOAT fVel = 50.0f + fDamage;

	vDir.y -= 1.0f;
	VEC_NORM(vDir);

	VEC_MULSCALAR(vDir, vDir, fVel);

//	ObjectCreateStruct ocStruct;
//	INIT_OBJECTCREATESTRUCT(ocStruct);

	DVector vPos;
	g_pServerDE->GetObjectPos(m_hObject, &vPos);

	if (m_bExploding)
		AddExplosion(vPos);
	
//	VEC_COPY(ocStruct.m_Pos, vPos);
//	g_pServerDE->GetObjectRotation(m_hObject, &ocStruct.m_Rotation);

	DVector vDims;
	g_pServerDE->GetObjectDims( m_hObject, &vDims );

	DDWORD dwCustom = (m_hstrModel1 || m_hstrTexture1 || m_hstrSound) ? TYPEFLAG_CUSTOM : 0;

//	CClientGibFX* pGib;

	if (m_bStone /* && (pGib = (CClientGibFX*)g_pServerDE->CreateObject(hClass, &ocStruct)) */)
	{
//		pGib->Setup(&vPos, &vDir, &vDims, SURFTYPE_STONE/10 | SIZE_SMALL | dwCustom, m_fScale, m_nAmount,
//			m_hstrModel1, m_hstrTexture1, m_hstrSound);
		SetupClientGibFX(&vPos, &vDir, &vDims, SURFTYPE_STONE/10 | SIZE_SMALL | dwCustom, m_fScale, m_nAmount,
			m_hstrModel1, m_hstrTexture1, m_hstrSound);
	}

	if (m_bMetal /* && (pGib = (CClientGibFX*)g_pServerDE->CreateObject(hClass, &ocStruct)) */)
	{
//		pGib->Setup(&vPos, &vDir, &vDims, SURFTYPE_METAL/10 | SIZE_SMALL | dwCustom, m_fScale, m_nAmount,
//			m_hstrModel1, m_hstrTexture1, m_hstrSound);
		SetupClientGibFX(&vPos, &vDir, &vDims, SURFTYPE_METAL/10 | SIZE_SMALL | dwCustom, m_fScale, m_nAmount,
			m_hstrModel1, m_hstrTexture1, m_hstrSound);
	}

	if (m_bWood /* && (pGib = (CClientGibFX*)g_pServerDE->CreateObject(hClass, &ocStruct)) */)
	{
//		pGib->Setup(&vPos, &vDir, &vDims, SURFTYPE_WOOD/10 | SIZE_SMALL | dwCustom, m_fScale, m_nAmount,
//			m_hstrModel1, m_hstrTexture1, m_hstrSound);
		SetupClientGibFX(&vPos, &vDir, &vDims, SURFTYPE_WOOD/10 | SIZE_SMALL | dwCustom, m_fScale, m_nAmount,
			m_hstrModel1, m_hstrTexture1, m_hstrSound);
	}

	if (m_bTerrain /*&& (pGib = (CClientGibFX*)g_pServerDE->CreateObject(hClass, &ocStruct)) */)
	{
//		pGib->Setup(&vPos, &vDir, &vDims, SURFTYPE_TERRAIN/10 | SIZE_SMALL | dwCustom, m_fScale, m_nAmount,
//			m_hstrModel1, m_hstrTexture1, m_hstrSound);
		SetupClientGibFX(&vPos, &vDir, &vDims, SURFTYPE_TERRAIN/10 | SIZE_SMALL | dwCustom, m_fScale, m_nAmount,
			m_hstrModel1, m_hstrTexture1, m_hstrSound);
	}

	if (m_bPlastic /*&& (pGib = (CClientGibFX*)g_pServerDE->CreateObject(hClass, &ocStruct))*/)
	{
//		pGib->Setup(&vPos, &vDir, &vDims, SURFTYPE_PLASTIC/10 | SIZE_SMALL | dwCustom, m_fScale, m_nAmount,
//			m_hstrModel1, m_hstrTexture1, m_hstrSound);
		SetupClientGibFX(&vPos, &vDir, &vDims, SURFTYPE_PLASTIC/10 | SIZE_SMALL | dwCustom, m_fScale, m_nAmount,
			m_hstrModel1, m_hstrTexture1, m_hstrSound);
	}

	if (m_bGlass /*&& (pGib = (CClientGibFX*)g_pServerDE->CreateObject(hClass, &ocStruct))*/)
	{
//		pGib->Setup(&vPos, &vDir, &vDims, SURFTYPE_GLASS/10 | SIZE_SMALL | dwCustom, m_fScale, m_nAmount,
//			m_hstrModel1, m_hstrTexture1, m_hstrSound);
		SetupClientGibFX(&vPos, &vDir, &vDims, SURFTYPE_GLASS/10 | SIZE_SMALL | dwCustom, m_fScale, m_nAmount,
			m_hstrModel1, m_hstrTexture1, m_hstrSound);
	}

	if (m_bFlesh /*&& (pGib = (CClientGibFX*)g_pServerDE->CreateObject(hClass, &ocStruct))*/)
	{
//		pGib->Setup(&vPos, &vDir, &vDims, SURFTYPE_FLESH/10 | SIZE_SMALL | dwCustom, m_fScale, m_nAmount,
//			m_hstrModel1, m_hstrTexture1, m_hstrSound);
		SetupClientGibFX(&vPos, &vDir, &vDims, SURFTYPE_FLESH/10 | SIZE_SMALL | dwCustom, m_fScale, m_nAmount,
			m_hstrModel1, m_hstrTexture1, m_hstrSound);
	}

	if (m_bLiquid /*&& (pGib = (CClientGibFX*)g_pServerDE->CreateObject(hClass, &ocStruct))*/)
	{
//		pGib->Setup(&vPos, &vDir, &vDims, SURFTYPE_LIQUID/10 | SIZE_SMALL | dwCustom, m_fScale, m_nAmount,
//			m_hstrModel1, m_hstrTexture1, m_hstrSound);
		SetupClientGibFX(&vPos, &vDir, &vDims, SURFTYPE_LIQUID/10 | SIZE_SMALL | dwCustom, m_fScale, m_nAmount,
			m_hstrModel1, m_hstrTexture1, m_hstrSound);
	}

	if (m_bEnergy /*&& (pGib = (CClientGibFX*)g_pServerDE->CreateObject(hClass, &ocStruct))*/)
	{
//		pGib->Setup(&vPos, &vDir, &vDims, SURFTYPE_ENERGY/10 | SIZE_SMALL | dwCustom, m_fScale, m_nAmount,
//			m_hstrModel1, m_hstrTexture1, m_hstrSound);
		SetupClientGibFX(&vPos, &vDir, &vDims, SURFTYPE_ENERGY/10 | SIZE_SMALL | dwCustom, m_fScale, m_nAmount,
			m_hstrModel1, m_hstrTexture1, m_hstrSound);
	}
}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebris::AddExplosion()
//
//	PURPOSE:	Add an explosion
//
// ----------------------------------------------------------------------- //

void CDebris::AddExplosion(DVector &vPos)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DDWORD		nType = EXP_GRENADE;
	DVector		vUp;
	VEC_SET(vUp, 0.0f, 1.0f, 0.0f);

	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&vPos);
	pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

	pServerDE->WriteToMessageVector(hMessage, &vPos);
	pServerDE->WriteToMessageVector(hMessage, &vUp);
	pServerDE->WriteToMessageDWord(hMessage, nType);

	pServerDE->EndMessage(hMessage);

	PlaySoundFromPos(&vPos, "Sounds\\Weapons\\c4\\explosion_1.wav", 1000.0f, SOUNDPRIORITY_MISC_MEDIUM);

	// Do some damage
	if (m_fExplodeDamage && m_fDamageRadius)
		DamageObjectsInRadius(m_hObject, pServerDE->HandleToObject(m_hObject), vPos, m_fDamageRadius, m_fExplodeDamage, DAMAGE_TYPE_EXPLODE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebris::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CDebris::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hObject);
	pServerDE->WriteToMessageHString(hWrite, m_hstrSound);
	pServerDE->WriteToMessageHString(hWrite, m_hstrTexture1);
	pServerDE->WriteToMessageHString(hWrite, m_hstrTexture2);
	pServerDE->WriteToMessageHString(hWrite, m_hstrModel1);
	pServerDE->WriteToMessageHString(hWrite, m_hstrModel2);
	pServerDE->WriteToMessageByte(hWrite, m_nAmount);
	pServerDE->WriteToMessageFloat(hWrite, m_fScale);
	pServerDE->WriteToMessageByte(hWrite, m_bExploding);
	pServerDE->WriteToMessageByte(hWrite, m_bStone);
	pServerDE->WriteToMessageByte(hWrite, m_bMetal);
	pServerDE->WriteToMessageByte(hWrite, m_bWood);	
	pServerDE->WriteToMessageByte(hWrite, m_bEnergy);
	pServerDE->WriteToMessageByte(hWrite, m_bGlass);
	pServerDE->WriteToMessageByte(hWrite, m_bTerrain);
	pServerDE->WriteToMessageByte(hWrite, m_bPlastic);
	pServerDE->WriteToMessageByte(hWrite, m_bFlesh);
	pServerDE->WriteToMessageByte(hWrite, m_bLiquid);
	pServerDE->WriteToMessageByte(hWrite, m_bCustom);
	pServerDE->WriteToMessageDWord(hWrite, (DDWORD)m_eType);

	pServerDE->WriteToMessageFloat(hWrite, m_fDamageRadius);
	pServerDE->WriteToMessageFloat(hWrite, m_fExplodeDamage);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebris::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CDebris::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hRead) return;

	pServerDE->ReadFromLoadSaveMessageObject(hRead, &m_hObject);
	m_hstrSound		= pServerDE->ReadFromMessageHString(hRead);
	m_hstrTexture1	= pServerDE->ReadFromMessageHString(hRead);
	m_hstrTexture2	= pServerDE->ReadFromMessageHString(hRead);
	m_hstrModel1	= pServerDE->ReadFromMessageHString(hRead);
	m_hstrModel2	= pServerDE->ReadFromMessageHString(hRead);
	m_nAmount		= pServerDE->ReadFromMessageByte(hRead);
	m_fScale		= pServerDE->ReadFromMessageFloat(hRead);
	m_bExploding	= pServerDE->ReadFromMessageByte(hRead);
	m_bStone		= pServerDE->ReadFromMessageByte(hRead);
	m_bMetal		= pServerDE->ReadFromMessageByte(hRead);
	m_bWood			= pServerDE->ReadFromMessageByte(hRead);
	m_bEnergy		= pServerDE->ReadFromMessageByte(hRead);
	m_bGlass		= pServerDE->ReadFromMessageByte(hRead);
	m_bTerrain		= pServerDE->ReadFromMessageByte(hRead);
	m_bPlastic		= pServerDE->ReadFromMessageByte(hRead);
	m_bFlesh		= pServerDE->ReadFromMessageByte(hRead);
	m_bLiquid		= pServerDE->ReadFromMessageByte(hRead);
	m_bCustom		= pServerDE->ReadFromMessageByte(hRead);
	m_eType			= (SurfaceType)pServerDE->ReadFromMessageDWord(hRead);

	m_fDamageRadius	= pServerDE->ReadFromMessageFloat(hRead);
	m_fExplodeDamage = pServerDE->ReadFromMessageFloat(hRead);

}



