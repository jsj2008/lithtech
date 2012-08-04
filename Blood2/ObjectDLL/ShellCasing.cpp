//----------------------------------------------------------
//
// MODULE  : SHELLCASING.CPP
//
// PURPOSE : defines classes for ejected shells
//
// CREATED : 1/29/98
//
//----------------------------------------------------------

// Includes....
#include <stdio.h>
#include <string.h>
#include "serverobj_de.h"
#include "ShellCasing.h"
#include "cpp_server_de.h"
#include "ObjectUtilities.h"
#include "generic_msg_de.h"
#include "SoundTypes.h"

void BPrint(char *msg);


DLink CShellCasing::m_Head;
DDWORD CShellCasing::m_dwNumShells = 0;


BEGIN_CLASS(CShellCasing)
END_CLASS_DEFAULT_FLAGS(CShellCasing, BaseClass, NULL, NULL, CF_HIDDEN)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShellCasing::CShellCasing
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

CShellCasing::CShellCasing() : BaseClass(OT_MODEL)
{
	m_hFiredFrom = 0;
	m_fExpireTime = 0.0f;
	m_bFiredFromLeft = DFALSE;
    m_bInVisible = DTRUE;

	if( m_dwNumShells == 0 )
	{
		dl_TieOff( &m_Head );
	}
	m_nBounceCount = 2;	// Set maximum bounces
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShellCasing::~CShellCasing
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CShellCasing::~CShellCasing()
{
	if( m_Link.m_pData && m_dwNumShells > 0 )
	{
		dl_Remove( &m_Link );
		m_dwNumShells--;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShellCasing::ObjectMessageFn
//
//	PURPOSE:	Handle object messages
//
// ----------------------------------------------------------------------- //

DDWORD CShellCasing::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	if( g_pServerDE )
	{
		switch(messageID)
		{
			case MID_DAMAGE:
			{
				g_pServerDE->RemoveObject(m_hObject);
				break;
			}
			default: break;
		}
	}

	return BaseClass::ObjectMessageFn (hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShellCasing::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD CShellCasing::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			if (!Update())
			{
				CServerDE* pServerDE = BaseClass::GetServerDE();
				if (!pServerDE) return 0;

				pServerDE->RemoveObject(m_hObject);
			}
			break;
		}

		case MID_TOUCHNOTIFY:
		{
			HandleTouch((HOBJECT)pData);
			break;
		}

		case MID_INITIALUPDATE:
		{
			InitialUpdate();
			break;
		}

		default : break;
	}


	return BaseClass::EngineMessageFn(messageID, pData, lData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShellCasing::Setup
//
//	PURPOSE:	Set up a shell casing with the information needed
//
// ----------------------------------------------------------------------- //

void CShellCasing::Setup(HOBJECT hFiredFrom)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	m_hFiredFrom = hFiredFrom;

	// insert it into the list
	dl_Insert( &m_Head, &m_Link );
	m_Link.m_pData = ( void * )this;
	m_dwNumShells++;

	// Check if too many marks...
	if( m_dwNumShells > MAX_SHELLS )
	{
		CShellCasing *pShell;

		// Delete the oldest mark...
		pShell = ( CShellCasing * )m_Head.m_pPrev->m_pData;

		if( pShell )
		{
			// Remove it from linked list...
			dl_Remove( m_Head.m_pPrev );

			// No data...
			pShell->m_Link.m_pData = NULL;
		}

		m_dwNumShells--;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShellCasing::InitialUpdate
//
//	PURPOSE:	Do initial updating
//
// ----------------------------------------------------------------------- //

DBOOL CShellCasing::InitialUpdate()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject) return DFALSE;

	m_fExpireTime = pServerDE->GetTime() + 20.0f;

	pServerDE->SetObjectFlags(m_hObject, /*| FLAG_GRAVITY*/ FLAG_TOUCH_NOTIFY);
    m_bInVisible = DTRUE;

	pServerDE->SetForceIgnoreLimit(m_hObject, 0.1f);
	pServerDE->SetFrictionCoefficient(m_hObject, 18.0);

	// Set up angular velocities
//	m_fPitchVel = m_fYawVel = 0.0f;
	m_fPitchVel = pServerDE->Random(-MATH_CIRCLE * 2, MATH_CIRCLE * 2);
	m_fYawVel = pServerDE->Random(-MATH_CIRCLE * 2, MATH_CIRCLE * 2);
	m_fPitch	= m_fYaw = 0.0f;

	pServerDE->SetNextUpdate(m_hObject, (DFLOAT)0.001);

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShellCasing::Init
//
//	PURPOSE:	Update the velocity
//
// ----------------------------------------------------------------------- //

void CShellCasing::Init(DBOOL bFiredFromLeft)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject) return;

	m_bFiredFromLeft = bFiredFromLeft;

	DRotation rRot;
	DVector vU, vR, vF;
	pServerDE->GetObjectRotation(m_hObject, &rRot);
	pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);
	DVector vVel;

	if(m_bFiredFromLeft)
		VEC_NEGATE(vR, vR);

	DFLOAT fUpVel = pServerDE->Random(60.0f, 90.0f);
	VEC_MULSCALAR(vU, vU, fUpVel);
	DFLOAT fRightVel = pServerDE->Random(50.0f, 70.0f);
	VEC_MULSCALAR(vR, vR, fRightVel);
	DFLOAT fForwardVel = pServerDE->Random(10.0f, 25.0f);
	VEC_MULSCALAR(vF, vF, fForwardVel);

	VEC_ADD(vVel, vU, vR);
	VEC_ADD(vVel, vVel, vF);
	pServerDE->SetVelocity(m_hObject, &vVel);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShellCasing::Update
//
//	PURPOSE:	Update the impact
//
// ----------------------------------------------------------------------- //

DBOOL CShellCasing::Update()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject) return DFALSE;

	DFLOAT fTime = pServerDE->GetTime();
	if (fTime >= m_fExpireTime)
		return DFALSE;

	pServerDE->GetObjectPos(m_hObject, &m_vLastPos);
	if (m_bInVisible)
	{
		m_bInVisible = DFALSE;
		pServerDE->SetObjectFlags(m_hObject, FLAG_VISIBLE | FLAG_TOUCH_NOTIFY);
	}

	DVector vVel;
	DVector vForce;
	DRotation rRot;

	pServerDE->GetVelocity(m_hObject, &vVel);
	pServerDE->GetObjectRotation(m_hObject, &rRot);

	// If the gravity is not turned on... manually move the shell
	if(!(pServerDE->GetObjectFlags(m_hObject) & FLAG_GRAVITY))
	{
		pServerDE->GetGlobalForce(&vForce);
//		VEC_NORM(vForce);
		VEC_MULSCALAR(vForce, vForce, 0.004f);
		VEC_ADD(vVel, vVel, vForce);
		pServerDE->SetVelocity(m_hObject, &vVel);
	}

	// If velocity slows enough, and we're on the ground, just stop bouncing and just wait to expire.
	if (VEC_MAG(vVel) < 5.0)
	{
		// Stop the spinning
		pServerDE->SetupEuler(&rRot, 0, m_fYaw, 0);
		pServerDE->SetObjectRotation(m_hObject, &rRot);	
		
		// Clear touch notify flag
		pServerDE->SetObjectFlags(m_hObject, FLAG_VISIBLE | FLAG_GRAVITY);
		pServerDE->SetNextUpdate(m_hObject, m_fExpireTime - fTime);

		// shell is at rest, we can add a check here to see if we really want
		// to keep it around depending on detail settings.
/*		ObjectList *ol = pServerDE->FindObjectsTouchingSphere(&m_vLastPos, 40.0f);

		int count = 0;

		if (ol)
		{
			ObjectLink* pLink = ol->m_pFirstLink;
			while(pLink)
			{
				count++;
				pLink = pLink->m_pNext;
				if (count > 20)
					break;
			}
			
			pServerDE->RelinquishList(ol);
		}

		if (count > 20)
			return DFALSE;*/
	}
	else
	{
		if (m_fPitchVel != 0 || m_fYawVel != 0)
		{
			DFLOAT fDeltaTime = pServerDE->GetFrameTime();

			m_fPitch += m_fPitchVel * fDeltaTime;
			m_fYaw += m_fYawVel * fDeltaTime;

			pServerDE->SetupEuler(&rRot, m_fPitch, m_fYaw, 0.0f);
			pServerDE->SetObjectRotation(m_hObject, &rRot);	
		}

		pServerDE->SetNextUpdate(m_hObject, (DFLOAT)0.001);
	}

	return DTRUE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShellCasing::HandleTouch()
//
//	PURPOSE:	Handle touch notify message
//
// ----------------------------------------------------------------------- //

void CShellCasing::HandleTouch(HOBJECT hObj)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject) return;

	// return if it hit a non solid object
	if (hObj == m_hFiredFrom || (hObj != pServerDE->GetWorldObject() && !(pServerDE->GetObjectFlags(hObj) & FLAG_SOLID))) // Ignore non-solid objects
		return;

	// Only bounce so many times..
	if (m_nBounceCount <=0)
		return;

	// Cast a ray from our last known position to see what we hit
	IntersectQuery iq;
	IntersectInfo  ii;
	DVector vVel;

	pServerDE->GetVelocity(m_hObject, &vVel);

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

		DFLOAT r = VEC_DOT(vVel, vNormal) * 0.3f;

		if (r > -100.0f) 
			r = 0;
		else
		{
			VEC_MULSCALAR(vNormal, vNormal, r);
			VEC_SUB(vVel, vVel, vNormal);

			// Adjust the bouncing..
			m_fPitchVel = pServerDE->Random(-MATH_CIRCLE * 2, MATH_CIRCLE * 2);
			m_fYawVel = pServerDE->Random(-MATH_CIRCLE * 2, MATH_CIRCLE * 2);
		}

		VEC_MULSCALAR(vVel, vVel, 0.6f);	// Lose some energy in the bounce.
		pServerDE->SetVelocity(m_hObject, &vVel);

		m_nBounceCount--;

		// Play a bounce sound...
		if (m_nBounceCount)
			PlaySoundFromObject(m_hObject, "Sounds\\Weapons\\Shelldrop.wav", 150.0f, SOUNDPRIORITY_MISC_MEDIUM, DFALSE, DFALSE, DFALSE, 100);

		// Turn on the gravity so the engine can take control of the movement and friction
		if(!m_nBounceCount)
			pServerDE->SetObjectFlags(m_hObject, FLAG_VISIBLE | FLAG_GRAVITY | FLAG_TOUCH_NOTIFY);
	}
}
