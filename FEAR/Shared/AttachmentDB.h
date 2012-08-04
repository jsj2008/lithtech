// ----------------------------------------------------------------------- //
//
// MODULE  : AttachmentDB.h
//
// PURPOSE : Attachment Database Declaration
//
// CREATED : 3/09/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __ATTACHMENTDB_H_
#define __ATTACHMENTDB_H_

// ----------------------------------------------------------------------- //
//
//	CLASS:		AttachmentDB
//
//	PURPOSE:	Defines the interface for accessing attachment related 
//				information.
//
// ----------------------------------------------------------------------- //

class AttachmentDB : public CGameDatabaseMgr
{
	DECLARE_SINGLETON( AttachmentDB );
	
public:

	bool		Init( const char *szDatabaseFile = DB_Default_File );
	void		Term();

	// Attachment access:

	HRECORD		GetObjectAttachmentRecordByName(const char *szName) const;
	HRECORD		GetPropAttachmentRecordByName(const char *szName) const;

	HCATEGORY	GetObjectAttachmentCategory() const;
	HCATEGORY	GetPropAttachmentCategory() const;

	// Base Attachment Queries

	const char* GetAttachentSocket(HRECORD hAttachment) const;
	void		GetAttachmentName(HRECORD hAttachment, char *pBuf, uint32 nBufLen) const;
	bool		GetAttachmentTranslcuent(HRECORD hAttachment) const;
	void		GetAttachmentProperties(HRECORD hAttachment, char *pBuf, uint32 nBufLen) const;
	bool		GetAttachmentDeleteOnDeath(HRECORD hAttachment) const;
	bool		GetAttachmentDetachWhenShot(HRECORD hAttachment ) const;

	// Object Attachment Queries

	void		GetAttachmentClass(HRECORD hAttachment, char *pBuf, uint32 nBufLen) const;

	// Prop Attachment Queries
	
	void		GetAttachmentModel(HRECORD hAttachment, char *pBuf, uint32 nBufLen) const;
	int			GetAttachmentMaterialCount(HRECORD hAttachment) const;
	void		GetAttachmentMaterial(HRECORD hAttachment, int nMaterialIndex, char *pBuf, uint32 nBufLen) const;

	//--------------------------------------------------------------------------
	// This is a temp category, going away when the player view system is removed.
	//
	// PVAttachment access:
	HCATEGORY	GetPVAttachmentCategory() const;
	HRECORD		GetPVAttachmentRecordByName(const char* szName) const;

	void		GetPVAttachmentModel(HRECORD hAttachment, char *pBuf, uint32 nBufLen) const;
	int			GetPVAttachmentMaterialCount(HRECORD hAttachment) const;
	void		GetPVAttachmentMaterial(HRECORD hAttachment, int nMaterialIndex, char *pBuf, uint32 nBufLen) const;
	float		GetPVAttachmentScale(HRECORD hPVAttachment) const;
	bool		GetPVAttachmentTranslcuent(HRECORD hAttachment) const;
	//--------------------------------------------------------------------------

private:
	HCATEGORY	m_hObjectAttachmentCat;
	HCATEGORY	m_hPropAttachmentCat;
	HCATEGORY	m_hPVAttachmentCat;
};



#endif // __ATTACHMENTDB_H_

