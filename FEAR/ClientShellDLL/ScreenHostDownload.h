// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenHostDownload.h
//
// PURPOSE : Interface screen to set server download options
//
// CREATED : 11/18/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SCREENHOSTDOWNLOAD_H__
#define __SCREENHOSTDOWNLOAD_H__

class LabeledEditCtrl;

class CScreenHostDownload : public CBaseScreen
{
public:
	CScreenHostDownload();
	virtual ~CScreenHostDownload();

	// Build the screen
	virtual bool	Build();

	virtual void    OnFocus(bool bFocus);


protected:
	uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
	static void RatePerClientValueChangingCB( std::wstring& wsValue, void* pUserData );
	static void RateOverallValueChangingCB( std::wstring& wsValue, void* pUserData );


	bool	m_bAllowContentDownload;
	std::wstring m_sMaxRatePerClient;
	std::wstring m_sMaxRateOverall;
	int		m_nMaxNumClients;
	uint8	m_nMaxDownload;

	CLTGUIToggle*		m_pAllowDownload;
	LabeledEditCtrl*	m_pMaxRatePerClient;
	LabeledEditCtrl*	m_pMaxRateOverall;
	CLTGUISlider*		m_pMaxNumClients;
	CLTGUICycleCtrl*	m_pMaxDownload;

	CLTGUITextCtrl*		m_pRatePerClient;
	CLTGUITextCtrl*		m_pRateOverall;

};



#endif  // __SCREENHOSTDOWNLOAD_H__
