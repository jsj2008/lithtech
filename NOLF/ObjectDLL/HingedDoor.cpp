// ----------------------------------------------------------------------- //
//
// MODULE  : HingedDoor.CPP
//
// PURPOSE : A HingedDoor object
//
// CREATED : 12/3/97
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HingedDoor.h"

BEGIN_CLASS(HingedDoor)
	ADD_BOOLPROP_FLAG(OpenAway, 1, 0)
	ADD_VECTORPROP_VAL_FLAG(RotationAngles, 0.0f, 90.0f, 0.0f, 0)//  Where to rotate to
	PROP_DEFINEGROUP(StateFlags, PF_GROUP4)
        ADD_BOOLPROP_FLAG(ActivateTrigger, LTTRUE, PF_GROUP4)
        ADD_BOOLPROP_FLAG(StartOpen, LTFALSE, PF_GROUP4)
        ADD_BOOLPROP_FLAG(TriggerClose, LTTRUE, PF_GROUP4)
        ADD_BOOLPROP_FLAG(RemainsOpen, LTTRUE, PF_GROUP4)
        ADD_BOOLPROP_FLAG(ForceMove, LTFALSE, PF_GROUP4)
	ADD_STRINGPROP(OpenSound, "Snd\\Doors\\03Start.wav")
	ADD_STRINGPROP(CloseSound, "Snd\\Doors\\03Start.wav")
	ADD_STRINGPROP(LockedSound, "Snd\\Doors\\03Locked.wav")
END_CLASS_DEFAULT(HingedDoor, RotatingDoor, NULL, NULL)

#define DEG2RAD(x)		(((x)*MATH_PI)/180.0f)



// ----------------------------------------------------------------------- //
// Sets up transforms for the hinged door.
// ----------------------------------------------------------------------- //
void SetupTransform_HingedDoor(ILTPreLight *pInterface,
	HPREOBJECT hObject,
	float fPercent,
    LTVector &vOutPos,
    LTRotation &rOutRotation)
{
    LTVector vStartAngles, vEndAngles, vAngles;
    LTVector vRotationPoint, vOriginalPos;
	GenericProp gProp;


	pInterface->GetPropGeneric(hObject, "RotationAngles", &gProp);
	vStartAngles = gProp.m_Vec;
	vEndAngles = -vStartAngles;

	pInterface->GetPropGeneric(hObject, "RotationPoint", &gProp);
	vRotationPoint = gProp.m_Vec;

	pInterface->GetWorldModelRuntimePos(hObject, vOriginalPos);

	vAngles = vStartAngles + (vEndAngles - vStartAngles) * fPercent;
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
//	ROUTINE:	HingedDoor::HingedDoor()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

HingedDoor::HingedDoor() : RotatingDoor()
{
    m_bOpeningNormal = LTTRUE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	HingedDoor::EngineMessageFn()
//
//	PURPOSE:	Handler for engine messages
//
// --------------------------------------------------------------------------- //

uint32 HingedDoor::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	switch (messageID)
	{
		case MID_PRECREATE:
		{
			// Need to call base class to have the object name read in before
			// we call ReadProp()

            uint32 dwRet = RotatingDoor::EngineMessageFn(messageID, pData, fData);

			if (fData == PRECREATE_WORLDFILE)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			return dwRet;
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
                uint32 dwRet = RotatingDoor::EngineMessageFn(messageID, pData, fData);
				InitialUpdate();
				return dwRet;
			}
		}
		break;

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

	return RotatingDoor::EngineMessageFn(messageID, pData, fData);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	HingedDoor::ReadProp()
//
//	PURPOSE:	Reads HingedDoor properties
//
// --------------------------------------------------------------------------- //

LTBOOL HingedDoor::ReadProp(ObjectCreateStruct *)
{
    ILTServer* pServerDE = GetServerDE();
    if (!pServerDE) return LTFALSE;

	pServerDE->GetPropBool("OpenAway", &m_bOpenAway);

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	HingedDoor::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

void HingedDoor::InitialUpdate()
{
    ILTServer* pServerDE = GetServerDE();
	if (!pServerDE) return;

	// Save original angles...

	m_vOriginalOpenAngles   = m_vOpenAngles;
	m_vOriginalOpenDir		= m_vOpenDir;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	HingedDoor::SetOpen()
//
//	PURPOSE:	Set the door to the closed state
//
// --------------------------------------------------------------------------- //

void HingedDoor::SetClosed(LTBOOL bInitialize)
{
    ILTServer* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (!bInitialize)
	{
		// Reset the open angles / direction...

		m_vOpenAngles = m_vOriginalOpenAngles;
		m_vOpenDir	  = m_vOriginalOpenDir;
	}

	RotatingDoor::SetClosed(bInitialize);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	HingedDoor::SetOpening()
//
//	PURPOSE:	Set the door to the opening state
//
// --------------------------------------------------------------------------- //

void HingedDoor::SetOpening()
{
	// Recalcualte the original position.  This is incase the door was
	// moved (e.g., was keyframed on an elevator)...

 	if (m_dwDoorState == DOORSTATE_CLOSED)
	{
        g_pLTServer->GetObjectPos(m_hObject, &m_vOriginalPos);
	}


	// If this is an open away from door, Determine what open angles and
	// direction to use...

	if (m_bOpenAway)
	{
		// To calculate the correct direction to open, calculate the position
		// of the door in both possible open positions.  Whichever direction
		// moves the door to the farthest position away from the activate
		// object's position is the direction we should open the door.

		if (m_hActivateObj)
		{
            LTVector vObjPos;
            g_pLTServer->GetObjectPos(m_hActivateObj, &vObjPos);

            LTVector vOldAngles(m_fPitch, m_fYaw, m_fRoll);

			// Calculate the door's open position if it opened normally...

			m_fPitch = m_vOriginalOpenAngles.x;
			m_fYaw	 = m_vOriginalOpenAngles.y;
			m_fRoll	 = m_vOriginalOpenAngles.z;

            LTVector vTestPos1, vTestPos2;
            LTRotation rTestRot;

			CalcPosAndRot(vTestPos1, rTestRot);

			// Calculate the door's open position if it opens in the direction
			// opposite to the normal direction...

			m_fPitch = m_vOriginalOpenAngles.x;
			m_fYaw	 = m_vOriginalOpenAngles.y - MATH_PI;
			m_fRoll	 = m_vOriginalOpenAngles.z;

			CalcPosAndRot(vTestPos2, rTestRot);

			// Restore real angles...

			m_fPitch = vOldAngles.x;
			m_fYaw	 = vOldAngles.y;
			m_fRoll	 = vOldAngles.z;

			// Set the direction to open the door...

			if (VEC_DISTSQR(vObjPos, vTestPos1) <
				VEC_DISTSQR(vObjPos, vTestPos2))
			{
                m_bOpeningNormal = LTFALSE;
				m_vOpenAngles.y	 = m_vOriginalOpenAngles.y - MATH_PI;
				m_vOpenDir.y     = -m_vOriginalOpenDir.y;
			}
			else
			{
                m_bOpeningNormal = LTTRUE;
				m_vOpenAngles	 = m_vOriginalOpenAngles;
				m_vOpenDir		 = m_vOriginalOpenDir;
			}
		}
	}

	RotatingDoor::SetOpening();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	HingedDoor::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void HingedDoor::Save(HMESSAGEWRITE hWrite, uint8 nType)
{
    ILTServer* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageByte(hWrite, m_bOpenAway);
	pServerDE->WriteToMessageVector(hWrite, &m_vOriginalOpenDir);
	pServerDE->WriteToMessageVector(hWrite, &m_vOriginalOpenAngles);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	HingedDoor::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void HingedDoor::Load(HMESSAGEREAD hRead, uint8 nType)
{
    ILTServer* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

    m_bOpenAway         = (LTBOOL) pServerDE->ReadFromMessageByte(hRead);
	pServerDE->ReadFromMessageVector(hRead, &m_vOriginalOpenDir);
	pServerDE->ReadFromMessageVector(hRead, &m_vOriginalOpenAngles);
}


void HingedDoor::SetLightAnimOpen()
{
	if(m_bOpeningNormal)
		ReallySetLightAnimPos(0.0f);
	else
		ReallySetLightAnimPos(1.0f);
}


float HingedDoor::GetRotatingLightAnimPercent()
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

	fClosedPercent = 0.5f;
	fOpenPercent = m_bOpeningNormal ? 0.0f : 1.0f;

	return fClosedPercent + (fOpenPercent - fClosedPercent) * fPercentOpen;
}