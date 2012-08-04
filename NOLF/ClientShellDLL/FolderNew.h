// FolderNew.h: interface for the CFolderNew class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FOLDERNEW_H__B9A9F281_225A_11D3_B2DB_006097097C7B__INCLUDED_)
#define AFX_FOLDERNEW_H__B9A9F281_225A_11D3_B2DB_006097097C7B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseSelectionFolder.h"

class CFolderNew : public CBaseSelectionFolder
{
public:
	CFolderNew();
	virtual ~CFolderNew();

	// Build the folder
    LTBOOL   Build();
	void	Term();

	// This is called when the folder gets or loses focus
    virtual void    OnFocus(LTBOOL bFocus);


	//this function creates a string, the caller must free it
    virtual HSTRING GetHelpString(uint32 dwHelpId, int nControlIndex);

	virtual	LTBOOL	UpdateSelection();

protected:
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
	void	BuildMissionList();
	void	ClearMissionList();

};

#endif // !defined(AFX_FOLDERNEW_H__B9A9F281_225A_11D3_B2DB_006097097C7B__INCLUDED_)