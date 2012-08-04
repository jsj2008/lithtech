// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __ATTACHBUTE_MGR_H__
#define __ATTACHBUTE_MGR_H__

#include "GameButeMgr.h"
#include "ltbasetypes.h"
#include "ButeListReader.h"

class CAttachButeMgr;
extern CAttachButeMgr* g_pAttachButeMgr;

class CAttachButeMgr : public CGameButeMgr
{
	public : // Public member variables

		CAttachButeMgr();
		~CAttachButeMgr();

        LTBOOL       Init(const char* szAttributeFile = "Attributes\\Attachments.txt");
		void		Term();

		// Attachments

		int			GetAttachmentIDByName(const char *szName);
		int			GetNumAttachments() { return m_cAttachmentID; }
		void		GetAttachmentName(int nAttachmentID, char *pBuf, uint32 nBufLen );
		int			GetAttachmentType(int nAttachmentID);
		void		GetAttachmentProperties(int nAttachmentID, char *pBuf, uint32 nBufLen);
		int			GetAttachmentWeapon(int nAttachmentID);
		void		GetAttachmentClass(int nAttachmentID, char *pBuf, uint32 nBufLen);
		void		GetAttachmentModel(int nAttachmentID, char *pBuf, uint32 nBufLen);
		void		GetAttachmentSkin(int nAttachmentID, char *pBuf, uint32 nBufLen);
		int			GetAttachmentDeath(int nAttachmentID);
		int			GetAttachmentShot(int nAttachmentID);
		bool		GetAttachmentDeleteOnDeath(int nAttachmentID);
		bool		GetAttachmentTranslcuent(int nAttachmentID);
		bool		GetAttachmentDetachWhenShot( int nAttachmentID );
		void		GetAttachmentDebrisType( int nAttachmentID, char *pBuf, uint32 nBufLen );

		void		GetAttachmentSkins(int nAttachmentID, CButeListReader* pSkinReader);
		void		CopyAttachmentSkins(int nAttachmentID, char* paszDest, int strLen);

		void		GetAttachmentRenderStyles(int nAttachmentID, CButeListReader* pRenderStyleReader);
		void		CopyAttachmentRenderStyles(int nAttachmentID, char* paszDest, int strLen);

		int			GetNumRequirements( const char *szModelName,char *szStyleName );
		int			GetRequirementIDs( const char *szModelName,int *pBuf,int nBufLen );
		int			GetRequirementAttachment(int nRequirementID);
		void		GetRequirementSocket(int nRequirementID, char *pBuf, uint32 nBufLen);

		int			GetPVAttachmentIDByName( const char *szName );
		int			GetNumPVAttachments() { return m_cPVAttachmentID; }
		void		GetPVAttachmentName( int nPVAttachmentID, char *pBuf, uint32 nBufLen );
		void		GetPVAttachmentModel( int nPVAttachmentID, char *pBuf, uint32 nBufLen );
		void		GetPVAttachmentSkins( int nPVAttachmentID, CButeListReader* pSkinReader );
		void		CopyPVAttachmentSkins( int nPVAttachmentID, char* paszDest, int strLen );
		void		GetPVAttachmentRenderStyles( int nPVAttachmentID, CButeListReader* pRenderStyleReader );
		void		CopyPVAttachmentRenderStyles( int nPVAttachmentID, char* paszDest, int strLen );
		bool		GetPVAttachmentTranslcuent( int nPVAttachmentID );
		LTVector	GetPVAttachmentScale( int nPVAttachmentID );

	private :

		int			m_cAttachmentID;
		int			m_cRequirementID;
		int			m_cPVAttachmentID;

		CButeListReader	m_blrAttachSkinReader;
		CButeListReader	m_blrAttachRenderStyleReader;
};

#endif // __AttachBUTE_MGR_H__