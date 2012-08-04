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
#include "RiotObjectUtilities.h"
#include "PolyGrid.h"
#include "BaseCharacter.h"
#include "SFXMsgIds.h"

#define UPDATE_DELTA					0.01f
#define LIQUID_GRAVITY					-200.0f
#define TRIGGER_MSG_ON					"ON"
#define TRIGGER_MSG_OFF					"OFF"

BEGIN_CLASS(VolumeBrush)
	ADD_VISIBLE_FLAG(0, 0)
	ADD_REALPROP(Viscosity, 0.0f)
	ADD_BOOLPROP(Hidden, DFALSE)
	ADD_BOOLPROP(ShowSurface, DTRUE)
	ADD_BOOLPROP(FogEnable, DFALSE)
    ADD_REALPROP(FogFarZ, 300.0f)   
    ADD_REALPROP(FogNearZ,-100.0f)   
    ADD_COLORPROP(FogColor, 0.0f, 0.0f, 0.0f)   
    ADD_STRINGPROP(SpriteSurfaceName, "")   
    ADD_REALPROP(XScaleMin, 15.0f)   
    ADD_REALPROP(XScaleMax, 25.0f)   
    ADD_REALPROP(YScaleMin, 15.0f)   
    ADD_REALPROP(YScaleMax, 25.0f)   
    ADD_REALPROP(XScaleDuration, 60.0f)   
    ADD_REALPROP(YScaleDuration, 60.0f)   
	ADD_REALPROP(SurfaceHeight, 5.0f)
    ADD_COLORPROP(SurfaceColor1, 255.0f, 255.0f, 255.0f)   
    ADD_COLORPROP(SurfaceColor2, 255.0f, 255.0f, 255.0f)   
	ADD_REALPROP(SurfaceAlpha, 0.7f)
	ADD_LONGINTPROP(NumSurfacePolies, 160)
	ADD_VECTORPROP_VAL(Current, 0.0f, 0.0f, 0.0f)
	ADD_REALPROP(Damage, 0.0f)
	ADD_LONGINTPROP(DamageType, DT_CHOKE)
END_CLASS_DEFAULT(VolumeBrush, BaseClass, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::VolumeBrush()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

VolumeBrush::VolumeBrush() : BaseClass(OT_CONTAINER)
{
	m_hPlayerClass		= DNULL;
	m_dwSaveFlags		= 0;
	m_eContainerCode	= CC_VOLUME;
	m_fDamage			= 0.0f;
	m_eDamageType		= DT_UNSPECIFIED;
	m_bShowSurface		= DTRUE;
	m_fSurfaceHeight	= 5.0f;
	m_fViscosity		= 0.0f;
	m_hSurfaceObj		= DNULL;
	m_bHidden			= DFALSE;
	m_fGravity			= LIQUID_GRAVITY;

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
	m_dwNumSurfPolies = 160;
	m_fSurfAlpha = 0.7f;

	VEC_SET(m_vSurfaceColor1, 255.0f, 255.0f, 255.0f);
	VEC_SET(m_vSurfaceColor2, 255.0f, 255.0f, 255.0f);

	m_dwFlags = FLAG_TOUCH_NOTIFY | FLAG_GOTHRUWORLD | FLAG_FORCECLIENTUPDATE;
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
	DDWORD dwRet;

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
			dwRet = BaseClass::EngineMessageFn(messageID, pData, fData);
			if( fData == PRECREATE_WORLDFILE )
				ReadProp(( ObjectCreateStruct * )pData );
			PostPropRead(( ObjectCreateStruct * )pData );
			return dwRet;
			break;
		}

		case MID_INITIALUPDATE:
		{
			CServerDE* pServerDE = GetServerDE();
			if (!pServerDE) break;

			m_hPlayerClass = pServerDE->GetClass("CPlayerObj");

			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}

			CacheFiles();
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

	return BaseClass::EngineMessageFn(messageID, pData, fData);
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
		}
		break;
	}
	
	return BaseClass::ObjectMessageFn(hSender, messageID, hRead);
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

	if (m_bHidden && (stricmp(pMsg, TRIGGER_MSG_ON) == 0))
	{
		DDWORD dwFlags = pServerDE->GetObjectUserFlags(m_hObject);
		pServerDE->SetObjectUserFlags(m_hObject, dwFlags | USRFLG_VISIBLE);
		pServerDE->SetObjectFlags(m_hObject, m_dwSaveFlags);

		if (m_hSurfaceObj)
		{
			dwFlags = pServerDE->GetObjectUserFlags(m_hSurfaceObj);
			pServerDE->SetObjectUserFlags(m_hSurfaceObj, dwFlags | USRFLG_VISIBLE);
		}

		m_bHidden = DFALSE;
	}
	else if (!m_bHidden && (stricmp(pMsg, TRIGGER_MSG_OFF) == 0))
	{
		m_dwSaveFlags  = pServerDE->GetObjectFlags(m_hObject);
		DDWORD dwFlags = pServerDE->GetObjectUserFlags(m_hObject);
		pServerDE->SetObjectUserFlags(m_hObject, dwFlags & ~USRFLG_VISIBLE);
		pServerDE->SetObjectFlags(m_hObject, 0);

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

void VolumeBrush::ReadProp( ObjectCreateStruct *pStruct )
{
	char szData[MAX_CS_FILENAME_LEN+1];

	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pStruct) return;

	long nLongVal;

	pServerDE->GetPropBool( "Hidden", &m_bHidden );
	pServerDE->GetPropBool( "ShowSurface", &m_bShowSurface );

	pServerDE->GetPropBool( "FogEnable", &m_bFogEnable );
	pServerDE->GetPropReal( "FogFarZ", &m_fFogFarZ );
	pServerDE->GetPropReal( "FogNearZ", &m_fFogNearZ );
	pServerDE->GetPropVector( "FogColor", &m_vFogColor );

	if ( pServerDE->GetPropString( "SpriteSurfaceName", szData, MAX_CS_FILENAME_LEN ) == DE_OK )
		m_hstrSurfaceSprite = pServerDE->CreateString( szData );

	pServerDE->GetPropReal( "XScaleMin", &m_fXScaleMin );
	pServerDE->GetPropReal( "XScaleMax", &m_fXScaleMax );
	pServerDE->GetPropReal( "YScaleMin", &m_fYScaleMin );
	pServerDE->GetPropReal( "YScaleMax", &m_fYScaleMax );
	pServerDE->GetPropReal( "XScaleDuration", &m_fXScaleDuration );
	pServerDE->GetPropReal( "YScaleDuration", &m_fYScaleDuration );
	pServerDE->GetPropReal( "SurfaceHeight", &m_fSurfaceHeight );
	pServerDE->GetPropColor( "SurfaceColor1", &m_vSurfaceColor1 );
	pServerDE->GetPropColor( "SurfaceColor2", &m_vSurfaceColor2 );
	pServerDE->GetPropReal( "Viscosity", &m_fViscosity );
	pServerDE->GetPropVector( "Current", &m_vCurrent );
	pServerDE->GetPropReal( "Damage", &m_fDamage );
	pServerDE->GetPropReal( "SurfaceAlpha", &m_fSurfAlpha);

	if (pServerDE->GetPropLongInt("Code", &nLongVal) == DE_OK)
	{
		m_eContainerCode = (ContainerCode)nLongVal;
	}

	if (pServerDE->GetPropLongInt("DamageType", &nLongVal) == DE_OK)
	{
		m_eDamageType = (DamageType)nLongVal;
	}

	if (pServerDE->GetPropLongInt("NumSurfacePolies", &nLongVal) == DE_OK)
	{
		m_dwNumSurfPolies = (DDWORD)nLongVal;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::PostPropRead
//
//	PURPOSE:	Set some final values.
//
// ----------------------------------------------------------------------- //

void VolumeBrush::PostPropRead(ObjectCreateStruct *pStruct) 
{
	if (!pStruct) return;

	pStruct->m_Flags |= m_dwFlags;
	pStruct->m_ObjectType = OT_CONTAINER;
	SAFE_STRCPY(pStruct->m_Filename, pStruct->m_Name);
	pStruct->m_SkinName[0] = '\0';
	pStruct->m_ContainerCode = (D_WORD)m_eContainerCode;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

void VolumeBrush::InitialUpdate()
{
	D_WORD wColor;

	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	wColor = Color255VectorToWord( &m_vFogColor );

	// Tell the client about any special fx (fog)...

	HMESSAGEWRITE hMessage = pServerDE->StartSpecialEffectMessage(this);
	pServerDE->WriteToMessageByte(hMessage, SFX_VOLUMEBRUSH_ID);
	pServerDE->WriteToMessageByte(hMessage, (DBYTE)m_bFogEnable);
	pServerDE->WriteToMessageFloat(hMessage, m_fFogFarZ);
	pServerDE->WriteToMessageFloat(hMessage, m_fFogNearZ);
	pServerDE->WriteToMessageWord(hMessage, wColor);
	pServerDE->EndMessage(hMessage);


	// Save volume brush's initial flags...

	m_dwSaveFlags = pServerDE->GetObjectFlags(m_hObject);


	DDWORD dwUserFlags = pServerDE->GetObjectUserFlags(m_hObject);
	dwUserFlags |= USRFLG_IGNORE_PROJECTILES;
	if (!m_bHidden) dwUserFlags |= USRFLG_VISIBLE;

	pServerDE->SetObjectUserFlags(m_hObject, dwUserFlags);


	// Create the surface if necessary.  We only need to do updates if we have
	// a surface (in case somebody decides to move the brush, we need to update
	// the surface's position)...

	if (m_bShowSurface) 
	{
		CreateSurface();
		pServerDE->SetNextUpdate(m_hObject, UPDATE_DELTA);
	}


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

	if (!Equal(m_vLastPos, vPos))
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

		if (!m_bHidden)
		{
			pServerDE->SetObjectUserFlags(m_hSurfaceObj, USRFLG_VISIBLE);
		}

		pSurface->Setup(&vDims, &m_vSurfaceColor1, &m_vSurfaceColor2,
						m_hstrSurfaceSprite, m_fXScaleMin, m_fXScaleMax, 
						m_fYScaleMin, m_fYScaleMax, m_fXScaleDuration, 
						m_fYScaleDuration, fXPan, fYPan, m_fSurfAlpha,
						m_dwNumSurfPolies);
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
	if (!m_hPlayerClass || m_bHidden || !pServerDE || !pCPStruct || !pCPStruct->m_hObject) return;

	DFLOAT fUpdateDelta = pServerDE->GetFrameTime();

	// Check to see if this object is the player object...

	if (!pServerDE->IsKindOf(pServerDE->GetObjectClass(pCPStruct->m_hObject), m_hPlayerClass))
	{
		// Update velocity...

		// Dampen velocity and acceleration based on the viscosity of the container...

		if (m_fViscosity > 0.0f)
		{
			VEC_MULSCALAR(pCPStruct->m_Velocity, pCPStruct->m_Velocity, m_fViscosity);
			VEC_MULSCALAR(pCPStruct->m_Acceleration, pCPStruct->m_Acceleration, m_fViscosity);
		}


		// Add any current...

		// Make current relative to update delta (actually change the REAL velocity)...

		DVector vCurrent;
		VEC_MULSCALAR(vCurrent, m_vCurrent, fUpdateDelta);

		DVector vVel;
		pServerDE->GetVelocity(pCPStruct->m_hObject, &vVel);

		VEC_ADD(vVel, vVel, vCurrent);
		pServerDE->SetVelocity(pCPStruct->m_hObject, &vVel);

		
		// Do special liquid / zero gravity handling...

		if (IsLiquid(m_eContainerCode))
		{
			UpdateLiquidPhysics(pCPStruct);
		}
		else if (IsZeroGravity(m_eContainerCode))
		{
			UpdateZeroGravity(pCPStruct);
		}
	}

	
	// Update damage...

	// Make damage relative to update delta...

	DFLOAT fDamage = 0.0f;
	if (m_fDamage > 0.0f) 
	{
		fDamage = m_fDamage * fUpdateDelta;
	}

	if (fDamage)
	{
		DVector vDir;
		VEC_INIT(vDir);

		HMESSAGEWRITE hMessage = pServerDE->StartMessageToObject(this, pCPStruct->m_hObject, MID_DAMAGE);
		pServerDE->WriteToMessageVector(hMessage, &vDir);
		pServerDE->WriteToMessageFloat(hMessage, fDamage);
		pServerDE->WriteToMessageByte(hMessage, m_eDamageType);
		pServerDE->WriteToMessageObject(hMessage, m_hObject);
		pServerDE->EndMessage(hMessage);
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

void VolumeBrush::UpdateLiquidPhysics(ContainerPhysics* pCPStruct)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pCPStruct || !pCPStruct->m_hObject) return;

	// Apply liquid gravity to object...	
	
	if (pCPStruct->m_Flags & FLAG_GRAVITY)
	{
		pCPStruct->m_Flags &= ~FLAG_GRAVITY;
		pCPStruct->m_Acceleration.y += m_fGravity;
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
//	ROUTINE:	VolumeBrush::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void VolumeBrush::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hSurfaceObj);
	pServerDE->WriteToMessageVector(hWrite, &m_vCurrent);
	pServerDE->WriteToMessageVector(hWrite, &m_vSurfaceColor1);
	pServerDE->WriteToMessageVector(hWrite, &m_vSurfaceColor2);
	pServerDE->WriteToMessageVector(hWrite, &m_vLastPos);
	pServerDE->WriteToMessageVector(hWrite, &m_vFogColor);

	pServerDE->WriteToMessageHString(hWrite, m_hstrSurfaceSprite);

	pServerDE->WriteToMessageFloat(hWrite, m_fViscosity);
	pServerDE->WriteToMessageFloat(hWrite, m_fDamage);
	pServerDE->WriteToMessageFloat(hWrite, m_fSurfaceHeight);
	pServerDE->WriteToMessageFloat(hWrite, m_fGravity);
	pServerDE->WriteToMessageFloat(hWrite, m_fXScaleMin);
	pServerDE->WriteToMessageFloat(hWrite, m_fXScaleMax);
	pServerDE->WriteToMessageFloat(hWrite, m_fYScaleMin);
	pServerDE->WriteToMessageFloat(hWrite, m_fYScaleMax);
	pServerDE->WriteToMessageFloat(hWrite, m_fXScaleDuration);
	pServerDE->WriteToMessageFloat(hWrite, m_fYScaleDuration);
	pServerDE->WriteToMessageFloat(hWrite, m_fFogFarZ);
	pServerDE->WriteToMessageFloat(hWrite, m_fFogNearZ);
	pServerDE->WriteToMessageFloat(hWrite, m_fSurfAlpha);

	pServerDE->WriteToMessageDWord(hWrite, m_dwNumSurfPolies);
	pServerDE->WriteToMessageDWord(hWrite, m_dwFlags);
	pServerDE->WriteToMessageDWord(hWrite, m_dwSaveFlags);

	pServerDE->WriteToMessageByte(hWrite, m_eDamageType);
	pServerDE->WriteToMessageByte(hWrite, m_eContainerCode);
	pServerDE->WriteToMessageByte(hWrite, m_bShowSurface);
	pServerDE->WriteToMessageByte(hWrite, m_bFogEnable);
	pServerDE->WriteToMessageByte(hWrite, m_bHidden);
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

	pServerDE->ReadFromLoadSaveMessageObject(hRead, &m_hSurfaceObj);
	pServerDE->ReadFromMessageVector(hRead, &m_vCurrent);
	pServerDE->ReadFromMessageVector(hRead, &m_vSurfaceColor1);
	pServerDE->ReadFromMessageVector(hRead, &m_vSurfaceColor2);
	pServerDE->ReadFromMessageVector(hRead, &m_vLastPos);
	pServerDE->ReadFromMessageVector(hRead, &m_vFogColor);

	m_hstrSurfaceSprite = pServerDE->ReadFromMessageHString(hRead);

	m_fViscosity		= pServerDE->ReadFromMessageFloat(hRead);
	m_fDamage			= pServerDE->ReadFromMessageFloat(hRead);
	m_fSurfaceHeight	= pServerDE->ReadFromMessageFloat(hRead);
	m_fGravity			= pServerDE->ReadFromMessageFloat(hRead);
	m_fXScaleMin		= pServerDE->ReadFromMessageFloat(hRead);
	m_fXScaleMax		= pServerDE->ReadFromMessageFloat(hRead);
	m_fYScaleMin		= pServerDE->ReadFromMessageFloat(hRead);
	m_fYScaleMax		= pServerDE->ReadFromMessageFloat(hRead);
	m_fXScaleDuration	= pServerDE->ReadFromMessageFloat(hRead);
	m_fYScaleDuration	= pServerDE->ReadFromMessageFloat(hRead);
	m_fFogFarZ			= pServerDE->ReadFromMessageFloat(hRead);
	m_fFogNearZ			= pServerDE->ReadFromMessageFloat(hRead);
	m_fSurfAlpha		= pServerDE->ReadFromMessageFloat(hRead);

	m_dwNumSurfPolies	= pServerDE->ReadFromMessageDWord(hRead);
	m_dwFlags			= pServerDE->ReadFromMessageDWord(hRead);
	m_dwSaveFlags		= pServerDE->ReadFromMessageDWord(hRead);

	m_eDamageType		= (DamageType) pServerDE->ReadFromMessageByte(hRead);
	m_eContainerCode	= (ContainerCode) pServerDE->ReadFromMessageByte(hRead);
	m_bShowSurface		= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bFogEnable		= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bHidden			= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
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