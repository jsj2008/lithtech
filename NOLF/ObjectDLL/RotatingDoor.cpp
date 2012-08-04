// ----------------------------------------------------------------------- //
//
// MODULE  : RotatingDoor.CPP
//
// PURPOSE : A RotatingDoor object
//
// CREATED : 12/3/97
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "RotatingDoor.h"

BEGIN_CLASS(RotatingDoor)
	ADD_REALPROP_FLAG(MoveDist, 0.0f, PF_HIDDEN)	//  hide some of our
	ADD_VECTORPROP_FLAG(MoveDir, PF_HIDDEN)			//  parent's properties
	ADD_VECTORPROP_FLAG(RotationPoint, 0)			//  point to rotate around
	ADD_VECTORPROP_FLAG(RotationAngles, 0)			//  where to rotate to
	ADD_REALPROP(Speed, 50.0f)						//  movement speed
    ADD_BOOLPROP(BoxPhysics, LTFALSE)
	ADD_STRINGPROP(OpenSound, "Snd\\Doors\\02Start.wav")
	ADD_STRINGPROP(CloseSound, "Snd\\Doors\\02Start.wav")
	ADD_STRINGPROP(LockedSound, "Snd\\Doors\\02Locked.wav")
END_CLASS_DEFAULT(RotatingDoor, Door, NULL, NULL)

#define DEG2RAD(x)		(((x)*MATH_PI)/180.0f)




// ----------------------------------------------------------------------- //
// Global functions.
// ----------------------------------------------------------------------- //
void RotatingDoor_CalcPosAndRot(
    ILTMath *pMathLT,
    LTVector vOriginalPos,   // Original object position.
    LTVector vRotationPoint, // Point to rotate around.
    LTVector vAngles,        // Rotation angles.
    LTVector &vOutPos,
    LTRotation &rOutRot)
{
    LTVector vDes, vPoint;
    LTVector vOriginTranslation;
    LTMatrix mMat;

	pMathLT->SetupEuler(rOutRot, vAngles.x, vAngles.y, vAngles.z);
	vPoint = vOriginalPos - vRotationPoint;

	pMathLT->SetupRotationMatrix(mMat, rOutRot);
	MatVMul_H(&vDes, &mMat, &vPoint);

	vOriginTranslation = vDes - vPoint;
	vOutPos = vOriginalPos + vOriginTranslation;
}

// ----------------------------------------------------------------------- //
// Sets up transforms for the hinged door.
// ----------------------------------------------------------------------- //
void SetupTransform_RotatingDoor(ILTPreLight *pInterface,
	HPREOBJECT hObject,
	float fPercent,
    LTVector &vOutPos,
    LTRotation &rOutRotation)
{
    LTVector vStartAngles, vAngles;
    LTVector vRotationPoint, vOriginalPos;
	GenericProp gProp;

	// Note : This code is currently not being used, so it may not be entirely correct.
	// (Specifically, the calculation of vAngles used to include an interpolation from
	// a starting angle to an ending angle, and this object only has one set of rotation 
	// angles..  In addition, Euler angles don't like interpolating, so this really 
	// won't work correctly in many situations without a conversion to/from a quaternion.)

	pInterface->GetPropGeneric(hObject, "RotationAngles", &gProp);
	vStartAngles = gProp.m_Vec;

	pInterface->GetPropGeneric(hObject, "RotationPoint", &gProp);
	vRotationPoint = gProp.m_Vec;

	pInterface->GetWorldModelRuntimePos(hObject, vOriginalPos);

	vAngles = vStartAngles * fPercent;
	vAngles.x = DEG2RAD(vAngles.x);
	vAngles.y = DEG2RAD(vAngles.y);
	vAngles.z = DEG2RAD(vAngles.z);

	RotatingDoor_CalcPosAndRot(
		pInterface->GetMathLT(),
		vOriginalPos,
		vRotationPoint,
		vAngles,
		vOutPos,
		rOutRotation);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingDoor::RotatingDoor()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

RotatingDoor::RotatingDoor() : Door()
{
	m_vRotationPoint.Init();
	m_vRotPtOffset.Init();
	m_vRotationAngles.Init();
	m_vOpenAngles.Init();
	m_vClosedAngles.Init();
	m_vOriginalPos.Init();
	m_vOpenDir.Init();

	m_bStuck = LTFALSE;

	m_fPitch = 0.0f;
	m_fYaw	 = 0.0f;
	m_fRoll	 = 0.0f;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingDoor::EngineMessageFn()
//
//	PURPOSE:	Handler for engine messages
//
// --------------------------------------------------------------------------- //

uint32 RotatingDoor::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	switch (messageID)
	{
		case MID_PRECREATE:
		{
			// Need to call base class to have the object name read in before
			// we call PostPropRead()

            uint32 dwRet = Door::EngineMessageFn(messageID, pData, fData);

			if (fData == 1.0f)
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
			{
				InitialUpdate();
			}
			break;
		}

		case MID_SAVEOBJECT:
		{
            Save((HMESSAGEWRITE)pData, (uint8)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((HMESSAGEREAD)pData, (uint8)fData);
		}
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

LTBOOL RotatingDoor::ReadProp(ObjectCreateStruct *)
{
    g_pLTServer->GetPropVector("RotationAngles", &m_vRotationAngles);
    g_pLTServer->GetPropVector("RotationPoint", &m_vRotationPoint);

    LTVector vTemp;
    g_pLTServer->GetPropRotationEuler("Rotation", &vTemp);

	m_fPitch = vTemp.x;
	m_fYaw	 = vTemp.y;
	m_fRoll	 = vTemp.z;

    return LTTRUE;
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
	// Set open/closed angle...

	VEC_SET(m_vClosedAngles, m_fPitch, m_fYaw, m_fRoll);
	m_vOpenAngles.x = DEG2RAD(m_vRotationAngles.x) + m_fPitch;
	m_vOpenAngles.y = DEG2RAD(m_vRotationAngles.y) + m_fYaw;
	m_vOpenAngles.z = DEG2RAD(m_vRotationAngles.z) + m_fRoll;


	// Save the object's original position...

    g_pLTServer->GetObjectPos(m_hObject, &m_vOriginalPos);


	// Calculate the rotation point offset (allows for the
	// door to be movied (keyframed) and still rotate correctly...

	m_vRotPtOffset = m_vRotationPoint - m_vOriginalPos;


	// The door must rotate at least 1 degree...

    const LTFLOAT c_fMinDelta = DEG2RAD(1.0f);


	// Determine direction to open door in X...

    LTFLOAT fOffset = m_vClosedAngles.x - m_vOpenAngles.x;

	if (fOffset > c_fMinDelta)
	{
		m_vOpenDir.x = -1.0f;
	}
	else if (fOffset < c_fMinDelta)
	{
		m_vOpenDir.x = 1.0f;
	}


	// Determine direction to open door in Y...

	fOffset = m_vClosedAngles.y - m_vOpenAngles.y;

	if (fOffset > c_fMinDelta)
	{
		m_vOpenDir.y = -1.0f;
	}
	else if (fOffset < c_fMinDelta)
	{
		m_vOpenDir.y = 1.0f;
	}


	// Determine direction to open door in Z...

	fOffset = m_vClosedAngles.z - m_vOpenAngles.z;

	if (fOffset > c_fMinDelta)
	{
		m_vOpenDir.z = -1.0f;
	}
	else if (fOffset < c_fMinDelta)
	{
		m_vOpenDir.z = 1.0f;
	}

}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingDoor::SetOpening()
//
//	PURPOSE:	Sets the door opening state
//
// --------------------------------------------------------------------------- //

void RotatingDoor::SetOpening()
{
	// Recalcualte the original position.  This is incase the door was
	// moved (e.g., was keyframed on an elevator)...

	if (m_dwDoorState == DOORSTATE_CLOSED)
	{
        g_pLTServer->GetObjectPos(m_hObject, &m_vOriginalPos);
	}

	Door::SetOpening();
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingDoor::SetOpen()
//
//	PURPOSE:	Sets the door open state
//
// --------------------------------------------------------------------------- //


void RotatingDoor::SetOpen(LTBOOL bInitialize)
{
	Door::SetOpen(bInitialize);
	m_bStuck = LTFALSE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingDoor::SetClosed()
//
//	PURPOSE:	Sets the door closed state
//
// --------------------------------------------------------------------------- //

void RotatingDoor::SetClosed(LTBOOL bInitialize)
{
	Door::SetClosed(bInitialize);
	m_bStuck = LTFALSE;
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
    LTBOOL bDoneInX = LTFALSE;
    LTBOOL bDoneInY = LTFALSE;
    LTBOOL bDoneInZ = LTFALSE;

    LTVector vOldAngles(m_fPitch, m_fYaw, m_fRoll);

	// Calculate new pitch, yaw, and roll...

	bDoneInX = CalcAngle(m_fPitch, m_vClosedAngles.x, m_vOpenAngles.x, m_vOpenDir.x, m_fSpeed);
	bDoneInY = CalcAngle(m_fYaw,   m_vClosedAngles.y, m_vOpenAngles.y, m_vOpenDir.y, m_fSpeed);
	bDoneInZ = CalcAngle(m_fRoll,  m_vClosedAngles.z, m_vOpenAngles.z, m_vOpenDir.z, m_fSpeed);


    LTVector vPos;
    LTRotation rRot;

	// Calcuate the new pos/rot for the door (testing object collisions)...
	// If we collide with an object in the new position, don't move
	// the door (if force move isn't set)...

    if (!CalcPosAndRot(vPos, rRot, LTTRUE) && !(m_dwStateFlags & DF_FORCEMOVE))
	{
		// Restore our angles...

		m_fPitch = vOldAngles.x;
		m_fYaw	 = vOldAngles.y;
		m_fRoll	 = vOldAngles.z;

		// Since we hit something, try to close...
		//SetClosing(); // Doesn't work...

		m_bStuck = LTTRUE;

		return;
	}

	// Set the object's new rotation and position...

    g_pLTServer->RotateObject(m_hObject, &rRot);
    g_pLTServer->MoveObject(m_hObject, &vPos);

	// Update the light animation.

	float fPercent = GetRotatingLightAnimPercent();
	ReallySetLightAnimPos(fPercent);

	if (bDoneInX && bDoneInY && bDoneInZ)
	{
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
    LTBOOL bDoneInX = LTFALSE;
    LTBOOL bDoneInY = LTFALSE;
    LTBOOL bDoneInZ = LTFALSE;

    LTVector vOldAngles(m_fPitch, m_fYaw, m_fRoll);

	// Calculate new pitch, yaw, and roll...

	bDoneInX = CalcAngle(m_fPitch, m_vOpenAngles.x, m_vClosedAngles.x, -m_vOpenDir.x, m_fClosingSpeed);
	bDoneInY = CalcAngle(m_fYaw,   m_vOpenAngles.y, m_vClosedAngles.y, -m_vOpenDir.y, m_fClosingSpeed);
	bDoneInZ = CalcAngle(m_fRoll,  m_vOpenAngles.z, m_vClosedAngles.z, -m_vOpenDir.z, m_fClosingSpeed);

    LTVector vPos;
    LTRotation rRot;


	// Calcuate the new pos/rot for the door (testing object collisions)...
	// If we collide with an object in the new position, don't move
	// the door (if force move isn't set)...

    if (!CalcPosAndRot(vPos, rRot, LTTRUE) && !(m_dwStateFlags & DF_FORCEMOVE))
	{
		// Restore our angles...

		m_fPitch = vOldAngles.x;
		m_fYaw	 = vOldAngles.y;
		m_fRoll	 = vOldAngles.z;

		// Since we hit something, try to open...
		//SetOpening(); // Doesn't work...

		return;
	}

	// Set the object's new rotation and position...

    g_pLTServer->RotateObject(m_hObject, &rRot);
    g_pLTServer->MoveObject(m_hObject, &vPos);

	// Update the light animation....

	float fPercent = GetRotatingLightAnimPercent();
	ReallySetLightAnimPos(fPercent);

	if (bDoneInX && bDoneInY && bDoneInZ)
	{
		SetClosed();
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingDoor::CalcPosAndRot()
//
//	PURPOSE:	Do the rotation calculation based on the current values of
//				m_fPitch, m_fYaw, and m_fRoll
//
// --------------------------------------------------------------------------- //

LTBOOL RotatingDoor::CalcPosAndRot(LTVector & vPos, LTRotation & rRot,
                                  LTBOOL bTestCollisions)
{
	RotatingDoor_CalcPosAndRot(
        g_pLTServer->GetMathLT(),
		m_vOriginalPos,
		m_vOriginalPos + m_vRotPtOffset /*m_vRotationPoint*/,
        LTVector(m_fPitch, m_fYaw, m_fRoll),
		vPos, rRot);

	if (bTestCollisions)
	{
        return !TestObjectCollision(LTNULL, vPos, rRot);
	}

    return LTTRUE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingDoor::CalcAngle()
//
//	PURPOSE:	Calculate the new value of fAngle
//
// --------------------------------------------------------------------------- //

LTBOOL RotatingDoor::CalcAngle(LTFLOAT & fAngle, LTFLOAT fInitial, LTFLOAT fTarget, LTFLOAT fDir, LTFLOAT fSpeed)
{
    LTBOOL bRet = LTFALSE; // Are we at the target angle?

    LTFLOAT fPercent = 1.0f - (fTarget - fAngle) / (fTarget - fInitial);
    LTFLOAT fAmount  = GetDoorWaveValue(fSpeed, fPercent, m_dwWaveform) * g_pLTServer->GetFrameTime();

	// Calculate percentage moved so far...

	if (fDir != 0.0f)
	{
		if (fDir > 0.0f)
		{
			if (fAngle < fTarget)
			{
				fAngle += fAmount;
			}
			else
			{
				fAngle = fTarget;
                bRet   = LTTRUE;
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
                bRet   = LTTRUE;
			}
		}
	}

	if (fDir != 0.0f)
	{
		if (fDir > 0.0f)
		{
			if (fAngle < fTarget)
			{
				fAngle += fAmount;
			}
			else
			{
				fAngle = fTarget;
                bRet   = LTTRUE;
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
                bRet   = LTTRUE;
			}
		}
	}
	else
	{
        bRet = LTTRUE;
	}

	return bRet;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingDoor::GetMoveTestPosRot()
//
//	PURPOSE:	Get the test position/rotation for the door
//
// --------------------------------------------------------------------------- //

LTBOOL RotatingDoor::GetMoveTestPosRot(LTVector & vTestPos, LTRotation & rTestRot)
{
    LTVector vFinalPos(0.0f, 0.0f, 0.0f);
    LTFLOAT fSpeed = 0.0f;

    LTVector vOldAngles(m_fPitch, m_fYaw, m_fRoll);

	switch (m_dwDoorState)
	{
		case DOORSTATE_CLOSING:
		{
			// Calculate new pitch, yaw, and roll...

			CalcAngle(m_fPitch, m_fPitch, m_vOpenAngles.x, m_vOpenDir.x, m_fSpeed);
			CalcAngle(m_fYaw,   m_fYaw,   m_vOpenAngles.y, m_vOpenDir.y, m_fSpeed);
			CalcAngle(m_fRoll,  m_fRoll,  m_vOpenAngles.z, m_vOpenDir.z, m_fSpeed);
		}
		break;

		case DOORSTATE_OPENING:
		{
			// Calculate new pitch, yaw, and roll...

			CalcAngle(m_fPitch, m_fPitch, m_vClosedAngles.x, -m_vOpenDir.x, m_fClosingSpeed);
			CalcAngle(m_fYaw,   m_fYaw,   m_vClosedAngles.y, -m_vOpenDir.y, m_fClosingSpeed);
			CalcAngle(m_fRoll,  m_fRoll,  m_vClosedAngles.z, -m_vOpenDir.z, m_fClosingSpeed);
		}
		break;

		default:
            return LTFALSE;
		break;
	}


	// Rotate the object...

	CalcPosAndRot(vTestPos, rTestRot);


	// Restore real angles...

	m_fPitch = vOldAngles.x;
	m_fYaw	 = vOldAngles.y;
	m_fRoll	 = vOldAngles.z;

    return LTTRUE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingDoor::TestObjectCollision()
//
//	PURPOSE:	Determine if the test object would collide with the door if
//				the door were oriented in the test position/rotation
//
// --------------------------------------------------------------------------- //

LTBOOL RotatingDoor::TestObjectCollision(HOBJECT hTest, LTVector vTestPos,
            LTRotation rTestRot, HOBJECT* pCollisionObj)
{
	// Door::TestObjectCollision(hTest, vTestPos, rTestRot);

    HOBJECT hCollisionObj = LTNULL;

	if (Door::TestObjectCollision(hTest, vTestPos, rTestRot, &hCollisionObj))
	{
		// If the test object is the activate object, allow the door to
		// collide if it is moving away from the activate object...

		if (hTest || m_hActivateObj == hCollisionObj)
		{
            LTVector vObjPos, vDoorCurPos;
            g_pLTServer->GetObjectPos(m_hActivateObj, &vObjPos);
            g_pLTServer->GetObjectPos(m_hObject, &vDoorCurPos);

			// If the door's new position is farther away from the touch object
			// then its current position, we'll assume there can't be a collision.

            LTFLOAT fCurDist  = VEC_DISTSQR(vDoorCurPos, vObjPos);
            LTFLOAT fTestDist = VEC_DISTSQR(vTestPos, vObjPos);

            //g_pLTServer->CPrint("Activate object collision!");
            //g_pLTServer->CPrint("CurDist <= TestDist == %s", (fCurDist <= fTestDist) ? "TRUE" : "FALSE");

			if (fCurDist <= fTestDist)
			{
                return LTFALSE;
			}
			else
			{
                return LTTRUE;
			}
		}
		else
		{
            return LTTRUE;
		}
	}

    return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingDoor::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void RotatingDoor::Save(HMESSAGEWRITE hWrite, uint8 nType)
{
	if (!hWrite) return;

    g_pLTServer->WriteToMessageVector(hWrite, &m_vRotationAngles);
    g_pLTServer->WriteToMessageVector(hWrite, &m_vRotationPoint);
    g_pLTServer->WriteToMessageVector(hWrite, &m_vRotPtOffset);
    g_pLTServer->WriteToMessageVector(hWrite, &m_vOpenAngles);
    g_pLTServer->WriteToMessageVector(hWrite, &m_vClosedAngles);
    g_pLTServer->WriteToMessageVector(hWrite, &m_vOriginalPos);
    g_pLTServer->WriteToMessageVector(hWrite, &m_vOpenDir);

    g_pLTServer->WriteToMessageFloat(hWrite, m_fPitch);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fYaw);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fRoll);

	SAVE_BOOL(m_bStuck);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingDoor::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void RotatingDoor::Load(HMESSAGEREAD hRead, uint8 nType)
{
	if (!hRead) return;

    g_pLTServer->ReadFromMessageVector(hRead, &m_vRotationAngles);
    g_pLTServer->ReadFromMessageVector(hRead, &m_vRotationPoint);
    g_pLTServer->ReadFromMessageVector(hRead, &m_vRotPtOffset);
    g_pLTServer->ReadFromMessageVector(hRead, &m_vOpenAngles);
    g_pLTServer->ReadFromMessageVector(hRead, &m_vClosedAngles);
    g_pLTServer->ReadFromMessageVector(hRead, &m_vOriginalPos);
    g_pLTServer->ReadFromMessageVector(hRead, &m_vOpenDir);

    m_fPitch    = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fYaw      = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fRoll     = g_pLTServer->ReadFromMessageFloat(hRead);

	LOAD_BOOL(m_bStuck);
}


void RotatingDoor::SetLightAnimOpen()
{
}


void RotatingDoor::SetLightAnimClosed()
{
	ReallySetLightAnimPos(0.5f);
}


float RotatingDoor::GetRotatingLightAnimPercent()
{
	float fOpenPercent, fClosedPercent, fPercentOpen, fMaxDiff, fTestDiff;
    uint32 iDim, iBestDim;

	// How far along are we?

	// Pick a dimension to use (we need a dimension it rotates in).
	iBestDim = 0;
	fMaxDiff = 0.0f;
	for(iDim=0; iDim < 3; iDim++)
	{
		fTestDiff = (float)fabs(m_vOpenAngles[iDim] - m_vClosedAngles[iDim]);
		if(fTestDiff > fMaxDiff)
		{
			iBestDim = iDim;
			fMaxDiff = fTestDiff;
		}
	}

	if(fMaxDiff < 0.001f)
	{
		return 0.0f;
	}

	// How close are we to being open?
	fPercentOpen = (GetCurAnglesDim(iBestDim) - m_vClosedAngles[iBestDim]) /
		(m_vOpenAngles[iBestDim] - m_vClosedAngles[iBestDim]);

	fClosedPercent	= 0.5f;
	fOpenPercent	= 0.0f;   // m_bOpeningNormal ? 0.0f : 1.0f;

	return fClosedPercent + (fOpenPercent - fClosedPercent) * fPercentOpen;
}


float RotatingDoor::GetCurAnglesDim(uint32 iDim)
{
	if(iDim == 0)
		return m_fPitch;
	else if(iDim == 1)
		return m_fYaw;
	else
		return m_fRoll;
}