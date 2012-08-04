// ----------------------------------------------------------------------- //
//
// MODULE  : Explosion.cpp
//
// PURPOSE : Explosion - Definition
//
// CREATED : 2/20/97
//
// ----------------------------------------------------------------------- //

#include "Explosion.h"
#include "cpp_server_de.h"
#include "generic_msg_de.h"
#include "ObjectUtilities.h"
#include "ModelObject.h"
#include "Impacts.h"
#include "ClientSmokeTrail.h"
#include <mbstring.h>
#include "SoundTypes.h"


static char *szDefExplosionModel = "Models\\Explosions\\Explosion_3.abc";
static char *szDefExplosionSkin  = "Skins\\Explosions\\Explosion_1.dtx";
static char *szDefShockwaveSprite = "Sprites\\Shockring.spr";


#define EXPLOSION_UPDATE_DELTA			0.01f
#define EXPLOSION_DAMAGE_UPDATE_DELTA	0.1f

BEGIN_CLASS(Explosion)
	ADD_STRINGPROP(Model, szDefExplosionModel)
	ADD_STRINGPROP(Skin, szDefExplosionSkin)
	ADD_STRINGPROP(Sound, "Sounds\\exp_tnt.wav")
	ADD_REALPROP_FLAG(SoundRadius, 1000.0f, PF_RADIUS)
	ADD_REALPROP_FLAG(DamageRadius, 200.0f, PF_RADIUS)
	ADD_VECTORPROP(RotateSpeed)
	ADD_REALPROP(MaxDamage, 50.0f)
	ADD_REALPROP(MinScale, 0.1f)
	ADD_REALPROP(MaxScale, 1.0f)
	ADD_BOOLPROP(CreateLight, DTRUE)
	ADD_COLORPROP(LightColor, 255, 255, 255)
	ADD_BOOLPROP(CreateSmoke, DTRUE)
	ADD_BOOLPROP(CreateShockwave, DFALSE)
	ADD_STRINGPROP(ShockwaveFile, szDefShockwaveSprite)
	ADD_VECTORPROP(ShockwaveScaleMin)
	ADD_VECTORPROP(ShockwaveScaleMax)
	ADD_REALPROP(ShockwaveDuration, 0.0f)
	ADD_REALPROP(Delay, 0.0f)
END_CLASS_DEFAULT(Explosion, B2BaseClass, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::Explosion()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Explosion::Explosion() : B2BaseClass()
{
	m_hstrSkinName			= DNULL;
	m_hstrModelName			= DNULL;
	m_hstrSound				= DNULL;
	m_fSoundRadius			= 1000.0f;
	m_fDamageRadius			= 200.0f;
	m_fMaxDamage			= 50.0f;
	m_fMinScale				= 0.1f;
	m_fMaxScale				= 1.0f;
	m_fDuration				= 1.5f;
	m_bCreateSmoke			= DTRUE;
	m_hModel				= DNULL;
	m_fLastDamageTime		= 0.0f;

	m_bCreateLight			= DTRUE;
	m_hLight				= DNULL;
	m_hShockwave			= DNULL;
	m_fMinLightRadius		= 100.0f;
	m_fMaxLightRadius		= 300.0f;
	VEC_SET(m_vLightColor, 1.0f, 0.5f, 0.0f);

	m_bCreateShockwave		= DTRUE;
	m_hstrShockwaveSprite	= DNULL;
	VEC_SET(m_vShockwaveScaleMin, 0.1f, 0.1f, 1.0f);
	VEC_SET(m_vShockwaveScaleMax, 1.0f, 1.0f, 1.0f);
	m_fShockwaveDuration	= 0.0f;

	m_bCreateMark			= DFALSE;
	m_bAddSparks			= DFALSE;
	m_fStartTime			= 0.0f;
	m_fDelay				= 0.0f;
	m_bFirstUpdate			= DFALSE;
	VEC_INIT(m_vRotation);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::Setup()
//
//	PURPOSE:	Setup the Explosion data members (for non DEdit created
//				explosions)
//
// ----------------------------------------------------------------------- //

void Explosion::Setup(char* pSound, DFLOAT fSoundRadius, DFLOAT fDuration,
					  char* pSkinName, DFLOAT fDamageRadius, DFLOAT fMaxDamage, 
					  DFLOAT fMinScale, DFLOAT fMaxScale, DBOOL bCreateMark, 
					  DBOOL bAddSparks, DBOOL bCreateSmoke)
{
	if (!g_pServerDE) return;

	if (pSound) m_hstrSound	= g_pServerDE->CreateString(pSound);
	if (pSkinName) m_hstrSkinName = g_pServerDE->CreateString(pSkinName);

	m_fSoundRadius			= fSoundRadius;
	m_fDamageRadius			= fDamageRadius;
	m_fMaxDamage			= fMaxDamage;
	m_fMinScale				= fMinScale;
	m_fMaxScale				= fMaxScale;
	m_fDuration				= fDuration;
	m_bCreateSmoke			= bCreateSmoke;
	m_bCreateMark			= bCreateMark;
	m_bAddSparks			= bAddSparks;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::SetupLight()
//
//	PURPOSE:	Setup the Explosion data members (for non DEdit created
//				explosions) if a light is desired
//
// ----------------------------------------------------------------------- //

void Explosion::SetupLight(DBOOL bCreateLight, DVector vLightColor, 
						   DFLOAT fMinRadius, DFLOAT fMaxRadius)
{	
	m_bCreateLight = bCreateLight;
	VEC_COPY(m_vLightColor, vLightColor);
	m_fMinLightRadius = fMinRadius;
	m_fMaxLightRadius = fMaxRadius;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::SetupShockwave()
//
//	PURPOSE:	Setup the Explosion data members (for non DEdit created
//				explosions) if a shockwave is desired
//
// ----------------------------------------------------------------------- //

void Explosion::SetupShockwave(char *pSprite, DVector vScaleMin, DVector vScaleMax,
								DFLOAT fDuration)
{	
	if (!g_pServerDE || !pSprite) return;

	if (pSprite) m_hstrShockwaveSprite = g_pServerDE->CreateString(pSprite);
	else m_hstrShockwaveSprite = g_pServerDE->CreateString(szDefShockwaveSprite);

	VEC_COPY(m_vShockwaveScaleMin, vScaleMin);
	VEC_COPY(m_vShockwaveScaleMax, vScaleMax);

	m_fShockwaveDuration	= fDuration;
	m_bCreateShockwave		= DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::~Explosion()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

Explosion::~Explosion()
{
	if (!g_pServerDE) return;

	if ( m_hstrSound )
	{
		g_pServerDE->FreeString( m_hstrSound );
	}

	if ( m_hstrSkinName )
	{
		g_pServerDE->FreeString( m_hstrSkinName );
	}

	if ( m_hstrModelName )
	{
		g_pServerDE->FreeString( m_hstrModelName );
	}
	
	if ( m_hstrShockwaveSprite )
	{
		g_pServerDE->FreeString( m_hstrShockwaveSprite );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

DDWORD Explosion::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
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

	return B2BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD Explosion::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			Update();
			break;
		}

		case MID_INITIALUPDATE:
		{
			m_bFirstUpdate = DTRUE;
			break;
		}

		case MID_PRECREATE:
		{
			if (fData == 1.0f)
				ReadProp((ObjectCreateStruct* )pData);
			break;
		}

		case MID_LINKBROKEN:
		{
			HOBJECT hObj = (HOBJECT)pData;
            if (m_hShockwave == hObj)
            {
	    		m_hShockwave = DNULL;
		    }
		}
		break;

		default : break;
	}

	return B2BaseClass::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::ReadProp()
//
//	PURPOSE:	Update Properties
//
// ----------------------------------------------------------------------- //

void Explosion::ReadProp(ObjectCreateStruct *pStruct)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pStruct) return;

	char buf[MAX_CS_FILENAME_LEN];

	buf[0] = '\0';
	pServerDE->GetPropString("Skin", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hstrSkinName = pServerDE->CreateString(buf);

	buf[0] = '\0';
	pServerDE->GetPropString("Model", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hstrModelName = pServerDE->CreateString(buf);

	buf[0] = '\0';
	pServerDE->GetPropString("Sound", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hstrSound = pServerDE->CreateString(buf);

	pServerDE->GetPropReal("SoundRadius", &m_fSoundRadius);
	pServerDE->GetPropReal("DamageRadius", &m_fDamageRadius);
	pServerDE->GetPropReal("MaxDamage", &m_fMaxDamage);
	pServerDE->GetPropReal("MinScale", &m_fMinScale);
	pServerDE->GetPropReal("MaxScale", &m_fMaxScale);
	pServerDE->GetPropReal("Duration", &m_fMinScale);
	pServerDE->GetPropBool("CreateLight", &m_bCreateLight);
	pServerDE->GetPropVector("LightColor", &m_vLightColor);
	VEC_DIVSCALAR(m_vLightColor, m_vLightColor, 255.0f);
	pServerDE->GetPropBool("CreateSmoke", &m_bCreateSmoke);
	pServerDE->GetPropBool("CreateShockwave", &m_bCreateShockwave);

	buf[0] = '\0';
	pServerDE->GetPropString("ShockwaveFile", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hstrShockwaveSprite = pServerDE->CreateString(buf);

	pServerDE->GetPropVector("ShockwaveScaleMin", &m_vShockwaveScaleMin);
	pServerDE->GetPropVector("ShockwaveScaleMax", &m_vShockwaveScaleMax);
	pServerDE->GetPropReal("ShockwaveDuration", &m_fShockwaveDuration);
	pServerDE->GetPropReal("Delay", &m_fDelay);
	pServerDE->GetPropVector("RotateSpeed", &m_vRotation);

	return;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::CreateExplosion()
//
//	PURPOSE:	Create explosion
//
// ----------------------------------------------------------------------- //

void Explosion::CreateExplosion(DVector *pvPos)
{
	if (!g_pServerDE) return;

	ObjectCreateStruct ocStruct;
	INIT_OBJECTCREATESTRUCT(ocStruct);
	ocStruct.m_Flags = FLAG_VISIBLE;
	VEC_COPY(ocStruct.m_Pos, *pvPos);

//	DRotation rRot;
//	g_pServerDE->GetObjectRotation(m_hObject, &rRot);
//	ROT_COPY(ocStruct.m_Rotation, rRot);
	DFLOAT fPitch	= g_pServerDE->Random(-MATH_PI, MATH_PI);
	DFLOAT fYaw		= g_pServerDE->Random(-MATH_PI, MATH_PI);
	DFLOAT fRoll	= g_pServerDE->Random(-MATH_PI, MATH_PI);
	g_pServerDE->SetupEuler(&ocStruct.m_Rotation, fPitch, fYaw, fRoll);

	if (m_hstrModelName) _mbscpy((unsigned char*)ocStruct.m_Filename, (const unsigned char*)g_pServerDE->GetStringData(m_hstrModelName));
	else _mbscpy((unsigned char*)ocStruct.m_Filename, (const unsigned char*)szDefExplosionModel );

	if (m_hstrSkinName)	_mbscpy((unsigned char*)ocStruct.m_SkinName, (const unsigned char*)g_pServerDE->GetStringData(m_hstrSkinName));
	else _mbscpy((unsigned char*)ocStruct.m_SkinName, (const unsigned char*)szDefExplosionSkin );

	// Create the explosion model
	HCLASS hClass = g_pServerDE->GetClass("CModelObject");
	if (!hClass) return;

	CModelObject* pImpact = (CModelObject*)g_pServerDE->CreateObject(hClass, &ocStruct);
	if (!pImpact) return;

	// Initialize the object...
	pImpact->Setup(m_fDuration, &m_vRotation, DFALSE, DTRUE);
	m_hModel = pImpact->m_hObject;

	// Gouraud shade and make full bright...
	DDWORD dwFlags = g_pServerDE->GetObjectFlags(m_hModel);
	g_pServerDE->SetObjectFlags(m_hModel, dwFlags | FLAG_MODELGOURAUDSHADE | FLAG_NOLIGHT);
	g_pServerDE->SetObjectColor(m_hModel, 1.0f, 1.0f, 1.0f, 1.0f);

	if (m_bCreateLight) CreateLight(pvPos);
	if (m_bCreateShockwave) AddShockwave(pvPos);
	if (m_bCreateMark) 	CreateMark(pvPos);
	if (m_bCreateSmoke) CreateSmoke(pvPos);
//	CreateFX(pvPos);

	// Play sound
	DVector vPos;
	g_pServerDE->GetObjectPos(m_hObject, &vPos);
	if ( m_hstrSound )
	{
		char* pSound = g_pServerDE->GetStringData(m_hstrSound);
		if (pSound) PlaySoundFromPos(&vPos, pSound, m_fSoundRadius, SOUNDPRIORITY_MISC_HIGH );
	}	
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::CreateLight()
//
//	PURPOSE:	Create light
//
// ----------------------------------------------------------------------- //
void Explosion::CreateLight( DVector *pvPos )
{
	ObjectCreateStruct ocStruct;
	INIT_OBJECTCREATESTRUCT(ocStruct);

	if (m_hLight)
	{
		g_pServerDE->RemoveObject(m_hLight);
		m_hLight = DNULL;
	}

	ocStruct.m_Flags = FLAG_VISIBLE;
	ocStruct.m_ObjectType = OT_LIGHT; 
	VEC_COPY(ocStruct.m_Pos, *pvPos);

	HCLASS hClass = g_pServerDE->GetClass("BaseClass");
	if (!hClass) return;

	LPBASECLASS	pLight = g_pServerDE->CreateObject(hClass, &ocStruct);
	if (!pLight) return;

	m_hLight = pLight->m_hObject;
	g_pServerDE->SetLightRadius(m_hLight, m_fMinLightRadius);
	g_pServerDE->SetLightColor(m_hLight, m_vLightColor.x, m_vLightColor.y, m_vLightColor.z);
	g_pServerDE->SetObjectColor(m_hModel, m_vLightColor.x, m_vLightColor.y, m_vLightColor.z, 1.0f);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CExplosion::CreateImpact()
//
//	PURPOSE:	Create an impact object.
//
// ----------------------------------------------------------------------- //

CImpact* Explosion::CreateImpact(ObjectCreateStruct & ocStruct, HSTRING hstrFile)
{
	if (!g_pServerDE) return DNULL;

	CImpact* pRet = DNULL;

	if (g_pServerDE && hstrFile)
	{
		HCLASS hClass = g_pServerDE->GetClass("CImpact");

		_mbscpy((unsigned char*)ocStruct.m_Filename, (const unsigned char*)g_pServerDE->GetStringData( hstrFile ));

		if (hClass)
		{
			pRet = (CImpact*)g_pServerDE->CreateObject(hClass, &ocStruct);
		}
	}

	return pRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::CreateFX
//
//	PURPOSE:	Create the weaponFX
//
// ----------------------------------------------------------------------- //

//void CWeapon::CreateFX(DBYTE nFX, DVector vPos, DVector vFire, DVector vNormal, DFLOAT fDamage, HOBJECT hObject, SurfaceType eType)
void Explosion::CreateFX(DVector *vPos)
{
/*	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	if (eType == SURFTYPE_STONE || eType == SURFTYPE_METAL || 
		eType == SURFTYPE_WOOD || eType == SURFTYPE_GLASS)		
	{
		nFX |= WFX_MARK;
	}

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);


	// Offset a bit so the sprite doesn't clip into the wall
	DVector vTmp;
	VEC_MULSCALAR(vTmp, vNormal, 0.5f);
	VEC_COPY(theStruct.m_Pos, vPos);
	VEC_ADD(theStruct.m_Pos, theStruct.m_Pos, vTmp);

//	VEC_COPY(theStruct.m_Pos, vPos);
	ROT_COPY(theStruct.m_Rotation, m_Rotation);

	HCLASS hClass = pServerDE->GetClass("CClientWeaponSFX");

	CClientWeaponSFX *pWeaponFX = DNULL;

	if (hClass)
	{
		pWeaponFX = (CClientWeaponSFX *)pServerDE->CreateObject(hClass, &theStruct);
	}

	if (pWeaponFX)
	{
		DRotation rRot;
		pServerDE->AlignRotation(&rRot, &vNormal, DNULL);

		DBOOL bAltFire = (m_eState == WS_ALT_FIRING);

		pWeaponFX->Setup(hObject, m_nType, GetAmmoType(bAltFire), eType, nFX, m_fDamage, &rRot, &m_Position);
	}
*/}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::HandleTrigger
//
//	PURPOSE:	Handle trigger message.
// 
// ----------------------------------------------------------------------- //

void Explosion::HandleTrigger(HOBJECT hSender, HMESSAGEREAD hRead)
{
	HSTRING hMsg = g_pServerDE->ReadFromMessageHString(hRead);
	if (!hMsg) return;

	// See if we should make big boom...

	HSTRING hstr = g_pServerDE->CreateString("TRIGGER");
	if (g_pServerDE->CompareStringsUpper(hMsg, hstr))
	{
		Explode(m_fDelay);
	}

	g_pServerDE->FreeString(hMsg);
	g_pServerDE->FreeString(hstr);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::Explode
//
//	PURPOSE:	Creates a triggered explosion
// 
// ----------------------------------------------------------------------- //

void Explosion::Explode(DFLOAT fDelay)
{
	if (!g_pServerDE) return;
	
//	DVector vPos;
//	g_pServerDE->GetObjectPos(m_hObject, &vPos);

//	CreateExplosion(&vPos);

//	g_pServerDE->SetNextUpdate(m_hObject, EXPLOSION_UPDATE_DELTA);
	if (fDelay == 0.0f)
		fDelay = 0.01f;

	g_pServerDE->SetNextUpdate(m_hObject, fDelay);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::AddShockwave()
//
//	PURPOSE:	Add a shockwave
//
// ----------------------------------------------------------------------- //

void Explosion::AddShockwave(DVector *pvPoint)
{
	if (!g_pServerDE) return;

	ObjectCreateStruct ocStruct;
	INIT_OBJECTCREATESTRUCT(ocStruct);

	DRotation rRot;
	ROT_INIT(rRot);
	g_pServerDE->SetupEuler(&rRot, MATH_HALFPI, 0.0f, 0.0f);

	ROT_COPY(ocStruct.m_Rotation, rRot);
	VEC_COPY(ocStruct.m_Pos, *pvPoint);
	ocStruct.m_Flags = FLAG_VISIBLE | FLAG_ROTATEABLESPRITE;

	CImpact* pShockwave = CreateImpact(ocStruct, m_hstrShockwaveSprite);
	if (!pShockwave) return;

	if (pShockwave)
	{
		DVector vNormal;
		VEC_SET(vNormal, 0.0f, 1.0f, 0.0f);
		pShockwave->Setup(vNormal, m_vShockwaveScaleMin, m_vShockwaveScaleMax, 
						  DNULL, 0, m_fShockwaveDuration, DTRUE, DFALSE);
		m_hShockwave = g_pServerDE->ObjectToHandle(pShockwave);

		g_pServerDE->CreateInterObjectLink(m_hObject, m_hShockwave);	
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::Update()
//
//	PURPOSE:	Update the explosion
//
// ----------------------------------------------------------------------- //

void Explosion::Update()
{
	if (!g_pServerDE) return;

	g_pServerDE->SetNextUpdate(m_hObject, 0.01f);

	// Init
	if (m_bFirstUpdate)
	{
		m_bFirstUpdate = DFALSE;
		m_fStartTime = g_pServerDE->GetTime();

		DVector vPos;
		g_pServerDE->GetObjectPos(m_hObject, &vPos);

		CreateExplosion(&vPos);
		return;
	}

	DFLOAT fTime = g_pServerDE->GetTime() - m_fStartTime;

	// Remove everything if we're done.
	if (fTime > m_fDuration)
	{
		if (m_hLight) g_pServerDE->RemoveObject(m_hLight);
		if (m_hModel) g_pServerDE->RemoveObject(m_hModel);
		g_pServerDE->RemoveObject(m_hObject);
		return;
	}

	DFLOAT fMul = (fTime / m_fDuration);
	// Square it to give a curve to the rise
	fMul = (DFLOAT)pow(fMul, 0.4);
	
	// Scale the model
	if (m_hModel)
	{
		DVector vScale, vDims;

		vScale.x = m_fMinScale + (fMul * (m_fMaxScale - m_fMinScale));
		vScale.y = vScale.z = vScale.x;

		g_pServerDE->GetObjectDims( m_hModel, &vDims );
		VEC_MUL( vDims, vDims, vScale);
//		g_pServerDE->SetObjectDims( m_hModel, &vDims );
		g_pServerDE->ScaleObject(m_hModel, &vScale);
	}

	DFLOAT fScaleUDur = m_fDuration*0.75f;
	DFLOAT fScaleDDur = m_fDuration*0.25f;

	// Scale light
	if (m_hLight)
	{
		if (m_fDuration <= 0) return;

		DFLOAT fRange  = m_fMaxLightRadius - m_fMinLightRadius;
		DFLOAT fRadius = 0.0f;
		DFLOAT fModelAlpha = 1.0f;

		if (fTime <= fScaleUDur)
		{
			fRadius = m_fMinLightRadius + (fTime * fRange / fScaleUDur);
		}
		else
		{	
			DFLOAT fNewDeltaTime = fTime - fScaleUDur;
			fRadius = m_fMaxLightRadius - (fNewDeltaTime * fRange / fScaleDDur);
			fModelAlpha = 1.0f - (fNewDeltaTime / fScaleDDur);
			if (fModelAlpha < 0.0f) fModelAlpha = 0.0f;
		}
		fModelAlpha *= 0.4f;

		g_pServerDE->SetLightRadius(m_hLight, fRadius);

		
		// Adjust alpha on model...

		if (m_hModel)
		{
			g_pServerDE->SetObjectColor(m_hModel, m_vLightColor.x*fModelAlpha, m_vLightColor.y*fModelAlpha, m_vLightColor.z*fModelAlpha, fModelAlpha);
		}

		// and on the shockwave
		if (m_hShockwave)
		{
			g_pServerDE->SetObjectColor(m_hShockwave, m_vLightColor.x*fModelAlpha, m_vLightColor.y*fModelAlpha, m_vLightColor.z*fModelAlpha, fModelAlpha);
		}
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::CreateMark()
//
//	PURPOSE:	Create a scorch mark at the impact point
//
// ----------------------------------------------------------------------- //
void Explosion::CreateMark(DVector *pvPos)
{
	// Create the outer dark area
	CreateScorchMark(pvPos, m_fMinLightRadius/3.0f, m_fMaxLightRadius/3.0f, 
					m_fDuration *0.2f, 
					60.0f, 
					0.0f,
					30.0f,
					50.0f, 50.0f, 50.0f);

	// Create the glowing red center
	CreateScorchMark(pvPos, m_fMinLightRadius/10.0f, m_fMaxLightRadius/10.0f,
					m_fDuration * 0.2f, 
					2.0f, 
					0.0f,
					2.0f,
					255.0f, 100.0f, 0.0f);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Explosion::CreateSmoke()
//
//	PURPOSE:	Creates a smoke trail from the impact point
//
// ----------------------------------------------------------------------- //
void Explosion::CreateSmoke(DVector *pvPos)
{
	if (!g_pServerDE) return;

//    DVector vPos;
//
//	ObjectCreateStruct theStruct;
//	INIT_OBJECTCREATESTRUCT(theStruct);
//
//    g_pServerDE->GetObjectPos(m_hObject, &vPos);
//   	vPos.y = vPos.y + 5;
//    
//    VEC_COPY(theStruct.m_Pos, vPos);
//	HCLASS hClass = g_pServerDE->GetClass("CClientSmokeTrail");
//
//	CClientSmokeTrail* pTrail = DNULL;
//
//	if (hClass)
//	{
//        pTrail = (CClientSmokeTrail*)g_pServerDE->CreateObject(hClass, &theStruct);
//	}
//
//    if (pTrail)
//	{
//        DVector vVel;
//        g_pServerDE->GetVelocity(m_hObject, &vVel);
//        pTrail->Setup(vVel, DFALSE);
////    	m_hSmokeTrail[x] = pTrail->m_hObject;
//	}
}  





