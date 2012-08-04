// ----------------------------------------------------------------------- //
//
// MODULE  : ScaleSprite.cpp
//
// PURPOSE : ScaleSprite class - implementation
//
// CREATED : 12/07/97
//
// ----------------------------------------------------------------------- //

#include "ScaleSprite.h"
#include "cpp_server_de.h"

BEGIN_CLASS(ScaleSprite)
	ADD_DESTRUCTABLE_AGGREGATE()
	ADD_STRINGPROP(Filename, "")
	ADD_VECTORPROP_VAL_FLAG(Dims, 20.0f, 20.0f, 1.0f, PF_DIMS | PF_LOCALDIMS) 
	ADD_COLORPROP(Color, 255.0f, 255.0f, 255.0f)
	ADD_REALPROP(Alpha, 1.0f)
	ADD_STRINGPROP(DamagedFilename, "")
	ADD_STRINGPROP(DestroyedFilename, "")
	ADD_REALPROP(ScaleX, 0.5f)
	ADD_REALPROP(ScaleY, 0.5f)
	ADD_BOOLPROP(FlushWithWorld, 0)
	ADD_BOOLPROP(Rotatable, 0)
	ADD_LONGINTPROP(AdditionalFlags, 0)
END_CLASS_DEFAULT(ScaleSprite, BaseClass, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScaleSprite::ScaleSprite
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

ScaleSprite::ScaleSprite() : BaseClass(OT_SPRITE)
{
	VEC_SET(m_vScale, 1.0f, 1.0f, 1.0f);
	VEC_SET(m_vColor, 1.0f, 1.0f, 1.0f);
	m_fAlpha			= 1.0f;
	m_bFlushWithWorld	= DFALSE;
	m_bRotatable		= DFALSE;
	m_dwAdditionalFlags	= 0;
	m_hstrDamagedFile	= DNULL;
	m_hstrDestroyedFile = DNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScaleSprite::~ScaleSprite
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

ScaleSprite::~ScaleSprite()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (m_hstrDestroyedFile)
	{
		pServerDE->FreeString(m_hstrDestroyedFile);
	}

	if (m_hstrDamagedFile)
	{
		pServerDE->FreeString(m_hstrDamagedFile);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScaleSprite::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD ScaleSprite::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	DDWORD dwRet;

	switch(messageID)
	{
		case MID_PRECREATE:
		{
			dwRet = BaseClass::EngineMessageFn(messageID, pData, fData);
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct *)pData);
				PostPropRead((ObjectCreateStruct *)pData);
			}
			return dwRet;
		}
		break;

		case MID_UPDATE:
		{
			Update();
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}

			CacheFiles();
		}
		break;

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

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScaleSprite::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

DDWORD ScaleSprite::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	switch(messageID)
	{
 		case MID_TRIGGER:
		{
			HSTRING hMsg = g_pServerDE->ReadFromMessageHString(hRead);
			char *pszMessage = g_pServerDE->GetStringData(hMsg);

			if (stricmp(pszMessage, "DESTROY") == 0)
			{
				SetDestroyed();
			} 
			else if (stricmp(pszMessage, "DAMAGE") == 0)
			{
				SetDamaged();
			}    
			
			g_pServerDE->FreeString(hMsg);
		}
		default : break;
	}

	return BaseClass::ObjectMessageFn (hSender, messageID, hRead);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScaleSprite::ReadProp()
//
//	PURPOSE:	Update Properties
//
// ----------------------------------------------------------------------- //

void ScaleSprite::ReadProp(ObjectCreateStruct *pStruct)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	GenericProp genProp;

	if (pServerDE->GetPropGeneric("DamagedFilename", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
			 m_hstrDamagedFile = pServerDE->CreateString(genProp.m_String);
	}

	if (pServerDE->GetPropGeneric("DestroyedFilename", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
			 m_hstrDestroyedFile = pServerDE->CreateString(genProp.m_String);
	}

	if (pServerDE->GetPropGeneric("ScaleX", &genProp) == DE_OK)
		m_vScale.x = genProp.m_Float;

	if (pServerDE->GetPropGeneric("ScaleY", &genProp) == DE_OK)
		m_vScale.y = genProp.m_Float;

	if (pServerDE->GetPropGeneric("Dims", &genProp) == DE_OK)
	{
		// Not used - This property is only to allow level designers
		// to see where the sprite is...

		// VEC_COPY(m_vDims, genProp.m_Vec);
	}

	if (pServerDE->GetPropGeneric("Alpha", &genProp) == DE_OK)
		m_fAlpha = genProp.m_Float;

	if (pServerDE->GetPropGeneric("Color", &genProp) == DE_OK)
	{
		VEC_COPY(m_vColor, genProp.m_Vec);
		VEC_MULSCALAR(m_vColor, m_vColor, 1.0f/255.0f);
	}

	if (pServerDE->GetPropGeneric("Rotatable", &genProp) == DE_OK)
		m_bRotatable = genProp.m_Bool;

	if (pServerDE->GetPropGeneric("FlushWithWorld", &genProp) == DE_OK)
		m_bFlushWithWorld = genProp.m_Bool;

	if (pServerDE->GetPropGeneric("AdditionalFlags", &genProp) == DE_OK)
		m_dwAdditionalFlags = (DDWORD)genProp.m_Long;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScaleSprite::PostPropRead()
//
//	PURPOSE:	Finalize some data.
//
// ----------------------------------------------------------------------- //

void ScaleSprite::PostPropRead(ObjectCreateStruct *pStruct)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	if (pStruct)
	{
		pStruct->m_SkinName[0] = '\0';
		pStruct->m_NextUpdate = 0.1f;
		VEC_COPY(pStruct->m_Scale, m_vScale)

		pStruct->m_Flags |= FLAG_VISIBLE;

		// Make sure the sprite is rotateable...

		if (m_bRotatable || m_bFlushWithWorld)
		{
			pStruct->m_Flags |= FLAG_ROTATEABLESPRITE;
		}

		pStruct->m_Flags |= m_dwAdditionalFlags;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScaleSprite::InitialUpdate
//
//	PURPOSE:	Do initial updating
//
// ----------------------------------------------------------------------- //

DBOOL ScaleSprite::InitialUpdate()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject) return DFALSE;

	// Do everything in Update (we need to make sure all the objects in
	// the world have been loaded)...

	pServerDE->SetNextUpdate(m_hObject, 0.001f);
	pServerDE->SetDeactivationTime(m_hObject, 0.0f);
	
	pServerDE->SetObjectColor(m_hObject, m_vColor.x, m_vColor.y, m_vColor.x, m_fAlpha);
	
	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScaleSprite::Update
//
//	PURPOSE:	Do one update
//
// ----------------------------------------------------------------------- //

DBOOL ScaleSprite::Update()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject) return DFALSE;

	pServerDE->SetNextUpdate(m_hObject, 0.0f);
	pServerDE->SetObjectState(m_hObject, OBJSTATE_AUTODEACTIVATE_NOW);

	// BUG - This isn't quite right.  Sometimes this works (flipping the sprite)
	// other times the sprite shouldn't be flipped...Not sure what the bug is.
	// For some reason the sprites are sometimes backwards...Get the rotation
	// so we can flip it...

	DRotation rRot;
	DVector vPos, vDir, vU, vR, vF;
	pServerDE->GetObjectPos(m_hObject, &vPos);
	pServerDE->GetObjectRotation(m_hObject, &rRot);
	pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

	if (m_bFlushWithWorld)
	{
		// Align the sprite to the surface directly behind the sprite 
		// (i.e., opposite the forward vector)...

		VEC_NORM(vF);
		VEC_MULSCALAR(vDir, vF, -1.0f);


		// Determine where on the surface to place the sprite...

		IntersectInfo iInfo;
		IntersectQuery qInfo;

		VEC_COPY(qInfo.m_From, vPos);
		VEC_COPY(qInfo.m_Direction, vDir);
		qInfo.m_Flags	 = IGNORE_NONSOLID | INTERSECT_OBJECTS;
		qInfo.m_FilterFn = DNULL;

		if (pServerDE->CastRay(&qInfo, &iInfo))
		{
			DVector vTemp;
			VEC_COPY(vPos, iInfo.m_Point);
			VEC_COPY(vDir, iInfo.m_Plane.m_Normal);

			// Place the sprite just above the surface...

			VEC_MULSCALAR(vTemp, vDir, 1.0f);
			VEC_ADD(vPos, vPos, vTemp);

			pServerDE->SetObjectPos(m_hObject, &vPos);
		}
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScaleSprite::SetDestroyed
//
//	PURPOSE:	Set to destroyed sprite
//
// ----------------------------------------------------------------------- //

void ScaleSprite::SetDestroyed()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject) return;

	if (m_hstrDestroyedFile)
	{
		pServerDE->SetObjectFilenames(m_hObject, pServerDE->GetStringData(m_hstrDestroyedFile), "");
	}
	else
	{
		pServerDE->RemoveObject(m_hObject);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScaleSprite::SetDamaged
//
//	PURPOSE:	Set to damaged sprite
//
// ----------------------------------------------------------------------- //

void ScaleSprite::SetDamaged()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject || !m_hstrDamagedFile) return;

	pServerDE->SetObjectFilenames(m_hObject, pServerDE->GetStringData(m_hstrDamagedFile), "");
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScaleSprite::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void ScaleSprite::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageVector(hWrite, &m_vScale);	
	pServerDE->WriteToMessageVector(hWrite, &m_vColor);	
	pServerDE->WriteToMessageFloat(hWrite, m_fAlpha);	
	pServerDE->WriteToMessageByte(hWrite, m_bFlushWithWorld);	
	pServerDE->WriteToMessageByte(hWrite, m_bRotatable);	
	pServerDE->WriteToMessageDWord(hWrite, m_dwAdditionalFlags);	
	pServerDE->WriteToMessageHString(hWrite, m_hstrDamagedFile);	
	pServerDE->WriteToMessageHString(hWrite, m_hstrDestroyedFile);	
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScaleSprite::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void ScaleSprite::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	pServerDE->ReadFromMessageVector(hRead, &m_vScale);	
	pServerDE->ReadFromMessageVector(hRead, &m_vColor);	
	m_fAlpha			= pServerDE->ReadFromMessageFloat(hRead);	
	m_bFlushWithWorld	= (DBOOL) pServerDE->ReadFromMessageByte(hRead);	
	m_bRotatable		= (DBOOL) pServerDE->ReadFromMessageByte(hRead);	
	m_dwAdditionalFlags = pServerDE->ReadFromMessageDWord(hRead);	
	m_hstrDamagedFile	= pServerDE->ReadFromMessageHString(hRead);	
	m_hstrDestroyedFile = pServerDE->ReadFromMessageHString(hRead);	
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScaleSprite::CacheFiles
//
//	PURPOSE:	Cache resources associated with this object
//
// ----------------------------------------------------------------------- //

void ScaleSprite::CacheFiles()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	char* pFile = DNULL;
	if (m_hstrDamagedFile)
	{
		pFile = pServerDE->GetStringData(m_hstrDamagedFile);
		if (pFile)
		{
			pServerDE->CacheFile(FT_SPRITE, pFile);
		}
	}

	if (m_hstrDestroyedFile)
	{
		pFile = pServerDE->GetStringData(m_hstrDestroyedFile);
		if (pFile)
		{
			pServerDE->CacheFile(FT_SPRITE, pFile);
		}
	}
}
