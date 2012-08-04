// BitmapCtrl.h: interface for the CBitmapCtrl class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _BITMAPCTRL_H_
#define _BITMAPCTRL_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LTGUIMgr.h"
#include "stdlith.h"

class CBitmapCtrl : public CLTGUICtrl
{
public:
	CBitmapCtrl();
	virtual ~CBitmapCtrl();
    LTBOOL   Create(ILTClient *pClientDE, char *lpszNormalBmp,char *lpszSelectedBmp = NULL,char *lpszDisabledBmp = NULL,
                        CLTGUICommandHandler *pCommandHandler = LTNULL, uint32 dwCommandID = LTNULL, uint32 dwParam1 = 0, uint32 dwParam2 = 0);


	// Destruction
	virtual void	Destroy();

	// Render the control
	virtual void	Render ( HSURFACE hDestSurf );

	// Width/Height calculations
	virtual int		GetWidth ( );
	virtual int		GetHeight ( );

    virtual LTBOOL   OnEnter();
    virtual LTBOOL   OnLButtonUp (int x, int y) {return OnEnter();}
	//determines whether the surfaces should be freed when changing bitmaps and on destruction
    void            AutoFreeSurfaces(LTBOOL bFreeSurf) {m_bFreeSurfaces = bFreeSurf;}


    virtual LTBOOL   SetBitmap(char *lpszNormalBmp,char *lpszSelectedBmp = NULL,char *lpszDisabledBmp = NULL);
	virtual void	FreeSurfaces();

	void			GetNormalBitmap(char* pBuf, int nBufLen);
	void			GetSelectedBitmap(char* pBuf, int nBufLen);
	void			GetDisabledBitmap(char* pBuf, int nBufLen);

    void            SetTransparentColor(HLTCOLOR hTrans) {m_hTrans = hTrans;}


protected:
	char			m_sNormalSurface[128];
	char			m_sSelectedSurface[128];
	char			m_sDisabledSurface[128];

	int				m_nNormalWidth;
	int				m_nNormalHeight;
	int				m_nSelectedWidth;
	int				m_nSelectedHeight;
	int				m_nDisabledWidth;
	int				m_nDisabledHeight;

    LTBOOL           m_bFreeSurfaces;

    HLTCOLOR        m_hTrans;

	// Receives a message when the "enter" key is pressed.
	CLTGUICommandHandler	*m_pCommandHandler;

};

#endif // !defined(AFX_BITMAPCTRL_H__639D44A0_1A85_11D3_B2DB_006097097C7B__INCLUDED_)