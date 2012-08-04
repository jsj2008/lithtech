// ----------------------------------------------------------------------- //
//
// MODULE  : ObjectiveControl.h
//
// PURPOSE : GUI control to display of objectives.
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#if !defined(_OBJ_CTRL_H_)
#define _OBJ_CTRL_H_

#include "LTGuiMgr.h"

class CObjectiveCtrl : public CLTGUITextCtrl
{
public:

	CObjectiveCtrl()
	{
		m_BulletTexture = LTNULL;
	}

    LTBOOL			Create (const char *pText, 	CUIFont *pFont, uint8 nFontSize, uint16 nTextOffset, HTEXTURE hTex);

    virtual void    SetBasePos ( LTIntPt pos );
	virtual void	SetScale(float fScale);
	virtual void	SetString(const char *pText);
	virtual void	SetTexture(HTEXTURE hTex);


	// Render the control
	virtual void	Render ();


protected:
	void			SetRenderState();
	void			InitPoly();
	void			ScalePoly();

protected:

	LTPoly_GT4	m_Poly;

	HTEXTURE	m_BulletTexture;
};

#endif // _OBJ_CTRL_H_