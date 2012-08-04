// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include "iltserver.h"
#include "Attachments.h"
#include "ServerUtilities.h"
#include "ObjectMsgs.h"
#include "AttachButeMgr.h"
#include "WeaponMgr.h"
#include "iltmodel.h"
#include "ilttransform.h"
#include "HHWeaponModel.h"
#include "prop.h"
#include "Spawner.h"
#include "Body.h"
#include "WeaponItems.h"
#include "GameServerShell.h"
#include "AI.h"
#include "CharacterHitBox.h"
#include "PlayerObj.h"

// Static constants

const static char c_szNoAttachment[] = NO_ATTACHMENT;

static CBankedList<CHumanAttachments> s_bankCHumanAttachments;
static CBankedList<CPlayerAttachments> s_bankCPlayerAttachments;
static CBankedList<CVehicleAttachments> s_bankCVehicleAttachments;
static CBankedList<CHelicopterAttachments> s_bankCHelicopterAttachments; 
static CBankedList<CAnimalAttachments> s_bankCAnimalAttachments;
static CBankedList<CSharkAttachments> s_bankCSharkAttachments;

static CBankedList<CAttachmentProp> s_bankCAttachmentProp;
static CBankedList<CAttachmentObject> s_bankCAttachmentObject;
static CBankedList<CAttachmentWeapon> s_bankCAttachmentWeapon;

CAttachments* CAttachments::Create(uint32 nAttachmentsType)
{
	switch ( nAttachmentsType )
	{
		case ATTACHMENTS_TYPE_HUMAN:
			return s_bankCHumanAttachments.New();
			break;
		case ATTACHMENTS_TYPE_PLAYER:
			return s_bankCPlayerAttachments.New();
			break;
		case ATTACHMENTS_TYPE_ANIMAL:
			return s_bankCAnimalAttachments.New();
			break;
		case ATTACHMENTS_TYPE_SHARK:
			return s_bankCSharkAttachments.New();
			break;
		case ATTACHMENTS_TYPE_VEHICLE:
			return s_bankCVehicleAttachments.New();
			break;
		case ATTACHMENTS_TYPE_HELICOPTER:
			return s_bankCHelicopterAttachments.New();
			break;
	}

	return LTNULL;
}

void CAttachments::Destroy(CAttachments* pAttachments)
{
	switch ( pAttachments->GetType() )
	{
		case ATTACHMENTS_TYPE_HUMAN:
			s_bankCHumanAttachments.Delete(static_cast<CHumanAttachments*>(pAttachments));
			break;
		case ATTACHMENTS_TYPE_PLAYER:
			s_bankCPlayerAttachments.Delete(static_cast<CPlayerAttachments*>(pAttachments));
			break;
		case ATTACHMENTS_TYPE_ANIMAL:
			s_bankCAnimalAttachments.Delete(static_cast<CAnimalAttachments*>(pAttachments));
			break;
		case ATTACHMENTS_TYPE_SHARK:
			s_bankCSharkAttachments.Delete(static_cast<CSharkAttachments*>(pAttachments));
			break;
		case ATTACHMENTS_TYPE_VEHICLE:
			s_bankCVehicleAttachments.Delete(static_cast<CVehicleAttachments*>(pAttachments));
			break;
		case ATTACHMENTS_TYPE_HELICOPTER:
			s_bankCHelicopterAttachments.Delete(static_cast<CHelicopterAttachments*>(pAttachments));
			break;
	}
}

CAttachment* CAttachment::Create(uint32 nAttachmentType)
{
	switch ( nAttachmentType )
	{
		case ATTACHMENT_TYPE_WEAPON:
			return s_bankCAttachmentWeapon.New();
			break;
		case ATTACHMENT_TYPE_OBJECT:
			return s_bankCAttachmentObject.New();
			break;
		case ATTACHMENT_TYPE_PROP:
			return s_bankCAttachmentProp.New();
			break;
	}

	return LTNULL;
}

void CAttachment::Destroy(CAttachment* pAttachment)
{
	switch ( pAttachment->GetType() )
	{
		case ATTACHMENT_TYPE_WEAPON:
			s_bankCAttachmentWeapon.Delete(static_cast<CAttachmentWeapon*>(pAttachment));
			break;
		case ATTACHMENT_TYPE_OBJECT:
			s_bankCAttachmentObject.Delete(static_cast<CAttachmentObject*>(pAttachment));
			break;
		case ATTACHMENT_TYPE_PROP:
			s_bankCAttachmentProp.Delete(static_cast<CAttachmentProp*>(pAttachment));
			break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::CAttachments
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CAttachments::CAttachments() : IAggregate()
{
    m_hObject = LTNULL;

	m_cAttachmentPositions = 0;

	m_cWeapons = 0;
	m_cObjects = 0;
	m_cProps = 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::~CAttachments
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CAttachments::~CAttachments()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::EngineMessageFn
//
//	PURPOSE:	Handle message from the engine
//
// ----------------------------------------------------------------------- //

uint32 CAttachments::EngineMessageFn(LPBASECLASS pObject, uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			int nInfo = (int)fData;
			if (nInfo == PRECREATE_WORLDFILE || nInfo == PRECREATE_STRINGPROP)
			{
				ReadProp(pObject, (ObjectCreateStruct*)pData);
			}
			break;
		}

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				Init(pObject->m_hObject);
			}
			break;
		}

		case MID_UPDATE:
		{
			Update();
		}
		break;

		case MID_LINKBROKEN :
		{
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
	}

    return IAggregate::EngineMessageFn(pObject, messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 CAttachments::ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	switch(messageID)
	{
		case MID_TRIGGER:
		{
			HandleTrigger(pObject, hSender, hRead);
		}
		break;

		default : break;
	}

	{for ( int iAttachmentPosition = 0 ; iAttachmentPosition < m_cAttachmentPositions ; iAttachmentPosition++ )
	{
		m_apAttachmentPositions[iAttachmentPosition]->ObjectMessageFn(pObject, hSender, messageID, hRead);
	}}

    return IAggregate::ObjectMessageFn(pObject, hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL CAttachments::ReadProp(BaseClass *pObject, ObjectCreateStruct *pStruct)
{
	_ASSERT(pObject && pStruct);
	if (!pObject || !pStruct) return LTFALSE;

	GenericProp genProp;

	{for ( int iAttachmentPosition = 0 ; iAttachmentPosition < m_cAttachmentPositions ; iAttachmentPosition++ )
	{
        if ( g_pLTServer->GetPropGeneric( (char*)m_apAttachmentPositions[iAttachmentPosition]->GetName(), &genProp ) == LT_OK )
			if ( genProp.m_String[0] )
			{
                m_apAttachmentPositions[iAttachmentPosition]->SetAttachmentName(g_pLTServer->CreateString(genProp.m_String));
			}
	}}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::Attach
//
//	PURPOSE:	Dynamically add an attachment
//
// ----------------------------------------------------------------------- //

void CAttachments::Attach(const char* szAttachmentPosition, const char* szAttachment)
{
	for ( int iAttachmentPosition = 0 ; iAttachmentPosition < m_cAttachmentPositions ; iAttachmentPosition++ )
	{
		CAttachmentPosition* pAttachmentPosition = m_apAttachmentPositions[iAttachmentPosition];

		if ( !_stricmp(szAttachmentPosition, pAttachmentPosition->GetName()) )
		{
			if ( pAttachmentPosition->HasAttachment() )
			{
				_ASSERT(!"CAttachments::Attach -- tried attaching to an occupied position");
			}
			else
			{
                pAttachmentPosition->SetAttachmentName(g_pLTServer->CreateString((char*)szAttachment));
				CreateAttachment(pAttachmentPosition);
			}

			return;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::Detach
//
//	PURPOSE:	Dynamically remove an attachment
//
// ----------------------------------------------------------------------- //

void CAttachments::Detach(const char* szAttachmentPosition)
{
	for ( int iAttachmentPosition = 0 ; iAttachmentPosition < m_cAttachmentPositions ; iAttachmentPosition++ )
	{
		CAttachmentPosition* pAttachmentPosition = m_apAttachmentPositions[iAttachmentPosition];

		if ( !_stricmp(szAttachmentPosition, pAttachmentPosition->GetName()) )
		{
			CAttachment* pAttachment = pAttachmentPosition->GetAttachment();
			if ( pAttachment )
			{
				switch ( pAttachment->GetType() )
				{
					case ATTACHMENT_TYPE_WEAPON:
						m_cWeapons--;
						break;

					case ATTACHMENT_TYPE_PROP:
						m_cProps--;
						break;

					case ATTACHMENT_TYPE_OBJECT:
						m_cObjects--;
						break;
				}

                g_pLTServer->RemoveObject(pAttachment->GetModel());
				CAttachment::Destroy(pAttachment);

				pAttachmentPosition->SetAttachment(NULL);
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::CreateAttachment
//
//	PURPOSE:	Creates an attachment
//
// ----------------------------------------------------------------------- //

void CAttachments::CreateAttachment(CAttachmentPosition *pAttachmentPosition)
{
	_ASSERT(pAttachmentPosition);
	if ( !pAttachmentPosition ) return;

	if ( pAttachmentPosition->HasAttachment() ) return;
	if ( !pAttachmentPosition->HasAttachmentName() ) return;

    char *szAttachmentName = g_pLTServer->GetStringData(pAttachmentPosition->GetAttachmentName());

    if ( !_stricmp(c_szNoAttachment, szAttachmentName) ) return;

    uint8 nWeaponID = WMGR_INVALID_ID;
    uint8 nAmmoID = WMGR_INVALID_ID;
	char szAttachmentNameCopy[128];
	strcpy(szAttachmentNameCopy, szAttachmentName);
	g_pWeaponMgr->ReadWeapon(szAttachmentNameCopy, nWeaponID, nAmmoID);

	if ( nWeaponID != WMGR_INVALID_ID )
	{
		CreateWeaponAttachment(pAttachmentPosition, szAttachmentName, nWeaponID, nAmmoID);
		return;
	}

	int nAttachmentID = g_pAttachButeMgr->GetAttachmentIDByName(szAttachmentName);

	if ( -1 == nAttachmentID )
	{
        g_pLTServer->CPrint("Unmatched attable name %s (Line %d of %s)", szAttachmentName, __LINE__, __FILE__);
		return;
	}

	switch ( g_pAttachButeMgr->GetAttachmentType(nAttachmentID) )
	{
		case ATTACHMENT_TYPE_OBJECT:
		{
			CreateObjectAttachment(pAttachmentPosition, nAttachmentID);
		}
		break;

		case ATTACHMENT_TYPE_PROP:
		{
			CreatePropAttachment(pAttachmentPosition, nAttachmentID);
		}
		break;

		default:
		{
            g_pLTServer->CPrint("Illegal attachment type encountered on attachment #%d", nAttachmentID);
			return;
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::CreateWeaponAttachment
//
//	PURPOSE:	Creates a weapon attachment
//
// ----------------------------------------------------------------------- //

void CAttachments::CreateWeaponAttachment(CAttachmentPosition *pAttachmentPosition, const char* szAttachmentName, uint8 nWeaponID, uint8 nAmmoID)
{
	// Prepare the object create struct

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	theStruct.m_Flags = FLAG_VISIBLE | FLAG_GOTHRUWORLD | FLAG_PORTALVISIBLE;
	theStruct.m_NextUpdate = 1.0f;

	// Get the attachment skin, model, and class names

	char szModel[MAX_CS_FILENAME_LEN];
	char szSkin[MAX_CS_FILENAME_LEN];
	char szClass[128];

	szModel[0] = 0;
	szSkin[0] = 0;

	CAttachment* pAttachment = CAttachment::Create(ATTACHMENT_TYPE_WEAPON);
	strcpy(szClass, "CHHWeaponModel");

	// Parse weapon/ammo info

	WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(nWeaponID);
	if (pWeapon)
	{
		if (pWeapon->szHHModel[0])
		{
			SAFE_STRCPY(szModel, pWeapon->szHHModel);
		}
		if (pWeapon->szHHSkin[0])
		{
			SAFE_STRCPY(szSkin, pWeapon->szHHSkin);
		}
	}

	// Set up the model/skin filenames

	SAFE_STRCPY(theStruct.m_Filename, szModel);
	SAFE_STRCPY(theStruct.m_SkinName, szSkin);

	// Create the attachment

    HCLASS hClass = g_pLTServer->GetClass(szClass);
    LPBASECLASS pModel = g_pLTServer->CreateObject(hClass, &theStruct);
	if ( !pModel ) return;

	// Attach it

	HATTACHMENT hAttachment;
    LTRESULT dRes = g_pLTServer->CreateAttachment(m_hObject, pModel->m_hObject, (char*)pAttachmentPosition->GetName(),
												 &LTVector(0,0,0), &LTRotation(0,0,0,1), &hAttachment);

	((CAttachmentWeapon*)pAttachment)->Init(m_hObject, pModel->m_hObject, -1, nWeaponID, nAmmoID);

	pAttachmentPosition->SetAttachment(pAttachment);

	m_cWeapons++;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::CreateObjectAttachment
//
//	PURPOSE:	Creates an object attachment
//
// ----------------------------------------------------------------------- //

void CAttachments::CreateObjectAttachment(CAttachmentPosition *pAttachmentPosition, int nAttachmentID)
{
	// Prepare the object create struct

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	theStruct.m_Flags = FLAG_VISIBLE | FLAG_GOTHRUWORLD | FLAG_PORTALVISIBLE;
	theStruct.m_NextUpdate = 1.0f;

	// Get the attachment skin, model, and class names

	int nAttachmentType = g_pAttachButeMgr->GetAttachmentType(nAttachmentID);

	char szClass[128];
	szClass[0] = '\0';

	SAFE_STRCPY(szClass, g_pAttachButeMgr->GetAttachmentClass(nAttachmentID));
	if ( !szClass[0] ) return;


	// Set up the model/skin filenames

	SAFE_STRCPY(theStruct.m_Filename, g_pAttachButeMgr->GetAttachmentModel(nAttachmentID));
	SAFE_STRCPY(theStruct.m_SkinName, g_pAttachButeMgr->GetAttachmentSkin(nAttachmentID));

	// Create the attachment

    HCLASS hClass = g_pLTServer->GetClass(szClass);
    LPBASECLASS pModel = g_pLTServer->CreateObjectProps(hClass, &theStruct, (char*)(LPCSTR)g_pAttachButeMgr->GetAttachmentProperties(nAttachmentID));
	if ( !pModel ) return;

	// Attach it

	HATTACHMENT hAttachment;
    LTRESULT dRes = g_pLTServer->CreateAttachment(m_hObject, pModel->m_hObject, (char*)pAttachmentPosition->GetName(),
											   &LTVector(0,0,0), &LTRotation(0,0,0,1), &hAttachment);

	CAttachment* pAttachment = CAttachment::Create(ATTACHMENT_TYPE_OBJECT);
	pAttachment->Init(m_hObject, pModel->m_hObject, nAttachmentID);
	pAttachmentPosition->SetAttachment(pAttachment);

	m_cObjects++;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::CreatePropAttachment
//
//	PURPOSE:	Creates a prop attachment
//
// ----------------------------------------------------------------------- //

void CAttachments::CreatePropAttachment(CAttachmentPosition *pAttachmentPosition, int nAttachmentID)
{
	// Prepare the object create struct

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	theStruct.m_Flags = FLAG_VISIBLE | FLAG_GOTHRUWORLD | FLAG_PORTALVISIBLE;
	theStruct.m_NextUpdate = 1.0f;

	// Get the attachment skin, model, and class names

	int nAttachmentType = g_pAttachButeMgr->GetAttachmentType(nAttachmentID);

	char szModel[MAX_CS_FILENAME_LEN];
	char szSkin[MAX_CS_FILENAME_LEN];
	char szClass[128];

	CAttachment* pAttachment = CAttachment::Create(ATTACHMENT_TYPE_PROP);
	strcpy(szClass, "Prop");
	strcpy(szModel, g_pAttachButeMgr->GetAttachmentModel(nAttachmentID));
	strcpy(szSkin, g_pAttachButeMgr->GetAttachmentSkin(nAttachmentID));

	// Set up the model/skin filenames

	SAFE_STRCPY(theStruct.m_Filename, szModel);
	SAFE_STRCPY(theStruct.m_SkinName, szSkin);

	// Create the attachment

    HCLASS hClass = g_pLTServer->GetClass(szClass);
    Prop* pModel = (Prop*)g_pLTServer->CreateObjectProps(hClass, &theStruct, (char*)(LPCSTR)g_pAttachButeMgr->GetAttachmentProperties(nAttachmentID));
	if ( !pModel ) return;

	pModel->GetDestructible()->SetNeverDestroy(LTTRUE);

	LTRotation rRot;
    g_pLTServer->GetObjectRotation(pModel->m_hObject, &rRot);

	// Attach it

	HATTACHMENT hAttachment;
    LTRESULT dRes = g_pLTServer->CreateAttachment(m_hObject, pModel->m_hObject, (char*)pAttachmentPosition->GetName(),
											   &LTVector(0,0,0), &rRot, &hAttachment);

	pAttachment->Init(m_hObject, pModel->m_hObject, nAttachmentID);
	pAttachmentPosition->SetAttachment(pAttachment);

	m_cProps++;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::ReInit
//
//	PURPOSE:	Re-initialize the attachments with a new object
//
// ----------------------------------------------------------------------- //

void CAttachments::ReInit(HOBJECT hNewObj)
{
	if (!hNewObj || !m_hObject) return;

	HOBJECT hOldObj = m_hObject;
	m_hObject = hNewObj;

	for (int i=0; i < m_cAttachmentPositions; i++)
	{
		CAttachmentPosition *pCurPos = m_apAttachmentPositions[i];
		if (!pCurPos) break;

		CAttachment* pCur = pCurPos->GetAttachment();

		if (pCur)
		{
			HOBJECT hModel = pCur->GetModel();

			if (hModel)
			{
				HATTACHMENT hAttachment;
                if (LT_OK == g_pLTServer->FindAttachment(hOldObj, hModel, &hAttachment))
				{
                    g_pLTServer->RemoveAttachment(hAttachment);
				}

				// Attach the model to us...

                g_pLTServer->CreateAttachment(m_hObject, hModel, (char*)pCurPos->GetName(),
					&LTVector(0,0,0), &LTRotation(0,0,0,1), &hAttachment);

				// Re-init to set m_hObject...

				pCur->Init(m_hObject, hModel, pCur->GetID());
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::EnumerateWeapons
//
//	PURPOSE:	Fills out an array with a list of all weapon attachments
//
// ----------------------------------------------------------------------- //

int CAttachments::EnumerateWeapons(CWeapon** apWeapons, CAttachmentPosition** apAttachmentPositions, int cMaxWeapons)
{
	int cWeapons = 0;

	for ( int iAttachmentPosition = 0 ; iAttachmentPosition < m_cAttachmentPositions ; iAttachmentPosition++ )
	{
		CAttachment* pAttachment = m_apAttachmentPositions[iAttachmentPosition]->GetAttachment();
		if ( pAttachment && (pAttachment->GetType() == ATTACHMENT_TYPE_WEAPON) )
		{
			CAttachmentWeapon* pAttachmentWeapon = (CAttachmentWeapon*)pAttachment;
            CHHWeaponModel* pHHWeaponModel = (CHHWeaponModel*)g_pLTServer->HandleToObject(pAttachmentWeapon->GetModel());

			apAttachmentPositions[cWeapons] = m_apAttachmentPositions[iAttachmentPosition];
			apWeapons[cWeapons] = pHHWeaponModel->GetParent();

			if ( ++cWeapons == cMaxWeapons )
			{
				break;
			}
		}
	}

	return cWeapons;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::EnumerateProps
//
//	PURPOSE:	Fills out an array with a list of all Prop attachments
//
// ----------------------------------------------------------------------- //

int CAttachments::EnumerateProps(Prop** apProps, CAttachmentPosition** apAttachmentPositions, int cMaxProps)
{
	int cProps = 0;

	for ( int iAttachmentPosition = 0 ; iAttachmentPosition < m_cAttachmentPositions ; iAttachmentPosition++ )
	{
		CAttachment* pAttachment = m_apAttachmentPositions[iAttachmentPosition]->GetAttachment();
		if ( pAttachment && (pAttachment->GetType() == ATTACHMENT_TYPE_PROP) )
		{
			CAttachmentProp* pAttachmentProp = (CAttachmentProp*)pAttachment;
            Prop* pProp = (Prop*)g_pLTServer->HandleToObject(pAttachmentProp->GetModel());

			apAttachmentPositions[cProps] = m_apAttachmentPositions[iAttachmentPosition];
			apProps[cProps] = pProp;

			if ( ++cProps == cMaxProps )
			{
				break;
			}
		}
	}

	return cProps;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::EnumerateObjects
//
//	PURPOSE:	Fills out an array with a list of all Object attachments
//
// ----------------------------------------------------------------------- //

int CAttachments::EnumerateObjects(BaseClass** apObjects, CAttachmentPosition** apAttachmentPositions, int cMaxObjects)
{
	int cObjects = 0;

	for ( int iAttachmentPosition = 0 ; iAttachmentPosition < m_cAttachmentPositions ; iAttachmentPosition++ )
	{
		CAttachment* pAttachment = m_apAttachmentPositions[iAttachmentPosition]->GetAttachment();
		if ( pAttachment && (pAttachment->GetType() == ATTACHMENT_TYPE_OBJECT) )
		{
			CAttachmentObject* pAttachmentObject = (CAttachmentObject*)pAttachment;
            BaseClass* pObject = g_pLTServer->HandleToObject(pAttachmentObject->GetModel());

			apAttachmentPositions[cObjects] = m_apAttachmentPositions[iAttachmentPosition];
			apObjects[cObjects] = pObject;

			if ( ++cObjects == cMaxObjects )
			{
				break;
			}
		}
	}

	return cObjects;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::GetInfiniteAmmo
//
//	PURPOSE:	Used by the AIs to get a big mess of ammunition
//
// ----------------------------------------------------------------------- //

void CAttachments::GetInfiniteAmmo()
{
	for ( int iAttachmentPosition = 0 ; iAttachmentPosition < m_cAttachmentPositions ; iAttachmentPosition++ )
	{
		CAttachment* pAttachment = m_apAttachmentPositions[iAttachmentPosition]->GetAttachment();
		if ( pAttachment && (pAttachment->GetType() == ATTACHMENT_TYPE_WEAPON) )
		{
			CAttachmentWeapon* pAttachmentWeapon = (CAttachmentWeapon*)pAttachment;
			CWeapons* pWeapons = pAttachmentWeapon->GetWeapons();

			int cAmmoTypes = g_pWeaponMgr->GetNumAmmoTypes();

			for ( int iAmmoType = 0 ; iAmmoType < cAmmoTypes ; iAmmoType++ )
			{
				pWeapons->AddAmmo(iAmmoType, 1000);
			}

			pWeapons->GetCurWeapon()->ReloadClip(LTFALSE);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::Init
//
//	PURPOSE:	Initializes us
//
// ----------------------------------------------------------------------- //

void CAttachments::Init(HOBJECT hObject)
{
	if (!hObject) return;
	if (!m_hObject) m_hObject = hObject;

	for ( int iAttachmentPosition = 0 ; iAttachmentPosition < m_cAttachmentPositions ; iAttachmentPosition++ )
	{
		CreateAttachment(m_apAttachmentPositions[iAttachmentPosition]);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::ResetRequirements
//
//	PURPOSE:	Initializes us
//
// ----------------------------------------------------------------------- //

void CAttachments::ResetRequirements()
{
	for ( int iAttachmentPosition = 0 ; iAttachmentPosition < m_cAttachmentPositions ; iAttachmentPosition++ )
	{
		CAttachmentPosition* pAttachmentPosition = m_apAttachmentPositions[iAttachmentPosition];
		CAttachment* pAttachment = m_apAttachmentPositions[iAttachmentPosition]->GetAttachment();

		if ( pAttachment && pAttachment->GetType() != ATTACHMENT_TYPE_WEAPON )
		{
			CAttachment::Destroy(pAttachment);

			pAttachmentPosition->SetAttachment(NULL);
		}
	}

	m_cWeapons = 1;
	m_cProps = 0;
	m_cObjects = 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::AddRequirements
//
//	PURPOSE:	Initializes us
//
// ----------------------------------------------------------------------- //

void CAttachments::AddRequirements(ModelId eModelId, ModelStyle eModelStyle)
{
	char* szModel = (char*)g_pModelButeMgr->GetModelName(eModelId);
	char* szStyle = (char*)g_pModelButeMgr->GetStyleName(eModelStyle);

	int anRequirements[32];
	int cRequirements = g_pAttachButeMgr->GetRequirementIDs(szModel, szStyle, anRequirements, 32);

	for ( int iRequirement = 0 ; iRequirement < cRequirements ; iRequirement++ )
	{
		int nAttachment = g_pAttachButeMgr->GetRequirementAttachment(anRequirements[iRequirement]);
		CString sSocket = g_pAttachButeMgr->GetRequirementSocket(anRequirements[iRequirement]);
		CString sAttachment = g_pAttachButeMgr->GetAttachmentName(nAttachment);

		Attach(sSocket, sAttachment);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::Update
//
//	PURPOSE:	Handle object updates
//
// ----------------------------------------------------------------------- //

void CAttachments::Update()
{
	/*	// Put all attachments in their place

	{for ( int iAttachmentPosition = 0 ; iAttachmentPosition < m_cAttachmentPositions ; iAttachmentPosition++ )
	{
		CAttachmentPosition* pAttachmentPosition = m_apAttachmentPositions[iAttachmentPosition];

		if ( pAttachmentPosition->GetAttachment()->HasObject() )
		{
			// Remove the attached model

			HOBJECT hModel = pAttachmentPosition->GetAttachment()->GetObject();

			HATTACHMENT hAttachment;
            if ( LT_OK == g_pLTServer->FindAttachment(m_hObject, hModel, &hAttachment) )
			{
                g_pLTServer->RemoveAttachment(hAttachment);
			}

			// Determine the direction to point

			LTRotation rRotModel;
            g_pLTServer->GetObjectRotation(hModel, &rRotModel);

			HMODELSOCKET hSocket;
			g_pModelLT->GetSocket(m_hObject, (char*)pAttachmentPosition->GetName(), hSocket);

			LTransform transform;
			g_pModelLT->GetSocketTransform(m_hObject, hSocket, transform, LTTRUE);

			LTRotation rRotSocket;
			g_pTransLT->GetRot(transform, rRotSocket);

            LTMatrix m1, m2, m3;

            g_pLTServer->SetupRotationMatrix(&m1, &rRotModel);
            g_pLTServer->SetupRotationMatrix(&m2, &rRotSocket);

			MatTranspose3x3(&m1);
			MatMul(&m3, &m2, &m1);
            g_pLTServer->SetupRotationFromMatrix(&rRotModel, &m3);

            g_pLTServer->SetObjectRotation(hModel, &rRotModel);

            LTRESULT dRes = g_pLTServer->CreateAttachment(m_hObject, hModel, (char*)pAttachmentPosition->GetName(),
													   &LTVector(0,0,0), &rRotModel, &hAttachment);
		}
	}}
*/}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::HandleProjectileImpact
//
//	PURPOSE:	Handles being hit by a projectile
//
// ----------------------------------------------------------------------- //

LTBOOL CAttachments::HandleProjectileImpact(CProjectile* pProjectile, IntersectInfo& iInfo, LTVector& vDir, LTVector& vFrom, ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	for ( int iAttachmentPosition = 0 ; iAttachmentPosition < m_cAttachmentPositions ; iAttachmentPosition++ )
	{
		CAttachmentPosition* pPosition = m_apAttachmentPositions[iAttachmentPosition];

		if ( !pPosition->HasAttachment() ) continue;

		CAttachment* pAttachment = pPosition->GetAttachment();
		if ( pAttachment->HandleProjectileImpact(pProjectile, pPosition, iInfo, vDir, vFrom) )
		{
			return LTTRUE;
		}
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::HandleDeath
//
//	PURPOSE:	Handles being killed
//
// ----------------------------------------------------------------------- //

void CAttachments::HandleDeath()
{
	for ( int iAttachmentPosition = 0 ; iAttachmentPosition < m_cAttachmentPositions ; iAttachmentPosition++ )
	{
		CAttachment* pAttachment = m_apAttachmentPositions[iAttachmentPosition]->GetAttachment();
		if ( pAttachment && (pAttachment->GetType() == ATTACHMENT_TYPE_OBJECT) )
		{
			// Need to remove any attached characters from the charactermgr

			HOBJECT hModel = pAttachment->GetModel();
			if ( IsCharacter(hModel) )
			{
                g_pCharacterMgr->Remove((CCharacter*)g_pLTServer->HandleToObject(hModel));
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::HandleTrigger
//
//	PURPOSE:	Handle trigger message.
//
// ----------------------------------------------------------------------- //

void CAttachments::HandleTrigger(LPBASECLASS pObject, HOBJECT hSender, HMESSAGEREAD hRead)
{
	const char* szMsg = (const char*)g_pLTServer->ReadFromMessageDWord(hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CAttachments::Save(HMESSAGEWRITE hWrite, uint8 nType)
{
	if (!hWrite) return;

	SAVE_HOBJECT(m_hObject);
	SAVE_INT(m_cAttachmentPositions);
	SAVE_INT(m_cWeapons);
	SAVE_INT(m_cObjects);
	SAVE_INT(m_cProps);

	for ( int iAttachmentPosition = 0 ; iAttachmentPosition < m_cAttachmentPositions ; iAttachmentPosition++ )
	{
		m_apAttachmentPositions[iAttachmentPosition]->Save(hWrite);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CAttachments::Load(HMESSAGEREAD hRead, uint8 nType)
{
	if (!hRead) return;

	LOAD_HOBJECT(m_hObject);
	LOAD_INT(m_cAttachmentPositions);
	LOAD_INT(m_cWeapons);
	LOAD_INT(m_cObjects);
	LOAD_INT(m_cProps);

	{for ( int iAttachmentPosition = 0 ; iAttachmentPosition < m_cAttachmentPositions ; iAttachmentPosition++ )
	{
		m_apAttachmentPositions[iAttachmentPosition]->Load(hRead);
	}}

	{for ( int iAttachmentPosition = m_cAttachmentPositions ; iAttachmentPosition < MAX_ATTACHMENT_POSITIONS ; iAttachmentPosition++ )
	{
        m_apAttachmentPositions[iAttachmentPosition] = LTNULL;
	}}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHumanAttachments::CAttachments
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CHumanAttachments::CHumanAttachments() : CAttachments()
{
	m_RightHand.SetName("RightHand");
	m_LeftHand.SetName("LeftHand");
	m_Head.SetName("Head");
	m_Eyes.SetName("Eyes");
	m_Nose.SetName("Nose");
	m_Chin.SetName("Chin");
	m_Back.SetName("Back");
	m_RightFoot.SetName("RightFoot");
	m_LeftFoot.SetName("LeftFoot");
	m_Motorcycle.SetName("Motorcycle");
	m_Snowmobile.SetName("Snowmobile");

	m_apAttachmentPositions[m_cAttachmentPositions++] = &m_RightHand;
	m_apAttachmentPositions[m_cAttachmentPositions++] = &m_LeftHand;
	m_apAttachmentPositions[m_cAttachmentPositions++] = &m_Head;
	m_apAttachmentPositions[m_cAttachmentPositions++] = &m_Eyes;
	m_apAttachmentPositions[m_cAttachmentPositions++] = &m_Nose;
	m_apAttachmentPositions[m_cAttachmentPositions++] = &m_Chin;
	m_apAttachmentPositions[m_cAttachmentPositions++] = &m_Back;
	m_apAttachmentPositions[m_cAttachmentPositions++] = &m_RightFoot;
	m_apAttachmentPositions[m_cAttachmentPositions++] = &m_LeftFoot;
	m_apAttachmentPositions[m_cAttachmentPositions++] = &m_Snowmobile;
	m_apAttachmentPositions[m_cAttachmentPositions++] = &m_Motorcycle;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerAttachments::CPlayerAttachments
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CPlayerAttachments::CPlayerAttachments()
{
    m_RightHand.SetAttachmentName(g_pLTServer->CreateString("Fisty Cuffs"));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerAttachments::HandleCheatFullAmmo
//
//	PURPOSE:	Do the full ammo cheat for the player
//
// ----------------------------------------------------------------------- //

void CPlayerAttachments::HandleCheatFullAmmo()
{
    if ( LTNULL != GetDefaultAttachmentWeapon() )
	{
		CWeapons* pWeapons = GetDefaultAttachmentWeapon()->GetWeapons();

        uint8 nNumAmmoTypes = g_pWeaponMgr->GetNumAmmoTypes();

		for (int i=0; i < nNumAmmoTypes; i++)
		{
			pWeapons->SetAmmo(i);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerAttachments::HandleCheatFullWeapon
//
//	PURPOSE:	Do the full weapon cheat for the player
//
// ----------------------------------------------------------------------- //

void CPlayerAttachments::HandleCheatFullWeapon()
{
    if ( LTNULL != GetDefaultAttachmentWeapon() )
	{
		HandleCheatFullAmmo();

		CWeapons* pWeapons = GetDefaultAttachmentWeapon()->GetWeapons();

		int cWeapons = g_pWeaponMgr->GetNumWeapons();
		for ( int iWeapon = 0 ; iWeapon < cWeapons ; iWeapon++ )
		{
			if (g_pWeaponMgr->IsPlayerWeapon(iWeapon))
			{
				pWeapons->ObtainWeapon(iWeapon, AMMO_DEFAULT_ID, 10000, LTTRUE);
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerAttachments::HandleCheatFullMods
//
//	PURPOSE:	Do the full mod cheat for the player
//
// ----------------------------------------------------------------------- //

void CPlayerAttachments::HandleCheatFullMods()
{
    if ( LTNULL != GetDefaultAttachmentWeapon() )
	{
		CWeapons* pWeapons = GetDefaultAttachmentWeapon()->GetWeapons();

		int cWeapons = g_pWeaponMgr->GetNumWeapons();

		for ( int iWeapon = 0 ; iWeapon < cWeapons ; iWeapon++ )
		{
			WEAPON* pWeaponData = g_pWeaponMgr->GetWeapon(iWeapon);

			if (pWeaponData)
			{
				for ( int iModNum = 0 ; iModNum < pWeaponData->nNumModTypes ; iModNum++ )
				{
					pWeapons->ObtainMod(iWeapon, pWeaponData->aModTypes[iModNum],LTTRUE);
				}
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleAttachments::CAttachments
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CVehicleAttachments::CVehicleAttachments() : CAttachments()
{
	m_Driver.SetName("Driver");

	m_apAttachmentPositions[m_cAttachmentPositions++] = &m_Driver;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHelicopterAttachments::CAttachments
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CHelicopterAttachments::CHelicopterAttachments() : CVehicleAttachments()
{
	m_FrontBelly.SetName("FrontBelly");
	m_RearBelly.SetName("RearBelly");
	m_RightGunner.SetName("RightGunner");
	m_LeftSkid.SetName("LeftSkid");
	m_RightSkid.SetName("RightSkid");
	m_TopRotor.SetName("TopRotor");
	m_BackRotor.SetName("BackRotor");

	m_apAttachmentPositions[m_cAttachmentPositions++] = &m_FrontBelly;
	m_apAttachmentPositions[m_cAttachmentPositions++] = &m_RearBelly;
	m_apAttachmentPositions[m_cAttachmentPositions++] = &m_RightGunner;
	m_apAttachmentPositions[m_cAttachmentPositions++] = &m_LeftSkid;
	m_apAttachmentPositions[m_cAttachmentPositions++] = &m_RightSkid;
	m_apAttachmentPositions[m_cAttachmentPositions++] = &m_TopRotor;
	m_apAttachmentPositions[m_cAttachmentPositions++] = &m_BackRotor;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSharkAttachments::CAttachments
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CSharkAttachments::CSharkAttachments() : CAnimalAttachments()
{
	m_Mouth.SetName("Mouth");

	m_apAttachmentPositions[m_cAttachmentPositions++] = &m_Mouth;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachment::CAttachment
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CAttachment::CAttachment()
{
    m_hModel        = LTNULL;
    m_hObject       = LTNULL;
	m_nAttachmentID	= -1;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachment::~CAttachment
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CAttachment::~CAttachment()
{
	if ( m_hObject && m_hModel )
	{
        HATTACHMENT hAttachment = LTNULL;

        g_pLTServer->FindAttachment(m_hObject, m_hModel, &hAttachment);

        if ( hAttachment != LTNULL )
		{
            g_pLTServer->RemoveAttachment(hAttachment);
		}

        g_pLTServer->RemoveObject(m_hModel);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachment::Init
//
//	PURPOSE:	Initialilze the attachment
//
// ----------------------------------------------------------------------- //

void CAttachment::Init(HOBJECT hObj, HOBJECT hModel, int nAttachmentID)
{
	m_hObject = hObj;
	m_hModel = hModel;
	m_nAttachmentID = nAttachmentID;

    uint32 dwUsrFlags = g_pLTServer->GetObjectUserFlags(hModel);
    g_pLTServer->SetObjectUserFlags(hModel, dwUsrFlags | USRFLG_ATTACH_HIDE1SHOW3);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachment::Save
//
//	PURPOSE:	Save
//
// ----------------------------------------------------------------------- //

void CAttachment::Save(HMESSAGEWRITE hWrite)
{
	SAVE_HOBJECT(m_hModel);
	SAVE_HOBJECT(m_hObject);
	SAVE_INT(m_nAttachmentID);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachment::Load
//
//	PURPOSE:	Load
//
// ----------------------------------------------------------------------- //

void CAttachment::Load(HMESSAGEREAD hRead)
{
	LOAD_HOBJECT(m_hModel);
	LOAD_HOBJECT(m_hObject);
	LOAD_INT(m_nAttachmentID);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachmentProp::HandleProjectileImpact
//
//	PURPOSE:	Tests being hit by a projectile
//
// ----------------------------------------------------------------------- //

LTBOOL CAttachmentProp::HandleProjectileImpact(CProjectile* pProjectile, CAttachmentPosition* pAttachmentPosition, IntersectInfo& iInfo, LTVector& vDir, LTVector& vFrom)
{
	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachmentProp::CreateSpawnString
//
//	PURPOSE:	Creates a spawn string for when we are dropped
//
// ----------------------------------------------------------------------- //

void CAttachmentProp::CreateSpawnString(char* szSpawn)
{
	sprintf(szSpawn, "Prop Filename %s; Skin %s; Gravity 0",
		g_pAttachButeMgr->GetAttachmentModel(m_nAttachmentID), g_pAttachButeMgr->GetAttachmentSkin(m_nAttachmentID));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachmentObject::HandleProjectileImpact
//
//	PURPOSE:	Tests being hit by a projectile
//
// ----------------------------------------------------------------------- //

LTBOOL CAttachmentObject::HandleProjectileImpact(CProjectile* pProjectile, CAttachmentPosition* pAttachmentPosition, IntersectInfo& iInfo, LTVector& vDir, LTVector& vFrom)
{
	if ( IsAI(m_hModel) )
	{
		LTVector vPos;
		LTVector vDims;
		HMODELSOCKET hSocket;
		LTransform transform;

		if ( !g_pModelLT->GetSocket(m_hObject, (char*)pAttachmentPosition->GetName(), hSocket) == LT_OK )
		{
			return LTFALSE;
		}

		if ( !g_pModelLT->GetSocketTransform(m_hObject, hSocket, transform, LTTRUE) == LT_OK )
		{
			return LTFALSE;
		}

		g_pTransLT->GetPos(transform, vPos);
        g_pLTServer->GetObjectDims(m_hModel, &vDims);

		LTVector vMinBox = vPos - vDims;
		LTVector vMaxBox = vPos + vDims;

		// TOOD: 2000 is kind of arbitrary. never mind the fact that we can shoot through the model this is
		// an attachment of ... so ... whatever.
		if ( DoesSegmentIntersectAABB(iInfo.m_Point, iInfo.m_Point+vDir*2000.0f, vMinBox, vMaxBox) )
		{
            CAI* pAI = (CAI*)g_pLTServer->HandleToObject(m_hModel);
			if ( pAI->CanBeDamagedAsAttachment() )
			{
                g_pLTServer->CPrint("dam: %f -> %f", g_pLTServer->GetTime(), pAI->GetHitPoints());
				iInfo.m_hObject = m_hModel;
				return LTTRUE;
			}
		}
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachmentObject::CreateSpawnString
//
//	PURPOSE:	Creates a spawn string for when we are dropped
//
// ----------------------------------------------------------------------- //

void CAttachmentObject::CreateSpawnString(char* szSpawn)
{
	sprintf(szSpawn, "Prop Filename %s; Skin %s; Gravity 0",
		g_pAttachButeMgr->GetAttachmentModel(m_nAttachmentID), g_pAttachButeMgr->GetAttachmentSkin(m_nAttachmentID));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachmentWeapon::Init
//
//	PURPOSE:	Init
//
// ----------------------------------------------------------------------- //

void CAttachmentWeapon::Init(HOBJECT hObj, HOBJECT hWeaponModel, int nAttachmentID, int nWeaponID, int nAmmoID)
{
	CAttachment::Init(hObj, hWeaponModel, nAttachmentID);

	m_Weapons.Init(hObj, hWeaponModel);
	m_Weapons.ObtainWeapon(nWeaponID);
	m_Weapons.ChangeWeapon(nWeaponID);
	m_Weapons.GetCurWeapon()->SetAmmoId(nAmmoID);

    uint32 dwUsrFlags = g_pLTServer->GetObjectUserFlags(m_hModel);
    g_pLTServer->SetObjectUserFlags(m_hModel, dwUsrFlags | USRFLG_NIGHT_INFRARED);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachmentWeapon::HandleProjectileImpact
//
//	PURPOSE:	Tests being hit by a projectile
//
// ----------------------------------------------------------------------- //

LTBOOL CAttachmentWeapon::HandleProjectileImpact(CProjectile* pProjectile, CAttachmentPosition* pAttachmentPosition, IntersectInfo& iInfo, LTVector& vDir, LTVector& vFrom)
{
	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachmentWeapon::CreateSpawnString
//
//	PURPOSE:	Creates a spawn string for when we are dropped
//
// ----------------------------------------------------------------------- //

void CAttachmentWeapon::CreateSpawnString(char* szSpawn)
{
	if ( !m_Weapons.GetCurWeapon() )
	{
		szSpawn[0] = '\0';
		return;
	}

	AMMO*	pAmmo	= g_pWeaponMgr->GetAmmo(m_Weapons.GetCurWeapon()->GetAmmoId());
	WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(m_Weapons.GetCurWeapon()->GetId());

	if (pAmmo && pWeapon && pAmmo->eInstDamageType != DT_MELEE)
	{
		sprintf(szSpawn, "WeaponItem Gravity 0;AmmoAmount %d;WeaponType %s;AmmoType %s;IsLevelPowerup 0",
			pAmmo->nSpawnedAmount, pWeapon->szName, pAmmo->szName);
	}
	else
	{
		szSpawn[0] = '\0';
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachmentWeapon::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 CAttachmentWeapon::ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	CAttachment::ObjectMessageFn(pObject, hSender, messageID, hRead);

	m_Weapons.ObjectMessageFn(pObject, hSender, messageID, hRead);

	return 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachmentWeapon::Save
//
//	PURPOSE:	Save
//
// ----------------------------------------------------------------------- //

void CAttachmentWeapon::Save(HMESSAGEWRITE hWrite)
{
	CAttachment::Save(hWrite);

	m_Weapons.Save(hWrite, 0);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachmentWeapon::Load
//
//	PURPOSE:	Load
//
// ----------------------------------------------------------------------- //

void CAttachmentWeapon::Load(HMESSAGEREAD hRead)
{
	CAttachment::Load(hRead);

	m_Weapons.Load(hRead, 0);
    uint8 nWeaponID = m_Weapons.GetCurWeapon()->GetId();
	m_Weapons.DeselectCurWeapon();	// Deselect so we'll change to it
	m_Weapons.ChangeWeapon(nWeaponID);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachmentPosition::CAttachmentPosition
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CAttachmentPosition::CAttachmentPosition()
{
    m_szName = LTNULL;
    m_hstrAttachmentName = LTNULL;
    m_pAttachment = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachmentPosition::~CAttachmentPosition
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CAttachmentPosition::~CAttachmentPosition()
{
	FREE_HSTRING(m_hstrAttachmentName);

	if ( m_pAttachment )
	{
		CAttachment::Destroy(m_pAttachment);
		m_pAttachment = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachmentPosition::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 CAttachmentPosition::ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	if ( m_pAttachment )
	{
		m_pAttachment->ObjectMessageFn(pObject, hSender, messageID, hRead);
	}

	return 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachmentPosition::Save
//
//	PURPOSE:	Save
//
// ----------------------------------------------------------------------- //

void CAttachmentPosition::Save(HMESSAGEWRITE hWrite)
{
	SAVE_BOOL(!!m_pAttachment);
	if ( m_pAttachment )
	{
		SAVE_DWORD(m_pAttachment->GetType());
		m_pAttachment->Save(hWrite);
	}
	SAVE_HSTRING(m_hstrAttachmentName);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachmentPosition::Load
//
//	PURPOSE:	Load
//
// ----------------------------------------------------------------------- //

void CAttachmentPosition::Load(HMESSAGEREAD hRead)
{
	LTBOOL bAttachment = LTFALSE;

	LOAD_BOOL(bAttachment);

	if ( bAttachment )
	{
        uint32  dwAttachmentType;

        m_pAttachment = LTNULL;

		LOAD_DWORD(dwAttachmentType);

		m_pAttachment = CAttachment::Create(dwAttachmentType);

		_ASSERT(m_pAttachment);

		m_pAttachment->Load(hRead);
	}

	LOAD_HSTRING(m_hstrAttachmentName);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachmentPosition::RemoveWeapon
//
//	PURPOSE:	RemoveWeapon
//
// ----------------------------------------------------------------------- //

void CAttachmentPosition::RemoveWeapon()
{
	CAttachment* pAttachment = GetAttachment();
	if ( pAttachment && (pAttachment->GetType() == ATTACHMENT_TYPE_WEAPON) )
	{
		g_pLTServer->RemoveObject(pAttachment->GetModel());
		CAttachment::Destroy(pAttachment);

		SetAttachment(NULL);
	}
}

// Plugin statics

LTBOOL CAttachmentsPlugin::sm_bInitted = LTFALSE;
CAttachButeMgr CAttachmentsPlugin::sm_AttachButeMgr;
CWeaponMgrPlugin CAttachmentsPlugin::sm_WeaponMgrPlugin;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachmentsPlugin::PreHook_EditStringList
//
//	PURPOSE:	Callback handler for EditStringList plugin
//
// ----------------------------------------------------------------------- //

LTRESULT CAttachmentsPlugin::PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	if ( !sm_bInitted )
	{
		char szFile[256];
		sprintf(szFile, "%s\\Attributes\\Attachments.txt", szRezPath);
		sm_AttachButeMgr.SetInRezFile(LTFALSE);
        sm_AttachButeMgr.Init(g_pLTServer, szFile);
		sm_bInitted = LTTRUE;
		sm_WeaponMgrPlugin.PreHook_EditStringList(szRezPath, szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength);

	}

	if ( 0 )
	{
		PopulateStringList(aszStrings, pcStrings, cMaxStrings, cMaxStringLength);
		return LT_OK;
	}
	else
	{
		return LT_UNSUPPORTED;
	}
};

void CAttachmentsPlugin::PopulateStringList(char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	// TODO: make sure we don't overflow cMaxStringLength or cMaxStrings
    uint32 cAttachments = sm_AttachButeMgr.GetNumAttachments();
	_ASSERT(cMaxStrings >= cAttachments);
	strcpy(aszStrings[(*pcStrings)++], c_szNoAttachment);
    for ( uint32 iAttachment = 0 ; iAttachment < cAttachments ; iAttachment++ )
	{
		strcpy(aszStrings[(*pcStrings)++], sm_AttachButeMgr.GetAttachmentName(iAttachment));
	}
	sm_WeaponMgrPlugin.PopulateStringList(aszStrings, pcStrings, cMaxStrings, cMaxStringLength);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachmentsPlugin::PreHook_EditStringList
//
//	PURPOSE:	Callback handler for EditStringList plugin
//
// ----------------------------------------------------------------------- //

LTRESULT CHumanAttachmentsPlugin::PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	if ( LT_OK == CAttachmentsPlugin::PreHook_EditStringList(szRezPath, szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength) )
	{
		return LT_OK;
	}

    if ( !_stricmp("LeftHand",      szPropName) ||
         !_stricmp("RightHand",     szPropName) ||
         !_stricmp("LeftFoot",      szPropName) ||
         !_stricmp("RightFoot",     szPropName) ||
         !_stricmp("Motorcycle",    szPropName) ||
         !_stricmp("Snowmobile",    szPropName) ||
         !_stricmp("Head",          szPropName) ||
         !_stricmp("Eyes",          szPropName) ||
         !_stricmp("Nose",          szPropName) ||
         !_stricmp("Chin",          szPropName) ||
         !_stricmp("Back",          szPropName) )
	{
		PopulateStringList(aszStrings, pcStrings, cMaxStrings, cMaxStringLength);
		return LT_OK;
	}
	else
	{
		return LT_UNSUPPORTED;
	}
};

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleAttachmentsPlugin::PreHook_EditStringList
//
//	PURPOSE:	Callback handler for EditStringList plugin
//
// ----------------------------------------------------------------------- //

LTRESULT CVehicleAttachmentsPlugin::PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	if ( LT_OK == CAttachmentsPlugin::PreHook_EditStringList(szRezPath, szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength) )
	{
		return LT_OK;
	}

    if ( !_stricmp("Driver",    szPropName) )
	{
		PopulateStringList(aszStrings, pcStrings, cMaxStrings, cMaxStringLength);
		return LT_OK;
	}
	else
	{
		return LT_UNSUPPORTED;
	}
};

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHelicopterAttachmentsPlugin::PreHook_EditStringList
//
//	PURPOSE:	Callback handler for EditStringList plugin
//
// ----------------------------------------------------------------------- //

LTRESULT CHelicopterAttachmentsPlugin::PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	if ( LT_OK == CVehicleAttachmentsPlugin::PreHook_EditStringList(szRezPath, szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength) )
	{
		return LT_OK;
	}

    if ( !_stricmp("FrontBelly", szPropName) ||
         !_stricmp("RearBelly", szPropName) ||
         !_stricmp("LeftSkid", szPropName) ||
         !_stricmp("RightSkid", szPropName) ||
         !_stricmp("TopRotor", szPropName) ||
         !_stricmp("BackRotor", szPropName) ||
         !_stricmp("RightGunner", szPropName) )
	{
		PopulateStringList(aszStrings, pcStrings, cMaxStrings, cMaxStringLength);
		return LT_OK;
	}
	else
	{
		return LT_UNSUPPORTED;
	}
};

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimalAttachmentsPlugin::PreHook_EditStringList
//
//	PURPOSE:	Callback handler for EditStringList plugin
//
// ----------------------------------------------------------------------- //

LTRESULT CAnimalAttachmentsPlugin::PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	if ( LT_OK == CAttachmentsPlugin::PreHook_EditStringList(szRezPath, szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength) )
	{
		return LT_OK;
	}

	if ( 0 )
	{
		PopulateStringList(aszStrings, pcStrings, cMaxStrings, cMaxStringLength);
		return LT_OK;
	}
	else
	{
		return LT_UNSUPPORTED;
	}
};

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSharkAttachmentsPlugin::PreHook_EditStringList
//
//	PURPOSE:	Callback handler for EditStringList plugin
//
// ----------------------------------------------------------------------- //

LTRESULT CSharkAttachmentsPlugin::PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	if ( LT_OK == CAnimalAttachmentsPlugin::PreHook_EditStringList(szRezPath, szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength) )
	{
		return LT_OK;
	}

    if ( !_stricmp("Mouth", szPropName) )
	{
		PopulateStringList(aszStrings, pcStrings, cMaxStrings, cMaxStringLength);
		return LT_OK;
	}
	else
	{
		return LT_UNSUPPORTED;
	}
};
