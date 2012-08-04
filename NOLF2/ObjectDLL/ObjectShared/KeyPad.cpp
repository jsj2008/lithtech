// ----------------------------------------------------------------------- //
//
// MODULE  : KeyPad.cpp
//
// PURPOSE : Implementation of the Key pad model
//
// CREATED : 4/30/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "KeyPad.h"
#include "SoundMgr.h"
#include "ServerUtilities.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "WeaponMgr.h"
#include "CommandMgr.h"
#include "Spawner.h"
#include "ServerSoundMgr.h"

LINKFROM_MODULE( KeyPad );

// Statics

static char s_szGadget[]	= "GADGET";

#pragma force_active on
BEGIN_CLASS(KeyPad)
	ADD_STRINGPROP_FLAG(Filename, "Props\\Models\\Keypad.ltb", PF_DIMS | PF_LOCALDIMS | PF_FILENAME | PF_MODEL)
	ADD_STRINGPROP_FLAG(Skin, "Props\\Skins\\Keypad.dtx", PF_FILENAME)
	ADD_VISIBLE_FLAG(1, PF_HIDDEN)
	ADD_GRAVITY_FLAG(0, PF_HIDDEN)
	ADD_SOLID_FLAG(1, 0)
    ADD_BOOLPROP_FLAG(MoveToFloor, LTFALSE, PF_HIDDEN)
	ADD_REALPROP_FLAG(Alpha, 1.0f, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(CanDamage, LTFALSE, PF_GROUP(1))
    ADD_BOOLPROP_FLAG(NeverDestroy, LTTRUE, PF_GROUP(1))
    ADD_BOOLPROP_FLAG(SpaceCodeBreaker, LTFALSE, 0)
	ADD_REALPROP_FLAG(MinPickTime, 5.0f, 0)
	ADD_REALPROP_FLAG(MaxPickTime, 10.0f, 0)
	ADD_STRINGPROP(DisabledCmd, "")
	ADD_STRINGPROP_FLAG(PickSound, "Guns\\Snd\\Code_dec\\activate.wav", PF_FILENAME)
	ADD_REALPROP_FLAG(PickSoundRadius, 500.0f, 0)
END_CLASS_DEFAULT_FLAGS(KeyPad, Prop, NULL, NULL, CF_HIDDEN)	// [RP] We no longer use this object, use GadgetTarget instead.
#pragma force_active off

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyPad::KeyPad()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

KeyPad::KeyPad() : Prop()
{
    m_hstrDisabledCmd       = LTNULL;
    m_hDeciphererModel.SetReceiver( *this );
    m_hstrPickSound         = LTNULL;
    m_hPickSound            = LTNULL;
	m_fPickSoundRadius		= 0.0f;
	m_fMinPickTime			= 0.0f;
	m_fMaxPickTime			= 0.0f;
	m_bSpaceCodeBreaker		= LTFALSE;

	m_pDebrisOverride = "Metal small";
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyPad::~KeyPad()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

KeyPad::~KeyPad()
{
	FREE_HSTRING(m_hstrDisabledCmd);

	if (m_hDeciphererModel)
	{
		// Remove the model...

		HATTACHMENT hAttachment;
        if (g_pLTServer->FindAttachment(m_hObject, m_hDeciphererModel, &hAttachment) == LT_OK)
		{
            g_pLTServer->RemoveAttachment(hAttachment);
		}
        g_pLTServer->RemoveObject(m_hDeciphererModel);
        m_hDeciphererModel = LTNULL;
	}

	if (m_hPickSound)
	{
        g_pLTServer->SoundMgr()->KillSoundLoop(m_hPickSound);
        m_hPickSound = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyPad::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 KeyPad::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE :
		{
			Update();
		}
		break;

		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}
		}
		break;

		case MID_INITIALUPDATE :
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}
		}
		break;

		case MID_SAVEOBJECT:
		{
			Save((ILTMessage_Write*)pData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((ILTMessage_Read*)pData);
		}
		break;

		default : break;
	}

	return Prop::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyPad::OnTrigger
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

bool KeyPad::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Gadget(s_szGadget);

	if (cMsg.GetArg(0) == s_cTok_Gadget)
	{
		HandleGadgetMsg(cMsg);
	}
	else
		return Prop::OnTrigger(hSender, cMsg);

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyPad::SetupDisabledState
//
//	PURPOSE:	Handle gadget messages
//
// ----------------------------------------------------------------------- //

void KeyPad::HandleGadgetMsg(const CParsedMsg &cMsg)
{
	if (cMsg.GetArgCount() < 2) return;

	AMMO const *pAmmo = g_pWeaponMgr->GetAmmo(atol(cMsg.GetArg(1)));
	if (!pAmmo) return;

	if (pAmmo->eInstDamageType == DT_GADGET_CODE_DECIPHERER)
	{
		SetupDisabledState();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyPad::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL KeyPad::ReadProp(ObjectCreateStruct *pInfo)
{
    if (!pInfo) return LTFALSE;

	GenericProp genProp;

    if (g_pLTServer->GetPropGeneric("DisabledCmd", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrDisabledCmd = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("PickSound", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrPickSound = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("MinPickTime", &genProp) == LT_OK)
	{
		m_fMinPickTime = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("MaxPickTime", &genProp) == LT_OK)
	{
		m_fMaxPickTime = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("PickSoundRadius", &genProp) == LT_OK)
	{
		m_fPickSoundRadius = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("SpaceCodeBreaker", &genProp) == LT_OK)
	{
		m_bSpaceCodeBreaker = genProp.m_Bool;
	}

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyPad::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

LTBOOL KeyPad::InitialUpdate()
{
    SetNextUpdate(UPDATE_NEVER);

	// Setup the object so we can be disabled by the code decipherer...

	m_dwUsrFlgs |= USRFLG_GADGET_CODE_DECIPHERER;

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyPad::SetupDisabledState
//
//	PURPOSE:	Setup the disabled state
//
// ----------------------------------------------------------------------- //

void KeyPad::SetupDisabledState()
{
	if (m_hDeciphererModel) return;

    g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, 0, USRFLG_GADGET_CODE_DECIPHERER);

	// Create the code decipherer, and attach it to the keypad...

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

    LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);

	theStruct.m_Pos = vPos;
	SAFE_STRCPY(theStruct.m_Filename, "Guns\\Models_HH\\Codedec_hh.ltb");
	SAFE_STRCPY(theStruct.m_SkinName, "Guns\\Skins_HH\\Codedec_hh.dtx");

	theStruct.m_Flags = FLAG_VISIBLE | FLAG_GOTHRUWORLD;
	theStruct.m_ObjectType  = OT_MODEL;

    HCLASS hClass = g_pLTServer->GetClass("BaseClass");
    LPBASECLASS pModel = g_pLTServer->CreateObject(hClass, &theStruct);
	if (!pModel) return;

	m_hDeciphererModel = pModel->m_hObject;

	// Don't eat ticks please...
	::SetNextUpdate(m_hDeciphererModel, UPDATE_NEVER);

	// Set the Decipherer's animation...

    HMODELANIM hAni = g_pLTServer->GetAnimIndex(m_hDeciphererModel, "Animation");
 	if (hAni)
	{
      g_pLTServer->SetModelLooping(m_hDeciphererModel, LTTRUE);
      g_pLTServer->SetModelAnimation(m_hDeciphererModel, hAni);
	}


	// Attach the model to the the keypad...

    LTVector vOffset(0, 0, 0);

    LTRotation rOffset;

	HATTACHMENT hAttachment;
    LTRESULT dRes = g_pLTServer->CreateAttachment(m_hObject, m_hDeciphererModel, "Decipher",
											     &vOffset, &rOffset, &hAttachment);
    if (dRes != LT_OK)
	{
        g_pLTServer->RemoveObject(m_hDeciphererModel);
        m_hDeciphererModel = LTNULL;
		return;
	}

	// Play the activate sound...

	if (m_hstrPickSound)
	{
        const char* pSound = g_pLTServer->GetStringData(m_hstrPickSound);

		if (pSound)
		{
            uint32 dwFlags = (PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE);

			m_hPickSound = g_pServerSoundMgr->PlaySoundFromPos(vPos, pSound,
				m_fPickSoundRadius, SOUNDPRIORITY_MISC_LOW, dwFlags);
		}
	}

	// Update the key pad until it is disabled...

    SetNextUpdate(UPDATE_NEXT_FRAME);
	m_DecipherTimer.Start(GetRandom(m_fMinPickTime, m_fMaxPickTime));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyPad::Update
//
//	PURPOSE:	Update
//
// ----------------------------------------------------------------------- //

void KeyPad::Update()
{
	// Update deciphering the keypad code...

	if (m_DecipherTimer.Stopped())
	{
		// Keypad has been disabled...

		if (m_hstrDisabledCmd)
		{
            const char* pCmd = g_pLTServer->GetStringData(m_hstrDisabledCmd);
			if (pCmd)
			{
				g_pCmdMgr->Process(pCmd, m_hObject, m_hObject);
			}
		}


		// Stop sound...

		if (m_hPickSound)
		{
            g_pLTServer->SoundMgr()->KillSoundLoop(m_hPickSound);
            m_hPickSound = LTNULL;
		}


		// Spawn decipherer gadget powerup...

		SpawnGadget();


		// Remove decipherer model...

		if (m_hDeciphererModel)
		{
			HATTACHMENT hAttachment;
            if (g_pLTServer->FindAttachment(m_hObject, m_hDeciphererModel, &hAttachment) == LT_OK)
			{
                g_pLTServer->RemoveAttachment(hAttachment);
			}

            g_pLTServer->RemoveObject(m_hDeciphererModel);
            m_hDeciphererModel = LTNULL;
		}
	}
	else
	{
        SetNextUpdate(UPDATE_NEXT_FRAME);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyPad::SpawnGadget
//
//	PURPOSE:	Spawn the decipher gadget
//
// ----------------------------------------------------------------------- //

void KeyPad::SpawnGadget()
{
    LTVector vPos;
    LTRotation rRot;

	HMODELSOCKET hSocket;
	if (g_pModelLT->GetSocket(m_hObject, "Decipher", hSocket) == LT_OK)
	{
		LTransform transform;
        if (g_pModelLT->GetSocketTransform(m_hObject, hSocket, transform, LTTRUE) == LT_OK)
		{
			g_pTransLT->Get(transform, vPos, rRot);

			// Spawn the weapon using the id because the name has a space
			// in it (which screws up parsing)...

            const char* pGadgetName = m_bSpaceCodeBreaker ? "Space Code Breaker" : "Code Breaker";

            WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon((char*)pGadgetName);
			if (pWeapon)
			{
				char szSpawn[100];
				sprintf(szSpawn, "WeaponItem Gravity 0;MoveToFloor 0;WeaponTypeId %d", pWeapon->nId);

				SpawnObject(szSpawn, vPos, rRot);
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyPad::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void KeyPad::Save(ILTMessage_Write *pMsg)
{
	if (!pMsg) return;

	m_DecipherTimer.Save(pMsg);

    SAVE_HOBJECT(m_hDeciphererModel);
	SAVE_HSTRING(m_hstrDisabledCmd);
    SAVE_HSTRING(m_hstrPickSound);
    SAVE_FLOAT(m_fMinPickTime);
    SAVE_FLOAT(m_fMaxPickTime);
    SAVE_FLOAT(m_fPickSoundRadius);
    SAVE_BOOL(m_bSpaceCodeBreaker);
    SAVE_bool(m_hPickSound != 0);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyPad::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void KeyPad::Load(ILTMessage_Read *pMsg)
{
	if (!pMsg) return;

	m_DecipherTimer.Load(pMsg);

	LOAD_HOBJECT(m_hDeciphererModel);
    LOAD_HSTRING(m_hstrDisabledCmd);
    LOAD_HSTRING(m_hstrPickSound);
    LOAD_FLOAT(m_fMinPickTime);
    LOAD_FLOAT(m_fMaxPickTime);
    LOAD_FLOAT(m_fPickSoundRadius);
	LOAD_BOOL(m_bSpaceCodeBreaker);
	LTBOOL bPlayingSound;
    LOAD_BOOL(bPlayingSound);

	// Play the sound if necessary...

	if (bPlayingSound)
	{
        const char* pSound = g_pLTServer->GetStringData(m_hstrPickSound);

		if (pSound)
		{
            LTVector vPos;
			g_pLTServer->GetObjectPos(m_hObject, &vPos);

            uint32 dwFlags = (PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE);
			m_hPickSound = g_pServerSoundMgr->PlaySoundFromPos(vPos, pSound,
				m_fPickSoundRadius, SOUNDPRIORITY_MISC_LOW, dwFlags);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyPad::OnLinkBroken
//
//	PURPOSE:	Handle attached object getting removed.
//
// ----------------------------------------------------------------------- //

void KeyPad::OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj )
{
	if( pRef == &m_hDeciphererModel )
	{
		HATTACHMENT hAttachment;
		if ( LT_OK == g_pLTServer->FindAttachment( m_hObject, hObj, &hAttachment) )
		{
			g_pLTServer->RemoveAttachment(hAttachment);
		}
	}

	Prop::OnLinkBroken( pRef, hObj );
}

