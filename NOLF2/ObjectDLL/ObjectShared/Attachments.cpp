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
#include "PlayerButes.h"
#include "Weapon.h"
#include "AIUtils.h"
#include "CharacterMgr.h"

#pragma optimize ("",off)

// Static constants

const static char c_szNoAttachment[] = NO_ATTACHMENT;
/*
static CBankedList<CHumanAttachments> s_bankCHumanAttachments;
static CBankedList<CPlayerAttachments> s_bankCPlayerAttachments;

static CBankedList<CAttachmentProp> s_bankCAttachmentProp;
static CBankedList<CAttachmentObject> s_bankCAttachmentObject;
static CBankedList<CAttachmentWeapon> s_bankCAttachmentWeapon;
*/
CAttachments* CAttachments::Create(uint32 nAttachmentsType)
{
	switch ( nAttachmentsType )
	{
		case ATTACHMENTS_TYPE_HUMAN:
			return new CHumanAttachments; //s_bankCHumanAttachments.New();
			break;
		case ATTACHMENTS_TYPE_PLAYER:
			return new CPlayerAttachments; //s_bankCPlayerAttachments.New();
			break;
	}

	return LTNULL;
}

void CAttachments::Destroy(CAttachments* pAttachments)
{
	delete pAttachments;
	/*
	switch ( pAttachments->GetType() )
	{
		case ATTACHMENTS_TYPE_HUMAN:
			s_bankCHumanAttachments.Delete(static_cast<CHumanAttachments*>(pAttachments));
			break;
		case ATTACHMENTS_TYPE_PLAYER:
			s_bankCPlayerAttachments.Delete(static_cast<CPlayerAttachments*>(pAttachments));
			break;
	}
	*/
}

CAttachment* CAttachment::Create(uint32 nAttachmentType)
{
	switch ( nAttachmentType )
	{
		case ATTACHMENT_TYPE_WEAPON:
			return new CAttachmentWeapon; //s_bankCAttachmentWeapon.New();
			break;
		case ATTACHMENT_TYPE_OBJECT:
			return new CAttachmentObject; //s_bankCAttachmentObject.New();
			break;
		case ATTACHMENT_TYPE_PROP:
			return new CAttachmentProp; //s_bankCAttachmentProp.New();
			break;
	}

	return LTNULL;
}

void CAttachment::Destroy(CAttachment* pAttachment)
{
	delete pAttachment;
	/*
	switch ( pAttachment->GetType() )
	{
		case ATTACHMENT_TYPE_WEAPON:
			//s_bankCAttachmentWeapon.Delete(static_cast<CAttachmentWeapon*>(pAttachment));
			break;
		case ATTACHMENT_TYPE_OBJECT:
			s_bankCAttachmentObject.Delete(static_cast<CAttachmentObject*>(pAttachment));
			break;
		case ATTACHMENT_TYPE_PROP:
			s_bankCAttachmentProp.Delete(static_cast<CAttachmentProp*>(pAttachment));
			break;
	}
	*/
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
	m_bHidden = false;
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

		case MID_SAVEOBJECT:
		{
            Save((ILTMessage_Write*)pData, (uint8)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((ILTMessage_Read*)pData, (uint8)fData);
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

uint32 CAttachments::ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg)
{
	for ( int iAttachmentPosition = 0 ; iAttachmentPosition < m_cAttachmentPositions ; iAttachmentPosition++ )
	{
		m_apAttachmentPositions[iAttachmentPosition]->ObjectMessageFn(pObject, hSender, pMsg);
	}

    return IAggregate::ObjectMessageFn(pObject, hSender, pMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL CAttachments::ReadProp(LPBASECLASS pObject, ObjectCreateStruct *pStruct)
{
	_ASSERT(pObject && pStruct);
	if (!pObject || !pStruct) return LTFALSE;

	GenericProp genProp;

	{for ( int iAttachmentPosition = 0 ; iAttachmentPosition < m_cAttachmentPositions ; iAttachmentPosition++ )
	{
		// Verify that the name is valid (in case this names were added since this map has been in existance)
		UBER_ASSERT( m_apAttachmentPositions[iAttachmentPosition]->GetName()!=NULL, "Attachment Name is null" );

        if ( g_pLTServer->GetPropGeneric( (char*)m_apAttachmentPositions[iAttachmentPosition]->GetName(), &genProp ) == LT_OK )
		{
			if ( genProp.m_String[0] )
			{
                m_apAttachmentPositions[iAttachmentPosition]->SetAttachmentName(g_pLTServer->CreateString(genProp.m_String));
			}
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

		// Verify that the name is valid (in case this names were added since
		// this map has been in existance)
		UBER_ASSERT( pAttachmentPosition->GetName()!=NULL, "Attachment Name is null" );

		if ( !_stricmp(szAttachmentPosition, pAttachmentPosition->GetName()) )
		{
			if ( pAttachmentPosition->HasAttachment() )
			{
				CAttachment* pAttachment = pAttachmentPosition->GetAttachment( );
				char szExistingName[256] = "";
				g_pAttachButeMgr->GetAttachmentName( pAttachment->GetID( ), szExistingName, ARRAY_LEN( szExistingName ));
				char szModelName[80];
				char szSkinName[80];
				g_pLTServer->GetModelFilenames(m_hObject, szModelName, ARRAY_LEN(szModelName), szSkinName, ARRAY_LEN(szSkinName));
				if( IsAI( m_hObject ))
				{
					AITRACE( AIShowCharacters, ( m_hObject, "WARNING: CAttachments::Attach: Attachment Position already occupied by %s: %s on model: %s", szExistingName, szAttachmentPosition, szModelName ) );
				}
				else
				{
					UBER_ASSERT3( 0, "CAttachments::Attach: Attachment Position already occupied by %s: %s on model: %s", szExistingName, szAttachmentPosition, szModelName );
				}
			}
			else
			{
				pAttachmentPosition->SetAttachmentName(g_pLTServer->CreateString((char*)szAttachment));
				CreateAttachment(pAttachmentPosition);
			}

			return;
		}
	}

	// Assert if the attachment position is not found.
	char szModelName[80];
	char szSkinName[80];
    g_pLTServer->GetModelFilenames(m_hObject, szModelName, ARRAY_LEN(szModelName), szSkinName, ARRAY_LEN(szSkinName));
	if( IsAI( m_hObject ))
	{
		AIASSERT2( 0, m_hObject, "CAttachments::Attach: Attachment Position not found: %s on model: %s", szAttachmentPosition, szModelName );
	}
	else
	{
		UBER_ASSERT2( 0, "CAttachments::Attach: Attachment Position already occupied: %s on model: %s", szAttachmentPosition, szModelName );
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
				HATTACHMENT hAttachment;
                if (LT_OK == g_pLTServer->FindAttachment(m_hObject, pAttachment->GetModel(), &hAttachment))
				{
                    if (LT_OK != g_pLTServer->RemoveAttachment(hAttachment))
						ASSERT(!"failed to remove attachment");
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
//	ROUTINE:	CAttachments::GetAttachment
//
//	PURPOSE:	Get attachment at some position
//
// ----------------------------------------------------------------------- //

CAttachment* CAttachments::GetAttachment(const char* szAttachmentPosition)
{
	for ( int iAttachmentPosition = 0 ; iAttachmentPosition < m_cAttachmentPositions ; iAttachmentPosition++ )
	{
		CAttachmentPosition* pAttachmentPosition = m_apAttachmentPositions[iAttachmentPosition];

		if ( !_stricmp(szAttachmentPosition, pAttachmentPosition->GetName()) )
		{
			return pAttachmentPosition->GetAttachment();
		}
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::HideAttachments
//
//	PURPOSE:	Hide/Show attachments.
//
// ----------------------------------------------------------------------- //

void CAttachments::HideAttachments(LTBOOL bHide)
{
	m_bHidden = !!bHide;

	for ( int iAttachmentPosition = 0 ; iAttachmentPosition < m_cAttachmentPositions ; ++iAttachmentPosition )
	{
		CAttachment* pAttachment = m_apAttachmentPositions[iAttachmentPosition]->GetAttachment( );
		if( pAttachment )
		{
			SetAttachmentHiddenState( *pAttachment );
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

    const char *szAttachmentName = g_pLTServer->GetStringData(pAttachmentPosition->GetAttachmentName());

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

	theStruct.m_Flags = FLAG_VISIBLE | FLAG_GOTHRUWORLD;
	theStruct.m_Flags2 |= FLAG2_DYNAMICDIRLIGHT;
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

	WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon(nWeaponID);
	if (pWeapon)
	{
		if (pWeapon->szHHModel[0])
		{
			SAFE_STRCPY(szModel, pWeapon->szHHModel);
		}
	
		pWeapon->blrHHSkins.CopyList(0, theStruct.m_SkinNames[0], MAX_CS_FILENAME_LEN+1);
		pWeapon->blrHHRenderStyles.CopyList(0, theStruct.m_RenderStyleNames[0], MAX_CS_FILENAME_LEN+1);
	}

	// Set up the model/skin filenames

	SAFE_STRCPY(theStruct.m_Filename, szModel);

	if(g_pAttachButeMgr->GetAttachmentTranslcuent(nWeaponID))
		theStruct.m_Flags2 |= FLAG2_FORCETRANSLUCENT;

	// Create the attachment

    HCLASS hClass = g_pLTServer->GetClass(szClass);
    LPBASECLASS pModel = g_pLTServer->CreateObject(hClass, &theStruct);
	if ( !pModel ) return;

	// Attach it

	HATTACHMENT hAttachment;
    LTRESULT dRes = g_pLTServer->CreateAttachment(m_hObject, pModel->m_hObject, (char*)pAttachmentPosition->GetName(),
												 &LTVector(0,0,0), &LTRotation(), &hAttachment);

	((CAttachmentWeapon*)pAttachment)->Init(m_hObject, pModel->m_hObject, -1, nWeaponID, nAmmoID);

 	// Notify the attachment that it is being attached.
 	SendTriggerMsgToObject(g_pLTServer->HandleToObject(m_hObject), pModel->m_hObject, LTFALSE, KEY_ATTACH);

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

	theStruct.m_Flags = FLAG_VISIBLE | FLAG_GOTHRUWORLD;
	theStruct.m_Flags2 |= FLAG2_DYNAMICDIRLIGHT;
	theStruct.m_NextUpdate = 1.0f;

	// Get the attachment skin, model, and class names

	int nAttachmentType = g_pAttachButeMgr->GetAttachmentType(nAttachmentID);

	char szClass[128];
	szClass[0] = '\0';

	g_pAttachButeMgr->GetAttachmentClass(nAttachmentID,szClass,sizeof(szClass));
	if ( !szClass[0] ) return;


	// Set up the model/skin filenames

	g_pAttachButeMgr->GetAttachmentModel(nAttachmentID,theStruct.m_Filename,sizeof(theStruct.m_Filename));

	g_pAttachButeMgr->CopyAttachmentSkins(nAttachmentID, theStruct.m_SkinNames[0], MAX_CS_FILENAME_LEN+1);

	// Set up the render styles.

	g_pAttachButeMgr->CopyAttachmentRenderStyles(nAttachmentID, theStruct.m_RenderStyleNames[0], MAX_CS_FILENAME_LEN+1);

	// Create the attachment

    HCLASS hClass = g_pLTServer->GetClass(szClass);
	if ( hClass == NULL )
	{
		char szError[512];
		sprintf(szError, "CreateObjectAttachment: No object with class: %s found", szClass );
		UBER_ASSERT( 0, szError );
	}

	if(g_pAttachButeMgr->GetAttachmentTranslcuent(nAttachmentID))
		theStruct.m_Flags2 |= FLAG2_FORCETRANSLUCENT;

	char szModel[128] = "";
	g_pAttachButeMgr->GetAttachmentProperties(nAttachmentID,szModel,sizeof(szModel));
    LPBASECLASS pModel = g_pLTServer->CreateObjectProps(hClass, &theStruct, szModel);
	if ( !pModel ) return;

	// Attach it

	HATTACHMENT hAttachment;
    LTRESULT dRes = g_pLTServer->CreateAttachment(m_hObject, pModel->m_hObject, (char*)pAttachmentPosition->GetName(), &LTVector(0,0,0), &LTRotation(), &hAttachment);

	// Notify the attachment that it is being attached.
	SendTriggerMsgToObject(g_pLTServer->HandleToObject(m_hObject), pModel->m_hObject, LTFALSE, KEY_ATTACH);

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

	theStruct.m_Flags = FLAG_VISIBLE | FLAG_GOTHRUWORLD | FLAG_FORCEOPTIMIZEOBJECT;
	theStruct.m_Flags2 |= FLAG2_DYNAMICDIRLIGHT;
	theStruct.m_NextUpdate = 1.0f;

	// Get the attachment skin, model, and class names

	int nAttachmentType = g_pAttachButeMgr->GetAttachmentType(nAttachmentID);

	char szModel[MAX_CS_FILENAME_LEN];
	char szSkin[MAX_CS_FILENAME_LEN];
	char szClass[128];

	CAttachment* pAttachment = CAttachment::Create(ATTACHMENT_TYPE_PROP);
	strcpy(szClass, "Prop");
	g_pAttachButeMgr->GetAttachmentModel(nAttachmentID, szModel, sizeof(szModel)); 
	g_pAttachButeMgr->GetAttachmentSkin(nAttachmentID,szSkin,sizeof(szSkin));

	// Set up the model/skin filenames

	SAFE_STRCPY(theStruct.m_Filename, szModel);

	g_pAttachButeMgr->CopyAttachmentSkins(nAttachmentID, theStruct.m_SkinNames[0], MAX_CS_FILENAME_LEN+1);

	// Set up the render styles.

	g_pAttachButeMgr->CopyAttachmentRenderStyles(nAttachmentID, theStruct.m_RenderStyleNames[0], MAX_CS_FILENAME_LEN+1);

	if(g_pAttachButeMgr->GetAttachmentTranslcuent(nAttachmentID))
		theStruct.m_Flags2 |= FLAG2_FORCETRANSLUCENT;

	// Create the attachment

    HCLASS hClass = g_pLTServer->GetClass(szClass);
	char szStr[128] = "";
	g_pAttachButeMgr->GetAttachmentProperties(nAttachmentID,szStr,sizeof(szStr));
    Prop* pModel = (Prop*)g_pLTServer->CreateObjectProps(hClass, &theStruct, szStr);
	if ( !pModel ) return;

	pModel->GetDestructible()->SetNeverDestroy(LTTRUE);

	LTRotation rRot;
	g_pLTServer->GetObjectRotation(pModel->m_hObject, &rRot);

	// Attach it

	HATTACHMENT hAttachment;
    LTRESULT dRes = g_pLTServer->CreateAttachment(m_hObject, pModel->m_hObject, (char*)pAttachmentPosition->GetName(),
											   &LTVector(0,0,0), &rRot, &hAttachment);

	if ( dRes != LT_OK ) 
	{
		// Error: Socket not found or other such error detected.  Print out a 
		// message so that we know this is going on and can fix it.
		char szError[1024];
		char szModelName[80];
		char szSkinName[80];
		g_pLTServer->GetModelFilenames(m_hObject, szModelName, ARRAY_LEN(szModelName), szSkinName, ARRAY_LEN(szSkinName));
		sprintf( szError, "CAttachments::Attach: unable to create socket: %s on model: %s", pAttachmentPosition->GetName(), szModelName );
		g_pLTServer->CPrint( szError );
		UBER_ASSERT( 0, szError );
	}

 	// Notify the attachment that it is being attached.
 	SendTriggerMsgToObject(g_pLTServer->HandleToObject(m_hObject), pModel->m_hObject, LTFALSE, KEY_ATTACH);

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
					&LTVector(0,0,0), &LTRotation(), &hAttachment);

				// Notify the attachment that it is being attached.
				SendTriggerMsgToObject(g_pLTServer->HandleToObject(m_hObject), hModel, LTFALSE, KEY_ATTACH);

				// Re-init to set m_hObject...

				pCur->Init(m_hObject, hModel, pCur->GetID());

				SetAttachmentHiddenState( *pCur );
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

			if( apWeapons[cWeapons] )
			{
				++cWeapons;
				if ( cWeapons == cMaxWeapons )
			{
				break;
			}
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
            BaseClass* pObject = ( BaseClass* )g_pLTServer->HandleToObject(pAttachmentObject->GetModel());

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

			int cAmmoTypes = g_pWeaponMgr->GetNumAmmoIds();

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

	m_bHidden = false;
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

void CAttachments::AddRequirements(ModelId eModelId)
{
	char* szModel = (char*)g_pModelButeMgr->GetModelName(eModelId);

	int anRequirements[32];
	int cRequirements = g_pAttachButeMgr->GetRequirementIDs(szModel, anRequirements, 32);

	for ( int iRequirement = 0 ; iRequirement < cRequirements ; iRequirement++ )
	{
		int nAttachment = g_pAttachButeMgr->GetRequirementAttachment(anRequirements[iRequirement]);
		char szSocket[128] = "";
		g_pAttachButeMgr->GetRequirementSocket(anRequirements[iRequirement],szSocket,sizeof(szSocket));

		char szAttachment[128] = "";
		g_pAttachButeMgr->GetAttachmentName(nAttachment,szAttachment,sizeof(szAttachment));

		Attach(szSocket, szAttachment);
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
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::HandleProjectileImpact
//
//	PURPOSE:	Handles being hit by a projectile
//
// ----------------------------------------------------------------------- //

LTBOOL CAttachments::HandleProjectileImpact(CProjectile* pProjectile, IntersectInfo& iInfo, const LTVector& vDir, const LTVector& vFrom, ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	for ( int iAttachmentPosition = 0 ; iAttachmentPosition < m_cAttachmentPositions ; iAttachmentPosition++ )
	{
		CAttachmentPosition* pPosition = m_apAttachmentPositions[iAttachmentPosition];

		if ( !pPosition->HasAttachment() ) continue;

		CAttachment* pAttachment = pPosition->GetAttachment();
		if ( pAttachment->HandleProjectileImpact(pProjectile, pPosition, iInfo, vDir, vFrom, eModelSkeleton, eModelNode) )
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

		// Check if attachment is flagged to delete on death.
		// (e.g. smoke puffs coming from a smoking AIs mouth should not
		// continue from the body's mouth!)

		if( pAttachment && (pAttachment->GetType() != ATTACHMENT_TYPE_WEAPON) &&
			g_pAttachButeMgr->GetAttachmentDeleteOnDeath( pAttachment->GetID() ) )
		{
			HATTACHMENT hAttachment;
			if (LT_OK == g_pLTServer->FindAttachment(m_hObject, pAttachment->GetModel(), &hAttachment))
			{
				if (LT_OK != g_pLTServer->RemoveAttachment(hAttachment))
					ASSERT(!"failed to remove attachment");
			}

			g_pLTServer->RemoveObject(pAttachment->GetModel());
			CAttachment::Destroy(pAttachment);

			m_apAttachmentPositions[iAttachmentPosition]->SetAttachment(NULL);
			pAttachment = LTNULL;
		}

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
//  ROUTINE:	CAttachments::AddToObjectList
//
//  PURPOSE:	Add every attachment to the specific list...
//
// ----------------------------------------------------------------------- //

void CAttachments::AddToObjectList( ObjectList *pObjList, eObjListControl eControl /*=eObjListNODuplicates*/ )
{
	if( !pObjList ) return;

	for( int i = 0; i < m_cAttachmentPositions; ++i )
	{
		CAttachmentPosition *pCurPos = m_apAttachmentPositions[i];		
		if( !pCurPos ) break;

		CAttachment *pCur = pCurPos->GetAttachment();
		
		if( pCur )
		{
			AddObjectToList( pObjList, pCur->GetModel(), eControl );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CAttachments::RemoveAndRecreateAttachments
//
//  PURPOSE:	Remove all of the attachments and then reattach them.
//				Lame! But if our model filename has changed we need to do this.
//
// ----------------------------------------------------------------------- //

void CAttachments::RemoveAndRecreateAttachments()
{
	for( int i = 0; i < m_cAttachmentPositions; ++i )
	{
		CAttachmentPosition *pAttachPos = m_apAttachmentPositions[i];
		if( !pAttachPos ) continue;

		CAttachment *pAttachment = pAttachPos->GetAttachment();
		if( pAttachment )
		{
			HATTACHMENT hAttachment;
			if( LT_OK == g_pLTServer->FindAttachment( m_hObject, pAttachment->GetModel(), &hAttachment ))
			{
				// Unattach

				g_pLTServer->RemoveAttachment( hAttachment );

				// Reattach
				
				if( LT_OK != g_pLTServer->CreateAttachment( m_hObject, pAttachment->GetModel(), pAttachPos->GetName(), &LTVector(0,0,0), &LTRotation(), &hAttachment ))
				{
					UBER_ASSERT1( 0, "CAttachments::RemoveAndRecreateAttachments: Unable to reattach attachment at position '%s'", pAttachPos->GetName() );
					continue;
				}
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CAttachments::DetachAttachmentsOfType
//
//  PURPOSE:	Detach all attachments of specified type.
//
// ----------------------------------------------------------------------- //

void CAttachments::DetachAttachmentsOfType( AttachmentType eType )
{
	for( int i = 0; i < m_cAttachmentPositions; ++i )
	{
		CAttachmentPosition *pAttachPos = m_apAttachmentPositions[i];
		if( !pAttachPos ) continue;

		CAttachment *pAttachment = pAttachPos->GetAttachment();
		if( pAttachment && pAttachment->GetType() == eType )
		{
			Detach( pAttachPos->GetName() );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CAttachments::Save(ILTMessage_Write *pMsg, uint8 nType)
{
	if (!pMsg) return;

	SAVE_HOBJECT(m_hObject);
	SAVE_INT(m_cAttachmentPositions);
	SAVE_INT(m_cWeapons);
	SAVE_INT(m_cObjects);
	SAVE_INT(m_cProps);
	SAVE_bool( m_bHidden );

	for ( int iAttachmentPosition = 0 ; iAttachmentPosition < m_cAttachmentPositions ; iAttachmentPosition++ )
	{
		m_apAttachmentPositions[iAttachmentPosition]->Save(pMsg);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CAttachments::Load(ILTMessage_Read *pMsg, uint8 nType)
{
	if (!pMsg) return;

	LOAD_HOBJECT(m_hObject);
	LOAD_INT(m_cAttachmentPositions);
	LOAD_INT(m_cWeapons);
	LOAD_INT(m_cObjects);
	LOAD_INT(m_cProps);
	LOAD_bool( m_bHidden );

	{for ( int iAttachmentPosition = 0 ; iAttachmentPosition < m_cAttachmentPositions ; iAttachmentPosition++ )
	{
		m_apAttachmentPositions[iAttachmentPosition]->Load(pMsg);
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
	AddPosition( "RightHand",		&m_RightHand	);
	AddPosition( "LeftHand",		&m_LeftHand		);
	AddPosition( "Head",			&m_Head			);
	AddPosition( "Eyes",			&m_Eyes			);
	AddPosition( "Nose",			&m_Nose			);
	AddPosition( "Chin",			&m_Chin			);
	AddPosition( "Back",			&m_Back			);
	AddPosition( "RightFoot",		&m_RightFoot	);
	AddPosition( "LeftFoot",		&m_LeftFoot		);
	AddPosition( "Snowmobile",		&m_Snowmobile	);
	AddPosition( "Light",			&m_Light		);
	AddPosition( "LeftLowerArm",	&m_LeftLowerArm );
	AddPosition( "LeftUpperArm",	&m_LeftUpperArm );
	AddPosition( "RightLowerArm",	&m_RightLowerArm);
	AddPosition( "RightUpperArm",	&m_RightUpperArm);
	AddPosition( "LeftLowerLeg",	&m_LeftLowerLeg );
	AddPosition( "LeftUpperLeg",	&m_LeftUpperLeg );
	AddPosition( "RightLowerLeg",	&m_RightLowerLeg);
	AddPosition( "RightUpperLeg",	&m_RightUpperLeg);
	AddPosition( "UpperTorso",		&m_UpperTorso	);
	AddPosition( "Torso",			&m_Torso		);

	AddPosition( "LeftShoulder",	&m_LeftShoulder	);
	AddPosition( "RightShoulder",	&m_RightShoulder);

	AddPosition( "RightHand2",		&m_RightHand2 );
	AddPosition( "LeftHand2",		&m_LeftHand2	);
	AddPosition( "LeftLowerArm2",	&m_LeftLowerArm2 );
	AddPosition( "LeftLowerLeg2",	&m_LeftLowerLeg2 );
	AddPosition( "LeftUpperArm2",	&m_LeftUpperArm2 );
	AddPosition( "LeftUpperLeg2",	&m_LeftUpperLeg2 );
	AddPosition( "RightUpperArm2",&m_RightUpperArm2 );
	AddPosition( "RightUpperLeg2",&m_RightUpperLeg2 );
	AddPosition( "RightLowerArm2",&m_RightLowerArm2 );
	AddPosition( "RightLowerLeg2",&m_RightLowerLeg2 );
	AddPosition( "UpperTorso2",	&m_UpperTorso2);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CHumanAttachments::AddPosition()
//              
//	PURPOSE:	NON VIRTUAL helper function for adding attachments/names
//              
//----------------------------------------------------------------------------
void CHumanAttachments::AddPosition(const char* szName, CAttachmentPosition* pAttachPosition)
{
	ASSERT( szName != NULL );
	ASSERT( pAttachPosition != NULL );
	ASSERT( m_cAttachmentPositions < MAX_ATTACHMENT_POSITIONS );
	
	pAttachPosition->SetName(szName);
	m_apAttachmentPositions[m_cAttachmentPositions] = pAttachPosition;
	m_cAttachmentPositions++;
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
	char aWeaponName[30] = {0};

	g_pServerButeMgr->GetPlayerAttributeString(PLAYER_BUTE_DEFAULTWEAPON,
		aWeaponName, ARRAY_LEN(aWeaponName));
	
	m_RightHand.SetAttachmentName(g_pLTServer->CreateString(aWeaponName));
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

        uint8 nNumAmmoTypes = g_pWeaponMgr->GetNumAmmoIds();

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

		// NOTE: This makes the dangerous assumption that the weapon
		// ID in the weapons array match the indexes they belong to.

		int cWeapons = g_pWeaponMgr->GetNumWeapons();
		for ( int iWeapon = 0 ; iWeapon < cWeapons ; iWeapon++ )
		{
			if (g_pWeaponMgr->IsPlayerWeapon(iWeapon))
			{
				pWeapons->ObtainWeapon(iWeapon, AMMO_DEFAULT_ID, 10000, LTTRUE);
/*
				// get the weapon data structure
				WEAPON const *pWeaponData;
				pWeaponData = g_pWeaponMgr->GetWeapon(iWeapon);

				// obtain all weapon ammo types
				for ( int iAmmo = 0; iAmmo < pWeaponData->nNumAmmoIds; ++iAmmo )
				{
					pWeapons->ObtainWeapon(iWeapon,
					                       pWeaponData->aAmmoIds[ iAmmo ],
					                       10000,
					                       LTTRUE);
				}
*/
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
			WEAPON const *pWeaponData = g_pWeaponMgr->GetWeapon(iWeapon);

			if (pWeaponData)
			{
				for ( int iModNum = 0 ; iModNum < pWeaponData->nNumModIds ; iModNum++ )
				{
					pWeapons->ObtainMod(iWeapon, pWeaponData->aModIds[iModNum], true);
				}
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerAttachments::AcquireWeapon
//
//	PURPOSE:	Do the specific weapon cheat for the player
//
// ----------------------------------------------------------------------- //

bool CPlayerAttachments::AcquireWeapon( uint8 nId )
{
	if( LTNULL != GetDefaultAttachmentWeapon() || !g_pWeaponMgr->IsValidWeaponId( nId ))
	{
		CWeapons* pWeapons = GetDefaultAttachmentWeapon()->GetWeapons();

		if( g_pWeaponMgr->IsPlayerWeapon( nId ))
		{
			pWeapons->ObtainWeapon( nId, AMMO_DEFAULT_ID, 10000, LTTRUE );
			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerAttachments::AcquireMod
//
//	PURPOSE:	Do the specific mod cheat for the player
//
// ----------------------------------------------------------------------- //

bool CPlayerAttachments::AcquireMod( uint8 nId , bool bDisplayMsg/* = true*/)
{
	bool bRet = false;

	if( LTNULL != GetDefaultAttachmentWeapon() || !g_pWeaponMgr->IsValidModId( nId ))
	{
		CWeapons* pWeapons = GetDefaultAttachmentWeapon()->GetWeapons();

		int cWeapons = g_pWeaponMgr->GetNumWeapons();

		for( int iWeapon = 0; iWeapon < cWeapons; ++iWeapon )
		{
			const WEAPON *pWeaponData = g_pWeaponMgr->GetWeapon(iWeapon);

			if( pWeaponData )
			{
				for( int iModNum = 0; iModNum < pWeaponData->nNumModIds; ++iModNum )
				{
					if( pWeaponData->aModIds[iModNum] == nId )
					{
						pWeapons->ObtainMod( iWeapon, nId, true, bDisplayMsg );
						bRet = true;
					}
				}
			}
		}
	}

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerAttachments::AcquireAmmo
//
//	PURPOSE:	Do the specific ammo cheat for the player
//
// ----------------------------------------------------------------------- //

bool CPlayerAttachments::AcquireAmmo( uint8 nId )
{
	if( LTNULL != GetDefaultAttachmentWeapon() || !g_pWeaponMgr->IsValidAmmoId( nId ) )
	{
		CWeapons* pWeapons = GetDefaultAttachmentWeapon()->GetWeapons();
		
		pWeapons->SetAmmo( nId );
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerAttachments::SetWeaponAttachmentHiddenState
//
//	PURPOSE:	If the weapon is visible make sure it's not supposed to be hidden...
//
// ----------------------------------------------------------------------- //

void CPlayerAttachments::SetWeaponAttachmentHiddenState( CAttachment& AttachmentWeapon )
{
	CWeapon *pWeapon = GetWeapon();
	if( !pWeapon || !pWeapon->IsHidden() )
	{
		SetAttachmentHiddenState( AttachmentWeapon );
	}	
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
    m_hModel.SetReceiver( *this );
    m_hObject       = LTNULL;
	m_nAttachmentID	= -1;
	m_nDebrisID		= DEBRISMGR_INVALID_ID;
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
       
	}

	if (m_hModel)
		g_pLTServer->RemoveObject(m_hModel);
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

	// Get the ID for the Debris type to create when shot off...

	char szDebrisType[DEBRIS_MAX_NAME_LENGTH] = {0};
	g_pAttachButeMgr->GetAttachmentDebrisType( m_nAttachmentID, szDebrisType, ARRAY_LEN( szDebrisType ));

	DEBRIS *pDebris = g_pDebrisMgr->GetDebris( szDebrisType );
	if( pDebris )
	{
		m_nDebrisID = pDebris->nId;
	}

    g_pCommonLT->SetObjectFlags(hModel, OFT_User, USRFLG_ATTACH_HIDE1SHOW3, USRFLG_ATTACH_HIDE1SHOW3);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachment::Save
//
//	PURPOSE:	Save
//
// ----------------------------------------------------------------------- //

void CAttachment::Save(ILTMessage_Write *pMsg)
{
	SAVE_HOBJECT(m_hModel);
	SAVE_HOBJECT(m_hObject);
	SAVE_INT(m_nAttachmentID);
	SAVE_BYTE( m_nDebrisID );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachment::Load
//
//	PURPOSE:	Load
//
// ----------------------------------------------------------------------- //

void CAttachment::Load(ILTMessage_Read *pMsg)
{
	LOAD_HOBJECT(m_hModel);
	LOAD_HOBJECT(m_hObject);
	LOAD_INT(m_nAttachmentID);
	LOAD_BYTE( m_nDebrisID );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachment::OnLinkBroken
//
//	PURPOSE:	Handle attached model getting removed.
//
// ----------------------------------------------------------------------- //

void CAttachment::OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj )
{
	if( pRef == &m_hModel )
	{
		HATTACHMENT hAttachment;
		if ( LT_OK == g_pLTServer->FindAttachment( m_hObject, hObj, &hAttachment) )
		{
			g_pLTServer->RemoveAttachment(hAttachment);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachmentProp::HandleProjectileImpact
//
//	PURPOSE:	Tests being hit by a projectile
//
// ----------------------------------------------------------------------- //

LTBOOL CAttachmentProp::HandleProjectileImpact(CProjectile* pProjectile, CAttachmentPosition* pAttachmentPosition, IntersectInfo& iInfo, 
											   const LTVector& vDir, const LTVector& vFrom, ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	if( !pAttachmentPosition )
		return LTFALSE;

	HOBJECT			hParent = GetObject();
	HMODELSOCKET	hAttachSocket;

	// Make sure we can be shot off...

	if( !g_pAttachButeMgr->GetAttachmentDetachWhenShot( m_nAttachmentID ))
		return LTFALSE;

	// Get the world position of the socket this attachment is attached to...

	g_pModelLT->GetSocket( hParent, const_cast<char*>(pAttachmentPosition->GetName()), hAttachSocket );
	if( hAttachSocket == INVALID_MODEL_SOCKET )
		return LTFALSE;

	LTransform transAttach;
	g_pModelLT->GetSocketTransform( hParent, hAttachSocket, transAttach, true );


	LTVector vModelDims;
	g_pPhysicsLT->GetObjectDims( m_hModel, &vModelDims );

	LTVector vMin = transAttach.m_Pos - vModelDims;
	LTVector vMax = transAttach.m_Pos + vModelDims;

	// See if this attachment got hit by the shot... 

	if( DoesSegmentIntersectAABB( iInfo.m_Point, iInfo.m_Point + vDir * 2000.0f, vMin, vMax ))
	{
		HATTACHMENT hAttachment = LTNULL;
        g_pLTServer->FindAttachment(m_hObject, m_hModel, &hAttachment);
        if ( hAttachment != LTNULL )
		{
            g_pLTServer->RemoveAttachment(hAttachment);
		}

		Prop *pProp = dynamic_cast<Prop*>(g_pLTServer->HandleToObject( m_hModel ));
		if( !pProp )
			return LTFALSE;

		if( m_nDebrisID != DEBRISMGR_INVALID_ID )
		{
			// Create some debris at the attachment position and then the model will get removed...

			::CreatePropDebris( transAttach.m_Pos, vDir, m_nDebrisID );
		}
		else
		{
			g_pLTServer->SetObjectPos( m_hModel, &transAttach.m_Pos );
			g_pCommonLT->SetObjectFlags( m_hModel, OFT_Flags, FLAG_TOUCH_NOTIFY, FLAG_TOUCH_NOTIFY | FLAG_FORCEOPTIMIZEOBJECT );
		
			// Let the prop get setup for being shot off...
			
			pProp->HandleAttachmentImpact( pAttachmentPosition, vDir );
			
			// By setting the model handle to null the object will not get removed when the attachment is destroyed...
			
			m_hModel = LTNULL;
		}
		
		CAttachment::Destroy( this );

		pAttachmentPosition->SetAttachment( LTNULL );
	}

	// Return false so other attachments close to the hit node get shot off...
	
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
	char szModel[128] = "";
	char szSkin[128] = "";
	g_pAttachButeMgr->GetAttachmentModel(m_nAttachmentID,szModel,sizeof(szModel));
	g_pAttachButeMgr->GetAttachmentSkin(m_nAttachmentID,szSkin,sizeof(szSkin));
	sprintf(szSpawn, "Prop Filename %s; Skin %s; Gravity 0", szModel, szSkin);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachmentObject::HandleProjectileImpact
//
//	PURPOSE:	Tests being hit by a projectile
//
// ----------------------------------------------------------------------- //

LTBOOL CAttachmentObject::HandleProjectileImpact(CProjectile* pProjectile, CAttachmentPosition* pAttachmentPosition, IntersectInfo& iInfo, 
												 const LTVector& vDir, const LTVector& vFrom, ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	if( !pAttachmentPosition )
		return LTFALSE;

	HOBJECT			hParent = GetObject();
	HMODELSOCKET	hAttachSocket;

	// Make sure we can be shot off...

	if( !g_pAttachButeMgr->GetAttachmentDetachWhenShot( m_nAttachmentID ) && !IsAI( m_hModel ))
		return LTFALSE;
	
	// Get the world position of the socket this attachment is attached to...

	g_pModelLT->GetSocket( hParent, const_cast<char*>(pAttachmentPosition->GetName()), hAttachSocket );
	if( hAttachSocket == INVALID_MODEL_SOCKET )
		return LTFALSE;

	LTransform transAttach;
	g_pModelLT->GetSocketTransform( hParent, hAttachSocket, transAttach, true );


	LTVector vModelDims;
	g_pPhysicsLT->GetObjectDims( m_hModel, &vModelDims );

	LTVector vMin = transAttach.m_Pos - vModelDims;
	LTVector vMax = transAttach.m_Pos + vModelDims;

	// See if this attachment got hit by the shot... 

	if( DoesSegmentIntersectAABB( iInfo.m_Point, iInfo.m_Point + vDir * 2000.0f, vMin, vMax ))
	{
		if( IsAI( m_hModel ))
		{
			CAI* pAI = (CAI*)g_pLTServer->HandleToObject(m_hModel);
			if ( pAI->CanBeDamagedAsAttachment() )
			{
                g_pLTServer->CPrint("dam: %f -> %f", g_pLTServer->GetTime(), pAI->GetHitPoints());
				iInfo.m_hObject = m_hModel;
				return LTTRUE;
			}
		}
		else
		{
			HATTACHMENT hAttachment = LTNULL;
			g_pLTServer->FindAttachment(m_hObject, m_hModel, &hAttachment);
			if ( hAttachment != LTNULL )
			{
				g_pLTServer->RemoveAttachment(hAttachment);
			}

			if( m_nDebrisID != DEBRISMGR_INVALID_ID )
			{
				// Create some debris at the attachment position and then the object will get removed...

				::CreatePropDebris( transAttach.m_Pos, vDir, m_nDebrisID );
			}
			
			CAttachment::Destroy( this );
			pAttachmentPosition->SetAttachment( LTNULL );
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
	char szModel[128] = "";
	char szSkin[128] = "";
	g_pAttachButeMgr->GetAttachmentModel(m_nAttachmentID,szModel,sizeof(szModel));
	g_pAttachButeMgr->GetAttachmentSkin(m_nAttachmentID,szSkin,sizeof(szSkin));
	sprintf(szSpawn, "Prop Filename %s; Skin %s; Gravity 0", szModel, szSkin);
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

    // g_pCommonLT->SetObjectFlags(m_hModel, OFT_User, USRFLG_SPY_VISION, USRFLG_SPY_VISION);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachmentWeapon::HandleProjectileImpact
//
//	PURPOSE:	Tests being hit by a projectile
//
// ----------------------------------------------------------------------- //

LTBOOL CAttachmentWeapon::HandleProjectileImpact(CProjectile* pProjectile, CAttachmentPosition* pAttachmentPosition, IntersectInfo& iInfo, 
											     const LTVector& vDir, const LTVector& vFrom, ModelSkeleton eModelSkeleton, ModelNode eModelNode)
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

	AMMO const *pAmmo     = g_pWeaponMgr->GetAmmo(m_Weapons.GetCurWeapon()->GetAmmoId());
	WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon(m_Weapons.GetCurWeapon()->GetId());

	if (pAmmo && pWeapon && pWeapon->szHHModel[0])
	{
		// [KLS 7/15/02] - Pick a random value between 1 and clip-size for the
		// ammo...
		int nAmmoCount = GetRandom(1, pWeapon->nShotsPerClip);
		sprintf(szSpawn, "WeaponItem Gravity 0;AmmoAmount %d;WeaponType %s;AmmoType %s;MPRespawn 0",
			nAmmoCount, pWeapon->szName, pAmmo->szName);
	}
	else
	{
		szSpawn[0] = '\0';
	}
}

void CAttachmentWeapon::CreateAttachString(CAttachmentPosition* pAttachmentPosition,char* szAttach)
{
	if ( !m_Weapons.GetCurWeapon() )
	{
		szAttach[0] = '\0';
		return;
	}

	AMMO const *pAmmo     = g_pWeaponMgr->GetAmmo(m_Weapons.GetCurWeapon()->GetAmmoId());
	WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon(m_Weapons.GetCurWeapon()->GetId());

	if (pAmmo && pWeapon)
	{

		sprintf(szAttach, "ATTACH %s (%s,%s)",pAttachmentPosition->GetName(), pWeapon->szName, pAmmo->szName);
	}
	else
	{
		szAttach[0] = '\0';
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachmentWeapon::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 CAttachmentWeapon::ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg)
{
	CAttachment::ObjectMessageFn(pObject, hSender, pMsg);

	m_Weapons.ObjectMessageFn(pObject, hSender, pMsg);

	return 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachmentWeapon::Save
//
//	PURPOSE:	Save
//
// ----------------------------------------------------------------------- //

void CAttachmentWeapon::Save(ILTMessage_Write *pMsg)
{
	CAttachment::Save(pMsg);

	m_Weapons.Save(pMsg, 0);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachmentWeapon::Load
//
//	PURPOSE:	Load
//
// ----------------------------------------------------------------------- //

void CAttachmentWeapon::Load(ILTMessage_Read *pMsg)
{
	CAttachment::Load(pMsg);

	m_Weapons.Load(pMsg, 0);

	// Check if we had a current weapon.
	int nCurWeaponId = m_Weapons.GetCurWeaponId();
	if( nCurWeaponId >= 0 )
	{
		m_Weapons.DeselectCurWeapon();	// Deselect so we'll change to it
		m_Weapons.ChangeWeapon(nCurWeaponId);
	}
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

uint32 CAttachmentPosition::ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg)
{
	if ( m_pAttachment )
	{
		m_pAttachment->ObjectMessageFn(pObject, hSender, pMsg);
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

void CAttachmentPosition::Save(ILTMessage_Write *pMsg)
{
	SAVE_BOOL(!!m_pAttachment);
	if ( m_pAttachment )
	{
		SAVE_DWORD(m_pAttachment->GetType());
		m_pAttachment->Save(pMsg);
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

void CAttachmentPosition::Load(ILTMessage_Read *pMsg)
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

		m_pAttachment->Load(pMsg);
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

#ifndef __PSX2
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
        sm_AttachButeMgr.Init(szFile);
		sm_bInitted = LTTRUE;
		sm_WeaponMgrPlugin.PreHook_EditStringList(szRezPath, szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength);

	}

	if ( !_strcmpi("Attachments", szPropName) )
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

	char szBuffer[128];
	strcpy(aszStrings[(*pcStrings)++], c_szNoAttachment);
    for ( uint32 iAttachment = 0 ; iAttachment < cAttachments ; iAttachment++ )
	{
		szBuffer[0] = '\0';
		sm_AttachButeMgr.GetAttachmentName(iAttachment, szBuffer, 128);
		if(szBuffer[0])
		{
			strcpy(aszStrings[(*pcStrings)++], szBuffer);
		}
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
         !_stricmp("Snowmobile",    szPropName) ||
         !_stricmp("Light",			szPropName) ||
         !_stricmp("Head",          szPropName) ||
         !_stricmp("Eyes",          szPropName) ||
         !_stricmp("Nose",          szPropName) ||
         !_stricmp("Chin",          szPropName) ||
         !_stricmp("Back",          szPropName) ||
         !_stricmp("LeftLowerArm",	szPropName) ||
         !_stricmp("LeftUpperArm",	szPropName) ||
         !_stricmp("RightLowerArm",	szPropName) ||
         !_stricmp("RightUpperArm",	szPropName) ||
         !_stricmp("LeftLowerLeg",	szPropName) ||
         !_stricmp("LeftUpperLeg",	szPropName) ||
         !_stricmp("RightLowerLeg",	szPropName) ||
         !_stricmp("RightUpperLeg",	szPropName) ||
         !_stricmp("UpperTorso",	szPropName) ||
         !_stricmp("Torso",			szPropName) ||
         !_stricmp("LeftShoulder",	szPropName) ||
         !_stricmp("RightShoulder",	szPropName) ||
		 !_stricmp("LeftHand2",		szPropName) ||
         !_stricmp("RightHand2",	szPropName) ||
         !_stricmp("LeftLowerArm2",	szPropName) ||
         !_stricmp("LeftLowerLeg2",	szPropName) ||
         !_stricmp("LeftUpperArm2",	szPropName) ||
         !_stricmp("LeftUpperLeg2",	szPropName) ||
         !_stricmp("RightUpperArm2",	szPropName) ||
         !_stricmp("RightUpperLeg2",	szPropName) ||
         !_stricmp("RightLowerArm2",	szPropName) ||
         !_stricmp("RightLowerLeg2",	szPropName) ||
         !_stricmp("UpperTorso2",		szPropName) 
		 )
	{
		PopulateStringList(aszStrings, pcStrings, cMaxStrings, cMaxStringLength);
		return LT_OK;
	}
	else
	{
		return LT_UNSUPPORTED;
	}
};

#pragma optimize ("",on)
#endif // __PSX2
