// ----------------------------------------------------------------------- //
//
// MODULE  : CameraSpot.cpp
//
// PURPOSE : CameraSpot object will be used to place cameras in Cut Scenes
//
// CREATED : 02/17/98
//
// ----------------------------------------------------------------------- //

#include <stdio.h>
#include "CameraSpot.h"

BEGIN_CLASS(CameraSpot)
END_CLASS_DEFAULT_FLAGS(CameraSpot, B2BaseClass, NULL, NULL, CF_ALWAYSLOAD)

void BPrint(char*);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CameraSpot::CameraSpot()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

CameraSpot::CameraSpot() : B2BaseClass(OT_NORMAL)
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CameraSpot::~CameraSpot()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

CameraSpot::~CameraSpot()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CameraSpot::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD CameraSpot::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
/*
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
*/

		case MID_PRECREATE:
		{
			DDWORD dwRet = B2BaseClass::EngineMessageFn(messageID, pData, fData);

			PostPropRead((ObjectCreateStruct*)pData);

			return dwRet;
		}
        break;


//		case MID_READPROP:
//		{
//			ReadProp((ReadPropInfo*)pData);
//		}
//		break;

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

void CameraSpot::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

//	char* pFilename = "Models\\CameraSpot.abc";
//	char* pSkin = "Skins\\CameraSpot.dtx";
//	_mbscpy((unsigned char*)pStruct->m_Filename, (const unsigned char*)pFilename);
//	_mbscpy((unsigned char*)pStruct->m_SkinName, (const unsigned char*)pSkin);
    
    // Set the Update!
	pStruct->m_NextUpdate = 0.001f;


// Check if debug is turned ON

//    if ( _mbsncmp((const unsigned char*)pStruct->m_Name,(const unsigned char*)"SMELL",5) == 0)
//    {
//	    pStruct->m_Flags = FLAG_TOUCH_NOTIFY;	
//    }
//    else
//    {
//	    pStruct->m_Flags = FLAG_VISIBLE | FLAG_TOUCH_NOTIFY;	
//    }

//    pStruct->m_Flags = FLAG_VISIBLE | FLAG_TOUCH_NOTIFY;	

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CameraSpot::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //
//DBOOL CameraSpot::ReadProp(ReadPropInfo *pInfo)
//{
//	CServerDE* pServerDE = GetServerDE();
//	if (!pServerDE) return DFALSE;
//	if (!pInfo) return DFALSE;
//
//    DBOOL bRet = DTRUE;  

//	if(_mbscmp((const unsigned char*)pInfo->m_pPropName, (const unsigned char*)"Action")==0 && pInfo->m_PropType == PT_STRING)
//	{
//		m_hstrAction = pServerDE->CreateString((char*)pInfo->m_pData);
//	}
//	else
//	{
//		bRet = DFALSE;
//	}

//	return bRet;
//}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CameraSpot::ObjectTouch
//
//	PURPOSE:	Handle object touch
//
// ----------------------------------------------------------------------- //

void CameraSpot::ObjectTouch (HOBJECT hObj)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

//    char buf[80];
//    sprintf(buf, "CameraSpot =%s", pServerDE->GetObjectName(m_hObject));
//	BPrint(buf);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CameraSpot::InitialUpdate
//
//	PURPOSE:	Initial update
//
// ----------------------------------------------------------------------- //

DBOOL CameraSpot::InitialUpdate(DVector *)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

//	m_bActive = DTRUE;
	pServerDE->SetNextUpdate(m_hObject, 0.0f);

//	DVector vDims;
//	VEC_SET(vDims, 20, 20, 20);
//	pServerDE->SetObjectDims(m_hObject, &vDims);

	return DTRUE;
}

/*
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CameraSpot::Update
//
//	PURPOSE:	Handle Update
//
// ----------------------------------------------------------------------- //

DBOOL CameraSpot::Update(DVector *)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

   	pServerDE->SetNextUpdate(m_hObject, 0.01f);
    
	return DTRUE;
}

*/
