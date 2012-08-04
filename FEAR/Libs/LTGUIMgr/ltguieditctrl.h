// ----------------------------------------------------------------------- //
//
// MODULE  : LTGUIListCtrl.h
//
// PURPOSE : Control to handle text entry
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#if !defined(_LTGUIEDITCTRL_H_)
#define _LTGUIEDITCTRL_H_

#include "ltguictrl.h"

// Set the character input mode
enum LTGUIInputMode
{
	kInputAll,
	kInputAlphaNumeric,
	kInputAlphaOnly,
	kInputNumberOnly,
	kInputFileFriendly
};

struct CLTGUIEditCtrl_create : public CLTGUICtrl_create
{
	CLTGUIEditCtrl_create();
	uint32 nMaxLength;
	wchar_t *pszValue;
	bool bUseCaret; 
	float fCaretTime;
	uint32 argbCaretColor;
	bool	bUseFrame;
	LTGUIInputMode eMode;
	bool	bPreventEmptyString; // if true, pressing Enter when the text field is empty will do nothing.
};

inline CLTGUIEditCtrl_create::CLTGUIEditCtrl_create() : 
		nMaxLength(0), pszValue(NULL), fCaretTime(0.5f), argbCaretColor(0xFFFFFFFF), bPreventEmptyString(false)
{
};


// FilterCallback functions are used to perform custom input filtering
// they take an incoming character and return a processed character
// return null to indicate the char should be ingnored.
typedef wchar_t (*FilterCallbackFn)(wchar_t c, uint32 nPos);


class CLTGUIEditCtrl : public CLTGUICtrl
{
public:
	CLTGUIEditCtrl();
	virtual ~CLTGUIEditCtrl();

	// Create the control
    virtual bool   Create ( ILTClient *pLTClient, const CFontInfo& Font, const CLTGUIEditCtrl_create& cs);

	// Destroys the control
	void			Destroy ( );

	// Update data
    void            UpdateData(bool bSaveAndValidate=true);

	// Render the control
	virtual void Render();
	virtual void RenderTransition(float fTrans) {Render();}

	// Set the font
	bool			SetFont ( const CFontInfo& Font );

	// Set/Get the text for the edit string
	void			SetText(const wchar_t *pString);
	virtual const wchar_t* GetText() const;
	bool			IsEmpty() const { return m_Text.IsEmpty(); }

	uint32	GetMaxLength() {return m_nMaxLength;}
	void	SetMaxLength(uint32 nMaxLength);

	virtual	void	SetBasePos( const LTVector2n& pos );
	virtual void	SetScale( const LTVector2& vfScale );
	virtual void	SetSize( const LTVector2n& sz );

	// Handles a key press
    bool           HandleKeyDown(int key, int rep);
    bool           HandleChar(wchar_t c);

	// Handle the Enter key being pressed
    virtual bool   OnEnter ( );
    virtual bool   OnLButtonUp(int x, int y);

	// Set the caret usage (default is no caret)
    void            EnableCaret(bool bUseCaret, float fToggleTime, uint32 argbColor);

	static const uint32	kMaxLength;

	void			SetInputMode(LTGUIInputMode mode)	{ m_eInputMode = mode;}

	// free texture memory by flushing any texture strings owned by the control
	virtual void	FlushTextureStrings();

	// rebuild any texture strings owned by the control
	virtual void	RecreateTextureStrings();

	//the input filter is processed before the inputmode filter
	void			SetInputFilter(FilterCallbackFn pFn) {m_pFn = pFn;}


	void			SetPreventEmptyString(bool bPreventEmptyStrings) { m_bPreventEmptyString = bPreventEmptyStrings; }

protected:
	// Add a character to the end
	void			AddCharacter(wchar_t c);

	// Remove a character from the end
	void			RemoveCharacter();

	// Should the caret be rendered
    bool           IsCaretOn();

	virtual void	CalculateSize();

protected:
    ILTClient       *m_pLTClient;

	CFontInfo	m_Font;					// The font for this control
	uint8		m_nBaseFontSize;		// The font size before scaling

	CLTGUIString m_Text;

	uint32		m_nMaxLength;			// The maximum length for the string
	uint32		m_nCaretPos;
	wchar_t*	m_pszValue;		// The pointer that is updated in the UpdateData function

    bool         m_bCaretEnabled;
    uint32		m_argbCaretColor;
	float		m_fCaretTime;
	LT_POLYG4	m_Caret;
	LT_POLYG4	m_Frame[4];

	bool		m_bUseFrame;

	LTGUIInputMode		m_eInputMode;

	// Receives a message when the "enter" key is pressed.
	CLTGUICommandHandler	*m_pCommandHandler;

	FilterCallbackFn m_pFn;

	bool		m_bPreventEmptyString;
};

#endif // !defined(_LTGUIEDITCTRL_H_)