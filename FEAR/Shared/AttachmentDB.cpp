// ----------------------------------------------------------------------- //
//
// MODULE  : AttachmentDB.cpp
//
// PURPOSE : Attachment Database implementation
//
// CREATED : 3/09/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //
//
//
// As PropAttachments and ObjectAttachments share many traits (and are treated 
// abstractly by part of the Attachment system, the data was structured in a 
// similar way.  The Prop and Object Attachments derive from a BaseAttachment 
// category.  This allows them to share attributes while being presented in the 
// editor as different types.
//
// the Player View Attachments (PVAttachment) is ouside of this hierarchy, and 
// is likely to go away soon.
//
// BaseAttachment			- Base class for shared attributes.
//		PropAttachment		- Prop specific attributes
//		ObjectAttachment	- Object specific attributes
//
// PlayerViewAttachment		- Temp Conversion as the PlayerView system is going away.  
//							Once it is removed, everything related to PVAttachment
//							can be deleted.
//
//

#include "Stdafx.h"
#include "AttachmentDB.h"

namespace
{

// Base Attachment Category
const char* const ADB_Attachment_SocketName			= "Socket";
const char* const ADB_Attachment_Translucent		= "Translucent";
const char* const ADB_Attachment_DeleteOnDeath		= "DeleteOnDeath";
const char* const ADB_Attachment_DetachWhenShot		= "DetachWhenShot";
const char* const ADB_Attachment_Properties			= "Properties";

// Prop Attachment Category
const char* const ADB_PropAttachmentCat				= "Attachment/PropAttachment";
const char* const ADB_Attachment_PropModel			= "Model";
const char* const ADB_Attachment_PropMaterial		= "Materials";

// Object Attachment Category
const char* const ADB_ObjectAttachmentCat			= "Attachment/ObjectAttachment";
const char* const ADB_Attachment_ObjectClass		= "Class";

// This is a temp category, going away when the player view system is removed.
//
// Player View Attachment Category
const char* const ADB_PVAttachmentCat				= "Attachment/PVAttachment";
const char* const ADB_Attachment_PVModel			= "PVModel";
const char* const ADB_Attachment_PVMaterial			= "Materials";
const char* const ADB_Attachment_PVScale			= "Scale";
const char* const ADB_Attachment_PVTranslucent		= "Translucent";
//

};

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AttachmentDB::AttachmentDB()
//
//	PURPOSE:	Constructor/Destructor to handle safely creating and 
//				destroying the Attachment Database
//
// ----------------------------------------------------------------------- //

AttachmentDB::AttachmentDB()
	:	m_hObjectAttachmentCat(NULL)
	, m_hPropAttachmentCat(NULL)
	, m_hPVAttachmentCat(NULL)
{
}

AttachmentDB::~AttachmentDB()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AttachmentDB::Init()
//
//	PURPOSE:	Initialize the Attachment Database
//
// ----------------------------------------------------------------------- //

bool AttachmentDB::Init( const char *szDatabaseFile )
{
	if( !OpenDatabase( szDatabaseFile ))
		return false;

	m_hObjectAttachmentCat = g_pLTDatabase->GetCategory(m_hDatabase, ADB_ObjectAttachmentCat);
	LTASSERT_PARAM1(NULL != m_hObjectAttachmentCat, "AttachmentDB::Init : Category %s not found.", ADB_ObjectAttachmentCat);

	m_hPropAttachmentCat = g_pLTDatabase->GetCategory(m_hDatabase, ADB_PropAttachmentCat);
	LTASSERT_PARAM1(NULL != m_hPropAttachmentCat, "AttachmentDB::Init : Category %s not found.", m_hPropAttachmentCat);

	m_hPVAttachmentCat = g_pLTDatabase->GetCategory(m_hDatabase, ADB_PVAttachmentCat);
	LTASSERT_PARAM1(NULL != m_hPVAttachmentCat, "AttachmentDB::Init : Category %s not found.", m_hPVAttachmentCat);

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AttachmentDB::Term()
//
//	PURPOSE:	Clean up the Attachment Database
//
// ----------------------------------------------------------------------- //

void AttachmentDB::Term()
{
}

// Attachment access:

HRECORD AttachmentDB::GetPropAttachmentRecordByName(const char *szName) const
{
	return CGameDatabaseReader::GetRecord(m_hPropAttachmentCat, szName);
}

HRECORD AttachmentDB::GetObjectAttachmentRecordByName(const char *szName) const
{
	return CGameDatabaseReader::GetRecord(m_hObjectAttachmentCat, szName);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AttachmentDB::GetAttachmentCategory()
//
//	PURPOSE:	Returns a handle to the Object Attachment category
//
// ----------------------------------------------------------------------- //

HCATEGORY AttachmentDB::GetObjectAttachmentCategory() const
{
	return m_hObjectAttachmentCat;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AttachmentDB::GetAttachmentCategory()
//
//	PURPOSE:	Returns a handle to the Prop Attachment category
//
// ----------------------------------------------------------------------- //

HCATEGORY AttachmentDB::GetPropAttachmentCategory() const
{
	return m_hPropAttachmentCat;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AttachmentDB::GetAttachentSocket()
//
//	PURPOSE:	Returns the name of the socket this attachment attaches 
//				to.  Asserts and returns an empty string ("") if there 
//				is no socket specified.
//
// ----------------------------------------------------------------------- //

const char* AttachmentDB::GetAttachentSocket(HRECORD hAttachment) const
{
	const char* pszSocketName = GetString(hAttachment, ADB_Attachment_SocketName);

	LTASSERT_PARAM1(0 != LTStrCmp(pszSocketName, ""),
		"AttachmentDB::GetAttachentSocket : No Socket named for Attachment %s", GetRecordName(hAttachment));

	return pszSocketName;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AttachmentDB::GetAttachmentName()
//
//	PURPOSE:	Returns by parameter the name of the record, or an empty 
//				string ("") if there is no such name.
//
// ----------------------------------------------------------------------- //

void AttachmentDB::GetAttachmentName(HRECORD hAttachment, char *pBuf, uint32 nBufLen) const
{
	LTStrCpy(pBuf, GetRecordName(hAttachment), nBufLen);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AttachmentDB::GetAttachmentTranslcuent()
//
//	PURPOSE:	Returns by true if the attachment is translucent, false 
//				if it is not
//
// ----------------------------------------------------------------------- //

bool AttachmentDB::GetAttachmentTranslcuent(HRECORD hAttachment) const
{
	return GetBool(hAttachment, ADB_Attachment_Translucent);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AttachmentDB::GetAttachmentProperties()
//
//	PURPOSE:	Returns by parameter the properties string.
//
// ----------------------------------------------------------------------- //

void AttachmentDB::GetAttachmentProperties(HRECORD hAttachment, char *pBuf, uint32 nBufLen) const
{
	LTStrCpy(pBuf, GetString(hAttachment, ADB_Attachment_Properties), nBufLen);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AttachmentDB::GetAttachmentDeleteOnDeath()
//
//	PURPOSE:	Returns by true if the attachment is should be deleted on 
//				the owners death, false it is should not (and should 
//				instead be attached to the body)
//
// ----------------------------------------------------------------------- //

bool AttachmentDB::GetAttachmentDeleteOnDeath(HRECORD hAttachment) const
{
	return GetBool(hAttachment, ADB_Attachment_DeleteOnDeath);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AttachmentDB::GetAttachmentDetachWhenShot()
//
//	PURPOSE:	Returns by true if the attachment is should be detached 
//				when shot, false if it should not.
//
// ----------------------------------------------------------------------- //

bool AttachmentDB::GetAttachmentDetachWhenShot(HRECORD hAttachment ) const
{
	return GetBool(hAttachment, ADB_Attachment_DetachWhenShot);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AttachmentDB::GetAttachmentClass()
//
//	PURPOSE:	Returns by parameter the class specified in the passed in 
//				record; returns an empty string ("") and asserts if there 
//				is no such data.
//
// ----------------------------------------------------------------------- //

void AttachmentDB::GetAttachmentClass(HRECORD hAttachment, char *pBuf, uint32 nBufLen) const
{
	LTStrCpy(pBuf, GetString(hAttachment, ADB_Attachment_ObjectClass), nBufLen);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AttachmentDB::GetAttachmentModel()
//
//	PURPOSE:	Returns by parameter the Model specified in the passed in 
//				record; returns an empty string ("") and asserts if there 
//				is no such data.
//
// ----------------------------------------------------------------------- //

void AttachmentDB::GetAttachmentModel(HRECORD hAttachment, char *pBuf, uint32 nBufLen) const
{
	LTStrCpy(pBuf, GetString(hAttachment, ADB_Attachment_PropModel), nBufLen);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AttachmentDB::GetAttachmentMaterialCount()
//
//	PURPOSE:	Returns the number of materials used by this attachment.  
//				Asserts and returns 0 if this field is not present.
//
// ----------------------------------------------------------------------- //

int	AttachmentDB::GetAttachmentMaterialCount(HRECORD hAttachment) const
{
	return GetNumValues(hAttachment, ADB_Attachment_PropMaterial);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AttachmentDB::GetAttachmentMaterial()
//
//	PURPOSE:	Returns by parameter the Materials specified in the passed in 
//				record; returns an empty string ("") and asserts if there 
//				is no such data.
//
// ----------------------------------------------------------------------- //

void AttachmentDB::GetAttachmentMaterial(HRECORD hAttachment, int nMaterialIndex, char *pBuf, uint32 nBufLen) const
{
	LTStrCpy(pBuf, GetString(hAttachment, ADB_Attachment_PropMaterial, nMaterialIndex), nBufLen);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AttachmentDB::GetPVAttachmentCategory()
//
//	PURPOSE:	Returns a handle to the PVAttachment  category
//
// ----------------------------------------------------------------------- //

HCATEGORY AttachmentDB::GetPVAttachmentCategory() const
{
	return m_hPVAttachmentCat;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AttachmentDB::GetPVAttachmentRecordByName()
//
//	PURPOSE:	Returns a handle to the player view attachment record with 
//				the passed in name.  Returns NULL if no such record exists.
//
// ----------------------------------------------------------------------- //

HRECORD AttachmentDB::GetPVAttachmentRecordByName(const char* szName) const
{
	return CGameDatabaseReader::GetRecord(m_hPVAttachmentCat, szName);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AttachmentDB::GetPVAttachmentModel()
//
//	PURPOSE:	Returns by parameter the name of the PVAttachment model.
//
// ----------------------------------------------------------------------- //

void AttachmentDB::GetPVAttachmentModel(HRECORD hPVAttachment, char *pBuf, uint32 nBufLen) const
{
	LTStrCpy(pBuf, GetString(hPVAttachment, ADB_Attachment_PVModel), nBufLen);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AttachmentDB::GetPVAttachmentMaterialCount()
//
//	PURPOSE:	Returns the number of materials on the passed in player view attachment.
//
// ----------------------------------------------------------------------- //

int	AttachmentDB::GetPVAttachmentMaterialCount(HRECORD hPVAttachment) const
{
	return GetNumValues(hPVAttachment, ADB_Attachment_PVMaterial);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AttachmentDB::GetPVAttachmentMaterial()
//
//	PURPOSE:	Returns by parameter the material, at the passed in index, 
//				for the player view record passed in.
//
// ----------------------------------------------------------------------- //

void AttachmentDB::GetPVAttachmentMaterial(HRECORD hPVAttachment, int nMaterialIndex, char *pBuf, uint32 nBufLen) const
{
	LTStrCpy(pBuf, GetString(hPVAttachment, ADB_Attachment_PVMaterial, nMaterialIndex), nBufLen);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AttachmentDB::GetPVAttachmentScale()
//
//	PURPOSE:	Returns the scale factor for the passed in player view 
//				attachment.
//
// ----------------------------------------------------------------------- //

float AttachmentDB::GetPVAttachmentScale(HRECORD hPVAttachment) const
{
	return GetFloat(hPVAttachment, ADB_Attachment_PVScale);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AttachmentDB::GetPVAttachmentTranslcuent()
//
//	PURPOSE:	Returns true if the passed player view attachment is 
//				translucent, false if it is not.
//
// ----------------------------------------------------------------------- //

bool AttachmentDB::GetPVAttachmentTranslcuent(HRECORD hPVAttachment) const
{
	return GetBool(hPVAttachment, ADB_Attachment_PVTranslucent);
}

