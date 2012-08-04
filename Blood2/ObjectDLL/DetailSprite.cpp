// ----------------------------------------------------------------------- //
//
// MODULE  : DetailSprite.cpp
//
// PURPOSE : DetailSprite class - implementation
//
// CREATED : 2/28/98
//
// ----------------------------------------------------------------------- //

#include "cpp_server_de.h"
#include "DetailSprite.h"
#include "SharedDefs.h"
#include "ClientServerShared.h"
#include <mbstring.h>



BEGIN_CLASS(DetailSprite)
	ADD_STRINGPROP(Filename, "")
	ADD_REALPROP(ScaleX, 1.0f)
	ADD_REALPROP(ScaleY, 1.0f)
	ADD_BOOLPROP(FlushWithWorld, DFALSE)
	ADD_BOOLPROP(Rotatable, DTRUE)
	ADD_BOOLPROP(Chromakey, DFALSE)
	ADD_BOOLPROP(Repeat, DFALSE)
	ADD_LONGINTPROP(RepeatX, 1)
	ADD_LONGINTPROP(RepeatY, 1)
	ADD_REALPROP(SpacingX, 0.0f)
	ADD_REALPROP(SpacingY, 0.0f)
END_CLASS_DEFAULT(DetailSprite, B2BaseClass, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DetailSprite::DetailSprite
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

DetailSprite::DetailSprite() : B2BaseClass(OT_SPRITE)
{
	VEC_SET(m_vScale, 1.0f, 1.0f, 1.0f);
	m_bFlushWithWorld = DFALSE;
	m_bRotatable	  = DFALSE;
	m_bChromaKey      = DFALSE;
	m_hstrFilename	  = DNULL;
	m_bRepeat	      = DFALSE;
	m_dwRepeatX		  = 1;
	m_dwRepeatY		  = 1;
	m_fSpacingX		  = 0.0f;
	m_fSpacingY		  = 0.0f;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DetailSprite::~DetailSprite
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

DetailSprite::~DetailSprite()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	if (m_hstrFilename)
	{
		pServerDE->FreeString(m_hstrFilename);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DetailSprite::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD DetailSprite::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			CServerDE* pServerDE = BaseClass::GetServerDE();
			if (!pServerDE) break;

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct *)pData);

				ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
				if (pStruct)
				{
					pStruct->m_SkinName[0] = '\0';
					pStruct->m_NextUpdate = 0.1f;
					VEC_COPY(pStruct->m_Scale, m_vScale)

					pStruct->m_Flags |= FLAG_VISIBLE;

					if (m_bRotatable || m_bFlushWithWorld)
					{
						pStruct->m_Flags |= FLAG_ROTATEABLESPRITE;
					}

					if (m_bChromaKey)
					{
						pStruct->m_Flags |= FLAG_SPRITECHROMAKEY;
					}

					if (m_hstrFilename)
					{
						char* pFilename = pServerDE->GetStringData(m_hstrFilename);
						if (pFilename)
						{
							_mbscpy((unsigned char*)pStruct->m_Filename, (const unsigned char*)pFilename);
						}
					}
				}
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
				InitialUpdate((DVector *)pData);
		}
		break;

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


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DetailSprite::ReadProp()
//
//	PURPOSE:	Update Properties
//
// ----------------------------------------------------------------------- //

void DetailSprite::ReadProp(ObjectCreateStruct *pStruct)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !pStruct) return;

	char buf[MAX_CS_FILENAME_LEN];

	buf[0] = '\0';
	pServerDE->GetPropString("Filename", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hstrFilename = pServerDE->CreateString(buf);

	pServerDE->GetPropReal("ScaleX", &m_vScale.x);
	pServerDE->GetPropReal("ScaleY", &m_vScale.y);
	pServerDE->GetPropBool("Rotatable", &m_bRotatable);
	pServerDE->GetPropBool("FlushWithWorld", &m_bFlushWithWorld);
	pServerDE->GetPropBool("ChromaKey", &m_bChromaKey);
	pServerDE->GetPropBool("Repeat", &m_bRepeat);
	pServerDE->GetPropReal("SpacingX", &m_fSpacingX);
	pServerDE->GetPropReal("SpacingY", &m_fSpacingY);
	long nLongVal;
	pServerDE->GetPropLongInt("RepeatX", &nLongVal);
	m_dwRepeatX = nLongVal;
	pServerDE->GetPropLongInt("RepeatY", &nLongVal);
	m_dwRepeatY = nLongVal;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DetailSprite::InitialUpdate
//
//	PURPOSE:	Do initial updating
//
// ----------------------------------------------------------------------- //

DBOOL DetailSprite::InitialUpdate(DVector*)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject) return DFALSE;

	pServerDE->SetNextUpdate(m_hObject, (DFLOAT)0.0);

	// Mark this object as savable
	DDWORD dwFlags = pServerDE->GetObjectUserFlags(m_hObject);
	dwFlags |= USRFLG_SAVEABLE;
	pServerDE->SetObjectUserFlags(m_hObject, dwFlags);

	if (m_bFlushWithWorld)
	{
		// Align the sprite to the surface directly along the forward vector

		DRotation rRot;
		DVector vPos, vU, vR, vF;
		pServerDE->GetObjectPos(m_hObject, &vPos);
		pServerDE->GetObjectRotation(m_hObject, &rRot);
		pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

		// Determine where on the surface to place the sprite...

		IntersectInfo ii;
		IntersectQuery iq;

		VEC_COPY(iq.m_From, vPos);
		VEC_COPY(iq.m_Direction, vF);
		iq.m_Flags	 = IGNORE_NONSOLID;
		iq.m_FilterFn = DNULL;

		if (pServerDE->CastRay(&iq, &ii))
		{
			DVector vTemp;
			VEC_COPY(vPos, ii.m_Point);
			VEC_COPY(vTemp, ii.m_Plane.m_Normal);
			VEC_COPY(vF, vTemp);

			// Place just in front of the wall

			VEC_MULSCALAR(vTemp, vTemp, 0.25f);
			VEC_ADD(vPos, vPos, vTemp);

			pServerDE->SetObjectPos(m_hObject, &vPos);
		}

		// Make more if desired
		if (m_bRepeat)
		{
			DVector vTmpPos, vTmpPos2;
			unsigned int x, y;

			VEC_COPY(vTmpPos, vPos);
			VEC_MULSCALAR(vU, vU, m_fSpacingY);
			VEC_MULSCALAR(vR, vR, m_fSpacingX);

			// Add in the Y direction;
			for (y = 1; y < m_dwRepeatY; y++)
			{
				VEC_ADD(vTmpPos, vTmpPos, vU);
				CreateAnotherSprite(&vTmpPos, &rRot);
			}

			// now repeat in the X (and Y) directions
			VEC_COPY(vTmpPos, vPos);
			for (x = 1; x < m_dwRepeatX; x++)
			{
				VEC_ADD(vTmpPos, vTmpPos, vR);
				VEC_COPY(vTmpPos2, vTmpPos);
				for (y = 0; y < m_dwRepeatY; y++)
				{
					CreateAnotherSprite(&vTmpPos2, &rRot);
					VEC_ADD(vTmpPos2, vTmpPos2, vU);
				}
			}
		}

	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DetailSprite::CreateAnotherSprite
//
//	PURPOSE:	Creates another sprite the same as the current one.
//
// ----------------------------------------------------------------------- //

void DetailSprite::CreateAnotherSprite(DVector *pvPos, DRotation *prRot)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject || !m_hstrFilename) return;

	ObjectCreateStruct ocStruct;
	INIT_OBJECTCREATESTRUCT(ocStruct);

	_mbscpy((unsigned char*)ocStruct.m_Filename, (const unsigned char*)pServerDE->GetStringData(m_hstrFilename));
	ROT_COPY(ocStruct.m_Rotation, *prRot);
	VEC_COPY(ocStruct.m_Pos, *pvPos);
	VEC_COPY(ocStruct.m_Scale, m_vScale);
	ocStruct.m_Flags = pServerDE->GetObjectFlags(m_hObject);

	HCLASS hClass = pServerDE->GetClass("DetailSprite");

	DetailSprite *pSprite = DNULL;

	// Something is seriously wrong if hClass is NULL ;)
	if (hClass)
	{
		pSprite = (DetailSprite*)pServerDE->CreateObject(hClass, &ocStruct);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DetailSprite::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void DetailSprite::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageVector(hWrite, &m_vScale);
	pServerDE->WriteToMessageHString(hWrite, m_hstrFilename);
	pServerDE->WriteToMessageByte(hWrite, m_bRotatable);
	pServerDE->WriteToMessageByte(hWrite, m_bChromaKey);
	pServerDE->WriteToMessageByte(hWrite, m_bAlignedToWorld);
	pServerDE->WriteToMessageByte(hWrite, m_bFlushWithWorld);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DetailSprite::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void DetailSprite::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	pServerDE->ReadFromMessageVector(hRead, &m_vScale);
	m_hstrFilename		= pServerDE->ReadFromMessageHString(hRead);
	m_bRotatable		= pServerDE->ReadFromMessageByte(hRead);
	m_bChromaKey		= pServerDE->ReadFromMessageByte(hRead);
	m_bAlignedToWorld	= pServerDE->ReadFromMessageByte(hRead);	
	m_bFlushWithWorld	= pServerDE->ReadFromMessageByte(hRead);
}



