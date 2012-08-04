// ----------------------------------------------------------------------- //
//
// MODULE  : LTGUIToggle.h
//
// PURPOSE : Text control which toggles between two values.
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#if !defined(_LTGUITOGGLE_H_)
#define _LTGUITOGGLE_H_


#include "ltguicyclectrl.h"

struct CLTGUIToggle_create : public CLTGUICycleCtrl_create
{
	CLTGUIToggle_create();
	bool *pbValue;
};

inline CLTGUIToggle_create::CLTGUIToggle_create() : 
	pbValue(NULL)
{
};


class CLTGUIToggle : public CLTGUICycleCtrl
{
public:
	CLTGUIToggle();
	virtual ~CLTGUIToggle();

	// Create the control
    bool           Create ( const wchar_t *pText, const CFontInfo& Font, const CLTGUIToggle_create& cs );

	void			SetOnString(const wchar_t *pStrOn);
	void			SetOffString(const wchar_t *pStrOff);

	// Update data
    void            UpdateData(bool bSaveAndValidate=true);

	// Sets/gets the on/off status
    bool           IsOn()              { return m_bOn; }
    virtual void    SetOn(bool bOn);

	// Left and Right key presses
    virtual bool   OnLeft()            { SetOn(!IsOn()); return true;}
    virtual bool   OnRight()           { SetOn(!IsOn()); return true;}

protected:
    bool           m_bOn;
    bool           *m_pbValue;     // Value to store the on/off status in when UpdateData is called.

};

#endif // !defined(_LTGUITOGGLE_H_)