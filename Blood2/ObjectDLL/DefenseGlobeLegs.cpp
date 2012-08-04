// ----------------------------------------------------------------------- //
//
// MODULE  : DefenseGlobeLegs.cpp
//
// PURPOSE : The bottom half of the DefenseGlobe
//           
//           
//
// CREATED : 10/17/97
//
// ----------------------------------------------------------------------- //

#include <stdio.h>
#include "DefenseGlobeLegs.h"

BEGIN_CLASS(DefenseGlobeLegs)
END_CLASS_DEFAULT_FLAGS(DefenseGlobeLegs, CBaseCharacter, NULL, NULL, CF_HIDDEN)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DefenseGlobeLegs::DefenseGlobeLegs()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

DefenseGlobeLegs::DefenseGlobeLegs() : B2BaseClass(OT_MODEL)
{
    m_bActive           = DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DefenseGlobeLegs::~DefenseGlobeLegs()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

DefenseGlobeLegs::~DefenseGlobeLegs()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DefenseGlobeLegs::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD DefenseGlobeLegs::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			if (!Update((DVector *)pData) )
            {
            	CServerDE* pServerDE = GetServerDE();
            	if (!pServerDE) break;

            	pServerDE->RemoveObject(m_hObject);		
            }
		}
		break;

		case MID_TOUCHNOTIFY:
		{
			ObjectTouch((HOBJECT)pData);
		}
		break;


		case MID_PRECREATE:
		{
			// Need to call base class to have the object name read in before
			// we call PostPropRead()

			DDWORD dwRet = B2BaseClass::EngineMessageFn(messageID, pData, fData);

			PostPropRead((ObjectCreateStruct*)pData);

			return dwRet;
		}


		case MID_INITIALUPDATE:
		{
			InitialUpdate((DVector *)pData);
		}
		break;

		default : break;
	}


	return B2BaseClass::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PostPropRead()
//
//	PURPOSE:	Update properties
//
// ----------------------------------------------------------------------- //

void DefenseGlobeLegs::PostPropRead(ObjectCreateStruct *pStruct)
{
    if (!pStruct) return;

	char* pFilename = "Models\\Enemies\\def_bot.abc";
	char* pSkin = "Skins\\Enemies\\def_bot.dtx";
	_mbscpy((unsigned char*)pStruct->m_Filename, (const unsigned char*)pFilename);
	_mbscpy((unsigned char*)pStruct->m_SkinName, (const unsigned char*)pSkin);
    
   	pStruct->m_NextUpdate = 0.0f;
    
	pStruct->m_Flags = FLAG_VISIBLE | FLAG_SOLID | FLAG_GRAVITY | FLAG_STAIRSTEP | FLAG_SHADOW;	
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DefenseGlobeLegs::ObjectTouch
//
//	PURPOSE:	Handle object touch
//
// ----------------------------------------------------------------------- //

void DefenseGlobeLegs::ObjectTouch (HOBJECT hObj)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DefenseGlobeLegs::InitialUpdate
//
//	PURPOSE:	Initial update
//
// ----------------------------------------------------------------------- //

DBOOL DefenseGlobeLegs::InitialUpdate(DVector *)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

	m_bActive = DTRUE;
	pServerDE->SetNextUpdate(m_hObject, 0.001f);

	// Set the player's dims...
	DVector vDims;
	VEC_SET(vDims, 8, 5, 8);
	pServerDE->SetObjectDims(m_hObject, &vDims);

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DefenseGlobeLegs::Update
//
//	PURPOSE:	Handle Update
//
// ----------------------------------------------------------------------- //

DBOOL DefenseGlobeLegs::Update(DVector *)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;
    
   	pServerDE->SetNextUpdate(m_hObject, 0.01f);

	return DTRUE;
}

