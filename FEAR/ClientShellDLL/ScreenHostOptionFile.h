// ScreenHostMission.h: interface for the ScreenHostMission class.
//
//////////////////////////////////////////////////////////////////////

#ifndef SCREEN_HOST_OPTIONFILE_H
#define SCREEN_HOST_OPTIONFILE_H


#include "BaseScreen.h"
#include "ProfileMgr.h"


#pragma warning (disable : 4503)
#pragma warning( disable : 4786 )
#include <vector>
#include <string>


class CScreenHostOptionFile : public CBaseScreen
{
public:
	
	CScreenHostOptionFile();
	virtual ~CScreenHostOptionFile();

	// Build the screen
    bool   Build();

    virtual uint32	OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
	void			Escape();
	void			OnFocus(bool bFocus);
	bool			OnRButtonUp(int x, int y);
	bool			OnLButtonUp(int x, int y);

private:
    void	HandleDlgCommand(uint32 dwCommand, uint16 nIndex);
	void	CreateFileList();
	void	UpdateName();
	void	Delete(const wchar_t* pwsFile);
	void	New(const wchar_t* pwsName);
	void	Rename(const wchar_t* oldName,const wchar_t* newName);
	void	SetName(const wchar_t* newName);
	
	CLTGUITextCtrl	*m_pCurrent;
	CLTGUITextCtrl	*m_pLoad;
	CLTGUITextCtrl	*m_pDelete;
	CLTGUITextCtrl	*m_pRename;


	WStringSet			m_List;
	CLTGUIListCtrl*		m_pListCtrl;
	CLTGUIWindow*		m_pDlg;
};

#endif // SCREEN_HOST_OPTIONFILE_H