// ----------------------------------------------------------------------- //
//
// MODULE  : Tchernotronic.cpp
//
// PURPOSE : Tchernotronic - Implementation
//
// CREATED : 5/11/98
//
// ----------------------------------------------------------------------- //

#include "Tchernotronic.h"
#include "ClientServerShared.h"
#include "Trigger.h"
#include "ObjectUtilities.h"
#include "PhysicalAttributes.h"
#include "SFXMsgIds.h"
#include "SoundTypes.h"

#define INVALID_ANI		((DDWORD)-1)

#define TIME_REMOVE		0.5f		// Remove 2 seconds after trigger

// Static global variables..
static char *g_szTriggerOn = "TRIGGER"; 


BEGIN_CLASS(Tchernotronic)
	ADD_STRINGPROP_FLAG(Filename, "Models\\WorldObjects\\Technoberg.abc", PF_HIDDEN)
	ADD_STRINGPROP_FLAG(Skin, "Skins\\WorldObjects\\Technoberg.dtx", PF_HIDDEN)
	ADD_STRINGPROP(DestroyTriggerTarget, "") 
	ADD_STRINGPROP(DestroyTriggerMessage, "")
END_CLASS_DEFAULT(Tchernotronic, B2BaseClass, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Tchernotronic::Tchernotronic
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

Tchernotronic::Tchernotronic() : B2BaseClass(OT_MODEL)
{ 
	m_hstrTriggerTarget = DNULL;
	m_hstrTriggerMessage = DNULL;

	m_nRestAnim		= INVALID_ANI;
	m_nMovingAnim	= INVALID_ANI;

	m_bTimer		= DFALSE;
	m_fRemoveTime	= 0.0f;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Tchernotronic::~Tchernotronic()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

Tchernotronic::~Tchernotronic()
{
	CServerDE* pServerDE = GetServerDE();

	if (m_hstrTriggerTarget)
	{
		pServerDE->FreeString(m_hstrTriggerTarget);
		m_hstrTriggerTarget = DNULL;
	}

	if (m_hstrTriggerMessage)
	{
		pServerDE->FreeString(m_hstrTriggerMessage);
		m_hstrTriggerMessage = DNULL;
	}
}

	
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Tchernotronic::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD Tchernotronic::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			if (!Update((DVector *)pData))
            {
            	CServerDE* pServerDE = GetServerDE();
            	if (!pServerDE) break;

            	pServerDE->RemoveObject(m_hObject);		
            }
			break;
		}

		case MID_PRECREATE:
		{
			ReadProp((ObjectCreateStruct *)pData);
			break;
		}

		case MID_INITIALUPDATE:
		{
			if (( DDWORD )fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate((DVector *)pData);
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

	return B2BaseClass::EngineMessageFn(messageID, pData, fData);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Tchernotronic::ObjectMessageFn()
//
//	PURPOSE:	Handler for server object messages.
//
// --------------------------------------------------------------------------- //

DDWORD Tchernotronic::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = GetServerDE();
	switch (messageID)
	{
		case MID_TRIGGER:
			HSTRING hMsg = pServerDE->ReadFromMessageHString(hRead);
			TriggerMsg(hSender, hMsg);
			pServerDE->FreeString(hMsg);
			break;
	}
	
	return B2BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Tchernotronic::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

DBOOL Tchernotronic::ReadProp(ObjectCreateStruct *pStruct)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pStruct) return DFALSE;

	char buf[MAX_CS_FILENAME_LEN];
	buf[0] = '\0';
	pServerDE->GetPropString("DestroyTriggerTarget", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hstrTriggerTarget = pServerDE->CreateString(buf);

	buf[0] = '\0';
	pServerDE->GetPropString("DestroyTriggerMessage", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hstrTriggerMessage = pServerDE->CreateString(buf);

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Tchernotronic::InitialUpdate
//
//	PURPOSE:	Handle initial Update
//
// ----------------------------------------------------------------------- //

void Tchernotronic::InitialUpdate(DVector *pMovement)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	// Mark this object as savable
	DDWORD dwUsrFlags = pServerDE->GetObjectUserFlags(m_hObject);
	dwUsrFlags |= USRFLG_SAVEABLE;
	pServerDE->SetObjectUserFlags(m_hObject, dwUsrFlags);

	DDWORD dwFlags = (FLAG_VISIBLE | FLAG_SOLID);

	pServerDE->SetObjectFlags(m_hObject, dwFlags);

	// Get model animations
	m_nRestAnim			= pServerDE->GetAnimIndex(m_hObject, "static_model");
	m_nMovingAnim		= pServerDE->GetAnimIndex(m_hObject, "animate");

	pServerDE->SetModelLooping(m_hObject, DFALSE);

	DVector vDims;
	pServerDE->GetModelAnimUserDims(m_hObject, &vDims, m_nRestAnim);
	pServerDE->SetObjectDims(m_hObject, &vDims);

	pServerDE->SetBlockingPriority(m_hObject, BLOCKPRIORITY_NONPUSHABLE);

	// Wait to be triggered
	pServerDE->SetNextUpdate(m_hObject, 0.0f);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Tchernotronic::Update
//
//	PURPOSE:	Handle Update
//
// ----------------------------------------------------------------------- //

DBOOL Tchernotronic::Update(DVector *pMovement)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

	DFLOAT fTime = pServerDE->GetTime();
	// See if the current anim is finished.. If so, blow up!
	if (m_bTimer)
	{
		if (fTime >= m_fRemoveTime)
		{
			// Send any trigger messages
			if (m_hstrTriggerTarget && m_hstrTriggerMessage)
				SendTriggerMsgToObjects(this, m_hstrTriggerTarget, m_hstrTriggerMessage);

			return DFALSE;
		}
	}
	else
	{
		DDWORD dwState	= pServerDE->GetModelPlaybackState(m_hObject);
		if (dwState & MS_PLAYDONE)
		{
			DVector vPos;
			pServerDE->GetObjectPos(m_hObject, &vPos);
			AddExplosion(vPos);
			m_fRemoveTime = fTime + TIME_REMOVE;
			m_bTimer = DTRUE;

		}
	}

	pServerDE->SetNextUpdate(m_hObject, 0.05f);

	return DTRUE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Tchernotronic::TriggerMsg()
//
//	PURPOSE:	Handler for Tchernotronic trigger messages.
//
// --------------------------------------------------------------------------- //

void Tchernotronic::TriggerMsg(HOBJECT hSender, HSTRING hMsg)
{
	CServerDE* pServerDE = GetServerDE();
	HSTRING hstr;

	// See if he's been triggered to start moving
	if (IsPlayer(hSender)) return;

	hstr = pServerDE->CreateString(g_szTriggerOn);
	if (pServerDE->CompareStringsUpper(hMsg, hstr))
	{
		PlayAnimation(m_nMovingAnim);
		pServerDE->SetNextUpdate(m_hObject, 0.1f);
	}
	pServerDE->FreeString(hstr);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Tchernotronic::PlayAnimation()
//
//	PURPOSE:	Set model to new animation
//
// ----------------------------------------------------------------------- //

DBOOL Tchernotronic::PlayAnimation(DDWORD dwNewAni)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || dwNewAni == INVALID_ANI) return DFALSE;

	DDWORD dwAni	= pServerDE->GetModelAnimation(m_hObject);
	DDWORD dwState	= pServerDE->GetModelPlaybackState(m_hObject);

	if (dwAni == dwNewAni && (dwState & MS_PLAYDONE))
	{
		return DFALSE;
	}
	else if (dwAni != dwNewAni)
	{
		pServerDE->SetModelAnimation(m_hObject, dwNewAni);
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Tchernotronic::AddExplosion()
//
//	PURPOSE:	Add an explosion
//
// ----------------------------------------------------------------------- //
/*
void Tchernotronic::AddExplosion(DVector &vPos)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	Explosion* pExplosion = DNULL;

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	HCLASS hClass = pServerDE->GetClass("Explosion");

	if (hClass)
	{

		DRotation rRot;
		pServerDE->GetObjectRotation(m_hObject, &rRot);
		ROT_COPY(theStruct.m_Rotation, rRot);

		for (int i=0; i<4; i++)
		{
			VEC_COPY(theStruct.m_Pos, vPos);
			// Vary the position of the explosions
			switch (i)
			{
			case 0:
				theStruct.m_Pos.y -= 40.0f;
				theStruct.m_Pos.x += 10.0f;
				break;
			case 1:
				theStruct.m_Pos.z -= 15.0f;
				break;
			case 2:
				theStruct.m_Pos.y += 50.0f;
				theStruct.m_Pos.x += 10.0f;
				break;
			case 3:
				theStruct.m_Pos.y -= 20.0f;
				theStruct.m_Pos.x += 5.0f;
				theStruct.m_Pos.z -= 5.0f;
				break;
			};
	
			pExplosion = (Explosion*)pServerDE->CreateObject(hClass, &theStruct);
			if (pExplosion)
			{
				pExplosion->Setup("Sounds\\exp_tnt.wav", 2000, 0.75f,
								  NULL, 150.0f, 100.0f, 1.0f,
								  20.0f, DTRUE, DFALSE, DFALSE);

				// Set up the impact light...
				DVector vLightColor;
				VEC_SET(vLightColor, 1.0f, 0.5f, 0.0f);

				pExplosion->SetupLight(DTRUE, vLightColor, 20.0f, 300.0f);

				pExplosion->Explode(i*0.25f);
			}
		}
	}
}
*/

void Tchernotronic::AddExplosion(DVector &vPos)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DDWORD		nType = EXP_GRENADE;
	DVector		vUp;
	VEC_SET(vUp, 0.0f, 1.0f, 0.0f);

	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&vPos);
	pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

	pServerDE->WriteToMessageVector(hMessage, &vPos);
	pServerDE->WriteToMessageVector(hMessage, &vUp);
	pServerDE->WriteToMessageDWord(hMessage, nType);

	pServerDE->EndMessage(hMessage);

	PlaySoundFromPos(&vPos, "Sounds\\Weapons\\c4\\explosion_1.wav", 1000.0f, SOUNDPRIORITY_MISC_MEDIUM);

	// Do some damage
	DamageObjectsInRadius(m_hObject, pServerDE->HandleToObject(m_hObject), vPos, 200.0f, 100.0f, DAMAGE_TYPE_EXPLODE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Tchernotronic::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Tchernotronic::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveGame)
{
	DFLOAT fTime;

	fTime = g_pServerDE->GetTime( );

	g_pServerDE->WriteToMessageHString( hWrite, m_hstrTriggerTarget );
	g_pServerDE->WriteToMessageHString( hWrite, m_hstrTriggerMessage );
	g_pServerDE->WriteToMessageDWord( hWrite, m_nRestAnim );
	g_pServerDE->WriteToMessageDWord( hWrite, m_nMovingAnim );
	g_pServerDE->WriteToMessageByte( hWrite, m_bTimer );
	g_pServerDE->WriteToMessageFloat( hWrite, m_fRemoveTime - fTime );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Tchernotronic::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Tchernotronic::Load(HMESSAGEREAD hRead, DDWORD dwLoadGame)
{
	DFLOAT fTime;

	fTime = g_pServerDE->GetTime( );

	if( m_hstrTriggerTarget )
	{
		g_pServerDE->FreeString( m_hstrTriggerTarget );
	}
	m_hstrTriggerTarget = g_pServerDE->ReadFromMessageHString( hRead );

	if( m_hstrTriggerMessage )
	{
		g_pServerDE->FreeString( m_hstrTriggerMessage );
	}
	m_hstrTriggerMessage = g_pServerDE->ReadFromMessageHString( hRead );

	m_nRestAnim = g_pServerDE->ReadFromMessageDWord( hRead );
	m_nMovingAnim = g_pServerDE->ReadFromMessageDWord( hRead );
	m_bTimer = g_pServerDE->ReadFromMessageByte( hRead );
	m_fRemoveTime = g_pServerDE->ReadFromMessageFloat( hRead ) + fTime;

}



