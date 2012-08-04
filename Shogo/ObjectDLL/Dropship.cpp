// ----------------------------------------------------------------------- //
//
// MODULE  : DropShip.cpp
//
// PURPOSE : Model DropShip - Definition
//
// CREATED : 8/17/98
//
// ----------------------------------------------------------------------- //

#include "DropShip.h"
#include "cpp_server_de.h"
#include "SFXMsgIds.h"

#define DEFAULT_THRUSTER_SPRITE "Sprites\\vehicle.spr"


BEGIN_CLASS(DropShip)
	ADD_GRAVITY_FLAG(0, 0)
	ADD_BOOLPROP(MoveToFloor, DFALSE)
	ADD_BOOLPROP(SmokeTrail, DFALSE)
	ADD_STRINGPROP(ThrusterFilename, DEFAULT_THRUSTER_SPRITE)
	ADD_VECTORPROP_VAL(ThrusterScale, 1.0f, 1.0f, 1.0f)
END_CLASS_DEFAULT(DropShip, Prop, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DropShip::DropShip()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

DropShip::DropShip() : Prop()
{
	m_bSmokeTrail			= DFALSE;
	m_hThruster1			= DNULL;
	m_hThruster2			= DNULL;
	m_hstrThrusterFilename	= DNULL;

	VEC_SET(m_vThrusterScale, 1.0, 1.0f, 1.0f);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DropShip::~DropShip()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

DropShip::~DropShip()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	HATTACHMENT hAttachment;
	if (m_hThruster1)
	{
		if (pServerDE->FindAttachment(m_hObject, m_hThruster1, &hAttachment) == DE_OK)
		{
			pServerDE->RemoveAttachment(hAttachment);
		}

		pServerDE->RemoveObject(m_hThruster1);
		m_hThruster1 = DNULL;
	}

	if (m_hThruster2)
	{
		if (pServerDE->FindAttachment(m_hObject, m_hThruster2, &hAttachment) == DE_OK)
		{
			pServerDE->RemoveAttachment(hAttachment);
		}

		pServerDE->RemoveObject(m_hThruster2);
		m_hThruster2 = DNULL;
	}

	if (m_hstrThrusterFilename)
	{
		pServerDE->FreeString(m_hstrThrusterFilename);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DropShip::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD DropShip::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			ReadProp((ObjectCreateStruct*)pData);
			break;
		}

		case MID_INITIALUPDATE:
		{
			DDWORD dwRet = Prop::EngineMessageFn(messageID, pData, fData);

			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}

			CacheFiles();

			return dwRet;
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

	return Prop::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DropShip::PropRead()
//
//	PURPOSE:	Update properties
//
// ----------------------------------------------------------------------- //

void DropShip::ReadProp(ObjectCreateStruct *)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	pServerDE->GetPropVector("ThrusterScale", &m_vThrusterScale);
	pServerDE->GetPropBool("SmokeTrail", &m_bSmokeTrail);

	char buf[MAX_CS_FILENAME_LEN];
	buf[0] = '\0';

	pServerDE->GetPropString("ThrusterFilename", buf, MAX_CS_FILENAME_LEN);
	if (buf)
	{
		m_hstrThrusterFilename = pServerDE->CreateString(buf);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DropShip::InitialUpdate()
//
//	PURPOSE:	Handle initial update
//
// ----------------------------------------------------------------------- //

void DropShip::InitialUpdate()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	CreateThrusterSprites();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DropShip::CreateThrusterSprites()
//
//	PURPOSE:	Create thruster sprites
//
// ----------------------------------------------------------------------- //

void DropShip::CreateThrusterSprites()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	// Create the first thruster...

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	DVector vPos;
	pServerDE->GetObjectPos(m_hObject, &vPos);
	VEC_COPY(theStruct.m_Pos, vPos);

	char* pFilename = m_hstrThrusterFilename ? pServerDE->GetStringData(m_hstrThrusterFilename) : DEFAULT_THRUSTER_SPRITE;
	pFilename = pFilename ? pFilename : DEFAULT_THRUSTER_SPRITE;

	SAFE_STRCPY(theStruct.m_Filename, pFilename);
	theStruct.m_Flags = FLAG_VISIBLE | FLAG_GLOWSPRITE | FLAG_GOTHRUWORLD;
	theStruct.m_ObjectType = OT_SPRITE;

	// Okay, this is weird so listen up.  If we're creating a smoke trail
	// we're going to attach a TOW special fx.  This special fx will create
	// a rocket flare, so we don't need (to see) the thruster sprite 
	// (however we still need to create it to give us something to add 
	// the special fx message to).
	//if (m_bSmokeTrail)
	//{
	//	theStruct.m_Flags &= ~FLAG_VISIBLE;
	//	theStruct.m_Flags |= FLAG_FORCECLIENTUPDATE;
	//}

	HCLASS hClass = pServerDE->GetClass("BaseClass");
	LPBASECLASS pSprite = pServerDE->CreateObject(hClass, &theStruct);
	if (!pSprite) return;

	m_hThruster1 = pSprite->m_hObject;

	pServerDE->ScaleObject(m_hThruster1, &m_vThrusterScale);

	// Attach the sprite to the model...

	DVector vOffset;
	VEC_INIT(vOffset);

	DRotation rOffset;
	ROT_INIT(rOffset);

	HATTACHMENT hAttachment;
	DRESULT dRes = pServerDE->CreateAttachment(m_hObject, m_hThruster1, "thruster_1", 
											   &vOffset, &rOffset, &hAttachment);
	if (dRes != DE_OK)
	{
		pServerDE->RemoveObject(m_hThruster1);
		m_hThruster1 = DNULL;
	}

	if (m_bSmokeTrail)
	{
		// Create the special fx message for the object...

		HMESSAGEWRITE hMessage = pServerDE->StartSpecialEffectMessage(pSprite);
		pServerDE->WriteToMessageByte(hMessage, SFX_PROJECTILE_ID);
		pServerDE->WriteToMessageByte(hMessage, GUN_TOW_ID);
		pServerDE->EndMessage(hMessage);
	}


	// Create the second thruster...

	pSprite = pServerDE->CreateObject(hClass, &theStruct);
	if (!pSprite) return;

	m_hThruster2 = pSprite->m_hObject;

	pServerDE->ScaleObject(m_hThruster2, &m_vThrusterScale);

	dRes = pServerDE->CreateAttachment(m_hObject, m_hThruster2, "thruster_2", 
									   &vOffset, &rOffset, &hAttachment);
	if (dRes != DE_OK)
	{
		pServerDE->RemoveObject(m_hThruster2);
		m_hThruster2 = DNULL;
	}


	if (m_bSmokeTrail)
	{
		// Create the special fx message for the object...

		HMESSAGEWRITE hMessage = pServerDE->StartSpecialEffectMessage(pSprite);
		pServerDE->WriteToMessageByte(hMessage, SFX_PROJECTILE_ID);
		pServerDE->WriteToMessageByte(hMessage, GUN_TOW_ID);
		pServerDE->EndMessage(hMessage);
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DropShip::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void DropShip::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hThruster1);
	pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hThruster2);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DropShip::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void DropShip::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	pServerDE->ReadFromLoadSaveMessageObject(hRead, &m_hThruster1);
	pServerDE->ReadFromLoadSaveMessageObject(hRead, &m_hThruster2);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DropShip::CacheFiles
//
//	PURPOSE:	Cache resources used by this the object
//
// ----------------------------------------------------------------------- //

void DropShip::CacheFiles()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	char* pFilename = DNULL;
	if (m_hstrThrusterFilename)
	{
		pFilename = pServerDE->GetStringData(m_hstrThrusterFilename);
	}

	pFilename = pFilename ? pFilename : DEFAULT_THRUSTER_SPRITE;
	pServerDE->CacheFile(FT_SPRITE, pFilename);
}