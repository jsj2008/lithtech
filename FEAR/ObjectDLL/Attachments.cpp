// ----------------------------------------------------------------------- //
//
// MODULE  : Attachments.cpp
//
// PURPOSE : Attachments aggregate object - Implementation
//
// CREATED : 
//
// (c) 1997-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "Attachments.h"
#include "ServerUtilities.h"
#include "ObjectMsgs.h"
#include "Prop.h"
#include "Spawner.h"
#include "AI.h"
#include "AIUtils.h"
#include "CharacterMgr.h"
#include "AttachmentDB.h"

// Static constants

const static char c_szNoAttachment[] = NO_ATTACHMENT;

CMDMGR_BEGIN_REGISTER_CLASS( CAttachments )
CMDMGR_END_REGISTER_CLASS( CAttachments, IAggregate )

CAttachments* CAttachments::Create( )
{
	return debug_new( CAttachments );
}

void CAttachments::Destroy(CAttachments* pAttachments)
{
	debug_delete( pAttachments );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetAttachmentRecord()
//
//	PURPOSE:	Returns the attachment with the passed in name.
//
//				There are current two attachment categories with a common
//				base category representing the types.  As the attachment
//				system treats them 'abstractly' (stores a record which 
//				may be of either type), a way to determine the type of the
//				record is needed.  This should be considered a temp solution
//				pending attachment refactoring.  For more information, see 
//				AttachmentDB
//
// ----------------------------------------------------------------------- //

HRECORD GetAttachmentRecord(const char* const pszAttachmentName)
{
	HRECORD hAttachment = NULL;

	hAttachment = AttachmentDB::Instance().GetPropAttachmentRecordByName(pszAttachmentName);
	if (NULL != hAttachment)
		return hAttachment;

	hAttachment = AttachmentDB::Instance().GetObjectAttachmentRecordByName(pszAttachmentName);
	if (NULL != hAttachment)
		return hAttachment;

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetAttachmentType()
//
//	PURPOSE:	Returns the type of the attachment.  Look at doing this a 
//				less error prone way when the attachment code is cleaned up.
//
//				There are current two attachment categories with a common
//				base category representing the types.  As the attachment
//				system treats them 'abstractly' (stores a record which 
//				may be of either type), a way to determine the type of the
//				record is needed.  This should be considered a temp solution
//				pending attachment refactoring.  For more information, see 
//				AttachmentDB
//
// ----------------------------------------------------------------------- //

AttachmentType GetAttachmentType(HRECORD hAttachmentRecord)
{
	HRECORD hAttachment = NULL;

	HCATEGORY hAttachmentCategory = g_pLTDatabase->GetRecordParent(hAttachmentRecord);
	if (AttachmentDB::Instance().GetPropAttachmentCategory() == hAttachmentCategory)
	{
		return ATTACHMENT_TYPE_PROP;
	}
	else if ( AttachmentDB::Instance().GetObjectAttachmentCategory() == hAttachmentCategory )
	{
		return ATTACHMENT_TYPE_OBJECT;
	}

	return ATTACHMENT_TYPE_INVALID;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::CAttachments
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CAttachments::CAttachments() 
:	IAggregate		( "CAttachments" ),
	m_hObject		( NULL ),
	m_cObjects		( 0 ),
	m_cProps		( 0 ),
	m_bHidden		( false )
{
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
	DeleteAllAttachmentPositions();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::EngineMessageFn
//
//	PURPOSE:	Handle message from the engine
//
// ----------------------------------------------------------------------- //

uint32 CAttachments::EngineMessageFn(LPBASECLASS pObject, uint32 messageID, void *pData, float fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			int nInfo = (int)fData;
			if (nInfo == PRECREATE_WORLDFILE || nInfo == PRECREATE_STRINGPROP)
			{
				ReadProp(pObject, &((ObjectCreateStruct*)pData)->m_cProperties);
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
//	ROUTINE:	CAttachments::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

bool CAttachments::ReadProp(LPBASECLASS pObject, const GenericPropList *pProps)
{
	LTASSERT(pObject && pProps, "Attachments encountered without either object or properties");
	if (!pObject || !pProps)
		return false;

	const char *pszAttachmentName = NULL;

	m_saAttachmentNames.reserve( MAX_ATTACHMENT_NAMES );

	// Read in the attachment names and insert them into the array to use when initializing...

	char szPropName[32] = {0};
	for( uint32 nAttachment = 0; nAttachment < MAX_ATTACHMENT_NAMES; ++nAttachment )
	{
		LTSNPrintF( szPropName, ARRAY_LEN(szPropName), "Attachment%d", nAttachment );

		pszAttachmentName = pProps->GetString( szPropName, c_szNoAttachment );

		// Don't set the attachment name if the selection was set to <none>...

		if( pszAttachmentName && pszAttachmentName[0] &&
			!LTStrIEquals( pszAttachmentName, c_szNoAttachment ))
		{
			m_saAttachmentNames.push_back( pszAttachmentName );
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::Create
//
//	PURPOSE:	Create/Destroy an instances of CAttachmentPosition
//
// ----------------------------------------------------------------------- //

bool CAttachments::m_bBankInitialized = false;
ObjectBank<CAttachmentPosition, LT_MEM_TYPE_OBJECTSHELL> CAttachments::m_AttachmentPositionBank;

CAttachmentPosition* CAttachments::CreateAttachmentPosition()
{
	if( !m_bBankInitialized )
	{
		m_AttachmentPositionBank.Init( 64, 64 );
		m_bBankInitialized = true;
	}

	return m_AttachmentPositionBank.Allocate( );
}

void CAttachments::DestroyAttachmentPosition( CAttachmentPosition* pAttachmentPosition )
{
	if( !pAttachmentPosition )
		return;

	m_AttachmentPositionBank.Free( pAttachmentPosition );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::Attach
//
//	PURPOSE:	Dynamically add an attachment
//
// ----------------------------------------------------------------------- //

void CAttachments::Attach( const char *szAttachment , const char *szPositionOverride /* = NULL  */ )
{
	if( !szAttachment )
		return;

	const char *szAttachmentPosition = szPositionOverride;

	// If an override position was specified use that otherwise use the attachments default socket...

	if( !szAttachmentPosition || !szAttachmentPosition[0] )
	{
		HRECORD hAttachmentRecord = GetAttachmentRecord( szAttachment );
		if( NULL == hAttachmentRecord)
			return;

		szAttachmentPosition = AttachmentDB::Instance().GetAttachentSocket( hAttachmentRecord );
	}

	AttachmentPositionArray::iterator iter;
	for( iter = m_vecAttachmentPositions.begin(); iter != m_vecAttachmentPositions.end(); ++iter )
	{
		CAttachmentPosition* pAttachmentPosition = *iter;

		// Verify that the name is valid (in case this names were added since
		// this map has been in existance)
		LTASSERT( pAttachmentPosition->GetName()!=NULL, "Attachment Name is null" );

		if( LTStrIEquals( szAttachmentPosition, pAttachmentPosition->GetName() ))
		{
			if ( pAttachmentPosition->HasAttachment() )
			{
				CAttachment* pAttachment = pAttachmentPosition->GetAttachment( );
				char szExistingName[256] = "";
				AttachmentDB::Instance().GetAttachmentName( pAttachment->GetID( ), szExistingName, ARRAY_LEN( szExistingName ));
				char szModelName[MAX_PATH];
				g_pLTServer->GetModelLT()->GetModelFilename(m_hObject, szModelName, ARRAY_LEN(szModelName));
				if( IsAI( m_hObject ))
				{
					AITRACE( AIShowCharacters, ( m_hObject, "WARNING: CAttachments::Attach: Attachment Position already occupied by %s: %s on model: %s", szExistingName, szAttachmentPosition, szModelName ) );
				}
				else
				{
					LTASSERT_PARAM3( 0, "CAttachments::Attach: Attachment Position already occupied by %s: %s on model: %s", szExistingName, szAttachmentPosition, szModelName );
				}
			}
			else
			{
				pAttachmentPosition->SetAttachmentName( szAttachment );
				CreateAttachment(pAttachmentPosition);
			}

			return;
		}
	}

	// Assert if the attachment position is not found.
	char szModelName[MAX_PATH];
    g_pLTServer->GetModelLT()->GetModelFilename(m_hObject, szModelName, ARRAY_LEN(szModelName));

	// Get the name of the socketset, as the socket must be listed in the set to be found, even if it is on the model.

	const char* pszSocketSetRecordName = "<no socket set specified>";
	CCharacter *pChar = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject( m_hObject ));
	if( pChar )
	{
		ModelsDB::HSOCKETSET hSocketSet = g_pModelsDB->GetModelSocketSet( pChar->GetModel() );
		const char* const pszSocketSet = g_pModelsDB->GetRecordName( hSocketSet );
		if ( pszSocketSet )
		{
			pszSocketSetRecordName = pszSocketSet;
		}
	}

	if( IsAI( m_hObject ))
	{
		AIASSERT3( 0, m_hObject, "CAttachments::Attach: Attachment Position not found: %s on model: %s.  Is the socket listed in the SocketSet: '%s'?", szAttachmentPosition, szModelName, pszSocketSetRecordName );
	}
	else
	{
		LTASSERT_PARAM3( 0, "CAttachments::Attach: Attachment Position already occupied: %s on model: %s.  Is the socket listed in the SocketSet: '%s'?", szAttachmentPosition, szModelName, pszSocketSetRecordName );
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::Detach
//
//	PURPOSE:	Dynamically remove an attachment
//
// ----------------------------------------------------------------------- //

void CAttachments::Detach( const char *szAttachment, const char *szPositionOverride /* = NULL  */ )
{
	if( !szAttachment )
		return;

	if( LTStrIEquals( szAttachment, SOCKET_ATTACHMENT ))
	{
		// Make sure a socket was specified...

		if( !szPositionOverride || !szPositionOverride[0] )
			return;
	}

	const char *szAttachmentPosition = szPositionOverride;

	// If an override position was specified use that otherwise use the attachments default socket...

	if( !szAttachmentPosition || !szAttachmentPosition[0] )
	{
		HRECORD hAttachmentRecord = GetAttachmentRecord( szAttachment );
		if( NULL == hAttachmentRecord )
			return;

		szAttachmentPosition = AttachmentDB::Instance().GetAttachentSocket( hAttachmentRecord );
	}
	
	AttachmentPositionArray::iterator iter;
	for( iter = m_vecAttachmentPositions.begin(); iter != m_vecAttachmentPositions.end(); ++iter )
	{
		CAttachmentPosition* pAttachmentPosition = *iter;

		if ( LTStrIEquals( szAttachmentPosition, pAttachmentPosition->GetName() ))
		{
			CAttachment* pAttachment = pAttachmentPosition->GetAttachment();
			if ( pAttachment )
			{
				switch ( pAttachment->GetType() )
				{
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
	AttachmentPositionArray::iterator iter;
	for( iter = m_vecAttachmentPositions.begin(); iter != m_vecAttachmentPositions.end(); ++iter )
	{
		CAttachmentPosition* pAttachmentPosition = *iter;

		if ( !LTStrICmp(szAttachmentPosition, pAttachmentPosition->GetName()) )
		{
			return pAttachmentPosition->GetAttachment();
		}
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::HideAttachments
//
//	PURPOSE:	Hide/Show attachments.
//
// ----------------------------------------------------------------------- //

void CAttachments::HideAttachments(bool bHide)
{
	m_bHidden = bHide;

	AttachmentPositionArray::iterator iter;
	for( iter = m_vecAttachmentPositions.begin(); iter != m_vecAttachmentPositions.end(); ++iter )
	{
		CAttachment* pAttachment = (*iter)->GetAttachment( );
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
	LTASSERT(pAttachmentPosition, "TODO: Add description here");
	if ( !pAttachmentPosition ) return;

	if ( pAttachmentPosition->HasAttachment() ) return;
	if ( !pAttachmentPosition->HasAttachmentName() ) return;

    const char *szAttachmentName = pAttachmentPosition->GetAttachmentName();

    if ( !LTStrICmp(c_szNoAttachment, szAttachmentName) ) return;

	HRECORD hAttachmentRecord = GetAttachmentRecord(szAttachmentName);

	if ( NULL == hAttachmentRecord )
	{
        g_pLTServer->CPrint("Unmatched attable name %s (Line %d of %s)", szAttachmentName, __LINE__, __FILE__);
		return;
	}

	switch ( GetAttachmentType(hAttachmentRecord) )
	{
		case ATTACHMENT_TYPE_OBJECT:
		{
			CreateObjectAttachment(pAttachmentPosition, hAttachmentRecord);
		}
		break;

		case ATTACHMENT_TYPE_PROP:
		{
			CreatePropAttachment(pAttachmentPosition, hAttachmentRecord);
		}
		break;

		default:
		{
			char szAttachmentRecord[80];
			AttachmentDB::Instance().GetAttachmentName(hAttachmentRecord, szAttachmentRecord, LTARRAYSIZE(szAttachmentRecord));
			g_pLTServer->CPrint("Illegal attachment type encountered on attachment #%s", szAttachmentRecord);
			return;
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::CreateObjectAttachment
//
//	PURPOSE:	Creates an object attachment
//
// ----------------------------------------------------------------------- //

void CAttachments::CreateObjectAttachment(CAttachmentPosition *pAttachmentPosition, HRECORD hAttachmentRecord)
{
	// Prepare the object create struct

	ObjectCreateStruct theStruct;

	theStruct.m_Flags = FLAG_VISIBLE | FLAG_GOTHRUWORLD;
	theStruct.m_NextUpdate = 1.0f;

	// Get the attachment material, model, and class names

	char szClass[128];
	szClass[0] = '\0';

	AttachmentDB::Instance().GetAttachmentClass(hAttachmentRecord,szClass,sizeof(szClass));
	if ( !szClass[0] ) return;


	// Set up the model/material filenames

	AttachmentDB::Instance().GetAttachmentModel(hAttachmentRecord,theStruct.m_Filename,sizeof(theStruct.m_Filename));
	int nMaterials = AttachmentDB::Instance().GetAttachmentMaterialCount(hAttachmentRecord);
	for (int nEachMaterial = 0; nEachMaterial < nMaterials; ++nEachMaterial)
	{
		if (MAX_MATERIALS_PER_MODEL <= nEachMaterial )
		{
			LTASSERT_PARAM1(0, "CAttachments::CreateObjectAttachment : Too many materials specified for attachment %s.", 
				g_pLTDatabase->GetRecordName(hAttachmentRecord));
			break;
		}

		AttachmentDB::Instance().GetAttachmentMaterial(hAttachmentRecord, nEachMaterial, theStruct.m_Materials[nEachMaterial], MAX_CS_FILENAME_LEN+1);
	}

	// Create the attachment

    HCLASS hClass = g_pLTServer->GetClass(szClass);
	if ( hClass == NULL )
	{
		char szError[512];
		LTSNPrintF( szError, LTARRAYSIZE( szError ), "CreateObjectAttachment: No object with class: %s found", szClass );
		LTERROR( szError );
	}

	if(AttachmentDB::Instance().GetAttachmentTranslcuent(hAttachmentRecord))
		theStruct.m_Flags2 |= FLAG2_FORCETRANSLUCENT;

	char szModel[128] = "";
	AttachmentDB::Instance().GetAttachmentProperties(hAttachmentRecord,szModel,sizeof(szModel));
    LPBASECLASS pModel = g_pLTServer->CreateObjectProps(hClass, &theStruct, szModel);
	if ( !pModel )
	{
		return;
	}

	// Attach it

	HATTACHMENT hAttachment;
	LTVector   cEmptyVector(0,0,0);
	LTRotation cEmptyRotation;
    g_pLTServer->CreateAttachment(m_hObject, pModel->m_hObject, (char*)pAttachmentPosition->GetName(), &cEmptyVector, &cEmptyRotation, &hAttachment);

	// Notify the attachment that it is being attached.
	g_pCmdMgr->QueueMessage( g_pLTServer->HandleToObject(m_hObject), pModel, KEY_ATTACH );

	CAttachment* pAttachment = CAttachment::Create(ATTACHMENT_TYPE_OBJECT);
	pAttachment->Init(m_hObject, pModel->m_hObject, hAttachmentRecord);
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

void CAttachments::CreatePropAttachment(CAttachmentPosition *pAttachmentPosition, HRECORD hAttachmentRecord)
{
	// Prepare the object create struct

	ObjectCreateStruct theStruct;

	theStruct.m_Flags = FLAG_VISIBLE | FLAG_GOTHRUWORLD | FLAG_NOTINWORLDTREE;

	// KLS 5/2/04 - Projectiles shouldn't impact on attachments...
	theStruct.m_UserData = USRFLG_IGNORE_PROJECTILES;
	
	theStruct.m_NextUpdate = 1.0f;

	// Create the attachment prop at the parents position to avoid it being removed if created outside the world...
	LTRigidTransform tParentTrans;
	g_pLTServer->GetObjectTransform( m_hObject, &tParentTrans );

	theStruct.m_Pos = tParentTrans.m_vPos;
	theStruct.m_Rotation = tParentTrans.m_rRot;

	// Get the attachment material, model, and class names

	char szModel[MAX_CS_FILENAME_LEN];
	char szMaterial[MAX_CS_FILENAME_LEN];
	char szClass[128];

	LTStrCpy( szClass, "Prop", LTARRAYSIZE( szClass ));
	AttachmentDB::Instance().GetAttachmentModel(hAttachmentRecord, szModel, LTARRAYSIZE(szModel)); 
	AttachmentDB::Instance().GetAttachmentMaterial(hAttachmentRecord, 0, szMaterial, LTARRAYSIZE(szMaterial));

	// Set up the model/material filenames

	theStruct.SetFileName(szModel);
	int nMaterials = AttachmentDB::Instance().GetAttachmentMaterialCount(hAttachmentRecord);
	for (int nEachMaterial = 0; nEachMaterial < nMaterials; ++nEachMaterial)
	{
		if (MAX_MATERIALS_PER_MODEL <= nEachMaterial )
		{
			LTASSERT_PARAM1(0, "CAttachments::CreateObjectAttachment : Too many materials specified for attachment %s.", 
				g_pLTDatabase->GetRecordName(hAttachmentRecord));
			break;
		}

		AttachmentDB::Instance().GetAttachmentMaterial(hAttachmentRecord, nEachMaterial, theStruct.m_Materials[nEachMaterial], MAX_CS_FILENAME_LEN+1);
	}

	if(AttachmentDB::Instance().GetAttachmentTranslcuent(hAttachmentRecord))
		theStruct.m_Flags2 |= FLAG2_FORCETRANSLUCENT;

	// Create the attachment

    HCLASS hClass = g_pLTServer->GetClass(szClass);
	char szStr[128] = "";
	AttachmentDB::Instance().GetAttachmentProperties(hAttachmentRecord,szStr,LTARRAYSIZE(szStr));
    Prop* pModel = (Prop*)g_pLTServer->CreateObjectProps(hClass, &theStruct, szStr);
	if ( !pModel ) 
	{
		return;
	}

	EEngineLOD eShadowLOD = eEngineLOD_Never;
	g_pLTServer->GetObjectShadowLOD( m_hObject, eShadowLOD );
	g_pLTServer->SetObjectShadowLOD( pModel->m_hObject, eShadowLOD );
	
	// Don't need to do any move to floor checks.
	pModel->SetMoveToFloor( false );

	pModel->GetDestructible()->SetNeverDestroy(true);

	// Attach it

	HATTACHMENT hAttachment;
	LTVector cEmptyVector(0,0,0);
	LTRotation cIdentityRotation = LTRotation::GetIdentity(); 
    LTRESULT dRes = g_pLTServer->CreateAttachment(m_hObject, pModel->m_hObject, (char*)pAttachmentPosition->GetName(),
													&cEmptyVector, &cIdentityRotation, &hAttachment);

	if ( dRes != LT_OK ) 
	{
		// Error: Socket not found or other such error detected.  Print out a 
		// message so that we know this is going on and can fix it.
		char szError[1024];
		char szModelName[MAX_PATH];
		g_pLTServer->GetModelLT()->GetModelFilename(m_hObject, szModelName, ARRAY_LEN(szModelName));
		LTSNPrintF( szError, LTARRAYSIZE( szError ), "CAttachments::Attach: unable to create socket: %s on model: %s", pAttachmentPosition->GetName(), szModelName );
		g_pLTServer->CPrint( szError );
		LTERROR( szError );
	}

 	// Notify the attachment that it is being attached.
	g_pCmdMgr->QueueMessage( g_pLTServer->HandleToObject( m_hObject ), pModel, KEY_ATTACH );

	CAttachment* pAttachment = CAttachment::Create(ATTACHMENT_TYPE_PROP);
	pAttachment->Init(m_hObject, pModel->m_hObject, hAttachmentRecord);
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

	AttachmentPositionArray::iterator iter;
	for( iter = m_vecAttachmentPositions.begin(); iter != m_vecAttachmentPositions.end(); ++iter )
	{
		CAttachmentPosition *pCurPos = *iter;
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
				LTVector   cEmptyVector(0,0,0);
				LTRotation cEmptyRotation;
                g_pLTServer->CreateAttachment(m_hObject, hModel, (char*)pCurPos->GetName(),
					&cEmptyVector, &cEmptyRotation, &hAttachment);

				// Notify the attachment that it is being attached.
				g_pCmdMgr->QueueMessage( g_pLTServer->HandleToObject( m_hObject ),
										 g_pLTServer->HandleToObject( hModel ),
										 KEY_ATTACH );

				// Re-init to set m_hObject...

				pCur->Init(m_hObject, hModel, pCur->GetID());

				SetAttachmentHiddenState( *pCur );
			}
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

	DeleteAllAttachmentPositions();

	// Use the sockets specified in modelbutes to add the attachment positions...

	if( IsCharacter( m_hObject ))
	{
		CCharacter *pChar = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject( m_hObject ));
		if( !pChar )
			return;

		ModelsDB::HSOCKETSET hSocketSet = g_pModelsDB->GetModelSocketSet( pChar->GetModel() );

		int nNumSockets = g_pModelsDB->GetSocketSetNumSockets( hSocketSet );
		m_vecAttachmentPositions.reserve( nNumSockets );

		// Setup every socket in the set and add it to the list...

		for( int i = 0; i < nNumSockets; ++i )
		{
			const char* pSocket = g_pModelsDB->GetSocketSetSocket( hSocketSet, i );
			if( pSocket )
			{
				CAttachmentPosition* pPosition = CreateAttachmentPosition( );
				pPosition->SetName( pSocket );
				m_vecAttachmentPositions.push_back( pPosition );
			}
		}
	}

	// Use the list of attachment names to create the initial attachmetns...

	StringArray::iterator saiter;
	for( saiter = m_saAttachmentNames.begin(); saiter != m_saAttachmentNames.end(); ++saiter )
	{
		HRECORD hAttachmentRecord = GetAttachmentRecord( (*saiter).c_str() );

		const char *pSocketName = AttachmentDB::Instance().GetAttachentSocket( hAttachmentRecord );
		if( !pSocketName )
			continue;

		AttachmentPositionArray::iterator iter;
		for( iter = m_vecAttachmentPositions.begin(); iter != m_vecAttachmentPositions.end(); ++iter )
		{
			CAttachmentPosition* pPosition = *iter;
			if( LTStrIEquals( pPosition->GetName(), pSocketName ))
			{
				pPosition->SetAttachmentName( (*saiter).c_str() );
				break;
			}
		}
	}

	// The list of attachment names should no longer be needed.  Clear the contents and free the memory...

	StringArray().swap( m_saAttachmentNames );


	// Create all the attachmetns that were added...

	AttachmentPositionArray::iterator apiter;
	for( apiter = m_vecAttachmentPositions.begin(); apiter != m_vecAttachmentPositions.end(); ++apiter )
	{
		CreateAttachment( *apiter );
	}

	m_bHidden = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::DeleteAllAttachmentPositions
//
//	PURPOSE:	Deletes all the CAttachmentPosition objects we own.
//
// ----------------------------------------------------------------------- //

void CAttachments::DeleteAllAttachmentPositions( )
{
	AttachmentPositionArray::iterator apiter;
	for( apiter = m_vecAttachmentPositions.begin(); apiter != m_vecAttachmentPositions.end(); ++apiter )
	{
		DestroyAttachmentPosition( *apiter );
	}

	m_vecAttachmentPositions.clear( );
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

bool CAttachments::HandleProjectileImpact(CProjectile* pProjectile, IntersectInfo& iInfo, const LTVector& vDir, const LTVector& vFrom, ModelsDB::HSKELETON hModelSkeleton, ModelsDB::HNODE hModelNode)
{
	AttachmentPositionArray::iterator iter;
	for( iter = m_vecAttachmentPositions.begin(); iter != m_vecAttachmentPositions.end(); ++iter )
	{
		CAttachmentPosition* pPosition = *iter;

		if( !pPosition->HasAttachment() )
			continue;

		CAttachment* pAttachment = pPosition->GetAttachment();
		if ( pAttachment->HandleProjectileImpact(pProjectile, pPosition, iInfo, vDir, vFrom, hModelSkeleton, hModelNode) )
		{
			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachments::HandleSever
//
//	PURPOSE:	Handles limbs being severed
//
// ----------------------------------------------------------------------- //

void CAttachments::HandleSever(const char* szAttachmentPosition, const LTVector& vDir)
{
	AttachmentPositionArray::iterator iter;
	for( iter = m_vecAttachmentPositions.begin(); iter != m_vecAttachmentPositions.end(); ++iter )
	{
		CAttachmentPosition* pAttachmentPosition = *iter;

		if ( !LTStrICmp(szAttachmentPosition, pAttachmentPosition->GetName()) )
		{
			CAttachment* pAttachment = pAttachmentPosition->GetAttachment();

			if (pAttachment)
				pAttachment->HandleSever(pAttachmentPosition,vDir);
			return;
		}
	}
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
	AttachmentPositionArray::iterator iter;
	for( iter = m_vecAttachmentPositions.begin(); iter != m_vecAttachmentPositions.end(); ++iter )
	{
		CAttachmentPosition *pPosition = *iter;
		CAttachment* pAttachment = pPosition->GetAttachment();

		// Check if attachment is flagged to delete on death.
		// (e.g. smoke puffs coming from a smoking AIs mouth should not
		// continue from the body's mouth!)

		if( pAttachment && 	AttachmentDB::Instance().GetAttachmentDeleteOnDeath( pAttachment->GetID() ) )
		{
			HATTACHMENT hAttachment;
			if (LT_OK == g_pLTServer->FindAttachment(m_hObject, pAttachment->GetModel(), &hAttachment))
			{
				if (LT_OK != g_pLTServer->RemoveAttachment(hAttachment))
					ASSERT(!"failed to remove attachment");
			}

			g_pLTServer->RemoveObject(pAttachment->GetModel());
			CAttachment::Destroy(pAttachment);

			pPosition->SetAttachment(NULL);
			pAttachment = NULL;
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

	AttachmentPositionArray::iterator iter;
	for( iter = m_vecAttachmentPositions.begin(); iter != m_vecAttachmentPositions.end(); ++iter )
	{
		CAttachmentPosition *pCurPos = *iter;
		if( !pCurPos )
			break;

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
	AttachmentPositionArray::iterator iter;
	for( iter = m_vecAttachmentPositions.begin(); iter != m_vecAttachmentPositions.end(); ++iter )
	{
		CAttachmentPosition *pAttachPos = *iter;
		if( !pAttachPos )
			continue;

		CAttachment *pAttachment = pAttachPos->GetAttachment();
		if( pAttachment )
		{
			HATTACHMENT hAttachment;
			if( LT_OK == g_pLTServer->FindAttachment( m_hObject, pAttachment->GetModel(), &hAttachment ))
			{
				// Unattach

				g_pLTServer->RemoveAttachment( hAttachment );

				// Reattach
				LTVector   cEmptyVector(0,0,0);
				LTRotation cEmptyRotation;				
				if( LT_OK != g_pLTServer->CreateAttachment( m_hObject, pAttachment->GetModel(), pAttachPos->GetName(), &cEmptyVector, &cEmptyRotation, &hAttachment ))
				{
					LTASSERT_PARAM1( 0, "CAttachments::RemoveAndRecreateAttachments: Unable to reattach attachment at position '%s'", pAttachPos->GetName() );
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
	AttachmentPositionArray::iterator iter;
	for( iter = m_vecAttachmentPositions.begin(); iter != m_vecAttachmentPositions.end(); ++iter )
	{
		CAttachmentPosition *pAttachPos = *iter;
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
	SAVE_INT(m_cObjects);
	SAVE_INT(m_cProps);
	SAVE_bool( m_bHidden );

	SAVE_DWORD( m_vecAttachmentPositions.size() );
	AttachmentPositionArray::iterator iter;
	for( iter = m_vecAttachmentPositions.begin(); iter != m_vecAttachmentPositions.end(); ++iter )
	{
		(*iter)->Save( pMsg );
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
	LOAD_INT(m_cObjects);
	LOAD_INT(m_cProps);
	LOAD_bool( m_bHidden );

	uint32 dwSize;
	LOAD_DWORD( dwSize );

	DeleteAllAttachmentPositions( );
	m_vecAttachmentPositions.reserve( dwSize );

	for( uint32 i = 0; i < dwSize; ++i )
	{
		CAttachmentPosition* pPosition = CreateAttachmentPosition( );
		pPosition->Load( pMsg );
	
		m_vecAttachmentPositions.push_back( pPosition );
	}	
}

CAttachment* CAttachment::Create(uint32 nAttachmentType)
{
	switch ( nAttachmentType )
	{
		case ATTACHMENT_TYPE_OBJECT:
			return debug_new( CAttachmentObject );
			break;
		case ATTACHMENT_TYPE_PROP:
			return debug_new( CAttachmentProp );
			break;
	}

	return NULL;
}

void CAttachment::Destroy(CAttachment* pAttachment)
{
	debug_delete( pAttachment );
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
	m_hObject		= NULL;
	m_hAttachmentRecord	= NULL;
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
        HATTACHMENT hAttachment = NULL;

        g_pLTServer->FindAttachment(m_hObject, m_hModel, &hAttachment);

        if ( hAttachment != NULL )
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

void CAttachment::Init(HOBJECT hObj, HOBJECT hModel, HRECORD hAttachmentRecord)
{
	m_hObject = hObj;
	m_hModel = hModel;
	
	// Make sure the object references are immediately valid, otherwise we need to remove the attachment...
	if( !m_hObject || !m_hModel )
	{
		HATTACHMENT hAttachment;

		// Need to check the original HOBJECTs since one of the references are invalid...
		if( LT_OK == g_pLTServer->FindAttachment( hObj, hModel, &hAttachment ))
		{
			g_pLTServer->RemoveAttachment( hAttachment );
		}
	}
	
	m_hAttachmentRecord = hAttachmentRecord;

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
	SAVE_HRECORD(m_hAttachmentRecord);
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
	LOAD_HRECORD(m_hAttachmentRecord, GetAttachmentCategory());
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

bool CAttachmentProp::HandleProjectileImpact(CProjectile* pProjectile, CAttachmentPosition* pAttachmentPosition, IntersectInfo& iInfo, 
											 const LTVector& vDir, const LTVector& vFrom, ModelsDB::HSKELETON hModelSkeleton, ModelsDB::HNODE hModelNode)
{
	if( !pAttachmentPosition )
		return false;

	HOBJECT			hParent = GetObject();
	HMODELSOCKET	hAttachSocket;

	// Make sure we can be shot off...

	if( !AttachmentDB::Instance().GetAttachmentDetachWhenShot( m_hAttachmentRecord ))
		return false;

	// Get the world position of the socket this attachment is attached to...

	g_pModelLT->GetSocket( hParent, const_cast<char*>(pAttachmentPosition->GetName()), hAttachSocket );
	if( hAttachSocket == INVALID_MODEL_SOCKET )
		return false;

	LTTransform transAttach;
	g_pModelLT->GetSocketTransform( hParent, hAttachSocket, transAttach, true );


	LTVector vModelDims;
	g_pPhysicsLT->GetObjectDims( m_hModel, &vModelDims );

	LTVector vMin = transAttach.m_vPos - vModelDims;
	LTVector vMax = transAttach.m_vPos + vModelDims;

	// See if this attachment got hit by the shot... 

	if( DoesSegmentIntersectAABB( iInfo.m_Point, iInfo.m_Point + vDir * 2000.0f, vMin, vMax ))
	{
		HATTACHMENT hAttachment = NULL;
        g_pLTServer->FindAttachment(m_hObject, m_hModel, &hAttachment);
        if ( hAttachment != NULL )
		{
            g_pLTServer->RemoveAttachment(hAttachment);
		}

		Prop *pProp = dynamic_cast<Prop*>(g_pLTServer->HandleToObject( m_hModel ));
		if( !pProp )
			return false;

		g_pLTServer->SetObjectPos( m_hModel, transAttach.m_vPos );
		g_pCommonLT->SetObjectFlags( m_hModel, OFT_Flags, FLAG_TOUCH_NOTIFY, FLAG_TOUCH_NOTIFY | FLAG_NOTINWORLDTREE );
		g_pLTServer->SetNetFlags( m_hModel, NETFLAG_POSUNGUARANTEED | NETFLAG_ROTUNGUARANTEED );
	
		// Let the prop get setup for being shot off...
		
		pProp->HandleAttachmentImpact( pAttachmentPosition, vDir );
		
		// By setting the model handle to null the object will not get removed when the attachment is destroyed...
		m_hModel = NULL;
		
		CAttachment::Destroy( this );

		pAttachmentPosition->SetAttachment( NULL );
	}

	// Return false so other attachments close to the hit node get shot off...
	
	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachmentProp::HandleSever
//
//	PURPOSE:	Handles having the limb we're attached to being severed
//
// ----------------------------------------------------------------------- //

void CAttachmentProp::HandleSever(CAttachmentPosition* pAttachmentPosition, const LTVector& vSeverDir )
{
	if( !pAttachmentPosition )
		return;

	HOBJECT			hParent = GetObject();
	HMODELSOCKET	hAttachSocket;

	// Make sure we can be shot off...

	if( AttachmentDB::Instance().GetAttachmentDetachWhenShot( m_hAttachmentRecord ))
	{
		// Get the world position of the socket this attachment is attached to...

		g_pModelLT->GetSocket( hParent, const_cast<char*>(pAttachmentPosition->GetName()), hAttachSocket );
		if( hAttachSocket != INVALID_MODEL_SOCKET )
		{
			LTTransform transAttach;
			g_pModelLT->GetSocketTransform( hParent, hAttachSocket, transAttach, true );


			HATTACHMENT hAttachment = NULL;
			g_pLTServer->FindAttachment(m_hObject, m_hModel, &hAttachment);
			if ( hAttachment != NULL )
			{
				g_pLTServer->RemoveAttachment(hAttachment);
			}

			Prop *pProp = dynamic_cast<Prop*>(g_pLTServer->HandleToObject( m_hModel ));
			if( pProp )
			{
				g_pLTServer->SetObjectPos( m_hModel, transAttach.m_vPos );
				g_pCommonLT->SetObjectFlags( m_hModel, OFT_Flags, FLAG_TOUCH_NOTIFY, FLAG_TOUCH_NOTIFY | FLAG_NOTINWORLDTREE );
				g_pLTServer->SetNetFlags( m_hModel, NETFLAG_POSUNGUARANTEED | NETFLAG_ROTUNGUARANTEED );

				// Let the prop get setup for being shot off...

				pProp->HandleAttachmentImpact( pAttachmentPosition, vSeverDir );

				// By setting the model handle to null the object will not get removed when the attachment is destroyed...
				m_hModel = NULL;
			}
		}
	}

	CAttachment::Destroy( this );

	pAttachmentPosition->SetAttachment( NULL );

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachmentProp::CreateSpawnString
//
//	PURPOSE:	Creates a spawn string for when we are dropped
//
// ----------------------------------------------------------------------- //

void CAttachmentProp::CreateSpawnString(char* szSpawn, uint32 nSpawnSize)
{
	char szModel[128] = "";
	char szMaterial[128] = "";
	AttachmentDB::Instance().GetAttachmentModel(m_hAttachmentRecord,szModel,sizeof(szModel));
	AttachmentDB::Instance().GetAttachmentMaterial(m_hAttachmentRecord, 0, szMaterial,sizeof(szMaterial));
	LTSNPrintF( szSpawn, nSpawnSize, "Prop Filename %s; Material %s; Gravity 0", szModel, szMaterial);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachmentProp::GetAttachmentCategory
//
//	PURPOSE:	Returns the handle to the category used by this prop type.
//
// ----------------------------------------------------------------------- //

HCATEGORY CAttachmentProp::GetAttachmentCategory()
{
	return AttachmentDB::Instance().GetPropAttachmentCategory();
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachmentObject::HandleProjectileImpact
//
//	PURPOSE:	Tests being hit by a projectile
//
// ----------------------------------------------------------------------- //

bool CAttachmentObject::HandleProjectileImpact(CProjectile* pProjectile, CAttachmentPosition* pAttachmentPosition, IntersectInfo& iInfo, 
											   const LTVector& vDir, const LTVector& vFrom, ModelsDB::HSKELETON hModelSkeleton, ModelsDB::HNODE hModelNode)
{
	if( !pAttachmentPosition )
		return false;

	HOBJECT			hParent = GetObject();
	HMODELSOCKET	hAttachSocket;

	// Make sure we can be shot off...

	if( !AttachmentDB::Instance().GetAttachmentDetachWhenShot( m_hAttachmentRecord ) && !IsAI( m_hModel ))
		return false;
	
	// Get the world position of the socket this attachment is attached to...

	g_pModelLT->GetSocket( hParent, const_cast<char*>(pAttachmentPosition->GetName()), hAttachSocket );
	if( hAttachSocket == INVALID_MODEL_SOCKET )
		return false;

	LTTransform transAttach;
	g_pModelLT->GetSocketTransform( hParent, hAttachSocket, transAttach, true );


	LTVector vModelDims;
	g_pPhysicsLT->GetObjectDims( m_hModel, &vModelDims );

	LTVector vMin = transAttach.m_vPos - vModelDims;
	LTVector vMax = transAttach.m_vPos + vModelDims;

	// See if this attachment got hit by the shot... 

	if( DoesSegmentIntersectAABB( iInfo.m_Point, iInfo.m_Point + vDir * 2000.0f, vMin, vMax ))
	{
		if( IsAI( m_hModel ))
		{
			CAI* pAI = (CAI*)g_pLTServer->HandleToObject(m_hModel);
			if ( pAI )
			{
				g_pLTServer->CPrint("dam: %f -> %f", g_pLTServer->GetTime(), pAI->GetDestructible()->GetHitPoints());
				iInfo.m_hObject = m_hModel;
				return true;
			}
		}
		else
		{
			HATTACHMENT hAttachment = NULL;
			g_pLTServer->FindAttachment(m_hObject, m_hModel, &hAttachment);
			if ( hAttachment != NULL )
			{
				g_pLTServer->RemoveAttachment(hAttachment);
			}

			CAttachment::Destroy( this );
			pAttachmentPosition->SetAttachment( NULL );
		}
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachmentObject::HandleSever
//
//	PURPOSE:	Handles having the limb we're attached to being severed
//
// ----------------------------------------------------------------------- //

void CAttachmentObject::HandleSever(CAttachmentPosition* pAttachmentPosition, const LTVector& vSeverDir )
{
	if( !pAttachmentPosition )
		return;

	//this shouldn't happen, but let's make sure...
	if (IsAI( m_hModel ))
		return;

	HATTACHMENT hAttachment = NULL;
	g_pLTServer->FindAttachment(m_hObject, m_hModel, &hAttachment);
	if ( hAttachment != NULL )
	{
		g_pLTServer->RemoveAttachment(hAttachment);
	}

	// If we can be shot off...
	if( AttachmentDB::Instance().GetAttachmentDetachWhenShot( m_hAttachmentRecord ))
	{
		// By setting the model handle to null the object will not get removed when the attachment is destroyed...
		m_hModel = NULL;
	}

	CAttachment::Destroy( this );
	pAttachmentPosition->SetAttachment( NULL );

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachmentObject::CreateSpawnString
//
//	PURPOSE:	Creates a spawn string for when we are dropped
//
// ----------------------------------------------------------------------- //

void CAttachmentObject::CreateSpawnString(char* szSpawn, uint32 nSpawnSize)
{
	char szModel[128] = "";
	char szMaterial[128] = "";
	AttachmentDB::Instance().GetAttachmentModel(m_hAttachmentRecord,szModel,sizeof(szModel));
	AttachmentDB::Instance().GetAttachmentMaterial(m_hAttachmentRecord, 0, szMaterial,sizeof(szMaterial));
	LTSNPrintF( szSpawn, nSpawnSize, "Prop Filename %s; Material %s; Gravity 0", szModel, szMaterial);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachmentObject::CreateSpawnString
//
//	PURPOSE:	Returns the handle to the category used by this Object type.
//
// ----------------------------------------------------------------------- //

HCATEGORY CAttachmentObject::GetAttachmentCategory()
{
	return AttachmentDB::Instance().GetObjectAttachmentCategory();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachmentPosition::CAttachmentPosition
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CAttachmentPosition::CAttachmentPosition() :
	m_pAttachment		( NULL )
{

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
	if ( m_pAttachment )
	{
		CAttachment::Destroy(m_pAttachment);
		m_pAttachment = NULL;
	}
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
	SAVE_STDSTRING( m_sName );
	SAVE_STDSTRING( m_sAttachmentName );
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
	bool bAttachment = false;

	LOAD_BOOL(bAttachment);

	if ( bAttachment )
	{
        uint32  dwAttachmentType;

        m_pAttachment = NULL;

		LOAD_DWORD(dwAttachmentType);

		m_pAttachment = CAttachment::Create(dwAttachmentType);

		LTASSERT(m_pAttachment, "TODO: Add description here");

		m_pAttachment->Load(pMsg);
	}

	LOAD_STDSTRING( m_sName );
	LOAD_STDSTRING( m_sAttachmentName );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAttachmentsPlugin::PreHook_EditStringList
//
//	PURPOSE:	Callback handler for EditStringList plugin
//
// ----------------------------------------------------------------------- //

LTRESULT CAttachmentsPlugin::PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	std::string sProperty( szPropName );

	if( sProperty.find( "Attachment" ) != sProperty.npos )
	{
		PopulateStringList(aszStrings, pcStrings, cMaxStrings, cMaxStringLength);
		return LT_OK;
	}
	else
	{
		return LT_UNSUPPORTED;
	}
}

void CAttachmentsPlugin::PopulateStringList(char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	// Add the <None> to the list

	LTStrCpy(aszStrings[(*pcStrings)++], c_szNoAttachment, cMaxStringLength);

	// Add the Object Attachments.

	HCATEGORY hObjectCategory = AttachmentDB::Instance().GetObjectAttachmentCategory();
	uint32 cObjectAttachments = g_pLTDatabase->GetNumRecords(hObjectCategory);
    for ( uint32 iAttachment = 0 ; iAttachment < cObjectAttachments ; iAttachment++ )
	{
		char szBuffer[128];
		szBuffer[0] = '\0';
		
		HRECORD hAttachmentRecord = g_pLTDatabase->GetRecordByIndex(hObjectCategory, iAttachment);
		AttachmentDB::Instance().GetAttachmentName(hAttachmentRecord, szBuffer, LTARRAYSIZE(szBuffer));
		if(szBuffer[0])
		{
			LTStrCpy(aszStrings[(*pcStrings)++], szBuffer, cMaxStringLength);
		}
	}

	// Add the Prop Attachments.
	
	HCATEGORY hPropCategory = AttachmentDB::Instance().GetPropAttachmentCategory();
	uint32 cPropAttachments = g_pLTDatabase->GetNumRecords(hPropCategory);
    for ( uint32 iAttachment = 0 ; iAttachment < cPropAttachments ; iAttachment++ )
	{
		char szBuffer[128];
		szBuffer[0] = '\0';
		
		HRECORD hAttachmentRecord = g_pLTDatabase->GetRecordByIndex(hPropCategory, iAttachment);
		AttachmentDB::Instance().GetAttachmentName(hAttachmentRecord, szBuffer, LTARRAYSIZE(szBuffer));
		if(szBuffer[0])
		{
			LTStrCpy(aszStrings[(*pcStrings)++], szBuffer, cMaxStringLength);
		}
	}

	// Sort the attachments alphabetically.

	qsort( aszStrings, *pcStrings, sizeof( char * ), CaseInsensitiveCompare );
}
