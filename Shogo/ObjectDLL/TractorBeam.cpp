// ----------------------------------------------------------------------- //
//
// MODULE  : TractorBeam.cpp
//
// PURPOSE : Ordog Tractor Beam Implementation
//
// CREATED : 2/24/98
//
// ----------------------------------------------------------------------- //

#include "TractorBeam.h"
#include "RiotMsgIds.h"
#include "cpp_server_de.h"
#include "ServerRes.h"
#include "RiotObjectUtilities.h"

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
//
//	CLASS:		TractorBeam
//
//	PURPOSE:	Ordog Tractor Beam Special Move
//
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

BEGIN_CLASS(TractorBeam)
END_CLASS_DEFAULT(TractorBeam, BaseClass, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TractorBeam::TractorBeam()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

TractorBeam::TractorBeam() : BaseClass (OT_MODEL)
{
	m_bTargetIsPlayer = DFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TractorBeam::~TractorBeam()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

TractorBeam::~TractorBeam()
{
	if( !g_pServerDE ) return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TractorBeam::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD TractorBeam::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	DDWORD dwRet;

	if( g_pServerDE )
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
				if (fData != INITIALUPDATE_SAVEGAME)
				{
					InitialUpdate();
				}
				break;
			}

			case MID_PRECREATE:
			{
				if (fData != PRECREATE_SAVEGAME)
				{
					dwRet = BaseClass::EngineMessageFn(messageID, pData, fData);
					PostPropRead ((ObjectCreateStruct*) pData);
					return dwRet;
				}
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
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TractorBeam::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

DBOOL TractorBeam::InitialUpdate()
{
	// set the beam's translucency and brightness

	g_pServerDE->SetObjectColor (m_hObject,1,1,1,0.58f);

	// set the beam's scale

	float nLength = VEC_DIST (m_info.vTo, m_info.vFrom);

	DVector vScale;
	VEC_SET (vScale, 3.5f, 3.5f, nLength);
	g_pServerDE->ScaleObject (m_hObject, &vScale);

	// find the midpoint of the originating point and the destination point

	DVector vMidPoint;
	VEC_SUB (vMidPoint, m_info.vTo, m_info.vFrom);
	VEC_DIVSCALAR (vMidPoint, vMidPoint, 2.0f);
	VEC_ADD (vMidPoint, vMidPoint, m_info.vFrom);

	// determine the beam's rotation

	DVector vUp;
	VEC_SET (vUp, 0, 1, 0);
	DVector vDir;
	VEC_SUB (vDir, m_info.vTo, m_info.vFrom);
	VEC_NORM (vDir);
	DRotation rot;
	g_pServerDE->AlignRotation (&rot, &vDir, &vUp);

	// set the beam's position and rotation

	g_pServerDE->SetObjectPos (m_hObject, &vMidPoint);
	g_pServerDE->SetObjectRotation (m_hObject, &rot);

	// set the next update

	g_pServerDE->SetNextUpdate (m_hObject, 0.001f);

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TractorBeam::Update()
//
//	PURPOSE:	Update
//
// ----------------------------------------------------------------------- //

DBOOL TractorBeam::Update()
{
	// if we're hooked to another player, update the destination point

	if (m_bTargetIsPlayer)
	{
		g_pServerDE->GetObjectPos (m_info.hObjectDst, &m_info.vTo);
	}

	// get direction vector

	g_pServerDE->GetObjectPos (m_info.hObjectSrc, &m_info.vFrom);
	DVector vDir;
	VEC_SUB (vDir, m_info.vTo, m_info.vFrom);
	
	// get halfway point

	DVector vLoc;
	VEC_DIVSCALAR (vLoc, vDir, 2.0f);
	VEC_ADD (vLoc, vLoc, m_info.vFrom);

	// normalize direction vector	
	
	VEC_NORM (vDir);

	// get length of beam
	
	float nLength = VEC_DIST (m_info.vFrom, m_info.vTo);

	// update length of beam

	DVector vScale;
	VEC_SET (vScale, 3.5f, 3.5f, nLength);
	g_pServerDE->ScaleObject (m_hObject, &vScale);

	// update beam's rotation

	DVector vUp;
	VEC_SET (vUp, 0, 1, 0);
	DRotation rot;
	g_pServerDE->AlignRotation (&rot, &vDir, &vUp);
	g_pServerDE->SetObjectRotation (m_hObject, &rot);

	// update beam's position

	g_pServerDE->SetObjectPos (m_hObject, &vLoc);

	// set the next update

	g_pServerDE->SetNextUpdate (m_hObject, 0.001f);

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TractorBeam::PostPropRead()
//
//	PURPOSE:	Fill in properties
//
// ----------------------------------------------------------------------- //

void TractorBeam::PostPropRead(ObjectCreateStruct* pStruct)
{
	if (pStruct)
	{
		HSTRING hstrFilename = g_pServerDE->FormatString (IDS_MODEL_BEAM);
		HSTRING hstrSkinName = g_pServerDE->FormatString (IDS_SKIN_BEAM);

		pStruct->m_ObjectType = OT_MODEL;
		pStruct->m_Flags = FLAG_VISIBLE | FLAG_MODELGOURAUDSHADE | FLAG_TOUCH_NOTIFY;
		SAFE_STRCPY(pStruct->m_Filename, hstrFilename ? g_pServerDE->GetStringData (hstrFilename) : "dummy string");
		SAFE_STRCPY(pStruct->m_SkinName, hstrSkinName ? g_pServerDE->GetStringData (hstrSkinName) : "dummy string");
		SAFE_STRCPY(pStruct->m_Name, "Tractor Beam");

		memcpy (&m_info, (BeamInfo*)pStruct->m_UserData, sizeof (BeamInfo));

		g_pServerDE->FreeString (hstrFilename);
		g_pServerDE->FreeString (hstrSkinName);

		// determine if the target object is a player object

		if (IsBaseCharacter (m_info.hObjectDst))
		{
			m_bTargetIsPlayer = DTRUE;
			g_pServerDE->GetObjectPos (m_info.hObjectDst, &m_info.vTo);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TractorBeam::Remove()
//
//	PURPOSE:	Turn off the tractor beam and remove it from the world
//
// ----------------------------------------------------------------------- //

void TractorBeam::Remove()
{
	// remove ourselves from the world

	g_pServerDE->RemoveObject (m_hObject);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TractorBeam::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void TractorBeam::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	m_info.Save(hWrite, dwSaveFlags);
	
	pServerDE->WriteToMessageByte(hWrite, m_bTargetIsPlayer);	
	pServerDE->WriteToMessageByte(hWrite, 0);	
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TractorBeam::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void TractorBeam::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	DBYTE dummyByte;

	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	m_info.Load(hRead, dwLoadFlags);
	
	m_bTargetIsPlayer = (DBOOL) pServerDE->ReadFromMessageByte(hRead);	
	dummyByte = pServerDE->ReadFromMessageByte(hRead);	
}