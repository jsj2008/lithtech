// Gib.cpp: implementation of the CGib class.
//
//////////////////////////////////////////////////////////////////////

#define GIB_REMOVETIME	20.0f


#include <stdio.h>
#include "Gib.h"
#include "ClientSmokeTrail.h"
#include "ObjectUtilities.h"
#include "clientbloodtrail.h"
#include "clientmarksfx.h"
#include "BloodServerShell.h"
#include "ClientWeaponSFX.h"
#include "SfxMsgIDs.h"
#include <mbstring.h>
#include "SoundTypes.h"

//FLESH
char *szFleshGibs[NUM_FLESH_GIBS] = 
{
	"models\\gibs\\giblarge2.abc",
	"models\\gibs\\giblarge2.abc",
	"models\\gibs\\giblarge3.abc"
};

char *szFleshSkins[NUM_FLESH_GIBS] = 
{
	"skins\\gibs\\giblarge2.dtx",
	"skins\\gibs\\giblarge2.dtx",
	"skins\\gibs\\giblarge3.dtx"
};

//METAL
char *szMetalGibs[NUM_METAL_GIBS] = 
{
	"models\\gibs\\gibmetal1.abc",
	"models\\gibs\\gibmetal2.abc",
};

char *szMetalSkins[NUM_METAL_GIBS] = 
{
	"skins\\gibs\\gibmetal1.dtx",
	"skins\\gibs\\gibmetal2.dtx",
};

//FLESH
char *szStoneGibs[NUM_STONE_GIBS] = 
{
	"models\\gibs\\giblarge2.abc",
	"models\\gibs\\giblarge3.abc"
};

char *szStoneSkins[NUM_STONE_GIBS] = 
{
	"skins\\gibs\\giblarge2.dtx",
	"skins\\gibs\\giblarge3.dtx"
};

//WOOD
char *szWoodGibs[NUM_WOOD_GIBS] = 
{
	"models\\gibs\\debriswood1.abc",
	"models\\gibs\\debriswood2.abc",
	"models\\gibs\\debriswood3.abc"
};

char *szWoodSkins[NUM_WOOD_GIBS] = 
{
	"skins\\gibs\\debriswood1.dtx",
	"skins\\gibs\\debriswood2.dtx",
	"skins\\gibs\\debriswood3.dtx"
};

//GLASS
char *szGlassGibs[NUM_GLASS_GIBS] = 
{
	"models\\gibs\\gibglass1.abc",
	"models\\gibs\\gibglass2.abc",
};

char *szGlassSkins[NUM_GLASS_GIBS] = 
{
	"skins\\gibs\\gibglass1.dtx",
	"skins\\gibs\\gibglass2.dtx",
};

BEGIN_CLASS(CGib)
	ADD_DESTRUCTABLE_AGGREGATE()
	ADD_STRINGPROP(InitFilename, "")
	ADD_STRINGPROP(InitSkin, "")
	ADD_LONGINTPROP(InitFlags, (FLAG_VISIBLE | FLAG_RAYHIT | FLAG_GRAVITY | FLAG_SHADOW))
	ADD_STRINGPROP(InitAnim,"")
	ADD_REALPROP(HitPoints, 20.0f)
	ADD_REALPROP(ObjectMass, 10.0f)
	ADD_BOOLPROP(Kickable, DTRUE)
END_CLASS_DEFAULT(CGib, B2BaseClass, NULL, NULL)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGib::CGib() : B2BaseClass(OT_MODEL)
{
	CServerDE* pServerDE = GetServerDE();

	AddAggregate(&m_damage);

	m_szInitAnim[0] = '\0';
	m_fMass = 3.0f;
	m_fHitPoints = 10.0f;

	m_dwGibType = 0;
	m_bGibbed = DFALSE;

	m_fPitchVel = pServerDE->Random(-MATH_CIRCLE, MATH_CIRCLE) / 3.0f;
	m_fYawVel	= pServerDE->Random(-MATH_CIRCLE, MATH_CIRCLE) / 3.0f;
	m_fRollVel	= pServerDE->Random(-MATH_CIRCLE, MATH_CIRCLE) / 3.0f;
	m_fPitch = 0.0;
	m_fYaw = 0.0;
	m_fRoll = 0.0;
	m_fLastTime = 0.0;

	m_hSmokeTrail = DNULL;
	m_hBloodTrail = DNULL;
	m_bAddSmokeTrail = DFALSE;
	m_bAddBloodTrail = DFALSE;

	m_bPlaySplat = DFALSE;

	VEC_INIT(m_vLastPos);

	m_hCorpseClass = DNULL;
	m_dwSurfType = SURFTYPE_FLESH;	// Default surface type
	m_nBounceCount = 0;

	m_bKickable = DTRUE;

	m_nAxisAlign = 0;

	m_bFirstUpdate = DTRUE;

	m_fRemoveTime = 0;

	m_fRotateTime = 0;
	m_bAddBloodSpurt = DFALSE;
}


CGib::~CGib()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (m_hSmokeTrail)
		pServerDE->RemoveObject(m_hSmokeTrail);

	if (m_hBloodTrail)
		pServerDE->RemoveObject(m_hBloodTrail);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGib::EngineMessageFn()
//
//	PURPOSE:	Handler for engine messages
//
// --------------------------------------------------------------------------- //

DDWORD CGib::EngineMessageFn(DDWORD messageID, void *pData, float fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			DDWORD dwRet = B2BaseClass::EngineMessageFn(messageID, pData, fData);

			if (fData == 1.0f)
				ReadProp((ObjectCreateStruct*)pData);

			PostPropRead((ObjectCreateStruct*)pData);

			return dwRet;
		}

		case MID_INITIALUPDATE:
		{
			InitialUpdate((DVector *)pData);
			break;
		}

		case MID_UPDATE:
		{
			if(m_dwGibType & GIB_SMALL || m_damage.IsDead())
				g_pServerDE->RemoveObject( m_hObject );
			else
				if (!Update())
					g_pServerDE->RemoveObject( m_hObject );
			break;
		}

		case MID_TOUCHNOTIFY:
		{
			HandleTouch((HOBJECT)pData, fData);
			break;
		}

	}

	return B2BaseClass::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGib::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

DDWORD CGib::ObjectMessageFn( HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead )
{
	CServerDE* g_pServerDE = GetServerDE();
	if (!g_pServerDE) return 0;

	switch(messageID)
	{
		case MID_DAMAGE:
		{
			DVector vDir;
				
			g_pServerDE->ReadFromMessageVector(hRead, &vDir);
			g_pServerDE->ResetRead(hRead);	// reset since I'm not reading all items.

			// Let Destructable aggregate do its job first...
			DDWORD ret = B2BaseClass::ObjectMessageFn(hSender, messageID, hRead);
			
			Damage(vDir);
			
			return ret;
		}

		default : break;
	}

	return B2BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGib::ReadProp()
//
//	PURPOSE:	Reads properties
//
// --------------------------------------------------------------------------- //

DBOOL CGib::ReadProp(ObjectCreateStruct *pStruct)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pStruct) return DFALSE;

	char buf[MAX_CS_FILENAME_LEN];
	GenericProp genProp;

	if (pServerDE->GetPropGeneric("Kickable", &genProp) == DE_OK)
	{
		m_bKickable = genProp.m_Bool;
	}

	pServerDE->GetPropReal("HitPoints", &m_fHitPoints);
	pServerDE->GetPropReal("ObjectMass", &m_fMass);

	// Read in the destroyed object props.
	buf[0] = '\0';
	pServerDE->GetPropString("InitFilename", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) 
	{
		_mbscpy((unsigned char*)pStruct->m_Filename, (const unsigned char*)buf);
		m_dwSurfType = SURFTYPE_PLASTIC;
	}

	buf[0] = '\0';
	pServerDE->GetPropString("InitSkin", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) _mbscpy((unsigned char*)pStruct->m_SkinName, (const unsigned char*)buf);

	long nLongVal;
	if (pServerDE->GetPropLongInt("InitFlags", &nLongVal) == DE_OK)
		pStruct->m_Flags = (DDWORD)nLongVal;

	buf[0] = '\0';
	pServerDE->GetPropString("InitAnim", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) _mbscpy((unsigned char*)m_szInitAnim, (const unsigned char*)buf);

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PostPropRead()
//
//	PURPOSE:	Update properties
//
// ----------------------------------------------------------------------- //

void CGib::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	pStruct->m_Flags = FLAG_VISIBLE | FLAG_RAYHIT | FLAG_GRAVITY | FLAG_TOUCH_NOTIFY;

	m_dwGibType = pStruct->m_UserData;

	if(pStruct->m_UserData & GIB_CORPSE)
	{
		m_dwSurfType = SURFTYPE_FLESH;

		m_fMass = 10.0f;
		m_fHitPoints = 50.0f;
	}
	else if(pStruct->m_UserData & GIB_HEAD || pStruct->m_UserData & GIB_ARM || pStruct->m_UserData & GIB_LEG)
	{
		m_fMass = 3.0f;
		m_fHitPoints = 10.0f;
	}
	else
	{
		if(pStruct->m_UserData & GIB_METAL)
		{
			m_dwSurfType = SURFTYPE_METAL;

			int nModel = (int)g_pServerDE->Random(0,NUM_METAL_GIBS - 1);

			if(pStruct->m_Filename[0] == DNULL)
				_mbscpy( (unsigned char*)pStruct->m_Filename, (const unsigned char*)szMetalGibs[nModel]);

			if(pStruct->m_SkinName[0] == DNULL)
				_mbscpy( (unsigned char*)pStruct->m_SkinName, (const unsigned char*)szMetalSkins[nModel]);
		}
		else if(pStruct->m_UserData & GIB_GLASS)
		{
			m_dwSurfType = SURFTYPE_GLASS;

			int nModel = (int)g_pServerDE->Random(0,NUM_GLASS_GIBS - 1);

			if(pStruct->m_Filename[0] == DNULL)
				_mbscpy( (unsigned char*)pStruct->m_Filename, (const unsigned char*)szGlassGibs[nModel]);

			if(pStruct->m_SkinName[0] == DNULL)
				_mbscpy( (unsigned char*)pStruct->m_SkinName, (const unsigned char*)szGlassSkins[nModel]);
		}
		else if(pStruct->m_UserData & GIB_WOOD)
		{
			m_dwSurfType = SURFTYPE_WOOD;

			int nModel = (int)g_pServerDE->Random(0,NUM_WOOD_GIBS - 1);

			if(pStruct->m_Filename[0] == DNULL)
				_mbscpy( (unsigned char*)pStruct->m_Filename, (const unsigned char*)szWoodGibs[nModel]);

			if(pStruct->m_SkinName[0] == DNULL)
				_mbscpy( (unsigned char*)pStruct->m_SkinName, (const unsigned char*)szWoodSkins[nModel]);
		}
		else if(pStruct->m_UserData & GIB_STONE)
		{
			m_dwSurfType = SURFTYPE_STONE;

			int nModel = (int)g_pServerDE->Random(0,NUM_STONE_GIBS - 1);

			if(pStruct->m_Filename[0] == DNULL)
				_mbscpy( (unsigned char*)pStruct->m_Filename, (const unsigned char*)szStoneGibs[nModel]);

			if(pStruct->m_SkinName[0] == DNULL)
				_mbscpy( (unsigned char*)pStruct->m_SkinName, (const unsigned char*)szStoneSkins[nModel]);
		}
		else if(pStruct->m_UserData & GIB_FLESH)
		{
			m_dwSurfType = SURFTYPE_FLESH;

			int nModel = (int)g_pServerDE->Random(0,NUM_FLESH_GIBS - 1);

			if(pStruct->m_Filename[0] == DNULL)
				_mbscpy( (unsigned char*)pStruct->m_Filename, (const unsigned char*)szFleshGibs[nModel]);

			if(pStruct->m_SkinName[0] == DNULL)
				_mbscpy( (unsigned char*)pStruct->m_SkinName, (const unsigned char*)szFleshSkins[nModel]);
		}

		m_fMass = 3.0f;
		m_fHitPoints = 10.0f;
	}

	// Is it smoking?
	if (pStruct->m_UserData & GIB_SMOKETRAIL)
	{
		m_bAddSmokeTrail = DTRUE;
	}

	if(pStruct->m_UserData & (GIB_BLOODTRAIL | GIB_FLESH))
	{
		m_bAddBloodTrail = DTRUE;
	}

	return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGib::InitialUpdate()
//
//	PURPOSE:	Handle initial update
//
// ----------------------------------------------------------------------- //

void CGib::InitialUpdate(DVector *pMovement)
{
	CServerDE* pServerDE = GetServerDE();

	m_damage.Init( m_hObject );

	// Setup damage stats...
	m_damage.SetMass( m_fMass );
	m_damage.SetHitPoints( m_fHitPoints );
	m_damage.SetMaxHitPoints( m_fHitPoints );
	m_damage.SetArmorPoints( 0.0f );
	m_damage.SetMaxArmorPoints( 0.0f );
	m_damage.SetApplyDamagePhysics(DTRUE);

	pServerDE->SetForceIgnoreLimit(m_hObject, 0.1f);

	if(m_szInitAnim[0])		//SCHLEGZ 3/6/98 12:30:43 PM: allow for setting corpses
	{
		pServerDE->SetModelAnimation(m_hObject,pServerDE->GetAnimIndex(m_hObject,m_szInitAnim));
	}
	pServerDE->SetModelLooping(m_hObject, DFALSE);

	if (m_bKickable)
	{
		DVector vDims;
		pServerDE->GetModelAnimUserDims(m_hObject, &vDims, pServerDE->GetModelAnimation(m_hObject));
		DFLOAT fMax = vDims.x;
		if (vDims.y > fMax) fMax = vDims.y;
		if (vDims.z > fMax) fMax = vDims.z;
		VEC_SET(vDims, fMax, fMax, fMax);

		pServerDE->SetObjectDims(m_hObject, &vDims);
	}

	if (m_bAddSmokeTrail)
	{
		ObjectCreateStruct theStruct;
		INIT_OBJECTCREATESTRUCT(theStruct);

		pServerDE->GetObjectPos(m_hObject, &theStruct.m_Pos);
		HCLASS hClass = pServerDE->GetClass("CClientSmokeTrail");

		CClientSmokeTrail* pTrail = DNULL;

		if (hClass)
		{
			pTrail = (CClientSmokeTrail*)pServerDE->CreateObject(hClass, &theStruct);
		}

		if (pTrail)
		{
			DVector vVel;
			pServerDE->GetVelocity(m_hObject, &vVel);
			pTrail->Setup(vVel, DFALSE);
			m_hSmokeTrail = pTrail->m_hObject;
		}
	}
/*
	if (m_bAddBloodTrail)
	{
		ObjectCreateStruct theStruct;
		INIT_OBJECTCREATESTRUCT(theStruct);

		pServerDE->GetObjectPos(m_hObject, &theStruct.m_Pos);
		HCLASS hClass = pServerDE->GetClass("CClientBloodTrail");

		CClientBloodTrail* pTrail = DNULL;

		if (hClass)
		{
			pTrail = (CClientBloodTrail*)pServerDE->CreateObject(hClass, &theStruct);
		}

		if (pTrail)
		{
			DVector vVel,vColor;
			VEC_SET(vColor,200.0f,0.0f,0.0f);

			pServerDE->GetVelocity(m_hObject, &vVel);
			pTrail->Setup(vVel, vColor);
			m_hBloodTrail = pTrail->m_hObject;
		}
	}
*/	
	if (m_dwSurfType)
	{
		DDWORD dwFlags = g_pServerDE->GetObjectUserFlags(m_hObject);
		dwFlags |= (m_dwSurfType << 24);
		g_pServerDE->SetObjectUserFlags(m_hObject,dwFlags);
	}

	if(m_dwGibType & GIB_CORPSE)	//so the corpse does not initially make a noise
		m_bPlaySplat = DTRUE;

	if (g_pBloodServerShell->IsMultiplayerGame())
	{
		m_fRemoveTime = g_pServerDE->GetTime() + GIB_REMOVETIME - 0.1f;
	}
	//scale the object down if small
	pServerDE->SetNextUpdate( m_hObject, 0.1f );
	pServerDE->SetDeactivationTime( m_hObject, 1.0f );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGib::Update
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //

DBOOL CGib::Update()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

	DVector vVel;
	DRotation rRot;

	DFLOAT fTime = pServerDE->GetTime();
	if (m_fRemoveTime && m_fRemoveTime < fTime) return DFALSE;

	if (m_bFirstUpdate)
	{
		// If any dim are significantly smaller than the others, set alignment so it'll come to 
		// rest aligned correctly.
		DVector vDims;
		pServerDE->GetObjectDims(m_hObject, &vDims);
		if (vDims.y < vDims.x * 0.7f)
			m_nAxisAlign = 2;
		if (vDims.x < vDims.y * 0.7f)
			m_nAxisAlign = 1;
		if (vDims.z < vDims.y * 0.7f)
			m_nAxisAlign = 3;

		m_bFirstUpdate = DFALSE;
	}

	pServerDE->GetObjectPos(m_hObject, &m_vLastPos);
	pServerDE->GetObjectRotation( m_hObject, &rRot );
	pServerDE->GetVelocity( m_hObject ,&vVel );


	if (m_bAddBloodSpurt && m_dwSurfType == SURFTYPE_FLESH)
	{
		DVector vSpurtVel;
		VEC_MULSCALAR(vSpurtVel, vVel, 0.1f);
		HMESSAGEWRITE hMessage = g_pServerDE->StartInstantSpecialEffectMessage(&m_vLastPos);
		g_pServerDE->WriteToMessageByte(hMessage, SFX_PARTICLEEXPLOSION_ID);
		g_pServerDE->WriteToMessageVector(hMessage, &m_vLastPos);
		g_pServerDE->WriteToMessageCompVector(hMessage, &vSpurtVel);
		g_pServerDE->EndMessage(hMessage);

		m_bAddBloodSpurt = DFALSE;
	}


	if(m_hSmokeTrail)
		pServerDE->SetObjectPos(m_hSmokeTrail, &m_vLastPos);

	if(m_hBloodTrail)
		pServerDE->SetObjectPos(m_hBloodTrail, &m_vLastPos);

	m_fRotateTime -= pServerDE->GetFrameTime();

	// If velocity slows enough, stop moving
	if (VEC_MAGSQR(vVel) <= 50.0 || m_fRotateTime <= 0.0f)
	{
		if(m_hSmokeTrail)
		{
			pServerDE->RemoveObject(m_hSmokeTrail);
			m_hSmokeTrail = DNULL;
		}

		if(m_hBloodTrail)
		{
			pServerDE->RemoveObject(m_hBloodTrail);
			m_hBloodTrail = DNULL;
		}

		m_fPitchVel = 0;
		m_fYawVel	= 0;
		m_fRollVel	= 0;

		// Align so that y axis is either straight up or down
		switch(m_nAxisAlign)
		{
			case 1:		// Yaw
			{
				m_fYaw = (m_fYaw > MATH_HALFPI && m_fYaw < MATH_HALFPI + MATH_PI) ? MATH_PI : 0;
			}
			break;

			case 2:		// Pitch
			{
				m_fPitch = (m_fPitch > MATH_HALFPI && m_fPitch < MATH_HALFPI + MATH_PI) ? MATH_PI : 0;
			}
			break;

			case 3:		// Roll
			{
				m_fRoll = (m_fRoll > MATH_HALFPI && m_fRoll < MATH_HALFPI + MATH_PI) ? MATH_PI : 0;
			}
			break;
		}

		pServerDE->SetupEuler(&rRot, m_fPitch, m_fYaw, m_fRoll);
		pServerDE->SetObjectRotation(m_hObject, &rRot);	

		if (g_pBloodServerShell->IsMultiplayerGame())
			pServerDE->SetNextUpdate( m_hObject, 0.01f );
		else
			pServerDE->SetNextUpdate( m_hObject, 0.0f );
	}
	else
	{
		if (m_fPitchVel != 0 || m_fYawVel != 0 || m_fRollVel != 0)
		{
			DFLOAT fDeltaTime = pServerDE->GetFrameTime();

			m_fPitch += m_fPitchVel * fDeltaTime;
			if (m_fPitch > MATH_CIRCLE) m_fPitch -= MATH_CIRCLE;

			m_fYaw += m_fYawVel * fDeltaTime;
			if (m_fYaw > MATH_CIRCLE) m_fYaw -= MATH_CIRCLE;

			m_fRoll += m_fRollVel * fDeltaTime;
			if (m_fRoll > MATH_CIRCLE) m_fRoll -= MATH_CIRCLE;

			pServerDE->SetupEuler(&rRot, m_fPitch, m_fYaw, m_fRoll);
			pServerDE->SetObjectRotation(m_hObject, &rRot);	
		}

		pServerDE->SetNextUpdate( m_hObject, 0.01f );
	}

	pServerDE->SetDeactivationTime( m_hObject, 1.0f );
	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGib::HandleTouch
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //

void CGib::HandleTouch(HOBJECT hObj, DFLOAT fData)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;


	DVector vPos;
	pServerDE->GetObjectPos(m_hObject, &vPos);

	// See if it should kick high..
	if (m_bKickable && IsBaseCharacter(hObj))
	{
		DVector vVel;
		pServerDE->GetVelocity(hObj, &vVel);

		if (VEC_MAGSQR(vVel) > 25.0f)
		{
			vVel.y += 300.0f;
			VEC_MULSCALAR(vVel, vVel, 2.0f);
			pServerDE->SetVelocity(m_hObject, &vVel);
			m_nBounceCount = 2;
			m_fPitchVel = pServerDE->Random(-MATH_CIRCLE, MATH_CIRCLE) / 3.0f;
			m_fYawVel	= pServerDE->Random(-MATH_CIRCLE, MATH_CIRCLE) / 3.0f;
			m_fRollVel	= pServerDE->Random(-MATH_CIRCLE, MATH_CIRCLE) / 3.0f;
			PlayBounceSound();
			if (m_dwSurfType == SURFTYPE_FLESH)
			{
				VEC_MULSCALAR(vVel, vVel, 0.02f);
				HMESSAGEWRITE hMessage = g_pServerDE->StartInstantSpecialEffectMessage(&vPos);
				g_pServerDE->WriteToMessageByte(hMessage, SFX_PARTICLEEXPLOSION_ID);
				g_pServerDE->WriteToMessageVector(hMessage, &vPos);
				g_pServerDE->WriteToMessageCompVector(hMessage, &vVel);
				g_pServerDE->EndMessage(hMessage);
			}

			m_fRotateTime = 1.0f;
			pServerDE->SetNextUpdate( m_hObject, 0.01f );
			return;
		}
	}

	// return if it hit a non solid object
	if (hObj != pServerDE->GetWorldObject() && !(pServerDE->GetObjectFlags(hObj) & FLAG_SOLID)) // Ignore non-solid objects
		return;

 	// Cast a ray from our last known position to see what we hit
	IntersectQuery iq;
	IntersectInfo  ii;
	DVector vVel;

	pServerDE->GetVelocity(m_hObject, &vVel);

    // If velocity is small then do not Cast segment...
    if ( ((int)vVel.x+(int)vVel.z) == 0 || ((int)vVel.x+(int)vVel.z) == -0 || --m_nBounceCount < 0)
    {
        return;
    }

	VEC_COPY(iq.m_From, m_vLastPos);			// Get start point at the last known position.
	VEC_MULSCALAR(iq.m_To, vVel, 1.1f);
	VEC_ADD(iq.m_To, iq.m_To, iq.m_From);	// Get destination point slightly past where we should be
	iq.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	iq.m_FilterFn = NULL;
	iq.m_pUserData = NULL;	

	// Hit something!
	if (pServerDE->IntersectSegment(&iq, &ii))
	{
    
		// Compute new velocity reflected off of the surface.
		DVector vNormal;
		VEC_COPY(vNormal, ii.m_Plane.m_Normal);

		DFLOAT r = VEC_DOT(vVel, vNormal);
		
		r *= 0.3f;

		if (r > -100.0f) 
			r = 0;
		else
		{
			PlayBounceSound();

			VEC_MULSCALAR(vNormal, vNormal, r);
			VEC_SUB(vVel, vVel, vNormal);

			// Adjust the bouncing..
			m_fPitchVel = pServerDE->Random(-MATH_CIRCLE, MATH_CIRCLE) / 3.0f;
			m_fYawVel	= pServerDE->Random(-MATH_CIRCLE, MATH_CIRCLE) / 3.0f;
			m_fRollVel	= pServerDE->Random(-MATH_CIRCLE, MATH_CIRCLE) / 3.0f;
		}

		VEC_MULSCALAR(vVel, vVel, 0.5f);	// Lose some energy in the bounce.
		pServerDE->SetVelocity(m_hObject, &vVel);

		m_fRotateTime = 1.0f;
		m_bAddBloodSpurt = DTRUE;
		pServerDE->SetNextUpdate( m_hObject, 0.01f );
	}

	return;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGib::PlayBounceSound()
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //

void CGib::PlayBounceSound()
{
	// Play a bounce sound...
	char szSound[MAX_CS_FILENAME_LEN+1];

	switch(m_dwSurfType)
	{
		case SURFTYPE_FLESH:
		{
			int index = GetRandom(1, NRES(5));
			sprintf(szSound, "sounds\\gibs\\flesh\\gib_impact%d.wav", index);
			break;
		}
		case SURFTYPE_PLASTIC:
		{
			sprintf(szSound, "sounds\\bounce\\pylon.wav");
			break;
		}
	}

	PlaySoundFromObject(m_hObject, szSound, 1000, SOUNDPRIORITY_MISC_LOW);
}

	
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGib::Damage()
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //

void CGib::Damage(DVector vDir)
{
	//if the gib is destroyed, create more possible
	if(m_damage.IsDead())
	{
		if(!m_bGibbed && !(m_dwGibType & GIB_SMALL) && !(m_dwGibType & GIB_FLESH))
		{
			if(m_dwGibType & GIB_CORPSE)
			{
				m_dwGibType = m_dwGibType ^ GIB_CORPSE;
				m_dwGibType |= GIB_FLESH;
			}
			else
				m_dwGibType |= GIB_SMALL;

			CreateGibs(m_hObject,m_dwGibType,m_damage.GetLastDamageType(),vDir, (int)m_fMass);
			m_bGibbed = DTRUE;
		}

		g_pServerDE->RemoveObject( m_hObject );
	}	

	return;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGib::CreateGibs()
//
//	PURPOSE:	Create more gibs
//
// ----------------------------------------------------------------------- //

void CGib::CreateGibs(HOBJECT hObject, DDWORD dwGibType, DBYTE dmgType, DVector vDmgDir, int nNumDebris)
{
	CServerDE* g_pServerDE = GetServerDE();

	int nItem;
	HCLASS hClass;
	DVector vPos, vVel, vParentDims;
	DRotation rRot;

	hClass = g_pServerDE->GetClass( "CGib" );
	if( !hClass )
		return;

	g_pServerDE->GetObjectPos( hObject, &vPos );
	g_pServerDE->GetObjectRotation( hObject, &rRot );

	// Get dims of this prop...
	g_pServerDE->GetObjectDims( hObject, &vParentDims );

	// Make the debris...
	for( nItem = 0; nItem < nNumDebris; nItem++ )
	{
		ObjectCreateStruct theStruct;
		INIT_OBJECTCREATESTRUCT(theStruct);

		theStruct.m_Flags = FLAG_VISIBLE | FLAG_RAYHIT | FLAG_GRAVITY | FLAG_TOUCH_NOTIFY;

		theStruct.m_UserData = dwGibType;
		if (dmgType == DAMAGE_TYPE_EXPLODE)
			theStruct.m_UserData |= GIB_SMOKETRAIL;

		if(dwGibType & GIB_FLESH)
			theStruct.m_UserData |= GIB_BLOODTRAIL;
			
		VEC_SET( theStruct.m_Pos, GetServerDE()->Random( -vParentDims.x, vParentDims.x ), GetServerDE()->Random( 0.1f, vParentDims.y ), 
			GetServerDE()->Random( -vParentDims.z, vParentDims.z ));
		VEC_ADD( theStruct.m_Pos, theStruct.m_Pos, vPos );

		// Make sure the velocity is somewhat upward...
		vVel.x = vDmgDir.x * g_pServerDE->Random(0.0f, 300.0f);
		vVel.y = g_pServerDE->Random(0.0f, 400.0f);
		vVel.z = vDmgDir.z * g_pServerDE->Random(0.0f, 300.0f);

		//Copy the rotation vector
		ROT_COPY(theStruct.m_Rotation,rRot);

		// Allocate an object...
		BaseClass* pObj = g_pServerDE->CreateObject( hClass, &theStruct );
		if( !pObj )
			break;

		// Randomly scale
		DVector vScale;
		vScale.x = (DFLOAT)g_pServerDE->Random(0.8f,1.2f);
		vScale.y = (DFLOAT)g_pServerDE->Random(0.8f,1.2f);
		vScale.z = (DFLOAT)g_pServerDE->Random(0.8f,1.2f);
		
		if(dwGibType & GIB_SMALL)
		{
			VEC_MULSCALAR(vScale,vScale,0.4f);
		}
		else
		{
			VEC_MULSCALAR(vScale,vScale,1.0f);
		}

		g_pServerDE->ScaleObject(pObj->m_hObject, &vScale);

		g_pServerDE->SetVelocity( pObj->m_hObject, &vVel );
	}
}



