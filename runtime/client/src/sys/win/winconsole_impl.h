//------------------------------------------------------------------
//	FILE	  : console_impl.h
//	PURPOSE	  : Defines the console implementation class.
//	CREATED	  : November 4 1999
// ------------------------------------------------------------------ //

#ifndef __WINCONSOLE_IMPL_H__
#define __WINCONSOLE_IMPL_H__

#ifndef __CONSOLE_H__
#include "console.h"
#endif

#ifndef __SYSLTHREAD_H__
#include "syslthread.h"
#endif

// Forward declarations
class CConsole;
class CUIFont;
class SharedTexture;
class CUIPolyString;

// The global console object
#define GETCONSOLE() (&g_Console)
extern CConsole g_Console;

#define MAX_CONSOLE_TEXTLEN		256
#define NUM_CONSOLE_CHARACTERS	127

#define CONSOLE_LEFT_BORDER	8
#define CONSOLE_TOP_BORDER	5

// One line of text
class CConTextLine : public CGLLNode
{
public:

	CONCOLOR	m_Color;
	char		m_Text[MAX_CONSOLE_TEXTLEN];

};

// A list of text lines
typedef CGLinkedList<CConTextLine*> CConTextList;

// The command box of the console
class CConCommandBox
{
protected:
	CConsole	*m_pConsole;

	char	TranslateKey( uint32 key ) const;

	int		m_iCurLength;
	char	m_CurCommand[MAX_CONSOLE_TEXTLEN + 1];

	CONCOLOR	m_TextColor;
	CONCOLOR	m_BackColor, m_BorderColor;

	LTRect	m_Rect;

	uint32	m_FlashTime;

	int		m_iCursorPos;

	void	MoveCursor( int iOffset );
	void	MoveWord( int iOffset );
	void	AddChar( char key );
	void	DeleteChar( int iOffset );
	void	DeleteWord( int iOffset );

public:

	// Construction/destruction

	CConCommandBox();
	~CConCommandBox() {};
	
	void	Init() {};
	void	Term();

	// Member access

	LTRect	GetRect() const { return m_Rect; };
	void	SetRect( LTRect cRect ) { m_Rect = cRect; };

	const char*		GetCommand() { return &m_CurCommand[0]; };
	void			SetCommand( const char *pCommand );

	CONCOLOR	GetTextColor() const { return m_TextColor; };
	void		SetTextColor( CONCOLOR cTextColor ) { m_TextColor = cTextColor; };

	CONCOLOR	GetBackColor() const { return m_BackColor; };
	void		SetBackColor( CONCOLOR cColor ) { m_BackColor = cColor; };

	CONCOLOR	GetBorderColor() const { return m_BorderColor; };
	void		SetBorderColor( CONCOLOR cColor ) { m_BorderColor = cColor; };

	CConsole*	GetConsole() const { return m_pConsole; };
	void		SetConsole( CConsole *pConsole ) { m_pConsole = pConsole; };

	uint32	GetFlashTime() const { return m_FlashTime; };
	void	SetFlashTime( uint32 dwTime ) { m_FlashTime = dwTime; };

	// Command manipulation
	void	Clear();

	// Rendering
	void	Draw();

	// Event handlers
	void	OnKeyPress( uint32 key );

};

// Tab-completion iterator base class
class CConIterator
{
protected:
	// Jumps to the beginning of the list
	//	Returns false if the list is empty
	virtual bool	Begin() { return false; };
	// Jumps to the end of the list
	//	Returns false if the list is empty
	virtual bool	End() { return false; };
	// Moves to the next item
	//	Returns false if past the end of the list
	virtual bool	NextItem() { return false; };
	// Moves to the previous item
	//	Returns false if past the beginning of the list
	virtual bool	PrevItem() { return false; };

public:
	CConIterator() {};
	virtual ~CConIterator() {};

	// Searches for the first instance of pValue in the list
	//	Returns false if the value was not found
	virtual bool	First( const char *pValue = LTNULL );
	// Searches for the last instance of pValue in the list
	//	Returns false if the value was not found
	virtual bool	Last( const char *pValue = LTNULL );
	// Moves to the next item in the list (pValue == LTNULL will move to the next item independant of matching)
	//	Returns false if past the end of the list
	virtual bool	Next( const char *pValue = LTNULL );
	// Moves to the previous item in the list (pValue == LTNULL will move to the previous item independant of matching)
	//	Returns false if past the beginning of the list
	virtual bool	Prev( const char *pValue = LTNULL );

	// Gets the current item in the list
	//	Returns LTNULL if outside of the bounds of the list
	virtual const char*		Get() const { return LTNULL; };
};


// ------------------------------------------------------------------ //
// Command history tracker for the console
// ------------------------------------------------------------------ //
class CConHistory : public CConIterator
{
protected:
	CConTextLine*	m_pLines;
	int		m_iIndex;
	int		m_iListSize;
	int		m_iListStart, m_iListEnd;

	// Parent class overrides
	virtual bool	Begin();
	virtual bool	End();
	virtual bool	NextItem();
	virtual bool	PrevItem();

protected:
	// Remove all instances of an entry from the history list
	// Returns : The number of entries removed
	virtual int		Remove( const char *pString );
	// Remove a specific entry from the history list
	virtual void	Remove( int iIndex );

public:
	CConHistory( int iListSize = 0 );
	virtual ~CConHistory();

	// Member access
	int		GetSize() const { return (m_iListSize) ? (m_iListSize - 1) : 0; };
	int		GetIndex() const;

	// Add a line to the history list
	void	Add( const char *pString, bool bUnique = true );
	// Resize the history list
	bool	Resize( int iSize );
	// Clear the history list
	void	Clear() { m_iListStart = m_iListEnd = m_iIndex = 0; };

	// Jump to a specific line
	bool	MoveTo( int iIndex );

	// Parent class overrides
	virtual const char*		Get() const;
};

// The actual console class
class CConsole
{
private:
	bool m_bInitialized;
	// This variable makes sure Term() isn't called from Init unless Init has been called un-paired
	bool m_bInitTerminate;

	// Setting this variable to true turns off the variable masking to avoid saving variables that
	//	haven't been loaded
	bool m_bSaveVariablesMask;

protected:

	enum EConState {
		STATE_NORMAL = 0,
		STATE_COMPLETE = 1,
		STATE_HISTORY = 2
	};

	EConState	m_eState;

	EConState	GetState() const { return m_eState; };
	void		SetState( EConState eState ) { m_eState = eState; };

	CConCommandBox	m_CommandBox;

	uint32	m_OutputFlags;
	int		m_FilterLevel;

	bool	InitFont();
	void	TermFont();

	void	FreeBackground();
	
	inline bool		GetTextLineBox( uint32 iLine, LTRect *pRect, bool bScreen = false );

	void	FinishCommand();
	void	CheckVariables();

	// Command navigation mode
	void	StartNav( EConState eState );
	void	EndNav();
	char	m_aNavCommand[MAX_CONSOLE_TEXTLEN];

	// Internal support for command completion
	void	NextCommand();
	void	PrevCommand();
	void	MatchCommands();

	uint32	m_BackColor, m_BorderColor;

	LCriticalSection	m_CS;

	// The structure used for buffer stuff.
	RenderStruct*	m_pStruct;

	LTRect	m_Rect, m_ScrRect;
	// Fill a rectangle with the calculated output rectangle
	void	CalcRect( LTRect &cRect );

	// The font (a 32-bit packed monochrome bitmap, m_FullFontHeight uint32's per character).
	LTRect	m_FontRect;
	uint32	*m_pFontBitmapData;

	// The 'ascent' part of the font. This is used for layout calculations.
	uint16	m_FontHeight;

	// The full font height.. Used for drawing.
	uint16	m_FullFontHeight;

	uint16	m_CharWidths[NUM_CONSOLE_CHARACTERS];
	
	HSURFACE	m_hBackground;
	float		m_fBackgroundAlpha;
	bool		m_bBackgroundOptimized;

	// Error log function.
	ErrorLogFn	m_ErrorLogFn;

	// The window it uses to get the DC and stuff.
	HWND	m_hWnd;

	CConTextList	m_TextLines;
	uint32			m_nTextLines;
	int				m_iScrollOffset;

	// The doskey-ish text command queue.
	CConHistory		m_cHistory;

	// The command callback handler
	CommandHandler	m_CommandHandler;

	// The command completion iterator
	CConIterator	*m_pCompletionIterator;

	CUIFont*		m_Font;		// Using the CUI font mgr for printing text...
	SharedTexture*	m_Tex;

public:

	CConsole();			
	~CConsole();

	bool Init( const LTRect *pRect, CommandHandler handler, RenderStruct *pStruct, CConIterator *pCompletionIterator = LTNULL );
	bool InitBare();
	void	Term( bool bDeleteTextLines = true );

	LTRESULT	LoadBackground();

	// Member access
	CConCommandBox*		GetCommandBox() { return &m_CommandBox; }
	
	int		GetFilterLevel() const { return m_FilterLevel; };
	void	SetFilterLevel( int iFilterLevel ) { m_FilterLevel = iFilterLevel; };

	uint32	GetBackColor() const { return m_BackColor; };
	void	SetBackColor( uint32 cColor ) { m_BackColor = cColor; };

	uint32	GetBorderColor() const { return m_BorderColor; };
	void	SetBorderColor( uint32 cColor ) { m_BorderColor = cColor; };

	float	GetBackgroundAlpha() const { return m_fBackgroundAlpha; };
	void	SetBackgroundAlpha( float fValue );

	RenderStruct*	GetRenderStruct() const { return m_pStruct; };
	void			SetRenderStruct( RenderStruct *pStruct ) { m_pStruct = pStruct; };

	CUIFont* GetFont() const { return m_Font; }

	LTRect	GetFontRect() const { return m_FontRect; };
	void	SetFontRect( LTRect cRect ) { m_FontRect = cRect; };

	void*	GetFontBitmapData() const { return (void *)m_pFontBitmapData; };
	void	SetFontBitmapData( void *pBitmapData ) { m_pFontBitmapData = (uint32 *)pBitmapData; };

	uint16	GetFontHeight() const { return m_FontHeight; };
	void	SetFontHeight( uint16 wHeight ) { m_FontHeight = wHeight; };

	uint16	GetFullFontHeight() const { return m_FullFontHeight; };
	void	SetFullFontHeight( uint16 wHeight ) { m_FullFontHeight = wHeight; };

	uint16	GetCharWidth( char cChar ) { return (cChar < NUM_CONSOLE_CHARACTERS) ? m_CharWidths[cChar] : 0; };
	void	SetCharWidths( const uint16 *pWidths ) { memcpy( m_CharWidths, pWidths, sizeof(uint16) * NUM_CONSOLE_CHARACTERS ); }; 
	const uint16*	GetCharWidths() const { return &m_CharWidths[0]; };

	HSURFACE	GetBackground() const { return m_hBackground; };
	void		SetBackground( HSURFACE hBackground ) { m_hBackground = hBackground; };

	ErrorLogFn	GetErrorLogFn() const { return m_ErrorLogFn; };
	void		SetErrorLogFn( ErrorLogFn fn ) { m_ErrorLogFn = fn; };

	// Note:  m_Rect is on the SCREEN!  The buffer is the exact size of m_Rect
	//        (thus, its origin for operations is at 0,0).
	LTRect	GetRect() const { return m_Rect; };
	void	SetRect( const LTRect &cRect );

	HWND	GetWnd() const { return m_hWnd; };
	void	SetWnd( HWND hWnd ) { m_hWnd = hWnd; };

	// All the text lines and how many it actually can display in its rectangle.
	const CConTextList*		GetTextLines() const { return &m_TextLines; };
	uint32	GetVisibleTextLines() const { return m_nTextLines; };
	bool ResizeTextLines( uint32 nNewSize );

	CConHistory*	GetCommandHistory() { return &m_cHistory; };
	bool ResizeHistory( int iNewSize );

	CommandHandler	GetCommandHandler() const { return m_CommandHandler; };
	void			SetCommandHandler( CommandHandler fHandler ) { m_CommandHandler = fHandler; };

	int		GetScrollOffset() const { return m_iScrollOffset; };

	bool		GetInitialized()	{ return m_bInitTerminate; }

	//////////////////////////////////////////////////////
	// Filtering

	bool	FilterAction( int iFilterLevel ) { return iFilterLevel > m_FilterLevel; };

	//////////////////////////////////////////////////////
	// Drawing routines

	// Filled rectangle with a border
	void	BorderedRectangle( uint32 fillColor, uint32 borderColor, LTRect rect );

	// Returns the length of the string in pixels
	int		CalcStringOffset( const char *pStr, int iLength = -1 );

	// Draws a line of text in the provided rectangle and color
	void	DrawTextLine( const char *pText, const LTRect *pRect, COLORREF textColor );

	// Draws a specified number of the text lines
	void	DrawTextLines( int nTextLines, bool bScreen = false );

	// Draws the entire contents of the console
	void	Draw();

	// Draws a number of lines in "small" mode
	void	DrawSmall( int nLines );

	// Print a string in a given color as long as the filter level is not too high
	void	PrintString( CONCOLOR theColor, int filterLevel, const char *pMsg );
	void	vPrintf( CONCOLOR theColor, int filterLevel, const char *pMsg, va_list vaArgs );
	void	Printf( CONCOLOR theColor, int filterLevel, const char *pMsg, ... );

	//////////////////////////////////////////////////////
	// Scrolling
	int		Scroll( int iOffset );

	//////////////////////////////////////////////////////
	// Command access

	// Cycle through the recent command list by the given amount
	void	CycleCommands( int iCount );

	// Tab completion iterator
	CConIterator*	GetCompletionIterator() { return m_pCompletionIterator; };
	void			SetCompletionIterator( CConIterator *pIterator );
		
	//////////////////////////////////////////////////////
	// Event handling

	void	OnKeyPress( uint32 key );

};

#endif //__CONSOLE_IMPL_H__
