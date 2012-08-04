//------------------------------------------------------------------
//
//   MODULE  : BOUNCYCHUNKFX.CPP
//
//   PURPOSE : Implements class CLTBBouncyChunkFX
//
//   CREATED : On 12/3/98 At 6:34:44 PM
//
//------------------------------------------------------------------

// Includes....

#include "stdafx.h"
#include "LTBBouncyChunkFX.h"
#include "ClientFX.h"

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CLTBBouncyChunkProps::CBouncyChunkProps
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
CLTBBouncyChunkProps::CLTBBouncyChunkProps() : 
	m_bPlayImpactSound(false),
	m_fChunkSpeed(0.0f),
	m_fChunkSpread(0.0f),
	m_vChunkDir(0.0f, 0.0f, 0.0f),
	m_vGravity(0.0f, 0.0f, 0.0f),
	m_fGravityAmount(0.0f)
{
	m_sModelName[0] = '\0';
	m_sSkinName[0] = '\0';
	m_sImpactSound[0] = '\0';
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CLTBBouncyChunkProps::ReadProps
//
//  PURPOSE:	Read in the proporty values that were set in FxED
//
// ----------------------------------------------------------------------- //

bool CLTBBouncyChunkProps::ParseProperties(FX_PROP* pProps, uint32 nNumProps)
{
	if(!CBaseFXProps::ParseProperties(pProps, nNumProps))
		return false;

	for(uint32 nCurrProp = 0; nCurrProp < nNumProps; nCurrProp++)
	{
		FX_PROP& fxProp = pProps[nCurrProp];

		if (!stricmp(fxProp.m_sName, "Model"))
		{
			char sTmp[128];
			strcpy(sTmp, fxProp.m_data.m_sVal);

			// Get the path name

			char *sExt  = strtok(sTmp, "|");
			char *sPath = strtok(NULL, "|");
			if (sPath) strcpy(m_sModelName, sPath);
		}
		else if (!stricmp(fxProp.m_sName, "Skin"))
		{
			char sTmp[128];
			strcpy(sTmp, fxProp.m_data.m_sVal);

			// Get the path name

			char *sExt  = strtok(sTmp, "|");
			char *sPath = strtok(NULL, "|");
			if (sPath) strcpy(m_sSkinName, sPath);
		}
		else if (!stricmp(fxProp.m_sName, "ChunkSound"))
		{
			char sTmp[128];
			strcpy(sTmp, fxProp.m_data.m_sVal);

			// Get the path name

			char *sExt  = strtok(sTmp, "|");
			char *sPath = strtok(NULL, "|");
			if (sPath)
			{
				strcpy(m_sImpactSound, sPath);
				m_bPlayImpactSound = true;
			}
		}
		else if (!stricmp(fxProp.m_sName, "Amount"))
		{
			m_fGravityAmount = fxProp.m_data.m_fVal;
		}
		else if (!stricmp(fxProp.m_sName, "Gravity"))
		{
			m_vGravity.x = fxProp.m_data.m_fVec[0];
			m_vGravity.y = fxProp.m_data.m_fVec[1];
			m_vGravity.z = fxProp.m_data.m_fVec[2];
			m_vGravity.Norm();
		}
		else if (!stricmp(fxProp.m_sName, "ChunkDir"))
		{
			m_vChunkDir.x = fxProp.m_data.m_fVec[0];
			m_vChunkDir.y = fxProp.m_data.m_fVec[1];
			m_vChunkDir.z = fxProp.m_data.m_fVec[2];
			m_vChunkDir.Norm();
		}				
		else if (!stricmp(fxProp.m_sName, "ChunkSpeed"))
		{
			m_fChunkSpeed = fxProp.m_data.m_fVal;
		}
		else if (!stricmp(fxProp.m_sName, "ChunkSpread"))
		{
			m_fChunkSpread = fxProp.m_data.m_fVal;
		}
	}

	m_vGravity *= m_fGravityAmount;

	return true;
}

//------------------------------------------------------------------
//
//   FUNCTION : CLTBBouncyChunkFX()
//
//   PURPOSE  : Standard constuctor
//
//------------------------------------------------------------------

CLTBBouncyChunkFX::CLTBBouncyChunkFX()
:	CBaseFX				( CBaseFX::eLTBBouncyChunkFX ),
	m_hBouncyChunk		( LTNULL ),
	m_hImpactSound		( LTNULL )
{

}

//------------------------------------------------------------------
//
//   FUNCTION : ~CLTBBouncyChunkFX
//
//   PURPOSE  : Standard destructor
//
//------------------------------------------------------------------

CLTBBouncyChunkFX::~CLTBBouncyChunkFX()
{
	Term();
}

//------------------------------------------------------------------
//
//   FUNCTION : Init()
//
//   PURPOSE  : Initialises class CLTBBouncyChunkFX
//
//------------------------------------------------------------------

bool CLTBBouncyChunkFX::Init(ILTClient *pClientDE, FX_BASEDATA *pBaseData, const CBaseFXProps *pProps)
{
	// Perform base class initialisation

	if (!CBaseFX::Init(pClientDE, pBaseData, pProps)) 
		return false;

	LTVector vChunkDir = GetProps()->m_vChunkDir;
	if (pBaseData->m_bUseTargetData)
	{
		vChunkDir = pBaseData->m_vTargetNorm;
	}

	LTVector vPos;
	LTRotation rRot;
	if (m_hParent)
	{
		m_pLTClient->GetObjectPos(m_hParent, &vPos);
		m_pLTClient->GetObjectRotation(m_hParent, &rRot);
	}	
	else
	{
		vPos = m_vCreatePos;
		rRot = m_rCreateRot;
	}

	float scale;
	CalcScale(m_tmElapsed, GetProps()->m_tmLifespan, &scale);

	LTVector vScale(scale, scale, scale);

	ObjectCreateStruct ocs;
	INIT_OBJECTCREATESTRUCT(ocs);

	ocs.m_ObjectType		= OT_MODEL;
	ocs.m_Flags				= FLAG_NOLIGHT | FLAG_VISIBLE;
	ocs.m_Pos				= vPos + GetProps()->m_vOffset;
	ocs.m_Rotation			= rRot;
	ocs.m_Scale				= vScale;
	strcpy(ocs.m_Filename, GetProps()->m_sModelName);
	strcpy(ocs.m_SkinName, GetProps()->m_sSkinName);

	m_hBouncyChunk = m_pLTClient->CreateObject(&ocs);

	// Setup an initial vector for the velocity

	LTVector vOther;
	vOther.x = 1.0f;
	vOther.y = 0.0f;
	vOther.z = 1.0f;
	vOther.Norm();

	LTVector vRight = vChunkDir.Cross(vOther);
	LTVector vUp    = vRight.Cross(vOther);

	m_vVel = vRight * (-GetProps()->m_fChunkSpread + (float)(rand() % (int)(GetProps()->m_fChunkSpread * 2.0f)));
	m_vVel += vUp * (-GetProps()->m_fChunkSpread + (float)(rand() % (int)(GetProps()->m_fChunkSpread * 2.0f)));
	m_vVel += vChunkDir * GetProps()->m_fChunkSpeed;
	m_vVel.Norm(GetProps()->m_fChunkSpeed);

	// Create the base object

	CreateDummyObject();
		
	// Success !!

	return true;
}

//------------------------------------------------------------------
//
//   FUNCTION : Term()
//
//   PURPOSE  : Terminates class CLTBBouncyChunkFX
//
//------------------------------------------------------------------

void CLTBBouncyChunkFX::Term()
{
	if (m_hBouncyChunk) m_pLTClient->RemoveObject(m_hBouncyChunk);
	m_hBouncyChunk = NULL;

	if (m_hObject) m_pLTClient->RemoveObject(m_hObject);
	m_hObject = NULL;

	if (m_hImpactSound)
	{
		m_pLTClient->SoundMgr()->KillSound(m_hImpactSound);
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : Update()
//
//   PURPOSE  : Updates class CLTBBouncyChunkFX
//
//------------------------------------------------------------------

bool CLTBBouncyChunkFX::Update(float tmFrameTime)
{
	// Base class update first
	
	if (!CBaseFX::Update(tmFrameTime)) 
		return false;

	if ((m_hImpactSound) && (m_pLTClient->IsDone(m_hImpactSound)))
	{
		m_pLTClient->SoundMgr()->KillSound(m_hImpactSound);
		m_hImpactSound = NULL;
	}

	// Set the object scale

	LTVector vScale(m_scale, m_scale, m_scale);

	m_pLTClient->SetObjectScale(m_hBouncyChunk, &vScale);

	LTVector vCur;
	m_pLTClient->GetObjectPos(m_hBouncyChunk, &vCur);
		
	// Compute the new position of the chunk

	LTVector vNew = vCur;
	vNew += m_vVel * tmFrameTime;
	
	m_vVel += GetProps()->m_vGravity * tmFrameTime;
	
	// Move the object and collide against the world

	ClientIntersectQuery ciq;
	ClientIntersectInfo  cii;

	ciq.m_From  = vCur;
	ciq.m_To    = vNew;

	if (m_pLTClient->IntersectSegment(&ciq, &cii))
	{
		vNew = cii.m_Point + cii.m_Plane.m_Normal;
		vCur = vNew;

		// Compute the reflected velocity

		LTVector N = cii.m_Plane.m_Normal;
		LTVector L = m_vVel;
		L.x = -L.x;
		L.y = -L.y;
		L.z = -L.z;
		
		LTVector vReflected = N * 2.0f;
		vReflected *= (N.Dot(L));
		vReflected -= L;

		vReflected.Norm();
		vReflected *= (m_vVel.Mag() * 0.7f);

		m_vVel = vReflected;

		const char *sImpactSound = GetProps()->m_sImpactSound;
		if (sImpactSound[0] != '.')
		{
			// Play the bounce sound

			PlaySoundInfo psi;
			memset(&psi, 0, sizeof(PlaySoundInfo));

			psi.m_dwFlags = PLAYSOUND_GETHANDLE |
							PLAYSOUND_CTRL_VOL |
							PLAYSOUND_CLIENT |
							PLAYSOUND_TIME |
							PLAYSOUND_3D | 
							PLAYSOUND_REVERB;

			psi.m_nVolume = 50;

			strcpy(psi.m_szSoundName, GetProps()->m_sImpactSound);
			psi.m_nPriority		= 0;
			psi.m_vPosition		= m_vPos;
			psi.m_fInnerRadius	= 100;
			psi.m_fOuterRadius	= 300;

			if (!m_hImpactSound)
			{
				if (m_pLTClient->SoundMgr()->PlaySound(&psi, m_hImpactSound) == LT_OK)
				{
					m_hImpactSound = psi.m_hSound;
				}
			}
		}
	}

	m_pLTClient->SetObjectPos(m_hBouncyChunk, &vNew);
	m_pLTClient->SetObjectColor(m_hBouncyChunk, m_red, m_green, m_blue, m_alpha);
	m_pLTClient->SetObjectPos(m_hObject, &vNew);

	// Success !!

	return true;
}


//------------------------------------------------------------------
//
//   FUNCTION : fxGetBouncyChunkFXProps()
//
//   PURPOSE  : Returns a list of properties for this FX
//
//------------------------------------------------------------------

void fxGetLTBBouncyChunkProps(CFastList<FX_PROP> *pList)
{
	FX_PROP fxProp;

	float fVec[3];
	fVec[0] = 0.0f;
	fVec[1] = 1.0f;
	fVec[2] = 0.0f;

	// Add the base props

	AddBaseProps(pList);

	// Add all the props to the list

	fxProp.Path("Model", "ltb|...");
	pList->AddTail(fxProp);

	fxProp.Path("Skin", "dtx|...");
	pList->AddTail(fxProp);

	fxProp.Vector("ChunkDir", fVec);
	pList->AddTail(fxProp);

	fxProp.Float("ChunkSpeed", 40.0f);
	pList->AddTail(fxProp);

	fxProp.Float("ChunkSpread", 40.0f);
	pList->AddTail(fxProp);

	fVec[0] = 0.0f;
	fVec[1] = -1.0f;
	fVec[2] = 0.0f;

	fxProp.Vector("Gravity", fVec);
	pList->AddTail(fxProp);

	fxProp.Float("Amount", 10.0f);
	pList->AddTail(fxProp);

	fxProp.Path("ChunkSound", "wav|...");
	pList->AddTail(fxProp);
}