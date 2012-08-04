// ----------------------------------------------------------------------- //
//
// MODULE  : VolumeBrush.cpp
//
// PURPOSE : VolumeBrush implementation
//
// CREATED : 1/29/98
//
// ----------------------------------------------------------------------- //

#include "VolumeBrush.h"
#include "cpp_server_de.h"
#include "ObjectUtilities.h"
#include "PolyGrid.h"
#include "BaseCharacter.h"
#include <stdio.h>
#include "ClientServerShared.h"
#include "SFXMsgIds.h"


void BPrint(char*);

#define UPDATE_DELTA			0.1f
#define DAMAGE_UPDATE_DELTA		0.1f
#define LIQUID_GRAVITY			-200.0f
#define TRIGGER_MSG_ON			"ON"
#define TRIGGER_MSG_OFF			"OFF"
#define TRIGGER_MSG_TOGGLE		"TOGGLE"


BEGIN_CLASS(VolumeBrush)
	ADD_VISIBLE_FLAG(0, 0)
	ADD_REALPROP(Viscosity, 0.0f)
	ADD_BOOLPROP(Hidden, DFALSE)
	ADD_BOOLPROP(ShowSurface, DTRUE)
    ADD_STRINGPROP(SpriteSurfaceName, "")   
	ADD_BOOLPROP(FogEnable, DFALSE)
    ADD_REALPROP(FogFarZ, 300.0f)   
    ADD_REALPROP(FogNearZ,-100.0f)   
    ADD_COLORPROP(FogColor, 0.0f, 0.0f, 0.0f)   
    ADD_REALPROP(XScaleMin, 15.0f)   
    ADD_REALPROP(XScaleMax, 25.0f)   
    ADD_REALPROP(YScaleMin, 15.0f)   
    ADD_REALPROP(YScaleMax, 25.0f)   
    ADD_REALPROP(XScaleDuration, 60.0f)   
    ADD_REALPROP(YScaleDuration, 60.0f)   
	ADD_REALPROP(SurfaceHeight, 2.0f)
    ADD_COLORPROP(SurfaceColor1, 255.0f, 255.0f, 255.0f)   
    ADD_COLORPROP(SurfaceColor2, 255.0f, 255.0f, 255.0f)   
	ADD_REALPROP(SurfaceAlpha, 0.7f)
	ADD_LONGINTPROP(Code, CC_NOTHING)
	ADD_VECTORPROP_VAL(Current, 0.0f, 0.0f, 0.0f)
	ADD_REALPROP(Damage, 0.0f)
	ADD_LONGINTPROP(DamageType, DAMAGE_TYPE_NORMAL)
	ADD_LONGINTPROP(NumSurfacePolies, 160)
	ADD_BOOLPROP(Locked, DFALSE)
	ADD_BOOLPROP(UnlockKeyRemove, DFALSE)
	ADD_STRINGPROP(UnlockKeyName, "")
END_CLASS_DEFAULT(VolumeBrush, B2BaseClass, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::VolumeBrush()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

VolumeBrush::VolumeBrush() : B2BaseClass(OT_CONTAINER)
{
	m_hPlayerClass		= DNULL;
	m_dwSaveFlags		= 0;
	m_eContainerCode	= CC_NOTHING;
	m_fDamage			= 0.0f;
	m_nDamageType		= DAMAGE_TYPE_NORMAL;
	m_bShowSurface		= DTRUE;
	m_fSurfaceHeight	= 2.0f;
	m_fSurfaceAlpha		= 0.7f;
	m_fViscosity		= 0.0f;
	m_hSurfaceObj		= DNULL;
	m_bHidden			= DFALSE;
	m_fGravity			= LIQUID_GRAVITY;
	m_fLastDamageTime	= 0.0f;

	VEC_INIT(m_vLastPos);
	VEC_INIT(m_vCurrent);

	m_bFogEnable	= DFALSE;
	m_fFogFarZ		= 300.0f;
	m_fFogNearZ		= -100.0f;
	VEC_INIT(m_vFogColor);

	// Surface related stuff...

	m_fXScaleMin = 15.0f;
	m_fXScaleMax = 25.0f;
	m_fYScaleMin = 15.0f;
	m_fYScaleMax = 25.0f;
	m_fXScaleDuration = 10.0f;
	m_fYScaleDuration = 10.0f;
	m_hstrSurfaceSprite = DNULL;

	m_dwNumSurfacePolies = 160;

	VEC_SET(m_vSurfaceColor1, 255.0f, 255.0f, 255.0f);
	VEC_SET(m_vSurfaceColor2, 255.0f, 255.0f, 255.0f);

	m_dwFlags = FLAG_TOUCH_NOTIFY | FLAG_FULLPOSITIONRES | FLAG_GOTHRUWORLD | FLAG_FORCECLIENTUPDATE;
	m_bLocked = DFALSE;
	m_bUnlockKeyRemove = DFALSE;
	m_hstrKeyName = DNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::~VolumeBrush
//
//	PURPOSE:	Deallocate
//
// ----------------------------------------------------------------------- //

VolumeBrush::~VolumeBrush()
{ 
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (m_hstrSurfaceSprite)
	{
		pServerDE->FreeString(m_hstrSurfaceSprite);
	}
	if (m_hstrKeyName)
	{
		pServerDE->FreeString(m_hstrKeyName);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD VolumeBrush::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			Update();
			break;
		}

		case MID_AFFECTPHYSICS:
		{
			UpdatePhysics((ContainerPhysics*)pData);
			break;
		}

		case MID_PRECREATE:
		{
			// Need to call base class to have the object name read in before
			// we call PostPropRead()

			DDWORD dwRet = B2BaseClass::EngineMessageFn(messageID, pData, fData);

			if ( fData == PRECREATE_WORLDFILE )
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
			
			if (pStruct)
			{
				pStruct->m_Flags |= m_dwFlags;
				pStruct->m_ObjectType = OT_CONTAINER;
				_mbscpy((unsigned char*)pStruct->m_Filename, (const unsigned char*)pStruct->m_Name);
				pStruct->m_SkinName[0] = '\0';
				pStruct->m_ContainerCode = (D_WORD)m_eContainerCode;
			}
			return dwRet;
		}
		break;

		case MID_INITIALUPDATE:
		{
//			if (fData != INITIALUPDATE_SAVEGAME)
//			{
				InitialUpdate((int)fData);
//			}

			CacheFiles();
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

	return B2BaseClass::EngineMessageFn(messageID, pData, fData);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::ObjectMessageFn()
//
//	PURPOSE:	Handler for server object messages.
//
// --------------------------------------------------------------------------- //

DDWORD VolumeBrush::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = GetServerDE();
	switch (messageID)
	{
		case MID_TRIGGER:
		{
			HSTRING hMsg = pServerDE->ReadFromMessageHString(hRead);
			HandleTrigger(hSender, hMsg);
			pServerDE->FreeString(hMsg);
			break;
		}

		case MID_KEYQUERYRESPONSE:
		{
			HSTRING hItemName = pServerDE->ReadFromMessageHString(hRead);
			DBOOL	bHaveItem = (DBOOL)pServerDE->ReadFromMessageByte(hRead);

			// Check the key & locked status
			if (m_bLocked && bHaveItem && pServerDE->CompareStringsUpper(hItemName, m_hstrKeyName))
			{
				m_bLocked = DFALSE;

				// Key is no longer needed, tell the sender to remove it
//				if (m_bUnlockKeyRemove)
//				{
//					HMESSAGEWRITE hMessage = pServerDE->StartMessageToObject((LPBASECLASS)this, hSender, MID_KEYREMOVE);
//					pServerDE->WriteToMessageHString(hMessage, m_hstrKeyName);
//					pServerDE->EndMessage(hMessage);
// 				}
			}
			pServerDE->FreeString(hItemName);
			break;
		}
		break;
	}
	
	return B2BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::TriggerMsg()
//
//	PURPOSE:	Handler for volume brush trigger messages.
//
// --------------------------------------------------------------------------- //

void VolumeBrush::HandleTrigger(HOBJECT hSender, HSTRING hMsg)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	char* pMsg = pServerDE->GetStringData(hMsg);
	if (!pMsg || !pMsg[0]) return;

	DBOOL bToggle = DFALSE;

	if (_mbsicmp((const unsigned char*)pMsg, (const unsigned char*)TRIGGER_MSG_TOGGLE) == 0)
	{
		bToggle = DTRUE;
	}

	if ((bToggle && m_bHidden) || _mbsicmp((const unsigned char*)pMsg, (const unsigned char*)TRIGGER_MSG_ON) == 0)
	{
		DDWORD dwFlags;
		pServerDE->SetObjectFlags(m_hObject, m_dwSaveFlags);

		dwFlags = pServerDE->GetObjectUserFlags(m_hObject);
		pServerDE->SetObjectUserFlags(m_hObject, dwFlags | USRFLG_VISIBLE);

		if (m_hSurfaceObj)
		{
			dwFlags = pServerDE->GetObjectUserFlags(m_hSurfaceObj);
			pServerDE->SetObjectUserFlags(m_hSurfaceObj, dwFlags | USRFLG_VISIBLE);
		}

		m_bHidden = DFALSE;
	}
	else if ((bToggle && !m_bHidden) || _mbsicmp((const unsigned char*)pMsg, (const unsigned char*)TRIGGER_MSG_OFF) == 0)
	{
		DDWORD dwFlags;
		pServerDE->SetObjectFlags(m_hObject, m_dwSaveFlags & ~FLAG_VISIBLE);

		dwFlags = pServerDE->GetObjectUserFlags(m_hObject);
		pServerDE->SetObjectUserFlags(m_hObject, dwFlags & ~USRFLG_VISIBLE);

		if (m_hSurfaceObj)
		{
			dwFlags = pServerDE->GetObjectUserFlags(m_hSurfaceObj);
			pServerDE->SetObjectUserFlags(m_hSurfaceObj, dwFlags & ~USRFLG_VISIBLE);
		}

		m_bHidden = DTRUE;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

void VolumeBrush::ReadProp(ObjectCreateStruct *pStruct)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pStruct) return;

	long nLongVal;
	char buf[MAX_CS_FILENAME_LEN];

	pServerDE->GetPropBool("Hidden", &m_bHidden);
	pServerDE->GetPropBool("ShowSurface", &m_bShowSurface);

	buf[0] = '\0';
	pServerDE->GetPropString("SpriteSurfaceName", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hstrSurfaceSprite = pServerDE->CreateString(buf);

	pServerDE->GetPropBool( "FogEnable", &m_bFogEnable );
	pServerDE->GetPropReal( "FogFarZ", &m_fFogFarZ );
	pServerDE->GetPropReal( "FogNearZ", &m_fFogNearZ );
	pServerDE->GetPropVector( "FogColor", &m_vFogColor );

	pServerDE->GetPropReal("XScaleMin", &m_fXScaleMin);
	pServerDE->GetPropReal("XScaleMax", &m_fXScaleMax);
	pServerDE->GetPropReal("YScaleMin", &m_fYScaleMin);
	pServerDE->GetPropReal("YScaleMax", &m_fYScaleMax);
	pServerDE->GetPropReal("XScaleDuration", &m_fXScaleDuration);
	pServerDE->GetPropReal("YScaleDuration", &m_fYScaleDuration);
	pServerDE->GetPropReal("SurfaceHeight", &m_fSurfaceHeight);
	pServerDE->GetPropVector("SurfaceColor1", &m_vSurfaceColor1);
	pServerDE->GetPropVector("SurfaceColor2", &m_vSurfaceColor2);
	pServerDE->GetPropReal("Viscosity", &m_fViscosity);
	pServerDE->GetPropReal( "SurfaceAlpha", &m_fSurfaceAlpha);

	if (pServerDE->GetPropLongInt("Code", &nLongVal) == DE_OK)
		m_eContainerCode = (ContainerCode)nLongVal;
	pServerDE->GetPropVector("Current", &m_vCurrent);
	pServerDE->GetPropReal("Damage", &m_fDamage);
	if (pServerDE->GetPropLongInt("DamageType", &nLongVal) == DE_OK)
		m_nDamageType = (DBYTE)nLongVal;

	pServerDE->GetPropBool("Locked", &m_bLocked);
	pServerDE->GetPropBool("UnlockKeyRemove", &m_bUnlockKeyRemove);

	buf[0] = '\0';
	pServerDE->GetPropString("UnlockKeyName", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hstrKeyName = pServerDE->CreateString(buf);

	if (pServerDE->GetPropLongInt("NumSurfacePolies", &nLongVal) == DE_OK)
	{
		m_dwNumSurfacePolies = (DDWORD)nLongVal;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

void VolumeBrush::InitialUpdate(int nData)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	// For save game restores, just recreate the surface
	if (nData == INITIALUPDATE_SAVEGAME)
		return;

	if (m_bShowSurface) 
	{
		CreateSurface();
	}

	m_hPlayerClass = pServerDE->GetClass("CPlayerObj");

	DDWORD dwUserFlags = m_bHidden ? 0 : USRFLG_VISIBLE;
	dwUserFlags |= USRFLG_SAVEABLE;

	pServerDE->SetObjectUserFlags(m_hObject, dwUserFlags);

	if (m_hSurfaceObj) 
	{
		pServerDE->SetObjectUserFlags(m_hSurfaceObj, dwUserFlags);
	}

	// Tell the client about any special fx (fog)...
	HMESSAGEWRITE hMessage = pServerDE->StartSpecialEffectMessage(this);
	pServerDE->WriteToMessageByte(hMessage, SFX_VOLUMEBRUSH_ID);
//pServerDE->WriteToMessageByte(hMessage, DTRUE);
	pServerDE->WriteToMessageByte(hMessage, (DBYTE)m_bFogEnable);
	pServerDE->WriteToMessageFloat(hMessage, m_fFogFarZ);
	pServerDE->WriteToMessageFloat(hMessage, m_fFogNearZ);
	pServerDE->WriteToMessageVector(hMessage, &m_vFogColor);
	pServerDE->EndMessage(hMessage);

	pServerDE->SetNextUpdate(m_hObject, 0.001f);


	// Save volume brush's initial flags...

	m_dwSaveFlags = pServerDE->GetObjectFlags(m_hObject);


	// Normalize viscosity (1 = no movement, 0 = full movement)...

	if (m_fViscosity < 0.0) m_fViscosity = 0.0f;
	else if (m_fViscosity > 1.0) m_fViscosity = 1.0f;

	// Okay, internally we really want it the opposite way (i.e., 1 = full
	// movement, 0 = no movement)...

	m_fViscosity = 1.0f - m_fViscosity;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::Update()
//
//	PURPOSE:	Update the brush
//
// ----------------------------------------------------------------------- //

void VolumeBrush::Update()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || m_bHidden) return;

	// Only do updates if we have a surface...

	if (m_hSurfaceObj)
	{
		pServerDE->SetNextUpdate(m_hObject, UPDATE_DELTA);
	}
	else
	{
		pServerDE->SetNextUpdate(m_hObject, 0.0f);
	}


	// See if we have moved...

	DVector vPos;
	pServerDE->GetObjectPos(m_hObject, &vPos);

	if (m_hSurfaceObj && !(m_vLastPos.x == vPos.x &&m_vLastPos.y == vPos.y && m_vLastPos.z == vPos.z))
	{
		VEC_COPY(m_vLastPos, vPos);

		// Set the surface to its new position...

		DVector vDims;
		pServerDE->GetObjectDims(m_hObject, &vDims);

		vPos.y += vDims.y - (m_fSurfaceHeight/2.0f); 

		pServerDE->SetObjectPos(m_hSurfaceObj, &vPos);		
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::CreateSurface()
//
//	PURPOSE:	Create the poly grid surface
//
// ----------------------------------------------------------------------- //

void VolumeBrush::CreateSurface()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	DVector vPos, vDims, vScale;
	VEC_INIT(vScale);

	pServerDE->GetObjectDims(m_hObject, &vDims);
	pServerDE->GetObjectPos(m_hObject, &vPos);

	DRotation rRot;
	pServerDE->GetObjectRotation(m_hObject, &rRot);

	VEC_COPY(m_vLastPos, vPos);

	vPos.y += vDims.y - (m_fSurfaceHeight/2.0f); 
	VEC_COPY(theStruct.m_Pos, vPos);
	ROT_COPY(theStruct.m_Rotation, rRot);

	HCLASS hClass = pServerDE->GetClass("PolyGrid");

	PolyGrid* pSurface = DNULL;

	if (hClass)
	{
		pSurface = (PolyGrid *)pServerDE->CreateObject(hClass, &theStruct);
	}

	if (pSurface)
	{
		m_hSurfaceObj = pSurface->m_hObject;
		vDims.y		  = m_fSurfaceHeight;

		DFLOAT fXPan = 1.0f + (m_vCurrent.x * 0.01f);
		DFLOAT fYPan = 1.0f + (m_vCurrent.y * 0.01f);

		pSurface->Setup(&vDims, &m_vSurfaceColor1, &m_vSurfaceColor2,
						m_hstrSurfaceSprite, m_fXScaleMin, m_fXScaleMax, 
						m_fYScaleMin, m_fYScaleMax, m_fXScaleDuration, 
						m_fYScaleDuration, fXPan, fYPan, m_fSurfaceAlpha, 
						m_dwNumSurfacePolies);

//		pServerDE->SetObjectColor(m_hSurfaceObj,1.0f,0,0,0.1f);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::UpdatePhysics()
//
//	PURPOSE:	Update the physics of the passed in object
//
// ----------------------------------------------------------------------- //

void VolumeBrush::UpdatePhysics(ContainerPhysics* pCPStruct)
{
	CServerDE* pServerDE = GetServerDE();
	if (m_bHidden || !pServerDE || !pCPStruct || !pCPStruct->m_hObject) return;

	DFLOAT fUpdateDelta = pServerDE->GetFrameTime();

	// Check to see if this object is the player object...

	if (!pServerDE->IsKindOf(pServerDE->GetObjectClass(pCPStruct->m_hObject), m_hPlayerClass))
	{
		// Check to see if this object is a character object...

		DBOOL bCharacter = DFALSE;
		HCLASS hBaseCharClass = pServerDE->GetClass("CBaseCharacter");

		if (pServerDE->IsKindOf(pServerDE->GetObjectClass(pCPStruct->m_hObject), hBaseCharClass))
		{
			bCharacter = DTRUE;

			if (m_bLocked)
			{
				// See if they have the key we need to unlock
				HMESSAGEWRITE hMessage = pServerDE->StartMessageToObject((LPBASECLASS)this, pCPStruct->m_hObject, MID_KEYQUERY);
				pServerDE->WriteToMessageHString(hMessage, m_hstrKeyName);
				pServerDE->EndMessage(hMessage);
			}
		}


		// Update velocity...

		// Dampen velocity and acceleration based on the viscosity of the container...

		if (m_fViscosity > 0.0f)
		{
			VEC_MULSCALAR(pCPStruct->m_Velocity, pCPStruct->m_Velocity, m_fViscosity);
			VEC_MULSCALAR(pCPStruct->m_Acceleration, pCPStruct->m_Acceleration, m_fViscosity);
		}


		// Do special liquid / zero gravity handling...

		if (IsLiquid(m_eContainerCode))
		{
			UpdateLiquidPhysics(pCPStruct, bCharacter);
		}

		
		// Add any current...

		// Make current relative to update delta (actually change the REAL velocity)...

		DVector vCurrent;
		VEC_MULSCALAR(vCurrent, m_vCurrent, fUpdateDelta);

		VEC_ADD(pCPStruct->m_Velocity, pCPStruct->m_Velocity, vCurrent);
	}
	
	// Update damage...

	// Make damage relative to update delta...

	if (m_fDamage)
	{
		DFLOAT fTime = pServerDE->GetTime();
		DFLOAT fDamageDeltaTime = fTime - m_fLastDamageTime;

		if (fDamageDeltaTime >= DAMAGE_UPDATE_DELTA || fTime == m_fLastDamageTime)
		{
			m_fLastDamageTime = fTime;

			DVector vDir;
			VEC_INIT(vDir);
	
			DamageObject(m_hObject, this, pCPStruct->m_hObject, m_fDamage * DAMAGE_UPDATE_DELTA, vDir, vDir, m_nDamageType);
		}
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::UpdateLiquidPhysics()
//
//	PURPOSE:	Update liquid specific physics of the passed in object
//				(really, under liquid physics)
//
// ----------------------------------------------------------------------- //

void VolumeBrush::UpdateLiquidPhysics(ContainerPhysics* pCPStruct, DBOOL bCharacter)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pCPStruct || !pCPStruct->m_hObject) return;


	// If we are affecting a BaseCharacter object, let it do its own update...

	if (bCharacter)
	{
		CBaseCharacter* pCharacter = (CBaseCharacter*)pServerDE->HandleToObject(pCPStruct->m_hObject);
		if (pCharacter)
		{
			pCharacter->UpdateInLiquid(this, pCPStruct);
		}
	}
	else  // Apply liquid gravity to object...	
	{
		if (pCPStruct->m_Flags & FLAG_GRAVITY)
		{
			pCPStruct->m_Flags &= ~FLAG_GRAVITY;
			pCPStruct->m_Acceleration.y += m_fGravity;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::UpdateZeroGravity()
//
//	PURPOSE:	Update zero gravity specific physics
//
// ----------------------------------------------------------------------- //

void VolumeBrush::UpdateZeroGravity(ContainerPhysics* pCPStruct)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pCPStruct || !pCPStruct->m_hObject) return;

	if (pCPStruct->m_Flags & FLAG_GRAVITY)
	{
		pCPStruct->m_Flags &= ~FLAG_GRAVITY;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::UndoVisocityCalculation()
//
//	PURPOSE:	Undo the viscosity calculation on this container physics
//				struct.
//
// ----------------------------------------------------------------------- //

void VolumeBrush::UndoViscosityCalculation(ContainerPhysics* pCPStruct)
{
	if (!pCPStruct) return;

	// Undo dampening...

	if (m_fViscosity > 0.0f)
	{
		VEC_DIVSCALAR(pCPStruct->m_Velocity, pCPStruct->m_Velocity, m_fViscosity);
		VEC_DIVSCALAR(pCPStruct->m_Acceleration, pCPStruct->m_Acceleration, m_fViscosity);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::CacheFiles
//
//	PURPOSE:	Cache resources used by this the object
//
// ----------------------------------------------------------------------- //

void VolumeBrush::CacheFiles()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	// {MD 9/23/98}
	if(!(pServerDE->GetServerFlags() & SS_CACHING))
		return;

	char* pFile = DNULL;
	if (m_hstrSurfaceSprite)
	{
		pFile = pServerDE->GetStringData(m_hstrSurfaceSprite);
		if (pFile)
		{
			pServerDE->CacheFile(FT_SPRITE, pFile);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void VolumeBrush::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	DFLOAT fTime = pServerDE->GetTime();
	
//	pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hSurfaceObj);

	pServerDE->WriteToMessageFloat(hWrite, m_fViscosity);
	pServerDE->WriteToMessageByte(hWrite, (DBYTE)m_eContainerCode);
	pServerDE->WriteToMessageVector(hWrite, &m_vCurrent);
	pServerDE->WriteToMessageFloat(hWrite, m_fDamage);
	pServerDE->WriteToMessageByte(hWrite, m_nDamageType);

	pServerDE->WriteToMessageFloat(hWrite, m_fSurfaceHeight);
	pServerDE->WriteToMessageByte(hWrite, m_bShowSurface);
	pServerDE->WriteToMessageVector(hWrite, &m_vSurfaceColor1);
	pServerDE->WriteToMessageVector(hWrite, &m_vSurfaceColor2);

	pServerDE->WriteToMessageVector(hWrite, &m_vLastPos);
	pServerDE->WriteToMessageByte(hWrite, m_bHidden);
	pServerDE->WriteToMessageFloat(hWrite, m_fGravity);
	pServerDE->WriteToMessageFloat(hWrite, (m_fLastDamageTime - fTime));
	pServerDE->WriteToMessageFloat(hWrite, m_fXScaleMin);

	pServerDE->WriteToMessageFloat(hWrite, m_fXScaleMax);
	pServerDE->WriteToMessageFloat(hWrite, m_fYScaleMin);
	pServerDE->WriteToMessageFloat(hWrite, m_fYScaleMax);
	pServerDE->WriteToMessageFloat(hWrite, m_fXScaleDuration);
	pServerDE->WriteToMessageFloat(hWrite, m_fYScaleDuration);

	pServerDE->WriteToMessageHString(hWrite, m_hstrSurfaceSprite);
	pServerDE->WriteToMessageDWord(hWrite, m_dwFlags);
	pServerDE->WriteToMessageDWord(hWrite, m_dwSaveFlags);
//		HOBJECT			m_hSurfaceObj;
	pServerDE->WriteToMessageByte(hWrite, m_bLocked);
	pServerDE->WriteToMessageByte(hWrite, m_bUnlockKeyRemove);
	pServerDE->WriteToMessageHString(hWrite, m_hstrKeyName);

	pServerDE->WriteToMessageByte(hWrite, m_bFogEnable);
	pServerDE->WriteToMessageFloat(hWrite, m_fFogFarZ);
	pServerDE->WriteToMessageFloat(hWrite, m_fFogNearZ);
	pServerDE->WriteToMessageVector(hWrite, &m_vFogColor);
	pServerDE->WriteToMessageFloat(hWrite, m_fSurfaceAlpha);
	pServerDE->WriteToMessageDWord(hWrite, m_dwNumSurfacePolies);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void VolumeBrush::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	DFLOAT fTime = pServerDE->GetTime();

//	pServerDE->ReadFromLoadSaveMessageObject(hRead, &m_hSurfaceObj);

	m_fViscosity			= pServerDE->ReadFromMessageFloat(hRead);
	m_eContainerCode		= (ContainerCode)pServerDE->ReadFromMessageByte(hRead);
	pServerDE->ReadFromMessageVector(hRead, &m_vCurrent);
	m_fDamage				= pServerDE->ReadFromMessageFloat(hRead);
	m_nDamageType			= pServerDE->ReadFromMessageByte(hRead);

	m_fSurfaceHeight		= pServerDE->ReadFromMessageFloat(hRead);
	m_bShowSurface			= pServerDE->ReadFromMessageByte(hRead);
	pServerDE->ReadFromMessageVector(hRead, &m_vSurfaceColor1);
	pServerDE->ReadFromMessageVector(hRead, &m_vSurfaceColor2);

	pServerDE->ReadFromMessageVector(hRead, &m_vLastPos);
	m_bHidden				= pServerDE->ReadFromMessageByte(hRead);
	m_fGravity				= pServerDE->ReadFromMessageFloat(hRead);
	m_fLastDamageTime		= pServerDE->ReadFromMessageFloat(hRead) + fTime;
	m_fXScaleMin			= pServerDE->ReadFromMessageFloat(hRead);

	m_fXScaleMax			= pServerDE->ReadFromMessageFloat(hRead);
	m_fYScaleMin			= pServerDE->ReadFromMessageFloat(hRead);
	m_fYScaleMax			= pServerDE->ReadFromMessageFloat(hRead);
	m_fXScaleDuration		= pServerDE->ReadFromMessageFloat(hRead);
	m_fYScaleDuration		= pServerDE->ReadFromMessageFloat(hRead);

	m_hstrSurfaceSprite		= pServerDE->ReadFromMessageHString(hRead);
	m_dwFlags				= pServerDE->ReadFromMessageDWord(hRead);
	m_dwSaveFlags			= pServerDE->ReadFromMessageDWord(hRead);

	m_bLocked				= pServerDE->ReadFromMessageByte(hRead);
	m_bUnlockKeyRemove		= pServerDE->ReadFromMessageByte(hRead);
	m_hstrKeyName			= pServerDE->ReadFromMessageHString(hRead);

	m_bFogEnable			= pServerDE->ReadFromMessageByte(hRead);
	m_fFogFarZ				= pServerDE->ReadFromMessageFloat(hRead);
	m_fFogNearZ				= pServerDE->ReadFromMessageFloat(hRead);
	pServerDE->ReadFromMessageVector(hRead, &m_vFogColor);
	m_fSurfaceAlpha			= pServerDE->ReadFromMessageFloat(hRead);
	m_dwNumSurfacePolies	= pServerDE->ReadFromMessageDWord(hRead);


	// Recreate the surface
	if (m_bShowSurface) 
	{
		CreateSurface();
	}

	DDWORD dwUserFlags = m_bHidden ? 0 : USRFLG_VISIBLE;
	dwUserFlags |= USRFLG_SAVEABLE;

	pServerDE->SetObjectUserFlags(m_hObject, dwUserFlags);

	if (m_hSurfaceObj) 
	{
		pServerDE->SetObjectUserFlags(m_hSurfaceObj, dwUserFlags);
	}

}

