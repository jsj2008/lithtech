#ifndef __MESSAGEBOX_H
#define __MESSAGEBOX_H

#include "clientheaders.h"
#include "ClientUtilities.h"
#include "TextHelper.h"

class ILTClient;

class CMessageBox
{
public:

	CMessageBox()		{ m_pClientDE = LTNULL; m_hMessageBox = LTNULL; m_szMessageBox.cx = m_szMessageBox.cy = 0; m_bYesNo = LTFALSE; }
	~CMessageBox();

	LTBOOL			Init (ILTClient* pClientDE, int nStringID, LTBOOL bYesNo = LTFALSE, int nAlignment = TH_ALIGN_LEFT, LTBOOL bCrop = LTTRUE);

	LTBOOL			IsYesNo()		{ return m_bYesNo; }

	void			Draw();

protected:

	ILTClient*		m_pClientDE;
	HSURFACE		m_hMessageBox;
	CSize			m_szMessageBox;
	LTBOOL			m_bYesNo;
};

#endif
