// ----------------------------------------------------------------------- //
//
// MODULE  : RotatingDoor.CPP
//
// PURPOSE : A RotatingDoor object
//
// CREATED : 12/3/97
//
// ----------------------------------------------------------------------- //

// Includes...

#include "RotatingDoor.h"
#include "ObjectUtilities.h"
#include "SoundTypes.h"

BEGIN_CLASS(RotatingDoor)
	ADD_REALPROP_FLAG(MoveDist, 0.0f, PF_HIDDEN)	//  hide some of our
	ADD_VECTORPROP_FLAG(MoveDir, PF_HIDDEN)			//  parent's properties
	ADD_VECTORPROP_FLAG(RotationPoint, 0)			//  point to rotate around
	ADD_VECTORPROP_FLAG(RotationAngles, 0)			//  where to rotate to
	ADD_BOOLPROP(BoxPhysics, DFALSE)				// Door does NOT use "box physics"
	ADD_BOOLPROP(PushPlayerBack, DTRUE)		
END_CLASS_DEFAULT(RotatingDoor, Door, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingDoor::RotatingDoor()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

RotatingDoor::RotatingDoor() : Door()
{
	VEC_INIT(m_vRotationPoint);
	VEC_INIT(m_vRotationAngles);
	VEC_INIT(m_vOpenAngles);
	VEC_INIT(m_vClosedAngles);
	VEC_INIT(m_vOriginalPos);

	m_nOpenDirX = 0;
	m_nOpenDirY = 0;
	m_nOpenDirZ = 0;

	m_fPitch = 0.0f;
	m_fYaw	 = 0.0f;
	m_fRoll	 = 0.0f;
	m_bPushPlayerBack = DTRUE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingDoor::EngineMessageFn()
//
//	PURPOSE:	Handler for engine messages
//
// --------------------------------------------------------------------------- //

DDWORD RotatingDoor::EngineMessageFn(DDWORD messageID, void *pData, float fData)
{
	switch (messageID)
	{
		case MID_PRECREATE:
		{
			// Need to call base class to have the object name read in before
			// we call PostPropRead()

			DDWORD dwRet = Door::EngineMessageFn(messageID, pData, fData);

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			// Convert speeds from degrees to radians
			m_fSpeed		= MATH_DEGREES_TO_RADIANS(m_fSpeed);
			m_fClosingSpeed	= MATH_DEGREES_TO_RADIANS(m_fClosingSpeed);

			return dwRet;
		}

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
				InitialUpdate();
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

	return Door::EngineMessageFn(messageID, pData, fData);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingDoor::ReadProp()
//
//	PURPOSE:	Reads RotatingDoor properties
//
// --------------------------------------------------------------------------- //

DBOOL RotatingDoor::ReadProp(ObjectCreateStruct *)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

	GenericProp genProp;

	if (g_pServerDE->GetPropGeneric("RotationAngles", &genProp) == DE_OK)
	{
		VEC_COPY(m_vRotationAngles, genProp.m_Vec);
	}

	if (g_pServerDE->GetPropGeneric("RotationPoint", &genProp) == DE_OK)
	{
		VEC_COPY(m_vRotationPoint, genProp.m_Vec);
	}

	DVector vTemp;
	pServerDE->GetPropRotationEuler("Rotation", &vTemp);

	m_fPitch = vTemp.x;
	m_fYaw	 = vTemp.y;
	m_fRoll	 = vTemp.z;


	if (g_pServerDE->GetPropGeneric("PushPlayerBack", &genProp) == DE_OK)
	{
		m_bPushPlayerBack = genProp.m_Bool;
	}


	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingDoor::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

void RotatingDoor::InitialUpdate()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	// Set open/closed angle...

	VEC_SET(m_vClosedAngles, m_fPitch, m_fYaw, m_fRoll);
	m_vOpenAngles.x = DEG2RAD(m_vRotationAngles.x) + m_fPitch;
	m_vOpenAngles.y = DEG2RAD(m_vRotationAngles.y) + m_fYaw;
	m_vOpenAngles.z = DEG2RAD(m_vRotationAngles.z) + m_fRoll;

	
	// Save the object's original position...

	pServerDE->GetObjectPos(m_hObject, &m_vOriginalPos);


	// The door must rotate at least 1 degree...
	
	const DFLOAT c_fMinDelta = DEG2RAD(1.0f);


	// Determine direction to open door in X...

	DFLOAT fOffset = m_vClosedAngles.x - m_vOpenAngles.x;

	if (fOffset > c_fMinDelta)
	{
		m_nOpenDirX = -1;
	}
	else if (fOffset < c_fMinDelta)
	{
		m_nOpenDirX = 1;
	}


	// Determine direction to open door in Y...

	fOffset = m_vClosedAngles.y - m_vOpenAngles.y;

	if (fOffset > c_fMinDelta)
	{
		m_nOpenDirY = -1;
	}
	else if (fOffset < c_fMinDelta)
	{
		m_nOpenDirY = 1;
	}


	// Determine direction to open door in Z...

	fOffset = m_vClosedAngles.z - m_vOpenAngles.z;

	if (fOffset > c_fMinDelta)
	{
		m_nOpenDirZ = -1;
	}
	else if (fOffset < c_fMinDelta)
	{
		m_nOpenDirZ = 1;
	}

	if (m_bPushPlayerBack)
		pServerDE->SetForceIgnoreLimit(m_hObject, 0.0f);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingDoor::Opening()
//
//	PURPOSE:	Handles the RotatingDoor opening state
//
// --------------------------------------------------------------------------- //

void RotatingDoor::Opening()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (!m_bStartedMoving)
	{
		m_bStartedMoving = DTRUE;
		StartSound(m_hstrOpenBusySound, DTRUE);
		SetPortalState(DTRUE);
	}

	DBOOL bDoneInX = DFALSE;
	DBOOL bDoneInY = DFALSE;
	DBOOL bDoneInZ = DFALSE;

	// Keep track of current rotation and position in case the door is blocked
	DRotation rCurRot;
	DVector vCurPos;
	pServerDE->GetObjectRotation(m_hObject, &rCurRot);
	pServerDE->GetObjectPos(m_hObject, &vCurPos);
	DFLOAT fLastPitch	= m_fPitch;
	DFLOAT fLastYaw		= m_fYaw;
	DFLOAT fLastRoll	= m_fRoll;


	// Calculate new pitch, yaw, and roll...
	bDoneInX = CalcAngle(m_fPitch, m_vClosedAngles.x, m_vOpenAngles.x, m_nOpenDirX, m_fSpeed);
	bDoneInY = CalcAngle(m_fYaw,   m_vClosedAngles.y, m_vOpenAngles.y, m_nOpenDirY, m_fSpeed);
	bDoneInZ = CalcAngle(m_fRoll,  m_vClosedAngles.z, m_vOpenAngles.z, m_nOpenDirZ, m_fSpeed);

	
	// Rotate the object...
	DoRotation();

	if (m_bDoorBlocked)
	{
		pServerDE->SetObjectRotation(m_hObject, &rCurRot);	
		pServerDE->MoveObject(m_hObject, &vCurPos);
		m_fPitch	= fLastPitch;
		m_fYaw		= fLastYaw;
		m_fRoll		= fLastRoll;
	}
	else if (bDoneInX && bDoneInY && bDoneInZ)
	{
		if (m_hstrOpenStopSound)
        {
			PlaySoundFromObject(m_hObject, pServerDE->GetStringData(m_hstrOpenStopSound), 1000, SOUNDPRIORITY_MISC_MEDIUM);
        }    
		SetOpen();
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingDoor::Closing()
//
//	PURPOSE:	Handles the RotatingDoor closing state
//
// --------------------------------------------------------------------------- //

void RotatingDoor::Closing()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	DBOOL bDoneInX = DFALSE;
	DBOOL bDoneInY = DFALSE;
	DBOOL bDoneInZ = DFALSE;

	// Keep track of current rotation and position in case the door is blocked
	DRotation rCurRot;
	DVector vCurPos;
	pServerDE->GetObjectRotation(m_hObject, &rCurRot);
	pServerDE->GetObjectPos(m_hObject, &vCurPos);
	DFLOAT fLastPitch	= m_fPitch;
	DFLOAT fLastYaw		= m_fYaw;
	DFLOAT fLastRoll	= m_fRoll;

	// Calculate new pitch, yaw, and roll...
	bDoneInX = CalcAngle(m_fPitch, m_vOpenAngles.x, m_vClosedAngles.x, -m_nOpenDirX, m_fClosingSpeed);
	bDoneInY = CalcAngle(m_fYaw,   m_vOpenAngles.y, m_vClosedAngles.y, -m_nOpenDirY, m_fClosingSpeed);
	bDoneInZ = CalcAngle(m_fRoll,  m_vOpenAngles.z, m_vClosedAngles.z, -m_nOpenDirZ, m_fClosingSpeed);

	
	// Rotate the object...
	DoRotation();

	if (m_bDoorBlocked)
	{
		pServerDE->SetObjectRotation(m_hObject, &rCurRot);	
		pServerDE->MoveObject(m_hObject, &vCurPos);
		m_fPitch	= fLastPitch;
		m_fYaw		= fLastYaw;
		m_fRoll		= fLastRoll;
	}
	else if (bDoneInX && bDoneInY && bDoneInZ)
	{
		if (m_hstrCloseStopSound)
        {
			PlaySoundFromObject(m_hObject, pServerDE->GetStringData(m_hstrCloseStopSound), 1000, SOUNDPRIORITY_MISC_MEDIUM);
        
        }            
		SetClosed();
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingDoor::DoRotation()
//
//	PURPOSE:	Do the rotation calculation based on the current values of
//				m_fPitch, m_fYaw, and m_fRoll
//
// --------------------------------------------------------------------------- //

void RotatingDoor::DoRotation()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	DVector vDes, vPoint, vPos;
	pServerDE->GetObjectPos(m_hObject, &vPos);

	DRotation rRot;
	pServerDE->SetupEuler(&rRot, m_fPitch, m_fYaw, m_fRoll);

	VEC_SUB(vPoint, m_vOriginalPos, m_vRotationPoint);

	DMatrix mat;
	pServerDE->SetupRotationMatrix(&mat, &rRot);

	MatVMul_H(&vDes, &mat, &vPoint);

	DVector vOriginTranslation;
	VEC_SUB(vOriginTranslation, vDes, vPoint);
	VEC_ADD(vPos, m_vOriginalPos, vOriginTranslation);

	// Set the object's new rotation and position...

	pServerDE->RotateObject(m_hObject, &rRot);	
	pServerDE->MoveObject(m_hObject, &vPos);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingDoor::CalcAngle()
//
//	PURPOSE:	Calculate the new value of fAngle
//
// --------------------------------------------------------------------------- //

DBOOL RotatingDoor::CalcAngle(DFLOAT & fAngle, DFLOAT fInitial, DFLOAT fTarget, int nDir, DFLOAT fSpeed)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

	DBOOL bRet = DFALSE; // Are we at the target angle?

	DFLOAT fPercent = 1.0f - (fTarget - fAngle) / (fTarget - fInitial);
	DFLOAT fAmount = GetWaveValue(fSpeed, fPercent, m_dwWaveform) * pServerDE->GetFrameTime();

	// Calculate percentage moved so far
	if (nDir != 0)
	{
		if (nDir > 0)
		{
			if (fAngle < fTarget)
			{
				fAngle += fAmount;
			}
			else
			{
				fAngle = fTarget;
				bRet   = DTRUE;
			}
		}
		else
		{
			if (fAngle > fTarget)
			{
				fAngle -= fAmount;
			}
			else
			{
				fAngle = fTarget;
				bRet   = DTRUE;
			}
		}
	}

	if (nDir != 0)
	{
		if (nDir > 0)
		{
			if (fAngle < fTarget)
			{
				fAngle += fAmount;
			}
			else
			{
				fAngle = fTarget;
				bRet   = DTRUE;
			}
		}
		else
		{
			if (fAngle > fTarget)
			{
				fAngle -= fAmount;
			}
			else
			{
				fAngle = fTarget;
				bRet   = DTRUE;
			}
		}
	}
	else
	{
		bRet = DTRUE;
	}

	return bRet;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingDoor::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void RotatingDoor::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageVector(hWrite, &m_vRotationAngles);	
	pServerDE->WriteToMessageVector(hWrite, &m_vRotationPoint);
	pServerDE->WriteToMessageVector(hWrite, &m_vOpenAngles);	
	pServerDE->WriteToMessageVector(hWrite, &m_vClosedAngles);	
	pServerDE->WriteToMessageVector(hWrite, &m_vOriginalPos);	

	pServerDE->WriteToMessageDWord(hWrite, m_nOpenDirX);
	pServerDE->WriteToMessageDWord(hWrite, m_nOpenDirY);
	pServerDE->WriteToMessageDWord(hWrite, m_nOpenDirZ);

	pServerDE->WriteToMessageFloat(hWrite, m_fPitch);
	pServerDE->WriteToMessageFloat(hWrite, m_fYaw);
	pServerDE->WriteToMessageFloat(hWrite, m_fRoll);

	pServerDE->WriteToMessageByte(hWrite, m_bPushPlayerBack);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingDoor::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void RotatingDoor::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	pServerDE->ReadFromMessageVector(hRead, &m_vRotationAngles);	
	pServerDE->ReadFromMessageVector(hRead, &m_vRotationPoint);
	pServerDE->ReadFromMessageVector(hRead, &m_vOpenAngles);	
	pServerDE->ReadFromMessageVector(hRead, &m_vClosedAngles);	
	pServerDE->ReadFromMessageVector(hRead, &m_vOriginalPos);	

	m_nOpenDirX = (short)pServerDE->ReadFromMessageDWord(hRead);
	m_nOpenDirY = (short)pServerDE->ReadFromMessageDWord(hRead);
	m_nOpenDirZ = (short)pServerDE->ReadFromMessageDWord(hRead);

	m_fPitch	= pServerDE->ReadFromMessageFloat(hRead);
	m_fYaw		= pServerDE->ReadFromMessageFloat(hRead);
	m_fRoll		= pServerDE->ReadFromMessageFloat(hRead);

	m_bPushPlayerBack = pServerDE->ReadFromMessageByte(hRead);
}

