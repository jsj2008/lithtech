//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
//------------------------------------------------------------------
//
//	FILE	  : ProjectDirDlg.h
//
//	PURPOSE	  : Overrides some CFileDialog stuff for the project directory selector.
//
//	CREATED	  : December 14 1996
//
//
//------------------------------------------------------------------

#ifndef __PROJECTDIRDLG_H__
	#define __PROJECTDIRDLG_H__


	class CProjectDirDlg : public CFileDialog
	{
		public:

						CProjectDirDlg( CWnd *pParent=NULL );

			int			DoModal();

	};


#endif  // __PROJECTDIRDLG_H__

