
#ifndef  _TIMEDTEXT_H_INCLUDED_
#define  _TIMEDTEXT_H_INCLUDED_

#include "CUIPolyString.h"

// When the text scrolls, the top line is "out of range" and should
// fade away.  Unfortunately there is an obstacle with the way the
// text and dropped shadowns is handled which makes this difficult.
// If we made copies of the original text passed instead of referencing
// the original, fading would work OK.  If want this (untested) ability
// uncomment the next line and fix the problem.  :(
// #define TIMED_TEXT_FADE_TOP_LINE

#define TIMED_TEXT_SOUND_NAME_LENGTH 64

typedef struct _TIMED_TEXT_INIT_STRUCT
{
	//
	// Visual Characteristics
	//
	CUIPolyString *text;			// string to display, formatted and ready to go
									//   NOTE: this class does NOT take ownership,
									//   therefore it must "stay alive" while this
									//   function needs it.
	uint32 color;					// the color of the font, note this poses as limitation
									//   that you can't have multi colored text, sorry!
	LTBOOL useDroppedShadow;		// display a dropped shadow?  LTTRUE = yes
	int numberOfLinesBeforeScroll;	// number of lines to display before starting to scroll
									// ex: if you want scrolling immediately after the first
									//    line is printed, you would specify 0.  If you wanted
									//    3 full lines to be printed to the screen before
									//    scrolling, you would specify 2.  Think of it as
									//    "2 lines will be printed without scrolling and
									//    scrolling will start after the 3rd full line is
									//    printed
	CUIRECT *clipRect;				// rectangle to use to clip (NULL to ignore it)
	
	// 
	// Timing Information
	//
	float initialDelay;				// time to wait before doing anything
	float characterDelay;			// time to pause after each new character is printed
	float lineDelay;				// time to pause after each line is completed
	float scrollTime;				// time to spend scrolling
	float completeDelay;			// time to wait after complete text is displayed
	float fadeTime;					// time to spend fading text to nothing

	//
	// Sound
	//
	char textDisplaySound[ TIMED_TEXT_SOUND_NAME_LENGTH ];	// sound to play when new character displayed (1st char NULL, no sound for this event)
	char scrollSound[ TIMED_TEXT_SOUND_NAME_LENGTH ];		// sound to play when scrolling text (1st char NULL, no sound for this event)
	
} TIMED_TEXT_INIT_STRUCT;



class CTimedText
{
private:
	//
	// Private Defines
	//
	#define CTIMEDTEXTFLAG_USE_DROPPED_SHADOW ( 0x1 << 0 )
	#define CTIMEDTEXTFLAG_USE_CLIP_RECT      ( 0x1 << 1 )
	#define CTIMEDTEXTFLAG_RUNNING            ( 0x1 << 2 )
	#define CTIMEDTEXTFLAG_DISPLAY            ( 0x1 << 3 )
	#define CTIMEDTEXTFLAG_SCROLLING          ( 0x1 << 4 )
	#define CTIMEDTEXTFLAG_ALLFLAGS ( ( TIMEDTEXT_USE_DROPPED_SHADOW ) |   \
									 ( TIMEDTEXT_USE_CLIP_RECT ) |        \
									 ( TIMEDTEXT_RUNNING ) |              \
									 ( TIMEDTEXT_SCROLLING ) |            \
									 ( TIMEDTEXT_DISPLAY ) )
	
	// testing or clearing or setting flag macros
	#define TESTFLAG( _flag_base, _flag )   ( ( _flag_base ) & ( _flag ) )
	#define SETFLAG( _flag_base, _flag )    ( ( _flag_base ) |= ( _flag ) )
	#define CLEARFLAG( _flag_base, _flag )  ( ( _flag_base ) &= ( ~( _flag ) ) )
	
	// uncomment the next line to add debugging information to the class
	#define TIMEDTEXT_DEBUG
	
public:
	~CTimedText();
	CTimedText();

	void Init( TIMED_TEXT_INIT_STRUCT const &initData );
	
	// control if we are updating
	void Start()   { SETFLAG( m_flags, CTIMEDTEXTFLAG_RUNNING );	m_prevUpdateTime = g_pLTClient->GetTime(); ASSERT( m_text );  m_fLineYPos = m_text->GetPolys()[ 0 ].verts[ 0 ].y; }
	void Pause()   { CLEARFLAG( m_flags, CTIMEDTEXTFLAG_RUNNING ); }
	void Resume()  { SETFLAG( m_flags, CTIMEDTEXTFLAG_RUNNING );	m_prevUpdateTime = g_pLTClient->GetTime(); }
	
	// control displaying the text
	void Show()    { SETFLAG( m_flags, CTIMEDTEXTFLAG_DISPLAY ); }
	void Hide()    { CLEARFLAG( m_flags, CTIMEDTEXTFLAG_DISPLAY ); }
	
	// other stuff
	void Update();
	void Render();
	
	void Clear() { InitInternalInfo(); }
	
	// want to make a change to the text?
	CUIPolyString *GetString();

private:
	//
	// Private Enumerations
	//
	typedef enum _CTimedTextState
	{
		CTIMEDTEXT_NONE,			// done nothing yet
		CTIMEDTEXT_INITIAL_DELAY,	// initial delay before doing anything
		CTIMEDTEXT_TIMING,			// still going through string
		CTIMEDTEXT_COMPLETE,		// done running, pausing before fade out
		CTIMEDTEXT_FADING,			// busy fading
		CTIMEDTEXT_FINISHED,		// finished fading, nothing to do
		
		CTIMEDTEXT_NUM_STATES	// always last
		
	} CTimedTextState;
	

	//
	// Externally specified data
	//
	
	// display
	CUIPolyString *m_text;			// string to display, formatted and ready to go
	uint32 m_color;					// color of the font
	int m_numberOfLinesBeforeScroll;// number of lines to display before starting to scroll
	CUIRECT m_clipRect;				// rectangle to use to clip text (optional)
	float m_fLineYPos;				// last knonw y position of the text (detects linefeeds)
	
	// timing
	float m_initialDelay;			// time to wait before doing anything
	float m_characterDelay;			// time to pause after each new character is printed
	float m_lineDelay;				// time to pause after each line is completed
	float m_scrollTime;				// time to spend scrolling
	float m_completeDelay;			// time to wait after complete text is displayed
	float m_fadeTime;				// time to spend fading text to nothing
	
	//  sound
	char m_textDisplaySoundName[ TIMED_TEXT_SOUND_NAME_LENGTH ];	// sound to play as characters are being displayed
	char m_scrollSoundName[ TIMED_TEXT_SOUND_NAME_LENGTH ];			// sound to play when scrolling text
	
	//
	// Internal Info:
	//
	CTimedTextState m_state;		// current state of the text display
	
	int m_flags;					// see flags under 'private defines' above
	int m_textLen;					// length of the passed in text

	float m_posX;					// screen position of the text
	float m_posY;
		
	float m_scrollRefTime;			// reference time to keep track of where in the scroll we are
	float m_scrollRefPosY;			// reference position to scroll text relative to
	
	// time tracking
	float m_timeElapsed;			// amount of time spent "running" text (relative)
		// means different things depending on the state
		//	none, means nothing
		//	timing means time elapsed since we started timing	
		//	complete means time elapsed since we were complete
		//  fading means time elapsed since we started fading
		//	finished, elapsed undefined	
		
	union _m_uTime
	{
		float m_timeElapseda;		// amount of time spent "running" text (relative)
		float m_timeCompleted;		// amount of time since the text was completed
	} m_uTime;
	float m_prevUpdateTime;			// absolute time of previous update 
									//    (-1 if no previous update yet)
	
	// character tracking (what we have seen to compute where we should be)
	// NOTE: max of 65535 characters
	uint16 m_charactersProcessed;	// number of printable characters displayed so far
	uint8 m_linesProcessed;			// number of lines processed
	uint8 m_nNewlinesProcessed;		// number of newlines '\n' encountered in the text
		
	// sound handles
	HLTSOUND m_hTextDisplaySound;	// sound to play when new character displayed
	
#ifdef TIMED_TEXT_FADE_TOP_LINE
	// references for fading the top line
	int m_nLineFadeCharStart = 0;
	int m_nLineFadeCharEnd = 0;  // noninclusive
#endif // TIMED_TEXT_FADE_TOP_LINE

	//
	// Internal Interfaces
	//
	void InitInternalInfo();
};



#endif //_TimedText_h_INCLUDED_
