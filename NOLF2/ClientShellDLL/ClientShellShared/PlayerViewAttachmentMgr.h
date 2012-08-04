// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerViewAttachmentMgr.h
//
// PURPOSE : Manager of objects attached to player-view models...
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PLAYER_VIEW_ATTACHMENT_MGR_H__
#define __PLAYER_VIEW_ATTACHMENT_MGR_H__

//
// Globals...
//

	extern CPlayerViewAttachmentMgr *g_pPVAttachmentMgr;



class CPlayerViewAttachmentMgr
{
	public: // Methods...

		CPlayerViewAttachmentMgr();
		~CPlayerViewAttachmentMgr();

		bool		Init();
		void		Term();

		void		CreatePVAttachments( HOBJECT hPVObject );
		void		UpdatePVAttachments();
		void		RemovePVAttachments();		
		void		ShowPVAttachments( bool bVisible = true );
		void		ShowPVAttachment( uint8 nPVAttachIndex, bool bVisible = true );


	protected: // Methods...

		void		CreatePVAttachment( const char *pszPVAttachmentPosition, const char *pszPVAttachment );
		void		UpdatePVAttachment( uint8 nPVAttachIndex );
		void		RemovePVAttachment( uint8 nPVAttachIndex );
		

	protected: // Members...

		class CPlayerViewAttachment
		{
			public:

				CPlayerViewAttachment()
				:	m_hModel	( LTNULL ),
					m_hSocket	( INVALID_MODEL_SOCKET ),
					m_bHidden	( false )
				{
				}

				LTObjRef		m_hModel;
				HMODELSOCKET	m_hSocket;
				bool			m_bHidden;
		};
		typedef std::vector<CPlayerViewAttachment> PVAttachmentList;
	
		PVAttachmentList	m_lstPVAttachments;		// List of all current player-view attachments

		HOBJECT				m_hObject;				// Object that the attachments are attached to
};

#endif // __PLAYER_VIEW_ATTACHMENT_MGR_H__