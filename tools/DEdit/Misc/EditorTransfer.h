//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
//------------------------------------------------------------------
//
//	FILE	  : EditorTransfer.h
//
//	PURPOSE	  : Defines the CEditorTransfer class .. any data used in
//              drag n drop operations for DirectEditor goes in here.
//
//	CREATED	  : December 19 1996
//
//
//------------------------------------------------------------------

#ifndef __EDITOR_TRANSFER_H__
	#define __EDITOR_TRANSFER_H__


	// Global stuff.
	extern UINT		g_EditorTransferFormat;
	void			RegisterEditorTransferFormat();



	// All the DirectEditor transfer types.
	#define DT_TEXTUREDRAG		0



	#define TRANSFERNAME_LEN	200


	class CEditorTransfer
	{
		public:

			DWORD		m_TransferType;
			char		m_StrData[TRANSFERNAME_LEN];

	};


#endif  // __EDITOR_TRANSFER_H__


