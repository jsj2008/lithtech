// ----------------------------------------------------------------------- //
//
// MODULE  : Keyframer.cpp
//
// PURPOSE : Keyframer implementation
//
// CREATED : 10/7/97
//
// ----------------------------------------------------------------------- //

#include "KeyFramer.h"
#include "cpp_server_de.h"
#include "ObjectUtilities.h"
#include "stdio.h"
#include "generic_msg_de.h"
#include "trigger.h"
#include "CameraObj.h"
#include "BloodServerShell.h"
#include "ClientServerShared.h"
#include <mbstring.h>
#include "SoundTypes.h"


BEGIN_CLASS(KeyFramer)
	ADD_STRINGPROP(ObjectName, "")
	ADD_STRINGPROP(BaseKeyName, "")
	ADD_BOOLPROP(StartActive, DFALSE)
	ADD_BOOLPROP(Looping, DFALSE)
END_CLASS_DEFAULT(KeyFramer, B2BaseClass, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::KeyFramer()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

KeyFramer::KeyFramer() : B2BaseClass(OT_NORMAL)
{
	m_hstrObjectName = NULL;
	m_hstrBaseKeyName = NULL;
	m_bStartActive = DFALSE;
	m_bLooping = DFALSE;

	m_bActive = DFALSE;
	m_pObjectList = NULL;
	m_pOffsets.SetMemCopyable(1);
	m_pOffsets.SetGrowBy(3);
	m_pRotations.SetMemCopyable(1);
	m_pRotations.SetGrowBy(3);
	
	m_pKeys = NULL;
	m_pCurKey = NULL;
	m_pPosition1 = NULL;
	m_pPosition2 = NULL;
	
	m_nNumKeys = 0;

	m_fCurTime = 0.0f;

	m_bFirstUpdate = DTRUE;

	m_hsndKeySound = DNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::~KeyFramer()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

KeyFramer::~KeyFramer()
{
	CServerDE* pServerDE = GetServerDE();
	if (pServerDE)
	{
		pServerDE->FreeString (m_hstrObjectName);
		pServerDE->FreeString (m_hstrBaseKeyName);
		if (m_pObjectList)
		{
			pServerDE->RelinquishList (m_pObjectList);
		}
	}

	while (m_pKeys)
	{
		KEYNODE* pNext = m_pKeys->pNext;
		delete m_pKeys;
		m_pKeys = pNext;
	}

	if (m_hsndKeySound)
	{
		pServerDE->KillSound(m_hsndKeySound);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::GoActive
//
//	PURPOSE:	Start the KeyFramer going!
//
// ----------------------------------------------------------------------- //

void KeyFramer::GoActive()
{
	CServerDE* pServerDE = GetServerDE();

	if (!pServerDE || m_bActive || !m_pKeys) return;
	
	m_bStartActive = DTRUE;

	// Go active

	m_bActive	= DTRUE;
	m_fCurTime	= 0.0f;
	m_pCurKey	= m_pKeys;
	pServerDE->SetNextUpdate(m_hObject, 0.001f);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::CreateKeyList
//
//	PURPOSE:	Create our list of keys.
//
// ----------------------------------------------------------------------- //

DBOOL KeyFramer::CreateKeyList()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

	// Create the list of keys...

	int nNum = 0;
	DFLOAT fTime = 0.0f;
	KEYNODE* pNode = NULL;

	while (1)
	{
		// Create the keyname string...

		char strKey[128]; memset (strKey, 0, 128);
		char strNum[18]; memset (strNum, 0, 18);

		_itoa (nNum, strNum, 10);
		char *pszBaseKeyName = pServerDE->GetStringData (m_hstrBaseKeyName);
		if( pszBaseKeyName )
			_mbscpy ((unsigned char*)strKey, (const unsigned char*)pszBaseKeyName);
		else
			strKey[0] = 0;
		_mbscat ((unsigned char*)strKey, (const unsigned char*)strNum);

		
		// Find the first key with that name...

		ObjectList* pTempKeyList = pServerDE->FindNamedObjects (strKey);
		ObjectLink* pLink = pTempKeyList->m_pFirstLink;
		
		DBOOL bFoundKey = DFALSE;
		while (pLink && !bFoundKey)
		{
			if (pServerDE->IsKindOf (pServerDE->GetObjectClass (pLink->m_hObject), pServerDE->GetClass ("Key")))
			{
				// Add this key to the list...

				if (!m_pKeys)
				{
					m_pKeys = new KEYNODE;
					pNode = m_pKeys;
				}
				else
				{
					pNode->pNext = new KEYNODE;
					pNode->pNext->pPrev = pNode;
					pNode = pNode->pNext;
				}

				
				// Copy the key...

				pNode->keyData.Copy((Key*)pServerDE->HandleToObject(pLink->m_hObject));

				fTime += pNode->keyData.m_fTimeStamp;
				pNode->keyData.m_fRealTime = fTime;

				
				// Remove the key from the world...No longer needed...
				
				pServerDE->RemoveObject(pLink->m_hObject);


				bFoundKey = DTRUE;
				m_nNumKeys++;
			}

			pLink = pLink->m_pNext;
		}

		pServerDE->RelinquishList (pTempKeyList);
		
		// If we didn't find a key, we're done looking...

		if (!pLink && !bFoundKey)
		{
			break;
		}

		// Increment the counter...

		nNum++;
	}

	
	// If we didn't find any keys, return...

	if (!m_pKeys)
	{
		return DFALSE;
	}

	
	// For each object we're controlling, save it's offset from the first key...

	KEYNODE* pFirstPosKey = GetNextPositionKey(m_pKeys);

	if (pFirstPosKey)
	{
		// Get the first position key's location

		DVector pos;
		DRotation rot;

		VEC_COPY(pos, pFirstPosKey->keyData.m_vPos);
		ROT_COPY(rot, pFirstPosKey->keyData.m_rRot);
		
		long i = 0;
		ObjectLink* pLink = m_pObjectList->m_pFirstLink;
		while (pLink)
		{
			DVector objpos;
			pServerDE->GetObjectPos(pLink->m_hObject, &objpos);
			VEC_SUB(m_pOffsets[i], objpos, pos);

			DRotation objrot;
			pServerDE->GetObjectRotation(pLink->m_hObject, &objrot);
			VEC_SUB(m_pRotations[i].m_Vec, objrot.m_Vec, rot.m_Vec);
			m_pRotations[i].m_Spin = objrot.m_Spin - rot.m_Spin;

			i++;
			pLink = pLink->m_pNext;
		}
	}
	
	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::BuildObjectList
//
//	PURPOSE:	Builds the list of objects to control
//
// ----------------------------------------------------------------------- //

DBOOL KeyFramer::BuildObjectList()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hstrObjectName) return DFALSE;

	if (!m_pObjectList)
	{
		// Find the objects...
		m_pObjectList = pServerDE->FindNamedObjects(pServerDE->GetStringData(m_hstrObjectName));

		if (m_pObjectList->m_nInList)
		{
			// Create links to all these objects so we know if/when they
			// get removed...

			ObjectLink* pLink = m_pObjectList->m_pFirstLink;
			while (pLink)
			{
				pServerDE->CreateInterObjectLink(m_hObject, pLink->m_hObject);
				pLink = pLink->m_pNext;
			}
		}
	}
	
	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::GetNextPositionKey
//
//	PURPOSE:	Returns the next key with position type
//
// ----------------------------------------------------------------------- //

KEYNODE* KeyFramer::GetNextPositionKey(KEYNODE* pNode)
{
	if (!pNode) return NULL;

	while (pNode)
	{
		if (pNode->keyData.m_nKeyType & POSITION_KEY)
		{
			return pNode;
		}

		pNode = pNode->pNext;
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::ProcessKey
//
//	PURPOSE:	Processes the given key
//
// ----------------------------------------------------------------------- //

void KeyFramer::ProcessKey (KEYNODE* pNode)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;
	
	if (pNode->keyData.m_nKeyType & POSITION_KEY)
	{
		m_pPosition1 = pNode;
		m_pPosition2 = GetNextPositionKey(pNode->pNext);
	}

	if (pNode->keyData.m_nKeyType & SOUND_KEY)
	{
		ObjectLink* pLink = m_pObjectList->m_pFirstLink;
		while (pLink && pLink->m_hObject)
		{
			// kill current sound
			if (m_hsndKeySound)
			{
				pServerDE->KillSound(m_hsndKeySound);
			}

			m_hsndKeySound = PlaySoundFromObject(pLink->m_hObject, pServerDE->GetStringData(pNode->keyData.m_hstrSoundName), pNode->keyData.m_fSoundRadius, SOUNDPRIORITY_MISC_MEDIUM, pNode->keyData.m_bLoopSound, DTRUE, DTRUE );
			pLink = pLink->m_pNext;
		}
	}

	if (pNode->keyData.m_nKeyType & MESSAGE_KEY)
	{
		SendTriggerMsgToObjects(this, pNode->keyData.m_hstrMessageTarget, pNode->keyData.m_hstrMessageName);
	}

	if (pNode->keyData.m_nKeyType & BPRINT_KEY)
	{
		pServerDE->BPrint(pServerDE->GetStringData(pNode->keyData.m_hstrBPrintMessage));
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD KeyFramer::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			Update((DVector *)pData);
			break;
		}

		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE)
			{
				ReadProp((ObjectCreateStruct *)pData);
			}
			break;
		}

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate((DVector *)pData);
			}
			break;
		}

		case MID_LINKBROKEN :
		{
			HOBJECT hLink = (HOBJECT)pData;
			if (m_pObjectList && hLink)
			{
				ObjectLink* pLink = m_pObjectList->m_pFirstLink;

				while (pLink)
				{
					if (pLink->m_hObject == hLink)
					{
						g_pServerDE->RemoveObjectFromList(m_pObjectList, pLink->m_hObject);
						break;
					}
					pLink = pLink->m_pNext;
				}
			}
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


	return B2BaseClass::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::ObjectMessageFn
//
//	PURPOSE:	Handle object messages
//
// ----------------------------------------------------------------------- //

DDWORD KeyFramer::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	if (!g_pServerDE) return 0;

	switch (messageID)
	{
		case MID_TRIGGER:
		{
			HSTRING hMsg = g_pServerDE->ReadFromMessageHString(hRead);
			char* pMsg = g_pServerDE->GetStringData(hMsg);
			if (!pMsg) return 0;

			if (_mbsicmp((const unsigned char*)pMsg, (const unsigned char*)"ON") == 0 || _mbsicmp((const unsigned char*)pMsg, (const unsigned char*)"TRIGGER") == 0)
			{
				GoActive();
			}
			else if (_mbsicmp((const unsigned char*)pMsg, (const unsigned char*)"OFF") == 0)
			{
				m_bActive	= DFALSE;
				g_pServerDE->SetNextUpdate(m_hObject, 0.0f);			
			}
			g_pServerDE->FreeString(hMsg);
		}
		break;

		default : break;
	}
	
	return B2BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

DBOOL KeyFramer::ReadProp(ObjectCreateStruct *pStruct)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pStruct) return DFALSE;

	char buf[MAX_CS_FILENAME_LEN];

	buf[0] = '\0';
	pServerDE->GetPropString("ObjectName", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hstrObjectName = pServerDE->CreateString(buf);

	buf[0] = '\0';
	pServerDE->GetPropString("BaseKeyName", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hstrBaseKeyName = pServerDE->CreateString(buf);

	pServerDE->GetPropBool("StartActive", &m_bStartActive);
	pServerDE->GetPropBool("Looping", &m_bLooping);

	pStruct->m_NextUpdate = 0.001f;

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

DBOOL KeyFramer::InitialUpdate(DVector* pMovement)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

	// Mark this object as savable
	DDWORD dwFlags = g_pServerDE->GetObjectUserFlags(m_hObject);
	dwFlags |= USRFLG_SAVEABLE;
	g_pServerDE->SetObjectUserFlags(m_hObject, dwFlags);

	// Build key list during the first update so that we can 
	// ensure that the keys have all been loaded.
	pServerDE->SetNextUpdate(m_hObject, 0.001f);

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::Update()
//
//	PURPOSE:	Update
//
// ----------------------------------------------------------------------- //

DBOOL KeyFramer::Update(DVector* pMovement)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;


	if (!m_pObjectList)
	{
		if (!BuildObjectList())
			return DFALSE;
	}

	// Need to check this here (instead of InitialUpdate)...This insures 
	// that all the Keys have been added to the world...

	if (m_bFirstUpdate)
	{
		m_bFirstUpdate = DFALSE;

		CreateKeyList();

		if (m_bStartActive)
		{
			GoActive();
		}
		// Don't start processing keys until the next update
		return DTRUE;
	}

	// See if we are even supposed to be here (today)...
	if (!m_bActive)
	{
		pServerDE->SetNextUpdate(m_hObject, 0.0f);
		return DTRUE;
	}
	else
	{
		pServerDE->SetNextUpdate(m_hObject, 0.001f);
	}

	// increment timer

	m_fCurTime += pServerDE->GetFrameTime();

	// process all keys that we might have passed by

	while (m_pCurKey)
	{
		if (m_pCurKey->keyData.m_fRealTime > m_fCurTime) break;

		ProcessKey (m_pCurKey);
		m_pCurKey = m_pCurKey->pNext;
	}

	// are we at the end of the key list?

	if (!m_pCurKey)
	{
		m_pCurKey = m_pKeys;
		m_fCurTime = 0.0f;

		if (!m_bLooping)
		{
			m_bActive = DFALSE;
			return DTRUE;
		}
	}

	// interpolate position

	if (m_pPosition1 && m_pPosition2)
	{
		if (m_fCurTime >= m_pPosition1->keyData.m_fRealTime)
		{
			const float c_fMinTimeDelta = 0.001f;
			float fTimeDelta = (m_pPosition2->keyData.m_fRealTime - m_pPosition1->keyData.m_fRealTime);
		
			DVector posNew, pos1, pos2;
			VEC_INIT(posNew);
			DRotation rotNew, rot1, rot2;
			ROT_INIT(rotNew);
	
			if (fTimeDelta <= c_fMinTimeDelta)
			{
				VEC_COPY(posNew, m_pPosition2->keyData.m_vPos);
				ROT_COPY(rotNew, m_pPosition2->keyData.m_rRot);
			}
			else if (m_fCurTime <= m_pPosition2->keyData.m_fRealTime)
			{
				float t = (m_fCurTime - m_pPosition1->keyData.m_fRealTime) / fTimeDelta;
				
				// get the new position from position 1's position

				VEC_COPY(pos1, m_pPosition1->keyData.m_vPos);
				VEC_COPY(pos2, m_pPosition2->keyData.m_vPos);
				VEC_LERP(posNew, pos1, pos2, t);

				// get the new angle from position 1's angle

				ROT_COPY(rot1, m_pPosition1->keyData.m_rRot);
				ROT_COPY(rot2, m_pPosition2->keyData.m_rRot);

	 			pServerDE->InterpolateRotation(&rotNew, &rot1, &rot2, t);
			}
			else
			{
				return DTRUE;
			}

			// Now add the relative position and rotation to every object in the list

			long i = 0;
			ObjectLink* pLink = m_pObjectList->m_pFirstLink;
			while (pLink)
			{
				pServerDE->GetObjectPos(pLink->m_hObject, &pos1);
				VEC_ADD (pos1, posNew, m_pOffsets[i]);

/*
				char msg[80];
				sprintf (msg, "moving to: %.2f, %.2f, %.2f", pos1.x, pos1.y, pos1.z);
				pServerDE->BPrint (msg);
*/				
				pServerDE->GetObjectRotation(pLink->m_hObject, &rot1);
				VEC_ADD(rot1.m_Vec, rotNew.m_Vec, m_pRotations[i].m_Vec);
				rot1.m_Spin = rotNew.m_Spin + m_pRotations[i].m_Spin;

				// If object is a camera, and it has a link, don't rotate it.
				DBOOL bRotate = DTRUE;
				if(pServerDE->IsKindOf(pServerDE->GetObjectClass(pLink->m_hObject), pServerDE->GetClass("CameraObj"))) 
				{
					CameraObj *pCamera = (CameraObj*)pServerDE->HandleToObject(pLink->m_hObject);
					if (pCamera->GetLinkObject())
					{
						bRotate = DFALSE;
					}
				}
				
				pServerDE->MoveObject(pLink->m_hObject, &pos1);
				if (bRotate)
					pServerDE->RotateObject(pLink->m_hObject, &rot1);

				i++;
				pLink = pLink->m_pNext;
			}
		}
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void KeyFramer::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageFloat(hWrite, m_fCurTime);
	pServerDE->WriteToMessageHString(hWrite, m_hstrObjectName);
	pServerDE->WriteToMessageHString(hWrite, m_hstrBaseKeyName);
	pServerDE->WriteToMessageByte(hWrite, m_bStartActive);
	pServerDE->WriteToMessageByte(hWrite, m_bLooping);
	pServerDE->WriteToMessageByte(hWrite, m_bActive);
	pServerDE->WriteToMessageByte(hWrite, m_bFirstUpdate);
	pServerDE->WriteToMessageByte(hWrite, m_nNumKeys);


	// Determine the position in the list of m_pCurKey, m_pPosition1, and
	// m_pPosition2.  Also, save out m_pKeys...

	int nCurKeyIndex    = -1;
	int nPosition1Index = -1;
	int nPosition2Index = -1;
	
	KEYNODE* pCurKey = m_pKeys;
	for (int i=0; i < m_nNumKeys && pCurKey; i++)
	{
		if (m_pCurKey == pCurKey)
		{
			nCurKeyIndex = i;
		}
		if (m_pPosition1 == pCurKey)
		{
			nPosition1Index = i;
		}
		if (m_pPosition2 == pCurKey)
		{
			nPosition2Index = i;
		}

		pCurKey->keyData.Save(hWrite, dwSaveFlags);
		pCurKey = pCurKey->pNext;
	}
		
	// Save out the positions of our pointer data members...

	pServerDE->WriteToMessageFloat(hWrite, (DFLOAT) nCurKeyIndex);
	pServerDE->WriteToMessageFloat(hWrite, (DFLOAT) nPosition1Index);
	pServerDE->WriteToMessageFloat(hWrite, (DFLOAT) nPosition2Index);


	for (i=0; i < m_nNumKeys; i++)
	{
		pServerDE->WriteToMessageVector(hWrite, &(m_pOffsets[i]));
		pServerDE->WriteToMessageRotation(hWrite, &(m_pRotations[i]));
	}	

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void KeyFramer::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	m_fCurTime			= pServerDE->ReadFromMessageFloat(hRead);
	m_hstrObjectName	= pServerDE->ReadFromMessageHString(hRead);
	m_hstrBaseKeyName	= pServerDE->ReadFromMessageHString(hRead);
	m_bStartActive		= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bLooping			= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bActive			= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bFirstUpdate		= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_nNumKeys			= pServerDE->ReadFromMessageByte(hRead);


	// Build the m_pKeys data member...

	KEYNODE* pNode = DNULL;
	for (int i=0; i < m_nNumKeys; i++)
	{
		if (!m_pKeys)
		{
			m_pKeys = new KEYNODE;
			pNode = m_pKeys;
		}
		else if (pNode)
		{
			pNode->pNext = new KEYNODE;
			pNode->pNext->pPrev = pNode;
			pNode = pNode->pNext;
		}

		if (pNode)
		{
			pNode->keyData.Load(hRead, dwLoadFlags);
		}
	}
		
	// Determine the positions of our pointer data members...

	int nCurKeyIndex	= (int) pServerDE->ReadFromMessageFloat(hRead);
	int nPosition1Index	= (int) pServerDE->ReadFromMessageFloat(hRead);
	int nPosition2Index	= (int) pServerDE->ReadFromMessageFloat(hRead);

	KEYNODE* pCurKey = m_pKeys;
	for (i=0; i < m_nNumKeys && pCurKey; i++)
	{
		if (nCurKeyIndex == i)
		{
			m_pCurKey = pCurKey;
		}
		if (nPosition1Index == i)
		{
			m_pPosition1 = pCurKey;
		}
		if (nPosition2Index == i)
		{
			m_pPosition2 = pCurKey;
		}

		pCurKey = pCurKey->pNext;
	}


	for (i=0; i < m_nNumKeys; i++)
	{
		pServerDE->ReadFromMessageVector(hRead, &(m_pOffsets[i]));
		pServerDE->ReadFromMessageRotation(hRead, &(m_pRotations[i]));
	}	
}
